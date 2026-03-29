# Bentley Build (bb) — Comprehensive Reference

> **Living document** — update this as new knowledge is discovered.

---

## 1. What Is BentleyBuild?

BentleyBuild (bb3) is Bentley's internal Python-based build orchestration system. It sits on top of **bmake** (Berkeley Make) and provides:

- **Dependency graph resolution** across hundreds of parts using `.PartFile.xml` manifests
- **Multi-repository source management** (Git-based, described in `bbconfig.json`)
- **Incremental and full-rebuild control** per-part or for the full tree
- **Test execution orchestration** via GTest, JUnit, XCTest, UWP frameworks
- **NuGet/UPack product packaging**

The entry point is:
```
BentleyBuild.py  (in /Users/affan.khan/bsw/git-master/src/BentleyBuild/)
```

---

## 2. Environment Setup

**Always source `env.sh` in the repo root before any build command:**

```bash
cd /Users/affan.khan/bsw/git-a1
source env.sh
```

`env.sh` sets these critical variables:

| Variable | Value | Purpose |
|---|---|---|
| `SrcRoot` | `<repoRoot>/src/` | Root of all source repositories |
| `BuildStrategy` | `iModelCore` | Which build strategy XML to use |
| `DEBUG` | `1` | Enables debug build (omit for release) |
| `LLVM_DEBUG` | `1` | Debug symbols in LLVM/clang |
| `OutRoot` | `<repoRoot>/out/debug/` | Root of all build outputs |
| `BuildArchitecture` | `macosarm64` | Target architecture |
| `BSI` | `1` | Bentley internal mode (enables `InternalPlatformSetup.mki`) |
| `ShellPid` | `$$` | Per-shell temp dir isolation |
| `SharedShellTmp` | `/tmp/$USER/Bentley/Build/tmp$$/ ` | Per-shell scratch space |

It also defines the `bb` alias:
```bash
alias bb='/Library/Developer/CommandLineTools/usr/bin/python3 \
          /Users/affan.khan/bsw/git-master/src/BentleyBuild/BentleyBuild.py'
```

### ⚠️ Python Version Warning

- `bb` **must use** `/Library/Developer/CommandLineTools/usr/bin/python3` (Python 3.9.6) because that's the one with `lxml` installed.
- `/opt/homebrew/bin/python3` (3.13/3.14) does **NOT** have `lxml` and will print `Error: python package lxml not available.`
- BentleyBuild warns that it wants Python 3.12+ but 3.9.6 still works.
- If you accidentally invoke `bb` without sourcing `env.sh`, the build may use a wrong Python or wrong `OutRoot`.

---

## 3. Common Build Commands

All commands below assume `env.sh` has been sourced (so `bb` alias is active).

### Full build of the active strategy
```bash
bb b
# equivalent to:
bb build
```

### Build a specific part
```bash
bb -p <PartFile>:<PartName> build
# Examples:
bb -p "iModelCore/ECDb/ECDb:ECDbLibrary" build   # builds library only
bb -p "iModelCore/ECDb/ECDb:ECDbLibrary" build -i # build ECDb, ignore dep failures
bb -p "iModelCore/ECDb/ECDb:UnitTests-NonPublished" build  # test binary
```

> **Note:** `-p` is a TOP-LEVEL flag, placed BEFORE the `build` subcommand.
> The part path must include the relative directory: `iModelCore/ECDb/ECDb:PartName`.
> When running with `--ignoreErrors` (`-i`), dep library link errors can be skipped to see compile output.


### Rebuild (force clean + build) a specific part
```bash
bb rebuild -p ECDb:ECDbLibrary
```

### Build flags

| Flag | Meaning |
|---|---|
| `-p <PartFile>:<PartName>` | Target a specific part |
| `-b` / `--brief` | Suppress output of successful parts (less noise) |
| `-i` / `--ignoreErrors` | Continue past failures |
| `-a` / `--autoRetry` | Clean and retry on failure |
| `--promptOnError` | Pause on failure so you can investigate |

---

## 4. Architecture and Output Directories

The build outputs into:
```
<OutRoot>/<Architecture>/
```

With `DEBUG=1` and `BuildArchitecture=macosarm64`:
```
/Users/affan.khan/bsw/git-a1/out/debug/MacOSARM64/
```

