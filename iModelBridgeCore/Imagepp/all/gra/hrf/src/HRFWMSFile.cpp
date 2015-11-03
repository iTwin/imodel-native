//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFWMSFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFWMSFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFWMSFile.h>
#include <Imagepp/all/h/HRFOGCServiceEditor.h>

#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HGF2DStretch.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HMDLayersWMS.h>
#include <Imagepp/all/h/HMDLayerInfoWMS.h>
#include <Imagepp/all/h/HMDVolatileLayers.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <BeXml/BeXml.h>




//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------

static const  WString s_Device(L"WMS connection");
static        uint32_t s_ConnectionTimeOut = 60000; // default : 60 sec

#define NB_BLOCK_READER_THREAD  4
HFC_IMPLEMENT_SINGLETON(HRFWMSCreator)

// 16384 = 2^14
#define WMS_MIN_SIZE_X (16384)

//-----------------------------------------------------------------------------
//public
//HRFWMSCapabilities constructor
//-----------------------------------------------------------------------------
HRFWMSCapabilities::HRFWMSCapabilities()
    : HRFOGCServiceCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFOGCServiceCodecCapabilities()));
    }

//-----------------------------------------------------------------------------
// public
// Constructor
// This is the creator to instantiate WMS format
//-----------------------------------------------------------------------------
HRFWMSCreator::HRFWMSCreator()
    : HRFRasterFileCreator(HRFWMSFile::CLASS_ID)
    {
    // WMS capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// public
// GetLabel
// Identification information
//-----------------------------------------------------------------------------
WString HRFWMSCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_WMS()); // WMS File Format
    }

//-----------------------------------------------------------------------------
// public
// GetSchemes
// Identification information
//-----------------------------------------------------------------------------
WString HRFWMSCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName() + L";" + HFCURLMemFile::s_SchemeName();
    }

//-----------------------------------------------------------------------------
// public
// GetExtensions
// Identification information
//
// Note : IsKindOfFile use GetExtensions. Adapt the method if the extensions
//        change...
//-----------------------------------------------------------------------------
WString HRFWMSCreator::GetExtensions() const
    {
    return WString(L"*.xwms");
    }

