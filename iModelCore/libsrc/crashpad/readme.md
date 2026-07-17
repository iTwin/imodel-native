At this time:
- Windows, Linux, and macOS builds always compile and include crashpad
- You must define `LINUX_MINIDUMP_ENABLED` (Linux only) **or** `IMODEL_ADDON_MINIDUMP_ENABLED` (any crashpad-supported OS) in the environment to enable crashpad handling. **Note:** This is done automatically inside certain tests that get run for certain build strategies.
- You must define `MINIDUMP_UPLOAD_URL=...` to enable automatic uploads (additionally `MINIDUMP_UPLOAD_BACKTRACE_TOKEN=...` for backtrace.io)
- See [VCPKG.md](../VCPKG.md) for information about vcpkg builds like this one.

## Symbol processing for sentry.io

- It seems recommended that you upload the entire binary (remember symbols begin embedded in the binary)
- You can first compress symbol information in-place: `objcopy --compress-debug-sections BINARY_PATH`
- And then upload: `sentry-cli upload-dif -t elf BINARY_PATH[ BINARY_PATH+]`
    - You must also upload system symbols, like node; it ships with symbols, so you just add /usr/bin/node to the upload call
    - You must initally `sentry-cli login` to establish the connection
    - You can call `sentry-cli difutil check BINARY_PATH` to verify symbols
- Strip symbols before packaging / publishing: `objcopy --strip-debug --strip-unneeded BINARY_PATH`

---
# Updating crashpad (2026)

There are **two** sources of crashpad depending on platform, and both must be kept in sync
when updating:

- **Windows** (all triplets — both MSVC and clang): built from the **local vcpkg overlay
  port** at `iModelCore/libsrc/crashpad/ports/crashpad/`. The overlay is wired in by the
  `--overlay-ports` handling in `iModelCore/libsrc/vcpkg_run_install.bat`, which passes any
  `ports/` subdirectory next to `vcpkg.json` to vcpkg. On Windows the overlay port — **not**
  the top-level `iModelCore/libsrc/crashpad/vcpkg.json` version constraint — determines the
  source that is pulled and built. (The overlay exists mainly to make crashpad build with
  clang-cl; `portfile.cmake` branches internally on `CRASHPAD_USE_CLANG` and also handles
  the MSVC case.)
- **Linux, macOS, Android**: built from the **upstream vcpkg registry** port.
  `iModelCore/libsrc/vcpkg_run_install.sh` does **not** pass `--overlay-ports`, so the
  overlay is ignored. Here the version is governed by `version>=`/`overrides` in the
  top-level `iModelCore/libsrc/crashpad/vcpkg.json` and the `baseline` commit ID in
  `iModelCore/libsrc/crashpad/vcpkg-configuration.json`.

**Overlay provenance:** the overlay port was forked from the upstream vcpkg registry
`crashpad` port at version `2024-04-11#13`, then modified locally (extra patches plus the
clang-cl / MSBuild-header handling in `portfile.cmake`). When re-syncing against a newer
upstream vcpkg port, diff against that upstream version to see which local changes still
need to be carried forward.

To change the crashpad version:

1. **Update the top-level manifest for the non-Windows platforms:** bump `version>=` and
   `overrides` in `iModelCore/libsrc/crashpad/vcpkg.json`, and (if needed) the `baseline`
   commit ID in `iModelCore/libsrc/crashpad/vcpkg-configuration.json`.
2. **Update the Windows overlay to match.** In `ports/crashpad/portfile.cmake` the version
   is pinned by three separate `REF` commit hashes, which typically move together:
   - `crashpad` (`URL .../crashpad/crashpad`)
   - `mini_chromium` (`URL .../chromium/mini_chromium`) — toolchains and build config
   - `lss` / linux-syscall-support (Android/Linux code path only)
3. **Regenerate the overlay patches** in `ports/crashpad/` against the new revisions. Some
   are Bentley-local (e.g. the `output_name` / library-name-conflict patches) and some
   mirror upstream vcpkg patches (e.g. `crashpad-memset-errors-5758170.diff`, whose source
   URL is noted inline). A patch that no longer applies must be rebased or dropped.
4. **Bump the overlay's own `ports/crashpad/vcpkg.json`** (`version-date` and/or
   `port-version`) and keep it `>=` the `version>=`/`overrides` values in the top-level
   `iModelCore/libsrc/crashpad/vcpkg.json`, otherwise vcpkg will not select the overlay on
   Windows.

Keep the Windows overlay and the non-Windows upstream version pointing at the **same**
crashpad revision so all platforms build the same code.

