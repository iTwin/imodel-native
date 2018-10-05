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

    //get all rootNodes, every individual entry on the server
    ConnectedResponse response = ConnectedNavNode::GetRootNodes(rootNodes);

    if(!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    //create a ConnectedRealityData object, with an instanceId. This will fetch the properties of
    //the corresponding RealityData on the server, for that instanceId
    ConnectedRealityData rd = ConnectedRealityData(rootNodes[0].GetInstanceId());

    std::cout << "RealityData name: " << rd.GetName() << std::endl;

    //get a list of all relationships for a given RealityData, a list of every project that uses this RD
    bvector<ConnectedRealityDataRelationshipPtr> relationships = bvector<ConnectedRealityDataRelationshipPtr>();
    response = ConnectedRealityDataRelationship::RetrieveAllForRDId(relationships, rd.GetIdentifier());

    std::cout << "has " << relationships.size() << " relationships" << std::endl;

    //get child nodes for a rootNode. This is equivalent to entering a folder in a file explorer, it shows
    //any files or folders contained within the node
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
    //Get all server stats (size on server, number of entries, etc)
    response = stats.GetEnterpriseStats();

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
    //set a handful of properties that you want, for the upload
    crd.SetName("SimpleRDSApiExample (DELETE THIS)");
    crd.SetClassification(RealityDataBase::Classification::TERRAIN);
    crd.SetRealityDataType("S3MX");
    crd.SetVisibility(RealityDataBase::Visibility::ENTERPRISE);
    crd.SetRootDocument("DummyRootDocument.json");
    crd.SetDataset("SRTM1");
    crd.SetDescription("example upload for SimpleRDSApi");
    
    Utf8String empty = "";

    //upload the data at "dummy root" with the properties stored in "crd"
    ConnectedResponse response = crd.Upload(dummyRoot, empty);

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return "";
        }

    //if the upload was successful, "crd" will have been given a GUID, by the server
    return crd.GetIdentifier();
    }

void FolderOperations(Utf8String guid)
    {
    //create a local folder to receive the download
    Utf8String folderGuid = Utf8PrintfString("%s~2F%s", guid, "DummySubFolder");
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

    //download the folder at "folderGuid" to the local folder "dummyRoot"
    //"crdf" will have its properties filled by the server
    crdf.Download(dummyRoot, folderGuid);

    //reupload the folder from "dummyRoot" to the server path "folderGuid"
    ConnectedResponse response = crdf.Upload(dummyRoot, folderGuid);

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    //remove the local folder
    BeFileName::EmptyAndRemoveDirectory(dummyRoot.c_str());
    }

void DocumentOperations(Utf8String guid)
    {
    //create a local path to the downloaded document
    Utf8String documentGuid = Utf8PrintfString("%s~2F%s", guid, "DummyRootDocument.json");
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

    //download the file at "documentGuid" to the local folder "dummyRoot"
    //"crdf" will have its properties filled by the server
    crdf.Download(dummyRoot, documentGuid);

    //reupload the folder from "dummyRoot" to the server path "documentGuid"
    ConnectedResponse response = crdf.Upload(dummyRoot, documentGuid);

    if (!response.simpleSuccess)
        {
        std::cout << response.simpleMessage << std::endl;
        return;
        }

    //remove the local document
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

    //retrieve the realitydata info for the guid
    ConnectedRealityData crd = ConnectedRealityData(guid);
    
    //make any desired modifications
    crd.SetName("SimpleModifiedExample (DELETE THIS)");
    //push these changes to the server
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
    //retrieve the realitydata info for the guid
    ConnectedRealityData crd = ConnectedRealityData(guid);
    //Delete the RealityData on the server
    crd.Delete();

    //attempt to retrieve the realitydata info for the guid. This should fail, proving the delete was successful
    ConnectedRealityData crdTester = ConnectedRealityData(guid);
    if(!crdTester.GetName().empty())
        {
        std::cout << "Delete failed" << std::endl;
        return;
        }
    }

int main(int argc, char *argv[])
    {
    //Do this first
    RDSRequestManager::Setup();

    //look at available entries, inspecting the first one in the list and looking at server stats
    Inspect();
    //uploading a new realitydata
    Utf8String guid = CreateUpload();
    if(guid.empty())
        return 1;
    //download/upload a single folder
    FolderOperations(guid);
    //download/upload a single file
    DocumentOperations(guid);
    //change properties of a realityData
    DownloadModify(guid);
    //remove the realityData from the server
    Delete(guid);

    return 0;
    }