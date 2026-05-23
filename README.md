# Xenon360

Driver userspace pour utiliser une guitare/manette Xbox 360 USB sur Mac Apple Silicon (M1/M2/M3/M4) avec Clone Hero, YARG, et tout jeu qui accepte le clavier.

Successeur spirituel de [Tattiebogle 360Controller](https://tattiebogle.net/ProjectRoot/Xbox360Controller/OsxDriver), abandonné en 2013 et incompatible avec Apple Silicon.

## Statut

**v0.1 alpha** : lecture USB + injection clavier. Cible : guitare 5 frettes filaire Xbox 360 pour Clone Hero.

Roadmap : virtual HID gamepad (whammy/tilt analogiques), support wireless receiver, support drums, migration DriverKit pour distribution sans modif système.

## Devices supportés (v0.1)

- Microsoft Xbox 360 Controller wired (0x045E:0x028E)
- RedOctane Guitar Hero wired (0x1430:0x4748, 0x474B, 0x474C, 0x4734)
- Harmonix Rock Band Guitar (0x1BAD:0x0002)
- Harmonix Rock Band Drums (0x1BAD:0x0003)
- Ion Drum Rocker (0x1BAD:0x0130)
- Mad Catz Guitar Hero (0x0738:0x4540)

Le binaire scanne aussi tous les devices USB vendor-specific et affiche leur VID/PID si rien de connu n'est detecte.

## Install

```bash
brew install libusb
git clone https://github.com/Vesanerie/Xenon360.git
cd Xenon360
make
```

## Usage

Branche ta guitare en USB, puis :

```bash
./xenon360
```

Mapping clavier (compatible Clone Hero par defaut) :

| Guitare         | Clavier  |
|-----------------|----------|
| Frette verte    | A        |
| Frette rouge    | S        |
| Frette jaune    | J        |
| Frette bleue    | K        |
| Frette orange   | L        |
| Strum up        | Fleche haut |
| Strum down      | Fleche bas  |
| Tilt (star power) | Espace |
| Start           | Entree   |
| Back            | Echap    |

Mode verbose pour debug protocole :

```bash
./xenon360 -v
```

## Permissions macOS

Au premier lancement macOS demande l'autorisation **Accessibility** pour Terminal (necessaire pour injecter des evenements clavier).

Reglages > Confidentialite et securite > Accessibilite > activer Terminal.

## Roadmap

- [x] v0.1 : injection clavier pour guitare wired Clone Hero
- [ ] v0.2 : Virtual HID gamepad via IOHIDUserDevice (whammy/tilt analogiques)
- [ ] v0.3 : support wireless receiver Xbox 360 USB (multiplex 4 manettes)
- [ ] v0.4 : menu bar app Swift + auto-launch
- [ ] v1.0 : migration DriverKit dext (zero modif systeme, distribuable Mac App Store)

## Architecture

Voir [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) (TODO).

## Credits

Protocole USB Xbox 360 documente par :
- [Parts Not Included](https://www.partsnotincluded.com/understanding-the-xbox-360-wired-controllers-usb-data/)
- [Linux kernel xpad driver](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c)
- [Tattiebogle USB info](https://tattiebogle.net/ProjectRoot/Xbox360Controller/UsbInfo)
- [Free60 wiki](https://free60.org/)

## License

GPL-2.0-or-later
