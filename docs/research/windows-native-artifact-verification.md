# Native Windows artifact verification

REA Evidence: `ev_024988561b517b6db78fd92691ba1ff951fcc454cca4f2fe102cdd1c26672e6b`.

- Artifact: `build/windows/Tabboz.exe`.
- SHA-256: `90f34a72f9d25d8a81bc6863bca1694f266e6986eb47592b3d0fd0529d85c5eb`.
- Size: 6861312 bytes; format PE32+.
- Resource inventory complete: 642 entries examined, 316 resources, including all 81 dialog templates.
- Embedded splash resource 5001 SHA-256: `c4eae75eab3b6d3cddd7fd2cb606acedc0e029951ca9a84b6741c4550d393bdb`, identical to the supplied PNG.
- Application icon group 1 includes 16, 24, 32, 48, 64, 96, 128 and 256 pixel variants.

GNU objdump reports x86-64 Windows GUI and only these direct DLL imports: COMCTL32, COMDLG32, GDI32, USER32, SHELL32, ADVAPI32, WINMM, ole32, gdiplus, SHLWAPI and KERNEL32. There is no BWCC/browser/runtime redistributable import.

The portable ZIP's Tabboz.exe matches the verified executable byte-for-byte. Installer compilation and isolated Windows 11 install/launch/upgrade/uninstall completed successfully, preserving the test save. GUI normal startup, all twelve activity dialogs and exit after a forced save failure passed. Native integration: 143 checks; Python resource/layout suite: six tests; native euro-format test: passed.

These are static artifact and bounded runtime observations. Windows 10 execution and physical multi-monitor DPI transitions were not tested. See [VALIDATION.md](../VALIDATION.md) for coverage and limitations. The retained REA bundle contains multiple build identities; use the Evidence ID and artifact digest above to select this build.
