# Tickets

One file per ticket. Ticket ids: `PLAY` for the play interface (`play/`), `ENG` for engine work (`src/`), `CARD` for partial or missing cards from the existing sets, `RAD` for the Radiance set.

Priorities: **P0** now, **P1** next, **P2** after that, **P3** later. The open list is in working order: each ticket comes after everything it depends on.

Source documents: [ui-roadmap.md](../ui-roadmap.md) and [card-status](../card-status/README.md). Card tickets were generated once, on 2026-11-06, from `docs/card-status/card-status.csv` and the card text. Edit them by hand from now on; rerunning `scripts/card_status.py` does not update them.

> **Re-checked 2026-11-22 against upstream `ec90f0f`** (2026-10-15), by reading the code. Upstream finished ENG-001, ENG-002, PLAY-004, PLAY-005 and 100 of the 105 cards in CARD-001 to CARD-012. The five left were fixed here, in CARD-013. No RAD ticket has been started. The other tickets were updated where they named code that changed.

## Working order

39 tickets open.

| # | Ticket | Priority | Area | Status | Depends on |
|---|---|---|---|---|---|
| 1 | [PLAY-007](PLAY-007-describe-what-each-legal-action-does.md): Describe what each legal action does | P1 | Phase 0 | todo | PLAY-003 |
| 2 | [PLAY-008](PLAY-008-event-stream-for-animation-and-a-readable-log.md): Event stream for animation and a readable log | P1 | Phase 0 | todo | PLAY-002 |
| 3 | [PLAY-009](PLAY-009-golden-frame-and-gesture-round-trip-tests.md): Golden-frame and gesture round-trip tests | P1 | Phase 0 | todo | PLAY-003, PLAY-007 |
| 4 | [PLAY-020](PLAY-020-animate-draws-channels-and-discards.md): Animate draws, channels and discards | P1 | Phase 2 | todo | PLAY-008, PLAY-010 |
| 5 | [PLAY-021](PLAY-021-drag-a-card-from-hand-to-play-it.md): Drag a card from hand to play it | P1 | Phase 2 | todo | PLAY-007, PLAY-010 |
| 6 | [PLAY-022](PLAY-022-chooser-when-one-drop-matches-several-actions.md): Chooser when one drop matches several actions | P1 | Phase 2 | todo | PLAY-021 |
| 7 | [PLAY-023](PLAY-023-pay-costs-by-clicking-runes.md): Pay costs by clicking runes | P1 | Phase 2 | todo | PLAY-003, PLAY-006, PLAY-021 |
| 8 | [PLAY-030](PLAY-030-drag-a-unit-to-move-it.md): Drag a unit to move it | P1 | Phase 3 | todo | PLAY-007, PLAY-010 |
| 9 | [PLAY-040](PLAY-040-chain-panel-with-target-arrows.md): Chain panel with target arrows | P1 | Phase 4 | todo | PLAY-003, PLAY-012 |
| 10 | [PLAY-041](PLAY-041-targeting-on-the-board.md): Targeting on the board | P1 | Phase 4 | todo | PLAY-006, PLAY-007 |
| 11 | [PLAY-042](PLAY-042-reactions-and-accept.md): Reactions and Accept | P1 | Phase 4 | todo | PLAY-004, PLAY-040 |
| 12 | [PLAY-050](PLAY-050-showdown-focus-and-pass.md): Showdown focus and Pass | P1 | Phase 5 | todo | PLAY-004, PLAY-040 |
| 13 | [PLAY-051](PLAY-051-combat-view-and-damage-preview.md): Combat view and damage preview | P1 | Phase 5 | todo | PLAY-006, PLAY-011 |
| 14 | [PLAY-053](PLAY-053-setup-and-mulligan-screens.md): Setup and mulligan screens | P1 | Phase 5 | todo | PLAY-006 |
| 15 | [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md): Refresh Radiance card data and scaffold the classes | P1 | Cards | todo |  |
| 16 | [PLAY-013](PLAY-013-board-parity-test-against-the-text-view.md): Board parity test against the text view | P2 | Phase 1 | todo | PLAY-011, PLAY-009 |
| 17 | [PLAY-024](PLAY-024-optional-costs-accelerate-additional-costs-repeat.md): Optional costs: Accelerate, additional costs, Repeat | P2 | Phase 2 | todo | PLAY-006, PLAY-021 |
| 18 | [PLAY-031](PLAY-031-move-several-units-together.md): Move several units together | P2 | Phase 3 | todo | PLAY-030, PLAY-006, ENG-002 |
| 19 | [PLAY-043](PLAY-043-activated-abilities-menu.md): Activated abilities menu | P2 | Phase 4 | todo | PLAY-007 |
| 20 | [PLAY-044](PLAY-044-triggers-modes-x-values-and-card-name-pickers.md): Triggers, modes, X values and card-name pickers | P2 | Phase 4 | todo | PLAY-006 |
| 21 | [PLAY-045](PLAY-045-face-down-cards.md): Face-down cards | P2 | Phase 4 | todo | PLAY-002, PLAY-021 |
| 22 | [PLAY-052](PLAY-052-score-control-and-conquer-animations.md): Score, control and conquer animations | P2 | Phase 5 | todo | PLAY-008 |
| 23 | [PLAY-054](PLAY-054-game-over-screen-and-log-export.md): Game over screen and log export | P2 | Phase 5 | todo | PLAY-008 |
| 24 | [PLAY-061](PLAY-061-click-to-select-fallback-and-keyboard-shortcuts.md): Click-to-select fallback and keyboard shortcuts | P2 | Cross-cutting | todo | PLAY-021, PLAY-030 |
| 25 | [RAD-001](RAD-001-radiance-deploy-gear-and-bomb-tokens.md): Radiance: Deploy gear and Bomb tokens | P2 | Cards | todo | RAD-000 |
| 26 | [RAD-002](RAD-002-radiance-disarm.md): Radiance: Disarm | P2 | Cards | todo | RAD-000 |
| 27 | [RAD-003](RAD-003-radiance-show-off.md): Radiance: Show Off | P2 | Cards | todo | RAD-000 |
| 28 | [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md): Radiance: Recruit and Mech tokens | P2 | Cards | todo | RAD-000 |
| 29 | [RAD-005](RAD-005-radiance-legends.md): Radiance: legends | P2 | Cards | todo | RAD-000 |
| 30 | [RAD-006](RAD-006-radiance-battlefields.md): Radiance: battlefields | P2 | Cards | todo | RAD-000 |
| 31 | [PLAY-060](PLAY-060-hot-seat-privacy-screen.md): Hot-seat privacy screen | P3 | Cross-cutting | todo | PLAY-002 |
| 32 | [PLAY-062](PLAY-062-ship-the-web-files-inside-the-binary.md): Ship the web files inside the binary | P3 | Cross-cutting | todo | PLAY-001 |
| 33 | [RAD-007](RAD-007-radiance-fury-cards.md): Radiance: Fury cards | P3 | Cards | todo | RAD-000, RAD-004 |
| 34 | [RAD-008](RAD-008-radiance-calm-cards.md): Radiance: Calm cards | P3 | Cards | todo | RAD-000, RAD-004 |
| 35 | [RAD-009](RAD-009-radiance-mind-cards.md): Radiance: Mind cards | P3 | Cards | todo | RAD-000, RAD-004 |
| 36 | [RAD-010](RAD-010-radiance-body-cards.md): Radiance: Body cards | P3 | Cards | todo | RAD-000, RAD-004 |
| 37 | [RAD-011](RAD-011-radiance-chaos-cards.md): Radiance: Chaos cards | P3 | Cards | todo | RAD-000, RAD-004 |
| 38 | [RAD-012](RAD-012-radiance-order-cards.md): Radiance: Order cards | P3 | Cards | todo | RAD-000, RAD-004 |
| 39 | [RAD-013](RAD-013-radiance-cards-with-no-domain-in-the-pre-release-data.md): Radiance: cards with no domain in the pre-release data | P3 | Cards | todo | RAD-000 |

