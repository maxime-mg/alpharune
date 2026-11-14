# RAD-010: Radiance: Body cards

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md)
- **Blocks:** nothing

## Why these belong together
The remaining Radiance Body units, spells and gear. Cards in one domain share themes and helpers, so they are written together.

16 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Avalanche (`RAD-079/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Bilgewater Tracker (`RAD-003/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Cataclysm (`RAD-084/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Evelynn, Consuming (`RAD-090/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Graves, Blasting Through (`RAD-085/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Hold 'Em (`RAD-070/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| K'Sante, Courageous (`RAD-086/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Last Caress (`RAD-154/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Raving Witness (`RAD-074/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Skystrike Squadron (`RAD-075/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Sneaky Zeebles (`RAD-076/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Stalwart Goatherd (`RAD-083/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Staunch Survivor (`RAD-088/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Titanic Shrine (`RAD-089/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Wrath of the Freljord (`RAD-077/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Yone, Blademaster (`RAD-181/167`, id 544) | RAD | unit | partial-documented |  | ENGINE GAP: printed condition is "When I conquer a battlefield THAT |

## Rules text
- **Avalanche**: Deal 3 to all units in bases.
- **Bilgewater Tracker**: When you play me, deal 1 to each of up to 2 units.
- **Cataclysm**: Opponents can't play spells this turn. Move a friendly unit to an occupied enemy battlefield.
- **Evelynn, Consuming**: [Ambush] (You may play me as a [Reaction] to a battlefield where you have units.) When you play a card from face down, ready me and give me +1 [M] this turn.
- **Graves, Blasting Through**: When I attack or defend, deal damage equal to my Might to all enemy units here. I don't deal combat damage.
- **Hold 'Em**: [Action] (Play on your turn or in showdowns.) This costs :rb_energy_2: less if you've played a unit or gear to a battlefield this turn. Give a unit +3 [M] this turn.
- **K'Sante, Courageous**: [Accelerate] (You may pay :rb_energy_1::rb_rune_body: as an additional cost to have me enter ready.) [Shield 2] (+2 [M] while I'm a defender.) When I hold, channel 1 rune.
- **Last Caress**: [Reaction] (Play any time, even before spells and abilities resolve.) The next unit you play this turn (even from face down) can be played to any battlefield. When you play it, [Buff] it. (Give it a +1 [M] buff if it doesn't have one.)
- **Raving Witness**: When I move, reveal the top card of your Main Deck. If it's [Mighty], draw it. Otherwise, recycle it. (A unit is Mighty while it has 5+ [M].)
- **Skystrike Squadron**: You may return a friendly gear to its owner's hand as an additional cost to play me. If you do, I cost :rb_energy_2: less. [Deflect] (Opponents must pay :rb_rune_rainbow: to choose me with a spell or ability.)
- **Sneaky Zeebles**: When I conquer, [Add] :rb_rune_rainbow:. (Abilities that add resources can't be reacted to.)
- **Stalwart Goatherd**: [Shield 2] (+2 [M] while I'm a defender.) When you play me, you may kill a gear with Energy cost :rb_energy_4: or less.
- **Staunch Survivor**: [Assault 2], [Shield 2] (+2 [M] while I'm an attacker or defender.)
- **Titanic Shrine**: When you play a unit with Might greater than each other unit on the board, exhaust me to draw 1.
- **Wrath of the Freljord**: I enter ready. [Shield 3] (+3 [M] while I'm a defender.)
- **Yone, Blademaster**: [Weaponmaster] (When you play me, you may [Equip] one of your Equipment to me for [A] less, even if it's already attached.) When I conquer a battlefield that was uncontrolled, deal damage equal to my Might to an enemy unit in a base.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
