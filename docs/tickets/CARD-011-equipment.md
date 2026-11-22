# CARD-011: Equipment

- **Priority:** P2
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 5 of 5 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. The text below describes the old code.

## Why these belong together
Equip costs that are not plain energy, bonus damage while attached, and moving with attachment.

5 cards, 11 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| Last Rites (`SFD-150/221`, id 471) | SFD | gear | fixed upstream | 6 | trigger, so we play it for free for now (documented engine gap). |
| Relentless Pursuit (`SFD-184/221`, id 503) | SFD | spell | fixed upstream | 3 | "Move a friendly unit." Best-effort: move to a battlefield where the |
| Experimental Hexplate (`SFD-073/221`, id 396) | SFD | gear | fixed upstream | 2 | rules text but no behaviour in the class |
| Rabadon's Deathcrown (`SFD-191/221`, id 508) | SFD | gear | fixed upstream |  | Attached 'deal 3 Bonus Damage' is never applied: only Annie sets bonus_damage_dealt |
| The Zero Drive (`SFD-090/221`, id 412) | SFD | gear | fixed upstream |  | activation cost path (engine limitation); banish-this is performed |

## Rules text
- **Experimental Hexplate**: [Equip] [B] ([B]: Attach this to a unit you control.) I am a Mech.
- **Last Rites**: [Equip] — [P], Recycle 2 cards from your trash (Pay the cost: Attach this to a unit you control.) When I conquer or hold, you may play a unit from your trash. (You still pay its costs.)
- **Rabadon's Deathcrown**: [Unique] (Your deck can have only 1 card with this name.) [Equip] [A] ([A]: Attach this to a unit you control.) Your spells and abilities deal 3 Bonus Damage (while this is attached).
- **Relentless Pursuit**: [Action] (Play on your turn or in showdowns.) Move a friendly unit. You may attach up to one Equipment with the same controller to it. This turn, that unit has "When I conquer, you may move me to my base."
- **The Zero Drive**: [Equip] [1][B] ([1][B]: Attach this to a unit you control.) [3][B], Banish this: Play all units banished with this, ignoring their costs. (Use only if unattached.) [Deathknell] — Banish me. (When I die, get the effect.)

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
