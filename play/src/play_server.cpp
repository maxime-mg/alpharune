#include "play_server.h"
#include "board_view.h"
#include "classic_page.h"
#include "human_agent.h"

#include "io/replay_writer.h"
#include "io/state_editor.h"
#include "io/state_renderer.h"
#include "core/card_db.h"
#include "core/game_state.h"
#include "core/intent.h"
#include "engine/game_engine.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/post.hpp>
#include <nlohmann/json.hpp>

#include <cctype>
#include <chrono>
#include <csignal>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <iostream>
#include <list>
#include <memory>
#include <set>

namespace beast = boost::beast;
namespace http  = beast::http;
namespace ws    = beast::websocket;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;
using json      = nlohmann::json;

namespace riftbound {

// ─── Forward decls ─────────────────────────────────────────────────────────
namespace {
class WsSession;

std::string playerToStr(PlayerId p) {
    switch (p) {
        case PlayerId::Player1: return "P1";
        case PlayerId::Player2: return "P2";
        default: return "none";
    }
}

PlayerId strToPlayer(const std::string& s) {
    if (s == "P1") return PlayerId::Player1;
    if (s == "P2") return PlayerId::Player2;
    return PlayerId::None;
}

const char* zoneToStr(ZoneType z) {
    switch (z) {
        case ZoneType::MainDeck:        return "MainDeck";
        case ZoneType::RuneDeck:        return "RuneDeck";
        case ZoneType::Hand:            return "Hand";
        case ZoneType::Base:            return "Base";
        case ZoneType::Trash:           return "Trash";
        case ZoneType::Banishment:      return "Banishment";
        case ZoneType::ChampionZone:    return "ChampionZone";
        case ZoneType::LegendZone:      return "LegendZone";
        case ZoneType::BattlefieldZone: return "BattlefieldZone";
        case ZoneType::Chain:           return "Chain";
        case ZoneType::FacedownZone:    return "FacedownZone";
    }
    return "?";
}

ZoneType strToZone(const std::string& s) {
    if (s == "MainDeck")        return ZoneType::MainDeck;
    if (s == "RuneDeck")        return ZoneType::RuneDeck;
    if (s == "Hand")            return ZoneType::Hand;
    if (s == "Base")            return ZoneType::Base;
    if (s == "Trash")           return ZoneType::Trash;
    if (s == "Banishment")      return ZoneType::Banishment;
    if (s == "ChampionZone")    return ZoneType::ChampionZone;
    if (s == "LegendZone")      return ZoneType::LegendZone;
    if (s == "BattlefieldZone") return ZoneType::BattlefieldZone;
    if (s == "Chain")           return ZoneType::Chain;
    if (s == "FacedownZone")    return ZoneType::FacedownZone;
    return ZoneType::Hand;  // safe fallback
}

} // namespace

// ─── Impl ──────────────────────────────────────────────────────────────────

struct PlayServer::Impl {
    PlayServer& parent;
    net::io_context ioc;
    std::optional<tcp::acceptor> acceptor;
    std::optional<net::signal_set> signals;
    std::thread io_thread;
    std::thread watcher_thread;

    // Precomputed HTML index page, built once at server start so the
    // CARD_IMAGES JSON / version banner are baked in.
    std::string index_html;

    // Active WS sessions — used to broadcast decision frames.
    std::mutex sessions_mu;
    std::list<std::weak_ptr<WsSession>> sessions;

    // Watcher coordination.
    std::atomic<bool> stopping{false};
    uint64_t last_p1_decision_id = 0;
    uint64_t last_p2_decision_id = 0;
    // Tracks whether the most recent broadcast was a "decision" frame.
    // Lets the watcher detect the at-decision→engine-busy transition
    // (human just clicked, AI is now thinking) and push a state frame
    // so the UI clears the stale buttons instead of letting the user
    // re-click an already-consumed decision.
    bool was_at_decision = false;

    StateEditor editor;

    // Most recent prompt-style trace line, captured by broadcastLog as
    // it streams. Cards / engine emit `<X>_PROMPT: <text>` right before
    // requesting an agent choice (MAY_PROMPT, X_PROMPT, MODE_PROMPT,
    // TGT_PROMPT). We attach the latest one to the next outbound
    // decision frame so the UI can show "what are we choosing?"
    // alongside the otherwise opaque MakeChoice options.
    std::mutex prompt_mu;
    std::string last_prompt;

    // Text view for every seat but god: StateRenderer::show_hand also
    // names face-down cards, so only the god seat gets the caller's
    // renderer (which may have show_hand on). Each seat prints only its
    // own hand; the spectator's prints neither.
    StateRenderer p1_renderer, p2_renderer, spectator_renderer;

