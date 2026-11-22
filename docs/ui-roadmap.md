# Playing interface roadmap

Goal: replace the ASCII play page with a visual board a human can play on with drag and drop, without the UI ever inventing a move. **Every gesture resolves to one index in the engine's legal list.** The UI does not know the rules. It reads what the engine offers and shows only the gestures that match an offered intent.

Phases follow the requested order: board, then draws and plays, then moves, then the chain, then showdowns and score. Phase 0 is server and protocol work that every later phase depends on.

## 1. How the engine decides, and what the UI must mirror

The UI is a view over the engine's decision loop, so this section is the contract.

- **One decision at a time.** The game thread asks `HumanAgent::selectAction` for an index into `legal`. `PlayServer` sends a `decision` frame, and the client answers `choose {player, index}`. There is nothing else a client can send outside god mode.
- **Composite actions are a series of decisions.** Nothing about a play is decided in one frame:
  1. `PlayCard` chooses the card, the location and the play source.
  2. Optional costs follow: Accelerate, additional costs and Repeat. Each is a yes or no `MakeChoice` with `chosen_value` (`maybePayOptionalAdditionalCost`, `game_engine.cpp` ~2106-2150 and ~5486).
  3. Payment follows, **one rune per decision**. Each step offers one `MakeChoice{chosen_objects={rune}}` per candidate rune. Ready runes exhaust for energy, and runes of the matching domain recycle to the bottom of the rune deck for power (~5955-6030). `cost_cursor` holds the remaining energy and power and the domains still owed.
  4. Targets follow, one `MakeChoice` per legal target, from `Card::pickTarget` (`card.cpp` ~380-440), including play-time targets on `needsPlayTimeTarget` spells.

  The UI must keep the player inside that series. A drag starts it, and the board then asks for each follow-up in place.
- **The prompt label is data.** `requestChoice(player, options, label)` stores `PendingChoice.label`, for example `"target: <label>"`. The server used to scrape the label out of the log; since PLAY-006 it sends the engine's own prompt description (`describePrompt`).
- **Moves:** `generateMainPhaseActions` (~2610) offers one `StandardMove` per unit and destination, for base→battlefield, battlefield→base unless blocked, and battlefield→battlefield with Ganking or `accepts_any_inbound`. `executeStandardMove` loops over `units_to_move`, so the executor already handles groups, but the generator never offers them (see §8).
- **Combat damage** is a menu of preset allocations (greedy-lethal, spread-even, focus-all, lethal-first), ordered over permutations inside the Tank, middle and Backline buckets. Dune Surfer ignores Tank (~4010-4060).
- **Trigger choices happen at finalization** (CR 355, 383.3). A triggered ability's "you may", its cost and its targets are asked when it goes on the chain, not when it resolves. The UI asks for them at that point. A target that is no longer legal at resolution fizzles, and the chain item should show that. The UI lets the player pick among them. It cannot let the player assign freely.
- **Draw, channel, awaken, scoring and expiration are automatic.** The UI animates them from events. It never offers a drag for them.
- **Mulligan** offers the empty, single and pair subsets (~2896). The UI shows them as card selection, mapped to the matching subset intent.

### Gesture to intent mapping

| Gesture | Intent(s) it must match | Matching fields |
|---|---|---|
| Drag hand card to base | `PlayCard` | `card`, `play_location=Base`, `play_source=Hand` |
| Drag hand card to battlefield | `PlayCard` (Ambush, Pounce, `accepts_any_inbound`, or paid location cost) | `play_location=Battlefield{id}`, `pay_location_cost` |
| Drag from champion zone, trash or facedown | `PlayCard` | `play_source` = ChampionZone, Trash, Hidden |
| Drop on a zone that has two matching intents | Small chooser: "normal" vs "alternate cost" | `use_alt_play_cost` |
| Drag hand card to a battlefield, face down | `HideCard` | `card`, battlefield |
| Drag unit(s) to base or battlefield | `StandardMove` | `units_to_move`, `move_destination` |
| Click card ability | `ActivateAbility`, `ActivateReactionAbility`, `ActivateActionAbility` | `ability_source`, `ability_index`, `granted_ability_def` |
| Click a rune during payment | `MakeChoice` | `chosen_objects={rune}` |
| Click a highlighted unit, card or player | `MakeChoice` | `targets` or `chosen_objects` |
| Yes or no, mode, X value | `MakeChoice` | `chosen_value`, `chosen_card_def` |
| Play a reaction on the chain | `PlayReaction` | `card` |
| "Accept" or "Pass" | `PassPriority`, `PassFocus` | none |
| Optional trigger | `PlaceOptionalTrigger`, `DeclineOptionalTrigger`, `PayTriggeredCost`, `DeclineTriggeredCost` | none |
| Combat damage | `AssignCombatDamage` | `damage_assignments` (preview the allocation) |
| End turn | `EndTurn` | none |
| Setup and mulligan | `ChooseBattlefield`, `PlayFirstDecision`, `MulliganDecision`, `SideboardSwap` | `chosen_battlefield`, `choose_first`, `cards_to_mulligan` |
| Concede | `Concede` | none |

