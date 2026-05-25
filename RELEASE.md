# Release process

How to cut a signed, notarized Xenon360.pkg ready for GitHub Releases.

## One-time setup

Done once per machine, never again.

### 1. Apple Developer membership

Required: an active Apple Developer account ($99/year). Xenon360 reuses the
Vesanerie / Gesturo team (Team ID `Q4W8LZ8636`).

### 2. Generate the two certificates

At https://developer.apple.com/account/resources/certificates/list

Generate two CSRs from Keychain Access (Keychain Access > Certificate Assistant >
Request a Certificate From a Certificate Authority > save to disk), then upload
each:

- **Developer ID Application** — signs the .app bundle
- **Developer ID Installer** — signs the .pkg installer

Download both `.cer` files and double-click to install into the login keychain.

### 3. App-specific password for notarytool

At https://appleid.apple.com > Sign-In and Security > App-Specific Passwords
> generate one named "Xenon360 notarization". Copy it once, you cannot view it
later.

### 4. Store credentials in the keychain

```bash
xcrun notarytool store-credentials xenon360-notary \
      --apple-id mardouv2@gmail.com \
      --team-id  Q4W8LZ8636 \
      --password APP_SPECIFIC_PASSWORD_FROM_STEP_3
```

This saves the credentials securely in the login keychain so `release.sh` can
submit without prompting.

## Release a new version

1. Bump version in `app/Info.plist`:
   ```xml
   <key>CFBundleShortVersionString</key>
   <string>0.3.2</string>
   <key>CFBundleVersion</key>
   <string>4</string>
   ```

2. Commit and push.

3. Run the pipeline:
   ```bash
   make release
   ```
   This:
   - Verifies the two certs and notarytool credentials are present
   - Builds Xenon360.app with hardened runtime + Developer ID signing
   - Wraps it in a wizard-style Xenon360.pkg (Welcome / License / Conclusion)
   - Signs the .pkg with Developer ID Installer
   - Submits to Apple notarization (waits, usually 1-15 minutes)
   - Staples the notarization ticket so the .pkg works offline

4. Tag and upload:
   ```bash
   git tag v0.3.2
   git push --tags
   gh release create v0.3.2 Xenon360.pkg \
       --title "v0.3.2" \
       --notes-file CHANGELOG.md
   ```

## Local dev iteration

For everyday work, no certs needed. Ad-hoc signing is automatic:

```bash
make app       # builds Xenon360.app, ad-hoc signed
make pkg       # builds Xenon360.pkg, ad-hoc signed (Gatekeeper will complain)
```

`make pkg` ad-hoc is useful to test the wizard / postinstall script locally
without notarizing. To actually run a notarized .pkg, you must use
`make release` (or `./release.sh`).

## Troubleshooting

- **`security find-identity` returns nothing**: certs not installed in login
  keychain. Re-download from developer.apple.com and double-click.

- **notarization fails "The signature does not include a secure timestamp"**:
  `--timestamp` flag missing somewhere. build_app.sh sets it for release mode;
  check that `SIGNING_IDENTITY` env var was actually exported when calling
  build_pkg.sh.

- **notarization fails "The binary is not signed with a valid Developer ID
  certificate"**: ad-hoc signing leaked into the release path. Confirm with
  `codesign -dvvv Xenon360.app` that `TeamIdentifier=Q4W8LZ8636` (not "not set").

- **stapler fails "The staple and validate action failed"**: notarization
  succeeded but the .pkg was modified after submission. Don't touch the file
  between `notarytool submit --wait` and `stapler staple`.

- **`spctl --assess` rejects**: Gatekeeper considers the file untrusted.
  Run `xcrun notarytool log <submission-id> --keychain-profile xenon360-notary`
  to see Apple's reason.
