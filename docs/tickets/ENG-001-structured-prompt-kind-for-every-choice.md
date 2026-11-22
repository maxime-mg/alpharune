# ENG-001: Structured prompt kind for every choice

- **Priority:** P0
- **Area:** Engine
- **Status:** done
- **Depends on:** nothing
- **Blocks:** [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md)

## Done (upstream, re-checked 2026-11-22)
`src/core/prompt.h`: `PromptKind` (37 kinds) and `Prompt` (kind, decider, source, chain item, question). Read it from `GameEngine::pendingPrompt()`, `StepResult::prompt` or `AgentInterface::decide(state, legal, prompt)`; `describePrompt` (`src/engine/prompt_description.h`) turns it into what one player may be shown. `PromptCensus.*` in `tests/test_prompt_description.cpp` is documented as the test that no decision of random games is `Unspecified`. Nothing in `play/` uses any of this yet: that is PLAY-006.

## Why
The UI learns what is being asked by scraping log lines (`TGT_PROMPT`, `MAY_PROMPT`, ...). The engine already has the label in `PendingChoice.label`.

## Scope
- Add a `PromptKind` enum (target, target pair A/B, may, mode, X, card name, discard, payment, move group, damage, mulligan, battlefield, trigger) to `requestChoice` and to engine-level decisions such as rune payment.
- Expose the current prompt (kind, label, source object, chooser) through `GameEngine`.

## Done when
Every `requestChoice` call site passes a kind, and a test enumerates the kinds seen in 200 random games with none unknown.
