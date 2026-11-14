# Riftbound evaluation engine: roadmap

**Status:** draft for review, 2026-11-02, against `b05e51d`.

**Goal:** given a Riftbound position, report who is winning and the top 3 lines to play, the way a chess engine does. Each stage removes something the engine is allowed to see:

| Stage | Engine sees | Hidden | Game type it becomes |
|---|---|---|---|
| 1 | Both decks in order, both hands | Nothing | Perfect information |
| 2 | Both hands, both decklists | Deck order | Perfect information with chance |
| 3 | Own hand, both decklists | Deck order, opponent's hand, facedown cards | Imperfect information |
| 4 | Own hand and list, public zones | All of the above, plus the opponent's list | Imperfect information with an unknown opponent model |

**The short version:**

- **The simulator is the limit, not the search.** An engine can only be as right as the game it simulates. About 250 of 1,099 cards are unimplemented, partial or known wrong, and a hand sample suggests about 1 in 5 untested cards has a bug no tool flags. Card correctness (Track A) runs before and alongside everything else.
- **Build one search, not four.** A single PUCT tree search with a pluggable *world sampler* and *evaluator* covers all four stages. Each stage replaces what is plugged in, not the core.
- **Fix forking first.** Only 21-49% of decisions can be forked from a state copy today, resuming from a copy does not always reproduce the game, and the repo's own clone-correctness test fails most of its games. No stage works until this does (Track B).

---

## 1. Where we are starting

### Measured baseline

From `tools/search_baseline.cpp` (new): 80 games across all 39 sample decks, one core, Release build in WSL2.

| Metric | Random agent | Scripted agent |
|---|---|---|
| Decisions per game (mean / p95) | 290 / 468 | 571 / 1,939 |
| Turns per game (mean) | 16.6 | 15.4 |
| Legal actions (mean / p95 / max) | 3.6 / 11 / 41 | 3.8 / 10 / 160 |
| Forced decisions (one option) | 41% | 33% |
| Options at a real choice (mean) | 5.5 | 5.2 |
| Decisions at a clean boundary | **21%** | **49%** |
| Resume disagrees with the live game | **11 of 293 (3.8%)** | **13 of 1,397 (0.9%)** |
| `applyChoice` / state copy / resume | 10 / 13 / 64 µs | 13 / 13 / 57 µs |

Reading these:

- **Branching is low; depth is the problem.** About 5 options at a real choice is friendly, but 18-37 decisions per turn over 16 turns is deep. Random games spend 30% of decisions paying costs and 37% with a chain open, and most of that is not strategy. Compressing it (B3) is worth more than a faster engine.
- **Budget:** one tree edge costs about 85 µs (copy + resume + apply), so roughly 12k expansions per second per core. A random game to the end costs about 3 ms. This is an estimate from the numbers above, not a measured search speed.
- **The scripted agent is not usable as a rollout policy yet.** One game reached the 3,000-decision cap and p95 is 1,939, which points to repeated-action loops.

### What already exists

| Asset | State | Use |
|---|---|---|
| Rules engine, fiber step machine | Works; all 1,495 unit tests pass | The simulator |
| `Clone()` / `resumeFromSnapshot` | **`riftbound_clone_equiv_test` fails: 3 of 10 games passed, then 1 of 10 on a rerun** | Replaced by B1 |
| OpenSpiel MCTS / ISMCTS wrappers | Run; MCTS-50 beat random 7-1 in a small test. See below on ISMCTS | Baseline to beat |
| `StepKind::ChanceNode` | A commented-out line | Implemented in Stage 2 |
| `core/knowledge_view.h` | Interface only; **no implementation** | Built in Stage 3 |
| `core/legal_action.h` | Interface only; no implementation | Built in B3 |
| `ml/feature_extractor` (6,188 dims) | Works, masks hidden info | Input to the learned evaluator |
| God-mode `state_editor` | Edits a live game; no save or load | Basis for the position format (B2) |
| `RiftboundHeuristicEvaluator` | Score difference only | Baseline evaluator E0 |
| `riftbound-trainer` sibling repo | Referenced throughout; **not public** | Unavailable; Stage 3-4 assume we build what we need |

Three things that look finished and are not:

