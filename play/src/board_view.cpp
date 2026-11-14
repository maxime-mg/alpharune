#include "board_view.h"

#include "core/card_db.h"
#include "core/game_state.h"
#include "io/state_renderer.h"

#include <algorithm>
#include <cctype>

namespace riftbound {

using json = nlohmann::json;

std::optional<Seat> parseSeat(const std::string& raw) {
    std::string s;
    for (char c : raw) s.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    if (s == "p1") return Seat::Player1;
    if (s == "p2") return Seat::Player2;
    if (s == "spectator") return Seat::Spectator;
    if (s == "god") return Seat::God;
    return std::nullopt;
}

const char* seatName(Seat s) {
    switch (s) {
        case Seat::Player1:   return "p1";
        case Seat::Player2:   return "p2";
        case Seat::Spectator: return "spectator";
        case Seat::God:       return "god";
    }
    return "spectator";
}

PlayerId seatPlayer(Seat s) {
    if (s == Seat::Player1) return PlayerId::Player1;
    if (s == Seat::Player2) return PlayerId::Player2;
    return PlayerId::None;
}

namespace {

const char* playerKey(PlayerId p) {
    switch (p) {
        case PlayerId::Player1: return "P1";
        case PlayerId::Player2: return "P2";
        default:                return "none";
    }
}

json playerOrNull(const std::optional<PlayerId>& p) {
    if (!p.has_value() || *p == PlayerId::None) return nullptr;
    return playerKey(*p);
}

class ViewBuilder {
public:
    ViewBuilder(const GameState& s, const CardDB& db, Seat viewer)
        : s_(s), db_(db), viewer_(viewer), me_(seatPlayer(viewer)) {}

    json build() const {
        json players = json::object();
        for (PlayerId p : {PlayerId::Player1, PlayerId::Player2})
            players[playerKey(p)] = player(p);
        json bfs = json::array();
        for (const auto& bf : s_.battlefields) bfs.push_back(battlefield(bf));
        return json{
            {"viewer", seatName(viewer_)},
            {"viewer_player", me_ == PlayerId::None ? json(nullptr) : json(playerKey(me_))},
            {"players", std::move(players)},
            {"battlefields", std::move(bfs)},
            {"turn", turn()},
            {"chain", chain()},
            {"cost", cost()},
        };
    }

private:
    bool god() const { return viewer_ == Seat::God; }

    /// A card the viewer may not see: no id, no name.
    static json back() { return json{{"back", true}}; }

    json object(GameObjectId id) const {
        const auto& o = s_.getObject(id);
        const CardDef* def = o.card_def_id != kInvalidId ? &db_.get(o.card_def_id) : nullptr;

        json kws = json::array();
        for (int k = 0; k < static_cast<int>(Keyword::Count); ++k) {
            const auto kw = static_cast<Keyword>(k);
            if (!o.hasKeyword(kw)) continue;
            int value = 0;
            if (kw == Keyword::Assault) value = o.assault_value + o.temp_assault_value;
            if (kw == Keyword::Shield)  value = o.shield_value + o.temp_shield_value;
            if (kw == Keyword::Deflect) value = o.deflect_value;
            json entry = {{"name", toString(kw)}};
            if (value > 0) entry["value"] = value;
            kws.push_back(std::move(entry));
        }
        json domains = json::array();
        for (auto d : (def ? def->domains : o.domains)) domains.push_back(toString(d));
        json attachments = json::array();
        for (auto a : o.attachments)
            if (s_.objectExists(a)) attachments.push_back(object(a));

        json j = {
            {"id", static_cast<int>(id.raw())},
            {"def_id", static_cast<int>(o.card_def_id.raw())},
            {"name", def ? def->name : (o.name.empty() ? std::string("(token)") : o.name.get())},
            {"type", toString(o.card_type)},
            {"owner", playerKey(o.owner)},
            {"controller", playerKey(o.controller)},
            {"domains", std::move(domains)},
            {"keywords", std::move(kws)},
            {"exhausted", o.is_exhausted},
            {"token", o.is_token},
            {"hidden", o.is_hidden},
            {"attachments", std::move(attachments)},
        };
        if (def) {
            j["energy_cost"] = def->energy_cost;
            j["power_cost"] = def->power_cost;
            j["text"] = def->ability_text;
            j["image"] = def->image_url;
            j["champion"] = def->super_type == SuperType::Champion;
        }
        if (o.isUnit()) {
            j["might"] = o.current_might;
            j["base_might"] = o.base_might;
            j["damage"] = o.damage_marked;
            j["stunned"] = o.is_stunned;
            j["buffs"] = o.buff_count + o.temp_buff_count;
            j["temp_might"] = o.temp_might_bonus;
            if (o.combat_designation != CombatDesignation::None)
                j["designation"] = toString(o.combat_designation);
        }
        if (o.empowered > 0) j["empowered"] = o.empowered;
        if (o.isRune()) j["rune_domains"] = j["domains"];
        return j;
    }

