//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFWMSFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFWMSFile.h>
#include "HRFOGCServiceEditor.h"

#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFCURLMemFile.h>
#include <ImagePP/all/h/HFCURLHTTP.h>
#include <ImagePP/all/h/HFCURLHTTPS.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>

#include <ImagePP/all/h/HGF2DStretch.h>

#include <ImagePP/all/h/HRFRasterFileCapabilities.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>

#include <ImagePP/all/h/HMDLayersWMS.h>
#include <ImagePP/all/h/HMDLayerInfoWMS.h>
#include <ImagePP/all/h/HMDVolatileLayers.h>
#include <ImagePP/all/h/HMDContext.h>
#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HCPGeoTiffKeys.h>
#include <BeXml/BeXml.h>
#include <ImagePPInternal/HttpConnection.h>

PUSH_DISABLE_DEPRECATION_WARNINGS

//-----------------------------------------------------------------------------
// Static Members
//-----------------------------------------------------------------------------

static const  Utf8String s_Device("WMS connection");
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
Utf8String HRFWMSCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_WMS()); // WMS File Format
    }

//-----------------------------------------------------------------------------
// public
// GetSchemes
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFWMSCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName() + ";" + HFCURLMemFile::s_SchemeName();
    }

//-----------------------------------------------------------------------------
// public
// GetExtensions
// Identification information
//
// Note : IsKindOfFile use GetExtensions. Adapt the method if the extensions
//        change...
//-----------------------------------------------------------------------------
Utf8String HRFWMSCreator::GetExtensions() const
    {
    return Utf8String("*.xwms");
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
        if (rpURL->GetExtension().EqualsI("xwms"))
            {
            Utf8String XMLFileName = rpURL->GetAbsoluteFileName();

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
            }
        }
    else if (pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID))
        {
        HFCPtr<HFCURLMemFile>& rpURL((HFCPtr<HFCURLMemFile>&)pi_rpURL);
        if (rpURL->GetExtension().EqualsI("xwms"))
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
                    pi_AccessMode,
                    pi_Offset),
    m_NeedAuthentification(true) // TR #259891, by default, always send a request to get the authentication
    {
    HPRECONDITION(pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID)||pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID));


    //Read-Only format
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rpURL->GetURL());
        }

    BeXmlDomPtr pXmlDom;
    BeXmlStatus xmlStatus=BEXML_ParseError;
    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        Utf8String XMLFileName;
        XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
        XMLFileName += "\\";
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
                                        "VERSION");

    Utf8String versionString;
    pChildNode->GetContent(versionString);

    if (versionString.Equals("1.0"))
        ReadWMS_1_0(pMainNode);
    else if (versionString.Equals("1.1"))
        ReadWMS_1_1(pMainNode);
    else if (versionString.Equals("1.2"))
        ReadWMS_1_2(pMainNode, versionString);
    else if (versionString.Equals("1.3"))
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

    // We are being to nice here. xwms file should contain a valid http/https URL but unfortunately we have files that have bad URL. why we created that?
    HFCPtr<HFCURL> pURL = HFCURL::Instanciate(m_ServerURL);
    if (pURL == nullptr)
        {
        // create an HTTP URL
        Utf8String httpURL = "http://" + m_ServerURL;
        HFCPtr<HFCURLHTTP> pHttpURL = new HFCURLHTTP(httpURL);

        // if we have an user or password, change the http url for an https url
        if (!pHttpURL->GetUser().empty() || !pHttpURL->GetPassword().empty())
            {
            httpURL = "https://" + m_ServerURL;
            pHttpURL = new HFCURLHTTP(httpURL);
            }
        pURL = pHttpURL.GetPtr();
        }

    if (!pURL->IsCompatibleWith(HFCURLHTTPBase::CLASS_ID))
        throw HFCInternetConnectionException(s_Device, HFCInternetConnectionException::CANNOT_CONNECT);

    HFCURLHTTPBase* pHttpUrl = static_cast<HFCURLHTTPBase*>(pURL.GetPtr());
    Utf8String searchBase;
    if (!pHttpUrl->GetSearchPart().empty())
            searchBase = pHttpUrl->GetSearchPart();

    Utf8String baseUrl(pHttpUrl->GetURL());
    if(pHttpUrl->GetSearchPart().empty())
        {
        baseUrl.append("?");        // ex: http://www.host.com/Path. Request append after '?'
        }
    else
        {
        baseUrl.append("&");        // ex: http://www.host.com/Path?wms=WorldMap.  Request append after '&'
        }

    m_requestTemplate.reset(new HttpRequest(baseUrl.c_str()));
    m_requestTemplate->SetTimeoutMs(m_ConnectionTimeOut);

    ValidateConnection(m_NeedAuthentification); // <<<< Will throw on errors.

    CreateDescriptors(m_Width, m_Height);

    // create the transfo model to map pixel into the bbox
    // the model set into the page descriptor contains the units ratio. This model
    // can't be use by the editor to map the tile into the bbox.
    double PixelScaleX = fabs(m_BBoxMaxX - m_BBoxMinX) / (double)m_Width;
    double PixelScaleY = fabs(m_BBoxMaxY - m_BBoxMinY) / (double)m_Height;

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

        Utf8String Layers;
        Utf8String Styles;
        const HMDLayerInfoWMS* pLayer;
        for (uint16_t Index = 0; Index < pVolatileLayers->GetNbVolatileLayers(); ++Index)
            {
            if (pVolatileLayers->GetVolatileLayerInfo(Index)->GetVisibleState())
                {
                HPRECONDITION(pVolatileLayers->GetLayerInfo(Index)->IsCompatibleWith(HMDLayerInfoWMS::CLASS_ID));

                pLayer = (const HMDLayerInfoWMS*)pVolatileLayers->GetLayerInfo(Index);
                if (!Layers.empty())
                    {
                    Layers += ",";
                    Styles += ",";
                    }

                Layers += pLayer->GetLayerName();
                Styles += pLayer->GetStyleName();
                }
            }
        m_Request = m_BaseRequest;
        m_Request += "&LAYERS=";
        m_Request += Layers;
        m_Request += "&STYLES=";
        m_Request += Styles;

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
    string Request = m_BaseRequest.c_str();
    CaseInsensitiveStringToolsA().ToLower(Request);

    Pos = Request.find("srs=");

    if (Pos == string::npos)
        Pos = Request.find("crs=");

    if (Pos == string::npos)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "srs or crs");

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


    GeoCoordinates::BaseGCSPtr pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS();

    if (!invalidEPSG)
        pBaseGCS->InitFromEPSGCode(nullptr, nullptr, EPSGCode);
    else
        {
        // This is not WMS complaint yet historically we allow non-EPSG/CRS codes (keynames)
        WString keyName;
        BeStringUtilities::CurrentLocaleCharToWChar(keyName, SRS.c_str());
        pBaseGCS->SetFromCSName(keyName.c_str());
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


    if (pBaseGCS == NULL || !pBaseGCS->IsValid())
        pPage->SetGeocoding(pBaseGCS.get());

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
Utf8String HRFWMSFile::_GetValidateConnectionRequest() const
    {
    ostringstream RequestEditor;
    RequestEditor.precision(16);
    RequestEditor << m_BaseRequest << "&LAYERS=" << m_Layers << "&STYLES=" << m_Styles ;
    RequestEditor << "&width=" << 10 << "&height=" << 10;
    RequestEditor << "&BBOX=";
    RequestEditor << m_BBoxMinX << ",";
    RequestEditor << m_BBoxMinY << ",";
    RequestEditor << m_BBoxMaxX << ",";
    RequestEditor << m_BBoxMaxY;

    return RequestEditor.str().c_str();
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
        throw HRFMissingParameterException(GetURL()->GetURL(), "URL");

    pChildNode->GetContent(m_ServerURL);

    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("REQUEST")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(), "REQUEST");

    WString requestW;
    pChildNode->GetContent(requestW);

    AString requestA;
    BeStringUtilities::WCharToCurrentLocaleChar(requestA, requestW.c_str());

    m_BaseRequest = requestA.c_str();

    string Request(m_BaseRequest.c_str());
    CaseInsensitiveStringToolsA().ToLower(Request);

    string::size_type Pos;
    if ((Pos = Request.find("srs=")) == string::npos && (Pos = Request.find("crs=")) == string::npos)
        {
        if (Request.find("crs=") == string::npos)
            {
            throw HRFMissingParameterException(GetURL()->GetURL(),
                                            "crs");
            }

        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "srs");
        }

    // skip "srs=" or "crs="
    Pos += 4;
    string::size_type Pos2 = Request.find("&", Pos);
    m_CRS = Request.substr(Pos, (Pos2 == string::npos ? string::npos : Pos2 - Pos)).c_str();


    // format
    if ((Pos = Request.find("format=")) == string::npos)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "format");

    // skip "format="
    Pos += 7;
    Pos2 = Request.find("&", Pos);
    m_Format = Request.substr(Pos, (Pos2 == string::npos ? string::npos : Pos2 - Pos)).c_str();

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "image/jpeg") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/png") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/bmp") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        "format");

    // tag BBOX (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("BBOX")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "BBOX");

    bvector<double> BBoxValues;
    if(BEXML_Success != pChildNode->GetContentDoubleValues(BBoxValues) || BBoxValues.size() != 4)
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "BBOX");

    m_BBoxMinX = BBoxValues[0];
    m_BBoxMinY = BBoxValues[1];
    m_BBoxMaxX = BBoxValues[2];
    m_BBoxMaxY = BBoxValues[3];

    if (HDOUBLE_EQUAL_EPSILON(m_BBoxMaxX - m_BBoxMinX, 0.0) || HDOUBLE_EQUAL_EPSILON(m_BBoxMaxY - m_BBoxMinY, 0.0))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "BBOX invalid");

    // tag IMAGESIZE (mandatory)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("IMAGESIZE")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "IMAGESIZE");

    // tag IMAGESIZE.WIDTH (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("WIDTH")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "IMAGESIZE.WIDTH");

    if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_Width))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "IMAGESIZE.WIDTH");

    // tag IMAGESIZE.HEIGHT (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("HEIGHT")) == NULL)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "IMAGESIZE.HEIGHT");

    if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_Height))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "IMAGESIZE.HEIGHT");

    // tag SERVERCAPABILTIES (optional)
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("SERVERCAPABILITIES")) != NULL)
        {
        // tag SERVERCAPABILITIES.MAXWIDTH (mandatory)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXWIDTH")) == NULL)
            throw HRFMissingParameterException(GetURL()->GetURL(),
                                            "SERVERCAPABILITIES.MAXWIDTH");

        if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Width))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "SERVERCAPABILITIES.MAXWIDTH");

        // tag SERVERCAPABILITIES.MAXHEIGHT (mandatory)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXHEIGHT")) == NULL)
            throw HRFMissingParameterException(GetURL()->GetURL(),
                                            "SERVERCAPABILITIES.MAXHEIGHT");

        if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Height))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "SERVERCAPABILITIES.MAXHEIGHT");
        }

    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyXMLFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != NULL)
        {
        if (BEXML_Success != pChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "CONNECTIONTIMEOUT");

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
                                        "URL");

    pChildNode->GetContent(m_ServerURL);

    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("REQUEST")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "REQUEST");

    m_BBoxMinX = 0.0;
    m_BBoxMinY = 0.0;
    m_BBoxMaxX = 0.0;
    m_BBoxMaxY = 0.0;

    // TFS #145512: it has been decided to always add "SERVICE=WMS" but make sure we don't add it when it is already present(#TFS 630573).
    WString tmp;
    BeStringUtilities::Utf8ToWChar(tmp, m_ServerURL.c_str());
    if (std::string::npos == tmp.FindI(L"SERVICE=WMS"))
        m_BaseRequest = "SERVICE=WMS&REQUEST=GetMap";
    else
        m_BaseRequest = "REQUEST=GetMap";

    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        Utf8String content;

        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "BBOX") == 0)
            {
            bvector<double> BBoxValues;
            if(BEXML_Success != pSubNode->GetContentDoubleValues(BBoxValues) || BBoxValues.size() != 4)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "BBOX");
            m_BBoxMinX = BBoxValues[0];
            m_BBoxMinY = BBoxValues[1];
            m_BBoxMaxX = BBoxValues[2];
            m_BBoxMaxY = BBoxValues[3];
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "WIDTH") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt64Value(m_Width) || m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "WIDTH");
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "HEIGHT") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt64Value(m_Height) || m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "HEIGHT");
            }
        else
            {
            WString contentW;
            pSubNode->GetContent(contentW);

            AString contentA;
            BeStringUtilities::WCharToCurrentLocaleChar(contentA, contentW.c_str());

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
                                                        "CRS");
                        }

                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                    "SRS");
                    }
                }
            else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "FORMAT") == 0)
                {
                m_Format = contentA.c_str();
                if (m_Format.empty())
                    {
                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                    "FORMAT");
                    }
                }
            }
        }

    // BBOX, WIDTH and HEIGHT are mandatory
    if (m_BBoxMinX == 0 && m_BBoxMaxX == 0 && m_BBoxMinY == 0 && m_BBoxMaxY == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "BBOX");

    if (HDOUBLE_EQUAL_EPSILON(m_BBoxMaxX - m_BBoxMinX, 0.0) || HDOUBLE_EQUAL_EPSILON(m_BBoxMaxY - m_BBoxMinY, 0.0))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "BBOX invalid");


    if (m_Width == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "WIDTH");

    if (m_Height == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "HEIGHT");

    if (m_CRS.empty())
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "CRS");

    if (m_Format.empty())
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "FORMAT");

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "image/jpeg") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/png") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/bmp") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/gif") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        "FORMAT");

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
                                                "IMAGESIZE.MINSIZE.width");

            if (m_MinImageSize.m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "IMAGESIZE.MINSIZE.height");
            }

        // tag IMAGESIZE.MAXSIZE (optional)
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXSIZE")) != 0)
            {
            pSubChildNode->GetAttributeUInt64Value(m_MaxImageSize.m_Width, "width");
            pSubChildNode->GetAttributeUInt64Value(m_MaxImageSize.m_Height, "height");

            if (m_MaxImageSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "IMAGESIZE.MAXSIZE.width");

            if (m_MaxImageSize.m_Height == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "IMAGESIZE.MAXSIZE.height");
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
                                                "SERVERCAPABILITIES.MAXBITMAPSIZE.width");

            if (m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "SERVERCAPABILITIES.MAXBITMAPSIZE.height");
            }
        }

    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != 0)
        {
        if (BEXML_Success != pChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "CONNECTIONTIMEOUT");

        m_ConnectionTimeOut *= 1000; // the value is in second, convert it in millisecond
        }

    }


