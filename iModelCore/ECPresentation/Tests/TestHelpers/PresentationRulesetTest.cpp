/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Logging.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/writer.h>
#include "PresentationRulesetTest.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

//------------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//----------------------------------------------------------------------------------------
int PresentationRulesetTester::CheckNode(NavNodeCR node, JsonValueCR tree, int index, ECPresentationManager* presentationManager,
    RequestWithRulesetParams const& rulesetParams)
    {
    Json::Value jsonValue = tree[index];
    int errorCount = 0;
    bool nodesMatch = true;

    if (!node.GetType().Equals(jsonValue.get("Type", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: Types do not match. Expected: %s, Actual: %s\n", jsonValue.get("Type", Json::Value::GetNull()).asString().c_str(), node.GetType().c_str());
        errorCount++;
        }

    if (!node.GetLabelDefinition().GetDisplayValue().Equals(jsonValue.get("Label", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: Labels do not match. Expected: %s, Actual: %s\n", jsonValue.get("Label", Json::Value::GetNull()).asString().c_str(), node.GetLabelDefinition().GetDisplayValue().c_str());
        errorCount++;
        }

    if (!node.GetDescription().Equals(jsonValue.get("Description", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: Descriptions do not match. Expected: %s, Actual: %s\n", jsonValue.get("Description", Json::Value::GetNull()).asString().c_str(), node.GetDescription().c_str());
        errorCount++;
        }

    if (!node.GetImageId().Equals(jsonValue.get("ImageId", Json::Value::GetNull()).asString()))
        {
        GetIssueReporter().Report("Error validating tree: ImageIds do not match. Expected: %s, Actual: %s\n", jsonValue.get("ImageId", Json::Value::GetNull()).asString().c_str(), node.GetImageId().c_str());
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
        Json::Value childArray = jsonValue.get("ChildNodes", Json::Value::GetNull());
        auto childListResponse = presentationManager->GetNodes(AsyncHierarchyRequestParams::Create(GetDb(), rulesetParams, &node)).get();
        auto const& childList = *childListResponse;
        auto jsonChildSize = childArray.size();
        auto nodeChildSize = childList.GetSize();
        if (jsonChildSize != nodeChildSize)
            {
            GetIssueReporter().Report("Error validating tree: Different number of child nodes. jsonNodes: %d, DgnDb nodes: %d \nExpected nodes: \n", static_cast<int>(jsonChildSize), static_cast<int>(nodeChildSize));
            for (Json::ArrayIndex jsonChildIndex = 0; jsonChildIndex < jsonChildSize; jsonChildIndex++)
                {
                GetIssueReporter().Report("\t%s \n", childArray[jsonChildIndex].get("Label", Json::Value::GetNull()).asString().c_str());
                }
            if (0 == jsonChildSize)
                GetIssueReporter().Report("\tN/A\n");

            GetIssueReporter().Report("Actual nodes: \n");
            for (NavNodeCPtr rootNode : childList)
                {
                GetIssueReporter().Report("\t%s \n", (*rootNode).GetLabelDefinition().GetDisplayValue().c_str());
                }
            if (0 == nodeChildSize)
                GetIssueReporter().Report("\tN/A\n");

            errorCount++;
            nodesMatch = false;
            }
        int childIndex = 0;
        for (NavNodeCPtr childNode : childList)
            {
            if (nodesMatch)
                errorCount += CheckNode(*childNode, childArray, childIndex, presentationManager, rulesetParams);
            childIndex++;
            }
        }
    return errorCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PresentationRulesetTester::PresentationRulesetTester(BeTest::Host& host, BeFileNameCR rulesetsDir)
    {
    BeFileName assetsDirectory, temporaryDirectory;
    host.GetDgnPlatformAssetsDirectory(assetsDirectory);
    host.GetTempDir(temporaryDirectory);
    ECPresentationManager::Paths paths(assetsDirectory, temporaryDirectory);

    ECPresentationManager::Params params(paths);
    params.SetLocalState(&m_localState);
    m_presentationManager = new ECPresentationManager(params);
    m_presentationManager->GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(rulesetsDir.GetNameUtf8().c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PresentationRulesetTester::PresentationRulesetTester(BeTest::Host& host, BeFileNameCR rulesetsDir, ECDbR db, ECPresentation::Tests::IIssueListener& listener)
    : PresentationRulesetTester(host, rulesetsDir)
    {
    AddIssueListener(listener);
    SetECDb(db);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PresentationRulesetTester::~PresentationRulesetTester()
    {
    DELETE_AND_CLEAR(m_presentationManager);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus PresentationRulesetTester::AddIssueListener(ECPresentation::Tests::IIssueListener & listener)
    {
    return m_issueReporter.AddListener(listener);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
int PresentationRulesetTester::ValidateTree(Utf8CP rulesetId, JsonValueCR treeFile)
    {
    JsonValueCR jsonNodes = treeFile.get("nodes", Json::Value::GetNull());

    StopWatch timer(nullptr, true);
    RequestWithRulesetParams rulesetParams(rulesetId, RulesetVariables());
    auto rootsResponse = m_presentationManager->GetNodes(AsyncHierarchyRequestParams::Create(GetDb(), rulesetParams)).get();
    auto const& roots = *rootsResponse;
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
            GetIssueReporter().Report("\t%s \n", (*rootNode).GetLabelDefinition().GetDisplayValue().c_str());
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
            errorCount += CheckNode(*rootNode, jsonNodes, jsonIndex, m_presentationManager, rulesetParams);
            jsonIndex++;
            }
        }
    timer.Stop();
    printf("Creating hierarchy for ruleset: %s, took: %f\n", rulesetId, timer.GetElapsedSeconds());
    return errorCount;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//----------------------------------------------------------------------------------------
Json::Value PresentationRulesetTester::CreateJsonNode(NavNodeCPtr node, RequestWithRulesetParams const& params)
    {
    Json::Value jsonNode(Json::objectValue);
    jsonNode["Type"] = node->GetType();
    jsonNode["Label"] = node->GetLabelDefinition().GetDisplayValue();
    jsonNode["Description"] = node->GetDescription();
    jsonNode["ImageId"] = node->GetImageId();
    if (node->HasChildren())
        {
        auto childListResponse = m_presentationManager->GetNodes(AsyncHierarchyRequestParams::Create(GetDb(), HierarchyRequestParams(params, node.get()))).get();
        Json::Value childJsonList(Json::arrayValue);
        for (auto childNode : *childListResponse)
            {
            childJsonList.append(CreateJsonNode(childNode, params));
            }
        jsonNode["ChildNodes"] = childJsonList;
        }
    return jsonNode;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
Json::Value PresentationRulesetTester::ExportJson(Utf8CP rulesetId)
    {
    Json::Value outputJson(Json::objectValue);
    outputJson["nodes"] = Json::Value(Json::arrayValue);
    RequestWithRulesetParams rulesetParams(rulesetId, RulesetVariables());
    auto rootsResponse = m_presentationManager->GetNodes(AsyncHierarchyRequestParams::Create(GetDb(), rulesetParams)).get();
    for (auto rootNode : *rootsResponse)
        outputJson["nodes"].append(CreateJsonNode(rootNode, rulesetParams));
    return outputJson;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void PresentationRulesetTester::SetECDb(ECDbR db)
    {
    m_db = &db;
    m_presentationManager->GetConnections().CreateConnection(db);
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void PresentationRulesetTest::SetUp()
    {
    ECPresentationTest::SetUp();
    m_rulesetTester = new PresentationRulesetTester(BeTest::GetHost(), _GetRulesetsDirectory());
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void PresentationRulesetTest::TearDown()
    {
    delete m_rulesetTester;
    if (nullptr != m_connection)
        delete m_connection;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//----------------------------------------------------------------------------------------
int PresentationRulesetTest::ValidateTree(Utf8CP rulesetId, Utf8CP datasetName, Utf8CP jsonFileName)
    {
    OpenConnection(datasetName);
    return m_rulesetTester->ValidateTree(rulesetId, GetJsonFilePath(jsonFileName));
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
int PresentationRulesetTest::ValidateTree(Utf8CP rulesetId, Utf8CP datasetName, JsonValueCR expectedTree)
    {
    OpenConnection(datasetName);
    return m_rulesetTester->ValidateTree(rulesetId, expectedTree);
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::_GetDatasetsDirectory()
    {
    BeFileName directory;
    BeTest::GetHost().GetDocumentsRoot(directory);
    return directory;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::_GetJsonFilesDirectory()
    {
    BeFileName directory;
    BeTest::GetHost().GetDocumentsRoot(directory);
    return directory;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::_GetRulesetsDirectory()
    {
    BeFileName directory;
    BeTest::GetHost().GetDocumentsRoot(directory);
    return directory;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::GetDatasetPath(Utf8CP datasetName)
    {
    BeFileName path = _GetDatasetsDirectory();
    path.AppendToPath(WString(datasetName, BentleyCharEncoding::Utf8).c_str());
    path.AppendExtension(L"bim");
    return path;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
BeFileName PresentationRulesetTest::GetJsonFilePath(Utf8CP jsonFileName)
    {
    BeFileName path = _GetJsonFilesDirectory();
    path.AppendToPath(WString(jsonFileName, BentleyCharEncoding::Utf8).c_str());
    path.AppendExtension(L"json");
    return path;
    }
