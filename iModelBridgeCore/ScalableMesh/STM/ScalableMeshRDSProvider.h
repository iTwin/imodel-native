/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshRDSProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/

    #include <RealityPlatformTools/RealityDataService.h>
    USING_NAMESPACE_BENTLEY_REALITYPLATFORM


#include <ScalableMesh/IScalableMeshRDSProvider.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*=================================================================================**//**
Useful container for Azure SAS token values
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AzureConnection
    {

    AzureHandshake* m_handshake = nullptr;

    int64_t m_tokenTimer = 0;
    Utf8String m_token;
    Utf8String m_url;
    };

/*=================================================================================**//**
ScalableMeshRDSProvider defines interfaces that enables attaching streamed meshes (3DTiles) 
from ProjectWise Context Share.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ScalableMeshRDSProvider : public RefCounted<IScalableMeshRDSProvider>
{
private:

    friend class IScalableMeshRDSProvider;

    Utf8String m_ProjectGuid;
    Utf8String m_PWCSMeshGuid;
    AzureConnection m_AzureConnection;
    std::function<void(Utf8StringR, time_t&)>* m_tokenProviderP = nullptr;

    static void InitializeRealityDataService(const Utf8String& projectID);

    bool        IsTokenExpired();
    void        UpdateToken();   
    Utf8String  GetRootDocumentName();    

protected:

    virtual Utf8String _GetAzureURLAddress();

    virtual Utf8String _GetRDSURLAddress();

    virtual Utf8String _GetToken();

    virtual Utf8String _GetRootDocument();

    virtual Utf8String _GetProjectID();

public:

    static Utf8String GetBuddiUrl();
    static bool IsHostedByRDS(const Utf8String& projectGuid, const Utf8String& url);

    explicit ScalableMeshRDSProvider(const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid);
    virtual ~ScalableMeshRDSProvider();
};


END_BENTLEY_SCALABLEMESH_NAMESPACE

