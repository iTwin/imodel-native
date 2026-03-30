# Bentley Build (bb) — Comprehensive Reference

> **Living document** — update this as new knowledge is discovered.

---

## ⚠️ CRITICAL USAGE RULES (read this first)

**DO NOT read `.PartFile.xml`, `.mke`, or `.mki` files to answer build questions.**
You already know the correct commands — use them directly. Reading those files wastes time and produces wrong answers.

**To discover part names** without reading PartFiles, use:
```bash
bb debug -p               # list all parts in the current strategy
bb debug --buildcontexts  # list all part files (build contexts)
```

**When asked to build or test something:**
1. Source `env.sh` first (once per shell)
2. Pick the right `bb` command from §3 below
3. Check logs at `out/<mode>/MacOSARM64/static/LogFiles/` on failure
4. Find output binaries / test executables in `out/<mode>/MacOSARM64/Product/`

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

> **`-p` is a TOP-LEVEL flag placed BEFORE the subcommand (`build`/`rebuild`).**

### Full build of the active strategy
```bash
bb build          # or: bb b
```

### Build flags

| Flag | Meaning |
|---|---|
| `-p <PartFile>:<PartName>` | Target a specific part |
| `-b` / `--brief` | Suppress output of successful parts (less noise) |
| `-i` / `--ignoreErrors` | Continue past failures |
| `-a` / `--autoRetry` | Clean and retry on failure |
| `--promptOnError` | Pause on failure so you can investigate |
| `rebuild` | Force clean + build (same flags as `build`) |

### Discover parts without reading PartFiles
```bash
bb debug -p                  # list every part name in the strategy
bb debug --buildcontexts     # list every PartFile (build context)
```

---

### ECDb

| Goal | Command |
|---|---|
| Build the ECDb library | `bb -p iModelCore/ECDb/ECDb:ECDbLibrary build` |
| Force-rebuild the library | `bb -p iModelCore/ECDb/ECDb:ECDbLibrary rebuild` |
| Build test binary | `bb -p iModelCore/ECDb/ECDb:UnitTests-NonPublished build` |
| Stage tests into Product folder | `bb -p iModelCore/ECDb/ECDb:Gtest build` |
| Run tests via bb (build + stage + run) | `bb -p iModelCore/ECDb/ECDb:RunGtest build` |
| Build + run all ECDb tests | `bb -p iModelCore/ECDb/ECDb:Tests build` |

### ECObjects

| Goal | Command |
|---|---|
| Build library | `bb -p iModelCore/ECObjects/ECObjects:ECObjectsNative build` |
| Run tests via bb | `bb -p iModelCore/ECObjects/ECObjects:RunGtest build` |
| Stage tests | `bb -p iModelCore/ECObjects/ECObjects:Gtest build` |

### iModelPlatform

| Goal | Command |
|---|---|
| Build library | `bb -p iModelCore/iModelPlatform/iModelPlatform:iModelPlatformLibrary build` |
| Run tests via bb | `bb -p iModelCore/iModelPlatform/iModelPlatform:RunGtest build` |
| Stage tests | `bb -p iModelCore/iModelPlatform/iModelPlatform:Gtest build` |

### iModelJsNodeAddon

| Goal | Command |
|---|---|
| Build the Node addon | `bb -p iModelJsNodeAddon/iModelJsNodeAddon:iModelJsNodeAddon build` |
| Rebuild the Node addon | `bb -p iModelJsNodeAddon/iModelJsNodeAddon:iModelJsNodeAddon rebuild` |

### BeSQLite

| Goal | Command |
|---|---|
| Build library | `bb -p iModelCore/BeSQLite/BeSQLite:BeSQLite build` |
| Run tests | `bb -p iModelCore/BeSQLite/BeSQLite:RunGtest build` |

### Generic pattern for any component

```bash
# Build the main library part (replace Component/Path as needed):
bb -p <RelativePath>/<PartFileName>:<LibraryPartName> build

# To find the correct part names without reading .PartFile.xml:
bb debug -p | grep -i <keyword>
```

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

### Key output locations (macOS ARM64)