Without `DEBUG=1` (release):
```
/Users/affan.khan/bsw/git-a1/out/release/MacOSARM64/
```

### Confirmed output paths (macOS ARM64, release build)

```
out/release/MacOSARM64/static/build/ECDb/ECDbLibrary/          ← object files, PCH, link list
out/release/MacOSARM64/static/BuildContexts/ECDb/Delivery/libiTwinSQLiteEC.a  ← final static lib (symlink)
out/release/MacOSARM64/static/LogFiles/ECDb/ECDbLibrary_build.log              ← build log
```

| Path | Contents |
|---|---|
| `out/<mode>/MacOSARM64/static/build/ECDb/ECDbLibrary/` | Object files, PCH, link file list |
| `out/<mode>/MacOSARM64/static/BuildContexts/ECDb/Delivery/` | Final library (`.a` or `.dylib`), symlinked |
| `out/<mode>/MacOSARM64/static/LogFiles/ECDb/` | Build logs per part |
| `out/<mode>/MacOSARM64/static/build/ECDb/UnitTests/` | Test object files |
| `out/<mode>/MacOSARM64/static/BuildContexts/ECDb/Delivery/UnitTests/Objects/` | Test binaries |

### Library names (macOS arm64)

| Type | Filename |
|---|---|
| Static | `libiTwinSQLiteEC.a` |
| Shared | `libiTwinSQLiteEC.dylib` |

The library is linked using `xcrun libtool` (for static) or the LLVM linker (for shared).

---

## 5. PartFile.xml — The Part Manifest

Every component declares its build structure in a `.PartFile.xml` file. The schema is `bentleybuild/PartFile.xsd`.

### Key ECDb parts (from `ECDb.PartFile.xml`)

| Part Name | What it builds | MakeFile |
|---|---|---|
| `PublicAPI` | Symlinks public headers and ECSchemas | `ECDb.prewire.mke` |
| `ECDbLibrary` | The actual ECDb shared/static library | `ECDb/ECDb.mke` |
| `PrewireForUnitTests` | Test data, ignore lists | `Tests/Prewire.mke` |
| `BackdoorForUnitTests` | BackDoor test library | `Tests/BuildTests.mke -dTestDir=BackDoor` |
| `UnitTests-NonPublished` | Main ECSQL test binary | `Tests/BuildTests.mke -dTestDir=NonPublished` |
| `Tests` | All ECDb tests | (aggregates above) |
| `Gtest` | Bundles test binary + assets for GTest runner | `buildGtest.mke` |
| `RunGtest` | Actually executes the tests | `RunGtest.mke` |

### PartFile structure concepts

```xml
<BuildContext ...>
  <!-- A Part = one logical build unit -->
  <Part Name="ECDbLibrary" BentleyBuildMakeFile="ECDb/ECDb.mke">
    <SubPart PartName="PublicAPI" />                          <!-- dependency -->
    <SubPart PartName="BeSQLite" PartFile="..." />           <!-- external dep -->
    <Bindings>
      <Libs>Delivery/$(libprefix)iTwinSQLiteEC$(libext)</Libs>
      <Assemblies>Delivery/...</Assemblies>
    </Bindings>
  </Part>

  <!-- DeferType controls when a part runs -->
  <Part Name="UnitTests-NonPublished" DeferType="BuildUnitTests" ...>
  <Part Name="RunGtest"               DeferType="RunUnitTests" ...>
</BuildContext>
```

**`DeferType` values:**
- `BuildUnitTests` — deferred until `bb buildunittests` or after main build
- `RunUnitTests` — deferred until `bb rununittests`

**`BentleyBuildMakeOptions`** passes `-d` defines to bmake:
```xml
BentleyBuildMakeFile="Tests/BuildTests.mke" BentleyBuildMakeOptions="-dTestDir=NonPublished"
```

---

## 6. .mke Files — The Actual Makefiles

`.mke` files are **bmake** makefiles (Berkeley Make syntax with Bentley extensions). They directly drive compilation and linking.

### bmake / mke syntax highlights

