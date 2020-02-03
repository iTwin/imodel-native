/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>

#include <stdio.h>
#include <conio.h>
#include <iostream> 
#include <iomanip>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


#define CmdNone                 0x0000
#define CmdList                 0x0001
#define CmdListDetail           0x0002
#define CmdListAll              0x0004
#define CmdListAllDetail        0x0008
#define CmdListItem             0x0010
static Utf8String s_itemPath;
#define CmdDownload             0x0100
static Utf8String s_outputPath;
#define CmdUpload               0x0400
#define CmdEntrStat             0x0800

#define OptFilterOwner          0x0001
static Utf8String s_filterOwner;
#define OptFilterProject        0x0002
static Utf8String s_filterProject;


#define OptSortModDate          0x0010
#define OptSortGroup            0x0020


static int s_cmd = CmdNone;
static int s_option = CmdNone;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Usage()
    {
    std::cout << "RealityDataServiceToolsExample console tools for RDS V1.0" << std::endl << std::endl;
    std::cout << "RealityDataServiceToolsExample [options...] [cmd...] [RealityDataId]" << std::endl;
    std::cout << "   [cmd...]" << std::endl;
    std::cout << "   -List                      : List all the RealityData root only)." << std::endl;
    std::cout << "   -ListDetail                : List all the RealityData root only) + all attributes " << std::endl;
    std::cout << "   -ListAll                   : List all the RealityData, recursively list all entries. " << std::endl;
    std::cout << "   -ListAllDetail             : List all the RealityData, recursively list all entries. + all attributes " << std::endl;
    std::cout << "   -ListItem root/xx/xx       : For a specific RealityData item all details. " << std::endl;
    //std::cout << "   -ListAll [ProjectId]  : **" << std::endl;
    std::cout << "   -Download NameId Output    : Download file(s) to Output directory" << std::endl;
    //std::cout << "   -Upload WindowsFolder guid  root  footprint type :  **" << std::endl;
    std::cout << "   -EnterpriseStat            : Display statistic, like Space used, nb of realitydata..." << std::endl;
    std::cout << "   [options...]" << std::endl;
    std::cout << "   -Owner=...                 :  Filter" << std::endl;
    std::cout << "   -Project=...               :  Filter" << std::endl;
    std::cout << "   -SortModDate               :  RealityData sorted by modification date" << std::endl;
    std::cout << "   -SortGroup                 :  RealityData sorted by group" << std::endl;


    std::cout << "  " << std::endl;
    std::cout << " -? or -help      : Show this usage" << std::endl;
    std::cout << " " << std::endl;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsingParamters (int argc, char* argv[])
    {
    bool    found = true;
    int     currentParamPos = 1;

    if (argc < 2)
        {
        Usage();
        exit(1);
        }

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
        else if (_stricmp(("-ListDetail"), argv[currentParamPos]) == 0)
        {
            s_cmd = CmdListDetail;
        }
        else if (_stricmp(("-ListAll"), argv[currentParamPos]) == 0)
            {
            s_cmd = CmdListAll;
            }
        else if (_stricmp(("-ListAllDetail"), argv[currentParamPos]) == 0)
        {
            s_cmd = CmdListAllDetail;
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
        else if (_stricmp(("-EnterpriseStat"), argv[currentParamPos]) == 0)
        {
            s_cmd = CmdEntrStat;
        }
        else if (_strnicmp(("-Owner="), argv[currentParamPos], 7) == 0)
        {
            s_option |= OptFilterOwner;
            Utf8String param (argv[currentParamPos]);
            size_t pos;

            if ((pos=param.GetNextToken(s_filterOwner, "=", 0)) != Utf8String::npos)
                s_filterOwner = &(argv[currentParamPos][pos]);
            else
                found = false;
        }
        else if (_strnicmp(("-Project="), argv[currentParamPos], 7) == 0)
        {
            s_option |= OptFilterProject;
            Utf8String param(argv[currentParamPos]);
            size_t pos;

            if ((pos = param.GetNextToken(s_filterProject, "=", 0)) != Utf8String::npos)
                s_filterProject = &(argv[currentParamPos][pos]);
            else
                found = false;
        }
        else if (_stricmp(("-SortByGroup"), argv[currentParamPos]) == 0)
        {
            s_option |= OptSortGroup;
        }
        else if (_stricmp(("-SortByModDate"), argv[currentParamPos]) == 0)
        {
            s_option |= OptSortModDate;
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ListSubItem(WSGServer& Server, Utf8String Repo, NavNode Root, Utf8String RootName, int MaxEntryDisplay=25)
    {
    RawServerResponse nodeResponse = RawServerResponse();
    bvector<NavNode> subNodes = NodeNavigator::GetInstance().GetChildNodes(Server, Repo, Root, nodeResponse);
    std::cout << RootName << std::endl;
    bool folderEmpty=true;
    int count=0;
    for (NavNode subNode : subNodes)
        {
        if (subNode.GetECClassName().Contains("Document"))
            {
            if (s_cmd & CmdListAllDetail)
                {
                Utf8String repoName(subNode.GetRootId() + "~2F" + subNode.GetInstanceId());
                repoName.ReplaceAll("/", "~2F");
                RawServerResponse idResponse = RawServerResponse();
                RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(repoName), idResponse);
                std::cout << "  " << subNode.GetInstanceId() << 
                    std::setw(12) << " Size(KB): " << document->GetSize() <<
                    std::setw(16) << " ContentType: " << document->GetContentType() << std::endl;
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
        if (subNode.GetECClassName().Contains("Folder"))
            {
            ListSubItem(Server, Repo, subNode, subNode.GetInstanceId());
            }
        }
    }

bool FilterByProject(Utf8StringCR RealityId, bvector<RealityDataRelationshipPtr>& ProjectRelation)
    {
    // Option not set
    if (!(s_option & OptFilterProject))
        return true;

    for (RealityDataRelationshipPtr pData : ProjectRelation)
    {
        if (pData->GetRealityDataId() == RealityId)
            return true;
    }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ListCmd()
    {
    if (s_cmd & (CmdList | CmdListDetail | CmdListAll | CmdListAllDetail))
        {
        RealityDataPagedRequest enterpriseReq;
        enterpriseReq.SetPageSize(25);

        // Filters ?
        if (s_option & (OptFilterOwner))
            {
            bvector<RDSFilter> filter1 = bvector<RDSFilter>();
            
            filter1.push_back(RealityDataFilterCreator::FilterByOwner(s_filterOwner));
            RDSFilter filters = RealityDataFilterCreator::GroupFiltersOR(filter1);

            enterpriseReq.SetFilter(filters);
            }

        if (s_option & (OptSortModDate))
            enterpriseReq.SortBy(RealityDataField::ModifiedTimestamp, true);

        if (s_option & (OptSortGroup))
            enterpriseReq.SortBy(RealityDataField::Group, true);

        RawServerResponse enterpriseResponse = RawServerResponse();
        bvector<RealityDataPtr> enterpriseVec = RealityDataService::Request(enterpriseReq, enterpriseResponse);

        bvector<RealityDataRelationshipPtr> relationships;
        if (s_option & (OptFilterProject))
            {
            RealityDataRelationshipByProjectIdRequest relationReq(s_filterProject);
            RawServerResponse relationResponse = RawServerResponse();
            relationships = RealityDataService::Request(relationReq, relationResponse);
            }


        size_t EnterpriseSizeKB(0);
        do {
            if(RequestStatus::BADREQ == enterpriseResponse.status)
                exit(-1);

            for (RealityDataPtr pData : enterpriseVec)
                {
                EnterpriseSizeKB += pData->GetTotalSize();

#if 0   // Validation only
                RequestStatus status2;
                RealityDataByIdRequest idReq(pData->GetIdentifier());
                RealityDataPtr entity = RealityDataService::Request(idReq, status2);

                assert(entity->GetOragnizationId() == pData->GetOragnizationId());
                assert(entity->GetApproximateFileSize() == pData->GetApproximateFileSize());
                assert(entity->GetOwner() == pData->GetOwner());
                assert(entity->GetRootDocument() == pData->GetRootDocument());
                assert(entity->GetCopyright() == pData->GetCopyright());
                assert(entity->GetTermsOfUse() == pData->GetTermsOfUse());

#endif
                if (FilterByProject(pData->GetIdentifier(), relationships))
                    std::cout << pData->GetIdentifier() << " -- " << pData->GetName() << std::endl;

                if (s_cmd & (CmdListDetail | CmdListAllDetail) && FilterByProject(pData->GetIdentifier(), relationships))
                    {
                    std::cout << "  " << 
                        " Dataset        : " << pData->GetDataset() << std::endl << "  " <<
                        " Group          : " << pData->GetGroup() << std::endl << "  " <<
                        " Classification : " << pData->GetClassificationTag() << std::endl << "  " <<
                        " Size(KB)       : " << pData->GetTotalSize() << std::endl << "  " <<
                        " Owner          : " << pData->GetOwner() << std::endl << "  " <<
                        " Created        : " << pData->GetCreationDateTime().ToString().c_str() << std::endl << "  " <<
                        " Modification   : " << pData->GetModifiedDateTime().ToString().c_str() << std::endl << "  " <<
                        " RootDocument   : " << pData->GetRootDocument() << std::endl << "  " <<
                        " Visibility     : " << pData->GetVisibilityTag() << std::endl << "  " <<
                        " Organization   : " << pData->GetOrganizationId() << std::endl << "  " <<
                        " Copyright      : " << pData->GetCopyright() << std::endl << "  " <<
                        " TermsOfUse     : " << pData->GetTermsOfUse() << std::endl << "  " <<
                        " Description    : " << pData->GetDescription() << std::endl;
                    }
                }

            enterpriseVec.clear();
            enterpriseResponse = RawServerResponse();
            }
        while ((enterpriseVec = RealityDataService::Request(enterpriseReq, enterpriseResponse)).size() > 0);
        std::cout << std::endl << "*** Size total : " << EnterpriseSizeKB << "KB" << std::endl;
        std::cout << std::endl;

        if (s_cmd & (CmdListAll | CmdListAllDetail))
            {
            WSGServer server = WSGServer("dev-realitydataservices-eus.cloudapp.net", false);
            RawServerResponse nodeResponse = RawServerResponse();
            bvector<NavNode> nodes = NodeNavigator::GetInstance().GetRootNodes(server, "S3MXECPlugin--Server", nodeResponse);

            for (NavNode root : nodes)
                {
                RawServerResponse idResponse = RawServerResponse();
                RealityDataPtr pData2 = RealityDataService::Request(RealityDataByIdRequest(root.GetInstanceId()), idResponse);

                ListSubItem(server, "S3MXECPlugin--Server", root, root.GetInstanceId() + " -- " + pData2->GetName());
                }
            }
        }

    if (s_cmd == CmdListItem)
        {
        s_itemPath.ReplaceAll("/", "~2F");
        RawServerResponse documentResponse = RawServerResponse();
        RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(s_itemPath), documentResponse);

        std::cout << document->GetFolderId() << document->GetName() <<
            std::setw(12) << " Size(KB): " << document->GetSize() <<
            std::setw(16) << " ContentType: " << document->GetContentType() << std::endl;

        }

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadCmd()
    {
    std::cout << "Downloading : ";

    s_itemPath.ReplaceAll("/", "~2F");
    RawServerResponse documentResponse = RawServerResponse();
    RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(s_itemPath), documentResponse);

    std::cout << document->GetId() << std::endl;

    BeFileName fileName (s_outputPath);
    fileName.AppendToPath(BeFileName(document->GetName()));
    BeFile file;
    BeFileStatus fileStatus = file.Create(fileName.GetName(), true);
    if (fileStatus == BeFileStatus::Success)
        {
        RealityDataDocumentContentByIdRequest contentRequest(s_itemPath);
        RawServerResponse contentResponse = RawServerResponse();
        RealityDataService::Request(contentRequest, &file, contentResponse);
        file.Close();
        std::cout << "  Done." << std::endl;
        }
    else
        std::cout << "Output folder invalid :" << fileName.GetNameUtf8() << std::endl;

    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
int main(int argc, char* argv[])
    {
    ParsingParamters(argc, argv);

    // List root
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");
    RealityDataService::SetUserAgent("RealityDataServiceToolsExample - dummy user agent");
    RealityDataService::SetProjectId("72524420-7d48-4f4e-8b0f-144e5fa0aa22");

    // List head by default only
    std::cout << "RealityData commander..." << std::endl;

    if (s_cmd & (CmdList | CmdListDetail | CmdListAll | CmdListAllDetail | CmdListItem))
        ListCmd();                       
                                            
    if (s_cmd == CmdDownload)         
        DownloadCmd();                     

    if (s_cmd == CmdEntrStat)
        {
        RawServerResponse statResponse = RawServerResponse();
        RealityDataEnterpriseStatRequest ptt("");
        RealityDataEnterpriseStat stat;
        RealityDataService::Request(ptt, stat, statResponse);

        std::cout << "Organization statistics: " << std::endl;
        std::cout << "   NbRealityData : %lu\n" << stat.GetNbRealityData() << std::endl;
        std::cout << "   TotalSize(KB) : %lu\n\n" << stat.GetTotalSizeKB() << std::endl;
        std::cout << "   OrganizationId: %s\n" << stat.GetOrganizationId().c_str() << std::endl;
        std::cout << "   UltimateId    : %s\n" << stat.GetUltimateId().c_str() << std::endl;
        std::cout << "   UltimateSite  : %s\n\n" << stat.GetUltimateSite().c_str() << std::endl;
        }
        

    std::cout << "Click a key to continue..." << std::endl;
    getch();
    return 0;
    }