//-----------------------------------------------------------------------------
// public
// Create
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFWMSCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFWMSFile(pi_rpURL,
                                                 pi_AccessMode,
                                                 pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// public
// IsKindOfFile
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFWMSCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        HFCPtr<HFCURLFile>& rpURL((HFCPtr<HFCURLFile>&)pi_rpURL);
        // at least, the file must be have the right extension...
        if (BeStringUtilities::Wcsicmp(rpURL->GetExtension().c_str(), L"xwms") == 0)
            {
            WString XMLFileName;
            XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
            XMLFileName += L"\\";
            XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

            (const_cast<HRFWMSCreator*>(this))->SharingControlCreate(pi_rpURL);
            HFCLockMonitor SisterFileLock(GetLockManager());

            // *** At some point in the development of WMS we were creating bad formated XML files.
            // The SERVERCAPABILITIES tag was closed with "</SERVERCAPABILTIES>" notice the missing 'I'.
            // The new xml lib reject the file because of that.
            // We never released this version of WMS to the public but some files have been created internally.
            // MS Beta1 8.11.00.43 ++ possibly 8.11.00.50
            // MS Beta2 8.11.01.36 ++
            // MS Release 8.11.05.xx
            BeXmlStatus xmlStatus;
            BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());
            if (!pXmlDom.IsNull() && BEXML_Success == xmlStatus)
                {            
                // Read data in XML file (strings)
                BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
                if (NULL != pMainNode && BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyWMSFile") == 0)
                    bResult = true;
                }

            SisterFileLock.ReleaseKey();

            HASSERT(!(const_cast<HRFWMSCreator*>(this))->m_pSharingControl->IsLocked());
            (const_cast<HRFWMSCreator*>(this))->m_pSharingControl = 0;
            }
        }
    else if (pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID))
        {
        HFCPtr<HFCURLMemFile>& rpURL((HFCPtr<HFCURLMemFile>&)pi_rpURL);
        if (BeStringUtilities::Wcsicmp(rpURL->GetExtension().c_str(), L"xwms") == 0)
            {
            HFCPtr<HFCBuffer> pBuffer(((HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetBuffer());

            // *** At some point in the development of WMS we were creating bad formated XML files.
            // The SERVERCAPABILITIES tag was closed with "</SERVERCAPABILTIES>" notice the missing 'I'.
            // The new xml lib reject the file because of that.
            // We never released this version of WMS to the public but some files have been created internally.
            // MS Beta1 8.11.00.43 ++ possibly 8.11.00.50
            // MS Beta2 8.11.01.36 ++
            // MS Release 8.11.05.xx
            BeXmlStatus xmlStatus;
            BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, pBuffer->GetData(), pBuffer->GetDataSize());
            if (!pXmlDom.IsNull() && BEXML_Success == xmlStatus)
                {            
                // Read data in XML file (strings)
                BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
                if (NULL != pMainNode && BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyWMSFile") == 0)
                    bResult = true;
                }
            }
        }

    return bResult;
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
// Create or get the singleton capabilities of BMP file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWMSCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFWMSCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// class HRFWMSFile
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// private
// constructor
//-----------------------------------------------------------------------------
HRFWMSFile::HRFWMSFile(const HFCPtr<HFCURL>& pi_rpURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFOGCService(pi_rpURL,
                    WMS, //OGC service type
                    pi_AccessMode,
                    pi_Offset),
    m_NeedAuthentification(true) // TR #259891, by default, always send a request to get the authentification
    {
    HPRECONDITION(pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID)||pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID));


    //Read-Only format
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rpURL->GetURL());
        }

    SharingControlCreate();

    BeXmlDomPtr pXmlDom;
    BeXmlStatus xmlStatus=BEXML_ParseError;
    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        WString XMLFileName;
        XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
        XMLFileName += L"\\";
        XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

        // Open XML file
        pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());
        }
    else if (pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID))
        {
        HFCPtr<HFCBuffer> pBuffer(((HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetBuffer());

        pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, pBuffer->GetData(), pBuffer->GetDataSize());
        }
    else
        {
        HASSERT(0);
        }

    // *** At some point in the development of WMS we were creating bad formated XML files.
    // The SERVERCAPABILITIES tag was closed with "</SERVERCAPABILTIES>" notice the missing 'I'.
    // The new xml lib reject the file because of that.
    // We never released this version of WMS to the public but some files have been created internally.
    // MS Beta1 8.11.00.43 ++ possibly 8.11.00.50
    // MS Beta2 8.11.01.36 ++
    // MS Release 8.11.05.xx
    if(!pXmlDom.IsValid() || BEXML_Success != xmlStatus)
        throw HFCCorruptedFileException(pi_rpURL->GetURL());

    BeXmlNodeP pMainNode = pXmlDom->GetRootElement();

    //The IsKindOf function checks this node, so there is a big problem if we get here
    //and the isn't found.
    if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyWMSFile") != 0)
        throw HFCCorruptedFileException(pi_rpURL->GetURL());
   
    // default values
    m_MinImageSize.m_Width  = 16;            // see capabilities
    m_MinImageSize.m_Height = 16;            // see capabilities
    m_MaxImageSize.m_Width  = UINT64_MAX;   // see capabilities
    m_MaxImageSize.m_Height = UINT64_MAX;   // see capabilities

    // server capabilities
    m_MaxBitmapSize.m_Width  = 1024;
    m_MaxBitmapSize.m_Height = 1024;

    m_Width = 0;
    m_Height = 0;

    BeXmlNodeP pChildNode;
    if ((pChildNode = pMainNode->SelectSingleNode("VERSION")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"VERSION");

    WString versionString;
    pChildNode->GetContent(versionString);

    if (wcscmp(versionString.c_str(), L"1.0") == 0)
        ReadWMS_1_0(pMainNode);
    else if (wcscmp(versionString.c_str(), L"1.1") == 0)
        ReadWMS_1_1(pMainNode);
    else if (wcscmp(versionString.c_str(), L"1.2") == 0)
        ReadWMS_1_2(pMainNode, versionString);
    else if (wcscmp(versionString.c_str(), L"1.3") == 0)
        ReadWMS_1_2(pMainNode, versionString);
    else
        throw HRFUnsupportedxWMSVersionException(pi_rpURL->GetURL());

    // the XML document is not longer use
    pChildNode = pMainNode = NULL;
    pXmlDom = NULL;

    // tr #241463
    // keep the file pointer while the HRF is alive
    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        m_pFile = HFCBinStream::Instanciate(pi_rpURL, pi_AccessMode);

    HFCPtr<HFCURL> pURL = HFCURL::Instanciate(m_ServerURL);
    WString URL;
    WString User;
    WString Password;

    if (pURL == 0)
        {
        // create an HTTP URL
        URL = L"http://" + m_ServerURL;
        pURL = new HFCURLHTTP(URL);

        if (!((HFCPtr<HFCURLHTTP>&)pURL)->GetSearchPart().empty())
            m_URLSearchPart = ((HFCPtr<HFCURLHTTP>&)pURL)->GetSearchPart();

        WString Host;
        WString Scheme;
        WString Port;
        WString Path;

        HFCURLCommonInternet::SplitPath(pURL->GetURL(),
                                        &Scheme,
                                        &Host,
                                        &Port,
                                        &User,
                                        &Password,
                                        &Path);

        // is we have an user or password, change the http url for an https url
        if (!User.empty() || !Password.empty())
            {
            URL = L"https://" + Host;
            if (!Port.empty())
                URL += L":" + Port;

            if (!Path.empty())
                URL += L"/" + Path;
            }
        }
    else if (!pURL->IsCompatibleWith(HFCURLHTTP::CLASS_ID) && !pURL->IsCompatibleWith(HFCURLHTTPS::CLASS_ID))
        {
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);
        }
    else
        {   // TR 269330
        if (!((HFCPtr<HFCURLHTTPBase>&)pURL)->GetSearchPart().empty())
            m_URLSearchPart = ((HFCPtr<HFCURLHTTPBase>&)pURL)->GetSearchPart();

        WString Host;
        WString Scheme;
        WString Port;
        WString Path;

        // extract user ans password
        HFCURLCommonInternet::SplitPath(pURL->GetURL(),
                                        &Scheme,
                                        &Host,
                                        &Port,
                                        &User,
                                        &Password,
                                        &Path);

        URL = Scheme + L"://" + Host;

        if (!Port.empty())
            URL += L":" + Port;

        if (!Path.empty())
            URL += L"/" + Path;
        }

    // first, try an HTTP URL
    m_pConnection = new HRFOGCServiceConnection(URL, User, Password);

    //Convert back the URL's search part as it was in the XML file.
    Utf8String utf8Str;
    BeStringUtilities::WCharToUtf8(utf8Str,m_URLSearchPart.c_str());

    string SearchBase = string(utf8Str.c_str());

    m_pConnection->SetSearchBase(SearchBase);

    HASSERT(m_pConnection != 0);
    m_pConnection->SetTimeOut(m_ConnectionTimeOut);

    // Connect to the server
    if (!m_pConnection->Connect(WString(L""), WString(L""), m_ConnectionTimeOut))
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);
    else
        {
        if (m_NeedAuthentification)
            {
            AUTHENTICATION_STATUS Status = AuthorizeConnection();
            m_pConnection->Disconnect();    // don't keep the connection

            if (AUTH_PERMISSION_DENIED == Status)
                throw HRFAuthenticationInvalidLoginException(GetURL()->GetURL());
            else if (AUTH_USER_CANCELLED == Status)
                throw HRFAuthenticationCancelledException(GetURL()->GetURL());
            else if (AUTH_MAX_RETRY_COUNT_REACHED == Status)
                throw HRFAuthenticationMaxRetryCountReachedException(GetURL()->GetURL());
            }
        else
            m_pConnection->Disconnect();    // don't keep the connection
        }

    CreateDescriptors(m_Width, m_Height);

    // create the transfo model to map pixel into the bbox
    // the model set into the page descriptor contains the units ratio. This model
    // can't be use by the editor to map the tile into the bbox.
    double PixelScaleX = abs(m_BBoxMaxX - m_BBoxMinX) / (double)m_Width;
    double PixelScaleY = abs(m_BBoxMaxY - m_BBoxMinY) / (double)m_Height;

    m_pModel = new HGF2DStretch(HGF2DDisplacement (m_BBoxMinX, m_BBoxMinY + (m_Height * PixelScaleY)),
                                PixelScaleX,
                                -PixelScaleY);

    m_Request = m_BaseRequest;

    // m_Layers is not set in 1.0 and 1.1 versions
    if (!m_Layers.empty())
        m_Request += "&LAYERS=" + m_Layers;

    m_Request += "&STYLES=";
    if (!m_Styles.empty())
        m_Request += m_Styles;
    }



