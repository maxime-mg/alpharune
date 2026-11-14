/// @file test_review_fix_io_cli.cpp
/// Regression tests for plan 04 (play server, HTML output, runners).
///
/// The play-server tests (play/tests/test_play_server_protocol.cpp) run a real game (GameEngine::runGame on its own
/// thread, a HumanAgent in seat 1) behind a real PlayServer on an ephemeral
/// loopback port, and talk to it over a WebSocket exactly as the browser
/// does.

#include "io/batch_report.h"
#include "io/replay_writer.h"
#include "io/state_editor.h"
#include "io/state_renderer.h"

#include "agents/random_agent.h"
#include "cards/card_registry.h"
#include "engine/batch_runner.h"
#include "rules/deck_validator.h"

#include <gtest/gtest.h>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>

// Unity builds merge file scopes: everything here lives in this file's namespace.
namespace riftbound::test::review_fix_io_cli {

using namespace std::chrono_literals;
namespace beast = boost::beast;
namespace http  = beast::http;
namespace ws    = beast::websocket;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;
using json      = nlohmann::json;

namespace {

// ─── Shared card data ────────────────────────────────────────────────────────

struct Cards {
    CardRegistry registry;
    CardDB db;
    DeckSubmission deck1, deck2;

    Cards() {
        registry.loadAll();
        db.buildFromClasses(registry);
        const char* env = std::getenv("RIFTBOUND_ROOT");
        const std::string root = env ? env : ".";
        deck1 = DeckValidator::loadFromDeckList(root + "/decks/fiora.txt", db);
        deck2 = DeckValidator::loadFromDeckList(root + "/decks/yi_rebuild.txt", db);
    }
};

Cards& cards() {
    static Cards c;
    return c;
}

// ─── A seat-2 AI the test can freeze mid-decision ────────────────────────────
// While `hold` is set the engine thread blocks inside selectAction: the
// engine is "busy" (not parked for the server) for as long as the test likes.


// ─── WebSocket test client ───────────────────────────────────────────────────


// ─── A live game behind a live server ────────────────────────────────────────



std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int countOf(const std::string& hay, const std::string& needle) {
    int n = 0;
    for (auto pos = hay.find(needle); pos != std::string::npos;
         pos = hay.find(needle, pos + needle.size())) ++n;
    return n;
}

} // namespace

// ─── 4.8 — god-mode moves keep the state consistent ──────────────────────────

namespace {

GameState editorState() {
    GameState s{};
    BattlefieldState bf;
    bf.id = BattlefieldId::fromRaw(0);
    s.battlefields.push_back(bf);
    auto add = [&](GameObjectId id, PlayerId who, ZoneType zone) -> GameObject& {
        GameObject o{};
        o.id = id;
        o.owner = who;
        o.controller = who;
        o.zone = zone;
        return s.objects[id] = o;
    };
    // 1: P2's unit at the battlefield, carrying gear 3.
    auto& unit = add(GameObjectId::fromRaw(1), PlayerId::Player2, ZoneType::BattlefieldZone);
    unit.location = BattlefieldLocation{0};
    unit.attachments = {GameObjectId::fromRaw(3)};
    auto& gear = add(GameObjectId::fromRaw(3), PlayerId::Player2, ZoneType::BattlefieldZone);
    gear.location = BattlefieldLocation{0};
    gear.attached_to = GameObjectId::fromRaw(1);
    // 2: P1's facedown card at the battlefield.
    auto& hidden = add(GameObjectId::fromRaw(2), PlayerId::Player1, ZoneType::FacedownZone);
    hidden.is_hidden = true;
    s.battlefields[0].facedown.push_back(GameObjectId::fromRaw(2));
    // 4: a spell on the chain.
    add(GameObjectId::fromRaw(4), PlayerId::Player1, ZoneType::Chain);
    // 5: a card in P1's hand.
    add(GameObjectId::fromRaw(5), PlayerId::Player1, ZoneType::Hand);
    s.player(PlayerId::Player1).hand.push_back(GameObjectId::fromRaw(5));
    return s;
}

bool contains(const std::vector<GameObjectId>& v, GameObjectId id) {
    return std::find(v.begin(), v.end(), id) != v.end();
}

} // namespace

TEST(StateEditorReviewFix, MovingAFacedownCardTakesItOffTheBattlefield) {
    GameState s = editorState();
    StateEditor ed;
    ASSERT_TRUE(ed.moveObject(s, GameObjectId::fromRaw(2), PlayerId::Player1, ZoneType::Trash).ok);
    EXPECT_TRUE(s.battlefields[0].facedown.empty());   // was left behind as an orphan
    EXPECT_FALSE(s.getObject(GameObjectId::fromRaw(2)).is_hidden);
    EXPECT_TRUE(contains(s.player(PlayerId::Player1).trash, GameObjectId::fromRaw(2)));
}

TEST(StateEditorReviewFix, MovingAUnitOffBoardDropsItsGearAndKeepsOwnership) {
    GameState s = editorState();
    StateEditor ed;
    // The UI sends the unit's own owner, not the battlefield's controller.
    ASSERT_TRUE(ed.moveObject(s, GameObjectId::fromRaw(1), PlayerId::Player2, ZoneType::Hand).ok);
    const auto& unit = s.getObject(GameObjectId::fromRaw(1));
    EXPECT_EQ(unit.owner, PlayerId::Player2);
    EXPECT_EQ(unit.controller, PlayerId::Player2);
    EXPECT_TRUE(contains(s.player(PlayerId::Player2).hand, GameObjectId::fromRaw(1)));
    EXPECT_FALSE(contains(s.player(PlayerId::Player1).hand, GameObjectId::fromRaw(1)));
    EXPECT_TRUE(unit.attachments.empty());
    const auto& gear = s.getObject(GameObjectId::fromRaw(3));
    EXPECT_FALSE(gear.attached_to.has_value());
    EXPECT_EQ(gear.zone, ZoneType::Base);
    ASSERT_TRUE(gear.location.has_value());
    EXPECT_EQ(*gear.location, LocationId{BaseLocation{PlayerId::Player2}});
}

TEST(StateEditorReviewFix, GivingACardToTheOtherPlayerChangesOwnerAndController) {
    GameState s = editorState();
    StateEditor ed;
    ASSERT_TRUE(ed.moveObject(s, GameObjectId::fromRaw(5), PlayerId::Player2, ZoneType::Hand).ok);
    EXPECT_EQ(s.getObject(GameObjectId::fromRaw(5)).owner, PlayerId::Player2);
    EXPECT_EQ(s.getObject(GameObjectId::fromRaw(5)).controller, PlayerId::Player2);
    EXPECT_FALSE(contains(s.player(PlayerId::Player1).hand, GameObjectId::fromRaw(5)));
    EXPECT_TRUE(contains(s.player(PlayerId::Player2).hand, GameObjectId::fromRaw(5)));
}

TEST(StateEditorReviewFix, MovingAttachedGearDetachesItFromItsUnit) {
    GameState s = editorState();
    StateEditor ed;
    ASSERT_TRUE(ed.moveObject(s, GameObjectId::fromRaw(3), PlayerId::Player2, ZoneType::Trash).ok);
    EXPECT_TRUE(s.getObject(GameObjectId::fromRaw(1)).attachments.empty());
    EXPECT_FALSE(s.getObject(GameObjectId::fromRaw(3)).attached_to.has_value());
}

TEST(StateEditorReviewFix, ChainAndFacedownMovesAreRefused) {
    GameState s = editorState();
    StateEditor ed;
    // To a zone that needs a container entry the editor cannot build.
    EXPECT_FALSE(ed.moveObject(s, GameObjectId::fromRaw(5), PlayerId::Player1, ZoneType::Chain).ok);
    EXPECT_FALSE(ed.moveObject(s, GameObjectId::fromRaw(5), PlayerId::Player1, ZoneType::FacedownZone).ok);
    EXPECT_EQ(s.getObject(GameObjectId::fromRaw(5)).zone, ZoneType::Hand);
    EXPECT_TRUE(contains(s.player(PlayerId::Player1).hand, GameObjectId::fromRaw(5)));
    // Out of the chain, which would keep an item pointing at it.
    EXPECT_FALSE(ed.moveObject(s, GameObjectId::fromRaw(4), PlayerId::Player1, ZoneType::Hand).ok);
    EXPECT_FALSE(ed.moveObjectToBattlefield(s, GameObjectId::fromRaw(4), BattlefieldId::fromRaw(0)).ok);
    EXPECT_EQ(s.getObject(GameObjectId::fromRaw(4)).zone, ZoneType::Chain);
}

TEST(StateEditorReviewFix, SetPhaseRejectsValuesOutsideTheEnum) {
    GameState s = editorState();
    StateEditor ed;
    EXPECT_FALSE(ed.setPhase(s, -1).ok);
    EXPECT_FALSE(ed.setPhase(s, static_cast<int>(TurnPhase::GameOver) + 1).ok);
    EXPECT_FALSE(ed.setPhase(s, 300).ok);   // would wrap into range as a uint8
    EXPECT_TRUE(ed.setPhase(s, static_cast<int>(TurnPhase::MainPhase)).ok);
    EXPECT_EQ(s.turn.phase, TurnPhase::MainPhase);
}

TEST(StateRendererReviewFix, LongOptionLabelsAreNotTruncated) {
    GameState s{};
    StateRenderer renderer(cards().db);
    const std::string common(45, 'x');
    std::vector<Intent> options;
    for (const char* tail : {"-deal 2 damage", "-draw 2 cards"}) {
        Intent it{};
        it.type = IntentType::MakeChoice;
        it.player = PlayerId::Player1;
        it.chosen_value = static_cast<int>(options.size());
        it.choice_label = common + tail;
        options.push_back(it);
    }
    const std::string text = renderer.renderActions(s, options);
    // Both labels share their first 45 characters; the old 38-column cap
    // rendered them identically.
    EXPECT_NE(text.find(common + "-deal 2 damage"), std::string::npos);
    EXPECT_NE(text.find(common + "-draw 2 cards"), std::string::npos);
}

TEST(HtmlOutputReviewFix, ReplayStringsCannotCloseTheScriptBlock) {
    const auto dir = std::filesystem::path(testing::TempDir());
    auto write = [&](const std::string& name, const std::string& header) {
        const std::string path = (dir / name).string();
        ReplayWriter w(cards().db, path);
        w.setGameHeader(header);
        w.addTraceLine("[TRC] " + header);
        w.writeHtml();
        return readFile(path);
    };
    const std::string a = write("io_cli_plain.html", "seed 1");
    const std::string b = write("io_cli_hostile.html", "x</script><script>alert(1)</script>");
    ASSERT_FALSE(a.empty());
    EXPECT_EQ(countOf(b, "</script"), countOf(a, "</script"));
    EXPECT_NE(b.find("x\\x3c/script>"), std::string::npos);
}

TEST(HtmlOutputReviewFix, CardImageMapIsLoadedOnce) {
    const auto& a = ReplayWriter::loadCardImageMap();
    const auto& b = ReplayWriter::loadCardImageMap();
    EXPECT_EQ(&a, &b);
}

// ─── 4.14 / 4.18 / 4.19 — batch and game runner ──────────────────────────────

TEST(BatchRunnerReviewFix, ExceptionInAWorkerIsRethrownNotFatal) {
    GameConfig cfg;
    cfg.base_seed = 7;
    cfg.agent_factory = [](int, uint64_t) -> std::unique_ptr<AgentInterface> {
        throw std::runtime_error("agent factory failed");
    };
    AggregateResults results;
    BatchRunner batch(cards().db, cards().registry, 3);
    // Used to escape the pool task and std::terminate the process.
    try {
        batch.runBatch(cards().deck1, cards().deck2, 6, cfg, results);
        FAIL() << "expected the worker's exception";
    } catch (const std::runtime_error& e) {
        // Names the game and its seed, then the cause.
        const std::string what = e.what();
        EXPECT_NE(what.find("game "), std::string::npos) << what;
        EXPECT_NE(what.find("[seed="), std::string::npos) << what;
        EXPECT_NE(what.find("agent factory failed"), std::string::npos) << what;
    }
    EXPECT_EQ(results.games_completed.load(), 0);
}

TEST(GameRunnerReviewFix, ReplayGoesToTheConfiguredDirectory) {
    const auto dir = std::filesystem::path(testing::TempDir()) / "io_cli_replays";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    GameConfig cfg;
    cfg.base_seed = 20261010;
    cfg.do_render = true;
    cfg.replay_dir = dir.string();
    AggregateResults results;
    results.total_games = 1;
    GameRunner runner(cards().db, cards().registry, cards().deck1, cards().deck2, cfg, results);
    testing::internal::CaptureStdout();
    runner.run();
    testing::internal::GetCapturedStdout();
    EXPECT_TRUE(std::filesystem::exists(dir / "replay_game1.html"));
}

TEST(GameRunnerReviewFix, DebugLogIsPrinted) {
    GameConfig cfg;
    cfg.base_seed = 20261010;
    cfg.debug_mode = true;
    cfg.trace_mode = true;
    AggregateResults results;
    results.total_games = 1;
    GameRunner runner(cards().db, cards().registry, cards().deck1, cards().deck2, cfg, results);
    testing::internal::CaptureStdout();
    runner.run();
    const std::string out = testing::internal::GetCapturedStdout();
    // The log used to be collected into a buffer nobody read.
    EXPECT_NE(out.find("[TRC] "), std::string::npos);
    EXPECT_NE(out.find("Game 1 [seed=20261010] log"), std::string::npos);
}

// ─── 9.L3 — what a batch does after one game throws ─────────────────────────

namespace {

/// Plays at random, except that it throws at its first decision in the game
/// whose seed is `bad_seed`.
class ThrowsInOneGameAgent : public AgentInterface {
public:
    ThrowsInOneGameAgent(uint64_t seed, int seat, uint64_t bad_seed)
        : inner_(seed * 2 + static_cast<uint64_t>(seat)), fail_(seed == bad_seed && seat == 0) {}
    Intent selectAction(const GameState& state, const std::vector<Intent>& legal) override {
        if (fail_) throw std::runtime_error("injected fault");
        return inner_.selectAction(state, legal);
    }
private:
    RandomAgent inner_;
    bool fail_;
};

} // namespace

// Pins the behaviour, single-threaded so the order is fixed: the games before
// the failing one ran and are counted, the failing game is named by number
// and seed, the games after it are NOT run, and runBatch throws (the binary
// then prints "Error: a game failed: ... (k of n games completed)" and exits
// 1 without a results table).
TEST(BatchRunnerFaultInjection, OneThread_EarlierGamesCountLaterGamesAreSkipped) {
    constexpr uint64_t kBase = 4100;
    constexpr int kGames = 5;
    constexpr int kBadIndex = 2;   // the third game
    std::atomic<int> games_started{0};
    GameConfig cfg;
    cfg.base_seed = kBase;
    cfg.agent_factory = [&](int seat, uint64_t seed) -> std::unique_ptr<AgentInterface> {
        if (seat == 0) games_started.fetch_add(1);
        return std::make_unique<ThrowsInOneGameAgent>(seed, seat, kBase + kBadIndex);
    };
    AggregateResults results;
    BatchRunner batch(cards().db, cards().registry, 1);
    std::string what;
    try {
        batch.runBatch(cards().deck1, cards().deck2, kGames, cfg, results);
        FAIL() << "expected the failing game's exception";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("game 3 [seed=" + std::to_string(kBase + kBadIndex) + "]"),
              std::string::npos) << what;
    EXPECT_NE(what.find("injected fault"), std::string::npos) << what;
    EXPECT_EQ(results.games_completed.load(), kBadIndex) << "games 1 and 2 finished and count";
    EXPECT_EQ(results.p1_wins.load() + results.p2_wins.load() + results.draws.load(), kBadIndex);
    EXPECT_EQ(games_started.load(), kBadIndex + 1) << "games 4 and 5 were never started";

    // 9.S1: what the binary prints then. The table for the two games that
    // completed comes first, the error after it.
    std::ostringstream printed;
    reportFailedBatch(printed, printed, results, kGames, 1, what);
    const std::string text = printed.str();
    const auto table = text.find("Results (2 of 5 games completed):");
    const auto error = text.find("Error: a game failed: game 3 [seed=");
    ASSERT_NE(table, std::string::npos) << text;
    ASSERT_NE(error, std::string::npos) << text;
    EXPECT_LT(table, error) << text;
    EXPECT_NE(text.find("  P1 wins: " + std::to_string(results.p1_wins.load()) + "\n"),
              std::string::npos) << text;
    EXPECT_NE(text.find("  Avg turns: " + std::to_string(results.total_turns.load() / 2) + "\n"),
              std::string::npos) << "averaged over the completed games: " << text;
    EXPECT_NE(text.find("(2 of 5 games completed)\n"), std::string::npos) << text;
}

// A batch that ran to the end prints the table it always did.
TEST(BatchReport, ACompleteBatchPrintsTheUsualTable) {
    AggregateResults results;
    results.p1_wins = 3;
    results.p2_wins = 1;
    results.total_turns = 40;
    results.total_decisions = 800;
    results.games_completed = 4;
    std::ostringstream out;
    printBatchResults(out, results, 4, 2);
    EXPECT_EQ(out.str(), "\nResults (4 games):\n  P1 wins: 3\n  P2 wins: 1\n  Draws:   0\n"
                         "  Avg turns: 10\n  Avg decisions: 200\n  Threads: 2\n");
}

// With several workers the games already running finish and count; a game not
// yet started when the failure is recorded is skipped. So the number completed
// is anything from 0 to all but the failing one: what is pinned is that it is
// never the failing game, that the process survives, and the game is named.
TEST(BatchRunnerFaultInjection, ManyThreads_TheFailedGameIsNamedAndNeverCounted) {
    constexpr uint64_t kBase = 4200;
    constexpr int kGames = 6;
    constexpr int kBadIndex = 3;
    GameConfig cfg;
    cfg.base_seed = kBase;
    cfg.agent_factory = [&](int seat, uint64_t seed) -> std::unique_ptr<AgentInterface> {
        return std::make_unique<ThrowsInOneGameAgent>(seed, seat, kBase + kBadIndex);
    };
    AggregateResults results;
    BatchRunner batch(cards().db, cards().registry, 3);
    std::string what;
    try {
        batch.runBatch(cards().deck1, cards().deck2, kGames, cfg, results);
        FAIL() << "expected the failing game's exception";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("game 4 [seed=" + std::to_string(kBase + kBadIndex) + "]"),
              std::string::npos) << what;
    EXPECT_LE(results.games_completed.load(), kGames - 1);
    EXPECT_EQ(results.p1_wins.load() + results.p2_wins.load() + results.draws.load(),
              results.games_completed.load());
}


}  // namespace riftbound::test::review_fix_io_cli
