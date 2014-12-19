//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFVirtualEarthFile.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFVirtualEarthFile
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HRFVirtualEarthFile.h>
#include <Imagepp/all/h/HRFVirtualEarthEditor.h>
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
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

#include <Imagepp/all/h/HCPException.h>

#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <BeXml/BeXml.h>

// Do not change this number without validating resolution descriptor creation.
#define NB_BLOCK_READER_THREAD 10

USING_NAMESPACE_IMAGEPP

// These are tags that we use to build a pseudo Bing map URL.
// They cannot be changed since MicroStation will persist these URL in its DGN format.
#define PSEUDO_VE_SERVER            L"BingMaps.com" 
#define PSEUDO_VE_AERIAL            L"/Imagery/Aerial"
#define PSEUDO_VE_AERIALWITHLABEL   L"/Imagery/AerialWithLabels"
#define PSEUDO_VE_ROAD              L"/Imagery/Road" 
#define PSEUDO_VE_SERVER_URL        HFCURLHTTP::s_SchemeName() + L"://" + PSEUDO_VE_SERVER

// These are the ImagerySet tag that we use when requesting tiles URI. 
// See Bing map "Get Imagery Metadata": http://msdn.microsoft.com/en-us/library/ff701716.aspx
// *** DO NOT MODIFY unless bing map API is changing.
#define BING_IMAGERYSET_AERIAL          L"Aerial"
#define BING_IMAGERYSET_AERIALWITHLABEL L"AerialWithLabels"
#define BING_IMAGERYSET_ROAD            L"Road"
#define BING_MAPS_SERVER                L"http://dev.virtualearth.net/REST/V1/Imagery/Metadata/{ImagerySet}?o=xml&key={BingMapsKey}"

// Tags use to format server request
#define SUBDOMAIN_TAG   L"{subdomain}"
#define QUADKEY_TAG     L"{quadkey}"
#define CULTURE_TAG     L"{culture}"
#define IMAGERYSET_TAG  L"{ImagerySet}"
#define BINGMAPSKEY_TAG L"{BingMapsKey}"

// Xml Response 
#define BING_XML_REST_1_0               "http://schemas.microsoft.com/search/local/ws/rest/v1"
#define BING_NAMESPACE_PREFIX           "BingMaps"
#define BING_RESPONSE_ELEMENT           "Response"
#define BING_RESOURCESETS_ELEMENT       "ResourceSets"
#define BING_RESOURCESET_ELEMENT        "ResourceSet"
#define BING_RESOURCES_ELEMENT          "Resources"
#define BING_IMAGERYMETADATA_ELEMENT    "ImageryMetadata"
#define BING_IMAGEURL_ELEMENT           "ImageUrl"
#define BING_IMAGEURLSUBDOMAINS_ELEMENT "ImageUrlSubdomains"

// Bentley master key to use when connecting to Microsoft Bing Maps server. From a Microsoft point of view, all of our users are using this key.
#define BING_MAPS_MASTER_KEY      L"AuKdXwGWIqiRlk_8sjFecGpFMTfTnW1a3YtAQAz4liSCzcGdcwWC3YgGmN2Tfasn"
#define BING_DEFAULT_CULTURE      L"en-US"

// Terra Pixel authentication. The key is provided by the users. Response is in plain text : "VALID" or "INVALID"
// License request ex: "http://license.terrapixel.com/bing/Register/license_check.php?KEY=9cd61748-be07-468b-b3bc-ce45b60750ac"
#define TERRAPIXEL_SERVER       L"http://license.terrapixel.com/bing/Register/license_check.php?KEY={TerraPixelKey}"    
#define TERRAPIXELKEY_TAG       L"{TerraPixelKey}"
#define TERRAPIXELKEY_VALID     "VALID"


