# riftbound_play — Web UI API

The play app lives in `play/`, apart from the engine. Its binary,
`riftbound_play`, runs one game and serves it over a Boost.Beast HTTP +
WebSocket server on `127.0.0.1:8080`, so people can play their seats
from a browser. The engine binary `riftbound` has no server and no
human seats: it runs AI games, batches and tools only.

This document is the wire-protocol reference for that server. The
visual board in `play/web/` (`index.html`, `board.js`, `board.css`) is
the reference client. The older text page is still served at `/classic`.
Any other tool can speak the same protocol.

| Part | Where | Depends on |
|---|---|---|
| Engine, `riftbound_core`, `riftbound`, `riftbound_tests` | `src/`, `tests/`, `tools/` | nothing in `play/` |
| Play library `riftbound_play_lib` (server, board view, human agent, classic page) | `play/src/` | `riftbound_core`, Boost.Beast |
| Play binary `riftbound_play` | `play/src/main.cpp` | play library, MCTS agents |
| Board client | `play/web/` | the WebSocket protocol only |
| Play tests `riftbound_play_tests` | `play/tests/` | play library |

Configure with `-DRIFTBOUND_BUILD_PLAY=OFF` to build the engine alone.

---

## CLI

```text
riftbound_play
    --deck1 PATH          # P1 deck list (required)
    --deck2 PATH          # P2 deck list (required)
    --agent1 SPEC         # human (default) | random | mcts:sims=N | ismcts:sims=N
    --agent2 SPEC         # random (default) | human | mcts:sims=N | ismcts:sims=N
    --port N              # default 8080
    --bind ADDR           # default 127.0.0.1; 0.0.0.0 accepts any interface
    --god-mode on|off     # default on: allow the god seat (sees everything, may edit)
    --web-root DIR        # default: play/web of this checkout (compile-time path)
    --seed N              # 0 = random
    --render-html on|off  # default on: write an HTML replay when the game ends
    --replay-dir DIR      # default replays → replays/<timestamp>/replay.html
    --wild                # allow banned cards
```

After the game ends, the server keeps serving the final board until
Ctrl-C, also in AI-only games. One Ctrl-C (or SIGTERM) concedes any
human seat that is waiting and shuts down. A second one force-quits.
AI moves are not slowed down, so a fast AI-only game may finish before
the page opens.

---

## Connection

| Path | Method | Behaviour |
|---|---|---|
| `/` | GET | `index.html` from the web root (the visual board) |
| `/board.js`, `/board.css`, … | GET | Plain file names from the web root only. No sub-directories, no `..`. Sent with `Cache-Control: no-store`, so edits show up on reload. |
| `/classic` | GET | The older single-page text UI, built into the binary |
| `/ws?seat=SEAT` | GET | WebSocket upgrade: the JSON message channel |
| anything else | not GET | `405` |

There is no auth. Keep the default loopback bind: a client on the god
seat can rewrite the game state.

### Seats

The `seat` query parameter picks what a connection may see and do.

| `seat=` | Sees | May choose for |
|---|---|---|
| `p1` / `p2` | Own hand. Own face-down cards. Public zones. Deck and rune-deck **counts**. | That player only |
| `spectator` | Public zones and counts only. No hand. | Nobody |
| `god` (needs `--god-mode on`) | Everything, including deck order and every face-down card. Also gets the engine log. | Either player. Edits allowed. |
| `auto` | P1 if P1 is human, else P2 if P2 is human, else spectator | as resolved |
| unknown name, or `god` with god mode off | spectator | Nobody |
| *(none)* | God when god mode is on, else spectator (what `/classic` uses) | as resolved |

Hidden cards (CR 107) are sent as `{"back": true}`. They carry no
object id, because ids are given out in deck order and could identify
the card. A face-down card shows its controller only:
`{"back": true, "controller": "P2"}`.

Every frame says which seat it was built for (`seat`) and which seats
are human (`human_seats`).

---

## Server → Client messages

Every WS frame is a single JSON object. The `type` field says which kind it is.

### `decision` and `state`

The server builds one frame per seat in use. A seat gets `decision` when
it is the seat being waited on (or it is god and a human seat is
waiting). Every other seat gets `state`, with an empty `legal` list and
no `prompt`.

```json
{
  "type": "decision",
  "decision_id": 7,
  "player": "P1",
  "waiting_for": "P1",
  "seat": "p1",
  "human_seats": ["p1"],
  "god_mode": true,
  "version": "…", "seed": 7,
  "render_text": "…ASCII board from StateRenderer, other hands hidden…",
  "legal": [
    {"index": 0, "type": "P1: EndTurn",  "label": "End Turn"},
    {"index": 1, "type": "P1: PlayCard card=41", "label": "Play Nami, Headstrong #614(3M) -> Base",
     "card_object_id": 41},
    {"index": 2, "type": "P1: MakeChoice", "label": "Choose: Calm Rune #42 (obj=44)@Base",
     "objects": [44]}
  ],
  "prompt": "…",
  "board": { … see below … },
  "zones": { … legacy, used by /classic … },
  "battlefields": [ … legacy … ],
  "turn": { … legacy summary … },
  "game_over": false
}
```

