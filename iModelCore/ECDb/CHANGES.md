# Changes

This document including important changes to syntax or file format.

| Module  | Version   |
| ------- | --------- |
<<<<<<< HEAD
| Profile | `4.0.0.2` |
| ECSQL   | `1.0.1.1` |
=======
| Profile | `4.0.0.4` |
| ECSQL   | `1.2.8.1` |

## `9/13/2023`: Prgma disqualify_type_filter only take effect if there was more then one class name in query

ECSql version updated `1.2.8.0` -> `1.2.8.1`

If following is set
```sql
    PRAGMA disqualify_type_filter=TRUE
        FOR BisCore.ExternalSourceAspect;
```

Then if we only select the `BisCore.ExternalSourceAspect` then the `disqualify_type_filter` will not take effect.

```sql
SELECT * FROM BisCore.ExternalSourceAspect
```

But if we join the `ExternalSourceAspect` with something else then the `disqualify_type_filter` will take effect and ECClassId expression will be disqualified.

## `9/26/2023`: Add support for ImportRequiresVersion and UseRequiresVersion custom attributes
* Two custom attributes were added to the ECDbMap schema, the schema version is incremented to `02.00.02`
    * They indicate whether the import process of a schema with the CA or anything using a CA requires a minimum ECDb version
* ECDb profile version updated to `4.0.0.3` -> `4.0.0.4`.

## `9/13/2023`: Runtime instance and property accessor no longer experimental

1. ECSql version updated `1.2.7.0` -> `1.2.8.0`.
2. Instance property access is no longer experimental and does not require the experimental features to be enabled for it's use.
3. The use of `$` and `$->prop` in ECSQL now requires neither enabling experimental features globally with PRAGMA nor specifying `OPTIONS ENABLE_EXPERIMENTAL_FEATURES`.

Following will work:

```sql
  SELECT $ FROM meta.ECClassDef
  SELECT $->name FROM meta.ECClassDef
```

## `9/7/2023`: Add option to customize ECSQL Instance

1. ECSql version updated to `1.2.6.0` -> `1.2.7.0`.
2. Added following `ECSQLOPTIONS` or `OPTIONS`
   * `USE_JS_PROP_NAMES` returns json compilable with iTwin.js typescript.
   * `DO_NOT_TRUNCATE_BLOB` return full blob instead of truncating it.
3. Instance access now add `json()` around `extract_inst()` function.

Following return iTwin.js compilable json

```sql
  SELECT $ FROM Bis.Element OPTIONS USE_JS_PROP_NAMES
```

Following return complete blob as base64

```sql
  SELECT $ FROM  Bis.GeometricElement3d OPTIONS DO_NOT_TRUNCATE_BLOB
```

## `9/6/2023`: Changes to ECSQLOPTIONS

1. ECSql version updated to `1.2.5.0` -> `1.2.6.0`.
2. `ECSQLOPTIONS` is now just called `OPTIONS`. `ECSQLOPTIONS` will continue be supported but is deprecated.
3. Options specified will be inherited by sub queries. Local query option take priority over inherited.

Following should work now

```sql
SELECT 1 FROM (
  SELECT $ FROM Bis.Element
) LIMIT 1 OPTIONS ENABLE_EXPERIMENTAL_FEATURES
```

## `8/31/2023`: Update behavior of instance properties

1. ECSql version updated to `1.2.3.0` -> `1.2.5.0`.
2. By default, all properties accessed via instance accessor i.e. `$->prop` must exist in the class identifying the row for that row to qualify for output.
3. If the user uses `?` after a property accessor e.g. `$->prop?`  then it will be considered optional, and the row class will not be checked to see if the `prop` exists or not.
4. Fixed issue where multiple required properties was not taken into account instead only alphabetically the first property was use to filter class-ids.

The following query will return no row if there is no subclass of `Bis.Element` that has both properties `CodeValue` and `Foo` in it.

