/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/WMSCapabilities.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/WMSCapabilities.h>
#include "WMSTag.h"

#ifndef BENTLEY_WINRT
#include <curl/curl.h>
#endif

USING_BENTLEY_NAMESPACE_WMSPARSER

//=====================================================================================
//                              UtilityFunctions
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
//&&JFC: To be removed eventually.
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*) userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL)
        return 0;

    //Byte* bob = ((Byte*)mem->memory);
    memcpy(&(((Byte*) mem->memory)[mem->size]), contents, realsize);
    mem->size += realsize;
    ((Byte*) mem->memory)[mem->size] = 0;

    return realsize;
    }

//=====================================================================================
//                              WMSCapabilities
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilities::WMSCapabilities(WString version) { m_version = version; }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilities::~WMSCapabilities() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSCapabilities::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }
    
    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Service))
        m_pService = WMSService::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Capability))
        m_pCapability = WMSCapability::Create(status, xmlDom, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
//&&JFC: To be removed eventually.
WMSCapabilitiesPtr WMSCapabilities::CreateAndReadFromUrl(WMSParserStatus& status, WCharCP url)
    {
    MemoryStruct chunk;
    chunk.memory = malloc(1);  // Will be grown as needed by the realloc above.
    chunk.size = 0;    // No data at this point.
    if (GetFromServer(chunk, url) != SUCCESS)
        return 0;

    return CreateAndReadFromMemory(status, chunk.memory, chunk.size);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilitiesPtr WMSCapabilities::CreateAndReadFromMemory(WMSParserStatus& status, void const* xmlBuffer, size_t xmlBufferSize, WStringP pParseError)
    {
    status = WMSParserStatus::Success;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, xmlBuffer, xmlBufferSize, pParseError);
    if (BEXML_Success != xmlStatus)
        return NULL;

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    WString version;
    if (BEXML_Success != pRootNode->GetAttributeStringValue(version, WMS_ROOT_ATTRIBUTE_Version) || version.empty())
        {
        BeDataAssert(!"Missing wms version attribute.");
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    pXmlDom->RegisterNamespace(WMS_PREFIX, WMS_NAMESPACE_1_0);

    WMSCapabilitiesPtr pCapabilities = WMSCapabilities::Create(status, version);
    if (WMSParserStatus::Success != status)
        return NULL;

    //Read nodes.
    pCapabilities->Read(status, WMS_ELEMENT_Service, *pXmlDom, *pRootNode);
    pCapabilities->Read(status, WMS_ELEMENT_Capability, *pXmlDom, *pRootNode);

    //if (WMSParserStatus::Success != status)
    //    return NULL;

    return pCapabilities;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilitiesPtr WMSCapabilities::CreateAndReadFromFile(WMSParserStatus& status)
    {
    //&&JFC: TODO
    return 0;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilitiesPtr WMSCapabilities::CreateFromString(WMSParserStatus& status, Utf8CP pSource, WStringP pParseError /* = NULL */)
    {
    status = WMSParserStatus::Success;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, pSource, 0, pParseError);
    if (BEXML_Success != xmlStatus)
        {
        status = WMSParserStatus::XmlReadError;
        return NULL;
        }

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    WString version;
    if (BEXML_Success != pRootNode->GetAttributeStringValue(version, WMS_ROOT_ATTRIBUTE_Version) || version.empty())
        {
        BeDataAssert(!"Missing wms version attribute.");
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    pXmlDom->RegisterNamespace(WMS_PREFIX, WMS_NAMESPACE_1_0);

    WMSCapabilitiesPtr pCapabilities = WMSCapabilities::Create(status, version);
    if (WMSParserStatus::Success != status)
        return NULL;

    //Read nodes.
    pCapabilities->Read(status, WMS_ELEMENT_Service, *pXmlDom, *pRootNode);
    //pCapabilities->Read(status, WMS_ELEMENT_Capability, *pXmlDom, *pRootNode);

    //if (WMSParserStatus::Success != status)
    //    return NULL;

    return pCapabilities;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
bool WMSCapabilities::IsValid() const
    {
    if (!m_version.Equals(L"1.3.0") ||
        !m_version.Equals(L"1.1.1") ||
        !m_version.Equals(L"1.1.0") ||
        !m_version.Equals(L"1.0.0"))
        return false;

    if (m_pService.IsNull() ||
        m_pCapability.IsNull())
        return false;

    return true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilitiesPtr WMSCapabilities::Create(WMSParserStatus& status, WString version)
    {
    if (version.Equals(L"1.3.0") ||
        version.Equals(L"1.1.1") ||
        version.Equals(L"1.1.0") ||
        version.Equals(L"1.0.0"))
        return new WMSCapabilities(version);

    status = WMSParserStatus::UnknownVersion;
    return NULL;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
//&&JFC: To be removed eventually.
StatusInt WMSCapabilities::GetFromServer(MemoryStruct& chunk, WCharCP url)
    {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    // Init the curl session.
    curl = curl_easy_init();
    if (!curl)
        return ERROR;

    // Specify URL to get.
    curl_easy_setopt(curl, CURLOPT_URL, Utf8String(url));

    // Send all data to this function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    // We pass our struct to the callback function.   
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    // Some servers don't like requests that are made 
    // without a user-agent field, so we provide one.
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Perform the request, res will get the return code.
    res = curl_easy_perform(curl);

    // Check for errors.
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    else
        //printf("%lu bytes retrieved\n", (long) xmlDoc.size);

        // Always cleanup.
        curl_easy_cleanup(curl);

    // We are done with libcurl, so clean it up.
    curl_global_cleanup();

    return SUCCESS;
    }

//=====================================================================================
//                                      WMSList
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSFormatListPtr WMSFormatList::Create(WMSParserStatus& status, BeXmlNodeR pParentNode)
    {
    WMSFormatListPtr pFormatList = new WMSFormatList();

    WString format = L"";
    pParentNode.GetContent(format);
    pFormatList->m_formatList.push_back(format);

    for (BeXmlNodeP siblingNode = pParentNode.GetNextSibling(); NULL != siblingNode; siblingNode = siblingNode->GetNextSibling())
        {
        if (0 == BeStringUtilities::Stricmp(siblingNode->GetName(), WMS_ELEMENT_FormatList))
            {
            siblingNode->GetContent(format);
            pFormatList->m_formatList.push_back(format);
            }
        }

    return pFormatList;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSElementListPtr WMSElementList::Create(WMSParserStatus& status, BeXmlNodeR pParentNode)
    {
    WMSElementListPtr pElementList = new WMSElementList();

    WString element = L"";
    for (BeXmlNodeP childElement = pParentNode.GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        childElement->GetContent(element);
        pElementList->m_elementList.push_back(element);
        }

    return pElementList;
    }

//=====================================================================================
//                              WMSService
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSService::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_KeywordList))
        m_pKeywordList = WMSElementList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_OnlineResource))
        m_pOnlineResource = WMSOnlineResource::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_ContactInformation))
        m_pContactInformation = WMSContactInformation::Create(status, xmlDom, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSServicePtr WMSService::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSServicePtr pService = new WMSService();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);
    
    // Read string.
    xmlDom.SelectNodeContent(pService->GetNameR(), WMS_ELEMENT_Name, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetTitleR(), WMS_ELEMENT_Title, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetAbstractR(), WMS_ELEMENT_Abstract, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetFeesR(), WMS_ELEMENT_Fees, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetAccessConstraintsR(), WMS_ELEMENT_AccessConstraints, pContext, BeXmlDom::NODE_BIAS_First);
    
    // Read positive integer.
    WString posInt = L"";
    xmlDom.SelectNodeContent(posInt, WMS_ELEMENT_LayerLimit, pContext, BeXmlDom::NODE_BIAS_First);
    pService->SetLayerLimit(posInt);
    xmlDom.SelectNodeContent(posInt, WMS_ELEMENT_MaxWidth, pContext, BeXmlDom::NODE_BIAS_First);
    pService->SetMaxWidth(posInt);
    xmlDom.SelectNodeContent(posInt, WMS_ELEMENT_MaxHeight, pContext, BeXmlDom::NODE_BIAS_First);
    pService->SetMaxHeight(posInt);
    
    // Read complex type.
    pService->Read(status, WMS_ELEMENT_KeywordList, xmlDom, parentNode);
    pService->Read(status, WMS_ELEMENT_OnlineResource, xmlDom, parentNode);
    pService->Read(status, WMS_ELEMENT_ContactInformation, xmlDom, parentNode);

    return pService;
    }

//=====================================================================================
//                              WMSOnlineResource
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSOnlineResourcePtr WMSOnlineResource::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSOnlineResourcePtr pOnlineRes = new WMSOnlineResource();

    parentNode.GetAttributeStringValue(pOnlineRes->GetTypeR(), "type");
    parentNode.GetAttributeStringValue(pOnlineRes->GetHrefR(), "href");

    return pOnlineRes;
    }