| Syntax | Meaning |
|---|---|
| `%include file.mki` | Include another mki/mke file |
| `%ifdef VAR` / `%ifndef VAR` | Conditional on variable |
| `%if expr` | Conditional expression |
| `%error msg` | Fatal error |
| `%warn msg` | Warning |
| `$(VAR)` | Variable expansion |
| `$(_MakeFilePath)` | Directory of the current mke file |
| `$(_MakeFileSpec)` | Full path of the current mke file |
| `$(o)` | Object output directory (set in each mke) |
| `$(oext)` | Object file extension (`.o` on Mac/Linux, `.obj` on Windows) |
| `$(appName)` | Library/executable name |
| `$(PartBuildDir)` | Build output dir for this part |

### Standard mke structure (ECDb.mke pattern)

```makefile
%include mdl.mki                        # Core build definitions

baseDir = $(_MakeFilePath)              # Directory of this mke
o       = $(PartBuildDir)              # Where objects go

%include $(CompileOptionsMki)           # Project-specific compile flags

ECDbAllHeaders = \                      # All headers (used as deps)
    $(publicApiDir)ECDb.h \
    ...

# Create output dir
always:
    !~@mkdir $(o)

# Precompiled header
PchCompiland = $(baseDir)ECDbPch.cpp
%include $(SharedMki)PreCompileHeader.mki

# Multi-compile batch 1 (uniform options)
%include MultiCppCompileRule.mki
$(o)Foo$(oext): $(baseDir)Foo.cpp $(ECDbAllHeaders) ${MultiCompileDepends}
$(o)Bar$(oext): $(baseDir)Bar.cpp $(ECDbAllHeaders) ${MultiCompileDepends}
%include MultiCppCompileGo.mki
cppObjects =% $(MultiCompileObjectList)

# Optionally: second batch with different compiler flags (e.g., third-party code)
LLVMCommonCompOpts + -Wno-error=deprecated-writable-strings
%include MultiCppCompileRule.mki
$(o)SqlBison$(oext): $(baseDir)ECSql/Parser/SqlBison.cpp ...
%include MultiCppCompileGo.mki
nonportObjs =% $(MultiCompileObjectList)
# Restore flags
LLVMCommonCompOpts = $(OldLLVMCommonCompOpts)

cppObjects + $(nonportObjs)

# Link
DLM_NAME         = $(appName)
DLM_OBJECT_FILES = $(cppObjects)
LINKER_LIBRARIES = $(ContextSubpartsLibs)$(BeSQLiteLib) ...
%include $(sharedMki)linkLibrary.mki
```

### Key mki files

| File | Purpose |
|---|---|
| `mdl.mki` | Master include — sets up toolchain, platform, common rules |
| `common.mki` | Commands for copy/mkdir/etc., file extensions |
| `InternalPlatformSetup.mki` | BSI internal flags (PRG builds, analyze, etc.) |
| `PreCompileHeader.mki` | Sets up PCH generation and `UsePrecompiledHeaderOptions` |
| `MultiCppCompileRule.mki` | Begins a batch-compile section |
| `MultiCppCompileGo.mki` | Ends batch-compile; sets `MultiCompileObjectList` |
| `linkLibrary.mki` | Links objects into static `.a` or shared `.dylib` |
| `DetectAndCompileUnitTests.mki` | Finds and compiles all `*Tests.cpp` files in a directory |
| `RunGtest.mke` | Copies test binary to run dir and executes it |

---

## 7. Adding New Source Files

### To add a new `.cpp` to `ECDb.mke`:

1. Add the header to `ECDbAllHeaders` list (if it has a corresponding `.h`).
2. Add the build rule in the appropriate multi-compile section:
   ```makefile
   $(o)MyNewFile$(oext): $(baseDir)ECSql/MyNewFile.cpp $(ECDbAllHeaders) ${MultiCompileDepends}
   ```
3. Rebuild:
   ```bash
   bb rebuild -p ECDb:ECDbLibrary
   ```

### To add a file that needs relaxed warnings (third-party code):

Place it in the second `MultiCppCompileRule.mki` / `MultiCppCompileGo.mki` bracket where warning suppressions are applied:
```makefile
# Save + suppress
OldLLVMCommonCompOpts =% $(LLVMCommonCompOpts)
LLVMCommonCompOpts + -Wno-error=some-warning
%include MultiCppCompileRule.mki
$(o)ThirdPartyFile$(oext): ...
%include MultiCppCompileGo.mki
# Restore
LLVMCommonCompOpts = $(OldLLVMCommonCompOpts)
```

