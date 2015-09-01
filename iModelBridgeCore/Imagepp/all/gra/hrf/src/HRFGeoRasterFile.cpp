//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoRasterFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFGeoRaster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFOracleException.h>
#include <Imagepp/all/h/HRFGeoRasterFile.h>
#include <Imagepp/all/h/HRFGeoRasterEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecZlib.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HVETileIDIterator.h>

#include <Imagepp/all/h/HTIFFUtils.h>

#include <Imagepp/all/h/HFCCallbackRegistry.h>
#include <Imagepp/all/h/HFCCallbacks.h>

#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>
#include <Imagepp/all/h/SDOGeoRasterWrapper.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>

#include <BeXml/BeXml.h>

using namespace ImagePP;

#define GEORASTER_XML_NAMESPACE         "http://xmlns.oracle.com/spatial/georaster"
#define GEORASTER_NAMESPACE_PREFIX      "GeoRaster"


struct SDOObjectInfo
    {
    WString        RasterType;
    bool           IsBlank;
    uint32_t       DefaultRed;
    uint32_t       DefaultGreen;
    uint32_t       DefaultBlue;
    bool           IsValid;
    bool           IsSupported;
    };

struct SDORasterInfo
    {
    uint32_t        TotalDimension;
    HAutoPtr<HRFInterleaveType> pInterleave;
    unsigned short  CellDepth;
    bool            CellDepthSigned;
    bool            CellDepthReal;
    uint32_t        Width;
    uint32_t        Height;
    unsigned short  Band;
    uint32_t        BlockWidth;
    uint32_t        BlockHeight;
    uint32_t        BandBlockSize;
    uint32_t        ResolutionCount;
    WString         Compression;
    Byte            CompressionQuality;
    bool            IsValid;
    bool            IsSupported;
    };

struct SDOSpatialReferenceInfo
    {
    bool            IsReferenced;
    bool            IsRectified;
    uint32_t        SRID;
    double          ScaleX;
    double          ScaleY;
    WString         CoordLocation;
    HArrayAutoPtr<double> pRowCoefficient;
    HArrayAutoPtr<double> pColumnCoefficient;
    bool            IsValid;
    bool            IsSupported;
    };

struct SDOLayerInfo
    {
    unsigned short LayerNumber;
    Byte           pPalette[256*4];     // at most 256 entry RGBA.
    unsigned short PaletteEntryCount;
    bool           IsValid;
    bool           IsSupported;
    };


class OracleAuthenticationError : public HFCAuthenticationError
    {


public:
    OracleAuthenticationError  (const HRFOracleException& pi_rException)
        :   m_RelatedException(pi_rException),
            HFCAuthenticationError()
        {
        }


private:

    virtual WString _ToString() const
        {
        return m_RelatedException.GetExceptionMessage();
        }

    virtual void _Throw() const
        {
        throw m_RelatedException;
        }

    const HRFOracleException m_RelatedException;
    };


// oracle


//-----------------------------------------------------------------------------
// HRFGeoRasterBlockCapabilities
//-----------------------------------------------------------------------------
class HRFGeoRasterBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFGeoRasterBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_ONLY,   // AccessMode
                                   LONG_MAX,        // MaxSizeInBytes
                                   1,               // MinHeight
                                   LONG_MAX,        // MaxHeight
                                   1));             // HeightIncrement

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,    // AccessMode
                                  ULONG_MAX,       // MaxSizeInBytes
                                  1,                // MinWidth
                                  ULONG_MAX,       // MaxWidth
                                  2,                // WidthIncrement
                                  1,                // MinHeight
                                  ULONG_MAX,       // MaxHeight
                                  2));              // HeightIncrement

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_ONLY,   // AccessMode
                                   ULONG_MAX,      // MaxSizeInBytes
                                   1,               // MinWidth
                                   ULONG_MAX,      // MaxWidth
                                   1,               // MinHeight
                                   ULONG_MAX));    // MaxHeight
        }
    };

//-----------------------------------------------------------------------------
// HRFGeoRasterLosslessCodecCapabilities
//-----------------------------------------------------------------------------
class HRFGeoRasterLosslessCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFGeoRasterLosslessCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFGeoRasterBlockCapabilities()));

        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecZlib::CLASS_ID,
                                     new HRFGeoRasterBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFGeoRasterCodecCapabilities
//-----------------------------------------------------------------------------
class HRFGeoRasterCodecCapabilities : public HRFGeoRasterLosslessCodecCapabilities
    {
public :
    // Constructor
    HRFGeoRasterCodecCapabilities()
        : HRFGeoRasterLosslessCodecCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIJG::CLASS_ID,
                                     new HRFGeoRasterBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFGeoRasterCapabilities
//-----------------------------------------------------------------------------
HRFGeoRasterCapabilities::HRFGeoRasterCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8A8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI1R8G8B8A8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // PixelTypeI4R8G8B8A8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI4R8G8B8A8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // PixelTypeV32R8G8B8A8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // PixelTypeV24R8G8B8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // PixelTypeV24B8G8R8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24B8G8R8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // PixelTypeV16Gray16
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // PixelTypeI8R8G8B8A8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI8R8G8B8A8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // HRPPixelTypeV8Gray8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // HRPPixelTypeV16Int16
    // Read capabilities
    // Don't use a lossy codec in case the signed 16 bits data are representing
    // elevation measurements.
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFGeoRasterLosslessCodecCapabilities()));

    // HRPPixelTypeV32Float32
    // Read capabilities
    // Don't use a lossy codec in case the signed 16 bits data are representing
    // elevation measurements.
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32Float32::CLASS_ID,
                                   new HRFGeoRasterLosslessCodecCapabilities()));

    // HRPPixelTypeV64R16G16B16X16
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV64R16G16B16X16::CLASS_ID,
                                   new HRFGeoRasterCodecCapabilities()));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // MultiResolution Capability
    Add(new HRFMultiResolutionCapability(HFC_READ_ONLY,
                                         true,              // SinglePixelType,
                                         true,              // SingleBlockType,
                                         false,             // ArbitaryXRatio,
                                         false,             // ArbitaryYRatio
                                         true,              // XYRatioLocked, default value
                                         256,               // smallest res width, default value
                                         256,               // smallest res height, default value
                                         ULONG_MAX,        // biggest res width
                                         ULONG_MAX,        // biggest res height
                                         false));           // UnlimitesResolution

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
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
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFGeoRasterCreator)

//-----------------------------------------------------------------------------
// HRFGeoRasterCreator
// This is the creator to instantiate GeoRaster format
//-----------------------------------------------------------------------------
HRFGeoRasterCreator::HRFGeoRasterCreator()
    : HRFRasterFileCreator(HRFGeoRasterFile::CLASS_ID)
    {
    // GeoRaster capabilities instance member initialization
    m_pCapabilities = 0;
    }

bool HRFGeoRasterCreator::CanRegister() const
    {
    HINSTANCE hWnd;

    uint32_t CurErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);


    //hWnd = LoadLibraryW(L"Bentley.ImagePP3.dll");
    WString OciDllFilename(DLL_NAME_FOR_OCI);
    hWnd = LoadLibraryW(OciDllFilename.c_str());

    SetErrorMode(CurErrorMode);

    if (hWnd != 0)
        {
        FreeLibrary(hWnd);
        return true;
        }

    return false;
    }

// Identification information
WString HRFGeoRasterCreator::GetLabel() const
    {

// to be define
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_SDOGeoRaster()); // SDO GeoRaster File Format
    }

