#!/bin/bash
# Full release pipeline: build + sign + notarize + staple.
# Produces Xenon360.pkg ready to upload to GitHub Releases.
#
# Requires once-only setup:
#   1. Generate "Developer ID Application" + "Developer ID Installer" certs
#      from developer.apple.com under your Team ID, install in Keychain.
#   2. Generate an app-specific password at appleid.apple.com.
#   3. Store notarytool credentials:
#        xcrun notarytool store-credentials xenon360-notary \
#              --apple-id YOUR_APPLE_ID \
#              --team-id  Q4W8LZ8636 \
#              --password APP_SPECIFIC_PASSWORD
#   4. Export env vars (or edit defaults below):
#        export TEAM_ID=Q4W8LZ8636
#        export DEVELOPER_NAME="Valentin Mardoukhaev"

set -euo pipefail

cd "$(dirname "$0")"

TEAM_ID="${TEAM_ID:-Q4W8LZ8636}"
DEVELOPER_NAME="${DEVELOPER_NAME:-Valentin Mardoukhaev}"
NOTARY_PROFILE="${NOTARY_PROFILE:-xenon360-notary}"

SIGNING_IDENTITY="Developer ID Application: ${DEVELOPER_NAME} (${TEAM_ID})"
INSTALLER_IDENTITY="Developer ID Installer: ${DEVELOPER_NAME} (${TEAM_ID})"

# Verify certs are present before doing any work.
echo "==> Checking codesigning identities"
if ! security find-identity -p codesigning -v | grep -q "$SIGNING_IDENTITY"; then
    echo "ERROR: missing certificate: $SIGNING_IDENTITY" >&2
    echo "       Generate it at developer.apple.com -> Certificates" >&2
    exit 1
fi
if ! security find-identity -p basic -v | grep -q "$INSTALLER_IDENTITY"; then
    echo "ERROR: missing certificate: $INSTALLER_IDENTITY" >&2
    exit 1
fi
echo "    OK"

# Verify notarytool credentials.
echo "==> Checking notarytool profile '$NOTARY_PROFILE'"
if ! xcrun notarytool history --keychain-profile "$NOTARY_PROFILE" >/dev/null 2>&1; then
    echo "ERROR: notarytool profile '$NOTARY_PROFILE' not set up." >&2
    echo "       Run:" >&2
    echo "         xcrun notarytool store-credentials $NOTARY_PROFILE \\" >&2
    echo "               --apple-id YOUR_APPLE_ID --team-id $TEAM_ID \\" >&2
    echo "               --password APP_SPECIFIC_PASSWORD" >&2
    exit 1
fi
echo "    OK"

# Build the .pkg with release signing.
echo "==> Build signed pkg"
SIGNING_IDENTITY="$SIGNING_IDENTITY" \
INSTALLER_IDENTITY="$INSTALLER_IDENTITY" \
    ./build_pkg.sh

# Submit to Apple notarization.
echo "==> Notarize (this can take 1-15 minutes)"
xcrun notarytool submit Xenon360.pkg \
    --keychain-profile "$NOTARY_PROFILE" \
    --wait

# Staple the notarization ticket so the .pkg works offline.
echo "==> Staple ticket"
xcrun stapler staple Xenon360.pkg

echo "==> Verify"
xcrun stapler validate Xenon360.pkg
spctl --assess --type install --verbose Xenon360.pkg 2>&1 | head -3

echo
echo "OK : Xenon360.pkg is signed, notarized, stapled."
echo "Upload to: https://github.com/Vesanerie/Xenon360/releases"
