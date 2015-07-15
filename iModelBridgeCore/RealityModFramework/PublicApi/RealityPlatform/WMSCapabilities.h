/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/WMSCapabilities.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/WMSParser.h>
#include <Bentley/WString.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_WMSPARSER_NAMESPACE

//=======================================================================================
//! A WMS_Capabilities document is returned in response to a GetCapabilities request 
//! made on a WMS.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSCapabilities : RefCountedBase
    {
    public:
        //! Attempts to parse the UTF-8 XML fragment provided.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromString(WMSParserStatus& status, Utf8CP source, WStringP errorMsg = NULL);
        
        //! Attempts to parse the XML document provided.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromMemory(WMSParserStatus& status, void const* xmlBuffer, size_t xmlBufferSize, WStringP errorMsg = NULL);
        
        //! Attempts to open and parse the XML document at the given file path.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromFile(WMSParserStatus& status, WCharCP fileName, WStringP errorMsg = NULL);

        //! Element.
        WStringCR       GetVersion() const { return m_version; }
        static Utf8CP   GetNamespace() { return m_namespace; }     

        //! Complex type.
        WMSServiceCP    GetServiceGroup() const { return m_pService.get(); }
        WMSCapabilityCP GetCapabilityGroup() const { return m_pCapability.get(); }

    private:
        WMSCapabilities(WStringCR version);
        ~WMSCapabilities();

        //! Create from version.
        static WMSCapabilitiesPtr   Create(WMSParserStatus& status, WStringCR version);

        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        static void     SetNamespace(Utf8CP xmlns) { m_namespace = xmlns; }

        //! Element.
        WString             m_version;
        static Utf8CP       m_namespace;

        //! Complex type.
        WMSServicePtr       m_pService;
        WMSCapabilityPtr    m_pCapability;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSFormatList : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSFormatListPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        bvector<WString> Get() const { return m_formatList; }

    private:
        bvector<WString> m_formatList;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSElementList : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSElementListPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        bvector<WString> Get() const { return m_elementList; }

    private:
        bvector<WString> m_elementList;
    };

//=======================================================================================
//! General service metadata.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSService : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSServicePtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetName() const { return m_name; }
        WStringR    GetNameR() { return m_name; }

        WStringCR   GetTitle() const { return m_title; }
        WStringR    GetTitleR() { return m_title; }

        WStringCR   GetAbstract() const { return m_abstract; }
        WStringR    GetAbstractR() { return m_abstract; }

        WStringCR   GetFees() const { return m_fees; }
        WStringR    GetFeesR() { return m_fees; }

        WStringCR   GetAccessConstraints() const { return m_accessConstraints; }
        WStringR    GetAccessConstraintsR() { return m_accessConstraints; }

        size_t const&   GetLayerLimit() const { return m_layerLimit; }
        void            SetLayerLimit(WStringR layerLimit) { m_layerLimit = BeStringUtilities::Wtoi(layerLimit.c_str()); }

        size_t const&   GetMaxWidth() const { return m_maxWidth; }
        void            SetMaxWidth(WStringR width) { m_maxWidth = BeStringUtilities::Wtoi(width.c_str()); }

        size_t const&   GetMaxHeight() const { return m_maxHeight; }
        void            SetMaxHeight(WStringR height) { m_maxHeight = BeStringUtilities::Wtoi(height.c_str()); }

        //! Complex type.
        bvector<WString>        GetKeywordList() const { return m_pKeywordList->Get(); }
        WMSOnlineResourceCP     GetOnlineResource() const { return m_pOnlineResource.get(); }
        WMSContactInformationCP GetContactInformation() const { return m_pContactInformation.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WString m_name;
        WString m_title;
        WString m_abstract;
        WString m_fees;
        WString m_accessConstraints;
        size_t m_layerLimit;
        size_t m_maxWidth;
        size_t m_maxHeight;

        //! Complex type.
        WMSElementListPtr m_pKeywordList;
        WMSOnlineResourcePtr m_pOnlineResource;
        WMSContactInformationPtr m_pContactInformation;
    };

