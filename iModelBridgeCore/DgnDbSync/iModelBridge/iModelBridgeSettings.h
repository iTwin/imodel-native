/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/BeSQLite.h>
#include <BeRapidJson/BeRapidJson.h>
#include <iModelBridge/iModelBridge.h>

BEGIN_BENTLEY_NAMESPACE namespace WebServices {
    struct ConnectAuthenticationHandler;
} END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_DGN_NAMESPACE

struct iModelBridgeSettings
    {
    private:
        BentleyApi::WebServices::IConnectSignInManagerPtr m_signInMgr;
        std::shared_ptr<BentleyApi::WebServices::ConnectAuthenticationHandler> m_authHandler;
        Utf8String m_requestGuid;
        Utf8String m_iModelGuid;
        Utf8String m_connectProjectGuid;
        
        Utf8String GetDocumentMappingUrl();
        BentleyStatus GetDocumentMappingJson(rapidjson::Document& doc);
        bool IsValid() const;
        std::shared_ptr<BentleyApi::WebServices::ConnectAuthenticationHandler> GetAuthHandler();
    public:
        IMODEL_BRIDGE_EXPORT iModelBridgeSettings(BentleyApi::WebServices::IConnectSignInManagerPtr mgr, Utf8StringCR requestGuid, Utf8StringCR iModelGuid, Utf8StringCR projectGuid);
        void SetiModelId(Utf8String iModelId) { m_iModelGuid = iModelId; }
        IMODEL_BRIDGE_EXPORT BentleyStatus GetBriefCaseId(Utf8StringCR documentGuid, BeSQLite::BeBriefcaseId& briefcaseId);
        IMODEL_BRIDGE_EXPORT BentleyStatus SetBriefCaseId(Utf8StringCR documentGuid, BeSQLite::BeBriefcaseId const& briefcaseId);
    };

END_BENTLEY_DGN_NAMESPACE