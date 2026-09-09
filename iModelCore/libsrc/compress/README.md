# Updating Compression Libraries

Follow the repository's [third-party library update workflow](../../../.github/skills/update-third-party-lib/SKILL.md) when updating the vendored LZMA SDK or Snappy sources. Follow [VCPKG.md](../VCPKG.md) when updating the vcpkg-managed zlib or minizip ports.

Whenever any compression library is updated, update its corresponding entry in [CompressNugetLicense.json](CompressNugetLicense.json) in the same change. Verify the upstream version, SPDX license identifier, and version-pinned license URL. Use the upstream library version without any vcpkg port revision.
