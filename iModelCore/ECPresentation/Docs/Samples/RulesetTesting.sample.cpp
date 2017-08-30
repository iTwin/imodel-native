/*--------------------------------------------------------------------------------------+
|
|     $Source: Docs/Samples/RulesetTesting.sample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Testing_Include.sampleCode
// the required header file for working with PresentationRulesetTests
#include <UnitTests/BackDoor/ECPresentation/PresentationRulesetTest.h>

// the required namespace for PresentationTesting
USING_NAMESPACE_ECPRESENTATIONTESTS
//__PUBLISH_EXTRACT_END__

#define PATH_TO_JSON_FILE L""
static BeFileName rulesetsDirectory;
static BeSQLite::EC::ECDb ecdb;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Emily.Pazienza                   11/2016
//---------------------------------------------------------------------------------------
void setUp()
    {
    //__PUBLISH_EXTRACT_START__ Testing_Basics.sampleCode
    // constructs a PresentationRulesetTester object using the test host object, rulesets directory and the ECDb
    PresentationRulesetTester rulesetTester(BeTest::GetHost(), rulesetsDirectory);
    rulesetTester.SetECDb(ecdb);
    //__PUBLISH_EXTRACT_END__
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Emily.Pazienza                   10/2016
//---------------------------------------------------------------------------------------
void readJson()
    {
    PresentationRulesetTester rulesetTester(BeTest::GetHost(), rulesetsDirectory);
//__PUBLISH_EXTRACT_START__ Testing_ReadJson.sampleCode
    //Read the JSON from a file
    BeFileName filePath(PATH_TO_JSON_FILE);
    Json::Value treeFile;
    rulesetTester.ReadJsonFromFile(filePath, treeFile);

    //Read the JSON from a RulesetId
    Json::Value rulesetJsonValue = rulesetTester.ExportJson("RulesetId");
//__PUBLISH_EXTRACT_END__
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Emily.Pazienza                   10/2016
//---------------------------------------------------------------------------------------
void validate()
    {
    Json::Value treeFile;
    BeFileName treePath;
    PresentationRulesetTester rulesetTester(BeTest::GetHost(), rulesetsDirectory);
//__PUBLISH_EXTRACT_START__ Testing_Validate.sampleCode
    // checks that the Json Value file read from memory matches the expected values from the indicated Ruleset
    rulesetTester.ValidateTree("RulesetId", treeFile);

    // checks that the file designated by the filePath matches the expected values from the indicated Ruleset
    rulesetTester.ValidateTree("RulesetId", treePath);
//__PUBLISH_EXTRACT_END__
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Emily.Pazienza                   10/2016
//---------------------------------------------------------------------------------------
void userSettings()
    {
    int intValue = 0;
    PresentationRulesetTester rulesetTester(BeTest::GetHost(), rulesetsDirectory);
//__PUBLISH_EXTRACT_START__ Testing_UserSettings.sampleCode
    // sets the designated setting within the ruleset to a given value
    rulesetTester.GetUserSettings("RulesetId").SetSettingIntValue("SettingId", intValue);

    // gets a given value for the designated setting
    rulesetTester.GetUserSettings("RulesetId").GetSettingIntValue("SettingId");
//__PUBLISH_EXTRACT_END__
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Emily.Pazienza                   10/2016
//---------------------------------------------------------------------------------------
//__PUBLISH_EXTRACT_START__ Testing_Example.sampleCode
// Derive from PresentationRulesetTest and override directory path functions
struct PresentationTests : PresentationRulesetTest
    {
    // Override _GetDatasetsDirectory to return path to datasets directory
    BeFileName _GetDatasetsDirectory() override {return BeFileName();}

    // Override _GetRulesetsDirectory to return path to rulesets directory
    BeFileName _GetRulesetsDirectory() override {return BeFileName();}

    // Override _GetJsonFilesDirectory to return path to json files directory
    BeFileName _GetJsonFilesDirectory() override {return BeFileName();}
    };

// checks if the tree is correct and then ASSERTs that there were no errors
TEST_F(PresentationTests, ExampleTest)
    {
    int errorCount = ValidateTree("RulesetId", "DatasetName", "JsonFileName");
    ASSERT_EQ(0, errorCount);
    }
//__PUBLISH_EXTRACT_END__
