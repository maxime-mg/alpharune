# RAD-013: Radiance: cards with no domain in the pre-release data

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** nothing

## Why these belong together
These had no domain in the pre-release gallery. Sort them into the domain tickets once RAD-000 refreshes the data.

5 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Entrenched Boar (`RAD-026/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Fanfare (`RAD-047/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Hush (`RAD-041/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| One-Two Punch (`RAD-121/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Scorched Earth (`RAD-006/167`) | RAD | spell | not-implemented |  | no class in src/cards |

## Rules text
- **Entrenched Boar**: You may have me enter ready. If you do, I can't move this turn.
- **Fanfare**: The next three times you play a unit this turn, draw 1.
- **Hush**: [Reaction] (Play any time, even before spells and abilities resolve.) Counter a spell with Power cost :rb_rune_rainbow::rb_rune_rainbow: or more.
- **One-Two Punch**: [Action] (Play on your turn or in showdowns.) [Buff] a friendly unit and [Stun] an enemy unit. (Give the friendly unit a +1 [M] buff if it doesn't have one. The enemy unit doesn't deal combat damage this turn.)
- **Scorched Earth**: [Action] (Play on your turn or in showdowns.) Deal 5 damage split among units at a single battlefield.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
