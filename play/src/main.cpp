/// @file main.cpp
/// riftbound_play — one interactive game in the browser.
///
/// Seats are human (played from the web board) or an engine agent
/// (random, mcts:sims=N, ismcts:sims=N). The engine itself lives in
/// riftbound_core and knows nothing about this program; batch games and
/// AI-vs-AI runs use the engine's own `riftbound` binary.
///
/// Open http://127.0.0.1:8080/ for the board (it picks the human seat),
/// add ?seat=p1|p2|spectator|god to choose, or /classic for the older
/// text page. Wire protocol: docs/play-api.md.

#include "human_agent.h"
#include "play_server.h"

#include "agents/mcts_agent.h"
#include "agents/random_agent.h"
#include "cards/card_registry.h"
#include "core/card_db.h"
#include "core/events.h"
#include "core/version.h"
#include "engine/game_engine.h"
#include "io/replay_writer.h"
#include "io/state_renderer.h"
#include "rules/deck_validator.h"

#include <boost/program_options.hpp>

#include <atomic>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>

#ifndef RIFTBOUND_PLAY_WEB_ROOT
#define RIFTBOUND_PLAY_WEB_ROOT "play/web"
#endif

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace riftbound;

namespace {

struct AgentSpec {
    std::string raw;
    std::string kind;  // human, random, mcts, ismcts
    int sims = 0;
};

AgentSpec parseAgentSpec(const std::string& s) {
    AgentSpec out;
    out.raw = s;
    auto colon = s.find(':');
    out.kind = colon == std::string::npos ? s : s.substr(0, colon);
    if (out.kind != "human" && out.kind != "random" && out.kind != "mcts" && out.kind != "ismcts")
        throw std::runtime_error("Unknown agent '" + out.kind +
                                 "'. Expected human, random, mcts:sims=N or ismcts:sims=N.");
    if (colon != std::string::npos) {
        std::stringstream ss(s.substr(colon + 1));
        for (std::string kv; std::getline(ss, kv, ',');)
            if (kv.rfind("sims=", 0) == 0) out.sims = std::stoi(kv.substr(5));
    }
    if ((out.kind == "mcts" || out.kind == "ismcts") && out.sims <= 0)
        throw std::runtime_error("Agent '" + out.kind + "' needs sims=N, e.g. " + out.kind + ":sims=50.");
    return out;
}

/// `game_seed` must be the engine's runGame seed: MCTS agents replay the
/// game in their own OpenSpiel state and need the same shuffles.
std::unique_ptr<AgentInterface> buildAgent(const AgentSpec& spec, uint64_t game_seed,
                                           uint64_t derived_seed, const std::string& deck1,
                                           const std::string& deck2, HumanAgent** human_out) {
    const std::string registry = "cards/registry.json";  // ignored by the agents
    if (spec.kind == "human") {
        auto a = std::make_unique<HumanAgent>();
        *human_out = a.get();
        return a;
    }
    if (spec.kind == "random") return std::make_unique<RandomAgent>(derived_seed);
    if (spec.kind == "mcts")
        return std::make_unique<MctsAgent>(deck1, deck2, registry, game_seed, derived_seed, spec.sims);
    return std::make_unique<IsMctsAgent>(deck1, deck2, registry, game_seed, derived_seed, spec.sims);
}

std::string timestampDirName() {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream s;
    s << std::put_time(&tm, "%Y%m%d-%H%M%S");
    return s.str();
}

}  // namespace

