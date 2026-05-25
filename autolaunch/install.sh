#!/bin/bash
# Installs the Clone Hero auto-launch LaunchAgent.
#
# macOS TCC blocks launchd from running executables located in protected
# folders like ~/Desktop or ~/Documents. We copy the watcher script, the
# Xenon360.app bundle, and the standalone CLI to ~/Library/Application
# Support/Xenon360/ which is unrestricted, then point a LaunchAgent at
# the watcher.
#
# The watcher launches Xenon360.app (not the bare CLI) because TCC for
# Accessibility uses bundle-ID matching: only the .app bundle has a
# stable identity that can be granted permission once.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_BIN="$PROJECT_DIR/xenon360"
SRC_APP="$PROJECT_DIR/Xenon360.app"
SRC_WATCHER="$SCRIPT_DIR/clone_hero_watcher.sh"
PLIST_TEMPLATE="$SCRIPT_DIR/dev.vesanerie.xenon360-watcher.plist"

INSTALL_DIR="$HOME/Library/Application Support/Xenon360"
INSTALLED_BIN="$INSTALL_DIR/xenon360"
INSTALLED_APP="$INSTALL_DIR/Xenon360.app"
INSTALLED_WATCHER="$INSTALL_DIR/clone_hero_watcher.sh"

LAUNCH_AGENTS="$HOME/Library/LaunchAgents"
LABEL="dev.vesanerie.xenon360-watcher"
PLIST_DEST="$LAUNCH_AGENTS/$LABEL.plist"

if [ ! -x "$SRC_BIN" ]; then
    echo "ERROR : xenon360 binary missing. Run 'make' first." >&2
    exit 1
fi

if [ ! -d "$SRC_APP" ]; then
    echo "ERROR : Xenon360.app missing. Run 'make app' first." >&2
    exit 1
fi

mkdir -p "$INSTALL_DIR"
cp "$SRC_BIN" "$INSTALLED_BIN"
cp "$SRC_WATCHER" "$INSTALLED_WATCHER"
rm -rf "$INSTALLED_APP"
cp -R "$SRC_APP" "$INSTALLED_APP"
chmod +x "$INSTALLED_BIN" "$INSTALLED_WATCHER"

mkdir -p "$LAUNCH_AGENTS"
sed "s|__WATCHER_PATH__|$INSTALLED_WATCHER|g" "$PLIST_TEMPLATE" > "$PLIST_DEST"

launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
sleep 1
launchctl bootstrap "gui/$(id -u)" "$PLIST_DEST"
launchctl enable "gui/$(id -u)/$LABEL"

echo "OK : LaunchAgent installe."
echo "  Watcher : $INSTALLED_WATCHER"
echo "  Binary  : $INSTALLED_BIN"
echo "  App     : $INSTALLED_APP"
echo "  Plist   : $PLIST_DEST"
echo
echo "A chaque fois que tu ouvres Clone Hero, Xenon360.app demarre"
echo "automatiquement et s'arrete avec le jeu."
echo
echo "Logs : tail -f /tmp/xenon360-watcher.log"
echo "Stop : ./uninstall.sh"
echo
echo "IMPORTANT : apres chaque 'make app', relance ./install.sh pour"
echo "recopier la nouvelle version du bundle vers l'install dir."
echo
echo "Si les frettes n'envoient pas de touches dans Clone Hero : accorde"
echo "'Accessibilite' a Xenon360.app (Reglages > Confidentialite >"
echo "Accessibilite, ajoute '$INSTALLED_APP')."
