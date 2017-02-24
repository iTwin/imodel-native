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
#define CmdUpload               0x0800

#define OptionFilter            0x0001

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
    std::cout << "   [options...]" << std::endl;
    //std::cout << "   -Filter          :  ?" << std::endl;

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
    bvector<NavNode> subNodes = NodeNavigator::GetInstance().GetChildNodes(Server, Repo, Root);
    std::cout << RootName << std::endl;
    bool folderEmpty=true;
    int count=0;
    for (NavNode subNode : subNodes)
        {
        if (subNode.GetClassName().Contains("Document"))
            {
            if (s_cmd & CmdListAllDetail)
                {
                Utf8String repoName(subNode.GetRootId() + "~2F" + subNode.GetInstanceId());
                repoName.ReplaceAll("/", "~2F");
                RealityDataDocumentPtr document = RealityDataService::Request(RealityDataDocumentByIdRequest(repoName));
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
        if (subNode.GetClassName().Contains("Folder"))
            {
            ListSubItem(Server, Repo, subNode, subNode.GetInstanceId());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ListCmd()
    {
    if (s_cmd & (CmdList | CmdListDetail | CmdListAll | CmdListAllDetail))
        {
        // With NavNode
        WSGServer server = WSGServer("dev-realitydataservices-eus.cloudapp.net", false);
        bvector<NavNode> nodes = NodeNavigator::GetInstance().GetRootNodes(server, "S3MXECPlugin--Server");

        for (NavNode root : nodes)
            {
            SpatialEntityPtr pData = RealityDataService::Request(RealityDataByIdRequest(root.GetInstanceId()));

            std::cout << root.GetInstanceId() << " -- " << pData->GetName() << std::endl;

            if (s_cmd & (CmdListDetail | CmdListAllDetail))
                {
                std::cout << "  " << 
                     " Dataset        : " << pData->GetDataset() << std::endl << "  " <<
                     " Classification : " << pData->GetClassification() << std::endl << "  " <<
                     " Size(KB)       : " << pData->GetDataset() << std::endl << "  " <<
                     " Owner          : " << pData->GetOwner() << std::endl << "  " <<
                     " Created        : " << pData->GetDate().ToString() << std::endl << "  " <<
                     " GetRootDocument: " << pData->GetRootDocument() << std::endl << "  " <<
                     " Enterprise     : " << pData->GetEnterprise() << std::endl << "  " <<
                     " Description    : " << pData->GetDescription() << std::endl;
                }
            }
        std::cout << std::endl;

        if (s_cmd & (CmdListAll | CmdListAllDetail))
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



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Donald.Morissette                  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
int main(int argc, char* argv[])
    {
    ParsingParamters(argc, argv);

    // List root
    RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "v2.4", "S3MXECPlugin--Server", "S3MX");

    // List head by default only
    std::cout << "RealityData commander..." << std::endl;

    if (s_cmd & (CmdList | CmdListDetail | CmdListAll | CmdListAllDetail | CmdListItem))
        ListCmd();                       
                                            
    if (s_cmd == CmdDownload)         
        DownloadCmd();                     

    std::cout << "Click a key to continue..." << std::endl;
    getch();
    return 0;
    }
