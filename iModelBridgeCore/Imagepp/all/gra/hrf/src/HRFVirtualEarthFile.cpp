//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFVirtualEarthFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFVirtualEarthFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HRFVirtualEarthFile.h>
#include "HRFVirtualEarthEditor.h"
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HFCCallbacks.h>
#include <Imagepp/all/h/HFCCallbackRegistry.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <BeXml/BeXml.h>

#include <ImagePPInternal/gra/Task.h>
#include <ImagePPInternal/HttpConnection.h>

// Do not change this number without validating resolution descriptor creation.
#define NB_BLOCK_READER_THREAD 10

// Debug only
//#define BINGMAPS_AUTO_PASSWORD


// These are the ImagerySet tag that we use when requesting tiles URI. 
// See Bing map "Get Imagery Metadata": http://msdn.microsoft.com/en-us/library/ff701716.aspx
// *** DO NOT MODIFY unless bing map API is changing.
#define BING_MAPS_METADATA_URL    "http://dev.virtualearth.net/REST/V1/Imagery/Metadata/{ImagerySet}?o=xml&incl=ImageryProviders&key={BingMapsKey}"

// Tags use to format server request
#define SUBDOMAIN_TAG   "{subdomain}"
#define QUADKEY_TAG     "{quadkey}"
#define CULTURE_TAG     "{culture}"
#define IMAGERYSET_TAG  "{ImagerySet}"
#define BINGMAPSKEY_TAG "{BingMapsKey}"

// These are tags that we use to build a pseudo Bing map URL.
// They cannot be changed since MicroStation will persist these URL in its DGN format.
// {ImagerySet} = "Road", "AerialWithLabels" or whatever bing map support
#define PSEUDO_BING_PARTIAL_URL "www.bing.com/maps/"
#define PSEUDO_BING_URL "http://" PSEUDO_BING_PARTIAL_URL IMAGERYSET_TAG    // http://www.bing.com/maps/{ImagerySet}

// Xml Response 
#define BING_XML_REST_1_0               "http://schemas.microsoft.com/search/local/ws/rest/v1"
#define BING_NAMESPACE_PREFIX           "BingMaps"
#define BING_RESPONSE_ELEMENT           "Response"
#define BING_BRANDLOGOURI_ELEMENT       "BrandLogoUri"
#define BING_RESOURCESETS_ELEMENT       "ResourceSets"
#define BING_RESOURCESET_ELEMENT        "ResourceSet"
#define BING_RESOURCES_ELEMENT          "Resources"
#define BING_IMAGERYMETADATA_ELEMENT    "ImageryMetadata"
#define BING_IMAGEURL_ELEMENT           "ImageUrl"
#define BING_IMAGEURLSUBDOMAINS_ELEMENT "ImageUrlSubdomains"
#define BING_IMAGERYPROVIDER_ELEMENT    "ImageryProvider"
#define BING_ATTRIBUTION_ELEMENT        "Attribution"
#define BING_COVERAGEAREA_ELEMENT       "CoverageArea"
#define BING_ZOOMMIN_ELEMENT            "ZoomMin"
#define BING_ZOOMMAX_ELEMENT            "ZoomMax"
#define BING_BOUNDINGBOX_ELEMENT        "BoundingBox"
#define BING_SOUTHLATITUDE_ELEMENT      "SouthLatitude"
#define BING_WESTLONGITUDE_ELEMENT      "WestLongitude"
#define BING_NORTHLATITUDE_ELEMENT      "NorthLatitude"
#define BING_EASTLONGITUDE_ELEMENT      "EastLongitude"

