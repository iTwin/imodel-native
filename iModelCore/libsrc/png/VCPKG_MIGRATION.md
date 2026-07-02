# Migrating the libpng build to vcpkg

This document enumerates the steps to replace the current build-from-source libpng
(`iModelCore/libsrc/png`, currently **1.6.56**, compiled file-by-file by
[`png.mke`](png.mke)) with a vcpkg-driven build, and to bump the version to **1.6.58**
at the same time.

The model to follow is the existing **compress** (zlib + minizip) vcpkg integration —
[`../compress/Zlib.mke`](../compress/Zlib.mke), [`../compress/vcpkg.json`](../compress/vcpkg.json),
[`../compress/vcpkg-configuration.json`](../compress/vcpkg-configuration.json),
[`../compress/triplets/`](../compress/triplets) — **not** OpenSSL. The key differences from
OpenSSL:

- **libpng is static on every platform, including Windows.** There is no DLL, no `.def`, no
  import lib, and no dynamic-vs-static split. This makes it a near-exact twin of `compress`.
- No custom version-string section (`BSIVer`), no hand-rolled assembly, and no export audit.

See the shared guide in [`../VCPKG.md`](../VCPKG.md) and the agent skill
`.github/skills/vcpkg/SKILL.md` for the canonical wiring rules.

---

## Current state (what we are replacing)

| Aspect | Today |
|--------|-------|
| Version | 1.6.56 (see [`vendor/CMakeLists.txt`](vendor/CMakeLists.txt) `PNGLIB_REVISION 56`) |
| Build | [`png.mke`](png.mke) compiles the `vendor/*.c` files into `BePng` (static only, `CREATE_STATIC_LIBRARIES = 1`) |
| Delivered lib | `Delivery/$(stlibprefix)BePng$(stlibext)` (+ debug variant) |
| Delivered headers | `VendorAPI/png/` ← `png.h`, `pngconf.h`, `pnginfo.h`, `pnglibconf.h`, `pngstruct.h` |
| License | `Delivery/png-license.txt` ← `vendor/LICENSE` |
| zlib dependency | Supplied by our `Zlib` part (SubPart in [`png.PartFile.xml`](png.PartFile.xml)) |
| Special defines | `PNG_ARM_NEON_OPT=0` (works around an unresolved `png_init_filter_functions_neon` on iOS) |
| Consumers | **Only** [`../../iModelPlatform/DgnCore/ImageSource.cpp`](../../iModelPlatform/DgnCore/ImageSource.cpp) via `#include <png/png.h>` (public API only). Delivered lib is linked by `iModelPlatform` and pulled into `imodeljs.node` (see [`../../../iModelJsNodeAddon/IModelJsNative_input_libs.mki`](../../../iModelJsNodeAddon/IModelJsNative_input_libs.mki), `libBePng.a`). |
| PartFile parts | `pnglib` (main), `png_VS2012` (legacy VS2012 variant), `PngNuget` product |

**Deliverable names must not change.** `BePng` and `VendorAPI/png/png.h` are the contract with
`iModelPlatform` and the node addon; keep them identical.

---

## Step 1 — ✅ Add the vcpkg manifest

> **Done.** Created [`vcpkg.json`](vcpkg.json) and [`vcpkg-configuration.json`](vcpkg-configuration.json).
> libpng **1.6.58** is confirmed available at the pinned baseline `81de6771…` (verified against
> the registry `versions/l-/libpng.json`), so no baseline bump and no overlay port were needed.
> The `zlib` override (`1.3.2`, matching `compress`) was included.

Create, in this folder (mirroring [`../compress/`](../compress)):

