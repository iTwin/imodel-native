# Changes

This document including important changes to syntax or file format.

| Module  | Version   |
| ------- | --------- |
| Profile | `4.0.0.5` |
| ECSQL   | `2.0.2.0` |

## ## `01/29/2025`: Made schema names optional for table valued functions
* ECSql version change `2.0.1.1` -> `2.0.2.0`.
* Made schema names optional for table valued functions
* Table valued functions will now work with and without schema names. Example :- `SELECT * FROM json_each(:json_param)` & 
    `SELECT * FROM json1.json_each(:json_param)` both are now valid queries from ECSQL perspective.
* Example: `Select test.str_prop, test.int_prop, v.id from ts.A test RIGHT OUTER JOIN IdSet(:idSet_param) v on test.ECInstanceId = v.id`,
           `SELECT * FROM json_each(:json_param)`.

## ## `01/22/2025`: Added IdSet Virtual Table in ECSQL
* ECSql version change `2.0.1.0` -> `2.0.1.1`.
* Added IdSet Virtual Table in ECSQL
* Example: `Select test.str_prop, test.int_prop, v.id from ts.A test RIGHT OUTER JOIN ECVLib.IdSet(:idSet_param) v on test.ECInstanceId = v.id`.

## ## `01/10/2025`: Added support for CTE subquery with alias
* ECSql version change `2.0.0.0` -> `2.0.1.0`.
* Added support for CTE subquery with alias
* Example: `select a.x from (with tmp(x) as (SELECT e.i FROM aps.TestElement e order by e.i LIMIT 1) select x from tmp) a`.


## ## `09/05/2024`: Remove class names ALIAS support and Disqualify_polymorphic_constraint(+) support in UPDATE & DELETE statements
* ECSql version change `1.2.14.0` -> `2.0.0.0`.
* Removed class names ALIAS support and Disqualify_polymorphic_constraint(+) support in UPDATE & DELETE statements
* Example: `UPDATE ONLY ecsql.PSA t SET t.I = 124, t.L = 100000000000, t.D = -1.2345678, t.S = 'hello, world' WHERE t.D > 0.0`,
           `UPDATE +ONLY ecsql.P SET I=10 WHERE LOWER(S) = UPPER(S)`. These statements will now result in InvalidECsql.
* Example: `DELETE FROM ONLY ecsql.P t WHERE t.D > 0.0`, `DELETE FROM +ALL ecsql.PASpatial WHERE Geometry_Array IS NULL`. 
            These statements will now result in InvalidECsql.

## ## `08/30/2024`: Add support for CTE without columns
* ECSql version change `1.2.13.0` -> `1.2.14.0`.
* Added support for use of WITH clause or CTE without columns.
* Example: `WITH el AS ( SELECT ECInstanceId, ECClassId FROM bis.Element ) SELECT * FROM el`

## `08/29/2024`: Add support for INSERT statements with ONLY keyword
* ECSql version change `1.2.12.0` -> `1.2.13.0`.
* Added support for use of ONLY keyword in INSERT statements
* Example: `INSERT INTO ONLY ts.A VALUES('A-1',100)`

## `08/16/2024`: Add CTE support in subquery
* ECSql version change `1.2.11.0` -> `1.2.12.0`.
* Added support for use of WITH clause or CTE in subqueries.
* Example: `SELECT COUNT(*) FROM (WITH el (Id, ClassId) AS ( SELECT ECInstanceId, ECClassId FROM bis.Element ) SELECT * FROM el)`

## `07/11/2024`: Add PRAGMA purge_orphan_relationships

* ECSql version change `1.2.10.0` -> `1.2.11.0`.
* Added new command `PRAGMA purge_orphan_relationships` which will delete all orphaned instances in link table relationship tables.
* The command requires neither any arguments nor any boolean assignments.
* The use of this command in ECSQL requires either enabling experimental features globally with PRAGMA or specifying `OPTIONS ENABLE_EXPERIMENTAL_FEATURES` in the ecsql.
* Example: `PRAGMA purge_orphan_relationships options enable_experimental_features`.

## `1/10/2024`: Add support for navigation value creation function

* Add support for navigation value creation function
  * Example: `SELECT NAVIGATION_VALUE(bis.Element.Model, 10, 100)`.

*ECSql version updated `1.2.9.1` -> `1.2.10.0`

## `10/27/2023`: Pragma integrity_check(check_nav_class_ids) performance improved

ECSql version updated `1.2.9.1` -> `1.2.9.2`

## `10/24/2023`: Pragma integrity_check(check_nav_class_ids) now checks if the navigation property represents a valid ClassId for the relationship

ECSql version updated `1.2.9.0` -> `1.2.9.1`

## `10/12/2023`: Add support for window functions

* Add support for window functions
  * Example: `SELECT rank() OVER(PARTITION BY ECClassId ORDER BY ECInstanceId) from bis.Model`
* ECSql version updated `1.2.8.1` -> `1.2.9.0`

## `9/13/2023`: Pragma disqualify_type_filter only take effect if there was more then one class name in query

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

## `8/9/2023`: Truncate BLob to {bytes:####} instead of a single byte via ECSqlRowAdaptor

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

## `5/16/2023`: Enable property and class deletion, property type change for dynamic schemas with a major schema update

* Added new schema import option "AllowMajorSchemaUpgradeForDynamicSchemas".
    1. "AllowMajorSchemaUpgradeForDynamicSchemas" takes precedence over the "DisallowMajorSchemaUpgrade" import option, but only for dynamic schemas.
    2. If major schema upgrades have been disabled by using the import option "DisallowMajorSchemaUpgrade" and the user wants to perform a major schema upgrade on a dynamic schema, the new option "AllowMajorSchemaUpgradeForDynamicSchemas" needs to be added to enable the upgrade.
    3. "AllowMajorSchemaUpgradeForDynamicSchemas" has no effect on schemas that are not dynamic.

* Enable deletion of properties in dynamic schemas with a major schema upgrade.
    1. Properties can be deleted only if they belong to an ECClass which is mapped to a shared column.
    2. Navigation properties cannot be deleted.
    3. When a property is deleted with a major schema upgrade, the data that it contains is set to NULL.

* Enable deletion of classes in dynamic schemas with a major schema upgrade.
    1. Classes can only be deleted from a dynamic schema with a major schema upgrade only if:
        1. The class does not have a derived class.
        2. The class is not an ECStructClass.
        3. The class is not an ECRelationshipClass with a Foreign Key mapped.
        4. The class has not been specified in a relationship constraint.
    2. If the class to be deleted has instances present, all the instances are deleted as well.

* Enable property type change in dynamic schemas with a major schema upgrade.
    1. Changing property type from a primitive type to another primitive type is now supported with a major schema upgrade.
    2. Type can be changed in any combination from the primitive set { int, long, double, binary, boolean, datetime }.
    3. Type change for composite primitive types Point2d and Point3d is NOT supported.
    4. Type change to and from an un-strict enum that has a different primitive type is now supported.
    5. Type change to a strict enum of a different primitive type is NOT supported.
    6. When a property type is changed, the data will not change/update as per the new primitive type specified.
    7. It is the user's responsibility to handle the data changes required when a property's type is changed to a different primitive type.