//=======================================================================================
//! An OnlineResource is typically an HTTP URL. The URL is placed in the xlink:href attribute, 
//! and the value "simple" is placed in the xlink:type attribute.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSOnlineResource : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSOnlineResourcePtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! The xlink:type attribute.
        WStringCR   GetType() const { return m_type; }
        WStringR    GetTypeR() { return m_type; }

        //! The xlink:href attribute.
        WStringCR   GetHref() const { return m_href; }
        WStringR    GetHrefR() { return m_href; }

    private:
        //! Element.
        WString m_type;
        WString m_href;
    };

//=======================================================================================
//! Information about a contact person for the service.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSContactInformation : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSContactInformationPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetPosition() const { return m_position; }
        WStringR    GetPositionR() { return m_position; }

        WStringCR   GetVoiceTelephone() const { return m_voiceTelephone; }
        WStringR    GetVoiceTelephoneR() { return m_voiceTelephone; }

        WStringCR   GetFacsimileTelephone() const { return m_facsimileTelephone; }
        WStringR    GetFacsimileTelephoneR() { return m_facsimileTelephone; }

        WStringCR   GetEmailAddress() const { return m_emailAddress; }
        WStringR    GetEmailAddressR() { return m_emailAddress; }

        //! Complex type.
        WMSContactPersonCP  GetPerson() const { return m_pPerson.get(); }
        WMSContactAddressCP GetAddress() const { return m_pAddress.get(); }

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WString m_position;
        WString m_voiceTelephone;
        WString m_facsimileTelephone;
        WString m_emailAddress;

        //! Complex type.
        WMSContactPersonPtr m_pPerson;
        WMSContactAddressPtr m_pAddress;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSContactPerson : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSContactPersonPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR GetName() const { return m_name; }
        WStringR GetNameR() { return m_name; }

        WStringCR GetOrganization() const { return m_organization; }
        WStringR GetOrganizationR() { return m_organization; }

    private:
        //! Element.
        WString m_name;
        WString m_organization;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSContactAddress : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSContactAddressPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR GetType() const { return m_type; }
        WStringR GetTypeR() { return m_type; }

        WStringCR GetAddress() const { return m_address; }
        WStringR GetAddressR() { return m_address; }

        WStringCR GetCity() const { return m_city; }
        WStringR GetCityR() { return m_city; }

        WStringCR GetStateOrProvince() const { return m_stateOrProvince; }
        WStringR GetStateOrProvinceR() { return m_stateOrProvince; }

        WStringCR GetPostCode() const { return m_postCode; }
        WStringR GetPostCodeR() { return m_postCode; }

        WStringCR GetCountry() const { return m_country; }
        WStringR GetCountryR() { return m_country; }

    private:
        //! Element.
        WString m_type;
        WString m_address;
        WString m_city;
        WString m_stateOrProvince;
        WString m_postCode;
        WString m_country;
    };

//=======================================================================================
//! A Capability lists available request types, how exceptions may be reported, and whether 
//! any extended capabilities are defined. It also includes an optional list of map layers 
//! available from this server.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSCapability : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSCapabilityPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSRequestCP GetRequest() const { return m_pRequest.get(); }
        bvector<WString> GetExceptionList() const { return m_pExceptionList->Get(); }
        bvector<WMSLayerPtr> GetLayerList() const { return m_pLayerList; }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSRequestPtr m_pRequest;
        WMSElementListPtr m_pExceptionList;
        bvector<WMSLayerPtr> m_pLayerList;
    };

//=======================================================================================
//! Available WMS Operations are listed in a Request element.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSRequest : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSRequestPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOperationTypeCP GetCapabilities() const { return m_pGetCapabilities.get(); }
        WMSOperationTypeCP GetMap() const { return m_pGetMap.get(); }
        WMSOperationTypeCP GetFeatureInfo() const { return m_pGetFeatureInfo.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOperationTypePtr m_pGetCapabilities;
        WMSOperationTypePtr m_pGetMap;
        WMSOperationTypePtr m_pGetFeatureInfo;
    };

//=======================================================================================
//! For each operation offered by the server, list the available output formats and 
//! the online resource.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSOperationType : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSOperationTypePtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Complex type.
        bvector<WString> GetFormatList() const { return m_pFormatList->Get(); }
        WMSDCPTypeCP GetDcpType() const { return m_pDcpType.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Complex type.
        WMSFormatListPtr m_pFormatList;
        WMSDCPTypePtr m_pDcpType;
    };

