# iModelConsole

An interactive command-line tool for iModel, ECDb and BeSQLite files: open a file, run ECSQL and SQL, inspect schemas, and see the SQL an ECSQL statement is translated to.

## Building

Use the `iModelConsole` strategy:

```sh
bb -s iModelConsole build
```

## Usage

Start the `iModelConsole` executable from the build output, then:

```text
.help                                  list all commands
.open [readonly|readwrite] <file>      open an iModel, ECDb or BeSQLite file (read-only by default)
.parse <ecsql>                         show the SQLite SQL that ECSQL is translated to
.parse exp <ecsql>                     show the parsed expression tree
.parse token <ecsql>                   show the raw token tree
```

`.parse` also accepts `json` and `normalize`. Input without a leading `.` runs as ECSQL; it can span several lines and ends with `;`.
