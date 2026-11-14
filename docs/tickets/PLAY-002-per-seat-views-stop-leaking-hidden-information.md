# PLAY-002: Per-seat views: stop leaking hidden information

- **Priority:** P0
- **Area:** Phase 1
- **Status:** done
- **Depends on:** [PLAY-001](PLAY-001-split-the-play-app-out-of-the-engine.md)
- **Blocks:** [PLAY-008](PLAY-008-event-stream-for-animation-and-a-readable-log.md), [PLAY-010](PLAY-010-visual-board-layout-with-every-zone.md), [PLAY-045](PLAY-045-face-down-cards.md), [PLAY-060](PLAY-060-hot-seat-privacy-screen.md)

## Why
Every client currently gets both hands and the order of both main decks and rune decks.

## Scope
- Each WebSocket connection picks a seat: `p1`, `p2`, `spectator` or `god`.
- Hidden zones become counts plus card backs. Face-down cards show only to their controller.
- `god` sees everything and is only allowed when god mode is on.
- The per-seat view is built in one place (`play/src/board_view.cpp`), not spread across the server.

## Done when
A `p1` frame contains no card names or ids from P2's hand, main deck or rune deck, or from P2's face-down cards. Covered by a unit test.