//=======================================================================================
//! Available Distributed Computing Platforms (DCPs) are listed here. At present, 
//! only HTTP is defined.
//!
//! Available HTTP request methods (at least "Get" shall be supported):
//!     Get: The URL prefix for the HTTP "Get" request method.
//!     Post: The URL prefix for the HTTP "Post" request method.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSDCPType : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSDCPTypePtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOnlineResourceCP GetHttpGet() const { return m_pHttpGet.get(); }
        WMSOnlineResourceCP GetHttpPost() const { return m_pHttpPost.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOnlineResourcePtr m_pHttpGet;
        WMSOnlineResourcePtr m_pHttpPost;
    };

//=======================================================================================
//! Nested list of zero or more map Layers offered by this server.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSLayer : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSLayerPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        // Attribute.
        bool const& IsQueryable() const { return m_queryable; }
        void        SetQueryable(WStringR queryable) { m_queryable = (BeStringUtilities::Wtoi(queryable.c_str()) != 0); }

        bool const&  IsOpaque() const { return m_opaque; }
        void         SetOpaque(WStringR opaque) { m_opaque = (BeStringUtilities::Wtoi(opaque.c_str()) != 0); }

        bool const& HasSubsets() const { return m_noSubsets; }
        void        SetSubsets(WStringR noSubsets) { m_noSubsets = (BeStringUtilities::Wtoi(noSubsets.c_str()) != 0); }

        size_t const&   GetCascaded() const { return m_cascaded; }
        void            SetCascaded(WStringR cascaded) { m_cascaded = BeStringUtilities::Wtoi(cascaded.c_str()); }

        size_t const&   GetFixedWidth() const { return m_fixedWidth; }
        void            SetFixedWidth(WStringR width) { m_fixedWidth = BeStringUtilities::Wtoi(width.c_str()); }

        size_t const&   GetFixedHeight() const { return m_fixedHeight; }
        void            SetFixedHeight(WStringR height) { m_fixedHeight = BeStringUtilities::Wtoi(height.c_str()); }

        //! Element.
        WStringCR   GetName() const { return m_name; }
        WStringR    GetNameR() { return m_name; }

        WStringCR   GetTitle() const { return m_title; }
        WStringR    GetTitleR() { return m_title; }

        WStringCR   GetAbstract() const { return m_abstract; }
        WStringR    GetAbstractR() { return m_abstract; }

        double const&   GetMinScaleDenom() const { return m_minScaleDenom; }
        void            SetMinScaleDenom(WStringR denom) { m_minScaleDenom = BeStringUtilities::Wcstod(denom.c_str(), NULL); }

        double const&   GetMaxScaleDenom() const { return m_maxScaleDenom; }
        void            SetMaxScaleDenom(WStringR denom) { m_maxScaleDenom = BeStringUtilities::Wcstod(denom.c_str(), NULL); }

        //! Complex type.
        bvector<WString>            GetKeywordList() const { return m_pKeywordList->Get(); }
        bvector<WString>            GetCRSList() const { return m_pCRSList->Get(); }
        WMSGeoBoundingBoxCP         GetGeoBBox() const { return m_pGeoBBox.get(); }
        WMSLatLonBoundingBoxCP      GetLatLonBBox() const { return m_pLatLonBBox.get(); }
        bvector<WMSBoundingBoxPtr>  GetBBox() const { return m_pBBoxList; }
        bvector<WMSDimensionPtr>    GetDimensionList() const { return m_pDimensionList; }
        WMSAttributionCP            GetAttribution() const { return m_pAttribution.get(); }
        WMSUrlCP                    GetAuthorityUrl() const { return m_pAuthorityUrl.get(); }
        WMSUrlCP                    GetFeatureListUrl() const { return m_pFeatureListUrl.get(); }
        bvector<WMSUrlPtr>          GetMetadataUrlList() const { return m_pMetadataUrlList; }
        WMSIdentifierCP             GetIdentifier() const { return m_pIdentifier.get(); }
        WMSStyleCP                  GetStyle() const { return m_pStyle.get(); }
        bvector<WMSLayerPtr>        GetLayerList() const { return m_pLayerList; }

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        void CreateChilds(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        // Attribute.
        bool m_queryable;
        bool m_opaque;
        bool m_noSubsets;
        size_t m_cascaded;
        size_t m_fixedWidth;
        size_t m_fixedHeight;

        //! Element.
        WString m_name;
        WString m_title;
        WString m_abstract;
        double m_minScaleDenom;
        double m_maxScaleDenom;

        //! Complex type.
        WMSElementListPtr m_pKeywordList;
        WMSFormatListPtr m_pCRSList;
        WMSGeoBoundingBoxPtr m_pGeoBBox;
        WMSLatLonBoundingBoxPtr m_pLatLonBBox;
        bvector<WMSBoundingBoxPtr> m_pBBoxList;
        bvector<WMSDimensionPtr> m_pDimensionList;
        WMSAttributionPtr m_pAttribution;
        WMSUrlPtr m_pAuthorityUrl;
        WMSIdentifierPtr m_pIdentifier;
        bvector<WMSUrlPtr> m_pMetadataUrlList;
        WMSUrlPtr m_pDataUrl;
        WMSUrlPtr m_pFeatureListUrl;
        WMSStylePtr m_pStyle;
        bvector<WMSLayerPtr> m_pLayerList;
    };