    explicit Impl(PlayServer& p)
        : parent(p), p1_renderer(p.card_db_), p2_renderer(p.card_db_), spectator_renderer(p.card_db_) {
        for (auto* r : {&p1_renderer, &p2_renderer, &spectator_renderer}) r->hide_other_hands = true;
        p1_renderer.hand_viewer = PlayerId::Player1;
        p2_renderer.hand_viewer = PlayerId::Player2;
    }

    const StateRenderer& rendererFor(Seat seat) const {
        switch (seat) {
            case Seat::God:       return parent.renderer_;
            case Seat::Player1:   return p1_renderer;
            case Seat::Player2:   return p2_renderer;
            case Seat::Spectator: return spectator_renderer;
        }
        return spectator_renderer;
    }

    /// Seat for a new connection, from the `seat` query parameter.
    /// No parameter (the classic page): god when god mode is on, else spectator.
    /// "auto": the only human seat, P1 when both are human, else spectator.
    /// "god" without god mode falls back to spectator.
    Seat resolveSeat(const std::string& target) const;

    /// Seats that are human, for the client's seat picker.
    json humanSeats() const;

    /// The decision or state frame as `seat` may see it.
    std::string frameFor(Seat seat, PlayerId active_player,
                         const std::vector<Intent>& legal, uint64_t decision_id,
                         bool at_decision, const std::string& prompt);

    void broadcastDecisionFrame();
    void runWatcher();
    void armSignalSet();
};

// ─── WS Session ────────────────────────────────────────────────────────────
namespace {

class WsSession : public std::enable_shared_from_this<WsSession> {
public:
    WsSession(tcp::socket socket, PlayServer::Impl& server_impl, PlayServer& server, Seat seat)
        : ws_(std::move(socket)), impl_(server_impl), server_(server), seat_(seat) {}

    Seat seat() const { return seat_; }

    void run(http::request<http::string_body> req) {
        ws_.set_option(ws::stream_base::timeout::suggested(beast::role_type::server));
        ws_.set_option(ws::stream_base::decorator(
            [](ws::response_type& res) {
                res.set(http::field::server, "riftbound_play");
            }));
        auto self = shared_from_this();
        ws_.async_accept(req,
            [self](beast::error_code ec) {
                if (ec) return;
                self->onAccept();
            });
    }

    /// Thread-safe send. Posts onto the ws's executor so that the
    /// write queue and async_write call run on the IO thread.
    void send(std::string text) {
        auto self = shared_from_this();
        net::post(ws_.get_executor(),
            [self, t = std::move(text)]() mutable {
                self->queue_.push_back(std::move(t));
                if (!self->writing_) self->doWrite();
            });
    }

private:
    void onAccept() {
        // Push initial decision frame if engine is already at a decision.
        impl_.broadcastDecisionFrame();
        doRead();
    }

    void doRead() {
        auto self = shared_from_this();
        ws_.async_read(read_buf_,
            [self](beast::error_code ec, std::size_t /*n*/) {
                if (ec == ws::error::closed || ec == net::error::operation_aborted) return;
                if (ec) return;
                self->onMessage();
                self->read_buf_.consume(self->read_buf_.size());
                self->doRead();
            });
    }

    void onMessage() {
        std::string msg(beast::buffers_to_string(read_buf_.data()));
        json m;
        try { m = json::parse(msg); }
        catch (const std::exception& e) {
            send(json{{"type", "err"}, {"msg", std::string("bad json: ") + e.what()}}.dump());
            return;
        }
        auto type = m.value("type", "");
        if      (type == "choose")             handleChoose(m);
        else if (type == "edit_move")          handleEditMove(m);
        else if (type == "edit_object")        handleEditObject(m);
        else if (type == "edit_player")        handleEditPlayer(m);
        else if (type == "edit_phase")         handleEditPhase(m);
        else if (type == "edit_reorder_deck")  handleEditReorderDeck(m);
        else if (type == "request_state")      impl_.broadcastDecisionFrame();
        else {
            send(json{{"type", "err"}, {"msg", "unknown message type: " + type}}.dump());
        }
    }

    void handleChoose(const json& m) {
        auto player_str = m.value("player", "");
        PlayerId pid = strToPlayer(player_str);
        int idx = m.value("index", -1);
        // A seat may only answer for itself; god may answer for either.
        if (seat_ != Seat::God && seatPlayer(seat_) != pid) {
            send(json{{"type", "err"}, {"msg", std::string("seat ") + seatName(seat_) +
                                               " cannot choose for " + player_str}}.dump());
            return;
        }
        HumanAgent* agent = nullptr;
        if (pid == PlayerId::Player1) agent = server_.config_p1();
        if (pid == PlayerId::Player2) agent = server_.config_p2();
        if (!agent) {
            send(json{{"type", "edit_err"}, {"msg", "no human agent for " + player_str}}.dump());
            return;
        }
        if (idx < 0) {
            send(json{{"type", "edit_err"}, {"msg", "invalid index"}}.dump());
            return;
        }
        agent->setChosenAction(idx);
    }

