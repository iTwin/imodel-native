# iModel Native Codebase Guide

## Architecture Overview

This is a **multi-layered C++ codebase** exposed to TypeScript via N-API:

- **ecobjects** (`iModelCore/ecobjects/`) - Low-level EC (Entity Class) schema API for metadata management
- **ECDb** (`iModelCore/ECDb/`) - SQLite-based object storage built on ecobjects, provides ECSQL query engine
- **iModelPlatform** (`iModelCore/iModelPlatform/`) - Business logic layer with predefined BIS schemas, elements, models, and change tracking
- **Node Addon** (`iModelJsNodeAddon/`) - N-API bindings exposing C++ to TypeScript via `IModelJsNative.cpp`
- **TypeScript API** (`iModelJsNodeAddon/api_package/ts/`) - Public TypeScript interface consumed by iTwin.js

**Data flow**: TypeScript → N-API glue (`IModelJsNative.cpp`) → iModelPlatform/ECDb → ecobjects → SQLite

## Coding Conventions
- Follow existing code style and patterns in each layer
- We have many typedefs that should be preferred, for example for strings:
    - `Utf8String` for UTF-8 encoded strings (uses std::string under the hood)
    - `Utf8CP` uses char const* for UTF-8 C-style strings
- We define several type aliases in macros that are widely used to indicate pointer/reference/const. For example for struct Utf8String these additional types exist:
    - `Utf8StringR` - reference (Utf8String&)
    - `Utf8StringCR` - const reference (const Utf8String&)
    - `Utf8StringP` - pointer (Utf8String*)
    - `Utf8StringCP` - const pointer (const Utf8String*)

## Build Notes
- Build system: BentleyBuild with files `.PartFile.xml` and low level bmake `.mke` files
- You cannot build this yourself, it requires an initialized shell environment from Bentley's internal build system, do not bother trying to build.

## Key Files to Reference

- `iModelCore/ecobjects/PublicApi/ECObjects/ECSchema.h` - huge header containing most of the ecobjects public API
- `iModelCore/ECDb/PublicAPI/ECDb/ECDb.h` - ECDb public API
- `iModelCore/iModelPlatform/PublicAPI/DgnPlatform/TxnManager.h` - Transaction management (changesets)
- `iModelJsNodeAddon/IModelJsNative.h` - N-API macro definitions and type aliases
- `iModelJsNodeAddon/IModelJsNative.cpp` - Main N-API entrypoint

