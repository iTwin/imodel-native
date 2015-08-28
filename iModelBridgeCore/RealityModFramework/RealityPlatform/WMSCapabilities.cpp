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

USING_NAMESPACE_BENTLEY_WMSPARSER

Utf8String WMSCapabilities::m_namespace = "";

//=====================================================================================
//                              UtilityFunctions
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 4/2015
//-------------------------------------------------------------------------------------
Utf8String BuildNodePath(Utf8CP nodeName)
    {
    if (WMSCapabilities::GetNamespace().empty())
        return nodeName;
    else 
        {
        // Append prefix to nodeName.
        Utf8String node;
        node.append(WMS_PREFIX);
        node.append(":");
        node.append(nodeName);

        /*
        WString nodePath;
        nodePath.AppendUtf8(WMS_PREFIX);
        nodePath.AppendUtf8(":");
        nodePath.AppendUtf8(nodeName);

        Utf8String node;
        BeStringUtilities::WCharToUtf8(node, nodePath.c_str(), nodePath.size());
        */

        return node;
        }
    }

//=====================================================================================
//                              WMSCapabilities
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSCapabilities::WMSCapabilities(Utf8StringCR version)
    : m_version(version)    
    {
    m_pService = WMSService::Create();
    m_pCapability = WMSCapability::Create(); 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSCapabilities::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
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
WMSCapabilitiesPtr WMSCapabilities::CreateAndReadFromString(WMSParserStatus& status, Utf8CP source, WStringP errorMsg)
    {
    status = WMSParserStatus::Success;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, source, 0, errorMsg);
    if (BEXML_Success != xmlStatus)
        {
        status = WMSParserStatus::XmlReadError;
        return NULL;
        }

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    // Read version attribute and valid it.
    Utf8String version;
    if (BEXML_Success != pRootNode->GetAttributeStringValue(version, WMS_ROOT_ATTRIBUTE_Version))
        {
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    if (!version.Equals("1.3.0") &&
        !version.Equals("1.1.1") &&
        !version.Equals("1.1.0") &&
        !version.Equals("1.0.0"))
        {
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    WMSCapabilitiesPtr pCapabilities = new WMSCapabilities(version);
    if (!pCapabilities.IsValid())
        return NULL;

    //Read namespace attribute.
    Utf8CP xmlns = pRootNode->GetNamespace();
    if (NULL != xmlns)
        {
        pCapabilities->SetNamespace(xmlns);
        pXmlDom->RegisterNamespace(WMS_PREFIX, xmlns);
        }

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
WMSCapabilitiesPtr WMSCapabilities::CreateAndReadFromMemory(WMSParserStatus& status, void const* xmlBuffer, size_t xmlBufferSize, WStringP errorMsg)
    {
    status = WMSParserStatus::Success;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, xmlBuffer, xmlBufferSize, errorMsg);
    if (BEXML_Success != xmlStatus)
        return NULL;

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    // Read and validate version attribute.
    Utf8String version;
    if (BEXML_Success != pRootNode->GetAttributeStringValue(version, WMS_ROOT_ATTRIBUTE_Version))
        {
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    if (!version.Equals("1.3.0") &&
        !version.Equals("1.1.1") &&
        !version.Equals("1.1.0") &&
        !version.Equals("1.0.0"))
        {
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    WMSCapabilitiesPtr pCapabilities = new WMSCapabilities(version);
    if (!pCapabilities.IsValid())
        return NULL;

    //Read namespace attribute.
    Utf8CP xmlns = pRootNode->GetNamespace();
    if (NULL != xmlns)
        {
        pCapabilities->SetNamespace(xmlns);
        pXmlDom->RegisterNamespace(WMS_PREFIX, xmlns);
        } 

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
WMSCapabilitiesPtr WMSCapabilities::CreateAndReadFromFile(WMSParserStatus& status, Utf8CP fileName, WStringP errorMsg)
    {
    status = WMSParserStatus::Success;

    BeXmlStatus xmlStatus = BEXML_Success;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, fileName, errorMsg);
    if (BEXML_Success != xmlStatus)
        {
        status = WMSParserStatus::XmlReadError;
        return NULL;
        }

    BeXmlNodeP pRootNode = pXmlDom->GetRootElement();
    if (NULL == pRootNode)
        return NULL;

    // Read version attribute and valid it.
    Utf8String version;
    if (BEXML_Success != pRootNode->GetAttributeStringValue(version, WMS_ROOT_ATTRIBUTE_Version))
        {
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    if (!version.Equals("1.3.0") &&
        !version.Equals("1.1.1") &&
        !version.Equals("1.1.0") &&
        !version.Equals("1.0.0"))
        {
        status = WMSParserStatus::UnknownVersion;
        return NULL;
        }

    WMSCapabilitiesPtr pCapabilities = new WMSCapabilities(version);
    if (!pCapabilities.IsValid())
        return NULL;

    //Read namespace attribute.
    Utf8CP xmlns = pRootNode->GetNamespace();
    if (NULL != xmlns)
        {
        pCapabilities->SetNamespace(xmlns);
        pXmlDom->RegisterNamespace(WMS_PREFIX, xmlns);
        }

    //Read nodes.
    pCapabilities->Read(status, WMS_ELEMENT_Service, *pXmlDom, *pRootNode);
    pCapabilities->Read(status, WMS_ELEMENT_Capability, *pXmlDom, *pRootNode);

    //if (WMSParserStatus::Success != status)
    //    return NULL;

    return pCapabilities;
    }

//=====================================================================================
//                                      WMSList
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSSingleLevelList::WMSSingleLevelList()
    {
    m_list = bvector<Utf8String>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSSingleLevelListPtr WMSSingleLevelList::Create()
    {
    return new WMSSingleLevelList();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSSingleLevelListPtr WMSSingleLevelList::Create(WMSParserStatus& status, BeXmlNodeR pParentNode, Utf8CP nodeName)
    {
    WMSSingleLevelListPtr pList = new WMSSingleLevelList();

    Utf8String content = "";
    pParentNode.GetContent(content);
    pList->Add(content.c_str());

    for (BeXmlNodeP siblingNode = pParentNode.GetNextSibling(); NULL != siblingNode; siblingNode = siblingNode->GetNextSibling())
        {
        if (0 == BeStringUtilities::Stricmp(siblingNode->GetName(), nodeName))
            {
            siblingNode->GetContent(content);
            pList->Add(content.c_str());
            }
        }

    return pList;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
void WMSSingleLevelList::Add(Utf8CP content)
    {
    m_list.push_back(content);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSMultiLevelList::WMSMultiLevelList()
    {
    m_list = bvector<Utf8String>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSMultiLevelListPtr WMSMultiLevelList::Create()
    {
    return new WMSMultiLevelList();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSMultiLevelListPtr WMSMultiLevelList::Create(WMSParserStatus& status, BeXmlNodeR pParentNode)
    {
    WMSMultiLevelListPtr pList = new WMSMultiLevelList();

    Utf8String content = "";
    for (BeXmlNodeP childElement = pParentNode.GetFirstChild(); NULL != childElement; childElement = childElement->GetNextSibling())
        {
        childElement->GetContent(content);
        pList->Add(content.c_str());
        }

    return pList;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
void WMSMultiLevelList::Add(Utf8CP content)
    {
    m_list.push_back(content);
    }

//=====================================================================================
//                              WMSService
//=====================================================================================
WMSService::WMSService()
    {
    m_name = "";
    m_title = "";
    m_abstract = "";
    m_fees = "";
    m_accessConstraints = "";

    m_pKeywordList = WMSMultiLevelList::Create();
    m_pOnlineResource = WMSOnlineResource::Create();
    m_pContactInformation = WMSContactInformation::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSService::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_KeywordList))
        m_pKeywordList = WMSMultiLevelList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_OnlineResource))
        m_pOnlineResource = WMSOnlineResource::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_ContactInformation))
        m_pContactInformation = WMSContactInformation::Create(status, xmlDom, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSServicePtr WMSService::Create()
    {
    return new WMSService();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSServicePtr WMSService::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSServicePtr pService = new WMSService();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);
    
    // Read string.
    xmlDom.SelectNodeContent(pService->GetNameR(), BuildNodePath(WMS_ELEMENT_Name).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetTitleR(), BuildNodePath(WMS_ELEMENT_Title).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetAbstractR(), BuildNodePath(WMS_ELEMENT_Abstract).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetFeesR(), BuildNodePath(WMS_ELEMENT_Fees).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pService->GetAccessConstraintsR(), BuildNodePath(WMS_ELEMENT_AccessConstraints).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    
    // Read positive integer.
    Utf8String posInt = "";
    xmlDom.SelectNodeContent(posInt, BuildNodePath(WMS_ELEMENT_LayerLimit).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pService->SetLayerLimit(posInt);
    xmlDom.SelectNodeContent(posInt, BuildNodePath(WMS_ELEMENT_MaxWidth).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pService->SetMaxWidth(posInt);
    xmlDom.SelectNodeContent(posInt, BuildNodePath(WMS_ELEMENT_MaxHeight).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSOnlineResource::WMSOnlineResource()
    {
    m_type = "";
    m_href = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSOnlineResourcePtr WMSOnlineResource::Create()
    {
    return new WMSOnlineResource();
    }

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
WMSContactInformation::WMSContactInformation()
    {
    m_position = "";
    m_voiceTelephone = "";
    m_facsimileTelephone = "";
    m_emailAddress = "";

    m_pPerson = WMSContactPerson::Create();
    m_pAddress = WMSContactAddress::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSContactInformation::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSContactInformationPtr WMSContactInformation::Create()
    {
    return new WMSContactInformation();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSContactInformationPtr WMSContactInformation::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSContactInformationPtr pContactInfo = new WMSContactInformation();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pContactInfo->GetPositionR(), BuildNodePath(WMS_ELEMENT_Position).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactInfo->GetVoiceTelephoneR(), BuildNodePath(WMS_ELEMENT_VoiceTel).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactInfo->GetFacsimileTelephoneR(), BuildNodePath(WMS_ELEMENT_FacsimileTel).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactInfo->GetEmailAddressR(), BuildNodePath(WMS_ELEMENT_EmailAddress).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    // Read complex type.
    pContactInfo->Read(status, WMS_ELEMENT_ContactPerson, xmlDom, parentNode);
    pContactInfo->Read(status, WMS_ELEMENT_ContactAddress, xmlDom, parentNode);

    return pContactInfo;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSContactPerson::WMSContactPerson()
    {
    m_name = "";
    m_organization = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSContactPersonPtr WMSContactPerson::Create()
    {
    return new WMSContactPerson();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSContactPersonPtr WMSContactPerson::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSContactPersonPtr pContactPerson = new WMSContactPerson();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pContactPerson->GetNameR(), BuildNodePath(WMS_ELEMENT_Person).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactPerson->GetOrganizationR(), BuildNodePath(WMS_ELEMENT_Organization).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    return pContactPerson;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSContactAddress::WMSContactAddress()
    {
    m_type = "";
    m_address = "";
    m_city = "";
    m_stateOrProvince = "";
    m_postCode = "";
    m_country = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSContactAddressPtr WMSContactAddress::Create()
    {
    return new WMSContactAddress();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSContactAddressPtr WMSContactAddress::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSContactAddressPtr pContactAddress = new WMSContactAddress();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pContactAddress->GetTypeR(), BuildNodePath(WMS_ELEMENT_Type).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetAddressR(), BuildNodePath(WMS_ELEMENT_Address).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetCityR(), BuildNodePath(WMS_ELEMENT_City).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetStateOrProvinceR(), BuildNodePath(WMS_ELEMENT_StateProv).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetPostCodeR(), BuildNodePath(WMS_ELEMENT_PostCode).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pContactAddress->GetCountryR(), BuildNodePath(WMS_ELEMENT_Country).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    return pContactAddress;
    }

//=====================================================================================
//                              WMSCapability
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSCapability::WMSCapability() 
    {
    m_pRequest = WMSRequest::Create();
    m_pExceptionList = WMSMultiLevelList::Create();
    m_pLayerList = bvector<WMSLayerPtr>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSCapability::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Request))
        m_pRequest = WMSRequest::Create(status, xmlDom, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Exception))
        m_pExceptionList = WMSMultiLevelList::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Layer))
        m_pLayerList.push_back(WMSLayer::Create(status, xmlDom, *pNode));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSCapabilityPtr WMSCapability::Create()
    {
    return new WMSCapability();
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSRequest::WMSRequest()
    {
    m_pGetCapabilities = WMSOperationType::Create();
    m_pGetMap = WMSOperationType::Create();
    m_pGetFeatureInfo = WMSOperationType::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSRequest::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GetCapabilities))
        m_pGetCapabilities = WMSOperationType::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GetMap))
        m_pGetMap = WMSOperationType::Create(status, *pNode);
    else if (0 == BeStringUtilities::Stricmp(nodeName,WMS_ELEMENT_GetFeatureInfo))
        m_pGetFeatureInfo = WMSOperationType::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSRequestPtr WMSRequest::Create()
    {
    return new WMSRequest();
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSOperationType::WMSOperationType()
    {
    m_pFormatList = WMSSingleLevelList::Create();
    m_pDcpType = WMSDCPType::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSOperationType::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_FormatList))
        m_pFormatList = WMSSingleLevelList::Create(status, *pNode, WMS_ELEMENT_FormatList);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_DCPType))
        m_pDcpType = WMSDCPType::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSOperationTypePtr WMSOperationType::Create()
    {
    return new WMSOperationType();
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSDCPType::WMSDCPType()
    {
    m_pHttpGet = WMSOnlineResource::Create();
    m_pHttpPost = WMSOnlineResource::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSDCPType::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    // No need for HTTP node, we only want to process his child's.
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(WMS_ELEMENT_Http).c_str())->SelectSingleNode(BuildNodePath(nodeName).c_str());
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_HttpGet))
        // No need for HTTPGet node, we only want to process his child's.
        m_pHttpGet = WMSOnlineResource::Create(status, *pNode->SelectSingleNode(BuildNodePath(WMS_ELEMENT_OnlineResource).c_str()));
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_HttpPost))
        // No need for HTTPPost node, we only want to process his child's.
        m_pHttpPost = WMSOnlineResource::Create(status, *pNode->SelectSingleNode(BuildNodePath(WMS_ELEMENT_OnlineResource).c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSDCPTypePtr WMSDCPType::Create()
    {
    return new WMSDCPType();
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSLayer::WMSLayer()
    {
    m_queryable = false;
    m_opaque = false;
    m_noSubsets = false;

    m_name = "";
    m_title = "";
    m_abstract = "";

    m_pKeywordList = WMSMultiLevelList::Create();
    m_pCRSList = WMSSingleLevelList::Create();
    m_pGeoBBox = WMSGeoBoundingBox::Create();
    m_pLatLonBBox = WMSLatLonBoundingBox::Create();
    m_pAttribution = WMSAttribution::Create();
    m_pAuthorityUrl = WMSUrl::Create();
    m_pIdentifier = WMSIdentifier::Create();
    m_pDataUrl = WMSUrl::Create();
    m_pFeatureListUrl = WMSUrl::Create();
    m_pStyle = WMSStyle::Create();

    m_pBBoxList = bvector<WMSBoundingBoxPtr>();
    m_pDimensionList = bvector<WMSDimensionPtr>();
    m_pMetadataUrlList = bvector<WMSUrlPtr>();
    m_pLayerList = bvector<WMSLayerPtr>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSLayer::CreateChilds(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    BeXmlDom::IterableNodeSet nodes;
    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);
    xmlDom.SelectNodes(nodes, BuildNodePath(WMS_ELEMENT_Layer).c_str(), pContext);

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
    BeXmlDom::IterableNodeSet nodes;
    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);
    xmlDom.SelectNodes(nodes, BuildNodePath(nodeName).c_str(), pContext);
    if (0 == nodes.size())
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    for (BeXmlDom::IterableNodeSetIter nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
        {
        if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_KeywordList))
            m_pKeywordList = WMSMultiLevelList::Create(status, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_CRS))
            {
            m_pCRSList = WMSSingleLevelList::Create(status, **nodeIt, WMS_ELEMENT_CRS);
            //&&JFC: Need to find a better way of skipping the nodes that were already processed when creating the list.
            for (size_t index = 1; index < m_pCRSList->Get().size(); ++index)
                ++nodeIt;
            }
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_SRS))
            {
            m_pCRSList = WMSSingleLevelList::Create(status, **nodeIt, WMS_ELEMENT_SRS);
            //&&JFC: Need to find a better way of skipping the nodes that were already processed when creating the list.
            for (size_t index = 1; index < m_pCRSList->Get().size(); ++index)
                ++nodeIt;
            }
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_GeoBoundingBox))
            m_pGeoBBox = WMSGeoBoundingBox::Create(status, xmlDom, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_LatLongBoundingBox))
            m_pLatLonBBox = WMSLatLonBoundingBox::Create(status, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_BoundingBox))
            m_pBBoxList.push_back(WMSBoundingBox::Create(status, **nodeIt));
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Dimension))
            m_pDimensionList.push_back(WMSDimension::Create(status, xmlDom, **nodeIt));
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Attribution))
            m_pAttribution = WMSAttribution::Create(status, xmlDom, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_AuthorityURL))
            m_pAuthorityUrl = WMSUrl::Create(status, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Identifier))
            m_pIdentifier = WMSIdentifier::Create(status, xmlDom, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_MetadataURL))
            m_pMetadataUrlList.push_back(WMSUrl::Create(status, **nodeIt));
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_DataURL))
            m_pDataUrl = WMSUrl::Create(status, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_FeatureListURL))
            m_pFeatureListUrl = WMSUrl::Create(status, **nodeIt);
        else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_Style))
            m_pStyle = WMSStyle::Create(status, xmlDom, **nodeIt);


        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSLayerPtr WMSLayer::Create()
    {
    return new WMSLayer();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSLayerPtr WMSLayer::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSLayerPtr pLayer = new WMSLayer();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read attributes.
    Utf8String stringValue = "";
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
    xmlDom.SelectNodeContent(pLayer->GetNameR(), BuildNodePath(WMS_ELEMENT_Name).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pLayer->GetTitleR(), BuildNodePath(WMS_ELEMENT_Title).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pLayer->GetAbstractR(), BuildNodePath(WMS_ELEMENT_Abstract).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(stringValue, BuildNodePath(WMS_ELEMENT_MinScaleDenom).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pLayer->SetMinScaleDenom(stringValue);
    xmlDom.SelectNodeContent(stringValue, BuildNodePath(WMS_ELEMENT_MaxScaleDenom).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pLayer->SetMaxScaleDenom(stringValue);

    // Read complex type.
    pLayer->Read(status, WMS_ELEMENT_KeywordList, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_BoundingBox, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_Dimension, xmlDom, parentNode);
    pLayer->Read(status, WMS_ELEMENT_Attribution, xmlDom, parentNode);
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSGeoBoundingBox::WMSGeoBoundingBox()
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSGeoBoundingBoxPtr WMSGeoBoundingBox::Create()
    {
    return new WMSGeoBoundingBox();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSGeoBoundingBoxPtr WMSGeoBoundingBox::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSGeoBoundingBoxPtr pGeoBBox = new WMSGeoBoundingBox();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    Utf8String stringValue = "";
    xmlDom.SelectNodeContent(stringValue, BuildNodePath(WMS_ELEMENT_WestBoundLong).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetWestBoundLong(stringValue);
    xmlDom.SelectNodeContent(stringValue, BuildNodePath(WMS_ELEMENT_EastBoundLong).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetEastBoundLong(stringValue);
    xmlDom.SelectNodeContent(stringValue, BuildNodePath(WMS_ELEMENT_SouthBoundLat).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetSouthBoundLat(stringValue);
    xmlDom.SelectNodeContent(stringValue, BuildNodePath(WMS_ELEMENT_NorthBoundLat).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    pGeoBBox->SetNorthBoundLat(stringValue);

    return pGeoBBox;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSLatLonBoundingBox::WMSLatLonBoundingBox()
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSLatLonBoundingBoxPtr WMSLatLonBoundingBox::Create()
    {
    return new WMSLatLonBoundingBox();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSLatLonBoundingBoxPtr WMSLatLonBoundingBox::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSLatLonBoundingBoxPtr pLatLonBBox = new WMSLatLonBoundingBox();

    // Read attributes.
    Utf8String stringValue = "";
    parentNode.GetAttributeStringValue(stringValue, "minx");
    pLatLonBBox->SetMinX(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "miny");
    pLatLonBBox->SetMinY(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "maxx");
    pLatLonBBox->SetMaxX(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "maxy");
    pLatLonBBox->SetMaxY(stringValue);

    return pLatLonBBox;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSBoundingBox::WMSBoundingBox()
    {

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSBoundingBoxPtr WMSBoundingBox::Create()
    {
    return new WMSBoundingBox();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSBoundingBoxPtr WMSBoundingBox::Create(WMSParserStatus& status, BeXmlNodeR parentNode)
    {
    WMSBoundingBoxPtr pBBox = new WMSBoundingBox();

    // Read attributes.
    Utf8String stringValue = "";
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSDimension::WMSDimension()
    {
    m_name = "";
    m_units = "";
    m_unitSymbol = "";
    m_default = "";
    m_dimension = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSDimensionPtr WMSDimension::Create()
    {
    return new WMSDimension();
    }

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
    Utf8String stringValue = "";
    parentNode.GetAttributeStringValue(stringValue, "multipleValues");
    pDimension->SetMultipleValues(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "nearestValue");
    pDimension->SetNearestValue(stringValue);
    parentNode.GetAttributeStringValue(stringValue, "current");
    pDimension->SetCurrent(stringValue);

    // Read string.
    xmlDom.SelectNodeContent(pDimension->GetDimensionR(), BuildNodePath(WMS_ELEMENT_Dimension).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    return pDimension;
    }

//=====================================================================================
//                              WMSAttribution
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSAttribution::WMSAttribution()
    {
    m_title = "";

    m_pOnlineRes = WMSOnlineResource::Create();
    m_pLogoUrl = WMSUrl::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSAttribution::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSAttributionPtr WMSAttribution::Create()
    {
    return new WMSAttribution();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSAttributionPtr WMSAttribution::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSAttributionPtr pAttribution = new WMSAttribution();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pAttribution->GetTitleR(), BuildNodePath(WMS_ELEMENT_Title).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    // Read complex type.
    pAttribution->Read(status, WMS_ELEMENT_OnlineResource, parentNode);
    pAttribution->Read(status, WMS_ELEMENT_LogoURL, parentNode);

    return pAttribution;
    }

//=====================================================================================
//                                  WMSUrl
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSUrl::WMSUrl()
    {
    m_type = "";
    m_name = "";

    m_pFormatList = WMSSingleLevelList::Create();
    m_pOnlineRes = WMSOnlineResource::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSUrl::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
    if (NULL == pNode)
        {
        status = WMSParserStatus::UnknownNode;
        return;
        }

    if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_FormatList))
        m_pFormatList = WMSSingleLevelList::Create(status, *pNode, WMS_ELEMENT_FormatList);
    else if (0 == BeStringUtilities::Stricmp(nodeName, WMS_ELEMENT_OnlineResource))
        m_pOnlineRes = WMSOnlineResource::Create(status, *pNode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSUrlPtr WMSUrl::Create()
    {
    return new WMSUrl();
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
    Utf8String stringValue = "";
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSIdentifier::WMSIdentifier()
    {
    m_authority = "";
    m_id = "";
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSIdentifierPtr WMSIdentifier::Create()
    {
    return new WMSIdentifier();
    }

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
    xmlDom.SelectNodeContent(pIdentifier->GetIDR(), BuildNodePath(WMS_ELEMENT_Identifier).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    return pIdentifier;
    }

//=====================================================================================
//                                  WMSStyle
//=====================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSStyle::WMSStyle()
    {
    m_name = "";
    m_title = "";
    m_abstract = "";

    m_pLegendUrl = WMSUrl::Create();
    m_pStyleSheetUrl = WMSUrl::Create();
    m_pStyleUrl = WMSUrl::Create();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
void WMSStyle::Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode)
    {
    BeXmlNodeP pNode = parentNode.SelectSingleNode(BuildNodePath(nodeName).c_str());
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
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
WMSStylePtr WMSStyle::Create()
    {
    return new WMSStyle();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
WMSStylePtr WMSStyle::Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode)
    {
    WMSStylePtr pStyle = new WMSStyle();

    xmlXPathContextPtr pContext = xmlDom.AcquireXPathContext(&parentNode);

    // Read string.
    xmlDom.SelectNodeContent(pStyle->GetNameR(), BuildNodePath(WMS_ELEMENT_Name).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pStyle->GetTitleR(), BuildNodePath(WMS_ELEMENT_Title).c_str(), pContext, BeXmlDom::NODE_BIAS_First);
    xmlDom.SelectNodeContent(pStyle->GetAbstractR(), BuildNodePath(WMS_ELEMENT_Abstract).c_str(), pContext, BeXmlDom::NODE_BIAS_First);

    // Read complex type.
    pStyle->Read(status, WMS_ELEMENT_LegendURL, parentNode);
    pStyle->Read(status, WMS_ELEMENT_StyleSheetURL, parentNode);
    pStyle->Read(status, WMS_ELEMENT_StyleURL, parentNode);

    return pStyle;
    }
