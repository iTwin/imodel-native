# imodel-native agent notes

[README.md](README.md) is the reference for architecture, modules, building with BentleyBuild, running tests, writing SQL, and the Node addon. Read the section relevant to your task before changing code. This file adds pitfalls that are easy to miss.

## Building and testing

- Builds need a BentleyBuild (`bb`) environment with `imodel-native-internal` checked out beside this repository. Without one, do not try to compile with other tools; `compile_flags.txt` only serves editors.
- Full builds take long. Build and run only the affected test product, as shown in the README (`bb -p ECDb-Gtest ...`, then `--gtest_filter`).
- A `--gtest_filter` replaces the suite's ignore list, so a filtered run can include tests that a normal run skips.
- Tests ignore `logging.config.xml`. To see ECDb logs in a test, add this temporarily and remove it before committing; logging stays on for every test that runs after it:
  ```cpp
  NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
  NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
  ```
- Skills for the build system, vcpkg and third-party library updates are in `.github/skills/`. Procedures such as changing the ECSQL grammar, bumping the ECDb profile version or updating SQLite are documented next to the code; the README's "Further documentation" section lists them.
- To see the SQLite SQL that an ECSQL statement turns into, use `.parse` in iModelConsole (`iModelCore/iModelPlatform/iModelConsole/README.md`).

## Code conventions

- Existing code uses Bentley types such as `Utf8String` (derives from `std::string`), `Utf8CP` (`char const*`), `bvector` (alias of `std::vector`) and pointer typedefs (`FooP`, `FooCP`, `FooR`, `FooCR`). Match the surrounding code; standard types work alongside them.
- Traps when mixing Bentley and standard types: `Utf8String(nullptr)` is an empty string while `std::string(nullptr)` is undefined behavior; `Nullable<T>` always default-constructs `T`; on Windows, `std::filesystem::path::string()` converts through the ANSI code page, so use `u8string()`.
- The build treats warnings as errors.
- Upper-case both letters of EC in names: `ECClass`, `ECDb`, never `EcClass`.
- ECDb runs concurrent queries on a thread pool. Code reachable from queries must not rely on shared mutable static state.
- Run DDL (`CREATE`, `ALTER`, `DROP`, triggers) through `Db::ExecuteDdl`, not `ExecuteSql`. Only `ExecuteDdl` records the change for changesets when change tracking is active.
- New tests that embed ECSchema XML use EC 3.2 (`http://www.bentley.com/schemas/Bentley.ECXML.3.2`). Existing tests may keep older versions.

## Domain pitfalls

- ECSQL names classes and properties, not tables and columns. Schema import maps them to SQLite tables using shared columns and overflow tables, so the physical layout cannot be guessed from the schema.
- Schema item names are compared case-insensitively across all item types: a `Phenomenon` named `TESTLENGTH` collides with a `KindOfQuantity` named `TestLength`.
- iModels written by older versions must keep working. Treat changes to persisted data, mapping, or `ec_*` tables as compatibility changes.
- `iModelJsNodeAddon/api_package/ts/src/NativeLibrary.ts` is the contract with `@itwin/core-backend` in itwinjs-core. Keep it in sync with any change to the addon surface.
