/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeDebugLog.h>
#include <UnitTests/BackDoor/ECPresentation/PresentationRulesetTest.h>
#include "../NonPublished/RulesEngine/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//=======================================================================================
//! Used to build and open a Bim file if it is not yet initialized
//! Sets up a PresentationRulesetTester pointer for use in tests
// @bsiclass                                               Emily.Pazienza        10/2016
//+===============+===============+===============+===============+===============+======
struct PresentationTestingUnitTests : ECPresentationTest, IIssueListener
    {
    static ECDbTestProject* s_project;
    PresentationRulesetTester* m_tester;
    bvector<Utf8String> errorVector;
    
    void _OnIssueReported(Utf8CP message) override
        {
        errorVector.push_back(message);
        }

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("PresentationTestingUnitTests");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        BeFileName rulesetsDir;
        BeTest::GetHost().GetDocumentsRoot(rulesetsDir);
        rulesetsDir.AppendToPath(L"ECPresentationTestData");

        m_tester = new PresentationRulesetTester(BeTest::GetHost(), rulesetsDir);
        m_tester->SetECDb(s_project->GetECDb());
        m_tester->GetIssueReporter().AddListener(*this);
        }

    void TearDown() override
        {
        delete m_tester;
        }
    };
ECDbTestProject* PresentationTestingUnitTests::s_project = nullptr;

//---------------------------------------------------------------------------------------
//! Control test to confirm that the original JSON file is correct and passes the 
//! ValidateTree test
//! @remarks This test should always pass if the JSON file is correct and is read into a 
//!          JSON value correctly
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, PassingTest)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_EQ(0, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Completely wipes out the JSON value, leaving it with no actual data
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, ClearFile)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    file.clear();

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Different number of root nodes. jsonNodes: 0, DgnDb nodes: 2 \nExpected nodes: \n", errorVector[0].c_str());
    EXPECT_STREQ("\tN/A\n", errorVector[1].c_str());
    EXPECT_STREQ("Actual nodes: \n", errorVector[2].c_str());
    EXPECT_STREQ("\tRootNode \n", errorVector[3].c_str());
    EXPECT_STREQ("\tClassNode \n", errorVector[4].c_str());

    EXPECT_EQ(5, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Removes the first node in the "nodes" array of a JSON value
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, RemoveFirstNode)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    file["nodes"].removeIndex(0);

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Different number of root nodes. jsonNodes: 1, DgnDb nodes: 2 \nExpected nodes: \n", errorVector[0].c_str());
    EXPECT_STREQ("\tClassNode \n", errorVector[1].c_str());
    EXPECT_STREQ("Actual nodes: \n", errorVector[2].c_str());
    EXPECT_STREQ("\tRootNode \n", errorVector[3].c_str());
    EXPECT_STREQ("\tClassNode \n", errorVector[4].c_str());

    EXPECT_EQ(5, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Changes the "Label" property of the first node in a JSON value's "nodes" array to an 
//! incorrect value, but one that is still the correct type
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, WrongNodeLabel)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    file["nodes"][0]["Label"] = "WRONG";

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Labels do not match. Expected: WRONG, Actual: RootNode\n", errorVector[0].c_str());

    EXPECT_EQ(1, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Swaps the values of the "Type" and "Label" properties for the first node in a JSON 
//! value's "nodes" array
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, SwapVals)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    Json::Value temp = file["nodes"][0]["Type"];

    file["nodes"][0]["Type"] = file["nodes"][0]["Label"];
    file["nodes"][0]["Label"] = temp;

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Types do not match. Expected: RootNode, Actual: NODE_TYPE_RootTestNode\n", errorVector[0].c_str());
    EXPECT_STREQ("Error validating tree: Labels do not match. Expected: NODE_TYPE_RootTestNode, Actual: RootNode\n", errorVector[1].c_str());

    EXPECT_EQ(2, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Swaps the "Type" property between the first and second node's in a JSON file's 
//! "nodes" array if the array has more than one node
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value or
//!          if the array has only one node
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, SwapNodesTypes)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));


    if (file["nodes"].size() >= 1)
        {
        Json::Value temp = file["nodes"][0]["Type"];

        file["nodes"][0]["Type"] = file["nodes"][1]["Type"];
        file["nodes"][1]["Type"] = temp;

        m_tester->ValidateTree("CustomNodeTesting", file);

        EXPECT_STREQ("Error validating tree: Types do not match. Expected: NODE_TYPE_ClassTestNode, Actual: NODE_TYPE_RootTestNode\n", errorVector[0].c_str());
        EXPECT_STREQ("Error validating tree: Types do not match. Expected: NODE_TYPE_RootTestNode, Actual: NODE_TYPE_ClassTestNode\n", errorVector[1].c_str());

        EXPECT_EQ(2, errorVector.size());
        }
    }

