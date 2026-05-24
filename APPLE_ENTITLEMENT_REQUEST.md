# Apple DriverKit Entitlement Request : Xenon360

Document pour soumettre la demande d'entitlements DriverKit sur https://developer.apple.com/contact/request/system-extension/

## Infos projet (a saisir dans le formulaire)

| Champ | Valeur |
|---|---|
| **Project name** | Xenon360 |
| **Team ID** | (ton Team ID Apple Developer, visible sur developer.apple.com > Membership) |
| **Bundle ID (container app)** | `dev.vesanerie.xenon360` |
| **Bundle ID (dext)** | `dev.vesanerie.xenon360.dext` |
| **GitHub repo** | https://github.com/Vesanerie/Xenon360 |
| **Distribution** | Free, outside Mac App Store (Developer ID signed, notarized) |
| **Target macOS** | 13.0 (Ventura) and later, Apple Silicon + Intel |
| **License** | GPL-2.0-or-later |

## Entitlements demandes

Coche les suivants dans le formulaire :

- [x] `com.apple.developer.driverkit`
- [x] `com.apple.developer.driverkit.transport.usb`
- [x] `com.apple.developer.driverkit.family.hid.device`
- [x] `com.apple.developer.driverkit.family.hid.eventservice`
- [x] `com.apple.developer.driverkit.userclient-access`
- [x] `com.apple.developer.system-extension.install` (sur l'app container)

## USB devices a claim (a fournir dans la section "USB matching")

```
Microsoft Corp.       VID 0x045E
  Xbox 360 Wireless Receiver for Windows    PID 0x0291
  Xbox 360 Wireless Receiver for Windows    PID 0x02A1
  Xbox 360 Wireless Receiver for Windows    PID 0x0719

RedOctane Inc.        VID 0x1430
  Guitar Hero X-plorer (wired)              PID 0x4748
  Guitar Hero (wired)                       PID 0x474B
  Guitar Hero (wired)                       PID 0x474C
  Guitar Hero (wired)                       PID 0x4734

Harmonix Music        VID 0x1BAD
  Rock Band Guitar (wired)                  PID 0x0002
  Rock Band Drumkit (wired)                 PID 0x0003
  Ion Drum Rocker                           PID 0x0130
  Wireless Guitar                           PID 0x074B

Mad Catz Inc.         VID 0x0738
  Guitar Hero (wired)                       PID 0x4540

RedOctane PS3 compat  VID 0x12BA
  Guitar Hero (Xbox-compatible)             PID 0x0100
```

## Justification text (a copier-coller dans "Detailed description")

```
Project name: Xenon360
URL: https://github.com/Vesanerie/Xenon360
Distribution: Free, open-source (GPL-2), outside Mac App Store, Developer ID signed and notarized.

WHAT THE DRIVER DOES

Xenon360 is a userspace replacement for the abandoned Tattiebogle "360Controller" kernel extension (last updated 2013, marked "no plans for Big Sur support" since 2020). It provides macOS support for Xbox 360 USB peripherals: wireless receivers, Guitar Hero / Rock Band guitars, drum kits, and dance pads. These devices use a vendor-specific USB protocol (interface class 0xFF, subclass 0x5D) and are not handled by any standard macOS class driver.

The dext claims a specific list of well-known VID/PIDs (Microsoft Xbox 360 wireless receivers, RedOctane Guitar Hero guitars, Harmonix Rock Band instruments, Mad Catz, Ion Drum Rocker) and exposes their inputs as a standard HID gamepad via IOUserHIDDevice. This allows rhythm games (Clone Hero, YARG, Fortnite Festival) and any game using GameController.framework or HID joysticks to recognize and use these instruments.

WHY THIS NEEDS DRIVERKIT

The Xbox 360 USB protocol is vendor-specific. macOS provides no generic class driver that handles it. The only options are:

1. A user-space libusb application that injects keyboard events (limited: cannot expose analog axes like the whammy bar or tilt sensor, breaks games that require gamepad input).
2. A virtual HID device created via IOHIDUserDeviceCreate. Requires `com.apple.developer.hid.virtual.device` entitlement, equivalent gatekeeping.
3. A DriverKit USB Transport Extension that claims the device and republishes input via HIDDriverKit. This is the proper, supported path.

WHAT macOS DOES NOT ALREADY PROVIDE

macOS Sequoia 15 added native support for the standard Xbox 360 wired controller (VID 0x045E PID 0x028E) via GameController.framework. This support DOES NOT cover:

- The Microsoft Xbox 360 Wireless Receiver (VID 0x045E PID 0x0291/0x02A1/0x0719), which uses a multiplexed protocol for up to 4 controllers
- Third-party Xbox 360 controllers (Mad Catz, Hori, Razer)
- Rhythm game peripherals: Guitar Hero / Rock Band guitars (5-fret, 6-fret), drum kits, DJ Hero turntable, dance pads
- The Xbox 360 ChatPad keyboard accessory
- Custom rumble patterns and LED ring control

These devices have an active user base (a 2024 GitHub issue search shows ongoing demand on the 360Controller repo despite its abandonment) and no actively maintained driver exists for Apple Silicon Macs as of 2026.

PRECEDENT

The pqrs-org Karabiner-Elements project (https://github.com/pqrs-org/Karabiner-Elements) is an individual-developer open-source DriverKit project that was granted similar entitlements (`com.apple.developer.driverkit`, `com.apple.developer.driverkit.family.hid.device`, `com.apple.developer.driverkit.family.hid.eventservice`). Karabiner exposes virtual HID keyboards and pointing devices. Xenon360 would similarly expose virtual HID gamepads, with a clear scope limited to a hardcoded VID/PID matching table.

SCOPE COMMITMENT

The transport.usb entitlement will be requested only for the specific VID/PID list provided (Xbox 360 family devices). It will NOT request any generic claim. The HID device exposed by the dext will be a standard gamepad descriptor (Usage Page 0x01, Usage 0x05), with no claim on system-wide HID services. The dext will be signed with Developer ID, notarized, and distributed via GitHub Releases for transparency. Source code is fully public (GPL-2-or-later).

CONTACT

Maintainer: Valentin Mardoukhaev (individual developer)
Apple Developer Program: active, paid subscription
Email: mardouv2@gmail.com
GitHub: https://github.com/Vesanerie
Project: https://github.com/Vesanerie/Xenon360
```

## Comment soumettre

1. Ouvre https://developer.apple.com/contact/request/system-extension/
2. Login avec ton Apple ID Developer (le meme que Gesturo)
3. Selectionne :
   - Capability category : **System Extensions and DriverKit**
   - Type of System Extension : **DriverKit**
4. Coche les entitlements listes ci-dessus
5. Copie-colle la justification dans le champ description
6. Liste les VID/PID dans le champ devices supportes
7. Mentionne https://github.com/Vesanerie/Xenon360 comme reference code
8. Submit

## Soumission effectuee

| Champ | Valeur |
|---|---|
| Date soumission | 2026-05-24 |
| Request ID Apple | **3A5XNCKQW4** |
| Type | Virtual HID |
| Team ID | Q4W8LZ8636 |
| Email contact | mardouv2@gmail.com |

## Apres soumission

- Apple repond par email a mardouv2@gmail.com
- Delai typique : 2 semaines a 3 mois (parfois jusqu'a 10 mois pour les cas complexes type eGPU)
- Si Apple demande plus d'infos : re-justifier en restant focus sur le scope precis + precedent Karabiner
- Si Apple refuse : on peut redemander avec ajustement (entite commerciale, scope reduit, etc.)
- Si Apple accorde : l'entitlement apparait dans Certificates, Identifiers & Profiles > ton App ID > Capabilities. Regenere le provisioning profile, et le dext peut etre signe et distribue.

## En attendant la reponse

- La v0.1 keyboard continue de marcher pour tout le monde (zero modif systeme)
- La v0.2 SIP off reste documentee pour les power users
- Tu peux commencer a coder la v1.0 DriverKit en local (test avec `systemextensionsctl developer on` + SIP off)