- **`vcpkg.json`** — declare the `libpng` dependency, pinned to 1.6.58:

  ```json
  {
    "name": "imodel-native-png",
    "version": "1.0.0",
    "dependencies": [
      { "name": "libpng", "version>=": "1.6.58" }
    ],
    "overrides": [
      { "name": "libpng", "version": "1.6.58" }
    ]
  }
  ```

  > **zlib note:** libpng depends on zlib. vcpkg will build its own private zlib inside the png
  > install root to *compile* libpng; that is fine and isolated. The **symbols** at final link
  > still come from our `BeZlib` (the `Zlib` SubPart is retained, see Step 6). For determinism,
  > consider adding an `override` pinning `zlib` to the same version `compress` uses (`1.3.2`)
  > so both install roots build the identical zlib. This is optional but recommended.

- **`vcpkg-configuration.json`** — copy verbatim from [`../compress/vcpkg-configuration.json`](../compress/vcpkg-configuration.json)
  (each manifest dir needs its own copy even though they are identical). It pins the registry
  `baseline` commit.

  > **Baseline / version availability check:** ✅ confirmed — the registry at the `compress`
  > baseline commit (`81de6771512413aaf89ea77add5ad1fda126b9d0`) packages libpng **1.6.58**
  > (it is the newest entry in `versions/l-/libpng.json` at that commit). No baseline bump and
  > no `overlay-ports/libpng/` were required.

---

## Step 2 — ✅ Add overlay triplets  (static lib, dynamic CRT on Windows)

> **Done.** Created [`triplets/`](triplets) with six files —
> [`x64-windows-static.cmake`](triplets/x64-windows-static.cmake),
> [`arm64-osx.cmake`](triplets/arm64-osx.cmake), [`arm64-ios.cmake`](triplets/arm64-ios.cmake),
> [`x64-linux.cmake`](triplets/x64-linux.cmake), [`arm64-android.cmake`](triplets/arm64-android.cmake),
> [`x64-android.cmake`](triplets/x64-android.cmake). Each sets `VCPKG_LIBRARY_LINKAGE static`
> (+ `VCPKG_CRT_LINKAGE dynamic` on Windows) and the platform/system settings mirrored from
> `compress`, but omits the minizip-only `NOCRYPT`/`NOUNCRYPT` defines and the `/RTC` debug
> flags. Because no triplet sets any `-RTC` flag, **no veracode overlay triplet is needed** and
> `vcpkgUseVeracodeTriplet` must stay undefined (Steps 3 & 5).

Create `triplets/` mirroring [`../compress/triplets/`](../compress/triplets), but **simplified**:

- Keep `set(VCPKG_LIBRARY_LINKAGE static)` and `set(VCPKG_CRT_LINKAGE dynamic)` on Windows so
  the archive matches the rest of the Bentley build (`/MD`), and force static linkage on
  macOS/iOS/Android (whose vcpkg built-in triplets default to dynamic).
- **Drop** the compress-specific bits: the `NOCRYPT`/`NOUNCRYPT` defines (minizip-only) and the
  `/RTCsu` debug runtime-check flags.
- Because these triplets set **no explicit `-RTC` flags**, libpng needs **no** veracode overlay
  triplet and must **not** set `vcpkgUseVeracodeTriplet` (see Step 3 / Step 6). This matches the
  OpenSSL precedent noted in [`../vcpkg.mki`](../vcpkg.mki).

Triplet files to create (one per platform we ship):
`arm64-osx.cmake`, `arm64-ios.cmake`, `arm64-android.cmake`, `x64-android.cmake`,
`x64-linux.cmake`, `x64-windows-static.cmake`.

Windows CRT: rely on the base `x64-windows-static` triplet name from [`../vcpkg.mki`](../vcpkg.mki)
with `VCPKG_CRT_LINKAGE dynamic` set inside the overlay (as compress does). Do **not** set
`vcpkgWindowsMDCRT` (that would switch to the built-in `x64-windows-static-md` triplet and skip
our overlay).

> **libpng feature selection:** the default libpng port builds the standard read/write library
> with zlib. It exposes optional features (e.g. `tools`, `apng`) that we do not want — take the
> defaults (no extra features). If NEON handling needs disabling (see Step 8), that is done via
> `VCPKG_C_FLAGS` in the triplet, not a feature.

