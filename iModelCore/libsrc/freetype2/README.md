# Updating FreeType

FreeType is built by vcpkg from the upstream vcpkg registry `freetype` port. Follow [VCPKG.md](../VCPKG.md) when updating it: set the exact version in the `overrides` of [vcpkg.json](vcpkg.json) (and its `version>=` constraint), and move the `baseline` in [vcpkg-configuration.json](vcpkg-configuration.json) to a vcpkg registry commit that contains that port version.

Whenever FreeType is updated, update [Freetype2NugetLicense.json](Freetype2NugetLicense.json) in the same change. Verify the upstream version, SPDX license identifier, and version-pinned license URL. Use the upstream FreeType version without any vcpkg port revision.

## Build notes

- Only the `zlib` feature of the port is enabled (`"default-features": false`), matching the previous file-by-file build, which used only `FT_CONFIG_OPTION_SYSTEM_ZLIB`. Enabling `png`, `bzip2`, or `brotli` would require delivering and linking those libraries too.
- [freetype2.prewire.mke](freetype2.prewire.mke) stages `ft2build.h` and `freetype/` from the vcpkg install into `VendorAPI/freetype2`, and delivers the license notice.
- [freetype2.mke](freetype2.mke) delivers the vcpkg archive as `iTwinFreetype2` for static builds. For Windows dynamic builds it re-links `iTwinFreetype2.dll` from the static archive, exporting the `FT_EXPORT` functions listed by [makeFreetypeDef.py](makeFreetypeDef.py). zlib symbols are resolved by `BeZlib`.