// Identification information
WString HRFGeoRasterCreator::GetSchemes() const
    {
// to be define
    return WString(HFCURLFile::s_SchemeName() + L";" +
                   HFCURLMemFile::s_SchemeName());
    }

// Identification information
WString HRFGeoRasterCreator::GetExtensions() const
    {
// to be define
    return WString(L"*.xora");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFGeoRasterCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                  HFCAccessMode         pi_AccessMode,
                                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFGeoRasterFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

// Opens the file and verifies if it is the right type
bool HRFGeoRasterCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        // at least, the file must be have the right extension...
        if (BeStringUtilities::Wcsicmp(((const  HFCPtr<HFCURLFile>&)pi_rpURL)->GetExtension().c_str(), L"xora") != 0)
            return bResult;

        WString XMLFileName;
        XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
        XMLFileName += L"\\";
        XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

        (const_cast<HRFGeoRasterCreator*>(this))->SharingControlCreate(pi_rpURL);
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Open XML file
        BeXmlStatus xmlStatus;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());
        if (!pXmlDom.IsNull() && BEXML_Success == xmlStatus)
            {            
            // Read data in XML file (strings)
            BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
            if (NULL != pMainNode && BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyOracleGeoRaster") == 0)
                bResult = true;
            }

        SisterFileLock.ReleaseKey();

        HASSERT(!(const_cast<HRFGeoRasterCreator*>(this))->m_pSharingControl->IsLocked());
        (const_cast<HRFGeoRasterCreator*>(this))->m_pSharingControl = 0;
        }
    else if (pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID))
        {
        // at least, the file must be have the right extension...
        if (BeStringUtilities::Wcsicmp(((const HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetExtension().c_str(), L"xora") != 0)
            return bResult;

        HFCPtr<HFCBuffer> pBuffer(((HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetBuffer());

        HPRECONDITION(pBuffer->GetData() != 0);

        BeXmlStatus xmlStatus;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, pBuffer->GetData(), pBuffer->GetDataSize());
        if (!pXmlDom.IsNull() && BEXML_Success == xmlStatus)
            {            
            // Read data in XML file (strings)
            BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
            if (NULL != pMainNode && BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyOracleGeoRaster") == 0)
                bResult = true;
            }
        }

    return bResult;
    }

// Create or get the singleton capabilities of GeoRaster file.
const HFCPtr<HRFRasterFileCapabilities>& HRFGeoRasterCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFGeoRasterCapabilities();

    return m_pCapabilities;
    }



//-----------------------------------------------------------------------------
// HRFGeoRasterFile class
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFGeoRasterFile::~HRFGeoRasterFile()
    {
    m_pSDOGeoRasterWrapper = 0;
    }


//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFGeoRasterFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                              unsigned short pi_Resolution,
                                                              HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);

    return new HRFGeoRasterEditor(this,
                                  pi_Page,
                                  pi_Resolution,
                                  pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::Save()
    {
    //Nothing do to here
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFGeoRasterFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HASSERT(0);
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFGeoRasterFile::GetCapabilities () const
    {
    return HRFGeoRasterCreator::GetInstance()->GetCapabilities();
    }




//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFGeoRasterFile::HRFGeoRasterFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   HFCAccessMode         pi_AccessMode,
                                   uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset) ,
      m_IsBigEndian(SystemIsBigEndian())
    {
    HPRECONDITION(pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID) ||
                  pi_rpURL->IsCompatibleWith(HFCURLMemFile::CLASS_ID));

    BeXmlDomPtr pXmlDom;
    BeXmlStatus xmlStatus;

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
        HPRECONDITION(((const HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetBuffer()->GetData() != 0);

        const char* pData = reinterpret_cast<const char*>(((const HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetBuffer()->GetData())+pi_Offset;
        size_t dataSize = ((const HFCPtr<HFCURLMemFile>&)pi_rpURL)->GetBuffer()->GetDataSize() - static_cast<size_t>(pi_Offset);

        pXmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, pData, dataSize);
        }
    else
        throw HFCFileNotSupportedException(GetURL()->GetURL());

    if(!pXmlDom.IsValid() || BEXML_Success != xmlStatus)
        throw HFCCorruptedFileException(pi_rpURL->GetURL());

    BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
    if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "BentleyOracleGeoRaster") != 0)
        throw HRFMissingParameterException(pi_rpURL->GetURL(), L"");

    // Optional "version" tag. Default is 1.0 if not present.
    WString versionStr(L"1.0");    
    BeXmlNodeP pVersionNode = NULL;
    //New XML reader is case sensitive and it seems we can have both "version" and "Version"...
    if ((pVersionNode = pMainNode->SelectSingleNode("version")) != NULL)
        pVersionNode->GetContent(versionStr);
    else if ((pVersionNode = pMainNode->SelectSingleNode("Version")) != NULL)
        pVersionNode->GetContent(versionStr);
  
    if (BeStringUtilities::Wcsicmp(versionStr.c_str(), L"1.0") == 0)
        ReadXORA_1_0(pMainNode, SDOGeoRasterWrapper::IsConnected());
    else if (BeStringUtilities::Wcsicmp(versionStr.c_str(), L"1.1") == 0)
        ReadXORA_1_1(pMainNode, SDOGeoRasterWrapper::IsConnected());
    else
        throw HFCFileNotSupportedException(GetURL()->GetURL());

    SharingControlCreate();

    if (!SDOGeoRasterWrapper::IsConnected())
        {
        const HFCAuthenticationCallback* pCallback(HFCAuthenticationCallback::
                                                   GetCallbackFromRegistry(HFCOracleAuthentication::CLASS_ID));

        HFCOracleAuthentication OracleAuthentication;
        if (pCallback == 0)
            throw HFCLoginInformationNotAvailableException();

        bool IsConnect;
        do
            {
            if (!pCallback->GetAuthentication(&OracleAuthentication))
                throw HFCLoginInformationNotAvailableException();


            SDOGeoRasterWrapper::OracleError OracleError;

            IsConnect = SDOGeoRasterWrapper::Connect(OracleAuthentication.GetUser(), OracleAuthentication.GetPassword(), 
                                                     OracleAuthentication.GetDatabaseName(), &OracleError);


            if (!OracleError.m_ErrorMsg.empty())
                {
                HFCPtr<HFCAuthenticationError> pError;
                if (OracleError.m_ErrorCode != OracleError.NO_CODE)
                    pError = new OracleAuthenticationError(HRFOracleException(GetURL()->GetURL(), OracleError.m_ErrorMsg, OracleError.m_ErrorCode));
                else
                    pError = new OracleAuthenticationError(HRFOracleException(GetURL()->GetURL(), OracleError.m_ErrorMsg));

                OracleAuthentication.PushLastError(pError);
                }

            // increment count in callback
            OracleAuthentication.IncrementRetryCount();
            }
        while ((IsConnect == false) && (OracleAuthentication.GetRetryCount() <= pCallback->RetryCount(HFCOracleAuthentication::CLASS_ID)));

        if (IsConnect == false)
            {
            throw HFCCannotConnectToDBException(L"");
            }
        }

    // Create Page and Res Descriptors.
    CreateDescriptors();
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//
// Get only the number of pages. The page was created only when is necessary
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::CreateDescriptors()
    {
    Utf16Char* pHeader;
    size_t HeaderSize;
    m_pSDOGeoRasterWrapper->GetHeader(&pHeader, &HeaderSize);

    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXMLHeader = BeXmlDom::CreateAndReadFromString(xmlStatus, pHeader, HeaderSize / sizeof(Utf16Char));
    if (!pXMLHeader.IsValid() || BEXML_Success != xmlStatus)
        throw HFCCorruptedFileException(GetURL()->GetURL());

    pXMLHeader->RegisterNamespace(GEORASTER_NAMESPACE_PREFIX, GEORASTER_XML_NAMESPACE);
        
    BeXmlNodeP pMainNode = pXMLHeader->GetRootElement();
    if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "geoRasterMetadata") != 0)
        throw HFCCorruptedFileException(GetURL()->GetURL());

    //-----------------------------------------------------------------------------
    // Object Info section
    BeXmlNodeP pObjectInfoNode = pMainNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "objectInfo");
    if (NULL == pObjectInfoNode)
        throw HFCFileNotSupportedException(GetURL()->GetURL());

    SDOObjectInfo ObjectInfo;
    ReadObjectInfo(pObjectInfoNode, &ObjectInfo);

    if (!ObjectInfo.IsValid)
        throw HFCCorruptedFileException(GetURL()->GetURL());

    if (!ObjectInfo.IsSupported)
        throw HFCFileNotSupportedException(GetURL()->GetURL());


    //-----------------------------------------------------------------------------
    // Raster Info section
    BeXmlNodeP pRasterInfoNode = pMainNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "rasterInfo");
    if (NULL == pRasterInfoNode)
        throw HFCFileNotSupportedException(GetURL()->GetURL());

    SDORasterInfo RasterInfo;
    ReadRasterInfo(pRasterInfoNode, &RasterInfo);

    if (!RasterInfo.IsValid)
        throw HFCCorruptedFileException(GetURL()->GetURL());

    if (!RasterInfo.IsSupported)
        throw HFCFileNotSupportedException(GetURL()->GetURL());

    //-----------------------------------------------------------------------------
    // Spatial Referecence section
    BeXmlNodeP pSpatialRefNode = pMainNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "spatialReferenceInfo");

    HAutoPtr<SDOSpatialReferenceInfo> pSpatialReferenceInfo;
    if (pSpatialRefNode != 0)
        {
        pSpatialReferenceInfo = new SDOSpatialReferenceInfo;
        ReadSpatialReferenceInfo(pSpatialRefNode, pSpatialReferenceInfo);

        if (!pSpatialReferenceInfo->IsValid)
            throw HFCCorruptedFileException(GetURL()->GetURL());

        if (!pSpatialReferenceInfo->IsSupported)
            throw HFCFileNotSupportedException(GetURL()->GetURL());
        }

    //-----------------------------------------------------------------------------
    // Layer Info section
    BeXmlNodeP pLayerInfoNode = pMainNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "layerInfo");
    HAutoPtr<SDOLayerInfo> pLayerInfo;
    if (pLayerInfoNode != 0)
        {
        pLayerInfo = new SDOLayerInfo;
        ReadLayerInfo(pLayerInfoNode, pLayerInfo);
        if (!pLayerInfo->IsValid)
            throw HFCCorruptedFileException(GetURL()->GetURL());

        if (!pLayerInfo->IsSupported)
            throw HFCFileNotSupportedException(GetURL()->GetURL());
        }

    // now, build the descriptor with the XML info

    // create PixelType
    HFCPtr<HRPPixelType> pPixelType;
    switch (RasterInfo.CellDepth)
        {
        case 1:
            if (RasterInfo.BandBlockSize == -1 || RasterInfo.BandBlockSize == 1)
                {
                HASSERT(RasterInfo.CellDepthSigned == false);

                if (pLayerInfo == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                    throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                    }

                if (pLayerInfo->PaletteEntryCount != 0)
                    {
                    pPixelType = new HRPPixelTypeI1R8G8B8A8;
                    (const_cast<HRPPixelPalette&>(pPixelType->GetPalette())).SetPalette(pLayerInfo->pPalette,
                                                                                        std::min<unsigned short>(pLayerInfo->PaletteEntryCount, 2));
                    }
                else
                    {
                    HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                    throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                    }
                }
            break;

        case 2:
            HASSERT(0);
            break;

        case 4:
            if (RasterInfo.BandBlockSize == -1 || RasterInfo.BandBlockSize == 1)
                {
                HASSERT(RasterInfo.CellDepthSigned == false);

                if (pLayerInfo == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                    throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                    }

                if (pLayerInfo->PaletteEntryCount != 0)
                    {
                    pPixelType = new HRPPixelTypeI4R8G8B8A8;
                    (const_cast<HRPPixelPalette&>(pPixelType->GetPalette())).SetPalette(pLayerInfo->pPalette,
                                                                                        std::min<unsigned short>(pLayerInfo->PaletteEntryCount, 16));
                    }
                else
                    {
                    HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                    throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                    }
                }
            break;

        case 8:
            {
            if (((RasterInfo.BandBlockSize == -1) || (RasterInfo.BandBlockSize == 1)) &&
                (RasterInfo.CellDepthSigned == false))
                {
                if (pLayerInfo == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                    throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                    }

                switch (RasterInfo.Band)
                    {
                    case (unsigned short)-1:   // no band specified
                        if (pLayerInfo->PaletteEntryCount != 0 && pLayerInfo)
                            {
                            pPixelType = new HRPPixelTypeI8R8G8B8A8;
                            (const_cast<HRPPixelPalette&>(pPixelType->GetPalette())).SetPalette(pLayerInfo->pPalette,
                                                                                                pLayerInfo->PaletteEntryCount); // ReadLayerInfo return at least 256 entries
                            }
                        else
                            {
                            HASSERT(RasterInfo.CellDepthSigned == false);
                            pPixelType = new HRPPixelTypeV8Gray8;
                            }
                        break;

                    case 3:
                        if (ObjectInfo.DefaultRed == 1 && ObjectInfo.DefaultGreen == 2 && ObjectInfo.DefaultBlue == 3)
                            pPixelType = new HRPPixelTypeV24R8G8B8();
                        else if (ObjectInfo.DefaultRed == 3 && ObjectInfo.DefaultGreen == 2 && ObjectInfo.DefaultBlue == 1)
                            pPixelType = new HRPPixelTypeV24B8G8R8();
                        else
                            {
                            HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                            }
                        break;

                    case 4:
                        if (ObjectInfo.DefaultRed != 1 || ObjectInfo.DefaultGreen != 2 || ObjectInfo.DefaultBlue != 3)
                            {
                            HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                            }
                        pPixelType = new HRPPixelTypeV32R8G8B8A8();
                        break;

                    default:
                        HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported pixel type");
                        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
                    }
                }
            break;
            }

        case 16:

            switch (RasterInfo.Band)
                {
                case USHRT_MAX:
                case 1:
                    if (RasterInfo.CellDepthSigned == false)
                        {
                        pPixelType = new HRPPixelTypeV16Gray16;
                        }
                    else
                        {
                        pPixelType = new HRPPixelTypeV16Int16;
                        }
                    break;
                }
            break;

        case 32:

            switch (RasterInfo.Band)
                {
                case USHRT_MAX:
                case 1:
                    if (RasterInfo.CellDepthReal == true)
                        {
                        HASSERT(RasterInfo.CellDepthSigned == true);
                        pPixelType = new HRPPixelTypeV32Float32;
                        }
                    break;
                }
            break;


        default:
            // not supported for now
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
        }

    if (pPixelType == 0)
        {
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
        }

    // compression
    HFCPtr<HCDCodec> pCodec;
    if (!RasterInfo.Compression.empty())
        {
        if (BeStringUtilities::Wcsicmp(RasterInfo.Compression.c_str(), L"JPEG-B") == 0)
            {
            if (RasterInfo.BandBlockSize == 1)
                pCodec = new HCDCodecIJG(RasterInfo.BlockWidth, RasterInfo.BlockHeight, 8);
            else if (RasterInfo.BandBlockSize == 3)
                pCodec = new HCDCodecIJG(RasterInfo.BlockWidth, RasterInfo.BlockHeight, 24);
            else
                {
                HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported compression");
                throw HRFCodecNotSupportedException(GetURL()->GetURL());
                }

            ((HFCPtr<HCDCodecIJG>&)pCodec)->SetSourceColorMode(HCDCodecIJG::BGR);
            if (RasterInfo.CompressionQuality != -1)
                ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality(RasterInfo.CompressionQuality);
            }
        else if (BeStringUtilities::Wcsicmp(RasterInfo.Compression.c_str(), L"JPEG-F") == 0)
            {
            if (RasterInfo.BandBlockSize == 1)
                pCodec = new HCDCodecIJG(RasterInfo.BlockWidth, RasterInfo.BlockHeight, 8);
            else if (RasterInfo.BandBlockSize == 3)
                pCodec = new HCDCodecIJG(RasterInfo.BlockWidth, RasterInfo.BlockHeight, 24);
            else
                {
                HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported compression");
                throw HRFCodecNotSupportedException(GetURL()->GetURL());
                }

            ((HFCPtr<HCDCodecIJG>&)pCodec)->SetSourceColorMode(HCDCodecIJG::BGR);
            ((HFCPtr<HCDCodecIJG>&)pCodec)->SetAbbreviateMode(true);

            if (RasterInfo.CompressionQuality != -1)
                ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality(RasterInfo.CompressionQuality);
            }
        else if (BeStringUtilities::Wcsicmp(RasterInfo.Compression.c_str(), L"DEFLATE") == 0)
            {
            pCodec = new HCDCodecZlib();
            }
        else if (BeStringUtilities::Wcsicmp(RasterInfo.Compression.c_str(), L"NONE") == 0)
            {
            pCodec = new HCDCodecIdentity;
            }
        else
            {
            HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported compression type");
            throw HRFCodecNotSupportedException(GetURL()->GetURL());
            }
        }
    else
        pCodec = new HCDCodecIdentity;

    HRFPageDescriptor::ListOfResolutionDescriptor ListOfResolutionDescriptor;
    uint32_t     ResWidth;
    uint32_t     ResHeight;
    uint32_t     BlockWidth  = RasterInfo.BlockWidth;
    uint32_t     BlockHeight = RasterInfo.BlockHeight;
    double      ResScale = 1.0;
    uint32_t     ResFactor = 1;

    for (unsigned short Res = 0; Res < RasterInfo.ResolutionCount; Res++)
        {
        // compute resolution size
        // according to Oracle documentation
        ResWidth = (uint32_t)(RasterInfo.Width / ResFactor);
        ResHeight = (uint32_t)(RasterInfo.Height / ResFactor);

        //If the block type of the first resolution is image, then ensure that the
        //block type of the sub-resolutions are also image.
        if ((Res > 0) &&
            (ListOfResolutionDescriptor[0]->GetBlockType() == HRFBlockType::IMAGE))
            {
            BlockWidth  = ResWidth;
            BlockHeight = ResHeight;
            }

        HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor =
            new HRFResolutionDescriptor(GetAccessMode(),
                                        GetCapabilities(),
                                        ResScale,
                                        ResScale,
                                        pPixelType,
                                        pCodec,
                                        HRFBlockAccess::RANDOM,                         // RBlockAccess,
                                        HRFBlockAccess::RANDOM,                         // WBlockAccess,
                                        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                        HRFInterleaveType::PIXEL,                       // InterleaveType
                                        false,
                                        ResWidth,
                                        ResHeight,
                                        BlockWidth,
                                        BlockHeight);

        ListOfResolutionDescriptor.push_back(pResolutionDescriptor);
        ResScale /= 2.0;
        ResFactor *= 2;
        }

    HFCPtr<HGF2DTransfoModel>     pModel;
    RasterFileGeocodingPtr pGeocoding(RasterFileGeocoding::Create());


    if (pSpatialReferenceInfo != 0 && pSpatialReferenceInfo->IsReferenced)
        {
        HFCPtr<HGF2DAffine> pAffineModel(new HGF2DAffine);
        pAffineModel->SetByMatrixParameters(pSpatialReferenceInfo->pColumnCoefficient[0],
                                            pSpatialReferenceInfo->pColumnCoefficient[1],
                                            pSpatialReferenceInfo->pColumnCoefficient[2],
                                            pSpatialReferenceInfo->pRowCoefficient[0],
                                            pSpatialReferenceInfo->pRowCoefficient[1],
                                            pSpatialReferenceInfo->pRowCoefficient[2]);

        pAffineModel->Reverse();

        if (BeStringUtilities::Wcsicmp(pSpatialReferenceInfo->CoordLocation.c_str(), L"CENTER") == 0)
            {
            // see HRFTWFPageFile::BuildTransfoModel() for detail
            HFCMatrix<3,3> Matrix(pAffineModel->GetMatrix());
            pAffineModel->SetByMatrixParameters(Matrix[0][2] - 0.5 * (Matrix[0][0] + Matrix[1][0]),
                                                Matrix[0][0],
                                                Matrix[0][1],
                                                Matrix[1][2] - 0.5 * (Matrix[0][1] + Matrix[1][1]),
                                                Matrix[1][0],
                                                Matrix[1][1]);
            }

        if ((pModel = pAffineModel->CreateSimplifiedModel()) == 0)
            pModel = pAffineModel;

        if (pModel != 0)
            {
            pGeocoding = ExtractGeocodingInformation(*pSpatialReferenceInfo);

            //Translate the geo-reference to meter
            pModel = pGeocoding->TranslateToMeter(pModel, 1.0, false, 0);
            }
        }

    HFCPtr<HRFPageDescriptor> pPage = new HRFPageDescriptor (GetAccessMode(),
                                                             GetCapabilities(),
                                                             ListOfResolutionDescriptor,
                                                             0,
                                                             0,
                                                             0,
                                                             0,
                                                             pModel,
                                                             0);

    pPage->InitFromRasterFileGeocoding(*pGeocoding);

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// private
// ConnectToOracle
//-----------------------------------------------------------------------------
bool HRFGeoRasterFile::ConnectToOracle (WStringCR pi_rConnectionString)
    {
    bool IsConnect = false;

    if (!pi_rConnectionString.empty())
        {
        IsConnect = SDOGeoRasterWrapper::Connect(pi_rConnectionString);
        }
    else
        {
        const HFCAuthenticationCallback* pCallback(HFCAuthenticationCallback::
                                                   GetCallbackFromRegistry(HFCOracleAuthentication::CLASS_ID));

        HFCOracleAuthentication OracleAuthentication;
        if (pCallback == 0)
            throw HFCLoginInformationNotAvailableException();

        do
            {
            if (OracleAuthentication.GetRetryCount() >= pCallback->RetryCount(HFCOracleAuthentication::CLASS_ID))
                {
                throw HRFAuthenticationMaxRetryCountReachedException(GetURL()->GetURL());
                }

            if (!pCallback->GetAuthentication(&OracleAuthentication))
                {
                if (pCallback->IsCancelled())
                    throw HRFAuthenticationCancelledException(GetURL()->GetURL());
                else
                    throw HFCLoginInformationNotAvailableException();
                }

            SDOGeoRasterWrapper::OracleError OracleError;

            if (OracleAuthentication.GetConnectionString().empty())
                {
                if (SDOGeoRasterWrapper::Connect(OracleAuthentication.GetUser(), OracleAuthentication.GetPassword(), 
                                                 OracleAuthentication.GetDatabaseName(), &OracleError))
                    {
                    IsConnect = true;
                    }

                }
            else
                {
                if (SDOGeoRasterWrapper::Connect(OracleAuthentication.GetConnectionString(), &OracleError))
                    {
                    IsConnect = true;
                    }
                }

            if (!OracleError.m_ErrorMsg.empty())
                {
                HFCPtr<HFCAuthenticationError> pError;
                if (OracleError.m_ErrorCode != OracleError.NO_CODE)
                    pError = new OracleAuthenticationError(HRFOracleException(GetURL()->GetURL(), OracleError.m_ErrorMsg, OracleError.m_ErrorCode));
                else
                    pError = new OracleAuthenticationError(HRFOracleException(GetURL()->GetURL(), OracleError.m_ErrorMsg));

                OracleAuthentication.PushLastError(pError);
                }

            // increment count in callback
            OracleAuthentication.IncrementRetryCount();
            }
        while (IsConnect == false);
        }

    return IsConnect;
    }

