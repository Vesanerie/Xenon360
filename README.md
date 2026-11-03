# Xenon360

[![Latest release](https://img.shields.io/github/v/release/Vesanerie/Xenon360?label=release&color=blue)](https://github.com/Vesanerie/Xenon360/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/Vesanerie/Xenon360/total?color=green)](https://github.com/Vesanerie/Xenon360/releases)
[![License](https://img.shields.io/badge/license-GPL--2.0--or--later-blue)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%2011%2B%20Apple%20Silicon-lightgrey)](#)

**Play Clone Hero, YARG, Rock Band on your Mac with your old Xbox 360 USB guitar or wireless controller.**

Plug in your Guitar Hero X-plorer (or any Xbox 360 wired controller), or connect an **official Microsoft Xbox 360 Wireless Receiver** for up to 4 wireless controllers. Install Xenon360, open Clone Hero. That's it. Your frets, your strums, your tilt, your whammy: all reach the game through the keyboard (or as a virtual gamepad with analog triggers).

Apple Silicon (M1/M2/M3/M4) only. macOS 11 Big Sur or later.

Spiritual successor to [Tattiebogle 360Controller](https://tattiebogle.net/ProjectRoot/Xbox360Controller/OsxDriver), abandoned in 2013 and incompatible with Apple Silicon.

---

## Install (the easy way)

1. Go to the [Releases page](https://github.com/Vesanerie/Xenon360/releases) and download the latest **Xenon360.pkg**.
2. Double-click the `.pkg` file. The Apple Installer wizard opens.
3. Click through Welcome → License → Install. Enter your Mac password when asked.
4. Done. The installer set everything up.

What got installed:
- `/Applications/Xenon360.app` (the menu bar app)
- A small background watcher in `~/Library/LaunchAgents/` that auto-starts Xenon360 whenever you open Clone Hero, and quits it when you close the game
- A copy of the bundled USB library (libusb) so you don't need Homebrew

The installer is **signed by Apple Developer ID and notarized**, so macOS Gatekeeper opens it without complaints.

## First launch (1-minute setup)

1. Plug your **Xbox 360 USB guitar** into the Mac.
2. Open **Clone Hero**.
3. macOS pops up: *"Xenon360 would like to control this computer using accessibility features."*
4. Click **Open System Settings**.
5. Find **Xenon360** in the list and **toggle it ON**. macOS may ask for your password.
6. Go back to Clone Hero. Strum.

This permission step only happens once per Mac. macOS remembers it forever after.

## How it works (in 30 seconds)

```
You strum the guitar
        ↓
USB packet arrives on the Mac
        ↓
Xenon360 reads it (in the background, via the menu bar app)
        ↓
It translates the fret/strum/tilt to a keyboard event (A, S, J, K, L, arrows, space...)
        ↓
Clone Hero receives the keypress and plays the note
```

The watcher daemon means you never have to think about Xenon360. Open the game, Xenon360 starts itself. Close the game, Xenon360 quits itself. Battery, USB, everything is clean.

**Games detected for auto-launch**: Clone Hero, YARG. For any other game: open `/Applications/Xenon360.app` manually before launching it.

### Default keyboard mapping (Clone Hero compatible)

| Guitar / Controller | Keyboard      |
|---------------------|---------------|
| Green fret (A)      | A             |
| Red fret (B)        | S             |
| Yellow fret (Y)     | J             |
| Blue fret (X)       | K             |
| Orange fret (LB)    | L             |
| **Right Bumper (RB)** | **R**       |
| **Guide (Xbox logo)** | **G**       |
| **Left Stick Click (L3)** | **C**   |
| **Right Stick Click (R3)** | **V**  |
| Strum up / D-pad Up | Arrow up      |
| Strum down / D-pad Down | Arrow down |
| **D-pad Left**      | **Arrow left** |
| **D-pad Right**     | **Arrow right** |
| Tilt (star power)   | Space         |
| Start               | Enter         |
| Back                | Escape        |

These are the Clone Hero defaults, so you don't need to configure anything.

## Supported devices (126 of them)

Anything in the [Linux kernel `xpad` driver table](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c) works:

- **Guitars and drums**: RedOctane Guitar Hero (X-plorer, World Tour, Warriors of Rock), Harmonix Rock Band Guitar and Drumkit, Ion Drum Rocker, Mad Catz Wireless Guitar, dancepads.
- **Controllers**: official Microsoft Xbox 360, Mad Catz, PDP, Hori, Razer, Logitech (F310/F510/F710), PowerA, Joytech, Pelican, Afterglow, Rock Candy, Snakebyte, GameSir, Nacon, BigBen, Saitek, Thrustmaster GPX, Amazon Game Controller, GPD Win 2, Wooting Legacy.
- **Fightsticks**: Mad Catz SFIV/SE/TE/TES+/TE2/Brawlstick, Hori Real Arcade Pro, Razer Atrox, Razer Onza, MLG Pro Circuit, Mortal Kombat FightStick.
- **Wheels**: Mad Catz MC2 MicroCON Racing Wheel, Thrustmaster Ferrari 458.

If your device shows up as "vendor-specific USB" but isn't in the list, run from a terminal: `Xenon360.app/Contents/Resources/xenon360 -v` and open a GitHub issue with the VID/PID it prints. Easy to add.

### Wireless receiver specifics

Supported receiver VIDs/PIDs:
- `0x045E:0x0291` — Microsoft Xbox 360 Wireless Receiver
- `0x045E:0x0719` — Microsoft Xbox 360 Wireless Receiver for Windows
- `0x045E:0x02A1` — Microsoft Xbox 360 Wireless Receiver (variant)
- `0x1BAD:0x0719` — Mad Catz Xbox 360 Wireless Receiver clone

If your receiver isn't detected, run with `-v` and check the output for the VID/PID.

## Uninstall

```bash
/Applications/Xenon360.app/Contents/Resources/uninstall.sh
```

This removes the LaunchAgent, the staging files in `~/Library/Application Support/Xenon360/`, and quits Xenon360.app. You can then move `Xenon360.app` from `/Applications` to the Trash.

## Troubleshooting

**Nothing happens when I strum.**
Check the menu bar (top right of your screen). The Xenon360 icon should be there. Click it:
- If it says "No guitar detected": unplug/replug the USB.
- If you see a ⚠ warning about Accessibility: click "Open Accessibility Settings" and toggle Xenon360 ON.
- If Clone Hero is the focused window but keys still don't register, fully quit Clone Hero and reopen it.

**Watcher isn't starting Xenon360 automatically.**
Check the log: `tail -f /tmp/xenon360-watcher.log`. It should say *"Launched Xenon360.app"* within 3 seconds of opening Clone Hero. If not, the watcher might not be loaded: run the installer .pkg again.

**The Accessibility popup didn't show up.**
That can happen if macOS thinks it already asked. Open *Settings > Privacy & Security > Accessibility*, click the **+** button, navigate to `/Applications/Xenon360.app`, and add it manually. Toggle it ON.

**Game not in the auto-launch list (anything other than Clone Hero / YARG).**
Open `/Applications/Xenon360.app` manually before launching it. Quit it from the menu bar when done. Or open an issue with the game's name and I'll add it.

### Wireless receiver troubleshooting

**Receiver detected but controller won't sync / LED keeps blinking:**
- The receiver may need a keep-alive signal. Xenon360 sends this automatically every 2 seconds. If LED still blinks, try unplugging/replugging the receiver.
- Make sure you're pressing the **sync button on the receiver** first, then the **Guide button on the controller**.

**"claim_interface failed" error in verbose output:**
- macOS's native driver may have claimed the interface. Try unplugging/replugging the receiver, or reboot.

**Controller connects but no input in RetroArch / Steam Input:**
- Use **gamepad mode** (`-g` flag) which exposes triggers as analog axes (Z/Rz).
- Requires SIP/AMFI disabled (see Advanced section above).
- In RetroArch: Settings → Input → Port 1 Controls → Device Index → select "Xenon360 Virtual Guitar Slot 1".

**LT/RT triggers not working in gamepad mode:**
- Triggers are mapped as axes Z (LT) and Rz (RT), not buttons. Map them in your emulator/game as "Axis 2-" / "Axis 5+" or similar.

## Advanced

### ⚠️ Important: Keyboard mode vs Gamepad mode

| Mode | Command | LT/RT (Triggers) | Whammy/Tilt | SIP/AMFI |
|------|---------|------------------|-------------|----------|
| **Keyboard (default)** | `xenon360` | ❌ Not supported | Binary (keys) | ✅ Not needed |
| **Gamepad** | `xenon360 -g` | ✅ **Analog axes (Z/Rz)** | ✅ **Analog** | ❌ **Required off** |

**For RetroArch, Steam Input, or any game needing analog triggers: you MUST use gamepad mode (`-g`) with SIP/AMFI disabled.**

---

### Analog whammy, tilt, and triggers (gamepad mode)

The keyboard mode is fine for 99% of players. But if you want **continuous analog whammy/tilt** (instead of binary keypress) or **analog triggers (LT/RT)**, there's a virtual HID gamepad mode invoked with the `-g` flag on the CLI.

```bash
/Applications/Xenon360.app/Contents/Resources/xenon360 -g
```

**Gamepad mode features:**
- 6 axes: LX, LY, LT (Z), RX, RY, RT (Rz)
- Hat switch for D-pad
- 12 buttons: A, B, X, Y, LB, RB, Back, Start, Guide, L3, R3, (star power)

**Requirements:** Disabling SIP and AMFI on your Mac (see below). Worth it for hardcore players / RetroArch / Steam Input use only.

**SIP/AMFI disable procedure:**
1. Reboot into Recovery (Shut down, hold power button)
2. Utilities → Terminal → `csrutil disable`
3. Normal reboot
4. `sudo nvram boot-args="amfi_get_out_of_my_way=1"`
5. Reboot

To re-enable: Recovery → `csrutil enable` → `sudo nvram -d boot-args` → Reboot.

> **Nota:** Estes comandos (`csrutil disable` e `amfi_get_out_of_my_way=1`) diminuem a segurança do macOS. São obrigatórios apenas para que os gatilhos LT/RT funcionem como eixos analógicos no modo `-g`. Em modo teclado padrão (`xenon360`), não são necessários.

### Wireless receiver (✅ tested on Apple Silicon)

If you plug in a Microsoft Xbox 360 Wireless Receiver (VID `0x045E`, PID `0x0291`, `0x0719` or `0x02A1`), Xenon360 auto-detects it and listens to all 4 slots simultaneously. Up to 4 wireless controllers can be used.

**Features:**
- All 10 buttons (A, B, X, Y, LB, RB, Back, Start, Guide, L3, R3)
- Full D-pad (Up, Down, Left, Right)
- Analog triggers (LT, RT) — exposed as Z/Rz axes in gamepad mode
- Analog sticks (LX, LY, RX, RY)
- LED keep-alive: player quadrant stays solid (no more blinking)

> **Note**: Tested on Mac M1 with official Microsoft Wireless Receiver for Windows (PID 0x0719). If you use a different variant, run `Xenon360.app/Contents/Resources/xenon360 -v` and open an issue with the VID/PID if anything behaves differently.

### Build from source

```bash
brew install libusb
git clone https://github.com/Vesanerie/Xenon360.git
cd Xenon360
make app                          # builds Xenon360.app (ad-hoc signed)
cd autolaunch && ./install.sh     # set up the auto-launch watcher
```

The full signed/notarized release pipeline is documented in [RELEASE.md](RELEASE.md).

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for the full history of releases.

## Status and roadmap

- [x] **v0.1**: wired USB read + keyboard injection
- [x] **v0.2**: virtual HID gamepad (analog whammy/tilt, requires SIP off)
- [x] **v0.3**: Xbox 360 USB wireless receiver, 4-slot multiplex (tested on Apple Silicon)
- [x] **v0.3.1**: signed PKG installer, auto-launch watcher, end-to-end tested
- [ ] **v0.4**: NSWorkspace event-driven autolaunch (replace 3s polling), drums-specific support
- [ ] **v1.0**: DriverKit migration for distribution without any system modifications

## How this was built

This project was developed with significant assistance from Claude (Anthropic's AI). I designed the goal, debugged the macOS edge cases on real hardware (Guitar Hero X-plorer + Clone Hero), made the architectural calls, and tested every change end-to-end on my own machine. Claude generated most of the C and shell code under my direction, helped untangle TCC / codesigning / notarization rabbit holes faster than I could solo, and proposed the audit that caught the bundle-relocation bug before any user hit it.

Disclosing upfront because some folks care about provenance, and you should know. The protocol knowledge itself comes from human-written sources, credited below.

## Credits

Xbox 360 USB protocol learned from:
- [Parts Not Included](https://www.partsnotincluded.com/understanding-the-xbox-360-wired-controllers-usb-data/)
- [Linux kernel xpad driver](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c)
- [Tattiebogle USB info](https://tattiebogle.net/ProjectRoot/Xbox360Controller/UsbInfo)
- [Free60 wiki](https://free60.org/)

## License

GPL-2.0-or-later. See [LICENSE](LICENSE).

This project bundles **libusb** (LGPL-2.1, source at https://github.com/libusb/libusb).

---

# 🎸 HAVE FUN. BE ROCK. 🎸

Built by [Vesanerie](https://github.com/Vesanerie) for everyone with an old plastic guitar gathering dust in a closet. Pull requests welcome.
