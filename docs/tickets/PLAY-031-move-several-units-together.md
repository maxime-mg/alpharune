# PLAY-031: Move several units together

- **Priority:** P2
- **Area:** Phase 3
- **Status:** todo
- **Depends on:** [PLAY-030](PLAY-030-drag-a-unit-to-move-it.md), [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md), [ENG-002](ENG-002-group-moves-cr-144-3.md)
- **Blocks:** nothing

## Scope
- After the first unit is dropped, other eligible units show a "+" badge from the `move_group` prompt, and a "Move N units" button sends Done.
- Shift-click or lasso before the drag replays the picks in the engine's order (increasing id).
