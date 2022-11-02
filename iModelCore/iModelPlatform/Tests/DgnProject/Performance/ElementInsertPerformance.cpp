/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTestFixture.h"
#include "PerformanceElementsCRUDTests.h"
#include <array>

struct PerformanceElementsTests : PerformanceElementsCRUDTestFixture
    {
    enum class Op
        {
        Select,
        Insert,
        Update,
        Delete
        };

    static std::vector<Utf8String> GetClasses()
        {
        std::vector<Utf8String> classes;
        classes.push_back(PERF_TEST_PERFELEMENT_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
        return classes;
        }
    static std::array<int, 3> GetInitalInstanceCount()
        {
        return  {10000, 100000, 1000000};
        }
    static std::array<int, 3> GetOpCount() { return  {1000, 2000, 3000}; }
    void Execute(Op op)
        {
        //WaitForUserInput();
        for (int initalInstanceCount : GetInitalInstanceCount())
            {
            if (initalInstanceCount <= 0)
                continue;

            for (int opCount : GetOpCount())
                {
                if (opCount <= 0)
                    continue;

                for (Utf8StringCR perfClass : GetClasses())
                    {
                    if (perfClass.empty())
                        continue;

                    if (op == Op::Select)
                        ApiSelectTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else if (op == Op::Insert)
                        ApiInsertTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else if (op == Op::Update)
                        ApiUpdateTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else if (op == Op::Delete)
                        ApiDeleteTime(perfClass.c_str(), initalInstanceCount, opCount);
                    else
                        {
                        ASSERT_TRUE(false);
                        }
                    }
                }

            //printf("%s\n", GetDbSettings().c_str());
            }
        }
    };

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsInsert) { Execute(Op::Insert); }
/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsRead)   { Execute(Op::Select); }
/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsUpdate) { Execute(Op::Update); }
/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ElementsDelete) { Execute(Op::Delete); }

#define CACHE_SPILL Pragma ("cache_spill = off")
#define CACHE_SIZE  Pragma (SqlPrintfString ("cache_size = %d", 300000));

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsInsert)
    {
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Insert);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsRead)
    {
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Select);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsUpdate)
    {
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Update);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceElementsTests, ServerElementsDelete)
    {
    CACHE_SPILL;
    CACHE_SIZE;
    Execute(Op::Delete);
    }


/*---------------------------------------------------------------------------------**//**
// @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PerformanceFixtureCRUD : PerformanceElementsCRUDTestFixture
    {
    enum class Op
        {
        Select,
        Insert,
        Update,
        Delete
        };

    static std::vector<Utf8String> GetClasses()
        {
        std::vector<Utf8String> classes;
        classes.push_back(PERF_TEST_PERFELEMENT_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
        classes.push_back(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
        return classes;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/

    void Execute(Op op, int percentageOfInitialCount, int initalInstanceCount)
        {

        ASSERT_NE(0, initalInstanceCount) << "InitialCount should be greater than zero for the test to execute.";

        ASSERT_NE(0, percentageOfInitialCount) << "Percentage of the initial count should not be zero.";

        //Calculating the %age of the initial count to be used as OperationCount
        int opCount = initalInstanceCount * percentageOfInitialCount / 100;

        for (Utf8StringCR perfClass : GetClasses())
            {
            if (perfClass.empty())
                continue;

            if (op == Op::Select)
                ApiSelectTime(perfClass.c_str(), initalInstanceCount, opCount);
            else if (op == Op::Insert)
                ApiInsertTime(perfClass.c_str(), initalInstanceCount, opCount);
            else if (op == Op::Update)
                ApiUpdateTime(perfClass.c_str(), initalInstanceCount, opCount);
            else if (op == Op::Delete)
                ApiDeleteTime(perfClass.c_str(), initalInstanceCount, opCount);
            else
                {
                ASSERT_TRUE(false);
                }
            }

        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ParseJsonAndRunTest(BeFileName jsonFilePath, Op op)
        {
        BeFile file;
        if (BeFileStatus::Success != file.Open(jsonFilePath.c_str(), BeFileAccess::Read))
            return;

        ByteStream byteStream;
        if (BeFileStatus::Success != file.ReadEntireFile(byteStream))
            return;

        Utf8String json((Utf8CP)byteStream.GetData(), byteStream.GetSize());
        file.Close();

        Json::Reader reader;
        Json::Value value;

        if (reader.Parse(json, value))
            {
            auto requirements = value["CrudTestRequirement"];

            for (auto iter : requirements)
                {
                auto InitialCount = iter["InitialCount"].asInt();
                for (auto percentage : iter["InitialCountPercentage"])
                    {
                    switch (op) {

                        case Op::Insert:
                            Execute(Op::Insert, percentage.asInt(), InitialCount);
                            break;

                        case Op::Delete:
                            Execute(Op::Delete, percentage.asInt(), InitialCount);
                            break;

                        case Op::Select:
                            Execute(Op::Select, percentage.asInt(), InitialCount);
                            break;

                        case Op::Update:
                            Execute(Op::Update, percentage.asInt(), InitialCount);
                            break;

                        default:
                            printf("Invalid operation.");
                        }
                    }
                }
            }
        }

    };


/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsInsert)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Insert);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsDelete)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Delete);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsUpdate)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Update);
    }

/*---------------------------------------------------------------------------------**//**
// @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerformanceFixtureCRUD, ElementsSelect)
    {
    WString jsonFile;
    jsonFile.Sprintf(L"ElementCrud.json");

    BeFileName jsonFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(jsonFilePath);
    jsonFilePath.AppendToPath(L"PerformanceTestData");
    jsonFilePath.AppendToPath(jsonFile.c_str());
    ASSERT_TRUE(jsonFilePath.DoesPathExist());

    ParseJsonAndRunTest(jsonFilePath, Op::Select);
    }
