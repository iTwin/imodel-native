# Changes

This document including important changes to syntax or file format.

| Module  | Version   |
| ------- | --------- |
| Profile | `4.0.0.2` |
| ECSQL   | `1.0.1.1` |

## `1/19/2023`: Add support for runtime instance and property accessor in ECSQL (beta)

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
