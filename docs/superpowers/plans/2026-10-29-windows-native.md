# Native Windows Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Build, verify and publish the native Windows port and installer in the user's GitHub repository.

**Architecture:** Retain original C gameplay modules. Add a focused Win32 presentation/persistence layer and modern resource generator; compile with MSVC/CMake. Preserve upstream history and the user's initial repository commit.

**Tech Stack:** C, Win32/Common Controls/GDI, Python standard library, CMake, MSVC 2022, PowerShell, Inno Setup.

**Spec:** `docs/superpowers/specs/2026-10-29-windows-native-design.md`, approved by user; additions: euros by default and publication to `luca-iannelli/tabboz-v2`.

## Global Constraints

- Preserve gameplay rules, original character, catalogues, sounds and attribution.
- Native x64 Windows 10/11 application, no BWCC or browser dependency.
- User-selected C and native Windows interface; euros use original game's 2:1 internal-unit conversion, preserving half euros.
- Use the existing dedicated branch and checkout; do not create a second worktree or change the existing upstream main branch.
- User authorizes commit/push to their repository; review before publishing. No upstream writes or public release required.

## Review Focus

- Dialog callbacks receiving 64-bit LPARAM/WPARAM must not truncate pointers.
- Legacy checkbox/radio handling must toggle once and preserve group selection.
- Save cancellation, invalid/corrupt files, write failure and Unicode paths must preserve the active game.
- Large DPI and small screens must retain accessible controls and correct character proportions.
- Installer update/uninstall must preserve user state and the portable package must work from another directory.

### Task 1: Native resources and compiler portability

Files: `tools/generate_resources.py`, `tests/test_resources.py`, `native/tabboz.rc`, `native/tabboz.manifest`, `CMakeLists.txt`, `tools/build-windows.ps1`, existing C/header files.

Interfaces: resource generator produces all 81 original dialog IDs and standard controls; CMake produces `Tabboz.exe` plus native tests; original gameplay function names stay unchanged.

- [x] Write/run failing tests for resource coverage, no Borland classes, sound/bitmap/icon identities and modern control semantics.
- [x] Implement resource conversion preserving IDs, font/layout improvements, and native manifest.
- [x] Correct callback types and obsolete compiler dependencies; run real Release x64 build.
- [x] Verify all resource tests pass and built binary is x64.

### Task 2: Euro UI and persistence

Files: `native/desktop.c`, `native/desktop.h`, `native/money.c`, `native/money.h`, `native/persistence.c`, `tests/native_tests.c`, `zarrosim.c`, `readkey.c`, `tabimg.c`.

Interfaces: `TabbozFormatMoney(unsigned long, char *, size_t)` preserves half units; `TabbozDesktopMessage(HWND,UINT,WPARAM,LPARAM,INT_PTR*)` handles layout; persistence wraps the original key serializer with staged atomic writes and bounded reads.

- [x] Write/run failing euro-format tests, then implement exact euro display and defaults.
- [x] Write/run failing persistence round-trip/invalid-file/write-failure tests; implement AppData storage and Unicode import/export with rollback.
- [x] Implement resizable main view, original avatar scaling, activity navigation, keyboard commands, standard themed dialogs and safe close/new-game.
- [x] Verify representative game actions and all dialog creation using a native harness.

### Task 3: Packaging and host validation

Files: `installer/tabboz.iss`, `tools/package-windows.ps1`, `tools/smoke-windows.ps1`, documentation and CI workflow.

Interfaces: package script emits versioned installer, portable ZIP, corresponding source archive and SHA256SUMS in `dist/`; smoke script writes a machine-readable report and screenshots in `build/validation/`.

- [x] Verify package contents and build an actual installer using existing or workspace-local Inno compiler.
- [x] Run Windows 11 native tests, dialog smoke, save/restart, isolated install/update/uninstall and portable launch.
- [x] Inspect screenshots/DPI cases; inspect final binary resources with REA. Record Windows 10 execution separately as unavailable if no host exists.

### Task 4: Review and publish

Files: `README.md`, `docs/BUILD_WINDOWS.md`, `docs/VALIDATION.md`, `.project/` and final review report.

- [x] Independent whole-change review; fix important findings and rerun affected/full checks.
- [x] Record exact observed results and limitations; validate continuity and Git diff.
- [x] Commit a coherent reviewed checkpoint, preserve both histories, push to the user's repository, verify remote commit.
