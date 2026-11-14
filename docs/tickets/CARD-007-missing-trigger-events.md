# CARD-007: Missing trigger events

- **Priority:** P1
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** nothing

## Why these belong together
These cards wait on events the TriggerManager never dispatches: discard, recycle, "chosen by a spell", spending a buff, taking damage, playing a specific tag, and attacks against a legend's battlefield. Add the events (most fit the string-keyed `ObjectStateChangedEvent`), then wire each card.

14 cards, 3 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Gentle Gemdragon (`UNL-104/219`, id 666) | UNL | unit | partial-documented | 3 | friendly unit play (documented approximation). |
| Battle Mistress (`SFD-203/221`, id 517) | SFD | legend | partial-documented |  | gear token exhausted" clause has no engine trigger event and is not |
| Corrupt Enforcer (`SFD-123/221`, id 444) | SFD | unit | partial-documented |  | No chain context (e.g. direct test invocation): best-effort |
| Fae Dragon (`SFD-101/221`, id 423) | SFD | unit | partial-documented |  | ENGINE GAP: "When you spend a buff, play a Gold gear token exhausted." |
| Harpoon Squad (`SFD-137/221`, id 458) | SFD | unit | partial-documented |  | any move (close approximation — combat units predominantly move off BFs). |
| Jae Medarda (`SFD-142/221`, id 463) | SFD | unit | partial-suspect |  | no code for: decision |
| Karma, Channeler (`SFD-237/221`, id 548) | SFD | unit | partial-documented |  | ENGINE GAP: "When you recycle one or more cards to your Main Deck, |
| Lonely Poro (`UNL-221/219`, id 777) | UNL | unit | partial-documented |  | as alone (documented approximation) and draw. |
| Nine-Tailed Fox (`OGN-255/298`, id 252) | OGN | legend | not-implemented |  | rules text but no behaviour in the class |
| Noxian Guillotine (`OGN-254/298`, id 251) | OGN | spell | partial-documented |  | ENGINE GAP: there is no "when it takes damage" trigger/event, so this |
| Relentless Storm (`OGN-300/298`, id 295) | OGN | legend | partial-documented |  | one currently in play (best-effort identification of the play that |
| Scrapheap (`OGN-182/298`, id 182) | OGN | gear | partial-documented |  | cannot fire: the TriggerManager does not subscribe to CardsDiscardedEvent |
| Solari Shrine (`OGN-072/298`, id 72) | OGN | gear | partial-documented |  | satisfied (documented approximation, as with Piltover Enforcer/Gauntlets). |
| The Dreaming Tree (`OGN-292/298`, id 287) | OGN | battlefield | partial-documented |  | APPROX/INERT: no engine site currently emits "a spell chose a friendly unit |

## Rules text
- **Battle Mistress**: When you recycle a rune, you may exhaust me to play a Gold gear token exhausted. When one or more enemy units die, ready me.
- **Corrupt Enforcer**: When I move to a battlefield, discard 1. When I win a combat, draw 1.
- **Fae Dragon**: When you play me, buff up to four friendly units. (Give each a +1 [M] buff if it doesn't have one.) When you spend a buff, play a Gold gear token exhausted.
- **Gentle Gemdragon**: When you play me or another Dragon, ready up to 2 runes.
- **Harpoon Squad**: When I move from a battlefield, give me +2 [M] this turn.
- **Jae Medarda**: When you choose me with a spell, draw 1.
- **Karma, Channeler**: [Vision] (When you play me, look at the top card of your Main Deck. You may recycle it.) When you recycle one or more cards to your Main Deck, buff a friendly unit. (If it doesn't have a buff, it gets a +1 [M] buff. Runes aren't cards.)
- **Lonely Poro**: [Deathknell][>] If I died alone, draw 1.
- **Nine-Tailed Fox**: When an enemy unit attacks a battlefield you control, give it -1 [M] this turn, to a minimum of 1 [M].
- **Noxian Guillotine**: [Action] (Play on your turn or in showdowns.) Choose a unit. Kill it the next time it takes damage this turn. [Legion] — Kill it now instead. (Get the effect if you've played another card this turn.)
- **Relentless Storm**: When you play a [Mighty] unit, you may exhaust me to channel 1 rune exhausted. (A unit is Mighty while it has 5+ [M].)
- **Scrapheap**: When this is played, discarded, or killed, draw 1.
- **Solari Shrine**: When you kill a stunned enemy unit, you may exhaust this to draw 1.
- **The Dreaming Tree**: When a player chooses a friendly unit here with a spell for the first time each turn, they draw 1.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