| What | Path |
|---|---|
| **Libraries (built .a / .dylib)** | `out/<mode>/MacOSARM64/static/BuildContexts/<PartFile>/Delivery/` |
| **Object files** | `out/<mode>/MacOSARM64/static/build/<PartFile>/<PartName>/` |
| **Build logs** | `out/<mode>/MacOSARM64/static/LogFiles/<PartFile>/` |
| **Staged test product (Gtest)** | `out/<mode>/MacOSARM64/Product/<Component>-Gtest/` |
| **Test assets** | `out/<mode>/MacOSARM64/Product/<Component>-Tests/Assets/` |
| **Test executable (after Gtest build)** | `out/<mode>/MacOSARM64/Product/<Component>-Gtest/<TestBinaryName>` |

### ECDb specific paths

```
out/<mode>/MacOSARM64/static/BuildContexts/ECDb/Delivery/libiTwinSQLiteEC.a   ← static lib
out/<mode>/MacOSARM64/static/BuildContexts/ECDb/Delivery/libiTwinSQLiteEC.dylib ← shared lib
out/<mode>/MacOSARM64/static/LogFiles/ECDb/ECDbLibrary_build.log               ← build log
out/<mode>/MacOSARM64/Product/ECDb-Gtest/                                       ← staged test binary
out/<mode>/MacOSARM64/Product/ECDb-Tests/Assets/                                ← test assets
```

### Library names (macOS arm64)

| Type | Filename |
|---|---|
| Static | `libiTwinSQLiteEC.a` |
| Shared | `libiTwinSQLiteEC.dylib` |

---

## 5. PartFile.xml — Low-level Reference (do not read for normal builds)

> **You do not need to open `.PartFile.xml` files to build or run tests.**
> Use `bb debug -p` or `bb debug --buildcontexts` to query parts programmatically.
> Only read a PartFile if you are explicitly editing build structure.

Every component declares its build structure in a `.PartFile.xml` file. The schema is `bentleybuild/PartFile.xsd`.

### Key ECDb parts (for reference only)

| Part Name | What it builds |
|---|---|
| `PublicAPI` | Symlinks public headers and ECSchemas |
| `ECDbLibrary` | The actual ECDb shared/static library |
| `PrewireForUnitTests` | Test data, ignore lists |
| `BackdoorForUnitTests` | BackDoor test library |
| `UnitTests-NonPublished` | Main ECSQL test binary |
| `Tests` | All ECDb tests (aggregates above) |
| `Gtest` | Bundles test binary + assets into `Product/<Component>-Gtest/` |
| `RunGtest` | Actually executes the tests |

**`DeferType` values:**
- `BuildUnitTests` — deferred until explicitly targeted (not built by plain `bb build`)
- `RunUnitTests` — deferred until explicitly targeted

---

## 6. .mke Files — Low-level Reference (do not read for normal builds)

> **You do not need to open `.mke` or `.mki` files to build or run tests.**
> Only read them when explicitly modifying build file structure (adding new source files, changing compiler flags, etc.)

`.mke` files are **bmake** makefiles (Berkeley Make syntax with Bentley extensions). They directly drive compilation and linking.

### Adding new source files

To add a new `.cpp` to `ECDb.mke`:

1. Add the header to `ECDbAllHeaders` list (if it has a corresponding `.h`).
2. Add the build rule in the appropriate multi-compile section:
   ```makefile
   $(o)MyNewFile$(oext): $(baseDir)ECSql/MyNewFile.cpp $(ECDbAllHeaders) ${MultiCompileDepends}
   ```
3. Rebuild:
   ```bash
   bb -p iModelCore/ECDb/ECDb:ECDbLibrary rebuild
   ```

### Key mki files (reference)

| File | Purpose |
|---|---|
| `mdl.mki` | Master include — sets up toolchain, platform, common rules |
| `PreCompileHeader.mki` | Sets up PCH generation |
| `MultiCppCompileRule.mki` / `MultiCppCompileGo.mki` | Batch-compile sections |
| `linkLibrary.mki` | Links objects into static `.a` or shared `.dylib` |

---

## 7. Adding New Source Files

