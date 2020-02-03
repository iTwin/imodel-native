/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Windows.h>

#include <RealityPlatformTools/SimpleRDSApi.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeTest.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "../Common/RealityModFrameworkTestsCommon.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            01/2018
//=====================================================================================
class SimpleRDSIntegrationTestFixture : public testing::Test
    {};

TEST_F(SimpleRDSIntegrationTestFixture, SimpleRDSIntegrationTest)
    {
    RDSRequestManager::Setup("https://dev-realitydataservices-eus.cloudapp.net/");

    bvector<ConnectedNavNode> rootNodes = bvector<ConnectedNavNode>();

    ConnectedResponse response = ConnectedNavNode::GetRootNodes(rootNodes);

    ASSERT_TRUE(response.simpleSuccess);

    ConnectedRealityDataEnterpriseStat stats = ConnectedRealityDataEnterpriseStat();

    response = stats.GetEnterpriseStats();

    ASSERT_TRUE(response.simpleSuccess);

    ASSERT_TRUE(stats.GetTotalSizeKB() > 0);
        
    //Creating a repo to upload./
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir);
    outDir.AppendToPath(L"SimpleRDSApiTestDirectory");
    WString directory(outDir);
    if (BeFileName::DoesPathExist(directory.c_str()))
        BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    BeFileName::CreateNewDirectory(directory.c_str());

    ASSERT_TRUE(BeFileName::DoesPathExist(directory.c_str()));

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderUp");
    BeFileName::CreateNewDirectory(dummyRoot.c_str());

    ASSERT_TRUE(BeFileName::DoesPathExist(dummyRoot.c_str()));

    BeFileName dummyFile = BeFileName(dummyRoot.c_str());
    dummyFile.AppendToPath(L"DummyRootDocument.json");

    FILE* pFile;
    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 5000000, SEEK_SET); //5Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummyFolder/
    dummyFile.AppendToPath(L"DummySubFolder");
    BeFileName::CreateNewDirectory(dummyFile.c_str());
    ASSERT_TRUE(BeFileName::DoesPathExist(dummyFile.c_str()));
    dummyFile.AppendToPath(L"smallfile1.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile2.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile3.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile4.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile5.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    ASSERT_TRUE(pFile != nullptr);
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    //the actual upload begins here
    ConnectedRealityData crd = ConnectedRealityData();
    crd.SetName("SimpleRDSApiExample (DELETE THIS)");
    crd.SetClassification(RealityDataBase::Classification::TERRAIN);
    crd.SetRealityDataType("S3MX");
    crd.SetVisibility(RealityDataBase::Visibility::ENTERPRISE);
    crd.SetRootDocument("DummyRootDocument.json");

    Utf8String empty = "";

    response = crd.Upload(dummyRoot, empty);

    Utf8String guid = crd.GetIdentifier();

    ASSERT_TRUE(!guid.empty());

    dummyRoot.PopDir();
    dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderDown");

    RealityDataRelationshipPtr relationship = RealityDataRelationship::Create();
    relationship->SetRealityDataId(crd.GetIdentifier());
    relationship->SetRelationType("CONNECT-Project");
    relationship->SetRelatedId("615f57e7-876e-46fc-ab11-79af30cae299");
    ConnectedRealityDataRelationship connectedRelationship(relationship);
    connectedRelationship.CreateOnServer();

    bvector<ConnectedRealityDataRelationshipPtr> relationships = bvector<ConnectedRealityDataRelationshipPtr>();
    response = ConnectedRealityDataRelationship::RetrieveAllForRDId(relationships, crd.GetIdentifier());

    ASSERT_TRUE(response.simpleSuccess);
    ASSERT_TRUE(relationships.size() > 0);
    ASSERT_TRUE(relationships[0]->GetRelatedId().Equals(connectedRelationship.GetRelatedId()));

    relationships = bvector<ConnectedRealityDataRelationshipPtr>();
    response = ConnectedRealityDataRelationship::RetrieveAllForProjectId(relationships, "615f57e7-876e-46fc-ab11-79af30cae299");

    ASSERT_TRUE(response.simpleSuccess);
    ASSERT_TRUE(!relationships.empty());
    ASSERT_TRUE((relationships[0])->GetRelatedId().Equals(connectedRelationship.GetRelatedId()));

    response = connectedRelationship.Delete();
    ASSERT_TRUE(response.simpleSuccess);

    relationships = bvector<ConnectedRealityDataRelationshipPtr>();
    response = ConnectedRealityDataRelationship::RetrieveAllForRDId(relationships, crd.GetIdentifier());

    ASSERT_TRUE(response.simpleSuccess);
    ASSERT_TRUE(relationships.empty());

    NavNode node = NavNode(rootNodes[0].GetSchemaName(), crd.GetIdentifier(), rootNodes[0].GetTypeSystem(), rootNodes[0].GetECClassName());
    ConnectedNavNode cNode = ConnectedNavNode(node);

    bvector<ConnectedNavNode> childNodes = bvector<ConnectedNavNode>();

    response = cNode.GetChildNodes(childNodes);

    ASSERT_TRUE(response.simpleSuccess);
    ASSERT_TRUE(childNodes.size() > 0);

    int testedTypes = 0;
    for (ConnectedNavNode node : childNodes)
        {
        if (node.GetECClassName() == "Folder" && !(testedTypes & 1))
            {
            ConnectedRealityDataFolder folder = ConnectedRealityDataFolder(node.GetInstanceId());
            ASSERT_TRUE(!folder.GetName().empty());
            dummyFile = BeFileName(dummyRoot.c_str());
            dummyFile.AppendToPath(L"DummyDownFolder");
            folder.Download(dummyFile, node.GetNavString());
            ASSERT_TRUE(BeFileName::DoesPathExist(dummyFile.c_str()));
            testedTypes |= 1;
            }
        else if (node.GetECClassName() == "Document" && !(testedTypes & 2))
            {
            ConnectedRealityDataDocument doc = ConnectedRealityDataDocument(node.GetInstanceId());
            ASSERT_TRUE(!doc.GetContentType().empty());
            dummyFile = BeFileName(dummyRoot.c_str());
            dummyFile.AppendToPath(L"DummyDownDocument.json");
            doc.Download(dummyFile, node.GetNavString());
            ASSERT_TRUE(BeFileName::DoesPathExist(dummyFile.c_str()));
            //doc.Upload()
            testedTypes |= 2;
            }
        }

    BeFileName::EmptyAndRemoveDirectory(dummyRoot.c_str());
    
    ConnectedRealityData crd2 = ConnectedRealityData(guid);
    crd2.Download(dummyRoot, guid);
    crd2.SetName("SimpleModifiedExample (DELETE THIS)");
    response = crd2.UpdateInfo();

    ASSERT_TRUE(response.simpleSuccess);

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);

    ConnectedRealityData crd3 = ConnectedRealityData(guid);
    crd3.Delete();

    ConnectedRealityData crdTester = ConnectedRealityData(guid);
    ASSERT_TRUE(crdTester.GetName().empty());
    }