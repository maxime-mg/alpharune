# ENG-002: Group moves (CR 144.3)

- **Priority:** P0
- **Area:** Engine
- **Status:** done
- **Depends on:** nothing
- **Blocks:** [PLAY-031](PLAY-031-move-several-units-together.md), [CARD-008](CARD-008-movement-combat-and-scoring-rules.md)

## Done
`GameState::move_cursor` and the `move_group` follow-up decision, covered by `tests/test_group_move.cpp`, including resuming from a snapshot taken mid-move.
