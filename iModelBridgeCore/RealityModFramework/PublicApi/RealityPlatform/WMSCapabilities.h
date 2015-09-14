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

// Required so that the class can be used from c++/cli.
// See compiler warning C4692.
#if defined (_MANAGED)
#define MPUBLIC public
#else
#define MPUBLIC
#endif

//=======================================================================================
//! A WMS_Capabilities document is returned in response to a GetCapabilities request 
//! made on a WMS.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSCapabilities : RefCountedBase
    {
    public:
        //! Attempts to parse the WideChar XML fragment provided.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromString(WMSParserStatus& status, WCharCP source, WStringP errorMsg = NULL);

        //! Attempts to parse the UTF-8 XML fragment provided.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromString(WMSParserStatus& status, Utf8CP source, WStringP errorMsg = NULL);
        
        //! Attempts to parse the XML document provided.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromMemory(WMSParserStatus& status, void const* xmlBuffer, size_t xmlBufferSize, WStringP errorMsg = NULL);
        
        //! Attempts to open and parse the XML document at the given file path.
        WMSPARSER_EXPORT static WMSCapabilitiesPtr CreateAndReadFromFile(WMSParserStatus& status, Utf8CP fileName, WStringP errorMsg = NULL);

        //! Element.
        Utf8StringCR           GetVersion() const { return m_version; }
        static Utf8StringCR    GetNamespace() { return m_namespace; }

        //! Complex type.
        WMSServiceCP       GetServiceGroup() const { return m_pService.get(); }
        WMSCapabilityCP    GetCapabilityGroup() const { return m_pCapability.get(); }

    protected:
        //! Constructor.
        WMSCapabilities(Utf8StringCR version);

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Set namespace if there is one.
        static void SetNamespace(Utf8CP xmlns) { m_namespace = xmlns; }

        //! Element.
        Utf8String          m_version;
        static Utf8String   m_namespace;

        //! Complex type.
        WMSServicePtr       m_pService;
        WMSCapabilityPtr    m_pCapability;
    };

//=====================================================================================
//! Example:
//! <Format> </Format>
//! <Format> </Format>
//! <Format> </Format>
//!
//! @bsiclass                                   Jean-Francois.Cote               8/2015
//=====================================================================================
MPUBLIC struct WMSSingleLevelList : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSSingleLevelListPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSSingleLevelListPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode, Utf8CP nodeName);

        const bvector<Utf8String>& Get() const { return m_list; };

    protected:
        //! Constructor.
        WMSSingleLevelList();

    private:
        //! Add element to the list.
        void Add(Utf8CP content);

        bvector<Utf8String> m_list;
    };

//=======================================================================================
//! Example:
//! <KeywordList>
//!     <Keyword> </Keyword>
//!     <Keyword> </Keyword>
//! </KeywordList>
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSMultiLevelList : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSMultiLevelListPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSMultiLevelListPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        const bvector<Utf8String>& Get() const { return m_list; };

    protected:
        //! Constructor.
        WMSMultiLevelList();

    private:
        //! Add element to the list.
        void Add(Utf8CP content);

        bvector<Utf8String> m_list;
    };