    bool requireGodMode() {
        if (!server_.godModeEnabled()) {
            send(json{{"type", "edit_err"}, {"msg", "god mode disabled (no human seat)"}}.dump());
            return false;
        }
        if (seat_ != Seat::God) {
            send(json{{"type", "edit_err"}, {"msg", "edits need the god seat"}}.dump());
            return false;
        }
        return true;
    }

    void handleEditMove(const json& m) {
        if (!requireGodMode()) return;
        GameObjectId oid = static_cast<GameObjectId>(m.value("object_id", 0));
        PlayerId pid = strToPlayer(m.value("to_player", ""));
        std::string zone_str = m.value("to_zone", "");
        bool to_top = m.value("to_top", true);

        EditResult res;
        {
            std::lock_guard<std::mutex> lock(server_.stateMutex());
            auto& state = server_.engineMutableState();
            if (zone_str == "BattlefieldZone") {
                BattlefieldId bid = static_cast<BattlefieldId>(m.value("battlefield_id", 0));
                res = impl_.editor.moveObjectToBattlefield(state, oid, bid);
            } else {
                res = impl_.editor.moveObject(state, oid, pid, strToZone(zone_str), to_top);
            }
        }
        replyEdit(res);
    }

    void handleEditObject(const json& m) {
        if (!requireGodMode()) return;
        GameObjectId oid = static_cast<GameObjectId>(m.value("object_id", 0));
        std::string field = m.value("field", "");
        EditResult res = EditResult::fail("unknown object field: " + field);
        {
            std::lock_guard<std::mutex> lock(server_.stateMutex());
            auto& state = server_.engineMutableState();
            if      (field == "exhausted") res = impl_.editor.setObjectExhausted(state, oid, m.value("value", false));
            else if (field == "damage")    res = impl_.editor.setObjectDamage(state, oid, m.value("value", 0));
            else if (field == "might")     res = impl_.editor.setObjectMight(state, oid, m.value("value", 0));
        }
        replyEdit(res);
    }

    void handleEditPlayer(const json& m) {
        if (!requireGodMode()) return;
        PlayerId pid = strToPlayer(m.value("player", ""));
        std::string field = m.value("field", "");
        EditResult res = EditResult::fail("unknown player field: " + field);
        {
            std::lock_guard<std::mutex> lock(server_.stateMutex());
            auto& state = server_.engineMutableState();
            if      (field == "score")           res = impl_.editor.setPlayerScore(state, pid, m.value("value", 0));
            else if (field == "energy")          res = impl_.editor.setPlayerEnergy(state, pid, m.value("value", 0));
            else if (field == "power")           res = impl_.editor.setPlayerPower(state, pid, m.value("domain_idx", -1), m.value("value", 0));
        }
        replyEdit(res);
    }

    void handleEditPhase(const json& m) {
        if (!requireGodMode()) return;
        std::string field = m.value("field", "");
        EditResult res = EditResult::fail("unknown phase field: " + field);
        {
            std::lock_guard<std::mutex> lock(server_.stateMutex());
            auto& state = server_.engineMutableState();
            if (field == "phase") {
                int p = m.value("value", 0);
                res = impl_.editor.setPhase(state, static_cast<TurnPhase>(p));
            } else if (field == "turn_player") {
                res = impl_.editor.setTurnPlayer(state, strToPlayer(m.value("value", "")));
            }
        }
        replyEdit(res);
    }

    void handleEditReorderDeck(const json& m) {
        if (!requireGodMode()) return;
        PlayerId pid = strToPlayer(m.value("player", ""));
        std::vector<GameObjectId> order;
        for (auto& v : m.value("order", json::array())) {
            order.push_back(static_cast<GameObjectId>(v.get<int>()));
        }
        EditResult res;
        {
            std::lock_guard<std::mutex> lock(server_.stateMutex());
            res = impl_.editor.reorderMainDeck(server_.engineMutableState(), pid, order);
        }
        replyEdit(res);
    }

    void replyEdit(const EditResult& res) {
        if (res.ok) send(json{{"type", "edit_ok"}}.dump());
        else        send(json{{"type", "edit_err"}, {"msg", res.error}}.dump());
        // Re-broadcast state so the UI re-renders post-edit.
        impl_.broadcastDecisionFrame();
    }

    void doWrite() {
        if (queue_.empty()) return;
        writing_ = true;
        auto self = shared_from_this();
        ws_.text(true);
        ws_.async_write(net::buffer(queue_.front()),
            [self](beast::error_code ec, std::size_t /*n*/) {
                self->writing_ = false;
                if (ec) {
                    self->queue_.clear();
                    return;
                }
                self->queue_.pop_front();
                if (!self->queue_.empty()) self->doWrite();
            });
    }

