//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFWcsFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFWCSFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFWCSFile.h>
#include <Imagepp/all/h/HRFOGCServiceEditor.h>
#include <Imagepp/all/h/HRFOGCService.h>

#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>
#include <Imagepp/all/h/HRFException.h>

#include <BeXml/BeXml.h>



//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------

static const  WString s_Device(L"WCS connection");
static        uint32_t s_ConnectionTimeOut = 60000; // default : 60 sec

#define NB_BLOCK_READER_THREAD  4

HFC_IMPLEMENT_SINGLETON(HRFWCSCreator)

//-----------------------------------------------------------------------------
// Capabilities
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFWCSCapabilities
//-----------------------------------------------------------------------------
HRFWCSCapabilities::HRFWCSCapabilities()
    : HRFOGCServiceCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFOGCServiceCodecCapabilities()));
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFOGCServiceCodecCapabilities()));
    }


//-----------------------------------------------------------------------------
// class HRFWCSCreator
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
// This is the creator to instantiate WCS format
//-----------------------------------------------------------------------------
HRFWCSCreator::HRFWCSCreator()
    : HRFRasterFileCreator(HRFWCSFile::CLASS_ID)
    {
    // WCS capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// public
// GetLabel
// Identification information
//-----------------------------------------------------------------------------
WString HRFWCSCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_WCS()); // WCS File Format
    }

//-----------------------------------------------------------------------------
// public
// GetSchemes
// Identification information
//-----------------------------------------------------------------------------
WString HRFWCSCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// public
// GetExtensions
// Identification information
//
// Note : IsKindOfFile use GetExtensions. Adapt the method if the extensions
//        change...
//-----------------------------------------------------------------------------
WString HRFWCSCreator::GetExtensions() const
    {
    return WString(L"*.xwcs");
    }

//-----------------------------------------------------------------------------
// public
// Create
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFWCSCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFWCSFile(pi_rpURL,
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
bool HRFWCSCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        HFCPtr<HFCURLFile>& rpURL((HFCPtr<HFCURLFile>&)pi_rpURL);
        // at least, the file must be have the right extension...
        if (BeStringUtilities::Wcsicmp(rpURL->GetExtension().c_str(), L"xwcs") == 0)
            {
            WString XMLFileName;
            XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
            XMLFileName += L"\\";
            XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

            (const_cast<HRFWCSCreator*>(this))->SharingControlCreate(pi_rpURL);
            HFCLockMonitor SisterFileLock(GetLockManager());

            // Open XML file
            BeXmlStatus xmlStatus;
            BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());
            if (!pXmlDom.IsNull() && BEXML_Success == xmlStatus)
                {            
                // Read data in XML file (strings)
                BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
                if (NULL != pMainNode && BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyWCSFile") == 0)
                    bResult = true;
                }

            SisterFileLock.ReleaseKey();

            HASSERT(!(const_cast<HRFWCSCreator*>(this))->m_pSharingControl->IsLocked());
            (const_cast<HRFWCSCreator*>(this))->m_pSharingControl = 0;
            }
        }

    return bResult;
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
// Create or get the singleton capabilities of WCS file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWCSCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFWCSCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// class HRFWCSFile
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRFWCSFile::HRFWCSFile(const HFCPtr<HFCURL>& pi_rpURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFOGCService(pi_rpURL,
                    WCS,
                    pi_AccessMode,
                    pi_Offset),
    m_NeedAuthentification(false)
    {
    HPRECONDITION(pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID));

    //Read-Only format
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rpURL->GetURL());
        }

    SharingControlCreate();

    WString XMLFileName;
    XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
    XMLFileName += L"\\";
    XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

    // Open XML file
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());
    if (pXmlDom.IsNull() || BEXML_Success != xmlStatus)
        throw HFCCorruptedFileException(pi_rpURL->GetURL());

    // Read data in XML file (strings)
    BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
    if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyWCSFile") != 0)
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

    WString version;
    BeXmlNodeP pChildNode = pMainNode->SelectSingleNode("VERSION");
    if(NULL == pChildNode || BEXML_Success != pChildNode->GetContent(version))
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"VERSION");

    
    if (BeStringUtilities::Wcsicmp(version.c_str(), L"1.0") == 0)
        ReadWCS_1_0(pMainNode);
    else
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"VERSION");

    // the XML document is not longer use
    pChildNode = pMainNode = NULL;
    pXmlDom = NULL;

    // tr #241463
    // keep the file pointer while the HRF is alive
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
        {
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

        URL = pURL->GetURL();
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
    }
