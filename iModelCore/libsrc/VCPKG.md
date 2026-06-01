# vcpkg Integration

This directory uses [vcpkg](https://github.com/microsoft/vcpkg) to manage select third-party library builds. vcpkg fetches source and builds libraries locally; the resulting headers and static libraries are delivered to Bentley Build via `.mke` files.

## Setup

### macOS/Linux

1. Clone vcpkg:

    ```bash
    git clone https://github.com/microsoft/vcpkg.git ~/src/vcpkg
    cd ~/src/vcpkg
    ./bootstrap-vcpkg.sh
    ```

2. Set `VCPKG_ROOT` (optional — defaults to `~/src/vcpkg`):

    ```bash
    export VCPKG_ROOT=~/src/vcpkg
    ```

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

Set `VCPKG_ROOT` (optional — defaults to `~/src/vcpkg`):

```bash
export VCPKG_ROOT=~/src/vcpkg
```

Verify tooling is available:

```bash
cmake --version
pkg-config --version
g++ --version
```

### Windows

1. Clone vcpkg:

    ```bat
    git clone https://github.com/microsoft/vcpkg.git %USERPROFILE%\src\vcpkg
    cd %USERPROFILE%\src\vcpkg
    .\bootstrap-vcpkg.bat
    ```

2. Set `VCPKG_ROOT` (optional — defaults to `%USERPROFILE%\src\vcpkg`):

    ```bat
    set VCPKG_ROOT=%USERPROFILE%\src\vcpkg
    ```

__TODO:__ Document remaining required tooling

### Windows

On Windows, the build wrapper supports a project-specific override variable named `IMODEL_VCPKG_ROOT`. Use this instead of `VCPKG_ROOT` when you want deterministic selection of a standalone vcpkg checkout in environments where Visual Studio tools may inject `VCPKG_ROOT`.

Install the following prerequisites **before running vcpkg**:

**Compiler and Build Tools:**
- Visual Studio 2022 Build Tools (or Visual Studio 2022) with C++ workload installed
- Git
- CMake

**vcpkg Tool Dependencies:**
- **7-Zip** (v21.7.0) — must be manually downloaded and extracted to `<vcpkg root>/downloads/tools/`
  - Download: https://www.7-zip.org/a/7z2107-extra.7z
- **PowerShell Core** (v7.2.11) — must be manually downloaded and extracted to `<vcpkg root>/downloads/tools/`
  - Download: https://github.com/PowerShell/PowerShell/releases (look for v7.2.11 release, Windows x64 zip)

**Installation Steps:**

1. **Install Visual Studio 2022 Build Tools (or Visual Studio 2022):**
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Run the installer and select the "Desktop development with C++" workload
   - Complete the installation

2. **Install CMake:**
   - Download from: https://cmake.org/download/
   - Install a current Windows x64 release
   - Verify: `cmake --version`

3. **Clone and Bootstrap vcpkg:**

   In a Developer Command Prompt or PowerShell:

   ```bat
   git clone https://github.com/microsoft/vcpkg.git %USERPROFILE%\src\vcpkg
   cd %USERPROFILE%\src\vcpkg
   bootstrap-vcpkg.bat
   ```

4. **Manually install 7-Zip portable:**
   - Download: https://www.7-zip.org/a/7z2107-extra.7z
   - Create directory: `%USERPROFILE%\src\vcpkg\downloads\tools`
   - Extract `7z2107-extra.7z` to a temporary location
   - Copy the extracted folder to: `%USERPROFILE%\src\vcpkg\downloads\tools\7zip\21.07`
   - Verify: `%USERPROFILE%\src\vcpkg\downloads\tools\7zip\21.07\7zr.exe` should exist

5. **Manually install PowerShell Core portable:**
   - Download from: https://github.com/PowerShell/PowerShell/releases (v7.2.11 release, PowerShell-7.2.11-win-x64.zip)
   - Extract to: `%USERPROFILE%\src\vcpkg\downloads\tools\powershell-core\7.2.11`
   - Verify: `%USERPROFILE%\src\vcpkg\downloads\tools\powershell-core\7.2.11\pwsh.exe` should exist

6. **Set `IMODEL_VCPKG_ROOT` to your standalone checkout (recommended):**

   ```bat
   setx IMODEL_VCPKG_ROOT "%USERPROFILE%\src\vcpkg"
   set IMODEL_VCPKG_ROOT=%USERPROFILE%\src\vcpkg
   ```

   You may still set `VCPKG_ROOT`, but `IMODEL_VCPKG_ROOT` has higher priority in `vcpkg_run_install.bat`.

7. **Optional: also set `VCPKG_ROOT` for direct shell usage of vcpkg:**

   ```bat
   setx VCPKG_ROOT "%USERPROFILE%\src\vcpkg"
   set VCPKG_ROOT=%USERPROFILE%\src\vcpkg
   ```

8. **Verify your setup:**

   ```bat
   cmake --version
   "%USERPROFILE%\src\vcpkg\vcpkg.exe" version
   ```

   With CMake installed and 7-Zip and PowerShell Core manually installed in `downloads/tools/`, vcpkg should now be ready to build zlib and minizip without downloading tools during the build.

#### Windows vcpkg selection order used by the wrapper

`vcpkg_run_install.bat` resolves roots in this order:

1. `IMODEL_VCPKG_ROOT` (if set and contains `vcpkg.exe`)
2. `VCPKG_ROOT` (if set, valid, and not the Visual Studio bundled root)
3. `D:\src\vcpkg`
4. `%USERPROFILE%\src\vcpkg`
5. Visual Studio bundled vcpkg (`%VCINSTALLDIR%\vcpkg`)

This avoids accidental use of the bundled root when `vcvars` or Developer Command Prompt modifies `VCPKG_ROOT`.

## How It Works

- Each subdirectory with a `vcpkg.json` manifest declares its vcpkg dependencies and version pins.
- The `vcpkg_run_install.sh` (macOS/Linux) and `vcpkg_run_install.bat` (Windows) scripts wrap `vcpkg install`, directing output to `$OutRoot/vcpkg_installed/<triplet>/`.
- `.mke` files invoke the script, then deliver the resulting headers and `.a`/`.lib` files to the Bentley Build context.

## Version Pinning

Versions are locked via two mechanisms in each manifest directory:

- `vcpkg.json` → `overrides` array pins exact versions
- `vcpkg-configuration.json` → `baseline` pins the vcpkg registry commit

To update a library version, change the override in `vcpkg.json` and (if needed) update the baseline commit in `vcpkg-configuration.json`.
