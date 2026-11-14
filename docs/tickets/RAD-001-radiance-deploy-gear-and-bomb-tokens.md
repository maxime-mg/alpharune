# RAD-001: Radiance: Deploy gear and Bomb tokens

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** nothing

## Why these belong together
Deploy is a new keyword on gear: it is played only to a battlefield and dies when an opponent holds there. Bomb tokens have Deploy. Build the keyword once, then the cards.

23 cards.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Amateur Demolitionist (`RAD-001/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Banner of Conquest (`RAD-002/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Blossoming Dreamroot (`RAD-024/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Bushwhack Trap (`RAD-108/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Command: Attack (`RAD-033/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Concert Poster (`RAD-116/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Demacian Standard (`RAD-156/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Detonation Amplifier (`RAD-017/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Dockside Lock-Up (`RAD-136/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Exemplar of Demacia (`RAD-155/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Gravity Field (`RAD-062/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Hexplosive Minefield (`RAD-142/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Hexplosives Expert (`RAD-141/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Jarvan IV, Martial Paragon (`RAD-091/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Overloaded Traveler (`RAD-072/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Pillaged Armory (`RAD-073/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Portable Portal (`RAD-087/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Resource Extractor (`RAD-052/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Satchel Charge (`RAD-096/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| The Ball (`RAD-146/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| The Good Stuff (`RAD-106/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Ziggs, Explosive (`RAD-112/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Ziggs, Short-Fused (`RAD-023/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Amateur Demolitionist**: When you play me, play a Bomb gear token to a battlefield. (It has [Deploy]. When it dies, deal 2 to an enemy there and kill your other Bombs there.)
- **Banner of Conquest**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) Friendly units here have [Assault]. (+1 [M] while they're attackers.)
- **Blossoming Dreamroot**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you win a combat here, draw 1.
- **Bushwhack Trap**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) While you or an ally control this battlefield, that player may hide an additional card here.
- **Command: Attack**: Choose one —Move a friendly gear with [Deploy] to a battlefield.Look at the top 5 cards of your Main Deck. You may reveal a gear from among them and draw it. Recycle the rest.
- **Concert Poster**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you play this or when you conquer here, you may pay :rb_energy_1: to play a 1 [M] Recruit unit token to your base.
- **Demacian Standard**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When one or more friendly units attack here, give one of them +2 [M] this turn. When you hold here, you may return this to its owner's hand.
- **Detonation Amplifier**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you play this, you may kill another friendly unit or gear to draw 1. Friendly [Deathknell] effects here trigger an additional time.
- **Dockside Lock-Up**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you play this, banish a unit here. When you hold here, kill this to play the banished card, ignoring its cost. (Tokens aren't cards.)
- **Exemplar of Demacia**: When you play a gear with [Deploy], you may exhaust me and pay :rb_rune_rainbow: to move a friendly unit to that battlefield.
- **Gravity Field**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) Opponents must pay :rb_energy_1::rb_rune_rainbow: to move one or more units to this battlefield. [Deathknell][>] Draw 1. (When I die, get the effect.)
- **Hexplosive Minefield**: [Action] (Play on your turn or in showdowns.) Kill any number of friendly Bomb tokens. At end of turn, play that many Bomb tokens to a single battlefield.
- **Hexplosives Expert**: :rb_energy_1::rb_rune_rainbow:, :rb_exhaust:: Play a Bomb gear token to a battlefield. (It has [Deploy]. When it dies, deal 2 to an enemy there and kill your other Bombs there.)
- **Jarvan IV, Martial Paragon**: Your gear with [Deploy] have [Action]. (They can be played on your turn or in showdowns.) When I conquer, if there is a friendly gear with [Deploy] here, channel 1 rune exhausted.
- **Overloaded Traveler**: While there's a friendly gear with [Deploy] at my battlefield, I have +1 [M].
- **Pillaged Armory**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you play this or when you conquer here, [Buff] a friendly unit. (Give it a +1 [M] buff if it doesn't have one.)
- **Portable Portal**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you play this, you may pay :rb_rune_body: to ready a unit here. Friendly units here have [Ganking]. (They can move from battlefield to battlefield.)
- **Resource Extractor**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) At the end of your turn, if you control this battlefield, play a Gold gear token exhausted. (It has "[Reaction][>] Kill this, :rb_exhaust:: [Add] :rb_rune_rainbow:.")
- **Satchel Charge**: [Action] (Play on your turn or in showdowns.) Move a friendly unit, then play a Bomb gear token to a battlefield. (It has [Deploy]. When it dies, deal 2 to an enemy there and kill your other Bombs there.)
- **The Ball**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) [Ganking] (While this is a unit, it can move from battlefield to battlefield.) When you play or move this, choose one —Draw 1.Deal 2 to enemy units here.
- **The Good Stuff**: [Deploy] (Play this only to a battlefield. When an opponent holds here, kill this.) When you play this, discard 1, then draw 3. When you hold here, return this to its owner's hand.
- **Ziggs, Explosive**: :rb_energy_1:, :rb_exhaust:: Play a Bomb gear token to a battlefield. (It has [Deploy]. When it dies, deal 2 to an enemy there and kill your other Bombs there.) When I defend, you may kill a friendly gear to give me +2 [M] this turn.
- **Ziggs, Short-Fused**: When you play me, play a Bomb gear token to a battlefield. (It has [Deploy]. When it dies, deal 2 to an enemy there and kill your other Bombs there.) Abilities of friendly gear deal 1 Bonus Damage.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
