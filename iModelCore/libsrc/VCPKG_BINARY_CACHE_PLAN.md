# Plan: Shared vcpkg binary cache for CI (Azure Artifacts Universal Packages)

> **Status: PLAN (not yet implemented).** This document describes the changes to wire a
> cross-agent vcpkg binary cache into the Azure Pipelines build. Update each step with a
> *Done* banner as it lands.

## Goal

Give CI a shared vcpkg binary cache so that PR builds whose vcpkg version pins haven't changed
**restore** prebuilt binaries in seconds instead of recompiling from source. OpenSSL alone is
~1,200 translation units across 6 triplets (~9 min/clean build); with a warm cache that work
collapses to a download. Developers get read-only access so their second build restores from the
same cache.

The install scripts already respect this: [`vcpkg_run_install.sh`](vcpkg_run_install.sh) /
[`vcpkg_run_install.bat`](vcpkg_run_install.bat) only fall back to the **local** file cache when
`VCPKG_BINARY_SOURCES` is empty. Wiring a shared cache is a pipeline-config + docs change with
**no** modification to the install scripts themselves.

## Chosen backend: `x-az-universal` (Azure Artifacts Universal Packages)

Azure Blob Storage (the original proposal's `x-azblob`) was rejected by the Azure admins.
Of the allowed alternatives, **Azure Artifacts Universal Packages** is the best fit:

- **Permissions already granted** — read/write to upack storage on Azure DevOps, including
  creating packages/versions.
- **No new runtime dependency.** BentleyBuild already *requires and version-checks* the Azure
  CLI plus the `azure-devops` extension, and already uses the standalone **ArtifactTool** binary
  on every platform (see `bblib/azurecli.py` `GetArtifactToolPath` / `GetPATForArtifactTool` and
  `bblib/universalpkg.py`). vcpkg's `x-az-universal` provider drives exactly that same tooling.
  By contrast, vcpkg's `nuget` provider would introduce a brand-new **mono** dependency on all
  non-Windows agents — a dependency BentleyBuild has deliberately avoided everywhere (it
  downloads NuGet packages via pure-Python HTTP against the V2 feed, never `nuget.exe`).
- **CI auth is the established pattern** — `AZURE_DEVOPS_EXT_PAT: $(System.AccessToken)` is
  already used elsewhere in the pipeline (e.g. `imodel-native-internal/build/templates/pipeline.yml`
  Pull step).
- **Identical behavior on all six triplets** and agent types.

### Feed

Feed creation is **not** available under the current account, so the shared **`upack`** feed is
used for now. This feed is **organization-scoped**, so the project field in the config string
must be left **empty** (the double comma below). Supplying a project makes vcpkg pass
`--scope project --project ...` to ArtifactTool, which fails with
`TF1600011: The feed with ID 'upack' doesn't exist` — the error means "no *project-scoped* feed
by that name," not "no feed at all." If a dedicated feed is later approved, only the feed name
(and possibly the scope) in the config string changes — nothing else.

- CI (read + write):
  ```
  clear;x-az-universal,https://dev.azure.com/bentleycs,,upack,readwrite
  ```
- Developers (read-only):
  ```
  clear;x-az-universal,https://dev.azure.com/bentleycs,,upack,read
  ```

> **Known caveat.** vcpkg's docs warn that `x-az-universal` can be slow with *large* numbers of
> binary packages and recommend a blob backend when possible. Our package count is small (a
> handful of ports × 6 triplets, keyed by ABI hash), so this is very likely fine — **measure it
> on the first warm build** and revisit if restore time regresses.

---

## Step 0 — ✅ Local round-trip validation (do this first)

> **Done.** The `upack` cache round-trips correctly (build → upload → delete local install →
> restore). Auth, ArtifactTool, and feed write permission all work. **Perf finding:** a build that
> pulls all vcpkg binaries from `upack` is only *marginally* faster than a full local build, and
> slower than a local build with a warm local cache — so the shared cache is **probably not worth
> it for developers**. This weakens the value of the read-only developer opt-in (Step 4); keep the
> CI `readwrite` path (the real payoff) and treat Step 4 as optional/low-priority.

De-risk the hard-to-debug parts (auth, ArtifactTool, feed write permission, upload/download
plumbing) on this Mac **before** touching the pipeline. This is cheap and catches most failures
early.

> **What this proves — and what it doesn't.** A local round trip (build → upload → delete local
> install → restore *on the same Mac*) validates the **plumbing and auth** only. It does **not**
> prove cross-machine ABI-hash equality, because you're restoring against the same compiler hash.
> The definitive cross-machine check is the **first real CI build** (Step 5): watch its log for
> `Restored ... from ...` (hash matched local) vs. a rebuild (hash diverged).

