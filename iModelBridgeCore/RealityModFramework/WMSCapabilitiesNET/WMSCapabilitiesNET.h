/*--------------------------------------------------------------------------------------+
|
|     $Source: WMSCapabilitiesNET/WMSCapabilitiesNET.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <RealityPlatform/WMSCapabilities.h>

using namespace System::Collections::Generic;

using namespace BentleyApi;

namespace RealityPlatformWMSCapabilities
{
     //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSOnlineResourceNet
    {
    public:
        WMSOnlineResourceNet(WMSParser::WMSOnlineResourceCP onlineResource);

        //! The xlink:type attribute.
        System::String^   GetTypeNet() { return m_type; }

        //! The xlink:href attribute.
        System::String^   GetHref() { return m_href; }

    private:
        //! Element.
        System::String^ m_type;
        System::String^ m_href;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSContactPersonNet
    {
    public:
        WMSContactPersonNet(WMSParser::WMSContactPersonCP contactPerson);

        //! Element.
        System::String^ GetName() { return m_name; }
        System::String^ GetOrganization() { return m_organization; }

    private:
        //! Element.
        System::String^ m_name;
        System::String^ m_organization;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSContactAddressNet
    {
    public:
        WMSContactAddressNet(WMSParser::WMSContactAddressCP contactAddress);

        //! Element.
        System::String^ GetTypeNet() { return m_type; }
        System::String^ GetAddress() { return m_address; }
        System::String^ GetCity() { return m_city; }
        System::String^ GetStateOrProvince() { return m_stateOrProvince; }
        System::String^ GetPostCode() { return m_postCode; }
        System::String^ GetCountry() { return m_country; }

    private:
        //! Element.
        System::String^ m_type;
        System::String^ m_address;
        System::String^ m_city;
        System::String^ m_stateOrProvince;
        System::String^ m_postCode;
        System::String^ m_country;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSContactInformationNet
    {
    public:
        WMSContactInformationNet(WMSParser::WMSContactInformationCP contactInformation);

        //! Element.
        System::String^    GetPosition() { return m_position; }
        System::String^    GetVoiceTelephone() { return m_voiceTelephone; }
        System::String^    GetFacsimileTelephone() { return m_facsimileTelephone; }
        System::String^    GetEmailAddress() { return m_emailAddress; }

        //! Complex type.
        WMSContactPersonNet^  GetPerson() { return m_pPerson; }
        WMSContactAddressNet^ GetAddress() { return m_pAddress; }

    private:
        //! Element.
        System::String^ m_position;
        System::String^ m_voiceTelephone;
        System::String^ m_facsimileTelephone;
        System::String^ m_emailAddress;

        //! Complex type.
        WMSContactPersonNet^ m_pPerson;
        WMSContactAddressNet^ m_pAddress;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSServiceNet
    {
    public:
        WMSServiceNet(WMSParser::WMSServiceCP service);

        //! Element.
        System::String^   GetName()  { return m_name; }
        System::String^   GetTitle()  { return m_title; }
        System::String^   GetAbstract()  { return m_abstract; }
        System::String^   GetFees()  { return m_fees; }
        System::String^   GetAccessConstraints()  { return m_accessConstraints; }

        size_t   GetLayerLimit() { return m_layerLimit; }
        size_t   GetMaxWidth() { return m_maxWidth; }
        size_t   GetMaxHeight() { return m_maxHeight; }

        //! Complex type.
        List<System::String^>^ GetKeywordList() { return m_pKeywordList; }
        WMSOnlineResourceNet^ GetOnlineResource() { return m_pOnlineResource; }
        WMSContactInformationNet^ GetContactInformation() { return m_pContactInformation; }

    private:
        //! Element.
        System::String^ m_name;
        System::String^ m_title;
        System::String^ m_abstract;
        System::String^ m_fees;
        System::String^ m_accessConstraints;
        size_t m_layerLimit;
        size_t m_maxWidth;
        size_t m_maxHeight;

        //! Complex type.
        List<System::String^>^ m_pKeywordList;
        WMSOnlineResourceNet^ m_pOnlineResource;
        WMSContactInformationNet^ m_pContactInformation;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSDCPTypeNet
    {
    public:
        WMSDCPTypeNet(WMSParser::WMSDCPTypeCP dcpType);

        //! Complex type.
        WMSOnlineResourceNet^ GetHttpGet() { return m_pHttpGet; }
        WMSOnlineResourceNet^ GetHttpPost() { return m_pHttpPost; }

    private:
        //! Complex type.
        WMSOnlineResourceNet^ m_pHttpGet;
        WMSOnlineResourceNet^ m_pHttpPost;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSOperationTypeNet
    {
    public:
        WMSOperationTypeNet(WMSParser::WMSOperationTypeCP operationType);

        //! Complex type.
        List<System::String^>^ GetFormatList() { return m_pFormatList; }
        WMSDCPTypeNet^ GetDcpType() { return m_pDcpType; }

    private:
        //! Complex type.
        List<System::String^>^ m_pFormatList;
        WMSDCPTypeNet^ m_pDcpType;

    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSRequestNet
    {
    public:
        WMSRequestNet(WMSParser::WMSRequestCP request);

        //! Complex type.
        WMSOperationTypeNet^ GetCapabilities() { return m_pGetCapabilities; }
        WMSOperationTypeNet^ GetMap() { return m_pGetMap; }
        WMSOperationTypeNet^ GetFeatureInfo() { return m_pGetFeatureInfo; }

    private:
        //! Complex type.
        WMSOperationTypeNet^ m_pGetCapabilities;
        WMSOperationTypeNet^ m_pGetMap;
        WMSOperationTypeNet^ m_pGetFeatureInfo;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSGeoBoundingBoxNet
    {
    public:
        WMSGeoBoundingBoxNet(WMSParser::WMSGeoBoundingBoxCP geoBoundingBox);

        //! Element.
        double GetWestBoundLong() { return m_westBoundLong; }
        double GetEastBoundLong() { return m_eastBoundLong; }
        double GetSouthBoundLat() { return m_southBoundLat; }
        double GetNorthBoundLat() { return m_northBoundLat; }

    private:
        //! Element.
        double m_westBoundLong;
        double m_eastBoundLong;
        double m_southBoundLat;
        double m_northBoundLat;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSLatLonBoundingBoxNet
    {
    public:
        WMSLatLonBoundingBoxNet(WMSParser::WMSLatLonBoundingBoxCP latLonBoundingBox);

        //! Element.
        double GetMinX() { return m_minX; }
        double GetMinY() { return m_minY; }
        double GetMaxX() { return m_maxX; }
        double GetMaxY() { return m_maxY; }

    private:
        //! Element.
        double m_minX;
        double m_minY;
        double m_maxX;
        double m_maxY;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSBoundingBoxNet
    {
    public:
        WMSBoundingBoxNet(WMSParser::WMSBoundingBoxCP pBoundingBox);

        //! Element.
        System::String^ GetCRS() { return m_crs; }
        double GetMinX() { return m_minX; }
        double GetMinY() { return m_minY; }
        double GetMaxX() { return m_maxX; }
        double GetMaxY() { return m_maxY; }
        double GetResX() { return m_resX; }
        double GetResY() { return m_resY; }

    private:
        //! Element.
        System::String^ m_crs; //! Identifier for a single Coordinate Reference System (CRS).
        double m_minX;
        double m_minY;
        double m_maxX;
        double m_maxY;
        double m_resX;
        double m_resY;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSDimensionNet
    {
    public:
        WMSDimensionNet(WMSParser::WMSDimensionCP pDimension);

        //! Element.
        System::String^   GetName() { return m_name; }
        System::String^   GetUnits() { return m_units; }
        System::String^   GetUnitSymbol() { return m_unitSymbol; }
        System::String^   GetDefault() { return m_default; }
        System::String^   GetDimension() { return m_dimension; }

        bool GetMultipleValues() { return m_multipleValues; }
        bool GetNearestValue() { return m_nearestValue; }
        bool GetCurrent() { return m_current; }

    private:
        //! Element.
        System::String^ m_name;
        System::String^ m_units;
        System::String^ m_unitSymbol;
        System::String^ m_default;
        System::String^ m_dimension;
        bool m_multipleValues;
        bool m_nearestValue;
        bool m_current;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSUrlNet
    {
    public:
        WMSUrlNet(WMSParser::WMSUrlCP url);

        //! Element.
        size_t GetHeight() { return m_height; }
        size_t GetWidth() { return m_width; }
        System::String^   GetTypeNet() { return m_type; }
        System::String^   GetName() { return m_name; }

        //! Complex type.
        List<System::String^>^ GetFormatList() { return m_pFormatList; }
        WMSOnlineResourceNet^  GetOnlineResource() { return m_pOnlineRes; }

    private:
        //! Element.
        size_t m_height;
        size_t m_width;
        System::String^ m_type;
        System::String^ m_name;

        //! Complex type.
        List<System::String^>^ m_pFormatList;
        WMSOnlineResourceNet^  m_pOnlineRes;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSAttributionNet
    {
    public:
        WMSAttributionNet(WMSParser::WMSAttributionCP attribution);

        //! Element.
        System::String^   GetTitle() { return m_title; }

        //! Complex type.
        WMSOnlineResourceNet^ GetOnlineResource() { return m_pOnlineRes; }
        WMSUrlNet^            GetLogoUrl() { return m_pLogoUrl; }

    private:
        //! Element.
        System::String^ m_title;

        //! Complex type.
        WMSOnlineResourceNet^ m_pOnlineRes;
        WMSUrlNet^ m_pLogoUrl;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSIdentifierNet
    {
    public:
        WMSIdentifierNet(WMSParser::WMSIdentifierCP identifier);

        //! Element.
        System::String^   GetAuthority() { return m_authority; }
        System::String^   GetID() { return m_id; }

    private:
        //! Element.
        System::String^ m_authority;
        System::String^ m_id;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSStyleNet
    {
    public:
        WMSStyleNet(WMSParser::WMSStyleCP style);

        //! Element.
        System::String^   GetName() { return m_name; }
        System::String^   GetTitle() { return m_title; }
        System::String^   GetAbstract() { return m_abstract; }

        //! Complex type.
        WMSUrlNet^    GetLegendUrl() { return m_pLegendUrl; }
        WMSUrlNet^    GetStyleSheetUrl() { return m_pStyleSheetUrl; }
        WMSUrlNet^    GetStyleUrl() { return m_pStyleUrl; }

    private:
        //! Element.
        System::String^ m_name;
        System::String^ m_title;
        System::String^ m_abstract;

        //! Complex type.
        WMSUrlNet^ m_pLegendUrl;
        WMSUrlNet^ m_pStyleSheetUrl;
        WMSUrlNet^ m_pStyleUrl;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSLayerNet
    {
    public:
        WMSLayerNet(WMSParser::WMSLayerCP layer);

        // Attribute.
        bool IsQueryable() { return m_queryable; }
        bool IsOpaque() { return m_opaque; }
        bool HasSubsets() { return m_noSubsets; }
        size_t GetCascaded() { return m_cascaded; }
        size_t GetFixedWidth() { return m_fixedWidth; }
        size_t GetFixedHeight() { return m_fixedHeight; }

        //! Element.
        System::String^   GetName() { return m_name; }
        System::String^   GetTitle() { return m_title; }
        System::String^   GetAbstract() { return m_abstract; }

        double GetMinScaleDenom() { return m_minScaleDenom; }
        double GetMaxScaleDenom() { return m_maxScaleDenom; }

        //! Complex type.
        List<System::String^>^      GetKeywordList() { return m_pKeywordList; }
        List<System::String^>^      GetCRSList() { return m_pCRSList; }

        WMSGeoBoundingBoxNet^       GetGeoBBox() { return m_pGeoBBox; }
        WMSLatLonBoundingBoxNet^    GetLatLonBBox() { return m_pLatLonBBox; }
        List<WMSBoundingBoxNet^>^   GetBBox() { return m_pBBoxList; }
        List<WMSDimensionNet^>^     GetDimensionList() { return m_pDimensionList; }
        WMSAttributionNet^          GetAttribution() { return m_pAttribution; }
        WMSUrlNet^                  GetAuthorityUrl() { return m_pAuthorityUrl; }
        WMSIdentifierNet^           GetIdentifier() { return m_pIdentifier; }
        List<WMSUrlNet^>^           GetMetadataUrlList() { return m_pMetadataUrlList; }
        WMSUrlNet^                  GetFeatureListUrl() { return m_pFeatureListUrl; }
        WMSStyleNet^                GetStyle() { return m_pStyle; }
        List<WMSLayerNet^>^         GetLayerList() { return m_pLayerList; }
        WMSUrlNet^                  GetDataListUrl() { return m_pDataUrl; }

    private:
        // Attribute.
        bool m_queryable;
        bool m_opaque;
        bool m_noSubsets;
        size_t m_cascaded;
        size_t m_fixedWidth;
        size_t m_fixedHeight;

        //! Element.
        System::String^ m_name;
        System::String^ m_title;
        System::String^ m_abstract;
        double m_minScaleDenom;
        double m_maxScaleDenom;

        //! Complex type.
        List<System::String^>^ m_pKeywordList;
        List<System::String^>^ m_pCRSList;
        WMSGeoBoundingBoxNet^ m_pGeoBBox;
        WMSLatLonBoundingBoxNet^ m_pLatLonBBox;
        List<WMSBoundingBoxNet^>^ m_pBBoxList;
        List<WMSDimensionNet^>^ m_pDimensionList;
        WMSAttributionNet^ m_pAttribution;
        WMSUrlNet^ m_pAuthorityUrl;
        WMSIdentifierNet^ m_pIdentifier;
        List<WMSUrlNet^>^ m_pMetadataUrlList;
        WMSUrlNet^ m_pFeatureListUrl;
        WMSStyleNet^ m_pStyle;
        List<WMSLayerNet^>^ m_pLayerList;
        WMSUrlNet^ m_pDataUrl;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSCapabilityNet
    {
    public:
        WMSCapabilityNet(WMSParser::WMSCapabilityCP capability);

        //! Complex type.
        WMSRequestNet^ GetRequest() { return m_pRequest; }
        List<System::String^>^ GetExceptionList() { return m_pExceptionList; }
        List<WMSLayerNet^>^ GetLayerList() { return m_pLayerList; }

    private:
        //! Complex type.
        WMSRequestNet^ m_pRequest;
        List<System::String^>^ m_pExceptionList;
        List<WMSLayerNet^>^ m_pLayerList;
    };

    //=====================================================================================
    //! @bsiclass                                   Martin-Yanick.Guillemette        7/2015
    //=====================================================================================
    public ref class WMSCapabilitiesNet
    {
    public:
        //WMSCapabilitiesNet(WMSParser::WMSParserStatus status, System::String^ source);
        WMSCapabilitiesNet(System::String^ source);

        //! Element.
        System::String^ GetVersion(){ return m_version; }

        //! Complex type.
        WMSServiceNet^ GetServiceGroup() { return m_pService; }
        WMSCapabilityNet^ GetCapabilityGroup() { return m_pCapability; }
    private:
        System::String^ m_version;
        WMSServiceNet^ m_pService;
        WMSCapabilityNet^ m_pCapability;
    };
}
