# CARD-001: Fix confirmed bugs in implemented cards

- **Priority:** P0
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** nothing

## Why these belong together
These cards run but do the wrong thing, and the audit confirmed it by reading the code. Most are small, local fixes.

9 cards, 14 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Ashe, Focused (`UNL-169/219`, id 731) | UNL | unit | partial-confirmed | 10 | 'When they hold, return it to their hand' is missing |
| Eager Apprentice (`OGN-084/298`, id 84) | OGN | unit | partial-confirmed | 2 | Discount has no spell-only condition, so units and gear get it too |
| Eclipse (`UNL-063/219`, id 625) | UNL | spell | partial-confirmed | 2 | [Predict] is only a keyword bit; nothing performs it |
| Apprentice Smith (`SFD-041/221`, id 364) | SFD | unit | partial-confirmed |  | Moves the gear to hand directly, so draw triggers do not fire |
| Baited Hook (`OGN-242/298`, id 242) | OGN | gear | partial-confirmed |  | [C] power is not charged, and the cost recycles the gear, which is not printed |
| Ekko, Recurrent (`OGN-110/298`, id 110) | OGN | unit | partial-confirmed |  | Readies your runes but never recycles itself |
| Forge of the Future (`OGN-212/298`, id 212) | OGN | gear | partial-confirmed |  | Play trigger kills targets[0] instead of making a Recruit; 'Kill this: recycle up to 4' is absent |
| Keeper of the Hammer (`UNL-203/219`, id 759) | UNL | legend | partial-confirmed |  | Gains XP on conquer as well as hold |
| Scorchclaw (`UNL-016/219`, id 578) | UNL | unit | partial-confirmed |  | Hunt XP is gated behind xp>=3, so it never starts; Level 3 +1 [M] and enter-ready are missing |

## Rules text
- **Apprentice Smith**: When I move, reveal the top card of your Main Deck. If it's a gear, draw it. Otherwise, recycle it.
- **Ashe, Focused**: When you play me, choose an opponent. They reveal their hand. Choose a card revealed this way and banish it. When they hold, return it to their hand (even if I'm no longer on the board).
- **Baited Hook**: [1][C], [E]: Kill a friendly unit. Look at the top 5 cards of your Main Deck. You may banish a unit from among them that has Might up to 1 more than the killed unit and play it, ignoring its cost. Then recycle the rest.
- **Eager Apprentice**: While I'm at a battlefield, the Energy costs for spells you play is reduced by [1], to a minimum of [1].
- **Eclipse**: [Reaction] (Play any time, even before spells and abilities resolve.) Give a unit -4 [M] this turn. [Predict]. (Look at the top card of your Main Deck. You may recycle it.)
- **Ekko, Recurrent**: [Accelerate] (You may pay [1][B] as an additional cost to have me enter ready.) [Deathknell] — Recycle me to ready your runes. (When I die, get the effect.)
- **Forge of the Future**: When you play this, play a 1 [M] Recruit unit token at your base. Kill this: Recycle up to 4 cards from trashes.
- **Keeper of the Hammer**: When you hold, gain 1 XP. Spend 3 XP, [E]: Draw 1.
- **Scorchclaw**: [Hunt 2] (When I conquer or hold, gain 2 XP.) [Level 3][>] I have +1 [M] and enter ready. (While you have 3+ XP, get the effect.)

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
