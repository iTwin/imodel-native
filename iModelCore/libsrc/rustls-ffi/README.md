# rustls-ffi

This directory contains the [rustls-ffi](https://github.com/rustls/rustls-ffi) library, which provides C FFI bindings to [rustls](https://github.com/rustls/rustls). It is used as the TLS backend for curl on macOS and iOS, replacing OpenSSL.

## Directory Structure

- `rustls.mke` / `rustls.PartFile.xml` — BentleyBuild files that invoke `cargo capi install`
- `cargo.sh` — Wrapper script to call `~/.cargo/bin/cargo`
- `install-rust.sh` — Installs rustup, cargo-c, and the iOS target (if on macOS)
- `vendor/` — Cargo workspace root
  - `librustls/` — The rustls-ffi crate (FFI bindings)
  - `tools/` — Developer utilities (not used in production builds)
  - `rustls-platform-verifier-patched/` — Patched copy of `rustls-platform-verifier` (see below)
  - `Cargo.toml` — Workspace config with `[patch.crates-io]` pointing to the patched verifier
  - `Cargo.lock` — Pinned dependency versions

## Platform Certificate Verification

On macOS/iOS, we use the platform certificate verifier (`rustls-platform-verifier`) so that TLS certificate validation goes through the OS trust store (Keychain / Security framework). This avoids the need to ship a `cacert.pem` bundle.

The platform verifier is activated at runtime when curl is configured with `CURLSSLOPT_NATIVE_CA` (see `BeSQLiteCurlHandleConfig.cpp`).

### Patched `rustls-platform-verifier`

We maintain a patched copy of `rustls-platform-verifier` in `vendor/rustls-platform-verifier-patched/`. The patch disables network fetch during certificate evaluation on macOS (`set_network_fetch_allowed(false)`) to prevent the Security framework from spawning background dispatch threads for OCSP/CRL lookups. Stapled OCSP responses are still checked.

The patch is applied via a `[patch.crates-io]` section in `vendor/Cargo.toml`:

```toml
[patch.crates-io]
rustls-platform-verifier = { path = "rustls-platform-verifier-patched" }
```

## Updating rustls-ffi

To update rustls-ffi to a new upstream version:

1. **Check the current version** in `vendor/librustls/Cargo.toml` (look at the `[package]` section).

2. **Download the new release** from https://github.com/rustls/rustls-ffi/releases.

3. **Replace the `vendor/` contents** with the new release, preserving:
   - `vendor/rustls-platform-verifier-patched/` (our patched crate — do NOT delete)
   - The `[patch.crates-io]` section at the bottom of `vendor/Cargo.toml` (re-add if overwritten)

4. **Update `vendor/Cargo.toml`**: Ensure the `[patch.crates-io]` section is present at the bottom:
   ```toml
   [patch.crates-io]
   rustls-platform-verifier = { path = "rustls-platform-verifier-patched" }
   ```

5. **Regenerate `Cargo.lock`**:
   ```sh
   cd vendor
   ~/.cargo/bin/cargo update
   ```

6. **Verify** the patched crate is used: in `vendor/Cargo.lock`, the `rustls-platform-verifier` entry should have NO `source = "registry+..."` line (local path sources omit this field).

7. **Build and test** via the normal BentleyBuild process.

## Updating `rustls-platform-verifier-patched`

When the upstream `rustls-platform-verifier` version required by rustls-ffi changes (check `vendor/Cargo.toml` workspace dependencies for the `rustls-platform-verifier` version), you must update the patched copy:

1. **Find the required version** in `vendor/Cargo.toml` under `[workspace.dependencies]`:
   ```toml
   rustls-platform-verifier = "0.6"
   ```

2. **Download the new upstream source**. Either:
   - Run `cargo download rustls-platform-verifier==<version>` (if you have `cargo-download`), or
   - Find it in `~/.cargo/registry/src/index.crates.io-*/rustls-platform-verifier-<version>/` after a `cargo update`, or
   - Download from https://crates.io/crates/rustls-platform-verifier

3. **Replace `vendor/rustls-platform-verifier-patched/`** with the new upstream source.

4. **Remove `.cargo-checksum.json`** if present (Cargo rejects patched crates with checksum files):
   ```sh
   rm vendor/rustls-platform-verifier-patched/.cargo-checksum.json
   ```

5. **Re-apply the patch** in `vendor/rustls-platform-verifier-patched/src/verification/apple.rs`. Find the `evaluate_with_error()` call and add the `set_network_fetch_allowed(false)` call immediately before it:
   ```rust
   // Disable network fetch to prevent the Security framework from spawning
   // background dispatch threads (for OCSP, CRL, or intermediate CA downloads)
   // that persist after evaluation and prevent clean process exit.
   // Stapled OCSP responses provided above are still checked.
   trust_evaluation
       .set_network_fetch_allowed(false)
       .map_err(|e| invalid_certificate(e.to_string()))?;

   let trust_error = match trust_evaluation.evaluate_with_error() {
   ```

6. **Verify the patched crate's `Cargo.toml`** has the correct package name (`rustls-platform-verifier`) and a version compatible with what the workspace requires.

7. **Regenerate `Cargo.lock`**:
   ```sh
   cd vendor
   ~/.cargo/bin/cargo update -p rustls-platform-verifier
   ```

8. **Verify** the lock file uses the patched source (no `source = "registry+..."` line on the `rustls-platform-verifier` entry).

9. **Build and test** via the normal BentleyBuild process.
