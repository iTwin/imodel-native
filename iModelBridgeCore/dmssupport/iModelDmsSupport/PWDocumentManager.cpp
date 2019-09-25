/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/DgnV8.h> // NB: Must include this first!
#include <functional>
#include "PWWorkspaceHelper.h"
#include <iModelDmsSupport/DmsSession.h>
#include <ProjectWise_InternalSDK/Include/aaatypes.h>
#include <ProjectWise_InternalSDK/Include/aadmsdef.h>
#include <ProjectWise_InternalSDK/Include/aadmsapi.fdf>

#include <VersionedDgnV8Api/DgnPlatform/DgnDocumentManager.h>
#include <BeXml/BeXml.h>

struct PWMoniker :  public Bentley::DgnPlatform::DgnDocumentMoniker
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct PWDocumentManager : public Bentley::DgnPlatform::DgnDocumentManager
    {
    private:
        typedef std::function<DgnDocumentMonikerPtr(WCharCP portableName, WCharCP fullPath, WCharCP providerId, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString)> CREATEMONIKERIMPLFUNC;

        bool                    FileNameIsDmsFormat(WCharCP fileName);
        DgnDocumentMonikerPtr   _CreateMonikerImpl(WCharCP portableName, WCharCP fullPath, WCharCP providerId, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString, CREATEMONIKERIMPLFUNC parentFunction);
        BentleyApi::Dgn::PWWorkspaceHelper&   m_helper;
    protected:
    /*virtual DgnDocumentPtr                  _CreateDocument0(DgnDocumentMonikerR moniker) override ;
    virtual DgnDocumentMonikerPtr           _CreateMonikerFromRawData(WCharCP, WCharCP, WCharCP, WCharCP, bool, WCharCP) override;
    virtual DgnDocumentMonikerPtr           _CreatePortableMonikerFromFileName(WCharCP userEnteredPath, WCharCP fullPath, WCharCP basePath, WCharCP directoryCfgVar, WCharCP searchPath, RelativePathPreference relativePref) override;
    virtual DgnDocumentMonikerPtr           _CreateMonikerForPackagedFile(WCharCP packageFilePortableName, WCharCP packageFilefullPath, Int32 embeddedId, WCharCP embedName, WCharCP providerID, bool isRelative, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString, WCharCP displayName = NULL) override;
    virtual DgnDocumentMonikerPtr           _CreateMonikerForPackagedFileFromPackageFileMoniker(DgnDocumentMonikerR packageMoniker, Int32 embeddedId, WCharCP embedName) override;
    virtual DgnDocumentPtr                  _CreateDocumentFromMoniker(StatusInt&, DgnDocumentMonikerR, int, DgnDocument::FetchMode, DgnDocument::FetchOptions) override
    virtual DgnDocumentPtr                  _CreateDocumentForNewFile(DgnFileStatus&, WCharCP, WCharCP, int, WCharCP, DgnDocument::OverwriteMode, DgnDocument::CreateOptions) override;
    virtual DgnDocumentPtr                  _CreateDocumentForEmbeddedFile(WCharCP) override;
    */
    virtual DgnDocumentMonikerPtr           _CreateMonikerFromFileName(WCharCP fileName, WCharCP basePath) override;
        
    virtual DgnDocumentMonikerPtr           _CreateMoniker(WCharCP portableName, WCharCP fullPath, WCharCP providerId, bool isRelative, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString, WCharCP displayName = NULL) override;
    virtual DgnDocumentMonikerPtr           _CreateMonikerFromURI(WCharCP uri, WCharCP basePath) override;
    virtual DgnDocumentMonikerPtr           _CreateMonikerFromRawData(WCharCP savedName, WCharCP fullPath, WCharCP providerId, WCharCP basePath, bool fullPathFirst, WCharCP customString) override;
    
    public:
        PWDocumentManager (BentleyApi::Dgn::PWWorkspaceHelper& helper)
            :m_helper(helper)
        {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr PWDocumentManager::_CreateMonikerFromFileName(WCharCP fileName, WCharCP basePath)
    {
    if (!FileNameIsDmsFormat(fileName))
        return DgnDocumentManager::_CreateMonikerFromFileName(fileName, basePath);

    return DgnDocumentManager::_CreateMonikerFromFileName(fileName, basePath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr   PWDocumentManager::_CreateMoniker(WCharCP portableName, WCharCP fullPathIn, WCharCP providerId, bool isRelative, WCharCP searchPathIn, bool findFullPathFirst, WCharCP customXMLString, WCharCP displayName)
    {
    CREATEMONIKERIMPLFUNC parentFunction =
        [this, isRelative, displayName](WCharCP portableName, WCharCP fullPath, WCharCP providerId, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString)
        {
        return DgnDocumentManager::_CreateMoniker(portableName, fullPath, providerId, isRelative, searchPath, findFullPathFirst, customXMLString, displayName);
        };

    return _CreateMonikerImpl(portableName, fullPathIn, providerId, searchPathIn, findFullPathFirst, customXMLString, parentFunction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr           PWDocumentManager::_CreateMonikerFromURI(WCharCP uri, WCharCP basePath)
    {
    return DgnDocumentManager::_CreateMonikerFromURI(uri, basePath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Jonathan.DeCarlo                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr PWDocumentManager::_CreateMonikerFromRawData(WCharCP portableName, WCharCP fullPathIn, WCharCP providerId, WCharCP searchPathIn, bool findFullPathFirst, WCharCP customXMLString)
    {
    CREATEMONIKERIMPLFUNC parentFunction = 
        [this](WCharCP portableName, WCharCP fullPath, WCharCP providerId, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString)
        {
        return DgnDocumentManager::_CreateMonikerFromRawData(portableName, fullPath, providerId, searchPath, findFullPathFirst, customXMLString);
        };

    return _CreateMonikerImpl(portableName, fullPathIn, providerId, searchPathIn, findFullPathFirst, customXMLString, parentFunction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   ExtractMonikerFromXml (BentleyApi::WString& dmsMoniker, WCharCP customXMLString)
    {
    if (NULL == customXMLString)
        return ERROR;

    //<DmsMoniker>pw://ewr-pw.bentley.com:EWR2/Documents/D{57597070-9378-4d8f-b551-cbc938dcb216}</DmsMoniker>
    BentleyApi::BeXmlStatus xmlStatus;
    BentleyApi::BeXmlDomPtr xmlDom = BentleyApi::BeXmlDom::CreateAndReadFromString(xmlStatus, customXMLString);
    if (!xmlDom.IsValid())
        return ERROR;
        
    BentleyApi::BeXmlNodeP root = xmlDom->GetRootElement();
    if (nullptr == root)
        return ERROR;

    if (0 != strcmp(root->GetName(), "DmsMoniker"))
        return ERROR;

    return BentleyApi::BEXML_Success == root->GetContent(dmsMoniker) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Jonathan.DeCarlo                  07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr PWDocumentManager::_CreateMonikerImpl(WCharCP portableName, WCharCP fullPathIn, WCharCP providerId, WCharCP searchPathIn, bool findFullPathFirst, WCharCP customXMLString, CREATEMONIKERIMPLFUNC parentFunction)
    {
    BentleyApi::WString dmsMoniker;
    if (NULL != providerId)
        dmsMoniker = providerId;

    if (dmsMoniker.empty())//Look at customXMLstring to resolve it
        {
        if (SUCCESS != ExtractMonikerFromXml (dmsMoniker, customXMLString))
            return parentFunction(portableName, fullPathIn, providerId, searchPathIn, findFullPathFirst, customXMLString);
        }

    int folderId, documentId;
    if (SUCCESS != m_helper.GetFolderIdFromMoniker(folderId, documentId, dmsMoniker.c_str()))
        return parentFunction(portableName, fullPathIn, providerId, searchPathIn, findFullPathFirst, customXMLString);

    WString refFolderName;
    refFolderName.Sprintf(L"%d_%d", folderId, documentId);

    WString searchPath;
    bvector<WString> paths;
    BentleyApi::BeFileName portableFileName(portableName);
    BentleyApi::WString refFileName = portableFileName.GetFileNameAndExtension();

    if (NULL != searchPathIn)
        {
        searchPath = searchPathIn;

        BentleyApi::BeFileName fullRefFileName;
        BeStringUtilities::Split(searchPathIn, L";", paths);
        for (WStringCR path : paths)
            {
            BentleyApi::BeFileName parentDir(path.c_str());
            if (!parentDir.DoesPathExist())
                continue;
            BentleyApi::BeFileName::FixPathName(parentDir, path.c_str(), false);
            bool foundRefDir = false;
            while (!parentDir.empty())
                {
                BentleyApi::WString dirName = parentDir.GetFileNameWithoutExtension();
                int cfolderId, cdocId;
                if (2 == swscanf(dirName.c_str(), L"%d_%d", &cfolderId, &cdocId))
                    {
                    parentDir.PopDir();
                    parentDir.AppendToPath(refFolderName.c_str());
                    fullRefFileName = parentDir;
                    foundRefDir = true;
                    break;
                    }
                BentleyApi::BeFileName  existingDir = parentDir;
                parentDir.PopDir();
                if (0 == existingDir.CompareTo(parentDir.c_str()))
                    break;
                }
            if (foundRefDir)
                break;
            }

        fullRefFileName.AppendToPath(refFileName.c_str());
        if (fullRefFileName.DoesPathExist())
            return parentFunction(portableName, fullRefFileName.c_str(), providerId, searchPath.c_str(), findFullPathFirst, customXMLString);
        }
    //else search for this in the PW workspace folder with folder_id pattern
    BentleyApi::WPrintfString dmsWorkspaceDirFilePath(L"%s\\dms%05d\\%s", m_helper.GetActiveWorkspaceDir().c_str(), folderId, refFileName.c_str());
    BentleyApi::BeFileName dmsworkspaceDirFile(dmsWorkspaceDirFilePath);
    if (dmsworkspaceDirFile.DoesPathExist())
        return parentFunction(portableName, dmsworkspaceDirFile.c_str(), providerId, searchPath.c_str(), findFullPathFirst, customXMLString);

    BentleyApi::WPrintfString workspaceDirFilePath(L"%s\\%s\\%s", m_helper.GetActiveWorkspaceDir().c_str(), refFolderName.c_str(), refFileName.c_str());
    BentleyApi::BeFileName workspaceDirFile(workspaceDirFilePath);
    if (workspaceDirFile.DoesPathExist())
        return parentFunction(portableName, workspaceDirFile.c_str(), providerId, searchPath.c_str(), findFullPathFirst, customXMLString);


    searchPath.append(L";");
    searchPath.append(m_helper.GetActiveWorkspaceDir().c_str());
    searchPath.append(L"\\*");

    return parentFunction(portableName, fullPathIn, providerId, searchPath.c_str(), findFullPathFirst, customXMLString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PWDocumentManager::FileNameIsDmsFormat(WCharCP fileName)
    {
    if (NULL == fileName || 0 == *fileName)
        return false;

    //MCMLOGTRACE(L"pwise.MCM.DmsUtil", L"Checking if the file " << CHECKNULLCONVERT(fileName) << L" is in DMS format!");

    static const WChar DmsMonikerProtocol[] = L"pw:\\\\";
    static const WChar DmsMonikerProtocol2[] = L"pw://";
    static const WChar DmsDescMonikerProtocol[] = L"pwdesc:\\\\";
    static const WChar DmsDescMonikerProtocol2[] = L"pwdesc://";
    static const WChar DmsNameMonikerProtocol[] = L"pwname:\\\\";
    static const WChar DmsNameMonikerProtocol2[] = L"pwname://";

    if (!_wcsnicmp(fileName, DmsMonikerProtocol, ARRAY_LENGTH(DmsMonikerProtocol) - 1)
        || !_wcsnicmp(fileName, DmsMonikerProtocol2, ARRAY_LENGTH(DmsMonikerProtocol2) - 1)
        || !_wcsnicmp(fileName, DmsDescMonikerProtocol, ARRAY_LENGTH(DmsMonikerProtocol2) - 1)
        || !_wcsnicmp(fileName, DmsDescMonikerProtocol2, ARRAY_LENGTH(DmsNameMonikerProtocol) - 1)
        || !_wcsnicmp(fileName, DmsNameMonikerProtocol, ARRAY_LENGTH(DmsNameMonikerProtocol) - 1)
        || !_wcsnicmp(fileName, DmsNameMonikerProtocol2, ARRAY_LENGTH(DmsNameMonikerProtocol2) - 1))
    {
        //MCMLOGTRACE(L"pwise.MCM.DmsUtil", L"The file " << CHECKNULLCONVERT(fileName) << L" is in DMS format!");
        return true;
    }
    //MCMLOGTRACE(L"pwise.MCM.DmsUtil", L"The file " << CHECKNULLCONVERT(fileName) << L" is NOT in DMS format!");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::DgnPlatform::DgnDocumentManager* BentleyApi::Dgn::PWWorkspaceHelper::_GetDgnDocumentManager()
    {
    return new PWDocumentManager(*this);
    }