//=====================================================================================
//                              WMSContactInformation
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSContactInformation::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_ContactPerson))
        m_pPerson = WMSContactPerson::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_ContactAddress))
        m_pAddress = WMSContactAddress::Create(status, xmlDom, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSContactInformationPtr WMSContactInformation::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSContactInformationPtr pContactInfo = new WMSContactInformation();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pContactInfo->GetPositionR(), WMS_ELEMENT_Position, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactInfo->GetVoiceTelephoneR(), WMS_ELEMENT_VoiceTel, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactInfo->GetFacsimileTelephoneR(), WMS_ELEMENT_FacsimileTel, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactInfo->GetEmailAddressR(), WMS_ELEMENT_EmailAddress, pContext, BeXmlDom::NODE_BIAS_First);

    // Read complex type.
    pContactInfo->Read(status, WMS_ELEMENT_ContactPerson, xmlDom, parentNode);
    pContactInfo->Read(status, WMS_ELEMENT_ContactAddress, xmlDom, parentNode);

    return pContactInfo;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSContactPersonPtr WMSContactPerson::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSContactPersonPtr pContactPerson = new WMSContactPerson();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pContactPerson->GetNameR(), WMS_ELEMENT_Person, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactPerson->GetOrganizationR(), WMS_ELEMENT_Organization, pContext, BeXmlDom::NODE_BIAS_First);

    return pContactPerson;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSContactAddressPtr WMSContactAddress::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSContactAddressPtr pContactAddress = new WMSContactAddress();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pContactAddress->GetTypeR(), WMS_ELEMENT_Type, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetAddressR(), WMS_ELEMENT_Address, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetCityR(), WMS_ELEMENT_City, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetStateOrProvinceR(), WMS_ELEMENT_StateProv, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetPostCodeR(), WMS_ELEMENT_PostCode, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetCountryR(), WMS_ELEMENT_Country, pContext, BeXmlDom::NODE_BIAS_First);

    return pContactAddress;
    }