## Done

24 tickets.

| # | Ticket | Priority | Area | Status | Depends on |
|---|---|---|---|---|---|
| — | [CARD-001](CARD-001-fix-confirmed-bugs-in-implemented-cards.md): Fix confirmed bugs in implemented cards | P0 | Cards | done |  |
| — | [CARD-002](CARD-002-offer-the-choices-that-cards-currently-make-for-the-player.md): Offer the choices that cards currently make for the player | P0 | Cards | done |  |
| — | [ENG-001](ENG-001-structured-prompt-kind-for-every-choice.md): Structured prompt kind for every choice | P0 | Engine | done |  |
| — | [ENG-002](ENG-002-group-moves-cr-144-3.md): Group moves (CR 144.3) | P0 | Engine | done |  |
| — | [CARD-008](CARD-008-movement-combat-and-scoring-rules.md): Movement, combat and scoring rules | P0 | Cards | done | ENG-002 |
| — | [PLAY-001](PLAY-001-split-the-play-app-out-of-the-engine.md): Split the play app out of the engine | P0 | Phase 1 | done |  |
| — | [PLAY-002](PLAY-002-per-seat-views-stop-leaking-hidden-information.md): Per-seat views: stop leaking hidden information | P0 | Phase 1 | done | PLAY-001 |
| — | [PLAY-003](PLAY-003-full-board-state-in-every-frame.md): Full board state in every frame | P0 | Phase 1 | done | PLAY-001 |
| — | [PLAY-004](PLAY-004-humanagent-reports-the-player-who-is-actually-deciding.md): HumanAgent reports the player who is actually deciding | P0 | Phase 0 | done | PLAY-001 |
| — | [PLAY-005](PLAY-005-reject-stale-and-out-of-range-clicks.md): Reject stale and out-of-range clicks | P0 | Phase 0 | done | PLAY-004 |
| — | [PLAY-010](PLAY-010-visual-board-layout-with-every-zone.md): Visual board layout with every zone | P0 | Phase 1 | done | PLAY-002, PLAY-003 |
| — | [PLAY-011](PLAY-011-card-rendering.md): Card rendering | P0 | Phase 1 | done | PLAY-010 |
| — | [PLAY-012](PLAY-012-side-panel-turn-priority-chain-actions-and-text-view.md): Side panel: turn, priority, chain, actions and text view | P0 | Phase 1 | done | PLAY-010 |
| — | [CARD-003](CARD-003-let-opponents-make-their-own-choices-during-a-resolution.md): Let opponents make their own choices during a resolution | P1 | Cards | done |  |
| — | [CARD-004](CARD-004-non-resource-additional-costs-and-conditional-cost-changes.md): Non-resource additional costs and conditional cost changes | P1 | Cards | done |  |
| — | [CARD-005](CARD-005-playing-cards-from-trash-deck-or-banishment-with-partial-cos.md): Playing cards from trash, deck or banishment with partial costs | P1 | Cards | done |  |
| — | [CARD-006](CARD-006-this-turn-effects-and-turn-scoped-tracking.md): "This turn" effects and turn-scoped tracking | P1 | Cards | done |  |
| — | [CARD-007](CARD-007-missing-trigger-events.md): Missing trigger events | P1 | Cards | done |  |
| — | [CARD-013](CARD-013-gaps-left-after-upstreams-card-fixes.md): Gaps left after upstream's card fixes | P1 | Cards | done |  |
| — | [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md): Send structured prompts to the client | P1 | Phase 0 | done | ENG-001, PLAY-003 |
| — | [CARD-009](CARD-009-hidden-cards.md): Hidden cards | P2 | Cards | done |  |
| — | [CARD-010](CARD-010-replacement-effects.md): Replacement effects | P2 | Cards | done |  |
| — | [CARD-011](CARD-011-equipment.md): Equipment | P2 | Cards | done |  |
| — | [CARD-012](CARD-012-keywords-and-targeting-rules.md): Keywords and targeting rules | P2 | Cards | done |  |

