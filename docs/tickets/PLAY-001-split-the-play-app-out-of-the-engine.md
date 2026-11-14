# PLAY-001: Split the play app out of the engine

- **Priority:** P0
- **Area:** Phase 1
- **Status:** done
- **Depends on:** nothing
- **Blocks:** [PLAY-002](PLAY-002-per-seat-views-stop-leaking-hidden-information.md), [PLAY-003](PLAY-003-full-board-state-in-every-frame.md), [PLAY-004](PLAY-004-humanagent-reports-the-player-who-is-actually-deciding.md), [PLAY-062](PLAY-062-ship-the-web-files-inside-the-binary.md)

## Why
The engine must build and run on its own: batch games, search, training and tests should not pull in a web server or UI code.

## Scope
- Move `PlayServer`, `HumanAgent` and the old embedded page out of `src/` into a new top-level `play/` directory.
- New `riftbound_play` executable for interactive games. The engine's `riftbound` binary keeps batch and AI-vs-AI play, without `--web` or `human` seats.
- The web front end lives in `play/web/` as plain files, served from disk.
- `-DRIFTBOUND_BUILD_PLAY=OFF` builds the engine with no play code at all.

## Done when
- `riftbound_core`, `riftbound` and `riftbound_tests` build with `RIFTBOUND_BUILD_PLAY=OFF`.
- No file under `src/` includes anything from `play/`.
