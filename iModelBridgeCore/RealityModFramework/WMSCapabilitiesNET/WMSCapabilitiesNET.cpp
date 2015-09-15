// This is the main DLL file.

//#include "stdafx.h"
#include "WMSCapabilitiesNET.h"
#include <msclr\marshal_cppstd.h>
#include <msclr\marshal.h>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace RealityPlatformWMSCapabilities;
using namespace BentleyApi;
using namespace BentleyApi::WMSParser;

// Class needed to read the url
using namespace System::Net;
using namespace System::IO;
using namespace System::Text;

//Used to marshal
using namespace msclr::interop;

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSCapabilitiesNet::WMSCapabilitiesNet(System::String^ source)
{
    WMSParser::WMSParserStatus status;
    WCharCP temp = 0;
    //Utf8String utf8Source = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source).ToPointer());
    //BeStringUtilities::WCharToUtf8(utf8Source, temp);
    //WMSParser::WMSCapabilitiesPtr pCapabilities = WMSCapabilities::CreateAndReadFromString(status, utf8Source.c_str());
    WMSParser::WMSCapabilitiesPtr pCapabilities = WMSCapabilities::CreateAndReadFromString(status, temp);
    if (pCapabilities.IsNull())
        throw gcnew NullReferenceException("Invalid WMS Capabilities content.");

    m_version = marshal_as<String^>(pCapabilities->GetVersion().c_str());

    pCapabilities->GetServiceGroup() != 0 ? m_pService = gcnew WMSServiceNet(pCapabilities->GetServiceGroup()) : m_pService = nullptr;
    pCapabilities->GetCapabilityGroup() != 0 ? m_pCapability = gcnew WMSCapabilityNet(pCapabilities->GetCapabilityGroup()) : m_pCapability = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSServiceNet::WMSServiceNet(WMSParser::WMSServiceCP service)
{
    m_name = marshal_as<String^>(service->GetName().c_str());
    m_title = marshal_as<String^>(service->GetTitle().c_str());
    m_abstract = marshal_as<String^>(service->GetAbstract().c_str());
    m_fees = marshal_as<String^>(service->GetFees().c_str());
    m_accessConstraints = marshal_as<String^>(service->GetAccessConstraints().c_str());

    m_layerLimit = service->GetLayerLimit();
    m_maxWidth = service->GetMaxWidth();
    m_maxHeight = service->GetMaxHeight();

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pKeywordList = gcnew List<String^>((UInt32)service->GetKeywordList().size());
        for (size_t i = 0; i < service->GetKeywordList().size(); i++)
        {
            System::String^ s = marshal_as<String^>(service->GetKeywordList()[i].c_str());
            m_pKeywordList->Add(s);
        }
    }
    catch (...)
    {
        m_pKeywordList = gcnew List<String^>(0);
    }

    service->GetOnlineResource() != 0 ? m_pOnlineResource = gcnew WMSOnlineResourceNet(service->GetOnlineResource()) : m_pOnlineResource = nullptr;

    service->GetContactInformation() != 0 ? m_pContactInformation = gcnew WMSContactInformationNet(service->GetContactInformation()) : m_pContactInformation = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSOnlineResourceNet::WMSOnlineResourceNet(WMSParser::WMSOnlineResourceCP onlineResource)
{
    m_type = marshal_as<String^>(onlineResource->GetType().c_str());
    m_href = marshal_as<String^>(onlineResource->GetHref().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSContactInformationNet::WMSContactInformationNet(WMSParser::WMSContactInformationCP contactInformation)
{
    m_position = marshal_as<String^>(contactInformation->GetPosition().c_str());
    m_voiceTelephone = marshal_as<String^>(contactInformation->GetVoiceTelephone().c_str());
    m_facsimileTelephone = marshal_as<String^>(contactInformation->GetFacsimileTelephone().c_str());
    m_emailAddress = marshal_as<String^>(contactInformation->GetEmailAddress().c_str());

    contactInformation->GetPerson() != 0 ? m_pPerson = gcnew WMSContactPersonNet(contactInformation->GetPerson()) : m_pPerson = nullptr;
    contactInformation->GetAddress() != 0 ? m_pAddress = gcnew WMSContactAddressNet(contactInformation->GetAddress()) : m_pAddress = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSContactPersonNet::WMSContactPersonNet(WMSParser::WMSContactPersonCP contactPerson)
{
    m_name = marshal_as<String^>(contactPerson->GetName().c_str());
    m_organization = marshal_as<String^>(contactPerson->GetOrganization().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSContactAddressNet::WMSContactAddressNet(WMSParser::WMSContactAddressCP contactAddress)
{
    m_type = marshal_as<String^>(contactAddress->GetType().c_str());
    m_address = marshal_as<String^>(contactAddress->GetAddress().c_str());
    m_city = marshal_as<String^>(contactAddress->GetCity().c_str());
    m_stateOrProvince = marshal_as<String^>(contactAddress->GetStateOrProvince().c_str());
    m_postCode = marshal_as<String^>(contactAddress->GetPostCode().c_str());
    m_country = marshal_as<String^>(contactAddress->GetCountry().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSCapabilityNet::WMSCapabilityNet(WMSParser::WMSCapabilityCP capability)
{
    capability->GetRequest() != 0 ? m_pRequest = gcnew WMSRequestNet(capability->GetRequest()) : m_pRequest = nullptr;

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pExceptionList = gcnew List<String^>((UInt32)capability->GetExceptionList().size());
        for (size_t i = 0; i < capability->GetExceptionList().size(); i++)
        {
            System::String^ s = marshal_as<String^>(capability->GetExceptionList()[i].c_str());
            m_pExceptionList->Add(s);
        }
    }
    catch (...)
    {
        m_pExceptionList = gcnew List<String^>(0);
    }

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pLayerList = gcnew List<WMSLayerNet^>((UInt32)capability->GetLayerList().size());
        for (size_t i = 0; i < capability->GetLayerList().size(); i++)
        {
            WMSLayerNet^ layer = gcnew WMSLayerNet(capability->GetLayerList()[i].get());
            m_pLayerList->Add(layer);
        }
    }
    catch (...)
    {
        m_pLayerList = gcnew List<WMSLayerNet^>(0);
    }

    //ToDo: finish to add content.
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSRequestNet::WMSRequestNet(WMSParser::WMSRequestCP request)
{
    request->GetCapabilities() != 0 ? m_pGetCapabilities = gcnew WMSOperationTypeNet(request->GetCapabilities()) : m_pGetCapabilities = nullptr;
    request->GetMap() != 0 ? m_pGetMap = gcnew WMSOperationTypeNet(request->GetMap()) : m_pGetMap = nullptr;
    request->GetFeatureInfo() != 0 ? m_pGetFeatureInfo = gcnew WMSOperationTypeNet(request->GetFeatureInfo()) : m_pGetFeatureInfo = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSOperationTypeNet::WMSOperationTypeNet(WMSParser::WMSOperationTypeCP operationType)
{
    // Used to pach a bug from the C++ parser.
    try
    {
        m_pFormatList = gcnew List<String^>((UInt32)operationType->GetFormatList().size());
        for (size_t i = 0; i < operationType->GetFormatList().size(); i++)
        {
            System::String^ s = marshal_as<String^>(operationType->GetFormatList()[i].c_str());
            m_pFormatList->Add(s);
        }
    }
    catch (...)
    {
        m_pFormatList = gcnew List<String^>(0);
    }

    operationType->GetDcpType() != 0 ? m_pDcpType = gcnew WMSDCPTypeNet(operationType->GetDcpType()) : m_pDcpType = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSDCPTypeNet::WMSDCPTypeNet(WMSParser::WMSDCPTypeCP dcpType)
{
    dcpType->GetHttpGet() != 0 ? m_pHttpGet = gcnew WMSOnlineResourceNet(dcpType->GetHttpGet()) : m_pHttpGet = nullptr;
    dcpType->GetHttpPost() != 0 ? m_pHttpPost = gcnew WMSOnlineResourceNet(dcpType->GetHttpPost()) : m_pHttpPost = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSLayerNet::WMSLayerNet(WMSParser::WMSLayerCP layer)
{
    // Attribute.
    m_queryable = layer->IsQueryable();
    m_opaque = layer->IsOpaque();
    m_noSubsets = layer->HasSubsets();
    m_cascaded = layer->GetCascaded();
    m_fixedWidth = layer->GetFixedWidth();
    m_fixedHeight = layer->GetFixedHeight();

    //! Element.
    m_name = marshal_as<String^>(layer->GetName().c_str());
    m_title = marshal_as<String^>(layer->GetTitle().c_str());
    m_abstract = marshal_as<String^>(layer->GetAbstract().c_str());
    m_minScaleDenom = layer->GetMinScaleDenom();
    m_maxScaleDenom = layer->GetMaxScaleDenom();

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pKeywordList = gcnew List<String^>((UInt32)layer->GetKeywordList().size());
        for (size_t i = 0; i < layer->GetKeywordList().size(); i++)
        {
            System::String^ s = marshal_as<String^>(layer->GetKeywordList()[i].c_str());
            m_pKeywordList->Add(s);
        }
    }
    catch (...)
    {
        m_pKeywordList = gcnew List<String^>(0);
    }

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pCRSList = gcnew List<String^>((UInt32)layer->GetCRSList().size());
        for (size_t i = 0; i < layer->GetCRSList().size(); i++)
        {
            System::String^ s = marshal_as<String^>(layer->GetCRSList()[i].c_str());
            m_pCRSList->Add(s);
        }
    }
    catch (...)
    {
        m_pCRSList = gcnew List<String^>(0);
    }

    layer->GetGeoBBox() != 0 ? m_pGeoBBox = gcnew WMSGeoBoundingBoxNet(layer->GetGeoBBox()) : m_pGeoBBox = nullptr;
    layer->GetLatLonBBox() != 0 ? m_pLatLonBBox = gcnew WMSLatLonBoundingBoxNet(layer->GetLatLonBBox()) : m_pLatLonBBox = nullptr;

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pBBoxList = gcnew List<WMSBoundingBoxNet^>((UInt32)layer->GetBBox().size());
        for (size_t i = 0; i < layer->GetBBox().size(); i++)
        {
            WMSBoundingBoxNet^ bBox = gcnew WMSBoundingBoxNet(layer->GetBBox()[i].get());
            m_pBBoxList->Add(bBox);
        }
    }
    catch (...)
    {
        m_pBBoxList = gcnew List<WMSBoundingBoxNet^>(0);
    }

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pDimensionList = gcnew List<WMSDimensionNet^>((UInt32)layer->GetDimensionList().size());
        for (size_t i = 0; i < layer->GetDimensionList().size(); i++)
        {
            WMSDimensionNet^ dimension = gcnew WMSDimensionNet(layer->GetDimensionList()[i].get());
            m_pDimensionList->Add(dimension);
        }
    }
    catch (...)
    {
        m_pDimensionList = gcnew List<WMSDimensionNet^>(0);
    }

    layer->GetAttribution() != 0 ? m_pAttribution = gcnew WMSAttributionNet(layer->GetAttribution()) : m_pAttribution = nullptr;

    layer->GetAuthorityUrl() != 0 ? m_pAuthorityUrl = gcnew WMSUrlNet(layer->GetAuthorityUrl()) : m_pAuthorityUrl = nullptr;

    layer->GetIdentifier() != 0 ? m_pIdentifier = gcnew WMSIdentifierNet(layer->GetIdentifier()) : m_pIdentifier = nullptr;

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pMetadataUrlList = gcnew List<WMSUrlNet^>((UInt32)layer->GetMetadataUrlList().size());
        for (size_t i = 0; i < layer->GetMetadataUrlList().size(); i++)
        {
            WMSUrlNet^ metadataUrl = gcnew WMSUrlNet(layer->GetMetadataUrlList()[i].get());
            m_pMetadataUrlList->Add(metadataUrl);
        }
    }
    catch (...)
    {
        m_pMetadataUrlList = gcnew List<WMSUrlNet^>(0);
    }

    layer->GetFeatureListUrl() != 0 ? m_pFeatureListUrl = gcnew WMSUrlNet(layer->GetFeatureListUrl()) : m_pFeatureListUrl = nullptr;

    layer->GetStyle() != 0 ? m_pStyle = gcnew WMSStyleNet(layer->GetStyle()) : m_pStyle = nullptr;

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pLayerList = gcnew List<WMSLayerNet^>((UInt32)layer->GetLayerList().size());
        for (size_t i = 0; i < layer->GetLayerList().size(); i++)
        {
            WMSLayerNet^ subLayer = gcnew WMSLayerNet(layer->GetLayerList()[i].get());
            m_pLayerList->Add(subLayer);
        }
    }
    catch (...)
    {
        m_pLayerList = gcnew List<WMSLayerNet^>(0);
    }

    //ToDo: Use GetDataListUrl istead of GetFeatureListUrl.
    // layer->GetDataListUrl() != 0 ? m_pDataUrl = gcnew WMSUrlNet(layer->GetDataListUrl()):m_pDataUrl = nullptr;
    layer->GetFeatureListUrl() != 0 ? m_pDataUrl = gcnew WMSUrlNet(layer->GetFeatureListUrl()) : m_pDataUrl = nullptr;
}


//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSGeoBoundingBoxNet::WMSGeoBoundingBoxNet(WMSParser::WMSGeoBoundingBoxCP geoBoundingBox)
{
    m_westBoundLong = geoBoundingBox->GetWestBoundLong();
    m_eastBoundLong = geoBoundingBox->GetEastBoundLong();
    m_southBoundLat = geoBoundingBox->GetSouthBoundLat();
    m_northBoundLat = geoBoundingBox->GetNorthBoundLat();
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSLatLonBoundingBoxNet::WMSLatLonBoundingBoxNet(WMSParser::WMSLatLonBoundingBoxCP latLonBoundingBox)
{
    m_minX = latLonBoundingBox->GetMinX();
    m_minX = latLonBoundingBox->GetMinY();
    m_maxX = latLonBoundingBox->GetMaxX();
    m_maxY = latLonBoundingBox->GetMaxY();
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSBoundingBoxNet::WMSBoundingBoxNet(WMSParser::WMSBoundingBoxCP pBoundingBox)
{
    m_crs = marshal_as<String^>(pBoundingBox->GetCRS().c_str());
    m_minX = pBoundingBox->GetMinX();
    m_minY = pBoundingBox->GetMinY();
    m_maxX = pBoundingBox->GetMaxX();
    m_maxY = pBoundingBox->GetMaxY();
    m_resX = pBoundingBox->GetResX();
    m_resY = pBoundingBox->GetResY();
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSDimensionNet::WMSDimensionNet(WMSParser::WMSDimensionCP pDimension)
{
    m_name = marshal_as<String^>(pDimension->GetName().c_str());
    m_units = marshal_as<String^>(pDimension->GetUnits().c_str());
    m_unitSymbol = marshal_as<String^>(pDimension->GetUnitSymbol().c_str());
    m_default = marshal_as<String^>(pDimension->GetDefault().c_str());
    m_dimension = marshal_as<String^>(pDimension->GetDimension().c_str());

    m_multipleValues = pDimension->GetMultipleValues();
    m_nearestValue = pDimension->GetNearestValue();
    m_current = pDimension->GetCurrent();
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSAttributionNet::WMSAttributionNet(WMSParser::WMSAttributionCP attribution)
{
    m_title = marshal_as<String^>(attribution->GetTitle().c_str());

    attribution->GetOnlineResource() != 0 ? m_pOnlineRes = gcnew WMSOnlineResourceNet(attribution->GetOnlineResource()) : m_pOnlineRes = nullptr;
    attribution->GetLogoUrl() != 0 ? m_pLogoUrl = gcnew WMSUrlNet(attribution->GetLogoUrl()) : m_pLogoUrl = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSUrlNet::WMSUrlNet(WMSParser::WMSUrlCP url)
{
    m_height = url->GetHeight();
    m_width = url->GetWidth();

    m_type = marshal_as<String^>(url->GetType().c_str());
    m_name = marshal_as<String^>(url->GetName().c_str());

    // Used to pach a bug from the C++ parser.
    try
    {
        m_pFormatList = gcnew List<String^>((UInt32)url->GetFormatList().size());
        for (size_t i = 0; i < url->GetFormatList().size(); i++)
        {
            System::String^ s = marshal_as<String^>(url->GetFormatList()[i].c_str());
            m_pFormatList->Add(s);
        }
    }
    catch (...)
    {
        m_pFormatList = gcnew List<String^>(0);
    }

    url->GetOnlineResource() != 0 ? m_pOnlineRes = gcnew WMSOnlineResourceNet(url->GetOnlineResource()) : m_pOnlineRes = nullptr;
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSIdentifierNet::WMSIdentifierNet(WMSParser::WMSIdentifierCP identifier)
{
    m_authority = marshal_as<String^>(identifier->GetAuthority().c_str());
    m_id = marshal_as<String^>(identifier->GetID().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                         Martin-Yanick.Guillemette         		 7/2015
//-------------------------------------------------------------------------------------
WMSStyleNet::WMSStyleNet(WMSParser::WMSStyleCP style)
{
    m_name = marshal_as<String^>(style->GetName().c_str());
    m_title = marshal_as<String^>(style->GetTitle().c_str());
    m_abstract = marshal_as<String^>(style->GetAbstract().c_str());

    style->GetLegendUrl() != 0 ? m_pLegendUrl = gcnew WMSUrlNet(style->GetLegendUrl()) : m_pLegendUrl = nullptr;
    style->GetStyleSheetUrl() != 0 ? m_pStyleSheetUrl = gcnew WMSUrlNet(style->GetStyleSheetUrl()) : m_pStyleSheetUrl = nullptr;
    style->GetStyleUrl() != 0 ? m_pStyleUrl = gcnew WMSUrlNet(style->GetStyleUrl()) : m_pStyleUrl = nullptr;
}