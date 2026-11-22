# Phase 1: visual board — work done and how to test

Date: 2026-11-06. Nothing is committed yet; all changes are in the working tree.

Closes PLAY-001, PLAY-002, PLAY-003, PLAY-010, PLAY-011 and PLAY-012
(see [tickets](tickets/README.md)). PLAY-013 (parity test between the
board and the text view) is still open.

## 1. What changed

### The engine and the play app are now separate

| | Before | After |
|---|---|---|
| Server, human agent, web page | `src/io/play_server.*`, `src/agents/human_agent.h`, `src/io/play_index_html.*` | `play/src/` (`play_server.*`, `human_agent.h`, `classic_page.*`) |
| Human seats | `riftbound --agent1 human` | `riftbound_play` only. `riftbound` rejects `human` and points to `riftbound_play`. |
| Web flags (`--web`, `--port`, god mode) | on `riftbound` | on `riftbound_play` |
| Human-agent test | `tests/test_human_agent.cpp` | `play/tests/test_human_agent.cpp` |
| Build | one tree | `play/CMakeLists.txt`, included by the root when `RIFTBOUND_BUILD_PLAY=ON` (the default) |

Nothing in `src/`, `tests/` or `tools/` includes a play header. With
`-DRIFTBOUND_BUILD_PLAY=OFF`, `riftbound` and `riftbound_tests` build
on their own (checked).

New targets, all in `<build>/play/`:

* `riftbound_play_lib`: the server, the board view, the human agent and the classic page.
* `riftbound_play`: the binary (`play/src/main.cpp`).
* `riftbound_play_tests`: the play tests.

The only engine-side change in this phase is an opt-in text-renderer
setting, `StateRenderer::hide_other_hands` / `hand_viewer`. It is off by
default, so logs, replays and the CLI still print both hands.

### Per-seat views (PLAY-002)

*Replaced in the rebase (section 7): upstream built its own seat model.
What follows is the board view's visibility, which still holds.*

Each WebSocket picks a seat with `/ws?seat=p1|p2|spectator|god|auto`.
The server builds a separate frame for each seat in use.

* **Hand**: visible to its owner (and god) only; the others get a count.
* **Main deck and rune deck**: counts only, except for god, who sees the order (top first).
* **Face-down cards**: visible to their controller (and god). Everyone else gets `{"back": true, "controller": …}`, with no object id, because ids follow deck order.
* **Legal actions and the prompt's question**: only the deciding seat (or god) gets them, since labels can name hand cards. The other seat is told the kind of decision and the asking card, if it may see it.
* **Engine log**: god only.
* **Text view**: the god seat gets the full renderer. A player seat sees its own hand only. The spectator sees no hand. (Found and fixed while testing: before this, every seat's text view printed both hands.)
* **`choose`**: accepted only for the connection's own player (god may answer for either).
* **`edit_*`**: need the god seat as well as `--god-mode on`.

### Full board state (PLAY-003)

Every frame carries `board`, built by `buildBoardView()` in
`play/src/board_view.cpp`. It holds both players (score, XP, legend,
champion, hand, decks, trash, banishment, base, runes, rune pool), the
battlefields (controller, contested, combat and showdown flags, units
per side, face-down cards), the turn (phase, priority, focus, passes),
the chain, the cost being paid, and the move group being built.

Each legal action now also lists `objects`: the targets, chosen runes
or units, and units to move. With that, a click on any card can be
matched to the options that use it. The legacy `zones`, `battlefields`
and `turn` fields stay for `/classic`. The full schema is in
[play-api.md](play-api.md).

### The visual board (PLAY-010, 011, 012)

The board is plain HTML, CSS and JavaScript in `play/web/`, with no
build step. It is served from disk (`--web-root`, default `play/web`),
so a browser reload picks up edits without a rebuild.

* **Layout**: the opponent is at the top and you are at the bottom, with both battlefields in the middle. A spectator or god sees P1 at the bottom. Each player area shows:
  * hand (card backs for the opponent)
  * legend and champion
  * base
  * runes, with a ready count and the rune pool
  * deck, rune deck, trash and banishment piles
  * a score track to the victory score, and XP
* **Piles**: trash and banishment open a list on click. Deck and rune deck open only for god.
* **Battlefields**: card art, who controls them, contested, combat (with the attacker), showdown, token, and "move target". Units are shown per side, with each side's might total. Face-down cards appear as backs.
* **Cards** show:
  * art and domain-coloured border
  * cost, and might against base might (green when raised, red when lowered)
  * damage, and a tilt when exhausted
  * tags for stunned, buffs, token, hidden, empowered, and attacker or defender
  * keyword chips and attached gear
  * a hover tooltip with the full text and the card's current state
* **Actions**:
  * Cards with a legal action get a yellow outline.
  * Clicking a card acts at once if it has exactly one action and is that action's subject (the card played, the ability's source, or the unit moved), or if the decision is a pick (such as choosing a rune to pay with). Otherwise the click highlights the matching buttons, so clicking a target never casts a spell by accident.
  * A card that can go to more than one place (a unit played to base or a battlefield, a unit with several moves) is selected instead: it lifts, the base and battlefields it can go to get a dashed outline, and clicking one picks that option. Esc or a second click on the card cancels. If two options lead to the same spot (an alternative cost, an extra cost to play there), clicking the spot highlights those buttons.
  * Clicking one of your units that has nothing to do says why, usually that it is exhausted (units enter play exhausted).
  * Hovering a button highlights the cards it names.
  * Buttons are disabled after a click until the next frame arrives.
