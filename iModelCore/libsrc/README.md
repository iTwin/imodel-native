# Third-Party Libraries

Libraries built in `iModelCore/libsrc/`.

| Directory | Library | Version | vcpkg? |
|-----------|---------|---------|--------|
| `compress/zlib` | zlib | 1.3.2#3 (minizip 1.3.2) | Yes |
| `compress/lzma` | LZMA SDK | 26.04 | No |
| `compress/snappy` | Google Snappy | 1.2.2 | No |
| `crashpad` | Chromium Crashpad | 2024-04-11#13 | Yes |
| `csmap` | CS-MAP | rev 2778 | No |
| `curl` | libcurl | 8.23.0 | Yes |
| `curl` | c-ares (macOS/Linux only) | 1.34.8 | Yes |
| `facebook` | Facebook Folly | 57.0 | No |
| `flatbuffers` | FlatBuffers | 1.12.0 | No |
| `freetype2` | FreeType | 2.15.0 | Yes |
| `google_re2` | Google RE2 | SONAME 9 | No |
| `icu4c` | ICU4C | 78.1 | No |
| `ios-cmake` | ios-cmake | unknown | No |
| `libjpegturbo` | libjpeg-turbo | 3.1.2 | No |
| `openssl` | OpenSSL | 3.5.9 | Yes |
| `png` | libpng | 1.6.59 | Yes |
| `proxyres` | proxyres | unknown | No |
| `pugixml` | pugixml | 1.15 | Yes |
| `rapidjson` | RapidJSON | 1.1.0 | No |

Only the vcpkg consumers built by iTwinNativeThirdParty require iOS ARM64 Simulator support:
`compress` (zlib/minizip), `png` (libpng), `openssl`, `curl` (libcurl), and `freetype2` (FreeType).
See [iOS ARM64 Simulator support](VCPKG.md#ios-arm64-simulator-support) for the triplets and
install-chain details. Other imodel-native libraries do not require simulator support for this build.