//-----------------------------------------------------------------------------
// private
// ReadXORA_1_0
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::ReadXORA_1_0(BeXmlNodeP pi_pMainNode, bool pi_Connected)
    {
    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    if (!pi_Connected)
        {
        BeXmlNodeP pUsr = pi_pMainNode->SelectSingleNode("user");
        BeXmlNodeP pPwd = pi_pMainNode->SelectSingleNode("password");
        BeXmlNodeP pService = pi_pMainNode->SelectSingleNode("service");

        if (NULL == pUsr && NULL ==  pPwd && NULL == pService)
            {
            SDOGeoRasterWrapper::OracleError Error;
            if (!SDOGeoRasterWrapper::Connect(WString(),&Error))
                throw HFCCannotConnectToDBException(L"");
            }
        else if (NULL != pUsr)
            {
            WString ConnectionString;
            pUsr->GetContent(ConnectionString);

            WString content;
            if (NULL != pPwd && BEXML_Success == pPwd->GetContent(content) && !content.empty())
                {
                ConnectionString += L"/";
                ConnectionString += content;
                }

            if (NULL != pService && BEXML_Success == pService->GetContent(content) && !content.empty())
                {
                ConnectionString += L"@";
                ConnectionString += content;
                }

            if (!ConnectToOracle(ConnectionString))
                throw HFCCannotConnectToDBException(L"");
            }
        else
            throw HFCCannotConnectToDBException(L"");
        }

    BeXmlNodeP pTableNameNode = pi_pMainNode->SelectSingleNode("tablename");
    BeXmlNodeP pColumnNameNode = pi_pMainNode->SelectSingleNode("columnname");
    BeXmlNodeP pRasterIDNode = pi_pMainNode->SelectSingleNode("rasterid");

    WString tableName, ColumnName, RasterId;

    if (NULL == pTableNameNode || BEXML_Success != pTableNameNode->GetContent(tableName))
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"tablename");
    if (NULL == pColumnNameNode || BEXML_Success != pColumnNameNode->GetContent(ColumnName))
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"columnname");
    if (NULL == pRasterIDNode || BEXML_Success != pRasterIDNode->GetContent(RasterId))
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"rasterid");

    m_pSDOGeoRasterWrapper = SDOGeoRasterWrapper::GetWrapper(tableName, ColumnName, RasterId);
    }


