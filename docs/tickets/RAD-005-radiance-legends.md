# RAD-005: Radiance: legends

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** [RAD-000](RAD-000-refresh-radiance-card-data-and-scaffold-the-classes.md)
- **Blocks:** nothing

## Why these belong together
The eight Radiance legends, plus Neeko, Blending In, who adds extra legend choices at deck building.

7 cards.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Agony's Embrace (`RAD-153/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Boy Who Shattered Time (`RAD-139/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Iron Revenant (`RAD-143/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Lady of Clockwork (`RAD-145/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Neeko, Blending In (`RAD-167/167`) | RAD | unit | not-implemented |  | no class in src/cards |
| Pride of Nazumah (`RAD-147/167`) | RAD | legend | not-implemented |  | no class in src/cards |
| Starry-Eyed Songstress (`RAD-151/167`) | RAD | legend | not-implemented |  | no class in src/cards |

## Rules text
- **Agony's Embrace**: Once each turn, when you play a card from face down, draw 1. Then, if it's not your turn, discard 1.
- **Boy Who Shattered Time**: [Empower] — :rb_energy_1:, :rb_exhaust: (Use only if not Empowered.) Disempower me, :rb_exhaust:: Reduce the cost of the next unit you play this turn by :rb_energy_2::rb_rune_rainbow: and give it "Kill me at end of turn."
- **Iron Revenant**: [Reaction][>] Banish a unit or gear from your trash, :rb_exhaust:: [Add] :rb_rune_rainbow:. Spend this Power only to play units or gear. (Abilities that add resources can't be reacted to.)
- **Lady of Clockwork**: At the start of your Main Phase, you may pay :rb_energy_1: to choose a friendly gear. It becomes a unit gear with Might equal to its Energy cost until you choose another gear with me.
- **Neeko, Blending In**: (Neutral cards can go in decks of any Domain.) If Neeko is in your deck, choose 3 different legends in addition to your starting legend. (Different means none match names.) When you play me, choose one of those legends and add it to your Legend Zone while I'm on the board.
- **Pride of Nazumah**: :rb_energy_1:, :rb_exhaust:: Give a friendly unit "I have [Assault] equal to my [Shield]" this turn. (+1 [M] for each instance of Assault while it's an attacker.)
- **Starry-Eyed Songstress**: As an additional cost to play a spell, you may exhaust me and three friendly units to reduce the cost of that spell by :rb_energy_2:.

## Done when
- Each card behaves as its text says, with a test under `tests/cards/`.