1. **Authenticate + point vcpkg at the feed with write access:**
   ```sh
   az login
   az extension add --name azure-devops   # if not already installed
   export AZURE_DEVOPS_EXT_PAT="$(az account get-access-token --resource 499b84ac-1321-427f-aa17-267ca6975798 --query accessToken -o tsv)"
   export VCPKG_BINARY_SOURCES="clear;x-az-universal,https://dev.azure.com/bentleycs,,upack,readwrite"
   ```
2. **Clean build one port** (e.g. the openssl chain) so it compiles from source and **uploads**
   to the feed. Confirm the log shows `vcpkg: binary-sources=clear;x-az-universal,...` and an
   upload (`Uploading` / `Stored ... to ...`).
3. **Restore test:** delete the local `vcpkg_installed/` (and the local `files` archive cache) so
   nothing local can satisfy it, rebuild, and confirm the log shows
   `Restored ... from ...` instead of recompiling.
4. **Re-upload / immutability check:** rebuild the same port with **no** changes and confirm
   vcpkg reports the entry as already present and skips the upload cleanly (Universal Packages
   are immutable, so a re-push would 409). vcpkg checks existence before writing and treats store
   failures as non-fatal, so this should be a benign skip — confirm it is not an error.
5. **Artifact sanity checks** (the cross-machine portability concern):
   - Path leakage — a few hits in static-lib debug info are fine, zero is ideal:
     ```sh
     strings <install-root>/<triplet>/lib/libssl.a | grep -c "$HOME" || echo "no home paths"
     ```
   - Determinism — build the port twice into clean buildtrees and compare archive size / `nm`
     symbol lists; identical symbol sets ⇒ safe to share.
   - **Functional proof (worth more than any byte diff):** confirm `imodeljs.node` links and the
     ecobjects/ECDb tests pass against the **restored** artifacts.

> If Step 0 uploads succeed, the build service still needs its own write grant for CI (Step 3) —
> your `az login` identity is not the pipeline's identity.

---

## Step 1 — ✅ Add the CI cache config as a plain pipeline variable

> **Done.** Added `VCPKG_BINARY_SOURCES` to the `variables:` block of
> `imodel-native-internal/build/templates/pipeline.yml` (just after `SIGNING_TOKEN`), with a
> comment noting it is non-secret and that auth is the separate `AZURE_DEVOPS_EXT_PAT` token.

The config string is **not secret** — it contains only the org URL, project, feed name, and the
`readwrite` flag. The actual credential is separate (the `AZURE_DEVOPS_EXT_PAT` token in Step 2).
So define it as a plain variable directly in the `variables:` block of
`imodel-native-internal/build/templates/pipeline.yml` (version-controlled and reviewable in the
PR), rather than as a secret variable-group entry:

```yaml
- name: VCPKG_BINARY_SOURCES
  value: clear;x-az-universal,https://dev.azure.com/bentleycs,,upack,readwrite
```

(It could equally be a non-secret entry in a variable group, but an inline pipeline variable
keeps it visible and versioned.)

---

## Step 2 — ✅ Expose the cache config + auth to the Build step

