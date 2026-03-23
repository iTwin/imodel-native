---
name: update-third-party-lib
description: Update third-party libraries in imodel-native's libsrc directory following the vendor branch workflow (libsrc-Vendor → libsrc-Main → main). USE FOR updating, porting, or syncing any library under iModelCore/libsrc/. Covers the full branch workflow, conflict resolution, and push steps.
---

# Updating Third-Party Libraries in imodel-native

This guide covers the process for updating most third-party libraries that you can find in `iModelCore/libsrc/`. There is other third-party code embedded within our code that does not follow this generic process.

Because of the varied nature of third-party builds, we create Bentley build parts and MKE/MKI files and manually build the libraries our way. This ensures they reuse our build chain setup, and that they build in the same way across all platforms. This also makes setup and maintenance more difficult, as you'll have to react to addition/removal of source files, and new preprocessor macros that may have to get defined. Many libraries are Unix-first, and can only be understood by trying to use their standard `./configure` & `make` process on a Unix system to see what they do so you can replicate it. Once a library has already been ported, updates tend to be easier (look for added/removed files and hope macros didn't change).

## Background

There are two primary considerations for this process:

1. Third-party code needs to be committed on a separate branch, with all revisions on the same branch, and then merged in to our branches that have our modifications. This keeps us from losing our Bentley changes.
2. The imodel-native repository requires squash commits, which make git forget that it has merged from a branch. Without these merge revisions, it constantly tries to re-merge everything.

With those considerations, we have a `libsrc-Vendor` branch to commit the raw drops from the vendor, and `libsrc-Main` where we merge from `main` and `libsrc-Vendor` but can keep the merge revisions because this branch is not directly merged into `main`. Once `libsrc-Main` is building, then we create a normal PR branch from `main` and merge in `libsrc-Main` which will go through the squash commit. All Bentley changes must be done in `libsrc-Main`; `libsrc-Vendor` must be the source as downloaded from the vendor.

### Branch roles

| Branch | Purpose | What goes here |
|--------|---------|----------------|
| `libsrc-Vendor` | Raw vendor drops only | Unmodified source as downloaded. No Bentley build files, no patches. |
| `libsrc-Main` | Integration branch | Merges from both `main` and `libsrc-Vendor`. Bentley build/config changes happen here if needed during conflict resolution. Keeps real merge commits. |
| `main` | Production branch | Receives squash-merged PRs. Never merge `libsrc-Vendor` directly here. |

### Flow diagram

```
libsrc-Vendor  ──vendor drop──►  libsrc-Main  ──PR branch──►  main
                                      ▲
                                      │
                                 merge main
```

## File conventions

Each library under `iModelCore/libsrc/` typically has:

- **Vendor source files** (`.cpp`, `.h`, license/notice) - go on `libsrc-Vendor`
- **Bentley build files** (`.mke`, `.PartFile.xml`) - exist only on `main`/`libsrc-Main`
- **Bentley config files** (e.g. hand-written `config.h` replacing CMake output) - exist only on `main`/`libsrc-Main`
- **Bentley patches** (inline modifications to vendor source) - applied on `libsrc-Main` after vendor merge

When copying vendor source, rename files to match existing conventions (e.g. `.cc` → `.cpp` if the repo already uses `.cpp`).

## Steps

### 1. Prepare: Get the vendor source

Download the exact release archive from the upstream project. Note the exact URL - you'll need it for the commit message.

### 2. Update libsrc-Vendor

```bash
git checkout libsrc-Vendor
# For an existing library - delete and replace the directory:
rm -rf iModelCore/libsrc/<category>/<library>/
# Copy only the vendor source files (no build system files, no tests unless needed):
cp <downloaded>/<relevant-files> iModelCore/libsrc/<category>/<library>/
# Check for unwanted new directories - add to .gitignore if needed
git status
git add iModelCore/libsrc/<category>/<library>/
git commit -m "Updating <library> to version X.Y.Z from <exact-download-url>"
```

**Important**: The commit message MUST include the exact download URL. Git sometimes produces different diffs for zip vs tar.gz due to line endings, so recording the archive format helps future maintainers.

If the library is new to `libsrc-Vendor` (never tracked before), this will be an add-only commit. Expect add/add merge conflicts when merging to `libsrc-Main` later.

### 3. Merge main into libsrc-Main

```bash
git checkout libsrc-Main
git merge main
# Usually clean. Commit if needed.
```

### 4. Merge libsrc-Vendor into libsrc-Main

```bash
git merge libsrc-Vendor
```

Conflicts are common here, especially for:
- **add/add conflicts** (library new to vendor branch): resolve by taking the vendor version (`git checkout --theirs <file>`)
- **Content conflicts** (library update changed lines near Bentley patches): manually merge, keeping Bentley modifications where they're still needed

After resolving:
```bash
git add .
git commit  # or let the merge auto-commit if clean
```

### 5. Create PR branch and merge libsrc-Main

```bash
git checkout main
git checkout -b user/merge<Library>
git merge libsrc-Main
# Build and verify
git push origin user/merge<Library>
```

Create a PR. The PR will show many commits from other people due to squash history - this is expected.

**When completing the PR**: Edit the squash commit message to remove all the excess commit messages GitHub auto-adds. Keep only your own commit descriptions.

### 6. Push all branches

```bash
git push origin libsrc-Vendor
git push origin libsrc-Main
# PR branch is pushed in step 5
```

## Practical tips

- If your changes are already on a PR branch against `main` (e.g. you did the update directly), you can still retroactively update the vendor branches. Stash your work, do the vendor/main branch dance, then restore.
- When resolving add/add conflicts for vendor files, `--theirs` means the vendor branch version (since you're on `libsrc-Main` merging `libsrc-Vendor`).
- Libraries vary wildly in build setup. The `.mke` files and `PartFile.xml` are Bentley's build system replacements for whatever the upstream project uses (CMake, autotools, Bazel, etc.). You may need to replicate `./configure` behavior manually.
- You cannot build locally without Bentley's initialized shell environment. CI will validate platform builds you couldn't test locally.

## Example: snappy 1.2.2 update (March 2026)

Snappy had never been on `libsrc-Vendor` before (it was committed directly to main in the initial open-source commit). The update to 1.2.2 involved:

1. **Vendor files** (on `libsrc-Vendor`): `snappy.cpp`, `snappy-internal.h`, `snappy-stubs-internal.h`, `snappy-sinksource.cpp`, `snappy-stubs-internal.cpp`, `snappy-notice.txt` - all raw from upstream, `.cc` renamed to `.cpp`
2. **Bentley files** (on PR branch / main only): `config.h` (hand-written replacement for CMake config), `snappy.mke` (Bentley build), 3-line `#include` append at end of `snappy.cpp` to fold compilation units for bmake
3. Conflicts were add/add (new to vendor branch), resolved by taking `--theirs` (vendor version)
