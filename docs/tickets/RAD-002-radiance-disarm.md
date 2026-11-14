# RAD-002: Radiance: Disarm

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** nothing

## Why these belong together
Disarm N is an attack trigger that gives an enemy unit -N Might and stacks. Build the keyword, then the cards.

7 cards.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Ahri, Confident (`RAD-038/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Clanging Sentry (`RAD-055/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Disposal Expert (`RAD-093/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Evelynn, In Control (`RAD-109/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Kai'Sa, Rebel (`RAD-063/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Mysterious Chronomancer (`RAD-050/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Siren of the Shallows (`RAD-097/167`) | RAD | unit | not-implemented |  | no class in src/cards |

## Rules text
- **Ahri, Confident**: [Disarm] (When I attack, give an enemy unit here -1 [M] this turn.) When you reduce the Might of an enemy unit here by 1 or more, give me +1 [M] this turn.
- **Clanging Sentry**: Your Mechs have [Disarm]. (When each of us attacks, give an enemy unit there -1 [M] this turn.)
- **Disposal Expert**: I enter ready if you control no other units. [Disarm 2] (When I attack, give an enemy unit here -2 [M] this turn.)
- **Evelynn, In Control**: [Disarm] (When I attack, give an enemy unit here -1 [M] this turn.) When I hold, if you control a facedown card here, score 1 point.
- **Kai'Sa, Rebel**: [Disarm] (When I attack, give an enemy unit here -1 [M] this turn). While I'm in a showdown, your spells have [Reaction]. (Play them any time, even before spells and abilities resolve.)
- **Mysterious Chronomancer**: When you play me, [Predict 2]. (Look at the top 2 cards of your Main Deck. Recycle any of them and put the rest back in any order.) [Disarm 2] (When I attack, give an enemy unit here -2 [M] this turn.)
- **Siren of the Shallows**: [Disarm] (When I attack, give an enemy unit here -1 [M] this turn.) [Ganking] (I can move from battlefield to battlefield.)

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
