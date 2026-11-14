/// @file search_baseline.cpp
/// The numbers a search budget is planned from.
///
/// Plays games through the step API and reports what tree search would pay:
/// decisions per game and turn, branching factor, how many decisions a search
/// could skip, how often a decision can be forked with a state copy, and what
/// each operation costs. At sampled clean decisions it also resumes a fresh
/// engine from the snapshot and checks it reaches the same decision.
///
///   search_baseline --decks decks/a.txt,decks/b.txt,... [--games 40]
///                   [--agent random|scripted] [--seed 1]

#include "agents/random_agent.h"
#include "agents/scripted_agent.h"
#include "cards/card_registry.h"
#include "core/card_db.h"
#include "core/events.h"
#include "engine/game_engine.h"
#include "rules/deck_validator.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace riftbound;
using Clock = std::chrono::steady_clock;

namespace {

double usSince(Clock::time_point t) {
    return std::chrono::duration<double, std::micro>(Clock::now() - t).count();
}

/// Same test as RiftboundState::ensureLive: can this decision be resumed from
/// a state copy, or does it need a replay from the start of the game?
bool cleanBoundary(const GameState& s) {
    if (s.game_over || s.turn.phase == TurnPhase::Setup ||
        s.turn.phase == TurnPhase::Mulligan ||
        s.turn.oc_state != OpenClosedState::Open ||
        s.turn.ns_state != NeutralShowdownState::Neutral ||
        s.chain.exists() || s.cost_cursor.has_value())
        return false;
    for (const auto& bf : s.battlefields)
        if (bf.combat_in_progress || bf.showdown_in_progress ||
            bf.combat_staged || bf.showdown_staged)
            return false;
    return true;
}

/// Why a decision is not clean, for the ones that are not.
std::string dirtyReason(const GameState& s) {
    if (s.turn.phase == TurnPhase::Setup || s.turn.phase == TurnPhase::Mulligan) return "setup/mulligan";
    if (s.cost_cursor.has_value()) return "paying a cost";
    if (s.chain.exists()) return "chain open";
    for (const auto& bf : s.battlefields)
        if (bf.combat_in_progress || bf.combat_staged) return "combat";
    for (const auto& bf : s.battlefields)
        if (bf.showdown_in_progress || bf.showdown_staged) return "showdown";
    return "other closed state";
}

struct Stats {
    long games = 0, decisions = 0, single = 0, clean = 0, turns = 0, capped = 0;
    long sum_legal = 0, sum_choice_legal = 0, choice = 0, max_legal = 0;
    long resume_checks = 0, resume_mismatch = 0, group_checks = 0, group_mismatch = 0;
    double apply_us = 0, copy_us = 0, resume_us = 0;
    long copies = 0;
    std::vector<int> legal_sizes, game_decisions, game_turns;
    std::map<std::string, long> dirty;
    std::map<int, long> p_wins;
};

int pct(std::vector<int> v, double p) {
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, static_cast<size_t>(p * v.size()))];
}

void playGame(const DeckSubmission& d1, const DeckSubmission& d2, const CardDB& db,
              const CardRegistry& reg, uint64_t seed, bool scripted, Stats& st) {
    EventBus bus;
    GameEngine engine(db, bus, reg);
    engine.beginGame(d1, d2, seed);
    std::mt19937_64 rng(seed ^ 0x9e3779b97f4a7c15ULL);
    ScriptedAgent script(db);
    long decisions = 0;
    constexpr long kCap = 3000;

    while (!engine.isStepDone() && decisions < kCap) {
        const StepResult step = engine.currentStep();
        const GameState& s = engine.state();
        const int n = static_cast<int>(step.legal.size());
        ++decisions;
        st.legal_sizes.push_back(n);
        st.sum_legal += n;
        st.max_legal = std::max<long>(st.max_legal, n);
        if (n <= 1) ++st.single;
        else { ++st.choice; st.sum_choice_legal += n; }
        const bool clean = cleanBoundary(s);
        if (clean) ++st.clean;
        else ++st.dirty[dirtyReason(s)];

        // Every group-move follow-up is checked: it is new and rare.
        const bool at_group = !step.legal.empty() &&
            step.legal[0].choice_label.rfind("move_group", 0) == 0;
        if (decisions % 16 == 0 || at_group) {
            auto t = Clock::now();
            GameState copy = s;
            st.copy_us += usSince(t);
            ++st.copies;
            if (clean) {
                auto r = Clock::now();
                EventBus bus2;
                GameEngine fork(db, bus2, reg);
                const StepResult resumed = fork.resumeFromSnapshot(std::move(copy), seed);
                st.resume_us += usSince(r);
                ++st.resume_checks;
                const bool group = !step.legal.empty() &&
                    step.legal[0].choice_label.rfind("move_group", 0) == 0;
                if (group) ++st.group_checks;
                if (resumed.perspective != step.perspective ||
                    resumed.legal.size() != step.legal.size()) {
                    ++st.resume_mismatch;
                    if (group) ++st.group_mismatch;
                }
            }
        }

        int pick = 0;
        if (scripted) pick = std::max(0, script.selectIndex(s, step.legal));
        else if (n > 1) pick = std::uniform_int_distribution<int>(0, n - 1)(rng);
        auto t = Clock::now();
        engine.applyChoice(pick);
        st.apply_us += usSince(t);
    }
    if (decisions >= kCap) ++st.capped;
    ++st.games;
    st.decisions += decisions;
    st.turns += engine.state().turn.turn_number;
    st.game_decisions.push_back(static_cast<int>(decisions));
    st.game_turns.push_back(engine.state().turn.turn_number);
}

} // namespace

