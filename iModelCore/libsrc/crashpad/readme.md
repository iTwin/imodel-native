# WIP Notes

At this time:
- Linux builds always compile and include crashpad
- You must define LINUX_MINIDUMP_ENABLED=1 in the environment to enable crashpad handling
- You must define MINIDUMP_UPLOAD_URL=... to enable automatic uploads (additionally MINIDUMP_UPLOAD_BACKTRACE_TOKEN=... for backtrace.io)

## Symbol processing for sentry.io

- It seems recommended that you upload the entire binary (remember symbols begin embedded in the binary)
- You can first compress symbol information in-place: `objcopy --compress-debug-sections BINARY_PATH`
- And then upload: `sentry-cli upload-dif -t elf BINARY_PATH[ BINARY_PATH+]`
    - You must also upload system symbols, like node; it ships with symbols, so you just add /usr/bin/node to the upload call
    - You must initally `sentry-cli login` to establish the connection
    - You can call `sentry-cli difutil check BINARY_PATH` to verify symbols
- Strip symbols before packaging / publishing: `objcopy --strip-debug --strip-unneeded BINARY_PATH`

# Updating source from Google

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

# First time
git checkout --orphan vendor/google_crashpad

# Subsequent times
git checkout vendor/google_crashpad
```

## Update source

```
cd ~/imodel02/src/imodel02
git rm -rf .
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

# First time
git merge --no-commit --allow-unrelated-histories vendor/google_crashpad
mkdir iModelCore/libsrc/crashpad/vendor
git mv crashpad/* iModelCore/libsrc/crashpad/vendor
rmdir crashpad

# Subsequent times
git merge vendor/google_crashpad

git commit ...
```
