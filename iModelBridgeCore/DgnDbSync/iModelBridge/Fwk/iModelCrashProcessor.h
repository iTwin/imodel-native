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
    BentleyApi::WebServices::ClientInfoPtr m_clientInfo;
    Utf8String m_jobRunGuid;
    Utf8String m_requestGuid;

    BentleyStatus GetCrashReportUrl(Utf8StringR url);
    iModelCrashProcessor();
    public:
        static iModelCrashProcessor& GetInstance();
        void SetClientInfo(BentleyApi::WebServices::ClientInfoPtr clientInfo);
        void SetRunInfo(Utf8StringCR jobRunGuid, Utf8StringCR requestGuid);//requestGuid == ActivityId == X-Correlation-Id
        BentleyStatus SendCrashReport(Utf8StringCR exceptionString, WCharCP filename);

    };