//-----------------------------------------------------------------------------
// private
// ReadXORA_1_1
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::ReadXORA_1_1(BeXmlNodeP pi_pMainNode, bool pi_Connected)
    {
    LangCodePage codePage;
    BeStringUtilities::GetCurrentCodePage (codePage);

    if (!pi_Connected)
        {
        BeXmlNodeP pConnectionString = pi_pMainNode->SelectSingleNode("connectionstring");

        WString connectString;
        pConnectionString->GetContent(connectString);

        if (!ConnectToOracle(connectString))
            throw HFCCannotConnectToDBException(L"");
        }

    //New XML reader is case sensitive and it seems we can have both "version" and "Version"...
    BeXmlNodeP pGeoRaster = pi_pMainNode->SelectSingleNode("GEORASTER");
    if (NULL == pGeoRaster)
        {
        //some where created with lower case
        pGeoRaster = pi_pMainNode->SelectSingleNode("georaster");
        if (NULL == pGeoRaster)
            throw HRFMissingParameterException(GetURL()->GetURL(),
                L"GEORASTER");
        }

    WString tableAttribute, ColumnAttribute, RasterIdAttribute;

    if (BEXML_Success != pGeoRaster->GetAttributeStringValue(tableAttribute, "table") || tableAttribute.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"table");

    if (BEXML_Success != pGeoRaster->GetAttributeStringValue(ColumnAttribute, "column") || ColumnAttribute.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"column");

    if (BEXML_Success != pGeoRaster->GetAttributeStringValue(RasterIdAttribute, "rasterid") || RasterIdAttribute.empty())
        throw HRFMissingParameterException(GetURL()->GetURL(),
                                        L"rasterid");

    m_pSDOGeoRasterWrapper = SDOGeoRasterWrapper::GetWrapper(tableAttribute, ColumnAttribute, RasterIdAttribute);
    }