    ws::stream<tcp::socket>      ws_;
    PlayServer::Impl&            impl_;
    PlayServer&                  server_;
    beast::flat_buffer           read_buf_;
    std::deque<std::string>      queue_;
    bool                         writing_ = false;
    Seat                         seat_;
};

// ─── HTTP-then-WS routing ───────────────────────────────────────────────────

/// Reads one file from the web root. Only plain names directly under the
/// root are served ("/board.js"), so a request cannot climb out of it.
bool readWebFile(const std::string& root, const std::string& path,
                 std::string& body, std::string& type) {
    if (root.empty() || path.size() < 2 || path[0] != '/') return false;
    const std::string name = path.substr(1);
    for (char c : name)
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_'))
            return false;
    if (name.find("..") != std::string::npos) return false;
    std::ifstream f(std::filesystem::path(root) / name, std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    body = ss.str();
    static const std::map<std::string, std::string> kTypes = {
        {".html", "text/html; charset=utf-8"}, {".js", "text/javascript; charset=utf-8"},
        {".css", "text/css; charset=utf-8"},   {".svg", "image/svg+xml"},
        {".png", "image/png"},                  {".json", "application/json"},
    };
    const auto dot = name.rfind('.');
    const auto it = dot == std::string::npos ? kTypes.end() : kTypes.find(name.substr(dot));
    type = it == kTypes.end() ? "application/octet-stream" : it->second;
    return true;
}

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket socket, PlayServer::Impl& impl, PlayServer& server)
        : socket_(std::move(socket)), impl_(impl), server_(server) {}

    void run() {
        doRead();
    }

private:
    void doRead() {
        req_ = {};
        auto self = shared_from_this();
        http::async_read(socket_, buf_, req_,
            [self](beast::error_code ec, std::size_t /*n*/) {
                if (ec) return;
                self->onRequest();
            });
    }

    void onRequest() {
        if (ws::is_upgrade(req_)) {
            // Hand off to a WsSession that owns the socket.
            const Seat seat = impl_.resolveSeat(std::string(req_.target()));
            auto session = std::make_shared<WsSession>(std::move(socket_), impl_, server_, seat);
            {
                std::lock_guard<std::mutex> lock(impl_.sessions_mu);
                impl_.sessions.push_back(session);
            }
            session->run(std::move(req_));
            return;
        }
        // Otherwise: the board from the web root, the classic page at
        // /classic, 404 elsewhere.
        http::response<http::string_body> res;
        std::string path(req_.target());
        if (auto q = path.find('?'); q != std::string::npos) path.resize(q);
        std::string body, type;
        if (req_.method() != http::verb::get) {
            res.result(http::status::method_not_allowed);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Method Not Allowed";
        } else if (path == "/classic") {
            res.result(http::status::ok);
            res.set(http::field::content_type, "text/html; charset=utf-8");
            res.body() = impl_.index_html;
        } else if (readWebFile(server_.webRoot(), path == "/" ? "/index.html" : path, body, type)) {
            res.result(http::status::ok);
            res.set(http::field::content_type, type);
            res.set(http::field::cache_control, "no-store");
            res.body() = std::move(body);
        } else {
            res.result(http::status::not_found);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Not Found";
        }
        res.set(http::field::server, "riftbound_play");
        res.prepare_payload();
        auto self = shared_from_this();
        auto sp = std::make_shared<http::response<http::string_body>>(std::move(res));
        http::async_write(socket_, *sp,
            [self, sp](beast::error_code, std::size_t) {
                beast::error_code ec;
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            });
    }

    tcp::socket                       socket_;
    beast::flat_buffer                buf_;
    http::request<http::string_body>  req_;
    PlayServer::Impl&                 impl_;
    PlayServer&                       server_;
};

// ─── Listener ───────────────────────────────────────────────────────────────

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(tcp::acceptor& acceptor, PlayServer::Impl& impl, PlayServer& server)
        : acceptor_(acceptor), impl_(impl), server_(server) {}

    void start() { doAccept(); }

private:
    void doAccept() {
        auto self = shared_from_this();
        acceptor_.async_accept(
            [self](beast::error_code ec, tcp::socket socket) {
                if (ec) {
                    if (ec == net::error::operation_aborted) return;
                    self->doAccept();
                    return;
                }
                std::make_shared<HttpSession>(std::move(socket),
                                              self->impl_, self->server_)->run();
                self->doAccept();
            });
    }

    tcp::acceptor&    acceptor_;
    PlayServer::Impl& impl_;
    PlayServer&       server_;
};

} // namespace

// ─── State snapshot builder ────────────────────────────────────────────────