## Dependencies

Done tickets are included.

```mermaid
graph LR
  PLAY_001 --> PLAY_002
  PLAY_001 --> PLAY_003
  PLAY_001 --> PLAY_004
  PLAY_004 --> PLAY_005
  ENG_001 --> PLAY_006
  PLAY_003 --> PLAY_006
  PLAY_003 --> PLAY_007
  PLAY_002 --> PLAY_008
  PLAY_003 --> PLAY_009
  PLAY_007 --> PLAY_009
  PLAY_002 --> PLAY_010
  PLAY_003 --> PLAY_010
  PLAY_010 --> PLAY_011
  PLAY_010 --> PLAY_012
  PLAY_011 --> PLAY_013
  PLAY_009 --> PLAY_013
  PLAY_008 --> PLAY_020
  PLAY_010 --> PLAY_020
  PLAY_007 --> PLAY_021
  PLAY_010 --> PLAY_021
  PLAY_021 --> PLAY_022
  PLAY_003 --> PLAY_023
  PLAY_006 --> PLAY_023
  PLAY_021 --> PLAY_023
  PLAY_006 --> PLAY_024
  PLAY_021 --> PLAY_024
  PLAY_007 --> PLAY_030
  PLAY_010 --> PLAY_030
  PLAY_030 --> PLAY_031
  PLAY_006 --> PLAY_031
  ENG_002 --> PLAY_031
  PLAY_003 --> PLAY_040
  PLAY_012 --> PLAY_040
  PLAY_006 --> PLAY_041
  PLAY_007 --> PLAY_041
  PLAY_004 --> PLAY_042
  PLAY_040 --> PLAY_042
  PLAY_007 --> PLAY_043
  PLAY_006 --> PLAY_044
  PLAY_002 --> PLAY_045
  PLAY_021 --> PLAY_045
  PLAY_004 --> PLAY_050
  PLAY_040 --> PLAY_050
  PLAY_006 --> PLAY_051
  PLAY_011 --> PLAY_051
  PLAY_008 --> PLAY_052
  PLAY_006 --> PLAY_053
  PLAY_008 --> PLAY_054
  PLAY_002 --> PLAY_060
  PLAY_021 --> PLAY_061
  PLAY_030 --> PLAY_061
  PLAY_001 --> PLAY_062
  ENG_002 --> CARD_008
  RAD_000 --> RAD_001
  RAD_000 --> RAD_002
  RAD_000 --> RAD_003
  RAD_000 --> RAD_004
  RAD_000 --> RAD_005
  RAD_000 --> RAD_006
  RAD_000 --> RAD_007
  RAD_004 --> RAD_007
  RAD_000 --> RAD_008
  RAD_004 --> RAD_008
  RAD_000 --> RAD_009
  RAD_004 --> RAD_009
  RAD_000 --> RAD_010
  RAD_004 --> RAD_010
  RAD_000 --> RAD_011
  RAD_004 --> RAD_011
  RAD_000 --> RAD_012
  RAD_004 --> RAD_012
  RAD_000 --> RAD_013
```