//=====================================================================================
//                              WMSCapability
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSCapability::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Request))
        m_pRequest = WMSRequest::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Exception))
        m_pExceptionList = WMSElementList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Layer))
        m_pLayerList.push_back(WMSLayer::Create(status, xmlDom, *pNode));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilityPtr WMSCapability::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSCapabilityPtr pCapability = new WMSCapability();

    // Read complex type.
    pCapability->Read(status, WMS_ELEMENT_Request, xmlDom, parentNode);
    pCapability->Read(status, WMS_ELEMENT_Exception, xmlDom, parentNode);
    pCapability->Read(status, WMS_ELEMENT_Layer, xmlDom, parentNode);

    return pCapability;
    }

//=====================================================================================
//                              WMSRequest
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSRequest::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GetCapabilities))
        m_pGetCapabilities = WMSOperationType::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GetMap))
        m_pGetMap = WMSOperationType::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GetFeatureInfo))
        m_pGetFeatureInfo = WMSOperationType::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSRequestPtr WMSRequest::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSRequestPtr pRequest = new WMSRequest();

    // Read complex type.
    pRequest->Read(status, WMS_ELEMENT_GetCapabilities, xmlDom, parentNode);
    pRequest->Read(status, WMS_ELEMENT_GetMap, xmlDom, parentNode);
    pRequest->Read(status, WMS_ELEMENT_GetFeatureInfo, xmlDom, parentNode);

    return pRequest;
    }

