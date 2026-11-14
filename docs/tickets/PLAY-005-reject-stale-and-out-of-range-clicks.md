# PLAY-005: Reject stale and out-of-range clicks

- **Priority:** P0
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [PLAY-004](PLAY-004-humanagent-reports-the-player-who-is-actually-deciding.md)
- **Blocks:** nothing

## Why
An out-of-range index silently falls back to option 0, and a click on an old decision is applied to the new one.

## Scope
- The client echoes `decision_id` with every `choose`. The server rejects a mismatch or an out-of-range index with an `err` frame.

## Done when
Tests cover a stale id, a negative index and an index past the end. None of them change the game.
