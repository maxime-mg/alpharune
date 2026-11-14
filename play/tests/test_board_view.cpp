/// @file test_board_view.cpp
/// buildBoardView must show each seat exactly what it may see: own hand,
/// public zones, deck counts. Hidden cards must not leak by name or by id.

#include "board_view.h"

#include "io/state_renderer.h"
#include "tests/cards/card_test_fixture.h"

#include <set>

namespace riftbound::test {
namespace {

using json = nlohmann::json;

/// Every "id" anywhere in the view, and every "name".
void collect(const json& j, std::set<int>& ids, std::set<std::string>& names) {
    if (j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.key() == "id" && it->is_number_integer()) ids.insert(it->get<int>());
            if (it.key() == "name" && it->is_string()) names.insert(it->get<std::string>());
            collect(*it, ids, names);
        }
    } else if (j.is_array()) {
        for (const auto& e : j) collect(e, ids, names);
    }
}

class BoardViewTest : public CardTestFixture {
protected:
    // Distinct real cards, so a leak shows up as a name.
    CardDefId p1_hand_def, p2_hand_def, p2_deck_def, p2_hidden_def, trash_def;
    GameObjectId p1_hand, p2_hand, p2_deck, p2_rune, p2_hidden, p2_trash, p1_unit;

    std::string nameOf(CardDefId d) { return card_db.get(d).name; }

    void SetUp() override {
        CardTestFixture::SetUp();
        // Pick five units with different names from the card table.
        std::vector<CardDefId> picks;
        std::set<std::string> seen;
        std::vector<CardDefId> ids;
        for (const auto& [id, def] : card_db.all()) ids.push_back(id);
        std::sort(ids.begin(), ids.end());
        for (auto id : ids) {
            const auto& def = card_db.get(id);
            if (def.card_type != CardType::Unit || seen.count(def.name)) continue;
            seen.insert(def.name);
            picks.push_back(id);
            if (picks.size() == 5) break;
        }
        ASSERT_EQ(picks.size(), 5u);
        p1_hand_def = picks[0]; p2_hand_def = picks[1]; p2_deck_def = picks[2];
        p2_hidden_def = picks[3]; trash_def = picks[4];

        p1_hand = addToHand(P1, p1_hand_def);
        p2_hand = addToHand(P2, p2_hand_def);
        p2_deck = addToDeck(P2, p2_deck_def);

        p2_rune = addRune(P2, Domain::Fury);
        {
            auto& r = state.getObject(p2_rune);
            r.zone = ZoneType::RuneDeck;
            r.location.reset();
            state.player(P2).rune_deck.push_back(p2_rune);
        }

        // A face-down P2 card at battlefield 0.
        p2_hidden = addToHand(P2, p2_hidden_def);
        {
            auto& h = state.player(P2).hand;
            h.erase(std::remove(h.begin(), h.end(), p2_hidden), h.end());
            auto& o = state.getObject(p2_hidden);
            o.zone = ZoneType::FacedownZone;
            o.location = BattlefieldLocation{0};
            o.is_hidden = true;
            state.battlefields[0].facedown.push_back(p2_hidden);
        }

        p2_trash = addToHand(P2, trash_def);
        {
            auto& h = state.player(P2).hand;
            h.erase(std::remove(h.begin(), h.end(), p2_trash), h.end());
            state.getObject(p2_trash).zone = ZoneType::Trash;
            state.player(P2).trash.push_back(p2_trash);
        }
        p1_unit = addUnit(P1, kInvalidId, 3, 1);
    }

    json view(Seat s) { return buildBoardView(state, card_db, s); }
};

TEST_F(BoardViewTest, PlayerSeesOwnHandButNotTheOpponents) {
    const auto v = view(Seat::Player1);
    std::set<int> ids;
    std::set<std::string> names;
    collect(v, ids, names);

    EXPECT_TRUE(names.count(nameOf(p1_hand_def)));
    EXPECT_TRUE(ids.count(static_cast<int>(p1_hand.raw())));
    EXPECT_FALSE(names.count(nameOf(p2_hand_def)));
    EXPECT_FALSE(ids.count(static_cast<int>(p2_hand.raw())));
    EXPECT_EQ(v["players"]["P2"]["hand"]["count"], 1);
    EXPECT_FALSE(v["players"]["P2"]["hand"].contains("cards"));
}