```sql
  SELECT ECClassId, ECInstanceId
  FROM Bis.Element
      WHERE $->CodeValue = 'Profiling' OR $->Foo = 'Hello'
  LIMIT 1
  ECSQLOPTIONS ENABLE_EXPERIMENTAL_FEATURES
```

On the other hand, the following query makes `Foo` optional by adding `?` at the end like `$->Foo?`. This will exclude this property from the list of instance properties that must exist in the class of a row for it to qualify for output.

```sql
  SELECT ECClassId, ECInstanceId
  FROM Bis.Element
      WHERE $->CodeValue = 'Profiling' OR $->Foo? = 'Hello'
  LIMIT 1
  ECSQLOPTIONS ENABLE_EXPERIMENTAL_FEATURES
```

> Note: Optional property may slow down performance while non-optional properties will improve the performance of instance query.

## `8/28/2023`: Add support for ImportRequiresVersion and UseRequiresVersion custom attributes
* Two custom attributes were added to the ECDbMap schema, the schema version is incremented to `02.00.02`
    * They indicate whether the import process of a schema with the CA or anything using a CA requires a minimum ECDb version
* ECDb profile version updated to `4.0.0.3` -> `4.0.0.4`.

## `8/18/2023`: Add support for CROSS Join

* Add support for CROSS Join.
  * Example: `SELECT 1 FROM meta.ECClassDef CROSS JOIN meta.ECPropertyDef`.
* ECSql version updated to `1.2.3.0` -> `1.2.4.0`.

## `8/9/2023`: Add support for FIRST/LAST

* Add support for ordering NULLs.
  * Example: `SELECT * FROM Meta.ECClassDef ORDER BY Displaylabel NULLS FIRST`.
* ECSql version updated to `1.2.2.0` -> `1.2.3.0`.

## `8/9/2023`: Add support for RIGHT/FULL Join

* Add support for RIGHT/FULL join.

## `8/9/2023`: Add support for `ECSQLOPTIONS ENABLE_EXPERIMENTAL_FEATURES`

* Add support for RIGHT/FULL join.
  * Example: `SELECT $->name FROM meta.ECClassDef ECSQLOPTIONS enable_experimental_features`.

## `8/9/2023`: Truncate BLob to {bytes:####} instead of a single byte via QueryJsonAdaptor

* This effect instance access and concurrent query.

## `8/7/2023`: Add support to get parse tree for ecsql using a `PRAGMA parse_tree`

* Change ECSQL version `1.2.0.0` -> `1.2.2.0`
* Add `PRAGMA parse_tree(<ecsql>)`
* Add `ECSQLOPTIONS` clause to `PRAGMA` statement.
  * Allow to run commands like `PRAGMA parse_tree("SELECT 1") ECSQLOPTIONS enable_experimental_features`

## `6/14/2023`: Add support for Schema sync

* Schema sync allows two or more briefcases to sync there schema without requiring schema lock.
* Schema lock might still be required in cases where data transformation require due to schema change.
* Schema sync is handled in import schema call.

## `5/22/2023`: Add `PRAGMA checksum(ecdb_schema|ecdb_map|sqlite_schema)`

* ECSql version change to `1.0.4.1` as new syntax and runtime changes that does not break any existing syntax or runtime.
* PRAGMA checksum(ecdb_schema|ecdb_map|sqlite_schema)
  * `PRAGMA checksum(ecdb_schema)`: Compute SHA1 checksum for not null data in ec_* table that hold schemas.
  * `PRAGMA checksum(ecdb_map)`: Compute SHA1 checksum for not null data in ec_* table that hold mapping.
  * `PRAGMA checksum(sqlite_schema)`: Compute SHA1 checksum over ddl store in sqlite_master for all facets.

## `6/14/2023`: Add support for Schema sync

* Schema sync allows two or more briefcases to sync there schema without requiring schema lock.
* Schema lock might still be required in cases where data transformation require due to schema change.
* Schema sync is handled in import schema call.

## `5/22/2023`: Add PRAGMA checksum(ecdb_schema|ecdb_map|sqlite_schema)