---

## 8. Running Tests

### Build the test binary
```bash
bb build -p ECDb:UnitTests-NonPublished
```

### Run via bb (full GTest pipeline)
```bash
bb build -p ECDb:RunGtest
```

### Run directly (after building)

The test binary is assembled into a `Gtest` product collection directory. Find it:
```
out/<mode>/MacOSARM64/static/BuildContexts/ECDb/Delivery/UnitTests/Objects/NonPublished/
```
Or look in build logs for the exact binary path. The `RunGtest.mke` copies it to a run dir:
```
out/<mode>/MacOSARM64/static/build/ECDb/RunGtest/ECDb-Gtest/ECDbTest/run/
```

Run with a filter:
```bash
./ECDbTest --gtest_filter="ECSql*"
./ECDbTest --gtest_filter="*InsertTest*"
```

### Test binary dependencies

The test binary links against:
- `ECDb-Tests` product (collects: `BackdoorForUnitTests`, `UnitTests-NonPublished`, `BeGTest/Base`, `napi-stub`)
- All ECDb library dependencies

---

## 9. Build Logs

Each part writes a build log:
```
out/<mode>/MacOSARM64/static/LogFiles/<PartFile>/<PartName>_build.log
```

Examples:
- `ECDb/ECDbLibrary_build.log` — compiler invocations, warnings, link step
- `ECDb/PublicAPI_build.log` — symlink operations

The log contains literal compiler command lines (clang invocations), link commands, and timing.

**Last line of a successful build log:**
```
Tue Feb 24 13:57:40 2026, elapsed time: 0:25
```

## 17. Provenance Logs

Each part also creates a `_provenance.log` (XML) recording the build version info and repository commit SHAs used:

```xml
<Provenance>
  <Product Name="iModelJsNodeAddon:iModelJsNodeAddonPRG" ReleaseVersion="99" .../>
  <Repository Name="imodel-native" Identifier="<git-sha>" Type="git" />
</Provenance>
```

Located alongside build logs:
```
out/<mode>/MacOSARM64/static/LogFiles/<PartFile>/<PartName>_provenance.log
```


Each library has one PCH:

```
ECDb/ECDb/ECDbPch.h   — the PCH header (included by all .cpp via precompilation)
ECDb/ECDb/ECDbPch.cpp — the PCH compiland (compiled once)
```

The PCH includes all internal and third-party headers. **When adding or removing includes from `ECDbPch.h`, a full rebuild of the library is required** (`bb rebuild -p ECDb:ECDbLibrary`).

The PCH is set up via:
```makefile
PchCompiland = $(baseDir)ECDbPch.cpp
PchOutputDir = $(o)
%include $(SharedMki)PreCompileHeader.mki
CCPchOpts = $(UsePrecompiledHeaderOptions)
CPchOpts  = $(UsePrecompiledHeaderOptions)
```

---

## 11. BuildContext Variable

Inside `.mke` files, `$(BuildContext)` refers to the per-part build staging area:
```
out/<mode>/MacOSARM64/static/BuildContexts/<PartFile>/
```

For ECDb:
```
out/<mode>/MacOSARM64/static/BuildContexts/ECDb/
├── Delivery/          ← final library, headers symlinked here
├── PublicAPI/ECDb/    ← public headers
├── VendorAPI/         ← vendor headers
├── ECSchemas/ECDb/    ← ECSchema XML files
└── Dox/               ← documentation inputs
```

---

## 12. Incremental Builds

BentleyBuild uses file timestamps for incrementality. A re-compile is triggered when:
- A `.cpp` file is newer than its `.o`
- Any header in `$(ECDbAllHeaders)` is newer than any `.o` (because all objects depend on all headers via `$(ECDbAllHeaders)` in dependency blocks)
- `${MultiCompileDepends}` (= the `.mke` file) changes

**This means editing any header in `ECDbAllHeaders` will recompile all files in the library** — expected and intentional.

To force a full rebuild:
```bash
bb rebuild -p ECDb:ECDbLibrary
```

---

## 13. Build Strategy

