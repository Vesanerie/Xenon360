#!/bin/bash
# Watches for Clone Hero and auto-starts/stops the xenon360 driver.
# Installed as a LaunchAgent; see install.sh.

set -u

XENON_BIN="$HOME/Desktop/OUTIL/Xenon360/xenon360"
PID_FILE="/tmp/xenon360-watcher.pid"
CHECK_INTERVAL=3

# Match the game process running inside the .app bundle, not the Launcher.
CLONE_HERO_PATTERN="Clone Hero.app/Contents/MacOS"

ts() { date "+%Y-%m-%d %H:%M:%S"; }

is_clone_hero_running() {
    pgrep -f "$CLONE_HERO_PATTERN" >/dev/null
}

is_xenon_running() {
    [ -f "$PID_FILE" ] && kill -0 "$(cat "$PID_FILE" 2>/dev/null)" 2>/dev/null
}

start_xenon() {
    if [ ! -x "$XENON_BIN" ]; then
        echo "[$(ts)] ERROR: xenon360 binary missing at $XENON_BIN" >&2
        return 1
    fi
    "$XENON_BIN" >/dev/null 2>&1 &
    local pid=$!
    echo "$pid" >"$PID_FILE"
    echo "[$(ts)] Started Xenon360 PID $pid"
}

stop_xenon() {
    if [ -f "$PID_FILE" ]; then
        local pid
        pid=$(cat "$PID_FILE" 2>/dev/null || echo "")
        if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
            kill "$pid" 2>/dev/null
            sleep 0.3
            kill -9 "$pid" 2>/dev/null || true
        fi
        rm -f "$PID_FILE"
        echo "[$(ts)] Stopped Xenon360"
    fi
}

cleanup() {
    stop_xenon
    exit 0
}
trap cleanup TERM INT

echo "[$(ts)] Watcher started, polling every ${CHECK_INTERVAL}s for '$CLONE_HERO_PATTERN'"

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
