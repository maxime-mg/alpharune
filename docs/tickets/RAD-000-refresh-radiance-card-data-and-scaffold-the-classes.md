# RAD-000: Refresh Radiance card data and scaffold the classes

- **Priority:** P1
- **Area:** Cards
- **Status:** todo
- **Depends on:** nothing
- **Blocks:** [RAD-001](RAD-001-radiance-deploy-gear-and-bomb-tokens.md), [RAD-002](RAD-002-radiance-disarm.md), [RAD-003](RAD-003-radiance-show-off.md), [RAD-004](RAD-004-radiance-recruit-and-mech-tokens.md), [RAD-005](RAD-005-radiance-legends.md), [RAD-006](RAD-006-radiance-battlefields.md), [RAD-007](RAD-007-radiance-fury-cards.md), [RAD-008](RAD-008-radiance-calm-cards.md), [RAD-009](RAD-009-radiance-mind-cards.md), [RAD-010](RAD-010-radiance-body-cards.md), [RAD-011](RAD-011-radiance-chaos-cards.md), [RAD-012](RAD-012-radiance-order-cards.md), [RAD-013](RAD-013-radiance-cards-with-no-domain-in-the-pre-release-data.md)

## Why
All 145 Radiance cards are missing: none has a class in `src/cards/`. The gallery data was pulled before release and is dirty: 21 cards had the wrong type and several have no domain (see `docs/card-coverage/set-update-2026-10.md`). Radiance released on 2026-10-23.

## Scope
- Re-run `scripts/fetch_cards.py` and check against `errata/radiance-errata.txt`.
- Generate one class per card under `src/cards/<type>/` with a correct `def()` (ids 956 and up are reserved for Radiance), no behaviour yet, so later tickets only add behaviour.
- Add any new rules from the Radiance core-rules patch notes that are not already in the engine.

## Done when
The Radiance cards are registered, `--dump-registry` lists them with the right type and domain, and `scripts/card_status.py` reports them as not implemented rather than "no class".