#define VE_MAP_RESOLUTION       21
#define VE_MAP_WIDTH            (256 * (1 << VE_MAP_RESOLUTION))
#define VE_MAP_HEIGHT           (256 * (1 << VE_MAP_RESOLUTION))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetNodeContentInt32(int32_t& value, BeXmlNodeP pNode, Utf8CP childPath)
    {
    if(NULL == pNode)
        return false;

    BeXmlNodeP pChildNode = pNode->SelectSingleNode(childPath);
    if(NULL == pChildNode || BEXML_Success != pChildNode->GetContentInt32Value(value))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetNodeContentDouble(double& value, BeXmlNodeP pNode, Utf8CP childPath)
    {
    if(NULL == pNode)
        return false;

    BeXmlNodeP pChildNode = pNode->SelectSingleNode(childPath);
    if(NULL == pChildNode || BEXML_Success != pChildNode->GetContentDoubleValue(value))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ReadImageryProvider(HRFVirtualEarthFile::ImageryProvider& provider, BeXmlNodeP pProviderNode)
    {
    if(NULL == pProviderNode)
        return false;

    provider.attribution.clear();
    provider.coverage.clear();

    for(BeXmlNodeP pSubProviderNode = pProviderNode->GetFirstChild (); NULL != pSubProviderNode; pSubProviderNode = pSubProviderNode->GetNextSibling())
        {
        if(pSubProviderNode->IsName(BING_ATTRIBUTION_ELEMENT))
            {
            pSubProviderNode->GetContent(provider.attribution);
            }
        else if(pSubProviderNode->IsName(BING_COVERAGEAREA_ELEMENT))
            {
            HRFVirtualEarthFile::CoverageArea area;
            if(GetNodeContentInt32(area.levelOfDetailMin, pSubProviderNode, BING_NAMESPACE_PREFIX ":" BING_ZOOMMIN_ELEMENT) && 
               GetNodeContentInt32(area.levelOfDetailMax, pSubProviderNode, BING_NAMESPACE_PREFIX ":" BING_ZOOMMAX_ELEMENT) && 
               GetNodeContentDouble(area.southLatitude, pSubProviderNode, BING_NAMESPACE_PREFIX ":" BING_BOUNDINGBOX_ELEMENT "/" BING_NAMESPACE_PREFIX ":" BING_SOUTHLATITUDE_ELEMENT) && 
               GetNodeContentDouble(area.westLongitude, pSubProviderNode, BING_NAMESPACE_PREFIX ":" BING_BOUNDINGBOX_ELEMENT "/" BING_NAMESPACE_PREFIX ":" BING_WESTLONGITUDE_ELEMENT) && 
               GetNodeContentDouble(area.northLatitude, pSubProviderNode, BING_NAMESPACE_PREFIX ":" BING_BOUNDINGBOX_ELEMENT "/" BING_NAMESPACE_PREFIX ":" BING_NORTHLATITUDE_ELEMENT) && 
               GetNodeContentDouble(area.eastLongitude, pSubProviderNode, BING_NAMESPACE_PREFIX ":" BING_BOUNDINGBOX_ELEMENT "/" BING_NAMESPACE_PREFIX ":" BING_EASTLONGITUDE_ELEMENT))
                {
                provider.coverage.push_back(area);
                }
            }
        }

    return !provider.coverage.empty();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ReplaceTagInString(Utf8StringR str, Utf8StringCR tag, Utf8StringCR tagValue)
{
    Utf8String::size_type pos = str.find(tag);
    if(Utf8String::npos == pos)
        return false;

    str.replace(pos, tag.size(), tagValue);
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void HRFVirtualEarthFile::QueryImageURI(Utf8StringCR bingMapKey)
    {
    m_ImageURI.clear();
    m_ImageURISubdomains.clear();
    m_LogoURI.clear();

    Utf8String imagerySetLabel;
    if(!FindImagerySetFromURL(imagerySetLabel, *GetURL()))
        throw HFCCorruptedFileException(GetURL()->GetURL());

    // Query server info from Microsoft
    // Need to send an http request like this:
    // http://dev.virtualearth.net/REST/V1/Imagery/Metadata/Road?incl=ImageryProviders&o=xml&key=AjdiY0PuqXOEdT0JWVjvXqC3nydpHgDEhLcUwXtnKUUH_BW5u3pV3-Zhk5Ez_mwt
    // http://dev.virtualearth.net/REST/V1/Imagery/Metadata/{ImagerySet}?incl=ImageryProviders&o=xml&key={bingMapKey}
    // ref: http://msdn.microsoft.com/en-us/library/ff701716.aspx
    Utf8String urlRequest(BING_MAPS_METADATA_URL);
    ReplaceTagInString(urlRequest, IMAGERYSET_TAG, imagerySetLabel);
    ReplaceTagInString(urlRequest, BINGMAPSKEY_TAG, bingMapKey);

    HttpSession session;
    HttpRequest request(urlRequest.c_str());
    HttpResponsePtr response;
    if(HttpRequestStatus::Success != session.Request(response, request) || response.IsNull() || response->GetBody().empty())
        throw HFCCannotOpenFileException(GetURL()->GetURL());

    // Analyze response
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory (xmlStatus, response->GetBody().data(), response->GetBody().size());

    //Validate pDom
    if (pXmlDom.IsNull() || (BEXML_Success != xmlStatus))
        throw HFCCannotOpenFileException(GetURL()->GetURL());

    pXmlDom->RegisterNamespace(BING_NAMESPACE_PREFIX, BING_XML_REST_1_0);

    // Select ImageryMetaDataNode
    BeXmlNodeP pImageryMetaDataNode;
    if(BEXML_Success == pXmlDom->SelectNode (pImageryMetaDataNode, "/" BING_NAMESPACE_PREFIX ":" BING_RESPONSE_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_RESOURCESETS_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_RESOURCESET_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_RESOURCES_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_IMAGERYMETADATA_ELEMENT,
                                                                   NULL, BeXmlDom::NODE_BIAS_First))
        {
        for(BeXmlNodeP pSubImageryMetaDataNode = pImageryMetaDataNode->GetFirstChild (); NULL != pSubImageryMetaDataNode; pSubImageryMetaDataNode = pSubImageryMetaDataNode->GetNextSibling())
            {
            if(pSubImageryMetaDataNode->IsName(BING_IMAGERYPROVIDER_ELEMENT))
                {
                ImageryProvider provider;
                if(ReadImageryProvider(provider, pSubImageryMetaDataNode))
                    m_Providers.push_back(provider);
                }
            else if(pSubImageryMetaDataNode->IsName(BING_IMAGEURL_ELEMENT))
                {
                pSubImageryMetaDataNode->GetContent(m_ImageURI);
                }
            else if(pSubImageryMetaDataNode->IsName(BING_IMAGEURLSUBDOMAINS_ELEMENT))
                {
                for(BeXmlNodeP pSubDomainNode = pSubImageryMetaDataNode->GetFirstChild (); NULL != pSubDomainNode || m_ImageURISubdomains.size() > 4; pSubDomainNode = pSubDomainNode->GetNextSibling())
                    {
                    Utf8String subDomain;
                    if(BEXML_Success != pSubDomainNode->GetContent(subDomain))
                        break;

                    m_ImageURISubdomains.push_back(subDomain);
                    }   
                }            
            }
        }        

    pXmlDom->SelectNodeContent(m_LogoURI, "/" BING_NAMESPACE_PREFIX ":" BING_RESPONSE_ELEMENT "/" BING_NAMESPACE_PREFIX ":" BING_BRANDLOGOURI_ELEMENT, NULL, BeXmlDom::NODE_BIAS_First);

    // Image URI must have the following form : "http://{subdomain}.tiles.virtualearth.net/tiles/r{quadkey}.jpeg?g=266&mkt={culture}"
    if(m_ImageURI.empty() || m_ImageURI.find(SUBDOMAIN_TAG) == Utf8String::npos ||
       m_ImageURI.find(QUADKEY_TAG) == Utf8String::npos ||
       // Not fatal! 
       //       m_ImageURI.find(CULTURE_TAG) == Utf8String::npos ||
       // Not Fatal. Some layer(ex. 'OrdnanceSurvey' and 'CollinsBart') do not have providers.
       //       m_Providers.empty() ||  
       // PM: Not a fatal error.
       //       m_LogoURI.empty()     
       m_ImageURISubdomains.size() != 4)
        throw HFCCannotOpenFileException(GetURL()->GetURL());

    // Replace the culture tag now, it won't change
    Utf8String CultureVal = ImagePPMessages::GetString(ImagePPMessages::BingMapsCultureId());

    ReplaceTagInString(m_ImageURI, CULTURE_TAG, !CultureVal.empty() ? CultureVal : CULTURE_TAG);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HRFVirtualEarthFile::GetTileURI(uint32_t pixelX, uint32_t pixelY, int32_t levelOfDetail) const
    {
    HASSERT(levelOfDetail >=1 && levelOfDetail <= 23); // 1 (lowest detail) to 23 (highest detail).

    Utf8String imageURI = m_ImageURI;
    string quadKey = HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToQuadKey(pixelX, pixelY, levelOfDetail);

    ReplaceTagInString(imageURI, SUBDOMAIN_TAG, m_ImageURISubdomains[quadKey[quadKey.size()-1] - '0']);
    ReplaceTagInString(imageURI, QUADKEY_TAG, Utf8String(quadKey.c_str()));

    return imageURI;
    }

//-----------------------------------------------------------------------------
// HRFVirtualEarthBlockCapabilities
// Note: Define in the .h because GeoTiff used the same.
//-----------------------------------------------------------------------------
class HRFVirtualEarthBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFVirtualEarthBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY, // AccessMode
                                  INT32_MAX,             // MaxSizeInBytes
                                  256,                  // MinWidth
                                  256,                  // MaxWidth
                                  0,                    // WidthIncrement
                                  256,                  // MinHeight
                                  256,                  // MaxHeight
                                  0,                    // HeightIncrement
                                  false));              // Not Square
        }
    };

//-----------------------------------------------------------------------------
// HRFVirtualEarthCodecTrueColorCapabilities
//-----------------------------------------------------------------------------
class HRFVirtualEarthCodecTrueColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFVirtualEarthCodecTrueColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFVirtualEarthBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFVirtualEarthBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFVirtualEarthCapabilities
//-----------------------------------------------------------------------------
HRFVirtualEarthCapabilities::HRFVirtualEarthCapabilities()
    :HRFRasterFileCapabilities()
    {
    // PixelTypeV24R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFVirtualEarthCodecTrueColorCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY,
                                             HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // MultiResolution Capability
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_ONLY, // AccessMode,
        true,                  // SinglePixelType,
        true,                  // SingleBlockType,
        false,                 // ArbitaryXRatio,
        false);                // ArbitaryYRatio);
    Add(pMultiResolutionCapability);

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability;

    pGeocodingCapability = new HRFGeocodingCapability(HFC_READ_ONLY);

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogCitation);
    pGeocodingCapability->AddSupportedKey(GeogGeodeticDatum);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridian);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnits);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogEllipsoid);
    pGeocodingCapability->AddSupportedKey(GeogSemiMajorAxis);
    pGeocodingCapability->AddSupportedKey(GeogSemiMinorAxis);
    pGeocodingCapability->AddSupportedKey(GeogInvFlattening);
    pGeocodingCapability->AddSupportedKey(GeogAzimuthUnits);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridianLong);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel1);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel2);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseNorthing);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginNorthing);
    pGeocodingCapability->AddSupportedKey(ProjCenterLong);
    pGeocodingCapability->AddSupportedKey(ProjCenterLat);
    pGeocodingCapability->AddSupportedKey(ProjCenterEasting);
    pGeocodingCapability->AddSupportedKey(ProjCenterNorthing);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtNatOrigin);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtCenter);
    pGeocodingCapability->AddSupportedKey(ProjAzimuthAngle);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    }

