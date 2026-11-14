# PLAY-003: Full board state in every frame

- **Priority:** P0
- **Area:** Phase 1
- **Status:** done
- **Depends on:** [PLAY-001](PLAY-001-split-the-play-app-out-of-the-engine.md)
- **Blocks:** [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md), [PLAY-007](PLAY-007-describe-what-each-legal-action-does.md), [PLAY-009](PLAY-009-golden-frame-and-gesture-round-trip-tests.md), [PLAY-010](PLAY-010-visual-board-layout-with-every-zone.md), [PLAY-023](PLAY-023-pay-costs-by-clicking-runes.md), [PLAY-040](PLAY-040-chain-panel-with-target-arrows.md)

## Why
`objToJson` sends only id, name, exhausted, damage and might. A visual board needs everything the text view shows.

## Scope
- Objects: card def id, type, owner, controller, base and current might, damage, exhausted, stunned, buffs, temporary might, keywords (with Assault, Shield and Deflect values), token, hidden, attachments, combat designation, image URL, and rules text.
- Players: score, XP, legend and champion zones, runes with domain and ready state, rune pool (energy and power by domain), deck and rune deck counts, banishment.
- Battlefields: controller, contested, combat and showdown state (staged or in progress), attacker and defender, face-down cards.
- Turn: phase, turn number, turn player, priority and focus holders.
- Chain items, and the cost being paid.

## Done when
For any state reached in scripted games, every field `StateRenderer` prints is present in the frame.
