# Migrating the OpenSSL build to vcpkg

> **Update (post-migration):** OpenSSL has since been bumped from 3.4.6 to **3.6.3**, which the
> vcpkg registry now packages natively. The overlay port is therefore a verbatim copy of the
> registry's 3.6.3 `ports/openssl` plus our two Bentley patches; it no longer needs to pin a
> version the registry lacks. See [`overlay-ports/README.md`](overlay-ports/README.md) for the
> current state. The 3.4.6-specific notes below are retained for historical context.

This document enumerates the steps to replace the current build-from-source OpenSSL
(`iModelCore/libsrc/openssl`) with a vcpkg-driven build, while retaining the three pieces
of custom behavior we depend on:

1. The `BSIVer` version-string section/symbol (see `BentleyVersionString.c`).
2. The Android `by_dir.c` patch (see `READ_BEFORE_UPDATING_CODE.md`).
3. Hardware-acceleration assembly (`.asm` / `.s` / `.S`).

The model to follow is the existing crashpad vcpkg integration:
[`../crashpad/client.mke`](../crashpad/client.mke), [`../crashpad/vcpkg.json`](../crashpad/vcpkg.json),
[`../crashpad/vcpkg-configuration.json`](../crashpad/vcpkg-configuration.json), and the shared
[`../vcpkg.mki`](../vcpkg.mki) / `vcpkg_run_install.sh` / `vcpkg_run_install.bat` helpers.

---

## Guiding decision: build static, re-link ourselves

vcpkg should build OpenSSL as **static archives** (`libcrypto`/`libssl`), and a thin
`BeOpenSSL.mke` should re-link/merge them into our `iTwinOpenSSL` library exactly the way
[`../crashpad/client.mke`](../crashpad/client.mke) merges the crashpad archives.

This single decision is what lets us keep full control of:
- the final exported symbol set (the `.def`), and
- injecting any object we own.

It also keeps "pick a new OpenSSL version" a one-line change in `vcpkg.json`.

> If, after the export audit below, we decide we do **not** need the non-standard asm
> exports, we can instead consume vcpkg's OpenSSL artifacts more directly. Until that is
> confirmed, assume the re-link approach.

### Per-platform output matrix

The final deliverable differs by platform, but **every** platform uses a vcpkg *static*
triplet — we never consume vcpkg's own OpenSSL DLLs. The split only changes our final link step.

| Platform | Final `iTwinOpenSSL` deliverable | Final link step | `.def` applied? | `BSIVer` section? |
|----------|----------------------------------|-----------------|-----------------|-------------------|
| Windows  | `iTwinOpenSSL.dll` + import `.lib` | link a DLL from the static archives | Yes (item 1 of custom behavior) | Yes |
| Linux    | static `iTwinOpenSSL.a` | merge archives | n/a | n/a (`_MSC_VER`-only) |
| macOS    | static `iTwinOpenSSL.a` | merge archives | n/a | n/a |
| iOS      | static `iTwinOpenSSL.a` | merge archives | n/a | n/a |
| Android  | static `iTwinOpenSSL.a` | merge archives | n/a | n/a |

This is why the custom export control (`.def`) and the version section (`BSIVer`) are purely
Windows concerns: Windows is the only platform that produces a DLL, and both are already
Windows-gated today (`.def` files say "Windows-only"; `BentleyVersionString.c` is `_MSC_VER`-only).
Windows is dynamic; Linux, macOS, iOS, and Android are all static-only (confirmed).

---

## Step 1 — Add the vcpkg manifest and registry pin

Create, in this folder, files mirroring crashpad's:

- `vcpkg.json` — declare the `openssl` dependency with a pinned version and an `overrides`
  entry, e.g.:

  ```json
  {
    "name": "imodel-native-openssl",
    "version": "1.0.0",
    "dependencies": [
      { "name": "openssl", "version>=": "<chosen-version>" }
    ],
    "overrides": [
      { "name": "openssl", "version": "<chosen-version>" }
    ]
  }
  ```

