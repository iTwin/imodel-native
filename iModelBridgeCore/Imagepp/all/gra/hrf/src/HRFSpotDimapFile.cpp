//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSpotDimapFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSpotDimapFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HRFSpotDimapFile.h>

#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>

#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>

#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecLZW.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>

#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <BeXml/BeXml.h>



#ifdef __HMR_DEBUG
#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>
double PRECISION_EPSILON = 0.0001999;
#endif

//-----------------------------------------------------------------------------
// HRFSpotDimapCodec1BitCapabilities
//-----------------------------------------------------------------------------

class HRFSpotDimapCodec1BitCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotDimapCodec1BitCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFSpotDimapCodecPaletteCapabilities
//-----------------------------------------------------------------------------

class HRFSpotDimapCodecPaletteCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotDimapCodecPaletteCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFSpotDimapCodecTrueColorCapabilities
//-----------------------------------------------------------------------------

class HRFSpotDimapCodecTrueColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotDimapCodecTrueColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFSpotDimapCodecV32RGBAlphaCapabilities
//-----------------------------------------------------------------------------

class HRFSpotDimapCodecV32RGBAlphaCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotDimapCodecV32RGBAlphaCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFSpotDimapCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------

class HRFSpotDimapCodec16BitsPerChannelCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotDimapCodec16BitsPerChannelCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFSpotDimapCodecFloatCapabilities
//-----------------------------------------------------------------------------

class HRFSpotDimapCodecFloatCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotDimapCodecFloatCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

/** ---------------------------------------------------------------------------
    HRFSpotDimapCapabilities
    ---------------------------------------------------------------------------
 */
HRFSpotDimapCapabilities::HRFSpotDimapCapabilities()
    :HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFSpotDimapCodec1BitCapabilities()));
    // PixelTypeV1Gray1
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFSpotDimapCodec1BitCapabilities()));
    // PixelTypeV1GrayWhite1
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV1GrayWhite1::CLASS_ID,
                                   new HRFSpotDimapCodec1BitCapabilities()));
    // PixelTypeV24R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFSpotDimapCodecTrueColorCapabilities()));

    // PixelTypeV32R8G8B8A8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFSpotDimapCodecV32RGBAlphaCapabilities()));

    // PixelTypeV32R8G8B8X8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8X8::CLASS_ID,
                                   new HRFSpotDimapCodecV32RGBAlphaCapabilities()));

    // HRPPixelTypeV32PR8PG8PB8A8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID,
                                   new HRFSpotDimapCodecV32RGBAlphaCapabilities()));

    // PixelTypeV8Gray8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFSpotDimapCodecTrueColorCapabilities()));
    // PixelTypeV8GrayWhite8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8GrayWhite8::CLASS_ID,
                                   new HRFSpotDimapCodecTrueColorCapabilities()));
    // PixelTypeI4R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI4R8G8B8::CLASS_ID,
                                   new HRFSpotDimapCodecPaletteCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFSpotDimapCodecPaletteCapabilities()));
    // PixelTypeI8VA8R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI8VA8R8G8B8::CLASS_ID,
                                   new HRFSpotDimapCodecPaletteCapabilities()));

    // PixelTypeV48R16G16B16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFSpotDimapCodec16BitsPerChannelCapabilities()));

    // PixelTypeV64R16G16B16A16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV64R16G16B16A16::CLASS_ID,
                                   new HRFSpotDimapCodec16BitsPerChannelCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV64R16G16B16X16::CLASS_ID,
                                   new HRFSpotDimapCodec16BitsPerChannelCapabilities()));

    // PixelTypeV16Gray16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFSpotDimapCodec16BitsPerChannelCapabilities()));

    // PixelTypeV16Int16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFSpotDimapCodec16BitsPerChannelCapabilities()));

    // PixelTypeV32Float32
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32Float32::CLASS_ID,
                                   new HRFSpotDimapCodecFloatCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

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

    // Multi Page capability
    Add(new HRFMultiPageCapability(HFC_READ_ONLY));
    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Embeding capability
    Add(new HRFEmbedingCapability(HFC_READ_ONLY));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeInkNames));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCopyright));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMinSampleValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMaxSampleValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageSlo(4)));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
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
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Transfo Model
    // Even if the format SPOT Dimap is only supported in read-only mode, the
    // transformation model can be modified when calling HRFGeoTiffFile::SetDefaultRatioToMeter.
    // This is a temporary fix since the way the format Spot DIMAP is handled need to be
    // modified.
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DProjective::CLASS_ID));
    }


HFC_IMPLEMENT_SINGLETON(HRFSpotDimapCreator)