//-----------------------------------------------------------------------------
// public
// Destructor
// Destroy WMS file object
//-----------------------------------------------------------------------------
HRFWMSFile::~HRFWMSFile()
    {
    if (m_pConnection->IsConnected())
        m_pConnection->Disconnect();
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWMSFile::GetCapabilities () const
    {
    return (HRFWMSCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Public
// SetContext
// Set the context
//-----------------------------------------------------------------------------
void HRFWMSFile::SetContext(uint32_t                 pi_Page,
                            const HFCPtr<HMDContext>& pi_rContext)
    {
    HPRECONDITION(pi_Page == 0);

    HRFRasterFile::SetContext(pi_Page, pi_rContext);

    HFCPtr<HMDMetaDataContainer> pDataContainer;

    pDataContainer = pi_rContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO);

    if (pDataContainer != 0)
        {
        HASSERT(pDataContainer->IsCompatibleWith(HMDVolatileLayers::CLASS_ID));

        HFCPtr<HMDVolatileLayers> pVolatileLayers((HFCPtr<HMDVolatileLayers>&)pDataContainer);

        WString Layers;
        WString Styles;
        const HMDLayerInfoWMS* pLayer;
        for (unsigned short Index = 0; Index < pVolatileLayers->GetNbVolatileLayers(); ++Index)
            {
            if (pVolatileLayers->GetVolatileLayerInfo(Index)->GetVisibleState())
                {
                HPRECONDITION(pVolatileLayers->GetLayerInfo(Index)->IsCompatibleWith(HMDLayerInfoWMS::CLASS_ID));

                pLayer = (const HMDLayerInfoWMS*)pVolatileLayers->GetLayerInfo(Index);
                if (!Layers.empty())
                    {
                    Layers += L",";
                    Styles += L",";
                    }

                Layers += pLayer->GetLayerName();
                Styles += pLayer->GetStyleName();
                }
            }
        m_Request = m_BaseRequest;
        m_Request += "&LAYERS=";

        size_t  destinationBuffSize = Layers.GetMaxLocaleCharBytes();
        char*  LayersMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(LayersMBS,Layers.c_str(),destinationBuffSize);

        m_Request += string(LayersMBS);
        m_Request += "&STYLES=";

        destinationBuffSize = Styles.GetMaxLocaleCharBytes();
        char*  StylesMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(StylesMBS,Styles.c_str(),destinationBuffSize);

        m_Request += string(StylesMBS);

        // Notify each editor that the context changed
        // find the resolution editor into the ResolutionEditorRegistry
        ResolutionEditorRegistry::const_iterator ResItr(m_ResolutionEditorRegistry.begin());
        while (ResItr != m_ResolutionEditorRegistry.end())
            {
            ((HRFOGCServiceEditor*)(*ResItr))->ContextChanged();
            ResItr++;
            }

        }
    }

//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// protected
// CreateDescriptors
// Create WMS File Descriptors
//-----------------------------------------------------------------------------
void HRFWMSFile::CreateDescriptors(uint64_t pi_Width,
                                   uint64_t pi_Height)
    {
    HFCPtr<HRPPixelType> pPixelType;

    pPixelType = new HRPPixelTypeV32R8G8B8A8();

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor =
        new HRFResolutionDescriptor(GetAccessMode(),
                                    GetCapabilities(),
                                    1.0,
                                    1.0,
                                    (const HFCPtr<HRPPixelType>&)pPixelType,
                                    new HCDCodecIdentity(),
                                    HRFBlockAccess::RANDOM,                         // RBlockAccess,
                                    HRFBlockAccess::RANDOM,                         // WBlockAccess,
                                    HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                    HRFInterleaveType::PIXEL,                       // InterleaveType
                                    false,
                                    pi_Width,
                                    pi_Height,
                                    256,
                                    256);


    // ProjectedCSType
    // extract the EPSG code from the request
    m_GTModelType = TIFFGeo_ModelTypeGeographic;

    string::size_type Pos;
    string Request = m_BaseRequest;
    CaseInsensitiveStringToolsA().ToLower(Request);

    Pos = Request.find("srs=");

    if (Pos == string::npos)
        Pos = Request.find("crs=");

    if (Pos == string::npos)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"srs or crs");

    // skip "srs="
    Pos += 4;
    string::size_type Pos2 = Request.find("&", Pos);
    int32_t EPSGCode=0;
    string SRS = Request.substr(Pos, (Pos2 == string::npos ? string::npos : Pos2 - Pos));

    bool invalidEPSG = false;
    if (strncmp(SRS.c_str(), "epsg:", 5) == 0)
        {
        sscanf(SRS.c_str(), "epsg:%d", &EPSGCode);
        }
    else if (strncmp(SRS.c_str(), "crs:", 4) == 0)
    	{
        if (strncmp(SRS.c_str(), "crs:83", 6) == 0)
        	{
            EPSGCode = 4269;
            m_GTModelType = TIFFGeo_ModelTypeGeographic;
        }
        else if (strncmp(SRS.c_str(), "crs:27", 6) == 0)
        {
            EPSGCode = 4267;
            m_GTModelType = TIFFGeo_ModelTypeGeographic;
        }
        else if (strncmp(SRS.c_str(), "crs:84", 6) == 0)
        {
            EPSGCode = 4326;
            m_GTModelType = TIFFGeo_ModelTypeGeographic;
        	}
    	}
    else
        {
        EPSGCode = TIFFGeo_Undefined;

        // Special case...
        if (BeStringUtilities::Stricmp(SRS.c_str(), "none") == 0)
            m_GTModelType = TIFFGeo_ModelTypeProjected;
        else
            invalidEPSG = true;
        }

    // Build keyname from EPSG code
    WString keyName;
    BeStringUtilities::CurrentLocaleCharToWChar(keyName, SRS.c_str());

    GeoCoordinates::BaseGCSPtr pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS(keyName.c_str());
    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys;


    // In all cases including if the assumption that it is a MS GCS keyname has been discared then we
    // create a set of geo keys...
    if (!invalidEPSG)
        {
        pGeoTiffKeys = new HCPGeoTiffKeys();

        // GTModelType
        pGeoTiffKeys->AddKey(GTModelType, (uint32_t)m_GTModelType);

        // GTRasterType
        pGeoTiffKeys->AddKey(GTRasterType, (uint32_t)TIFFGeo_RasterPixelIsArea);

        if (EPSGCode != TIFFGeo_Undefined)
            {
            if ((EPSGCode >= 4000) && (EPSGCode < 5000))
                pGeoTiffKeys->AddKey(GeographicType, (uint32_t)EPSGCode);
            else
                {
                if (EPSGCode <= USHRT_MAX)
                    {
                    pGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)EPSGCode);
                    }
                else
                    {
                    pGeoTiffKeys->AddKey(ProjectedCSTypeLong, (uint32_t)EPSGCode);
                    }
                }
            }
        else
        	{
            // GeogLinearUnits
            pGeoTiffKeys->AddKey(GeogLinearUnits, (uint32_t)TIFFGeo_Linear_Meter);
        	}
        }


    HFCPtr<HRFPageDescriptor> pPage = new HRFPageDescriptor (GetAccessMode(),
                                                             GetCapabilities(),
                                                             pResolutionDescriptor,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             CreateTransfoModel(pBaseGCS.get(), pi_Width, pi_Height),
                                                             0,
                                                             0,
                                                             0,
                                                             false, // resizable
                                                             true,  // unlimited
                                                             m_MinImageSize.m_Width,
                                                             m_MinImageSize.m_Height,
                                                             m_MaxImageSize.m_Width,
                                                             m_MaxImageSize.m_Height);


    if (pBaseGCS == NULL)
        {
        pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys));
        }
    else
        {
        pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pBaseGCS.get()));
        }

    HFCPtr<HMDMetaDataContainerList> pMDContainers(new HMDMetaDataContainerList());

    //HMD_LAYER_INFO
    if ((m_pLayers != 0))
        {
        pMDContainers->SetMetaDataContainer((HFCPtr<HMDMetaDataContainer>&)m_pLayers);
        }

    pPage->SetListOfMetaDataContainer(pMDContainers);

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// protected
// AuthorizeConnection
//
// Send a dummy request to validate if the server need a user/password
//-----------------------------------------------------------------------------
HRFOGCService::AUTHENTICATION_STATUS HRFWMSFile::AuthorizeConnection()
    {
    // We first call the ancester method
    AUTHENTICATION_STATUS Status = HRFOGCService::AuthorizeConnection();

    if (Status == AUTH_SUCCESS)
    {
        ostringstream RequestEditor;
        RequestEditor.precision(16);
        RequestEditor << m_BaseRequest << "&LAYERS=" << m_Layers << "&STYLES=" << m_Styles ;
        RequestEditor << "&width=" << 10 << "&height=" << 10;
        RequestEditor << "&BBOX=";
        RequestEditor << m_BBoxMinX << ",";
        RequestEditor << m_BBoxMinY << ",";
        RequestEditor << m_BBoxMaxX << ",";
        RequestEditor << m_BBoxMaxY << "\r\n";

        Status = HRFOGCService::AuthorizeConnectionSpecificRequest(RequestEditor.str());
        }

    return Status;
    }