- `vcpkg-configuration.json` — pin the registry `baseline` commit (same shape as crashpad's).
  Add an `overlay-ports` entry pointing at the overlay port created in Step 2.

- `vcpkg-configuration.json` `default-registry` baseline drives the version set;
  bumping OpenSSL = change the version in `vcpkg.json` and (if needed) the `baseline`.

> Chosen version: **3.4.6** (the actual version we currently ship; `BentleyVersionString.c`
> incorrectly said 3.1.8). **The vcpkg registry does not package 3.4.6** — at our baseline the
> 3.4 line only has 3.4.0 and 3.4.1, then it jumps to 3.5.x/3.6.x. Because we are building an
> overlay port anyway (Step 2), the overlay port **pins the version to 3.4.6** itself (it fetches
> the 3.4.6 source), which satisfies the `version>=` / `overrides` constraints in `vcpkg.json`.
> The registry baseline then only matters for openssl's transitive build deps, not openssl itself.

Reuse the shared triplet resolution in [`../vcpkg.mki`](../vcpkg.mki) (it already maps
`TARGET_PROCESSOR_ARCHITECTURE` → vcpkg triplet for all our platforms). Use **static**
triplets on **every** platform — including Windows — so we control the final link and the
exported surface ourselves (see the output matrix above). Do **not** use vcpkg's dynamic
Windows triplet: it would produce upstream-named `libcrypto-3-x64.dll` / `libssl-3-x64.dll`
with OpenSSL's stock exports and no `BSIVer` section, instead of our single `iTwinOpenSSL.dll`.

> Windows CRT note: pick the static triplet whose CRT linkage matches the rest of our Windows
> binaries. If we build against the dynamic CRT (`/MD`), use `x64-windows-static-md` rather
> than `x64-windows-static` (`/MT`); a CRT mismatch when linking OpenSSL into `iTwinOpenSSL.dll`
> will cause runtime/link problems. Confirm what [`../vcpkg.mki`](../vcpkg.mki) currently selects
> for `x64` and adjust if needed.

---

## Step 2 — Create an overlay port for our patches  ✅ DONE

> **Status:** Complete. The overlay port lives at `overlay-ports/openssl/`, based on the
> registry's 3.4.1 port with `vcpkg.json` `"version"` bumped to `3.4.6` and `portfile.cmake`'s
> `REF`/`SHA512` pointed at the `openssl-3.4.6` GitHub archive
> (`SHA512 243e250e…79a84`). Our two patches — `android-by_dir.patch` and
> `bsiver-version-string.patch` — are appended to the `PATCHES` list. All 11 patches (the 9
> carried from 3.4.1 plus our 2) were verified to apply cleanly to fresh 3.4.6 source with both
> `patch -p1 --dry-run` and `git apply --check`.

We need a thin overlay port so that our source patches are applied during the vcpkg build.
Copy the upstream `openssl` port into an `overlay-ports/openssl/` directory under this folder
and reference it from `vcpkg-configuration.json` (`overlay-ports`).

Because the registry has no 3.4.6, this overlay port also **pins the version**:
- Base it on the registry's **3.4.1** port (closest in the same minor series, so its build logic
  and bundled patches are most likely compatible with 3.4.6 source).
- In `portfile.cmake`, update the `vcpkg_from_github` `REF`/tag to the `openssl-3.4.6` release
  and update the `SHA512` to the 3.4.6 source archive's hash.
- Set the overlay port's `vcpkg.json` `"version": "3.4.6"` so it satisfies our manifest pin.
- Re-verify any patches the upstream 3.4.1 port bundles still apply to 3.4.6 source.

In the port's `portfile.cmake`, add our patches to the `PATCHES` list of the
`vcpkg_from_github` / source-extraction call:

- `android-by_dir.patch` — item (2). Reproduce the existing diff from
  `READ_BEFORE_UPDATING_CODE.md` (`X509_NAME_hash_old` under `#if defined(__ANDROID__)`).
  It is guarded by `__ANDROID__`, so it is safe to apply on all triplets.
- `bsiver-version-string.patch` — item (1). See Step 3.

Keep the patches minimal and anchored to stable file context so they survive version bumps.

> Maintenance: on every OpenSSL bump, re-confirm both patches still apply cleanly.

---

## Step 3 — Inject the `BSIVer` version string via a patch  ✅ DONE

> **Status:** Complete, implemented as `overlay-ports/openssl/bsiver-version-string.patch`
> (already wired into `portfile.cmake`'s `PATCHES`). It appends a Windows-only (`_MSC_VER`)
> `BSIVer` `const_seg` block to `crypto/cversion.c` — chosen because that TU is always compiled
> and `OpenSSL_version*` are always linked. The array is `static` (faithful to the original
> `BentleyVersionString.c`) and the string is derived from `OPENSSL_VERSION_STR` (already in
> scope there, expands to `"3.4.6"`), producing `#@!~BeOpenSSL 3.4.6; OpenSSL 3.4.6, 3.4.6~!@#`
> — byte-for-byte the original format, now auto-tracking the pinned version. No `.def` entry and
> no `/INCLUDE` are used: the audit tool matches the `BSIVer` **section name** via DumpBin, and
> without `/Gw` the named-section data is not COMDAT, so `/OPT:REF` cannot strip it (matching the
> long-shipping original). Validated with `patch -p1` and `git apply --check` against 3.4.6.

Rather than compiling our own `BentleyVersionString.c` into upstream's libraries (vcpkg owns
that build), inject the `BSIVer` section by **patching an always-compiled, always-linked
OpenSSL source file** (e.g. `crypto/cversion.c`).

