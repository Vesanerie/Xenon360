# Changelog

All notable changes to Xenon360 are documented here. Format inspired by
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [v0.3.1] - 2026-05-26

First publicly distributed release. Signed with Apple Developer ID and
notarized so Gatekeeper accepts it without warnings.

### Added
- **Signed PKG installer** (`Xenon360.pkg`). Drops the .app in
  `/Applications`, sets up the auto-launch watcher, ready to use.
- **Auto-launch watcher** as a LaunchAgent. Detects Clone Hero, YARG,
  Fortnite Festival, and Rock Band 4 by their process path and
  spawns/quits Xenon360.app automatically with the game.
- **Wireless receiver support** (alpha, untested on real hardware).
  Detects Microsoft Xbox 360 Wireless Receivers (VID 0x045E + PIDs
  0x0291/0x0719/0x02A1, plus the Mad Catz clone 0x1BAD:0x0719), claims
  the 4 data interfaces, dispatches via async libusb, and exposes one
  virtual HID per slot in gamepad mode.
- **Auto-prompt for Accessibility** at first launch. The .app calls
  `AXIsProcessTrustedWithOptions(prompt: true)` so macOS shows its
  standard permission popup, attributed to the bundle ID. Once granted,
  it never prompts again.
- **126 supported devices** from the upstream Linux `xpad.c` table:
  guitars, drums, dancepads, controllers, fightsticks, wheels.
- **Internal test harness** (`make test`). 21 assertions covering the
  wireless dispatch logic, packet parsing, and slot collision guard.
- **End-to-end PKG install test** verified on a clean machine state:
  Gatekeeper accept, payload landed at `/Applications`, postinstall
  staged the watcher, LaunchAgent loaded, Xenon360.app launched on
  Clone Hero open, keys injected into the game.

### Fixed
- **TCC denial under launchd** root cause. The bundled CLI was
  linker-signed with identifier `xenon360`, which TCC treated as a
  separate identity from the parent .app bundle (auth=DENIED). Now
  re-signed with `dev.vesanerie.xenon360.cli` before the bundle seal,
  so Accessibility grants on the bundle propagate to the spawned CLI.
- **PKG bundle relocation** failure. macOS Installer was detecting
  any existing Xenon360.app on the user's machine (e.g. in
  `~/Downloads/`) and trying to update in place, which failed under
  TCC when the target path was protected (Desktop, Documents). The
  PKG now declares `BundleIsRelocatable=false` and always installs to
  `/Applications`.
- **Watcher path resolution**. The previous version hardcoded the .app
  path to `~/Library/Application Support/`, which is correct for the
  source build but wrong for the PKG install (which uses
  `/Applications/`). The watcher now probes both locations and an
  optional `XENON_APP` env override.

### Changed
- Bundled `libusb` inside the .app at
  `Contents/Frameworks/libusb-1.0.0.dylib`, so end users don't need
  Homebrew. Load path rewritten via `install_name_tool`.
- Watcher launches the .app via AppleEvents
  (`osascript 'tell ... to launch'`) instead of `open`, so the
  responsible-process attribution falls on `loginwindow` rather than
  the launchd-spawned bash. Avoids a known TCC inheritance edge case.
- Build pipeline supports both ad-hoc (dev) and Developer ID (release)
  signing via the `SIGNING_IDENTITY` env var.

### Known limitations
- **Apple Silicon only** (M1/M2/M3/M4). The arm64 build won't run on
  Intel Macs. Universal binary may come in a future release.
- **macOS 11 Big Sur minimum**. Developed and tested on Sequoia /
  Tahoe; older macOS versions are not regression-tested.
- **Wireless receiver code path is untested on real hardware**.

## Earlier alpha releases (pre-v0.3.1)

The project lived as a development tree before v0.3.1, with these
milestones tagged loosely in commit history:

- **v0.1** (April 2026): wired USB read + keyboard injection.
- **v0.2** (May 2026): virtual HID gamepad (analog whammy/tilt,
  requires SIP/AMFI disabled).
- **v0.3 alpha** (May 2026): wireless receiver protocol implementation.
