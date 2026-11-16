# Results: property types, enumerations, KindOfQuantity, DateTimeInfo, units and formats

All tests are in PropertyTypeUpdateTests.cpp and pass. "Read as" means how the unchanged stored bytes read after the update. No update rewrote any data. `DISABLED_` tests document a bug and are noted at the end.

**Import options.** "Major" means a Read version bump on plain ECDb. "Dynamic" means a `DynamicSchema` with the iModel options (`DisallowMajorSchemaUpgrade | AllowMajorSchemaUpgradeForDynamicSchemas`).

## 1. Primitive type changes (T01–T08)

| Row | Change | Result | Existing data | Test |
|---|---|---|---|---|
| T01 | int -> long | Accepted (Major) | Same | `WidenIntToLong` |
| T01 | int -> double | Accepted (Major) | Read as double | `WidenIntToDouble` |
| T01 | int -> string (shared column and own table) | Accepted (Major) | Read as decimal text | `ConvertIntToStringInSharedColumn`, `…InOwnTable` |
| T01 | boolean -> int | Accepted (Major) | true=1, false=0 | `ConvertBooleanToInt` |
| T01 | string -> int, numeric text | Accepted (Major) | Same numbers | `ConvertNumericStringsToInt` |
| T01 | string -> int, non-numeric text | Accepted (Major) | **Read as 0** | `ConvertNonNumericStringsToInt_ReadAsZero` (+ own table) |
| T01 | double -> int / long | Accepted (Major) | **Fraction dropped on read** | `NarrowDoubleToInt_FractionDropped`, `NarrowDoubleToLong_FractionDropped` |
| T01 | long -> int | Accepted (Major) | Low 32 bits on read | `NarrowLongToInt` |
| T01 | int -> dateTime | Accepted (Major) | **Read as Julian day (~4713 BC)** | `ConvertIntToDateTime_ReadAsJulianDay` |
| T01 | dateTime -> string | Accepted (Major) | Read as Julian day text | `ConvertDateTimeToString_ReadAsJulianDayText` |
| T01 | string (ISO 8601) -> dateTime (customer case) | Accepted (Major) | **Read as wrong date** | `ConvertIsoDateTimeStringsToDateTime_ReadAsWrongDate` |
| T01 | string -> binary | Accepted (Major) | UTF-8 bytes as blob | `ConvertStringToBinary_ReadAsUtf8Bytes` |
| T01 | binary -> string | Accepted (Major) | Bytes read as text | `ConvertBinaryToString_ReadAsText` |
| T01 | string -> boolean | Accepted (Major) | `"1"`=true; `"0"` and `"true"` read as **false** | `ConvertStringToBoolean_TextTrueReadsAsFalse` |
| T01 | IGeometry -> binary | Accepted (Major) | Flatbuffer read as blob | `ConvertIGeometryToBinary_ReadAsFlatBuffer` |
| T01 | binary -> IGeometry | Accepted (Major) | Valid when blobs are flatbuffers | `ConvertBinaryHoldingFlatBuffersToIGeometry` |
| T01 | Round trips (int↔string, double↔int, string↔int, string↔dateTime) | Accepted | **Original values restored**, so nothing was rewritten | `RoundTrip*` |
| T01 | Chain int -> string -> dateTime | Accepted | Same as direct int -> dateTime | `ChainIntToStringToDateTime_ReadAsJulianDay` |
| T01 | Rows written between two changes | Accepted | New text reads 0 after the way back | `TextWrittenBetweenTypeChangesReadsAsZero` |
| T01 | Any change with no rows | Accepted | n/a | `ConvertStringToIntWithoutRows` |
| T01 | Any change without Major | **Rejected** `ECDb_0336` | n/a | `TypeChangeWithoutMajorVersionIsRejected` |
| T01 | Struct member type change without Major | **Rejected** `ECDb_0336` | n/a | `StructMemberTypeChangeWithoutMajorVersionIsRejected` |
| T02 | To or from point2d / point3d | **Rejected** `ECDb_0336` | n/a | `ConvertingIntToPoint3dIsRejected`, `ConvertingPoint2dToStringIsRejected` |
| T03 | Primitive -> non-strict enum, same type | Accepted (Minor) | Same | `ConvertIntToNonStrictEnumeration` |
| T04 | Primitive -> strict enum, same type | **Rejected** `0336`, also when all values are enumerators | n/a | `ConvertingIntToStrictEnumeration…` (2 tests) |
| T05 | Primitive -> non-strict enum, other type | Accepted (Major); rejected without Major | Read as new type | `ConvertIntToNonStrictStringEnumeration`, `ConvertStringToNonStrictIntEnumeration_ReadAsZero` |
| T06 | Primitive -> strict enum, other type | **Rejected** `0336` | n/a | `ConvertingIntToStrictStringEnumerationIsRejected` |
| T07 | Enum -> primitive, same type | Accepted (Minor) | Same | `ConvertEnumerationToItsBackingType` |
| T08 | Enum -> primitive, other type | Accepted (Major); rejected without Major | Read as new type | `ConvertEnumerationToOtherPrimitiveWithMajorVersion`, `…ToDouble` |