//=======================================================================================
//! General service metadata.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSService : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSServicePtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSServicePtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR   GetName() const { return m_name; }
        Utf8StringR    GetNameR() { return m_name; }

        Utf8StringCR   GetTitle() const { return m_title; }
        Utf8StringR    GetTitleR() { return m_title; }

        Utf8StringCR   GetAbstract() const { return m_abstract; }
        Utf8StringR    GetAbstractR() { return m_abstract; }

        Utf8StringCR   GetFees() const { return m_fees; }
        Utf8StringR    GetFeesR() { return m_fees; }

        Utf8StringCR   GetAccessConstraints() const { return m_accessConstraints; }
        Utf8StringR    GetAccessConstraintsR() { return m_accessConstraints; }

        size_t const&  GetLayerLimit() const { return m_layerLimit; }
        void           SetLayerLimit(Utf8StringR layerLimit) { m_layerLimit = atoi(layerLimit.c_str()); }

        size_t const&  GetMaxWidth() const { return m_maxWidth; }
        void           SetMaxWidth(Utf8StringR width) { m_maxWidth = atoi(width.c_str()); }

        size_t const&  GetMaxHeight() const { return m_maxHeight; }
        void           SetMaxHeight(Utf8StringR height) { m_maxHeight = atoi(height.c_str()); }

        //! Complex type.
        const bvector<Utf8String>& GetKeywordList() const { return m_pKeywordList->Get(); }
        WMSOnlineResourceCP        GetOnlineResource() const { return m_pOnlineResource.get(); }
        WMSContactInformationCP    GetContactInformation() const { return m_pContactInformation.get(); }

    protected:
        //! Constructor.
        WMSService();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8String  m_name;
        Utf8String  m_title;
        Utf8String  m_abstract;
        Utf8String  m_fees;
        Utf8String  m_accessConstraints;
        size_t  m_layerLimit;
        size_t  m_maxWidth;
        size_t  m_maxHeight;

        //! Complex type.
        WMSMultiLevelListPtr        m_pKeywordList;
        WMSOnlineResourcePtr        m_pOnlineResource;
        WMSContactInformationPtr    m_pContactInformation;
    };

//=======================================================================================
//! An OnlineResource is typically an HTTP URL. The URL is placed in the xlink:href attribute, 
//! and the value "simple" is placed in the xlink:type attribute.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSOnlineResource : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSOnlineResourcePtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSOnlineResourcePtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! The xlink:type attribute.
        Utf8StringCR  GetType() const { return m_type; }
        Utf8StringR   GetTypeR() { return m_type; }

        //! The xlink:href attribute.
        Utf8StringCR  GetHref() const { return m_href; }
        Utf8StringR   GetHrefR() { return m_href; }

    protected:
        //! Constructor.
        WMSOnlineResource();

    private:
        //! Element.
        Utf8String m_type;
        Utf8String m_href;
    };

//=======================================================================================
//! Information about a contact person for the service.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSContactInformation : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSContactInformationPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSContactInformationPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetPosition() const { return m_position; }
        Utf8StringR   GetPositionR() { return m_position; }

        Utf8StringCR  GetVoiceTelephone() const { return m_voiceTelephone; }
        Utf8StringR   GetVoiceTelephoneR() { return m_voiceTelephone; }

        Utf8StringCR  GetFacsimileTelephone() const { return m_facsimileTelephone; }
        Utf8StringR   GetFacsimileTelephoneR() { return m_facsimileTelephone; }

        Utf8StringCR  GetEmailAddress() const { return m_emailAddress; }
        Utf8StringR   GetEmailAddressR() { return m_emailAddress; }

        //! Complex type.
        WMSContactPersonCP     GetPerson() const { return m_pPerson.get(); }
        WMSContactAddressCP    GetAddress() const { return m_pAddress.get(); }

    protected:
        //! Constructor.
        WMSContactInformation();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8String m_position;
        Utf8String m_voiceTelephone;
        Utf8String m_facsimileTelephone;
        Utf8String m_emailAddress;

        //! Complex type.
        WMSContactPersonPtr m_pPerson;
        WMSContactAddressPtr m_pAddress;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSContactPerson : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSContactPersonPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSContactPersonPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetName() const { return m_name; }
        Utf8StringR   GetNameR() { return m_name; }

        Utf8StringCR  GetOrganization() const { return m_organization; }
        Utf8StringR   GetOrganizationR() { return m_organization; }

    protected:
        //! Constructor.
        WMSContactPerson();

    private:
        //! Element.
        Utf8String m_name;
        Utf8String m_organization;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSContactAddress : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSContactAddressPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSContactAddressPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetType() const { return m_type; }
        Utf8StringR   GetTypeR() { return m_type; }

        Utf8StringCR  GetAddress() const { return m_address; }
        Utf8StringR   GetAddressR() { return m_address; }

        Utf8StringCR  GetCity() const { return m_city; }
        Utf8StringR   GetCityR() { return m_city; }

        Utf8StringCR  GetStateOrProvince() const { return m_stateOrProvince; }
        Utf8StringR   GetStateOrProvinceR() { return m_stateOrProvince; }

        Utf8StringCR  GetPostCode() const { return m_postCode; }
        Utf8StringR   GetPostCodeR() { return m_postCode; }

        Utf8StringCR  GetCountry() const { return m_country; }
        Utf8StringR   GetCountryR() { return m_country; }

    protected:
        //! Constructor.
        WMSContactAddress();

    private:
        //! Element.
        Utf8String m_type;
        Utf8String m_address;
        Utf8String m_city;
        Utf8String m_stateOrProvince;
        Utf8String m_postCode;
        Utf8String m_country;
    };

