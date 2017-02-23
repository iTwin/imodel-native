/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/RealityDataServiceToolsExample.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityDataService.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

void Test()
    {
        Utf8String id = "05610e4c-79d4-43ef-a9e5-e02e6328d843";
        Utf8String projectId = "1";
        Utf8String folderId = "ab9c6aa6-91ad-424b-935c-28a3c396a041~2FGraz~2F";
        Utf8String documentId = "ab9c6aa6-91ad-424b-935c-28a3c396a041~2FGraz~2FScene~2FProduction_Graz_3MX.3mx";
        Utf8String enterpriseId = "5e41126f-6875-400f-9f75-4492c99ee544";
        //RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");
        RealityDataService::SetServerComponents("s3mxcloudservice.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");

        std::cout << RealityDataService::GetServerName() << std::endl;
        std::cout << RealityDataService::GetWSGProtocol() << std::endl;
        std::cout << RealityDataService::GetRepoName() << std::endl;
        std::cout << RealityDataService::GetSchemaName() << std::endl << std::endl;

        bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
        properties.Insert(RealityDataField::Name, "exampleUpload");
        properties.Insert(RealityDataField::Classification, "Terrain");
        properties.Insert(RealityDataField::Type, "3mx");
        properties.Insert(RealityDataField::Footprint, "{ \\\"points\\\" : [[-112.101512,40.700246],[-111.7394581,40.700246],[-111.7394581,40.8529699],[-112.101512,40.8529699],[-112.101512,40.700246]], \\\"coordinate_system\\\" : \\\"4326\\\" }");
        properties.Insert(RealityDataField::OwnedBy, "francis.boily@bentley.com");

        Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

        /*BeFileName Montgomery = BeFileName("D:/RealityModFrameworkFolder");
        RealityDataServiceUpload* upload = new RealityDataServiceUpload(Montgomery, "604f9be9-e74f-4614-a23e-b02e2dc129f5", propertyString, true);
        if(upload->IsValidUpload())
        upload->Perform();

        RealityDataService::SetServerComponents("s3mxcloudservice.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");*/

        RealityDataByIdRequest* idReq = new RealityDataByIdRequest(id);
        SpatialEntityPtr entity = RealityDataService::Request(*idReq);

        std::cout << "Entity provenance for Id " << id << ":" << std::endl;
        std::cout << entity->GetName() << std::endl << std::endl;


        RealityDataProjectRelationshipByProjectIdRequest* relationReq = new RealityDataProjectRelationshipByProjectIdRequest(projectId);
        bvector<RealityDataProjectRelationshipPtr> relationships = RealityDataService::Request(*relationReq);

        std::cout << "number of relationships found for projectId " << projectId << " :" << std::endl;
        std::cout << relationships.size() << std::endl;


        RealityDataFolderByIdRequest* folderReq = new RealityDataFolderByIdRequest(folderId);
        RealityDataFolderPtr folder = RealityDataService::Request(*folderReq);

        std::cout << "folder found for Id " << folderId << " :" << std::endl;
        std::cout << folder->GetName() << std::endl;


        RealityDataDocumentByIdRequest* documentReq = new RealityDataDocumentByIdRequest(documentId);
        RealityDataDocumentPtr document = RealityDataService::Request(*documentReq);

        std::cout << "document with Id " << documentId << " :" << std::endl;
        std::cout << document->GetName() << std::endl;

        RealityDataDocumentContentByIdRequest* contentRequest = new RealityDataDocumentContentByIdRequest(documentId);

        WChar exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        WString exeDir = exePath;
        size_t pos = exeDir.find_last_of(L"/\\");
        exeDir = exeDir.substr(0, pos + 1);
        BeFileName fileName = BeFileName(exeDir);
        fileName.AppendToPath(BeFileName("testFile"));
        char outfile[1024] = "";
        strcpy(outfile, fileName.GetNameUtf8().c_str());
        FILE* file = fopen(outfile, "wb");

        RealityDataService::Request(*contentRequest, file);


        bvector<Utf8String> filter1 = bvector<Utf8String>();
        bvector<Utf8String> filter2 = bvector<Utf8String>();
        filter1.push_back(RealityDataFilterCreator::FilterByOwner("francis.boily@bentley.com"));
        filter1.push_back(RealityDataFilterCreator::FilterByCreationDate(DateTime(2016, 12, 01), DateTime(2017, 01, 05)));
        filter2.push_back(RealityDataFilterCreator::GroupFiltersAND(filter1));
        filter2.push_back(RealityDataFilterCreator::FilterByType("3mx"));
        filter2.push_back(RealityDataFilterCreator::FilterPublic(true));

        // important note: parentheses are not currently supported, which means that all filters (AND/OR) are evaluated together
        // results may differ from their intended goal
        Utf8String filters = RealityDataFilterCreator::GroupFiltersOR(filter2);

        RealityDataPagedRequest* filteredRequest = new RealityDataPagedRequest();

        filteredRequest->SetFilter(filters);
        filteredRequest->SortBy(RealityDataField::ModifiedTimestamp, true);

        bvector<SpatialEntityPtr> filteredSpatialEntities = RealityDataService::Request(*filteredRequest);

        std::cout << "Number of spatial entities found for filter : " << std::endl;
        std::cout << filteredSpatialEntities.size() << std::endl;


        RealityDataListByEnterprisePagedRequest* enterpriseReq = new RealityDataListByEnterprisePagedRequest(enterpriseId);
        bvector<SpatialEntityPtr> enterpriseVec = RealityDataService::Request(*enterpriseReq);

        std::cout << "Number of spatial entities found for enterprise" << enterpriseId << " :" << std::endl;
        std::cout << enterpriseVec.size() << std::endl;


        RealityDataProjectRelationshipByProjectIdPagedRequest* relationByIdReq = new RealityDataProjectRelationshipByProjectIdPagedRequest(projectId);
        bvector<RealityDataProjectRelationshipPtr> relationVec = RealityDataService::Request(*relationByIdReq);

        std::cout << "Number of relationships found for project " << projectId << " :" << std::endl;
        std::cout << relationVec.size() << std::endl;

        std::cout << "Click to continue...";
        getch();
    }

