/// @file test_champion_zone_play.cpp
/// A Chosen Champion is played from the Champion Zone like a unit from hand
/// (CR 355.9.a.5, 813.3.a). It must get the same play locations as the same
/// card in hand (base, battlefields its controller controls, and whatever the
/// board grants or takes away), and the same Ambush and reaction-to-attack
/// plays.
///
/// Found while playing the board: the Chosen Champion was offered "play to
/// base" only, even with a battlefield under the player's control.

#include "tests/cards/card_test_fixture.h"

#include "engine/game_engine.h"

#include <set>

namespace riftbound::test {
namespace {

constexpr CardDefId kAhriAlluring = 66;   // unit, champion, 5E + 1 Calm power
constexpr CardDefId kInferna = 560;       // unit with [Ambush], 2E

std::string locKey(const LocationId& loc) {
    if (std::holds_alternative<BaseLocation>(loc)) return "base";
    return "bf" + std::to_string(std::get<BattlefieldLocation>(loc).id.raw());
}

class ChampionZonePlayTest : public CardTestFixture {
protected:
    std::unique_ptr<GameEngine> engine_;
    FirstChoiceAgent p1_agent_;
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
            bf.id = BattlefieldId::fromRaw(s.battlefields.size());
            s.battlefields.push_back(bf);
        }
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
        // Enough ready Calm runes for Ahri (5E + 1 Calm power).
        for (int i = 0; i < 7; ++i) rune(P1, Domain::Calm);
    }

    GameState& st() { return engine_->mutableState(); }

    GameObjectId card(PlayerId owner, CardDefId def_id) {
        auto& s = st();
        auto id = s.createObject();
        auto& o = s.getObject(id);
        const auto& def = card_db.get(def_id);
        o.card_def_id = def_id;
        o.owner = o.controller = owner;
        o.name = def.name;
        o.card_type = def.card_type;
        o.super_type = def.super_type;
        o.keywords = def.keywords;
        o.domains = def.domains;
        o.tags = def.tags;
        o.base_might = o.current_might = def.might;
        return id;
    }

    GameObjectId inChampionZone(PlayerId owner, CardDefId def_id) {
        auto id = card(owner, def_id);
        st().getObject(id).zone = ZoneType::ChampionZone;
        st().player(owner).champion_zone = id;
        return id;
    }

    GameObjectId inHand(PlayerId owner, CardDefId def_id) {
        auto id = card(owner, def_id);
        st().getObject(id).zone = ZoneType::Hand;
        st().player(owner).hand.push_back(id);
        return id;
    }

    void rune(PlayerId owner, Domain d) {
        auto& s = st();
        auto id = s.createObject();
        auto& r = s.getObject(id);
        r.owner = r.controller = owner;
        r.card_type = CardType::Rune;
        r.name = "Rune";
        r.domains = {d};
        r.zone = ZoneType::Base;
        r.location = BaseLocation{owner};
    }

    GameObjectId unitAt(PlayerId owner, int bf_slot) {
        const auto bf = BattlefieldId::fromRaw(bf_slot);
        auto& s = st();
        auto id = s.createObject();
        auto& u = s.getObject(id);
        u.owner = u.controller = owner;
        u.card_type = CardType::Unit;
        u.name = "Unit#" + std::to_string(id.raw());
        u.base_might = u.current_might = 2;
        u.zone = ZoneType::BattlefieldZone;
        u.location = BattlefieldLocation{bf};
        return id;
    }

    /// Where `id` may be played with a printed-cost PlayCard.
    std::set<std::string> playLocations(const std::vector<Intent>& actions,
                                        GameObjectId id, IntentType type) {
        std::set<std::string> out;
        for (const auto& a : actions)
            if (a.type == type && a.card == id && a.play_location &&
                !a.use_alt_play_cost)
                out.insert(locKey(*a.play_location));
        return out;
    }
};