The patch appends the existing Windows-only block from `BentleyVersionString.c`:

```c
#ifdef _MSC_VER
#pragma const_seg("BSIVer")
#pragma const_seg()
static __declspec(allocate("BSIVer")) char szSourceFileVersionString[] =
    "#@!~BeOpenSSL " OPENSSL_VERSION_TEXT " ...";
#endif
```

Improvements to make at the same time:
- Derive the string from `OPENSSL_VERSION_TEXT` instead of hardcoding `3.1.8`, so a version
  bump needs no edit here.
- The audit tool locates this by **section name via DumpBin**, not by an exported symbol,
  so `static` is fine and no `.def` entry is required for it.

**Verify it is not linker-stripped.** A `static`, unreferenced array can be removed by MSVC
`/OPT:REF` (or `--gc-sections`). If it is stripped, the cheap fixes are:
- `#pragma comment(linker, "/INCLUDE:szSourceFileVersionString")` (requires non-`static`), or
- export it via the `.def` (Step 5), or
- mark it used (`__attribute__((used))` under clang-cl).

Confirm behavior under the default `WINDOWS_CLANG` toolset (the existing file already relies on
`const_seg` + `__declspec(allocate)` there).

---

## Step 4 — Drop the hand-rolled hardware-acceleration machinery  ◩ PARTIAL

> **Status:** The `BeOpenSSL.mke` portion is done — the new makefile (Step 6) contains none of
> the `AES_ASM`/`VPAES_ASM`/`BSAES_ASM`/`OPENSSL_CPUID_OBJ` defines, no `.asm`/`.s` compile
> rules, and no AndroidARM64 special-casing; vcpkg's `Configure` generates and assembles the
> perlasm itself. The **file deletions** below (checked-in generated asm, `generateAssemblyFor
> Windows.pl`, the `OPENSSL_NO_ASM` edit, and the whole `vendor/` tree) are intentionally
> **deferred to the cleanup phase** so the source-based build remains a working fallback until
> the vcpkg build is verified on every platform (Step 8). Remaining action item: confirm the
> Windows build agents allow vcpkg to fetch/run `nasm` (normally self-provisioned).

vcpkg's OpenSSL port runs OpenSSL's own `Configure`, which **generates and assembles** the
perlasm (`.s` / `.asm` / `.S`) for every supported architecture (it provisions `nasm` itself
on Windows). Item (3) is therefore handled automatically. Remove the now-obsolete machinery:

- The checked-in generated assembly under `vendor/.../*.asm`, `*.s`, `*.S`.
- `generateAssemblyForWindows.pl`.
- The `OPENSSL_NO_ASM` edit to `vendor/include/openssl/opensslconf.h`.
- The `AES_ASM` / `VPAES_ASM` / `BSAES_ASM` / `OPENSSL_CPUID_OBJ` defines and the
  per-platform / AndroidARM64 special-casing in `BeOpenSSL.mke`.

Only action item: confirm the Windows build agents allow vcpkg to fetch/run `nasm`
(normally self-provisioned).

---

## Step 5 — Resolve the exported symbol set (`.def`)  ✅ DECIDED (Option A)

> **Decision:** Keep the static vcpkg triplet on Windows and **retain the full hand-maintained
> `.def`** (`BeOpenSSL.def` + the clang-filtered `BeOpenSSL_clang.def`). Rationale below.

> **Critical finding:** We use the **static** vcpkg triplet on *every* platform — Windows is
> `x64-windows-static` (see [`../vcpkg.mki`](../vcpkg.mki)). A static `libcrypto.lib`/`libssl.lib`
> carries **no export information**, so when we re-link `iTwinOpenSSL.dll` ourselves the export
> list must come from *us*. The earlier assumption that "vcpkg/OpenSSL regenerates the export set
> via `mkdef.pl`" only holds for the *dynamic* vcpkg triplet (`x64-windows`), which we are not
> using. Therefore the existing ~8,300-symbol `.def` is **still required** on Windows and is still
> the source of truth for exports. This also means the ~32 non-standard asm symbols are retained
> for free (they are already in the `.def`), which is the safe choice given external consumers
> (outside this repo/build tree) cannot be audited from here and remain **unconfirmed**.

