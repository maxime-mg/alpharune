# PLAY-023: Pay costs by clicking runes

- **Priority:** P1
- **Area:** Phase 2
- **Status:** todo
- **Depends on:** [PLAY-003](PLAY-003-full-board-state-in-every-frame.md), [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md), [PLAY-021](PLAY-021-drag-a-card-from-hand-to-play-it.md)
- **Blocks:** nothing

## Scope
- During payment the card waits in a "being played" slot with pips for the energy and power still owed.
- Show every pool field in the frame and on the board: `unit_energy`, `spell_energy`, `showdown_energy`, `spell_power`, `gear_power` (only `unit_energy` is sent today).
- Prompt kinds: `RuneForEnergy`, `RuneForPower`, `AddForPayment`, `CostSplit`. Equip, abilities, Deflect and effect payments ask which rune too, not only card plays.
- Candidate runes glow. Exhaust (energy) and recycle (power) look different.
- "Auto-pay" is a client loop that takes the first offered rune each time.
