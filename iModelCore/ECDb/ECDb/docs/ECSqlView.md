# View support in ECDb

View in ECDb is basically a `ECEntityClass` or `ECRelationshipClass` backed by a ECSQL query. The view are readonly and user cannot change them. There are two type of views

1. Persisted Views
2. Transient Views

## **Transient View**

Transient view simply substitute view class for query it represent.

### Rules for transient view

1. View class must be `Abstract` and mapped to a *Virtual Table*.
2. View class must not have derived classes.
3. View query must return `ECInstanceId` property
4. View query must not return `ECClassId`.
5. View query must alias and return property name same as view class properties.
6. View query properties must have sam type as view class properties.
7. View query cannot be a CTE.

Following is the custom attribute for `TransientView`

```xml
    <ECCustomAttributeClass
        typeName="TransientView"
        modifier="Sealed"
        appliesTo="EntityClass,RelationshipClass">

        <ECProperty propertyName="Query" typeName="string" description=""/>
    </ECCustomAttributeClass>
```

This can be applied to any `ECEntityClass` or `ECRelationshipClass` that follow the rules describe before.

```xml
    <ECSchema
            schemaName="test_schema"
            alias="ts"
            version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
        <ECEntityClass typeName="SchemaClassesView" description="" displayLabel="" modifier="Abstract">
            <ECCustomAttributes>
                <TransientView xmlns="ClassView.01.00.00">
                    <Query>SELECT cd.ECInstanceId,  sc.Name SchemaName, cd.Name ClassName FROM meta.ECSchemaDef sc JOIN meta.ECClassDef cd ON cd.Schema.Id=sc.ECInstanceId</Query>
                </TransientView>
           </ECCustomAttributes>
            <ECProperty propertyName="SchemaName" typeName="string" />
            <ECProperty propertyName="ClassName"  typeName="string"/>
        </ECEntityClass>
    </ECSchema>
```

Above view always substitute view query for when ever the class itself appear in a ECSQL.

```sql

    SELECT * FROM ts.SchemaClassesView WHERE SchemaName='bis' and ClassName='Element'

```

## **Persisted View**

Persisted view are computed by running the view query and stored permanently in table to which view class is mapped. Persisted view need to be populated using a call to `ECDb::RefreshViews()`. `RefreshView` can fail for some view if there was runtime error like `NOT NULL`,  `UNIQUE INDEX` or `FOREIGN KEY` constraint failure. It is import for user when designing view to make sure view definition is compilable with view query and never generate any row that will violate any constraint set on view class or one of its base class.

### Rules for persisted view

1. View class must be marked as `Sealed`.
1. View class must be mapped to a table that which is not virtual.
1. View class can inherit from other classes but it must take care of `ECInstanceId` generation.
    * For example if view inherit from `bis.GeometricElement3d` it must provide ECInstanceId compilable with the table.
1. Map properties of view class need to have compilable type. If they are not then user must use `CAST()` expression to cast them into right type.
1. `PersistenceMethod` is required attribute which must  be set. Currently the only supported value is `Permanent`.
   * `Permanent` method will store the view data in a regular table and will be captured by changeset.
1. `RefreshMethod` is required attribute which must be set. Currently the only supported value is `Recompute`.
   * `Recompute` mean delete all rows and re-execute view query to file the view data.
1. View class must define `PropertyMaps`.
    * PropertyMap is array of string where each index array map to query selection. While the string it hold is one of the property in view class.
    * The mapping does not need to be one to one. Query might return more properties then class but only map a subset or vice versa view class may have more properties but map subset of them while leave other to have `Null` value.

`PersistedView`custom attribute is define as following

```xml
    <ECEnumeration typeName="PersistenceMethod" backingTypeName="string" isStrict="true">
        <ECEnumerator name="Permanent" value="Permanent" displayLabel="Permanent"/>
    </ECEnumeration>

    <ECEnumeration typeName="RefreshMethod" backingTypeName="string" isStrict="true">
        <ECEnumerator name="Recompute" value="Recompute" displayLabel="Recompute"/>
    </ECEnumeration>

    <ECCustomAttributeClass typeName="PersistedView" modifier="Sealed" appliesTo="EntityClass,RelationshipClass">
        <ECProperty propertyName="PersistenceMethod" typeName="PersistenceMethod" description=""/>
        <ECProperty propertyName="RefreshMethod" typeName="RefreshMethod" description=""/>
        <ECProperty propertyName="Query" typeName="string" description=""/>
        <ECArrayProperty propertyName="PropertyMaps" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
    </ECCustomAttributeClass>
```

Following is a example of `PersistedView` definition which also define a index on view class.

```xml
    <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdb' />
    <ECSchemaReference name='ClassView' version='01.00.00' alias='classView' />
    <ECEntityClass typeName="ModelCategories3d" description="" displayLabel="" modifier="Sealed">
        <ECCustomAttributes>
            <PersistedView xmlns="ClassView.01.00.00">
                <PersistenceMethod>Permanent</PersistenceMethod>
                <RefreshMethod>Recompute</RefreshMethod>
                <Query>
                    SELECT DISTINCT
                        phymod.ECInstanceId [ModelId],
                        cat.ECInstanceId [CategoryId],
                        g3d.ECClassId [Geom3dElementClassId]
                    FROM [bis].[GeometricElement3d] [g3d]
                        INNER JOIN [bis].[PhysicalModel] [phymod]
                            ON [phymod].[ECInstanceId] = [g3d].[Model].[Id]
                        INNER JOIN [bis].[SpatialCategory] [cat]
                            ON [cat].[ECInstanceId] = [g3d].[Category].[Id]
                </Query>
                <PropertyMaps>
                    <string>ModelId</string>
                    <string>CategoryId</string>
                    <string>Geom3dElementClassId</string>
                </PropertyMaps>
            </PersistedView>
            <DbIndexList xmlns="ECDbMap.02.00.00">
                <DbIndex>
                    <Name>ix_modelCategories</Name>
                    <IsUnique>True</IsUnique>
                    <Properties>
                        <string>ModelId</string>
                        <string>CategoryId</string>
                        <string>Geom3dElementClassId</string>
                    </Properties>
                </DbIndex>
            </DbIndexList>
        </ECCustomAttributes>
        <ECProperty propertyName="ModelId" typeName="long" extendedType="Id"/>
        <ECProperty propertyName="CategoryId" typeName="long" extendedType="Id"/>
        <ECProperty propertyName="Geom3dElementClassId" typeName="long" extendedType="ClassId"/>
    </ECEntityClass>
```

The query in above case might take a minute or two to run for worse case with 21 million geom elements. But subsequent query to find category for a given imodel or find category and imodel for a geom type could take less then few milliseconds.

We can make sure ECDb::RefreshViews() is called after named version is created or when the connector finalize changes. Changes to View will be captured in changeset so not every down stream application would need to run it.

# Todo

1. View query can reference other view and thus if those view are persisted then when refreshing view, view dependency has to be use to to order view refresh for each view.
1. View query can reference it self which will cause validation in view manager to recursively check it self and will fail.
   1. We need to prepare select clause to ensure it can be prepared. For Transient view it will cause view manager to substitute query again causing recursion and crash.
1. Data version check is require to ensure refresh view can skip view that does not need update.
   1. use be_prop to keep track of a data version field that is incremented every time we do save changes()
   2. Each view use the db data version when it is refreshed.
   3. If view data version is less then db data version then view need refresh.
1. Add method to ECSqlStatement to get GetClassReferences();
