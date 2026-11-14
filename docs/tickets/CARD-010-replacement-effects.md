# CARD-010: Replacement effects

- **Priority:** P2
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** nothing

## Why these belong together
"Instead" and "the next time it would die" effects. Replacements still run on raw `ability_text` matching in `killUnit()`. Needs the structured `ReplacementEffect` type the engine notes call for.

5 cards, 0 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Endless Riches (`VEN-022/166`, id 809) | VEN | gear | partial-suspect |  | no code for: decision |
| Smite (`UNL-007/219`, id 568) | UNL | spell | partial-documented |  | no engine hook; we apply the banish on the immediate lethal damage |
| The Boss (`OGN-269/298`, id 264) | OGN | legend | partial-suspect |  | no code for: decision |
| Unlicensed Armory (`OGN-023/298`, id 23) | OGN | gear | partial-documented |  | by the flag — the recall happens automatically (documented approximation). |
| Zilean, Time Mage (`UNL-086/219`, id 648) | UNL | unit | partial-documented |  | turn). Approximated as automatic (token creation is not an agent decision |

## Rules text
- **Endless Riches**: When you play this, banish your hand and trash, then [Burn 7]. Skip your Draw Phase. You may play cards from your trash. If a card would go to your trash from anywhere other than your Main Deck, banish it instead.
- **Smite**: [Action] (Play on your turn or in showdowns.) Deal 3 to a unit at a battlefield. If it would die this turn, banish it instead.
- **The Boss**: If a buffed unit you control would die, you may pay [C], exhaust me, and spend its buff to heal it, exhaust it, and recall it instead. (Send it to base. This isn't a move.) When you conquer, ready me.
- **Unlicensed Armory**: Discard 1, [E]: Choose a friendly unit. The next time it would die this turn, you may pay [C] to heal it, exhaust it, and recall it instead. (Send it to base. This isn't a move.)
- **Zilean, Time Mage**: Once each turn, if you would play a token unit while I'm at a battlefield, you may play that token and an additional copy of it instead.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
