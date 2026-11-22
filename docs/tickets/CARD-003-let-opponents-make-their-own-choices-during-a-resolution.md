# CARD-003: Let opponents make their own choices during a resolution

- **Priority:** P1
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 6 of 6 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. The text below describes the old code.

## Why these belong together
These cards ask another player to choose mid-resolution, and the code approximates the opponent's "rational pick" instead of asking them. Needs one engine path for asking a non-controller through `requestChoice`, then each card uses it.

6 cards, 2 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| King's Edict (`OGN-237/298`, id 237) | OGN | spell | fixed upstream | 2 | so we approximate their rational pick (lowest-Might unit they control). |
| Call to Battle (`UNL-101/219`, id 663) | UNL | spell | fixed upstream |  | agent; approximate their rational pick (highest-Might unit not |
| Divine Judgment (`OGN-244/298`, id 244) | OGN | spell | fixed upstream |  | The "which 2 to keep" is the player's choice; approximated here by |
| Promising Future (`OGN-115/298`, id 115) | OGN | spell | fixed upstream |  | (Power cost approximated as free.) |
| Safety Inspector (`UNL-164/219`, id 726) | UNL | unit | fixed upstream |  | OPPONENT's forced kill is approximated as their rational pick — lowest-Might |
| Shard of Undoing (`UNL-174/219`, id 736) | UNL | gear | fixed upstream |  | through this controller's agent, so approximate the rational pick |

## Rules text
- **Call to Battle**: Move a unit you control to a battlefield you control. Then, choose an opponent. They move a unit they control to the same battlefield.
- **Divine Judgment**: Each player chooses 2 units, 2 gear, 2 runes, and 2 cards in their hands. Recycle the rest.
- **King's Edict**: Starting with the next player, each other player chooses a unit you don't control that hasn't been chosen for this spell. Kill those units.
- **Promising Future**: Each player looks at the top 5 cards of their Main Deck, banishes one of them, then recycles the rest. Starting with the next player, each player plays those cards, ignoring Energy costs. (They must still pay Power costs.)
- **Safety Inspector**: You may spend 3 XP as an additional cost to play me. When you play me, each player must kill one of their units. If you paid my additional cost, you don't kill a unit this way.
- **Shard of Undoing**: The first time a friendly unit dies during your Beginning Phase each turn, each opponent must kill one of their units.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