//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// private
// ReadWMS_1_0
//
// Read the version 1.0 of WMS file
//-----------------------------------------------------------------------------
void HRFWMSFile::ReadWMS_1_0(BeXmlNodeP pi_pBentleyXMLFileNode)
    {
    HPRECONDITION(pi_pBentleyXMLFileNode != 0);

    BeXmlNodeP pChildNode;
    BeXmlNodeP pSubChildNode;

    // tag URL (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("URL")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"URL");

    pChildNode->GetContent(m_ServerURL);

    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("REQUEST")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"REQUEST");

    WString content;
    pChildNode->GetContent(content);

    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    AString requestA;
    BeStringUtilities::WCharToLocaleChar (requestA, codePage, content.c_str());

    m_BaseRequest = requestA.c_str();

    string Request(m_BaseRequest);
    CaseInsensitiveStringToolsA().ToLower(Request);

    string::size_type Pos;
    if ((Pos = Request.find("srs=")) == string::npos && (Pos = Request.find("crs=")) == string::npos)
        {
        if (Request.find("crs=") == string::npos)
            {
            throw HRFMissingParameterException(GetURL()->GetURL(),
                                            L"crs");
            }

        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"srs");
        }

    // skip "srs=" or "crs="
    Pos += 4;
    string::size_type Pos2 = Request.find("&", Pos);
    m_CRS = Request.substr(Pos, (Pos2 == string::npos ? string::npos : Pos2 - Pos));


    // format
    if ((Pos = Request.find("format=")) == string::npos)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"format");

    // skip "format="
    Pos += 7;
    Pos2 = Request.find("&", Pos);
    m_Format = Request.substr(Pos, (Pos2 == string::npos ? string::npos : Pos2 - Pos));

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "image/jpeg") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/png") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/bmp") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        L"format");

    // tag BBOX (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("BBOX")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"BBOX");

    bvector<double> BBoxValues;
    if(BEXML_Success != pi_pBentleyXMLFileNode->GetContentDoubleValues(BBoxValues) || BBoxValues.size() != 4)
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"BBOX");

    m_BBoxMinX = BBoxValues[0];
    m_BBoxMinY = BBoxValues[1];
    m_BBoxMaxX = BBoxValues[2];
    m_BBoxMaxY = BBoxValues[3];

    if (HDOUBLE_EQUAL_EPSILON(m_BBoxMaxX - m_BBoxMinX, 0.0) || HDOUBLE_EQUAL_EPSILON(m_BBoxMaxY - m_BBoxMinY, 0.0))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"BBOX invalid");

    // tag IMAGESIZE (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("IMAGESIZE")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"IMAGESIZE");

    // tag IMAGESIZE.WIDTH (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("WIDTH")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"IMAGESIZE.WIDTH");

    if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_Width))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"IMAGESIZE.WIDTH");

    // tag IMAGESIZE.HEIGHT (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("HEIGHT")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"IMAGESIZE.HEIGHT");

    if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_Height))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"IMAGESIZE.HEIGHT");

    // tag SERVERCAPABILTIES (optional)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("SERVERCAPABILITIES")) != NULL)
        {
        // tag SERVERCAPABILITIES.MAXWIDTH (mandatory)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXWIDTH")) == NULL)
            throw HRFMissingParameterException(GetURL()->GetURL(),
                                            L"SERVERCAPABILITIES.MAXWIDTH");

        if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Width))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            L"SERVERCAPABILITIES.MAXWIDTH");

        // tag SERVERCAPABILITIES.MAXHEIGHT (mandatory)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXHEIGHT")) == NULL)
            throw HRFMissingParameterException(GetURL()->GetURL(),
                                            L"SERVERCAPABILITIES.MAXHEIGHT");

        if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Height))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            L"SERVERCAPABILITIES.MAXHEIGHT");
        }

    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != NULL)
        {
        if (BEXML_Success != pChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            L"CONNECTIONTIMEOUT");

        m_ConnectionTimeOut *= 1000; // the value is in second, convert it in millisecond
        }
    }

