# Updating code guideline

## Status: retained as a historical record (does not apply as-is)

These instructions predate the vcpkg migration of OpenSSL. OpenSSL is no longer checked in
under `iModelCore/libsrc/openssl/vendor/`; it is built by vcpkg from a pinned upstream source
that is downloaded and extracted at build time. Consequently, the `vendor/...` paths, the inline
diffs, and the "re-create this patch against the checked-in vendor tree" guidance in the
sections below **no longer apply literally**. They are kept as a record of *why* each
modification exists so the intent survives future OpenSSL version bumps.

Under vcpkg, required source modifications live as overlay-port patch files under
`iModelCore/libsrc/openssl/overlay-ports/openssl/` and are applied on top of the pinned
upstream. Editing any overlay file invalidates vcpkg's ABI/binary cache and forces an OpenSSL
rebuild.

### Lessons from this file and how they were applied to the vcpkg build

1. **Android certificate-hash crash (`by_dir.c`).** The Android-only use of
   `X509_NAME_hash_old()` (needed so V2 checkpoints download) leaves the `i` out-parameter
   uninitialized, so the following `if (i == 0)` check runs on garbage and intermittently
   SIGSEGVs during TLS certificate verification. Fix: initialize `i = 1;`. **Applied** as the
   overlay patch
   [overlay-ports/openssl/android-by_dir.patch](./overlay-ports/openssl/android-by_dir.patch),
   guarded by `#if defined(__ANDROID__)`, targeting `crypto/x509/by_dir.c`. The
   `X509_NAME_hash_old()` switch **and** the `i = 1;` line must be preserved together — the
   latter was dropped once during the migration and reintroduced the crash (see PR #986).

2. **Hardware acceleration / assembly.** The old vendor build disabled assembly
   (`OPENSSL_NO_ASM`) and then hand-added specific `.asm/.s` files to re-enable acceleration.
   **No longer needed:** the vcpkg OpenSSL port builds upstream's standard,
   architecture-specific assembly by default (perlasm-generated per triplet), so we do not
   manage individual asm files. Do not re-introduce `OPENSSL_NO_ASM`.

3. **Root cause of "patches keep getting lost."** The underlying problem this file warns about
   was that vendor code was checked in on the Bentley branch (merged with upstream) rather than
   isolated, making Bentley modifications easy to overwrite on an update. **Addressed** by the
   overlay-port model: our modifications are now discrete patch files applied over a pinned,
   unmodified upstream (version-pinned in `vcpkg.json`), kept separate from the source they
   patch.

# Original information follows

## Android patch
When updating code make sure to that following change is not overridden. It is required to ensure V2 checkpoint can be downloaded on Android.

The `i = 1;` line is REQUIRED: `X509_NAME_hash_old()` does not set the `i` out-parameter
(unlike `X509_NAME_hash_ex()`), so without it the subsequent `if (i == 0)` check reads an
uninitialized variable and intermittently crashes during TLS certificate verification.
This was originally fixed in [PR #986](https://github.com/iTwin/imodel-native/pull/986);
do not drop it when re-creating this patch.

File [by_dir.c](./vendor/crypto/x509/by_dir.c) as following [patch](https://github.com/iTwin/imodel-native/pull/315/commits/b6c95911cc66756f9572c875f59ed45eaa3cc053)
`````
--- a/iModelCore/libsrc/openssl/vendor/crypto/x509/by_dir.c
+++ b/iModelCore/libsrc/openssl/vendor/crypto/x509/by_dir.c
@@ -261,7 +261,14 @@ static int get_cert_by_subject_ex(X509_LOOKUP *xl, X509_LOOKUP_TYPE type,
     }
 
     ctx = (BY_DIR *)xl->method_data;
+
+#if defined(__ANDROID__)
+    h = X509_NAME_hash_old(name);
+    i = 1; // X509_NAME_hash_old() does not set i; assume ok (PR #986)
+#else
     h = X509_NAME_hash_ex(name, libctx, propq, &i);
+#endif
+
     if (i == 0)
         goto finish;
     for (i = 0; i < sk_BY_DIR_ENTRY_num(ctx->dirs); i++) {
`````
## Hardware Acceleration
The following changes in VENDOR have been made to support hardware acceleration in OpenSSL. Make sure that the following files / changes are not removed or overridden.
````` 
vendor\include\openssl\opensslconf.h
--- a/vendor/include/openssl/opensslconf.h	Thu Sep 23 13:46:42 2021 -0400
+++ b/vendor/include/openssl/opensslconf.h	Fri Nov 12 09:56:54 2021 -0500
@@ -45,9 +45,6 @@
-#ifndef OPENSSL_NO_ASM
-# define OPENSSL_NO_ASM
-#endif
NEW FILES ADDED TO VENDOR:
 vendor/crypto/aes/asm/aesni-x86_64.asm 
 vendor/crypto/aes/asm/aesni-x86_64.s 
 vendor/crypto/modes/asm/aesni-gcm-x86_64.asm 
 vendor/crypto/modes/asm/aesni-gcm-x86_64.s 
 vendor/crypto/modes/asm/ghash-x86_64.asm 
 vendor/crypto/modes/asm/ghash-x86_64.s 
 vendor/crypto/x86_64cpuid.s 
 vendor/crypto/x86_64cpuid.asm
 vendor/crypto/arm64cpuid.S
`````

This potential loss is caused by the vendor code being checked in on the Bentley branch rather than the Vendor branch and merged
with any Bentley code. It will take some effort to sort this out so skipping just now.