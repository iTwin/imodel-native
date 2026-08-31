---
name: vcpkg
description: Authoritative guide for vcpkg library integration in imodel-native. USE FOR adding a new vcpkg-managed library, migrating an existing library to vcpkg, updating a vcpkg-managed library version, or diagnosing a failing Mend source scan. Covers the sequential install chain, PartFile wiring, mke patterns, triplet selection, version pinning, and Mend source-scan configuration (`vcpkg-mend.json`).
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
              └─► vcpkg_install_pugixml
                    └─► vcpkg_install_openssl
                          └─► vcpkg_install_curl
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
- `vcpkg-mend.json` — list the triplet graph(s) whose union downloads all upstream source used by the consumer; the scan never compiles for the selected target, so triplets need not match the Mend host; prefer one source-superset graph, and add multiple triplets only for platform-specific downloads
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

Insert a new link into the chain.  The chain must stay **linear**, and — with one important
exception below — where a link sits does not affect correctness because every consumer depends on
its own named part.  Prefer placing a more basic/foundational library (e.g. a compression or image
codec that other libraries build on) earlier in the chain, and insert a new link wherever it reads
most naturally alongside its peers.

**Platform-coverage constraint (do not violate).** The real invariant is that the chain must be
**linear on every individual platform**: no two `vcpkg_install_*` parts that are *both built on a
given platform* may run without a dependency edge between them, or they race on
`vcpkg-running.lock` and corrupt the build.  Across platforms the graph may **fork into a tree** —
only each platform's projection of it has to be a single line.  Consequences:

- A chain link must depend on a predecessor built on **at least the same platforms** as the link
  itself.  Some links are platform-restricted (e.g. `vcpkg_install_crashpad` carries
  `OnlyPlatforms="linux*,x*,macos*"` — desktop only).  If an all-platform link were appended after a
  platform-restricted one, then on the excluded platforms (iOS/Android) its predecessor is skipped,
  the link loses its chain predecessor, and it can run concurrently with an earlier link.
- **Keep platform-restricted links at (or near) the tail**, after every all-platform link.  When
  adding an **all-platform** link, depend it on the last **all-platform** predecessor (not on a
  platform-restricted one).  If a restricted link currently sits where the new one belongs, insert
  the new link **before** it and re-parent the restricted link onto the new one.
- Two links whose platform sets **overlap** must stay linearly ordered (one depends on the other).
- Two links whose platform sets are **disjoint** never build on the same platform, so they can never
  run concurrently — they may safely **fork** off a common all-platform predecessor.  This is when
  the tail becomes a tree: e.g. a hypothetical iOS-only library and desktop-only crashpad would each
  depend on the last all-platform link (`vcpkg_install_curl`), one branch live on iOS, the other on
  desktop.  Do **not** instead chain a desktop-only link behind an iOS-only link (or vice-versa):
  that edge is dead on *both* platforms yet still strands one link without a predecessor.

Example: curl (all platforms) was inserted **before** crashpad (desktop-only) — `curl → openssl`,
and crashpad was re-parented from openssl onto curl.  On iOS/Android crashpad is skipped and curl
is the tail, still ordered after openssl.

To append at the end (only when the current last link is built on a superset of your platforms):

```xml
<Part Name="vcpkg_install_<mylib>" BentleyBuildMakeFile="vcpkg_install_<mylib>.mke">
    <SubPart PartName="vcpkg_install_<current-last>" LibType="Static"/>
</Part>
```

To insert mid-chain, point the new part at its predecessor and re-parent the following
link onto the new part, keeping the chain linear.  Whichever position you choose, update
the sibling `.mke` comment ("Runs after vcpkg_install_<prev>…") on every link whose
predecessor changed.