---

## Step 3 — ✅ Add `vcpkg_install_png.mke`

> **Done.** Created [`../vcpkg_install_png.mke`](../vcpkg_install_png.mke), mirroring
> [`../vcpkg_install_compress.mke`](../vcpkg_install_compress.mke) but **without**
> `vcpkgUseVeracodeTriplet` (the png triplets set no `-RTC` flags). It installs the png manifest
> into `$(OutputRootDir)vcpkg_installed/png/`; `vcpkg_run_install.{bat,sh}` auto-passes
> `--overlay-triplets=$(pngDir)triplets` because that directory exists.

Create `iModelCore/libsrc/vcpkg_install_png.mke` next to the other `vcpkg_install_*.mke`
files, mirroring [`../vcpkg_install_compress.mke`](../vcpkg_install_compress.mke) but **without**
`vcpkgUseVeracodeTriplet`:

```makefile
%include mdl.mki

pngDir      = $(_MakeFilePath)png/
installRoot = $(OutputRootDir)vcpkg_installed/png/

%include $(_MakeFilePath)vcpkg.mki

always:
%if defined (winNT)
    $(_MakeFilePath)vcpkg_run_install.bat $(pngDir) $(installRoot) $(vcpkgTriplet)
%else
    $(_MakeFilePath)vcpkg_run_install.sh $(pngDir) $(installRoot) $(vcpkgTriplet)
%endif
```

`vcpkg_run_install.{bat,sh}` automatically pass `--overlay-triplets=$pngDir/triplets` because
that directory exists.

---

## Step 4 — ✅ Extend the sequential install chain

> **Done.** Inserted `vcpkg_install_png` into the chain in
> [`../vcpkg.PartFile.xml`](../vcpkg.PartFile.xml) **after compress and before openssl** with
> `<SubPart PartName="vcpkg_install_compress" LibType="Static"/>` (and openssl now chains off
> `vcpkg_install_png`), keeping it static-only. Updated the "How It Works" chain description
> in [`../VCPKG.md`](../VCPKG.md) to `compress → png → openssl → crashpad` and added `png.mke`
> to the consumer-mke list.

In [`../vcpkg.PartFile.xml`](../vcpkg.PartFile.xml), append a new link at the **end** of the
chain (after `vcpkg_install_crashpad`) so no two vcpkg processes ever run concurrently:

```xml
<Part Name="vcpkg_install_png" BentleyBuildMakeFile="vcpkg_install_png.mke">
    <!-- LibType="Static": the chain always runs static-only. -->
    <SubPart PartName="vcpkg_install_crashpad" LibType="Static"/>
</Part>
```

Update the block comment above the chain to name `vcpkg_install_png` as the new last link, and
update the chain description in [`../VCPKG.md`](../VCPKG.md) ("How It Works" mentions the chain
`compress → openssl → crashpad`).

---

## Step 5 — ✅ Rewrite `png.mke`  (compress/Zlib.mke style, static-only)