1. **Clones through a mid-chain position replay the whole game from turn 0**, using action ids that the code itself documents as collision-prone. That silently discards any modified state, so a determinized world would be lost at the first dirty node.
2. **ISMCTS is not really ISMCTS.** Its resampler is a plain `Clone()`, so, as the code's own comment says, it behaves like perfect-information MCTS: it can see hidden cards. Any strength result from `ismcts:` agents is not an imperfect-information result. The game also declares `kSampledStochastic` with no chance outcomes.
3. **The RNG lives on `GameEngine`, not `GameState`.** A resumed game is reseeded, so it does not continue the same random stream.

One piece of good news: **no card touches the RNG directly.** All randomness runs through 6 sites in the engine and executor (coin toss, mulligan order, four shuffles), so Stage 2's chance layer has a small, clean seam.

---

## 2. Track A: card correctness (high priority)

Full list: [`docs/card-status/README.md`](card-status/README.md), with one row per card in [`card-status.csv`](card-status/card-status.csv). Regenerate with `python scripts/card_status.py`.

| Status | Cards | Meaning |
|---|---|---|
| Not implemented | **151** | 144 Radiance with no class; 7 older cards that do nothing |
| Partial, confirmed | **19** | Read by hand against the card text and found wrong |
| Partial, documented | **72** | A comment in the file admits a gap |
| Partial, suspect | **8** | Heuristic flag: a printed decision has no matching code |
| Implemented, untested | **305** | No flags; no test references the card |
| Implemented, tested | **484** | No flags; a test references the card |
| Vanilla | **60** | Keywords and stats only |

**How far to trust this.** It is a static triage. The repo's own `card_coverage.py` reports 5 incomplete cards because it only asks whether effect code exists. To size what both tools miss, I read 20 randomly chosen unflagged cards: **3 of 14 untested cards were wrong, 0 of 6 tested cards were.** The sample is small (a 95% interval on the untested rate runs roughly 5-50%), but the point estimate implies about 65 more bugs among the 305. `docs/card-implementation-audit.md` and `scripts/audit_deck_cards.py` are stale: they describe a `src/cards/manual/` layout that no longer exists.

### Work order

