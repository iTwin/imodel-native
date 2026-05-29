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

__TODO:__ Document required packages (cmake, pkg-config, build-essential, etc.)

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

## How It Works

- Each subdirectory with a `vcpkg.json` manifest declares its vcpkg dependencies and version pins.
- The `vcpkg_run_install.sh` script wraps `vcpkg install`, directing output to `$OutRoot/vcpkg_installed/<triplet>/`.
- `.mke` files invoke the script, then deliver the resulting headers and `.a`/`.lib` files to the Bentley Build context.

## Version Pinning

Versions are locked via two mechanisms in each manifest directory:

- `vcpkg.json` → `overrides` array pins exact versions
- `vcpkg-configuration.json` → `baseline` pins the vcpkg registry commit

To update a library version, change the override in `vcpkg.json` and (if needed) update the baseline commit in `vcpkg-configuration.json`.
