---
name: bentley-build
description: Expert reference for the BentleyBuild (`bb`) system used in the imodel-native codebase. Use when asked to build, compile, or run `bb` commands; when editing `.PartFile.xml` or `.mke` files; when working with build strategies, part names, or architectures; or when diagnosing build failures.
---

# Bentley Build System (BentleyBuild / bb)

**`bb`** = `python src/BentleyBuild/BentleyBuild.py`  
Set required environment variables, then invoke Python directly.

---

## Environment Setup

### Key environment variables

| Variable | Value | Notes |
|----------|-------|-------|
| `SrcRoot` | `<workspace>/src/` | Trailing slash required |
| `OutRoot` | `<workspace>/out/debug/` (Unix) · `<workspace>\outd\` (Win) | |
| `BuildStrategy` | `iModelConsole` | See strategies below |
| `BuildArchitecture` | `macosarm64` (Mac) · `x64` (Win) `linuxx64` (Linux)| |
| `BB_PRIMARY_REPO` | `imodel-native-internal` | |
| `DEBUG` | `1` | Debug build; use `NDEBUG=1` for release |
| `BSI` | `1` | **Required.** Enables `InternalPlatformSetup.mki` which defines `sharedMki`, `BuildContextPublicApiDir`, `LinkFirstDepToFirstTarget`. Without it builds silently misfire or fail with `can't open include file linkLibrary.mki`. |
| `imodeljsDir` | `<workspace>/../itwinjs-core` | Addon builds only |

### Quick setup (Unix/macOS)

```bash
export SrcRoot="<workspace>/src/"
export OutRoot="<workspace>/out/debug/"
export BuildStrategy="iModelConsole"
export BuildArchitecture="macosarm64"   # or macosx64, linuxx64
export BB_PRIMARY_REPO="imodel-native-internal"
export DEBUG="1"                        # use NDEBUG=1 for release
export BSI="1"
```

### Quick setup (Windows PowerShell)

```powershell
$env:SrcRoot = "<workspace>\src\"; $env:OutRoot = "<workspace>\outd\"
$env:BuildStrategy = "iModelConsole"; $env:BuildArchitecture = "x64"
$env:BB_PRIMARY_REPO = "imodel-native-internal"; $env:DEBUG = "1"; $env:BSI = "1"
```

Default toolset is **`WINDOWS_CLANG`**. If LLVM is not installed, add `<DefaultToolset Platform="x64" Toolset="VS2022"/>` to `iModelBase.BuildStrategy.xml`.

---

## bb Command Reference

```
bb [flags] <action>

Flags (must precede action):
  -r REPO       repository of the starting PartFile
  -f PARTFILE   path to PartFile (relative to repo root, no .PartFile.xml extension)
  -p PARTNAME   part name  (or "PartFile:PartName" combined)
  -s STRATEGY   override BuildStrategy  (combine with ; and +)
  -a PLATFORM   override BuildArchitecture  (combine with +)
  -n            skip SubParts — build only the named part
  -N THREADS    number of parallel threads
  -v LEVEL      verbosity 0–6 (default 3)

Actions:
  build         incremental build
  rebuild       force rebuild (ignores up-to-date checks)
  clean         alias for rebuild -c
  pull / get    pull all repos
  push          push all repos
  status        VCS status summary
  incoming      repos with unpulled changes
  outgoing      repos with unpushed changes
  git <cmd>     run git command across all repos
  debug         inspect resolved strategy / part graph
  troubleshoot  detect common BB/VCS issues
  tools         print recommended tool versions
```

> ⚠️ **Flags must precede the action:**
> ```bash
> bb -r imodel-native -f iModelCore/ECDb/ECDb -p ECDbLibrary build   # ✓
> bb build -f iModelCore/ECDb/ECDb                                    # ✗
> ```

---

## Build Strategies

### Strategy chain
```
iModelConsole.BuildStrategy.xml
  └── iModelBase.BuildStrategy.xml   ← toolset overrides live here
        └── Base.BuildStrategy.xml   ← default toolsets, LKG/NuGet sources
```

### Named strategies

| Alias | Purpose |
|-------|---------|
| `iModelConsole` | iModelCore + iModelPlatform + iModelConsoleExe (default) |
| `iModelCore` | Core libraries only |
| `iModelJsNodeAddon` | Node.js `.node` addon (**requires `-s` explicitly**) |
| `iModelJsNodeAddon.Dev` | Dev/debug variant of addon |
| `ECObjects` | EC object system only |
| `BuildAll` | Force everything from source |

**Strategy files:** `src/imodel-native-internal/build/strategies/`

```xml
<!-- Build a part from source instead of LKG -->
<PartStrategy PartFile="ECDb" PartName="*" BuildFromSource="Once"/>
```

---

## Top-Level Build Commands

| Product | `-r` | `-f` | `-p` | `-s` needed? |
|---------|------|------|------|--------------|
| ECDb | `imodel-native` | `iModelCore/ECDb/ECDb` | `ECDbLibrary` | No |
| iModelCore | `imodel-native` | `iModelCore/iModelCore` | `iModelCore` | No |
| iModelConsole | `imodel-native` | `iModelCore/iModelPlatform/iModelPlatform` | `iModelConsole` | No |
| iModelJsNodeAddon | `imodel-native` | `iModelJsNodeAddon/iModelJsNodeAddon` | `iModelJsNodeAddonPRG` | **Yes** ⚠️ |