* ECSql version change to `1.0.4.1` as new syntax and runtime changes that does not break any existing syntax or runtime.
* PRAGMA checksum(ecdb_schema|ecdb_map|sqlite_schema)
  * `PRAGMA checksum(ecdb_schema)`: Compute SHA1 checksum for not null data in ec_* table that hold schemas.
  * `PRAGMA checksum(ecdb_map)`: Compute SHA1 checksum for not null data in ec_* table that hold mapping.
  * `PRAGMA checksum(sqlite_schema)`: Compute SHA1 checksum over ddl store in sqlite_master for all facets.

## `5/3/2023`: Enhanced Instance properties

* ECSQL version changed from `1.1.0.0` -> `1.2.0.0`
* **Removed** `PROP_EXISTS()` This is not required anymore as instance prop is now auto-filtered internally.
* Add VirtualTab `class_props()` that is used to filter instance property query rows.
* Added support for dynamic property metadata for instance property.
  * `ColumnInfo::IsDynamic()` check is an ECSqlStatement output property that has dynamic data and might change on each call to `Step()`
  * `ECSqlStatement::GetColumnInfo(int)` will update on each `Step()` for dynamic properties.
  * Expose `IsDynamicProp()` via NativeAddon for use from the typescript side.
* Major improvement to Instance prop functionality where the use of a virtual table to filter rows based on the property selected.
* Fix bug that causes assertion when access instance that mapped to overflow table.
* Instance prop continues to be an experimental feature.
* Add instance properties docs on wiki.

## `4/10/2023`: Add comprehensive ECDb integrity checks and support for enable/disabling experimental features

* In future all experimental syntax will be only available when `PRAGMA experimental_features_enabled` is set to true.
* ECSQL version changed from `1.0.2.1` -> `1.1.0.0`.
* **Removed** PRAGMA `class_id_check`, `nav_prop_id_check` and `validate`. (Breaking change in ECSQL. ECSqlVersion -> `1.1.0.0`)
* Add `PRAGMA experimental_features_enabled` allow to enable experimental feature that disable by default.
* Made following as experimental features
    1. PRAGMA integrity_check
    2. Instance property access. The use of `$` and `$->prop` in ECSQL.
* Add `PRAGMA integrity_check` with following checks
    1. `check_ec_profile` - checks if the profile table, indexes, and triggers are present. Does not check of be_* tables. Issues are returned as a list of tables/indexes/triggers which was not found or have different DDL.
    2. `check_data_schema` - checks if all the required data tables and indexes exist for mapped classes.  Issues are returned as a list of tables/columns which was not found or have different DDL.
    3. `check_data_columns` - checks if all the required columns exist in data tables. Issues are returned as a list of those tables/columns.
    4. `check_nav_class_ids` - checks if `RelClassId` of a Navigation property is a valid ECClassId. It does not check the value to match the relationship class.
    5. `check_nav_ids` - checks if `Id` of a Navigation property matches a valid row primary class.
    6. `check_linktable_fk_class_ids` - checks if `SourceECClassId` or `TargetECClassId`  of a link table matches a valid ECClassId.
    7. `check_linktable_fk_ids`- checks if `SourceECInstanceId` or `TargetECInstanceId`  of a link table matches a valid row in primary class.
    8. `check_class_ids`- checks persisted `ECClassId` in all data tables and make sure they are valid.
    9. `check_schema_load` - checks if all schemas can be loaded into memory.

## `2/7/2023`: Add ECDb validity/integrity checks

* ECSql version change to `1.0.3.1` as new syntax and runtime changes that does not break any existing syntax or runtime.
* New PRAGMA commands added:
  * `PRAGMA class_id_check`: Checks if there are no classId references to non existing class definitions.
  * `PRAGMA nav_prop_id_check`: Checks if all navigation properties have valid classIds.
  * `PRAGMA validate`: Runs all checks.

## `2/6/2023`: Add support for GREATEST/LEAST Sql functions

The function let you find inline a greatest or least value out of specified values.

