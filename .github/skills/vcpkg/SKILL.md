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
        └─► vcpkg_install_openssl
              └─► vcpkg_install_crashpad
```

Each link is a separate Part with its own `vcpkg_install_<consumer>.mke` that calls
`vcpkg_run_install.bat` / `vcpkg_run_install.sh`.  Chaining ensures no two `vcpkg`
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

### 2. Create `iModelCore/libsrc/vcpkg_install_<mylib>.mke`

```makefile
%include mdl.mki

mylibDir    = $(_MakeFilePath)<mylib>/
installRoot = $(OutputRootDir)vcpkg_installed/<mylib>/

# Add vcpkgWindowsMDCRT = 1 here if the library must link /MD on Windows (like openssl).
%include $(_MakeFilePath)vcpkg.mki

always:
%if defined (winNT)
    $(_MakeFilePath)vcpkg_run_install.bat $(mylibDir) $(installRoot) $(vcpkgTriplet)
%else
    $(_MakeFilePath)vcpkg_run_install.sh $(mylibDir) $(installRoot) $(vcpkgTriplet)
%endif
```

`$(_MakeFilePath)` resolves to `libsrc/` because the file lives there, so the paths to
`<mylib>/`, `vcpkg_run_install.bat/sh`, and `vcpkg.mki` are all correct.

### 3. Extend the chain in `iModelCore/libsrc/vcpkg.PartFile.xml`

Append at the end of the chain (after the current last link):

```xml
<Part Name="vcpkg_install_<mylib>" BentleyBuildMakeFile="vcpkg_install_<mylib>.mke">
    <SubPart PartName="vcpkg_install_<current-last>"/>
</Part>
```

Update the block comment above the chain to name the new last link.

### 4. Wire the consumer PartFile

In your library's `.PartFile.xml`, depend on the chain part — **not** the bare `vcpkg` part:

```xml
<Part Name="MyLib" BentleyBuildMakeFile="MyLib.mke">
    <SubPart PartName="vcpkg_install_<mylib>" PartFile="iModelCore/libsrc/vcpkg"/>
    ...
```

### 5. Write the consumer `.mke`

Include `vcpkg.mki` to get `vcpkgTriplet`, set `vcpkgInstallRoot` to match the install
root used in `vcpkg_install_<mylib>.mke`, then consume the installed outputs.
**Do not call `vcpkg_run_install` here.**

```makefile
%include mdl.mki

mylibDir        = $(_MakeFilePath)
libsrcDir       = $(mylibDir)../

%include $(libsrcDir)vcpkg.mki

vcpkgInstallRoot = $(OutputRootDir)vcpkg_installed/<mylib>/
vcpkgTripletDir  = $(vcpkgInstallRoot)$(vcpkgTriplet)/
vcpkgIncludeDir  = $(vcpkgTripletDir)include/
vcpkgLibDir      = $(vcpkgTripletDir)lib/

# vcpkg install was already run by vcpkg_install_<mylib>.mke — just consume the outputs.
always:
    !~@mkdir $(o)
    # ... linkfile / linkdir / lib merge steps ...
```

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
| `iModelCore/libsrc/vcpkg_run_install.bat` / `.sh` | Wrapper that invokes the `vcpkg` executable |
| `iModelCore/libsrc/VCPKG.md` | Human-facing documentation; keep in sync when changing patterns |
