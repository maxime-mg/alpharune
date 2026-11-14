# PLAY-006: Send structured prompts to the client

- **Priority:** P1
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [ENG-001](ENG-001-structured-prompt-kind-for-every-choice.md), [PLAY-003](PLAY-003-full-board-state-in-every-frame.md)
- **Blocks:** [PLAY-023](PLAY-023-pay-costs-by-clicking-runes.md), [PLAY-024](PLAY-024-optional-costs-accelerate-additional-costs-repeat.md), [PLAY-031](PLAY-031-move-several-units-together.md), [PLAY-041](PLAY-041-targeting-on-the-board.md), [PLAY-044](PLAY-044-triggers-modes-x-values-and-card-name-pickers.md), [PLAY-051](PLAY-051-combat-view-and-damage-preview.md), [PLAY-053](PLAY-053-setup-and-mulligan-screens.md)

## Scope
- Replace the log-scraped `last_prompt` with `prompt: {kind, label, source, chooser}` from ENG-001.

## Done when
No prompt text in the play server comes from log parsing.
