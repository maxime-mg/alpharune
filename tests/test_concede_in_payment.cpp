/// @file test_concede_in_payment.cpp
/// A concession is accepted at every decision (GameEngine::askAgent), and at a
/// payment's prompts too it is the last thing anybody is asked: the payment in
/// progress is finished in the engine's own order and the game is over. The
/// rune prompts of a card's cost used to be asked again without end, and the
/// [Add] prompt up to its bound (GameEngine::askRunesFor,
/// EffectExecutor::addForPayment, EffectExecutor::runeChooser).
///
/// The agents here throw when they are asked much too often, so that a loop
/// that does not end is a failure and not a hang.

#include "cards/card_registry.h"
#include "cards/card_test_fixture.h"
#include "engine/game_engine.h"
#include "rules/deck_validator.h"

#include <gtest/gtest.h>

#include <boost/signals2/connection.hpp>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

namespace riftbound::test {
namespace test_concede_in_payment {  // file-unique: test sources are unity-built
namespace {

constexpr CardDefId kAnnieFiery{299};   // unit, [5] and one Fury Power
constexpr CardDefId kDredgeUp{836};     // spell, [2]; [Flow] [2]
constexpr CardDefId kGold{564};         // "[E], kill this: [Add] [A]"
constexpr int kTooOften = 1000;

/// Answers like `inner` until the first prompt of kind `kind`, concedes there
/// and at every such prompt after it, and counts what it is asked once the
/// game is over.
class ConcedesAt : public AgentInterface {
public:
    ConcedesAt(PlayerId me, PromptKind kind, AgentInterface* inner = nullptr)
        : me_(me), kind_(kind), inner_(inner ? inner : &default_) {}

    int conceded = 0;      // prompts answered with a concession
    int asked_after = 0;   // prompts met with the game already over

    Intent selectAction(const GameState& st, const std::vector<Intent>& legal) override {
        return inner_->selectAction(st, legal);
    }
    Intent decide(const GameState& st, const std::vector<Intent>& legal,
                  const Prompt& prompt) override {
        if (st.game_over && ++asked_after > kTooOften)
            throw std::runtime_error("a player is asked without end after the game is over");
        if (prompt.kind == kind_) {
            ++conceded;
            return Intent::concede(me_);
        }
        return inner_->decide(st, legal, prompt);
    }

private:
    PlayerId me_;
    PromptKind kind_;
    EnginePathAgent default_;
    AgentInterface* inner_;
};

class ConcedeInPaymentTest : public CardTestFixture {
protected:
    int game_overs = 0;
    boost::signals2::scoped_connection conn_;

