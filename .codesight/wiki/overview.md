# imodel-native — Overview

> **Navigation aid.** This article shows WHERE things live (routes, models, files). Read actual source files before implementing new features or making changes.

**imodel-native** is a javascript project built with raw-http.

## Scale

84 library files · 32 middleware layers · 30 environment variables

**Libraries:** 84 files — see [libraries.md](./libraries.md)

## High-Impact Files

Changes to these files have the widest blast radius across the codebase:

- `iModelJsNodeAddon/api_package/ts/src/test/utils.ts` — imported by **13** files
- `iModelJsNodeAddon/api_package/ts/src/NativeLibrary.ts` — imported by **12** files
- `/compat.py` — imported by **7** files
- `iModelJsNodeAddon/api_package/ts/src/test/index.ts` — imported by **5** files
- `iModelCore/libsrc/flatbuffers/source/java/com/google/flatbuffers/FlatBufferBuilder.java` — imported by **4** files
- `iModelCore/libsrc/flatbuffers/source/tests/MyGame/Example/Monster.java` — imported by **3** files

## Required Environment Variables

- `A` — `iModelJsNodeAddon/api_package/ts/src/test/DgnDb.test.ts`
- `APPVEYOR` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `APPVEYOR_REPO_BRANCH` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `APPVEYOR_REPO_TAG` — `iModelCore/libsrc/flatbuffers/source/conan/appveyor/build.py`
- `APPVEYOR_REPO_TAG_NAME` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `CMAKE_VS_VERSION` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `COMPARE_GENERATED_TO_GO` — `iModelCore/libsrc/flatbuffers/source/tests/py_test.py`
- `COMPARE_GENERATED_TO_JAVA` — `iModelCore/libsrc/flatbuffers/source/tests/py_test.py`
- `CONAN_ARCHS` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `CONAN_BUILD_TYPES` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `CONAN_LOGIN_USERNAME` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- `CONAN_STABLE_BRANCH_PATTERN` — `iModelCore/libsrc/flatbuffers/source/conan/build.py`
- _...18 more_

---
_Back to [index.md](./index.md) · Generated 2026-05-12_