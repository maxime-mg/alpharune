# RAD-007: Radiance: Fury cards

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md)
- **Blocks:** nothing

## Why these belong together
The remaining Radiance Fury units, spells and gear. Cards in one domain share themes and helpers, so they are written together.

15 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Akali, Brash (`RAD-015/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Chronobreak (`RAD-140/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Clocktower Guardian (`RAD-067/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Demon Shade (`RAD-102/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Drakehound Packmaster (`RAD-018/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Ekko, Revolutionary (`RAD-021/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Elnuk Herd (`RAD-081/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Firebrand Raider (`RAD-019/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Lingering Animus (`RAD-005/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Lost to the Sands (`RAD-013/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Mechanical Bandit (`RAD-111/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Mordekaiser, Twice Slain (`RAD-020/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Realm of Death (`RAD-144/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Shimmering Swimmer (`RAD-014/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| War Scout (`RAD-008/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Akali, Brash**: The first time I move each turn, [Add] :rb_energy_1::rb_rune_rainbow:. The second time I move each turn, banish the top card of your Main Deck. Until your next Ending Phase, you may play that card. (You still pay its costs.)
- **Chronobreak**: As an additional cost to play this, pay any amount of Power. Banish that many units you control, then for each, its owner plays that unit back to the same location or to their base, ignoring its cost. (You choose the order.)
- **Clocktower Guardian**: When you play me or when I hold, [Predict 3]. [Deathknell][>] Banish the top card of your Main Deck. You may play it, ignoring its Energy cost.
- **Demon Shade**: [Hidden] (Hide now for :rb_rune_rainbow: to react with later for :rb_energy_0:.) Choose a friendly unit, then choose one —Move it to an open battlefield.Give it [Ganking] this turn if there are no open battlefields. (It can move from battlefield to battlefield.)
- **Drakehound Packmaster**: When you play me, reveal the top card of each opponent's Main Deck. Choose one and banish it, then play it, ignoring its cost. Recycle the rest.
- **Ekko, Revolutionary**: I enter ready. [Assault] (+1 [M] while I'm an attacker.) [Action][>] Discard 1, :rb_energy_1::rb_rune_fury:: Banish me, then play me to your base, ignoring my cost.
- **Elnuk Herd**: [Hidden] (Hide now for :rb_rune_rainbow: to react with later for :rb_energy_0:.) When you play me, give another friendly unit +3 [M] this turn, then recycle me if I was played from face down.
- **Firebrand Raider**: I enter ready. When you play me, ready all enemy units. Other units enter ready. Units don't ready during the Awaken Phase.
- **Lingering Animus**: I enter ready if a friendly unit or gear died this turn.
- **Lost to the Sands**: [Action] (Play on your turn or in showdowns.) Choose one —Deal 4 to a unit at a battlefield.Kill a gear.
- **Mechanical Bandit**: At the start of your Beginning Phase, each player reveals the top card of their Main Deck. If you revealed a card with the lowest Energy cost, draw it. Recycle the rest.
- **Mordekaiser, Twice Slain**: [Accelerate] (You may pay :rb_energy_1::rb_rune_fury: as an additional cost to have me enter ready.) [Assault 3] (+3 [M] while I'm an attacker.) Once each turn, if I would die, you may kill a friendly unit to heal me, exhaust me, and recall me instead.
- **Realm of Death**: Give an enemy unit -2 [M] this turn and a friendly unit +2 [M] this turn. They deal damage equal to their Mights to each other.
- **Shimmering Swimmer**: You may kill a friendly unit or gear as an additional cost to play me. When you play me, if you paid the additional cost, deal 2 to a unit.
- **War Scout**: [Accelerate] (You may pay :rb_energy_1::rb_rune_fury: as an additional cost to have me enter ready.) When you play me, give a unit +2 [M] this turn.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
