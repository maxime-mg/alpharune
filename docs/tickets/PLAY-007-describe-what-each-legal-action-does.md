# PLAY-007: Describe what each legal action does

- **Priority:** P1
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [PLAY-003](PLAY-003-full-board-state-in-every-frame.md)
- **Blocks:** [PLAY-009](PLAY-009-golden-frame-and-gesture-round-trip-tests.md), [PLAY-021](PLAY-021-drag-a-card-from-hand-to-play-it.md), [PLAY-030](PLAY-030-drag-a-unit-to-move-it.md), [PLAY-041](PLAY-041-targeting-on-the-board.md), [PLAY-043](PLAY-043-activated-abilities-menu.md)

## Why
Drag and drop has to match a gesture to exactly one legal intent. The client needs each intent's source, destination and targets without re-implementing the rules.

## Scope
- Implement `LegalActionDesc` (declared, never implemented) and send `{index, type, label, source_ids, dest, target_ids, chosen_ids, value, flags}` per legal intent.

## Done when
A fuzz test over random games shows every legal index has a descriptor that matches only that index.