* **Side panel**: turn, phase, open or closed state, priority, focus, who passed, score, and seat. It also holds the current decision with its prompt and buttons, the cost being paid and the move group, the chain (newest first, with targets), and a text-view toggle (remembered in the browser). God also gets the engine log. The game-over banner shows here too.
* **Top bar**: seat picker (it reloads with `?seat=`), connection status, and a link to `/classic`.
* **Phone width**: one column, no sideways scroll, and a floating "Your move (n)" button that jumps to the actions.

## 2. How to build

From WSL (see the project memory for the user-space Boost setup):

```bash
cmake -S . -B ~/alpharune-build
cmake --build ~/alpharune-build -j10
```

To show the engine builds without the play app:

```bash
cmake -S . -B ~/alpharune-noplay -DRIFTBOUND_BUILD_PLAY=OFF
cmake --build ~/alpharune-noplay -j10 --target riftbound riftbound_tests
```

## 3. Automated tests

```bash
RIFTBOUND_ROOT=. ~/alpharune-build/riftbound_tests
RIFTBOUND_ROOT=. ~/alpharune-build/play/riftbound_play_tests
```

Expected: **1505** engine tests and **12** play tests pass. (The engine
count drops from 1510 because the 5 human-agent tests moved to the play
suite.)

The new play tests are in `play/tests/test_board_view.cpp`:

| Test | Checks |
|---|---|
| `PlayerSeesOwnHandButNotTheOpponents` | P1's view names and ids P1's hand, and has neither for P2's hand; P2's hand is a count |
| `DeckContentsAreHiddenFromEverySeatButGod` | No deck or rune-deck card (name or id) reaches P1, P2 or a spectator; god sees the deck top first |
| `FaceDownCardsShowOnlyToTheirController` | The opponent gets a back with no id or name; the controller and god get the card; it is never listed as a unit |
| `PublicZonesAreVisibleToEveryone` | Trash and battlefield units look the same from every seat |
| `SpectatorSeesNoHand` | No hand cards, and `viewer_player` is null |
| `TextViewHidesOtherHands` | The per-seat text renderer prints only the viewer's hand, and no hand for a spectator |
| `BoardViewSeat.ParsesSeatNames` | Seat parsing |

## 4. Manual test steps

Start a game: you as P1, a random agent as P2.

From a WSL shell in the repo:

```bash
LD_LIBRARY_PATH=$HOME/deps/boost/lib RIFTBOUND_ROOT=. ~/alpharune-build/play/riftbound_play \
    --deck1 decks/ahri.txt --deck2 decks/fiora.txt --seed 7 --bind 0.0.0.0
```

From PowerShell, the same thing on one line (the single quotes stop
PowerShell from reading the `$` and `=`):

```powershell
wsl -d Ubuntu-22.04 --cd /mnt/e/Dev/RiftCompute/alpharune -- bash -lc 'LD_LIBRARY_PATH=$HOME/deps/boost/lib RIFTBOUND_ROOT=. $HOME/alpharune-build/play/riftbound_play --deck1 decks/ahri.txt --deck2 decks/fiora.txt --seed 7 --bind 0.0.0.0'
```

