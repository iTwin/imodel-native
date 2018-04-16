/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/SamlToken.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/SamlToken.h>

#include <Bentley/Base64Utilities.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlToken::SamlToken() :
m_token(),
m_dom(nullptr)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
SamlToken::SamlToken(Utf8String token) :
m_token(std::move(token))
    {
    WString error;
    BeXmlStatus status;

    BeMutexHolder lock(m_domMutex);

    m_dom = BeXmlDom::CreateAndReadFromString(status, m_token.c_str(), m_token.size(), &error);
    if (BeXmlStatus::BEXML_Success != status)
        {
        m_dom = nullptr;
        }

    if (m_dom.IsValid())
        {
        m_dom->RegisterNamespace("saml", "urn:oasis:names:tc:SAML:1.0:assertion");
        m_dom->RegisterNamespace("ds", "http://www.w3.org/2000/09/xmldsig#");
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Adam.Eichelkraut    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlToken::SamlToken(SamlTokenCR other) :
m_token(other.m_token)
    {
    // Hold onto other mutex since we are reading from other object
    BeMutexHolder otherLock(other.m_domMutex);
    m_dom = other.m_dom;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Adam.Eichelkraut    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SamlToken& SamlToken::operator=(const SamlToken& other)
    {
    if (this == &other)
        return *this;

    // Hold onto both mutexes since we are accessing both
    BeMutexHolder lock(m_domMutex);
    BeMutexHolder otherLock(other.m_domMutex);

    m_dom = other.m_dom;
    m_token = other.m_token;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool SamlToken::IsEmpty() const
    {
    return m_token.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool SamlToken::IsSupported() const
    {
    BeMutexHolder lock(m_domMutex);

    if (m_dom.IsNull())
        {
        return false;
        }

    BeXmlNodeP root = m_dom->GetRootElement();

    if (nullptr == root ||
        0 != strcmp(root->GetNamespace(), "urn:oasis:names:tc:SAML:1.0:assertion") ||
        0 != strcmp(root->GetName(), "Assertion"))
        {
        m_dom = nullptr;
        return false;
        }

    int32_t majorVersion = 0;
    int32_t minorVersion = 0;
    if (BeXmlStatus::BEXML_Success != root->GetAttributeInt32Value(majorVersion, "MajorVersion") ||
        BeXmlStatus::BEXML_Success != root->GetAttributeInt32Value(minorVersion, "MinorVersion"))
        {
        m_dom = nullptr;
        return false;
        }

    if (majorVersion == 1 && minorVersion == 1)
        {
        return true;
        }

    m_dom = nullptr;
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool SamlToken::IsValidAt(DateTimeCR dateTimeUtc) const
    {
    if (!IsSupported())
        {
        return false;
        }

    if (DateTime::Kind::Utc != dateTimeUtc.GetInfo().GetKind())
        {
        BeAssert(false);
        return false;
        }

    DateTime notBeforeUtc;
    DateTime notOnOrAfterUtc;
    if (SUCCESS != GetConditionDates(notBeforeUtc, notOnOrAfterUtc))
        {
        BeAssert(false);
        return false;
        }

    auto resultA = DateTime::Compare(dateTimeUtc, notBeforeUtc);
    auto resultB = DateTime::Compare(dateTimeUtc, notOnOrAfterUtc);

    if ((
        resultA == DateTime::CompareResult::LaterThan ||
        resultA == DateTime::CompareResult::Equals
        ) &&
        resultB == DateTime::CompareResult::EarlierThan)
        {
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool SamlToken::IsValidNow(uint32_t offsetMinutes) const
    {
    int64_t unixMilliseconds;
    if (SUCCESS != DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(unixMilliseconds))
        {
        BeAssert(false);
        return false;
        }

    unixMilliseconds += offsetMinutes * 60 * 1000;

    DateTime offsetedDateTime;
    if (SUCCESS != DateTime::FromUnixMilliseconds(offsetedDateTime, unixMilliseconds))
        {
        BeAssert(false);
        return false;
        }

    return IsValidAt(offsetedDateTime);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SamlToken::GetConditionDates(DateTime& notBeforeUtc, DateTime& notOnOrAfterUtc) const
    {
    BeMutexHolder lock(m_domMutex);

    BentleyStatus result = ERROR;

    xmlXPathContextPtr context = m_dom->AcquireXPathContext(m_dom->GetRootElement());
    xmlXPathObjectPtr conditionsPtr = m_dom->EvaluateXPathExpression("/saml:Assertion/saml:Conditions", context);

    BeXmlDom::IterableNodeSet conditions;
    conditions.Init(conditionsPtr->nodesetval);
    if (conditions.size() == 1)
        {
        if (SUCCESS == GetAttributteDateTimeUtc(conditions.front(), "NotBefore", notBeforeUtc) &&
            SUCCESS == GetAttributteDateTimeUtc(conditions.front(), "NotOnOrAfter", notOnOrAfterUtc))
            {
            result = SUCCESS;
            }
        }

    m_dom->FreeXPathObject(*conditionsPtr);
    m_dom->FreeXPathContext(*context);

    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SamlToken::GetAttributteDateTimeUtc(BeXmlNodeP node, Utf8CP name, DateTimeR dateTimeOut)
    {
    if (nullptr == node)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String dateStr;
    node->GetAttributeStringValue(dateStr, name);

    if (SUCCESS != DateTime::FromString(dateTimeOut, dateStr.c_str()))
        {
        BeAssert(false);
        return ERROR;
        }

    if (DateTime::Kind::Utc != dateTimeOut.GetInfo().GetKind())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t SamlToken::GetLifetime() const
    {
    if (!IsSupported())
        return 0;

    DateTime notBefore;
    DateTime notOnOrAfter;
    if (SUCCESS != GetConditionDates(notBefore, notOnOrAfter))
        return 0;

    int64_t notBeforeMs;
    int64_t notOnOrAfterMs;
    if (SUCCESS != notBefore.ToUnixMilliseconds(notBeforeMs) ||
        SUCCESS != notOnOrAfter.ToUnixMilliseconds(notOnOrAfterMs) ||
        notOnOrAfterMs <= notBeforeMs)
        return 0;

    int64_t lifetimeMs = notOnOrAfterMs - notBeforeMs;
    return (uint32_t) (lifetimeMs / (1000 * 60));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SamlToken::GetAttributes(bmap<Utf8String, Utf8String>& attributesOut) const
    {
    if (!IsSupported())
        {
        return ERROR;
        }

    BeMutexHolder lock(m_domMutex);

    BentleyStatus status = SUCCESS;

    xmlXPathContextPtr context = m_dom->AcquireXPathContext(m_dom->GetRootElement());
    xmlXPathObjectPtr xpathObject = m_dom->EvaluateXPathExpression("/saml:Assertion/saml:AttributeStatement/saml:Attribute", context);

    BeXmlDom::IterableNodeSet attributeNodes;
    attributeNodes.Init(*m_dom, xpathObject);

    for (BeXmlNodeP attributeNode : attributeNodes)
        {
        Utf8String name;
        Utf8String value;

        if (BEXML_Success != attributeNode->GetAttributeStringValue(name, "AttributeName"))
            {
            status = ERROR;
            break;
            }

        BeXmlNodeP valueNode = attributeNode->SelectSingleNode("saml:AttributeValue");
        if (nullptr == valueNode ||
            BEXML_Success != valueNode->GetContent(value))
            {
            status = ERROR;
            break;
            }

        attributesOut[name] = value;
        }

    m_dom->FreeXPathObject(*xpathObject);
    m_dom->FreeXPathContext(*context);

    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Bentley Systems 04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SamlToken::GetX509Certificate(Utf8StringR certOut) const
    {
    if (!IsSupported())
        {
        return ERROR;
        }

    BeMutexHolder lock(m_domMutex);

    BentleyStatus status = ERROR;
    auto expression = "/saml:Assertion/ds:Signature/ds:KeyInfo/ds:X509Data/ds:X509Certificate/text()";

    xmlXPathContextPtr context = m_dom->AcquireXPathContext(m_dom->GetRootElement());
    xmlXPathObjectPtr xpathObject = m_dom->EvaluateXPathExpression(expression, context);

    BeXmlDom::IterableNodeSet certificates;
    certificates.Init(*m_dom, xpathObject);

    if (certificates.size() == 1 &&
        BeXmlStatus::BEXML_Success == certificates.front()->GetContent(certOut))
        {
        status = SUCCESS;
        }

    m_dom->FreeXPathObject(*xpathObject);
    m_dom->FreeXPathContext(*context);

    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SamlToken::ToAuthorizationString() const
    {
    return Base64Utilities::Encode(m_token);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SamlToken::AsString() const
    {
    return m_token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SamlToken::operator==(const ISecurityToken& other) const
    {
    const SamlToken* token = dynamic_cast<const SamlToken*>(&other);
    if (token == nullptr) return false;
    return m_token == token->m_token;
    }