//-----------------------------------------------------------------------------
// private
// ReadObjectInfo
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::ReadObjectInfo(BeXmlNodeP        pi_pObjectInfoNode,
                                      SDOObjectInfo*    po_pSDOObjectInfo)
    {
    HPRECONDITION(pi_pObjectInfoNode != 0);
    HPRECONDITION(po_pSDOObjectInfo != 0);

    po_pSDOObjectInfo->IsBlank = false;
    po_pSDOObjectInfo->DefaultRed = 0;
    po_pSDOObjectInfo->DefaultGreen = 0;
    po_pSDOObjectInfo->DefaultBlue = 0;
    po_pSDOObjectInfo->IsSupported = true;
    po_pSDOObjectInfo->IsValid = true;

    for(BeXmlNodeP pSubNode = pi_pObjectInfoNode->GetFirstChild (); 
        NULL != pSubNode && po_pSDOObjectInfo->IsValid && po_pSDOObjectInfo->IsSupported;
        pSubNode = pSubNode->GetNextSibling())
        {
        // rasterType
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "rasterType") == 0)
            {
            pSubNode->GetContent(po_pSDOObjectInfo->RasterType);
            }

        // isBlank
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "isBlank") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentBooleanValue(po_pSDOObjectInfo->IsBlank))
                po_pSDOObjectInfo->IsBlank = false;
            }

        // defaultRed
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "defaultRed") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt32Value(po_pSDOObjectInfo->DefaultRed))
                {
                HASSERT(!L"HRFGeoRasterFile::ReadObjectInfo() : invalid XML header");
                po_pSDOObjectInfo->IsValid = false;
                }
            }

        // defaultGreen
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "defaultGreen") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt32Value(po_pSDOObjectInfo->DefaultGreen))
                {
                HASSERT(!L"HRFGeoRasterFile::ReadObjectInfo() : invalid XML header");
                po_pSDOObjectInfo->IsValid = false;
                }
            }

        // defaultBlue
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "defaultBlue") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt32Value(po_pSDOObjectInfo->DefaultBlue))
                {
                HASSERT(!L"HRFGeoRasterFile::ReadObjectInfo() : invalid XML header");
                po_pSDOObjectInfo->IsValid = false;
                }
            }
        else
            {
            HASSERT(0);
            }
        }
    }