int main(int argc, char** argv) {
    std::string decks_arg;
    std::string agent = "random";
    int games = 40;
    uint64_t seed = 1;
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string k = argv[i];
        if (k == "--decks") decks_arg = argv[i + 1];
        else if (k == "--games") games = std::stoi(argv[i + 1]);
        else if (k == "--agent") agent = argv[i + 1];
        else if (k == "--seed") seed = std::stoull(argv[i + 1]);
    }

    CardRegistry registry;
    registry.loadAll();
    CardDB db;
    db.buildFromClasses(registry);

    std::vector<DeckSubmission> decks;
    std::stringstream ss(decks_arg);
    for (std::string path; std::getline(ss, path, ',');)
        decks.push_back(DeckValidator::loadFromDeckList(path, db));

    Stats st;
    for (int g = 0; g < games; ++g) {
        const auto& a = decks[g % decks.size()];
        const auto& b = decks[(g * 7 + 3) % decks.size()];
        playGame(a, b, db, registry, seed + g, agent == "scripted", st);
    }

    const double d = static_cast<double>(st.decisions);
    printf("agent %s, %ld games over %zu decks, sizeof(GameState)=%zu\n",
           agent.c_str(), st.games, decks.size(), sizeof(GameState));
    printf("decisions/game  mean %.0f  p50 %d  p95 %d  (%ld hit the 3000 cap)\n",
           d / st.games, pct(st.game_decisions, .5), pct(st.game_decisions, .95), st.capped);
    printf("turns/game      mean %.1f  p50 %d  p95 %d  -> decisions/turn %.1f\n",
           static_cast<double>(st.turns) / st.games, pct(st.game_turns, .5),
           pct(st.game_turns, .95), d / st.turns);
    printf("legal actions   mean %.1f  p50 %d  p95 %d  p99 %d  max %ld\n",
           st.sum_legal / d, pct(st.legal_sizes, .5), pct(st.legal_sizes, .95),
           pct(st.legal_sizes, .99), st.max_legal);
    printf("forced          %.1f%% of decisions have one option; real choices average %.1f options\n",
           100.0 * st.single / d, static_cast<double>(st.sum_choice_legal) / std::max(1L, st.choice));
    printf("clean boundary  %.1f%% of decisions (the rest need replay from game start)\n",
           100.0 * st.clean / d);
    for (const auto& [k, v] : st.dirty)
        printf("  dirty: %-20s %5.1f%%\n", k.c_str(), 100.0 * v / d);
    printf("cost            applyChoice %.1f us  state copy %.1f us  resume %.1f us\n",
           st.apply_us / d, st.copy_us / std::max(1L, st.copies),
           st.resume_us / std::max(1L, st.resume_checks));
    printf("resume fidelity %ld checks, %ld mismatched the live decision\n",
           st.resume_checks, st.resume_mismatch);
    printf("  of which      %ld group-move checks, %ld mismatched\n",
           st.group_checks, st.group_mismatch);
    printf("throughput      %.0f decisions/s on one core (apply only)\n", 1e6 * d / st.apply_us);
    return 0;
}
