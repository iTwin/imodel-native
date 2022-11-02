/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#if defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
#include <windows.h>
#include <Psapi.h>
#endif

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//---------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
WString PerformanceTestFixture::GetStandardsPath(WString fileName)
    {
    return ECTestFixture::GetAssetsDataPath({L"ECSchemas", L"Standard", fileName});
    }

//---------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
WString PerformanceTestFixture::GetAssetsGSchemaPath(WString domain, WString schemaFile)
    {
    return ECTestFixture::GetAssetsGDataPath({L"ECSchemas", domain, schemaFile});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
void PerformanceTestFixture::StoreStringToFile(Utf8CP fileName, Utf8StringCR content)
    {
    FILE* targetFile= nullptr;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot(dir);
    dir.AppendToPath(L"TestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory (dir.c_str());

    dir.AppendUtf8(fileName);

    bool existingFile = dir.DoesPathExist();

    errno_t err = fopen_s(&targetFile, dir.GetNameUtf8().c_str(), "a+");
    if (err != 0)
        {
        PERFORMANCELOG.errorv(L"The file %ls was not opened\n", dir.GetName());
        return;
        }

    if (!existingFile)
        fprintf(targetFile, content.c_str());

    fclose(targetFile);
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void PerformanceTestFixture::TimeSchemaXml(Utf8String schemaXml, std::vector<WString>& referencePaths, bool acceptLegacyImperfectLatestCompatibleMatch, bool includeFilesWithNoVerExt, Utf8String testcaseName)
    {
    size_t repeats = PerformanceTestFixture::MinimumRepeats;
    ECSchemaPtr schema;
    std::vector<double> serializationResults;
    std::vector<double> deserializationResults;
    for(size_t i = 0; i < repeats; i++)
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(acceptLegacyImperfectLatestCompatibleMatch, includeFilesWithNoVerExt);
        for(auto referencePath : referencePaths)
            schemaContext->AddSchemaPath(referencePath.c_str());

        StopWatch deserializationTimer("Deserialization", true);
        SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext);

        deserializationTimer.Stop();
        EXPECT_EQ(SchemaReadStatus::Success, status);
        deserializationResults.push_back(deserializationTimer.GetElapsedSeconds());

        StopWatch serializationTimer("Serialization", true);
        Utf8String ecSchemaXml;
        SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXml, schema->OriginalECXmlVersionLessThan(ECVersion::V3_1) ? ECVersion::V3_0 : ECVersion::Latest);
        serializationTimer.Stop();

        EXPECT_EQ(SchemaWriteStatus::Success, status2) << schema->GetFullSchemaName().c_str();
        serializationResults.push_back(serializationTimer.GetElapsedSeconds());

        if(i == 0)
            { //after the first run, recalculate number of repeats
            repeats = CalculateNumberOfRepeats(serializationResults[0] + deserializationResults[0]);
            }
        }

    double serializationMedian;
    Utf8String serializationJson = GenerateResultJson(serializationResults, serializationMedian);
    Utf8String serializeDesc = Utf8PrintfString(R"(Serializing schema: %s)", schema->GetFullSchemaName().c_str());
    LOGPERFDB(testcaseName.c_str(), serializeDesc.c_str(), serializationMedian, serializationJson.c_str());

    double deserializationMedian;
    Utf8String deserializationJson = GenerateResultJson(deserializationResults, deserializationMedian);
    Utf8String deserializeDesc = Utf8PrintfString(R"(De-serializing schema: %s)", schema->GetFullSchemaName().c_str());
    LOGPERFDB(testcaseName.c_str(), deserializeDesc.c_str(), deserializationMedian, deserializationJson.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void PerformanceTestFixture::TimeSchemaXmlFile(WString schemaPath, std::vector<WString>& referencePaths, bool acceptLegacyImperfectLatestCompatibleMatch, bool includeFilesWithNoVerExt, Utf8String testcaseName)
    {
    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Open(schemaPath.c_str(), BeFileAccess::Read));
    ByteStream fileContents;
    EXPECT_EQ(BeFileStatus::Success, file.ReadEntireFile(fileContents));
    Utf8String schemaXml ((Utf8CP)fileContents.GetData(), fileContents.GetSize());
    referencePaths.push_back(BeFileName::GetDirectoryName(schemaPath.c_str()));

    TimeSchemaXml(schemaXml, referencePaths, acceptLegacyImperfectLatestCompatibleMatch, includeFilesWithNoVerExt, testcaseName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// static
void PerformanceTestFixture::TimeInstance(WCharCP schemaName, WCharCP instanceXmlFile, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName)
    {
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(schemaName).c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);

    size_t repeats = PerformanceTestFixture::MinimumRepeats;
    std::vector<double> readingResults;
    std::vector<double> writingResults;
    Utf8String className;
    for(size_t i = 0; i < repeats; i++)
        {
        IECInstancePtr  testInstance;
        InstanceReadStatus instanceStatus;

        StopWatch readingTimer("Reading", true);
        instanceStatus = IECInstance::ReadFromXmlFile(testInstance, ECTestFixture::GetTestDataPath(instanceXmlFile).c_str(), *instanceContext);
        readingTimer.Stop();
        readingResults.push_back(readingTimer.GetElapsedSeconds());
        EXPECT_EQ(InstanceReadStatus::Success, instanceStatus);

        StopWatch writingTimer("Serialization", true);
        WString ecInstanceXml;
        InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, true);
        writingTimer.Stop();
        writingResults.push_back(writingTimer.GetElapsedSeconds());
        EXPECT_EQ(InstanceWriteStatus::Success, status2);

        if(i == 0)
            { //after the first run, recalculate number of repeats
            className = testInstance->GetClass().GetName();
            repeats = CalculateNumberOfRepeats(readingResults[0] + writingResults[0]);
            }
        }

    Utf8String readingInstanceDesc = Utf8PrintfString(R"(Reading instance from class: %s:%s)", schema->GetFullSchemaName().c_str(), className.c_str());
    Utf8String writingInstanceDesc = Utf8PrintfString(R"(Writing instance from class: %s:%s)", schema->GetFullSchemaName().c_str(), className.c_str());

    double readingMedian;
    Utf8String readingJson = GenerateResultJson(readingResults, readingMedian);
    LOGPERFDB(testcaseName.c_str(), readingInstanceDesc.c_str(), readingMedian, readingJson.c_str());

    double writingMedian;
    Utf8String writingJson = GenerateResultJson(writingResults, writingMedian);
    LOGPERFDB(testcaseName.c_str(), writingInstanceDesc.c_str(), writingMedian, writingJson.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
std::size_t PerformanceTestFixture::GetCurrentMemoryUsage()
    {
    std::size_t memoryUsage = 0;

#if defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
    //get the handle to this process
    auto myHandle = GetCurrentProcess();

    //to fill in the process' memory usage details
    PROCESS_MEMORY_COUNTERS pmc;
    //return the usage (bytes)
    if (GetProcessMemoryInfo(myHandle, &pmc, sizeof(pmc)))
        memoryUsage = pmc.WorkingSetSize;
#endif

    return memoryUsage;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
std::size_t PerformanceTestFixture::GetPeakMemoryUsage()
    {
    std::size_t memoryUsage = 0;

#if defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
    //get the handle to this process
    auto myHandle = GetCurrentProcess();

    //to fill in the process' memory usage details
    PROCESS_MEMORY_COUNTERS pmcBefore;
    //return the usage (bytes)
    if (GetProcessMemoryInfo(myHandle, &pmcBefore, sizeof(pmcBefore)))
        memoryUsage = pmcBefore.PeakWorkingSetSize;
#endif

    return memoryUsage;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void PerformanceTestFixture::MeasureSchemaMemory(Utf8String schemaXml, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName)
    {
    std::size_t memoryBefore = 0, memoryAfter = 0;

    memoryBefore = GetCurrentMemoryUsage();
    
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    memoryAfter = GetCurrentMemoryUsage();

    Utf8String description = Utf8PrintfString("Deserializing schema: %s", schema->GetFullSchemaName().c_str());
    LOGPERFDB(testcaseName.c_str(), description.c_str(), "MemoryUsageInBytes", (double)(memoryAfter - memoryBefore), "{}");
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void PerformanceTestFixture::MeasureSchemaMemory(WString ecSchemaPath, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName)
    {
    BeFile file;
    EXPECT_EQ(BeFileStatus::Success, file.Open(ecSchemaPath.c_str(), BeFileAccess::Read)) << ecSchemaPath.c_str();
    ByteStream fileContents;
    EXPECT_EQ(BeFileStatus::Success, file.ReadEntireFile(fileContents));
    Utf8String schemaXml ((Utf8CP)fileContents.GetData(), fileContents.GetSize());
    schemaContext->AddSchemaPath(BeFileName::GetDirectoryName(ecSchemaPath.c_str()).c_str());

    MeasureSchemaMemory(schemaXml, schemaContext, testcaseName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void PerformanceTestFixture::MeasureSchemaMemoryUsage (ECSchemaPtr (*generateSchema) (void), Utf8String testcaseName)
    {
    size_t memoryBefore = 0, memoryAfter = 0;
    memoryBefore = GetCurrentMemoryUsage();
    ECSchemaPtr testSchema = (*generateSchema)();
    EXPECT_TRUE(testSchema.IsValid());
    memoryAfter = GetCurrentMemoryUsage();
    LOGPERFDB(testcaseName.c_str(), testSchema->GetDescription().c_str(), "MemoryUsageInBytes", (double)(memoryAfter - memoryBefore), "{}");
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaPtr PerformanceTestFixture::GenerateFlatTestSchema(size_t numberOfClasses, size_t numberOfProperties)
    {
    ECSchemaPtr schema;
    Utf8PrintfString schemaName("FlatSchema_%zuC_%zuP", numberOfClasses, numberOfProperties);
    ECSchema::CreateSchema(schema, schemaName, "FTS", 1, 1, 1);
    Utf8PrintfString description("Schema with %zu Classes %zu Properties per class", numberOfClasses, numberOfProperties);
    schema->SetDescription(description);

    for (size_t i = 0; i < numberOfClasses; i++) 
        {
        ECEntityClassP c;
        Utf8PrintfString className("Class%zu", i);
        schema->CreateEntityClass(c, className);

        for (size_t x = 0; x < numberOfProperties; x++) 
            {
            PrimitiveECPropertyP p;
            Utf8PrintfString propertyName("Property%zu", x);
            c->CreatePrimitiveProperty(p, propertyName, PrimitiveType::PRIMITIVETYPE_String);
            }
        }
    
    return schema;
    }

void addPropertyOverrides (ECClassP currentClass)
    {
    if (!currentClass->HasBaseClasses())
        return;
    //in this case we override 50% of a classes properties
    for(auto baseClass : currentClass->GetBaseClasses())
        {
        auto nProperties = baseClass->GetPropertyCount(false);
        auto countUpTo = nProperties / 2;
        for(uint32_t i = 0; i < countUpTo; i++)
            {
            auto prop = baseClass->GetPropertyByIndex(i);
            PrimitiveECPropertyP p;
            Utf8PrintfString label("Property override for %s in class %s", prop->GetName().c_str(), currentClass->GetName().c_str());
            currentClass->CreatePrimitiveProperty(p, prop->GetName().c_str(), PrimitiveType::PRIMITIVETYPE_String);
            p->SetDisplayLabel(label);
            }
        addPropertyOverrides(baseClass);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaPtr PerformanceTestFixture::GenerateDeepHierarchyTestSchema(size_t numberOfClassHierarchies, size_t numberOfClassesPerHierarchy, size_t numberOfMixinsPerHierarchy, size_t numberOfPropertiesPerClass, bool overrideProperties)
    {
    ECSchemaPtr schema;
    Utf8PrintfString schemaName("HierarchSchema_%zuH_%zuC_%zuM_%zuP", numberOfClassHierarchies, numberOfClassesPerHierarchy, numberOfMixinsPerHierarchy, numberOfPropertiesPerClass);
    ECSchema::CreateSchema(schema, schemaName, "HTS", 1, 1, 1);
    Utf8PrintfString description("Schema with %zu hierarchies %zu deep %zu mixins %zu properties per class%s",
        numberOfClassHierarchies, numberOfClassesPerHierarchy, numberOfMixinsPerHierarchy, numberOfPropertiesPerClass, overrideProperties ? " and property overrides" : "" );
    schema->SetDescription(description);

    for (size_t hierarchy = 0; hierarchy < numberOfClassHierarchies; hierarchy++) 
        {
        ECEntityClassP rootClass = nullptr;
        ECEntityClassP baseClass = nullptr;
        ECEntityClassP previousClass = nullptr;
        for (size_t nClass = 0; nClass < numberOfClassesPerHierarchy; nClass++) 
            {
            ECEntityClassP currentClass;
            Utf8PrintfString className("Hierarchy%zu_Class%zu", hierarchy, nClass);
            schema->CreateEntityClass(currentClass, className);
            if(previousClass != nullptr)
                currentClass->AddBaseClass(*previousClass);

            for (size_t nProp = 0; nProp < numberOfPropertiesPerClass; nProp++) 
                {
                PrimitiveECPropertyP p;
                Utf8PrintfString propertyName("Hierarchy%zu_Class%zu_Property%zu", hierarchy, nClass, nProp);
                currentClass->CreatePrimitiveProperty(p, propertyName, PrimitiveType::PRIMITIVETYPE_String);
                }

            if (nullptr == previousClass)
                rootClass = currentClass;
            previousClass = currentClass;
            if(baseClass == nullptr)
                baseClass = currentClass;
            }

        for (size_t nMixin = 0; nMixin < numberOfMixinsPerHierarchy; nMixin++) 
            {
            ECEntityClassP mixin;
            Utf8PrintfString className("Hierarchy%zu_Mixin%zu", hierarchy, nMixin);
            schema->CreateMixinClass(mixin, className, *baseClass);
            for (size_t nProp = 0; nProp < numberOfPropertiesPerClass; nProp++) 
                {
                PrimitiveECPropertyP p;
                Utf8PrintfString propertyName("Hierarchy%zu_Mixin%zu_Property%zu", hierarchy, nMixin, nProp);
                mixin->CreatePrimitiveProperty(p, propertyName, PrimitiveType::PRIMITIVETYPE_String);
                }

            rootClass->AddBaseClass(*mixin);
            }

        if(overrideProperties)
            addPropertyOverrides(previousClass);
        }
    
    return schema;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaPtr PerformanceTestFixture::AddCustomAttributesToSchema(ECSchemaPtr schema, size_t numberOfItemsToSkipBetweenEach)
    {
    ECCustomAttributeClassP customAttributeClass;
    schema->CreateCustomAttributeClass(customAttributeClass, "CustomAttributeClass");
    customAttributeClass->SetContainerType(CustomAttributeContainerType::Any);
    PrimitiveECPropertyP stringProp;
    customAttributeClass->CreatePrimitiveProperty(stringProp, "StringProp", PRIMITIVETYPE_String);

    size_t itemsToSkip = 0;
    size_t caCounter = 0;
    StandaloneECEnablerPtr enabler = customAttributeClass->GetDefaultStandaloneEnabler();
    auto generateCaInstance = [&] () -> IECInstancePtr {
        IECInstancePtr result = enabler->CreateInstance();
        Utf8PrintfString strVal("CA Value No %zu", caCounter++);
        ECValue v (strVal.c_str(), true);
        result->SetValue("StringProp",v);
        return result;
    };

    schema->SetCustomAttribute(*generateCaInstance());
    itemsToSkip = numberOfItemsToSkipBetweenEach;
    for(auto schemaItem : schema->GetClasses())
        {
        if(schemaItem->IsCustomAttributeClass())
            continue;

        if(itemsToSkip-- > 0)
            {
            schemaItem->SetCustomAttribute(*generateCaInstance());
            itemsToSkip = numberOfItemsToSkipBetweenEach;
            }

        for(auto property : schemaItem->GetProperties(false))
            {
            if(itemsToSkip-- > 0)
                {
                property->SetCustomAttribute(*generateCaInstance());
                itemsToSkip = numberOfItemsToSkipBetweenEach;
                } 
            }
        }
    return schema;
    }

Utf8String PerformanceTestFixture::GenerateResultJson(std::vector<double>& results, double& median)
    {
    median = 0.0;
    if(results.empty())
        {
        return "{}";
        }

    double mean = 0.0;
    double min = results[0];
    double max = results[0];
    for(auto& value : results)
        {
        mean += value;
        if(min > value)
            min = value;

        if(max < value)
            max = value;
        }
    mean /= results.size();

    size_t halfSize = results.size() / 2;
    std::nth_element(results.begin(), results.begin()+halfSize, results.end());
    median = results[halfSize];

    double stdDev = 0.0;
    if(results.size() > 1)
        {
        for(auto& value : results)
            {
            stdDev += pow((value - mean), 2);
            }
        stdDev /= (results.size() - 1);
        stdDev = sqrt(stdDev);
        }

    Utf8PrintfString json(R"json("{""repeats"":%zu,""mean"":%f,""min"":%f,""max"":%f,""stdDev"":%f}")json", results.size(), mean, min, max, stdDev);
    return json;
    }

size_t PerformanceTestFixture::CalculateNumberOfRepeats(double firstDuration)
    {
    if(firstDuration == 0.0)
        return 1;

    size_t numberOfRepeats = (size_t)std::round(DesiredNumberOfSecondsPerTest / firstDuration);
    if(numberOfRepeats < MinimumRepeats)
        return MinimumRepeats;

    if(numberOfRepeats > MaximumRepeats)
        return MaximumRepeats;

    return numberOfRepeats;
    }
END_BENTLEY_ECN_TEST_NAMESPACE
