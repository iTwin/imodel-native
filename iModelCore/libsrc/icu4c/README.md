# Updating ICU4C

Follow the repository's [third-party library update workflow](../../../.github/skills/update-third-party-lib/SKILL.md) when updating ICU4C. Obtain the source and data archives for the same release from the [ICU releases page](https://github.com/unicode-org/icu/releases), then copy the source archive followed by the data archive into `vendor`.

Whenever ICU4C is updated, update [IcuNugetLicense.json](IcuNugetLicense.json) in the same change. Verify the upstream version, SPDX license identifier, and version-pinned license URL.

## Bentley Integration

1. Update `BeIcu4cLibrary.Compiland.mki` for source files added or removed by the release.
2. Update `DataFileBaseName` in `BeIcu4cCommon.mki`.
3. Remove the release's `vendor/source/data/in/icudt##l.dat`; the reduced data file must be generated locally.
4. Build the reduced data file from an initialized BentleyBuild environment:

   ```text
   bb -f iModelCore/libsrc/icu4c/BeIcu4cData -p BeIcu4cData build --tmrbuild --noprompt
   ```

   The environment may require `;buildall` in `env.dat`.
5. Copy the generated `.dat` file from the output directory into this directory. Update `ExistingDataDir` in `BeIcu4cData.mke`, then rerun the data build to verify that the committed data file works.
6. Copy the generated `.dat` file into `vendor/source/data/in/` and update `BeIcu4cLibrary.PartFile.xml` to reference its new name.
7. Apply any release-specific language-standard or data-file changes required by the upstream release.