//-----------------------------------------------------------------------------
// private
// ReadRasterInfo
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::ReadRasterInfo(BeXmlNodeP     pi_pRasterInfoNode,
                                      SDORasterInfo* po_pSDORasterInfo)
    {
    HPRECONDITION(pi_pRasterInfoNode != 0);
    HPRECONDITION(po_pSDORasterInfo != 0);

    po_pSDORasterInfo->pInterleave          = 0;
    po_pSDORasterInfo->CellDepth            = -1;
    po_pSDORasterInfo->Width                = -1;
    po_pSDORasterInfo->Height               = -1;
    po_pSDORasterInfo->Band                 = -1;
    po_pSDORasterInfo->BlockWidth           = -1;
    po_pSDORasterInfo->BlockHeight          = -1;
    po_pSDORasterInfo->BandBlockSize        = -1;
    po_pSDORasterInfo->ResolutionCount      = -1;
    po_pSDORasterInfo->Compression.clear();
    po_pSDORasterInfo->CompressionQuality   = -1;
    po_pSDORasterInfo->IsValid              = true;
    po_pSDORasterInfo->IsSupported          = true;

    for(BeXmlNodeP pSubNode = pi_pRasterInfoNode->GetFirstChild (); 
        NULL != pSubNode && po_pSDORasterInfo->IsValid && po_pSDORasterInfo->IsSupported;
        pSubNode = pSubNode->GetNextSibling())
        {
        WString nodeContent;

        // cellRepresentation
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "cellRepresentation") == 0)
            {
            pSubNode->GetContent(nodeContent);
            if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"UNDEFINED") != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported = false;
                }
            }

        // cellDepth
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "CellDepth") == 0)
            {
            po_pSDORasterInfo->CellDepthSigned = false;
            po_pSDORasterInfo->CellDepthReal = false;

            pSubNode->GetContent(nodeContent);

            if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"1BIT") == 0)
                po_pSDORasterInfo->CellDepth = 1;
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"2BIT") == 0)
                po_pSDORasterInfo->CellDepth = 2;
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"4BIT") == 0)
                po_pSDORasterInfo->CellDepth = 4;
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"8BIT_U") == 0)
                po_pSDORasterInfo->CellDepth = 8;
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"8BIT_S") == 0)
                {
                po_pSDORasterInfo->CellDepth = 8;
                po_pSDORasterInfo->CellDepthSigned = true;
                }
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"16BIT_U") == 0)
                po_pSDORasterInfo->CellDepth = 16;
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"16BIT_S") == 0)
                {
                po_pSDORasterInfo->CellDepth = 16;
                po_pSDORasterInfo->CellDepthSigned = true;
                }
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"32BIT_U") == 0)
                po_pSDORasterInfo->CellDepth = 32;
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"32BIT_S") == 0)
                {
                po_pSDORasterInfo->CellDepth = 32;
                po_pSDORasterInfo->CellDepthSigned = true;
                }
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"32BIT_REAL") == 0)
                {
                po_pSDORasterInfo->CellDepth = 32;
                po_pSDORasterInfo->CellDepthReal = true;
                po_pSDORasterInfo->CellDepthSigned = true;
                }
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"64BIT_REAL") == 0)
                {
                po_pSDORasterInfo->CellDepth = 64;
                po_pSDORasterInfo->CellDepthReal = true;
                po_pSDORasterInfo->CellDepthSigned = true;
                }
            else if (BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"128BIT_REAL") == 0)
                {
                po_pSDORasterInfo->CellDepth = 128;
                po_pSDORasterInfo->CellDepthReal = true;
                po_pSDORasterInfo->CellDepthSigned = true;
                }
            else
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported = false;
                }
            }

        // totalDimensions
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "totalDimensions") == 0)
            {
            if (BEXML_Success != pSubNode->GetContentUInt32Value(po_pSDORasterInfo->TotalDimension))
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                po_pSDORasterInfo->IsValid = false;
                }
            }

        // dimensionSize
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "dimensionSize") == 0)
            {
            BeXmlNodeP pNode;
            WString AttValue;
            pSubNode->GetAttributeStringValue(AttValue, "type");

            if (BeStringUtilities::Wcsicmp(AttValue.c_str(), L"ROW") == 0)
                {
                if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "size")) == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                    po_pSDORasterInfo->IsSupported = false;
                    }

                if (BEXML_Success != pNode->GetContentUInt32Value(po_pSDORasterInfo->Height))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }

                }
            else if (BeStringUtilities::Wcsicmp(AttValue.c_str(), L"COLUMN") == 0)
                {
                if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "size")) == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                    po_pSDORasterInfo->IsSupported = false;
                    }

                if (BEXML_Success != pNode->GetContentUInt32Value(po_pSDORasterInfo->Width))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }
                }
            else if (BeStringUtilities::Wcsicmp(AttValue.c_str(), L"BAND") == 0)
                {
                if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "size")) == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                    po_pSDORasterInfo->IsSupported = false;
                    }

                uint32_t nbBand = 0;
                if (BEXML_Success != pNode->GetContentUInt32Value(nbBand) || nbBand > USHRT_MAX)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }
                else
                    po_pSDORasterInfo->Band = (unsigned short)po_pSDORasterInfo->Band;
                }
            else
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported;
                }
            }

        // blocking
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "blocking") == 0)
            {
            BeXmlNodeP pNode;

            if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "rowBlockSize")) != 0)
                {
                if (BEXML_Success != pNode->GetContentUInt32Value(po_pSDORasterInfo->BlockHeight))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : Invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }
                }

            if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "columnBlockSize")) != 0)
                {
                if (BEXML_Success != pNode->GetContentUInt32Value(po_pSDORasterInfo->BlockWidth))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : Invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }
                }

            if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "bandBlockSize")) != 0)
                {
                if (BEXML_Success != pNode->GetContentUInt32Value(po_pSDORasterInfo->BandBlockSize))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : Invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }
                }
            }

        // interleaving
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "interleaving") == 0)
            {
            WString content;
            pSubNode->GetContent(content);

            if (BeStringUtilities::Wcsicmp(content.c_str(), L"BSQ") == 0)
                po_pSDORasterInfo->pInterleave = new HRFInterleaveType(HRFInterleaveType::PLANE);
            else if (BeStringUtilities::Wcsicmp(content.c_str(), L"BIL") == 0)
                po_pSDORasterInfo->pInterleave = new HRFInterleaveType(HRFInterleaveType::LINE);
            else if (BeStringUtilities::Wcsicmp(content.c_str(), L"BIP") == 0)
                po_pSDORasterInfo->pInterleave = new HRFInterleaveType(HRFInterleaveType::PIXEL);
            else
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported = false;
                }
            }

        // pyramid
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "pyramid") == 0)
            {
            BeXmlNodeP pTypeNode;
            if ((pTypeNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "type")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported = false;
                }

            WString typeContent;
            pTypeNode->GetContent(typeContent);

            if (BeStringUtilities::Wcsicmp(typeContent.c_str(), L"DECREASE") == 0)
                {
                BeXmlNodeP maxLevelNode;
                // maxLevel
                if ((maxLevelNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "maxLevel")) == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                    po_pSDORasterInfo->IsSupported = false;
                    }

                if (BEXML_Success != maxLevelNode->GetContentUInt32Value(po_pSDORasterInfo->ResolutionCount))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }
                }
            else if (BeStringUtilities::Wcsicmp(typeContent.c_str(), L"NONE") == 0)
                po_pSDORasterInfo->ResolutionCount = 1;
            else
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported = false;
                }
            }

        // compression
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "compression") == 0)
            {
            BeXmlNodeP pTypeNode;
            if ((pTypeNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "type")) != 0)
                {
                pTypeNode->GetContent(po_pSDORasterInfo->Compression);
                }
            else
                {
                HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : unsupported XML header");
                po_pSDORasterInfo->IsSupported = false;
                }

            BeXmlNodeP pQualityNode;
            if ((pQualityNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "quality")) != 0)
                {
                uint32_t Quality;
                if (BEXML_Success != pQualityNode->GetContentUInt32Value(Quality))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }

                if (Quality > 100)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadRasterInfo() : invalid XML header");
                    po_pSDORasterInfo->IsValid = false;
                    }

                po_pSDORasterInfo->CompressionQuality = (Byte)Quality;
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// private
// ReadSpatialReferenceInfo
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::ReadSpatialReferenceInfo(BeXmlNodeP                  pi_pSpatialReferenceInfoNode,
                                                SDOSpatialReferenceInfo*    po_pSDOSpatialReferenceInfo)
    {
    HPRECONDITION(pi_pSpatialReferenceInfoNode != 0);
    HPRECONDITION(po_pSDOSpatialReferenceInfo != 0);

    po_pSDOSpatialReferenceInfo->SRID = 999999;
    po_pSDOSpatialReferenceInfo->ScaleX = DBL_MAX;
    po_pSDOSpatialReferenceInfo->ScaleY = DBL_MAX;

    BeXmlNodeP pIsReferencedNode = NULL;
    BeXmlNodeP pIsRectifiedNode = NULL;

    po_pSDOSpatialReferenceInfo->IsValid = true;
    po_pSDOSpatialReferenceInfo->IsSupported = true;

    for(BeXmlNodeP pSubNode = pi_pSpatialReferenceInfoNode->GetFirstChild (); 
        NULL != pSubNode && po_pSDOSpatialReferenceInfo->IsValid && po_pSDOSpatialReferenceInfo->IsSupported;
        pSubNode = pSubNode->GetNextSibling())
        {    
        WString nodeContent;

        // isReferenced
        if (BeStringUtilities::Stricmp(pSubNode->GetName(), "isReferenced") == 0)
            {
            pIsReferencedNode = pSubNode;
            pSubNode->GetContentBooleanValue(po_pSDOSpatialReferenceInfo->IsReferenced);
            }

        // isRectified / Not supported presently
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "isRectified") == 0)
            {
            pIsRectifiedNode = pSubNode;
            pSubNode->GetContentBooleanValue(po_pSDOSpatialReferenceInfo->IsRectified);
            }

        // SRID
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "SRID") == 0)
            {
            if(BEXML_Success != pSubNode->GetContentUInt32Value(po_pSDOSpatialReferenceInfo->SRID))
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }
            }

        // spatialResolution
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "spatialResolution") == 0)
            {
            BeXmlNodeP pNode;

            WString dimensionType;
            pSubNode->GetAttributeStringValue(dimensionType, "dimensionType");

            if (BeStringUtilities::Wcsicmp(dimensionType.c_str(), L"X") == 0)
                {
                if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "resolution")) == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                    po_pSDOSpatialReferenceInfo->IsSupported = false;
                    goto WRAPUP;
                    }

                if (BEXML_Success != pNode->GetContentDoubleValue(po_pSDOSpatialReferenceInfo->ScaleX))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                    po_pSDOSpatialReferenceInfo->IsSupported = false;
                    goto WRAPUP;
                    }
                }
            else if (BeStringUtilities::Wcsicmp(dimensionType.c_str(), L"Y") == 0)
                {
                if ((pNode = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "resolution")) == 0)
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                    po_pSDOSpatialReferenceInfo->IsSupported = false;
                    goto WRAPUP;
                    }

                if (BEXML_Success != pNode->GetContentDoubleValue(po_pSDOSpatialReferenceInfo->ScaleY))
                    {
                    HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                    po_pSDOSpatialReferenceInfo->IsSupported = false;
                    goto WRAPUP;
                    }
                }
            else
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }
            }

        // modelCoordinateLocation
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "modelCoordinateLocation") == 0)
            pSubNode->GetContent(po_pSDOSpatialReferenceInfo->CoordLocation);

        // modelType
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "modelType") == 0 &&
                 BEXML_Success != pSubNode->GetContent(nodeContent) && 
                 BeStringUtilities::Wcsicmp(nodeContent.c_str(), L"FunctionalFitting") != 0)
            {
            HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
            po_pSDOSpatialReferenceInfo->IsSupported = false;
            goto WRAPUP;
            }

        // polynomialModel
        else if (BeStringUtilities::Stricmp(pSubNode->GetName(), "polynomialModel") == 0)
            {
            double AttValue;
            if(BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "rowOff") || AttValue != 0.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "columnOff") || AttValue != 0.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "xOff") || AttValue != 0.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "yOff") || AttValue != 0.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "zOff") || AttValue != 0.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "rowScale") || AttValue != 1.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "columnScale") || AttValue != 1.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "xScale") || AttValue != 1.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "yScale") || AttValue != 1.0 ||
               BEXML_Success != pSubNode->GetAttributeDoubleValue(AttValue, "zScale") || AttValue != 1.0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            BeXmlNodeP pPolynomial;
            // pPolynomial
            if ((pPolynomial = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "pPolynomial")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            BeXmlNodeP pCoefficients;
            if (BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "pType") || AttValue != 1 ||
                BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nVars") || AttValue != 2 ||
                BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "order") || AttValue != 1 ||
                BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nCoefficients") || AttValue != 3)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if ((pCoefficients = pPolynomial->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "polynomialCoefficients")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            pCoefficients->GetContent(nodeContent);

            po_pSDOSpatialReferenceInfo->pRowCoefficient = new double[3];
            HArrayAutoPtr<WChar> pValues(new WChar[nodeContent.length() + 1]);
            WCharP pBegin;
            WCharP pEnd;
            wcscpy(pValues, nodeContent.c_str());
            pBegin = pValues;
            errno = 0;
            po_pSDOSpatialReferenceInfo->pRowCoefficient[0] = wcstod(pBegin, &pEnd);
            if (pEnd == pBegin || errno != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }

            pBegin = pEnd;
            po_pSDOSpatialReferenceInfo->pRowCoefficient[1] = wcstod(pBegin, &pEnd);
            if (pEnd == pBegin || errno != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }

            pBegin = pEnd;
            po_pSDOSpatialReferenceInfo->pRowCoefficient[2] = wcstod(pBegin, &pEnd);
            if (pEnd == pBegin || errno != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }

            // qPolynomial
            if ((pPolynomial = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "qPolynomial")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if(BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "pType") || AttValue != 1 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nVars") || AttValue != 0 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "order") || AttValue != 0 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nCoefficients") || AttValue != 1)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if ((pCoefficients = pPolynomial->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "polynomialCoefficients")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            double Coefficient;
            if (BEXML_Success != pCoefficients->GetContentDoubleValue(Coefficient) || Coefficient != 1.0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            // rPolynomial
            if ((pPolynomial = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "rPolynomial")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if(BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "pType") || AttValue != 1 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nVars") || AttValue != 2 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "order") || AttValue != 1 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nCoefficients") || AttValue != 3)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if ((pCoefficients = pPolynomial->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "polynomialCoefficients")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            pCoefficients->GetContent(nodeContent);

            po_pSDOSpatialReferenceInfo->pColumnCoefficient = new double[3];
            pValues = new WChar[nodeContent.length() + 1];
            wcscpy(pValues, nodeContent.c_str());
            pBegin = pValues;
            errno = 0;
            po_pSDOSpatialReferenceInfo->pColumnCoefficient[0] = wcstod(pBegin, &pEnd);
            if (pEnd == pBegin || errno != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }

            pBegin = pEnd;
            po_pSDOSpatialReferenceInfo->pColumnCoefficient[1] = wcstod(pBegin, &pEnd);
            if (pEnd == pBegin || errno != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }

            pBegin = pEnd;
            po_pSDOSpatialReferenceInfo->pColumnCoefficient[2] = wcstod(pBegin, &pEnd);
            if (pEnd == pBegin || errno != 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : invalid XML header");
                po_pSDOSpatialReferenceInfo->IsValid = false;
                goto WRAPUP;
                }

            // sPolynomial
            if ((pPolynomial = pSubNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "sPolynomial")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if(BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "pType") || AttValue != 1 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nVars") || AttValue != 0 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "order") || AttValue != 0 ||
               BEXML_Success != pPolynomial->GetAttributeDoubleValue(AttValue, "nCoefficients") || AttValue != 1)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if ((pCoefficients = pPolynomial->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "polynomialCoefficients")) == 0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }

            if (BEXML_Success != pCoefficients->GetContentDoubleValue(Coefficient) || Coefficient != 1.0)
                {
                HASSERT(!L"HRFGeoRasterFile::ReadSpatialReferenceInfo() : unsupported XML header");
                po_pSDOSpatialReferenceInfo->IsSupported = false;
                goto WRAPUP;
                }
            }
        }

    if (pIsReferencedNode == 0 ||
        /* pIsRectifiedNode == 0 ||    TR 273265 FME create that field, but we don't know what to do with that presently */
        po_pSDOSpatialReferenceInfo->ScaleX == 0.0 ||
        po_pSDOSpatialReferenceInfo->ScaleY == 0.0 ||
        po_pSDOSpatialReferenceInfo->CoordLocation.empty() ||
        po_pSDOSpatialReferenceInfo->pRowCoefficient == 0 ||
        po_pSDOSpatialReferenceInfo->pColumnCoefficient == 0)
        {
        po_pSDOSpatialReferenceInfo->IsValid= false;
        po_pSDOSpatialReferenceInfo->IsSupported = false;
        }

