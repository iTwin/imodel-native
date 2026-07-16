# Plan: Shared vcpkg binary cache for CI (Azure Blob Storage via AzCopy)

> **Status: IMPLEMENTED — CI writes to the shared cache (confirmed 2026-07-15).** This document
> describes the changes that wired a cross-agent vcpkg binary cache into the Azure Pipelines build.
> The CI **write** path is live and verified (uploads succeed); remaining items are optional (the
> developer read-only opt-in, Step 4) and operational (retention).

> **🔄 BACKEND PIVOT: `x-az-universal` → `x-azcopy`.** The original `x-az-universal` (Azure
> Artifacts Universal Packages) backend was **blocked** on the first CI build because that provider
> hard-requires **Azure CLI ≥ 2.64.0** and every CI agent shipped an older `az` (macOS 2.49.0,
> Windows 2.63.0); vcpkg then fails fatally trying to fetch its own `az` (see "Step 6" history).
> Azure admins have now **approved a dedicated Azure Blob Storage account** for the cache —
> `imodelnativevcpkg` in resource group `dev-iTwinBuildCache-rg` (subscription **BentleyConnect
> Dev**, `13cac5cb-2ec6-48c6-9a90-5a776f6aed90`, East US, RA-GRS, StorageV2).
>
> **Why `x-azcopy`, not `x-azblob`.** [**SAS tokens are not allowed**](https://learn.microsoft.com/en-us/vcpkg/reference/binarycaching#azcopy)
> in this environment, which rules out `x-azblob` **and** `x-azcopy-sas` (both are SAS-only).
> vcpkg's [`x-azcopy`](https://learn.microsoft.com/en-us/vcpkg/reference/binarycaching#azure-blob-storage-with-azcopy)
> provider uses the **AzCopy** tool, which supports **Microsoft Entra ID** (non-SAS) auth — exactly
> what's allowed here. Crucially, the `az ≥ 2.64.0` blocker was **specific to `x-az-universal`**;
> **AzCopy is a *separate* tool** that vcpkg can auto-fetch (unlike `az`, which it cannot), so the
> `az` version is irrelevant to `x-azcopy`. On CI, AzCopy authenticates via Entra ID using
> **`AZCOPY_AUTO_LOGIN_TYPE=AZCLI`**, which reuses an active `az` session — **but a build step only
> has an `az` session if it runs inside an `AzureCLI@2` task** (see "Step 2" / the 2026-07-14 CI
> finding); Azure DevOps agents are **not** persistently `az login`'d. The remaining work is:
> create a container, wrap the Build step in `AzureCLI@2` with a service connection, and grant that
> connection's service principal (and developers) the right **RBAC data role** (no SAS).
> **Done & verified (2026-07-15):** all of the above are in place — `VCPKG_BINARY_SOURCES` is set to
> the `x-azcopy` config in CI, the Build step runs inside `AzureCLI@2` (connection
> **`vcpkg-cache updater`**, SP `c598ebd4…` with `Storage Blob Data Contributor`), and a CI build
> **uploaded** zlib+minizip to the container (`Completed submission … to 1 binary cache(s)`), and a
> warm same-pins build **restores** from it (spot-checked 2026-07-15; see the "CI build times"
> table for measured per-job savings).

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

## Chosen backend: `x-azcopy` (Azure Blob Storage via AzCopy + Entra ID)

The original `x-az-universal` (Universal Packages) backend was shelved after it hit a hard
**Azure CLI ≥ 2.64.0** requirement no CI agent could satisfy (Step 6). Azure admins approved a
dedicated **Azure Blob Storage** account instead, and of the blob-capable vcpkg providers,
**`x-azcopy`** is the only one compatible with our constraints:

- **No SAS tokens** (an environment rule). This eliminates `x-azblob` and `x-azcopy-sas`, which
  both authenticate *only* with a Shared Access Signature. `x-azcopy` authenticates with the
  **AzCopy** tool, which supports **Microsoft Entra ID** (Azure AD) auth.
- **Not blocked by the old `az` version.** The 2.64.0 requirement was intrinsic to
  `x-az-universal`'s use of the `az` CLI + ArtifactTool. `x-azcopy` uses **AzCopy**, a distinct
  binary that **vcpkg can auto-fetch** (unlike `az`), so the agents' `az` version no longer
  matters for the cache path.
- **Reuses the existing CI login.** The agents already have `az` installed **and are already
  logged in**. Setting **`AZCOPY_AUTO_LOGIN_TYPE=AZCLI`** tells AzCopy to reuse that existing `az`
  session for its Entra ID token — non-interactively, and independent of the `az` version. (Other
  values like `MSI`/`SPN` exist if a managed identity or service principal is ever preferred.)
- **Recommended by vcpkg for real caches.** vcpkg's docs explicitly recommend a `x-azcopy` blob
  backend over `x-az-universal` for anything beyond a trivial number of packages.

> **AzCopy auto-fetch — verify on first run.** An earlier finding suggested `x-azcopy` may
> **auto-install `azcopy`** the way vcpkg fetches its other tools. Confirm this in Step 0: if
> vcpkg fetches AzCopy itself, no agent provisioning is needed; if not, AzCopy must be added to
> the agent image / `PATH` (it is a single self-contained binary, unlike the `az` upgrade the old
> plan required). Either way, **no `az` upgrade is required.**

### Container

Create a blob **container** (e.g. `vcpkg-cache`) in the approved storage account
`imodelnativevcpkg`. The `x-azcopy` `<baseuri>` is the blob endpoint **plus** the container path:
`https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache`. Auth is **not** in the config
string (no SAS) — it comes from AzCopy's Entra ID login — so, unlike the old `x-az-universal`
config, `VCPKG_BINARY_SOURCES` here contains **no secret** and can stay a plain, versioned
pipeline variable.

- CI (read + write):
  ```
  clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,readwrite
  ```
- Developers (read-only):
  ```
  clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,read
  ```

Access is granted by **Azure RBAC data roles** on the storage account/container, not by tokens in
the string:

- **CI build identity** → **Storage Blob Data Contributor** (read + write).
- **Developers** → **Storage Blob Data Reader** (read-only).

> **AzCopy environment.** In non-interactive CI, set `AZCOPY_AUTO_LOGIN_TYPE=AZCLI` (reuse the
> agent's `az` login). The [`azcopy env`](https://github.com/Azure/azure-storage-azcopy/wiki/azcopy_env)
> command lists other variables that affect AzCopy if tuning is needed.

---

## Step 0 — ✅ Local round-trip validation (do this first)

> **⚠️ Redo for `x-azcopy`.** The earlier round trip validated the **`upack`** backend, not
> `x-azcopy`. Re-run this Step against the blob container to validate the new plumbing:
> AzCopy **auto-fetch**, `AZCOPY_AUTO_LOGIN_TYPE=AZCLI` auth, and blob upload/restore.
>
> **Prior `upack` result (kept for the perf finding).** The `upack` cache round-tripped correctly
> (build → upload → delete local install → restore); auth/ArtifactTool/feed write all worked.
> **Perf finding (still relevant):** a build that pulls all vcpkg binaries from the shared cache
> was only *marginally* faster than a full local build, and slower than a local build with a warm
> local cache — so the shared cache is **probably not worth it for developers**. This weakens the
> read-only developer opt-in (Step 4); keep the CI `readwrite` path (the real payoff) and treat
> Step 4 as optional/low-priority.

De-risk the hard-to-debug parts (auth, ArtifactTool, feed write permission, upload/download
plumbing) on this Mac **before** touching the pipeline. This is cheap and catches most failures
early.

> **What this proves — and what it doesn't.** A local round trip (build → upload → delete local
> install → restore *on the same Mac*) validates the **plumbing and auth** only. It does **not**
> prove cross-machine ABI-hash equality, because you're restoring against the same compiler hash.
> The definitive cross-machine check is the **first real CI build** (Step 5): watch its log for
> `Restored ... from ...` (hash matched local) vs. a rebuild (hash diverged).

1. **Authenticate + point vcpkg at the container with write access:**
   ```sh
   az login                                   # AzCopy will reuse this session
   export AZCOPY_AUTO_LOGIN_TYPE=AZCLI         # AzCopy gets its Entra ID token from az
   export VCPKG_BINARY_SOURCES="clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,readwrite"
   ```
   (Your `az` identity needs **Storage Blob Data Contributor** on the container for the write
   path. If vcpkg does **not** auto-fetch AzCopy, install it and put `azcopy` on `PATH` first.)
2. **Clean build one port** (e.g. the openssl chain) so it compiles from source and **uploads**
   to the container. Confirm the log shows `vcpkg: binary-sources=clear;x-azcopy,...` and an
   upload (`Uploading` / `Stored ... to ...`), and note whether vcpkg **auto-fetched AzCopy**.
3. **Restore test:** delete the local `vcpkg_installed/` (and the local `files` archive cache) so
   nothing local can satisfy it, rebuild, and confirm the log shows
   `Restored ... from ...` instead of recompiling.
4. **Re-upload check:** rebuild the same port with **no** changes and confirm vcpkg reports the
   entry as already present and skips the upload cleanly. (Blob is *mutable*, so a re-push simply
   overwrites rather than 409-ing; vcpkg checks existence first and treats store failures as
   non-fatal, so either way it is a benign no-op — confirm it is not an error.)
5. **Artifact sanity checks** (the cross-machine portability concern):
   - Path leakage — a few hits in static-lib debug info are fine, zero is ideal:
     ```sh
     strings <install-root>/<triplet>/lib/libssl.a | grep -c "$HOME" || echo "no home paths"
     ```
   - Determinism — build the port twice into clean buildtrees and compare archive size / `nm`
     symbol lists; identical symbol sets ⇒ safe to share.
   - **Functional proof (worth more than any byte diff):** confirm `imodeljs.node` links and the
     ecobjects/ECDb tests pass against the **restored** artifacts.

> If Step 0 uploads succeed, the build service still needs its own **Storage Blob Data
> Contributor** grant for CI (Step 3) — your `az login` identity is not the pipeline's identity.

> **🔎 Finding (2026-07-14) — auto-fetch ✅, auth ✅, upload ❌ on RBAC.** First `x-azcopy` round
> trip on this Mac: vcpkg **auto-fetched AzCopy** `v10.32.3` (no provisioning needed) and
> `AZCOPY_AUTO_LOGIN_TYPE=AZCLI` authenticated (`Authenticating to destination using Azure AD`).
> Restores work (empty cache → `Restored 0 package(s)`). **Uploads fail** with
> `403 AuthorizationPermissionMismatch` on the `PUT`. Cause: the signed-in dev identity
> (`Travis.Cobbs@bentley.com`) has only **management-plane** roles on the account — **Contributor**
> (inherited from the subscription via `bc-dev-ar-admins`), Cost Management Reader, Key Vault
> Secrets Officer, Log Analytics Reader, Monitoring Contributor — and **no `Storage Blob Data *`
> role**. `Contributor` manages the account but grants **no blob data access**, so the write is
> denied. **Fix pending:** an Owner/User Access Administrator must assign **Storage Blob Data
> Contributor** at the account (or `vcpkg-cache` container) scope to the dev identity (or the
> `bc-dev-ar-admins` group). `Contributor` **cannot** self-assign roles (needs
> `Microsoft.Authorization/roleAssignments/write`) — check the account's **Eligible assignments**
> (PIM) tab for an activatable Owner/UAA, else request the grant from the account owner.

---

## Step 1 — ✅ Add the CI cache config as a plain pipeline variable (`x-azcopy`)

> **Done (2026-07-14).** `VCPKG_BINARY_SOURCES` in
> `imodel-native-internal/build/templates/pipeline.yml` now reads
> `clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,readwrite`, with a
> comment noting it is non-secret (no SAS/token) and that auth is AzCopy's Entra ID login via
> `AZCOPY_AUTO_LOGIN_TYPE=AZCLI` in the Build step (Step 2).

The config string is **not secret** — with `x-azcopy` it contains only the blob endpoint,
container, and the `readwrite` flag (no SAS, no token). Define it as a plain variable directly in
the `variables:` block of `imodel-native-internal/build/templates/pipeline.yml`
(version-controlled and reviewable in the PR), rather than as a secret variable-group entry:

```yaml
- name: VCPKG_BINARY_SOURCES
  value: clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,readwrite
```

(It could equally be a non-secret entry in a variable group, but an inline pipeline variable
keeps it visible and versioned.)

---

## Step 2 — ✅ Give the Build step an `az` session (`AzureCLI@2` wrapper)

> **Done (2026-07-15) — wired.** The Build step in
> `imodel-native-internal/build/templates/build.yml` was converted from `- powershell:` to a
> **`- task: AzureCLI@2`** with `azureSubscription: 'vcpkg-cache updater'`,
> `scriptType: $(azCliScriptType)` (per-OS, see below), `scriptLocation: inlineScript` (the
> existing PowerShell body moved verbatim into `inlineScript`), keeping the `env:` block
> (`VCPKG_BINARY_SOURCES`, `AZCOPY_AUTO_LOGIN_TYPE: AZCLI`). The task gives the step an
> authenticated `az` session as the connection's workload-identity SP
> (`c598ebd4-4084-4e64-8463-891b44dbcd8a`), so AzCopy's `AZCLI` auto-login succeeds.
>
> **History (2026-07-14).** First wired with the wrong connection `IModel-NA-EXT-BLD-2` (SP
> `c0e1d0b2…`, no data role) — CI reproduced the exact local `403 AuthorizationPermissionMismatch`
> on the `PUT` (SP authenticated but unauthorized), which confirmed the wrapper worked and isolated
> the gap to RBAC. Swapped to the purpose-built **`vcpkg-cache updater`** connection
> (2026-07-15), whose SP **already holds `Storage Blob Data Contributor`** (Step 3) — so uploads
> should now succeed (`Stored … to …`).
>
> **Per-OS `scriptType` (2026-07-14 fix).** `AzureCLI@2` requires an explicit `scriptType`, and a
> single value can't span all agents: the **Windows** agents have Windows PowerShell (`ps`) but
> **not `pwsh`** (`scriptType: pscore` failed with `Unable to locate executable file: 'pwsh'`),
> while mac/linux have `pwsh` (`pscore`). Two tiny `condition:`-gated steps set
> `azCliScriptType` (`ps` on `Agent.OS == Windows_NT`, else `pscore`) and the task uses
> `scriptType: $(azCliScriptType)` — mirroring the old `- powershell:` step's cross-OS behavior.
>
> **Required change:** run the Build step **inside an `AzureCLI@2` task** bound to an Azure
> **service connection**. That task performs the `az login` (as the connection's service
> principal) for the step's duration, giving AzCopy's `AzureCLICredential` a session. Because
> vcpkg runs *transitively* inside `bb build`, the **whole** build command must be inside the
> task. Sketch (pscore is cross-platform; move the existing PowerShell body into `inlineScript`):
>
> ```yaml
>   - task: AzureCLI@2
>     displayName: Build
>     ${{ if parameters.retryOnFailure }}:
>       retryCountOnTaskFailure: 1
>     inputs:
>       azureSubscription: 'vcpkg-cache updater'   # ARM connection (iModelTechnologies); SP has the data role (Step 3)
>       scriptType: pscore
>       scriptLocation: inlineScript
>       inlineScript: |
>         # ... existing Build PowerShell body (call_bb.py build ...) ...
>     env:
>       SIGNING_TOKEN_VALUE: $(SIGNING_TOKEN)
>       DEBUG_CONFIGURATION: ${{ parameters.DEBUG_CONFIGURATION }}
>       VCPKG_BINARY_SOURCES: $(VCPKG_BINARY_SOURCES)
>       AZCOPY_AUTO_LOGIN_TYPE: AZCLI
> ```
>
> **Blocked on infra: no reusable service connection.** The only `AzureCLI@2` usages in
> `imodel-native-internal` are in the **TilesetPublisher** pipelines
> (`iModelCore/Visualization/TilesetPublisher/pipelines/templates/`), using `azureSubscription:
> tilesetpublisher2` and `azureSubscription: fdd29357-77c2-4da7-ac67-799dfe2dc572`. Both are
> **TilesetPublisher-scoped** (the latter just fetches test config from the unrelated
> `tilesetpublisher2` blob account) and are **not** appropriate for the native build \u2014 wrong
> storage context, possibly a different Azure DevOps project, and their SPs have no reason to
> access `imodelnativevcpkg`. **Action:** a project admin must create a dedicated **ARM
> (workload-identity-federation) service connection** in the native build's Azure DevOps project,
> then grant **that connection's service principal** **Storage Blob Data Contributor** on the
> `vcpkg-cache` container (Step 3). Once its name/GUID is known, drop it into `azureSubscription:`
> and convert the `- powershell:` Build step to the `AzureCLI@2` form above. Until then, the
> wrapper can't be wired, so leave `VCPKG_BINARY_SOURCES` effectively inert (uploads will warn and
> no-op \u2014 non-fatal).

