# Updating libjpeg-turbo

Before updating libjpeg-turbo, read the [Bentley Integration](#bentley-integration) section below. Then follow the repository's [third-party library update workflow](../../../.github/skills/update-third-party-lib/SKILL.md), applying the Bentley-specific instructions at the relevant stage.

Whenever libjpeg-turbo is updated, update [JpegTurboNugetLicense.json](JpegTurboNugetLicense.json) in the same change. Verify the upstream version, SPDX license identifier, and version-pinned license URL.

## Bentley Integration

The upstream source layout changed in libjpeg-turbo 3.1.2. Source files now live under `src`, while this integration retains generated headers and `.in` files in specific locations.

1. Merge the vendor update into `libsrc-Main` and resolve conflicts while retaining Bentley changes, including includes that refer to `.in` files.
2. In a separate configured copy of libjpeg-turbo, run `make Configure` with Windows configuration. This generates `jconfig.h`, `jversion.h`, and `jconfigint.h`.
3. Copy only the generated `jversion.h` to `vendor/jversion.h`.
4. Ensure `jconfig.h`, `jversion.h`, and `jconfigint.h` are copied from `vendor/` into `vendor/src/`; keep the originals in place.
5. Copy the generated contents of `jconfig.h` and `jconfigint.h` into their corresponding `.h.in` files under `vendor/src/`.
6. Update the Bentley build files for upstream source-file additions, removals, and any newly required symbols, then build all supported configurations.
7. After a successful build, return to the [third-party library update workflow](../../../.github/skills/update-third-party-lib/SKILL.md) and complete its remaining steps.
