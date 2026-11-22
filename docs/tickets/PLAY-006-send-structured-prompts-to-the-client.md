# PLAY-006: Send structured prompts to the client

- **Priority:** P1
- **Area:** Phase 0
- **Status:** todo
- **Depends on:** [ENG-001](ENG-001-structured-prompt-kind-for-every-choice.md), [PLAY-003](PLAY-003-full-board-state-in-every-frame.md)
- **Blocks:** [PLAY-023](PLAY-023-pay-costs-by-clicking-runes.md), [PLAY-024](PLAY-024-optional-costs-accelerate-additional-costs-repeat.md), [PLAY-031](PLAY-031-move-several-units-together.md), [PLAY-041](PLAY-041-targeting-on-the-board.md), [PLAY-044](PLAY-044-triggers-modes-x-values-and-card-name-pickers.md), [PLAY-051](PLAY-051-combat-view-and-damage-preview.md), [PLAY-053](PLAY-053-setup-and-mulligan-screens.md)

## Why
The engine now describes every decision (ENG-001, done upstream), but the play server still shows the text it finds in `*_PROMPT:` log lines (`last_prompt` in `play/src/play_server.cpp`), so many choices arrive with no question and labels like `=0` / `=1`.

## Scope
- Override `AgentInterface::decide` in `HumanAgent` to receive the `Prompt`.
- Send `describePrompt(state, prompt, legal, observer, db)` per seat as `prompt: {kind, decider, source_card, question, text, options[]}`. Each option has `index`, `role` (Yes, No, Value, Object, NamedCard, ChainItem, Action) and `text`. The other seat gets the kind and the asking card, not the words or options, which `describePrompt` already ensures.
- Order options as `describePrompt` does (by action id), never by list index.
- Delete `last_prompt` and the log scraping.

## Done when
No prompt text in the play server comes from log parsing.
