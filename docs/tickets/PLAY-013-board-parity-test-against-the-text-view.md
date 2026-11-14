# PLAY-013: Board parity test against the text view

- **Priority:** P2
- **Area:** Phase 1
- **Status:** todo
- **Depends on:** [PLAY-011](PLAY-011-card-rendering.md), [PLAY-009](PLAY-009-golden-frame-and-gesture-round-trip-tests.md)
- **Blocks:** nothing

## Scope
- For states from scripted games, assert that every fact `StateRenderer` prints has a matching field in the frame, and that the page renders it (headless browser check).
