/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/writer.h>
#include "PresentationRulesetTest.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//------------------------------------------------------------------------------------------
// @bsimethod                                    David.Le                          09/2016
//------------------------------------------------------------------------------------------
BentleyStatus PresentationRulesetTester::ReadJsonFromFile(BeFileNameCR configurationFilePath, JsonValueR configuration)
    {
    // make sure the config file exists
    if (!configurationFilePath.DoesPathExist())
        {
        GetIssueReporter().Report("Error reading JSON from file: The indicated file path, %s, does not exist\n", configurationFilePath.GetNameUtf8().c_str());
        return ERROR;
        }

    // open the configuration file
    BeFile configFile;
    if (BeFileStatus::Success != configFile.Open(configurationFilePath.c_str(), BeFileAccess::Read))
        {
        GetIssueReporter().Report("Error reading JSON from file: Failed to open configuration file %s\n", configurationFilePath.c_str());
        return ERROR;
        }

    bvector<Byte> data;
    if (BeFileStatus::Success != configFile.ReadEntireFile(data))
        {
        GetIssueReporter().Report("Error reading JSON from file: Failed to read entire file\n");
        return ERROR;
        }

    data.push_back(0);

    // parse the json
    Json::Reader reader;
    bool result = reader.parse((Utf8P)&data[0], configuration);
    if (!result)
        {
        GetIssueReporter().Report("Error reading JSON from file: Failed to parse JSON file: %s\n", configurationFilePath.GetNameUtf8().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   David.Le                          09/2016
//----------------------------------------------------------------------------------------
int PresentationRulesetTester::CheckNode(NavNodeCR node, JsonValueCR tree, int index, RulesDrivenECPresentationManager* m_presentationManager,
    RulesDrivenECPresentationManager::NavigationOptions simpleTest)
    {
    Json::Value jsonValue = tree[index];
    int errorCount = 0;
    bool nodesMatch = true;
        
    if (!node.GetType().Equals(jsonValue.get("Type", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: Types do not match. Expected: %s, Actual: %s\n", jsonValue.get("Type", Json::Value::GetNull()).asString().c_str(), node.GetType().c_str());
        errorCount++;
        }

    if (!node.GetLabel().Equals(jsonValue.get("Label", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: Labels do not match. Expected: %s, Actual: %s\n", jsonValue.get("Label", Json::Value::GetNull()).asString().c_str(), node.GetLabel().c_str());
        errorCount++;
        }
        
    if (!node.GetDescription().Equals(jsonValue.get("Description", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: Descriptions do not match. Expected: %s, Actual: %s\n", jsonValue.get("Description", Json::Value::GetNull()).asString().c_str(), node.GetDescription().c_str());
        errorCount++;
        }

    if (!node.GetExpandedImageId().Equals(jsonValue.get("ImageId", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: ImageIds do not match. Expected: %s, Actual: %s\n", jsonValue.get("ImageId", Json::Value::GetNull()).asString().c_str(), node.GetExpandedImageId().c_str());
        errorCount++;
        }


    if (!node.HasChildren())
        {
        Json::Value childArray = jsonValue.get("ChildNodes", Json::Value::GetNull());
        auto jsonChildSize = childArray.size();
        if (0 != jsonChildSize)
            {
            GetIssueReporter().Report("Error validating tree: The Node read from the DgnDb does not have children, but the Json Node does, the Json node is: %s, DgnDb Node is: %s\n",
                       jsonValue.get("Type", Json::Value::GetNull()).asString().c_str(), node.GetType().c_str());
            errorCount++;
            }
        }
    else
        {
        Json::Value childArray;
        childArray = jsonValue.get("ChildNodes", Json::Value::GetNull());
        int childIndex = 0;
        auto childList = m_presentationManager->GetChildren(GetDb(), node, PageOptions(), simpleTest.GetJson()).get();

        auto jsonChildSize = childArray.size();
        auto nodeChildSize = childList.GetSize();
        if (jsonChildSize != nodeChildSize)
            {
            GetIssueReporter().Report("Error validating tree: Different number of child nodes. jsonNodes: %d, DgnDb nodes: %d \nExpected nodes: \n", static_cast<int>(jsonChildSize), static_cast<int>(nodeChildSize));
            for (unsigned int index = 0; index < jsonChildSize; index++)
                {
                GetIssueReporter().Report("\t%s \n", childArray[index].get("Label", Json::Value::GetNull()).asString().c_str());
                }
            if (0 == jsonChildSize)
                GetIssueReporter().Report("\tN/A\n");

            GetIssueReporter().Report("Actual nodes: \n");
            for (NavNodeCPtr rootNode : childList)
                {
                GetIssueReporter().Report("\t%s \n", (*rootNode).GetLabel().c_str());
                }
            if (0 == nodeChildSize)
                GetIssueReporter().Report("\tN/A\n");
                        
            errorCount++;
            nodesMatch = false;
            }
        for (NavNodeCPtr childNode : childList)
            {
            if (nodesMatch)
                errorCount += CheckNode(*childNode, childArray, childIndex, m_presentationManager, simpleTest);
            childIndex++;
            }
        }
    return errorCount;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Goehrig                    09/2016
//---------------------------------------------------------------------------------------
PresentationRulesetTester::PresentationRulesetTester(BeTest::Host& host, BeFileNameCR rulesetsDir)
    {    
    BeFileName assetsDirectory, temporaryDirectory;
    host.GetDgnPlatformAssetsDirectory(assetsDirectory);
    host.GetTempDir(temporaryDirectory);
    RulesDrivenECPresentationManager::Paths paths(assetsDirectory, temporaryDirectory);
    
    RulesDrivenECPresentationManager::Params params(paths);
    params.SetLocalState(&m_localState);
    m_presentationManager = new RulesDrivenECPresentationManager(params);
    m_presentationManager->GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(rulesetsDir.GetNameUtf8().c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David.Le                        10/2016
//---------------------------------------------------------------------------------------
PresentationRulesetTester::PresentationRulesetTester(BeTest::Host& host, BeFileNameCR rulesetsDir, ECDbR db, IIssueListener& listener)
    : PresentationRulesetTester(host, rulesetsDir)
    {
    AddIssueListener(listener);
    SetECDb(db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Goehrig                    11/2016
//---------------------------------------------------------------------------------------
PresentationRulesetTester::~PresentationRulesetTester()
    {
    DELETE_AND_CLEAR(m_presentationManager);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David.Le                        10/2016
//---------------------------------------------------------------------------------------
BentleyStatus PresentationRulesetTester::AddIssueListener(IIssueListener & listener)
    {
    return m_issueReporter.AddListener(listener);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Saulius.Skliutas                09/2016
//---------------------------------------------------------------------------------------
int PresentationRulesetTester::ValidateTree(Utf8CP rulesetId, BeFileNameCR expectedTreeFile)
    {
    Json::Value correctOutput;
    if (SUCCESS != ReadJsonFromFile(expectedTreeFile, correctOutput))
        {
        BeTest::Fail(L"ReadJsonFromFile failed");
        return 1;
        }
    return ValidateTree(rulesetId, correctOutput);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Emily.Pazienza                  10/2016
//---------------------------------------------------------------------------------------
int PresentationRulesetTester::ValidateTree(Utf8CP rulesetId, JsonValueCR treeFile)
    {
    RulesDrivenECPresentationManager::NavigationOptions options(rulesetId, RuleTargetTree::TargetTree_MainTree);
    JsonValueCR jsonNodes = treeFile.get("nodes", Json::Value::GetNull());

    StopWatch timer(nullptr, true);
    auto roots = m_presentationManager->GetRootNodes(GetDb(), PageOptions(), options.GetJson()).get();
    int jsonIndex = 0;
    int errorCount = 0;
    bool nodesMatch = true;

    size_t rootCount = roots.GetSize();
    auto jsonRootCount = jsonNodes.size();
    if (jsonRootCount != rootCount)
        {
        GetIssueReporter().Report("Error validating tree: Different number of root nodes. jsonNodes: %d, DgnDb nodes: %d \nExpected nodes: \n", static_cast<int>(jsonRootCount), static_cast<int>(rootCount));
        for (unsigned int index = 0; index < jsonNodes.size(); index++)
            {
            GetIssueReporter().Report("\t%s \n", jsonNodes[index].get("Label", Json::Value::GetNull()).asString().c_str());
            }
        if (jsonRootCount == 0)
            GetIssueReporter().Report("\tN/A\n");

        GetIssueReporter().Report("Actual nodes: \n");
        for (NavNodeCPtr rootNode : roots)
            {
            GetIssueReporter().Report("\t%s \n", (*rootNode).GetLabel().c_str());
            }
        if (rootCount == 0)
            GetIssueReporter().Report("\tN/A\n");
        errorCount++;
        nodesMatch = false;
        }

    if (nodesMatch)
        {
        for (NavNodeCPtr rootNode : roots)
            {
            errorCount += CheckNode(*rootNode, jsonNodes, jsonIndex, m_presentationManager, options);
            jsonIndex++;
            }
        }
    timer.Stop();
    printf("Creating hierarchy for ruleset: %s, took: %f\n", rulesetId, timer.GetElapsedSeconds());
    return errorCount;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Emily.Pazienza                    11/2016
//----------------------------------------------------------------------------------------
int PresentationRulesetTester::GenerateJsonFile(JsonValueCR jsonValue, BeFileNameCR fileName)
    {
    BeFile jsonFile;
    Json::StyledWriter jsonWriter;
    Utf8String jsonString = jsonWriter.write(jsonValue);

    if (BeFileStatus::Success != jsonFile.Create(fileName))
        {
        GetIssueReporter().Report("Unable to create json file.\n");
        return 1;
        }
    
    int byteSize = static_cast<int>(jsonString.SizeInBytes());

    if (BeFileStatus::Success != jsonFile.Write(NULL, jsonString.c_str(), byteSize))
        {
        GetIssueReporter().Report("Unable to write to json file.\n");
        return 1;
        }

    if (BeFileStatus::Success != jsonFile.Close())
        {
        GetIssueReporter().Report("Unable to close json file.\n");
        return 1;
        }

    return 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   David.Le                          11/2016
//----------------------------------------------------------------------------------------
Json::Value PresentationRulesetTester::CreateJsonNode(NavNodeCPtr node, RulesDrivenECPresentationManager::NavigationOptions options)
    {
    Json::Value jsonNode(Json::objectValue);
    jsonNode["Type"] = node->GetType();
    jsonNode["Label"] = node->GetLabel();
    jsonNode["Description"] = node->GetDescription();
    jsonNode["ImageId"] = node->GetExpandedImageId();
    if (node->HasChildren())
        {
        auto childList = m_presentationManager->GetChildren(GetDb(), *node, PageOptions(), options.GetJson()).get();
        Json::Value childJsonList(Json::arrayValue);
        for (auto childNode : childList)
            {
            childJsonList.append(CreateJsonNode(childNode, options));
            }
        jsonNode["ChildNodes"] = childJsonList;
        }
    return jsonNode;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   David.Le                          11/2016
//----------------------------------------------------------------------------------------
Json::Value PresentationRulesetTester::ExportJson(Utf8CP rulesetId)
    {
    Json::Value outputJson(Json::objectValue);
    outputJson["nodes"] = Json::Value(Json::arrayValue);
    RulesDrivenECPresentationManager::NavigationOptions options(rulesetId, RuleTargetTree::TargetTree_MainTree);
    auto roots = m_presentationManager->GetRootNodes(GetDb(), PageOptions(), options.GetJson()).get();
    for (auto rootNode : roots)
        {
        outputJson["nodes"].append(CreateJsonNode(rootNode, options));
        }
    return outputJson;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
void PresentationRulesetTester::SetECDb(ECDbR db)
    {
    m_db = &db;
    m_presentationManager->GetConnections().CreateConnection(db);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
void PresentationRulesetTest::SetUp()
    {
    ECPresentationTest::SetUp();
    m_rulesetTester = new PresentationRulesetTester(BeTest::GetHost(), _GetRulesetsDirectory());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
void PresentationRulesetTest::TearDown()
    {
    delete m_rulesetTester;
    if (nullptr != m_connection)
        delete m_connection;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
void PresentationRulesetTest::OpenConnection(Utf8CP datasetName)
    {
    BeFileName path = GetDatasetPath(datasetName);
    if (!path.DoesPathExist())
        {
        BeAssert(false);
        return;
        }

    m_connection = new ECDb();
    if (BeSQLite::DbResult::BE_SQLITE_OK != m_connection->OpenBeSQLiteDb(path, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)))
        {
        BeAssert(false);
        delete m_connection;
        return;
        }

    m_rulesetTester->SetECDb(*m_connection);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
int PresentationRulesetTest::ValidateTree(Utf8CP rulesetId, Utf8CP datasetName, Utf8CP jsonFileName)
    {
    OpenConnection(datasetName);
    return m_rulesetTester->ValidateTree(rulesetId, GetJsonFilePath(jsonFileName));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
int PresentationRulesetTest::ValidateTree(Utf8CP rulesetId, Utf8CP datasetName, JsonValueCR expectedTree)
    {
    OpenConnection(datasetName);
    return m_rulesetTester->ValidateTree(rulesetId, expectedTree);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::_GetDatasetsDirectory()
    {
    BeFileName directory;
    BeTest::GetHost().GetDocumentsRoot(directory);
    return directory;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::_GetJsonFilesDirectory()
    {
    BeFileName directory;
    BeTest::GetHost().GetDocumentsRoot(directory);
    return directory;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::_GetRulesetsDirectory()
    {
    BeFileName directory;
    BeTest::GetHost().GetDocumentsRoot(directory);
    return directory;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::GetDatasetPath(Utf8CP datasetName)
    {
    BeFileName path = _GetDatasetsDirectory();
    path.AppendToPath(WString(datasetName, BentleyCharEncoding::Utf8).c_str());
    path.AppendExtension(L"bim");
    return path;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Saulius.Skliutas                  06/2017
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::GetJsonFilePath(Utf8CP jsonFileName)
    {
    BeFileName path = _GetJsonFilesDirectory();
    path.AppendToPath(WString(jsonFileName, BentleyCharEncoding::Utf8).c_str());
    path.AppendExtension(L"json");
    return path;
    }
