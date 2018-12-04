/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Configuration/BuddiClient.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <Bentley/Tasks/AsyncResult.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <BeHttp/HttpClient.h>
#include <BeXml/BeXml.h>

#include <WebServices/Configuration/BuddiError.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

typedef std::shared_ptr<struct IBuddiClient> IBuddiClientPtr;
typedef AsyncResult<bvector<struct BuddiRegion>, BuddiError> BuddiRegionsResult;
typedef AsyncResult<Utf8String, BuddiError> BuddiUrlResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE IBuddiClient
    {
    virtual ~IBuddiClient() {}
    virtual AsyncTaskPtr<BuddiRegionsResult> GetRegions() = 0;
    virtual AsyncTaskPtr<BuddiUrlResult> GetUrl(Utf8StringCR urlName, uint32_t regionId = 0) = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE BuddiClient : public IBuddiClient
    {
    private:
        HttpClient m_client;
        Utf8String m_url;
        ITaskSchedulerPtr m_sheduler;

    private:
        static BeXmlDomPtr ParseSoapXml(Utf8StringCR xmlBody);
        static bool IsSupported(BeXmlDomPtr dom);

    public:
        //! Create new client to BUDDI service
        //! @param customHandler 
        //! @param customUrl - provide custom URL for client to connect to
        //! @param sheduler - custom task sheduler in order to use blocking APIs without deadlocking default sheduler.
        WSCLIENT_EXPORT BuddiClient
            (
            IHttpHandlerPtr customHandler = nullptr,
            Utf8String customUrl = nullptr,
            ITaskSchedulerPtr sheduler = nullptr
            );

        //! Sends http request and returns regions
        WSCLIENT_EXPORT AsyncTaskPtr<BuddiRegionsResult> GetRegions() override;

        //! Sends http request and gets URL registered in BUDDI (https://buddi.bentley.com)
        //! @param[in] urlName    URL name, registered in the BUDDI
        //! @param[in] regionId   BUDDI region ID. 0 to use default URLs
        WSCLIENT_EXPORT AsyncTaskPtr<BuddiUrlResult> GetUrl(Utf8StringCR urlName, uint32_t regionId = 0) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct BuddiRegion
    {
    private:
        Utf8String m_name;
        uint32_t m_id;

    public:
        BuddiRegion(Utf8String name, uint32_t id) : m_name(name), m_id(id) {}
        Utf8StringCR GetName() { return m_name; }
        uint32_t GetId() { return m_id; }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