/** ---------------------------------------------------------------------------
    Constructor.
    Creator.
    ---------------------------------------------------------------------------
 */
HRFSpotDimapCreator::HRFSpotDimapCreator()
    : HRFGeoTiffCreator()
    {
    // SpotDimap capabilities instance member initialization
    m_ClassID = HRFSpotDimapFile::CLASS_ID;
    }


/** ---------------------------------------------------------------------------
    Return file format label.

    @return string SpotDimap file format label.
    ---------------------------------------------------------------------------
 */
WString HRFSpotDimapCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_SpotDigital());
    }


/** ---------------------------------------------------------------------------
    Return file format scheme.

    @return string scheme of URL.
    ---------------------------------------------------------------------------
 */
WString HRFSpotDimapCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }


/** ---------------------------------------------------------------------------
    Return file format extension.

    @return string SpotDimap extension.
    ---------------------------------------------------------------------------
 */
WString HRFSpotDimapCreator::GetExtensions() const
    {
    return WString(L"*.dim");
    }


/** ---------------------------------------------------------------------------
    Open/Create SpotDimap raster file.

    @param pi_rpURL      File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.

    @return HFCPtr<HRFRasterFile> Address of the created HRFRasterFile instance.
    ---------------------------------------------------------------------------
 */
HFCPtr<HRFRasterFile> HRFSpotDimapCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                  HFCAccessMode         pi_AccessMode,
                                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFSpotDimapFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


/** ---------------------------------------------------------------------------
    Verify file validity.

    @param pi_rpURL  File's URL.
    @param pi_Offset Starting point in the file (not supported).

    @return true if the file is a valid SpotDimap file, false otherwise.
    ---------------------------------------------------------------------------
 */
bool HRFSpotDimapCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset) const
    {
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    WString XMLFileName;
    
    // Extract the standard file name from the main URL
    XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
    XMLFileName += L"\\";
    XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

    (const_cast<HRFSpotDimapCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager()); // will unlock implicitly at the end of method

    // Open XML file
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());
    if (pXmlDom.IsNull() || (BEXML_Success != xmlStatus))
        return false;
    
    // Read data in XML file (strings)
    BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
    if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "Dimap_Document") != 0)
        return false;

    // Validate VERSION node presence (for now only validate node presence)
    BeXmlNodeP pMetatDataNode = pMainNode->SelectSingleNode("Metadata_Id/METADATA_FORMAT");
    if(NULL == pMetatDataNode)
        return false;

    WString format;
    if(BEXML_Success != pMetatDataNode->GetContent(format) || BeStringUtilities::Wcsicmp (format.c_str(), L"DIMAP") != 0)
       return false;

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFSpotDimapCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFSpotDimapCreator*>(this))->m_pSharingControl = 0;

    return true;
    }

/** ---------------------------------------------------------------------------
    Get capabilities of the SpotDimap file format.

    @return SpotDimap format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFSpotDimapCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFSpotDimapCapabilities();

    return m_pCapabilities;
    }


/** ---------------------------------------------------------------------------
    Constructor.
    Open SpotDimap raster file.

    @param pi_rpURL      File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.
    ---------------------------------------------------------------------------
 */
HRFSpotDimapFile::HRFSpotDimapFile(const HFCPtr<HFCURL>& pi_rpURL,
                                          HFCAccessMode         pi_AccessMode,
                                          uint64_t             pi_Offset)
    : HRFGeoTiffFile(pi_rpURL, pi_AccessMode, pi_Offset, false)
    {

    // The ancestor stores the access mode
    m_IsOpen = false;


    Initialize();

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException( pi_rpURL->GetURL());
        }

    if(!Open())
        {
        throw HFCCorruptedFileException(pi_rpURL->GetURL());
        }
    else
        CreateDescriptors();


#ifdef __HMR_DEBUG_MEMBER
    //to know if both georef are similar
    XMLGeoRefSimilarToGeoTiff =  IsXMLGeoRefSimilarToGeoTiff(&m_pTiePoints[0], m_pTiePoints->size());
#endif


    }

/** ---------------------------------------------------------------------------
    Destructor.
    Destroy SpotDimap file object.
    ---------------------------------------------------------------------------
 */
HRFSpotDimapFile::~HRFSpotDimapFile()
    {
    // Nothing todo
    }

/** ---------------------------------------------------------------------------
    Create wrapper editor for all channel data manipulation (read).

       @param pi_Page        Page to create an editor for.
    @param pi_Resolution  Resolution.
    @param pi_AccessMode  Access and sharing mode.

    @return appropriate created resolution editor wrapper.
    ---------------------------------------------------------------------------
 */
HRFResolutionEditor* HRFSpotDimapFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                              unsigned short pi_Resolution,
                                                              HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return HRFGeoTiffFile::CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);
    }