//-----------------------------------------------------------------------------
// private
// ReadWMS_1_2
//
// Read the version 1.2 and 1.3 of WMS file
//-----------------------------------------------------------------------------
void HRFWMSFile::ReadWMS_1_2(BeXmlNodeP pi_pBentleyWMSFileNode, Utf8String const& version)
    {
    HPRECONDITION(pi_pBentleyWMSFileNode != 0);

    BeXmlNodeP pChildNode;
    BeXmlNodeP pSubChildNode;

    // tag URL
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("URL")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "URL");

    pChildNode->GetContent(m_ServerURL);


    // tag REQUEST (mandatory)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("REQUEST")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "REQUEST");

    // TFS #145512: it has been decided to always add "SERVICE=WMS" but make sure we don't add it when it is already present(#TFS 630573).
    WString tmp;
    BeStringUtilities::Utf8ToWChar(tmp, m_ServerURL.c_str());
    if (std::string::npos == tmp.FindI(L"SERVICE=WMS"))
        m_BaseRequest = "SERVICE=WMS&REQUEST=GetMap";
    else
        m_BaseRequest = "REQUEST=GetMap";


    // We need to extract the WMS version in order to correctly form the SRS or CRS request (MWS 1.3 requires CRS)
    bool WMSVersionIs1_3 = true;
    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "VERSION") == 0)
            {
            // If the version is not 1.3 then we set the flag
            Utf8String version;
            pSubNode->GetContent(version);
            if (BeStringUtilities::Strnicmp(version.c_str(), "1.3", 3) != 0)
                WMSVersionIs1_3 = false;

            // Only one VERSION clause is possible ... we can break out of the loop
            break;
            }
        }


    for(BeXmlNodeP pSubNode = pChildNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
        {
        Utf8String content;
        pSubNode->GetContent(content);

        // BBOX must be set into MAPEXTENT tag
        // LAYERS and STYLES will be added with the SetLayersContext on
        // the editor
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "LAYERS") == 0)
            m_Layers = content;
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "STYLES") == 0)
            m_Styles = content;
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "BBOX") != 0)
            {
            m_BaseRequest += "&";

            // If the WMS server is 1.3 and parameter is SRS then we make sure the request contains CRS instead of CRS
            if (WMSVersionIs1_3 && ((BeStringUtilities::Stricmp(pSubNode->GetName(), "SRS") == 0 || BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)))
                m_BaseRequest += "CRS"; // Regardless most servers accept SRS or CRS, the official specs are for CRS to be used whatever is stored as a XWMS file node name.
            else
                m_BaseRequest += pSubNode->GetName();

            m_BaseRequest += "=";
            m_BaseRequest += content;
            }

        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "SRS") == 0 || BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
            {
            m_CRS = content;
            if (m_CRS.empty())
                {
                if (BeStringUtilities::Stricmp(pSubNode->GetName(), "CRS") == 0)
                    {
                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                    "CRS");
                    }

                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "SRS");
                }
            }
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "FORMAT") == 0)
            {
            m_Format = content;
            if (m_Format.empty())
                {
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "FORMAT");
                }
            }
        }


    if (m_CRS.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "CRS");

    if (m_Format.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "FORMAT");

    if (BeStringUtilities::Stricmp(m_Format.c_str(), "image/jpeg") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/png") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/bmp") != 0 &&
        BeStringUtilities::Stricmp(m_Format.c_str(), "image/gif") != 0)
        throw HRFMimeFormatNotSupportedException(GetURL()->GetURL(),
                                        "FORMAT");

    // tag MAPEXTENT (mandatory)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("MAPEXTENT")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "MAPEXTENT");

    // tag BBOX (mandatory)
    if ((pSubChildNode = pChildNode->SelectSingleNode("BBOX")) == 0)
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        "BBOX");

    m_BBoxMinX = 0.0;
    m_BBoxMinY = 0.0;
    m_BBoxMaxX = 0.0;
    m_BBoxMaxY = 0.0;

    bvector<double> BBoxValues;
    if(BEXML_Success != pSubChildNode->GetContentDoubleValues(BBoxValues) || BBoxValues.size() != 4)
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "BBOX");
    m_BBoxMinX = BBoxValues[0];
    m_BBoxMinY = BBoxValues[1];
    m_BBoxMaxX = BBoxValues[2];
    m_BBoxMaxY = BBoxValues[3];

    if (HDOUBLE_EQUAL_EPSILON(m_BBoxMaxX - m_BBoxMinX, 0.0) ||
        HDOUBLE_EQUAL_EPSILON(m_BBoxMaxY - m_BBoxMinY, 0.0))
        throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                        "BBOX invalid");


    if (version == "1.3")
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

    m_Height = (uint32_t)((double)m_Width * fabs((m_BBoxMaxY - m_BBoxMinY) / (m_BBoxMaxX - m_BBoxMinX)));

    // tag SERVICE (optional)
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("SERVICE")) != 0)
        {
        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXWIDTH")) != 0)
            {
            if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Width) || m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "MaxWidth");
            }

        if ((pSubChildNode = pChildNode->SelectSingleNode("MAXHEIGHT")) != 0)
            {
            if (BEXML_Success != pSubChildNode->GetContentUInt64Value(m_MaxBitmapSize.m_Height) || m_MaxBitmapSize.m_Width == 0)
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "MaxHeight");
            }

        if ((pSubChildNode = pChildNode->SelectSingleNode("LOGINREQUIRED")) != 0)
            {
            if (BEXML_Success != pSubChildNode->GetContentBooleanValue(m_NeedAuthentification))
                throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                "LOGINREQUIRED");
            }
        }



    // tag CONNECTIONTIMEOUT (optional)
    m_ConnectionTimeOut = s_ConnectionTimeOut; // default : 60 sec
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("CONNECTIONTIMEOUT")) != 0)
        {
        if (BEXML_Success != pChildNode->GetContentUInt32Value(m_ConnectionTimeOut))
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "CONNECTIONTIMEOUT");

        m_ConnectionTimeOut *= 1000; // the value is in second, convert it in millisecond
        }

    // layers list
    if ((pChildNode = pi_pBentleyWMSFileNode->SelectSingleNode("LayerList")) != 0)
        {
        m_pLayers = new HMDLayersWMS();

        for(BeXmlNodeP pLayerNode = pChildNode->GetFirstChild (); NULL != pLayerNode; pLayerNode = pLayerNode->GetNextSibling())
            {
            Utf8String LayerName;
            Utf8String LayerTitle;
            Utf8String LayerAbstract;
            uint32_t LayerOpaque = 0;
            Utf8String StyleName;
            Utf8String StyleTitle;
            Utf8String StyleAbstract;
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
POP_DISABLE_DEPRECATION_WARNINGS
