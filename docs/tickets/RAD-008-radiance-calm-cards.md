# RAD-008: Radiance: Calm cards

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md)
- **Blocks:** nothing

## Why these belong together
The remaining Radiance Calm units, spells and gear. Cards in one domain share themes and helpers, so they are written together.

16 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Blade of the Exile (`RAD-150/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Consult the Heavens (`RAD-025/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Ephemeral Ward (`RAD-039/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Heartening Soul (`RAD-040/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Hunker Down (`RAD-034/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Invigorating Bloom (`RAD-035/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Iridescent Avian (`RAD-027/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| K'Sante, Dauntless (`RAD-045/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Ki Burst (`RAD-028/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Life from Stone (`RAD-029/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Ntofo Strikes (`RAD-148/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Resplendent Rostrum (`RAD-046/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Riven, Broken Blade (`RAD-043/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Sanctum Conservator (`RAD-030/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Storm Raptor (`RAD-036/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Watchful Curator (`RAD-037/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Blade of the Exile**: [Equip] — :rb_rune_rainbow: or exhaust a friendly unit. (Pay the cost: Attach this to a unit you control.)
- **Consult the Heavens**: [Reaction] (Play any time, even before spells and abilities resolve.) Give a unit [Shield 2] this turn. (+2 [M] while it's a defender.) [Predict]. (Look at the top card of your Main Deck. You may recycle it.)
- **Ephemeral Ward**: Your units have +2 [M]. When you play a unit, kill this.
- **Heartening Soul**: When you play me, ready a friendly legend. Friendly legends have ":rb_exhaust:: Give a unit +3 [M] this turn."
- **Hunker Down**: [Reaction] (Play any time, even before spells and abilities resolve.) Ready a friendly unit. It can't move this turn. Draw 1.
- **Invigorating Bloom**: When you play this, give a friendly unit +3 [M] this turn. Exhaust a friendly [Mighty] unit, :rb_exhaust:: Draw 1. (A unit is Mighty while it has 5+ [M].)
- **Iridescent Avian**: :rb_exhaust:: [Buff] a unit. If it's already buffed, give it +2 [M] this turn instead. (To buff a unit, give it a +1 [M] buff if it doesn't have one.)
- **K'Sante, Dauntless**: [Shield] (+1 [M] while I'm a defender.) When I attack or defend, double the [Shield] of a unit you control here this turn.
- **Ki Burst**: [Action] (Play on your turn or in showdowns.) As an additional cost to play this, exhaust a friendly unit. [Stun] a unit. (It doesn't deal combat damage this turn.)
- **Life from Stone**: [Reaction] (Play any time, even before spells and abilities resolve.) As an additional cost to play this, exhaust a friendly unit. Draw 1 and channel 1 rune exhausted.
- **Ntofo Strikes**: [Action] (Play on your turn or in showdowns.) Give a friendly unit [Shield 2] this turn, then deal damage equal to its [Shield] to an enemy unit. (A unit with Shield has +1 [M] for each instance of Shield while it's a defender.)
- **Resplendent Rostrum**: :rb_exhaust:: Draw 1 and channel 1 rune exhausted. When I become exhausted, give a friendly unit +3 [M] this turn.
- **Riven, Broken Blade**: When I become exhausted, choose one that hasn't been chosen this turn —[Buff] me. (Give me a +1 [M] buff if I don't have one.)Spend my buff to [Stun] an enemy unit. (It doesn't deal combat damage this turn.)Spend my buff to ready me.
- **Sanctum Conservator**: When you play a card with Energy cost :rb_energy_7: or more, ready me.
- **Storm Raptor**: [Deflect] (Opponents must pay :rb_rune_rainbow: to choose me with a spell or ability.) When a stunned enemy unit dies here, draw 1.
- **Watchful Curator**: Your gear have [Deflect]. (Opponents must pay :rb_rune_rainbow: to choose each with a spell or ability.)

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
