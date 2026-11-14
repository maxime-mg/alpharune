# RAD-004: Radiance: Recruit and Mech tokens

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** [RAD-007](RAD-007-radiance-fury-cards.md), [RAD-008](RAD-008-radiance-calm-cards.md), [RAD-009](RAD-009-radiance-mind-cards.md), [RAD-010](RAD-010-radiance-body-cards.md), [RAD-011](RAD-011-radiance-chaos-cards.md), [RAD-012](RAD-012-radiance-order-cards.md)

## Why these belong together
Cards built around making Recruit and Mech unit tokens, including token replacement (Heimerdinger).

7 cards.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Antagonistic Apparatus (`RAD-009/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Black Rose Sanctum (`RAD-159/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Encore (`RAD-152/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Heimerdinger, Academy Instructor (`RAD-132/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Industrious Mechsmith (`RAD-049/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Rally for War (`RAD-007/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Seraphine, Not Alone (`RAD-138/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Antagonistic Apparatus**: I enter ready. When you play me, play a ready 3 [M] Mech unit token here.
- **Black Rose Sanctum**: For each player, the first time each turn a non-token unit they control dies here, they play a 1 [M] Recruit unit token to their base.
- **Encore**: Choose two —Deal 2 to a unit.Play two 1 [M] Recruit unit tokens.Give up to two units each +1 [M] this turn.Look at the top 2 cards of your Main Deck, draw 1 and recycle the other.
- **Heimerdinger, Academy Instructor**: If you would play a unit token to a location while I'm at a battlefield, you may play a 3 [M] Mech unit token to that same location instead.
- **Industrious Mechsmith**: When you play me, play a 3 [M] Mech unit token here.
- **Rally for War**: Choose an occupied enemy battlefield. Play three 1 [M] Recruit unit tokens there.
- **Seraphine, Not Alone**: When you play me or when I become exhausted, play a 1 [M] Recruit unit token to your base. I have +1 [M] for each other exhausted unit you control.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
