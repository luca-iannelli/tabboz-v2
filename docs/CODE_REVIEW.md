# Code review and regressions

An independent read-only review of the complete native port was completed before the first publication checkpoint. A first reviewer returned two findings but could not finish because its account usage limit was reached; an available Codex reviewer completed the full assessment.

The following findings were reproduced before correction, then verified with passing regressions:

- Closing a scooter purchase preview with X bypassed the original cancel handler and could leave a purchase selected. Secondary close now dispatches the original cancellation command before ending the dialog; the old scooter and money remain intact.
- Artwork/static-control order split clothing radio choices into multiple groups. Converted legacy dialogs now have a single logical radio group; selecting across the affected product positions clears the previous choice.
- The native validator and loader restricted unsigned money/score values to INT32_MAX. They now preserve the complete serialized uint32 domain. Original gameplay arithmetic remains unchanged.
- A small main window at simulated 200% DPI clipped bottom actions without a visible scrollbar or keyboard focus scrolling. The window now exposes scrolling and brings keyboard-focused controls into view.
- The original Extreme filter uses catalogue index 4, which must remain valid. Invalid activity indices are rejected before indexing the status table.
- A long non-checksummed Sesso field could overflow an old temporary buffer. Validation rejects malformed nonempty values and the loader reads only the first character without a string copy.
- Visual inspection and resource tests identified clipped settings text and overlapping school actions. Dedicated settings geometry and school spacing correct those issues.

The error message for a failed Save As now names the attempted destination. Six Python tests, the native euro-format test and 143 native integration assertions pass after these changes.

Actual Windows 10 execution, physical multi-monitor DPI transitions and live CI execution were excluded from the read-only review. Windows 11 runtime/installer results and the current CI status are recorded separately in VALIDATION.md.