//=======================================================================================
//! The EX_GeographicBoundingBox attributes indicate the limits of the enclosing 
//! rectangle in longitude and latitude decimal degrees.
//!
//! Specific to version 1.3.0
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSGeoBoundingBox : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSGeoBoundingBoxPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        double const&   GetWestBoundLong() const { return m_westBoundLong; }
        void            SetWestBoundLong(WStringR longitude) { m_westBoundLong = BeStringUtilities::Wcstod(longitude.c_str(), NULL); }

        double const&   GetEastBoundLong() const { return m_eastBoundLong; }
        void            SetEastBoundLong(WStringR longitude) { m_eastBoundLong = BeStringUtilities::Wcstod(longitude.c_str(), NULL); }

        double const&   GetSouthBoundLat() const { return m_southBoundLat; }
        void            SetSouthBoundLat(WStringR latitude) { m_southBoundLat = BeStringUtilities::Wcstod(latitude.c_str(), NULL); }

        double const&   GetNorthBoundLat() const { return m_northBoundLat; }
        void            SetNorthBoundLat(WStringR latitude) { m_northBoundLat = BeStringUtilities::Wcstod(latitude.c_str(), NULL); }

    private:
        //! Element.
        double m_westBoundLong;
        double m_eastBoundLong;
        double m_southBoundLat;
        double m_northBoundLat;
    };

//=======================================================================================
//! The LatLonBoundingBox attributes indicate the edges of the enclosing rectangle in
//! latitude/longitude decimal degrees(as in SRS EPSG:4326[WGS1984 lat/lon]).
//! 
//! Specific to version 1.0.0, 1.1.0 and 1.1.1
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSLatLonBoundingBox : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSLatLonBoundingBoxPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Element.
        double const&   GetMinX() const { return m_minX; }
        void            SetMinX(WStringR minX) { m_minX = BeStringUtilities::Wcstod(minX.c_str(), NULL); }

        double const&   GetMinY() const { return m_minY; }
        void            SetMinY(WStringR minY) { m_minY = BeStringUtilities::Wcstod(minY.c_str(), NULL); }

        double const&   GetMaxX() const { return m_maxX; }
        void            SetMaxX(WStringR maxX) { m_maxX = BeStringUtilities::Wcstod(maxX.c_str(), NULL); }

        double const&   GetMaxY() const { return m_maxY; }
        void            SetMaxY(WStringR maxY) { m_maxY = BeStringUtilities::Wcstod(maxY.c_str(), NULL); }

    private:
        //! Element.
        double m_minX;
        double m_minY;
        double m_maxX;
        double m_maxY;
    };