WRAPUP:

    // need an instruction after WRAPUP label
    return;
    }

//-----------------------------------------------------------------------------
// private
// ReadLayerInfo
//-----------------------------------------------------------------------------
void HRFGeoRasterFile::ReadLayerInfo(BeXmlNodeP     pi_pLayerInfoNode,
                                     SDOLayerInfo*  po_pSDOLayerInfo)
    {
    HPRECONDITION(pi_pLayerInfoNode != 0);
    HPRECONDITION(po_pSDOLayerInfo != 0);

    po_pSDOLayerInfo->IsValid = true;
    po_pSDOLayerInfo->IsSupported = true;
    po_pSDOLayerInfo->PaletteEntryCount = 0;

    BeXmlNodeP pCurrentNode;
    if ((pCurrentNode = pi_pLayerInfoNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "sublayer")) != 0)
        {
        if ((pCurrentNode = pCurrentNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "colorMap")) != 0)
            {
            if ((pCurrentNode = pCurrentNode->SelectSingleNode(GEORASTER_NAMESPACE_PREFIX ":" "colors")) != 0)
                {
                for(BeXmlNodeP pSubNode = pCurrentNode->GetFirstChild (); NULL != pSubNode; pSubNode = pSubNode->GetNextSibling())
                    {
                    if(po_pSDOLayerInfo->PaletteEntryCount >= 255)
                        {
                        HASSERT(!L"HRFGeoRasterFile::CreateDescriptor() : unsupported color map");
                        po_pSDOLayerInfo->IsSupported = false;
                        goto WRAPUP;
                        }

                    // build an RGBA palette
                    uint32_t Entry;
                    uint32_t Red;
                    uint32_t Green;
                    uint32_t Blue;
                    uint32_t Alpha;

                    if (BEXML_Success != pSubNode->GetAttributeUInt32Value(Entry, "value") || Entry > 255 ||
                        BEXML_Success != pSubNode->GetAttributeUInt32Value(Red, "red") || Red > 255 ||
                        BEXML_Success != pSubNode->GetAttributeUInt32Value(Green, "green") || Green > 255 ||
                        BEXML_Success != pSubNode->GetAttributeUInt32Value(Blue, "blue") || Blue > 255)
                        {
                        HASSERT(!L"HRFGeoRasterFile::ReadLayerInfo() : invalid color map");
                        po_pSDOLayerInfo->IsSupported = false;
                        goto WRAPUP;
                        }

                    if (BEXML_Success != pSubNode->GetAttributeUInt32Value(Alpha, "alpha"))
                        Alpha = 255;
                    else if (Alpha > 255)
                        {
                        HASSERT(!L"HRFGeoRasterFile::ReadLayerInfo() : invalid color map");
                        po_pSDOLayerInfo->IsSupported = false;
                        goto WRAPUP;
                        }

                    po_pSDOLayerInfo->pPalette[Entry * 4]     = (Byte)Red;
                    po_pSDOLayerInfo->pPalette[Entry * 4 + 1] = (Byte)Green;
                    po_pSDOLayerInfo->pPalette[Entry * 4 + 2] = (Byte)Blue;
                    po_pSDOLayerInfo->pPalette[Entry * 4 + 3] = (Byte)Alpha;
                    ++po_pSDOLayerInfo->PaletteEntryCount;
                    }
                }
            }
        }

