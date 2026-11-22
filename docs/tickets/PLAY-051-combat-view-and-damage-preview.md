# PLAY-051: Combat view and damage preview

- **Priority:** P1
- **Area:** Phase 5
- **Status:** todo
- **Depends on:** [PLAY-006](PLAY-006-send-structured-prompts-to-the-client.md), [PLAY-011](PLAY-011-card-rendering.md)
- **Blocks:** nothing

## Scope
- Attacker and defender sides with might totals, including Assault and Shield, and Tank and Backline order.
- Each `AssignCombatDamage` option previewed as the units it kills, not raw text.
- Two prompt shapes: `CombatDamage` offers whole assignments (up to 64); past that, `CombatDamageNext` asks which unit takes lethal damage next, one at a time.
