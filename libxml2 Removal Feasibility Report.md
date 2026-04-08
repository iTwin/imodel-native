# libxml2 Removal Feasibility Report

**Date:** 2026-04-08
**Author:** Hoang Nam Le
**Status:** Experiment complete — findings documented

---

## Objective

Evaluate whether `libxml2` (and its C++ wrapper `BeXml`) can be removed from the imodel-native codebase, and estimate the effort required.

## Background

`libxml2` is vendored at `iModelCore/libsrc/libxml2/` and wrapped by a Bentley C++ layer called **BeXml** (`iModelCore/Bentley/BeXml/`). BeXml provides `BeXmlDom`, `BeXmlReader`, `BeXmlWriter`, and `BeXmlNode` — high-level C++ classes built on top of raw libxml2 APIs.

The codebase already includes **pugixml** as an alternative XML library, and many newer code paths use it directly.

### Motivation

- The vendored libxml2 headers are incompatible with the **macOS 26 SDK** — `xmlCtxtSetOptions` was renamed, and `xmlSetStructuredErrorFunc` has a signature change. The baseline BeXml build already fails on this SDK.
- Reducing third-party dependencies simplifies the build and shrinks the dependency surface.

---

## Experiment Setup

- **Workspace:** `/Users/hoangnamle/Documents/imodel-native-ws/`
- **Build system:** BentleyBuild (`bb`) with `BuildArchitecture=macosarm64`, `BuildStrategy=iModelConsole`
- **Approach:** Comment out all libxml2/BeXml references from PartFile.xml and .mke files, then attempt compilation to discover all source-level dependencies.

---

## Build System Changes Made

### PartFile.xml modifications (4 files)

| File | Change |
|------|--------|
| `iModelCore/Bentley/Bentley.PartFile.xml` | Commented out entire `BeXml` part definition (lines ~196-204) |
| `iModelCore/ecobjects/ECObjects.PartFile.xml` | Commented out `BeXml` SubPart reference |
| `iModelCore/GeoCoord/GeoCoord.PartFile.xml` | Commented out `BeXml` SubPart reference |
| `iModelCore/iModelPlatform/iModelPlatform.PartFile.xml` | Commented out `BeXml` SubPart reference |

### Makefile modifications (4 files)

| File | Change |
|------|--------|
| `iModelCore/ecobjects/src/ecobjects.mke` | Commented out `iTwinXml` and `iTwinLibxml2` linker libs |
| `iModelCore/GeoCoord/BaseGeoCoord/GeoCoord.mke` | Commented out `iTwinXml` linker lib |
| `iModelCore/iModelPlatform/iModelPlatform.mke` | Commented out `iTwinXml` linker lib |
| `iModelCore/ECDb/ECDb/ECDb.mke` | Commented out `iTwinXml` and `iTwinLibxml2` linker libs |

### Not yet modified (5 ecobjects tool .mke files)

- `iModelCore/ecobjects/tools/SchemaValidator/SchemaValidator.mke`
- `iModelCore/ecobjects/tools/SchemaConverter/SchemaConverter.mke`
- `iModelCore/ecobjects/tools/SchemaJsonSerializer/SchemaJsonSerializer.mke`
- `iModelCore/ecobjects/tools/SchemaComparison/SchemaComparison.mke`
- `iModelCore/ecobjects/tools/SchemaRoundtrip/SchemaRoundtrip.mke`

---

## Results

### Build system level — ✅ clean removal

Removing BeXml from PartFile.xml and .mke files works cleanly. The `BentleyDll` part compiles successfully without BeXml. No build system errors from the removal itself.

### Source code level — ❌ deep dependency in ecobjects

After removing `#include <BeXml/BeXml.h>` from the precompiled header (`ECObjectsPch.h`), **13 source files** fail to compile:

| File | Error Count | Primary Usage |
|------|-------------|---------------|
| `ECClass.cpp` | 7 | `WriteXml()` via `BeXmlWriter` |
| `ECCustomAttribute.cpp` | 4 | `WriteXml()` via `BeXmlWriter` |
| `ECEnumeration.cpp` | 1 | `WriteXml()` via `BeXmlWriter` |
| `ECInstance.cpp` | 15 | `BeXmlWriter`, `BeXmlDom`, `ReadFromBeXmlDom/Node`, `WriteToBeXmlNode` |
| `ECProperty.cpp` | 6 | `WriteXml()` via `BeXmlWriter` |
| `ECSchema.cpp` | 3 | `WriteXml()` via `BeXmlWriter` |
| `ECUnit.cpp` | 3 | `WriteXml()` via `BeXmlWriter` |
| `Format.cpp` | 1 | `WriteXml()` via `BeXmlWriter` |
| `KindOfQuantity.cpp` | 1 | `WriteXml()` via `BeXmlWriter` |
| `Phenomenon.cpp` | 1 | `WriteXml()` via `BeXmlWriter` |
| `PropertyCategory.cpp` | 1 | `WriteXml()` via `BeXmlWriter` |
| `SchemaXml.cpp` | 2 | XML serialization helpers |
| `UnitSystem.cpp` | 1 | `WriteXml()` via `BeXmlWriter` |

