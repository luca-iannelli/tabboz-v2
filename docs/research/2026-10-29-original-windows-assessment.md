# Original Windows build assessment

## Identity and acquired material

- Source: https://github.com/andreax79/tabboz
- Pinned source commit: `142fa7de71fe3d4700fa0027f18d45c6ccdaa9be`.
- Original executable: `release/tabboz.exe`, 2,412,544 bytes.
- SHA-256: `002f3a76801adb2e1e891426ef7ae7f6f15895b853ef20a22573794d912bb683`.
- License: upstream `LICENSE`, GPL-3.0-or-later as stated in source headers.
- Work was performed in Ubuntu WSL on a Windows 11 host. The original executable has not been run.

## REA observations

REA 6.7.0 `inspect_pe_resources` Evidence:
`ev_7fcc9d6c10538dfb4c959833c8f396b85818cd9d16c97ad15d85ea3524cc2b71`.

Complete bounded static resource inventory: 575 directory entries examined, 283 resources:

| Resource type | Count |
| --- | ---: |
| Cursor images | 2 |
| Bitmaps | 123 |
| Icon images | 23 |
| Menus | 1 |
| Dialogs | 81 |
| String blocks | 27 |
| Cursor groups | 2 |
| Icon groups | 23 |
| Version info | 1 |

REA `open_binary` identified PE, executable, x86. REA `inspect_with_objdump` file-header Evidence:
`ev_6b2c11b893b79dadd1db2376f0a2da1999b907849ee55d66d2d7a9b5f67827a0`.
The observed file format is `pei-i386`, architecture i386, entry point `0x00410000`; the provider exited 0, its output was complete, and its before/after digest check found no artifact change.

Limitations: static resource payloads are opaque except icon-group relationships. This does not establish UI appearance, dynamic behavior, DLL loading, save compatibility, or matching behavior between binary and source. Native decompiler providers Ghidra and Hopper are unavailable. Full source is present, so gameplay investigation should use source rather than install an unnecessary decompiler.

## Source observations, separate from REA

- About 9,200 lines of C across the top-level modules, including disabled/obsolete modules. The active source set is recorded in `Makefile`.
- `resources/dialogs/dialogs.rc` contains 81 dialog templates, matching the original executable's dialog count. It includes Borland classes: 362 `BorShade`, 170 `BorBtn`, 58 `BorRadio`, 157 `BorStatic`, 91 `BorCheck`, plus dialog classes and two custom bitmap views.
- `os.h` already selects the native Windows path outside Emscripten, but `zarrosim.c` requires obsolete `bwcc.h` and `dos.h`. It registers BWCC during startup. The current build recipe targets Emscripten/Novantotto.
- The C modules hold gameplay, catalogues, prices, randomness, event handling, and character composition. Preserving them is substantially safer for fidelity than rewriting these rules.
- Several callbacks use `WORD`/`LONG` parameters or `BOOL` returns and are cast to Windows callback types. These need genuine pointer-width corrections for x64, rather than additional casts.
- `tabimg.c` draws original bitmap layers with GDI; these assets can be retained. Bitmap view paint lifecycle and DPI sizing need examination during implementation.
- Native persistence in `readkey.c` uses HKCU `Software\\Obscured Truckware\\Tabboz Simulator 32` and optionally INI-style `.tbz` files. State values include legacy encoded/checksummed numbers. Do not promise imported-save compatibility until a real fixture round trip passes.
- 28 `.wav` assets exist in `resources/wavs`; playback currently assumes the current working directory.
- The timer only advances the day under a specific minimized-window condition. UI modernization must not accidentally change the frequency of gameplay events.
- A legacy exit option invokes Windows shutdown. The proposed modern exit flow saves and closes the game; operating-system shutdown is outside the requested gameplay preservation boundary.
- The existing native network feature is compile-time disabled. No network feature is needed for this port.

## Host tools observed

Files exist under Windows Visual Studio 2022 Build Tools: MSVC 14.42.34433 x64 compiler, developer command environment, and bundled CMake. Windows SDK resource compilers 10.0.22621.0 and 10.0.19041.0 exist. PowerShell 5.1.26100.9711 ran successfully after sandbox escalation. Finding the files does not by itself verify a complete build environment.

No Linux cross compiler, CMake, NSIS, Wine, or standalone `rea` command was found on this shell PATH. REA MCP is connected and usable. Inno Setup and a Windows 10 test machine have not been verified.

## Technical references

- Native controls and visual styles: https://learn.microsoft.com/en-us/windows/win32/controls/visual-styles-overview
- Per-monitor DPI and bitmap/layout handling: https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
- Per-user installer behavior: https://jrsoftware.org/ishelp/topic_admininstallmode.htm
- Installer OS version declaration: https://jrsoftware.org/ishelp/topic_setup_minversion.htm
