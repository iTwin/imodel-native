# vcpkg Integration

This directory uses [vcpkg](https://github.com/microsoft/vcpkg) to manage select third-party library builds. vcpkg fetches source and builds libraries locally; the resulting headers and static libraries are delivered to Bentley Build via `.mke` files. The Bentley Build process installs the required version of vcpkg automatically, but the prerequisites listed below must be installed separately.

A specific version of vcpkg is cloned from its official git URL during `bb pull`. The version used is controlled by the `Guid` setting for the `vcpkg` entry in [bbconfig.json](../../../imodel-native-internal/bbconfig.json). At build time, the vcpkg part runs the appropriate vcpkg install script located in this source tree.

By default, vcpkg-based builds will be cached locally in the `vcpkg` source tree. You can set the `VCPKG_BINARY_SOURCES` environment variable to `clear` if you want to force it to build every time, although this is not recommended. It's possible that in the future this will default to some Bentley shared binary cache, but for now it is always local to the build machine.

> **Note:** On all platforms, use `IMODEL_VCPKG_ROOT` (not `VCPKG_ROOT`) if you need to override the vcpkg location. The build wrappers check `IMODEL_VCPKG_ROOT` first, avoiding conflicts with tooling that may set `VCPKG_ROOT` to an undesired location. Since the build system installs the required version of vcpkg automatically, setting `IMODEL_VCPKG_ROOT` is not recommended.

## Updating Libraries to Use vcpkg

You must add vcpkg as a SubPart of your library Part in order to insure that its install script is run before your library is built, like is done for the Zlib part in [Zlib.PartFile.xml](compress/Zlib.PartFile.xml).

Most libraries that we build with vcpkg will need their own triplet files to configure the library to build with the settings that we want. The triplet files are platform-specific. See `./compress/triplets` for example triplets. [`vcpkg.mki`](./vcpkg.mki) is used to determine the proper triplet to use at build time. Include that from any `.mke` file that builds using vcpkg. This will set the `vcpkgTriplet` environment variable.

Additionally, we pin the libraries to a specific version via their vcpkg.json file. ([here](./compress/vcpkg.json) is the one used for zlib and minizip.) `vcpkg.json` will contain both a `version>=` setting under `dependencies` and a `version` entry under `overrides`.

Each library also needs a copy of `vcpkg-configuration.json`. Unfortunately, this file must be in the same directory as the library-specific `vcpkg.json`, so even though all of them should be identical, they need to be copied to the library's directory. See [here](./compress/vcpkg-configuration.json) for an existing one. The `baseline` hash may have to be updated to support some specific version of a library.

To actually build your library with vcpkg, run `vcpkg_run_install.bat` when building on Windows, or `vcpkg_run_install.sh` otherwise. **Note:** make sure to pick the right `vcpkg_run_install` based on the build platform, not the target platform. Android builds work fine on both Windows and macOS. See [Zlib.mke](./compress/Zlib.mke) for an example.

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

   With CMake installed and 7-Zip and PowerShell Core manually installed in `downloads/tools/`, vcpkg should now be ready to build zlib and minizip without downloading tools during the build.

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
- The `vcpkg_run_install.sh` (macOS/Linux) and `vcpkg_run_install.bat` (Windows) scripts wrap `vcpkg install`, directing output to `$OutRoot/vcpkg_installed/<triplet>/`.
- `.mke` files invoke the script, then deliver the resulting headers and `.a`/`.lib` files to the Bentley Build context.

## Version Pinning

Versions are locked via two mechanisms in each manifest directory:

- `vcpkg.json` → `overrides` array pins exact versions
- `vcpkg-configuration.json` → `baseline` pins the vcpkg registry commit

To update a library version, change the override in `vcpkg.json` and (if needed) update the baseline commit in `vcpkg-configuration.json`.
