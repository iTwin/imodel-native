/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/BaseCacheTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BaseCacheTest.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

BeFileName BaseCacheTest::s_seedCacheFolderPath;
BeFileName BaseCacheTest::s_targetCacheFolderPath;
BeFileName BaseCacheTest::s_seedCachePath;
BeFileName BaseCacheTest::s_targetCachePath;
CacheEnvironment BaseCacheTest::s_seedEnvironment;
CacheEnvironment BaseCacheTest::s_targetEnvironment;

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
            <ECClass typeName="TestClass4" >
                <ECProperty propertyName="TestProperty" typeName="string" />
                <ECProperty propertyName="TestCalculatedProperty" typeName="string" >
                    <ECCustomAttributes>
                        <CalculatedECPropertySpecification xmlns="Bentley_Standard_CustomAttributes.01.00">
                            <RequiredSymbolSets />
                            <ECExpression>this.TestProperty</ECExpression>
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
            <ECRelationshipClass typeName="TestHoldingRelationshipClass" isDomainClass="True" strength="holding" strengthDirection="forward">
                <Source cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
                </Source>
                <Target cardinality="(0,N)" polymorphic="True">
                    <Class class="TestClass" />
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

std::shared_ptr<DataSourceCache> BaseCacheTest::GetTestCache(CacheEnvironment targetEnvironment)
    {
    // Prepare seed files
    if (!s_seedCacheFolderPath.DoesPathExist())
        {
        auto cache = CreateTestCache(s_seedCachePath, s_seedEnvironment);
        EXPECT_FALSE(nullptr == cache);
        }

    // Prepare target files
    if (s_targetCacheFolderPath.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(s_targetCacheFolderPath));
    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CloneDirectory(s_seedCacheFolderPath, s_targetCacheFolderPath));

    // Open target cache
    auto cache = std::make_shared<DataSourceCache>();
    cache->Open(s_targetCachePath, targetEnvironment);
    EXPECT_FALSE(nullptr == cache);
    return cache;
    }

std::shared_ptr<DataSourceCache> BaseCacheTest::CreateTestCache(Utf8StringCR fileName)
    {
    BeFileName filePath(":memory:");
    filePath = GetTestsTempDir().AppendToPath(BeFileName(fileName));

    if (filePath.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeDeleteFile(filePath));

    return CreateTestCache(filePath, StubCacheEnvironemnt());
    }

std::shared_ptr<DataSourceCache> BaseCacheTest::CreateTestCache(BeFileName filePath, CacheEnvironment environment)
    {
    auto cache = std::make_shared<DataSourceCache>();
    EXPECT_EQ(SUCCESS, cache->Create(filePath, environment));
    EXPECT_EQ(SUCCESS, cache->UpdateSchemas(std::vector<ECSchemaPtr> {GetTestSchema(), GetTestSchema2()}));
    return cache;
    }

void BaseCacheTest::SetUpTestCase()
    {
    s_seedCacheFolderPath = GetTestsOutputDir().AppendToPath(L"BaseCacheTest-Seeds");
    s_targetCacheFolderPath = GetTestsTempDir().AppendToPath(L"BaseCacheTest-TestCaches");

    s_seedCachePath = BeFileName(s_seedCacheFolderPath).AppendToPath(L"testcache.ecdb");
    s_seedEnvironment.persistentFileCacheDir = BeFileName(s_seedCacheFolderPath).AppendToPath(L"persistent/");
    s_seedEnvironment.temporaryFileCacheDir = BeFileName(s_seedCacheFolderPath).AppendToPath(L"temporary/");
    s_seedEnvironment.externalFileCacheDir = BeFileName(s_seedCacheFolderPath).AppendToPath(L"external/");

    s_targetCachePath = BeFileName(s_targetCacheFolderPath).AppendToPath(L"testcache.ecdb");
    s_targetEnvironment.persistentFileCacheDir = BeFileName(s_targetCacheFolderPath).AppendToPath(L"persistent/");
    s_targetEnvironment.temporaryFileCacheDir = BeFileName(s_targetCacheFolderPath).AppendToPath(L"temporary/");
    s_targetEnvironment.externalFileCacheDir = BeFileName(s_targetCacheFolderPath).AppendToPath(L"external/");

    if (s_seedCacheFolderPath.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(s_seedCacheFolderPath));

    WSClientBaseTest::SetUpTestCase();
    }

CacheEnvironment BaseCacheTest::GetTestCacheEnvironment()
    {
    return s_targetEnvironment;
    }