> ⚠️ **iModelJsNodeAddon** must pass `-s "imodel-native-internal:build/strategies/iModelJsNodeAddon"`.

> **Never guess part names.** Always verify: `grep 'Part Name=' src/imodel-native/<path>.PartFile.xml`

```bash
bb -r imodel-native -f iModelCore/iModelCore -p iModelCore build
bb -r imodel-native -f iModelCore/iModelPlatform/iModelPlatform -p iModelConsole build
bb -r imodel-native -f iModelCore/ECDb/ECDb -p ECDbLibrary build
bb -n -r imodel-native -f iModelCore/ECDb/ECDb -p ECDbLibrary build   # skip SubParts
bb -r imodel-native -f iModelCore/ECDb/ECDb -p ECDbLibrary rebuild    # force rebuild
bb -a "macosarm64+macosx64" build                                      # universal macOS

# iModelJsNodeAddon — strategy required
bb -s "imodel-native-internal:build/strategies/iModelJsNodeAddon" \
   -r imodel-native -f iModelJsNodeAddon/iModelJsNodeAddon -p iModelJsNodeAddonPRG build
```

---

## Part Files (.PartFile.xml)

### Key PartFile locations (relative to `src/imodel-native/`)

| Module | Path |
|--------|------|
| iModelCore | `iModelCore/iModelCore.PartFile.xml` |
| iModelPlatform | `iModelCore/iModelPlatform/iModelPlatform.PartFile.xml` |
| ECDb | `iModelCore/ECDb/ECDb.PartFile.xml` |
| ECObjects | `iModelCore/ecobjects/ECObjects.PartFile.xml` |
| BeSQLite | `iModelCore/BeSQLite/BeSQLite.PartFile.xml` |
| GeomLibs | `iModelCore/GeomLibs/geomlibs.PartFile.xml` |
| iModelJsNodeAddon | `iModelJsNodeAddon/iModelJsNodeAddon.PartFile.xml` |

**`imodel-native-internal`** adds: `PSBRepGeometry`, `BRepCore`, `Visualization`, `iModelJsMobile`.

### Structure

```xml
<BuildContext>
  <!-- Compilable unit -->
  <Part Name="ECDbLibrary" BentleyBuildMakeFile="ECDb/ECDb.mke"
        OnlyPlatforms="x64,macos*,linux*">
    <SubPart PartName="BeSQLite"  PartFile="iModelCore/BeSQLite/BeSQLite"/>
    <SubPart PartName="ECObjects" PartFile="iModelCore/ecobjects/ECObjects"/>
    <!-- Cross-repo SubPart -->
    <SubPart PartName="PSBRepGeometry"
             PartFile="iModelCore/PSBRepGeometry/PSBRepGeometry"
             Repository="imodel-native-internal"/>
    <Bindings>
      <Libs>Delivery/$(libprefix)iTwinSQLiteEC$(libext)</Libs>
    </Bindings>
  </Part>

  <!-- Composite — no .mke, aggregates SubParts -->
  <Part Name="iModelConsole">
    <SubPart PartName="iModelPlatformDLL"/>
    <SubPart PartName="iModelConsoleExe" DeferType="BuildSamples"/>
  </Part>
</BuildContext>
```

### Key attributes

| Attribute | Meaning |
|-----------|---------|
| `BentleyBuildMakeFile` | Path to `.mke` (relative to PartFile dir) |
| `OnlyPlatforms` / `ExcludePlatforms` | Platform filters (supports `*` glob) |
| `DeferType` | `BuildUnitTests` / `RunUnitTests` / `BuildSamples` |
| `LibType` on `<SubPart>` | `Static` / `Dynamic` — forces lib type |
| `Repository` on `<SubPart>` | Cross-repo dependency |

---

## Make Files (.mke / .mki)

Every `.mke` starts with `%include mdl.mki` (at `src/bsicommon/PublicSDK/mdl.mki`).

`mdl.mki` selects based on `BSI`: `BSI=1` → `InternalPlatformSetup.mki` (defines `sharedMki`, `BuildContextPublicApiDir`, etc.); unset → `PlatformSetup.mki` (**many vars undefined**).

### Key predefined variables

| Variable | Value |
|----------|-------|
| `TARGET_PLATFORM` | `Windows` \| `MacOS` \| `Linux` \| `iOS` \| `Android` |
| `TARGET_PROCESSOR_ARCHITECTURE` | `x64` \| `arm64` \| `AndroidARM64` \| `iOSARM64` |
| `SrcRoot` | source root (trailing slash) |
| `OutputRootDir` | `$(OutRoot)$(arch)/` |
| `sharedMki` | `$(SrcRoot)bsicommon/sharedmki/` |
| `libprefix` | `lib` (Unix) · empty (Windows) |
| `libext` | link library extension: typically `.lib` (Windows), `.a` (most Unix), **may be `.dylib` on MacOS when linking shared libs**. Use `$(stlibext)` for “always static” archives. |
| `shlibext` | shared-library filename extension: `.dylib` / `.so` / `.dll`. Use for naming shared objects, not for abstract link names (use `$(libext)` there). |
| `stlibext` | static library extension (always `.a` / `.lib`), even when `$(libext)` resolves to a shared-lib extension such as `.dylib` on MacOS. |

