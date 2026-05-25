#!/bin/bash
# Build Xenon360.app bundle.
#
# Modes:
#   ./build_app.sh           ad-hoc signing (dev local, no Gatekeeper trust)
#   SIGNING_IDENTITY="Developer ID Application: ..." ./build_app.sh
#                            release signing (hardened runtime, ready for notarization)

set -euo pipefail

APP_NAME="Xenon360"
APP_BUNDLE="${APP_NAME}.app"
CONTENTS="${APP_BUNDLE}/Contents"
ENTITLEMENTS="$(cd "$(dirname "$0")" && pwd)/xenon360.entitlements"
ARCHS="-arch arm64 -arch x86_64"

cd "$(dirname "$0")"

SIGNING_IDENTITY="${SIGNING_IDENTITY:--}"   # default '-' = ad-hoc
RELEASE_MODE=0
if [ "$SIGNING_IDENTITY" != "-" ]; then
    RELEASE_MODE=1
fi

echo "Build CLI binary (universal arm64+x86_64)..."
make >/dev/null

echo "Build menu bar app (universal arm64+x86_64)..."
clang -O2 -Wall -fobjc-arc $ARCHS \
      -framework Cocoa -framework ApplicationServices \
      app/main.m -o app/${APP_NAME}

echo "Create bundle..."
rm -rf "${APP_BUNDLE}"
mkdir -p "${CONTENTS}/MacOS"
mkdir -p "${CONTENTS}/Resources"

cp app/Info.plist                                  "${CONTENTS}/Info.plist"
cp app/${APP_NAME}                                 "${CONTENTS}/MacOS/${APP_NAME}"
cp xenon360                                        "${CONTENTS}/Resources/xenon360"

# Stage watcher payload inside the .app so the PKG postinstall (or a user with
# a downloaded .app) can find it without needing the source repo.
cp autolaunch/clone_hero_watcher.sh                "${CONTENTS}/Resources/clone_hero_watcher.sh"
cp autolaunch/dev.vesanerie.xenon360-watcher.plist "${CONTENTS}/Resources/dev.vesanerie.xenon360-watcher.plist"
cp autolaunch/uninstall.sh                         "${CONTENTS}/Resources/uninstall.sh"
chmod +x "${CONTENTS}/Resources/clone_hero_watcher.sh" "${CONTENTS}/Resources/uninstall.sh"

# Note: libusb is statically linked into both binaries via libs/libusb-1.0.a
# (universal arm64+x86_64 archive), no dylib to bundle. Cleaner, smaller.

rm app/${APP_NAME}

if [ "$RELEASE_MODE" = "1" ]; then
    echo "Sign RELEASE with: $SIGNING_IDENTITY"
    SIGN_OPTS="--options runtime --timestamp --entitlements $ENTITLEMENTS"
else
    echo "Sign ad-hoc (local dev)..."
    SIGN_OPTS=""
fi

# Strip stale signatures first; --deep does not re-sign already-signed Mach-Os.
for bin in "${CONTENTS}/Resources/xenon360" \
           "${CONTENTS}/MacOS/${APP_NAME}"; do
    [ -f "$bin" ] && codesign --remove-signature "$bin" 2>/dev/null || true
done

# Re-sign nested CLI with a bundle-scoped identifier so TCC can match
# Accessibility grants made on the parent bundle ID.
codesign --force --sign "$SIGNING_IDENTITY" $SIGN_OPTS \
         --identifier dev.vesanerie.xenon360.cli \
         "${CONTENTS}/Resources/xenon360"

codesign --force --sign "$SIGNING_IDENTITY" $SIGN_OPTS \
         --identifier dev.vesanerie.xenon360 \
         "${CONTENTS}/MacOS/${APP_NAME}"

# Final bundle seal.
codesign --force --deep --sign "$SIGNING_IDENTITY" $SIGN_OPTS "${APP_BUNDLE}" \
    2>&1 | grep -v "replacing existing signature" || true

echo
echo "Verify:"
codesign -dvvv "${APP_BUNDLE}" 2>&1 | grep -E "Identifier|Signature|TeamIdentifier|Authority" | head -6
codesign -dvvv "${CONTENTS}/Resources/xenon360" 2>&1 | grep -E "Identifier|Signature" | head -2
echo "Architectures:"
lipo -info "${CONTENTS}/MacOS/${APP_NAME}" 2>&1
lipo -info "${CONTENTS}/Resources/xenon360" 2>&1

if [ "$RELEASE_MODE" = "1" ]; then
    echo
    echo "Hardened-runtime verification:"
    codesign --verify --strict --verbose=2 "${APP_BUNDLE}" 2>&1
fi

echo
echo "OK : ${APP_BUNDLE} cree."
echo "Double-clic pour lancer, ou : open ${APP_BUNDLE}"
