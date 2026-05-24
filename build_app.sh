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
codesign --force --deep --sign - "${APP_BUNDLE}" 2>&1 | grep -v "replacing existing signature" || true

echo
echo "OK : ${APP_BUNDLE} cree."
echo "Double-clic pour lancer, ou : open ${APP_BUNDLE}"
