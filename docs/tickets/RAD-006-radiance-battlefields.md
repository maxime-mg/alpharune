# RAD-006: Radiance: battlefields

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** nothing

## Why these belong together
The ten Radiance battlefields.

9 cards.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Bandle Scouts' Academy (`RAD-157/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Black Market (`RAD-158/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Cosmic Vista (`RAD-160/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Durand Memorial (`RAD-161/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Firelights' Hideout (`RAD-162/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Hunters' Circle (`RAD-163/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Packed Amphitheater (`RAD-164/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Rakelstake (`RAD-165/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |
| Ruined Monastery (`RAD-166/167`) | RAD | battlefield | not-implemented |  | no class in src/cards |

## Rules text
- **Bandle Scouts' Academy**: When you hide a card here, play a Gold gear token exhausted. (It has "[Reaction][>] Kill this, :rb_exhaust:: [Add] :rb_rune_rainbow:.")
- **Black Market**: :rb_rune_rainbow::rb_rune_rainbow:: Draw 1. Each player may use this ability once on their turn (even if they don't control this battlefield).
- **Cosmic Vista**: When you hold here, if your Rune Deck is empty, you score 1 point.
- **Durand Memorial**: When you defend here, choose a unit you control here. Give it [Tank] this combat. (It must be assigned combat damage first.)
- **Firelights' Hideout**: When you conquer here, the next unit you play this turn has [Accelerate]. (You may pay :rb_energy_1: and a Power of its Domain as an additional cost to have it enter ready.)
- **Hunters' Circle**: For each player, the first unit or gear they play here on their turn costs :rb_energy_1: less.
- **Packed Amphitheater**: Champion units here have +1 [M].
- **Rakelstake**: For each player, the first time they play a unit or gear here each turn, they [Predict]. (They look at the top card of their Main Deck. They may recycle it.)
- **Ruined Monastery**: When you conquer here, you may discard 1 and channel 1 rune exhausted.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
