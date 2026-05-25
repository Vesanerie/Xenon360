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

# Games that should trigger Xenon360. Add a new line below for each. The
# pattern matches against the full process command line via `pgrep -f`,
# so use the .app's executable path to avoid false positives from songs,
# folders, or shell history that happens to contain the game's name.
GAME_PATTERNS=(
    "Clone Hero.app/Contents/MacOS"
    "YARG.app/Contents/MacOS"
    "Fortnite Festival.app/Contents/MacOS"
    "Rock Band 4.app/Contents/MacOS"
)

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

ts() { date "+%Y-%m-%d %H:%M:%S"; }

is_game_running() {
    for pattern in "${GAME_PATTERNS[@]}"; do
        if pgrep -f "$pattern" >/dev/null 2>&1; then
            return 0
        fi
    done
    return 1
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

echo "[$(ts)] Watcher started, polling every ${CHECK_INTERVAL}s for: ${GAME_PATTERNS[*]}"
echo "[$(ts)] Will launch: ${APP_PATH:-<not yet resolved>}"

while true; do
    if is_game_running; then
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
