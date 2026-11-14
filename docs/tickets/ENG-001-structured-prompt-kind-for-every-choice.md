# ENG-001: Structured prompt kind for every choice

- **Priority:** P0
- **Area:** Engine
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md)

## Why
The UI learns what is being asked by scraping log lines (`TGT_PROMPT`, `MAY_PROMPT`, ...). The engine already has the label in `PendingChoice.label`.

## Scope
- Add a `PromptKind` enum (target, target pair A/B, may, mode, X, card name, discard, payment, move group, damage, mulligan, battlefield, trigger) to `requestChoice` and to engine-level decisions such as rune payment.
- Expose the current prompt (kind, label, source object, chooser) through `GameEngine`.

## Done when
Every `requestChoice` call site passes a kind, and a test enumerates the kinds seen in 200 random games with none unknown.
