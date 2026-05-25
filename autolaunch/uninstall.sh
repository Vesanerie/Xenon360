#!/bin/bash
# Removes the Clone Hero auto-launch LaunchAgent.

set -euo pipefail

LABEL="dev.vesanerie.xenon360-watcher"
PLIST_DEST="$HOME/Library/LaunchAgents/$LABEL.plist"
INSTALL_DIR="$HOME/Library/Application Support/Xenon360"
APP_BUNDLE_ID="dev.vesanerie.xenon360"

launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
rm -f "$PLIST_DEST"

# Quit any running Xenon360.app instance the watcher may have spawned.
osascript -e "tell application id \"$APP_BUNDLE_ID\" to quit" 2>/dev/null || true
pkill -f "Xenon360.app/Contents/MacOS/Xenon360" 2>/dev/null || true
pkill -f "Xenon360.app/Contents/Resources/xenon360" 2>/dev/null || true

# Remove the installed copies (keep user's project intact).
rm -rf "$INSTALL_DIR"

echo "OK : LaunchAgent et fichiers d'install retires."
