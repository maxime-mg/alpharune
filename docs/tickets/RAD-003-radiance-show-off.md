# RAD-003: Radiance: Show Off

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** nothing

## Why these belong together
Show Off reveals a card from hand or picks a friendly object as you play the card, and later effects read what was shown.

3 cards.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Primordial Roar (`RAD-082/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Rousing Display (`RAD-127/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| The Empyrean (`RAD-060/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Primordial Roar**: [Show Off] a unit. (As you play this, you may reveal a unit from hand or pick a friendly unit.) If you showed off a unit, deal damage equal to that unit's Might to a unit.
- **Rousing Display**: [Show Off] a card with Energy cost :rb_energy_7: or more. (As you play this, you may reveal such a card from hand or pick a friendly one.) Play two 1 [M] Recruit unit tokens to your base. If you showed off, play four to your base instead.disa
- **The Empyrean**: [Show Off] a card. (As you play me, you may reveal a card from hand or pick another friendly one.) When you play me, if you showed off a card, [Predict] equal to its Power cost, then draw 2. (Look at that many cards from your Main Deck. Recycle any of them and put the rest back in any order.)

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
