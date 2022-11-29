# Changes

This document including important changes to syntax or file format.

| Module  | Version   |
| ------- | --------- |
| Profile | `4.0.0.2` |
| ECSQL   | `1.0.0.0` |

## `11/29/2022`: Add `PRAGMA` support in `ECSQL`
+ Added support for `PRAGMA` in ECSQL. Syntax is as following
    ```sql
    -- Syntax
    PRAGMA <pragma-name> [ = <val>]
        [FOR <path>]
    val  -> INTEGER | DOUBLE | BOOLEAN | NULL | STRING | NAME
    path -> path '.' NAME

    -- Examples
    PRAGMA file_info;
    PRAGMA version FOR BisCore;
    PRAGMA disqualify_type_filter=TRUE
        FOR BisCore.ExternalSourceAspect;
    ```

* `PRAGMA disqualify_type_filter`: Allow to hint ECSQL to disqualify expression generated to apply polymorphic filter expression.
    ```sql
    -- Following is set by default in iModelPlatform when iModel is opened.
    PRAGMA disqualify_type_filter=TRUE
        FOR BisCore.ExternalSourceAspect
    ```

* `PRAGMA file_info`: Allow to run basic information about file, ecsql, ecdb and sqlite.
* `PRAGMA version FOR <schema>`: Return version of the EC Schema.
* `PRAGMA ecdb_version`: Return version of the EC Schema.

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
