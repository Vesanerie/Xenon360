#!/bin/bash
# Installs the Clone Hero auto-launch LaunchAgent.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WATCHER="$SCRIPT_DIR/clone_hero_watcher.sh"
PLIST_TEMPLATE="$SCRIPT_DIR/dev.vesanerie.xenon360-watcher.plist"
LAUNCH_AGENTS="$HOME/Library/LaunchAgents"
PLIST_DEST="$LAUNCH_AGENTS/dev.vesanerie.xenon360-watcher.plist"
LABEL="dev.vesanerie.xenon360-watcher"

if [ ! -x "$WATCHER" ]; then
    chmod +x "$WATCHER"
fi

mkdir -p "$LAUNCH_AGENTS"

# Substitute the absolute watcher path into the plist template.
sed "s|__WATCHER_PATH__|$WATCHER|g" "$PLIST_TEMPLATE" > "$PLIST_DEST"

# Reload if already loaded.
launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
launchctl bootstrap "gui/$(id -u)" "$PLIST_DEST"
launchctl enable "gui/$(id -u)/$LABEL"

echo "OK : LaunchAgent installe ($PLIST_DEST)."
echo
echo "Le watcher tourne en permanence. A chaque fois que tu ouvres"
echo "Clone Hero, xenon360 demarre automatiquement et s'arrete avec le jeu."
echo
echo "Logs : tail -f /tmp/xenon360-watcher.log"
echo "Stop : ./uninstall.sh"
echo
echo "IMPORTANT : Si les frettes n'envoient pas de touches dans le jeu,"
echo "il faut accorder 'Accessibilite' au binaire xenon360 directement"
echo "(et non plus seulement a Terminal). Quand le watcher le lance"
echo "la premiere fois, macOS affichera une popup. Va dans :"
echo "  Reglages > Confidentialite > Accessibilite"
echo "et ajoute / coche '$HOME/Desktop/OUTIL/Xenon360/xenon360'."