namespace {

json buildLegalActionJson(const CardDB& db,
                          const StateRenderer& renderer,
                          const GameState& state,
                          const std::vector<Intent>& actions) {
    json arr = json::array();
    for (size_t i = 0; i < actions.size(); ++i) {
        const auto& a = actions[i];
        std::string label = renderer.renderChosenAction(state, a);

        // Special-case the pre-game ChooseBattlefield decision: the
        // generic renderer just emits "ChooseBattlefield bf=N" where N
        // is the deck index — useless to a player. Look up the actual
        // BF CardDef name from the controller's battlefield_pool and
        // surface it. (After game start, the same intent type is also
        // used by some card effects targeting an EXISTING battlefield;
        // the renderer's regular path handles those.)
        if (a.type == IntentType::ChooseBattlefield) {
            const auto& pool = state.player(a.player).battlefield_pool;
            auto idx = static_cast<size_t>(a.chosen_battlefield);
            if (idx < pool.size()) {
                const auto& def = db.get(pool[idx]);
                label = "Choose battlefield: " + def.name;
                if (!def.ability_text.empty()) {
                    // Trim to the first sentence so the button stays compact.
                    auto first_dot = def.ability_text.find('.');
                    std::string flavour = first_dot == std::string::npos
                        ? def.ability_text
                        : def.ability_text.substr(0, first_dot + 1);
                    label += "  — " + flavour;
                }
            }
        }

        // The renderer can include newlines / "→" arrows; collapse for buttons.
        for (auto& c : label) if (c == '\n') c = ' ';
        json item = {
            {"index", static_cast<int>(i)},
            {"label", label.empty() ? a.describe() : label},
            {"type",  a.describe().substr(0, a.describe().find('('))},
        };
        if (a.card != kInvalidId) item["card_object_id"] = static_cast<int>(a.card);
        if (a.ability_source != kInvalidId) item["ability_source"] = static_cast<int>(a.ability_source);
        // Every other object the intent names (targets, chosen runes or
        // units, units to move), so the board can link a card to the
        // options that use it. Hidden objects only reach the seat that
        // may see them: legal options go to the deciding seat alone.
        json objects = json::array();
        auto addObj = [&](GameObjectId id) {
            if (id == kInvalidId || id == a.card || id == a.ability_source) return;
            for (const auto& o : objects) if (o.get<int>() == static_cast<int>(id)) return;
            objects.push_back(static_cast<int>(id));
        };
        for (auto id : a.targets) addObj(id);
        for (auto id : a.chosen_objects) addObj(id);
        for (auto id : a.units_to_move) addObj(id);
        if (!objects.empty()) item["objects"] = std::move(objects);
        // Where the option puts its card or units: a move's destination or
        // a play's location. The board uses it to let a player click the
        // destination instead of reading button labels.
        const auto& loc = a.move_destination ? a.move_destination : a.play_location;
        if (loc) {
            if (std::holds_alternative<BaseLocation>(*loc))
                item["location"] = {{"base", toString(std::get<BaseLocation>(*loc).player)}};
            else
                item["location"] = {{"battlefield",
                                     static_cast<int>(std::get<BattlefieldLocation>(*loc).id)}};
        }
        arr.push_back(std::move(item));
    }
    return arr;
}

json objToJson(const CardDB& db, const GameState& state, GameObjectId oid) {
    const auto& obj = state.getObject(oid);
    std::string name;
    if (obj.card_def_id != kInvalidId) {
        name = db.get(obj.card_def_id).name;
    } else {
        name = obj.name.empty() ? "(token)" : obj.name;
    }
    return json{
        {"id", static_cast<int>(oid)},
        {"name", name},
        {"exhausted", obj.is_exhausted},
        {"damage", obj.damage_marked},
        {"might", obj.current_might},
    };
}

// Legacy zone summary for the classic page. Private zones are filtered by
// seat the same way as board_view: hands for their owner, deck contents
// for god only. The board itself comes from buildBoardView.
json buildZoneJson(const CardDB& db, const GameState& state, PlayerId pid, Seat seat) {
    const auto& ps = state.player(pid);
    const bool god = seat == Seat::God;
    const bool own = god || seatPlayer(seat) == pid;
    auto vecToJson = [&](const std::vector<GameObjectId>& v) {
        json a = json::array();
        for (auto oid : v) a.push_back(objToJson(db, state, oid));
        return a;
    };
    static const std::vector<GameObjectId> kNone;

    // Sweep all GameObjects for ones at this player's base. Battlefield
    // units live in a separate per-BF bucket — see buildBattlefieldsJson.
    json base = json::array();
    for (const auto& [oid, obj] : state.objects) {
        if (obj.zone != ZoneType::Base) continue;
        if (!obj.location.has_value()) continue;
        if (!std::holds_alternative<BaseLocation>(*obj.location)) continue;
        if (std::get<BaseLocation>(*obj.location).player != pid) continue;
        base.push_back(objToJson(db, state, oid));
    }

    json z = {
        {"hand",       vecToJson(own ? ps.hand : kNone)},
        {"main_deck",  vecToJson(god ? ps.main_deck : kNone)},
        {"rune_deck",  vecToJson(god ? ps.rune_deck : kNone)},
        {"hand_count",      ps.hand.size()},
        {"main_deck_count", ps.main_deck.size()},
        {"rune_deck_count", ps.rune_deck.size()},
        {"trash",      vecToJson(ps.trash)},
        {"banishment", vecToJson(ps.banishment)},
        {"base",       std::move(base)},
        {"score",      ps.score},
        {"energy",     ps.rune_pool.energy},
        {"power_total", ps.rune_pool.totalPower()},
    };
    return z;
}

json buildBattlefieldsJson(const CardDB& db, const GameState& state) {
    json arr = json::array();
    for (const auto& bf : state.battlefields) {
        json units = json::array();
        for (const auto& [oid, obj] : state.objects) {
            if (obj.zone != ZoneType::BattlefieldZone) continue;
            if (!obj.location.has_value()) continue;
            if (!std::holds_alternative<BattlefieldLocation>(*obj.location)) continue;
            if (std::get<BattlefieldLocation>(*obj.location).id != bf.id) continue;
            if (obj.is_hidden) continue;  // face-down: only in the board view, per seat
            units.push_back(objToJson(db, state, oid));
        }
        std::string controller_str = "none";
        if (bf.controller.has_value()) controller_str = playerToStr(*bf.controller);
        // Friendly name that disambiguates mirror-match BFs (two
        // "Vilemaw's Lair" → "Vilemaw's Lair (P1's)" / "(P2's)"). The
        // UI uses this for the move-destination dropdown and the
        // god-mode Battlefields section so the player can tell which
        // is which without reading the integer BF id.
        std::string name = StateRenderer::battlefieldLabel(state, bf.id);
        std::string contributor_str = "none";
        if (bf.contributed_by == PlayerId::Player1) contributor_str = "P1";
        else if (bf.contributed_by == PlayerId::Player2) contributor_str = "P2";
        arr.push_back(json{
            {"id", static_cast<int>(bf.id)},
            {"name", std::move(name)},
            {"controller", controller_str},
            {"contributed_by", contributor_str},
            {"units", std::move(units)},
            {"is_token", bf.is_token},
        });
    }
    return arr;
}

} // namespace

