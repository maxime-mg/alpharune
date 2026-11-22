# CARD-004: Non-resource additional costs and conditional cost changes

- **Priority:** P1
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Re-check (2026-11-22, upstream `ec90f0f`)
Upstream fixed this group: 15 of 16 cards match their text, checked by reading each class and the engine fields it sets, not by playing them. Crescent Guardian was fixed later, in [CARD-013](CARD-013-gaps-left-after-upstreams-card-fixes.md). The text below describes the old code.

## Why these belong together
Kill a unit, return a gear, spend a buff or XP as a cost, and costs that change with game state. `optionalAdditionalCost()` only pays energy and power today. Needs a structured additional-cost type the action generator and payment path understand.

16 cards, 3 copies across the sample decks.

## Cards
| Card | Set | Type | Status | Copies in sample decks | What was wrong at `b05e51d` |
|---|---|---|---|---|---|
| Poro Snax (`SFD-046/221`, id 369) | SFD | gear | fixed upstream | 3 | [1][G], [E], Kill this. Approximated as [1] energy + exhaust |
| Atakhan (`UNL-170/219`, id 732) | UNL | unit | fixed upstream |  | ENGINE GAP: this is an OPTIONAL non-resource additional cost (kill a |
| Commander Ledros (`OGN-231/298`, id 231) | OGN | unit | fixed upstream |  | rules text but no behaviour in the class |
| Crescent Guardian (`UNL-122/219`, id 684) | UNL | unit | fixed here, CARD-013 |  | The "played a spell this turn" precondition cannot be enforced on the |
| Cruel Patron (`OGN-208/298`, id 208) | OGN | unit | fixed upstream |  | NOTE: there is no engine hook for a "kill a unit" additional play cost, |
| Heedless Resurrection (`UNL-142/219`, id 704) | UNL | spell | fixed upstream |  | has no structured non-resource additional-cost surface for spells). Pick the |
| Irelia, Graceful (`SFD-141/221`, id 462) | SFD | unit | fixed upstream |  | no code for: decision |
| Kraken Hunter (`OGN-150/298`, id 150) | OGN | unit | fixed upstream |  | rules text but no behaviour in the class |
| Legion Quartermaster (`SFD-044/221`, id 367) | SFD | unit | fixed upstream |  | owner's hand." Mandatory additional cost. The engine has no structured |
| Master Yi, Unstoppable (`UNL-059/219`, id 621) | UNL | unit | fixed upstream |  | power portion of each tier is an ENGINE GAP and is NOT applied here. |
| Needlessly Large Yordle (`SFD-055/221`, id 378) | SFD | unit | fixed upstream |  | discount — 1 Order power per hold point — cannot be expressed. The engine |
| Poppy, Defender of the Meek (`UNL-178/219`, id 740) | UNL | unit | fixed upstream |  | play-cost-reduction hook) + an XP deduction at play. APPROXIMATION: the |
| Spoils of War (`OGN-144/298`, id 144) | OGN | spell | fixed upstream |  | APPROXIMATION: there is no turn-scoped death tracker in GameState, so |
| Stalking Wolf (`UNL-166/219`, id 728) | UNL | unit | fixed upstream |  | agent-chosen Bird/Cat/Dog/Poro. APPROXIMATIONS / ENGINE GAP: |
| Undying Loyalty (`UNL-168/219`, id 730) | UNL | spell | fixed upstream |  | clause is approximated as no Power gate — no structured Power-budget hook). |
| Wallop (`OGN-146/298`, id 146) | OGN | spell | fixed upstream |  | ENGINE LIMITATION: optionalAdditionalCost() only pays energy/power and |

## Rules text
- **Atakhan**: You may kill a friendly unit as an additional cost to play me. If you do, I cost [1] less for each Energy it costs and [Y] less for each Power it costs. [Ganking] (I can move from battlefield to battlefield.) When I attack, the defender must kill one of their units here.
- **Commander Ledros**: As you play me, you may kill any number of friendly units as an additional cost. Reduce my cost by [Y] for each killed this way. [Deflect] (Opponents must pay [A] to choose me with a spell or ability.) [Ganking] (I can move from battlefield to battlefield.)
- **Crescent Guardian**: If you've played a spell this turn, you may pay [P] as an additional cost to play me. If you do, I enter ready.
- **Cruel Patron**: As an additional cost to play me, kill a friendly unit.
- **Heedless Resurrection**: [Reaction] (Play any time, even before spells and abilities resolve.) As an additional cost to play this, kill a friendly unit. Play a unit from your trash that costs no more Energy and no more Power than the killed unit, ignoring its cost.
- **Irelia, Graceful**: Your spells that choose me cost [1] or [A] less.
- **Kraken Hunter**: [Accelerate] (You may pay [1][O] as an additional cost to have me enter ready.) [Assault] (+1 [M] while I'm an attacker.) As you play me, you may spend any number of buffs as an additional cost. Reduce my cost by [O] for each buff you spend.
- **Legion Quartermaster**: As an additional cost to play me, return a friendly gear to its owner's hand.
- **Master Yi, Unstoppable**: [Level 3][>] I cost [2][G] less. (While you have 3+ XP, get the effect.) [Level 6][>] I cost [4][G][G] less instead. [Level 11][>] I cost [6][G][G][G] less instead. [Level 16][>] I can't be chosen by enemy spells and abilities.
- **Needlessly Large Yordle**: [Shield 5] (+5 [M] while I'm a defender.) [Tank] (I must be assigned combat damage first.) I cost [2][G] less for each point you scored from holding this turn.
- **Poppy, Defender of the Meek**: You may spend 3 XP as an additional cost to play me. If you do, I cost [3] less. [Ambush] (You may play me as a [Reaction] to a battlefield where you have units.) [Tank] (I must be assigned combat damage first.)
- **Poro Snax**: When you play this, draw 1. [1][G], [E], Kill this: Draw 1.
- **Spoils of War**: [Reaction] (Play any time, even before spells and abilities resolve.) If an enemy unit has died this turn, this costs [2] less. Draw 2.
- **Stalking Wolf**: [Ambush] (You may play me as a [Reaction] to a battlefield where you have units.) As an additional cost to play me, kill a Bird, Cat, Dog, or Poro you control. You may [Ambush] me to its battlefield, even if you don't have other units there.
- **Undying Loyalty**: This costs [2] less if you choose a Bird, Cat, Dog, or Poro. Play a unit with cost no more than [2] and no more than [A] from your trash, ignoring its cost.
- **Wallop**: [Action] (Play on your turn or in showdowns.) As you play this, you may spend a buff as an additional cost. If you do, ignore this spell's cost. Ready a unit.

## Done when
- Each card behaves as its text says, or the ticket records why an approximation stays.
- Each card has a test under `tests/cards/`.
- `scripts/card_status.py` reports none of them as partial or not implemented.