## 2. Struct and array types, and property kinds (T13–T23)

| Row | Change | Result | Test |
|---|---|---|---|
| T13 | Array element int -> long | **Rejected** `0336`, though the scalar change is accepted | `WideningArrayElementIntToLongIsRejected` |
| T14 | Array element int -> enum | **Rejected** `0336` | `ConvertingArrayElementIntToEnumerationIsRejected` |
| T15, T16 | Struct or struct-array property points to a different struct | **Rejected** `0336` | `ChangingStruct*PropertyTypeIsRejected` |
| T17–T23 | All kind changes (primitive, array, struct, struct array, navigation, in both directions) | **Rejected**: `0337` for a pure kind change, `0336` when the type name also changes | `Changing*To*IsRejected` (15 tests) |
| T17 | Kind change with DoNotFail | **Rejected**, not skipped silently | `KindChangeWithDoNotFailKeepsOldProperty` |
| P30 | Navigation property points to another relationship | **Rejected** `0339` | `ChangingNavigationRelationshipIsRejected` |

## 3. Enumerations (E01–E20, T09–T12)

| Row | Change | Result | Test |
|---|---|---|---|
| T09 | Non-strict enum -> other non-strict enum, same backing type | Accepted (Minor); values outside both enums kept | `SwitchToOtherNonStrictEnumeration…` (2 tests) |
| T10 | Enum -> strict enum, same backing type | Accepted, **even when stored values (99) are not enumerators (B5)** | `SwitchToStrictEnumerationWithOtherValues_Accepted` |
| T11 | Enum -> non-strict enum, other backing type | Accepted (Major); rejected without Major | `ConvertEnumerationToNonStrictEnumerationWithOtherBackingType` |
| T12 | Enum -> strict enum, other backing type | **Rejected** `0336` | `ConvertingEnumerationToStrictEnumerationWithOtherBackingTypeIsRejected` |
| E02 | Delete enum (property becomes int) | Accepted (Major); rejected without Major (`0678`) | `DeleteEnumerationAndKeepValuesAsInt` |
| E03 | Rename enum, referenced or not | Accepted (Major) as delete + add | `Rename*Enumeration*` |
| E06 | Change backing type | **Rejected** `0397` | `ChangingEnumerationBackingTypeIsRejected` |
| E07 | Strict -> non-strict | Accepted | `MakeEnumerationNonStrict` |
| E08 | Non-strict -> strict | **Rejected** `0399`, also when the data is valid | `MakingEnumerationStrict…` (2 tests) |
| E09 | Remove `isStrict` | **Rejected** `0399` (defaults to strict) | `RemoveIsStrictFromNonStrictEnumerationIsRejected` |
| E10, E11 | Add int / string enumerator | Accepted | `AddEnumerator`, `AddStringEnumerator` |
| E12 | Delete enumerator, in use or not | **Rejected** `0402` | `DeletingEnumerator…Rejected` (2 tests) |
| E12 | Delete enumerator with DoNotFail | Accepted, but the **enumerator is deleted anyway (B6)** | `SkippedEnumeratorDeletion_EnumeratorIsDeletedAnyway` |
| E13, E14 | Change enumerator value | **Rejected**: `0400` for int, `0401` for string | `ChangingEnumeratorValueIsRejected`, `ChangingStringEnumeratorValueIsRejected` |
| E17, E18 | Enumerator label / description | Accepted | `ChangeEnumeratorLabelAndDescription` |
| E19 | Enumerator value kind (int -> string literal) | **Rejected** `0400` | `ChangeEnumeratorValueKindIsRejected` |
| E20 | Reorder enumerators | Accepted, no change detected | `ReorderEnumerators` |

