# PLAY-009: Golden-frame and gesture round-trip tests

- **Priority:** P1
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [PLAY-003](PLAY-003-full-board-state-in-every-frame.md), [PLAY-007](PLAY-007-describe-what-each-legal-action-does.md)
- **Blocks:** [PLAY-013](PLAY-013-board-parity-test-against-the-text-view.md)

## Scope
- Golden JSON frames for one decision of each prompt kind, from seeded scripted games.
- Round-trip: for every legal index, the descriptor-driven matcher returns that index.

## Done when
Both run in CI next to `riftbound_tests`.
