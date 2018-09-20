/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/PWDocumentManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/DgnV8.h> // NB: Must include this first!
#include "PWWorkspaceHelper.h"
#include <iModelDmsSupport/DmsSession.h>
#include <ProjectWise_InternalSDK/Include/aaatypes.h>
#include <ProjectWise_InternalSDK/Include/aadmsdef.h>
#include <ProjectWise_InternalSDK/Include/aadmsapi.fdf>

#include <VersionedDgnV8Api/DgnPlatform/DgnDocumentManager.h>


struct PWMoniker :  public Bentley::DgnPlatform::DgnDocumentMoniker
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct PWDocumentManager : public Bentley::DgnPlatform::DgnDocumentManager
    {
    private:
        bool            FileNameIsDmsFormat(WCharCP fileName);
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
    WString searchPath;
    bvector<WString> paths;
    if (NULL != searchPathIn && NULL != providerId)
        {
        searchPath = searchPathIn;
        int folderId, documentId;
        if (SUCCESS != m_helper.GetFolderIdFromMoniker(folderId, documentId, providerId))
            return DgnDocumentManager::_CreateMoniker(portableName, fullPathIn, providerId, isRelative, searchPath.c_str(), findFullPathFirst, customXMLString, displayName);

        BentleyApi::BeFileName portableFileName(portableName);
        BentleyApi::WString refFileName =  portableFileName.GetFileNameAndExtension();
        WString refFolderName;
        refFolderName.Sprintf(L"%d_%d", folderId, documentId);

        BentleyApi::BeFileName fullRefFileName;
        BeStringUtilities::Split(searchPathIn, L";", paths);
        for (WStringCR path : paths)
            {
            BentleyApi::BeFileName parentDir(path.c_str());
            if (!parentDir.DoesPathExist())
                continue;
            
            bool foundRefDir = false;
            while (!parentDir.empty())
                {
                BentleyApi::WString dirName = parentDir.GetFileNameWithoutExtension();
                int cfolderId, cdocId;
                if (2== swscanf(dirName.c_str(), L"%d_%d", &cfolderId, &cdocId))
                    { 
                    parentDir.PopDir();
                    parentDir.AppendToPath(refFolderName.c_str());
                    fullRefFileName = parentDir;
                    foundRefDir = true;
                    break;
                    }
                parentDir.PopDir();
                }
            if (foundRefDir)
                break;
            }

        fullRefFileName.AppendToPath(refFileName.c_str());
        if (fullRefFileName.DoesPathExist())
            return DgnDocumentManager::_CreateMoniker(portableName, fullRefFileName.c_str(), providerId, isRelative, searchPath.c_str(), findFullPathFirst, customXMLString, displayName);

        }

    
    return DgnDocumentManager::_CreateMoniker(portableName, fullPathIn, providerId, isRelative, searchPath.c_str(), findFullPathFirst, customXMLString, displayName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDocumentMonikerPtr           PWDocumentManager::_CreateMonikerFromURI(WCharCP uri, WCharCP basePath)
    {
    return DgnDocumentManager::_CreateMonikerFromURI(uri, basePath);
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