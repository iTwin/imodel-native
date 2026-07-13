# vcpkg Integration

This directory uses [vcpkg](https://github.com/microsoft/vcpkg) to manage select third-party library builds. vcpkg fetches source and builds libraries locally; the resulting headers and static libraries are delivered to Bentley Build via `.mke` files. The Bentley Build process installs the required version of vcpkg automatically, but the prerequisites listed below must be installed separately.

A specific version of vcpkg is cloned from its official git URL during `bb pull`. The version used is controlled by the `Guid` setting for the `vcpkg` entry in [bbconfig.json](../../../imodel-native-internal/bbconfig.json). At build time, the vcpkg part runs the appropriate vcpkg install script located in this source tree.

By default, vcpkg-based builds will be cached locally in the `vcpkg` source tree. You can set the `VCPKG_BINARY_SOURCES` environment variable to `clear` if you want to force it to build every time, although this is not recommended. It's possible that in the future this will default to some Bentley shared binary cache, but for now it is always local to the build machine.

> **Note:** On all platforms, use `IMODEL_VCPKG_ROOT` (not `VCPKG_ROOT`) if you need to override the vcpkg location. The build wrappers check `IMODEL_VCPKG_ROOT` first, avoiding conflicts with tooling that may set `VCPKG_ROOT` to an undesired location. Since the build system installs the required version of vcpkg automatically, setting `IMODEL_VCPKG_ROOT` is not recommended.

## Updating Libraries to Use vcpkg

All vcpkg installs are driven by the sequential chain in [vcpkg.PartFile.xml](vcpkg.PartFile.xml) rather than by individual consumer `.mke` files. This ensures that no two `vcpkg` processes ever run concurrently, which is important because concurrent runs against the same install root will collide on `vcpkg-running.lock` and corrupt the build.

To add a new library:

1. Update `iModelCore/libsrc/README.md` — add a row to the library table with the directory, library name, initial version, and `Yes` in the vcpkg column.
2. Create a subdirectory (e.g. `mylib/`) containing `vcpkg.json`, `vcpkg-configuration.json`, and any custom triplet files under `mylib/triplets/`. See `./compress/` for examples.
3. Add a new `vcpkg_install_mylib.mke` in `libsrc/` (next to the other `vcpkg_install_*.mke` files) that calls `vcpkg_run_install.bat`/`vcpkg_run_install.sh` for that manifest directory.
4. Add a `vcpkg_install_mylib` part to [vcpkg.PartFile.xml](vcpkg.PartFile.xml), with `<SubPart PartName="vcpkg_install_<previous>" LibType="Static"/>` to preserve sequential ordering. Appending at the end of the chain is the simplest choice, but the only requirement is that the chain stays **linear** — position does not affect correctness, since each consumer depends on its own named part. A more basic/foundational library (e.g. a compression or image codec that other libraries build on) may read more naturally inserted earlier in the chain; if you insert mid-chain, re-parent the following link onto the new part and update the sibling `.mke` "Runs after vcpkg_install_<prev>…" comment on every link whose predecessor changed. The `LibType="Static"` is required — the chain runs static-only, so without it a dynamic build pass could run this chain part concurrently with the static one and collide on `vcpkg-running.lock`.
5. In your library's PartFile, depend on `vcpkg_install_mylib` (from `iModelCore/libsrc/vcpkg`) instead of the bare `vcpkg` part.
6. In your library's `.mke`, include [`vcpkg.mki`](./vcpkg.mki) to get the `vcpkgTriplet` variable, then consume the already-installed outputs from `$(OutputRootDir)vcpkg_installed/mylib/`. Do **not** call `vcpkg_run_install` from the `.mke` — the install was already performed by step 3. This path is only correct for **static** builds; for libraries with both static and dynamic deliverables, gate the path with `CREATE_STATIC_LIBRARIES` so dynamic builds read from the static chain's output:

   ```makefile
   %if defined (CREATE_STATIC_LIBRARIES)
   vcpkgInstallRoot = $(OutputRootDir)vcpkg_installed/mylib/
   %else
   vcpkgInstallRoot = $(OutputRootDir)static/vcpkg_installed/mylib/
   %endif
   ```

> **When migrating an existing (previously vendored) library to vcpkg:** the vendored source deletion belongs in the **same** PR as the vcpkg wiring, but do **not** delete it up front. Keep the vendored source in place (the PR will likely be draft/WIP at this stage) until **after** the PR has passed its Copilot review, then delete the vendored code in a separate standalone commit within that same PR. Deleting the vendored source up front produces too many modified files for Copilot to review, and the review may not run at all.

Versions are pinned per-library: `vcpkg.json` uses an `overrides` entry for the exact version, and `vcpkg-configuration.json` pins the registry baseline commit. Even though all `vcpkg-configuration.json` files should be identical, each manifest directory requires its own copy.

## Setup

### macOS

Install these prerequisites via Homebrew:

```bash
brew install cmake pkg-config
```

- **cmake** >= 4.3.2
- **pkg-config** (required by vcpkg for `.pc` fixup)

### Linux

Install these prerequisites before running vcpkg:

- C++ toolchain (gcc/g++, make)
- CMake
- pkg-config
- zip/unzip
- tar
- curl

Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config zip unzip tar curl git
```

RHEL/CentOS/Fedora (dnf):

```bash
sudo dnf install -y gcc gcc-c++ make cmake pkgconf-pkg-config zip unzip tar curl git
```

Set `IMODEL_VCPKG_ROOT` to override the vcpkg location (optional, not recommended):

```bash
export IMODEL_VCPKG_ROOT=~/src/vcpkg
```

Do not set `VCPKG_ROOT` directly; use `IMODEL_VCPKG_ROOT` instead.

Verify tooling is available:

```bash
cmake --version
pkg-config --version
g++ --version
```

### Windows

As on all platforms, the build wrapper uses `IMODEL_VCPKG_ROOT` as the override variable for specifying a standalone vcpkg checkout. Do not set `VCPKG_ROOT` directly — on Windows, Visual Studio tools may inject their own value for it. Since a specific version of vcpkg is installed as part of the Bentley Build, it is not recommended that you set `IMODEL_VCPKG_ROOT`.

Install the following prerequisites **before running vcpkg**:

**Compiler and Build Tools:**
- Visual Studio 2022 Build Tools (or Visual Studio 2022) with C++ workload installed
- Git
- CMake

<!-- **vcpkg Tool Dependencies:**
- **7-Zip** (v21.7.0) — must be manually downloaded and extracted to `<vcpkg root>/downloads/tools/`
  - Download: https://www.7-zip.org/a/7z2107-extra.7z
- **PowerShell Core** (v7.2.11) — must be manually downloaded and extracted to `<vcpkg root>/downloads/tools/`
  - Download: https://github.com/PowerShell/PowerShell/releases (look for v7.2.11 release, Windows x64 zip) -->

**Installation Steps:**

1. **Install Visual Studio 2022 Build Tools (or Visual Studio 2022):**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Run the installer and select the "Desktop development with C++" workload
   - Complete the installation

2. **Install CMake:**
   - Download from: https://cmake.org/download/
   - Install a current Windows x64 release
   - Verify: `cmake --version`

<!-- 3. **Manually install 7-Zip portable:**
   - Download: https://www.7-zip.org/a/7z2107-extra.7z
   - Create directory: `%USERPROFILE%\src\vcpkg\downloads\tools`
   - Extract `7z2107-extra.7z` to a temporary location
   - Copy the extracted folder to: `%USERPROFILE%\src\vcpkg\downloads\tools\7zip\21.07`
   - Verify: `%USERPROFILE%\src\vcpkg\downloads\tools\7zip\21.07\7zr.exe` should exist

4. **Manually install PowerShell Core portable:**
   - Download from: https://github.com/PowerShell/PowerShell/releases (v7.2.11 release, PowerShell-7.2.11-win-x64.zip)
   - Extract to: `%USERPROFILE%\src\vcpkg\downloads\tools\powershell-core\7.2.11`
   - Verify: `%USERPROFILE%\src\vcpkg\downloads\tools\powershell-core\7.2.11\pwsh.exe` should exist -->

3. **Set `IMODEL_VCPKG_ROOT` to your standalone checkout (not recommended):**

   ```bat
   setx IMODEL_VCPKG_ROOT "%USERPROFILE%\src\vcpkg"
   set IMODEL_VCPKG_ROOT=%USERPROFILE%\src\vcpkg
   ```

   Do not set `VCPKG_ROOT` directly. The build wrapper checks `IMODEL_VCPKG_ROOT` first, and if it is not set, the vcpkg inside the build tree is used.

4. **Verify your setup:**

   ```bat
   cmake --version
   "%USERPROFILE%\src\vcpkg\vcpkg.exe" version
   ```

#### Windows vcpkg selection order used by the wrapper

`vcpkg_run_install.bat` resolves roots in this order:

1. `IMODEL_VCPKG_ROOT` (if set and contains `vcpkg.exe`)
2. `<imodel-native>\iModelCore\libsrc\vcpkg`
3. `VCPKG_ROOT` (if set, valid, not the Visual Studio bundled root, and something causes the one above to fail to install)
4. `%USERPROFILE%\src\vcpkg`
5. Visual Studio bundled vcpkg (`%VCINSTALLDIR%\vcpkg`)

This avoids accidental use of the bundled root when `vcvars` or Developer Command Prompt modifies `VCPKG_ROOT`.

#### macOS/Linux vcpkg selection order used by the wrapper

`vcpkg_run_install.sh` resolves roots in this order:

1. `IMODEL_VCPKG_ROOT` (if set, not recommended)
2. `VCPKG_ROOT` (if already set in the environment, very not recommended)
3. `<imodel-native>/iModelCore/libsrc/vcpkg` (default)

## How It Works

- Each subdirectory with a `vcpkg.json` manifest declares its vcpkg dependencies and version pins.
- `vcpkg_run_install.sh` (macOS/Linux) and `vcpkg_run_install.bat` (Windows) wrap `vcpkg install`, directing output to `$OutRoot/vcpkg_installed/<consumer>/`.
- All `vcpkg install` calls are driven by a sequential chain of parts in [vcpkg.PartFile.xml](vcpkg.PartFile.xml) (`vcpkg_install_compress` → `vcpkg_install_png` → `vcpkg_install_openssl` → `vcpkg_install_crashpad`), each blocked on the previous one completing. This prevents concurrent `vcpkg` processes from colliding on shared state.
- Consumer `.mke` files (e.g. `Zlib.mke`, `BeOpenSSL.mke`, `png.mke`) depend on their corresponding chain part and only consume the already-installed outputs — they do not call `vcpkg_run_install` themselves.

## Version Pinning

Versions are locked via two mechanisms in each manifest directory:

- `vcpkg.json` → `overrides` array pins exact versions
- `vcpkg-configuration.json` → `baseline` pins the vcpkg registry commit

To update a library version, change the override in `vcpkg.json` and (if needed) update the baseline commit in `vcpkg-configuration.json`. Also update the version in the `iModelCore/libsrc/README.md` library table.
