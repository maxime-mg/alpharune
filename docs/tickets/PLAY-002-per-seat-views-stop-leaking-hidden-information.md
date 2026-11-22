# PLAY-002: Per-seat views: stop leaking hidden information

- **Priority:** P0
- **Area:** Phase 1
- **Status:** done
- **Depends on:** [PLAY-001](PLAY-001-split-the-play-app-out-of-the-engine.md)
- **Blocks:** [PLAY-008](PLAY-008-event-stream-for-animation-and-a-readable-log.md), [PLAY-010](PLAY-010-visual-board-layout-with-every-zone.md), [PLAY-045](PLAY-045-face-down-cards.md), [PLAY-060](PLAY-060-hot-seat-privacy-screen.md)

## Why
Every client currently gets both hands and the order of both main decks and rune decks.

## Done
- With two human seats the server is seat-bound: a connection claims `P1` or `P2` with a `seat` message and gets only that player's view. `--hot-seat` turns this off for two players on one screen.
- With `--god-mode off`, hidden zones are counts plus card backs, face-down cards show only to their controller, and log lines are filtered by `LogAudience`.
- The view is built in `play/src/board_view.cpp`.

The client no longer picks `god` or `spectator`. A connection that claims no seat on a seat-bound server is a spectator and sees neither hand; whether everything is revealed is the server's `reveal_hidden`.

**Caveat:** god mode is on by default when a seat is human, so everything is revealed unless the game is started with `--god-mode off`. See [play-ui-followups.md](../play-ui-followups.md); changing the default is a decision for the owner.

## Done when
With god mode off, a P1 frame contains no card names or ids from P2's hand, main deck or rune deck, or from P2's face-down cards. Covered by a unit test.