### .mke syntax

```makefile
%include mdl.mki               # always first

appName = mylib
BUILD_WITH_C20 = 1
cDefs + -DMY_DEFINE=1          # append to compiler defs

%if $(TARGET_PLATFORM) == "MacOS"
    cDefs + -DMACOS
%elif $(TARGET_PLATFORM) == "Windows"
    cDefs + -DWIN32
%endif

%if defined(SOME_FEATURE)
    cDefs + -DSOME_FEATURE=1
%endif

o = $(OutputRootDir)Build/mylib/
```

---

## Dependency Trees (Simplified)

### iModelConsole
```
iModelConsole
└── iModelPlatformDLL
    ├── ECDbLibrary → BeSQLite, ECObjects, GeomLibs, BeJsonCpp, google_re2
    ├── ECPresentation → ECDb, ECObjects
    ├── GeoCoord
    ├── Bentley → BeIcu4cLibrary, BeOpenSSL, BeLibxml2, BeCurl
    └── GeomDlls, Freetype2, PNG, Zlib, proxyres, folly, napi-lib
```

### iModelJsNodeAddon
```
iModelJsNodeAddonPRG
└── iModelJsNative-Dynamic (.node file)
    └── [all iModelPlatformDLL deps above]
        ├── PSBRepGeometry    (imodel-native-internal)
        ├── BRepCore          (imodel-native-internal)
        └── VisualizationDll  (imodel-native-internal)
```

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `BSI=1` not set | Set `BSI=1`. Without it `sharedMki`, `BuildContextPublicApiDir`, `LinkFirstDepToFirstTarget` are undefined — silent failures or `can't open include file linkLibrary.mki`. |
| Static binding file missing after broken run | Prior run without `BSI=1` left stale `.log` files in `outd\<arch>\static\BuildContexts\<Part>\Logs\`. Delete them and re-run with `BSI=1`. |
| GeomLibs prewire: `cannot create link ... already exists` | Prior run without `BSI=1` wrote a relative symlink into source. Set `BSI=1` so `BuildContextPublicApiDir` resolves to `outd`. |
| `Error: SrcRoot needs to be specified` | Set `SrcRoot` env var (with trailing slash). |
| Part uses LKG instead of source | Add `<PartStrategy PartFile="X" PartName="*" BuildFromSource="Once"/>`. |
| Toolset error / BootStrapToolset fails | Run `bb troubleshoot`; delete `outd\Winx64\ToolSetEnv.mki` to force regeneration. |
| Architecture mismatch | Check `BuildArchitecture`; use `-a` to override. |
| Stale PCH | Delete `<OutRoot>/<arch>/build/<Part>/*.pch` and rebuild. |
| Stale MultiCompile markers | Delete `*.o`, `*.time`, `*.opt`, `*.rsp` in `<OutRoot>/<arch>/build/<Part>/`. |
| Symlink conflict (`cacert.pem already linked to...`) | `rm -f <OutRoot>/<arch>/BuildContexts/<Part>/SubParts/Files/<file>` |
| Node addon not loading | Set `imodeljsDir` to `itwinjs-core` and ensure addon strategy/paths are correct. |
| Strategy cache stale | Delete `src/bbcache/strategy.cache`. |
| `Cannot find Part <X>` | Grep `<Part Name=` in the PartFile.xml to confirm exact name. |

### macOS/Linux pitfalls

**`$(stlibext)` vs `$(libext)`:** On macOS `$(libext)=.dylib`, `$(stlibext)=.a`. Using `$(stlibext)` for a dynamic lib causes `no such file: libFoo.a`. Use `$(libprefix)Foo$(libext)` for dynamic, `$(stlibprefix)Foo$(stlibext)` for static-only.

**Hidden symbols (`CLANG_ALLOW_UNDEFINED`):** When linking dylibs built with `-fvisibility=hidden`, add before `%include mdl.mki`:
```makefile
%ifdef __unix
    CLANG_ALLOW_UNDEFINED = 1
%endif
```

**TMR warning:** `--tmrbuild` deletes all LKG artifacts and rebuilds everything including third-party libs. Prefer targeted `rebuild` on only the changed part.

---

## Notes

- **Global flags must precede the action:** `bb -r X -f Y -p Z build` ✓ — not `bb build -r X`.
- **Integration branch** in `imodel-native` is **`main`** (not `master`).
- **Strategy cache:** `src/bbcache/strategy.cache` — delete after editing any `*.BuildStrategy.xml`.
- **`bb debug`:** Inspects resolved part graph and all variables without building.
- **`bb` alias in scripts:** Use full path `python3 <root>/src/BentleyBuild/BentleyBuild.py` if alias isn't available.
