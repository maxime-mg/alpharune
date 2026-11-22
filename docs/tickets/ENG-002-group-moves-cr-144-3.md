# ENG-002: Group moves (CR 144.3)

- **Priority:** P0
- **Area:** Engine
- **Status:** done
- **Depends on:** nothing
- **Blocks:** [PLAY-031](PLAY-031-move-several-units-together.md), [CARD-008](CARD-008-movement-combat-and-scoring-rules.md)

## Done (upstream)
One `StandardMove` intent per unit and destination; `executeStandardMove` then asks which other units join, one prompt per unit (`PromptKind::MoveJoin`, first option "Move no more units"). The units move as one action. Tests: `tests/test_review_fix_sweep_flow.cpp` and `tests/test_review_fix_verify_engine.cpp`. Our own version (`GameState::move_cursor`, `tests/test_group_move.cpp`) was dropped in the rebase.
