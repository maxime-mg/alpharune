# CARD-009: Hidden cards

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** nothing

## Why these belong together
Rules about hiding: extra hide slots, alternate hide costs, "can't be revealed here", hiding ignoring costs, and a card with a deck-construction exception.

5 cards, 3 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Guerilla Warfare (`OGN-264/298`, id 260) | OGN | spell | partial-documented | 2 | TODO: "You can hide cards ignoring costs this turn." — the engine |
| Swift Scout (`OGN-263/298`, id 259) | OGN | legend | partial-documented | 1 | ENGINE GAP: hide-cost is an alternative-cost modifier with no card-side |
| Bandle Tree (`OGN-278/298`, id 273) | OGN | battlefield | partial-suspect |  | no code for: decision |
| Noxus Saboteur (`OGN-018/298`, id 18) | OGN | unit | partial-documented |  | currently INERT — it is the faithful state representation, ready for the |
| Spiderling (`VEN-097/166`, id 884) | VEN | unit | not-implemented |  | rules text but no behaviour in the class |

## Rules text
- **Bandle Tree**: You may hide an additional card here.
- **Guerilla Warfare**: Return up to two cards with [Hidden] from your trash to your hand. You can hide cards ignoring costs this turn.
- **Noxus Saboteur**: Your opponents' [Hidden] cards can't be revealed here.
- **Spiderling**: [Hidden] (Hide now for [A] to react with later for [0].) I have +1 [M] for each other unit you control here with my name. Your deck can have any number of cards named Spiderling.
- **Swift Scout**: You may pay [1] to hide a card with [Hidden] instead of [A]. [1], [E]: Put a Teemo unit you own into your hand from your Champion Zone or the board.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