Findings from the export audit:
- `BeOpenSSL.def` / `BeOpenSSL_clang.def` export ~8,300 symbols — essentially OpenSSL's full
  public/internal surface. vcpkg/OpenSSL regenerates this set itself (via `mkdef.pl`), so the
  bulk no longer needs hand-maintenance.
- The only **non-standard** additions are ~32 low-level asm/CPU-dispatch helpers that a stock
  OpenSSL DLL does not export: `gcm_*`, `aesni_*`, `OPENSSL_ia32_cpuid`/`rdrand_bytes`/
  `rdseed_bytes`, `OPENSSL_rdtsc`, `OPENSSL_wipe_cpu`, `OPENSSL_instrument_bus`/`bus2`,
  `OPENSSL_atomic_add`.
- An on-disk search across all sibling repos (`src/*`) found **zero** consumers of those ~32
  symbols. **However, this OpenSSL build is consumed outside this repo and outside this build
  tree**, so absence on disk is not proof.

Actions:
1. Confirm with the owners of the external consumers whether any depend on those ~32 asm
   exports. The standard symbols are exported either way. *(Still unconfirmed; Option A keeps
   them regardless, so this is no longer blocking.)*
2. Under Option A the full `.def` is kept and applied at the Windows re-link step (Step 6),
   exactly as the old build did. If the external consumers are later confirmed to not need the
   ~32 asm symbols, the team may revisit switching Windows to the dynamic vcpkg triplet to drop
   `.def` maintenance entirely.

---

## Step 6 — Rewrite `BeOpenSSL.mke` (client.mke style)  ✅ DONE (pending build verification)

> **Status:** `BeOpenSSL.mke` and `BeOpenSSL.prewire.mke` rewritten. The 2,567-line file-by-file
> compile makefile is replaced by a thin vcpkg-based one:
> - Runs the shared `vcpkg_run_install.{sh,bat}` (idempotent; prewire installs first so headers
>   are staged at the PublicAPI stage, the `.mke` reuses the cached install for the libs).
> - **Static delivery** (Linux/macOS/iOS/Android + Windows-static): merges vcpkg `libssl` +
>   `libcrypto` into one `iTwinOpenSSL` archive (`libtool` on macOS/iOS, `merge_static_libs.sh`
>   on Linux/Android, `lib /OUT` on Windows), delivered under the PartFile-bound name.
> - **Dynamic delivery** (Windows only): reuses `linkLibrary.mki`/`dlmlink` with the vcpkg static
>   `.lib`s as `DLM_OBJECT_FILES` plus the full export `.def` (Option A), preserving the import
>   lib, versioning, signing, and BuildContext delivery. The `iTwinNativeThirdParty` rename and
>   the `WINDOWS_CLANG` filtered `.def` are preserved.
> - Headers/license now come from the vcpkg output, not the checked-in `vendor/` tree.
>
> **Open verification items (cannot be built here):**
> 1. **Unix symbol visibility.** The old build compiled OpenSSL with `GCC_DEFAULT_VISIBILITY=
>    hidden` to avoid clashing with Node's bundled OpenSSL (the Node 18 segfault). This must be
>    reproduced by the vcpkg static triplet (`-fvisibility=hidden`); if the stock triplet does
>    not, add a custom overlay triplet under `./triplets` (the shared install helpers already
>    pass `--overlay-triplets` when that dir exists).
> 2. **dlmlink from `.lib` inputs.** Confirm `dlmlink` accepts `.lib` files (not `.obj`) as
>    `DLM_OBJECT_FILES` and that `link.exe` pulls the exported objects per the `.def`.
> 3. **Internal header.** `crypto/engineerr.h` is no longer delivered (vcpkg ships only public
>    headers). Confirm no downstream consumer needs it.
> 4. **PartFile rebuild inputs** (Step 7): declare the vcpkg manifest/overlay files as inputs.

Replace the file-by-file compile rules with a thin makefile that mirrors
[`../crashpad/client.mke`](../crashpad/client.mke):

1. Include [`../vcpkg.mki`](../vcpkg.mki) to resolve the triplet.
2. Run `vcpkg_run_install.{sh,bat}` (the shared helpers) to build OpenSSL.
3. Deliver headers to `VendorAPI/openssl` via `~linkdir` (as crashpad does for its headers).
4. **Static platforms (Linux, macOS, iOS, Android):** merge
   vcpkg's `libcrypto`/`libssl` static archives into a single static `iTwinOpenSSL` archive
   using the platform-appropriate tool (`libtool -static` on macOS, `merge_static_libs.sh` on
   Linux — see client.mke; iOS/Android use the same archive-merge approach). No `.def`, no DLL,
   and the `BSIVer` patch is a `_MSC_VER`-guarded no-op here.
