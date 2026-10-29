# Current Task

Native Windows 10/11 port of Tabboz Simulator with conservative usability improvements, euro currency, native installer and portable package.

## Authorization and ownership

The user approved the design and implementation, selected C plus native Windows controls, requested euros, supplied splash/icon artwork, and authorized code/documentation publication to the public repository `luca-iannelli/tabboz-v2`. Actor and sole write owner: codex. No Live Coordination issue is adopted.

The dedicated branch is `native/windows-modernization`, based on upstream `142fa7de71fe3d4700fa0027f18d45c6ccdaa9be`. The destination's README-only initial commit `000ef96ec047fd9805ffdc158da4296eae887101` is retained as a merge parent. Independent code review is required before the first checkpoint. No upstream write, public release, or unrelated message is authorized.

## Implementation and evidence

- Native MSVC/CMake x64 build with all 81 dialogs, original gameplay modules, themed standard controls, resizable main screen and simulated 96/120/144/192 DPI layout checks.
- Euros with cents preserve the original internal economy.
- User-supplied splash and eight-resolution icon embedded in the executable.
- Raw atomic save writes, checksum validation, corrupt-file recovery, read-only import of legacy nested registry keys and safe choices when closing after a write failure.
- Six Python tests, euro C test and 143 native integration checks pass. Test fixtures cover Unicode/UNC WSL saves and nested legacy registry data; the temporary registry tree is removed.
- GUI normal startup/save/exit and forced-save-error exit pass. Installer install/launch/upgrade/uninstall passes with data retained. Detailed local evidence lives under `build/validation*/` and `dist/`.
- Original REA evidence and new artifact inspection are tracked under `docs/research/`.

## Boundaries and next action

Windows 10 execution and physical multi-monitor DPI transitions are unverified; Windows 11 and simulated main-window DPI checks are observed. Preserve original formulas, assets, tone, attribution and existing acquired evidence.

All implementation, artifact and installer checks are recorded in docs/VALIDATION.md, docs/CODE_REVIEW.md and docs/research/windows-native-artifact-verification.md. The reviewed checkpoint retains both source and destination history. Do not repeat source acquisition or install duplicate compiler toolchains.