## 4. KindOfQuantity and property categories (K, P26–P29)

| Row | Change | Result | Test |
|---|---|---|---|
| P26 | Add KoQ to a property (scalar, array) | Accepted; **assigns a unit to existing numbers, no rescaling** | `AddKindOfQuantityToProperty`, `…ToArrayProperty` |
| P27 | Replace KoQ with one using the same persistence unit | Accepted | `ReplaceKindOfQuantityWithSamePersistenceUnit` |
| P28 | Replace KoQ with a different persistence unit | **Rejected** `0344`; accepted with a matching `AllowUnitChange`; **rejected** if `From` is wrong | `ReplacingKindOfQuantity…`, `…WithAllowUnitChange`, `AllowUnitChangeWithWrongFromUnitIsRejected` |
| P29 | Remove KoQ from a property | **Rejected** `0344` without `AllowUnitChange`; accepted with empty `To` | `RemovingKindOfQuantityWithoutAllowUnitChangeIsRejected`, `RemoveKindOfQuantityWithAllowUnitChange` |
| K08 | Delete KoQ, used or not | Accepted only with Major (`0677` without) | `DeleteUnusedKindOfQuantity`, `DeletingUnusedKindOfQuantityWithoutMajorVersionIsRejected` |
| K09 | Rename KoQ, referenced or not | Accepted (Major) as delete + add | `Rename*KindOfQuantity*` |
| K12, K15, K16, K17 | Relative error and presentation formats | Accepted | `ChangeKindOfQuantity…`, `Replace…`, `Remove…` |
| K13 | Change KoQ persistence unit | **Rejected** `0391`, **even with `AllowUnitChange` on every property** | `ChangingKindOfQuantityPersistenceUnitIsRejected` (+ `…DespiteAllowUnitChange`) |
| K02 | Delete property category (unreferenced, or reference removed too) | Accepted | `DeleteUnusedPropertyCategory`, `DeletePropertyCategoryAndRemoveItsReference` |
| K03 | Rename category | Accepted (Major) as delete + add | `Rename*PropertyCategory*` |
| P25 | Change category | Accepted | `ChangePropertyCategory` |

## 5. DateTimeInfo (X08)

| Change | Result | Existing data | Test |
|---|---|---|---|
| Kind Utc -> Unspecified, or remove the CA | Accepted | **Z marker lost on read** | `ChangeDateTimeKindUtcToUnspecified…`, `RemoveDateTimeInfo…` |
| Add `Utc` to a plain dateTime | Accepted | **Z marker gained on read** | `AddDateTimeInfoUtc_ValuesGainUtcMarker` |
| Component Date -> DateAndTime | Accepted | Time `T00:00:00.000` shown | `WidenDateTimeComponentToDateAndTime` |
| Component DateAndTime -> Date | Accepted | **Time of day hidden on read, still stored** | `NarrowDateTimeComponentToDate_TimeOfDayHidden` |
| Any change, no rows | Accepted | n/a | `ChangeDateTimeKindWithoutRows` |
| Kind -> `Local` | Accepted, **but the property can no longer be written (new finding)** | n/a | `DISABLED_ChangingDateTimeKindToLocalIsRejected` |

