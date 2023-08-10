# Updating code guideline

## Android patch
When updating code make sure to that following change is not overridden. It is required to ensure V2 checkpoint can be downloaded on android.

File [by_dir.c](./vendor/crypto/x509/by_dir.c) as following [patch](https://github.com/iTwin/imodel-native/pull/315/commits/b6c95911cc66756f9572c875f59ed45eaa3cc053)
`````
--- a/iModelCore/libsrc/openssl/vendor/crypto/x509/by_dir.c
+++ b/iModelCore/libsrc/openssl/vendor/crypto/x509/by_dir.c
@@ -261,7 +261,13 @@ static int get_cert_by_subject_ex(X509_LOOKUP *xl, X509_LOOKUP_TYPE type,
     }
 
     ctx = (BY_DIR *)xl->method_data;
+
+#if defined(__ANDROID__)
+    h = X509_NAME_hash_old(name);
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