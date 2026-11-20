# zstd

[Zstandard](https://github.com/facebook/zstd) is installed by vcpkg (see [VCPKG.md](../VCPKG.md)) and delivered by
the `Zstd` part as the static library `BeZstd`, with its headers in `VendorAPI/zstd` and its license as
`zstd-license.txt`. It is built for Windows, Linux and macOS only.

## Build options

The triplets change these options of the vcpkg port:

- `ZSTD_LEGACY_SUPPORT=OFF`: frames written by zstd versions before 0.8 cannot be decoded.
- `ZSTD_MULTITHREAD_SUPPORT=OFF`: compression always runs on the calling thread, so the library does not need
  pthreads. Setting `ZSTD_c_nbWorkers` above 0 fails.
- Linux and macOS: zstd's symbols are hidden (`-fvisibility=hidden`, with `ZSTDLIB_VISIBLE`, `ZSTDERRORLIB_VISIBLE`
  and `ZDICTLIB_VISIBLE` set to `hidden`), so that they cannot bind to another zstd loaded in the same process, such
  as Node's. After an update, check the archive's symbols as [VCPKG.md](../VCPKG.md) describes (`nm -m` on macOS
  reports them as `private external`).
- Windows: only the release library is built (`VCPKG_BUILD_TYPE release`) and linked, in debug builds too. bmake
  links the release CRT in debug builds, and zstd's CMake ignores `CMAKE_MSVC_RUNTIME_LIBRARY`, so a debug archive
  could not be switched to the release CRT.

## License

zstd is dual licensed under BSD-3-Clause or GPL-2.0-only; Bentley uses it under BSD-3-Clause.
`zstd-license.txt` is the port's `copyright` file, which contains both license texts.

## Updating

Change the version and its override in [vcpkg.json](vcpkg.json) and the baseline in
[vcpkg-configuration.json](vcpkg-configuration.json) as [VCPKG.md](../VCPKG.md) describes. Check that the port's
CMake options still match the ones above, then update the version in [../README.md](../README.md).
