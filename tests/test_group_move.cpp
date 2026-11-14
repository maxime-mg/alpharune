/// @file test_group_move.cpp
/// CR 144.3 — moving several units in one Standard Move.
///
/// The generator keeps offering one StandardMove per unit and destination.
/// After one is chosen, and before anything moves, the engine asks the mover
/// a follow-up "add another unit or done" decision (docs/ui-roadmap.md,
/// "Group move design"). The contract these tests pin down:
///
///   - Every follow-up option is an IntentType::MakeChoice from the mover,
///     with a `choice_label` starting with "move_group".
///   - Done comes first: chosen_objects empty, chosen_value = 0. Agents
///     that always take legal[0] keep today's single-unit moves.
///   - Each other option adds one unit: chosen_objects = {unit}.
///   - A unit is offered only if it has its own single-unit StandardMove
///     to the same destination, and only if its id is higher than the
///     last unit picked, so each group is reachable in exactly one order.
///   - No follow-up is asked when no unit qualifies.
///   - On Done (or when nothing more qualifies) all chosen units move
///     together: each is exhausted, each emits UnitMoved, and the contest
///     and staging checks run once for the group.
///   - The follow-up is a resumable decision: a game snapshotted at it
///     resumes to the same legal list.

#include "tests/cards/card_test_fixture.h"

#include "engine/game_engine.h"
#include "rules/deck_validator.h"

#include <random>

namespace riftbound::test {
namespace {

bool isGroupOption(const Intent& i) {
    return i.type == IntentType::MakeChoice &&
           i.choice_label.rfind("move_group", 0) == 0;
}

bool isGroupPrompt(const std::vector<Intent>& legal) {
    return !legal.empty() &&
           std::all_of(legal.begin(), legal.end(), isGroupOption);
}

bool isDone(const Intent& i) {
    return i.chosen_objects.empty() && i.chosen_value == 0;
}

/// The units a group prompt offers to add (Done excluded).
std::vector<GameObjectId> offeredUnits(const std::vector<Intent>& legal) {
    std::vector<GameObjectId> out;
    for (const auto& i : legal)
        if (!isDone(i) && i.chosen_objects.size() == 1)
            out.push_back(i.chosen_objects[0]);
    std::sort(out.begin(), out.end());
    return out;
}

Intent pickUnit(const std::vector<Intent>& legal, GameObjectId unit) {
    for (const auto& i : legal)
        if (i.chosen_objects == std::vector<GameObjectId>{unit}) return i;
    ADD_FAILURE() << "unit " << unit << " was not offered";
    return legal.front();
}

Intent done(const std::vector<Intent>& legal) {
    for (const auto& i : legal)
        if (isDone(i)) return i;
    ADD_FAILURE() << "group prompt has no Done option";
    return legal.front();
}

/// Records every group prompt and answers it from a script. Any other
/// decision gets legal[0].
class GroupMoveAgent : public AgentInterface {
public:
    using Answer = std::function<Intent(const GameState&, const std::vector<Intent>&)>;
    std::vector<Answer> answers;
    std::vector<std::vector<Intent>> prompts;

    Intent selectAction(const GameState& s,
                        const std::vector<Intent>& legal) override {
        if (!isGroupPrompt(legal)) return legal.empty() ? Intent{} : legal.front();
        prompts.push_back(legal);
        const size_t k = prompts.size() - 1;
        if (k < answers.size()) return answers[k](s, legal);
        return done(legal);
    }
};

class GroupMoveTest : public CardTestFixture {
protected:
    std::unique_ptr<GameEngine> engine_;
    GroupMoveAgent p1_agent_;
    FirstChoiceAgent p2_agent_;

    void SetUp() override {
        CardTestFixture::SetUp();
        engine_ = std::make_unique<GameEngine>(card_db, events, card_registry);
        auto& s = st();
        s.mode = ModeOfPlay{};
        s.players[0].id = P1;
        s.players[1].id = P2;
        for (int i = 0; i < 2; ++i) {
            BattlefieldState bf;
            bf.id = static_cast<int>(s.battlefields.size());
            s.battlefields.push_back(bf);
        }
        // Phase-step hooks sweep both legend zones.
        for (int p = 0; p < 2; ++p) {
            auto leg = s.createObject();
            auto& lo = s.getObject(leg);
            lo.owner = lo.controller = s.players[p].id;
            lo.card_type = CardType::Legend;
            lo.zone = ZoneType::LegendZone;
            s.players[p].legend_zone = leg;
        }
        s.turn.turn_player = P1;
        s.turn.phase = TurnPhase::MainPhase;
        engine_->testHook_setAgents(&p1_agent_, &p2_agent_);
    }

