# BIS schemas in the build

`BisSchemas.PartFile.xml` delivers schemas from the [bis-schemas](https://github.com/iTwin/bis-schemas) repository, checked out as `$SrcRoot/BisSchemas`, into build products. `findSchemaPath.mki` picks the file for each schema through `SchemaInventory.json` in that repository.

By default a part gets the latest released version of a schema, or the work-in-progress version if none has been released. To build against work-in-progress versions, set `WIP_SCHEMAS` to a semicolon-separated list of schema names, or `*` for all:

```sh
export WIP_SCHEMAS="BisCore;Generic"
```

Each part such as `Core` delivers a group of schemas. To deliver another schema, add a part for it in `BisSchemas.PartFile.xml`, modeled on the existing ones.
