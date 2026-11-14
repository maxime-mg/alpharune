# RAD-011: Radiance: Chaos cards

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md)
- **Blocks:** nothing

## Why these belong together
The remaining Radiance Chaos units, spells and gear. Cards in one domain share themes and helpers, so they are written together.

12 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| An Offer You Can't Refuse (`RAD-101/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Bedeviling Stranger (`RAD-107/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Brawl (`RAD-004/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Lulu, Whimsical (`RAD-110/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Momentary Companion (`RAD-095/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Reckless Innovator (`RAD-105/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Riven, Deserter (`RAD-114/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Runeblade Anvil (`RAD-012/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Siren's Song (`RAD-098/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Suave Negotiator (`RAD-099/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| The Waters' Wage (`RAD-100/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Transpositionist (`RAD-115/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **An Offer You Can't Refuse**: [Action] (Play on your turn or in showdowns.) Kill a gear. Its controller plays a Gold gear token exhausted. (It has "[Reaction][>] Kill this, :rb_exhaust:: [Add] :rb_rune_rainbow:.")
- **Bedeviling Stranger**: [Hidden] (Hide now for :rb_rune_rainbow: to react with later for :rb_energy_0:.) When you play me, choose a battlefield. If there are 4 or more units there, you may return an enemy unit there to its owner's hand.
- **Brawl**: [Action] (Play on your turn or in showdowns.) Give a unit [Assault] this turn equal to its Might. (+1 [M] for each instance of Assault while it's an attacker.)
- **Lulu, Whimsical**: When I move, you may have another unit's base Might become 4 this turn.
- **Momentary Companion**: [Hidden] (Hide now for :rb_rune_rainbow: to react with later for :rb_energy_0:.) When you play me, if it's not your turn, give me [Temporary]. (Kill me at the start of my controller's Beginning Phase, before scoring.)
- **Reckless Innovator**: [Accelerate] (You may pay :rb_energy_1::rb_rune_chaos: as an additional cost to have me enter ready.) When I become exhausted, choose one —Play a Gold gear token exhausted. (It has "[Reaction][>] Kill this, :rb_exhaust:: [Add] :rb_rune_rainbow:.")Pay :rb_rune_chaos: to draw 1.
- **Riven, Deserter**: When another friendly unit becomes exhausted, give me +1 [M] this turn. Exhaust a friendly unit: Ready a unit. Use this ability only once each turn.
- **Runeblade Anvil**: When you play this or when this leaves the board, banish the top card of your Main Deck. Until your next Ending Phase, you may play that card. (You still pay its costs.) :rb_energy_2::rb_rune_chaos:, :rb_exhaust:: Kill this.
- **Siren's Song**: [Reaction] (Play any time, even before spells and abilities resolve.) Move an enemy unit that moved this turn.
- **Suave Negotiator**: When you play me, choose an opponent. You and that player each draw 1.
- **The Waters' Wage**: [Hidden] (Hide now for :rb_rune_rainbow: to react with later for :rb_energy_0:.) Play a Gold gear token exhausted. If it's your turn and you played this from face down, play three ready Gold gear tokens instead. (Gold tokens have "[Reaction][>] Kill this, :rb_exhaust:: [Add] :rb_rune_rainbow:.")
- **Transpositionist**: :rb_rune_chaos:, :rb_exhaust:: Return a friendly unit to its owner's hand. You may play a unit with less Might than that unit had, ignoring its cost.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