TEST_F(ChampionZonePlayTest, ChampionCanBePlayedToABattlefieldYouControl) {
    st().battlefields[0].controller = P1;
    auto champ = inChampionZone(P1, kAhriAlluring);

    auto actions = engine_->testHook_generateMainPhaseActions(P1);
    EXPECT_EQ(playLocations(actions, champ, IntentType::PlayCard),
              (std::set<std::string>{"base", "bf0"}));
}

TEST_F(ChampionZonePlayTest, ChampionGetsTheSameLocationsAsTheCardInHand) {
    st().battlefields[1].controller = P1;
    auto champ = inChampionZone(P1, kAhriAlluring);
    auto copy = inHand(P1, kAhriAlluring);

    auto actions = engine_->testHook_generateMainPhaseActions(P1);
    auto from_zone = playLocations(actions, champ, IntentType::PlayCard);
    EXPECT_EQ(from_zone, playLocations(actions, copy, IntentType::PlayCard));
    EXPECT_EQ(from_zone, (std::set<std::string>{"base", "bf1"}));
}

TEST_F(ChampionZonePlayTest, ChampionFollowsBaseOnlyRestriction) {
    st().battlefields[0].controller = P1;
    st().player(P1).units_play_base_only = true;   // Mageseeker Warden
    auto champ = inChampionZone(P1, kAhriAlluring);

    auto actions = engine_->testHook_generateMainPhaseActions(P1);
    EXPECT_EQ(playLocations(actions, champ, IntentType::PlayCard),
              (std::set<std::string>{"base"}));
}

TEST_F(ChampionZonePlayTest, CantPlayCardsLockoutCoversTheChampion) {
    st().battlefields[0].controller = P1;
    st().player(P1).cant_play_cards_this_turn = true;   // Brynhir Thundersong
    auto champ = inChampionZone(P1, kAhriAlluring);

    auto actions = engine_->testHook_generateMainPhaseActions(P1);
    EXPECT_TRUE(playLocations(actions, champ, IntentType::PlayCard).empty());
}

TEST_F(ChampionZonePlayTest, PlayingToABattlefieldPutsTheChampionThere) {
    st().battlefields[0].controller = P1;
    auto champ = inChampionZone(P1, kAhriAlluring);

    Intent picked;
    for (const auto& a : engine_->testHook_generateMainPhaseActions(P1))
        if (a.type == IntentType::PlayCard && a.card == champ && a.play_location &&
            locKey(*a.play_location) == "bf0")
            picked = a;
    ASSERT_EQ(picked.card, champ);
    engine_->testHook_executeIntent(picked);

    const auto& o = st().getObject(champ);
    ASSERT_TRUE(o.location.has_value());
    EXPECT_EQ(locKey(*o.location), "bf0");
    // The zone follows the location once it resolves (it used to be left
    // at Base, which hid units played to a battlefield from the board view).
    EXPECT_EQ(o.zone, ZoneType::BattlefieldZone);
    EXPECT_EQ(st().player(P1).champion_zone, kInvalidId);
}

TEST_F(ChampionZonePlayTest, AmbushChampionCanBePlayedInAShowdown) {
    unitAt(P1, 0);
    auto champ = inChampionZone(P1, kInferna);
    auto& t = st().turn;
    t.ns_state = NeutralShowdownState::Showdown;
    t.oc_state = OpenClosedState::Open;
    t.focus_holder = P1;

    auto actions = engine_->generateLegalActions();
    EXPECT_EQ(playLocations(actions, champ, IntentType::PlayActionCard),
              (std::set<std::string>{"bf0"}));
}

TEST_F(ChampionZonePlayTest, AmbushChampionCanBePlayedAsAReaction) {
    unitAt(P1, 1);
    auto champ = inChampionZone(P1, kInferna);
    auto& t = st().turn;
    t.oc_state = OpenClosedState::Closed;
    t.priority_holder = P1;

    auto actions = engine_->generateLegalActions();
    EXPECT_EQ(playLocations(actions, champ, IntentType::PlayReaction),
              (std::set<std::string>{"bf1"}));
}

}  // namespace
}  // namespace riftbound::test