/** ---------------------------------------------------------------------------
    Get capabilities of SpotDimap file format.

    @return SpotDimap file format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFSpotDimapFile::GetCapabilities () const
    {
    return HRFSpotDimapCreator::GetInstance()->GetCapabilities();
    }

/** ---------------------------------------------------------------------------

    Creator's IsKindOfFile() method should have been called first to
    validate files.

    @return true, raster files have been created.
    ---------------------------------------------------------------------------
 */
bool HRFSpotDimapFile::Open(bool pi_CreateBigTifFormat)
    {

    bool Result = true;
    // Open the file
    if (!m_IsOpen)
        {
        HFCPtr<HFCURL> pImageryFileURL = 0;

        if(ReadHeaderFromXMLFile())
            {
            pImageryFileURL = ComposeImageryURL();

            // Open files read-only


            if (!HRFGeoTiffFile::Open(pImageryFileURL))
                throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                            pImageryFileURL->GetURL());

            m_IsOpen = true;
            }
        else
            {
            Result = false;
            }
        }


    return Result;
    }


/** ---------------------------------------------------------------------------
    Create resolutions and page descriptors based on red channel raster file.
    ---------------------------------------------------------------------------
 */
void HRFSpotDimapFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    HRFGeoTiffFile::CreateDescriptors();

    }



/** ---------------------------------------------------------------------------
Close all channel raster files.
---------------------------------------------------------------------------
*/

bool HRFSpotDimapFile::ReadHeaderFromXMLFile()
    {
    WString XMLFileName;

    // Extract the standard file name from the main URL
    XMLFileName = ((HFCPtr<HFCURLFile>&)GetURL())->GetHost();
    XMLFileName += L"\\";
    XMLFileName += ((HFCPtr<HFCURLFile>&)GetURL())->GetPath();

    // Open XML file
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, XMLFileName.c_str());

    // Validate pDom
    if (pXmlDom.IsNull() || (BEXML_Success != xmlStatus))
        return false;
    
    // Read data in XML file (strings)
    BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
    if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "Dimap_Document") != 0)
        return false;

    BeXmlNodeP pDataFormatNode = pMainNode->SelectSingleNode("Data_Access/DATA_FILE_FORMAT");
    BeXmlNodeP pDataPathNode   = pMainNode->SelectSingleNode("Data_Access/Data_File/DATA_FILE_PATH");
    if (NULL == pDataPathNode)
        pDataPathNode = pMainNode->SelectSingleNode("Data_Access/Data_file/DATA_FILE_PATH");    // XML is now case sensitive - the 'F' of Data_File

    if(NULL == pDataFormatNode || NULL == pDataPathNode)
        return false;

    if(BEXML_Success != pDataFormatNode->GetContent(m_Header.DataFileFormat) ||
       BEXML_Success != pDataPathNode->GetAttributeStringValue(m_Header.DataFilePath, "href"))
       return false;

    
#ifdef __HMR_DEBUG_MEMBER
    //get the xml georef info to compare it later to the one from the GeoTiff header
    BeXmlNodeP pGeoPosNode = pMainNode->SelectSingleNode("Geoposition/Geoposition_Points");

    for(BeXmlNodeP pTiePointsNode = pGeoPosNode->GetFirstChild (); 
        NULL != pTiePointsNode; 
        pTiePointsNode = pTiePointsNode->GetNextSibling())
        {
        m_pTiePoints.push_back(pTiePointsNode->SelectSingleNode("TIE_POINT_DATA_X")->GetAttributeDoubleValue());
        m_pTiePoints.push_back(pTiePointsNode->SelectSingleNode("TIE_POINT_DATA_Y")->GetAttributeDoubleValue());
        m_pTiePoints.push_back(0.0);

        m_pTiePoints.push_back(pTiePointsNode->SelectSingleNode("TIE_POINT_CRS_X")->GetAttributeDoubleValue());
        m_pTiePoints.push_back(pTiePointsNode->SelectSingleNode("TIE_POINT_CRS_Y")->GetAttributeDoubleValue());
        m_pTiePoints.push_back(pTiePointsNode->SelectSingleNode("TIE_POINT_CRS_Z")->GetAttributeDoubleValue());
        }
