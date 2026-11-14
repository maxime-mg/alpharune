# RAD-012: Radiance: Order cards

- **Priority:** P3
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md)
- **Blocks:** nothing

## Why these belong together
The remaining Radiance Order units, spells and gear. Cards in one domain share themes and helpers, so they are written together.

15 cards. Multi-domain cards are filed under their first domain.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Battlefield Armorer (`RAD-130/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Demacian Illuminators (`RAD-131/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Dune Devourer (`RAD-125/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| For Demacia! (`RAD-117/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Herald of the Emperor (`RAD-118/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Hunting Dog (`RAD-119/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Jarvan IV, Royal Heir (`RAD-133/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Meticulous Restorationist (`RAD-120/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Mordekaiser, Dominating (`RAD-137/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Mournful Spirit (`RAD-126/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Nazuman Ntofo (`RAD-071/167`) | RAD | gear | not-implemented |  | no class in src/cards |
| Party Crasher (`RAD-122/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Steadfast Outrider (`RAD-128/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Supernova (`RAD-059/167`) | RAD | spell | not-implemented |  | no class in src/cards |
| Trifarian Captain (`RAD-129/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Battlefield Armorer**: When a friendly gear at a battlefield dies, you may pay :rb_energy_1: to return it to its owner's hand.
- **Demacian Illuminators**: When one or more friendly units die, [Buff] a friendly unit. (Give it a +1 [M] buff if it doesn't have one.)
- **Dune Devourer**: You may kill a friendly unit as an additional cost to play me. If you do, I cost :rb_energy_2: less. [Deflect] (Opponents must pay :rb_rune_rainbow: to choose me with a spell or ability.)
- **For Demacia!**: [Reaction] (Play any time, even before spells and abilities resolve.) Choose a battlefield. Give friendly units there [Shield 2] this turn. (+2 [M] while they're defenders.)
- **Herald of the Emperor**: When you play me, choose a friendly unit. Its base Might becomes 5 this turn.
- **Hunting Dog**: [Deathknell][>] Play a 1 [M] Bird unit token with [Deflect] to your base. (When I die, get the effect. Opponents must pay :rb_rune_rainbow: to choose a [Deflect] unit with a spell or ability.)
- **Jarvan IV, Royal Heir**: [Assault] (+1 [M] while I'm attacking.) [Tank] (I must be assigned combat damage first.) When I attack, you may move a unit you control to my battlefield.
- **Meticulous Restorationist**: When you play a card with Energy cost :rb_energy_7: or more, you may play me from your trash for :rb_rune_order:.
- **Mordekaiser, Dominating**: When you play me or when I attack, you may discard 1 to kill a unit or gear.
- **Mournful Spirit**: [Deathknell][>] You may kill a unit or gear here. (When I die, get the effect.)
- **Nazuman Ntofo**: Equip :rb_energy_1: (:rb_energy_1:: Attach this to a unit you control.)
- **Party Crasher**: I cost :rb_energy_1: less for each exhausted unit you control. [Tank] (I must be assigned combat damage first.)
- **Steadfast Outrider**: [Tank] (I must be assigned combat damage first.) While there are 2 or more other friendly units and/or gear at my battlefield, I have +2 [M].
- **Supernova**: Deal 10 to a unit. Draw 2.
- **Trifarian Captain**: When you play me, look at the top 4 cards of your Main Deck. You may reveal from among them a unit or gear with Energy cost no more than :rb_energy_3: and with Power cost no more than :rb_rune_rainbow: and draw it. Recycle the rest.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
