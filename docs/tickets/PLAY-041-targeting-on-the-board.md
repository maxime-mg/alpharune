# PLAY-041: Targeting on the board

- **Priority:** P1
- **Area:** Phase 4
- **Status:** todo
- **Depends on:** [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md), [PLAY-007](PLAY-007-describe-what-each-legal-action-does.md)
- **Blocks:** nothing

## Scope
- On a target prompt, dim the board and leave clickable only the objects the legal choices name. Show the prompt label as a banner.
- Two-target spells run the A and B prompts in turn, keeping the first pick marked.
- Prompt kinds: `Target`, `TargetFirst` / `TargetSecond`, `ChooseObject`, `SplitTargets` / `SplitAmount`, `ChainItem` (a counter's choice, shown on the chain panel), `Location`.
- A spell's targets are chosen before its cost is paid, so the rune prompts come after.