void Usage()
    {
    std::cout << "RealityDataServiceToolsExample console tools for RDS V1.0" << std::endl << std::endl;
    std::cout << "RealityDataServiceToolsExample [cmd...] [options...] [RealityData]" << std::endl;
    std::cout << "   [cmd...]" << std::endl;
    std::cout << "   -List                      : (default)List all the RealityData (head only)." << std::endl;
    std::cout << "   -ListAll                   : For a specific RealityData, recursively list all entries. " << std::endl;
    std::cout << "   -ListItem root/xx/xx       : For a specific RealityData item [recursively]. " << std::endl;
    //std::cout << "   -ListAll [ProjectId]  : **" << std::endl;
    std::cout << "   -Download NameId Output    : Download file(s) to Output directory" << std::endl;
    //std::cout << "   -Upload WindowsFolder guid  root  footprint type :  **" << std::endl;
    std::cout << "   [options...]" << std::endl;
    std::cout << "   -Detail-0        : (default) Fields-> Name " << std::endl;
    std::cout << "   -Detail-1        : Fields-> Name, Size " << std::endl;
    //std::cout << "   -Detail-2        : Fields-> Name, Dataset, Classification, Size, Owner, Created " << std::endl;
    //std::cout << "   -Filter          :  ?" << std::endl;

    std::cout << "  " << std::endl;
    std::cout << " -? or -help      : Show this usage" << std::endl;
    std::cout << " " << std::endl;
    }



#define CmdNone             0x0000
#define CmdList             0x0001
#define CmdListAll          0x0002
#define CmdListItem         0x0004
static Utf8String s_itemPath;
#define CmdDownload         0x0100
static Utf8String s_outputPath;
#define CmdUpload           0x0800

#define OptionDetail0       0x0001
#define OptionDetail1       0x0002
#define OptionDetail2       0x0004
#define OptionFilter        0x0008

static int s_cmd = CmdNone;
static int s_option= CmdNone;

void ParsingParamters (int argc, char* argv[])
    {
    bool    found = true;
    int     currentParamPos = 1;

    while (found && currentParamPos < argc)
        {
        if (_stricmp(("-?"), argv[currentParamPos]) == 0 ||
            _stricmp(("-help"), argv[currentParamPos]) == 0)
            {
            Usage();
            exit(1);
            }
        else if (_stricmp(("-List"), argv[currentParamPos]) == 0)
            {
            s_cmd = CmdList;
            }
        else if (_stricmp(("-ListAll"), argv[currentParamPos]) == 0)
            {
            s_cmd = CmdListAll;
            }
        else if (_stricmp(("-ListItem"), argv[currentParamPos]) == 0)
            {
            s_cmd = CmdListItem;
            currentParamPos++;
            if (currentParamPos < argc)
                s_itemPath = argv[currentParamPos];
            else
                found = false;
            }
        else if (_stricmp(("-Download"), argv[currentParamPos]) == 0)
        {
            s_cmd = CmdDownload;
            currentParamPos++;
            if (currentParamPos < argc)
                s_itemPath = argv[currentParamPos];
            currentParamPos++;
            if (currentParamPos < argc)
                s_outputPath = argv[currentParamPos];
            else
                found = false;
        }
        else if (_stricmp(("-Detail-0"), argv[currentParamPos]) == 0)
            {
            s_option |= OptionDetail0;
            }
        else if (_stricmp(("-Detail-1"), argv[currentParamPos]) == 0)
            {
            s_option |= OptionDetail1;
            }
        else
            found = false;

        // Increment the current position and check that it's valid.
        if (found)
            currentParamPos++;
        }

    if (!found)
        {
        Usage();
        exit(1);
        }
    }