The `BuildStrategy` env var points to a `.BuildStrategy.xml` file in:
```
src/imodel-native-internal/build/strategies/
```

The active strategy for this workspace is `iModelCore`, which imports `iModelNative`:
```xml
<BuildStrategy>
    <ImportStrategy Name="iModelNative"/>
</BuildStrategy>
```

The strategy controls which top-level parts and repositories are included in `bb b` (full build).

---

## 14. Toolchain

On macOS ARM64:

| Tool | Path | Notes |
|---|---|---|
| C/C++ compiler | `xcrun clang` / `xcrun clang++` | Apple Clang via Xcode Command Line Tools |
| Archiver (static lib) | `xcrun libtool -static` | Creates `.a` |
| Linker (shared lib) | `xcrun clang++` with `-dynamiclib` | Creates `.dylib` |
| SDK | `MacOSX26.2.sdk` (Xcode) | `-arch arm64` target |
| BUILD_TOOLSET | `APPLE_CLANG` | Controls warning/flag macros in mki files |

Compiler flags include `-std=c++20` (`BUILD_WITH_C20=1` in ECDb.mke).

---

## 15. Key Variables Summary

| Variable | Where set | Meaning |
|---|---|---|
| `SrcRoot` | `env.sh` | Root of all source repos (`src/`) |
| `OutRoot` | `env.sh` | Root of build output (`out/debug/`) |
| `BuildArchitecture` | `env.sh` | `macosarm64` |
| `DEBUG` | `env.sh` | `1` = debug build |
| `BSI` | `env.sh` | `1` = internal Bentley build |
| `BuildStrategy` | `env.sh` | Name of `.BuildStrategy.xml` to use |
| `baseDir` | each `.mke` | Directory of the mke file |
| `o` | each `.mke` | Object output directory |
| `oext` | `common.mki` | `.o` (Mac) / `.obj` (Win) |
| `PartBuildDir` | BB runtime | BB-provided build dir for this part |
| `BuildContext` | BB runtime | BB-provided context staging dir |
| `SharedMki` | BB runtime | Path to shared mki files |
| `publicApiDir` | typically set in mke | Path to PublicAPI headers |
| `ContextSubpartsLibs` | BB runtime | Path to dependent parts' libs |

---

## 16. Tips and Gotchas

1. **Always source `env.sh` first** — without it, `OutRoot`, `BuildArchitecture`, etc. are unset and the build will go to the wrong place or fail silently.

2. **Use CommandLineTools python3 for `bb`**, not homebrew python — only the former has `lxml`.

3. **Editing `ECDbPch.h` triggers full recompile** of all ~80 ECDb object files.

4. **The `ECDbAllHeaders` list in ECDb.mke is the universal dependency** — every `.cpp` in the library depends on it. When adding a new header, add it to this list.

5. **Third-party code (Parser/ directory) uses a separate multi-compile section** with warning suppressions. New first-party code should go in the first section.

6. **Build logs are the best debugging tool** — check `LogFiles/<Part>/<Part>_build.log` for full compiler invocations and errors.

7. **`bb rebuild`** = clean + build. Use when timestamps are unreliable or when you remove files from the build.

8. **The `DeferType` attribute** controls when parts build in the pipeline: `BuildUnitTests` parts don't build during a plain `bb build` unless explicitly targeted.

9. **`-p PartFile:PartName` syntax**: `PartFile` is relative to `SrcRoot` and omits `.PartFile.xml`; e.g., `ECDb` → `/Users/affan.khan/bsw/git-a1/src/imodel-native/iModelCore/ECDb/ECDb.PartFile.xml`.

10. **SubPart dependencies are transitive** — building `UnitTests-NonPublished` automatically builds `BackdoorForUnitTests` → `PrewireForUnitTests` → `ECDb` → `ECDbLibrary` → `PublicAPI`.

11. **`bb b` runs the full strategy** — this can take 10–30+ minutes. For iteration, always target a specific part with `-p`.

12. **Build timing** — `ECDbLibrary` alone takes **~25 seconds** (confirmed from build log: `elapsed time: 0:25`). Full iModelCore build takes much longer (10–30+ min).

13. **Symlinks in Delivery** — many files in the `Delivery/` output are symlinks back to source or build intermediates. Don't copy them directly; use the actual build process to update them.