//=======================================================================================
//! A Capability lists available request types, how exceptions may be reported, and whether 
//! any extended capabilities are defined. It also includes an optional list of map layers 
//! available from this server.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSCapability : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSCapabilityPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSCapabilityPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSRequestCP               GetRequest() const { return m_pRequest.get(); }
        const bvector<Utf8String>& GetExceptionList() const { return m_pExceptionList->Get(); }
        bvector<WMSLayerPtr>       GetLayerList() const { return m_pLayerList; }

    protected:
        //! Constructor.
        WMSCapability();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSRequestPtr m_pRequest;
        WMSMultiLevelListPtr m_pExceptionList;
        bvector<WMSLayerPtr> m_pLayerList;
    };

//=======================================================================================
//! Available WMS Operations are listed in a Request element.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSRequest : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSRequestPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSRequestPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOperationTypeCP GetCapabilities() const { return m_pGetCapabilities.get(); }
        WMSOperationTypeCP GetMap() const { return m_pGetMap.get(); }
        WMSOperationTypeCP GetFeatureInfo() const { return m_pGetFeatureInfo.get(); }

    protected:
        //! Constructor.
        WMSRequest();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

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
MPUBLIC struct WMSOperationType : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSOperationTypePtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSOperationTypePtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Complex type.
        bvector<Utf8String>    GetFormatList() const { return m_pFormatList->Get(); }
        WMSDCPTypeCP           GetDcpType() const { return m_pDcpType.get(); }

    protected:
        //! Constructor.
        WMSOperationType();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Complex type.
        WMSSingleLevelListPtr m_pFormatList;
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
MPUBLIC struct WMSDCPType : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSDCPTypePtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSDCPTypePtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOnlineResourceCP    GetHttpGet() const { return m_pHttpGet.get(); }
        WMSOnlineResourceCP    GetHttpPost() const { return m_pHttpPost.get(); }

    protected:
        //! Constructor.
        WMSDCPType();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Complex type.
        WMSOnlineResourcePtr m_pHttpGet;
        WMSOnlineResourcePtr m_pHttpPost;
    };

