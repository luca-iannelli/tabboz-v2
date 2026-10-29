# ChatGPT Handoff

> This is a disposable read-oriented projection. Canonical repository state overrides conversational memory and this handoff.
> It does not authorize work beyond canonical task and governance state.

## Resume Snapshot

- **Project ID/name:** Not recorded / Tabboz Simulator — Windows native modernization
- **Canonical repository locator:** github: luca-iannelli/tabboz-v2
- **State/schema version:** 2
- **State base commit:** 142fa7de71fe3d4700fa0027f18d45c6ccdaa9be
- **Verification provenance:** Not recorded
- **Persisted ref observation (non-authoritative): Not recorded**
- **Current phase:** completed
- **Current task:** No active task.
- **Next task:** None recorded

## Frozen Invariants

- Retain original C gameplay rules, economy, events, activity outcomes, original character assets, and tone.
- Deliver a native Windows application and installer, preserving upstream attribution and GPL-3.0-or-later licensing.
- The user selected C with modernized native Windows controls on 2026-10-29.
- Euros are the default currency with cents; preserve the original internal-unit conversion and economy.

## Active Durable Decisions

- ADR-0001: Native C/Win32, preserved economy, modern euro UI and safe persistence (.project/decisions/ADR-0001-native-windows-euros.md)

## Blockers and Known Issues

### Blockers

- None recorded

### Known Issues

- windows-10-runtime
- physical-multi-dpi
- original-phone-arithmetic

## DO-NOT-REDO

- Upstream source is already cloned locally; reuse it and its pinned commit.
- REA original PE resource analysis is retained in docs/research; do not repeat it without a new question.
- MSVC 2022, Windows SDK, and CMake exist on the Windows host; do not install duplicate toolchains.

## Important Acquired External Data/Assets

- user-splash: artwork from User-provided Tabboz Simulator_ Edizione Windows 10_11.png at assets/splash.png (reacquisition: prohibited)
- user-icon: icon from User-provided tabboz-simulator.ico at assets/app.ico (reacquisition: prohibited)

## Canonical Pointers

- `.project/ROADMAP.md` — milestone index
- `.project/state.json` — canonical structured state
- `.project/CURRENT_TASK.md` — current-task working detail
- `.project/DATA_INVENTORY.json` — external-data index, when present
- `.project/decisions/` — ADRs referenced by the active decision index
