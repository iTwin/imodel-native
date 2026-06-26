# OpenSSL overlay ports

This directory is referenced by [`../vcpkg-configuration.json`](../vcpkg-configuration.json)
(`overlay-ports`). The `openssl/` overlay port here pins and patches the OpenSSL build used by
[`../vcpkg.json`](../vcpkg.json).

The overlay port serves two purposes:

1. **Pins the version to 3.4.6.** The vcpkg registry does not package 3.4.6 (its 3.4 line only
   has 3.4.0/3.4.1), so the overlay port fetches the 3.4.6 source itself and declares
   `"version": "3.4.6"`, satisfying the pin in [`../vcpkg.json`](../vcpkg.json). It is based on
   the registry's 3.4.1 port with the `REF`/`SHA512` bumped to 3.4.6.
2. **Applies our patches** via the `PATCHES` list in `openssl/portfile.cmake`:
   - `android-by_dir.patch` — the Android `X509_NAME_hash_old` change (item 2).
   - `bsiver-version-string.patch` — the `BSIVer` version-string section (item 1, see Step 3).

All patches in the `PATCHES` list (the nine carried over from the 3.4.1 port plus the two Bentley
patches above) have been verified to apply cleanly to fresh 3.4.6 source with both `patch -p1` and
`git apply --check`.

See [`../VCPKG_MIGRATION.md`](../VCPKG_MIGRATION.md) for the full migration plan. No build is wired
up to consume this port until Step 6, so the `3.4.6` pin is not yet exercised by a real install.