> **Done.** [`png.mke`](png.mke) rewritten as a thin static-only makefile modeled on
> [`../compress/Zlib.mke`](../compress/Zlib.mke). Details confirmed against the libpng vcpkg
> port (`ports/libpng/portfile.cmake`):
> - **Lib name:** vcpkg produces a single archive whose file name literally starts with `lib`
>   on every platform — `libpng16.a` / `libpng16.lib` (release) and `libpng16d.a` /
>   `libpng16d.lib` (debug). The mke selects it with a `%if defined (DEBUG)` guard and
>   re-archives it into `BePng` via the per-platform tool (`libtool` / `ar qcL` /
>   `merge_static_libs.sh` / `lib -OUT`), so the delivered `$(stlibprefix)BePng$(stlibext)`
>   name is unchanged.
> - **Headers:** delivers the three public headers (`png.h`, `pngconf.h`, `pnglibconf.h`) from
>   the vcpkg `include/` dir to `VendorAPI/png/`. The internal `pnginfo.h` / `pngstruct.h` are
>   no longer delivered (vcpkg ships only public headers); the sole consumer `ImageSource.cpp`
>   does not include them (still to be re-confirmed downstream in Step 8).
> - **iOS NEON:** the old `PNG_ARM_NEON_OPT=0` workaround is dropped — the port already sets
>   `-DPNG_HARDWARE_OPTIMIZATIONS=OFF` for iOS, so the previous `png_init_filter_functions_neon`
>   link error cannot recur there.
> - **License:** delivered from `share/libpng/copyright`.
>
> Also dropped the VS2012 (`VC_Version`) handling from this makefile; the legacy `png_VS2012`
> part is addressed separately in Step 6.
>
> **Build-verified correction (macOS arm64):** although png delivers only a static lib, the
> `pnglib` part can be built in a *dynamic* context (e.g. requested directly under the
> `iModelJsNodeAddon` strategy). The install chain always runs static-only, so the packages
> land under `.../static/vcpkg_installed/png/`. The `vcpkgInstallRoot` is therefore gated on
> `CREATE_STATIC_LIBRARIES` (static builds use `$(OutputRootDir)vcpkg_installed/png/`; dynamic
> builds redirect to `$(OutputRootDir)static/vcpkg_installed/png/`) — the same pattern the skill
> prescribes. The first test build failed without this gate (`png.h ... desired target does not
> exist`) because the dynamic `OutputRootDir` omits the `static/` segment. vcpkg confirmed
> **libpng 1.6.58** builds with our overlay triplet, and the headers install directly to
> `include/{png.h,pngconf.h,pnglibconf.h}` (release lib `lib/libpng16.a`).

Replace the file-by-file compile makefile with a thin one modeled on
[`../compress/Zlib.mke`](../compress/Zlib.mke). libpng is **static on all platforms**, so there
is **no** `CREATE_STATIC_LIBRARIES` conditional and no dynamic path — the chain always installs
under `$(OutputRootDir)vcpkg_installed/png/`.

1. Resolve the triplet: `%include $(libsrcDir)vcpkg.mki` (do **not** set
   `vcpkgUseVeracodeTriplet`).
2. Set the install-root paths:

   ```makefile
   vcpkgInstallRoot = $(OutputRootDir)vcpkg_installed/png/
   vcpkgTripletDir  = $(vcpkgInstallRoot)$(vcpkgTriplet)/
   vcpkgIncludeDir  = $(vcpkgTripletDir)include/
   vcpkgLibDir      = $(vcpkgTripletDir)lib/
   vcpkgDbgLibDir   = $(vcpkgTripletDir)debug/lib/
   ```

3. **Deliver headers** to `VendorAPI/png/`. Consumers use `#include <png/png.h>` and only touch
   the public API, so stage the three public headers vcpkg installs:
   `png.h`, `pngconf.h`, `pnglibconf.h`.

   ```makefile
   always:
       ~linkfile "$(BuildContext)VendorAPI/png/png.h=$(vcpkgIncludeDir)png.h"
       ~linkfile "$(BuildContext)VendorAPI/png/pngconf.h=$(vcpkgIncludeDir)pngconf.h"
       ~linkfile "$(BuildContext)VendorAPI/png/pnglibconf.h=$(vcpkgIncludeDir)pnglibconf.h"
   ```

   > **Internal headers:** the old build also delivered `pnginfo.h` and `pngstruct.h`. vcpkg
   > installs only the **public** libpng headers, so these two are not available. Our sole
   > consumer (`ImageSource.cpp`) does **not** include them, so dropping them is expected to be
   > safe — **verify** no downstream repo relies on `VendorAPI/png/pnginfo.h` /
   > `pngstruct.h` before deleting. If one does, extract them from the source archive as a
   > fallback, but treat that as a red flag (internal headers are unstable across versions).
   > vcpkg may place headers under `include/libpng16/`; if `include/png.h` is absent, point the
   > `~linkfile` sources at `$(vcpkgIncludeDir)libpng16/`.