//=======================================================================================
//! Nested list of zero or more map Layers offered by this server.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSLayer : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSLayerPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSLayerPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Attribute.
        bool const&    IsQueryable() const { return m_queryable; }
        void           SetQueryable(Utf8StringR queryable) { m_queryable = (atoi(queryable.c_str()) != 0); }

        bool const&    IsOpaque() const { return m_opaque; }
        void           SetOpaque(Utf8StringR opaque) { m_opaque = (atoi(opaque.c_str()) != 0); }

        bool const&    HasSubsets() const { return m_noSubsets; }
        void           SetSubsets(Utf8StringR noSubsets) { m_noSubsets = (atoi(noSubsets.c_str()) != 0); }

        size_t const&  GetCascaded() const { return m_cascaded; }
        void           SetCascaded(Utf8StringR cascaded) { m_cascaded = atoi(cascaded.c_str()); }

        size_t const&  GetFixedWidth() const { return m_fixedWidth; }
        void           SetFixedWidth(Utf8StringR width) { m_fixedWidth = atoi(width.c_str()); }

        size_t const&  GetFixedHeight() const { return m_fixedHeight; }
        void           SetFixedHeight(Utf8StringR height) { m_fixedHeight = atoi(height.c_str()); }

        //! Element.
        Utf8StringCR  GetName() const { return m_name; }
        Utf8StringR   GetNameR() { return m_name; }

        Utf8StringCR  GetTitle() const { return m_title; }
        Utf8StringR   GetTitleR() { return m_title; }

        Utf8StringCR  GetAbstract() const { return m_abstract; }
        Utf8StringR   GetAbstractR() { return m_abstract; }

        double const&  GetMinScaleDenom() const { return m_minScaleDenom; }
        void           SetMinScaleDenom(Utf8StringR denom) { m_minScaleDenom = strtod(denom.c_str(), NULL); }

        double const&  GetMaxScaleDenom() const { return m_maxScaleDenom; }
        void           SetMaxScaleDenom(Utf8StringR denom) { m_maxScaleDenom = strtod(denom.c_str(), NULL); }

        //! Complex type.
        bvector<Utf8String>         GetKeywordList() const { return m_pKeywordList->Get(); }
        bvector<Utf8String>         GetCRSList() const { return m_pCRSList->Get(); }
        WMSGeoBoundingBoxCP         GetGeoBBox() const { return m_pGeoBBox.get(); }
        WMSLatLonBoundingBoxCP      GetLatLonBBox() const { return m_pLatLonBBox.get(); }
        bvector<WMSBoundingBoxPtr>  GetBBox() const { return m_pBBoxList; }
        bvector<WMSDimensionPtr>    GetDimensionList() const { return m_pDimensionList; }
        WMSAttributionCP            GetAttribution() const { return m_pAttribution.get(); }
        WMSUrlCP                    GetAuthorityUrl() const { return m_pAuthorityUrl.get(); }
        WMSUrlCP                    GetDataUrl() const { return m_pDataUrl.get(); }
        WMSUrlCP                    GetFeatureListUrl() const { return m_pFeatureListUrl.get(); }
        bvector<WMSUrlPtr>          GetMetadataUrlList() const { return m_pMetadataUrlList; }
        WMSIdentifierCP             GetIdentifier() const { return m_pIdentifier.get(); }
        WMSStyleCP                  GetStyle() const { return m_pStyle.get(); }
        bvector<WMSLayerPtr>        GetLayerList() const { return m_pLayerList; }

    protected:
        //! Constructor.
        WMSLayer();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        void CreateChilds(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Attribute.
        bool m_queryable;
        bool m_opaque;
        bool m_noSubsets;
        size_t m_cascaded;
        size_t m_fixedWidth;
        size_t m_fixedHeight;

        //! Element.
        Utf8String m_name;
        Utf8String m_title;
        Utf8String m_abstract;
        double m_minScaleDenom;
        double m_maxScaleDenom;

        //! Complex type.
        WMSMultiLevelListPtr m_pKeywordList;
        WMSSingleLevelListPtr m_pCRSList;
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
MPUBLIC struct WMSGeoBoundingBox : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSGeoBoundingBoxPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSGeoBoundingBoxPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        double const&  GetWestBoundLong() const { return m_westBoundLong; }
        void           SetWestBoundLong(Utf8StringR longitude) { m_westBoundLong = strtod(longitude.c_str(), NULL); }

        double const&  GetEastBoundLong() const { return m_eastBoundLong; }
        void           SetEastBoundLong(Utf8StringR longitude) { m_eastBoundLong = strtod(longitude.c_str(), NULL); }

        double const&  GetSouthBoundLat() const { return m_southBoundLat; }
        void           SetSouthBoundLat(Utf8StringR latitude) { m_southBoundLat = strtod(latitude.c_str(), NULL); }

        double const&  GetNorthBoundLat() const { return m_northBoundLat; }
        void           SetNorthBoundLat(Utf8StringR latitude) { m_northBoundLat = strtod(latitude.c_str(), NULL); }

    protected:
        //! Constructor.
        WMSGeoBoundingBox();

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
MPUBLIC struct WMSLatLonBoundingBox : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSLatLonBoundingBoxPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSLatLonBoundingBoxPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Element.
        double const&  GetMinX() const { return m_minX; }
        void           SetMinX(Utf8StringR minX) { m_minX = strtod(minX.c_str(), NULL); }

        double const&  GetMinY() const { return m_minY; }
        void           SetMinY(Utf8StringR minY) { m_minY = strtod(minY.c_str(), NULL); }

        double const&  GetMaxX() const { return m_maxX; }
        void           SetMaxX(Utf8StringR maxX) { m_maxX = strtod(maxX.c_str(), NULL); }

        double const&  GetMaxY() const { return m_maxY; }
        void           SetMaxY(Utf8StringR maxY) { m_maxY = strtod(maxY.c_str(), NULL); }

    protected:
        //! Constructor.
        WMSLatLonBoundingBox();

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
MPUBLIC struct WMSBoundingBox : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSBoundingBoxPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSBoundingBoxPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR   GetCRS() const { return m_crs; }
        Utf8StringR    GetCRSR() { return m_crs; }

        double const&  GetMinX() const { return m_minX; }
        void           SetMinX(Utf8StringR minX) { m_minX = strtod(minX.c_str(), NULL); }

        double const&  GetMinY() const { return m_minY; }
        void           SetMinY(Utf8StringR minY) { m_minY = strtod(minY.c_str(), NULL); }

        double const&  GetMaxX() const { return m_maxX; }
        void           SetMaxX(Utf8StringR maxX) { m_maxX = strtod(maxX.c_str(), NULL); }

        double const&  GetMaxY() const { return m_maxY; }
        void           SetMaxY(Utf8StringR maxY) { m_maxY = strtod(maxY.c_str(), NULL); }

        double const&  GetResX() const { return m_resX; }
        void           SetResX(Utf8StringR resX) { m_resX = strtod(resX.c_str(), NULL); }

        double const&  GetResY() const { return m_resY; }
        void           SetResY(Utf8StringR resY) { m_resY = strtod(resY.c_str(), NULL); }

    protected:
        //! Constructor.
        WMSBoundingBox();

    private:
        //! Element.
        Utf8String m_crs; //! Identifier for a single Coordinate Reference System (CRS).
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
MPUBLIC struct WMSDimension : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSDimensionPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSDimensionPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetName() const { return m_name; }
        Utf8StringR   GetNameR() { return m_name; }

        Utf8StringCR  GetUnits() const { return m_units; }
        Utf8StringR   GetUnitsR() { return m_units; }

        Utf8StringCR  GetUnitSymbol() const { return m_unitSymbol; }
        Utf8StringR   GetUnitSymbolR() { return m_unitSymbol; }

        Utf8StringCR  GetDefault() const { return m_default; }
        Utf8StringR   GetDefaultR() { return m_default; }

        Utf8StringCR  GetDimension() const { return m_dimension; }
        Utf8StringR   GetDimensionR() { return m_dimension; }

        bool const&    GetMultipleValues() const { return m_multipleValues; }
        void           SetMultipleValues(Utf8StringR multipleValues) { m_multipleValues = (atoi(multipleValues.c_str()) != 0); }

        bool const&    GetNearestValue() const { return m_nearestValue; }
        void           SetNearestValue(Utf8StringR nearestValue) { m_nearestValue = (atoi(nearestValue.c_str()) != 0); }

        bool const&    GetCurrent() const { return m_current; }
        void           SetCurrent(Utf8StringR current) { m_current = (atoi(current.c_str()) != 0); }

    protected:
        //! Constructor.
        WMSDimension();

    private:
        //! Element.
        Utf8String m_name;
        Utf8String m_units;
        Utf8String m_unitSymbol;
        Utf8String m_default;
        Utf8String m_dimension;
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
MPUBLIC struct WMSAttribution : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSAttributionPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSAttributionPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetTitle() const { return m_title; }
        Utf8StringR   GetTitleR() { return m_title; }

        //! Complex type.
        WMSOnlineResourceCP    GetOnlineResource() const { return m_pOnlineRes.get(); }
        WMSUrlCP               GetLogoUrl() const { return m_pLogoUrl.get(); }

    protected:
        //! Constructor.
        WMSAttribution();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Element.
        Utf8String m_title;

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
MPUBLIC struct WMSUrl : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSUrlPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSUrlPtr Create(WMSParserStatus& status, BeXmlNodeR parentNode);

        //! Element.
        size_t const&  GetHeight() const { return m_height; }
        void           SetHeight(Utf8StringR height) { m_height = atoi(height.c_str()); }

        size_t const&  GetWidth() const { return m_width; }
        void           SetWidth(Utf8StringR width) { m_width = atoi(width.c_str()); }

        Utf8StringCR  GetType() const { return m_type; }
        Utf8StringR   GetTypeR() { return m_type; }

        Utf8StringCR  GetName() const { return m_name; }
        Utf8StringR   GetNameR() { return m_name; }

        //! Complex type.
        bvector<Utf8String> GetFormatList() const { return m_pFormatList->Get(); }
        WMSOnlineResourceCP GetOnlineResource() const { return m_pOnlineRes.get(); }

    protected:
        //! Constructor.
        WMSUrl();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Element.
        size_t m_height;
        size_t m_width;
        Utf8String m_type;
        Utf8String m_name;

        //! Complex type.
        WMSSingleLevelListPtr m_pFormatList;
        WMSOnlineResourcePtr m_pOnlineRes;
    };

