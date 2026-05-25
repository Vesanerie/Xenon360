#!/bin/bash
# Builds a distributable .pkg installer for Xenon360.
#
# Modes:
#   ./build_pkg.sh
#       ad-hoc build for local testing (not Gatekeeper-trusted, no notarization)
#
#   INSTALLER_IDENTITY="Developer ID Installer: Valentin Mardoukhaev (Q4W8LZ8636)" \
#   SIGNING_IDENTITY="Developer ID Application: Valentin Mardoukhaev (Q4W8LZ8636)" \
#       ./build_pkg.sh
#       release build, signed and ready for notarization (cf. release.sh).
#
# Output: Xenon360.pkg in the project root.

set -euo pipefail

cd "$(dirname "$0")"

APP_NAME="Xenon360"
APP_BUNDLE="${APP_NAME}.app"
PKG_OUT="${APP_NAME}.pkg"
COMPONENT_PKG="xenon360-component.pkg"
PKG_ID="dev.vesanerie.xenon360.pkg"
APP_VERSION="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' app/Info.plist 2>/dev/null || echo 0.3.1)"

INSTALLER_IDENTITY="${INSTALLER_IDENTITY:-}"

# Step 1: ensure .app exists and is signed appropriately for this build mode.
echo "==> build_app.sh (mode: ${SIGNING_IDENTITY:-ad-hoc})"
./build_app.sh

# Step 2: build the component package (the payload to lay down + scripts).
# --root points to a staging dir containing exactly what should land on disk.
echo "==> Stage payload"
STAGE_DIR="$(mktemp -d)"
trap 'rm -rf "$STAGE_DIR"' EXIT
mkdir -p "${STAGE_DIR}/Applications"
cp -R "${APP_BUNDLE}" "${STAGE_DIR}/Applications/"

echo "==> pkgbuild component"
pkgbuild \
    --root      "${STAGE_DIR}" \
    --identifier "${PKG_ID}" \
    --version    "${APP_VERSION}" \
    --install-location / \
    --scripts    pkg/scripts \
    "${COMPONENT_PKG}"

# Step 3: wrap the component package in a productbuild distribution package
# so we get the Welcome / License / Conclusion wizard pages.
echo "==> productbuild distribution"
mkdir -p pkg/build
cp "${COMPONENT_PKG}" pkg/build/

PRODUCTBUILD_ARGS=(
    --distribution pkg/distribution.xml
    --package-path pkg/build
    --resources    pkg/resources
)
if [ -n "$INSTALLER_IDENTITY" ]; then
    echo "==> Signing with: $INSTALLER_IDENTITY"
    PRODUCTBUILD_ARGS+=(--sign "$INSTALLER_IDENTITY")
fi

productbuild "${PRODUCTBUILD_ARGS[@]}" "${PKG_OUT}"

# Cleanup intermediate.
rm -f "${COMPONENT_PKG}"
rm -rf pkg/build

echo
echo "OK : ${PKG_OUT} (${APP_VERSION})"
ls -lh "${PKG_OUT}"

if [ -n "$INSTALLER_IDENTITY" ]; then
    echo
    echo "Verify signature:"
    pkgutil --check-signature "${PKG_OUT}" | head -20
fi