The matcher rule: the client finds every legal intent whose fields match the gesture.

- **One match:** send its index.
- **Several matches:** show a chooser that lists only those intents, with their labels.
- **No match:** the drop zone was never highlighted, so the drop cannot happen.

The client never builds an intent itself.

## 2. Phase 0: server and protocol (blocks everything)

The current protocol has gaps that a visual UI cannot work around.

1. **Per-seat views, to fix a hidden-information leak.** `buildZoneJson` sends both players' hands and the full order of both main decks and both rune decks to every client. Build the frame per viewer:
   - Hidden zones become counts plus card backs.
   - Hidden and facedown cards show only to their controller.
   - Revealed cards are tracked from `CardRevealed` and `observed_cards`.
   - A spectator view shows only public information. God mode stays as an explicit, labelled option.
2. **Fix `HumanAgent`'s player.** `paused_player_ = state.turn.turn_player` is wrong for decisions made by the non-turn player: reactions, defender choices, focus passes and opponent prompts. Use the deciding player that the engine passes.
3. **Stale-click protection.** The client must echo `decision_id`, and the server must reject a mismatch. Today an out-of-range index silently falls back to 0, which can play a move the player never chose. Reject it with an error instead.
4. **Rich object JSON.** `objToJson` sends only id, name, exhausted, damage and might. Add:
   - `card_def_id` (for art and text), type, owner and controller.
   - Base and current might, `damage_marked`, `temp_might_bonus`, `buff_count`, `empowered`.
   - Keywords and aura keywords, with Assault, Shield and Deflect values.
   - Stunned, hidden and `hidden_at`, `is_token`.
   - `attachments` and `attached_to`.
   - `combat_designation` (attacker or defender).
   - Untargetable flags and `granted_abilities`.
5. **Missing zones and state:**
   - Legend zone with its ready or exhausted state; the champion zone.
   - The rune board: each rune's domain and ready state, plus rune deck count.
   - The pool: energy, unit-only energy, power by domain, universal power.
   - Score and XP; banishment.
   - Battlefield state: `is_contested` and `contested_by`, facedown cards, showdown and combat (staged and in progress), attacker and defender.
   - Turn state: phase, `priority_holder`, `focus_holder`, players who passed.
   - The chain, as items with source, controller, targets, chosen objects, Pending or Finalized status and kind.
   - `cost_cursor` during payment.
6. **Structured prompts.** Send `prompt: {kind, label, source, min, max}` from `PendingChoice`, where kind is one of target, target pair A or B, may, mode, X, payment, damage, mulligan, battlefield, trigger. Stop scraping the log. This needs a small engine change: `requestChoice` takes a kind, or the label prefixes become an enum.
7. **Affordances per legal intent.** Today `legal[]` carries index, label, type and `card_object_id`. Add the matching fields from the table in §1:

   ```
   {index, type, label, source_ids[], dest, target_ids[], chosen_ids[], value, flags{alt_cost, location_cost, accelerate}}
   ```

   `LegalActionDesc` is declared in the headers but not implemented, so it is the natural home for this.
8. **Event stream.** Send `events[]` between decisions from the EventBus, carrying each event's ids: UnitMoved, CardsDrawn, CardPlayed, RuneChanneled, DamageDealt, UnitDied, Score, PointsGained, ControlChanged, Contested, Combat and Showdown Started or Ended, the Chain events, CardRevealed, UnitStunned, UnitReadied, PhaseChanged, PriorityGranted and GameOver. Animations and the readable log both come from this stream. Text logs stay for debugging only.
9. **Tests.** Golden-frame tests for each decision kind on scripted games. Add a fuzz test: for random games, every legal index's affordance must round-trip through the matcher to exactly that index.

**Exit:** a client can render the full board and every prompt from the frame alone, and receives nothing it should not see.

## 3. Phase 1: visual board, read only

Layout per player, with the opponent mirrored at the top:

- **Main deck:** card back with count. Draws animate from here.
- **Trash:** top card face up with count. Click to open the list.
- **Banishment:** like trash.
- **Rune deck:** card back with count.
- **Runes area:** each rune as a card with domain colour, ready or exhausted. Show the pool next to it (energy, power by domain).
- **Base:** units and gear with attachments stacked on their host.
- **Legend zone:** legend card, ready or exhausted, with its abilities.
- **Champion zone:** champion card, highlighted when it is playable.
- **Score:** a points track to 8, with XP.
- **Hand:** face up for the viewer; the opponent's as backs with a count.

