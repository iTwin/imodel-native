# Using the schema compiler

Usage:

    flatc [ -c ] [ -j ] [ -b ] [ -t ] [ -o PATH ] [ -I PATH ] [ -S ] FILES...
          [ -- FILES...]

The files are read and parsed in order, and can contain either schemas
or data (see below). Later files can make use of definitions in earlier
files.

`--` indicates that the following files are binary files in
FlatBuffer format conforming to the schema(s) indicated before it.
Incompatible binary files currently will give unpredictable results (!)

Depending on the flags passed, additional files may
be generated for each file processed:

-   `-c` : Generate a C++ header for all definitions in this file (as
    `filename_generated.h`). Skipped for data.

-   `-j` : Generate Java classes. Skipped for data.

-   `-n` : Generate C# classes. Skipped for data.

-   `-g` : Generate Go classes. Skipped for data.

-   `-b` : If data is contained in this file, generate a
    `filename.bin` containing the binary flatbuffer.

-   `-t` : If data is contained in this file, generate a
    `filename.json` representing the data in the flatbuffer.

-   `-o PATH` : Output all generated files to PATH (either absolute, or
    relative to the current directory). If omitted, PATH will be the
    current directory. PATH should end in your systems path separator,
    e.g. `/` or `\`.

-   `-I PATH` : when encountering `include` statements, attempt to load the
    files from this path. Paths will be tried in the order given, and if all
    fail (or none are specified) it will try to load relative to the path of
    the schema file being parsed.

-   `--strict-json` : Generate strict JSON (field names are enclosed in quotes).
    By default, no quotes are generated.

-   `--no-prefix` : Don't prefix enum values in generated C++ by their enum
    type.

-   `--gen-includes` : Generate include statements for included schemas the
    generated file depends on (C++).

-   `--proto`: Expect input files to be .proto files (protocol buffers).
    Output the corresponding .fbs file.
    Currently supports: `package`, `message`, `enum`.
    Does not support, but will skip without error: `import`, `option`.
    Does not support, will generate error: `service`, `extend`, `extensions`,
    `oneof`, `group`, custom options, nested declarations.