## 6. Units, phenomena, unit systems, formats

| Row | Change | Result | Test |
|---|---|---|---|
| U01, U06, U12, U13, U14 | Add unit system, phenomenon, unit, constant, inverted unit | Accepted | `AddUnit*`, `AddConstantAndInvertedUnit` |
| U02, U07, U15 | Delete unit system, phenomenon, unit (even unused, even with Major) | **Rejected** (`0411`, `0406`, `0416`) | `DeletingUnused*IsRejected` |
| U04, U09, U17 | Labels | Accepted | `Change*Label` |
| U11 | Phenomenon definition | **Rejected** `0409` | `ChangingPhenomenonDefinitionIsRejected` |
| U19 | Unit definition (persistence unit or unused) | **Rejected** `0419` | `RedefiningPersistenceUnitIsRejected`, `RedefiningUnusedUnitIsRejected` |
| U20, U21 | Unit phenomenon / unit system | **Rejected** `0419` | `ChangeUnitPhenomenon`, `ChangeUnitSystemOfUnit` |
| U22, U24, U26 | Implicit numerator / denominator / offset made explicit as the default | Accepted | `SetImplicitUnit*Explicitly` |
| U23, U25, U27 | Numerator, denominator or offset to a non-default value | **Rejected** `0419` | `ChangeUnitNumerator`, `ChangingUnit*IsRejected` |
| U28, U29, U30 | Constant ↔ unit, inverted-unit target, inverted ↔ unit | **Rejected** `0419` | `Change*IsRejected` |
| F01 | Add format | Accepted | `AddUnitAndFormat` |
| F02, F03 | Delete / rename format | **Rejected** `0421`, even unused | `DeletingUnusedFormatIsRejected`, `RenameUnreferencedFormatIsRejected` |
| F04–F19 | Label, description, precision, type, traits, round factor, sign, separators, min width | Accepted | `ChangeFormat*`, `ChangeNumericSpecSettingsOfFormat` |
| F20, F23–F26 | Composite added, or a unit slot changed or removed | **Rejected** `0424` | `AddingFormatCompositeIsRejected`, `ChangingFormatCompositeUnitIsRejected`, `Change/RemoveComposite*IsRejected` |
| F22, F27, F28 | Composite spacer, slot labels | Accepted | `ChangeFormatCompositeSpacer`, `…UnitLabel`, `ChangeCompositeMiddleSlotLabel` |
| F31 | Reorder composite slots | **Not reachable**: ECObjects refuses to read an out-of-order composite | comment only |
| F32–F35 | `ratioType`, azimuth settings | Accepted and verified persisted (`VerifyPersistedSchemas`) | `ChangeFormatRatioType`, `ChangeFormatAzimuthSettings` |

## 7. Other property facets and array bounds