`LD_LIBRARY_PATH` is only needed because Boost is installed in user space
(`~/deps/boost`). `--bind 0.0.0.0` lets the Windows browser reach the
server inside WSL. Off loopback the server requires an access token: open
the URL it prints (`http://<addr>:8080/?token=...`), replacing the address
with `localhost`. (The test decks `ahri_test` / `anni_test` were removed
upstream in `581ef91`.)

| # | Do | Expect |
|---|---|---|
| 1 | Open `http://localhost:8080/` | The board loads and shows "connected". The tab title reads "● Your move". Three battlefield buttons appear on the right. |
| 2 | Pick a battlefield, then keep your hand | Your hand appears face up at the bottom. P2's hand shows as backs at the top. |
| 3 | On your main phase, hover a hand card | A tooltip shows the art, text, cost and might. Cards you can play have a yellow outline. |
| 4 | Click a playable unit in hand | It plays, and the "Paying a cost" panel shows the energy left. Your runes get outlines. |
| 5 | Click runes to pay | The energy left counts down. The unit lands at base, tilted (exhausted). |
| 6 | Hover an action button | The cards it names are outlined in yellow. |
| 7 | Click a pile: Trash, then Banished | A list opens. Esc or × closes it. Your Deck pile does not open. |
| 8 | Open a second tab at `/?seat=spectator` | Both hands are backs, there are no action buttons, it says "Waiting for Player 1…", and there is no engine log. |
| 9 | Open `/?seat=god` | Both hands are face up. Deck piles open and show the order. The engine log fills in, and action buttons appear when P1 is waiting. |
| 10 | In the spectator tab, tick "Text view" | The text board shows "(N cards, hidden)" for both hands. |
| 11 | Open `/?seat=p2` | P2's own hand is shown. There are no buttons for P1's decision. |
| 12 | Narrow the window to phone width (or use device mode) | Single column, no sideways scroll, and a floating "Your move (n)" button that scrolls to the actions. |
| 13 | Open `/classic` | The old text page still works. |
| 13a | While you control a battlefield, click your Chosen Champion (when you can afford it) | It lifts. Your base and the battlefield you control get a dashed outline. Click the battlefield: the champion is played there. |
| 13b | Next turn, click a ready unit at your base | Both battlefields get a dashed outline. Click one: the unit moves there (then the group-move "add a unit or done" buttons appear if another unit could join). |
| 13c | Click a unit you played this turn | The prompt says it is exhausted. |
| 14 | Play on to the end (or pass every turn) | The side panel shows "Player N wins (…)", and the actions say the game is over. The server keeps serving until Ctrl-C. |
| 15 | Ctrl-C in the terminal | "received signal 2; shutting down", and the process exits. If a decision was open, that seat concedes. |

Leak check from the browser console, in any tab, mid-game:

```js
const grab = s => new Promise(r => { const w = new WebSocket(`ws://${location.host}/ws?seat=${s}`);
  w.onopen = () => w.send('{"type":"request_state"}');
  w.onmessage = e => { const m = JSON.parse(e.data); if (m.board) { w.close(); r(m); } }; });