5. **Windows (dynamic):** link `iTwinOpenSSL.dll` + its import `.lib` from the static archives,
   applying the `.def` from Step 5 (if retained). The `BSIVer` section comes for free from the
   Step 3 patch baked into `libcrypto`. This is the `LibraryDynamic` part (Windows-only per the
   PartFile). Confirm whether a static `iTwinOpenSSL.lib` is also still expected on Windows (the
   PartFile lists it as `IfNotPresent="Continue"`).
6. Deliver the OpenSSL license from the vcpkg tree (crashpad delivers `share/crashpad/copyright`;
   OpenSSL is under `share/openssl/copyright`).

Follow the build-ordering lesson from the crashpad handler work: do the link/deliver steps as
ordered `always:` blocks (not dependency rules) so they run after the vcpkg install, and guard
each `~linkfile` with `-$(deleteCmd) "<target>"` so rebuilds don't fail on an existing link.

Preserve the existing platform `cDefs` that are still relevant and **not** asm-related
(e.g. `OPENSSL_NO_ASYNC` for iOS/WinRT/clang-cl, the Android `char16_t`/`char32_t` defines)
only if vcpkg's port does not already cover them; most should now be the port's responsibility.

---

## Step 7 — Update the PartFile  ✅ DONE

> **Status:** `BeOpenSSL.PartFile.xml` updated. Both parts that now invoke vcpkg — `__PublicAPI`
> (runs `BeOpenSSL.prewire.mke`) and `__Library` (runs `BeOpenSSL.mke`) — gained
> `<SubPart PartName="vcpkg" PartFile="iModelCore/libsrc/vcpkg"/>`, matching crashpad's pattern
> so the vcpkg checkout/bootstrap runs first. The per-platform split, the `__PublicAPI`/
> `__Library`/`Library`/`LibraryStatic`/`LibraryDynamic` structure, the `iTwinOpenSSL` delivery
> names, and the `OpenSslNuget` product are unchanged. Manifest-change detection follows the
> crashpad precedent: the `always:` vcpkg-install step runs every build and vcpkg's own manifest
> hashing rebuilds when `vcpkg.json` / the overlay port changes (no explicit PartFile input
> element is used for this — crashpad does not either).

In `BeOpenSSL.PartFile.xml`:
- The PartFile **already encodes the per-platform split** correctly: `LibraryDynamic` carries
  `ExcludePlatforms="Android*,iOS*,Linux*,MacOS*"` (Windows-only DLL), while `LibraryStatic`
  covers all platforms — which matches the confirmed Windows-dynamic / everything-else-static
  reality. Keep this as-is.
- Keep the `__PublicAPI` / `__Library` / `Library` / `LibraryStatic` / `LibraryDynamic`
  structure and the `iTwinOpenSSL` delivery names so downstream parts are unaffected.
- Ensure the part that runs `BeOpenSSL.mke` declares the vcpkg manifest/overlay files as
  inputs so changes trigger rebuilds.
- Confirm the `NuGetProduct` (`OpenSslNuget`) still packages the same deliverables.

---

## Step 8 — Verify

1. Build each target platform: Windows (x64), Linux (x64), macOS (arm64), iOS (arm64),
   Android (arm64 and x64). Confirm `nasm`/asm generation works on Windows.
2. Confirm the `BSIVer` section is present in the Windows binary (DumpBin) and reflects the
   vcpkg OpenSSL version.
3. Confirm the Android `by_dir.c` patch is applied (V2 checkpoint download on Android still
   works).
4. Confirm the exported symbol set matches expectations (Step 5 outcome) — especially the
   ~32 asm symbols if retained.
5. Bump the OpenSSL version once (change only `vcpkg.json` / baseline) to confirm the
   "easily pick a new version" goal, and re-confirm both patches still apply.
6. Smoke-test the external consumers that live outside this build tree.

---

## Cleanup once green

- Delete the checked-in `vendor/` OpenSSL source tree.
- Delete `generateAssemblyForWindows.pl` and the generated asm files.
- Fold the relevant guidance from `READ_BEFORE_UPDATING_CODE.md` into the overlay-port patch
  notes (the Android and HW-accel sections become "see the overlay port patches").
- Update `readme`/notes to point at this vcpkg workflow.
