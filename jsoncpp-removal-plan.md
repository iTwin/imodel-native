# Remove JsonCpp from Units/DgnUnits/GeoCoord

## Context Change: @pmconne's Comment (May 2026)
The tile content ID hash of project extents is NO LONGER the blocker it was. Since iTwin.js PR #6727 (May 2024), the **default** behavior is to use "maximal extents" (rounded to nearest power of 10) for the tile hash — controlled by `expandProjectExtents` flag (defaults `true`, `@internal`). The rounding happens in `ModelState::ComputeMaximalExtents()` in `imodel-native-internal/iModelCore/Visualization/src/ModelState.cpp`.

**Key insight:** If we always use maximal (rounded) extents for the hash (remove the `@internal` flag / always expand), the exact binary representation of stored project extents no longer matters. Rounded values like `-1000.0`, `10000.0` serialize identically in jsoncpp and RapidJSON.

## Revised Approach for DgnUnits.cpp

### Option A: Simplify NOW (recommended)
1. Remove `Precision17` serialization from `SetProjectExtents` — use normal `Stringify()`
2. Remove `ParseFullPrecision()` from `LoadProjectExtents` — use normal `Parse()`
3. In Visualization code (`Tree.cpp`): Remove `ExtentsBasis::Project` path, always use `ExtentsBasis::Maximal`
4. Update `ProducesConsistentProjectExtentsHash` test — hash will be different since it's now based on rounded extents
5. In itwinjs-core (separate PR): Remove `expandProjectExtents` option (or deprecate)

**Why this works:** `ComputeMaximalExtents` rounds each axis to the nearest power of 10 via `std::ceil(std::log10(distance))` + `std::pow(10, log)`. These "nice" numbers serialize identically in any JSON library.

**Trade-off:** Existing cached tiles using `ExtentsBasis::Project` would be invalidated. Since `expandProjectExtents` has defaulted to `true` for 2 years, only consumers who explicitly set it to `false` would be affected — and it's `@internal`.

### Option B: Keep current Precision17 approach (safe fallback)
Keep what we have (already passing CI). Only simplify later when coordinated with itwinjs-core.

## GeoCoord Test Issue (SEPARATE from precision)

The `GCSTransformFromJsonSpecific` test failure is **NOT related to floating-point precision**. It's a **JSON parsing leniency** issue:
- jsoncpp's `Json::Reader` accepts non-standard JSON that RapidJSON rejects
- Both `Parse()` and `ParseFullPrecision()` fail on the same 3 entries
- `ParseFullPrecision` does NOT help here

### Diagnosis needed:
- Extract each entry from `s_listOfTestJsonGCS` and validate with a strict JSON parser
- Likely culprits: trailing commas, comments, malformed numbers, or C++ string concatenation artifacts
- Suspicious: GCSUnitTests.cpp line 3653 has an empty line between string literals (valid C++ but check resulting JSON)

### Fix options:
1. Fix the non-standard JSON in test data (preferred)
2. Use lenient RapidJSON flags (`kParseCommentsFlag | kParseTrailingCommasFlag`)
3. Keep jsoncpp for these specific tests only (last resort)

## Is `ParseFullPrecision` Still Useful?

**Revisited** — after switching GeoCoord tests to `EXPECT_DOUBLE_EQ` (allows 4 ULP tolerance), no test sites remain that require `ParseFullPrecision`. The tests now use normal `Parse()` and tolerate the minor double-parsing differences between RapidJSON's fast path and strtod.

**Consider removing `ParseFullPrecision` from BeJsValue.h** once DgnUnits.cpp is also simplified (Option A). If Option B is chosen for DgnUnits, keep it as a utility for that one site.

## Remaining Work

### Immediate (this PR):
1. **Fix GeoCoord test parse failures** — identify & fix the 3 bad JSON entries
2. **Decide: Option A or B for DgnUnits** — if A, also modify Visualization code

### Already done:
- ✅ DgnUnits.cpp converted (SetProjectExtents + LoadProjectExtents)
- ✅ BaseEllipsoidUnitTests.cpp converted
- ✅ BaseGCSUnitTests.cpp converted
- ✅ Precision17 writer + ParseFullPrecision utility created

### Future modules:
- Units formatting code (FormattingApi.h, NumericFormatSpec.cpp, etc.)
- ecobjects (~137 refs), ECDb (~409 refs), GeomLibs (~216 refs)
- ECPresentation (~234 refs), iModelJsNodeAddon (~149 refs)

## Conversion Patterns (reference)
- `Json::Value jval` → `BeJsDocument jval`
- `Json::Reader::Parse(str, jval)` → `BeJsDocument jval(str)` or `jval.Parse(str)`
- `JsonValueCR val` → `BeJsConst val`
- `jval.type() == Json::objectValue` → `jval.isObject()`
- `jval.isMember(name)` → `jval.isMember(name)` (same API)
- `val.asDouble()` → `val.asDouble()`
- Iterator: `for (iter = jval.begin()...)` → `jval.ForEachProperty([](Utf8CP name, BeJsConst val) { ... })`
- Array: `for (iter = jval.begin()...)` → `jval.ForEachArrayMember([](ArrayIndex, BeJsConst val) { ... })`

## Key File Locations

| File | Purpose |
|------|---------|
| `iModelCore/Bentley/BeRapidJson/PublicAPI/BeRapidJson/BeJsValue.h` | `Precision17`, `JsonCppCompatWriter`, `ParseFullPrecision()` |
| `iModelCore/iModelPlatform/DgnCore/DgnUnits.cpp` | `SetProjectExtents` (line ~463), `LoadProjectExtents` (line ~551) |
| `iModelCore/GeoCoord/Tests/NonPublished/GCSUnitTests.cpp` | Failing test data `s_listOfTestJsonGCS` (line 1503-3666) |
| `iModelCore/GeoCoord/Tests/NonPublished/BaseEllipsoidUnitTests.cpp` | 1 site using `ParseFullPrecision` |
| `iModelCore/GeoCoord/Tests/NonPublished/BaseGCSUnitTests.cpp` | 4 sites using `ParseFullPrecision` |
| `imodel-native-internal/iModelCore/Visualization/src/Tree.cpp` | `FormatExtentsHash`, `ExtentsBasis` logic (line ~395-553) |
| `imodel-native-internal/iModelCore/Visualization/src/ModelState.cpp` | `ComputeMaximalExtents` (line ~83-117) |
| `imodel-native-internal/iModelCore/Visualization/test/Tile_Test.cpp` | `ProducesConsistentProjectExtentsHash` test (line ~702) |
