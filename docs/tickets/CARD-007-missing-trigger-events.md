# CARD-007: Missing trigger events

- **Priority:** P1
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 13 of 14 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. The Dreaming Tree was fixed later, in [CARD-013](CARD-013-gaps-left-after-upstreams-card-fixes.md). The text below describes the old code.

## Why these belong together
These cards wait on events the TriggerManager never dispatches: discard, recycle, "chosen by a spell", spending a buff, taking damage, playing a specific tag, and attacks against a legend's battlefield. Add the events (most fit the string-keyed `ObjectStateChangedEvent`), then wire each card.

14 cards, 3 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| Gentle Gemdragon (`UNL-104/219`, id 666) | UNL | unit | fixed upstream | 3 | friendly unit play (documented approximation). |
| Battle Mistress (`SFD-203/221`, id 517) | SFD | legend | fixed upstream |  | gear token exhausted" clause has no engine trigger event and is not |
| Corrupt Enforcer (`SFD-123/221`, id 444) | SFD | unit | fixed upstream |  | No chain context (e.g. direct test invocation): best-effort |
| Fae Dragon (`SFD-101/221`, id 423) | SFD | unit | fixed upstream |  | ENGINE GAP: "When you spend a buff, play a Gold gear token exhausted." |
| Harpoon Squad (`SFD-137/221`, id 458) | SFD | unit | fixed upstream |  | any move (close approximation — combat units predominantly move off BFs). |
| Jae Medarda (`SFD-142/221`, id 463) | SFD | unit | fixed upstream |  | no code for: decision |
| Karma, Channeler (`SFD-237/221`, id 548) | SFD | unit | fixed upstream |  | ENGINE GAP: "When you recycle one or more cards to your Main Deck, |
| Lonely Poro (`UNL-221/219`, id 777) | UNL | unit | fixed upstream |  | as alone (documented approximation) and draw. |
| Nine-Tailed Fox (`OGN-255/298`, id 252) | OGN | legend | fixed upstream |  | rules text but no behaviour in the class |
| Noxian Guillotine (`OGN-254/298`, id 251) | OGN | spell | fixed upstream |  | ENGINE GAP: there is no "when it takes damage" trigger/event, so this |
| Relentless Storm (`OGN-300/298`, id 295) | OGN | legend | fixed upstream |  | one currently in play (best-effort identification of the play that |
| Scrapheap (`OGN-182/298`, id 182) | OGN | gear | fixed upstream |  | cannot fire: the TriggerManager does not subscribe to CardsDiscardedEvent |
| Solari Shrine (`OGN-072/298`, id 72) | OGN | gear | fixed upstream |  | satisfied (documented approximation, as with Piltover Enforcer/Gauntlets). |
| The Dreaming Tree (`OGN-292/298`, id 287) | OGN | battlefield | fixed here, CARD-013 |  | APPROX/INERT: no engine site currently emits "a spell chose a friendly unit |

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
