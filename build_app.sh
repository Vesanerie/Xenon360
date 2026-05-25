#!/bin/bash
set -euo pipefail

APP_NAME="Xenon360"
APP_BUNDLE="${APP_NAME}.app"
CONTENTS="${APP_BUNDLE}/Contents"

cd "$(dirname "$0")"

echo "Build CLI binary..."
make >/dev/null

echo "Build menu bar app..."
clang -O2 -Wall -fobjc-arc -framework Cocoa \
      app/main.m -o app/${APP_NAME}

echo "Create bundle..."
rm -rf "${APP_BUNDLE}"
mkdir -p "${CONTENTS}/MacOS"
mkdir -p "${CONTENTS}/Resources"

cp app/Info.plist           "${CONTENTS}/Info.plist"
cp app/${APP_NAME}          "${CONTENTS}/MacOS/${APP_NAME}"
cp xenon360                 "${CONTENTS}/Resources/xenon360"

rm app/${APP_NAME}

echo "Sign ad-hoc (local dev)..."
# Re-sign the bundled CLI with a stable identifier tied to the parent bundle.
# Without this, the linker-signed CLI keeps identifier "xenon360" and TCC treats
# it as its own identity (matching any stale auth=0 entry) instead of inheriting
# from the .app bundle ID. --remove-signature first because --deep skips
# already-signed Mach-Os.
codesign --remove-signature "${CONTENTS}/Resources/xenon360" 2>/dev/null || true
codesign --force --sign - \
         --identifier dev.vesanerie.xenon360.cli \
         "${CONTENTS}/Resources/xenon360"

# Sign the bundle (deep will now seal the freshly-signed nested CLI properly).
codesign --force --deep --sign - "${APP_BUNDLE}" 2>&1 | grep -v "replacing existing signature" || true

echo "Verify nested signing:"
codesign -dvvv "${CONTENTS}/Resources/xenon360" 2>&1 | grep -E "Identifier|Signature"

echo
echo "OK : ${APP_BUNDLE} cree."
echo "Double-clic pour lancer, ou : open ${APP_BUNDLE}"
