---
name: vcpkg
description: Authoritative guide for vcpkg library integration in imodel-native. USE FOR adding a new vcpkg-managed library, migrating an existing library to vcpkg, or updating a vcpkg-managed library version. Covers the sequential install chain, PartFile wiring, mke patterns, triplet selection, and version pinning.
---

# vcpkg Integration in imodel-native

All vcpkg documentation for humans lives in [`iModelCore/libsrc/VCPKG.md`](../../../iModelCore/libsrc/VCPKG.md).
This skill summarises the patterns an agent needs to get the build wiring right.

---

## The Sequential Install Chain

All `vcpkg install` calls run through a **single sequential chain** defined in
`iModelCore/libsrc/vcpkg.PartFile.xml`.  The chain currently is:

```
vcpkg (bootstrap)
  └─► vcpkg_install_compress
        └─► vcpkg_install_png
              └─► vcpkg_install_openssl
                    └─► vcpkg_install_crashpad
```

Each link is a separate Part with its own `vcpkg_install_<consumer>.mke` that calls
`vcpkg_run_install.ps1` / `vcpkg_run_install.sh`.  Chaining ensures no two `vcpkg`
processes ever run concurrently — concurrent runs against the same install root collide
on `vcpkg-running.lock` and corrupt the build.

**Consumer `.mke` files do NOT call `vcpkg_run_install` themselves.**  By the time a
consumer `.mke` runs, its install is already complete.

---

## Adding a New vcpkg Library

> **Always update `iModelCore/libsrc/README.md`** — add a row to the library table with the directory, library name, version, and `Yes` in the vcpkg column.

### 1. Create the manifest directory

Under `iModelCore/libsrc/<mylib>/`:
- `vcpkg.json` — list dependency with `version>=` under `dependencies` and exact version under `overrides`
- `vcpkg-configuration.json` — copy from an existing consumer (e.g. `compress/`); update `baseline` if needed
- `triplets/` — platform-specific triplet files if the defaults in `iModelCore/libsrc/` are not sufficient (see `compress/triplets/` for examples)