Seat PlayServer::Impl::resolveSeat(const std::string& target) const {
    const bool god_ok = parent.config_.god_mode_enabled;
    std::string want;
    if (auto q = target.find('?'); q != std::string::npos) {
        std::string query = target.substr(q + 1);
        std::stringstream ss(query);
        for (std::string kv; std::getline(ss, kv, '&');)
            if (kv.rfind("seat=", 0) == 0) want = kv.substr(5);
    }
    if (want.empty()) return god_ok ? Seat::God : Seat::Spectator;
    if (want == "auto") {
        const bool h1 = parent.config_.p1_human != nullptr;
        const bool h2 = parent.config_.p2_human != nullptr;
        if (h1) return Seat::Player1;
        if (h2) return Seat::Player2;
        return Seat::Spectator;
    }
    auto seat = parseSeat(want);
    if (!seat) return Seat::Spectator;
    if (*seat == Seat::God && !god_ok) return Seat::Spectator;
    return *seat;
}

json PlayServer::Impl::humanSeats() const {
    json a = json::array();
    if (parent.config_.p1_human) a.push_back("p1");
    if (parent.config_.p2_human) a.push_back("p2");
    return a;
}

namespace {
const char* phaseShortName(TurnPhase p) {
    switch (p) {
        case TurnPhase::Setup:           return "Setup";
        case TurnPhase::Mulligan:        return "Mulligan";
        case TurnPhase::AwakenPhase:     return "Awaken";
        case TurnPhase::BeginningStep:   return "Beginning";
        case TurnPhase::ScoringStep:     return "Scoring";
        case TurnPhase::ChannelPhase:    return "Channel";
        case TurnPhase::DrawPhase:       return "Draw";
        case TurnPhase::MainPhase:       return "Main";
        case TurnPhase::EndingStep:      return "Ending";
        case TurnPhase::ExpirationStep:  return "Expiration";
        default:                         return "?";
    }
}
}  // namespace