//-----------------------------------------------------------------------------//
//                      HRFVirtualEarthCreator                                 //
//-----------------------------------------------------------------------------//
HFC_IMPLEMENT_SINGLETON(HRFVirtualEarthCreator)

//-----------------------------------------------------------------------------
// VirtualEarth capabilities instance member definition
//-----------------------------------------------------------------------------
HRFVirtualEarthCreator::HRFVirtualEarthCreator()
    : HRFRasterFileCreator(HRFVirtualEarthFile::CLASS_ID)
    {
    // VirtualEathFile capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFVirtualEarthCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_BingMaps());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFVirtualEarthCreator::GetSchemes() const
    {
    return Utf8String(HFCURLHTTP::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFVirtualEarthCreator::GetExtensions() const
    {
    return Utf8String("");
    }

//-----------------------------------------------------------------------------
// Create an instance of an object that represents an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFVirtualEarthCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                     HFCAccessMode         pi_AccessMode,
                                                     uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFVirtualEarthFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFVirtualEarthCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                           uint64_t             pi_Offset) const
    {
    return HRFVirtualEarthFile::IsURLVirtualEarth(*pi_rpURL);
    }

//-----------------------------------------------------------------------------
// Verifies that the file format supports this kind of URL.
//-----------------------------------------------------------------------------
bool HRFVirtualEarthCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    return IsKindOfFile(pi_rpURL, 0);
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of the Virtual File file format.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFVirtualEarthCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFVirtualEarthCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------//
//                         HRFVirtualEarthFile                                 //
//-----------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFVirtualEarthFile::HRFVirtualEarthFile(const HFCPtr<HFCURL>& pi_rURL,
                                         HFCAccessMode         pi_AccessMode,
                                         uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {

    HFCAuthenticationCallback* pCallback = (HFCAuthenticationCallback*)HFCCallbackRegistry::GetInstance()->GetCallback(HFCAuthenticationCallback::CLASS_ID);

    if(pCallback == 0)
        throw HFCLoginInformationNotAvailableException();

    if(!pi_rURL->IsCompatibleWith(HFCURLHTTPBase::CLASS_ID))
        throw HFCCannotOpenFileException(pi_rURL->GetURL());

#ifdef BINGMAPS_AUTO_PASSWORD        
    QueryImageURI("AjdiY0PuqXOEdT0JWVjvXqC3nydpHgDEhLcUwXtnKUUH_BW5u3pV3-Zhk5Ez_mwt");
#else
    HFCInternetAuthentication VEAuthentication(pi_rURL->GetURL());

    if (!pCallback->GetAuthentication(&VEAuthentication))
        {
        if (pCallback->IsCancelled())
            throw HRFAuthenticationCancelledException(GetURL()->GetURL());
        else
            throw HRFAuthenticationInvalidLoginException(GetURL()->GetURL());
        }             

    // Will contact virtual earth server and request image URI. Will throw appropriate exception if an error occurs.
    QueryImageURI(VEAuthentication.GetPassword());
#endif   

    // Create Page and Res Descriptors.
    CreateDescriptors();
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor that allows the creation of an image file object without opening
// the file.
//-----------------------------------------------------------------------------
HRFVirtualEarthFile::HRFVirtualEarthFile(const HFCPtr<HFCURL>& pi_rURL,
                                         HFCAccessMode         pi_AccessMode,
                                         uint64_t             pi_Offset,
                                         bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFVirtualEarthFile::~HRFVirtualEarthFile()
    {
    }

//-----------------------------------------------------------------------------
// Compose a Virtual Earth URL
//-----------------------------------------------------------------------------
Utf8String HRFVirtualEarthFile::ComposeURL(Utf8StringCR imagerySet)
    {
    Utf8String bingpseudoURL = PSEUDO_BING_URL;
    ReplaceTagInString(bingpseudoURL, IMAGERYSET_TAG, imagerySet);

    return bingpseudoURL;
    }

//-----------------------------------------------------------------------------
// Find the map style from an URL and return the label needed for the http request.
//-----------------------------------------------------------------------------
bool HRFVirtualEarthFile::FindImagerySetFromURL(Utf8StringR imagerySet, HFCURL const& bingURL)
    {
    //Input: http://www.bing.com/maps/{ImagerySet}
    imagerySet.clear();

    if(!bingURL.IsCompatibleWith(HFCURLHTTPBase::CLASS_ID) || bingURL.GetSchemeType() != HFCURLHTTP::s_SchemeName())
        return false;

    HFCURLHTTPBase const& HttpURL = static_cast<HFCURLHTTPBase const&>(bingURL);

    // Avoid the default port(:80) added by HFCURLCommonInternet::GetURL()
    Utf8String cleanedUrl = HttpURL.GetHost() + "/" + HttpURL.GetURLPath();

    Utf8String partialUrl(PSEUDO_BING_PARTIAL_URL);
    partialUrl.ToLower();   // using to lower should not affect bing URL. We do not have a case-insensitive find for UTF8

    Utf8String::size_type partialPos = partialUrl.find(partialUrl);
    if(Utf8String::npos == partialPos)
        {
        HASSERT(!"Invalid bing URL");
        return false;
        }        

    // look for first delimiter and remove it if any.
    Utf8String::size_type imageryStartPos = partialPos + partialUrl.length();
    Utf8String::size_type imageryEndPos = cleanedUrl.find_first_of("\\/?", imageryStartPos);
    imagerySet = cleanedUrl.substr(imageryStartPos, imageryEndPos-imageryStartPos);

    return !imagerySet.empty();
    }

//-----------------------------------------------------------------------------
// Is a Virtual Earth URL
//-----------------------------------------------------------------------------
bool HRFVirtualEarthFile::IsURLVirtualEarth(HFCURL const& bingURL)
    {
    if(!bingURL.IsCompatibleWith(HFCURLHTTPBase::CLASS_ID) || 
        bingURL.GetSchemeType() != HFCURLHTTP::s_SchemeName())
        return false;

    HFCURLHTTPBase const& HttpURL = static_cast<HFCURLHTTPBase const&>(bingURL);

    // Avoid the default port(:80) added by HFCURLCommonInternet::GetURL()
    Utf8String cleanedUurl = HttpURL.GetHost() + "/" + HttpURL.GetURLPath();

    return cleanedUurl.ContainsI(PSEUDO_BING_PARTIAL_URL);
    }

//-----------------------------------------------------------------------------
// Create a resolution editor
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFVirtualEarthFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                                 uint16_t pi_Resolution,
                                                                 HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFVirtualEarthEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    return pEditor;
    }

//-----------------------------------------------------------------------------
// Save
//-----------------------------------------------------------------------------
void HRFVirtualEarthFile::Save()
    {
    //Virtual Earth can be opened only in read-only mode.
    HASSERT(0);
    }

//-----------------------------------------------------------------------------
// Get the capabilities supported by the Virtual Earth file format
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFVirtualEarthFile::GetCapabilities () const
    {
    return HRFVirtualEarthCreator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Create the descriptors
//-----------------------------------------------------------------------------
void HRFVirtualEarthFile::CreateDescriptors()
    {
    HPRECONDITION (IsURLVirtualEarth(*GetURL()));

    // Pixel Type
    HFCPtr<HRPPixelType> PixelType = new HRPPixelTypeV24R8G8B8();

    // Transfo model
    double Scale = HRFVirtualEarthEditor::VirtualEarthTileSystem::GroundResolution(0.0, VE_MAP_RESOLUTION);
    double offsetLatitude;
    double offsetLongitude;
    HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToLatLong(0, 0, VE_MAP_RESOLUTION, &offsetLatitude, &offsetLongitude);
    if (offsetLongitude < -179.9999999999)
        offsetLongitude = -179.9999999999;
    if (offsetLongitude > 179.9999999999)
        offsetLongitude = 179.9999999999;

    // Geocoding and Reference
    HFCPtr<HGF2DTransfoModel> pTransfoModel;

    GeoCoordinates::BaseGCSPtr pBaseGCS;


     WString WKTString = L"PROJCS[\"EPSG:900913\", \
                                       GEOGCS[\"GCS_Sphere_WGS84\", \
                                DATUM[\"SphereWGS84\", \
                                SPHEROID[\"SphereWGS84\",6378137.0,0.0], \
                                TOWGS84[0, 0, 0, 0, 0, 0, 0] \
                                ], \
                                    PRIMEM[\"Greenwich\",0.0], \
                                    UNIT[\"Degree\",0.0174532925199433 ], \
                                ], \
                                PROJECTION[\"Mercator\"], \
                                PARAMETER[\"False_Easting\",0.0], \
                                PARAMETER[\"False_Northing\",0.0], \
                                PARAMETER[\"Central_Meridian\",0.0], \
                                PARAMETER[\"Standard_Parallel_1\",0.0], \
                                UNIT[\"Meter\", 1.0] \
                                ]";

     // Obtain the GCS
     pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS();
     if(SUCCESS == pBaseGCS->InitFromWellKnownText (NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, WKTString.c_str()))
         {       
         GeoPoint geoPoint = {offsetLongitude, offsetLatitude, 0.0};
         DPoint3d cartesianPoint;
         pBaseGCS->CartesianFromLatLong (cartesianPoint, geoPoint);
         pTransfoModel = new HGF2DStretch(HGF2DDisplacement(cartesianPoint.x, cartesianPoint.y), Scale, Scale);
         }


    if(pTransfoModel == nullptr)
        pTransfoModel = new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), Scale, Scale);
        
    // Flip the Y Axe because the origin of ModelSpace is lower-left
    HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();
    pFlipModel->SetYScaling(-1.0);
    pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);
    // Instantiation of Resolution descriptor
    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;

    // We used to had problems when going larger than INT_MAX. Need to test that if VE_MAP_WIDTH exceed that.
    HASSERT(VE_MAP_WIDTH <= INT_MAX && VE_MAP_HEIGHT <= INT_MAX);

    uint32_t Width = VE_MAP_WIDTH;
    uint32_t Height= VE_MAP_HEIGHT;

    HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
        HFC_READ_ONLY,                                      // AccessMode,
        GetCapabilities(),                                  // Capabilities,
        1.0,                                                // XResolutionRatio,
        1.0,                                                // YResolutionRatio,
        PixelType,                                          // PixelType,
        new HCDCodecIdentity(),                             // Codecs,
        HRFBlockAccess::RANDOM,                             // RBlockAccess,
        HRFBlockAccess::RANDOM,                             // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                           // InterleaveType
        false,                                              // IsInterlace,
        Width,                                              // Width,
        Height,                                             // Height,
        256,                                                // BlockWidth,
        256,                                                // BlockHeight,
        0,                                                  // BlocksDataFlag
        HRFBlockType::TILE);                                // BlockType

    ListOfResolutionDescriptor.push_back(pResolution);

    for (uint16_t Resolution = 1; Resolution < VE_MAP_RESOLUTION; ++Resolution)
        {
        Width /= 2;
        Height /= 2;

        double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(VE_MAP_WIDTH, Width);

        HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
            HFC_READ_ONLY,                                      // AccessMode,
            GetCapabilities(),                                  // Capabilities,
            Ratio,                                              // XResolutionRatio,
            Ratio,                                              // YResolutionRatio,
            PixelType,                                          // PixelType,
            new HCDCodecIdentity(),                             // Codecs,
            HRFBlockAccess::RANDOM,                             // RBlockAccess,
            HRFBlockAccess::RANDOM,                             // WBlockAccess,
            HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
            HRFInterleaveType::PIXEL,                           // InterleaveType
            false,                                              // IsInterlace,
            Width,                                              // Width,
            Height,                                             // Height,
            256,                                                // BlockWidth,
            256,                                                // BlockHeight,
            0,                                                  // BlocksDataFlag
            HRFBlockType::TILE);                                // BlockType

        ListOfResolutionDescriptor.push_back(pResolution);
        }

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (HFC_READ_ONLY,
                                   GetCapabilities(),           // Capabilities,
                                   ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                   0,                           // RepresentativePalette,
                                   0,                           // Histogram,
                                   0,                           // Thumbnail,
                                   0,                           // ClipShape,
                                   pTransfoModel,               // TransfoModel,
                                   0,                           // Filters
                                   0);                           // Defined tag


    if (!pBaseGCS.IsNull() && pBaseGCS->IsValid())
        pPage->SetGeocoding(pBaseGCS.get());

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Get the file current size
//-----------------------------------------------------------------------------
uint64_t HRFVirtualEarthFile::GetFileCurrentSize() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// Get the file current size using the given binary stream
//-----------------------------------------------------------------------------
uint64_t HRFVirtualEarthFile::GetFileCurrentSize(HFCBinStream* pi_pBinStream) const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// protected
// RequestLookAhead
//
// HRFVirtualEarthFile support the LookAhead by extent. This means that all blocks
// must be on the same resolution.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Protected
// Function that is called to request some tiles to be fetch to the server
// before that are actually read by the application. HRFVirtualEarthFile
// supports only look ahead by tiles.
//-----------------------------------------------------------------------------
void HRFVirtualEarthFile::RequestLookAhead(uint32_t             pi_Page,
                                           const HGFTileIDList& pi_rBlocks,
                                           bool                pi_Async)
    {
    HGFTileIDList::const_iterator Itr(pi_rBlocks.begin());
    if (Itr != pi_rBlocks.end())
        {
        uint16_t Resolution = (uint16_t)HRFRasterFile::s_TileDescriptor.GetLevel(*Itr);

        //Find the resolution editor into the ResolutionEditorRegistry
        ResolutionEditorRegistry::const_iterator ResItr(m_ResolutionEditorRegistry.begin());
        HRFVirtualEarthEditor* pResEditor = 0;
        while (pResEditor == 0 && ResItr != m_ResolutionEditorRegistry.end())
            {
            if (((*ResItr)->GetPage() == pi_Page) && ((*ResItr)->GetResolutionIndex() == Resolution))
                {
                pResEditor = (HRFVirtualEarthEditor*)*ResItr;
                }
            else
                {
                ResItr++;
                }
            }
        //The editor must exist
        HASSERT(pResEditor != 0);

        pResEditor->RequestLookAhead(pi_rBlocks);
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Cancel the current look ahead
//-----------------------------------------------------------------------------
void HRFVirtualEarthFile::CancelLookAhead(uint32_t pi_Page)
    {
    HPRECONDITION(pi_Page == 0);

    // I assumed that HRF is not thread safe and that there will be only one thread that will execute LookAHead request and copyFrom(ReadBlock).
    m_tileQueryMap.clear();
    }

//-----------------------------------------------------------------------------
// Get the world identificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFVirtualEarthFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
    }

BEGIN_IMAGEPP_NAMESPACE
struct ThreadLocalHttp : ThreadLocalStorage<HttpSession>{};
END_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
WorkerPool& HRFVirtualEarthFile::GetWorkerPool()
    {
    if(m_pWorkerPool == nullptr)
        {
        m_pWorkerPool.reset(new WorkerPool(NB_BLOCK_READER_THREAD));
        m_threadLocalHttp.reset(new ThreadLocalHttp()); // must be allocated before threads start querying.
        }

    return *m_pWorkerPool;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
HttpSession& HRFVirtualEarthFile::GetThreadLocalHttpSession()
    {
    // Call by multiple threads. do no allocate m_threadLocalHttp here.
    return m_threadLocalHttp->GetLocal();
    }

//-----------------------------------------------------------------------------
// Indicates that the Virtual Earth file format supports the look ahead by
// block (i.e. tile).
//-----------------------------------------------------------------------------
bool HRFVirtualEarthFile::HasLookAheadByBlock(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return true;
    }

//-----------------------------------------------------------------------------
// Indicates that the a look ahead can be performed for the Virtual Earth
// file format.
//-----------------------------------------------------------------------------
bool HRFVirtualEarthFile::CanPerformLookAhead(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page == 0);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HRFVirtualEarthFile::GetBrandLogoURI() const
    {
    return m_LogoURI;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRFVirtualEarthFile::ImageryProviders const& HRFVirtualEarthFile::GetProviders() const
    {
    return m_Providers;
    }
