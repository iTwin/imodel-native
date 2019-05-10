/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/Configuration/BuddiClient.h>
BEGIN_BENTLEY_NAMESPACE namespace WebServices {
    typedef std::shared_ptr<struct ClientInfo> ClientInfoPtr;
} END_BENTLEY_NAMESPACE


struct iModelCrashProcessor
    {
    private:
    BentleyApi::WebServices::IBuddiClientPtr m_buddi;
    BentleyStatus GetCrashReportUrl(Utf8StringR url);
    public:
        iModelCrashProcessor();
        BentleyStatus SendCrashReport(BentleyApi::WebServices::ClientInfoPtr clientInfo, Utf8StringCR requestId);

    };