// Called with state_mutex_ held.
std::string PlayServer::Impl::frameFor(Seat seat, PlayerId active_player,
                                       const std::vector<Intent>& legal,
                                       uint64_t decision_id, bool at_decision,
                                       const std::string& prompt) {
    const auto& st = parent.engine_.state();
    // Only the deciding seat (or god) sees the options: their labels can
    // name cards in that player's hand.
    const bool mine = at_decision && (seat == Seat::God || seatPlayer(seat) == active_player);
    static const std::vector<Intent> kNoActions;
    const auto& renderer = rendererFor(seat);

    json frame = {
        {"type", mine ? "decision" : "state"},
        {"decision_id", decision_id},
        {"player", playerToStr(active_player)},
        {"waiting_for", at_decision ? json(playerToStr(active_player)) : json(nullptr)},
        {"seat", seatName(seat)},
        {"human_seats", humanSeats()},
        {"render_text", renderer.render(st)},
        {"god_mode", parent.config_.god_mode_enabled},
        {"version", parent.config_.version_tag},
        {"seed", parent.config_.seed},
    };
    frame["legal"] = buildLegalActionJson(parent.card_db_, renderer, st, mine ? legal : kNoActions);
    frame["zones"] = {
        {"P1", buildZoneJson(parent.card_db_, st, PlayerId::Player1, seat)},
        {"P2", buildZoneJson(parent.card_db_, st, PlayerId::Player2, seat)},
    };
    frame["battlefields"] = buildBattlefieldsJson(parent.card_db_, st);
    frame["board"] = buildBoardView(st, parent.card_db_, seat);
    frame["turn"] = {
        {"phase",       static_cast<int>(st.turn.phase)},
        {"phase_name",  phaseShortName(st.turn.phase)},
        {"turn_player", playerToStr(st.turn.turn_player)},
        {"turn_number", st.turn.turn_number},
        {"p1_score",    st.player(PlayerId::Player1).score},
        {"p2_score",    st.player(PlayerId::Player2).score},
        {"victory_score", st.mode.victory_score},
    };
    frame["game_over"] = st.game_over;
    if (st.game_over) {
        frame["winner"] = playerToStr(st.winner);
        frame["game_over_reason"] = st.game_over_reason;
    }
    if (mine && !prompt.empty()) frame["prompt"] = prompt;
    return frame.dump();
}

void PlayServer::Impl::broadcastDecisionFrame() {
    std::list<std::shared_ptr<WsSession>> live;
    {
        std::lock_guard<std::mutex> lock(sessions_mu);
        for (auto it = sessions.begin(); it != sessions.end();) {
            if (auto s = it->lock()) { live.push_back(std::move(s)); ++it; }
            else it = sessions.erase(it);
        }
    }

    // The most recent prompt (set by broadcastLog when it saw a *_PROMPT
    // trace line) heads the next decision's options. Taken once so it
    // does not bleed into later, unrelated decisions.
    std::string prompt;
    {
        std::lock_guard<std::mutex> lock(prompt_mu);
        prompt.swap(last_prompt);
    }

    // One frame per seat in use, built under the state mutex so an
    // in-flight edit can't tear it.
    std::map<Seat, std::string> frames;
    {
        std::lock_guard<std::mutex> lock(parent.state_mutex_);
        PlayerId active_player = PlayerId::None;
        std::vector<Intent> legal;
        uint64_t decision_id = 0;
        bool at_decision = false;
        auto* p1 = parent.config_.p1_human;
        auto* p2 = parent.config_.p2_human;
        if (p1 && p1->atDecision()) {
            at_decision = true;
            active_player = PlayerId::Player1;
            legal = p1->pendingLegal();
            decision_id = p1->decisionId();
            last_p1_decision_id = decision_id;
        } else if (p2 && p2->atDecision()) {
            at_decision = true;
            active_player = PlayerId::Player2;
            legal = p2->pendingLegal();
            decision_id = p2->decisionId();
            last_p2_decision_id = decision_id;
        }
        for (const auto& session : live) {
            const Seat seat = session->seat();
            if (!frames.count(seat))
                frames[seat] = frameFor(seat, active_player, legal, decision_id, at_decision, prompt);
        }
    }
    for (const auto& session : live) session->send(frames[session->seat()]);
}

void PlayServer::Impl::runWatcher() {
    using namespace std::chrono_literals;
    while (!stopping.load()) {
        std::this_thread::sleep_for(100ms);
        if (stopping.load()) break;

        auto* p1 = parent.config_.p1_human;
        auto* p2 = parent.config_.p2_human;
        bool currently_at_decision =
            (p1 && p1->atDecision()) || (p2 && p2->atDecision());

        bool changed = false;
        if (p1 && p1->atDecision() && p1->decisionId() != last_p1_decision_id) changed = true;
        if (p2 && p2->atDecision() && p2->decisionId() != last_p2_decision_id) changed = true;
        // Transition from human-at-decision to engine-busy (AI is thinking
        // or engine is processing internally). Push a state frame so the
        // UI clears the stale legal-action buttons — otherwise the user
        // would still see the buttons from the just-completed decision
        // and could re-click them, queuing a phantom choice for the next
        // human decision.
        if (was_at_decision && !currently_at_decision) changed = true;
        // Also re-push when game ends so the UI shows the terminal state.
        if (parent.engine_.state().game_over) changed = true;

        if (changed) {
            broadcastDecisionFrame();
            was_at_decision = currently_at_decision;
        }
    }
}

// ─── PlayServer member functions ───────────────────────────────────────────