4. **Deliver the static library** as `BePng`. libpng builds a single archive; just re-deliver
   it under our name (a lib-merge like Zlib's zlib+minizip is unnecessary — there is only one
   input). Handle the platform lib name and debug suffix:

   - Non-Windows: `libpng16.a` (release) / `libpng16d.a` (debug) — confirm the debug `d` suffix
     from the actual vcpkg output.
   - Windows: `libpng16.lib` (release) / `libpng16d.lib` (debug).

   ```makefile
   BePngOut = $(o)$(stlibprefix)$(appName)$(stlibext)
   %if defined (DEBUG)
       vcpkgPng = $(vcpkgDbgLibDir)$(stlibprefix)libpng16d$(stlibext)   # verify suffix
   %else
       vcpkgPng = $(vcpkgLibDir)$(stlibprefix)libpng16$(stlibext)
   %endif

   always:
       ~linkfile "$(BuildContext)Delivery/$(stlibprefix)$(appName)$(stlibext)=$(vcpkgPng)"
   ```

   > If simply re-linking the vcpkg archive under a new name causes trouble on any platform,
   > fall back to the exact per-platform merge idiom in [`../compress/Zlib.mke`](../compress/Zlib.mke)
   > (`libtool -static` / `ar qcL` / `merge_static_libs.sh` / `lib -OUT`) with libpng as the
   > single input. Keep `appName = BePng` so the delivered name is unchanged.

5. **Deliver the license** from the vcpkg tree:

   ```makefile
   always:
       ~linkfile "$(BuildContext)Delivery/png-license.txt=$(vcpkgTripletDir)share/libpng/copyright"
   ```

6. **Drop** the obsolete machinery from the old `png.mke`: the `MultiCppCompile*` rules, the
   per-`.c` object dependencies, `linkLibrary.mki`, the `DLM_*` block, and the
   `PNG_ARM_NEON_OPT=0` define (NEON is now the port's concern — but see Step 8 for the iOS
   verification item).

Follow the crashpad/compress build-ordering lesson: do link/deliver steps as ordered `always:`
blocks (not dependency rules) so they run after the vcpkg install completed by the chain part.

---

## Step 6 — ✅ Update the PartFile

> **Done.** Updated [`png.PartFile.xml`](png.PartFile.xml):
> - `pnglib` now depends on `<SubPart PartName="vcpkg_install_png" PartFile="iModelCore/libsrc/vcpkg" LibType="Static"/>`
>   (chain runs first, static-only) and keeps the `Zlib` SubPart (supplies zlib symbols at the
>   final link).
> - Bindings switched to the vcpkg static-lib pattern used by `BeZlib`:
>   `ProductDirectoryName="BeStaticLibDir" Required="false"`, with the debug variant marked
>   `IfNotPresent="Continue"` (the mke delivers a single `$(stlibext)`-named archive, so the
>   debug-suffixed binding must be optional). Verified iModelPlatform consumes `BePng` via
>   `$(ContextSubPartsStaticLibs)` exactly like `BeZlib`, so `BeStaticLibDir` is correct.
> - **Removed the `png_VS2012` part** — a grep confirmed no other part references it or
>   `BePng_VC11`, and the rewritten `png.mke` no longer honors `BUILD_USING_VS2012`.
> - `PngNuget` product unchanged. No explicit rebuild-input element needed (vcpkg's manifest
>   hashing handles it), matching crashpad/compress.

In [`png.PartFile.xml`](png.PartFile.xml):

