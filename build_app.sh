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

cd "$(dirname "$0")"

SIGNING_IDENTITY="${SIGNING_IDENTITY:--}"   # default '-' = ad-hoc
RELEASE_MODE=0
if [ "$SIGNING_IDENTITY" != "-" ]; then
    RELEASE_MODE=1
fi

echo "Build CLI binary..."
make >/dev/null

echo "Build menu bar app..."
clang -O2 -Wall -fobjc-arc -framework Cocoa \
      app/main.m -o app/${APP_NAME}

echo "Create bundle..."
rm -rf "${APP_BUNDLE}"
mkdir -p "${CONTENTS}/MacOS"
mkdir -p "${CONTENTS}/Resources"
mkdir -p "${CONTENTS}/Frameworks"

cp app/Info.plist                                  "${CONTENTS}/Info.plist"
cp app/${APP_NAME}                                 "${CONTENTS}/MacOS/${APP_NAME}"
cp xenon360                                        "${CONTENTS}/Resources/xenon360"

# Stage watcher payload inside the .app so the PKG postinstall (or a user with
# a downloaded .app) can find it without needing the source repo.
cp autolaunch/clone_hero_watcher.sh                "${CONTENTS}/Resources/clone_hero_watcher.sh"
cp autolaunch/dev.vesanerie.xenon360-watcher.plist "${CONTENTS}/Resources/dev.vesanerie.xenon360-watcher.plist"
cp autolaunch/uninstall.sh                         "${CONTENTS}/Resources/uninstall.sh"
chmod +x "${CONTENTS}/Resources/clone_hero_watcher.sh" "${CONTENTS}/Resources/uninstall.sh"

# Bundle libusb so the .app is portable across machines that may not have brew.
LIBUSB_SRC="/opt/homebrew/lib/libusb-1.0.0.dylib"
if [ -f "$LIBUSB_SRC" ]; then
    cp "$LIBUSB_SRC" "${CONTENTS}/Frameworks/libusb-1.0.0.dylib"
    # Rewrite the load path on both binaries so they pick up the bundled copy.
    install_name_tool -change "$LIBUSB_SRC" "@executable_path/../Frameworks/libusb-1.0.0.dylib" \
                      "${CONTENTS}/Resources/xenon360" 2>/dev/null || true
    install_name_tool -change "@rpath/libusb-1.0.0.dylib" "@executable_path/../Frameworks/libusb-1.0.0.dylib" \
                      "${CONTENTS}/Resources/xenon360" 2>/dev/null || true
fi

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
           "${CONTENTS}/Frameworks/libusb-1.0.0.dylib" \
           "${CONTENTS}/MacOS/${APP_NAME}"; do
    [ -f "$bin" ] && codesign --remove-signature "$bin" 2>/dev/null || true
done

# Re-sign nested bits with a bundle-scoped identifier so TCC can match
# Accessibility grants made on the parent bundle ID.
[ -f "${CONTENTS}/Frameworks/libusb-1.0.0.dylib" ] && \
    codesign --force --sign "$SIGNING_IDENTITY" $SIGN_OPTS \
             --identifier dev.vesanerie.xenon360.libusb \
             "${CONTENTS}/Frameworks/libusb-1.0.0.dylib"

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

if [ "$RELEASE_MODE" = "1" ]; then
    echo
    echo "Hardened-runtime verification:"
    codesign --verify --strict --verbose=2 "${APP_BUNDLE}" 2>&1
fi

echo
echo "OK : ${APP_BUNDLE} cree."
echo "Double-clic pour lancer, ou : open ${APP_BUNDLE}"