* `decision_id` goes up by one for each new decision. Send it back in
  `choose`. (The server does not check it yet: PLAY-005.)
* `waiting_for` is the player the engine waits on, or `null` while the
  engine or an AI is working.
* `legal[].index` is the index into the engine's own legal-intent list.
  Send that exact integer back. The engine decides what is legal; the UI
  never builds actions itself.
* `legal[].label` comes from `StateRenderer::renderChosenAction`. The
  pre-game battlefield choice names the battlefield instead.
* `legal[].type` is the intent's `describe()` stem, prefixed with the
  player (`"P1: PlayCard"`).
* `legal[].card_object_id` is the card being played, if any.
  `ability_source` is the object whose ability is used.
* `legal[].objects` lists every other object the intent names: targets,
  chosen runes or units, units to move. The board uses it to link a
  card to the options that use it.
* `legal[].location` is where the option puts its card or units:
  `{"base": "P1"}` or `{"battlefield": 0}`. It is a move's destination
  or a play's location, and is absent for options that place nothing.
  The board uses it for click-the-unit-then-click-the-destination.
* `prompt` (deciding seat only) is the question for mid-resolution
  choices, captured from the card's `*_PROMPT` trace line. Many
  `MakeChoice` decisions still have no prompt and bare labels like
  `=0` / `=1`; PLAY-006 replaces this with structured prompts.
* `render_text` is the text view. The god seat gets the full renderer.
  Each player seat sees only its own hand. The spectator sees no hand.

A `state` frame is also sent when a client connects or sends
`request_state`, after an edit, after a human choice is taken (so old
buttons disappear), and when the game ends.

### `board` — the per-seat board view

Built by `buildBoardView(state, card_db, seat)` in
`play/src/board_view.cpp`. It holds everything the visual board draws.

```text
board = {
  viewer: "p1" | "p2" | "spectator" | "god",
  viewer_player: "P1" | "P2" | null,
  players: { P1: player, P2: player },
  battlefields: [battlefield],
  turn, chain: [chain_item] (newest first),
  cost: cost | null,              // a cost being paid right now
  move_group: move_group | null   // units being gathered for a move
}

player = {
  id, score, xp,
  legend: object | null, champion: object | null,   // champion null once played
  hand:      { count, cards?: [object] },            // cards for own seat and god
  main_deck: { count, cards?: [object] },            // cards (top first) for god only
  rune_deck: { count, cards?: [object] },            // god only
  trash: [object], banishment: [object],             // public
  base: [object],   // units and gear at base; attached gear sits on its host
  runes: [object],  // runes on the board
  pool: { energy, unit_energy, power: {Domain: n}, universal_power }
}

object = {
  id, def_id, name, type, owner, controller, domains: [..],
  keywords: [{name, value?}], exhausted, token, hidden,
  attachments: [object],
  energy_cost?, power_cost?, text?, image?, champion?,   // when it has a card definition
  might?, base_might?, damage?, stunned?, buffs?, temp_might?, designation?,  // units
  empowered?, rune_domains?
}

battlefield = {
  id, name, card: object | null, controller, contested, contested_by, contributed_by,
  is_token,
  showdown: {staged, in_progress},
  combat: {staged, in_progress, attacker, defender},
  units: { P1: [object], P2: [object] },    // face-up units only
  facedown: [object (hidden: true) | {back: true, controller}]
}

turn = { number, phase, turn_player, priority, focus, showdown, closed,
         passed_priority: [..], passed_focus: [..], victory_score,
         game_over, winner?, reason? }

chain_item = { id, name, source: object | null, controller,
               kind: "spell" | "permanent" | "ability" | "other",
               status: "pending" | "finalized", targets: [{id, name}] }

cost = { card, player, energy_remaining, power_remaining, power_domains, paying: "energy" | "power" }
move_group = { player, units: [id], destination }
```

### `log`

Engine log lines (`{"type": "log", "level": "TRC", "msg": "…"}`) go to
**god seats only**. They name cards as they are drawn and searched.
Players will get a filtered event stream in PLAY-008.

### `edit_ok` / `edit_err`

Sent in reply to any `edit_*` message from the client:

```json
{"type": "edit_ok"}
{"type": "edit_err", "msg": "object not found: 137"}
```

A `state` frame is broadcast right after, so the UI updates even if the
reply is lost.

### `err`

