# PLAY-008: Event stream for animation and a readable log

- **Priority:** P1
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [PLAY-002](PLAY-002-per-seat-views-stop-leaking-hidden-information.md)
- **Blocks:** [PLAY-020](PLAY-020-animate-draws-channels-and-discards.md), [PLAY-052](PLAY-052-score-control-and-conquer-animations.md), [PLAY-054](PLAY-054-game-over-screen-and-log-export.md)

## Scope
- Forward EventBus events (moves, draws, plays, channels, damage, deaths, score, control, contest, combat and showdown, chain, reveals, stun, ready, phase, game over) as `events[]` between decisions.
- Filter per seat: an opponent's draw is "drew 1", not the card.
- Render each event as a sentence for the log panel.

## Done when
The log panel shows a human-readable line for every event in a full game, and no hidden card name reaches the wrong seat.