//-----------------------------------------------------------------------------
// private
// ReadWMS_1_1
//
// Read the version 1.1 of WMS file
//-----------------------------------------------------------------------------
void HRFWMSFile::ReadWMS_1_1(BeXmlNodeP pi_pBentleyWMSFileNode)
    {
    HPRECONDITION(pi_pBentleyWMSFileNode != 0);

    BeXmlNodeP pChildNode;
    BeXmlNodeP pSubChildNode;
    
    // tag URL
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("URL")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"URL");

    pChildNode->GetContent(m_ServerURL);

    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("REQUEST")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"REQUEST");

    m_BBoxMinX = 0.0;
    m_BBoxMinY = 0.0;
    m_BBoxMaxX = 0.0;
    m_BBoxMaxY = 0.0;

    m_BaseRequest = "SERVICE=WMS&REQUEST=GetMap";

    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        WString content;
    
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "BBOX") == 0)
            {
            bvector<double> BBoxValues;
            if(BEXML_Success != pSubNode->GetContentDoubleValues(BBoxValues) || BBoxValues.size() != 4)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"BBOX");
            m_BBoxMinX = BBoxValues[0];
            m_BBoxMinY = BBoxValues[1];
            m_BBoxMaxX = BBoxValues[2];
            m_BBoxMaxY = BBoxValues[3];
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "WIDTH") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt64Value(m_Width) || m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"WIDTH");
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "HEIGHT") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt64Value(m_Height) || m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"HEIGHT");
            }
        else
            {
            WString content;
            pSubNode->GetContent(content);
            
            AString contentA;
            BeStringUtilities::WCharToLocaleChar (contentA, codePage, content.c_str());

            m_BaseRequest += "&";

            if (BeStringUtilities::Stricmp(pSubNode->GetName(), "SRS") == 0 || BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
                m_BaseRequest += "CRS"; // Regardless most servers accept SRS or CRS, the official specs are for CRS to be used whatever is stored as a XWMS file node name.
            else
                m_BaseRequest += pSubNode->GetName();

            m_BaseRequest += "=";
            m_BaseRequest += contentA.c_str();

            if (BeStringUtilities::Stricmp(pSubNode->GetName(), "SRS") == 0 || BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
                {
                m_CRS = contentA.c_str();
                if (m_CRS.empty())
                    {
                    if (BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
                        {
                        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                        L"CRS");
                        }

                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                    L"SRS");
                    }
                }
            else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "FORMAT") == 0)
                {
                m_Format = contentA.c_str();
                if (m_Format.empty())
                    {
                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                    L"FORMAT");
                    }
                }
            }
        }

    // BBOX, WIDTH and HEIGHT are mandatory
    if (m_BBoxMinX == 0 && m_BBoxMaxX == 0 && m_BBoxMinY == 0 && m_BBoxMaxY == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"BBOX");

    if (HDOUBLE_EQUAL_EPSILON(m_BBoxMaxX - m_BBoxMinX, 0.0) || HDOUBLE_EQUAL_EPSILON(m_BBoxMaxY - m_BBoxMinY, 0.0))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"BBOX invalid");


    if (m_Width == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"WIDTH");

    if (m_Height == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"HEIGHT");

    if (m_CRS.empty())
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"CRS");

    if (m_Format.empty())
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"FORMAT");

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "image/jpeg") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/png") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/bmp") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/gif") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        L"FORMAT");

    // tag IMAGESIZE (optional)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("IMAGESIZE")) != 0)
        {
        // tag IMAGESIZE.MINSIZE (optional)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MINSIZE")) != 0)
            {
            pSubChildNode->GetAttributeUInt64Value(m_MinImageSize.m_Width, "width");
            pSubChildNode->GetAttributeUInt64Value(m_MinImageSize.m_Height, "height");

            if (m_MinImageSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"IMAGESIZE.MINSIZE.width");

            if (m_MinImageSize.m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"IMAGESIZE.MINSIZE.height");
            }

        // tag IMAGESIZE.MAXSIZE (optional)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXSIZE")) != 0)
            {
            pSubChildNode->GetAttributeUInt64Value(m_MaxImageSize.m_Width, "width");
            pSubChildNode->GetAttributeUInt64Value(m_MaxImageSize.m_Height, "height");

            if (m_MaxImageSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"IMAGESIZE.MAXSIZE.width");

            if (m_MaxImageSize.m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"IMAGESIZE.MAXSIZE.height");
            }
        }

    // tag SERVERCAPABILITIES (optional)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("SERVERCAPABILITIES")) != 0)
        {
        // tag SERVERCAPABILITIES.MAXBITMAPSIZE (optional)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXBITMAPSIZE")) != 0)
            {
            pSubChildNode->GetAttributeUInt64Value(m_MaxBitmapSize.m_Width,  "width");
            pSubChildNode->GetAttributeUInt64Value(m_MaxBitmapSize.m_Height, "height");

            if (m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"SERVERCAPABILITIES.MAXBITMAPSIZE.width");

            if (m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"SERVERCAPABILITIES.MAXBITMAPSIZE.height");
            }
        }

    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != 0)
        {
        if (BEXML_Success != pChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            L"CONNECTIONTIMEOUT");

        m_ConnectionTimeOut *= 1000; // the value is in second, convert it in millisecond
        }

    }


