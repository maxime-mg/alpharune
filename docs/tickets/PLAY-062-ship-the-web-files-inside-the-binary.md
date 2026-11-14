# PLAY-062: Ship the web files inside the binary

- **Priority:** P3
- **Area:** Cross-cutting
- **Status:** todo
- **Depends on:** [PLAY-001](PLAY-001-split-the-play-app-out-of-the-engine.md)
- **Blocks:** nothing

## Why
`riftbound_play` serves `play/web/` from disk, which is easiest while the UI changes. A release build should be one file.

## Scope
- Embed `play/web/` at build time behind an option, keeping disk serving for development.