> **Done.** Extended the **Build** step's `env:` block in
> `imodel-native-internal/build/templates/build.yml` with `VCPKG_BINARY_SOURCES:
> $(VCPKG_BINARY_SOURCES)` and `AZURE_DEVOPS_EXT_PAT: $(System.AccessToken)`.

In `imodel-native-internal/build/templates/build.yml`, extend the `env:` block of the **Build**
step so vcpkg (invoked transitively by `bb build`) sees both the cache config and a
non-interactive Azure DevOps token:

```yaml
    displayName: Build
    ${{ if parameters.retryOnFailure }}:
      retryCountOnTaskFailure: 1
    env:
      SIGNING_TOKEN_VALUE: $(SIGNING_TOKEN)
      DEBUG_CONFIGURATION: ${{ parameters.DEBUG_CONFIGURATION }}
      VCPKG_BINARY_SOURCES: $(VCPKG_BINARY_SOURCES)     # ← shared vcpkg cache
      AZURE_DEVOPS_EXT_PAT: $(System.AccessToken)       # ← auth for x-az-universal / ArtifactTool
```

`AZURE_DEVOPS_EXT_PAT` is the token variable ArtifactTool and the `azure-devops` extension use;
`$(System.AccessToken)` is the build service identity (same pattern already used by the Pull
step in `pipeline.yml`).

---

## Step 3 — ✅ Grant the build service write access to the feed

> **Already satisfied — no action needed for write access.** Both "Limit job authorization scope
> to current project" switches (release **and** non-release pipelines) are **off**, so
> `$(System.AccessToken)` resolves to the **collection-scoped** identity
> **`Project Collection Build Service (bentleycs)`**. The `upack` feed's Permissions already grant
> that identity **Feed Publisher (Contributor)** (read + write), so the CI `readwrite` path can
> upload cache entries as-is. (Everyone's existing read access comes from
> `[bentleycs]\Project Collection Valid Users` → **Feed Reader**.) The first CI build (Step 5) is
> the final confirmation — watch for `Uploading` / `Stored … to …` with no 403.
>
> **Still open — retention policy.** Only the write grant is confirmed. Ask the feed owner
> (**Matthew Kelly**) to enable a **retention policy** (see "Retention & immutability" below);
> Universal Packages can't be manually deleted, so without retention the cache grows unbounded.

The build service identity that runs the pipeline (e.g. *"<Project> Build Service (<org>)"*)
must have **Feed Contributor** (read + write) on the `upack` feed for the CI `readwrite` path to
upload cache entries. Set this in the feed's **Settings → Permissions**. Developers only need
the default **Reader** role for the read-only path.

**Also ask the feed owner to enable a retention policy** (see "Retention & immutability" below) —
same owner, same conversation.

---

## Step 4 — ✅ Document the developer opt-in in `VCPKG.md`

> **Done.** Added a **"Shared binary cache (optional)"** section to [`VCPKG.md`](VCPKG.md) with the
> `az login` + read-only `VCPKG_BINARY_SOURCES` opt-in (shell + PowerShell), updated the intro
> caching paragraph to point at it, and added a **binary cache resolution** bullet to "How It
> Works" (unset → local `files` cache; set → shared feed; `clear` → no cache). Per the Step 0
> perf finding, the section **emphasizes it is not worthwhile for macOS developers** (a clean mac
> build is already the fastest platform and the shared cache is only marginally faster than — and
> slower than a warm-local — build), notes it may help more on **other OSes**, and frames CI as
> the real beneficiary.

Add a section to [`VCPKG.md`](VCPKG.md) telling developers how to enable read-only cache
restores. Unlike a blob SAS token, Universal Packages require per-developer auth, so the
instructions are "log in once, then set the env var":

```sh
# one-time: authenticate az to Azure DevOps (BentleyBuild already requires the az CLI + azure-devops extension)
az login
az extension add --name azure-devops   # if not already installed

# ~/.zshrc or ~/.bashrc
export VCPKG_BINARY_SOURCES="clear;x-az-universal,https://dev.azure.com/bentleycs,,upack,read"
```

