#!/bin/bash
# Watches for Clone Hero and auto-starts/stops Xenon360.app.
#
# Why .app and not the bare CLI? macOS TCC for Accessibility uses
# responsible-process attribution. When launchd executes the CLI binary
# directly, its responsible process is launchd (which has no
# Accessibility), so CGEventPost is silently denied. Launching via
# Xenon360.app routes through Launch Services, the app gets its own
# bundle-ID-scoped TCC permission, and the bundled CLI subprocess
# inherits it.

set -u

APP_BUNDLE_ID="dev.vesanerie.xenon360"
CHECK_INTERVAL=3

# Resolve where Xenon360.app actually lives.
# PKG installer drops it in /Applications; source-build install.sh stages it
# in ~/Library/Application Support/Xenon360/. Honor an explicit XENON_APP env
# override first, then probe the standard locations.
resolve_app_path() {
    if [ -n "${XENON_APP:-}" ] && [ -d "$XENON_APP" ]; then
        echo "$XENON_APP"; return
    fi
    for candidate in \
        "/Applications/Xenon360.app" \
        "$HOME/Applications/Xenon360.app" \
        "$HOME/Library/Application Support/Xenon360/Xenon360.app"; do
        if [ -d "$candidate" ]; then echo "$candidate"; return; fi
    done
    echo ""
}
APP_PATH="$(resolve_app_path)"

CLONE_HERO_PATTERN="Clone Hero.app/Contents/MacOS"

ts() { date "+%Y-%m-%d %H:%M:%S"; }

is_clone_hero_running() {
    # -a anchors to the start of the command name, avoiding false positives
    # from any other process that might happen to contain the path substring.
    pgrep -f "$CLONE_HERO_PATTERN" 2>/dev/null | grep -v "^$$\$" >/dev/null
}

is_xenon_running() {
    pgrep -f "Xenon360.app/Contents/MacOS/Xenon360" >/dev/null
}

start_xenon() {
    # Re-resolve every time in case the bundle moved between polls (e.g. user
    # ran installer mid-session). Avoids a stale APP_PATH cached at boot.
    APP_PATH="$(resolve_app_path)"
    if [ -z "$APP_PATH" ] || [ ! -d "$APP_PATH" ]; then
        echo "[$(ts)] ERROR: Xenon360.app not found in /Applications, ~/Applications, or ~/Library/Application Support/Xenon360/" >&2
        return 1
    fi
    # AppleScript launch via Finder/loginwindow context. Empirically more
    # reliable than `open -gj` for TCC inheritance: `open` from a launchd-
    # spawned shell can attribute the responsible process to bash, while
    # `tell application id ... to launch` routes through AppleEvents and
    # attaches the new process to the user's loginwindow session.
    osascript -e "tell application id \"$APP_BUNDLE_ID\" to launch" 2>/dev/null \
        || open -gj "$APP_PATH"
    echo "[$(ts)] Launched Xenon360.app"
}

stop_xenon() {
    if osascript -e 'tell application id "'"$APP_BUNDLE_ID"'" to quit' 2>/dev/null; then
        echo "[$(ts)] Quit Xenon360.app via AppleScript"
    else
        pkill -f "Xenon360.app/Contents/MacOS/Xenon360" 2>/dev/null && \
            echo "[$(ts)] Killed Xenon360.app via pkill"
    fi
}

cleanup() {
    stop_xenon
    exit 0
}
trap cleanup TERM INT

echo "[$(ts)] Watcher started, polling every ${CHECK_INTERVAL}s for '$CLONE_HERO_PATTERN'"
echo "[$(ts)] Will launch: ${APP_PATH:-<not yet resolved>}"

while true; do
    if is_clone_hero_running; then
        if ! is_xenon_running; then
            start_xenon
        fi
    else
        if is_xenon_running; then
            stop_xenon
        fi
    fi
    sleep "$CHECK_INTERVAL"
done
