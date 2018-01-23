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
#include <ScalableMesh/IScalableMeshRDSProvider.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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

    void        InitializeRealityDataService();
    bool        IsTokenExpired();
    void        UpdateToken();   
    Utf8String  GetRootDocumentName();    

protected:

    virtual Utf8String _GetAzureURLAddress();

    virtual Utf8String _GetRDSURLAddress();

    virtual Utf8String _GetToken();

    virtual Utf8String _GetRootDocument();

public:

    static Utf8String GetBuddiUrl();

    explicit ScalableMeshRDSProvider(const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid);
    virtual ~ScalableMeshRDSProvider();
};


END_BENTLEY_SCALABLEMESH_NAMESPACE

