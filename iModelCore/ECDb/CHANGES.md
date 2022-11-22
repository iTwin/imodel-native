# Changes

This document including important changes to syntax or file format.

| Tables          | Version   |
| --------------- | --------- |
| Profile Version | `4.0.0.2` |
| ECSQL Version   | `1.0.0.0` |

## `11/22/2022`: Add `PRAGMA` support in `ECSQL`

1. Added support for `PRAGMA` in ECSQL. Syntax is as following

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

1. `PRAGMA disqualify_type_filter`: Allow to hint ECSQL to disqualify expression generated to apply polymorphic filter expression.

    ```sql
        -- Following is set by default in iModelPlatform when iModel is opened.
        PRAGMA disqualify_type_filter=TRUE
            FOR BisCore.ExternalSourceAspect
    ```

1. `PRAGMA file_info`: Allow to run basic information about file, ecsql, ecdb and sqlite.
1. `PRAGMA help`: Display all the pragma supported.
1. Added ECSQL version and set it to `1.0.0.0`. In future this version will be incremented as new feature in ECSQL is added or improved.