//---------------------------------------------------------------------------------------
//! Deletes the "Type" member for the first node in a JSON value's "nodes" array
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, NoType)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    file["nodes"][0].removeMember("Type");

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Types do not match. Expected: , Actual: NODE_TYPE_RootTestNode\n", errorVector[0].c_str());

    EXPECT_EQ(1, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Removes the "ChildNode" member from the first node in a JSON value's "nodes" array
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, NoChildNodes)
    {
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    file["nodes"][0].removeMember("ChildNodes");

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Different number of child nodes. jsonNodes: 0, DgnDb nodes: 3 \nExpected nodes: \n", errorVector[0].c_str());
    EXPECT_STREQ("\tN/A\n", errorVector[1].c_str());
    EXPECT_STREQ("Actual nodes: \n", errorVector[2].c_str());
    EXPECT_STREQ("\tChildNode \n", errorVector[3].c_str());
    EXPECT_STREQ("\tElementNode \n", errorVector[4].c_str());
    EXPECT_STREQ("\tBottomNode \n", errorVector[5].c_str());

    EXPECT_EQ(6, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Removes all child node's from the first node in a JSON value's "nodes" array
//! @remarks This test passes if ValidateTree catches the error(s) in the JSON value 
// @betest                                                      Emily.Pazienza   10/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, ClearChildren)
    {
    //Clears all children from the first node
    BeFileName filepath;
    BeTest::GetHost().GetDocumentsRoot(filepath);
    filepath.AppendToPath(L"ECPresentationTestData");
    filepath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value file;
    ASSERT_EQ(SUCCESS,m_tester->ReadJsonFromFile(filepath, file));

    int size = file["nodes"][0]["ChildNodes"].size();

    for (int i = 0; i <= size; i++)
        {
        file["nodes"][0]["ChildNodes"].removeIndex(0);
        }

    m_tester->ValidateTree("CustomNodeTesting", file);

    EXPECT_STREQ("Error validating tree: Different number of child nodes. jsonNodes: 0, DgnDb nodes: 3 \nExpected nodes: \n", errorVector[0].c_str());
    EXPECT_STREQ("\tN/A\n", errorVector[1].c_str());
    EXPECT_STREQ("Actual nodes: \n", errorVector[2].c_str());
    EXPECT_STREQ("\tChildNode \n", errorVector[3].c_str());
    EXPECT_STREQ("\tElementNode \n", errorVector[4].c_str());
    EXPECT_STREQ("\tBottomNode \n", errorVector[5].c_str());

    EXPECT_EQ(6, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Test if the ExportJson function creates the correct Json representation of a ruleset
// @betest                                                      David.Le  11/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, ExportJsonTest)
    {
    Json::Value exportedJson = m_tester->ExportJson("CustomNodeTesting");
    m_tester->ValidateTree("CustomNodeTesting", exportedJson);
    EXPECT_EQ(0, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Creates a JSON Value based on a supplied JSON file, generates a new JSON file based 
//! on that same JSON Value, and then validates that the new generated file matches the 
//! tree generated by the ruleset
// @betest                                                      Emily.Pazienza   11/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, GenerateJsonFile)
    {
    BeFileName outputJsonFile;
    BeTest::GetHost().GetOutputRoot(outputJsonFile);
    outputJsonFile.AppendToPath(L"Testing.json");

    BeFileName inputFilePath;
    BeTest::GetHost().GetDocumentsRoot(inputFilePath);
    inputFilePath.AppendToPath(L"ECPresentationTestData");
    inputFilePath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value testValue;
    m_tester->ReadJsonFromFile(inputFilePath, testValue);

    m_tester->GenerateJsonFile(testValue, outputJsonFile);
    m_tester->ValidateTree("CustomNodeTesting", outputJsonFile);

    EXPECT_EQ(0, errorVector.size());
    }

//---------------------------------------------------------------------------------------
//! Creates a JSON Value based on a given ruleset, generates a JSON file based on that 
//! Value, and then tries to validate that file against a different ruleset. This test 
//! should pass if there are errors generated by comparing an inaccurate file to a given 
//! ruleset
// @betest                                                      Emily.Pazienza   11/2016
//---------------------------------------------------------------------------------------
TEST_F(PresentationTestingUnitTests, IncorrectJsonFile)
    {
    BeFileName outputJsonFile;
    BeTest::GetHost().GetOutputRoot(outputJsonFile);
    outputJsonFile.AppendToPath(L"IncorrectTesting.json");

    BeFileName inputFilePath;
    BeTest::GetHost().GetDocumentsRoot(inputFilePath);
    inputFilePath.AppendToPath(L"ECPresentationTestData");
    inputFilePath.AppendToPath(L"TestingRulesetNodes.json");

    Json::Value testValue;
    m_tester->ReadJsonFromFile(inputFilePath, testValue);

    m_tester->GenerateJsonFile(testValue, outputJsonFile);
    m_tester->ValidateTree("ClassesDiagram", outputJsonFile);

    EXPECT_STREQ("Error validating tree: Different number of root nodes. jsonNodes: 2, DgnDb nodes: 0 \nExpected nodes: \n", errorVector[0].c_str());
    EXPECT_STREQ("\tRootNode \n", errorVector[1].c_str());
    EXPECT_STREQ("\tClassNode \n", errorVector[2].c_str());
    EXPECT_STREQ("Actual nodes: \n", errorVector[3].c_str());
    EXPECT_STREQ("\tN/A\n", errorVector[4].c_str());

    EXPECT_EQ(5,errorVector.size());
    }