//=======================================================================================
//! The BoundingBox attributes indicate the limits of the bounding box in units of the 
//! specified coordinate reference system.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSBoundingBox : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSBoundingBoxPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetCRS() const { return m_crs; }
        WStringR    GetCRSR() { return m_crs; }

        double const&   GetMinX() const { return m_minX; }
        void            SetMinX(WStringR minX) { m_minX = BeStringUtilities::Wcstod(minX.c_str(), NULL); }

        double const&   GetMinY() const { return m_minY; }
        void            SetMinY(WStringR minY) { m_minY = BeStringUtilities::Wcstod(minY.c_str(), NULL); }

        double const&   GetMaxX() const { return m_maxX; }
        void            SetMaxX(WStringR maxX) { m_maxX = BeStringUtilities::Wcstod(maxX.c_str(), NULL); }

        double const&   GetMaxY() const { return m_maxY; }
        void            SetMaxY(WStringR maxY) { m_maxY = BeStringUtilities::Wcstod(maxY.c_str(), NULL); }

        double const&   GetResX() const { return m_resX; }
        void            SetResX(WStringR resX) { m_resX = BeStringUtilities::Wcstod(resX.c_str(), NULL); }

        double const&   GetResY() const { return m_resY; }
        void            SetResY(WStringR resY) { m_resY = BeStringUtilities::Wcstod(resY.c_str(), NULL); }

    private:
        //! Element.
        WString m_crs; //! Identifier for a single Coordinate Reference System (CRS).
        double m_minX;
        double m_minY;
        double m_maxX;
        double m_maxY;
        double m_resX;
        double m_resY;
    };

//=======================================================================================
//! The Dimension element declares the existence of a dimension and indicates what 
//! values along a dimension are valid.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSDimension : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSDimensionPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetName() const { return m_name; }
        WStringR    GetNameR() { return m_name; }

        WStringCR   GetUnits() const { return m_units; }
        WStringR    GetUnitsR() { return m_units; }

        WStringCR   GetUnitSymbol() const { return m_unitSymbol; }
        WStringR    GetUnitSymbolR() { return m_unitSymbol; }

        WStringCR   GetDefault() const { return m_default; }
        WStringR    GetDefaultR() { return m_default; }

        WStringCR   GetDimension() const { return m_dimension; }
        WStringR    GetDimensionR() { return m_dimension; }

        bool const& GetMultipleValues() const { return m_multipleValues; }
        void        SetMultipleValues(WStringR multipleValues) { m_multipleValues = (BeStringUtilities::Wtoi(multipleValues.c_str()) != 0); }

        bool const& GetNearestValue() const { return m_nearestValue; }
        void        SetNearestValue(WStringR nearestValue) { m_nearestValue = (BeStringUtilities::Wtoi(nearestValue.c_str()) != 0); }

        bool const& GetCurrent() const { return m_current; }
        void        SetCurrent(WStringR current) { m_current = (BeStringUtilities::Wtoi(current.c_str()) != 0); }

    private:
        //! Element.
        WString m_name;
        WString m_units;
        WString m_unitSymbol;
        WString m_default;
        WString m_dimension;
        bool m_multipleValues;
        bool m_nearestValue;
        bool m_current;
    };

//=======================================================================================
//! Attribution indicates the provider of a Layer or collection of Layers. The provider's 
//! URL, descriptive title string, and/or logo image URL may be supplied. Client applications 
//! may choose to display one or more of these items. A format element indicates the MIME 
//! type of the logo image located at LogoURL. The logo image's width and height assist 
//! client applications in laying out space to display the logo.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSAttribution : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSAttributionPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetTitle() const { return m_title; }
        WStringR    GetTitleR() { return m_title; }

        //! Complex type.
        WMSOnlineResourceCP GetOnlineResource() const { return m_pOnlineRes.get(); }
        WMSUrlCP            GetLogoUrl() const { return m_pLogoUrl.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Element.
        WString m_title;

        //! Complex type.
        WMSOnlineResourcePtr m_pOnlineRes;
        WMSUrlPtr m_pLogoUrl;
    };