Protocol error: unknown message type, bad JSON, or a seat choosing for a
player it does not sit for.

```json
{"type": "err", "msg": "seat p2 cannot choose for P1"}
```

---

## Client → Server messages

### `choose` — pick a legal action

```json
{"type": "choose", "player": "P1", "index": 1, "decision_id": 7}
```

* `player` must be the connection's own player. The god seat may answer
  for either player.
* `index` is the engine's legal-action index (from `legal[].index`).

If `player` has no human seat, the server replies `edit_err`. An
out-of-range index falls back to 0 inside `HumanAgent::selectAction`.
Old clicks are not rejected yet (PLAY-005). The board disables its
buttons after a click until the next frame arrives.

### `request_state` — ask for a fresh frame

```json
{"type": "request_state"}
```

### `edit_*` — god-seat mutations *(gated)*

All `edit_*` messages need `--god-mode on` **and** a connection on the
god seat. Anything else gets `edit_err` (`"edits need the god seat"`,
or `"god mode disabled (no human seat)"`).

Edits skip engine validation. They can leave the game in a state normal
play cannot reach (a card in two zones, a phase out of order). Use them
on purpose, for tests and reproductions.

#### `edit_move` — move a card between zones

```json
{"type": "edit_move", "object_id": 42, "to_player": "P1", "to_zone": "Hand", "to_top": true}
```

`to_zone` is one of `MainDeck`, `RuneDeck`, `Hand`, `Trash`,
`Banishment`, `ChampionZone`, `LegendZone`, `BattlefieldZone`,
`FacedownZone`, `Base`, `Chain` (see `ZoneType` in
`src/core/types.h`). For `BattlefieldZone`, add `battlefield_id`.

#### `edit_object` — set an object's field

```json
{"type": "edit_object", "object_id": 42, "field": "exhausted", "value": true}
{"type": "edit_object", "object_id": 42, "field": "damage",    "value": 2}
{"type": "edit_object", "object_id": 42, "field": "might",     "value": 5}
```

#### `edit_player` — set a player field

```json
{"type": "edit_player", "player": "P1", "field": "score",  "value": 4}
{"type": "edit_player", "player": "P1", "field": "energy", "value": 3}
{"type": "edit_player", "player": "P1", "field": "power",  "domain_idx": -1, "value": 2}
```

For `power`, `domain_idx` is the `Domain` enum value, or `-1` for the
universal power bucket.

#### `edit_phase` — force a turn-state change

```json
{"type": "edit_phase", "field": "phase",       "value": 7}
{"type": "edit_phase", "field": "turn_player", "value": "P2"}
```

**Risky**: a phase jump skips the cleanup, scoring and triggers the
engine runs between phases. Debugging only.

#### `edit_reorder_deck` — replace the deck order

```json
{"type": "edit_reorder_deck", "player": "P1", "order": [42, 17, 9, 3]}
```

`order` must be a permutation of the player's current `main_deck`.
The top of the deck is the end of the array.

---

## Threading & safety

| Thread | Job |
|---|---|
| Game thread | Runs `GameEngine::runGame`. Blocks in `HumanAgent::selectAction` at each human decision. |
| IO thread | `asio::io_context::run`: HTTP accept and WebSocket reads and writes. |
| Watcher | Polls `HumanAgent::atDecision()` and broadcasts per-seat frames when a decision arrives or ends. |

The state mutex (`PlayServer::stateMutex`) keeps edits and frame
builds apart. The engine thread does not take it: edits only arrive
while the engine waits in `selectAction`, when it is not touching the
state.

`AgentInterface` has no edit surface. Agents cannot reach
`StateEditor`. God mode belongs to the play app, not the engine.

---

## Examples

Binaries land in `<build>/play/`.

### Human vs random (default)

```bash
./build/play/riftbound_play --deck1 decks/ahri_test.txt --deck2 decks/anni_test.txt
```

Open `http://127.0.0.1:8080/`. The board connects with `seat=auto`,
which is P1 here. Add `?seat=spectator` or `?seat=god` in a second tab.

### Hot-seat on two devices

```bash
./build/play/riftbound_play --agent1 human --agent2 human --bind 0.0.0.0 \
    --god-mode off --deck1 … --deck2 …
```

Player 1 opens `/?seat=p1` and player 2 opens `/?seat=p2`. Each sees
only their own hand. `--god-mode off` stops either of them from opening
the god seat.

### Watching two AIs

```bash
./build/play/riftbound_play --agent1 mcts:sims=200 --agent2 random --deck1 … --deck2 …
```

### Programmatic client (websocat)

```bash
websocat -t 'ws://127.0.0.1:8080/ws?seat=p1'
> {"type":"request_state"}
< {"type":"decision",...}
> {"type":"choose","player":"P1","index":0,"decision_id":3}
```
