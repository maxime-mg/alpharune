// Curtain Call (743): "Choose one you haven't already chosen", with three
// [Repeat] costs. Every execution's choices are made as the spell is played
// (CR 820.2), so a mode is taken when it is chosen, not when it resolves.

#include "card_test_fixture.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

namespace riftbound::test::curtain_call_repeat {

constexpr CardDefId kRecruit = 266;
constexpr CardDefId kCurtainCall = 743;   // [4]; [Repeat] — [1] / [A] / [1][A]

// Its modes: 0 Draw 1, 1 Deal 2 (battlefield), 2 Deal 3 (base), 3 -4 [M] (battlefield).
class CurtainCallRepeatTest : public CardTestFixture {
protected:
    GameObject& o(GameObjectId id) { return state.getObject(id); }
    static int v(GameObjectId id) { return static_cast<int>(id.raw()); }
    void SetUp() override {
        CardTestFixture::SetUp();
        for (int i = 0; i < 8; ++i) addToDeck(P1, kRecruit);
        state.player(P1).rune_pool.energy = 20;
        state.player(P1).rune_pool.universal_power = 20;
    }
    void playSpell(GameObjectId card, AgentInterface* p1, AgentInterface* p2 = nullptr,
                   GameEngine** live = nullptr) {
        Intent i;
        i.type = IntentType::PlayCard;
        i.player = P1;
        i.card = card;
        withEngine([&](GameEngine& e) {
            if (live) *live = &e;
            e.testHook_executeIntent(i);
            settleLikeTheEngine(e);
        }, p1, p2);
    }
    static bool isModePrompt(const std::vector<Intent>& p) {
        const std::set<std::string> labels{"Draw 1", "Deal 2 (BF)", "Deal 3 (base)", "-4M (BF)"};
        return !p.empty() && std::all_of(p.begin(), p.end(), [&](const Intent& i) {
            return labels.count(i.choice_label) > 0;
        });
    }
    /// The modes each mode prompt offered, in order.
    static std::vector<std::set<int>> modesOffered(const EnginePathAgent& agent) {
        std::vector<std::set<int>> out;
        for (const auto& p : agent.prompts) {
            if (!isModePrompt(p)) continue;
            std::set<int> modes;
            for (const auto& i : p) modes.insert(i.chosen_value.value_or(-1));
            out.push_back(modes);
        }
        return out;
    }
};

TEST_F(CurtainCallRepeatTest, EachExecutionIsOfferedOnlyModesNotYetChosen) {
    auto at_bf = addUnit(P2, kInvalidId, 30, 0);
    auto at_base = addUnit(P2, kInvalidId, 30);
    auto spell = addToHand(P1, kCurtainCall);
    EnginePathAgent agent(EnginePathAgent::script({
        0, 1, 1, 1,         // Draw 1; pay all three Repeats
        1, v(at_bf), 2, v(at_base), 3, v(at_bf)}));
    playSpell(spell, &agent);
    const std::vector<std::set<int>> want{{0, 1, 2, 3}, {1, 2, 3}, {2, 3}, {3}};
    EXPECT_EQ(modesOffered(agent), want);
    EXPECT_EQ(handSize(P1), 1) << "Draw 1, once";
    EXPECT_EQ(o(at_bf).damage_marked, 2);
    EXPECT_EQ(o(at_base).damage_marked, 3);
    EXPECT_EQ(o(at_bf).temp_might_bonus, -4);
}

TEST_F(CurtainCallRepeatTest, AnExecutionWithNoModeLeftDoesNothing) {
    // Three printed Repeats and a granted one: five executions, four modes.
    auto at_bf = addUnit(P2, kInvalidId, 30, 0);
    auto at_base = addUnit(P2, kInvalidId, 30);
    auto spell = addToHand(P1, kCurtainCall);
    state.player(P1).grant_repeat_base_to_next_spell = true;
    EnginePathAgent agent(EnginePathAgent::script({
        0, 1, 1, 1, 1, 1, v(at_bf), 2, v(at_base), 3, v(at_bf)}));
    playSpell(spell, &agent);
    EXPECT_EQ(modesOffered(agent).size(), 4u);
    EXPECT_EQ(handSize(P1), 1);
    EXPECT_EQ(o(at_bf).damage_marked, 2);
    EXPECT_EQ(o(at_base).damage_marked, 3);
    EXPECT_EQ(o(at_bf).temp_might_bonus, -4);
    EXPECT_EQ(o(spell).zone, ZoneType::Trash);
}

TEST_F(CurtainCallRepeatTest, WithNoUnitOnlyTheDrawRunsOnce) {
    auto spell = addToHand(P1, kCurtainCall);
    EnginePathAgent agent(EnginePathAgent::script({0, 1, 0, 0}));   // Draw 1, one Repeat
    playSpell(spell, &agent);
    EXPECT_EQ(modesOffered(agent).size(), 1u) << "no mode is left to choose";
    EXPECT_EQ(handSize(P1), 1);
}

TEST_F(CurtainCallRepeatTest, PlayedByAnEffectTheModesAreStillDistinct) {
    auto at_bf = addUnit(P2, kInvalidId, 30, 0);
    auto spell = addToHand(P1, kCurtainCall);
    EnginePathAgent agent(EnginePathAgent::script({0, 1, 0, 0, 1, v(at_bf)}));
    resolveThroughEngine([&](EffectExecutor& x, GameState&) {
        x.playPayingCost(P1, spell, std::nullopt, {});
    }, &agent);
    const std::vector<std::set<int>> want{{0, 1, 3}, {1, 3}};
    EXPECT_EQ(modesOffered(agent), want);
    EXPECT_EQ(handSize(P1), 1);
    EXPECT_EQ(o(at_bf).damage_marked, 2);
}

TEST_F(CurtainCallRepeatTest, NewChoicesKeepOffTheModesOfTheOtherExecutions) {
    auto at_bf = addUnit(P2, kInvalidId, 30, 0);
    auto spell = addToHand(P1, kCurtainCall);
    EnginePathAgent p1(EnginePathAgent::script({
        0, 1, 0, 0, 1, v(at_bf),    // Draw 1; one Repeat: Deal 2
        3, v(at_bf)}));             // the first execution's new choice
    GameEngine* live = nullptr;
    bool done = false;
    PickByPredicateAgent p2([&](int, const std::vector<Intent>& legal) {
        if (live && !done)
            for (const auto& ci : live->mutableState().chain.items)
                if (ci.source == spell) {
                    done = true;
                    live->testHook_executor().makeNewChoices(ci.id);
                    break;
                }
        for (const auto& i : legal)
            if (i.type == IntentType::PassPriority) return i;
        return legal.front();
    });
    playSpell(spell, &p1, &p2, &live);
    ASSERT_TRUE(done);
    const std::vector<std::set<int>> want{{0, 1, 3}, {1, 3}, {0, 3}};
    EXPECT_EQ(modesOffered(p1), want) << "Deal 2 is the Repeat's; Draw 1 is its own again";
    EXPECT_EQ(handSize(P1), 0);
    EXPECT_EQ(o(at_bf).damage_marked, 2);
    EXPECT_EQ(o(at_bf).temp_might_bonus, -4);
}

TEST_F(CurtainCallRepeatTest, ALaterPlayStartsFresh) {
    auto at_bf = addUnit(P2, kInvalidId, 30, 0);
    for (int n = 0; n < 2; ++n) {
        auto spell = addToHand(P1, kCurtainCall);
        EnginePathAgent agent(EnginePathAgent::script({0, 1, 0, 0, 1, v(at_bf)}));
        playSpell(spell, &agent);
        const std::vector<std::set<int>> want{{0, 1, 3}, {1, 3}};
        EXPECT_EQ(modesOffered(agent), want);
    }
    EXPECT_EQ(handSize(P1), 2);
    EXPECT_EQ(o(at_bf).damage_marked, 4);
}

}  // namespace riftbound::test::curtain_call_repeat
