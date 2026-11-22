// CARD-013: the gaps left after upstream's card fixes (docs/tickets). Each
// defect has a test here that fails on the code the ticket describes.

#include "card_test_fixture.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <optional>
#include <vector>

namespace riftbound::test::card_013 {

constexpr CardDefId kMeditation = 48;          // spell
constexpr CardDefId kStandUnited = 53;         // [Hidden]
constexpr CardDefId kSpriteCall = 94;          // [Hidden]
constexpr CardDefId kProgressDay = 114;        // [6][B]: Draw 4.
constexpr CardDefId kPrimalStrength = 154;     // [4][O]: "Give a unit +7 [M] this turn."
constexpr CardDefId kFoxFire = 253;            // [Hidden]
constexpr CardDefId kTheBoss = 264;
constexpr CardDefId kRecruit = 266;
constexpr CardDefId kBandleTree = 273;
constexpr CardDefId kDreamingTree = 287;
constexpr CardDefId kSerratedDirk = 332;       // gear
constexpr CardDefId kApprenticeSmith = 364;
constexpr CardDefId kFrigidTouch = 389;        // [2], [Repeat] [2]: "Give a unit -2 [M] this turn."
constexpr CardDefId kCrescentGuardian = 684;
constexpr CardDefId kSyndra = 708;             // in a showdown, your spells have [Repeat] [2][P]

class Card013Test : public CardTestFixture {
protected:
    GameObject& o(GameObjectId id) { return state.getObject(id); }
    bool isOnBoard(GameObjectId id) { return o(id).location.has_value(); }
    GameObjectId battlefield(int bf, CardDefId def_id, PlayerId controller) {
        const auto id = state.createObject();
        auto& b = o(id);
        b.card_def_id = def_id;
        b.card_type = CardType::Battlefield;
        b.name = card_db.get(def_id).name;
        b.owner = b.controller = controller;
        b.zone = ZoneType::BattlefieldZone;
        state.battlefields[bf].card_object_id = id;
        state.battlefields[bf].controller = controller;
        return id;
    }
    GameObjectId addLegend(PlayerId owner, CardDefId def_id) {
        const auto id = state.createObject();
        auto& l = o(id);
        const auto& def = card_db.get(def_id);
        l.card_def_id = def_id;
        l.owner = l.controller = owner;
        l.name = def.name;
        l.card_type = def.card_type;
        l.domains = def.domains;
        l.zone = ZoneType::LegendZone;
        state.player(owner).legend_zone = id;
        return id;
    }
    void setPool(PlayerId p, int energy, int any_power) {
        state.player(p).rune_pool = RunePool{};
        state.player(p).rune_pool.energy = energy;
        state.player(p).rune_pool.universal_power = any_power;
    }
    /// Play a spell from exactly what the test laid out. `settle_first` runs a
    /// cleanup, so that continuous effects are in force as the spell is played.
    void playSpell(GameObjectId card, AgentInterface* agent, std::vector<GameObjectId> targets = {},
                   bool settle_first = false) {
        Intent i;
        i.type = IntentType::PlayCard;
        i.player = o(card).controller;
        i.card = card;
        i.targets = std::move(targets);
        withEngine([&](GameEngine& e) {
            if (settle_first) e.testHook_cleanup();
            e.testHook_executeIntent(i);
            settleLikeTheEngine(e);
        }, agent);
    }
    /// Accepts every Repeat, and chooses `unit` whenever objects are offered.
    static EnginePathAgent::Picker payAndChoose(GameObjectId unit) {
        return [unit](const std::vector<Intent>& legal) {
            for (const auto& i : legal)
                if (!i.chosen_objects.empty() && i.chosen_objects.front() == unit) return i;
            for (const auto& i : legal)
                if (i.chosen_value == std::optional<int>{1}) return i;
            return legal.front();
        };
    }
};

// ── 364 Apprentice Smith ────────────────────────────────────────────────────

TEST_F(Card013Test, ApprenticeSmith_TheGearIsDrawn) {
    auto smith = addUnit(P1, kApprenticeSmith, 0);
    auto gear = addToDeck(P1, kSerratedDirk);
    fireTriggerThroughEngine(smith, TriggerType::WhenIMove);
    EXPECT_TRUE(inHand(P1, gear));
    EXPECT_EQ(state.player(P1).draws_this_turn, 1) << "\"draw it\" is a draw";
}

TEST_F(Card013Test, ApprenticeSmith_ANonGearIsRecycledAndNotDrawn) {
    auto smith = addUnit(P1, kApprenticeSmith, 0);
    auto below = addToDeck(P1, kSerratedDirk);
    auto spell = addToDeck(P1, kMeditation);   // top
    fireTriggerThroughEngine(smith, TriggerType::WhenIMove);
    EXPECT_TRUE(inDeck(P1, spell));
    EXPECT_TRUE(inDeck(P1, below));
    EXPECT_EQ(state.player(P1).draws_this_turn, 0);
}

// ── 684 Crescent Guardian ───────────────────────────────────────────────────

TEST_F(Card013Test, CrescentGuardian_EntersReadyWithNothingToRespondTo) {
    state.player(P1).played_spell_this_turn = true;
    auto guardian = addToHand(P1, kCrescentGuardian);
    setPool(P1, 4, 1);
    bool priority_after_it_entered = false;
    bool seen_exhausted = false;
    GameState* live = nullptr;
    PickByPredicateAgent p2([&](int, const std::vector<Intent>& legal) {
        if (live && live->getObject(guardian).location.has_value()) {
            priority_after_it_entered = true;
            seen_exhausted = seen_exhausted || live->getObject(guardian).is_exhausted;
        }
        for (const auto& i : legal)
            if (i.type == IntentType::PassPriority) return i;
        return legal.front();
    });
    Intent i;
    i.type = IntentType::PlayCard;
    i.player = P1;
    i.card = guardian;
    i.play_location = LocationId{BaseLocation{P1}};
    EnginePathAgent yes(EnginePathAgent::script({1}));
    withEngine([&](GameEngine& e) {
        live = &e.mutableState();
        e.testHook_executeIntent(i);
        settleLikeTheEngine(e);
    }, &yes, &p2);
    EXPECT_FALSE(seen_exhausted) << "it enters ready: it is never exhausted on the board";
    EXPECT_FALSE(priority_after_it_entered) << "no ability goes on the chain to respond to";
    ASSERT_TRUE(isOnBoard(guardian));
    EXPECT_FALSE(o(guardian).is_exhausted);
    EXPECT_EQ(state.player(P1).rune_pool.totalPower(), 0);
}

TEST_F(Card013Test, CrescentGuardian_PlayedByAnEffectItAlsoEntersReady) {
    state.player(P1).played_spell_this_turn = true;
    auto guardian = addToHand(P1, kCrescentGuardian);
    setPool(P1, 4, 1);
    EnginePathAgent yes(EnginePathAgent::script({1}));
    resolveThroughEngine([&](EffectExecutor& x, GameState&) {
        x.playPayingCost(P1, guardian, LocationId{BaseLocation{P1}}, {});
    }, &yes);
    ASSERT_TRUE(isOnBoard(guardian));
    EXPECT_EQ(state.player(P1).rune_pool.totalPower(), 0) << "the additional cost was paid";
    EXPECT_FALSE(o(guardian).is_exhausted);
}

TEST_F(Card013Test, CrescentGuardian_PlayedAgainWithoutPayingEntersExhausted) {
    state.player(P1).played_spell_this_turn = true;
    auto guardian = addToHand(P1, kCrescentGuardian);
    setPool(P1, 0, 1);
    EnginePathAgent yes(EnginePathAgent::script({1}));
    playThroughEngine(guardian, &yes);
    ASSERT_FALSE(o(guardian).is_exhausted);
    resolveThroughEngine([&](EffectExecutor& x, GameState&) { x.bounceToHand(guardian); });
    ASSERT_TRUE(inHand(P1, guardian));

    // Later, with no spell played that turn: the cost is not even offered.
    state.player(P1).played_spell_this_turn = false;
    setPool(P1, 0, 1);
    EnginePathAgent none(EnginePathAgent::script({}));
    playThroughEngine(guardian, &none);
    ASSERT_TRUE(isOnBoard(guardian));
    EXPECT_TRUE(o(guardian).is_exhausted) << "what was paid for an earlier play counts no more";
}

TEST_F(Card013Test, CrescentGuardian_DecliningTheCostEntersExhausted) {
    state.player(P1).played_spell_this_turn = true;
    auto guardian = addToHand(P1, kCrescentGuardian);
    setPool(P1, 0, 1);
    EnginePathAgent no(EnginePathAgent::script({0}));
    playThroughEngine(guardian, &no);
    ASSERT_TRUE(isOnBoard(guardian));
    EXPECT_TRUE(o(guardian).is_exhausted);
    EXPECT_EQ(state.player(P1).rune_pool.totalPower(), 1);
}

// ── 401 Temporal Portal / 772 The Academy / 708 Syndra (CR 820.3) ───────────

TEST_F(Card013Test, GrantedRepeat_IsOfferedBesideThePrintedOne) {
    auto enemy = addUnit(P2, kInvalidId, 9, 0);
    auto spell = addToHand(P1, kFrigidTouch);
    state.player(P1).grant_repeat_base_to_next_spell = true;   // Temporal Portal
    setPool(P1, 6, 0);   // [2], its own Repeat [2], the granted Repeat [2]
    EnginePathAgent agent(payAndChoose(enemy));
    playSpell(spell, &agent, {enemy});
    EXPECT_EQ(state.player(P1).rune_pool.energy, 0) << "both Repeats were paid";
    EXPECT_EQ(o(enemy).temp_might_bonus, -6) << "the effect ran three times";
    EXPECT_FALSE(state.player(P1).grant_repeat_base_to_next_spell);
}

TEST_F(Card013Test, GrantedRepeat_OneOfTheTwoCanBePaidAlone) {
    auto enemy = addUnit(P2, kInvalidId, 9, 0);
    auto spell = addToHand(P1, kFrigidTouch);
    state.player(P1).grant_repeat_base_to_next_spell = true;
    setPool(P1, 4, 0);
    EnginePathAgent agent(payAndChoose(enemy));
    playSpell(spell, &agent, {enemy});
    EXPECT_EQ(state.player(P1).rune_pool.energy, 0);
    EXPECT_EQ(o(enemy).temp_might_bonus, -4);
}

TEST_F(Card013Test, GrantedRepeat_SyndrasIsOfferedBesideThePrintedOne) {
    addUnit(P1, kSyndra, 6, 0);
    auto enemy = addUnit(P2, kInvalidId, 30, 0);
    state.battlefields[0].showdown_in_progress = true;
    auto spell = addToHand(P1, kFrigidTouch);
    setPool(P1, 2 + 2 + 2, 1);   // base, its own Repeat, Syndra's [2][P]
    EnginePathAgent agent(payAndChoose(enemy));
    playSpell(spell, &agent, {enemy}, /*settle_first=*/true);
    EXPECT_EQ(state.player(P1).rune_pool.energy, 0);
    EXPECT_EQ(state.player(P1).rune_pool.totalPower(), 0);
    EXPECT_EQ(o(enemy).temp_might_bonus, -6);
}

TEST_F(Card013Test, GrantedRepeat_SyndrasAndTheOneShotBothApply) {
    addUnit(P1, kSyndra, 6, 0);
    addUnit(P2, kInvalidId, 30, 0);
    state.battlefields[0].showdown_in_progress = true;
    state.player(P1).grant_repeat_base_to_next_spell = true;
    for (int i = 0; i < 14; ++i) addToDeck(P1, kRecruit);
    auto spell = addToHand(P1, kProgressDay);   // [6][B], no printed Repeat
    setPool(P1, 6 + 6 + 2, 1 + 1 + 1);          // base, its cost again, Syndra's [2][P]
    EnginePathAgent agent(payAndChoose(kInvalidId));
    playSpell(spell, &agent, {}, /*settle_first=*/true);
    EXPECT_EQ(state.player(P1).rune_pool.energy, 0);
    EXPECT_EQ(state.player(P1).rune_pool.totalPower(), 0);
    EXPECT_EQ(handSize(P1), 12) << "Draw 4, three times";
}

// ── 264 The Boss (CR 135.2.e.6.c) ───────────────────────────────────────────

TEST_F(Card013Test, TheBoss_ThePlayerChoosesWhichDomainPays) {
    for (Domain paid : {Domain::Body, Domain::Order}) {
        SCOPED_TRACE(toString(paid));
        auto sett = addLegend(P1, kTheBoss);
        auto unit = addUnit(P1, kInvalidId, 2, 0);
        o(unit).buff_count = 1;
        auto body = addRune(P1, Domain::Body);
        auto order = addRune(P1, Domain::Order);
        const auto wanted = paid == Domain::Body ? body : order;
        const auto kept = paid == Domain::Body ? order : body;
        EnginePathAgent agent(EnginePathAgent::script({1, static_cast<int>(wanted.raw())}));
        resolveThroughEngine([&](EffectExecutor& x, GameState&) { x.killObject(unit); }, &agent);
        EXPECT_TRUE(agent.wasOffered(body));
        EXPECT_TRUE(agent.wasOffered(order));
        EXPECT_TRUE(o(unit).isAtBase()) << "recalled instead of dying";
        EXPECT_TRUE(o(sett).is_exhausted);
        EXPECT_FALSE(isOnBoard(wanted)) << "the chosen rune was recycled";
        EXPECT_TRUE(isOnBoard(kept));
        // Out of the way of the next round.
        for (auto id : {unit, body, order}) o(id).location = std::nullopt;
    }
}

// ── 287 The Dreaming Tree (CR 383.1) ────────────────────────────────────────

TEST_F(Card013Test, DreamingTree_TheFirstChoiceCountsEvenIfItsAbilityNeverResolves) {
    auto tree = battlefield(0, kDreamingTree, P1);
    auto unit = addUnit(P1, kInvalidId, 2, 0);
    auto theirs = addUnit(P2, kInvalidId, 2, 0);
    pushSpellOnChain(P1, kPrimalStrength, {unit});
    pushSpellOnChain(P2, kPrimalStrength, {theirs});
    withEngine([&](GameEngine& e) {
        auto& items = e.mutableState().chain.items;
        auto from_tree = [&](PlayerId p) {
            return std::count_if(items.begin(), items.end(), [&](const ChainItem& i) {
                return i.source == tree && i.controller == p;
            });
        };
        events.emit(ObjectStateChangedEvent{unit, "chosen_by_controller"});
        EXPECT_EQ(from_tree(P1), 1);
        events.emit(ObjectStateChangedEvent{unit, "chosen_by_controller"});
        EXPECT_EQ(from_tree(P1), 1) << "the second choice is not the first this turn";
        // The ability leaves the chain without resolving, as a countered one does.
        items.erase(std::remove_if(items.begin(), items.end(),
                                   [&](const ChainItem& i) { return i.source == tree; }),
                    items.end());
        events.emit(ObjectStateChangedEvent{unit, "chosen_by_controller"});
        EXPECT_EQ(from_tree(P1), 0) << "the choice that was first stays the first";
        // "A player": the other player's first choice is their own.
        events.emit(ObjectStateChangedEvent{theirs, "chosen_by_controller"});
        EXPECT_EQ(from_tree(P2), 1);
        // A new turn: there is a first time again.
        e.mutableState().turn.turn_number++;
        events.emit(ObjectStateChangedEvent{unit, "chosen_by_controller"});
        EXPECT_EQ(from_tree(P1), 1);
    });
}

TEST_F(Card013Test, DreamingTree_OneDrawForTwoSpellsInATurn) {
    battlefield(0, kDreamingTree, P1);
    for (int i = 0; i < 4; ++i) addToDeck(P1, kRecruit);
    auto unit = addUnit(P1, kInvalidId, 2, 0);
    for (int n = 0; n < 2; ++n) {
        auto spell = addToHand(P1, kPrimalStrength);
        setPool(P1, 4, 1);
        EnginePathAgent agent;
        playSpell(spell, &agent, {unit});
    }
    EXPECT_EQ(o(unit).temp_might_bonus, 14);
    EXPECT_EQ(handSize(P1), 1);
}

// ── 273 Bandle Tree ─────────────────────────────────────────────────────────

TEST_F(Card013Test, BandleTree_ASecondCardCanBeHiddenThereAndNoThird) {
    battlefield(0, kBandleTree, P1);
    state.battlefields[1].controller = P1;
    addUnit(P1, kInvalidId, 2, 0);      // control that survives a cleanup
    addUnit(P1, kInvalidId, 2, 1);
    const std::vector<GameObjectId> cards{addToHand(P1, kFoxFire), addToHand(P1, kStandUnited),
                                          addToHand(P1, kSpriteCall)};
    setPool(P1, 0, 6);
    auto offeredAt = [&](GameObjectId card, int bf) {
        bool found = false;
        withEngine([&](GameEngine& e) {
            e.testHook_cleanup();
            for (const auto& i : e.generateLegalActions())
                if (i.type == IntentType::HideCard && i.card == card &&
                    i.chosen_battlefield == BattlefieldId::fromRaw(bf)) found = true;
        });
        return found;
    };
    auto hide = [&](GameObjectId card, int bf) {
        Intent i;
        i.type = IntentType::HideCard;
        i.player = P1;
        i.card = card;
        i.chosen_battlefield = BattlefieldId::fromRaw(bf);
        withEngine([&](GameEngine& e) {
            e.testHook_cleanup();
            e.testHook_executeIntent(i);
            settleLikeTheEngine(e);
        });
    };
    for (int n = 0; n < 2; ++n) {
        ASSERT_TRUE(offeredAt(cards[n], 0)) << "card " << n;
        hide(cards[n], 0);
        EXPECT_TRUE(o(cards[n]).is_hidden);
    }
    EXPECT_EQ(state.battlefields[0].facedown.size(), 2u) << "both stay through a cleanup";
    EXPECT_FALSE(offeredAt(cards[2], 0)) << "an ADDITIONAL card: one more, not any number";
    ASSERT_TRUE(offeredAt(cards[2], 1));
    hide(cards[2], 1);
    auto fourth = addToHand(P1, kFoxFire);
    EXPECT_FALSE(offeredAt(fourth, 1)) << "any other battlefield still holds one";
}

}  // namespace riftbound::test::card_013