In `imodel-native-internal/build/templates/build.yml`, extend the `env:` block of the **Build**
step so vcpkg (invoked transitively by `bb build`) sees the cache config and tells AzCopy to use
Entra ID via the existing `az` session:

```yaml
    displayName: Build
    ${{ if parameters.retryOnFailure }}:
      retryCountOnTaskFailure: 1
    env:
      SIGNING_TOKEN_VALUE: $(SIGNING_TOKEN)
      DEBUG_CONFIGURATION: ${{ parameters.DEBUG_CONFIGURATION }}
      VCPKG_BINARY_SOURCES: $(VCPKG_BINARY_SOURCES)     # ← shared vcpkg cache (x-azcopy)
      AZCOPY_AUTO_LOGIN_TYPE: AZCLI                      # ← AzCopy reuses the agent's az login (Entra ID)
```

This relies on the agents **already being logged in with `az`** — which they are. The `az`
version is irrelevant here (the 2.64.0 requirement was `x-az-universal`-only), and AzCopy is
fetched by vcpkg (verify in Step 0) rather than needing the `azure-devops` extension.

---

## Step 3 — ✅ (CI) Grant the build identity write access to the container (RBAC)

> **CI done (2026-07-15); developer grant deferred.** The `vcpkg-cache updater` service
> connection's SP (`c598ebd4-…`) **already holds `Storage Blob Data Contributor`** on
> `imodelnativevcpkg`, so the CI write path is satisfied. The **per-developer** read grant is
> **not being provisioned for now** (needs Owner/UAA, and is low-value per Step 0) — see Step 4.