//=======================================================================================
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSIdentifier : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSIdentifierPtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSIdentifierPtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetAuthority() const { return m_authority; }
        Utf8StringR   GetAuthorityR() { return m_authority; }

        Utf8StringCR  GetID() const { return m_id; }
        Utf8StringR   GetIDR() { return m_id; }

    protected:        
        //! Constructor.
        WMSIdentifier();

    private:
        //! Element.
        Utf8String m_authority;
        Utf8String m_id;
    };

//=======================================================================================
//! A Style element lists the name by which a style is requested and a human-readable title 
//! for pick lists, optionally (and ideally) provides a human-readable description, and 
//! optionally gives a style URL.
//!
//! @bsiclass                                     Jean-Francois.Cote              03/2015
//=======================================================================================
MPUBLIC struct WMSStyle : RefCountedBase
    {
    public:
        //! Create empty in case the information is missing from the WmsCapabilities.
        WMSPARSER_EXPORT static WMSStylePtr Create();

        //! Create from xml node.
        WMSPARSER_EXPORT static WMSStylePtr Create(WMSParserStatus& status, BeXmlDomR xmlDom, BeXmlNodeR parentNode);

        //! Element.
        Utf8StringCR  GetName() const { return m_name; }
        Utf8StringR   GetNameR() { return m_name; }

        Utf8StringCR  GetTitle() const { return m_title; }
        Utf8StringR   GetTitleR() { return m_title; }

        Utf8StringCR  GetAbstract() const { return m_abstract; }
        Utf8StringR   GetAbstractR() { return m_abstract; }

        //! Complex type.
        WMSUrlCP   GetLegendUrl() const { return m_pLegendUrl.get(); }
        WMSUrlCP   GetStyleSheetUrl() const { return m_pStyleSheetUrl.get(); }
        WMSUrlCP   GetStyleUrl() const { return m_pStyleUrl.get(); }

    protected:
        //! Constructor.
        WMSStyle();

    private:
        //! Read complex node.
        void Read(WMSParserStatus& status, Utf8CP nodeName, BeXmlNodeR parentNode);

        //! Element.
        Utf8String m_name;
        Utf8String m_title;
        Utf8String m_abstract;

        //! Complex type.
        WMSUrlPtr m_pLegendUrl;
        WMSUrlPtr m_pStyleSheetUrl;
        WMSUrlPtr m_pStyleUrl;
    };

END_BENTLEY_WMSPARSER_NAMESPACE