> **Check whether the library links cleanly into Windows DEBUG builds.** Some libraries fail to
> link into Windows DEBUG unless their debug artifact is made release-CRT-compatible — either by
> forcing release-only triplets (`set(VCPKG_BUILD_TYPE release)`) or by fixing up the vcpkg Debug
> config to link the release CRT while keeping its diagnostics; others are fine without any
> change. See the
> [Windows debug builds link the release CRT](#windows-debug-builds-link-the-release-crt)
> pitfall below to decide, and for both fixes. `crashpad/triplets/` and
> `pugixml/triplets/` are working examples of libraries that needed it.

### 2. Create `iModelCore/libsrc/vcpkg_install_<mylib>.mke`

```makefile
%include mdl.mki

mylibDir    = $(_MakeFilePath)<mylib>
installRoot = $(OutputRootDir)vcpkg_installed/<mylib>

# Add vcpkgWindowsMDCRT = 1 here if the library must link /MD on Windows (like openssl).
# Add vcpkgUseVeracodeTriplet = 1 here ONLY if this library's base triplet sets explicit
# -RTC flags and you have provided a triplets/x64-windows-static-veracode.cmake overlay
# that omits them (see compress/ and crashpad/). Libraries whose triplets set no -RTC
# flags need no veracode variant and should leave this undefined.
%include $(_MakeFilePath)vcpkg.mki

always:
%if defined (winNT)
    powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "$(_MakeFilePath)vcpkg_run_install.ps1" "$(mylibDir)" "$(installRoot)" "$(vcpkgTriplet)"
%else
    $(_MakeFilePath)vcpkg_run_install.sh $(mylibDir) $(installRoot) $(vcpkgTriplet)
%endif
```

`$(_MakeFilePath)` resolves to `libsrc/` because the file lives there, so the paths to
`<mylib>/`, `vcpkg_run_install.ps1`/`.sh`, and `vcpkg.mki` are all correct.

### 3. Extend the chain in `iModelCore/libsrc/vcpkg.PartFile.xml`

Insert a new link into the chain.  Appending at the end is the simplest choice, but the
chain only needs to stay **linear** — where a link sits does not affect correctness because
every consumer depends on its own named part.  Prefer placing a more basic/foundational
library (e.g. a compression or image codec that other libraries build on) earlier in the
chain, and insert a new link wherever it reads most naturally alongside its peers.

To append at the end (after the current last link):

```xml
<Part Name="vcpkg_install_<mylib>" BentleyBuildMakeFile="vcpkg_install_<mylib>.mke">
    <SubPart PartName="vcpkg_install_<current-last>" LibType="Static"/>
</Part>
```

To insert mid-chain, point the new part at its predecessor and re-parent the following
link onto the new part, keeping the chain linear.  Whichever position you choose, update
the sibling `.mke` comment ("Runs after vcpkg_install_<prev>…") on every link whose
predecessor changed.

### 4. Wire the consumer PartFile

In your library's `.PartFile.xml`, depend on the chain part with **`LibType="Static"`**.

**Critical:** `$(OutputRootDir)` differs between static and dynamic builds (`static/vcpkg_installed/…`
vs `vcpkg_installed/…`).  The chain always runs static-only; dynamic builds do **not** run
`vcpkg install` at all — their `.mke` reads the packages the static chain produced under
`static/vcpkg_installed/…` (see step 5).
Using `LibType="Static"` here ensures the static chain completes (and populates that install
root) before a dynamic build starts, without triggering a redundant dynamic chain build that
would race against the static one on the shared vcpkg git repo.

```xml
<Part Name="MyLib" BentleyBuildMakeFile="MyLib.mke">
    <!-- LibType="Static": chain is static-only.  Dynamic builds read the packages from the
         static install root; they do not run vcpkg themselves (see step 5). -->
    <SubPart PartName="vcpkg_install_<mylib>" PartFile="iModelCore/libsrc/vcpkg" LibType="Static"/>
    ...
```

If a separate prewire/PublicAPI part also needs the install, give it the same
`LibType="Static"` SubPart (see `BeOpenSSL.PartFile.xml` for the `__PublicAPI` example).

### 5. Write the consumer `.mke`

Include `vcpkg.mki` to get `vcpkgTriplet`, then set `vcpkgInstallRoot`.

**Key rule:** the chain always runs as Static, so the installed packages always live under
the **static** `OutputRootDir`.  Dynamic builds must redirect `vcpkgInstallRoot` to that
same static location — do **not** call `vcpkg_run_install` from the dynamic `.mke` path,
because this creates a concurrent vcpkg process that races against the static chain on
shared global locks (registry git lock, cmake download rename, etc.).

Use `CREATE_STATIC_LIBRARIES` (defined by bmake for static builds) to pick the right root:

```makefile
# Static builds: packages are at $(OutputRootDir)vcpkg_installed/<mylib>/
# Dynamic builds: chain ran as Static; redirect to the same location.
%if defined (CREATE_STATIC_LIBRARIES)
vcpkgInstallRoot = $(OutputRootDir)vcpkg_installed/<mylib>/
%else
vcpkgInstallRoot = $(OutputRootDir)static/vcpkg_installed/<mylib>/
%endif
vcpkgTripletDir  = $(vcpkgInstallRoot)$(vcpkgTriplet)/
vcpkgIncludeDir  = $(vcpkgTripletDir)include/
vcpkgLibDir      = $(vcpkgTripletDir)lib/
```

No `vcpkg_run_install` call in the `.mke` at all — the chain part handles it.

Libraries that **only** build as static (e.g. compress, crashpad client) can skip the
`%if defined (CREATE_STATIC_LIBRARIES)` conditional and use `$(OutputRootDir)vcpkg_installed/…`
directly — their `OutputRootDir` is always the static one.

**If this library needs the release-only Windows triplets** (see the pitfall below), also link
the release archive on Windows. Because those triplets set `VCPKG_BUILD_TYPE release`, no
`debug/lib/` archive is produced, so a `%if defined (DEBUG)` branch that reaches for
`$(vcpkgDbgLibDir)` would point at a nonexistent file. Gate the archive selection on the
platform first:

```makefile
%if $(TARGET_PLATFORM) == "Windows"
    # Windows always links the release archive: the Windows triplets force VCPKG_BUILD_TYPE
    # release because bmake links the release CRT even in DEBUG builds.
    vcpkgMyLib = $(vcpkgLibDir)$(myLibName)
%elif defined (DEBUG)
    vcpkgMyLib = $(vcpkgDbgLibDir)$(myLibName)
%else
    vcpkgMyLib = $(vcpkgLibDir)$(myLibName)
%endif
```

Libraries that do **not** need the release-only triplets keep the plain
`%if defined (DEBUG)` → `$(vcpkgDbgLibDir)` selection (e.g. `openssl/BeOpenSSL.mke`).

### 6. Migrating an existing (previously vendored) library

When the library you are moving to vcpkg was previously vendored (its source checked into
`iModelCore/libsrc/<mylib>/`), the vendored source deletion belongs in the **same** PR as the
vcpkg wiring, but do **not** delete it up front. Keep the vendored source in place (the PR will
likely be draft/WIP at this stage) until **after** the PR has passed its Copilot review, then
remove the vendored code in a separate standalone commit within that same PR. Deleting the
vendored source up front produces too many modified files for Copilot to review, and the review
may not run at all.

---

## Updating an Existing Library Version

> **Always update `iModelCore/libsrc/README.md`** — bump the version in the library table to match the new version in `vcpkg.json`.

1. Edit `iModelCore/libsrc/<consumer>/vcpkg.json`:
   - Update the `version>=` value under `dependencies`
   - Update the matching entry in `overrides`
2. If the new version requires a newer port registry, update `baseline` in
   `iModelCore/libsrc/<consumer>/vcpkg-configuration.json`.
3. No changes to `.mke` or `.PartFile.xml` files are needed — the next build will pick
   up the new version via the binary cache or a fresh build.

---

## Pitfall: Windows debug builds link the release CRT

<a id="windows-debug-builds-link-the-release-crt"></a>

The bmake link settings in this pipeline use the **release CRT even in Windows DEBUG builds**.
Bentley wrapper objects (e.g. `BePugiXml`, the crashpad client wrapper) are therefore compiled
`MD_DynamicRelease` with `_ITERATOR_DEBUG_LEVEL=0`.

**This does not affect every library — only those whose debug artifact actually depends on the
debug CRT.** A vcpkg debug build is compiled with `_DEBUG` defined. If the library's source
gates on `_DEBUG` at compile time and then calls debug-CRT-only functionality (or raises
`_ITERATOR_DEBUG_LEVEL` to 2), that debug artifact references symbols that do not exist in the
release CRT, so linking it into our release-CRT DEBUG build fails with unresolved-symbol /
CRT-mismatch / `_ITERATOR_DEBUG_LEVEL` (2 vs 0) errors. Libraries whose debug artifact does not
touch debug-CRT-only functionality link fine and have deliberately been left on the normal
debug/release selection (e.g. `openssl`).

**So this is a per-library check, not a blanket rule.** If a new Windows vcpkg library links
cleanly in a DEBUG build, leave it alone. If its DEBUG link fails with the symptoms above, you
have two ways to fix it.

### Fix A (simplest): make the library release-only on Windows

Produce only a release artifact and link it everywhere. Two halves, do both:

1. **Force release-only builds in the Windows triplets.** Add `set(VCPKG_BUILD_TYPE release)` to
   every Windows triplet the library uses (`x64-windows-static.cmake`, the `-clang` variant, and
   any `-md` / `-veracode` variants). This stops vcpkg from producing a `debug/lib/` archive at
   all.
2. **Always link the release archive on Windows in the consumer `.mke`.** Select the archive
   with a `%if $(TARGET_PLATFORM) == "Windows"` branch that uses `$(vcpkgLibDir)` (release)
   *before* any `%if defined (DEBUG)` branch (see step 5).

The tradeoff: the Release config also defines `NDEBUG` (stripping the library's internal
`assert()`s) and optimizes, so you lose the library's debug diagnostics on Windows DEBUG only
(other platforms keep their debug archive). For most libraries that is an acceptable price for a
one-line fix.

### Fix B (preserves debug diagnostics): keep the debug archive, force it onto the release CRT

If you want to keep the Windows DEBUG archive (asserts, unoptimized code) and only the CRT is the
problem, fix up the vcpkg **Debug** config to use the release CRT instead of dropping it. Leave
both configs building and, in each Windows triplet, pin the Debug runtime library and iterator
level:

```cmake
set(VCPKG_CMAKE_CONFIGURE_OPTIONS_DEBUG "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL")
set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=0")
set(VCPKG_C_FLAGS_DEBUG   "${VCPKG_C_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=0")
```

The Debug config keeps `NDEBUG` undefined, so `assert()`s survive; only the CRT/iterator level
change to match our build. The consumer `.mke` keeps the normal `%if defined (DEBUG)` →
`$(vcpkgDbgLibDir)` selection, since a `debug/lib/` archive is still produced. This works only if
the library's CMake honors the cache-set `CMAKE_MSVC_RUNTIME_LIBRARY` (CMP0091) — most modern
ports do — and it is more machinery: apply it to **every** Windows triplet and validate a real
Windows DEBUG link on **both** the MSVC and clang-cl toolsets. `pugixml/triplets/x64-windows-static.cmake`
documents this recipe in its header comment; pugixml itself ships Fix B to preserve debug diagnostics.

This has bitten two libraries so far — crashpad (first) and pugixml (second). Reference
implementations: `pugixml/triplets/*.cmake` + `pugixml/pugixml.mke`, and
`crashpad/triplets/*.cmake` + `crashpad/client.mke`.

---

## Triplet Selection (`vcpkg.mki`)

`iModelCore/libsrc/vcpkg.mki` maps `TARGET_PROCESSOR_ARCHITECTURE` to `vcpkgTriplet`:

| Architecture        | Triplet                     | Notes                                      |
|---------------------|-----------------------------|--------------------------------------------|
| `x64`               | `x64-windows-static`        | Default Windows; set `vcpkgWindowsMDCRT=1` before `%include` for `-md` variant |
| `x64` + `vcpkgWindowsMDCRT=1` | `x64-windows-static-md` | OpenSSL and other /MD libs on Windows |
| `MacOSARM64`        | `arm64-osx`                 |                                            |
| `LinuxX64`          | `x64-linux`                 |                                            |
| `AndroidARM64`      | `arm64-android`             |                                            |
| `AndroidX64`        | `x64-android`               |                                            |
| `iOSARM64`          | `arm64-ios`                 |                                            |

---

## Key Files

| File | Purpose |
|------|---------|
| `iModelCore/libsrc/vcpkg.PartFile.xml` | Sequential chain — edit to add new install parts |
| `iModelCore/libsrc/vcpkg_install_*.mke` | One file per consumer; calls `vcpkg_run_install` |
| `iModelCore/libsrc/vcpkg.mki` | Triplet selection; include from any install or consumer mke |
| `iModelCore/libsrc/vcpkg_run_install.ps1` / `.sh` | Wrapper that invokes the `vcpkg` executable |
| `iModelCore/libsrc/VCPKG.md` | Human-facing documentation; keep in sync when changing patterns |