//=====================================================================================
//                              WMSOperationType
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSOperationType::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_FormatList))
        m_pFormatList = WMSFormatList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_DCPType))
        m_pDcpType = WMSDCPType::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSOperationTypePtr WMSOperationType::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSOperationTypePtr pOperationType = new WMSOperationType();

    // Read complex type.
    pOperationType->Read(status, WMS_ELEMENT_FormatList, parentNode);
    pOperationType->Read(status, WMS_ELEMENT_DCPType, parentNode);

    return pOperationType;
    }

//=====================================================================================
//                                  WMSDCPType
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSDCPType::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    // No need for HTTP node, we only want to process his child's.
    BeXmlNodeP pNode = parentNode.SelectSingleNode(WMS_ELEMENT_Http)->SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_HttpGet))
        // No need for HTTPGet node, we only want to process his child's.
        m_pHttpGet = WMSOnlineResource::Create(status, *pNode->SelectSingleNode(WMS_ELEMENT_OnlineResource));
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_HttpPost))
        // No need for HTTPPost node, we only want to process his child's.
        m_pHttpPost = WMSOnlineResource::Create(status, *pNode->SelectSingleNode(WMS_ELEMENT_OnlineResource));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSDCPTypePtr WMSDCPType::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSDCPTypePtr pDcpType = new WMSDCPType();

    // Read complex type.
    pDcpType->Read(status, WMS_ELEMENT_HttpGet, parentNode);
    pDcpType->Read(status, WMS_ELEMENT_HttpPost, parentNode);

    return pDcpType;
    }

