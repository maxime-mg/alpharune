# RAD-009: Radiance: Mind cards

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md)
- **Blocks:** nothing

## Why these belong together
The remaining Radiance Mind units, spells and gear. Cards in one domain share themes and helpers, so they are written together.

10 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Coalesced Energy (`RAD-016/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Ekko, Ingenious (`RAD-061/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| High Note (`RAD-048/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Improvised Projectile (`RAD-056/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Orianna, Mechanical Marvel (`RAD-068/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Parallel Convergence (`RAD-051/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Piltovan Guidebot (`RAD-057/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Riptide Mage (`RAD-058/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Scrutinizing Scope (`RAD-064/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Zero Point Inflection (`RAD-069/167`) | RAD | gear | not-implemented |  | no class in src/cards |

## Rules text
- **Coalesced Energy**: At the end of your turn, recycle all your runes, then channel 4 runes exhausted.
- **Ekko, Ingenious**: As an additional cost to play me, return a friendly unit to its owner's hand. [Ganking] (I can move from battlefield to battlefield.)
- **High Note**: [Reaction] (Play any time, even before spells and abilities resolve.) Give a unit –[M] this turn equal to the number of exhausted units you control.
- **Improvised Projectile**: [Action] (Play on your turn or in showdowns.) Choose a friendly gear. Deal damage equal to its Energy cost to a unit at a battlefield.
- **Orianna, Mechanical Marvel**: Once each turn, while I'm at a battlefield, you may pay :rb_energy_1::rb_rune_rainbow: less to play a gear.
- **Parallel Convergence**: As an additional cost to play this, return a friendly unit to its owner's hand. Deal 5 to a unit at a battlefield.
- **Piltovan Guidebot**: When you play a card, you may have my base Might become equal to that card's Energy cost this turn.
- **Riptide Mage**: [Accelerate] (You may pay :rb_energy_1::rb_rune_mind: as an additional cost to have me enter ready.) When you play me, give an enemy unit -3 [M] this turn.
- **Scrutinizing Scope**: :rb_energy_1:, :rb_exhaust:: Choose an opponent. They guess a card type. Reveal the top card of your Main Deck. If it has that type, recycle it. Otherwise, draw it. (Unit, gear, and spell are types.)
- **Zero Point Inflection**: When you play a card, draw 2. When you draw 1 or more cards with 9 or more other cards in your hand, discard your hand, kill this, and choose an opponent. They score 1 point.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