int main(int argc, char* argv[]) {
    po::options_description desc(
        "riftbound_play — play Riftbound in the browser\n"
        "\n"
        "  ./riftbound_play --deck1 decks/darius_test.txt --deck2 decks/annie_test.txt\n"
        "      (you are P1 against the random agent; open http://127.0.0.1:8080/)\n"
        "  ./riftbound_play --deck1 ... --deck2 ... --agent2 human\n"
        "      (two humans: open ?seat=p1 and ?seat=p2 in two windows)\n"
        "\n"
        "Flags");
    desc.add_options()
        ("help,h", "Show help")
        ("deck1", po::value<std::string>()->required(), "Player 1 deck list (.txt)")
        ("deck2", po::value<std::string>()->required(), "Player 2 deck list (.txt)")
        ("agent1", po::value<std::string>()->default_value("human"),
         "Player 1: human | random | mcts:sims=N | ismcts:sims=N")
        ("agent2", po::value<std::string>()->default_value("random"),
         "Player 2: human | random | mcts:sims=N | ismcts:sims=N")
        ("port,p", po::value<uint16_t>()->default_value(8080), "TCP port")
        ("bind", po::value<std::string>()->default_value("127.0.0.1"),
         "Bind address (0.0.0.0 accepts connections from any interface)")
        ("god-mode", po::value<std::string>()->default_value("on"),
         "Allow the god seat (sees everything, can edit the state): on | off")
        ("web-root", po::value<std::string>()->default_value(RIFTBOUND_PLAY_WEB_ROOT),
         "Directory with index.html, board.js and board.css")
        ("seed,s", po::value<uint64_t>()->default_value(0), "RNG seed (0 = random)")
        ("render-html", po::value<std::string>()->default_value("on"),
         "Write an HTML replay when the game ends: on | off")
        ("replay-dir", po::value<std::string>()->default_value("replays"),
         "Replays go to <dir>/<timestamp>/replay.html")
        ("wild", po::bool_switch()->default_value(false), "Allow banned cards")
    ;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) { std::cout << desc << "\n"; return 0; }
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << "\n\n" << desc << std::endl;
        return 1;
    }

    const auto deck1_path = vm["deck1"].as<std::string>();
    const auto deck2_path = vm["deck2"].as<std::string>();
    AgentSpec spec1, spec2;
    try {
        spec1 = parseAgentSpec(vm["agent1"].as<std::string>());
        spec2 = parseAgentSpec(vm["agent2"].as<std::string>());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    const auto god_arg = vm["god-mode"].as<std::string>();
    if (god_arg != "on" && god_arg != "off") {
        std::cerr << "Error: --god-mode must be on or off\n";
        return 1;
    }
    const auto web_root = vm["web-root"].as<std::string>();
    if (!fs::exists(fs::path(web_root) / "index.html")) {
        std::cerr << "Error: no index.html in --web-root " << web_root << "\n";
        return 1;
    }

    std::cout << "Riftbound " << kVersionTag << " (built " << kBuildDate << ")\n";

    CardDB card_db;
    CardRegistry card_registry;
    card_registry.loadAll();
    card_db.buildFromClasses(card_registry);

    DeckValidator validator(card_db);
    validator.setWild(vm["wild"].as<bool>());
    DeckSubmission deck1, deck2;
    try {
        deck1 = DeckValidator::loadFromDeckList(deck1_path, card_db);
        deck2 = DeckValidator::loadFromDeckList(deck2_path, card_db);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load deck: " << e.what() << "\n";
        return 1;
    }
    auto v1 = validator.validate(deck1);
    auto v2 = validator.validate(deck2);
    if (!v1.is_legal || !v2.is_legal) {
        std::cerr << "Deck validation failed.\n";
        for (auto& e : v1.errors) std::cerr << "  P1: " << e << "\n";
        for (auto& e : v2.errors) std::cerr << "  P2: " << e << "\n";
        return 1;
    }

    // OpenSpiel registers the seed as a 31-bit int; truncate once so MCTS
    // agents and the live engine use the same value.
    const uint64_t seed = vm["seed"].as<uint64_t>();
    const uint64_t game_seed = (seed == 0 ? std::random_device{}() : seed) & 0x7FFFFFFFu;
    std::cout << "Seed: " << game_seed << "\n"
              << "P1: " << card_db.get(deck1.legend).name << "  (" << spec1.raw << ")\n"
              << "P2: " << card_db.get(deck2.legend).name << "  (" << spec2.raw << ")\n";

    HumanAgent* p1_human = nullptr;
    HumanAgent* p2_human = nullptr;
    std::unique_ptr<AgentInterface> agent1, agent2;
    try {
        agent1 = buildAgent(spec1, game_seed, game_seed * 2, deck1_path, deck2_path, &p1_human);
        agent2 = buildAgent(spec2, game_seed, game_seed * 2 + 1, deck1_path, deck2_path, &p2_human);
    } catch (const std::exception& e) {
        std::cerr << "Failed to build agents: " << e.what() << "\n";
        return 1;
    }

    EventBus events;
    GameEngine engine(card_db, events, card_registry);
    StateRenderer renderer(card_db);
    renderer.show_hand = true;  // used only for the god seat's text view

    std::unique_ptr<ReplayWriter> replay;
    std::string replay_path;
    if (vm["render-html"].as<std::string>() == "on") {
        const auto dir = (fs::path(vm["replay-dir"].as<std::string>()) / timestampDirName()).string();
        fs::create_directories(dir);
        replay_path = dir + "/replay.html";
        replay = std::make_unique<ReplayWriter>(card_db, replay_path);
        replay->setGameHeader("seed " + std::to_string(game_seed) + " | P1=" + spec1.raw +
                              " | P2=" + spec2.raw);
        events.on_log.connect([rp = replay.get()](const LogEvent& e) {
            const char* prefix = "[TRC]";
            if (e.level == LogLevel::Debug)   prefix = "[DBG]";
            if (e.level == LogLevel::Info)    prefix = "[INF]";
            if (e.level == LogLevel::Warning) prefix = "[WRN]";
            rp->addTraceLine(std::string(prefix) + " " + e.message);
        });
        events.on_phase_changed.connect([rp = replay.get(), &renderer, &engine](const PhaseChangedEvent& e) {
            rp->recordPhaseSeam(engine.state(), e.old_phase, e.new_phase, renderer);
        });
        engine.on_decision = [rp = replay.get(), &renderer](const GameState& s,
                                                             const std::vector<Intent>& actions,
                                                             const Intent& chosen) {
            rp->recordDecision(s, actions, chosen, renderer);
        };
    }

    PlayServerConfig cfg;
    cfg.port = vm["port"].as<uint16_t>();
    cfg.bind_address = vm["bind"].as<std::string>();
    cfg.god_mode_enabled = god_arg == "on";
    cfg.p1_human = p1_human;
    cfg.p2_human = p2_human;
    cfg.version_tag = kVersionTag;
    cfg.build_date = kBuildDate;
    cfg.seed = game_seed;
    cfg.web_root = web_root;
    static std::atomic<bool> stop_requested{false};
    cfg.on_shutdown_signal = [p1_human, p2_human] {
        stop_requested = true;
        if (p1_human) p1_human->abort();
        if (p2_human) p2_human->abort();
    };
    PlayServer server(engine, renderer, card_db, std::move(cfg));
    const std::string url = server.start();
    std::cout << "\n  -> Open " << url << "/ in your browser to play.\n\n";

    // The god seat's log panel shows every engine line.
    events.on_log.connect([&server](const LogEvent& e) {
        const char* level = "TRC";
        if (e.level == LogLevel::Debug)   level = "DBG";
        if (e.level == LogLevel::Info)    level = "INF";
        if (e.level == LogLevel::Warning) level = "WRN";
        server.broadcastLog(level, e.message);
    });

    std::thread game_thread([&] {
        try {
            auto result = engine.runGame(deck1, deck2, *agent1, *agent2, game_seed);
            std::cout << "\nGame over: winner=" << static_cast<int>(result.winner)
                      << " | reason=" << result.termination_reason
                      << " | turns=" << result.total_turns << "\n";
            if (replay) {
                replay->addTraceLine("[TRC] GAME_OVER: " + result.termination_reason);
                replay->writeHtml();
                std::cout << "Replay saved to " << replay_path << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[engine] exception: " << e.what() << "\n";
        }
    });
    game_thread.join();
    // Keep serving the final board until the user stops the program,
    // also for AI-only games, so spectators can still look at it.
    if (!stop_requested) std::cout << "Game finished. Press Ctrl-C to stop the server.\n";
    while (!stop_requested) std::this_thread::sleep_for(std::chrono::milliseconds(200));
    server.stop();
    return 0;
}