//-----------------------------------------------------------------------------
// public
// Destructor
// Destroy WMS file object
//-----------------------------------------------------------------------------
HRFWCSFile::~HRFWCSFile()
    {
    if (m_pConnection->IsConnected())
        m_pConnection->Disconnect();
    }
//-----------------------------------------------------------------------------
// protected
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// protected
// CreateDescriptors
// Create WCS File Descriptors
//-----------------------------------------------------------------------------
void HRFWCSFile::CreateDescriptors(uint64_t pi_Width,
                                   uint64_t pi_Height)
    {

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor =
        new HRFResolutionDescriptor(GetAccessMode(),
                                    GetCapabilities(),
                                    1.0,
                                    1.0,
                                    (const HFCPtr<HRPPixelType>&)m_pPixelType,
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

    Pos = Request.find("crs=");

    if (Pos == string::npos)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"crs");

    GeoCoordinates::BaseGCSPtr pBaseGCS;

    // skip "crs="
    Pos += 4;
    string::size_type Pos2 = Request.find("&", Pos);
    int32_t EPSGCode;
    string CRS = Request.substr(Pos, (Pos2 == string::npos ? string::npos : Pos2 - Pos));
    if (strncmp(CRS.c_str(), "epsg:", 5) == 0)
        {
        // The CRS is in the EPSG format
        WString keyName;
        BeStringUtilities::CurrentLocaleCharToWChar(keyName, CRS.c_str());

        pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS(keyName.c_str());

#if (0)


        if ((sscanf(CRS.c_str(), "epsg:%d", &EPSGCode) != 1) || EPSGCode < 0)
            EPSGCode = TIFFGeo_Undefined;
        else
            {
            // Load Tables
            HAutoPtr<HRFGeoTiffTable> pGeoTables(new HRFGeoTiffTable());
            pGeoTables->LoadTables();

            HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord  UnitsRecord;
            if (pGeoTables->FindCoordSysUnits((unsigned short)EPSGCode, &UnitsRecord) || (EPSGCode > USHRT_MAX))
                {
                m_GTModelType = TIFFGeo_ModelTypeProjected;
                }

            pGeoTables->UnloadTables();
            }

#endif
        }
    else
        {
        EPSGCode = TIFFGeo_Undefined;

        // Special case...
        if (BeStringUtilities::Stricmp(CRS.c_str(), "none") == 0)
            m_GTModelType = TIFFGeo_ModelTypeProjected;
        }

#if (0)
    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys(new HCPGeoTiffKeys());

    // GTModelType
    pGeoTiffKeys->AddKey(GTModelType, (uint32_t)m_GTModelType);

    // GTRasterType
    pGeoTiffKeys->AddKey(GTRasterType, (uint32_t)TIFFGeo_RasterPixelIsArea);

    if (EPSGCode != TIFFGeo_Undefined)
        {
        if ((EPSGCode > 0) && (EPSGCode < 20000))
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

    GeoCoordinates::BaseGCSPtr pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS();

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    pBaseGCS->InitFromGeoTiffKeys (NULL, NULL, pGeoTiffKeys.GetPtr());
#endif

    if (pBaseGCS != NULL && !pBaseGCS->IsValid())
        pBaseGCS = NULL;


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

    pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pBaseGCS.get()));


    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWCSFile::GetCapabilities () const
    {
    return (HRFWCSCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// private
// Read_WCS
//
// Read the WCS file
//-----------------------------------------------------------------------------
void HRFWCSFile::ReadWCS_1_0(BeXmlNodeP pi_pBentleyWCSFileNode)
    {
    HPRECONDITION(pi_pBentleyWCSFileNode != 0);

    BeXmlNodeP pChildNode;
    BeXmlNodeP pSubChildNode;

    // tag version must be present
    if ((pChildNode = pi_pBentleyWCSFileNode->SelectSingleNode("VERSION")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"VERSION");

    // tag URL
    if ((pChildNode = pi_pBentleyWCSFileNode->SelectSingleNode("URL")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"URL");

    pChildNode->GetContent(m_ServerURL);

    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyWCSFileNode->SelectSingleNode("REQUEST")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"REQUEST");

    m_BaseRequest = "REQUEST=GetCoverage&SERVICE=WCS";

    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        WString content;

        // BBOX must be set into MAPEXTENT tag
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "BBOX") != 0 && BeStringUtilities::Stricmp(pSubNode->GetName(), "GRAYPIXELTYPE") != 0 &&
            BEXML_Success == pSubNode->GetContent(content))
            {
            AString contentA;
            BeStringUtilities::WCharToLocaleChar (contentA, codePage, content.c_str());

            m_BaseRequest += "&";
            m_BaseRequest += pSubNode->GetName();
            m_BaseRequest += "=";
            m_BaseRequest += contentA.c_str();
            }

        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0 && BEXML_Success == pSubNode->GetContent(content))
            {
            AString contentA;
            BeStringUtilities::WCharToLocaleChar (contentA, codePage, content.c_str());

            m_CSR = contentA.c_str();
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "FORMAT") == 0 && BEXML_Success == pSubNode->GetContent(content))
            {
            AString contentA;
            BeStringUtilities::WCharToLocaleChar (contentA, codePage, content.c_str());

            m_Format = contentA.c_str();
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "GRAYPIXELTYPE") == 0 && BEXML_Success == pSubNode->GetContent(content))
            {
            if (BeStringUtilities::Wcsicmp(content.c_str(), L"Byte") == 0)
                m_pPixelType = new HRPPixelTypeV8Gray8();
            else if (BeStringUtilities::Wcsicmp(content.c_str(), L"int16") == 0)
                m_pPixelType = new HRPPixelTypeV16Gray16();
            else
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"GRAYPIXELTYPE");
            }
        }

    if (m_CSR.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"CSR");

    if (m_Format.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"FORMAT");

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "geotiff") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "geotiffint16") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        L"FORMAT");

    // tag MAPEXTENT (mandatory)
    if ((pChildNode = pi_pBentleyWCSFileNode->SelectSingleNode("MAPEXTENT")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"MAPEXTENT");

    // tag BBOX (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("BBOX")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"BBOX");

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


    // temporary
    // calculate a raster width and height based on the bbox ratio
    m_Width = 1000;
    m_Height = (uint32_t)(1000.0 * abs((m_BBoxMaxY - m_BBoxMinY) / (m_BBoxMaxX - m_BBoxMinX)));

    // tag SERVICE (optional)
    if ((pChildNode = pi_pBentleyWCSFileNode->SelectSingleNode("SERVICE")) != NULL)
        {
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXWIDTH")) != NULL)
            {
            if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Width) || m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"MaxWidth");
            }

        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXHEIGHT")) != NULL)
            {
            if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Height) || m_MaxBitmapSize.m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"MaxHeight");
            }

        if ((pSubChildNode = pChildNode->SelectSingleNode("LOGINREQUIRED")) != NULL)
            {
            if (BEXML_Success != pSubChildNode->GetContentBooleanValue(m_NeedAuthentification))
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                L"LoginRequired");
            }
        }

    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyWCSFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != NULL)
        {
        if (BEXML_Success != pSubChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            L"CONNECTIONTIMEOUT");

        m_ConnectionTimeOut *= 1000; // the value is in second, convert it in millisecond
        }
    }
