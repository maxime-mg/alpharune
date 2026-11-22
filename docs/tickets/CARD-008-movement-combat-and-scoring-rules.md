# CARD-008: Movement, combat and scoring rules

- **Priority:** P0
- **Area:** Cards
- **Status:** done
- **Depends on:** [ENG-002](ENG-002-group-moves-cr-144-3.md)
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 9 of 9 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. The text below describes the old code.

## Why these belong together
Excess combat damage is not reported to triggers, "can't move" and "can't be readied" flags are missing or inert, the attacker rule for spell moves is missing, and "triggers an additional time" for hold and conquer is not modelled. Mageseeker Investigator can now be built on group moves (ENG-002).

9 cards, 21 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| Vex, Apathetic (`UNL-150/219`, id 712) | UNL | unit | fixed upstream | 13 | TODO: "They can't move it this turn / it doesn't deal combat damage this |
| Blue Sentinel (`UNL-087/219`, id 649) | UNL | unit | fixed upstream | 3 | "When I hold, [Add] [A] ..." — approximated as immediate add (the |
| Void Assault (`UNL-202/219`, id 758) | UNL | spell | fixed upstream | 3 | ENGINE GAP: the parenthetical "you're the attacker" designation when both |
| Minotaur Reckoner (`SFD-014/221`, id 337) | SFD | unit | fixed upstream | 2 | approximation for the common case where I remain in play. |
| Maduli the Gatekeeper (`UNL-144/219`, id 706) | UNL | unit | fixed upstream |  | ENGINE GAP: there is no GameObject "can't be readied" flag, and |
| Mageseeker Investigator (`UNL-163/219`, id 725) | UNL | unit | fixed upstream |  | flag is the faithful state representation but is currently inert. (Wiring a |
| Red Brambleback (`UNL-029/219`, id 591) | UNL | unit | fixed upstream |  | ENGINE GAP: "Your conquer effects for conquering here trigger an |
| Sivir, Ambitious (`SFD-120/221`, id 441) | SFD | unit | fixed upstream |  | ENGINE GAP: the amount of excess combat damage assigned to enemy |
| Yeti Brawler (`UNL-018/219`, id 580) | UNL | unit | fixed upstream |  | Excess-damage assignment is not surfaced to triggers (same engine gap |

## Rules text
- **Blue Sentinel**: [Shield 2] (+2 [M] while I'm a defender.) Your hold effects for holding here trigger an additional time. When I hold, [Add] [A] at the start of your next Main Phase. (Abilities that add resources can't be reacted to.)
- **Maduli the Gatekeeper**: I can't be readied. [P]: Move me to an occupied enemy battlefield if my Might is greater than the total Might of enemy units there.
- **Mageseeker Investigator**: Opponents must pay [A] for each unit beyond the first to move multiple units to my battlefield at the same time.
- **Minotaur Reckoner**: Units can't move to base.
- **Red Brambleback**: [Accelerate] (You may pay [1][R] as an additional cost to have me enter ready.) Your conquer effects for conquering here trigger an additional time. When I conquer, [Buff] a friendly unit. (Give it a +1 [M] buff if it doesn't have one.)
- **Sivir, Ambitious**: [Deflect 2] (Opponents must pay [A][A] to choose me with a spell or Ability.) When I conquer after an attack, if you assigned 5 or more excess damage to enemy units, you may deal that much to an enemy unit.
- **Vex, Apathetic**: [Deflect] (Opponents must pay [A] to choose me with a spell or ability.) When an opponent plays a unit while I'm at a battlefield, [Stun] it. They can't move it this turn. (It doesn't deal combat damage this turn.)
- **Void Assault**: Move a friendly unit, then move an enemy unit. (If they both move to a battlefield you don't control, you're the attacker.)
- **Yeti Brawler**: When I conquer, if you assigned 3 or more excess damage, play two Gold gear tokens exhausted. (They have "[Reaction][>] Kill this, [E]: [Add] [A].")

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