#endif


    //we assume the values related to the image dimensions are the same in the xml and in the tif header
    /*
    XMLNode* pRasterDimensionsNode = pMainNode->GetChildNode("Raster_Dimensions");

    if(!pRasterDimensionsNode)
        return false;

    m_Header.NbCols = atol((pRasterDimensionsNode->GetChildNode("NCOLS")->GetValue()).c_str());
    m_Header.NbRows = atol((pRasterDimensionsNode->GetChildNode("NROWS")->GetValue()).c_str());
    m_Header.NbBands = atol((pRasterDimensionsNode->GetChildNode("NBANDS")->GetValue()).c_str());

    XMLNode* pRasterEncodingNode = pMainNode->GetChildNode("Raster_Encoding");

    if(!pRasterEncodingNode)
        return false;

    m_Header.NbBits = atol(pRasterEncodingNode->GetChildNode("NBITS")->GetValue().c_str());
    m_Header.Compression = pRasterEncodingNode->GetChildNode("COMPRESSION_NAME")->GetValue();
    */

    return true;
    }

/** ---------------------------------------------------------------------------
URL composition utility.

@return HFCPtr<HFCURLFile>
---------------------------------------------------------------------------
*/
HFCPtr<HFCURL> HRFSpotDimapFile::ComposeImageryURL()
    {
    //we need to read the XML header first
    HPRECONDITION(m_Header.DataFilePath.compare(L"") != 0);

    WString XMLFileName;
    HFCPtr<HFCURL> pImageryFileURL = 0;
    WString ImgFilePathStr;

    // Path is relative to xml file location
    WString Path = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetPath();
    WString Host = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetHost();
    WString FileName = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetFilename();

    WString::size_type FileNamePos = Path.rfind(FileName);

    if (FileNamePos != WString::npos)
        Path = Path.substr(0, FileNamePos);

    ImgFilePathStr = WString(HFCURLFile::s_SchemeName() + L"://")
                     + Host + L"\\"
                     + Path
                     + m_Header.DataFilePath;


    // Compose real URL
    try
        {
        pImageryFileURL = HFCURL::Instanciate(ImgFilePathStr);
        return pImageryFileURL;
        }
    catch(...)
        {
        return 0; // not a valid URL
        }
    }

/** ---------------------------------------------------------------------------
Initialize
initialize members
---------------------------------------------------------------------------
*/

void  HRFSpotDimapFile::Initialize()
    {
    m_Header.DataFileFormat = (L"");
    m_Header.DataFilePath = (L"");
    }

/** ---------------------------------------------------------------------------
    Get the world identificator of the SpotDimap file format.
    (Based on red channel raster file)
    ---------------------------------------------------------------------------
 */
const HGF2DWorldIdentificator HRFSpotDimapFile::GetWorldIdentificator () const
    {
    return HRFGeoTiffFile::GetWorldIdentificator();
    }


#ifdef __HMR_DEBUG_MEMBER
/** ---------------------------------------------------------------------------
IsXMLGeoRefSimilarToGeoTiff
Return true if the transfoModel Matrix created from the info in the xml header
is equivalent to the matrix
---------------------------------------------------------------------------
*/

bool   HRFSpotDimapFile::IsXMLGeoRefSimilarToGeoTiff(double* pi_TiePointsMatrix, unsigned short pi_NbVal_GeoTiePoint)
    {
    bool Result = false;

    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();

    double pMatrix[4][4];

    if(H_SUCCESS ==GetTransfoMatrixFromScaleAndTiePts(pMatrix, pi_NbVal_GeoTiePoint, pi_TiePointsMatrix, 0, NULL))
        {

        HFCMatrix<3, 3> TheXMLMatrix;

        TheXMLMatrix[0][0] = pMatrix[0][0];
        TheXMLMatrix[0][1] = pMatrix[0][1];
        TheXMLMatrix[0][2] = pMatrix[0][3];
        TheXMLMatrix[1][0] = pMatrix[1][0];
        TheXMLMatrix[1][1] = pMatrix[1][1];
        TheXMLMatrix[1][2] = pMatrix[1][3];
        TheXMLMatrix[2][0] = pMatrix[3][0];
        TheXMLMatrix[2][1] = pMatrix[3][1];
        TheXMLMatrix[2][2] = pMatrix[3][3];

        pTransfoModel = new HGF2DProjective(TheXMLMatrix);

        // Get the simplest model possible.
        HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();
        //we get the matrix back for comparison
        TheXMLMatrix = pTempTransfoModel->GetMatrix();

        //get the geotiff TransfoModel
        HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
        HFCPtr<HGF2DTransfoModel> pModel = pPageDescriptor->GetTransfoModel();
        HFCMatrix<3, 3> TheGeoTiffMatrix;

        // Check if the transformation can be represented by a matrix.
        if (pModel->CanBeRepresentedByAMatrix())
            {
            TheGeoTiffMatrix = pModel->GetMatrix();

            if(TheGeoTiffMatrix.IsEqualTo(TheXMLMatrix,PRECISION_EPSILON))
                Result = true;

            }
        }

    return Result;

    }

#endif