- Add the chain SubPart to the `pnglib` part, with **`LibType="Static"`** (the chain is
  static-only):

  ```xml
  <Part Name="pnglib" BentleyBuildMakeFile="png.mke">
      <SubPart PartName="vcpkg_install_png" PartFile="iModelCore/libsrc/vcpkg" LibType="Static"/>
      <SubPart LibType="Static" PartName="Zlib" PartFile="iModelCore\libsrc\compress\Zlib"/>
      <Bindings>
          <VendorAPI Domain="png"/>
          <Libs>Delivery/$(stlibprefix)BePng$(stlibext)</Libs>
          <Libs>Delivery/$(stlibprefix)BePng$(stlibdebugext)</Libs>
          <VendorNotices>Delivery/png-license.txt</VendorNotices>
      </Bindings>
  </Part>
  ```

  > Keep the existing `Zlib` SubPart — it supplies the zlib **symbols** at the final link and
  > guarantees `BeZlib` is built first. Keep the `Bindings` (`VendorAPI Domain="png"`, the two
  > `BePng` libs, the license) exactly as-is so downstream parts are unaffected.

- **`png_VS2012` part:** this legacy VS2012 variant compiled the sources with an old toolset.
  vcpkg does not support that toolset. Decide with the owner whether VS2012 is still required;
  most likely it can be **removed** along with the `vendor/` source tree (Step 9). If it must
  stay, it cannot use vcpkg and would need the old source retained — flag this explicitly.

- **`PngNuget` product:** unchanged; it packages the same `pnglib` deliverables.

- **Manifest rebuild inputs:** as with crashpad/compress, no explicit PartFile `input` element
  is needed — the `always:` install step runs every build and vcpkg's own manifest hashing
  rebuilds when `vcpkg.json` / the triplets change.

---

## Step 7 — 🔲 Update `README.md` and docs

- [`../README.md`](../README.md) library table: bump the `png` row version `1.6.56` → `1.6.58`
  and change the vcpkg column from `No` → `Yes`.
- [`../VCPKG.md`](../VCPKG.md): add libpng to the chain description in "How It Works"
  (`compress → openssl → crashpad → png`) and the consumer-mke list.

---

## Step 8 — 🔲 Verify

1. Build every target platform: Windows (x64), Linux (x64), macOS (arm64), iOS (arm64),
   Android (arm64 and x64). Confirm libpng resolves at version 1.6.58 in each install root.
2. Confirm the delivered `BePng` archive and `VendorAPI/png/{png.h,pngconf.h,pnglibconf.h}`
   appear with the exact old names.
3. **iOS NEON check.** The old build set `PNG_ARM_NEON_OPT=0` to dodge an unresolved
   `png_init_filter_functions_neon`. With vcpkg the port controls NEON; confirm the iOS
   (`arm64-ios`) link succeeds. If it fails, disable NEON via the iOS/arm64 triplets by adding
   `set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -DPNG_ARM_NEON_OPT=0")` (and the `arm64-android` triplet
   if it regresses there too), which reproduces the old behavior at the vcpkg build level.
4. Functional test: exercise PNG decode/encode through `ImageSource` (the only consumer) — e.g.
   the image round-trip paths in iModelPlatform tests — on at least macOS and Windows.
5. Confirm `imodeljs.node` still links (`libBePng.a` is consumed via
   [`../../../iModelJsNodeAddon/IModelJsNative_input_libs.mki`](../../../iModelJsNodeAddon/IModelJsNative_input_libs.mki)).
6. Confirm no downstream repo referenced the now-removed `pnginfo.h` / `pngstruct.h`.

---

## Step 9 — 🔲 Cleanup once green

- Delete the checked-in `vendor/` libpng source tree.
- Remove the `png_VS2012` part (if retired in Step 6) and `PngNugetLicense.json` references that
  point at `vendor/`.
- Update [`.gitignore`](.gitignore) if it referenced `vendor/` build artifacts.
- Fold any remaining libpng build notes into this file / [`../VCPKG.md`](../VCPKG.md).