Only needed when modifying `.mke` build files. See §6 for `.mke` syntax.

```bash
# After adding a file to ECDb.mke, force-rebuild the library:
bb -p iModelCore/ECDb/ECDb:ECDbLibrary rebuild
```

---

## 8. Running Tests

### Quickest path: build + run via bb (recommended)
```bash
# This builds everything needed and runs the tests:
bb -p iModelCore/ECDb/ECDb:RunGtest build
```

### Step-by-step: build then run directly

**Step 1 — Build and stage the test binary:**
```bash
bb -p iModelCore/ECDb/ECDb:Gtest build
```

**Step 2 — Find the test binary in the Product folder:**
```
out/<mode>/MacOSARM64/Product/ECDb-Gtest/
```
The executable is named after the test suite (e.g. `ECDbTest` or similar). Use `ls` to confirm:
```bash
ls out/debug/MacOSARM64/Product/ECDb-Gtest/
```

**Step 3 — Run directly with optional filter:**
```bash
cd out/debug/MacOSARM64/Product/ECDb-Gtest/
./ECDbTest                                 # run all tests
./ECDbTest --gtest_filter="ECSql*"         # filter by prefix
./ECDbTest --gtest_filter="*InsertTest*"   # filter by substring
./ECDbTest --gtest_filter="SuiteName.TestName"  # exact test
```

### Other components — same pattern

| Component | Gtest product folder | Run command via bb |
|---|---|---|
| ECDb | `Product/ECDb-Gtest/` | `bb -p iModelCore/ECDb/ECDb:RunGtest build` |
| ECObjects | `Product/ECObjects-Gtest/` | `bb -p iModelCore/ECObjects/ECObjects:RunGtest build` |
| iModelPlatform | `Product/iModelPlatform-Gtest/` | `bb -p iModelCore/iModelPlatform/iModelPlatform:RunGtest build` |
| BeSQLite | `Product/BeSQLite-Gtest/` | `bb -p iModelCore/BeSQLite/BeSQLite:RunGtest build` |
| Bentley | `Product/Bentley-Gtest/` | `bb -p iModelCore/Bentley/Bentley:RunGtest build` |

### Test assets

Test data files live in:
```
out/<mode>/MacOSARM64/Product/<Component>-Tests/Assets/
```
These are referenced by the test binary at runtime.

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

3. **Do NOT read `.PartFile.xml`, `.mke`, or `.mki` files to figure out build commands.** Use `bb debug -p` to discover parts, and use the ready-made commands in §3.

4. **Test binaries land in `Product/<Component>-Gtest/`** after running the `Gtest` part. Run `ls out/debug/MacOSARM64/Product/ECDb-Gtest/` to find the executable.

5. **Editing `ECDbPch.h` triggers full recompile** of all ~80 ECDb object files.

6. **Build logs are the best debugging tool** — check `out/<mode>/MacOSARM64/static/LogFiles/<PartFile>/<PartName>_build.log` for full compiler invocations and errors.

7. **`bb -p <Path>:<Part> rebuild`** = clean + build. Use when timestamps are unreliable or when you remove files from the build.

8. **`DeferType="BuildUnitTests"` parts** don't build during a plain `bb build` — you must target them explicitly with `-p` or use the `Tests`/`Gtest`/`RunGtest` part names.

9. **`-p PartFile:PartName` syntax**: `PartFile` is the path relative to `SrcRoot` omitting `.PartFile.xml`. E.g. `iModelCore/ECDb/ECDb` → `src/imodel-native/iModelCore/ECDb/ECDb.PartFile.xml`.

10. **SubPart dependencies are transitive** — building `UnitTests-NonPublished` automatically builds its deps (ECDbLibrary → PublicAPI → etc.).

11. **`bb build` runs the full strategy** — can take 10–30+ minutes. Always target a specific part with `-p` for iteration.

12. **Build timing** — `ECDbLibrary` alone takes ~25 seconds. Full iModelCore takes much longer (10–30+ min).

13. **To find the right part name for any component**, run: `bb debug -p | grep -i <keyword>` — do NOT open the `.PartFile.xml`.