| # | Task | Size | Done when |
|---|---|---|---|
| A0 | Adopt `card_status.py` as the status source; delete or redirect the stale audit docs and script | S | One source of truth |
| A1 | Fix the 19 confirmed bugs, each with a failing test first | M | List empty |
| A2 | Fix the 23 sample-deck cards ([list](card-status/README.md#sample-decks-23-of-354-distinct-cards-in-decks-are-not-fully-implemented)); these bias every evaluation of those decks. Top by copies: Vex Apathetic (13), Ashe Focused (10), Grim Apothecary (7), Elder Dragon (7), Last Rites (6) | M | Stage 1 gate |
| A3 | Root-cause fixes below, which clear most of the 72 documented gaps in groups | L | Documented list under 15 |
| A4 | **Decision-surface audit:** find every card that auto-picks where the text says "you may", "choose" or "any number". Add `pick*` calls | M | No hidden decisions |
| A5 | Clause-level review plus a test for the 305 untested cards | L | Untested list empty |
| A6 | Per-card fuzz: put each card in a generated deck, run random games, check invariants and crashes | M | All cards pass in CI |
| A7 | Adjudicate the 51 text differences against `errata/` | S | List empty or annotated |
| A8 | Re-pull Radiance (the local data is the pre-release pull of 10-10; the set released 10-23), then add Deploy, Disarm, Show Off, Becomes Unit and the Legend Zone choice, then the 144 cards | XL | Needed for Stage 4 on the live meta |

**A4 matters more for this project than for a simulator.** A wrong number gives a slightly wrong evaluation. A hidden decision means the line **cannot be found at all**, because search can only explore options the engine offers. Azir Sovereign forcing every token to move, or Elder Dragon always hitting the first enemy, removes real lines from the tree. A human-authored "first match" loop passes every automated check, which is why A4 needs review rather than a script.

### Root causes (A3)

Grouped from the 72 documented gaps. Fix once, clear many.

| Cause | Example cards | Fix |
|---|---|---|
| **Hook now exists, card never updated** | Karma Channeler (`WhenYouRecycle`), Sivir Ambitious and Yeti Brawler (`excess_combat_damage`), Spoils of War (`any_unit_died_this_turn`) | Quick wins: wire the card; drop the stale comment |
| Effect plays for free instead of paying | Last Rites, Kai'Sa, Promising Future, Soulgorger, The Harrowing, Here to Help, Cursed Sarcophagus, Rift Herald | Migrate to `playPayingCost`, which landed after these were written |
| Opponent's choice decided by a built-in "rational pick" | King's Edict, Safety Inspector, Shard of Undoing, Call to Battle, Atakhan, Divine Judgment | Route the prompt to the opponent, so it becomes a real search node |
| Non-resource additional costs | Kraken Hunter, Commander Ledros, Cruel Patron, Legion Quartermaster, Atakhan, Wallop | General additional-cost model |
| "Units enter ready" scoped to a turn or player | Magma Wurm, Confront, Bushwhack, Wuju Master | One turn-scoped rule on the player |
| Missing events | Fae Dragon and Battle Mistress (spend a buff), Lillia (token played), Noxian Guillotine (takes damage) | Add events to `TriggerManager` |
| Multi-fire delayed abilities, extra trigger counts | Rally the Troops, Red Brambleback | Extend `DelayedAbility` |

Open engine items from `docs/rules-audit-2026-10.md` also belong here: modal spells choosing at resolution rather than on play, split damage chosen at resolution and capped at 12 prompts, no cleanup between chain items, and 23 triggers not yet moved to finalization-time choices.

---

## 3. Track B: search foundations

Everything in this track must exist before Stage 1 produces a number worth reading.

### B1. Fork at any decision (blocker)

Today 51-79% of decisions cannot be copied, because the rest of the game state is on the fiber's C++ stack.

| Option | Cost | Verdict |
|---|---|---|
| **Checkpoint + suffix:** a node stores the last clean snapshot plus the exact legal indices chosen since. To fork, resume the snapshot and replay the short suffix | About 100-200 µs per fork; small change | **Choose this** |
| Rewrite the engine as an explicit state machine | Fastest forks, but a rewrite of a 6,657-line `game_engine.cpp` with high regression risk | Only if profiling later demands it |
| Copy the fiber stack | Pointers into the stack make this unsound | Rejected |

Required with it:

1. **Move the RNG into `GameState`,** so a copy continues the same stream.
2. **Replay legal indices, not encoded action ids,** which collide.
3. **Get clone and resume fidelity to zero.** Two failures are on record, and neither is root-caused:
   - `resumeFromSnapshot` disagrees with the live game at 0.9-3.8% of clean decisions.
   - `riftbound_clone_equiv_test` fails most games, with a different count on each run, so it is also nondeterministic. Failing clones almost always report exactly 3 legal actions at step 1, whatever the original had. That suggests the clone is landing near game setup rather than at the snapshotted decision: a lead, not a diagnosis.

   Both tests become CI gates. `search_baseline` must also compare the following state, not just the legal list.

### B2. Position format and `riftbound_analyze`

Chess has FEN. We need the same: a JSON position capturing every zone, with each card's state (damage, exhaustion, buffs, attachments), turn state, scores, runes, and the chain. Save and load through the existing `state_editor` paths, and add `--export-position` to the web UI and replays.

Add a `riftbound_analyze` binary: position in, JSON report out. This contract stays stable through all four stages:

```json
{
  "stage": 1,
  "evaluation": { "win_p1": 0.71, "ci95": [0.68, 0.74], "points_equiv": 1.8 },
  "lines": [
    { "rank": 1, "win_p1": 0.71, "visits": 41200,
      "pv": ["Play Vex, Apathetic to Base", "Move Vex to The Grand Plaza", "End Turn",
             "(opp) Pass", "(you) Conquer: 7 -> 8"],
      "notes": ["wins on the following turn unless opponent has a Reaction"] }
  ],
  "search": { "nodes": 812345, "seconds": 10, "evaluator": "E1", "world_samples": 1 },
  "warnings": ["line 2 uses Last Rites (partial implementation)"]
}
```

`warnings` is not decoration: any line touching a card that is not fully implemented says so, read directly from `card-status.csv`.

### B3. Decision compression

Search should see *plans*, not rune taps. This is the largest single gain available.

- **Auto-apply forced decisions** (33-41% of the total).
- **Canonicalize payment.** Collapse rune choices that leave the same resources into one option; expand them only when they differ.
- **Skip empty priority passes** when the player has no legal response.
- **Implement `LegalActionDesc`.** Its injective description means no two actions collide, which also replaces the vocab slots that break replay. It already has a specified test (`tests/test_action_encoding.cpp`), which does not exist yet.
- **Define a line.** A line is the player's macro-actions up to the end of their turn, plus the principal opponent reply. Two orderings reaching the same state are the same line.

### B4. State hashing

Zobrist-style hash over `GameState` for transpositions and for spotting repeated states. This also gives a cheap regression tool: a `perft`-style count of reachable positions from fixed setups, which should change only when rules change.

---

## 4. Engine architecture

```
position.json ─┐
               ▼
        ┌─────────────┐   samples a fully specified game   ┌───────────────┐
        │WorldSampler │ ─────────────────────────────────▶ │  Search core  │
        └─────────────┘                                    │ PUCT + chance │
          Stage 1: the position itself                     │ nodes + TT    │
          Stage 2: + chance layer                          └───┬───────┬───┘
          Stage 3: + hand beliefs                 forks states │       │ leaf value
          Stage 4: + decklist beliefs                          ▼       ▼
                                              ┌────────────────┐  ┌───────────┐
                                              │ Forkable engine│  │ Evaluator │
                                              │ (B1, B3, B4)   │  │  E0 -> E3 │
                                              └────────────────┘  └───────────┘
```

New code goes in `src/search/`, kept apart from the OpenSpiel wrapper. OpenSpiel stays as the baseline to beat: its bots cannot express checkpoint forks, belief sampling or line extraction, and its interface would need more bending than a focused search costs to write.

**Why tree search rather than alpha-beta, given the "chess-like" goal.** Alpha-beta needs a strong evaluator and fast nodes, and it has no natural extension to chance or hidden information. PUCT with a value estimate carries across all four stages. Alpha-beta earns one place, as a **short-horizon solver:** at a leaf, prove or refute "can this player win this turn?". Riftbound positions often turn on exact lethal counts, and a proof beats any estimate there.

### The evaluator ladder

Each rung must beat the previous one in self-play before it is adopted. This is not optional: a code comment in `mcts_agent.cpp` records that adding might and hand-size signals made MCTS *worse* at low budgets.

| Level | What | When |
|---|---|---|
| E0 | Score difference (exists) | Baseline |
| E1 | Hand-built features, logistic win probability | Stage 1 |
| E2 | E1 + short playouts with a fixed scripted policy | Stage 1 |
| E3 | Value and policy network trained on engine self-play, using `feature_extractor` | After Stage 2; sets up priors for 3 and 4 |

Candidate E1 features, drawn from how the game is won:

- **Race:** points to Victory for each side; projected points next turn from holds and conquers; whether the winning point is reachable (CR 466.1.b: a Conquer win requires scoring every battlefield that turn).
- **Board:** Might by battlefield, ready versus exhausted, keyword value such as Tank and Ganking, equipment.
- **Resources:** runes in play, Energy and Power available next turn by domain, cards in hand, playable hand value, champion available, legend ability ready, XP.
- **Risk:** cards left in deck (Burn Out), who acts next, open mana representing a Reaction.

---

## 5. Stage 1: full information

**Sees:** everything, including deck order.

**Build:**

1. Single-threaded PUCT on B1 and B3, using E0.
2. `riftbound_analyze` end to end, reporting evaluation and three lines.
3. E1, then E2, each gated by self-play.
4. Win-this-turn solver at leaves.
5. Transposition table on B4.
6. Multi-threading with virtual loss.

Rare in-game shuffles stay sampled here; true chance handling arrives in Stage 2.

**Exit criteria:**

- Solves a hand-written puzzle set of forced wins (start with 50 "win this turn" and 20 "win next turn" positions).
- Beats OpenSpiel MCTS at equal time by a statistically significant margin in SPRT.
- Win probability is calibrated: predicted 70% wins about 70% in held-out self-play.
- The Track A sample-deck gate (A1, A2) is closed.

---

## 6. Stage 2: hidden deck order

**Sees:** both hands and lists; decks become unordered multisets.

With hands open this is still perfect information, just stochastic, like backgammon. That matters: chance nodes are sound here, and none of the Stage 3 complications apply yet.

**Build:**

1. Enable `StepKind::Chance`, routing the 6 RNG sites through a `ChanceProvider`.
2. Represent an unordered deck by late binding: a card is sampled when drawn, and known-position cards stay fixed (a Predict result, a recycled card at the bottom).
3. Chance nodes in search. Group outcomes by card identity rather than slot, so 40 positions become about 15 distinct outcomes, then use progressive widening.
4. Report outs in lines: "needs a Reaction off the top: 3 outs, 18%".

**Do not use root-sampled determinization here.** Fixing the deck order for a whole search makes the searcher clairvoyant: it plans as if it knows what it will draw. Search must branch at the moment of the draw.

**Simplification to note:** peeking creates private knowledge (a player who looked at their top card knows it). Stage 2 treats that as public; Stage 3 fixes it.

**Exit criteria:**

- Chance probabilities match hypergeometric math exactly in unit tests.
- Clairvoyance gap measured: the same positions under Stage 1 and Stage 2, reporting how much value perfect deck knowledge was worth.
- Beats the Stage 1 engine run on sampled decks in a head-to-head where neither may see order.

---

## 7. Stage 3: hidden hands

**Sees:** own hand, both lists. This is the largest conceptual step.

**Build:**

1. **`KnowledgeTracker`,** implementing `KnowledgeView`: what each player knows, across reveals, shuffles and recycles. The header specifies the rules; the extractor must read through it, never from `GameState` directly.
2. **`retcon(object, card)`:** swap a hidden object's identity and rebuild what derives from its definition, so a sampled world is a consistent game.
3. **Belief state:** a weighted particle set of worlds (opponent hand, deck split, facedown cards). The prior is uniform over the list minus seen cards. Updates come from an **opponent model:** if they passed with mana open, worlds where they held a good Reaction become less likely. Likelihood is a softmax over evaluator scores of their options.
4. **Information-set search:** sample a world per iteration, with tree nodes keyed by what the deciding player knows. Base it on single-observer ISMCTS, extended to multiple observers if self-play shows it is needed.
5. **Report risk:** "line 1 loses to Counter in hand (24%)". This is what a player actually wants from Stage 3.

**Known limitation:** ISMCTS does not compute a game-theoretic equilibrium. It suffers strategy fusion and will not bluff correctly, nor will it value hiding information. That is acceptable for a position evaluator, and it should be stated in the output. If it proves material, ReBeL-style subgame solving is the upgrade, and that is where the unavailable trainer repo's work would have fit.

**Exit criteria:**

- **No-cheating test:** two states identical from the player's view give identical output. This runs in CI and guards every later change.
- Beliefs are calibrated: predicted P(card in hand) matches actual, by Brier score over self-play.
- Beats Stage 2 run on naively sampled hands.
- The gap to omniscient play is measured and reported.

---

## 8. Stage 4: unknown decklists

**Sees:** public zones and revealed cards. Legend, battlefields and rune domains narrow the opponent's list immediately, so this stage starts with more information than it first appears.

**Build:**

1. **Meta corpus:** collect tournament lists by Legend. Piltover Archive is the natural source, since its format is already what `decks/` uses. Check its terms of use first. The 39 sample decks are test decks, not a meta.
2. **Deck prior:** P(card | Legend, domains) from the corpus, with a fallback from card pools when a Legend has few lists.
3. **Legal deck sampler:** generate complete lists (40 main including champion, 12 runes, 3 battlefields, max copies, domain identity, ban list), with `deck_validator` as a hard filter.
4. **Joint belief:** extend particles from `(hand, deck split)` to `(decklist, hand, deck split)`. Every revealed card reweights lists.
5. **Honest output:** a wider confidence interval, and a statement of what drives it: "evaluation depends on whether opponent is the control or tempo build (60/40)".

**Exit criteria:**

- Decklist inference: top-predicted cards match the real list at rates improving turn by turn.
- Interval coverage: the true result falls inside the stated 95% interval about 95% of the time.
- Win rate against Stage 3 is close, with the gap measured by format and turn number.

**Card dependency:** a sampled opponent deck will contain whatever the meta plays, including Radiance. Stage 4 output should report the share of sampled worlds that touch an unimplemented or partial card, and lower its confidence accordingly. On the live meta it needs A8.

---

## 9. Validation

How we know any of this is right, without a strong human reference:

| Check | Purpose |
|---|---|
| Puzzle suite | Positions with a known answer, growing with every found bug |
| SPRT self-play | Every evaluator or search change must prove itself, as chess engines do |
| Calibration plots | Win probabilities mean what they say |
| Resume fidelity | Zero mismatches between forked and live play, in CI |
| Golden replays | Fixed-seed games must not change unless a rules change explains it |
| `perft` counts | Detect unintended changes to legal moves |
| No-cheating test | Stage 3+: output depends only on available information |
| Human review | Show annotated replays to strong players; their "why did it do that?" finds what metrics miss |

**State of this checkout (`b05e51d`):** builds clean on Ubuntu 22.04 (GCC 11.4, Boost 1.83). 1,495 of 1,495 unit tests pass in 1.6 s, with 1 disabled. `riftbound_clone_equiv_test` **fails** (see B1). 500 random games ran at about 2,700 games per second on 8 threads, with no crashes or hangs. The unit tests never exercised the clone failure, which is why it needs its own gate.

**Baseline to beat:** the current `mcts:sims=N` bot.

---

## 10. Order of work

Track A runs throughout. Sizes are relative, not schedules.

| Milestone | Contains | Size | Result |
|---|---|---|---|
| **M0** | A0, A1, quick-win root causes; B1 | M | Search can fork anywhere; known bugs gone |
| **M1** | B2, B3, B4, A2 | L | Positions load; decisions compress; `riftbound_analyze` runs with E0 |
| **M2** | Stage 1 search, E1, E2, solver, A3, A4 | L | **First real evaluation and top 3 lines** |
| **M3** | Stage 2, E3 groundwork, A5, A6 | L | Works without seeing deck order |
| **M4** | Stage 3, A7 | XL | Works without seeing the opponent's hand |
| **M5** | A8, meta corpus, Stage 4 | XL | Rough evaluation from public information |

### Risks

| Risk | Mitigation |
|---|---|
| Resume mismatch has a deep cause | Tackled first in M0; explicit state machine is the fallback |
| Too few nodes for useful depth | B3 first, then E2; profile before optimizing |
| Hidden card bugs bias evaluation invisibly | A5 and A6, plus `warnings` in every report |
| Heuristic evaluator misleads search | Every rung gated by SPRT |
| New sets keep arriving | Status report regenerates; card work is a standing track |
| Windows builds | The project targets Linux; WSL2 is the supported path (see below) |

### Decisions needed

1. **Meta source for Stage 4.** Is Piltover Archive acceptable, or is there another corpus?
2. **Target format.** Is Radiance legal play the target? If so, A8 moves earlier and Stage 1's gate should use real meta decks.
3. **Trainer repo.** If the sibling `riftbound-trainer` can be obtained, E3 and the Stage 3 upgrade path get much cheaper.

---

## Appendix: building on this machine

`apt-get` needs a password here, so dependencies live in user space in the `Ubuntu-22.04` WSL distro. Sources stay on `E:`; the build tree is on the WSL filesystem, which is much faster than `/mnt/e`.

- Boost 1.83 built to `~/deps/boost` (`program_options`, `filesystem`, `system`, `context`, `atomic`)
- ninja via `pip3 install --user ninja`; cmake from snap (`/snap/bin`)
- Build tree at `~/alpharune-build`
- `core.autocrlf=false` and `core.eol=lf` set on this clone: CRLF would put `\r` into multi-line card text

```bash
wsl -d Ubuntu-22.04 -- bash -lc 'PATH=$HOME/.local/bin:/snap/bin:$PATH cmake --build ~/alpharune-build -j10'
```

Run tests from the repo root inside WSL:

```bash
LD_LIBRARY_PATH=$HOME/deps/boost/lib RIFTBOUND_ROOT=. ~/alpharune-build/riftbound_tests
```

Use `-j10` rather than all 20 threads: unity batches are memory-heavy and the distro has 15 GB. A clean build takes about 3 minutes.
