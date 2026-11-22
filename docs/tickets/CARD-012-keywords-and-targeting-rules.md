# CARD-012: Keywords and targeting rules

- **Priority:** P2
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 5 of 5 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. The text below describes the old code.

## Why these belong together
Vision granted to other units, Predict on spells, resources that can only be spent on spells, making new choices for a stolen spell, "can't be chosen by enemy spells", and the open-battlefield play grant.

5 cards, 1 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| Lux, Crownguard (`OGS-014/024`, id 312) | OGS | unit | fixed upstream | 1 | engine gap; over-permissive but never under-delivers). |
| Alpha Wildclaw (`UNL-057/219`, id 619) | UNL | unit | fixed upstream |  | rules text but no behaviour in the class |
| Gemcraft Seer (`OGN-100/298`, id 100) | OGN | unit | fixed upstream |  | (documented engine gap shared with Forecaster); the keyword presence is |
| Miss Fortune, Buccaneer (`OGN-193/298`, id 193) | OGN | unit | fixed upstream |  | no code for: decision |
| Mystic Reversal (`OGN-080/298`, id 80) | OGN | spell | fixed upstream |  | (documented engine gap). |

## Rules text
- **Alpha Wildclaw**: [Tank] (I must be assigned combat damage first.) Your units here with less Might than me can't be chosen by enemy spells and abilities.
- **Gemcraft Seer**: [Vision] (When you play me, look at the top card of your Main Deck. You may recycle it.) Other friendly units have [Vision].
- **Lux, Crownguard**: [E]: [Reaction] — [Add] [2]. Use only to play spells. (Abilities that add resources can't be reacted to.)
- **Miss Fortune, Buccaneer**: You may play me to an open battlefield. Friendly units may be played to open battlefields.
- **Mystic Reversal**: [Reaction] (Play any time, even before spells and abilities resolve.) Gain control of a spell. You may make new choices for it.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