ECSQL version change from  `1.0.1.1` to  `1.0.2.1`

```sql
    SELECT GREATEST(1,2,3), LEAST (1,2,3)
```
>>>>>>> 963a6e22 (Update ECDbMap, bump ECDb profile version to 4.0.0.4)

## `2/1/2023`: Add support for runtime instance and property accessor in ECSQL (beta)

Following has been added or modified tos support extraction of instance and property from current row.

* ECSql version change to `1.0.1.1` as new syntax and runtime changes that does not break any existing syntax or runtime.
* Following new functions added and should not be called directly but rather use syntax as the implementation might change.
    1. `EXTRACT_INST(ECClassId, ECInstanceId)` - returns JSON string of instance or NULL.
    2. `EXTRACT_PROP(ECClassId, ECInstanceId, AccessString)` - returns typed primitive value for single value properties or return JSON for composite or NULL if not found or value is NULL.
    3. `PROP_EXISTS(ECClassId,AccessString)` - Can be used by user directly. As there is no syntax for this at the moment.
* New syntax added and should be used to access respective functionality.
    1. `$` represent current instance and returned a json.
    2. `$-><AccessString>` cause a property to be dynamically extracted from select instance, more efficient `json_extract()`

### Access current instance

This allow render a complete instance from current row into ECSQL-JSON.

```sql
    -- $' return complete instance for current row with all properties.
    -- It call EXTRACT_INST() to read the instance.
    SELECT $ FROM bis.Element
```

### Accessing a property within current instance

This allow a arbitrary property that may exist anywhere in derive hierarchy to be extracted and returned. It has following syntax.
For primitive types that has single value, following will return typed value while any composite value is returned as JSON.

> Syntax: `$-><access-string>`

```sql
    -- return given properties for rows for which it exists or return null.
    SELECT $->PropertyThatMayOrMayNotExists FROM bis.Element
```



## `12/06/2022`: Add `PRAGMA` support in `ECSQL`
+ Added support for `PRAGMA` in ECSQL. Syntax is as following
    ```sql
    -- Syntax
    PRAGMA <pragma-name> [ = <val>]
        [FOR <path>]
    val  -> INTEGER | DOUBLE | BOOLEAN | NULL | STRING | NAME
    path -> path '.' NAME

    -- Examples
    PRAGMA disqualify_type_filter=TRUE
        FOR BisCore.ExternalSourceAspect;
    ```

* `PRAGMA disqualify_type_filter`: Allow to hint ECSQL to disqualify expression generated to apply polymorphic filter expression.
    ```sql
    -- Following is set by default in iModelPlatform when iModel is opened.
    PRAGMA disqualify_type_filter=TRUE
        FOR BisCore.ExternalSourceAspect
    ```

* `PRAGMA ecdb_ver`: Return version of the EC Schema.

* `PRAGMA help`: Display all the pragma supported.
* Added `ECSQL version` and set it to `1.0.0.0`. In future this version will be incremented as new feature in ECSQL is added or improved.
   1. Type of changes that can be made to ECSQL.
      1. **Syntax change** Change to ECSQL grammar.
      2. **Runtime change** A change to runtime behavior, that normally passes `Prepare()` e.g. argument or result of a `Sql function`.
   2. `ECSql Version` description for left to right digit in version string i.e. `Major.Minor.Sub1.Sub2`
       1. **Major**: Any breaking change to `Syntax`. This will cause a `Prepare()` to fail with InvalidECSql which in previous version prepared successfully.  e.g. Changing or removing support for existing supported ECSql syntax.
      1. **Minor**: Any breaking change to `Runtime` e.g. Removing support for a previously accessible sql function or change it in a way where it will not work as before. In this case `Prepare()` phase may or may not detect a failure but result are not as expected as it use to in previous version. e.g. Remove a sql function or change required argument or format of its return value.
      2. **Sub1**:  Backward compatible change to `Syntax`. For example adding new syntax but not breaking any existing.
      3. **Sub2**:  Backward compatible change to `Runtime`. For example adding a new sql function.