The two battlefields sit in the middle. Each shows the card, controller colour, both sides' units, facedown cards, and Contested, Combat and Showdown badges.

Card rendering: art from `card_def_id` (the existing image pane already finds art), might as current/base with damage shown, a stun overlay, an exhausted rotation, keyword chips (from the abbreviations `state_renderer.cpp` already uses), buff counters, token marking, and ATK or DEF tags. Hover shows full text and the computed numbers.

A side panel holds the turn, phase, whose priority or focus it is, the chain (empty in this phase), and a readable event log.

Keep the ASCII view as a toggle. It is the reference that the visual board must match.

**Exit:** every field that `state_renderer.cpp` shows is visible on the board, for any state the scripted games reach. Check this with a test that compares the two.

## 4. Phase 2: draws and plays from hand

- **Draws:** animate `CardsDrawn` from the deck to the hand. Also animate channel (`RuneChanneled`, rune deck to runes area) and discards. No drag here: the engine does these on its own.
- **Drag to play:** picking up a hand card highlights every drop zone that a legal `PlayCard` or `HideCard` intent names for that card, at the base or a battlefield. Champion zone, trash (Flow and similar) and facedown cards are draggable sources too, but only when an intent names them.
- **Cost labels on drop zones:** show "+location cost" or "alternate cost" when the intent carries those flags. If a zone has several intents, show the chooser after the drop.
- **Payment stepper:** after the drop, the card waits in a "being played" slot.
  - The cost shows as pips for the energy and power still owed, from `cost_cursor`.
  - Candidate runes glow. Clicking one sends its `MakeChoice`. Exhaust and recycle look different, because recycling sends the rune to the bottom of the rune deck.
  - An "auto-pay" button is only a client loop that takes the first offered rune each time. It is still one decision per rune.
- **Optional costs:** Accelerate, additional costs and Repeat are yes or no prompts on the card, stated with their cost.
- **Unit-only energy and gear:** the pool display tells apart energy that only units can spend.
- Animate the play itself: the card moves to its location, and units enter exhausted unless Accelerated.

**Exit:** each sample deck's units and gear can be played by drag, including every variant those decks offer.

## 5. Phase 3: moving units

- Picking up a ready unit in the main phase highlights the destinations that `StandardMove` intents offer: base, battlefields (any battlefield with Ganking), and "cannot leave" when blocked.
- Moves are hidden during a showdown or combat (CR 144.1.c), because the engine offers none.
- **Group moves (implemented in the engine; the UI part is still to do).** CR 144.3 allows moving several units at once, and the executor already loops over `units_to_move`, but the generator offers single units only. So two units cannot reach a battlefield together, which changes combat. The fix is a follow-up decision after the move is chosen, not one intent per group. See "Group move design" below.
- **UI:** drag one unit to start the move. Then the other eligible units glow with a "+" badge, and a **Move N units** button confirms. A lasso or shift-click selection before the drag just replays those picks as the follow-up choices, one at a time.
- Animate `UnitMoved`. Arriving at a held battlefield shows Contested and then the showdown or combat start.

### Group move design

The generated `StandardMove` intents stay exactly as they are: one per unit and destination. That keeps the action list, the trainer's action vocabulary and existing tests unchanged. The group is built after the first pick:

1. The agent picks `StandardMove{units_to_move={u1}, move_destination=D}` as today.
2. Before anything moves, the engine opens a **move cursor**: the destination, the units chosen so far, and the player. The cursor follows the same resumable pattern as `cost_cursor`, so the step machine yields cleanly at each pick. It must not use the synchronous `decideNow`, because forking from a state copy (Track B of the eval roadmap) depends on decisions being resumable.
3. While at least one other unit could legally make the same move, the engine asks a `MakeChoice` with prompt kind `move_group`:
   - one option per eligible unit, `chosen_objects={u}`, which adds it and asks again;
   - one **Done** option, `chosen_value=0`.

   A unit is eligible when it has a single-unit `StandardMove` to `D` in the same generation pass. Reusing that one check means a Ganking unit can join a battlefield-to-battlefield move and a unit that cannot leave never joins.
4. On Done, `executeStandardMove` runs once with every chosen unit. It already exhausts each unit and emits `UnitMoved` per unit, and the contest and combat checks run once for the whole group, which is what CR 144.3 requires.
5. If no other unit is eligible, no follow-up is asked, so single moves cost nothing extra.

**Order.** Each group can be reached in several orders (u1 then u2, or u2 then u1). Requiring picks in increasing object id removes the duplicates from search: only units with a higher id than the last pick are offered. The UI hides this by sending the user's picks sorted.

**Cost to search.** A move of k units takes k decisions instead of 1. That is far smaller than listing every subset, which grows as 2^n per destination.