    json maybe(GameObjectId id) const {
        if (id == kInvalidId || !s_.objectExists(id)) return nullptr;
        return object(id);
    }

    /// Objects in a public list, in list order.
    json list(const std::vector<GameObjectId>& ids) const {
        json a = json::array();
        for (auto id : ids)
            if (s_.objectExists(id)) a.push_back(object(id));
        return a;
    }

    /// A private pile: the count for everyone, the cards for `who_sees`.
    json pile(const std::vector<GameObjectId>& ids, bool visible) const {
        json j = {{"count", ids.size()}};
        if (visible) {
            // Top of a deck is the back of the vector; show it first.
            json cards = json::array();
            for (auto it = ids.rbegin(); it != ids.rend(); ++it)
                if (s_.objectExists(*it)) cards.push_back(object(*it));
            j["cards"] = std::move(cards);
        }
        return j;
    }

    std::vector<GameObjectId> sortedObjects() const {
        std::vector<GameObjectId> ids;
        ids.reserve(s_.objects.size());
        for (const auto& [id, o] : s_.objects) ids.push_back(id);
        std::sort(ids.begin(), ids.end());
        return ids;
    }

    json player(PlayerId p) const {
        const auto& ps = s_.player(p);
        const bool own = god() || me_ == p;

        json base = json::array(), runes = json::array();
        for (auto id : sortedObjects()) {
            const auto& o = s_.getObject(id);
            if (o.zone != ZoneType::Base || !o.location) continue;
            if (!std::holds_alternative<BaseLocation>(*o.location)) continue;
            if (std::get<BaseLocation>(*o.location).player != p) continue;
            if (o.attached_to.has_value()) continue;  // shown on its host
            (o.isRune() ? runes : base).push_back(object(id));
        }

        json hand = {{"count", ps.hand.size()}};
        if (own) hand["cards"] = list(ps.hand);

        const auto& pool = ps.rune_pool;
        json power = json::object();
        for (int d = 0; d < static_cast<int>(Domain::Count); ++d)
            if (pool.power[d] > 0) power[toString(static_cast<Domain>(d))] = pool.power[d];

        return json{
            {"id", playerKey(p)},
            {"score", ps.score},
            {"xp", ps.xp},
            {"legend", maybe(ps.legend_zone)},
            {"champion", maybe(ps.champion_zone)},
            {"hand", std::move(hand)},
            {"main_deck", pile(ps.main_deck, god())},
            {"rune_deck", pile(ps.rune_deck, god())},
            {"trash", list(ps.trash)},
            {"banishment", list(ps.banishment)},
            {"base", std::move(base)},
            {"runes", std::move(runes)},
            {"pool", {
                {"energy", pool.energy},
                {"unit_energy", pool.unit_energy},
                {"power", std::move(power)},
                {"universal_power", pool.universal_power},
            }},
        };
    }

