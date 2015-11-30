/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/BaseCacheTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BaseCacheTest.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

std::shared_ptr<DataSourceCache> BaseCacheTest::s_reusableCache;

ECSchemaPtr BaseCacheTest::GetTestSchema()
    {
    Utf8String schemaXml =
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECSchemaReference name="Bentley_Standard_CustomAttributes" version="01.00" prefix="bsca" />
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
                <ECProperty propertyName="TestProperty2" typeName="string" />
                <ECProperty propertyName="TestProperty3" typeName="string" />
            </ECClass>
            <ECClass typeName="TestClass2" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
            <ECClass typeName="TestClassWithStruct" >
                <ECProperty propertyName="TestProperty" typeName="string" />
                <ECStructProperty propertyName="TestStructProperty" typeName="TestStructClass" />
            </ECClass>
            <ECClass typeName="TestStructClass" isStruct="True" isDomainClass="False">
                <ECProperty propertyName="TestStringProperty" typeName="string" />
                <ECArrayProperty propertyName="TestArrayProperty" typeName="string"  minOccurs="0" maxOccurs="unbounded" />
            </ECClass>
            <ECClass typeName="TestDerivedClass" >
                <BaseClass>TestClass</BaseClass>
                <ECProperty propertyName="TestProperty3" typeName="string" />
            </ECClass>
            <ECClass typeName="TestClassA" >
                <BaseClass>TestClass</BaseClass>
            </ECClass>
            <ECClass typeName="TestClassB" >
                <BaseClass>TestClass</BaseClass>
            </ECClass>
            <ECClass typeName="TestClass3" >
                <ECProperty propertyName="TestReadOnlyProperty" typeName="string" readOnly="True" />
                <ECProperty propertyName="TestCalculatedProperty" typeName="string" >
                    <ECCustomAttributes>
                        <CalculatedECPropertySpecification xmlns="Bentley_Standard_CustomAttributes.01.00">
                            <RequiredSymbolSets />
                            <ECExpression>"CalculatedValue"</ECExpression>
                        </CalculatedECPropertySpecification>
                    </ECCustomAttributes>
                </ECProperty>
            </ECClass>
            <ECClass typeName="TestLabeledClass" >
                <ECProperty propertyName="Name" typeName="string" />
            </ECClass>
            <ECClass typeName="TestDerivedLabeledClass" >
                <BaseClass>TestLabeledClass</BaseClass>
            </ECClass>
            <ECClass typeName="TestFolderClass" >  
                <ECProperty propertyName="TestName" typeName="string" />        
                <ECCustomAttributes>
                    <FolderDependentProperties xmlns="Bentley_Standard_CustomAttributes.01.00">
                        <FolderName>TestName</FolderName>
                    </FolderDependentProperties>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="TestFileClass" >  
                <ECProperty propertyName="TestSize" typeName="long" />   
                <ECProperty propertyName="TestName" typeName="string" />        
                <ECCustomAttributes>
                    <FileDependentProperties xmlns="Bentley_Standard_CustomAttributes.01.00">
                        <FileName>TestName</FileName>
                        <FileSize>TestSize</FileSize>
                    </FileDependentProperties>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="TestFileClass2" >  
                <ECProperty propertyName="TestSize" typeName="long" />   
                <ECProperty propertyName="TestName" typeName="string" />        
                <ECCustomAttributes>
                    <FileDependentProperties xmlns="Bentley_Standard_CustomAttributes.01.00">
                        <FileName>TestName</FileName>
                    </FileDependentProperties>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="TestFileClass3" >  
                <ECProperty propertyName="TestSize" typeName="long" />   
                <ECProperty propertyName="TestName" typeName="string" />        
                <ECCustomAttributes>
                    <FileDependentProperties xmlns="Bentley_Standard_CustomAttributes.01.00">
                        <FileSize>TestSize</FileSize>
                    </FileDependentProperties>
                </ECCustomAttributes>
            </ECClass>
            <ECClass typeName="TestFileClass4" >  
                <ECProperty propertyName="Name" typeName="string" />      
                <ECCustomAttributes>
                    <FileDependentProperties xmlns="Bentley_Standard_CustomAttributes.01.00">
                    </FileDependentProperties>
                </ECCustomAttributes>
            </ECClass>
            <ECRelationshipClass typeName="TestRelationshipClass" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <Source cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Source>
                <Target cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="TestRelationshipClass2" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <Source cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Source>
                <Target cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="TestDerivedRelationshipClass" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <BaseClass>TestRelationshipClass</BaseClass>
                <Source cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Source>
                <Target cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="TestRelationshipPropertiesClass" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <ECProperty propertyName="TestProperty" typeName="string" />  
                <Source cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Source>
                <Target cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="TestOneToOneRelationshipClass" isDomainClass="True" strength="referencing" strengthDirection="forward">
                <Source cardinality="(1,1)" polymorphic="True">
                    <Class class="TestClassA" />
                </Source>
                <Target cardinality="(1,1)" polymorphic="True">
                    <Class class="TestClassB" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ECSchemaReadContext::CreateContext());
    return schema;
    }

ECSchemaPtr BaseCacheTest::GetTestSchema2()
    {
    Utf8String schemaXml =
        R"xml(<ECSchema schemaName="TestSchema2" nameSpacePrefix="TS2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" >
                <ECProperty propertyName="TestProperty" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ECSchemaReadContext::CreateContext());
    return schema;
    }

BeFileName BaseCacheTest::GetTestSchemaPath()
    {
    BeFileName testSchemaPath(GetTestsTempDir().AppendToPath(L"TestSchema"));

    if (!testSchemaPath.DoesPathExist())
        {
        SchemaWriteStatus status = GetTestSchema()->WriteToXmlFile(testSchemaPath);
        EXPECT_EQ(SCHEMA_WRITE_STATUS_Success, status);
        }

    return testSchemaPath;
    }

std::shared_ptr<DataSourceCache> BaseCacheTest::GetTestCache()
    {
    if (s_reusableCache)
        {
        BentleyStatus status = s_reusableCache->Reset();
        EXPECT_EQ(SUCCESS, status);
        return s_reusableCache;
        }

    s_reusableCache = std::make_shared<DataSourceCache>();

    BeFileName cachePath(":memory:");

    cachePath = GetTestsTempDir().AppendToPath(L"testCache.ecdb");
    BeFileName::BeDeleteFile(cachePath);

    BentleyStatus status = s_reusableCache->Create(cachePath, StubCacheEnvironemnt());
    EXPECT_EQ(SUCCESS, status);

    status = s_reusableCache->UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema(), GetTestSchema2()});
    EXPECT_EQ(SUCCESS, status);

    return s_reusableCache;
    }

void BaseCacheTest::SetUpTestCase()
    {
    s_reusableCache = nullptr;
    WSClientBaseTest::SetUpTestCase();
    }
