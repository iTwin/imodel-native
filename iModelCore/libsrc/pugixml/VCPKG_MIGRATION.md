# Migrating the pugixml build to vcpkg

> **Status: WIRING LANDED (Steps 1–7 done; Steps 8–9 pending).** The vcpkg wiring is
> implemented and checked in. Remaining: **Step 8** (build/verify on every platform — requires
> the Bentley build environment) and **Step 9** (delete the now-vendored upstream source in a
> separate commit after Copilot review). Treat the checked-in files as authoritative wherever
> this plan and the final files disagree.
>
> **Key deviations from the original plan, as implemented:**
> - Target version confirmed **1.15** (newest at the png baseline `81de6771…`).
> - No `.patch` file: the overlay `portfile.cmake` injects the `pugiconfig.hpp` dllexport block
>   with `vcpkg_replace_string` (robust against upstream byte changes).
> - `__PUGIXML_BUILD__` is defined via `-DPUGIXML_BUILD_DEFINES=__PUGIXML_BUILD__` (pugixml
>   1.15's CMake wires that cache var into the **static** target) rather than a triplet flag.
> - `merge_static_libs.sh` was extended to accept a raw `.o` input (one merge input is our
>   `BePugiXml.o`, not a second archive).
> - The vcpkg archive is `pugixml.lib` / `libpugixml.a` with **no** debug postfix (debug differs
>   only by directory).
> - The `PugiXml` PartFile bindings are left unchanged (they already tolerate both linkages);
>   only the `vcpkg_install_pugixml` chain SubParts were added.

This document enumerates the steps to replace the current build-from-source pugixml
(`iModelCore/libsrc/pugixml`, currently **1.12**, compiled file-by-file by
[`pugixml.mke`](pugixml.mke)) with a vcpkg-driven build, and to bump the version to the newest
release packaged at our pinned registry baseline **at the same time** (verify — see Step 1).

## Why pugixml is harder than png (and than openssl)

Read the shared guide in [`../VCPKG.md`](../VCPKG.md) and the agent skill
`.github/skills/vcpkg/SKILL.md` first. pugixml differs from the two libraries already migrated
in one decisive way:

| Library | Vendored source | Bentley wrapper we compile | Final artifact |
|---------|-----------------|----------------------------|----------------|
| **png** | pure upstream | none | static lib, all platforms |
| **openssl** | pure upstream | none (a `.def` re-exports; a patch injects `BSIVer`) | static lib everywhere; DLL only on Windows |
| **pugixml** | upstream **+** our `BePugiXml.cpp` / `BePugiXml.h` / `BePugiXmlHelper.h` | **yes — we always compile it** | shared **and** static, on **every** platform |

The two consequences that drive the whole design:

1. **We keep compiling our own wrapper.** [`src/BePugiXml.cpp`](src/BePugiXml.cpp) (plus the
   header-only [`src/BePugiXmlHelper.h`](src/BePugiXmlHelper.h)) is *Bentley* code that wraps
   `pugi::xml_node` in a `BeXml`-compatible API. vcpkg cannot build it. So `pugixml.mke` cannot
   become a thin re-delivery like [`../png/png.mke`](../png/png.mke); it must still compile
   `BePugiXml.cpp` and **combine** the result with vcpkg's upstream pugixml archive.

2. **Consumers use `pugi::` symbols directly, so the pugi surface must stay exported.** The
   delivered `iTwinPugixml` today re-exports the whole pugixml API. Unlike OpenSSL (which
   controls exports with a hand-maintained `.def`), pugixml exports via **`__declspec(dllexport)`
   annotations** injected by our customized [`src/pugiconfig.hpp`](src/pugiconfig.hpp) (see the
   `__PUGIXML_BUILD__` block at the top). Any vcpkg build must reproduce that annotation so the
   symbols still leave `iTwinPugixml`. **No `.def` is involved and none should be added.**

**Deliverable names must not change.** `iTwinPugixml` (shared + import lib), the VendorAPI
headers under `VendorAPI/pugixml/src/`, and the `pugixml-license.md` notice are the contract
with every consumer; keep them byte-for-byte identical in name and include path.

---

## Current state (what we are replacing)

| Aspect | Today |
|--------|-------|
| Version | 1.12 (see [`src/pugixml.hpp`](src/pugixml.hpp) `PUGIXML_VERSION 1120`) |
| Build | [`pugixml.mke`](pugixml.mke) compiles `src/pugixml.cpp` **and** `src/BePugiXml.cpp` into a single shared lib `iTwinPugixml` via `linkLibrary.mki` (`DLM_*`) |
| Linkage | **Shared or static** — the same `DLM_*` machinery, driven by `CREATE_STATIC_LIBRARIES`, produces a DLL/import-lib pair (dynamic) or a static archive (static). The node addon consumes the **static** `libiTwinPugixml.a` as a thin-archive input (see [`../../../iModelJsNodeAddon/IModelJsNative_input_libs.mki`](../../../iModelJsNodeAddon/IModelJsNative_input_libs.mki)) |
| Exports | pugixml symbols exported via `__declspec(dllexport)` from the customized [`src/pugiconfig.hpp`](src/pugiconfig.hpp) (`__PUGIXML_BUILD__` → dllexport, else dllimport); wrapper symbols exported via `BEPUGIXML_EXPORT` (same `__PUGIXML_BUILD__` gate in [`src/BePugiXml.h`](src/BePugiXml.h)). No `.def`. |
| Delivered headers | `VendorAPI/pugixml/src/` ← `pugixml.hpp`, `pugiconfig.hpp` (upstream, one Bentley `#if` block), `BePugiXml.h`, `BePugiXmlHelper.h` (both Bentley) — staged by [`prewire.mke`](prewire.mke) |
| Delivered lib | `Delivery/$(shlibprefix)iTwinPugixml$(shlibext)` + `Delivery/$(libprefix)iTwinPugixml$(libext)` (import lib) |
| License | `Delivery/pugixml-license.md` ← [`LICENSE.md`](LICENSE.md) |
| Bentley dependency | links `iTwinBentley` (`BentleyDll` SubPart in [`pugixml.PartFile.xml`](pugixml.PartFile.xml); `LINKER_LIBRARIES + …iTwinBentley` in the mke) |
| Language | built `BUILD_WITH_C20=1` |
| Consumers | 4 PartFiles SubPart `PugiXml`: [`../../ecobjects/ECObjects.PartFile.xml`](../../ecobjects/ECObjects.PartFile.xml), [`../../ECDb/ECDb.PartFile.xml`](../../ECDb/ECDb.PartFile.xml), [`../../GeoCoord/GeoCoord.PartFile.xml`](../../GeoCoord/GeoCoord.PartFile.xml), [`../../iModelPlatform/iModelPlatform.PartFile.xml`](../../iModelPlatform/iModelPlatform.PartFile.xml). Source consumers include `<pugixml/src/pugixml.hpp>`, `<pugixml/src/BePugiXml.h>`, `<pugixml/src/BePugiXmlHelper.h>` (ecobjects public API `ECSchema.h` / `ECInstance.h` / `ECUnit.h`, ECDb `SchemaViewWriter.cpp`, GeoCoord `basegeocoord.cpp`). Static archive pulled into `imodeljs.node`. |
| PartFile parts | `Prewire` (headers + license), `PugiXml` (main) |

**The `pugi::` symbols are part of the public contract.** Because `ECSchema.h` / `ECInstance.h`
(shipped ecobjects public API) `#include <pugixml/src/pugixml.hpp>` and use `pugi::xml_node`
etc., the exported pugixml surface cannot shrink. Reproduce it exactly (Step 2 / Step 5).

---

## Guiding decision: build upstream pugixml *static* with vcpkg, keep producing `iTwinPugixml` ourselves

Mirror the OpenSSL decision: vcpkg builds **upstream pugixml as a static archive on every
platform** (never vcpkg's own shared lib), and `pugixml.mke` combines that archive with our
compiled `BePugiXml.cpp` object into the `iTwinPugixml` deliverable — shared on dynamic builds,
static on static builds. This is what lets us keep:

- our wrapper object (`BePugiXml.o`) inside the same library, and
- the exact exported pugixml surface (via the dllexport annotation, reproduced by the overlay
  port in Step 2 — **not** a `.def`).

### Per-linkage output matrix

Every platform uses a vcpkg **static** triplet. The split below is our *final* step only.

| Build | `iTwinPugixml` deliverable | Final step | pugi exports |
|-------|----------------------------|-----------|--------------|
| Dynamic (any platform, e.g. under the node-addon strategy on Windows) | shared lib + import lib | link a shared lib from `BePugiXml.o` + vcpkg `libpugixml.a`, reusing `linkLibrary.mki` (`DLM_*`) | `__declspec(dllexport)` (Windows) / default visibility (others), reproduced by the overlay-port `pugiconfig.hpp` |
| Static (all platforms; node addon thin-archive input) | single static archive | **merge** `BePugiXml.o` + vcpkg `libpugixml.a` into `libiTwinPugixml.a` (libtool / `ar qcL` / `merge_static_libs.sh` / `lib -OUT`) | n/a — symbols present as objects |

> **Merge precedent — `BeZlib`.** The cleanest existing example of combining libraries into one
> archive is [`../compress/Zlib.mke`](../compress/Zlib.mke), which merges the **two** vcpkg
> archives `z` + `minizip` into a single `BeZlib` with the exact per-platform tools we need
> (`libtool -static` on macOS/iOS, `$(BENTLEY_ANDROID_TOOLCHAIN_ar) qcL` on Android,
> `merge_static_libs.sh` on Linux, `lib -OUT` on Windows). pugixml uses the *same* merge idiom,
> with one twist: one of its two inputs is **our own compiled `BePugiXml.o`**, not a second
> vcpkg archive. (OpenSSL's static path merges two vcpkg archives the same way.)

> **Why the merge matters.** `imodeljs.node` links the **static** `libiTwinPugixml.a` as a
> thin-archive input. If vcpkg's pugixml objects are not merged into that archive, every
> `pugi::` symbol used across ecobjects/ECDb/GeoCoord/iModelPlatform goes unresolved at the node
> link. The static path therefore behaves like `BeZlib`'s (and OpenSSL's) static merge, **plus**
> it must also fold in our own `BePugiXml.o`.

---

## Step 1 — ✅ Add the vcpkg manifest

Create, in this folder (mirroring [`../png/`](../png) and [`../openssl/`](../openssl)):

- **`vcpkg.json`** — declare the `pugixml` dependency, pinned:

  ```json
  {
    "name": "imodel-native-pugixml",
    "version": "1.0.0",
    "dependencies": [
      { "name": "pugixml", "version>=": "<chosen-version>" }
    ],
    "overrides": [
      { "name": "pugixml", "version": "<chosen-version>" }
    ]
  }
  ```

- **`vcpkg-configuration.json`** — copy from [`../png/vcpkg-configuration.json`](../png/vcpkg-configuration.json)
  (same registry `baseline` commit) and add an `overlay-ports` entry pointing at the overlay
  port created in Step 2 (same shape as [`../openssl/vcpkg-configuration.json`](../openssl/vcpkg-configuration.json)):

  ```json
  "overlay-ports": [ "./overlay-ports" ]
  ```

> **Version / availability check (do this first).** Inspect `versions/p-/pugixml.json` in the
> vcpkg registry at the `png`/`compress` baseline commit and pick the newest release there as
> `<chosen-version>` (as of this writing that is expected to be **1.15** — verify). Confirm the
> chosen version is packaged at that baseline; if you want a version the baseline lacks, the
> overlay port from Step 2 can pin it by pointing `REF`/`SHA512` at that release (the same trick
> OpenSSL used for 3.4.6). Record the final choice here and bump [`../README.md`](../README.md)
> to match (Step 7).
>
> **API-compatibility check.** pugixml is source-stable across minor versions, but confirm the
> symbols our consumers use (`xml_node`, `xml_document`, `xml_writer`, XPath) are unchanged
> between 1.12 and the target, and that the target still ships `pugixml.hpp` + `pugiconfig.hpp`
> to `include/`.

---

## Step 2 — ✅ Create an overlay port that reproduces our export config

This is the pugixml-specific heart of the migration. vcpkg's stock pugixml port installs the
**upstream** `pugiconfig.hpp`, which has **none** of our `__declspec(dllexport/dllimport)`
customization. If we build against the stock header, the `pugi::` symbols will **not** be
exported from `iTwinPugixml` on Windows and every consumer link breaks.

Create `overlay-ports/pugixml/` (copy the registry's `ports/pugixml` at the chosen version)
and make it the single source of truth for the export macros:

1. **Patch `pugiconfig.hpp`** so the installed header carries our Bentley block verbatim —
   `__declspec(dllexport)` when `__PUGIXML_BUILD__` is defined, `__declspec(dllimport)`
   otherwise (copy the top of the current [`src/pugiconfig.hpp`](src/pugiconfig.hpp)). Add the
   patch to the port's `PATCHES` list and verify it applies cleanly (`git apply --check`).

2. **Define `__PUGIXML_BUILD__` while vcpkg compiles pugixml** so the objects in
   `libpugixml.a` are annotated dllexport. Do this in the overlay port's `portfile.cmake`
   (e.g. add `-D__PUGIXML_BUILD__` to the CMake compile options) rather than in the triplet, so
   it applies only to the pugixml build and not to unrelated ports. (A triplet
   `VCPKG_C_FLAGS`/`VCPKG_CXX_FLAGS` `-D__PUGIXML_BUILD__` is an acceptable fallback if the port
   makes CMake option injection awkward — but the overlay port already exists for the header
   patch, so keep both concerns together there.)

> **Result:** vcpkg installs the *patched* `pugiconfig.hpp` (which we then deliver from the
> vcpkg include dir in Step 5, keeping a single source of truth) and produces a `libpugixml.a`
> whose objects export the pugi API from any DLL they are linked into — exactly today's
> behavior, with no `.def`.
>
> **Non-Windows note:** the current build sets **no** visibility flags, so pugi symbols are
> default-visible in the shared lib on macOS/Linux/iOS/Android. Do **not** add
> `-fvisibility=hidden` in the triplets (that was an OpenSSL-only measure to avoid clashing with
> Node's bundled OpenSSL; pugixml is not bundled by Node). Preserve default visibility.

---

## Step 3 — ✅ Add overlay triplets (static lib, dynamic CRT on Windows)

Create `triplets/` by copying [`../png/triplets/`](../png/triplets) (the simplified set — no
`NOCRYPT`/`NOUNCRYPT`, no `/RTC` flags, no veracode variant). One file per shipped platform:
`arm64-osx.cmake`, `arm64-ios.cmake`, `arm64-android.cmake`, `x64-android.cmake`,
`x64-linux.cmake`, `x64-windows-static.cmake`.

- Each sets `set(VCPKG_LIBRARY_LINKAGE static)` (we produce the shared lib ourselves).
- Windows sets `set(VCPKG_CRT_LINKAGE dynamic)` so pugixml objects embed `/MD` (matching the
  rest of the Bentley Windows build and the `iTwinPugixml.dll` we link). Rely on the base
  `x64-windows-static` triplet name from [`../vcpkg.mki`](../vcpkg.mki); do **not** set
  `vcpkgWindowsMDCRT` (that switches to the built-in `-md` triplet and skips our overlay).
- Because no triplet sets `-RTC`, **no veracode overlay triplet is needed** and
  `vcpkgUseVeracodeTriplet` must stay undefined (Steps 4 & 5).
- Do **not** add visibility flags (see Step 2 note).

---

## Step 4 — ✅ Add `vcpkg_install_pugixml.mke` and extend the chain

**`vcpkg_install_pugixml.mke`** in `iModelCore/libsrc/`, mirroring
[`../vcpkg_install_png.mke`](../vcpkg_install_png.mke) (i.e. **without** `vcpkgUseVeracodeTriplet`):

```makefile
%include mdl.mki

pugixmlDir  = $(_MakeFilePath)pugixml/
installRoot = $(OutputRootDir)vcpkg_installed/pugixml/

%include $(_MakeFilePath)vcpkg.mki

always:
%if defined (winNT)
    $(_MakeFilePath)vcpkg_run_install.bat $(pugixmlDir) $(installRoot) $(vcpkgTriplet)
%else
    $(_MakeFilePath)vcpkg_run_install.sh $(pugixmlDir) $(installRoot) $(vcpkgTriplet)
%endif
```

`vcpkg_run_install.{bat,sh}` auto-passes `--overlay-triplets=$(pugixmlDir)triplets` because that
directory exists, and picks up `overlay-ports` from `vcpkg-configuration.json`.

**Extend the chain** in [`../vcpkg.PartFile.xml`](../vcpkg.PartFile.xml). Insert a new linear
link so no two vcpkg processes run concurrently. A natural spot is **after `png`, before
`openssl`** (pugixml is a foundational codec-like dependency used broadly by ecobjects/ECDb):

```xml
<Part Name="vcpkg_install_pugixml" BentleyBuildMakeFile="vcpkg_install_pugixml.mke">
    <!-- LibType="Static": the chain always runs static-only. -->
    <SubPart PartName="vcpkg_install_png" LibType="Static"/>
</Part>
```

Re-parent the following link (`vcpkg_install_openssl`) onto `vcpkg_install_pugixml`, keeping the
chain linear, and update the sibling `.mke`/PartFile comments and the "How It Works" chain
description in [`../VCPKG.md`](../VCPKG.md) to
`compress → png → pugixml → openssl → crashpad`.

---

## Step 5 — ✅ Rewrite `pugixml.mke` (hybrid: compile wrapper + combine vcpkg archive)

Keep [`pugixml.mke`](pugixml.mke) as a **real** build makefile (not a thin re-delivery). The
skeleton:

1. Preserve the front matter: `BUILD_WITH_C20=1`, `%include mdl.mki`,
   `cDefs +% -D__PUGIXML_BUILD__` (so `BePugiXml.cpp` gets the export macros),
   `appName = iTwinPugixml`, and the `-Wno-unknown-pragmas` flag.

2. Resolve the triplet and install root (do **not** set `vcpkgUseVeracodeTriplet`). Gate the
   install root on `CREATE_STATIC_LIBRARIES` exactly as the skill and png/openssl require —
   the chain always installs static-only:

   ```makefile
   %include $(libsrcDir)vcpkg.mki
   %if defined (CREATE_STATIC_LIBRARIES)
   vcpkgInstallRoot = $(OutputRootDir)vcpkg_installed/pugixml/
   %else
   vcpkgInstallRoot = $(OutputRootDir)static/vcpkg_installed/pugixml/
   %endif
   vcpkgTripletDir = $(vcpkgInstallRoot)$(vcpkgTriplet)/
   vcpkgLibDir     = $(vcpkgTripletDir)lib/
   vcpkgDbgLibDir  = $(vcpkgTripletDir)debug/lib/
   ```

3. Resolve the vcpkg pugixml archive (verify the exact names/`d` debug suffix from the port
   output — expected `pugixml.lib`/`pugixmld.lib` on Windows, `libpugixml.a`/`libpugixmld.a`
   elsewhere):

   ```makefile
   %if $(TARGET_PLATFORM) == "Windows"
       pugixmlLibName = pugixml.lib   # pugixmld.lib in DEBUG
   %else
       pugixmlLibName = libpugixml.a  # libpugixmld.a in DEBUG
   %endif
   ```

4. **Always compile our wrapper.** Keep the `MultiCppCompileRule.mki` / `MultiCppCompileGo.mki`
   block, but reduce it to the **one** Bentley source we still own —
   `$(o)BePugiXml$(oext) : $(srcDir)BePugiXml.cpp …`. **Drop** the `src/pugixml.cpp` compile
   (vcpkg builds it now). `cppObjects` now contains only `BePugiXml.o`.

5. **Produce `iTwinPugixml`, two ways:**

   - **Static build** (`CREATE_STATIC_LIBRARIES`): merge `BePugiXml.o` + the vcpkg pugixml
     archive into `libiTwinPugixml.a`. Copy the per-platform merge block **verbatim** from
     [`../compress/Zlib.mke`](../compress/Zlib.mke) (`libtool -static` on macOS/iOS,
     `$(BENTLEY_ANDROID_TOOLCHAIN_ar) qcL` on Android, `merge_static_libs.sh` on Linux,
     `lib -OUT` on Windows) — that makefile is the canonical two-input merge; just substitute
     our `BePugiXml.o` for one of its two vcpkg inputs — then deliver under the existing name.
     (Because there are two inputs here — our object plus the vcpkg archive —
     `merge_static_libs.sh` is valid on Linux, unlike the single-input png case.)

   - **Dynamic build**: feed **both** `BePugiXml.o` and the vcpkg pugixml archive into the
     existing `linkLibrary.mki` (`DLM_*`) machinery so the shared lib + import lib are produced
     with versioning/signing/BuildContext delivery exactly as before:

     ```makefile
     DLM_NAME         = $(appName)
     DLM_OBJECT_FILES = $(cppObjects) $(vcpkgPugixmlLib)
     DLM_EXPORT_OBJS  = $(DLM_OBJECT_FILES)
     ...
     LINKER_LIBRARIES + $(BuildContext)SubParts/Libs/$(libprefix)iTwinBentley$(stlibext)
     %include $(sharedMki)linkLibrary.mki
     ```

     The dllexport annotations baked into the vcpkg archive (Step 2) re-export the pugi surface
     from the DLL; `BEPUGIXML_EXPORT` re-exports the wrapper symbols. **No `.def`.**

   > Follow the crashpad/compress/openssl ordering lesson: run the merge/link as ordered
   > `always:` blocks (or via the `DLM_*`/dep rules already in `linkLibrary.mki`) so they execute
   > after the chain's vcpkg install has populated the install root.

6. Keep the `iTwinBentley` link dependency (`BentleyDll` SubPart supplies it). Remove any
   `src/pugixml.cpp`-specific object rules and the `PugiXmlHeaders` dep list that pointed at the
   now-vendorless `src/pugixml.hpp` (headers come from vcpkg — Step 5 of prewire below).

---

## Step 6 — ✅ Update `prewire.mke` and the PartFile

**`prewire.mke`** — stage headers + license from the vcpkg install instead of the deleted
`src/` tree, keeping the destination paths identical so `#include <pugixml/src/…>` is unchanged:

- `VendorAPI/pugixml/src/pugixml.hpp` ← `$(vcpkgIncludeDir)pugixml.hpp`
- `VendorAPI/pugixml/src/pugiconfig.hpp` ← `$(vcpkgIncludeDir)pugiconfig.hpp` (the **patched**
  header from Step 2)
- `VendorAPI/pugixml/src/BePugiXml.h` ← keep sourcing from the checked-in Bentley header
- `VendorAPI/pugixml/src/BePugiXmlHelper.h` ← keep sourcing from the checked-in Bentley header
- `Delivery/pugixml-license.md` ← `$(vcpkgTripletDir)share/pugixml/copyright` (verify the vcpkg
  license path; fall back to the checked-in [`LICENSE.md`](LICENSE.md) if absent)

The prewire must resolve the same `vcpkgInstallRoot` (include `vcpkg.mki`; the prewire runs in
the static chain context — mirror [`../openssl/BeOpenSSL.prewire.mke`](../openssl/BeOpenSSL.prewire.mke)).
`BePugiXml.h` / `BePugiXmlHelper.h` stay checked in under `src/` (they are ours), so those two
`~linkfile`s keep pointing at `$(SrcDir)`.

**`pugixml.PartFile.xml`** — wire the chain dependency and keep deliverable bindings identical:

```xml
<Part Name="Prewire" BentleyBuildMakeFile="prewire.mke">
    <SubPart PartName="vcpkg_install_pugixml" PartFile="iModelCore/libsrc/vcpkg" LibType="Static"/>
    <Bindings>
        <VendorAPI Domain="pugixml"/>
        <VendorNotices>Delivery/pugixml-license.md</VendorNotices>
    </Bindings>
</Part>

<Part Name="PugiXml" BentleyBuildMakeFile="pugixml.mke">
    <SubPart PartName="Prewire" />
    <SubPart PartName="vcpkg_install_pugixml" PartFile="iModelCore/libsrc/vcpkg" LibType="Static"/>
    <SubPart PartName="BentleyDll" PartFile="iModelCore/Bentley/Bentley" />
    <Bindings>
        <Assemblies>Delivery/$(shlibprefix)iTwinPugixml$(shlibext)</Assemblies>
        <Libs>Delivery/$(libprefix)iTwinPugixml$(libext)</Libs>
    </Bindings>
</Part>
```

Keep `LibType="Static"` on both chain SubParts (the chain is static-only; dynamic builds read
the static install root and must not trigger a concurrent dynamic chain run). Leave the four
consumer PartFiles (`ECObjects`, `ECDb`, `GeoCoord`, `iModelPlatform`) **unchanged** — they
still SubPart `PugiXml` and see the same bindings.

---

## Step 7 — ✅ Update `README.md` and docs

- [`../README.md`](../README.md) `pugixml` row: bump `1.12` → `<chosen-version>` and change the
  vcpkg column `No` → `Yes`.
- [`../VCPKG.md`](../VCPKG.md): add pugixml to the "How It Works" chain description
  (`compress → png → pugixml → openssl → crashpad`) and the consumer-mke list (already touched in
  Step 4).

---

## Step 8 — ⬜ Verify

1. Build every target platform: Windows (x64, both dynamic and static contexts), Linux (x64),
   macOS (arm64), iOS (arm64), Android (arm64 and x64). Confirm pugixml resolves at the chosen
   version in each install root.
2. Confirm the delivered `iTwinPugixml` shared lib + import lib **and** the static
   `libiTwinPugixml.a` appear with the exact old names, and that
   `VendorAPI/pugixml/src/{pugixml.hpp,pugiconfig.hpp,BePugiXml.h,BePugiXmlHelper.h}` are present.
3. **Export check (Windows).** Dump the exports of `iTwinPugixml.dll` (e.g. `dumpbin /exports`)
   and confirm the `pugi::` surface is present — compare against a pre-migration build. A missing
   pugi export means Step 2's `__PUGIXML_BUILD__` / `pugiconfig.hpp` patch did not take effect.
4. **Node link (static).** Confirm `imodeljs.node` links: `libiTwinPugixml.a` (thin-archive
   input) must now contain both `BePugiXml.o` and the merged vcpkg pugixml objects, so every
   `pugi::` symbol used by ecobjects/ECDb/GeoCoord/iModelPlatform resolves.
5. **Functional test.** Exercise schema serialization/deserialization (ecobjects tests use
   `BePugiXml` heavily — see `SchemaDeserializationTests.cpp`, `InstanceSerializationTests.cpp`)
   on at least macOS and Windows.
6. Confirm no consumer needed a source change (include paths and the `pugi::` / `BePugiXml` APIs
   are unchanged).

---

## Step 9 — ⬜ Cleanup once green

- Delete the checked-in upstream source that vcpkg now owns: `src/pugixml.cpp`, `src/pugixml.hpp`,
  `src/pugiconfig.hpp` (its Bentley block now lives in the overlay-port patch), plus
  `readme.txt` / [`README_BENTLEY.txt`](README_BENTLEY.txt) references that point at the vendored
  source. **Keep** `src/BePugiXml.cpp`, `src/BePugiXml.h`, `src/BePugiXmlHelper.h` — those are
  ours and are still compiled/delivered.
- Keep [`LICENSE.md`](LICENSE.md) only if it is still the license source; otherwise the vcpkg
  `share/pugixml/copyright` supersedes it (decide in Step 6).
- Follow the migration ordering lesson from the skill: land the vcpkg wiring first, get the PR
  through Copilot review, then delete the vendored source in a **separate standalone commit**
  within the same PR (deleting up front produces too many changed files for review).
- Fold any remaining pugixml build notes into this file / [`../VCPKG.md`](../VCPKG.md), and add a
  ✅ *Done* banner to each step above as it lands.
```