//=====================================================================================
//                              WMSLayer
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSLayer::CreateChilds(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlDom::IterableNodeSet nodes;
    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);
    xmlDom.SelectNodes(nodes, WMS_ELEMENT_Layer, pContext);

    WMSLayerPtr pChildLayer = new WMSLayer();
    for (BeXmlNodeP pChildNode : nodes)
        {
        pChildLayer = WMSLayer::Create(status, xmlDom, *pChildNode);

        m_pLayerList.push_back(pChildLayer);
        }

    /*
    WMSLayerPtr pChildLayer = new WMSLayer();

    for (BeXmlNodeP childNode = parentNode.GetFirstChild(); NULL != childNode; childNode = childNode->GetNextSibling())
        {
        if (0 == BeStringUtilities::Stricmp(childNode->GetName(), WMS_ELEMENT_Layer))
            {
            pChildLayer = WMSLayer::Create(status, xmlDom, *childNode);

            m_pLayerList.push_back(pChildLayer);
            }
        }
    */
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSLayer::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_KeywordList))
        m_pKeywordList = WMSElementList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_CRS) || 
             0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_SRS))
        m_pCRSList = WMSFormatList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GeoBoundingBox) ||
             0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_LatLongBoundingBox))
        m_pGeoBBox = WMSGeoBoundingBox::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_BoundingBox))
        m_pBBoxList.push_back(WMSBoundingBox::Create(status, *pNode));
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Dimension))
        m_pDimensionList.push_back(WMSDimension::Create(status, xmlDom, *pNode));
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Attribution))
        m_pAttribution = WMSAttribution::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_AuthorityURL))
        m_pAuthorityUrl = WMSUrl::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Identifier))
        m_pIdentifier = WMSIdentifier::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_MetadataURL))
        m_pMetadataUrlList.push_back(WMSUrl::Create(status, *pNode));
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_DataURL))
        m_pDataUrl = WMSUrl::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_FeatureListURL))
        m_pFeatureListUrl = WMSUrl::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Style))
        m_pStyle = WMSStyle::Create(status, xmlDom, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSLayerPtr WMSLayer::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSLayerPtr pLayer = new WMSLayer();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read attributes.
    WString stringValue = L"";
    parentNode.GetAttributeStringValue(stringValue, "queryable");
    pLayer->SetQueryable(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "opaque");
    pLayer->SetOpaque(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "noSubsets");
    pLayer->SetSubsets(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "cascaded");
    pLayer->SetCascaded(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "fixedWidth");
    pLayer->SetFixedWidth(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "fixedHeight");
    pLayer->SetFixedHeight(stringValue);

    // Read simple type.
    xmlDom.SelectNodeContent(pLayer->GetNameR(), WMS_ELEMENT_Name, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pLayer->GetTitleR(), WMS_ELEMENT_Title, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pLayer->GetAbstractR(), WMS_ELEMENT_Abstract, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(stringValue, WMS_ELEMENT_MinScaleDenom, pContext, BeXmlDom::NODE_BIAS_First);
    pLayer->SetMinScaleDenom(stringValue);
    xmlDom.SelectNodeContent(stringValue, WMS_ELEMENT_MaxScaleDenom, pContext, BeXmlDom::NODE_BIAS_First);
    pLayer->SetMaxScaleDenom(stringValue);

    // Read complex type.
    pLayer->Read(status, WMS_ELEMENT_KeywordList, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_BoundingBox, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_AuthorityURL, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_Identifier, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_MetadataURL, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_DataURL, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_FeatureListURL, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_Style, xmlDom, parentNode);

    pLayer->Read(status, WMS_ELEMENT_CRS, xmlDom, parentNode);
    if (WMSParserStatus::Success != status)
        pLayer->Read(status, WMS_ELEMENT_SRS, xmlDom, parentNode);

    pLayer->Read(status, WMS_ELEMENT_GeoBoundingBox, xmlDom, parentNode);
    if (WMSParserStatus::Success != status)
        pLayer->Read(status, WMS_ELEMENT_LatLongBoundingBox, xmlDom, parentNode);

    // Create sublayers.
    pLayer->CreateChilds(status, xmlDom, parentNode);
            

    return pLayer;
    }

//=====================================================================================
//                              WMSBoundingBoxes
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSGeoBoundingBoxPtr WMSGeoBoundingBox::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSGeoBoundingBoxPtr pGeoBBox = new WMSGeoBoundingBox();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    WString stringValue = L"";
    xmlDom.SelectNodeContent(stringValue, WMS_ELEMENT_WestBoundLong, pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetWestBoundLong(stringValue);
    xmlDom.SelectNodeContent(stringValue, WMS_ELEMENT_EastBoundLong, pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetEastBoundLong(stringValue);
    xmlDom.SelectNodeContent(stringValue, WMS_ELEMENT_SouthBoundLat, pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetSouthBoundLat(stringValue);
    xmlDom.SelectNodeContent(stringValue, WMS_ELEMENT_NorthBoundLat, pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetNorthBoundLat(stringValue);

    return pGeoBBox;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSBoundingBoxPtr WMSBoundingBox::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSBoundingBoxPtr pBBox = new WMSBoundingBox();

    // Read attributes.
    WString stringValue = L"";
    parentNode.GetAttributeStringValue(stringValue, "minx");
    pBBox->SetMinX(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "miny");
    pBBox->SetMinY(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "maxx");
    pBBox->SetMaxX(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "maxy");
    pBBox->SetMaxY(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "resx");
    pBBox->SetResX(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "resy");
    pBBox->SetResY(stringValue);

    parentNode.GetAttributeStringValue(pBBox->GetCRSR(), "CRS");
    if (pBBox->m_crs.empty())
        parentNode.GetAttributeStringValue(pBBox->GetCRSR(), "SRS");

    return pBBox;
    }

//=====================================================================================
//                              WMSDimension
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSDimensionPtr WMSDimension::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSDimensionPtr pDimension = new WMSDimension();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read attributes.
    parentNode.GetAttributeStringValue(pDimension->GetNameR(), "name");
    parentNode.GetAttributeStringValue(pDimension->GetUnitsR(), "units");
    parentNode.GetAttributeStringValue(pDimension->GetUnitSymbolR(), "unitSymbol");
    parentNode.GetAttributeStringValue(pDimension->GetDefaultR(), "default");
    WString stringValue = L"";
    parentNode.GetAttributeStringValue(stringValue, "multipleValues");
    pDimension->SetMultipleValues(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "nearestValue");
    pDimension->SetNearestValue(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "current");
    pDimension->SetCurrent(stringValue);

    // Read string.
    xmlDom.SelectNodeContent(pDimension->GetDimensionR(), WMS_ELEMENT_Dimension, pContext, BeXmlDom::NODE_BIAS_First);

    return pDimension;
    }

//=====================================================================================
//                              WMSAttribution
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSAttribution::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_OnlineResource))
        m_pOnlineRes = WMSOnlineResource::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_LogoURL))
        m_pLogoUrl = WMSUrl::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSAttributionPtr WMSAttribution::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSAttributionPtr pAttribution = new WMSAttribution();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pAttribution->GetTitleR(), WMS_ELEMENT_Title, pContext, BeXmlDom::NODE_BIAS_First);

    // Read complex type.
    pAttribution->Read(status, WMS_ELEMENT_OnlineResource, parentNode);
    pAttribution->Read(status, WMS_ELEMENT_LogoURL, parentNode);

    return pAttribution;
    }

