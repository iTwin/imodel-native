/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//=======================================================================================    
//! @bsiclass
//=======================================================================================    
struct PerformanceTestFixture : public ECTestFixture
{
protected:
    static void StoreStringToFile(Utf8CP fileName, Utf8StringCR content);

    static void TimeSchemaXml(Utf8String schemaXml, std::vector<WString>& referencePaths, bool acceptLegacyImperfectLatestCompatibleMatch, bool includeFilesWithNoVerExt, Utf8String testcaseName);
    static void TimeSchemaXmlFile(WString schemaPath, std::vector<WString>& referencePaths, bool acceptLegacyImperfectLatestCompatibleMatch, bool includeFilesWithNoVerExt, Utf8String testcaseName);
    static void TimeSchemaXml(Utf8String schemaXml, Utf8String testcaseName)
    {
        std::vector<WString> referencePaths;
        TimeSchemaXml(schemaXml, referencePaths, false, false, testcaseName);
    };

    static void TimeInstance(WCharCP schemaName, WCharCP instanceXmlFile, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName);
    static std::size_t GetCurrentMemoryUsage();
    static std::size_t GetPeakMemoryUsage();
    static void MeasureSchemaMemory(Utf8String schemaXml, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName);
    static void MeasureSchemaMemory(WString ecSchemaPath, ECSchemaReadContextPtr schemaContext, Utf8String testcaseName);
    static void MeasureSchemaMemoryUsage (ECSchemaPtr (*generateSchema) (void), Utf8String testcaseName);

    static ECSchemaPtr GenerateFlatTestSchema(size_t numberOfClasses, size_t numberOfProperties);
    static ECSchemaPtr GenerateDeepHierarchyTestSchema(size_t numberOfClassHierarchies, size_t numberOfClassesPerHierarchy, size_t numberOfMixinsPerHierarchy, size_t numberOfPropertiesPerClass, bool overrideProperties);
    static ECSchemaPtr AddCustomAttributesToSchema(ECSchemaPtr schema, size_t numberOfItemsToSkipBetweenEach);

    static ECSchemaPtr GenerateSchema20000Classes1PropsPerClass() { return AddCustomAttributesToSchema(GenerateFlatTestSchema(20000, 1), 3); }
    static ECSchemaPtr GenerateSchema2000Classes10PropsPerClass() { return AddCustomAttributesToSchema(GenerateFlatTestSchema(2000, 10), 3); }
    static ECSchemaPtr GenerateSchema1000Classes20PropsPerClass() { return AddCustomAttributesToSchema(GenerateFlatTestSchema(1000, 20), 3); }
    static ECSchemaPtr GenerateSchema100Classes200PropsPerClass() { return AddCustomAttributesToSchema(GenerateFlatTestSchema(100, 200), 3); }
    static ECSchemaPtr GenerateSchema10Classes2000PropsPerClass() { return AddCustomAttributesToSchema(GenerateFlatTestSchema(10, 2000), 3); }
    static ECSchemaPtr GenerateSchema10Root15Deep3Mixin5PropsAndOverrides() { return AddCustomAttributesToSchema(GenerateDeepHierarchyTestSchema(10, 15, 3, 5, true), 3); }
    static ECSchemaPtr GenerateSchema300Root3Deep200Props() { return AddCustomAttributesToSchema(GenerateDeepHierarchyTestSchema(300, 3, 0, 200, false), 3); }
    static ECSchemaPtr GenerateSchema50Root5Deep3Mixin5Props() { return AddCustomAttributesToSchema(GenerateDeepHierarchyTestSchema(50, 5, 3, 5, false), 3); }

    WString GetStandardsPath(WString fileName);
    WString GetAssetsGSchemaPath(WString domain, WString schemaFile);

    //Generates some statistics and calculates the median from a set of results. The json is wrapped in quotes and uses double-quoting around the property names
    static Utf8String GenerateResultJson(std::vector<double>& results, double& median);
    static size_t CalculateNumberOfRepeats(double firstDuration);
    static const size_t MinimumRepeats = 5;
    static const size_t MaximumRepeats = 70;
    static const int DesiredNumberOfSecondsPerTest = 3;
};

END_BENTLEY_ECN_TEST_NAMESPACE
