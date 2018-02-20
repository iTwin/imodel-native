/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformTools/Example/SimpleRDSApiExample.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Windows.h>

#include <RealityPlatformTools/SimpleRDSApi.h>
#include <Bentley/BeFile.h>

#include <stdio.h>
#include <conio.h>
#include <iomanip>
#include <sstream>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

void Inspect()
    {
    bvector<ConnectedNavNode> rootNodes = bvector<ConnectedNavNode>();

    ConnectedResponse response = ConnectedNavNode::GetRootNodes(rootNodes);

    if(!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    ConnectedRealityData rd = ConnectedRealityData(rootNodes[0].GetInstanceId());

    std::cout << "RealityData name: " << rd.GetName() << std::endl;

    bvector<ConnectedRealityDataRelationshipPtr> relationships = bvector<ConnectedRealityDataRelationshipPtr>();
    response = ConnectedRealityDataRelationship::RetrieveAllForRDId(relationships, rd.GetIdentifier());

    std::cout << "has " << relationships.size() << " relationships" << std::endl;

    bvector<ConnectedNavNode> childNodes = bvector<ConnectedNavNode>();
    response = rootNodes[0].GetChildNodes(childNodes);

    if(!childNodes.empty())
        {
        if(childNodes[0].GetECClassName() == "Folder")
            {
            ConnectedRealityDataFolder folder = ConnectedRealityDataFolder(childNodes[0].GetInstanceId());
            std::cout << "folder name: " << folder.GetName() << std::endl;
            }
        else if (childNodes[0].GetECClassName() == "Document")
            {
            ConnectedRealityDataDocument doc = ConnectedRealityDataDocument(childNodes[0].GetInstanceId());
            std::cout << "document content type: " << doc.GetContentType() << std::endl;
            }
        }

    ConnectedRealityDataEnterpriseStat stats = ConnectedRealityDataEnterpriseStat();

    response = stats.GetStats();

    std::cout << "total size on server: " << stats.GetTotalSizeKB() << "KB" << std::endl;
    }

Utf8String CreateUpload()
    {
    //Creating a repo to upload.
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

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderUp");
    BeFileName::CreateNewDirectory(dummyRoot.c_str());

    BeFileName dummyFile = BeFileName(dummyRoot.c_str());
    dummyFile.AppendToPath(L"DummyRootDocument.json");

    FILE* pFile;
    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 5000000, SEEK_SET); //5Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummyFolder/
    dummyFile.AppendToPath(L"DummySubFolder");
    BeFileName::CreateNewDirectory(dummyFile.c_str());
    dummyFile.AppendToPath(L"smallfile1.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile2.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile3.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile4.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile5.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
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
    crd.SetOrganizationId("72adad30-c07c-465d-a1fe-2f2dfac950d7");
    crd.SetUltimateId("72adad30-c07c-465d-a1fe-2f2dfac950d8");
    crd.SetUltimateSite("72adad30-c07c-465d-a1fe-2f2dfac950d9");
    crd.SetContainerName("72adad30-c07c-465d-a1fe-2f2dfac950da");
    crd.SetDataLocationGuid("72adad30-c07c-465d-a1fe-2f2dfac950db");
    crd.SetDataset("SRTM1");
    crd.SetDescription("example upload for SimpleRDSApi");
    
    Utf8String empty = "";

    ConnectedResponse response = crd.Upload(dummyRoot, empty);

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return "";
        }

    return crd.GetIdentifier();
    }

void FolderOperations(Utf8String guid)
    {
    Utf8String folderGuid = Utf8PrintfString("%s/%s", guid, "DummySubFolder");
    ConnectedRealityDataFolder crdf = ConnectedRealityDataFolder(folderGuid);

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir);
    outDir.AppendToPath(L"SimpleRDSApiTestDirectory");
    WString directory(outDir);

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderDownFolder");

    crdf.Download(dummyRoot, folderGuid);

    folderGuid.append("2");
    ConnectedResponse response = crdf.Upload(dummyRoot, folderGuid);

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    BeFileName::EmptyAndRemoveDirectory(dummyRoot.c_str());
    }

void DocumentOperations(Utf8String guid)
    {
    Utf8String documentGuid = Utf8PrintfString("%s/%s", guid, "DummyRootDocument.json");
    ConnectedRealityDataDocument crdf = ConnectedRealityDataDocument(documentGuid);

    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir);
    outDir.AppendToPath(L"SimpleRDSApiTestDirectory");
    WString directory(outDir);

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderDownDocument");

    crdf.Download(dummyRoot, documentGuid);

    documentGuid.append("2");
    ConnectedResponse response = crdf.Upload(dummyRoot, documentGuid);

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    BeFileName::EmptyAndRemoveDirectory(dummyRoot.c_str());
    }

void DownloadModify(Utf8String guid)
    {
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    BeFileName outDir = BeFileName(exeDir);
    outDir.AppendToPath(L"SimpleRDSApiTestDirectory");
    WString directory(outDir);

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolderDown");

    ConnectedRealityData crd = ConnectedRealityData(guid);
    crd.Download(dummyRoot, guid);
    crd.SetName("SimpleModifiedExample (DELETE THIS)");
    ConnectedResponse response = crd.UpdateInfo();

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }

void Delete(Utf8String guid)
    {
    ConnectedRealityData crd = ConnectedRealityData(guid);
    crd.Delete();

    ConnectedRealityData crdTester = ConnectedRealityData(guid);
    if(!crdTester.GetName().empty())
        {
        std::cout << "Delete failed" << std::endl;
        return;
        }
    }

int main(int argc, char *argv[])
    {
    RDSRequestManager::Setup();

    Inspect();
    Utf8String guid = CreateUpload();
    if(guid.empty())
        return 1;
    FolderOperations(guid);
    DocumentOperations(guid);
    DownloadModify(guid);
    Delete(guid);

    return 0;
    }