> **⬜ Follow-up — narrow the writer grant to the container (PR review, least privilege).** The
> existing CI grant is at **account** scope (`imodelnativevcpkg`), which confers read/write/delete
> over **every current and future container** in that account, while this integration needs only
> the single `vcpkg-cache` container. This is achievable (Azure RBAC supports container-scoped data
> roles — the container-scoped `az role assignment create` is already shown below) and **should**
> be done: create the **container-scoped** `Storage Blob Data Contributor` assignment for SP
> `c598ebd4-…`, then **remove** the account-scoped one. It's an **Owner / User Access
> Administrator** action (plain Contributor can't (re)assign roles), so it's tracked as a follow-up
> rather than done here. This matters more once the **PR-build cache-writer trust boundary** is
> tightened: limiting the writer to one container caps the blast radius of a compromised or
> untrusted PR build to the cache container alone, not the whole account. (Caveat: if a writer's
> role were **inherited** from a subscription/account assignment it couldn't be narrowed at the
> child scope — but this SP's grant is a dedicated account-scoped assignment, so it can simply be
> replaced with a container-scoped one.)

> **Open (new for `x-azcopy`).** The old `upack` **Feed Publisher** grant is irrelevant now.
> Access to the blob container is via **Azure RBAC data roles**, not feed permissions or a SAS.
> The identity the agents' `az` session resolves to (Step 0 tells you which) needs the write role.

> **🔎 CI finding (2026-07-14) — `AZCLI` has no session on a plain step.** With Steps 1–2 wired,
> the first CI upload failed **differently** from the local dev failure. AzCopy logged:
> `INFO: Authenticating to destination using Unknown ... failed to perform Auto-login:
> AzureCLICredential: ... Please run 'az login' to setup account.` I.e. `AZCOPY_AUTO_LOGIN_TYPE=AZCLI`
> found **no `az` session** — Azure DevOps agents are not persistently `az login`'d; a step only has
> an `az` context when it runs inside an **`AzureCLI@2`** task. **Fix:** wrap the Build step in
> `AzureCLI@2` (Step 2) using a service connection, and grant **that connection's service
> principal** the data role below (not the agent user). This supersedes the earlier guess that the
> `bentleycs-beconnect` SPs would automatically cover CI — they only help if the *service
> connection* is backed by one of them.

Whichever Entra ID identity AzCopy authenticates as on CI (the agent's `az login` identity — a
service principal or managed identity) must have **Storage Blob Data Contributor** (read + write)
scoped to the **`vcpkg-cache` container** (preferred, least privilege — grant it on the whole
storage account `imodelnativevcpkg` only if a container-scoped assignment isn't feasible) for the
CI `readwrite` path to upload. Developers need only **Storage Blob Data Reader** for the read-only
path. Set these in the storage account's **Access Control (IAM) → Role assignments** (ask the
storage account owner if you can't self-assign).

> **Likely already satisfied for CI (2026-07-14).** The role's **Assignments** tab shows two
> **App** service principals — `bentleycs-beconnect-13cac5cb-2ec6-48c6-9a90-5a776f6aed90` and
> `bentleycs-beconnect-BentleyConnect Dev` — already holding **Storage Blob Data Contributor** at
> **subscription** scope (inherited), plus the `BCSM-Cloud-Ops-Engineers` group. If the CI agents'
> `az` session resolves to one of those `bentleycs-beconnect` SPs (probable — they're the
> BentleyConnect Dev connection identities), CI's write path needs **no new grant**. Confirm from
> the first CI build's AzCopy log / `az account show` which principal it uses. The still-open item
> is the **per-developer** grant (and it's the dev round-trip in Step 0 that's currently blocked).

> **Note the distinction:** *Storage Blob Data Contributor/Reader* are **data-plane** roles (read/
> write blob contents). The generic *Contributor*/*Owner* management roles do **not** grant blob
> data access by themselves — the data role is what AzCopy needs. **Confirmed on 2026-07-14** (see
> Step 0 finding): the dev identity's inherited **Contributor** was *not* enough; the upload got
> `403 AuthorizationPermissionMismatch` until a `Storage Blob Data` role is added.
>
> **Who can grant it.** Creating the assignment needs `Microsoft.Authorization/roleAssignments/write`
> (**Owner** / **User Access Administrator** / **RBAC Administrator**). Plain **Contributor cannot**
> self-assign, so this likely requires the account owner (or a PIM-activated Owner/UAA):
> ```sh
> az role assignment create \
>   --assignee <objectId-or-group> \
>   --role "Storage Blob Data Contributor" \
>   --scope "/subscriptions/13cac5cb-2ec6-48c6-9a90-5a776f6aed90/resourceGroups/dev-iTwinBuildCache-rg/providers/Microsoft.Storage/storageAccounts/imodelnativevcpkg"
> ```
>
> **CI grant satisfied (2026-07-15).** The purpose-built **`vcpkg-cache updater`** service
> connection (endpoint id `c871193a-87cd-4adc-acee-0d627135f9be`, **Workload Identity Federation**,
> target sub *BentleyConnect Dev*) is backed by SP appId
> **`c598ebd4-4084-4e64-8463-891b44dbcd8a`**, which **already holds `Storage Blob Data Contributor`**
> (+ `Reader`) on `imodelnativevcpkg` — verified via `az role assignment list`. So the CI write path
> needs **no further grant**; the next build's `PUT` should return `Stored … to …`. **This grant is
> at account scope and should be narrowed to the `vcpkg-cache` container** (least privilege — see the
> follow-up note at the top of this Step): add the container-scoped assignment below, then remove
> the account-scoped one.
>
> The earlier candidate `IModel-NA-EXT-BLD-2` (SP `c0e1d0b2-…`, target *Dev Test Labs*) was the
> **wrong** connection and had no data role; it is no longer referenced. The **preferred**
> (container-scoped, least-privilege) grant command is:
> ```sh
> az role assignment create \
>   --assignee c598ebd4-4084-4e64-8463-891b44dbcd8a \
>   --role "Storage Blob Data Contributor" \
>   --scope "/subscriptions/13cac5cb-2ec6-48c6-9a90-5a776f6aed90/resourceGroups/dev-iTwinBuildCache-rg/providers/Microsoft.Storage/storageAccounts/imodelnativevcpkg/blobServices/default/containers/vcpkg-cache"
> ```
> After it's in place, delete the broader account-scoped assignment:
> ```sh
> az role assignment delete \
>   --assignee c598ebd4-4084-4e64-8463-891b44dbcd8a \
>   --role "Storage Blob Data Contributor" \
>   --scope "/subscriptions/13cac5cb-2ec6-48c6-9a90-5a776f6aed90/resourceGroups/dev-iTwinBuildCache-rg/providers/Microsoft.Storage/storageAccounts/imodelnativevcpkg"
> ```
> (Drop the `/blobServices/.../vcpkg-cache` suffix only if a container-scoped assignment isn't
> feasible.) The **per-developer** grant (your own identity, for Step 0) is the same `create`
> command with your object id — likewise scope it to the container.

**Blob lifecycle-management rule** (see "Retention" below) is **done (2026-07-15)**: blobs not
accessed in 30 days are auto-deleted, so the cache is self-pruning.

---

## Step 4 — 🚫 Developer opt-in — NOT supported for now (permissions)

> **Not being supported for now.** The read-only developer opt-in requires developers to hold
> `Storage Blob Data Reader` on the container. This need **not** be per-developer: granting the
> role once to an existing **developer Entra group** (the same way `BCSM-Cloud-Ops-Engineers`
> already holds `Storage Blob Data Contributor` on the account) would cover everyone in that group
> with a single assignment — the practical path if this is ever enabled. Either way it still needs
> **Owner / User Access Administrator** to create that one assignment (not being provisioned for
> this now). Combined with the Step 0 perf finding — the shared cache is only *marginally* faster
> than a clean build and **slower than a warm local `files` cache**, so it's **not worthwhile for
> developers** anyway — the developer path is **deferred**. CI (the real beneficiary) is
> unaffected; `VCPKG.md` is **not** being updated with a shared-cache opt-in. The instructions
> below are retained only as a reference **if** a group read grant is ever approved.

Developers need **Storage Blob Data Reader** on the container (Step 3) plus a one-time `az login`;
AzCopy then reuses that session:

```sh
# one-time: log in (AzCopy will reuse this session for its Entra ID token)
az login

# ~/.zshrc or ~/.bashrc
export AZCOPY_AUTO_LOGIN_TYPE=AZCLI
export VCPKG_BINARY_SOURCES="clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,read"
```

```powershell
# $PROFILE
$env:AZCOPY_AUTO_LOGIN_TYPE = "AZCLI"
$env:VCPKG_BINARY_SOURCES = "clear;x-azcopy,https://imodelnativevcpkg.blob.core.windows.net/vcpkg-cache,read"
```

Also add `x-azcopy` / the shared cache to the "How It Works" narrative so the fallback behavior
(local `files` cache when `VCPKG_BINARY_SOURCES` is unset) stays documented.

---

## Step 5 — ✅ Verify (uploads + restores confirmed 2026-07-15)

> **End-to-end confirmed (2026-07-15).** *Uploads:* a CI build logged `Completed submission of
> zlib:arm64-osx@1.3.2#1 … / minizip … to 1 binary cache(s)` (write path, no 403). *Restores:* warm
> same-pins builds were spot-checked and **restore** prebuilt binaries (per-package `Installing
> N/M …` handled in a few ms, no `Building …`), and the measured **Warm shared cache** times in the
> "CI build times" table below beat both the cold and local-`files` baselines on every job except
> **macOS** (which is ~flat, exactly as the Step 0 perf finding predicted).

1. **First CI build after the change** logs
   `vcpkg: binary-sources=clear;x-azcopy,...` (from
   [`vcpkg_run_install.sh`](vcpkg_run_install.sh)) and shows AzCopy being fetched/used with no
   auth error. If Step 0 already populated the container, this build is also the **cross-machine
   ABI-hash check**: `Restored ... from ...` means CI's hash matched your local Mac's; a rebuild +
   upload (`Uploading` / `Stored ...`) means the hash diverged (harmless — just no cross-machine
   reuse; investigate toolchain/CMake pinning).
2. **Second CI build** with unchanged version pins logs restores
   (`Restored N package(s) from ...`) and shows the vcpkg install steps completing in seconds.
3. **Developer local build** with the read-only env var set restores from the shared cache on
   the second build.
4. **Measure** the `x-azcopy` restore time; if it regresses, revisit container region/tiering or
   AzCopy concurrency env vars.

---

## Step 6 — ✅ RESOLVED by the `x-azcopy` pivot (history: Azure CLI version blocker)

> **Resolved.** This blocker was **specific to `x-az-universal`** and does **not** apply to the
> chosen `x-azcopy` backend: AzCopy is a separate, vcpkg-fetchable tool, and CI auth is the
> agents' existing `az` login via `AZCOPY_AUTO_LOGIN_TYPE=AZCLI` — **no `az` upgrade required**.
> The history below is kept for context; Options A/B/C are superseded.

**What happened.** The first CI build (Step 5) failed on every platform with:

```
error: error: Could not fetch az. You may be able to install this tool via your system package manager.
The following executables were considered but discarded because of the version requirement of 2.64.0:
  <path>/az: 2.63.0   # Windows
  /opt/homebrew/bin/az: 2.49.0   # macOS
```

**Root cause.** vcpkg's `x-az-universal` provider hard-requires **Azure CLI ≥ 2.64.0**. When the
system `az` is older, vcpkg attempts to *fetch its own* `az` and fails (unlike most tools, `az`
is not vcpkg-fetchable). The failure is **fatal** and happens up front during "Detecting compiler
hash," so it aborts the build entirely — it is **not** a graceful cache-miss fallback.

**Agent versions (all too old for `x-az-universal`):** macOS **2.49.0** (`/opt/homebrew/bin/az`),
Windows **2.63.0**. Step 0 passed only because the local Mac happened to have `az ≥ 2.64.0`.

**Why the pivot fixes it.** `x-azcopy` never invokes `az` for the cache transport — it uses
**AzCopy**, which vcpkg *can* auto-fetch, and which authenticates via Entra ID using the agent's
existing `az` session (`AZCOPY_AUTO_LOGIN_TYPE=AZCLI`) **regardless of the `az` version**. So the
fleet-wide `az` upgrade the options below contemplated is unnecessary.

### Historical options (superseded by the pivot)

- **A — Upgrade `az` fleet-wide to ≥ 2.64.0.** Would have touched every agent type — a
  shared-infrastructure change. Avoided by the `x-azcopy` pivot.
- **B — Shelve the shared cache.** The interim mitigation (unset in CI) that kept builds green
  until the blob backend was approved.
- **C — Conditional enablement** on agents with `az ≥ 2.64.0`. Moot — no agent qualified, and
  `x-azcopy` removes the version dependency entirely.

---

## CI build times

Measured **whole Build-job** wall-clock times (`hh:mm:ss`, each cell prefixed with the CI machine /
build id) across three scenarios, to quantify the shared cache's payoff. These are full-job times,
not just the vcpkg-install portion (see note 2), so the cache's effect is diluted by the rest of
`bb build` — yet the **Warm shared cache** column is still the fastest on every job except macOS
(≈flat there, as Step 0 predicted).

- **Cold (populates cache)** — clean build with an empty shared cache: vcpkg **compiles from
  source** and **uploads** (`Completed submission … to 1 binary cache(s)`). Upper bound.
- **Local `files` cache (other branch)** — the pre-shared-cache baseline: `VCPKG_BINARY_SOURCES`
  effectively the per-agent local `files` cache only (a branch/agent with no shared-cache hit).
- **Warm shared cache** — shared cache already populated with matching ABI hashes: vcpkg
  **restores** (`Restored N package(s) from …`). The target/best case.

| CI job | Cold (populates cache) | Local `files` cache (other branch) | Warm shared cache |
| --- | --- | --- | --- |
| Build Winx64 | imode18a80014Q7: 00:35:32 | imode18a80014QW: 00:42:26 | imode18a80014R7: 00:32:59 |
| Build Winx64 (Clang) | imode18a80014Q8: 00:35:32 | imode18a80014QZ: 00:34:17 | imode18a80014QQ: 00:27:35 |
| Build LinuxX64 | imodee307000JKY: 00:29:46 | imodee307000JLD: 00:33:00 | imodee307000JLN: 00:26:42 |
| Build MacOSARM64 | BuildMacM1Minion57: 00:16:58 | BuildMacM1Minion56: 00:15:34 | BuildMacM1Minion57: 00:15:10 |
| Build iOS XCFramework | BuildMacM1Minion71: 00:13:14 | BuildMacM1Minion54: 00:16:05 | BuildMacM1Minion56: 00:13:11 |
| Build Android Package | imode18a80014Q4: 01:01:52 | imode18a80014QQ: 01:06:58 | imode18a80014R4: 00:52:24 |

> Notes: (1) The `Build iOSARM64` and `Build AndroidARM64` stages are **skipped** in when manually running a CI pipeline; the iOS
> and Android binaries are produced by **Build iOS XCFramework** (`arm64-ios` + `x64-ios`) and
> **Build Android Package** (`arm64-android` + `x64-android`) respectively in this case. `Build itwinjs-core`
> is the TypeScript build and doesn't use the vcpkg cache. `x64-windows-static-veracode` is the
> Veracode static-analysis pipeline, not one of these stages. (2) For the fairest comparison,
> capture the **vcpkg install** portion (from `vcpkg: binary-sources=…` to `All requested
> installations completed…`) rather than the whole Build job, since the rest of `bb build` is
> unaffected by the cache. (3) A **Warm** cell that shows a *rebuild* instead of `Restored …` means
> the ABI hash diverged from the cold build (cross-machine hash mismatch) — flag it; the cache gave
> no benefit for that job.

---

## Retention

Unlike Universal Packages, Azure **Blob** entries are **mutable and deletable**, so the cache is
much easier to manage — but it still grows with every hash-changing event (dependency bump,
compiler/CMake update, port edit), so cap it automatically:

- **Unbounded growth.** Enable a blob **lifecycle-management rule** on the storage account
  (*Data management → Lifecycle management* in the portal) to delete blobs not accessed in N days.
  vcpkg's own docs call out this exact feature for capping a binary cache. **Done (2026-07-15):** a
  rule now **deletes blobs not accessed in the last 30 days**. (Last-access conditions require
  blob **last-access-time tracking** enabled on the account — confirm it's on, else the rule falls
  back to last-modified.) The **7-day soft-delete** already enabled on `imodelnativevcpkg` gives an
  undo window for accidental deletes.
- **Poisoned entry.** If a bad binary ever lands under a hash, blob lets you simply **delete that
  blob** (or overwrite it). The no-delete-rights escape hatch also still works: **rotate the ABI
  hash** (bump the port or add a comment line to the overlay triplet) and vcpkg routes to a fresh
  entry. Rare.

Re-uploading an already-present entry (no-change rebuild, or racing agents) is a harmless
overwrite on blob (no 409); vcpkg checks existence first and treats store failures as non-fatal,
so builds don't break (verify in Step 0).

## Notes / future

- Swapping to a different container/account later is a small change to the `<baseuri>` in the
  `VCPKG_BINARY_SOURCES` values (Steps 1 & 4); nothing else changes.
- If a **managed identity** on agents becomes available, switch `AZCOPY_AUTO_LOGIN_TYPE` from
  `AZCLI` to `MSI` (and assign the same *Storage Blob Data Contributor* role to the identity) with
  no other changes.
- This unblocks the full CI time savings for the vcpkg library migrations (zlib/minizip, png,
  openssl, pugixml, crashpad).

## References

- [`VCPKG.md`](VCPKG.md) — existing vcpkg setup docs
- [`vcpkg_run_install.sh`](vcpkg_run_install.sh) — install script (already respects `VCPKG_BINARY_SOURCES`)
- `imodel-native-internal/build/templates/build.yml` — Build step `env:` block (Step 2)
- `imodel-native-internal/build/templates/pipeline.yml` — `variables:` block (Step 1)
- Storage account: `imodelnativevcpkg` (RG `dev-iTwinBuildCache-rg`, sub **BentleyConnect Dev**
  `13cac5cb-2ec6-48c6-9a90-5a776f6aed90`, East US, RA-GRS, StorageV2)
- vcpkg `x-azcopy` reference: <https://learn.microsoft.com/en-us/vcpkg/reference/binarycaching#azure-blob-storage-with-azcopy>
- AzCopy auth / `AZCOPY_AUTO_LOGIN_TYPE`: <https://github.com/Azure/azure-storage-azcopy/wiki/azcopy_login>
- vcpkg binary caching reference: <https://learn.microsoft.com/en-us/vcpkg/reference/binarycaching>
