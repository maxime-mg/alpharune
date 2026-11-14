# PLAY-021: Drag a card from hand to play it

- **Priority:** P1
- **Area:** Phase 2
- **Status:** todo
- **Depends on:** [PLAY-007](PLAY-007-describe-what-each-legal-action-does.md), [PLAY-010](PLAY-010-visual-board-layout-with-every-zone.md)
- **Blocks:** [PLAY-022](PLAY-022-chooser-when-one-drop-matches-several-actions.md), [PLAY-023](PLAY-023-pay-costs-by-clicking-runes.md), [PLAY-024](PLAY-024-optional-costs-accelerate-additional-costs-repeat.md), [PLAY-045](PLAY-045-face-down-cards.md), [PLAY-061](PLAY-061-click-to-select-fallback-and-keyboard-shortcuts.md)

## Scope
- On pickup, highlight every drop zone that a legal `PlayCard` or `HideCard` intent names for that card: base or a battlefield.
- Champion zone, trash and face-down cards are draggable only when an intent names them.
- A drop sends the matching index. A drop that matches nothing is impossible, because the zone was never highlighted.
