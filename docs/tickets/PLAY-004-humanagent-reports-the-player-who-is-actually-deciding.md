# PLAY-004: HumanAgent reports the player who is actually deciding

- **Priority:** P0
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [PLAY-001](PLAY-001-split-the-play-app-out-of-the-engine.md)
- **Blocks:** [PLAY-005](PLAY-005-reject-stale-and-out-of-range-clicks.md), [PLAY-042](PLAY-042-reactions-and-accept.md), [PLAY-050](PLAY-050-showdown-focus-and-pass.md)

## Why
`paused_player_ = state.turn.turn_player` is wrong for reactions, defender choices, focus passes and opponent prompts, so the wrong seat is told it is their turn.

## Scope
- Pass the deciding player into `selectAction` (or derive it from the legal intents' `player`) and publish that.

## Done when
A test with a P2 reaction during P1's turn reports P2 as the deciding player.
