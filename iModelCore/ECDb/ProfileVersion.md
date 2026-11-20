# Changing the ECDb profile version

The ECDb profile version (`ECDb::CurrentECDbProfileVersion()` in `PublicAPI/ECDb/ECDb.h`) describes the ECDb file format. Announce an upcoming change to the iTwin.js and connector teams, and record it in [CHANGES.md](CHANGES.md).

## ECDb

- `PublicAPI/ECDb/ECDb.h`: increase the version returned by `CurrentECDbProfileVersion()`.
- `ECDb/ProfileUpgrader.h` and `.cpp`: add an upgrader for the new version.
- `ECDb/ProfileManager.cpp`: add the upgrader to `RunUpgraders()`.
- `Tests/NonPublished/ECDbTests.cpp`: adjust `CurrentECDbProfileVersion` and `NewFileECDbProfileVersion`.
- `Tests/NonPublished/FileFormatCompatibilityTests.cpp` and `Tests/NonPublished/ProfileTests.cpp`: adjust to the new version.
- Run `FileFormatCompatibilityTests.ImportSchemas` to create a new `imodel2.ecdb`, and add it as `Tests/Data/fileformatbenchmark/<profile>/imodel2.ecdb` (for example `4005` for 4.0.0.5).

## iModelPlatform

A new ECDb profile version also needs a new DgnDb profile version.

- `PublicAPI/DgnPlatform/DgnDb.h`: increment `DGNDB_CURRENT_VERSION_*`.
- `Tests/DgnProject/Compatibility/ECDbCompatibilityTests.cpp`, `IModelCompatibilityTests.cpp` and `TestIModelCreators.cpp`: adjust.

## ECDb system schemas

If the change touches `ECDbMap`, `ECDbMeta` or another schema in `ECDb/ECSchemas/`:

- Add the new schema version as a new file next to the existing ones. Older versions stay.
- Update the file names in `ECDb.prewire.mke` and `ECDb/ECDb.mke`.
- Add the released schema to [bis-schemas](https://github.com/iTwin/bis-schemas) (`System/ECDb/Released/`), following that repository's README.
- Some tests use hard-coded class ids and may need adjusting.

## Validation

- `build/profile-version-validation.yml` runs the Bentley-internal profileversion-tests suite on pull requests. If the change needs an itwinjs-core change as well, link that pull request in the imodel-native PR description as `itwinjs-core: https://github.com/iTwin/itwinjs-core/pull/<number>`. To run those tests locally or add new ones, follow the profileversion-tests README. After merging, update `CurrentProfileVersion.txt` in that repository.
- Run the iModel evolution tests before merging (see [README](../../README.md#imodel-compatibility)). They need `NuGetFeed_Address` set to the internal NuGet feed:

  ```sh
  bb -s "imodel-native-internal:build/strategies/iModelEvolutionTests" -r imodel-native -f iModelCore/iModelPlatform/iModelPlatform -p RunIModelEvolutionTests build
  ```

  Fix any failures in the same PR.
- After merging, new evolution test packages are published through the `iModelEvolutionTests_Publish_Nugets` pipeline in imodel-native-internal, so later changes are tested against files of the new version.