| Row | Change | Result | Test |
|---|---|---|---|
| P18–P21 | Min/max length and value (narrow, raise, remove) | Accepted, **values not checked** | `NarrowMaximum*`, `SetMinimumLength`, `RaiseMinimumValue…`, `RemoveMinimumAndMaximumValue` |
| P24 | Plain extended type name | Accepted | `AddPlainExtendedTypeName` |
| P24 | `Id` on long | Accepted; **rendered as hex** | `AddIdExtendedTypeName_ValuesRenderedAsHex` |
| P24 | `Id` removed again | Accepted; plain numbers again | `RemoveIdExtendedTypeName_…` |
| P24 | `BeGuid` on non-16-byte blobs | Accepted; **blobs can't be rendered as GUIDs** | `AddBeGuidExtendedTypeName_Accepted` |
| A01 | Increase `minOccurs` | **Rejected** `0338` | `IncreasingArrayMinOccursIsRejected` |
| A02 | Decrease `minOccurs` | Accepted (matrix predates #1688) | `DecreaseArrayMinOccurs` |
| A03 | Increase `maxOccurs`, or to unbounded | Accepted | `IncreaseArrayMaxOccurs`, `ChangeArrayMaxOccursToUnbounded` |
| A04 | Decrease `maxOccurs`, or bound an unbounded array | **Rejected** `0338` | `DecreasingArrayMaxOccursIsRejected`, `BoundingUnboundedArrayIsRejected` |
| T / P12 | Type change combined with moving a property to a base class | Accepted with a data transform; **rejected `0590` without** `AllowDataTransformDuringSchemaUpgrade` | `MovePropertyToBaseClassWithOtherType`, `…WithoutDataTransformIsRejected` |

## 8. Import paths (Rob's I2 and I7)

| Scenario | Result | Test |
|---|---|---|
| Non-dynamic schema, iModel options, Read bump | **Rejected** `0427` (veto of the bump) | `TypeChangeWithIModelOptionsIsRejectedForNonDynamicSchema` |
| Non-dynamic schema, iModel options, Minor | **Rejected** `0336` | `…AndMinorVersionIsRejectedForNonDynamicSchema` |
| Dynamic schema, iModel options, Minor: type change | Accepted; same data effect as plain ECDb | `TypeChangeInDynamicSchemaWithIModelOptions` |
| Dynamic schema: the lossy customer case (string -> dateTime) | Accepted | `LossyTypeChangeInDynamicSchema…` |
| Dynamic schema: delete enum, delete KoQ | Accepted | `DeleteEnumeration…`, `DeleteKindOfQuantityInDynamicSchema…` |
| Dynamic schema: delete enumerator | **Still rejected** `0402`; the dynamic route does not open other gates | `DeletingEnumeratorInDynamicSchemaIsRejected` |
| `DoNotCreateOrUpdateDataTables` (schema sync): type change | **Rejected** `0336` | `TypeChangeWithDoNotCreateOrUpdateDataTablesIsRejected` |

## 9. Findings

| ID | Finding | Evidence |
|---|---|---|
| B5 | Strict-enum switch with out-of-set values is accepted, and the error message is built but ignored | Enabled test records current behaviour. `DISABLED_SwitchingToStrictEnumerationWithOtherValuesIsRejected` holds the expected behaviour. |
| B6 | With DoNotFail, a skipped enumerator deletion is reported as kept but is deleted | `SkippedEnumeratorDeletion_EnumeratorIsDeletedAnyway`, `DISABLED_SkippedEnumeratorDeletionKeepsTheEnumerator` |
| B7 | With DoNotFail, a skipped type change also skips other legal changes on the same property or KoQ (confirmed) | `DISABLED_SkippedTypeChangeKeepsOtherChangesOfTheProperty`, `DISABLED_SkippedPersistenceUnitChangeKeepsOtherChangesOfTheKindOfQuantity` |
| B9 | KoQ removal with a non-empty `AllowUnitChange.To` dereferences null | Test commented out because it crashes. Already reported. |
| New | `DateTimeKind=Local` is accepted but the property can no longer be bound | `DISABLED_ChangingDateTimeKindToLocalIsRejected` |
| New | `BeGuid` is accepted on a property whose blobs are not GUIDs | `AddBeGuidExtendedTypeName_Accepted` |
| New | F31 is unreachable: ECObjects enforces composite order before ECDb sees the change | comment in file |
| Matrix | A01–A04 predate #1688. E14 reports `0401`, not `0400`. E03, K03 and K09 are accepted as delete + add, not "No". | n/a |

## Other disabled tests to follow up

- `DISABLED_DeletingReferencedPropertyCategoryWithDoNotFailKeepsCategory`
- `DISABLED_ChangeStructMemberTypeDoubleToString`
- `DISABLED_ChangeStructMemberTypeDoubleToInt`

## Decision input for D2 (mapping impact only, or mapping and meaning)

The changes below are accepted but change what stored values mean:

- double -> int
- string -> int, string -> boolean, string -> dateTime
- int -> dateTime
- `DateTimeInfo` kind and component edits
- `AllowUnitChange`
- `Id` and `BeGuid` extended types

Of these, only the type changes are reversible without data loss, because nothing is rewritten. Every other accepted change reads the same bytes differently.