Also add the new part to the `vcpkg_install_all` aggregate so the shared binary-cache warmer includes
it. Mend source scanning treats a directory as a consumer only when it holds all three of
`vcpkg.json`, `vcpkg-configuration.json`, and `vcpkg-mend.json`; anything else is skipped, so
vendored upstream trees that ship their own `vcpkg.json` are harmless. Because an omitted
`vcpkg-mend.json` would just skip the library, `vcpkg_run_install.ps1` and `vcpkg_run_install.sh`
refuse to install a manifest directory that has no `vcpkg-mend.json` with a non-empty `triplets`
array — so the omission fails that library's normal build on every platform. Every configured
triplet must also have a matching overlay file. Select the smallest triplet set whose manifest and portfile branches
cover all upstream downloads, and make sure it reaches every port pinned in `overrides` — the scan
verifies that the ports named in `dependencies` **and** `overrides` all produced extracted source.
The selected triplet does not need to match the Mend host because nothing is compiled for that
target; for example, curl's `x64-linux` graph includes its common sources plus conditional c-ares,
so no Windows graph is needed even though Mend runs on Windows. The host still needs a working
toolchain of its own, since vcpkg builds host-triplet helper ports (`vcpkg-cmake` and friends)
either way. The cross-consumer checks the wrappers cannot make — a misplaced `vcpkg-mend.json`, or
a triplet with no overlay file — also run during Windows builds via the `vcpkg_validate_mend` part,
so those mistakes fail the PR rather than the Mend pipeline.

#### Auditing platform coverage

Every consumer currently lists only `x64-linux`, which is a source-superset claim rather than a
default. Nothing enforces it: the materialization check only sees ports named in `dependencies` and
`overrides`, so a download that happens solely on an unlisted platform is skipped in silence. Two
things put a download there, and both are easy to grep for:

```bash
# platform-qualified dependency or feature (curl's c-ares is "osx | linux")
grep -rn --include='vcpkg.json' '"platform"' iModelCore/libsrc/
# source fetch inside a target branch (crashpad pulls linux-syscall-support on Linux/Android only)
grep -rn --include='*.cmake' 'VCPKG_TARGET_IS_' iModelCore/libsrc/*/*ports/
```

Neither grep reaches a port whose portfile we do not vendor, so for those compare the observed
graphs instead — the ports a real build resolved on a given platform against the ports the scan
materialized:

```bash
ls $OutRoot/vcpkg_installed/<consumer>/<triplet>/share/*/vcpkg_abi_info.txt   # per-platform
ls -d <scan-root>/<consumer>/<triplet>/buildtrees/*/src                       # what Mend saw
```

Host helper ports (`vcpkg-cmake` and friends) appear only in the first list; ignore them. Anything
else present on a supported platform but absent from the scan needs a covering triplet added to that
consumer's `vcpkg-mend.json`.

Pinning a platform-conditional port in `overrides` is worth doing even when its version is already
correct, because the materialization check then demands that the port appear in some configured
graph — turning an invisible coverage gap into a scan failure. That lever does nothing for a
conditional sub-source fetched inside another port's portfile (crashpad's `lss`); only a covering
triplet helps there.

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
   - Re-run the [platform coverage audit](#auditing-platform-coverage): a new upstream version can
     add a platform-conditional download that the consumer's `vcpkg-mend.json` triplets miss.
2. **Audit every vcpkg consumer graph, not only the library's own manifest.** Search all
   `iModelCore/libsrc/**/vcpkg.json` files for the port name and old version. Inspect both
   `dependencies` and `overrides`: consumers deliberately use overrides to pin transitive ports,
   and an explicit override takes precedence over a newer registry baseline. Update every graph
   that should consume the new version. OpenSSL currently appears in its own, curl, and crashpad
   graphs; zlib appears in several graphs. Apply the same check to minizip and every other port.
3. For each affected consumer graph, ensure `vcpkg-configuration.json` uses a registry `baseline`
   that contains the requested version. Updating one consumer's baseline does not affect any other
   manifest directory.
4. Resolve or install every affected graph with a triplet that activates platform-conditional
   dependencies; use each consumer's `vcpkg-mend.json` triplets as the starting point. Search again
   for the old version and do not finish while a relevant manifest still pins it.
5. No changes to `.mke` or `.PartFile.xml` files are needed — the next build will pick
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
| `iModelCore/libsrc/vcpkg_run_install.ps1` / `.sh` | Wrapper that invokes vcpkg; Mend passes the explicit `-MendScan` / `--mend-scan` option rather than ambient environment controls |
| `iModelCore/libsrc/VCPKG.md` | Human-facing documentation; keep in sync when changing patterns |