//=====================================================================================
//                                  WMSUrl
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSUrl::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_FormatList))
        m_pFormatList = WMSFormatList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_OnlineResource))
        m_pOnlineRes = WMSOnlineResource::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSUrlPtr WMSUrl::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSUrlPtr pUrl = new WMSUrl();

    // Read attributes.
    parentNode.GetAttributeStringValue(pUrl->GetTypeR(), "type");
    parentNode.GetAttributeStringValue(pUrl->GetNameR(), "name");
    WString stringValue = L"";
    parentNode.GetAttributeStringValue(stringValue, "height");
    pUrl->SetHeight(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "width");
    pUrl->SetWidth(stringValue);

    // Read complex type.
    pUrl->Read(status, WMS_ELEMENT_FormatList, parentNode);
    pUrl->Read(status, WMS_ELEMENT_OnlineResource, parentNode);

    return pUrl;
    }

//=====================================================================================
//                              WMSIdentifier
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSIdentifierPtr WMSIdentifier::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSIdentifierPtr pIdentifier = new WMSIdentifier();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read attributes.
    parentNode.GetAttributeStringValue(pIdentifier->GetAuthorityR(), "authority");

    // Read string.
    xmlDom.SelectNodeContent(pIdentifier->GetIDR(), WMS_ELEMENT_Identifier, pContext, BeXmlDom::NODE_BIAS_First);

    return pIdentifier;
    }

//=====================================================================================
//                                  WMSStyle
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSStyle::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(nodeName);
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_LegendURL))
        m_pLegendUrl = WMSUrl::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_StyleSheetURL))
        m_pStyleSheetUrl = WMSUrl::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_StyleURL))
        m_pStyleUrl = WMSUrl::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSStylePtr WMSStyle::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSStylePtr pStyle = new WMSStyle();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pStyle->GetNameR(), WMS_ELEMENT_Name, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pStyle->GetTitleR(), WMS_ELEMENT_Title, pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pStyle->GetAbstractR(), WMS_ELEMENT_Abstract, pContext, BeXmlDom::NODE_BIAS_First);

    // Read complex type.
    pStyle->Read(status, WMS_ELEMENT_LegendURL, parentNode);
    pStyle->Read(status, WMS_ELEMENT_StyleSheetURL, parentNode);
    pStyle->Read(status, WMS_ELEMENT_StyleURL, parentNode);

    return pStyle;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
/*
static size_t HttpBodyParser(void* ptr, size_t size, size_t nmemb, void* userp)
{
size_t totalSize = size * nmemb;
auto buffer = (bvector<Byte>*) userp;
buffer->insert(buffer->end(), (Byte*) ptr, (Byte*) ptr + totalSize);
return totalSize;
}
*/
