# Riftbound Simulation Engine

A high-performance C++20 game engine that simulates 1v1 Riftbound TCG matches
end-to-end. Drop-in compatible with [OpenSpiel](https://github.com/google-deepmind/open_spiel)
so any tree-search algorithm (MCTS, ISMCTS, CFR, …) can plug straight in.

> **Disclaimer.** Released **as-is** for research and hobby use.
> Expect rough edges in: card coverage (~240 of 787 cards manually implemented;
> rest are auto-generated stubs of varying fidelity), edge-case rules
> interactions, performance on stress workloads, and corners of the OpenSpiel
> wrapper. PRs welcome; production deployment at your own risk.

> The training / ML side — neural agents, self-play loops, model checkpoints —
> lives in a separate sibling repo. This repo is just the simulator + AI
> baselines via OpenSpiel.

## What works today
- Uses Piltover archive formatted game decks
- **Full rules engine.** Awaken → channel → draw → main → end turn loop,
  FEPR chain resolution, combat with damage assignment, scoring, mulligans,
  battlefields, gear/equip, 23 keyword mechanics, replacement effects.
- **787 cards**, each a hand-authored C++ class owning its own data + behavior. ~240 manually implemented
  with full behavior (champions, legends, key spells in test decks); the
  rest have auto-generated stubs that cover simple effects. Implementation
  fidelity per card is tracked in [`docs/card-implementation-audit.md`](docs/card-implementation-audit.md).
- **Deterministic.** Given a seed, the same game replays identically.
- **OpenSpiel integration.** `RiftboundGame` / `RiftboundState` implement
  the OpenSpiel `Game` / `State` interfaces. MCTS, ISMCTS, and random
  agents work out of the box.
- **HTML replays.** Per-game rendered HTML with board snapshots,
  decision points, trace log, arrow-key navigation.
- **855 unit tests** covering engine behavior + card mechanics.
- **Engine-fiber step machine.** `boost::context::fiber`-based cooperative
  yields at decision points. No OS thread per game state, supports
  thousands of `Clone()` calls per decision for branching search.

## Quick start

```bash
# Install deps (Ubuntu/Debian)
sudo apt-get install cmake ninja-build g++ libboost-all-dev nlohmann-json3-dev

# Configure + build (first time clones OpenSpiel + abseil into build/_deps, ~30s)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run unit tests
RIFTBOUND_ROOT=. ./build/riftbound_tests

# Play random vs MCTS (single game, stdout + HTML replay)
./build/riftbound \
    --agent1 random --agent2 mcts:sims=50 \
    --deck1 decks/miss_fortune_test.txt \
    --deck2 decks/miss_fortune_test.txt \
    --render-html on

# Batch 100 games on 8 threads (random vs random, no UI)
./build/riftbound \
    --agent1 random --agent2 random \
    --deck1 decks/miss_fortune_test.txt \
    --deck2 decks/miss_fortune_test.txt \
    --games 100 --threads 8

# Play AS A HUMAN against MCTS-50 in your browser. Human seats live in
# the separate play app (play/), built as build/play/riftbound_play.
./build/play/riftbound_play \
    --agent1 human --agent2 mcts:sims=50 \
    --deck1 decks/miss_fortune_test.txt \
    --deck2 decks/miss_fortune_test.txt

# Hot-seat on two devices: P1 opens /?seat=p1, P2 opens /?seat=p2
./build/play/riftbound_play --agent1 human --agent2 human --bind 0.0.0.0 \
    --god-mode off \
    --deck1 decks/miss_fortune_test.txt \
    --deck2 decks/miss_fortune_test.txt

# Spectate an AI-vs-AI game in the browser
./build/play/riftbound_play --agent1 random --agent2 mcts:sims=20 \
    --deck1 decks/miss_fortune_test.txt \
    --deck2 decks/miss_fortune_test.txt
```

Pass `--help` for the full flag list. The web UI's WebSocket protocol
is documented in [`docs/play-api.md`](docs/play-api.md).

## Play against the AI (web UI)

The play app lives in `play/`, apart from the engine: `riftbound`
itself has no server and no human seats. Configure with
`-DRIFTBOUND_BUILD_PLAY=OFF` to build the engine alone.

To play as Player 1 against a 50-simulation MCTS bot:

```bash
./build/play/riftbound_play \
    --agent1 human --agent2 mcts:sims=50 \
    --deck1 decks/miss_fortune_test.txt \
    --deck2 decks/miss_fortune_test.txt
```

Then open `http://127.0.0.1:8080/`. The game waits at each of your
decisions until you click; the bot moves on its own. MCTS cost grows
with `sims`: `sims=50` is quick, while `sims=500` can take minutes per
decision.

Useful flags (`--help` lists them all):

| Flag | Default | Meaning |
|---|---|---|
| `--agent1` / `--agent2` | `human` / `random` | Per-seat agent: `human`, `random`, `mcts:sims=N`, `ismcts:sims=N`. |
| `--port` / `--bind` | `8080` / `127.0.0.1` | Where the UI listens. `--bind 0.0.0.0` lets other machines connect. |
| `--god-mode` | `on` | Allows the god seat, which sees everything and can edit the state. Turn it off when the two players are on different devices. |
| `--seed` | `0` | Fixes the RNG for a reproducible game (`0` = random). |
| `--wild` | off | Allows banned cards. |

### Seats

Add `?seat=` to the URL, or use the picker in the top bar:

| Seat | Sees |
|---|---|
| `auto` (default) | Your human seat |
| `p1` / `p2` | That player's hand and face-down cards, public zones, deck counts |
| `spectator` | Public zones and counts only |
| `god` | Everything, including deck order. Also the engine log and the state editor (on `/classic`). |

### The board

`/` is the visual board (`play/web/`):

- Your opponent is at the top and you are at the bottom, with the battlefields in between.
- Cards show their art, might, damage, exhaustion, keywords and gear. Hover a card for its full text.
- Cards you can act with are outlined. Click one to play it, or to pick it when paying a cost.
- The side panel holds the turn, priority, your options, the cost being paid, the chain and an optional text view.

The older text dashboard, with its god-mode editor, is still at `/classic`.

See [`docs/phase1-visual-board.md`](docs/phase1-visual-board.md) for
what the board covers and how to test it, and
[`docs/play-api.md`](docs/play-api.md) for the WebSocket protocol.

## Binaries

| Binary | Purpose |
|---|---|
| `build/riftbound` | Engine game runner: single and batch games, all AI agents (`random` / `mcts:sims=N` / `ismcts:sims=N`), HTML replay, trace and debug logging. No human seats. |
| `build/play/riftbound_play` | Play app: one game served to the browser, with human seats, spectators and the god seat. |
| `build/play/riftbound_play_tests` | Play app tests (board view, seats, human agent). |
| `build/riftbound_tests` | Google Test suite. |
| `build/src/openspiel/riftbound_clone_equiv_test` | Validates Clone() correctness — clones mid-game, runs the same actions on original + clone, asserts identical terminal state. |
| `build/src/openspiel/riftbound_clone_microbench` | Microbenchmark — times Clone() throughput. |
| `build/src/openspiel/riftbound_parity_baseline` | Diagnostic — confirms OpenSpiel wrapper produces statistically identical results to the raw engine. |

## Plug in your own agent

Subclass `riftbound::AgentInterface` (see `src/agents/agent_interface.h`)
and add a branch to `buildAgent()` in `src/main.cpp` — adding a new
`--agent foo:...` spec is ~30 lines. The `MctsAgent` / `IsMctsAgent`
adapters in `src/agents/mcts_agent.{h,cpp}` show how to wrap an
OpenSpiel `Bot` behind the same interface.

For learned agents: a sibling repo holds the training pipeline + model
architectures (Deep CFR, OSFP, online CFR / ReBeL inference, etc.).
That repo links against this one as a static library — engine repo
stays slim and dependency-light.

## Card data

Card data is **hand-authored C++** — the source files are the single source of
truth. There is no `registry.json` and no code-generation pipeline. Each card
class implements `Card::def()`, returning its `CardDef` (cost, domains, tags,
keywords, art URL, `banned` flag, …); the engine builds its card table from those
at startup (`CardDB::buildFromClasses`). To add or edit a card, edit its
`src/cards/<type>/<id>_<slug>.cpp` directly.

```bash
# Export the compiled card table to registry-style JSON (validation / external use)
./build/riftbound --dump-registry /tmp/registry.json

# (optional) Pull raw card metadata from the official gallery, for reference when
# authoring new cards. Writes cards/raw/gallery_raw.json — it does NOT feed the engine.
python3 scripts/fetch_cards.py
```

| Script | Purpose |
|---|---|
| `scripts/fetch_cards.py` | Scrape Riftbound's official gallery into `cards/raw/gallery_raw.json`. Reference data for authoring new cards; not consumed by the engine. |
| `scripts/generate_replays.py` | Bulk-generate HTML replays of every deck pair under `decks/`. Useful for visual V&V after card changes. |
| `scripts/audit_deck_cards.py` | Per-deck audit of card-implementation status (FULL / PARTIAL / STUB / MISSING). |

## Repository layout

```
src/core/        ID types, events, intents, game state, card database
src/engine/      Turn loop, chain manager, effect executor, trigger manager,
                 batch runner, step driver (fiber-based)
src/cards/       Card subclasses — one TU per card under units/ spells/ gear/
                 legends/ battlefields/ runes/ (named <id>_<slug>.cpp), each
                 owning its CardDef data + behavior. Per-type bases live in
                 their type dir (gear/equip_base.h, units/weaponmaster_base.h);
                 cross-cutting helpers in card_helpers.h; cards_init.cpp is the
                 generated registration aggregator.
src/agents/      AgentInterface + RandomAgent + HumanAgent
src/io/          HTML replay writer, ASCII state renderer
src/openspiel/   OpenSpiel Game / State subclasses, action vocabulary,
                 match runner, clone-correctness tests
src/ml/          Engine-side shared infra: feature extractor +
                 CFR utilities. No torch — these emit data for
                 downstream ML consumers.
src/rules/       Deck validator (tournament rules)
src/effects/     Effect type definitions

tests/           Google Test — engine behavior + card mechanics
cards/           raw/gallery_raw.json (reference art/metadata for authoring)
decks/           Sample decks (per-archetype .txt deck lists, Piltover Archive format)
rules/           core-rules.md, tournament-rules.md, core-rules.pdf
errata/          Official errata documents
scripts/         Card-data pipeline + replay generation
```

## Reference

- `rules/core-rules.md` — full game rules (sections 000–826)
- `rules/tournament-rules.md` — deck construction + tournament policies
- `CLAUDE.md` — coding standards + architecture conventions for contributors

## Status

This is a working development snapshot. The engine, OpenSpiel wrapper, and
test suite are stable. Card coverage is improving steadily — see
`scripts/audit_deck_cards.py` output for per-deck implementation status.

## License

TBD. Card data fetched from public Riot sources; card art is *not* bundled
(images are loaded on-demand from Riot's CDN by replay viewers / web UIs).
Riftbound is the property of Riot Games — this engine reproduces published
public rules for research and educational use.