```powershell
# $PROFILE
$env:VCPKG_BINARY_SOURCES = "clear;x-az-universal,https://dev.azure.com/bentleycs,,upack,read"
```

Also add `x-az-universal` / the shared cache to the "How It Works" narrative so the fallback
behavior (local `files` cache when `VCPKG_BINARY_SOURCES` is unset) stays documented.

---

## Step 5 — ⬜ Verify

1. **First CI build after the change** logs
   `vcpkg: binary-sources=clear;x-az-universal,...` (from
   [`vcpkg_run_install.sh`](vcpkg_run_install.sh)). If Step 0 already populated the feed, this
   build is also the **cross-machine ABI-hash check**: `Restored ... from ...` means CI's hash
   matched your local Mac's; a rebuild + upload (`Uploading` / `Stored ...`) means the hash
   diverged (harmless — just no cross-machine reuse; investigate toolchain/CMake pinning).
2. **Second CI build** with unchanged version pins logs restores
   (`Restored N package(s) from ...`) and shows the vcpkg install steps completing in seconds.
3. **Developer local build** with the read-only env var set restores from the shared cache on
   the second build.
4. **Measure** the `x-az-universal` restore time against the known caveat; if it regresses,
   reconsider a dedicated feed or blob (if ever unblocked).

---

## Retention & immutability

Azure Artifacts Universal Packages are **immutable and cannot be manually deleted** under the
current account. This is compatible with vcpkg caching — the cache is append-only (vcpkg only
reads or writes *new* ABI-hash entries, never overwrites) — but it has two operational
consequences:

- **Unbounded growth.** Every hash-changing event (dependency bump, compiler/CMake update, port
  edit) adds entries that never age out on their own. The clean fix is a **feed retention
  policy** ("delete older versions" / "delete after N days"), applied by the *system* so it prunes
  without anyone needing delete rights. Ask the feed **owner/admin** to enable it (Step 3).
  Retention won't purge versions downloaded within the protection window — hot entries stay.
- **Can't evict a poisoned entry.** If a bad binary ever lands under a hash, you can't delete it.
  The escape hatch needs no delete rights: **rotate the ABI hash** (bump the port or add a comment
  line to the overlay triplet) and vcpkg routes to a fresh entry, ignoring the bad one. Rare.

Re-uploading an already-present entry (no-change rebuild, or racing agents) hits a 409; vcpkg
checks existence first and treats store failures as non-fatal, so builds don't break (verify in
Step 0).

## Notes / future

- Swapping to a dedicated `vcpkg-cache` feed later is a **one-word** change to the feed name in
  the `VCPKG_BINARY_SOURCES` values (Steps 1 & 4); nothing else changes.
- If managed identity on agents becomes available, `AZURE_DEVOPS_EXT_PAT` can be replaced with
  no other changes.
- This unblocks the full CI time savings for the vcpkg library migrations (zlib/minizip, png,
  openssl, pugixml, crashpad).

## References

- [`VCPKG.md`](VCPKG.md) — existing vcpkg setup docs
- [`vcpkg_run_install.sh`](vcpkg_run_install.sh) — install script (already respects `VCPKG_BINARY_SOURCES`)
- `imodel-native-internal/build/templates/build.yml` — Build step `env:` block (Step 2)
- `imodel-native-internal/build/templates/pipeline.yml` — variable groups + existing
  `AZURE_DEVOPS_EXT_PAT: $(System.AccessToken)` pattern
- BentleyBuild `bblib/azurecli.py`, `bblib/universalpkg.py` — existing az CLI / ArtifactTool /
  Universal Packages usage that `x-az-universal` reuses
- vcpkg binary caching reference: <https://learn.microsoft.com/en-us/vcpkg/reference/binarycaching>
