# CARD-005: Playing cards from trash, deck or banishment with partial costs

- **Priority:** P1
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 9 of 9 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. The text below describes the old code.

## Why these belong together
Most of these approximate "you must still pay its Power cost" as free, or play the card without the right cost. Needs a play-from-zone path that charges the remaining cost through the normal payment cursor.

9 cards, 3 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| Nocturne, Horrifying (`OGN-194/298`, id 194) | OGN | unit | fixed upstream | 3 | helpers run once per resolution). The [A] cost is approximated as |
| Cursed Sarcophagus (`UNL-148/219`, id 710) | UNL | gear | fixed upstream |  | cost payment for the replayed unit is approximated as free (playIgnoringCost) |
| Here to Help (`SFD-111/221`, id 433) | SFD | spell | fixed upstream |  | ALL cost). The chosen unit is played for free — documented approximation of |
| Kai'Sa, Evolutionary (`OGN-112/298`, id 112) | OGN | unit | fixed upstream |  | "you must still pay its Power cost" is approximated as free — same |
| Reinforce (`OGN-062/298`, id 62) | OGN | spell | fixed upstream |  | (documented approximation of "-[5]"; most reachable units cost <= 5). |
| Soulgorger (`OGN-196/298`, id 196) | OGN | unit | fixed upstream |  | Energy cost." (Power cost approximated as free — same convention as |
| Super Mega Death Rocket! (`OGN-252/298`, id 250) | OGN | spell | fixed upstream |  | ENGINE GAP: "When you conquer, you may discard 1 to return this from your |
| The Harrowing (`OGN-198/298`, id 198) | OGN | spell | fixed upstream |  | approximated as free.) Target lives in trash — defer to resolve-time pick. |
| Undying Legion (`UNL-025/219`, id 587) | UNL | unit | fixed upstream |  | no code for: decision |

## Rules text
- **Cursed Sarcophagus**: When you play this, banish all units from your trash. [E]: Play a unit banished with this. (You must pay its costs.)
- **Here to Help**: [Hidden] (Hide now for [A] to react with later for .) [Action] (Play on your turn or in showdowns.) You may play a unit from hand to a battlefield you control, reducing its cost by [3].
- **Kai'Sa, Evolutionary**: [Ganking] (I can move from battlefield to battlefield.) When I conquer, you may play a spell from your trash with Energy cost less than your points without paying its Energy cost. Then recycle it. (You must still pay its Power cost.)
- **Nocturne, Horrifying**: [Ganking] (I can move from battlefield to battlefield.) As you look at or reveal me from the top of your deck, you may banish me. If you do, you may play me for [A].
- **Reinforce**: Look at the top 5 cards of your Main Deck. You may banish a unit from among them, then play it, reducing its cost by [5]. Recycle the remaining cards.
- **Soulgorger**: When you play me, you may play a unit from your trash, ignoring its Energy cost. (You must still pay its Power cost.)
- **Super Mega Death Rocket!**: Deal 5 to a unit. When you conquer, you may discard 1 to return this from your trash to your hand.
- **The Harrowing**: Play a unit from your trash, ignoring its Energy cost. (You must still pay its Power cost.)
- **Undying Legion**: [Legion][>] You may play me from your trash for [3][R]. (Get the effect if you've played another card this turn.)

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