const god = await grab('god'), spec = await grab('spectator');
const hand = god.board.players.P1.hand.cards.map(c => c.name);
hand.filter(n => JSON.stringify(spec).includes(n));   // expect [] (unless a copy is public)
```

## 5. Verified in this session

* A full build, with 1505 engine tests and 12 play tests passing.
* The engine-only build with `RIFTBOUND_BUILD_PLAY=OFF`.
* In the browser:
  * Steps 1, 2, 4, 5, 8, 9, 12, 13 and 14. For step 7, only god's deck list was opened.
  * Steps 3, 6, 10 and 11 were not clicked through. The automated tests cover what step 10 and the P2 view (step 11) should show.
  * Spectator, P1 and P2 frames, raw JSON and text view: no names from the other hand.
  * Static files: `..`, `%2e%2e` and source-file paths return 404, and POST returns 405.
* SIGINT and SIGTERM shut the server down cleanly while a human decision is open.
* An AI-only game keeps serving the final board after it ends, and SIGINT stops it.

## 5b. Fixes after the first play-test (2026-11-06)

Reported: "the Chosen Champion can only be played to base" and "champion
units could not be moved from base". The replay showed three causes.

* **Engine: the Chosen Champion got only "play to base".** The Champion
  Zone play ignored battlefields the player controls and every location
  rule that units in hand follow. Both now share
  `GameEngine::appendUnitPlayLocations`. Ambush and reaction-to-attack
  plays (Rengar, Pouncing) also look at the Champion Zone now
  (`unitPlaySources`). The "can't play cards" lockout (Brynhir
  Thundersong) now covers the champion too. Tests:
  `tests/test_champion_zone_play.cpp` (7 tests).
* **Engine: a unit played to a battlefield kept zone Base.**
  `resolvePermanent` set `zone = Base` whatever the location. Moves and
  tokens set the zone from the location, so the board view (which lists
  battlefield units by zone) did not show units played straight to a
  battlefield. The zone now follows the location.
* **Board: moves were hard to find.** The engine did offer the moves
  (units are exhausted the turn they are played), but clicking a unit
  only highlighted buttons. Now clicking a unit or card with more than
  one destination selects it, and the destinations get a dashed outline
  (see section 1). Clicking an exhausted unit says it is exhausted.
  Legal actions carry a `location` field for this.

Checked in the browser (seed 7): Nami moved by clicking it and then the
Grove; Ahri played from the Champion Zone to a controlled Grove by
clicking it and then the Grove, and shown there; the exhausted-unit hint.
Tests: 1512 engine tests (1505 + 7 new) and 12 play tests pass.

## 6. Known gaps (later tickets)

*Written before the rebase; section 7 says which of these upstream closed.*

* **No question shown for some choices.** Fixed by PLAY-006: every decision has a heading (its kind and the asking card) and its options have words.
* **Old clicks are not rejected.** The server does not check `decision_id`. The board guards against double clicks by disabling its buttons. → PLAY-005.
* **Text labels only.** Actions are still the engine's text labels. There is no drag to play, and no click-a-target-then-confirm flow. → PLAY-007 (action descriptors), then Phase 2 (PLAY-020 to 024).
* **No move gathering on the board.** Group moves use the two-step engine decision through buttons. → Phase 3 (PLAY-030, 031).
* **No event stream for players.** Players get no event log, only god does. → PLAY-008.
* **No parity test** between the board and the text view. → PLAY-013.
* **Card images load from the gallery CDN** (`image` URLs in the card definitions). Offline, cards fall back to a name-only face.

## 7. Rebased onto upstream (2026-11-13)

The work was rebased from `b05e51d` onto upstream `ec90f0f` (947 commits).
Upstream had reworked the same areas, so most of it now comes from
upstream:

* **Engine.** Upstream already implemented group moves (CR 144.3: after
  the first unit, the mover is asked to add others, a `MoveJoin` choice)
  and Champion Zone plays with the same locations, Ambush and reactions
  as from hand. Our engine changes were dropped in favour of upstream's;
  `tests/test_champion_zone_play.cpp` (7 tests) still passes against it.
  `tests/test_group_move.cpp` was dropped: it tested our version, and
  upstream's tests cover 144.3. The board's `move_group` field is gone.
* **Play server.** Upstream added seat-bound sessions, park-gated state
  access, fair-play frames, origin and token checks, a spectator mode and
  `decision_id` checks on `choose`. These overlap PLAY-002, PLAY-004 and
  PLAY-005, which were re-checked on 2026-11-22: 002 is done differently,
  004 and 005 are done. `play/src/play_server.cpp` is now upstream's server, moved
  to `play/`, plus: the board page from `--web-root` at `/` (falling back to
  the embedded page when there is none), the embedded page at `/classic`,
  `board` in every frame (built with the frame's own visibility), and
  `legal[].objects` / `legal[].location`.
* **Seats.** The `?seat=` query on the WebSocket is gone. With two human
  seats the board sends `{"type":"seat","player":"P1"}` when its URL has
  `?seat=P1`. With one human seat every client shares one stream. The log
  panel shows whatever lines the server sends that view.
* **Binaries.** `src/main.cpp` (the `riftbound` binary) is upstream's
  without human seats or the web UI; `play/src/main.cpp` is upstream's
  whole CLI plus `--web-root`, defaulting to `--agent1 human`.
* **Tests.** Upstream's `tests/test_review_fix_io_cli.cpp` was split: its
  play-server tests are now `play/tests/test_play_server_protocol.cpp`.
* **GCC 11 fixes to upstream code.** `CardDefId granted_by = 0` in
  `src/core/legal_action.h` and four bare id literals in
  `tests/cards/test_sweep_costs.cpp` did not compile with the WSL GCC.

Results: 3611 engine tests, 41 play tests and the 27 ctest-only checks
pass. In the browser, clicking a unit and then a battlefield moved it, and
upstream's showdown followed.