**Tests.**
- Two units moved together to an enemy-held battlefield start one combat, with both as attackers.
- A unit that cannot leave is never offered.
- Ganking units can join a battlefield-to-battlefield group, and others cannot.
- A clone taken at a `move_group` decision resumes to the same result.
- A single eligible unit produces no follow-up decision.

**Exit:** every `StandardMove` in a recorded game can be reproduced by drag.

## 6. Phase 4: spells, abilities and the chain

- **Chain panel:** a vertical stack, newest on top. Each item shows source art, controller, kind (spell, ability, trigger), and Pending or Finalized status, with arrows to its targets on the board.
- **Targeting mode:** on a `MakeChoice` of target kind, the board dims, and only `chosen_objects` and `targets` from the legal list stay clickable. The prompt label sits in a banner ("target: enemy unit"). Two-target spells run the A and B prompts in turn, and the first pick stays marked.
- **Priority:** when the opponent holds priority, the client shows their pending item and offers playable `PlayReaction` cards and `ActivateReactionAbility` abilities (highlighted in hand and on the board), plus **Accept**, which is `PassPriority`. Make it clear when both passes resolve the top item (CR 311-313).
- **Abilities:** clicking a card with an activatable ability, including granted ones, lists them by `ability_index` with their cost.
- **Triggers:** optional triggers and triggered costs appear as cards with Place or Decline and Pay or Decline.
- **Modes, X values and "may":** buttons limited to the offered values. Choose-a-card prompts (`chosen_card_def`) open a picker.
- **Hidden cards:** face-down cards at a battlefield (owner sees them, opponent sees backs), with play-from-facedown when it is offered.
- Animate resolution with `ChainItemResolved` and `SpellResolved`, plus damage and death.

**Exit:** every card in the sample decks that has a target or a choice is playable from the board in a scripted test.

## 7. Phase 5: showdowns, combat, score and game flow

- **Showdown:** a battlefield banner with focus holder, `PlayActionCard` and `ActivateActionAbility` highlighted, and **Pass** for `PassFocus`. Show which players have passed.
- **Combat:** ATK and DEF sides with the might totals, Assault and Shield included, and Tank and Backline ordering shown. `AssignCombatDamage` options show as a preview of each preset allocation (who dies) rather than raw text.
- **Control and score:** animate `ControlChanged`, Hold and Conquer from `Score` and `PointsGained` on the points track, and XP gains.
- **Setup:** choose a battlefield (card picker), play first or second, sideboard swap, and mulligan as card selection mapped to the offered subsets.
- **End:** game over screen with the winner, final score and reason, and a log export.

**Exit:** a complete human vs scripted-agent game is playable without the ASCII view.

## 8. Engine and server gaps found

These need code changes outside the UI. The first four block the phase shown.

| Gap | Where | Blocks |
|---|---|---|
| Both players' hands and deck orders sent to all clients | `buildZoneJson` in `play_server.cpp` | Phase 0 |
| Prompt scraped from logs; `PendingChoice.label` not forwarded; no prompt kind | `play_server.cpp`, `effect_executor.h`, `card.cpp` | Phase 0 |
| `HumanAgent` reports the turn player for every decision | `human_agent.h` | Phase 0, matters in Phase 4 |
| Out-of-range index falls back to 0; no `decision_id` check | `human_agent.h`, `play_server.cpp` | Phase 0 |
| No group moves (CR 144.3) | `generateMainPhaseActions`, `executeStandardMove` | **Done:** `GameState::move_cursor` and the `move_group` follow-up (§5), covered by `tests/test_group_move.cpp` |
| `LegalActionDesc` declared but not implemented | headers | Phase 0, item 7 |
| Combat damage limited to preset allocations | `game_engine.cpp` ~4010 | Fine for now; note it in the UI |
| Payment is rune by rune; no floating mana | `game_engine.cpp` ~5955 | None; the UI follows it |

## 9. Cross-cutting

- **Readability:** every intent label uses `renderChosenAction`, so the UI and logs agree. Card text on hover. The event log is written in sentences ("Ahri moves to Altar"), not log codes.
- **Hot-seat privacy:** a "pass the device" screen hides the hand between seats, built on per-seat views.
- **Undo:** none, to match the engine. Instead, a confirm step before irreversible drops is a setting.
- **Accessibility:** a click-to-select fallback for every drag, with keyboard focus and shortcuts (Space to pass, Enter to accept).
- **Tech:** keep the embedded single page in `play_index_html.cpp` while the protocol settles. Move to a separate built front end only if it outgrows that.
- **Order of work:** Phase 0 → 1 → 2 → 3 → 4 → 5, with the golden-frame and round-trip tests from Phase 0 kept green the whole way.
