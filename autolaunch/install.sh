#!/bin/bash
# Installs the Clone Hero auto-launch LaunchAgent.
#
# macOS TCC blocks launchd from running executables located in protected
# folders like ~/Desktop or ~/Documents, so we copy the watcher script
# and the xenon360 binary into ~/Library/Application Support/Xenon360/
# which is unrestricted.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_BIN="$PROJECT_DIR/xenon360"
SRC_WATCHER="$SCRIPT_DIR/clone_hero_watcher.sh"
PLIST_TEMPLATE="$SCRIPT_DIR/dev.vesanerie.xenon360-watcher.plist"

INSTALL_DIR="$HOME/Library/Application Support/Xenon360"
INSTALLED_BIN="$INSTALL_DIR/xenon360"
INSTALLED_WATCHER="$INSTALL_DIR/clone_hero_watcher.sh"

LAUNCH_AGENTS="$HOME/Library/LaunchAgents"
LABEL="dev.vesanerie.xenon360-watcher"
PLIST_DEST="$LAUNCH_AGENTS/$LABEL.plist"

if [ ! -x "$SRC_BIN" ]; then
    echo "ERROR : xenon360 binary missing. Run 'make' first." >&2
    exit 1
fi

mkdir -p "$INSTALL_DIR"
cp "$SRC_BIN" "$INSTALLED_BIN"
cp "$SRC_WATCHER" "$INSTALLED_WATCHER"
chmod +x "$INSTALLED_BIN" "$INSTALLED_WATCHER"

mkdir -p "$LAUNCH_AGENTS"
sed "s|__WATCHER_PATH__|$INSTALLED_WATCHER|g" "$PLIST_TEMPLATE" > "$PLIST_DEST"

launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
launchctl bootstrap "gui/$(id -u)" "$PLIST_DEST"
launchctl enable "gui/$(id -u)/$LABEL"

echo "OK : LaunchAgent installe."
echo "  Watcher : $INSTALLED_WATCHER"
echo "  Binary  : $INSTALLED_BIN"
echo "  Plist   : $PLIST_DEST"
echo
echo "A chaque fois que tu ouvres Clone Hero, xenon360 demarre automatiquement"
echo "et s'arrete avec le jeu."
echo
echo "Logs : tail -f /tmp/xenon360-watcher.log"
echo "Stop : ./uninstall.sh"
echo
echo "IMPORTANT : apres chaque 'make', relance ./install.sh pour recopier"
echo "la nouvelle version du binaire vers l'install dir."
echo
echo "Si les frettes n'envoient pas de touches dans le jeu : accorde"
echo "'Accessibilite' au binaire xenon360 (Reglages > Confidentialite >"
echo "Accessibilite, ajoute '$INSTALLED_BIN')."