WRAPUP:
    return;    // compiler need something after WRAPUP
    }


//-----------------------------------------------------------------------------
// private
// GetGeocoding
//-----------------------------------------------------------------------------
RasterFileGeocodingPtr HRFGeoRasterFile::ExtractGeocodingInformation(SDOSpatialReferenceInfo const&     pi_rSpatialRefInfo)
    {
    RasterFileGeocodingPtr pGeocoding(RasterFileGeocoding::Create());

    // If the SRID is in the 0 to 32767 range it should be an EPSG code ..
    // We try to obtain the baseGCS directly from dictionary
    if ((pi_rSpatialRefInfo.SRID > 0) && (pi_rSpatialRefInfo.SRID < 32767))
        {
        WChar  TempBuffer[10];
        errno_t Err = _itow_s(pi_rSpatialRefInfo.SRID,
                              TempBuffer,
                              sizeof(TempBuffer) / sizeof(WChar),
                              10);

        HASSERT(Err == 0);

        WString EspgBasedKeyName(L"EPSG:");

        EspgBasedKeyName += WString(TempBuffer);

        try
            {
            IRasterBaseGcsPtr pRasterGcs = GCSServices->_CreateRasterBaseGcsFromKeyName(EspgBasedKeyName.c_str());
            pGeocoding = RasterFileGeocoding::Create(pRasterGcs.get());
            }
        catch(HFCException&)
            {
            }
        }

    // If the baseGCS was not determined ... we will try parsing the WKT
    if (GCSServices->_IsAvailable() && pGeocoding->GetGeocodingCP()==NULL)
        {
        WString WKTFromOracle;
        if (m_pSDOGeoRasterWrapper->GetWkt(pi_rSpatialRefInfo.SRID, WKTFromOracle))
            {
            try
                {
                IRasterBaseGcsPtr pRasterGcs = HRFGeoCoordinateProvider::CreateRasterGcsFromFromWKT(NULL, NULL, IRasterGeoCoordinateServices::WktFlavorOracle, WKTFromOracle.c_str());
                pGeocoding = RasterFileGeocoding::Create(pRasterGcs.get());
                }
            catch(HFCException&)
                {
                }
            }
        }
    return pGeocoding;
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFGeoRasterFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
    }