TEST_F(BoardViewTest, DeckContentsAreHiddenFromEverySeatButGod) {
    for (Seat s : {Seat::Player1, Seat::Player2, Seat::Spectator}) {
        const auto v = view(s);
        std::set<int> ids;
        std::set<std::string> names;
        collect(v, ids, names);
        EXPECT_FALSE(names.count(nameOf(p2_deck_def))) << seatName(s);
        EXPECT_FALSE(ids.count(static_cast<int>(p2_deck.raw()))) << seatName(s);
        EXPECT_FALSE(ids.count(static_cast<int>(p2_rune.raw()))) << seatName(s);
        EXPECT_EQ(v["players"]["P2"]["main_deck"]["count"], 1) << seatName(s);
        EXPECT_EQ(v["players"]["P2"]["rune_deck"]["count"], 1) << seatName(s);
        EXPECT_FALSE(v["players"]["P2"]["main_deck"].contains("cards")) << seatName(s);
    }
    const auto god = view(Seat::God);
    ASSERT_TRUE(god["players"]["P2"]["main_deck"].contains("cards"));
    EXPECT_EQ(god["players"]["P2"]["main_deck"]["cards"][0]["name"], nameOf(p2_deck_def));
}

TEST_F(BoardViewTest, FaceDownCardsShowOnlyToTheirController) {
    const auto p1_view = view(Seat::Player1);
    const auto& p1_fd = p1_view["battlefields"][0]["facedown"];
    ASSERT_EQ(p1_fd.size(), 1u);
    EXPECT_TRUE(p1_fd[0].value("back", false));
    EXPECT_FALSE(p1_fd[0].contains("id"));
    EXPECT_FALSE(p1_fd[0].contains("name"));
    EXPECT_EQ(p1_fd[0]["controller"], "P2");

    for (Seat s : {Seat::Player2, Seat::God}) {
        const auto v = view(s);
        const auto& fd = v["battlefields"][0]["facedown"];
        ASSERT_EQ(fd.size(), 1u) << seatName(s);
        EXPECT_EQ(fd[0]["name"], nameOf(p2_hidden_def)) << seatName(s);
        EXPECT_TRUE(fd[0]["hidden"].get<bool>()) << seatName(s);
    }
    // And never listed as a unit at the battlefield, for anyone.
    for (Seat s : {Seat::Player1, Seat::Player2, Seat::Spectator, Seat::God})
        EXPECT_TRUE(view(s)["battlefields"][0]["units"]["P2"].empty()) << seatName(s);
}

TEST_F(BoardViewTest, PublicZonesAreVisibleToEveryone) {
    for (Seat s : {Seat::Player1, Seat::Player2, Seat::Spectator}) {
        const auto v = view(s);
        ASSERT_EQ(v["players"]["P2"]["trash"].size(), 1u) << seatName(s);
        EXPECT_EQ(v["players"]["P2"]["trash"][0]["name"], nameOf(trash_def)) << seatName(s);
        const auto& units = v["battlefields"][1]["units"]["P1"];  // v outlives this
        ASSERT_EQ(units.size(), 1u) << seatName(s);
        EXPECT_EQ(units[0]["id"], static_cast<int>(p1_unit.raw())) << seatName(s);
        EXPECT_EQ(units[0]["might"], 3) << seatName(s);
    }
}

TEST_F(BoardViewTest, SpectatorSeesNoHand) {
    const auto v = view(Seat::Spectator);
    EXPECT_FALSE(v["players"]["P1"]["hand"].contains("cards"));
    EXPECT_FALSE(v["players"]["P2"]["hand"].contains("cards"));
    EXPECT_EQ(v["viewer_player"], nullptr);
}

// The text view each seat receives must hide the other hand too.
TEST_F(BoardViewTest, TextViewHidesOtherHands) {
    StateRenderer r(card_db);
    const std::string p1_card = nameOf(p1_hand_def), p2_card = nameOf(p2_hand_def);
    EXPECT_NE(r.render(state).find(p2_card), std::string::npos) << "default prints both hands";

    // The server renders a seat's text with that seat's RenderVisibility.
    const auto p1_text = r.render(state, RenderVisibility{true, false});
    EXPECT_NE(p1_text.find(p1_card), std::string::npos);
    EXPECT_EQ(p1_text.find(p2_card), std::string::npos);

    const auto spec_text = r.render(state, RenderVisibility{false, false});  // spectator
    EXPECT_EQ(spec_text.find(p1_card), std::string::npos);
    EXPECT_EQ(spec_text.find(p2_card), std::string::npos);
}

TEST(BoardViewSeat, ParsesSeatNames) {
    EXPECT_EQ(parseSeat("p1"), Seat::Player1);
    EXPECT_EQ(parseSeat("P2"), Seat::Player2);
    EXPECT_EQ(parseSeat("spectator"), Seat::Spectator);
    EXPECT_EQ(parseSeat("god"), Seat::God);
    EXPECT_EQ(parseSeat("admin"), std::nullopt);
    EXPECT_EQ(seatPlayer(Seat::God), PlayerId::None);
}

}  // namespace
}  // namespace riftbound::test
