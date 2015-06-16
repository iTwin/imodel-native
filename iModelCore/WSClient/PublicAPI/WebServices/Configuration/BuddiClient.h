/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Configuration/BuddiClient.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Threading/AsyncResult.h>
#include <MobileDgn/Utils/Threading/AsyncTask.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <BeXml/BeXml.h>

#include <WebServices/Configuration/BuddiError.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef AsyncResult<bvector<struct BuddiRegion>, BuddiError> BuddiRegionsResult;
typedef AsyncResult<Utf8String, BuddiError> BuddiUrlResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct BuddiClient
    {
    private:
        HttpClient m_client;
        Utf8String m_url;

    private:
        static BeXmlDomPtr ParseSoapXml(Utf8StringCR xmlBody);
        static bool IsSupported(BeXmlDomPtr dom);

    public:
        WSCLIENT_EXPORT BuddiClient(IHttpHandlerPtr customHandler = nullptr, Utf8String url = "http://buddi.bentley.com/discovery.asmx");

        //! Sends http request and returns regions
        WSCLIENT_EXPORT AsyncTaskPtr<BuddiRegionsResult> GetRegions();

        //! Sends http request and gets URL registered in BUDDI (http://buddi.bentley.com)
        //@param[in] urlName    URL name, registered in the BUDDI
        //@param[in] regionId   BUDDI region ID. 0 to use default URLs
        WSCLIENT_EXPORT AsyncTaskPtr<BuddiUrlResult> GetUrl(Utf8StringCR urlName, uint32_t regionId = 0);
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
        
        Utf8StringCR GetName()
            {
            return m_name;
            }

        uint32_t GetId()
            {
            return m_id;
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