    void SetUp() override {
        CardTestFixture::SetUp();
        conn_ = events.on_game_over.connect([this](const GameOverEvent&) { ++game_overs; });
    }
    /// P1 conceded at one prompt and was asked nothing after it.
    void expectP1Conceded(const ConcedesAt& p1) {
        EXPECT_EQ(p1.conceded, 1);
        EXPECT_EQ(p1.asked_after, 0) << "nobody is asked anything once the game is over";
        EXPECT_TRUE(state.game_over);
        EXPECT_EQ(state.winner, PlayerId::Player2);
        EXPECT_EQ(state.game_over_reason, "P1 conceded");
        EXPECT_EQ(game_overs, 1) << "announced once";
        EXPECT_FALSE(state.cost_cursor.has_value()) << "no payment left open";
    }
    /// A ready Gold token in `p`'s base.
    void addGold(PlayerId p) {
        auto& g = state.getObject(state.createObject());
        const auto& def = card_db.get(kGold);
        g.owner = g.controller = p;
        g.card_def_id = kGold;
        g.card_type = CardType::Gear;
        g.name = def.name;
        g.tags = def.tags;
        g.super_type = def.super_type;
        g.zone = ZoneType::Base;
        g.location = BaseLocation{p};
    }
    int exhaustedOf(const std::vector<GameObjectId>& runes) {
        return static_cast<int>(std::count_if(runes.begin(), runes.end(), [&](GameObjectId r) {
            return state.getObject(r).is_exhausted;
        }));
    }
};

TEST_F(ConcedeInPaymentTest, AtARuneForEnergyPrompt) {
    const auto annie = addToHand(PlayerId::Player1, kAnnieFiery);
    std::vector<GameObjectId> runes;
    for (int i = 0; i < 6; ++i) runes.push_back(addRune(PlayerId::Player1, Domain::Fury));
    ConcedesAt p1(PlayerId::Player1, PromptKind::RuneForEnergy);
    playThroughEngine(annie, &p1, std::nullopt, nullptr, /*fund=*/false);
    expectP1Conceded(p1);
    // Finished in the engine's order: five runes for [5], and one of those
    // recycled for the Power.
    EXPECT_EQ(state.player(PlayerId::Player1).rune_deck.size(), 1u);
    EXPECT_EQ(exhaustedOf(runes), 4);
}

TEST_F(ConcedeInPaymentTest, AtARuneForPowerPrompt) {
    const auto annie = addToHand(PlayerId::Player1, kAnnieFiery);
    for (int i = 0; i < 6; ++i) addRune(PlayerId::Player1, Domain::Fury);
    ConcedesAt p1(PlayerId::Player1, PromptKind::RuneForPower);
    playThroughEngine(annie, &p1, std::nullopt, nullptr, /*fund=*/false);
    expectP1Conceded(p1);
    EXPECT_EQ(state.player(PlayerId::Player1).rune_deck.size(), 1u);
}

// GameEngine::payLikeACardCost: a Flow cost, paid like a card's cost.
TEST_F(ConcedeInPaymentTest, AtARuneOfAPlayFromTheTrash) {
    const auto dredge = addToHand(PlayerId::Player1, kDredgeUp);
    {
        auto& h = state.player(PlayerId::Player1).hand;
        h.erase(std::remove(h.begin(), h.end(), dredge), h.end());
        state.getObject(dredge).zone = ZoneType::Trash;
        state.player(PlayerId::Player1).trash.push_back(dredge);
    }
    for (int i = 0; i < 3; ++i) addToDeck(PlayerId::Player1, kAnnieFiery);
    const std::vector<GameObjectId> runes{addRune(PlayerId::Player1, Domain::Mind),
                                          addRune(PlayerId::Player1, Domain::Mind)};
    Intent play;
    withEngine([&](GameEngine& e) {
        e.testHook_cleanup();
        for (const auto& i : e.generateLegalActions())
            if (i.card == dredge && i.play_source == Intent::PlaySource::Trash) { play = i; break; }
    });
    ASSERT_EQ(play.card, dredge) << "the Flow play is offered";
    ConcedesAt p1(PlayerId::Player1, PromptKind::RuneForEnergy);
    withEngine([&](GameEngine& e) {
        e.testHook_executeIntent(play);
        settleLikeTheEngine(e);
    }, &p1);
    expectP1Conceded(p1);
    EXPECT_EQ(exhaustedOf(runes), 2);
}

// EffectExecutor::payResources: runes of different kinds, each point asked.
// The concession at the first is the last question.
TEST_F(ConcedeInPaymentTest, AtARuneOfAnEffectsPayment) {
    for (Domain d : {Domain::Fury, Domain::Mind, Domain::Body, Domain::Calm})
        addRune(PlayerId::Player1, d);
    ConcedesAt p1(PlayerId::Player1, PromptKind::RuneForPower);
    bool paid = false;
    withEngine([&](GameEngine& e) {
        paid = e.testHook_executor().payAnyPower(PlayerId::Player1, 3);
    }, &p1);
    expectP1Conceded(p1);
    EXPECT_TRUE(paid);
    EXPECT_EQ(state.player(PlayerId::Player1).rune_deck.size(), 3u);
}

// EffectExecutor::addForPayment: more than one [Add] Reaction could pay. A
// payment its caller has already judged payable is still made: one that is
// not is an engine error to some of them (GameEngine::payAndQueueAbility).
TEST_F(ConcedeInPaymentTest, AtAnAddForPaymentPrompt) {
    for (int i = 0; i < 2; ++i) addRune(PlayerId::Player1, Domain::Body);
    for (int i = 0; i < 3; ++i) addGold(PlayerId::Player1);
    ConcedesAt p1(PlayerId::Player1, PromptKind::AddForPayment);
    bool paid = false;
    withEngine([&](GameEngine& e) {
        paid = e.testHook_executor().payAnyPower(PlayerId::Player1, 4);
    }, &p1);
    expectP1Conceded(p1);
    EXPECT_TRUE(paid);
}

// Whole games: the engine's own turn loop returns, whichever seat concedes.
TEST_F(ConcedeInPaymentTest, AWholeGameEndsThere) {
    /// Plays a card when it can, else takes the last option.
    class Plays : public AgentInterface {
    public:
        Intent selectAction(const GameState&, const std::vector<Intent>& legal) override {
            for (const auto& i : legal)
                if (i.type == IntentType::PlayCard) return i;
            return legal.back();
        }
    };
    const char* root = std::getenv("RIFTBOUND_ROOT");
    const std::string decks = std::string(root ? root : "..") + "/decks/";
    const auto d1 = DeckValidator::loadFromDeckList(decks + "ahri.txt", card_db);
    const auto d2 = DeckValidator::loadFromDeckList(decks + "fiora.txt", card_db);
    for (PromptKind kind : {PromptKind::RuneForEnergy, PromptKind::RuneForPower}) {
        for (PlayerId seat : {PlayerId::Player1, PlayerId::Player2}) {
            SCOPED_TRACE(std::string(kind == PromptKind::RuneForEnergy ? "energy, " : "power, ") +
                         toString(seat));
            Plays plays, other;
            ConcedesAt quits(seat, kind, &plays);
            EventBus bus;
            GameEngine engine(card_db, bus, card_registry);
            const GameResult r = seat == PlayerId::Player1
                ? engine.runGame(d1, d2, quits, other, 7)
                : engine.runGame(d1, d2, other, quits, 7);
            EXPECT_EQ(quits.conceded, 1);
            EXPECT_EQ(quits.asked_after, 0);
            EXPECT_EQ(r.winner, opponent(seat));
            EXPECT_EQ(r.termination_reason, std::string(toString(seat)) + " conceded");
        }
    }
}

}  // namespace
}  // namespace test_concede_in_payment
}  // namespace riftbound::test