PlayServer::PlayServer(GameEngine& engine,
                       StateRenderer& renderer,
                       const CardDB& card_db,
                       PlayServerConfig config)
    : engine_(engine),
      renderer_(renderer),
      card_db_(card_db),
      config_(std::move(config)) {
    // After the members above: Impl's renderer binds to card_db_.
    impl_ = std::make_unique<Impl>(*this);
}

PlayServer::~PlayServer() {
    stop();
}

std::string PlayServer::start() {
    if (running_.exchange(true)) {
        return "http://" + config_.bind_address + ":" + std::to_string(config_.port);
    }
    // Build the served HTML once; reused across all GET /.
    auto images = ReplayWriter::loadCardImageMap();
    impl_->index_html = buildClassicPageHtml(
        images, config_.version_tag, config_.build_date, config_.seed,
        config_.god_mode_enabled);

    auto addr = net::ip::make_address(config_.bind_address);
    impl_->acceptor.emplace(impl_->ioc, tcp::endpoint{addr, config_.port});
    std::make_shared<Listener>(*impl_->acceptor, *impl_, *this)->start();
    impl_->armSignalSet();

    impl_->io_thread = std::thread([this] {
        try { impl_->ioc.run(); }
        catch (const std::exception& e) {
            std::cerr << "[play_server] io_context exception: " << e.what() << "\n";
        }
    });
    impl_->watcher_thread = std::thread([this] { impl_->runWatcher(); });

    return "http://" + config_.bind_address + ":" + std::to_string(config_.port);
}

void PlayServer::Impl::armSignalSet() {
    signals.emplace(ioc, SIGINT, SIGTERM);
    signals->async_wait([this](beast::error_code ec, int sig) {
        if (ec) return;  // cancelled during shutdown — fine
        std::cerr << "\n[riftbound_play] received signal " << sig
                  << "; shutting down (signal again to force-quit)\n";
        if (parent.config_.on_shutdown_signal) {
            parent.config_.on_shutdown_signal();
        }
        // Re-arm so a second signal does NOT get handled here — we want
        // the second signal to use the default handler (kernel kills us).
        // Boost.Asio resets pending registrations once a handler fires;
        // re-installing the default disposition for SIGINT/SIGTERM lets
        // a second Ctrl-C terminate immediately if cleanup wedges.
        std::signal(SIGINT,  SIG_DFL);
        std::signal(SIGTERM, SIG_DFL);
    });
}

void PlayServer::stop() {
    if (!running_.exchange(false)) return;
    impl_->stopping.store(true);
    beast::error_code ec;
    if (impl_->acceptor) impl_->acceptor->close(ec);
    impl_->ioc.stop();
    if (impl_->io_thread.joinable()) impl_->io_thread.join();
    if (impl_->watcher_thread.joinable()) impl_->watcher_thread.join();
}

void PlayServer::broadcastLog(const std::string& level, const std::string& message) {
    if (!running_.load()) return;

    // Capture *_PROMPT trace lines into impl_->last_prompt so the next
    // decision frame can render them as a header. Format the engine
    // uses: "MAY_PROMPT: <text>", "X_PROMPT: <text>", "MODE_PROMPT:
    // <text> [...]", "TGT_PROMPT: <text> [...]". We strip the prefix
    // for display and keep the question.
    auto pos = message.find("_PROMPT:");
    if (pos != std::string::npos) {
        // Trim past the "_PROMPT: " marker (9 chars).
        std::string prompt = message.substr(pos + 9);
        // Drop the bracketed option summary at the end if present —
        // the legal-actions list already shows the options.
        auto bracket = prompt.rfind(" [");
        if (bracket != std::string::npos) prompt = prompt.substr(0, bracket);
        std::lock_guard<std::mutex> lock(impl_->prompt_mu);
        impl_->last_prompt = std::move(prompt);
    }

    // Engine log lines name cards as they are drawn, searched and hidden,
    // so only the god seat receives them. Seats get a filtered, readable
    // event log in PLAY-008.
    json frame = {
        {"type", "log"},
        {"level", level},
        {"msg", message},
    };
    std::string text = frame.dump();
    std::list<std::weak_ptr<WsSession>> snapshot;
    {
        std::lock_guard<std::mutex> lock(impl_->sessions_mu);
        snapshot = impl_->sessions;
    }
    for (auto& w : snapshot) {
        if (auto s = w.lock(); s && s->seat() == Seat::God) s->send(text);
    }
}

// ─── Accessor shims used by the session classes ────────────────────────────
// (declared inline rather than in the header to keep Beast out of the
//  public interface).

HumanAgent* PlayServer::config_p1() const noexcept { return config_.p1_human; }
HumanAgent* PlayServer::config_p2() const noexcept { return config_.p2_human; }
bool PlayServer::godModeEnabled() const noexcept   { return config_.god_mode_enabled; }
const std::string& PlayServer::webRoot() const noexcept { return config_.web_root; }
GameState& PlayServer::engineMutableState() noexcept { return engine_.mutableState(); }

} // namespace riftbound