#### Unique error categories

| Error | Root Cause |
|-------|-----------|
| `incomplete type 'BeXmlWriter'` | Forward-declared in public headers but definition removed |
| `unknown type name 'BeXmlWriterPtr'` | Typedef from `BeXml.h` |
| `use of undeclared identifier 'BEXML_Success'` | Enum from `BeXml.h` |
| `XML_CHAR_ENCODING_UTF8` / `UTF16LE` | Raw libxml2 constants leaked into ecobjects |

### Public API impact

The public header `ECObjects/ECInstance.h` forward-declares `BeXmlWriter` (line 16), and several public methods take `BeXmlWriterR` parameters:

- `IECInstance::WriteToBeXmlNode()`
- `IECInstance::ReadFromBeXmlDom()`
- `IECInstance::ReadFromBeXmlNode()`

This means removing BeXml is an **API-breaking change** for ecobjects.

### Other consumers (not yet tested)

| Component | Usage |
|-----------|-------|
| **GeoCoord** (`basegeocoord.cpp`) | `#include <BeXml/BeXml.h>` — XML parsing for coordinate system data |
| **BeXml itself** (`BeXml.cpp`) | The wrapper library — would be entirely removed |
| **5 ecobjects CLI tools** | Link against `BeLibxml2` |

---

## Effort Estimate

### Option A: Replace BeXml with pugixml (recommended)

pugixml is already a dependency of ecobjects. The main work is:

1. **Replace `BeXmlWriter`** — used for XML serialization across all EC types (`WriteXml()` methods). pugixml's `xml_document`/`xml_node` can serve the same purpose but with a different API pattern (DOM-based vs streaming writer).
   - ~13 files, ~44+ call sites in ecobjects
   - Pattern is consistent: `xmlWriter.WriteElementString(name, value)` → pugixml DOM manipulation

2. **Replace `BeXmlDom`/`BeXmlReader`** in `ECInstance.cpp` — XML instance deserialization. Some of this already uses pugixml (`ReadFromBeXmlDom` takes a `pugi::xml_document&`).

3. **Update public API headers** — Remove `BeXmlWriter` forward declarations and replace method signatures in `ECInstance.h`, `ECSchema.h`.

4. **GeoCoord** — Separate effort, likely similar scope.

5. **Remove `BeXml` part** and `libsrc/libxml2/` entirely.

**Estimated scope:** Medium — mostly mechanical replacement of `BeXmlWriter` calls with pugixml equivalents. The patterns are repetitive across files.

### Option B: Update vendored libxml2 to fix macOS 26 SDK

If the goal is just fixing the macOS 26 build, updating the vendored libxml2 headers or adding compatibility shims for the renamed APIs would be significantly less work.

---

## Recommendation

If the long-term goal is to reduce dependencies, **Option A** (replace with pugixml) is the right path. The dependency is deep but mechanical — every `WriteXml()` method follows the same pattern. A focused effort could port ecobjects in a few days.

If the immediate need is just macOS 26 compatibility, **Option B** (update vendored libxml2) is faster.

---

## Appendix: Dependency Chain

```
iModelConsole
└── iModelPlatformDLL
    ├── ECDbLibrary ──→ links iTwinXml, iTwinLibxml2
    │   └── ECObjects ──→ links iTwinXml, iTwinLibxml2
    │       └── BeXml (SubPart from Bentley.PartFile.xml)
    │           └── BeLibxml2 (libsrc/libxml2/)
    ├── GeoCoord ──→ links iTwinXml
    │   └── BeXml
    └── Bentley
        └── BeXml ──→ wraps libxml2 in C++ classes
```

## Appendix: Files Modified in Experiment

All changes were made in the workspace copy at `imodel-native-ws/src/imodel-native/` (not the main repo). Changes are uncommitted and can be discarded.