void ListSubItem(WSGServer& Server, Utf8String Repo, NavNode Root, Utf8String RootName, int MaxEntryDisplay=25)
    {
    bvector<NavNode> subNodes = NodeNavigator::GetInstance().GetChildNodes(Server, Repo, Root);
    std::cout << RootName << std::endl;
    bool folderEmpty=true;
    int count=0;
    for (NavNode subNode : subNodes)
        {
        if (subNode.GetClassName().Contains("Document"))
            {
            if (s_option & OptionDetail1)
                {
                Utf8String repoName(subNode.GetRootId() + "~2F" + subNode.GetInstanceId());
                repoName.ReplaceAll("/", "~2F");
                RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(repoName));
                std::cout << "  " << subNode.GetInstanceId() << "  Size: " << document->GetSize() << std::endl;
                }
            else
                std::cout << "  " << subNode.GetInstanceId() << std::endl;

            folderEmpty = false;
            }
        else
            {
            std::cout << "  " << subNode.GetInstanceId() << std::endl;
            folderEmpty = false;
            }

        ++count;
        if (count > MaxEntryDisplay)
            {
            std::cout << "     More than  " << MaxEntryDisplay << "entries..." << std::endl;
            break;
            }
        }
    if (folderEmpty)
        std::cout << "     Empty" << std::endl;

    std::cout << std::endl;

    for (NavNode subNode : subNodes)
        {
        if (subNode.GetClassName().Contains("Folder"))
            {
            ListSubItem(Server, Repo, subNode, subNode.GetInstanceId());
            }
        }
    }



void Download()
    {
    std::cout << "Downloading : ";

    s_itemPath.ReplaceAll("/", "~2F");
    RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(s_itemPath));

    std::cout << document->GetId() << std::endl;

    BeFileName fileName (s_outputPath);
    fileName.AppendToPath(BeFileName(document->GetName()));
    char outfile[_MAX_PATH] = "";
    strcpy(outfile, fileName.GetNameUtf8().c_str());
    FILE* pFile = fopen(outfile, "wb");
    if (0 != pFile)
        {
        RealityDataDocumentContentByIdRequest* contentRequest = new RealityDataDocumentContentByIdRequest(s_itemPath);
        RealityDataService::Request(*contentRequest, pFile);
        fclose (pFile);
        std::cout << "  Done." << std::endl;
        }
    else
        std::cout << "Output  folder invalide :" << outfile << std::endl;

    }




int main(int argc, char* argv[])
    {
    ParsingParamters(argc, argv);

    // List root
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");


    // List head by default only
    std::cout << "RealityData commander..." << std::endl;

    // List commande
    if (s_cmd == CmdList || s_cmd == CmdListAll)
        {
        // With NavNode
        WSGServer server = WSGServer("dev-realitydataservices-eus.cloudapp.net", false);
        bvector<NavNode> nodes = NodeNavigator::GetInstance().GetRootNodes(server, "S3MXECPlugin--Server");

        for (NavNode root : nodes)
            {
            SpatialEntityPtr pData = RealityDataService::Request(RealityDataByIdRequest(root.GetInstanceId()));

            std::cout << root.GetInstanceId() << " -- " << pData->GetName()<< std::endl;

            }
        std::cout << std::endl;

        if (s_cmd == CmdListAll)
            {
            for (NavNode root : nodes)
                {
                SpatialEntityPtr pData = RealityDataService::Request(RealityDataByIdRequest(root.GetInstanceId()));

                ListSubItem(server, "S3MXECPlugin--Server", root, root.GetInstanceId() + " -- " + pData->GetName());
                }
            }
        }


    if (s_cmd == CmdListItem)
        {
        s_itemPath.ReplaceAll("/", "~2F");
        RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(s_itemPath));

        std::cout << document->GetFolderId() << std::endl;
        std::cout << document->GetName() << std::endl;
        }

    if (s_cmd == CmdDownload)
        Download();

    std::cout << "Click a key to continue..." << std::endl;
    getch();
    return 0;
    }