---
# Old WIP Notes on pulling it now (2023)

**Note:** Now that we are using `vcpkg` to build crashpad, the below no longer applies, but I felt that deleting it wasn't a good idea.

Latest attempt (2023)

They changed the code a bit so it more heavily relies on Ninja and the Google build system.

I make this build on Windows for imodel02 to copy and because it was convenient to edit on Windows; however this is only built with Linux in imodel-native. You can modify imodeljsnodeaddon to add CrashpadShim as a subpart of iModelJsNative-Dynamic and then remove the Linux-only restriction on the CrashpadShim part and it should build on Windows.

I don't think this attempt was particularly efficient so it's not so much a procedure as a starting point to develop one.
Probably starting on Linux will be a better workflow.

1. Pull crashpad with vcpkg and get it to build on Windows. Using vcpkg rather than loading depot tools and trying to build myself.
    `vcpkg install crashpad`

2. In a separate directory, pull crashpad source to same GUID from https://chromium.googlesource.com/crashpad/crashpad
    - You can get the GUID by going into the soure directory vcpkg\buildtrees\crashpad\src\[Id1-Id2].clean directory and doing a "git log -1"
    - This will get a clean set of source since the one in vcpkg will have all the tools and other stuff pulled into it. At this point I'm trying to avoid committing Ninja et al. to the source tree.
    - `Robocopy /mir checked out source to clean out deleted files in iModelCore\libsrc\crashpad\vendor`

3. I needed to copy in several files from vcpkg that aren't in the source tree but are generated at build time
    - `copy vcpkg\buildtrees\crashpad\src\[GUID].clean\third_party\mini_chromium\mini_chromium\base imodel-native\iModelCore\libsrc\crashpad\vendor\third_party\mini_chromium\mini_chromium\base`
    - `copy vcpkg\buildtrees\crashpad\src\[GUID].clean\third_party\mini_chromium\mini_chromium\build imodel-native\iModelCore\libsrc\crashpad\vendor\third_party\mini_chromium\mini_chromium\build`

4. I also needed to pull the vcpkg on Linux and copy in lss for tools
    - `scp -r chuck@chuckdeb11.bentley.com:/home/chuck/vcpkg/vcpkg/buildtrees/crashpad/src/4c3e2d1c10-27b97453fa.clean/third_party/lss vendor\third_party\lss`


- Make sure mini-chromium and lss are commented out in vendor/.gitignore so they will commit

- Build, adjusting to add/remove files as necessary

- Do the usual merging between libsrc-Vendor and libsrc-Main as described on the wiki.

---
# The previous technique (2019)

## Updating source from Google

https://chromium.googlesource.com/crashpad/crashpad/+/master/doc/developing.md

## Get the source
```
cd ~
mkdir crashpad
cd crashpad
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=~/crashpad/depot_tools:$PATH
fetch crashpad
```

## Generate revision report
```
cd ~/crashpad/crashpad

# Main Repo
git rev-parse HEAD

# Submodules
find `pwd` -iname .git | xargs -n 1 -- bash -c 'echo $0 && git -C $0/.. rev-parse HEAD && git -C $0/.. remote -v && echo'
```

### (if needed) Build it Google's way (for reference to update MKE files)

```
cd ~/crashpad/crashpad
gn gen out/Default
ninja -C out/Default
```

The ninja files for each component in out/Default (e.g. out/Default/obj/client/client.ninja) describe which source files are part of each component, and the arguments used to build them.

## Switch to vendor branch in imodel02

```
cd ~/imodel02/src/imodel02

# FIRST TIME
git checkout --orphan vendor/google_crashpad

# OTHER TIMES
git checkout vendor/google_crashpad
```

## Update source

```
cd ~/imodel02/src/imodel02

# FIRST TIME
git rm -rf .

# OTHER TIMES
rm -rf crashpad

mkdir crashpad
cp -R ~/crashpad/crashpad/* crashpad
cd crashpad
find . -iname .git | xargs rm -rf
git add .
```

Then `git commit` and include the revision report.

## Merge

```
cd ~/imodel02/src/imodel02
git checkout master
git checkout -b update_crashpad

# FIRST TIME
git merge --no-commit --allow-unrelated-histories vendor/google_crashpad
mkdir iModelCore/libsrc/crashpad/vendor
git mv crashpad/* iModelCore/libsrc/crashpad/vendor
rmdir crashpad

# OTHER TIMES
git merge vendor/google_crashpad

git commit ...
```
