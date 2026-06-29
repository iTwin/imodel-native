# OpenSSL overlay ports

This directory is referenced by [`../vcpkg-configuration.json`](../vcpkg-configuration.json)
(`overlay-ports`). The `openssl/` overlay port here pins and patches the OpenSSL build used by
[`../vcpkg.json`](../vcpkg.json).

The overlay port exists solely to **apply our patches** on top of the registry's OpenSSL port,
via the `PATCHES` list in `openssl/portfile.cmake`:
   - `android-by_dir.patch` — the Android `X509_NAME_hash_old` change (item 2).
   - `bsiver-version-string.patch` — the `BSIVer` version-string section (item 1, see Step 3).

The version pinned is **3.6.3**, which the vcpkg registry now packages natively. Because of that,
every other file in `openssl/` (portfile, registry patches, helper scripts, `vcpkg.json`) is a
verbatim copy of the registry's 3.6.3 `ports/openssl`; only the two Bentley patches above and
their two lines in `portfile.cmake`'s `PATCHES` list are our additions. To re-sync after a future
registry bump, overwrite these files from `microsoft/vcpkg`'s `ports/openssl` and re-append the
two Bentley patch lines.

All patches in the `PATCHES` list (the ten carried over from the registry's 3.6.3 port plus the
two Bentley patches above) have been verified to apply cleanly to fresh 3.6.3 source with
`git apply --check`.

See [`../VCPKG_MIGRATION.md`](../VCPKG_MIGRATION.md) for the full migration plan.
