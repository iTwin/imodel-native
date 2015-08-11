/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Configuration/BuddiClient.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Configuration/BuddiClient.h>
#include <MobileDgn/Utils/Http/HttpError.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BuddiClient::BuddiClient(IHttpHandlerPtr customHandler, Utf8String url) :
m_client(nullptr, customHandler), m_url(url)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<BuddiRegionsResult> BuddiClient::GetRegions()
    {
    HttpRequest request = m_client.CreatePostRequest(m_url);
    Utf8String body = R"xml(<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
    <soap:Body>
        <GetRegions xmlns="http://tempuri.org/" />
    </soap:Body>
</soap:Envelope>)xml";

    request.SetValidateCertificate(true);
    request.SetRequestBody(HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("text/xml; charset=utf-8");

    return request.PerformAsync()->Then<BuddiRegionsResult>([=] (HttpResponse& response)
        {
        if (response.GetConnectionStatus() != ConnectionStatus::OK ||
            response.GetHttpStatus() != HttpStatus::OK)
            {
            return BuddiRegionsResult::Error(BuddiError(response));
            }

        BeXmlDomPtr dom = ParseSoapXml(response.GetBody().AsString());
        if (!IsSupported(dom))
            {
            BeAssert(false && "Invalid XML");
            return BuddiRegionsResult::Error(BuddiError());
            }

        auto xpathExpression =
            "/soap:Envelope/soap:Body/*[namespace-uri()='http://tempuri.org/' and "
            "local-name()='GetRegionsResponse']/*[namespace-uri()='http://tempuri.org/' and "
            "local-name()='GetRegionsResult']/regions/region";

        xmlXPathContextPtr context = dom->AcquireXPathContext(dom->GetRootElement());
        xmlXPathObjectPtr xpathObject = dom->EvaluateXPathExpression(xpathExpression, context);

        BeXmlDom::IterableNodeSet attributeNodes;
        attributeNodes.Init(*dom, xpathObject);

        bvector<BuddiRegion> regions;

        for (BeXmlNodeP attributeNode : attributeNodes)
            {
            Utf8String name;
            uint32_t id;

            if (BEXML_Success != attributeNode->GetAttributeStringValue(name, "name") ||
                BEXML_Success != attributeNode->GetAttributeUInt32Value(id, "id"))
                {
                BeAssert(false && "Unable to get value from XML");
                return BuddiRegionsResult::Error(BuddiError());
                }

            regions.push_back(BuddiRegion(name, id));
            }

        dom->FreeXPathObject(*xpathObject);
        dom->FreeXPathContext(*context);

        return BuddiRegionsResult::Success(regions);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<BuddiUrlResult> BuddiClient::GetUrl(Utf8StringCR url, uint32_t id)
    {
    HttpRequest request = m_client.CreatePostRequest(m_url);
    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
    <soap:Body>
        <GetUrl xmlns="http://tempuri.org/">
            <urlName>%s</urlName>
            <region>%u</region>
        </GetUrl>
    </soap:Body>
</soap:Envelope>)xml", url.c_str(), id);

    request.SetValidateCertificate(true);
    request.SetRequestBody(HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("text/xml; charset=utf-8");

    return request.PerformAsync()->Then<BuddiUrlResult>([=] (HttpResponse& response)
        {
        if (response.GetConnectionStatus() != ConnectionStatus::OK ||
            response.GetHttpStatus() != HttpStatus::OK)
            {
            return BuddiUrlResult::Error(BuddiError(response));
            }

        BeXmlDomPtr dom = ParseSoapXml(response.GetBody().AsString());
        if (!IsSupported(dom))
            {
            BeAssert(false && "Invalid XML");
            return BuddiUrlResult::Error(BuddiError());
            }

        auto xpathExpression =
            "/soap:Envelope/soap:Body/*[namespace-uri()='http://tempuri.org/' and "
            "local-name()='GetUrlResponse']/*[namespace-uri()='http://tempuri.org/' and "
            "local-name()='GetUrlResult']";

        Utf8String url;
        BeXmlNodeP valueNode = dom->GetRootElement()->SelectSingleNode(xpathExpression);
        if (nullptr == valueNode ||
            BEXML_Success != valueNode->GetContent(url))
            {
            BeAssert(false && "Unable to get value from XML");
            return BuddiUrlResult::Error(BuddiError());
            }

        if (url.empty())
            {
            return BuddiUrlResult::Error(BuddiError(BuddiError::UrlNotConfigured));
            }

        return BuddiUrlResult::Success(url);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlDomPtr BuddiClient::ParseSoapXml(Utf8StringCR result)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr dom = BeXmlDom::CreateAndReadFromString(xmlStatus, result.c_str(), result.size());
    if (BeXmlStatus::BEXML_Success != xmlStatus || !dom.IsValid())
        {
        return nullptr;
        }

    dom->RegisterNamespace("soap", "http://schemas.xmlsoap.org/soap/envelope/");
    return dom;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool BuddiClient::IsSupported(BeXmlDomPtr dom)
    {
    if (dom.IsNull())
        {
        return false;
        }

    if (nullptr == dom->GetRootElement())
        {
        return false;
        }

    return true;
    }
