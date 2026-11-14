# CARD-006: "This turn" effects and turn-scoped tracking

- **Priority:** P1
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** nothing

## Why these belong together
Units that enter ready this turn, delayed abilities that should fire more than once, and "if you've done X this turn" checks. Needs per-player, per-turn flags and counters in `GameState` that reset in the expiration step.

10 cards, 4 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What is wrong |
|---|---|---|---|---|---|
| Confront (`OGN-129/298`, id 129) | OGN | spell | partial-documented | 4 | ENGINE LIMITATION: there is no per-player "units enter ready this |
| Bushwhack (`SFD-004/221`, id 327) | SFD | spell | partial-documented |  | so we approximate by readying the controller's currently-on-board |
| Dancing Grenade (`UNL-020/219`, id 582) | UNL | spell | partial-documented |  | The recast-for-[A] loop has no engine "may play this again + charge [A]" |
| Lillia, Protector of Dreams (`UNL-058/219`, id 620) | UNL | unit | partial-documented |  | this turn" is an ENGINE GAP — tokens enter via EffectExecutor::createToken |
| Magma Wurm (`OGN-011/298`, id 11) | OGN | unit | not-implemented |  | rules text but no behaviour in the class |
| Prepared Neophyte (`UNL-004/219`, id 563) | UNL | unit | partial-documented |  | this turn (reset at end of turn). Approximation: any single spell of |
| Rally the Troops (`SFD-166/221`, id 487) | SFD | spell | partial-documented |  | ENGINE GAP: the delayed-ability mechanism is one-shot (fires once |
| Stand United (`OGN-053/298`, id 53) | OGN | spell | partial-documented |  | ENGINE GAP: there is no buff-amplification hook (a per-player field that |
| Temporal Portal (`SFD-078/221`, id 401) | SFD | gear | partial-documented |  | with a magnitude equal to its own cost — has no engine primitive: there |
| Wuju Master (`UNL-231/219`, id 784) | UNL | legend | partial-documented |  | that engine gap is closed. |

## Rules text
- **Bushwhack**: [Hidden] (Hide now for [A] to react with later for .) Friendly units enter ready this turn. Play a Gold gear token exhausted.
- **Confront**: [Action] (Play on your turn or in showdowns.) Units you play this turn enter ready. Draw 1.
- **Dancing Grenade**: Deal 2 to a unit. Its controller may play this spell again for [A]. If they do, this deals 1 additional Bonus Damage for each time this spell has dealt damage this turn.
- **Lillia, Protector of Dreams**: When you play a token unit, give me +1 [M] this turn. Your token units have [Tank]. (They must be assigned combat damage first.)
- **Magma Wurm**: Other friendly units enter ready.
- **Prepared Neophyte**: If you've spent [4] or more to play a spell this turn, I have +4 [M].
- **Rally the Troops**: [Action] (Play on your turn or in showdowns.) When a friendly unit is played this turn, buff it. (If it doesn't have a buff, it gets a +1 [M] buff.) Draw 1.
- **Stand United**: [Hidden] (Hide now for [A] to react with later for .) [Action] (Play on your turn or in showdowns.) Buff a friendly unit. Buffs give an additional +1 [M] to friendly units this turn. (To buff a unit, give it a +1 [M] buff if it doesn't already have one.)
- **Temporal Portal**: [A], [E]: Give the next spell you play this turn [Repeat] equal to its cost. (You may pay the additional cost to repeat the spell's effect.)
- **Wuju Master**: [Level 6][>] Your units have +1 [M]. [Level 11][>] Your units enter ready.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
