#!/bin/bash
# Removes the Clone Hero auto-launch LaunchAgent.

set -euo pipefail

LABEL="dev.vesanerie.xenon360-watcher"
PLIST_DEST="$HOME/Library/LaunchAgents/$LABEL.plist"

launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
rm -f "$PLIST_DEST"

# Kill any leftover xenon360 spawned by the watcher.
if [ -f /tmp/xenon360-watcher.pid ]; then
    pid=$(cat /tmp/xenon360-watcher.pid 2>/dev/null || echo "")
    if [ -n "$pid" ]; then
        kill "$pid" 2>/dev/null || true
    fi
    rm -f /tmp/xenon360-watcher.pid
fi

echo "OK : LaunchAgent desinstalle."
