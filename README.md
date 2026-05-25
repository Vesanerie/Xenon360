# Xenon360

Userspace driver to use an Xbox 360 USB guitar/controller on Apple Silicon Mac (M1/M2/M3/M4) with Clone Hero, YARG, and any game that accepts keyboard input.

Spiritual successor to [Tattiebogle 360Controller](https://tattiebogle.net/ProjectRoot/Xbox360Controller/OsxDriver), abandoned in 2013 and incompatible with Apple Silicon.

## Status

- **v0.1**: wired USB read + keyboard injection (tested on X-plorer guitar).
- **v0.2**: virtual HID gamepad (analog whammy/tilt, requires SIP/AMFI off).
- **v0.3 alpha**: Xbox 360 USB wireless receiver support, 4-slot multiplex. Code written from the xpad.c reference but **not yet tested** by the maintainer (no wireless hardware on hand). GitHub issues / test reports very welcome.

Remaining roadmap: dedicated drums support, DriverKit migration for distribution without system modifications.

## Supported devices (126 entries, xpad.c table)

All common wired Xbox 360 peripherals are recognized:

**Official and third-party controllers**: Microsoft, Mad Catz, PDP, Hori, Razer, Logitech (F310/F510/F710), PowerA, Joytech, Pelican, Afterglow, Rock Candy, Snakebyte, GameSir, Nacon, BigBen, Saitek, Thrustmaster GPX, Amazon Game Controller, GPD Win 2, Wooting Legacy.

**Rhythm game instruments**:
- RedOctane Guitar Hero (X-plorer, World Tour, Warriors of Rock, other variants)
- Harmonix Rock Band Guitar and Drumkit
- Ion Drum Rocker
- Mad Catz Wireless Guitar
- Dancepads (HSM3, Honey Bee)

**Fightsticks and arcade**: Mad Catz SFIV/SE/TE/TES+/TE2/Brawlstick, Hori Real Arcade Pro, Razer Atrox, Razer Onza, MLG Pro Circuit, Mortal Kombat FightStick.

**Wheels**: Mad Catz MC2 MicroCON Racing Wheel, Thrustmaster Ferrari 458.

The binary also scans all vendor-specific USB devices and displays their VID/PID if nothing known is detected (makes it easy to add new models).

## Install

```bash
brew install libusb
git clone https://github.com/Vesanerie/Xenon360.git
cd Xenon360
make
```

## Usage

Plug in your guitar via USB, then:

```bash
./xenon360
```

Keyboard mapping (compatible with Clone Hero by default):

| Guitar          | Keyboard |
|-----------------|----------|
| Green fret      | A        |
| Red fret        | S        |
| Yellow fret     | J        |
| Blue fret       | K        |
| Orange fret     | L        |
| Strum up        | Arrow up |
| Strum down      | Arrow down |
| Tilt (star power) | Space |
| Start           | Enter    |
| Back            | Escape   |

Verbose mode for protocol debugging:

```bash
./xenon360 -v
```

## macOS Permissions

On first launch, macOS requests **Accessibility** authorization for Terminal (required to inject keyboard events).

Settings > Privacy & Security > Accessibility > enable Terminal.

## Gamepad mode (v0.2): analog whammy and tilt

Keyboard mode cannot send the whammy continuously (keys are binary). For analog bending, run with `-g`:

```bash
./xenon360 -g
```

But beware: on Apple Silicon, creating a virtual HID requires disabling SIP + AMFI. Procedure:

### 1. csrutil disable (in Recovery)

1. Shut down your Mac
2. Hold the power button until you see "Loading startup options..."
3. Options > your account > password
4. Utilities menu > Terminal
5. `csrutil disable` then `y` and admin password
6. Reboot

### 2. AMFI disable

Once reconnected in normal mode:

```bash
sudo nvram boot-args="amfi_get_out_of_my_way=0x1 ipc_control_port_options=0"
sudo shutdown -r now
```

### 3. Verification

```bash
csrutil status        # must show "disabled"
nvram boot-args       # must show amfi_get_out_of_my_way=0x1
```

### 4. Launch in gamepad mode

```bash
./xenon360 -g
```

The "Xenon360 Virtual Guitar" device appears in Clone Hero (Settings > Controls).

### Revert (re-enable SIP)

Reverse everything:

```bash
sudo nvram -d boot-args
```

Then reboot into Recovery and `csrutil enable`.

## Wireless receiver mode (v0.3, untested)

If you plug in a Microsoft Xbox 360 Wireless Receiver (VID `0x045E`, PID `0x0291`, `0x0719` or `0x02A1`), the binary auto-detects it, claims all 4 data interfaces, and starts listening on the 4 slots simultaneously.

On each controller connection:
- The corresponding LED quadrant lights up (slot 1 = top-left, 2 = top-right, 3 = bottom-left, 4 = bottom-right).
- The slot status is logged to stdout.

In keyboard mode (default), only **slot 1** injects keys (to avoid collisions when multiple players press the same fret). For multi-player, use `--gamepad`:

```bash
./xenon360 -g
```

Each slot then exposes its own virtual HID device named `Xenon360 Virtual Guitar Slot N`, with a distinct ProductID (`0x5860 + slot`) so Clone Hero can bind them independently.

**Limitation**: this code path has not been tested against real hardware (the maintainer only has a wired X-plorer). It is based on the `xpad.c` Linux kernel driver protocol. Please open a GitHub issue if you test it, success or failure.

## Roadmap

- [x] v0.1: keyboard injection for wired Clone Hero guitar
- [x] v0.2: Virtual HID gamepad via IOHIDUserDevice (analog whammy/tilt, requires SIP off)
- [x] v0.3: Xbox 360 USB wireless receiver support, 4-slot multiplex (alpha, untested)
- [ ] v0.4: Swift menu bar app + auto-launch
- [ ] v1.0: DriverKit dext migration (zero system modification, Mac App Store distributable)

## Architecture

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) (TODO).

## Credits

Xbox 360 USB protocol documented by:
- [Parts Not Included](https://www.partsnotincluded.com/understanding-the-xbox-360-wired-controllers-usb-data/)
- [Linux kernel xpad driver](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c)
- [Tattiebogle USB info](https://tattiebogle.net/ProjectRoot/Xbox360Controller/UsbInfo)
- [Free60 wiki](https://free60.org/)

## License

GPL-2.0-or-later
