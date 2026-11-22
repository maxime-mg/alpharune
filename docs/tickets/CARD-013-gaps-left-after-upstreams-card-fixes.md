# CARD-013: Gaps left after upstream's card fixes

- **Priority:** P1
- **Area:** Cards
- **Status:** done
- **Depends on:** nothing
- **Blocks:** nothing

## Done (2026-11-22)
All five are fixed on the `play-ui` branch, with tests in `tests/cards/test_card_013.cpp`. Each defect's test failed on the old code. The Dreaming Tree now uses `firstTimeEachTurn`, which counts per player on a battlefield. These changes are in the fork only. The text below describes the old code.

### Found while fixing: Curtain Call (id 743)
"Choose one you haven't already chosen" was not enforced across its [Repeat] executions: the used modes were recorded as each one resolved, after all of them had chosen (CR 820.2). Fixed with `Card::modesOfOtherExecutions`, which reads the choices already made for the spell; tests in `tests/cards/test_curtain_call_repeat.cpp`. A Repeat that can be paid with no mode left to choose is still offered, and that execution does nothing. Whether it may be paid at all is the open Hard Bargain question in `docs/testing-findings-decks-2026-10.md`.

## Why
Re-reading the 105 cards of CARD-001 to CARD-012 against upstream `ec90f0f` found 100 fixed. These five are not, or not completely. Apprentice Smith is in the sample decks (3 copies), so fix it first; the others are not, and The Dreaming Tree is banned.

## Cards
| Card | Where | What is wrong | Fix |
|---|---|---|---|
| Apprentice Smith (id 364) | `src/cards/units/0364_apprentice_smith.cpp` | "Draw it" uses `deck::putInHand`, so `draws_this_turn` and "when you draw" abilities miss it. It is the only "draw" text in the pool that does | `ctx.executor.drawHeldCard(controller, top, /*seen_by_all=*/true)`, as Ornn, Blacksmith does |
| Crescent Guardian (id 684) | `src/cards/units/0684_crescent_guardian.cpp` | "I enter ready" is a "when you play me" ability: the unit enters exhausted and is readied by a chain item the opponent can respond to. It is the only card that does this | Override the three-argument `entersReadyOnPlay`, reading the paid flag, and drop the trigger |
| Temporal Portal (id 401) | `GameEngine` Repeat offers (`game_engine.cpp`, "GRANTED [Repeat]") | A spell that prints [Repeat] does not get the granted one, and the grant is used up anyway. CR 820.3: each instance can be paid separately. The Academy (772) and Syndra, Transcendent (708) go through the same code, and the one-shot grant also hides Syndra's | Append the granted tranches to the printed ones instead of requiring the list to be empty |
| The Boss (id 264) | `src/cards/legends/0264_the_boss.cpp` | [C] on a Body/Order card may be paid with either Domain (CR 135.2.e.6.c). `applyReplacement` takes the first payable Domain, so the player is not asked which rune to recycle | Pay once with `resource::maskOf(def().domains)` so the rune prompt covers both Domains |
| The Dreaming Tree (id 287) | `src/cards/battlefields/0287_the_dreaming_tree.cpp` | "For the first time each turn" is stamped as the ability resolves. If the first ability is countered, the second choice still draws; CR 383.1 counts the event | Count in `triggerCondition` from per-turn state, as Star Spring and the `*_this_turn` counters do |

Also add a test for Bandle Tree (id 273): its code is right, but no test covers hiding a second card there.

## Done when
- Each card has a test that fails on the code above and passes after the fix.
- The five entries are removed from `CONFIRMED` in `scripts/card_status.py`, and `docs/card-status/` is regenerated.

## Upstream
These are engine and card files that upstream owns, so a pull request to upstream avoids carrying a permanent difference in the fork.
