/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelDmsSupport/PWDocumentManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnDbSync/DgnV8/DgnV8.h> // NB: Must include this first!
#include "PWWorkspaceHelper.h"
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
    //virtual DgnDocumentMonikerPtr           _CreateMonikerFromFileName(WCharCP fileName, WCharCP basePath) override;
        
    //virtual DgnDocumentMonikerPtr           _CreateMoniker(WCharCP portableName, WCharCP fullPath, WCharCP providerId, bool isRelative, WCharCP searchPath, bool findFullPathFirst, WCharCP customXMLString, WCharCP displayName = NULL) override;
    //virtual DgnDocumentMonikerPtr           _CreateMonikerFromURI(WCharCP uri, WCharCP basePath) override;
    
    };

template<typename T, int N>
//template argument deduction
int ARRAY_LENGTH(T(&arr1)[N]) //Passing the array by reference 
{
    //return sizeof(arr1) / sizeof(arr1[0]); //Correctly returns the size of 'list'
    // or
    return N; //Correctly returns the size too [cool trick ;-)]
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
    return new PWDocumentManager();
    }