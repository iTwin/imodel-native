# Updating Compression Libraries

Follow the repository's [third-party library update workflow](../../../.github/skills/update-third-party-lib/SKILL.md) when updating the vendored LZMA SDK or Snappy sources. Follow [VCPKG.md](../VCPKG.md) when updating the vcpkg-managed zlib or minizip ports.

Whenever any compression library is updated, update its corresponding entry in [CompressNugetLicense.json](CompressNugetLicense.json) in the same change. Verify the upstream version, SPDX license identifier, and version-pinned license URL. Use the upstream library version without any vcpkg port revision.

## LZMA SDK

`lzma/` and `VendorAPI/liblzma/` contain the C LZMA/LZMA2 subset of [LZMA SDK 26.04](https://www.7-zip.org/a/lzma2604.7z). The upstream files are unmodified; `lzma/lzma-sdk.txt` retains the SDK's public-domain dedication. The license URL points to the versioned archive containing that document because public domain has no single SPDX license identifier.

`lzma/lzma.mke` builds the portable C implementation with Windows threads or pthreads. `CpuArch.c` and `LzFindOpt.c` support the match finder; `MtDec.c` supplies progress synchronization used by `MtCoder.c`, and requires the stream helpers in `7zStream.c`. The decoder remains single-threaded. SDK assembly implementations are not included.

BeSQLite uses raw LZMA2 with its own container headers. Keep those headers and dictionary limits compatible when changing encoder settings. Test both old-reader/new-writer and new-reader/old-writer combinations, including embedded files and compressed databases. `CompressNuget` supplies the SDK through the `lzma` library and `liblzma` header domain.
