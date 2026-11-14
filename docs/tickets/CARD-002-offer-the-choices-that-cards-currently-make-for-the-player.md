# CARD-002: Offer the choices that cards currently make for the player

- **Priority:** P0
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** nothing

## Why these belong together
Each of these cards says "you may" or "choose", but the code takes the first option. The `confirmOptional`, `pickTarget` and `pickMode` helpers already exist, so these are mostly mechanical.

12 cards, 19 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Elder Dragon (`UNL-118/219`, id 680) | UNL | unit | partial-confirmed | 7 | Hits the first enemy at each location; 'choose up to one' offers no choice |
| Grim Apothecary (`UNL-021/219`, id 583) | UNL | unit | partial-confirmed | 7 | Auto-picks the first friendly unit at a battlefield; the target is not offered |
| Disposal Order (`UNL-103/219`, id 665) | UNL | spell | partial-documented | 3 | documented approximation: take the most recent up to 3). |
| Ornn, Blacksmith (`SFD-058/221`, id 381) | SFD | unit | partial-confirmed | 1 | Takes the first gear with no choice ('you may reveal' is not offered) |
| The Candlelit Sanctum (`OGN-291/298`, id 286) | OGN | battlefield | partial-suspect | 1 | no code for: decision |
| Azir, Sovereign (`SFD-177/221`, id 497) | SFD | unit | partial-confirmed |  | Moves every token; 'you may move any number' offers no choice |
| Buhru Captain (`SFD-091/221`, id 413) | SFD | unit | partial-confirmed |  | Always draws; the 'or buff me' choice is missing |
| Double Trouble (`UNL-032/219`, id 594) | UNL | spell | partial-confirmed |  | Takes the first unit with no choice ('you may reveal' is not offered) |
| Hallowed Tomb (`OGN-281/298`, id 276) | OGN | battlefield | partial-confirmed |  | Forced, not 'you may', and takes any champion rather than your Chosen Champion |
| On the Hunt (`SFD-204/221`, id 518) | SFD | spell | partial-confirmed |  | Readies one chosen unit; the card readies all your units |
| Rift Herald (`UNL-179/219`, id 741) | UNL | unit | partial-documented |  | Agent-choice of WHICH unit approximated: draw the first revealed unit. |
| Void Rush (`SFD-188/221`, id 505) | SFD | spell | partial-confirmed |  | Takes the first playable card; 'you may banish one' offers no choice |

## Rules text
- **Azir, Sovereign**: [Accelerate] (You may pay [1][Y] as an additional cost to have me enter ready.) When I attack, you may move any number of your token units to this battlefield.
- **Buhru Captain**: When you play me, you may draw 1 or buff me. (To buff a unit, give it a +1 [M] buff if it doesn't already have one.)
- **Disposal Order**: [Reaction] (Play any time, even before spells and abilities resolve.) Choose one — Choose up to 3 cards from opponents' trashes. Their owners recycle them.Draw 1.
- **Double Trouble**: [Repeat] [2] (You may pay the additional cost to repeat this spell's effect.) Look at the top 3 cards of your Main Deck. You may reveal a unit from among them and draw it. Recycle the rest.
- **Elder Dragon**: Any amount of your damage is enough to kill enemy units. When you play me, choose up to one enemy unit at each location. Deal 1 to them.
- **Grim Apothecary**: [Ambush] (You may play me as a [Reaction] to a battlefield where you have units.) When you play me, you may return a friendly unit at a battlefield to its owner's hand.
- **Hallowed Tomb**: When you hold here, you may return your Chosen Champion from your trash to your Champion Zone if it is empty.
- **On the Hunt**: Ready your units.
- **Ornn, Blacksmith**: When you play me or when I hold, look at the top 4 cards of your Main Deck. You may reveal a gear from among them and draw it. Then recycle the rest.
- **Rift Herald**: When I move to a battlefield, look at the top 3 cards of your Main Deck. You may reveal a unit from among them and draw it. Recycle the rest. [Deathknell][>] Play a unit from your hand to your base, ignoring its Energy cost. (When I die, get the effect. You must still pay its Power cost.)
- **The Candlelit Sanctum**: When you conquer here, look at the top two cards of your Main Deck. You may recycle one or both of them. Put those you don't back in any order.
- **Void Rush**: Reveal the top 2 cards of your Main Deck. You may banish one, then play it, reducing its cost by [2]. Draw any you didn't banish.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