#define VE_MAP_RESOLUTION       21
#define VE_MAP_WIDTH            (256 * (1 << VE_MAP_RESOLUTION))
#define VE_MAP_HEIGHT           (256 * (1 << VE_MAP_RESOLUTION))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFVirtualEarthFile::ReplaceTagInString(WStringR str, WStringCR tag, WStringCR tagValue) const
{
    WString::size_type pos = str.find(tag);
    if(WString::npos == pos)
        return false;

    str.replace(pos, tag.size(), tagValue);
    return true;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void HRFVirtualEarthFile::QueryImageURI()
    {
    m_ImageURI.clear();
    m_ImageURISubdomains.clear();

    WString imagerySetLabel;
    if(!DetectImagerySetFromURL(imagerySetLabel, *GetURL()))
        throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, GetURL()->GetURL());

    // Query server info from Microsoft
    WString URLRequest(BING_MAPS_SERVER);
    ReplaceTagInString(URLRequest, IMAGERYSET_TAG, imagerySetLabel);
    ReplaceTagInString(URLRequest, BINGMAPSKEY_TAG, BING_MAPS_MASTER_KEY);

    HFCPtr<HFCBuffer> pResponse = SendAndReceiveRequest(URLRequest);

    if(pResponse == NULL ||  pResponse->GetDataSize() == 0)
        throw HFCFileException(HFC_CANNOT_OPEN_FILE_EXCEPTION, GetURL()->GetURL());

    // Analyse response
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory (xmlStatus, pResponse->GetData(), pResponse->GetDataSize());

    //Validate pDom
    if (pXmlDom.IsNull() || (BEXML_Success != xmlStatus))
        throw HFCFileException(HFC_CANNOT_OPEN_FILE_EXCEPTION, GetURL()->GetURL());

    pXmlDom->RegisterNamespace(BING_NAMESPACE_PREFIX, BING_XML_REST_1_0);

    // Validate main node presence
    BeXmlNodeP pImageryMetaDataNode;
    if(BEXML_Success == pXmlDom->SelectNode (pImageryMetaDataNode, "/" BING_NAMESPACE_PREFIX ":" BING_RESPONSE_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_RESOURCESETS_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_RESOURCESET_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_RESOURCES_ELEMENT
                                                                   "/" BING_NAMESPACE_PREFIX ":" BING_IMAGERYMETADATA_ELEMENT,
                                                                   NULL, BeXmlDom::NODE_BIAS_First))
        {
        BeXmlNodeP pImageUrlNode = pImageryMetaDataNode->SelectSingleNode(BING_NAMESPACE_PREFIX ":" BING_IMAGEURL_ELEMENT);
        BeXmlNodeP pImageUrlSubDomainsNode = pImageryMetaDataNode->SelectSingleNode(BING_NAMESPACE_PREFIX ":" BING_IMAGEURLSUBDOMAINS_ELEMENT);
        if(NULL != pImageUrlNode && NULL != pImageUrlSubDomainsNode)
            {
            pImageUrlNode->GetContent(m_ImageURI);

            for(BeXmlNodeP pSubDomainNode = pImageUrlSubDomainsNode->GetFirstChild (); 
                NULL != pSubDomainNode || m_ImageURISubdomains.size() > 4; 
                pSubDomainNode = pSubDomainNode->GetNextSibling())
                {
                WString subDomain;
                if(BEXML_Success != pSubDomainNode->GetContent(subDomain))
                    break;

                m_ImageURISubdomains.push_back(subDomain);
                }   
            }
        }


    // Image URI must have the following form : "http://{subdomain}.tiles.virtualearth.net/tiles/r{quadkey}.jpeg?g=266&mkt={culture}"
    if(m_ImageURI.empty() || m_ImageURI.find(SUBDOMAIN_TAG) == WString::npos ||
       m_ImageURI.find(QUADKEY_TAG) == std::string::npos ||
       //Not fatal! m_ImageURI.find(CULTURE_TAG) == WString::npos ||
       m_ImageURISubdomains.size() != 4)
        throw HFCFileException(HFC_CANNOT_OPEN_FILE_EXCEPTION, GetURL()->GetURL());

    // Replace the culture tag now, it won't change
    WString CultureVal = HFCResourceLoader::GetInstance()->GetString(IDS_BingMapsCultureId);

    ReplaceTagInString(m_ImageURI, CULTURE_TAG, !CultureVal.empty() ? CultureVal : BING_DEFAULT_CULTURE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HFCBuffer> HRFVirtualEarthFile::SendAndReceiveRequest(WStringCR URLRequest) const
    {
    WString::size_type requestPos = URLRequest.find('?');
    if(requestPos == WString::npos)
        return NULL;

    WString serverURL  = URLRequest.substr(0, requestPos);
    WString request    = URLRequest.substr(requestPos + 1);

    HFCPtr<HRFVirtualEarthConnection> pConnection = new HRFVirtualEarthConnection(serverURL);

    if (pConnection->Connect(pConnection->GetUserName(), pConnection->GetPassword()))
        {
        uint32_t codePage;
        BeStringUtilities::GetCurrentCodePage (codePage);

        AString requestA;
        BeStringUtilities::WCharToLocaleChar (requestA, codePage, request.c_str());

        //&&MM should have a unicode version of pConnection->Send
        pConnection->Send((Byte const*)requestA.c_str(), requestA.size());

        HFCPtr<HFCBuffer> pResponse = new HFCBuffer(1,1);
        while (!pConnection->RequestEnded())
            {
            size_t DataAvailable;

            // Wait for data to arrive
            if ((DataAvailable = pConnection->WaitDataAvailable(pConnection->GetTimeOut())) > 0)
                {
                // read it
                pConnection->Receive(pResponse->PrepareForNewData(DataAvailable), DataAvailable);
                pResponse->SetNewDataSize(DataAvailable);
                }
            }

        return pResponse;
        }

    return NULL;    // ERROR.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WString HRFVirtualEarthFile::GetTileURI(unsigned int pixelX, unsigned int pixelY, int levelOfDetail) const
    {
    HASSERT(levelOfDetail >=1 && levelOfDetail <= 23); // 1 (lowest detail) to 23 (highest detail).

    WString imageURI = m_ImageURI;
    string quadKey = HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToQuadKey(pixelX, pixelY, levelOfDetail);

    ReplaceTagInString(imageURI, SUBDOMAIN_TAG, m_ImageURISubdomains[quadKey[quadKey.size()-1] - '0']);
    ReplaceTagInString(imageURI, QUADKEY_TAG, WString(quadKey.c_str(),false));

    return imageURI;
    }

//-----------------------------------------------------------------------------
// class VEAuthenticationError
//
// Authentication error type specific to Virtual Earth files.
//-----------------------------------------------------------------------------
class VEAuthenticationError : public HFCAuthenticationError
    {
public:
    explicit VEAuthenticationError(const HRFException& pi_rException)
        :   m_RelatedException(pi_rException),
            HFCAuthenticationError(static_cast<ExceptionID>(pi_rException.GetID()))
        {
        }

private:
    WString _ToString() const
        {
        return m_RelatedException.GetExceptionMessage();
        }

    void _Throw() const
        {
        throw m_RelatedException;
        }

    HRFException m_RelatedException;
    };


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
                                  LONG_MAX,             // MaxSizeInBytes
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
WString HRFVirtualEarthCreator::GetLabel() const
    {
    return HFCResourceLoader::GetInstance()->GetString(IDS_FILEFORMAT_BingMaps);
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFVirtualEarthCreator::GetSchemes() const
    {
    return WString(HFCURLHTTP::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFVirtualEarthCreator::GetExtensions() const
    {
    return WString(L"");
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
    return HRFVirtualEarthFile::IsURLVirtualEarth(pi_rpURL);
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
//                      HRFVirtualEarthConnection                              //
//-----------------------------------------------------------------------------//
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFVirtualEarthConnection::HRFVirtualEarthConnection(const WString& pi_rServer,
                                                     const WString& pi_rUsr,
                                                     const WString& pi_rPwd)
    : HFCHTTPConnection(pi_rServer, pi_rUsr, pi_rPwd)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRFVirtualEarthConnection::HRFVirtualEarthConnection(const HRFVirtualEarthConnection& pi_rObj)
    : HFCHTTPConnection(const_cast<HRFVirtualEarthConnection&>(pi_rObj).GetServer(),
                        const_cast<HRFVirtualEarthConnection&>(pi_rObj).GetUserName(),
                        const_cast<HRFVirtualEarthConnection&>(pi_rObj).GetPassword())
    {
    SetTimeOut(pi_rObj.GetTimeOut());
    SetExtention(pi_rObj.GetExtention());
    SetSearchBase(pi_rObj.GetSearchBase());
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFVirtualEarthConnection::~HRFVirtualEarthConnection()
    {
    }

//-----------------------------------------------------------------------------
// New request
//-----------------------------------------------------------------------------
void HRFVirtualEarthConnection::NewRequest()
    {
    m_RequestEnded = false;
    }

//-----------------------------------------------------------------------------
// End current request
//-----------------------------------------------------------------------------
bool HRFVirtualEarthConnection::RequestEnded() const
    {
    bool Result = false;

    if (m_RequestEnded)
        {
        HFCMonitor BufferMonitor(m_BufferKey);
        Result = (m_Buffer.GetDataSize() == 0);
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Protected
// Verify if the request has ended
//-----------------------------------------------------------------------------
void HRFVirtualEarthConnection::RequestHasEnded(bool pi_Success)
    {
    m_RequestEnded = true;
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
        throw HFCException(HFC_LOGIN_INFORMATION_NOT_AVAILABLE_EXCEPTION);

    if(!pi_rURL->IsCompatibleWith(HFCURLHTTPBase::CLASS_ID))
        throw HFCFileException(HFC_CANNOT_OPEN_FILE_EXCEPTION, pi_rURL->GetURL());
        
    HFCInternetAuthentication VEAuthentication(pi_rURL->GetURL());

    bool  HasValidCredential = false;
    bool MaxRetryCountReached = false;
    do
        {
        if (!pCallback->GetAuthentication(&VEAuthentication))
            {
            if (pCallback->IsCancelled())
                throw HRFException(HRF_AUTHENTICATION_CANCELLED_EXCEPTION, GetURL()->GetURL());
            else
                throw HRFException(HRF_AUTHENTICATION_INVALID_LOGIN_EXCEPTION, GetURL()->GetURL());
            }
            
        // Authenticate with Terra pixel server. Response in plain text : "VALID" or "INVALID"
        WString serverURL(TERRAPIXEL_SERVER);
        ReplaceTagInString(serverURL, TERRAPIXELKEY_TAG, VEAuthentication.GetPassword());

        HFCPtr<HFCBuffer> pResponse = SendAndReceiveRequest(serverURL);
        if(pResponse != NULL && pResponse->GetDataSize() != 0)
        {
            HArrayAutoPtr<char> ResponseString(new char[pResponse->GetDataSize() + 1]);
            memcpy(ResponseString.get(), pResponse->GetData(), pResponse->GetDataSize());
            ResponseString[pResponse->GetDataSize()] = '\0';

            if(BeStringUtilities::Stricmp(TERRAPIXELKEY_VALID, ResponseString) == 0)
                {
                HasValidCredential = true;
                break;
            }
        }

        // TODO: Generate a more explicit exception
        HFCPtr<HFCAuthenticationError> pAuthError(new VEAuthenticationError(HRFException(HRF_AUTHENTICATION_INVALID_LOGIN_EXCEPTION, GetURL()->GetURL())));
        VEAuthentication.PushLastError(pAuthError);

        VEAuthentication.IncrementRetryCount();    
         
        MaxRetryCountReached = VEAuthentication.GetRetryCount() > pCallback->RetryCount(HFCInternetAuthentication::CLASS_ID);
        }
    while (!MaxRetryCountReached);

    if (MaxRetryCountReached)
        throw HRFException(HRF_AUTHENTICATION_MAX_RETRY_COUNT_REACHED_EXCEPTION, GetURL()->GetURL());

    if(!HasValidCredential)
        throw HRFException(HRF_AUTHENTICATION_INVALID_LOGIN_EXCEPTION, GetURL()->GetURL());

    // Will contact virtual earth server and request image URI. Will throw appropriate exception if an error occurs.
    QueryImageURI();

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
    if (m_ppBlocksReadersThread != 0)
        {
        // Stop all threads and then wait.
        for (uint32_t i = 0; i < NB_BLOCK_READER_THREAD; i++)
            {
            if (m_ppBlocksReadersThread[i] != 0)
                m_ppBlocksReadersThread[i]->StopThread();
        }

        for (uint32_t i = 0; i < NB_BLOCK_READER_THREAD; i++)
            {
            if (m_ppBlocksReadersThread[i] != 0)
                m_ppBlocksReadersThread[i]->WaitUntilSignaled();
            }
        }
    }

//-----------------------------------------------------------------------------
// Compose a Virtual Earth URL
//-----------------------------------------------------------------------------
WString HRFVirtualEarthFile::ComposeVirtualEarthURL(MapStyle const& pi_Style)
    {
    switch(pi_Style)
        {
        case MAPSTYLE_Road:
            return PSEUDO_VE_SERVER_URL + PSEUDO_VE_ROAD;
        case MAPSTYLE_Aerial:
            return PSEUDO_VE_SERVER_URL + PSEUDO_VE_AERIAL;
        case MAPSTYLE_AerialWihtLabels:
            return PSEUDO_VE_SERVER_URL + PSEUDO_VE_AERIALWITHLABEL;
        default:
            HASSERT(!"Invalid MapStyle : HRFVirtualEarthFile::CreateVirtualEarthURL");
            break;
        }

    return L"";
    }

//-----------------------------------------------------------------------------
// Find the map style from an URL
//-----------------------------------------------------------------------------
HRFVirtualEarthFile::MapStyle HRFVirtualEarthFile::FindMapStyleFromURL(WString const& pi_pURL)
    {
    HRFVirtualEarthFile::MapStyle MapStyle = (HRFVirtualEarthFile::MapStyle)-1;

    if (CaseInsensitiveStringTools().Find(pi_pURL, PSEUDO_VE_ROAD) != WString::npos)
        {
        MapStyle = MAPSTYLE_Road;
        }
    else if (CaseInsensitiveStringTools().Find(pi_pURL, PSEUDO_VE_AERIALWITHLABEL) != WString::npos)
        {
        MapStyle = MAPSTYLE_AerialWihtLabels;
        }
    else if (CaseInsensitiveStringTools().Find(pi_pURL, PSEUDO_VE_AERIAL) != WString::npos)
        {
        MapStyle = MAPSTYLE_Aerial;
        }

    return MapStyle;
}

//-----------------------------------------------------------------------------
// Find the map style from an URL and return the label needed for the http request.
//-----------------------------------------------------------------------------
bool HRFVirtualEarthFile::DetectImagerySetFromURL(WStringR po_imagerySetLabel, HFCURL const& pi_URL) const
{
    HPRECONDITION(IsURLVirtualEarth(&pi_URL));

    const WString& urlSpec = pi_URL.GetSchemeSpecificPart();

    if (CaseInsensitiveStringTools().Find(urlSpec, PSEUDO_VE_ROAD) != WString::npos)
    {
        po_imagerySetLabel = BING_IMAGERYSET_ROAD;
        return true;
    }
    else if (CaseInsensitiveStringTools().Find(urlSpec, PSEUDO_VE_AERIALWITHLABEL) != WString::npos)
    {
        po_imagerySetLabel = BING_IMAGERYSET_AERIALWITHLABEL;        
        return true;
    }
    else if (CaseInsensitiveStringTools().Find(urlSpec, PSEUDO_VE_AERIAL) != WString::npos)
    {
        po_imagerySetLabel = BING_IMAGERYSET_AERIAL;        
        return true;
    }

    // Not found.
    po_imagerySetLabel.clear();
    return false;
    }

//-----------------------------------------------------------------------------
// Is a Virtual Earth URL
//-----------------------------------------------------------------------------
bool HRFVirtualEarthFile::IsURLVirtualEarth(const HFCURL* pi_pURL)
    {
    if(!pi_pURL->IsCompatibleWith(HFCURLHTTPBase::CLASS_ID) || 
       pi_pURL->GetSchemeType() != HFCURLHTTP::s_SchemeName())
        return false;

    const HFCURLHTTPBase* pHTTPUrl = (const HFCURLHTTPBase*)pi_pURL;

    return CaseInsensitiveStringTools().Find(pHTTPUrl->GetHost(), PSEUDO_VE_SERVER) != WString::npos;            
    }

//-----------------------------------------------------------------------------
// Create a resolution editor
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFVirtualEarthFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                                 unsigned short pi_Resolution,
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
    HPRECONDITION (IsURLVirtualEarth(GetURL().GetPtr()));

    // Pixel Type
    HFCPtr<HRPPixelType> PixelType = new HRPPixelTypeV24R8G8B8();

    // Transfo model
    double Scale = HRFVirtualEarthEditor::VirtualEarthTileSystem::GroundResolution(0.0, VE_MAP_RESOLUTION);
    double offsetLatitude;
    double offsetLongitude;
    HRFVirtualEarthEditor::VirtualEarthTileSystem::PixelXYToLatLong(0, 0, VE_MAP_RESOLUTION, &offsetLatitude, &offsetLongitude);
    if (offsetLongitude < -179.9999)
        offsetLongitude = -179.9999;
    if (offsetLongitude > 179.9999)
        offsetLongitude = 179.9999;

    // Geocoding and Reference
    HFCPtr<HGF2DTransfoModel> pTransfoModel;
    IRasterBaseGcsPtr baseGCS;

    if (GCSServices->_IsAvailable())
        {
        try
            {
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
            baseGCS = HRFGeoCoordinateProvider::CreateRasterGcsFromFromWKT(NULL, NULL, IRasterGeoCoordinateServices::WktFlavorOGC, WKTString.c_str());
            double cartesianPoint[3];
            double geoPoint[3];
            geoPoint[0] = offsetLongitude;
            geoPoint[1] = offsetLatitude;
            geoPoint[2] = 0.0;
            baseGCS->GetCartesianFromLatLong (cartesianPoint, geoPoint);

            double OffsetX = cartesianPoint[0];
            double OffsetY = cartesianPoint[1];

            pTransfoModel = new HGF2DStretch(HGF2DDisplacement(OffsetX, OffsetY),
                                             Scale, 
                                             Scale);
            }
        catch (...)
            {
            }
        }
    else
        {
        pTransfoModel = new HGF2DStretch(HGF2DDisplacement(0.0, 0.0),
                                         Scale, 
                                         Scale);
        }

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

    for (unsigned short Resolution = 1; Resolution < VE_MAP_RESOLUTION; ++Resolution)
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

    pPage->SetGeocoding(baseGCS);


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
        unsigned short Resolution = (unsigned short)HRFRasterFile::s_TileDescriptor.GetLevel(*Itr);

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

    HFCMonitor Monitor(m_RequestKey);
    m_RequestList.clear();
    }

//-----------------------------------------------------------------------------
// Private
// Start the threads that are going to be used to read tiles from the server.
//-----------------------------------------------------------------------------
void HRFVirtualEarthFile::StartReadingThreads()
    {
    HPRECONDITION(m_ppBlocksReadersThread == 0);

    m_ppBlocksReadersThread = new HAutoPtr<HRFVirtualEarthTileReaderThread>[NB_BLOCK_READER_THREAD];

    for (uint32_t i = 0; i < NB_BLOCK_READER_THREAD; i++)
        {
        ostringstream ThreadName;
        ThreadName << "HRFVirtualEarthBlockReader " << i + 1;

        m_ppBlocksReadersThread[i] = new HRFVirtualEarthTileReaderThread(ThreadName.str(),
                                                                         HFCPtr<HRFVirtualEarthFile>(this));

        if (!m_ppBlocksReadersThread[i]->StartThread())
            {
            m_ppBlocksReadersThread[i] = 0;
            }
        }
    }

//-----------------------------------------------------------------------------
// Get the world identificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFVirtualEarthFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
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