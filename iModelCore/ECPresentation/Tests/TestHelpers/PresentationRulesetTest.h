/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include "IssueReporter.h"
#include <ECPresentation/ECPresentationManager.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PresentationRulesetTester
{
private:
    ECPresentationManager* m_presentationManager;
    ECDb* m_db;
    ECPresentation::Tests::IssueReporter m_issueReporter;

    //! Compare a node we read from the BIM file with the corresponding node in the JSON expected output file
    //! The last 3 arguments are used to recursively call the function on the node's children
    //! Any errors will be logged
    //! @param[in] node The node we read from the BIM file
    //! @param[in] tree The current array of node that contains our corresponding JSON node
    //! @param[in] index The index of the corresponding JSON node that we want to compare with node (the first argument)
    //! @param[in] presentationManager The presentationManager used to navigate the DgnDb
    //! @param[in] rulesetParams Params for the tested ruleset
    //! @return the number of errors encountered in the comparison. Any errors will be logged
    int CheckNode(NavNodeCR node, BeJsConst tree, int index, ECPresentationManager* presentationManager,
        RequestWithRulesetParams const& rulesetParams);

public:
    PresentationRulesetTester(BeTest::Host&, BeFileNameCR rulesetsDir);

    //! Construct a PresentationRulesetTest object with a given IIssueListener
    PresentationRulesetTester(BeTest::Host&, BeFileNameCR rulesetsDir, ECDbR db, ECPresentation::Tests::IIssueListener & listener);

    ~PresentationRulesetTester();

    //! Add an IssueListener to existing PresentationRulesetTest object
    //! Currently, only 1 listener is allowed. If the object does not have a listener, we add the listener and return SUCCESS.
    //! Otherwise, return ERROR.
    //! @param[in] listener The IIssueListener we want to add
    BentleyStatus AddIssueListener(ECPresentation::Tests::IIssueListener & listener);

    //! Apply the a Presentation Rule Set to the database, then compare the output with the
    //! expected output in JSON format
    //! @param[in] rulesetId The rulesetId used to find the Presentation Rule Set in the designated folder
    //! @param[in] expectedTreeFile The file containing expected output in JSON format. We will read the file
    //!              and compare the result after we apply the Presentation Ruleset
    //! @return the number of errors encountered during the comparison
    int ValidateTree(Utf8CP rulesetId, BeFileNameCR expectedTreeFile);

    //! Apply the a Presentation Rule Set to the database, then compare the output with the
    //! expected output in JSON format
    //! @param[in] rulesetId The rulesetId used to find the Presentation Rule Set in the designated folder
    //! @param[in] treeFile The expected output in JSON format
    //! @return the number of errors encountered during the comparison
    int ValidateTree(Utf8CP rulesetId, BeJsConst treeFile);

    //! Create a JSON file from a BeJsConst
    //! @param[in] jsonValue The BeJsConst to be read into the file
    //! @param[in] fileName The name of the file to be written to
    int GenerateJsonFile(BeJsConst jsonValue, BeFileNameCR fileName);

    //! Generates the JSON Value of a NavNode to be used in the creation of a JSON Value of an entire ruleset
    //! @param[in] node The NavNode that the output JSON Value is to be created from
    //! @param[in] options The current set of options which determine the state for the JSON Value to be based in
    void WriteCreateJsonNode(BeJsValue jsonNode, NavNodeCPtr node, RequestWithRulesetParams const& options);

    //! Create a JSON Value based on a given rulset
    //! @param[in] rulesetId The name of the rulseset to be used
    BeJsDocument ExportJson(Utf8CP rulesetId);

    //! Set ECDb to use for testing
    void SetECDb(ECDbR db);

    //! @return A reference to the private ECPresentationManager
    ECPresentationManager& GetPresentationManager() { return *m_presentationManager; }

    //! @return A reference to the private UserSettings
    IUserSettings& GetUserSettings(Utf8CP rulesetId) { return m_presentationManager->GetUserSettings(rulesetId); }

    //! @return A reference to the private ECDb
    ECDbR GetDb() {return *m_db;}

    //! @return A reference to the private IssueReporter
    ECPresentation::Tests::IssueReporter& GetIssueReporter() { return m_issueReporter; };

    //! Read json from a file, this method is similar to IJsonConfigurationReader::Read
    //! method in DgnClientFx/ConfigurableUI/Configurable.h
    BentleyStatus ReadJsonFromFile(BeFileNameCR configurationFilePath, BeJsDocument& configuration);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PresentationRulesetTest : ECPresentationTest
{
private:
    PresentationRulesetTester* m_rulesetTester;
    ECDb* m_connection;

private:
    void OpenConnection(Utf8CP datasetName);
    BeFileName GetDatasetPath(Utf8CP datasetName);
    BeFileName GetJsonFilePath(Utf8CP jsonFileName);

protected:
    //! @return A path to Datasets directory (Default returns Documents directory)
    virtual BeFileName _GetDatasetsDirectory();

    //! @return A path to Json files directory (Default returns Documents directory)
    virtual BeFileName _GetJsonFilesDirectory();

    //! @return A path to Rulesets directory (Default returns Documents directory)
    virtual BeFileName _GetRulesetsDirectory();

public:
    virtual void SetUp() override;
    virtual void TearDown() override;

    //! Apply the a Presentation Rule Set to the database, then compare the output with the
    //! expected output in JSON format
    //! @param[in] rulesetId The rulesetId used to find the Presentation Rule Set in the designated folder
    //! @param[in] datasetName The datasetName used to find the Dataset in designated folder
    //! @param[in] jsonFileName The jsonFileName used to find the Json file containing expected output in JSON format.
    //!            We will read the file and compare the result after we apply the Presentation Ruleset
    //! @return the number of errors encountered during the comparison
    int ValidateTree(Utf8CP rulesetId, Utf8CP datasetName, Utf8CP jsonFileName);

    //! Apply the a Presentation Rule Set to the database, then compare the output with the
    //! expected output in JSON format
    //! @param[in] rulesetId The rulesetId used to find the Presentation Rule Set in the designated folder
    //! @param[in] datasetName The datasetName used to find the Dataset in designated folder
    //! @param[in] treeFile The expected output in JSON format
    //! @return the number of errors encountered during the comparison
    int ValidateTree(Utf8CP rulesetId, Utf8CP datasetName, BeJsConst treeFile);

    //! Gets the UserSettingsManager for a given ruleset
    //! @param[in] rulesetId The name of the rulseset to be used
    IUserSettings& GetUserSettings(Utf8CP rulesetId) { return m_rulesetTester->GetPresentationManager().GetUserSettings(rulesetId); }
};

END_ECPRESENTATIONTESTS_NAMESPACE