    GameState& st() { return engine_->mutableState(); }

    /// A ready unit in the engine's state. at_bf < 0 means the owner's base.
    GameObjectId unit(PlayerId owner, int at_bf = -1, int might = 2) {
        auto& s = st();
        auto id = s.createObject();
        auto& u = s.getObject(id);
        u.owner = u.controller = owner;
        u.card_type = CardType::Unit;
        u.name = "Unit#" + std::to_string(id);
        u.base_might = u.current_might = might;
        if (at_bf >= 0) {
            u.zone = ZoneType::BattlefieldZone;
            u.location = BattlefieldLocation{static_cast<BattlefieldId>(at_bf)};
        } else {
            u.zone = ZoneType::Base;
            u.location = BaseLocation{owner};
        }
        return id;
    }

    GameObject& obj(GameObjectId id) { return st().getObject(id); }

    bool atBf(GameObjectId id, BattlefieldId bf) {
        const auto& loc = obj(id).location;
        return loc && std::holds_alternative<BattlefieldLocation>(*loc) &&
               std::get<BattlefieldLocation>(*loc).id == bf;
    }
    bool atBase(GameObjectId id) {
        const auto& loc = obj(id).location;
        return loc && std::holds_alternative<BaseLocation>(*loc);
    }

    /// Choose the generated single-unit move of `u` to `dest`, the way an
    /// agent would, and run it.
    void move(GameObjectId u, LocationId dest) {
        Intent picked;
        bool found = false;
        for (const auto& i : engine_->testHook_generateMainPhaseActions(P1)) {
            if (i.type == IntentType::StandardMove &&
                i.units_to_move == std::vector<GameObjectId>{u} &&
                i.move_destination == dest) {
                picked = i;
                found = true;
            }
        }
        ASSERT_TRUE(found) << "no generated StandardMove for unit " << u;
        engine_->testHook_executeIntent(picked);
    }
};

// ─── No follow-up when nothing else can join ────────────────────────────────

TEST_F(GroupMoveTest, SingleEligibleUnitAsksNoFollowUp) {
    auto a = unit(P1);
    move(a, BattlefieldLocation{0});

    EXPECT_TRUE(p1_agent_.prompts.empty());
    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(obj(a).is_exhausted);
}

TEST_F(GroupMoveTest, ExhaustedUnitsDoNotCount) {
    auto a = unit(P1);
    auto b = unit(P1);
    obj(b).is_exhausted = true;
    move(a, BattlefieldLocation{0});

    EXPECT_TRUE(p1_agent_.prompts.empty());
    EXPECT_TRUE(atBase(b));
}

// ─── Shape of the follow-up ─────────────────────────────────────────────────

TEST_F(GroupMoveTest, FollowUpPutsDoneFirstAndOffersEligibleUnits) {
    auto a = unit(P1);
    auto b = unit(P1);
    auto c = unit(P1);
    move(a, BattlefieldLocation{0});

    ASSERT_EQ(p1_agent_.prompts.size(), 1u);
    const auto& legal = p1_agent_.prompts[0];
    EXPECT_TRUE(isDone(legal.front())) << "Done must be legal[0]";
    for (const auto& i : legal) EXPECT_EQ(i.player, P1);
    EXPECT_EQ(offeredUnits(legal), (std::vector<GameObjectId>{b, c}));
}

TEST_F(GroupMoveTest, DoneMovesOnlyTheFirstUnit) {
    auto a = unit(P1);
    auto b = unit(P1);
    move(a, BattlefieldLocation{0});

    EXPECT_EQ(p1_agent_.prompts.size(), 1u);
    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(atBase(b));
    EXPECT_FALSE(obj(b).is_exhausted);
}

TEST_F(GroupMoveTest, NothingMovesUntilTheGroupIsClosed) {
    auto a = unit(P1);
    auto b = unit(P1);
    bool a_waited = false;
    p1_agent_.answers.push_back([&](const GameState& s, const std::vector<Intent>& legal) {
        const auto& loc = s.getObject(a).location;
        a_waited = loc && std::holds_alternative<BaseLocation>(*loc) &&
                   !s.getObject(a).is_exhausted;
        return pickUnit(legal, b);
    });
    move(a, BattlefieldLocation{0});

    EXPECT_TRUE(a_waited) << "the first unit moved before the group was closed";
    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(atBf(b, 0));
}

// ─── Canonical order: each group is reachable once ─────────────────────────

TEST_F(GroupMoveTest, OnlyHigherIdUnitsAreOffered) {
    auto a = unit(P1);
    auto b = unit(P1);
    auto c = unit(P1);
    ASSERT_LT(a, b);
    ASSERT_LT(b, c);
    move(b, BattlefieldLocation{0});

    ASSERT_EQ(p1_agent_.prompts.size(), 1u);
    EXPECT_EQ(offeredUnits(p1_agent_.prompts[0]), (std::vector<GameObjectId>{c}))
        << "a < b, so {a, b} is only reachable by starting from a";
}

TEST_F(GroupMoveTest, PromptRepeatsUntilNoHigherUnitIsLeft) {
    auto a = unit(P1);
    auto b = unit(P1);
    auto c = unit(P1);
    p1_agent_.answers.push_back([&](const GameState&, const std::vector<Intent>& l) {
        return pickUnit(l, b);
    });
    move(a, BattlefieldLocation{0});

    // After a then b, only c is left, so a second prompt offers just c.
    ASSERT_EQ(p1_agent_.prompts.size(), 2u);
    EXPECT_EQ(offeredUnits(p1_agent_.prompts[1]), (std::vector<GameObjectId>{c}));
    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(atBf(b, 0));
    EXPECT_TRUE(atBase(c));
}

TEST_F(GroupMoveTest, PickingTheLastUnitClosesTheGroupWithoutAnotherPrompt) {
    auto a = unit(P1);
    auto b = unit(P1);
    p1_agent_.answers.push_back([&](const GameState&, const std::vector<Intent>& l) {
        return pickUnit(l, b);
    });
    move(a, BattlefieldLocation{0});

    EXPECT_EQ(p1_agent_.prompts.size(), 1u);
    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(atBf(b, 0));
}

// ─── Eligibility follows the single-unit move rules ────────────────────────

TEST_F(GroupMoveTest, UnitThatCannotMoveToBaseIsNeverOffered) {
    auto a = unit(P1, 0);
    auto sentry = unit(P1, 0);
    obj(sentry).cant_move_to_base = true;  // Determined Sentry
    move(a, BaseLocation{P1});

    EXPECT_TRUE(p1_agent_.prompts.empty());
    EXPECT_TRUE(atBase(a));
    EXPECT_TRUE(atBf(sentry, 0));
}

TEST_F(GroupMoveTest, OnlyGankingUnitsJoinABattlefieldToBattlefieldMove) {
    auto a = unit(P1, 0);
    obj(a).keywords.set(Keyword::Ganking);
    auto plain = unit(P1, 0);
    auto ganker = unit(P1, 0);
    obj(ganker).keywords.set(Keyword::Ganking);
    move(a, BattlefieldLocation{1});

    ASSERT_EQ(p1_agent_.prompts.size(), 1u);
    EXPECT_EQ(offeredUnits(p1_agent_.prompts[0]), (std::vector<GameObjectId>{ganker}));
    EXPECT_TRUE(atBf(plain, 0));
}

TEST_F(GroupMoveTest, UnitsFromBaseAndFromAnotherBattlefieldCanGoTogether) {
    // CR 144.3: a group may come from different locations, as long as each
    // unit could make the move alone. A Ganking unit at BF#1 can reach BF#0.
    auto a = unit(P1);
    auto ganker = unit(P1, 1);
    obj(ganker).keywords.set(Keyword::Ganking);
    auto plain = unit(P1, 1);
    p1_agent_.answers.push_back([&](const GameState&, const std::vector<Intent>& l) {
        return pickUnit(l, ganker);
    });
    move(a, BattlefieldLocation{0});

    ASSERT_GE(p1_agent_.prompts.size(), 1u);
    EXPECT_EQ(offeredUnits(p1_agent_.prompts[0]), (std::vector<GameObjectId>{ganker}));
    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(atBf(ganker, 0));
    EXPECT_TRUE(atBf(plain, 1));
}

TEST_F(GroupMoveTest, OpponentUnitsAreNeverOffered) {
    auto a = unit(P1);
    unit(P2);
    move(a, BattlefieldLocation{0});

    EXPECT_TRUE(p1_agent_.prompts.empty());
}

// ─── The group arrives together ─────────────────────────────────────────────

TEST_F(GroupMoveTest, GroupIntoEnemyBattlefieldStagesOneCombat) {
    auto& bf = st().battlefields[0];
    bf.controller = P2;
    unit(P2, 0, 3);
    auto a = unit(P1);
    auto b = unit(P1);

    int contested = 0;
    std::vector<GameObjectId> moved;
    auto c1 = events.on_contested.connect([&](const ContestedEvent&) { ++contested; });
    auto c2 = events.on_unit_moved.connect([&](const UnitMovedEvent& e) {
        EXPECT_TRUE(e.is_standard_move);
        moved.push_back(e.object);
    });

    p1_agent_.answers.push_back([&](const GameState&, const std::vector<Intent>& l) {
        return pickUnit(l, b);
    });
    move(a, BattlefieldLocation{0});
    c1.disconnect();
    c2.disconnect();

    EXPECT_TRUE(atBf(a, 0));
    EXPECT_TRUE(atBf(b, 0));
    EXPECT_TRUE(obj(a).is_exhausted);
    EXPECT_TRUE(obj(b).is_exhausted);
    EXPECT_EQ(moved, (std::vector<GameObjectId>{a, b}));
    EXPECT_EQ(contested, 1);
    EXPECT_TRUE(bf.combat_staged);
    EXPECT_FALSE(bf.showdown_staged);
    EXPECT_EQ(bf.contested_by, P1);
}

TEST_F(GroupMoveTest, GroupIntoEmptyBattlefieldStagesOneShowdown) {
    auto& bf = st().battlefields[1];
    auto a = unit(P1);
    auto b = unit(P1);
    p1_agent_.answers.push_back([&](const GameState&, const std::vector<Intent>& l) {
        return pickUnit(l, b);
    });
    move(a, BattlefieldLocation{1});

    EXPECT_TRUE(atBf(a, 1));
    EXPECT_TRUE(atBf(b, 1));
    EXPECT_TRUE(bf.showdown_staged);
    EXPECT_FALSE(bf.combat_staged);
}

// ─── Step machine: the follow-up is resumable ──────────────────────────────

std::string decksDir() {
    const char* root = std::getenv("RIFTBOUND_ROOT");
    return std::string(root ? root : "..") + "/decks/";
}

TEST(GroupMoveStepMachine, SnapshotAtGroupPromptResumesToTheSameDecisions) {
    CardRegistry registry;
    registry.loadAll();
    CardDB db;
    db.buildFromClasses(registry);
    const auto d1 = DeckValidator::loadFromDeckList(decksDir() + "darius_test.txt", db);
    const auto d2 = DeckValidator::loadFromDeckList(decksDir() + "annie_test.txt", db);

    int prompts = 0, checked = 0;
    for (uint64_t seed = 1; seed <= 40 && checked < 20; ++seed) {
        EventBus bus;
        GameEngine engine(db, bus, registry);
        engine.beginGame(d1, d2, seed);
        std::mt19937_64 rng(seed);

        for (int n = 0; n < 3000 && !engine.isStepDone(); ++n) {
            StepResult step = engine.currentStep();
            if (step.legal.empty()) break;

            if (isGroupPrompt(step.legal)) {
                ++prompts;
                ASSERT_TRUE(isDone(step.legal.front()));
                EventBus bus2;
                GameEngine fork(db, bus2, registry);
                StepResult resumed;
                ASSERT_NO_THROW(resumed = fork.resumeFromSnapshot(engine.state(), seed));
                ASSERT_EQ(resumed.perspective, step.perspective) << "seed " << seed;
                ASSERT_EQ(resumed.legal, step.legal) << "seed " << seed;

                // Finish the group the same way in both and compare each step,
                // plus the first decision after the group closes.
                for (int k = 0; k < 12; ++k) {
                    const int pick = std::uniform_int_distribution<int>(
                        0, static_cast<int>(step.legal.size()) - 1)(rng);
                    const bool was_group = isGroupPrompt(step.legal);
                    step = engine.applyChoice(pick);
                    resumed = fork.applyChoice(pick);
                    ASSERT_EQ(resumed.kind, step.kind) << "seed " << seed;
                    ASSERT_EQ(resumed.legal, step.legal) << "seed " << seed;
                    if (!was_group || step.legal.empty()) break;
                }
                ++checked;
                continue;
            }

            const int pick = std::uniform_int_distribution<int>(
                0, static_cast<int>(step.legal.size()) - 1)(rng);
            engine.applyChoice(pick);
        }
    }
    ASSERT_GT(prompts, 0) << "no move_group prompt reached in 40 random games: "
                             "group moves are not implemented yet";
    EXPECT_GT(checked, 0);
}

}  // namespace
}  // namespace riftbound::test