    json battlefield(const BattlefieldState& bf) const {
        json units = {{"P1", json::array()}, {"P2", json::array()}};
        for (auto id : sortedObjects()) {
            const auto& o = s_.getObject(id);
            if (o.zone != ZoneType::BattlefieldZone || !o.location) continue;
            if (!std::holds_alternative<BattlefieldLocation>(*o.location)) continue;
            if (std::get<BattlefieldLocation>(*o.location).id != bf.id) continue;
            if (o.is_hidden || o.attached_to.has_value()) continue;
            if (std::find(bf.facedown.begin(), bf.facedown.end(), id) != bf.facedown.end()) continue;
            units[playerKey(o.controller)].push_back(object(id));
        }
        json facedown = json::array();
        for (auto id : bf.facedown) {
            if (!s_.objectExists(id)) continue;
            const auto& o = s_.getObject(id);
            if (god() || o.controller == me_) {
                json card = object(id);
                card["hidden"] = true;
                facedown.push_back(std::move(card));
            } else {
                json b = back();
                b["controller"] = playerKey(o.controller);
                facedown.push_back(std::move(b));
            }
        }
        return json{
            {"id", static_cast<int>(bf.id.raw())},
            {"name", StateRenderer::battlefieldLabel(s_, bf.id)},
            {"card", maybe(bf.card_object_id)},
            {"controller", playerOrNull(bf.controller)},
            {"contested", bf.is_contested},
            {"contested_by", playerOrNull(bf.contested_by)},
            {"contributed_by", playerOrNull(bf.contributed_by)},
            {"is_token", bf.is_token},
            {"showdown", {{"staged", bf.showdown_staged}, {"in_progress", bf.showdown_in_progress}}},
            {"combat", {
                {"staged", bf.combat_staged},
                {"in_progress", bf.combat_in_progress},
                {"attacker", playerOrNull(bf.attacker)},
                {"defender", playerOrNull(bf.defender)},
            }},
            {"units", std::move(units)},
            {"facedown", std::move(facedown)},
        };
    }

    json turn() const {
        const auto& t = s_.turn;
        json passed_priority = json::array(), passed_focus = json::array();
        for (auto p : t.players_passed_priority) passed_priority.push_back(playerKey(p));
        for (auto p : t.players_passed_focus) passed_focus.push_back(playerKey(p));
        json j = {
            {"number", t.turn_number},
            {"phase", toString(t.phase)},
            {"turn_player", playerKey(t.turn_player)},
            {"priority", playerOrNull(t.priority_holder)},
            {"focus", playerOrNull(t.focus_holder)},
            {"showdown", t.ns_state == NeutralShowdownState::Showdown},
            {"closed", t.oc_state == OpenClosedState::Closed},
            {"passed_priority", std::move(passed_priority)},
            {"passed_focus", std::move(passed_focus)},
            {"victory_score", s_.mode.victory_score},
            {"game_over", s_.game_over},
        };
        if (s_.game_over) {
            j["winner"] = playerKey(s_.winner);
            j["reason"] = s_.game_over_reason;
        }
        return j;
    }

    json chain() const {
        json items = json::array();
        // Newest (top of the stack, the back of the vector) first.
        for (auto it = s_.chain.items.rbegin(); it != s_.chain.items.rend(); ++it) {
            const auto& c = *it;
            std::string name;
            if (c.card_def_id != kInvalidId) name = db_.get(c.card_def_id).name;
            else if (s_.objectExists(c.source)) name = s_.getObject(c.source).name.get();
            json targets = json::array();
            for (auto t : c.targets) {
                if (!s_.objectExists(t)) continue;
                const auto& o = s_.getObject(t);
                targets.push_back({{"id", static_cast<int>(t.raw())},
                                   {"name", o.card_def_id != kInvalidId ? db_.get(o.card_def_id).name : o.name.get()}});
            }
            items.push_back({
                {"id", static_cast<int>(c.id.raw())},
                {"name", name},
                {"source", maybe(c.source)},
                {"controller", playerKey(c.controller)},
                {"kind", c.is_spell ? "spell" : c.is_permanent ? "permanent" : c.is_ability ? "ability" : "other"},
                {"status", c.status == ChainItemStatus::Finalized ? "finalized" : "pending"},
                {"targets", std::move(targets)},
            });
        }
        return items;
    }

    json cost() const {
        if (!s_.cost_cursor.has_value()) return nullptr;
        const auto& c = *s_.cost_cursor;
        json domains = json::array();
        for (auto d : c.power_domains) domains.push_back(toString(d));
        return json{
            {"card", maybe(c.card)},
            {"player", playerKey(c.player)},
            {"energy_remaining", c.energy_remaining},
            {"power_remaining", c.power_remaining},
            {"power_domains", std::move(domains)},
            {"paying", c.phase == GameState::CostPaymentCursor::Phase::PayingEnergy ? "energy" : "power"},
        };
    }

    const GameState& s_;
    const CardDB& db_;
    Seat viewer_;
    PlayerId me_;
};

}  // namespace

json buildBoardView(const GameState& state, const CardDB& db, Seat viewer) {
    return ViewBuilder(state, db, viewer).build();
}

}  // namespace riftbound