//=======================================================================================
//! MetadataURL: A Map Server may use zero or more MetadataURL elements to offer detailed, 
//! standardized metadata about the data underneath a particular layer. The type attribute 
//! indicates the standard to which the metadata complies. The format element indicates 
//! how the metadata is structured.
//!
//! AuthorityURL: A Map Server may use zero or more Identifier elements to list ID numbers 
//! or labels defined by a particular Authority. For example, the Global Change Master 
//! Directory (gcmd.gsfc.nasa.gov) defines a DIF_ID label for every dataset. The authority 
//! name and explanatory URL are defined in a separate AuthorityURL element, which may be 
//! defined once and inherited by subsidiary layers. Identifiers themselves are not inherited.
//!
//! DataURL: A Map Server may use DataURL offer a link to the underlying data represented 
//! by a particular layer.
//!
//! FeatureListURL: A Map Server may use FeatureListURL to point to a list of the features 
//! represented in a Layer.
//!
//! LegendURL: A Map Server may use zero or more LegendURL elements to provide an image(s) 
//! of a legend relevant to each Style of a Layer. The Format element indicates the MIME type 
//! of the legend. Width and height attributes may be provided to assist client applications 
//! in laying out space to display the legend.
//!
//! StyleSheeetURL: StyleSheeetURL provides symbology information for each Style of a Layer.
//!
//! StyleURL: A Map Server may use StyleURL to offer more information about the data or 
//! symbology underlying a particular Style. While the semantics are not well-defined, as 
//! long as the results of an HTTP GET request against the StyleURL are properly MIME-typed, 
//! Viewer Clients and Cascading Map Servers can make use of this. A possible use could be 
//! to allow a Map Server to provide legend information.
//!
//! LogoURL:
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSUrl : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSUrlPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Element.
        size_t const&   GetHeight() const { return m_height; }
        void            SetHeight(WStringR height) { m_height = BeStringUtilities::Wtoi(height.c_str()); }

        size_t const&   GetWidth() const { return m_width; }
        void            SetWidth(WStringR width) { m_width = BeStringUtilities::Wtoi(width.c_str()); }

        WStringCR   GetType() const { return m_type; }
        WStringR    GetTypeR() { return m_type; }

        WStringCR   GetName() const { return m_name; }
        WStringR    GetNameR() { return m_name; }

        //! Complex type.
        bvector<WString>    GetFormatList() const { return m_pFormatList->Get(); }
        WMSOnlineResourceCP GetOnlineResource() const { return m_pOnlineRes.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Element.
        size_t m_height;
        size_t m_width;
        WString m_type;
        WString m_name;

        //! Complex type.
        WMSFormatListPtr m_pFormatList;
        WMSOnlineResourcePtr m_pOnlineRes;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSIdentifier : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSIdentifierPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetAuthority() const { return m_authority; }
        WStringR    GetAuthorityR() { return m_authority; }

        WStringCR   GetID() const { return m_id; }
        WStringR    GetIDR() { return m_id; }

    private:
        //! Element.
        WString m_authority;
        WString m_id;
    };

//=======================================================================================
//! A Style element lists the name by which a style is requested and a human-readable title 
//! for pick lists, optionally (and ideally) provides a human-readable description, and 
//! optionally gives a style URL.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
struct WMSStyle : RefCountedBase
    {
    public:
        //! Create from xml node.
        static WMSStylePtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        WStringCR   GetName() const { return m_name; }
        WStringR    GetNameR() { return m_name; }

        WStringCR   GetTitle() const { return m_title; }
        WStringR    GetTitleR() { return m_title; }

        WStringCR   GetAbstract() const { return m_abstract; }
        WStringR    GetAbstractR() { return m_abstract; }

        //! Complex type.
        WMSUrlCP    GetLegendUrl() const { return m_pLegendUrl.get(); }
        WMSUrlCP    GetStyleSheetUrl() const { return m_pStyleSheetUrl.get(); }
        WMSUrlCP    GetStyleUrl() const { return m_pStyleUrl.get(); }

    private:
        //! Read complex node.
        void    Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Element.
        WString m_name;
        WString m_title;
        WString m_abstract;

        //! Complex type.
        WMSUrlPtr m_pLegendUrl;
        WMSUrlPtr m_pStyleSheetUrl;
        WMSUrlPtr m_pStyleUrl;
    };

END_BENTLEY_WMSPARSER_NAMESPACE