//-----------------------------------------------------------------------------
// private
// ReadWMS_1_2
//
// Read the version 1.2 and 1.3 of WMS file
//-----------------------------------------------------------------------------
void HRFWMSFile::ReadWMS_1_2(BeXmlNodeP pi_pBentleyWMSFileNode, WString const& version)
    {
    HPRECONDITION(pi_pBentleyWMSFileNode != 0);

    BeXmlNodeP pChildNode;
    BeXmlNodeP pSubChildNode;

    // tag URL
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("URL")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"URL");

    pChildNode->GetContent(m_ServerURL);


    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("REQUEST")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"REQUEST");

    m_BaseRequest = "SERVICE=WMS&REQUEST=GetMap";

    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    // We need to extract the WMS version in order to correctly form the SRS or CRS request (MWS 1.3 requires CRS)
    bool WMSVersionIs1_3 = true;
    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "VERSION") == 0)
            {
            // If the version is not 1.3 then we set the flag
            WString version;
            pSubNode->GetContent(version);
            if (BeStringUtilities::Wcsnicmp(version.c_str(), L"1.3", 3) != 0)
                WMSVersionIs1_3 = false;
            
            // Only one VERSION clause is possible ... we can break out of the loop
            break;    
            }
        }


    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        WString content;
        pSubNode->GetContent(content);
            
        AString contentA;
        BeStringUtilities::WCharToLocaleChar (contentA, codePage, content.c_str());

   
        // BBOX must be set into MAPEXTENT tag
        // LAYERS and STYLES will be added with the SetLayersContext on
        // the editor
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "LAYERS") == 0)
            m_Layers = contentA.c_str();
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "STYLES") == 0)
            m_Styles = contentA.c_str();
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "BBOX") != 0)
            {
            m_BaseRequest += "&";

            // If the WMS server is 1.3 and parameter is SRS then we make sure the request contains CRS instead of CRS
            if (WMSVersionIs1_3 && ((BeStringUtilities::Stricmp(pSubNode->GetName(), "SRS") == 0 || BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)))
                m_BaseRequest += "CRS"; // Regardless most servers accept SRS or CRS, the official specs are for CRS to be used whatever is stored as a XWMS file node name.
            else
                m_BaseRequest += pSubNode->GetName();

            m_BaseRequest += "=";
            m_BaseRequest += contentA.c_str();
            }

        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "SRS") == 0 || BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
            {
            m_CRS = contentA.c_str();
            if (m_CRS.empty())
                {
                if (BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
                    {
                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                    L"CRS");
                    }

                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"SRS");
                }
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "FORMAT") == 0)
            {
            m_Format = contentA.c_str();
            if (m_Format.empty())
                {
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"FORMAT");
                }
            }
        }


    if (m_CRS.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"CRS");

    if (m_Format.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"FORMAT");

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "image/jpeg") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/png") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/bmp") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/gif") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        L"FORMAT");

    // tag MAPEXTENT (mandatory)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("MAPEXTENT")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"MAPEXTENT");

    // tag BBOX (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("BBOX")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"BBOX");

    m_BBoxMinX = 0.0;
    m_BBoxMinY = 0.0;
    m_BBoxMaxX = 0.0;
    m_BBoxMaxY = 0.0;

    bvector<double> BBoxValues;
    if(BEXML_Success != pSubChildNode->GetContentDoubleValues(BBoxValues) || BBoxValues.size() != 4)
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"BBOX");
    m_BBoxMinX = BBoxValues[0];
    m_BBoxMinY = BBoxValues[1];
    m_BBoxMaxX = BBoxValues[2];
    m_BBoxMaxY = BBoxValues[3];

    if (HDOUBLE_EQUAL_EPSILON(m_BBoxMaxX - m_BBoxMinX, 0.0) ||
        HDOUBLE_EQUAL_EPSILON(m_BBoxMaxY - m_BBoxMinY, 0.0))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        L"BBOX invalid");


    if (version == L"1.3")
        {
        m_Width = WMS_MIN_SIZE_X;

        // version 1.3
        // tag WIDTH (optional)
        if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("RASTERWIDTH")) != 0)
            {
            if(BEXML_Success != pChildNode->GetContentUInt64Value(m_Width) || 0 == m_Width)
                m_Width = WMS_MIN_SIZE_X;
            }
        }
    else
        {
        // version 1.2
        m_Width = 1000;
        }

    m_Height = (uint32_t)((double)m_Width * abs((m_BBoxMaxY - m_BBoxMinY) / (m_BBoxMaxX - m_BBoxMinX)));

    // tag SERVICE (optional)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("SERVICE")) != 0)
        {
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXWIDTH")) != 0)
            {
            if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Width) || m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"MaxWidth");
            }

        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXHEIGHT")) != 0)
            {
            if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Height) || m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"MaxHeight");
            }

        if ((pSubChildNode = pChildNode->SelectSingleNode("LOGINREQUIRED")) != 0)
            {
            if (BEXML_Success != pSubChildNode->GetContentBooleanValue(m_NeedAuthentification))
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"LOGINREQUIRED");
            }
        }



    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != 0)
        {
        if (BEXML_Success != pChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            L"CONNECTIONTIMEOUT");

        m_ConnectionTimeOut *= 1000; // the value is in second, convert it in millisecond
        }

    // layers list
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("LayerList")) != 0)
        {
        m_pLayers = new HMDLayersWMS();

        for(BeXmlNodeP pLayerNode = pChildNode->GetFirstChild (); NULL != pLayerNode; pLayerNode = pLayerNode->GetNextSibling())
            {
            WString LayerName;
            WString LayerTitle;
            WString LayerAbstract;
            uint32_t LayerOpaque = 0;
            WString StyleName;
            WString StyleTitle;
            WString StyleAbstract;
            double MinScaleHint = -1;
            double MaxScaleHint = -1;
             
            pLayerNode->GetAttributeStringValue(LayerName,      "NAME");           
            pLayerNode->GetAttributeStringValue(LayerTitle,     "TITLE");           
            pLayerNode->GetAttributeStringValue(LayerAbstract,  "ABSTRACT");            
            pLayerNode->GetAttributeUInt32Value(LayerOpaque,    "Opaque");
                      
            for(BeXmlNodeP pSubLayerNode = pLayerNode->GetFirstChild (); NULL != pSubLayerNode; pSubLayerNode = pSubLayerNode->GetNextSibling())
                {
                if (BeStringUtilities::Stricmp(pSubLayerNode->GetName(), "ScaleHint") == 0)
                    {
                    BeXmlStatus attributeStatus;
                    
                    // get ScaleHint attributes
                    if(BEXML_Success != (attributeStatus = pSubLayerNode->GetAttributeDoubleValue(MinScaleHint, "Min")))
                        {
                        // This parameter is not mandatory. Use a default value
                        MinScaleHint = -1;
                        }

                    if(BEXML_Success != (attributeStatus = pSubLayerNode->GetAttributeDoubleValue(MaxScaleHint, "Max")))
                        {
                        // This parameter is not mandatory. Use a default value
                        MaxScaleHint = -1;
                        }
                    }
                else if (BeStringUtilities::Stricmp(pSubLayerNode->GetName(), "STYLE") == 0)
                    {
                    // get style attributes
                    pSubLayerNode->GetAttributeStringValue(StyleName, "NAME");
                    pSubLayerNode->GetAttributeStringValue(StyleTitle, "TITLE");
                    pSubLayerNode->GetAttributeStringValue(StyleAbstract, "ABSTRACT");
                    }
                else
                    {
                    // not supported
                    HASSERT(0);
                    }
                }

            m_pLayers->AddLayer(new HMDLayerInfoWMS(LayerName,
                                                    LayerName,
                                                    LayerTitle,
                                                    LayerAbstract,
                                                    MinScaleHint,
                                                    MaxScaleHint,
                                                    StyleName,
                                                    StyleTitle,
                                                    StyleAbstract,
                                                    0 != LayerOpaque));
            }
        }
    }
