//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTiffFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTiffFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFTiffTileEditor.h>
#include <Imagepp/all/h/HRFTiffStripEditor.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HTIFFDirectory.h>
#include <ImagePP/all/h/HTIFFGeoKey.h>
#include <Imagepp/all/h/HRPPixelType.h>

#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecJBIG.h>
#include <Imagepp/all/h/HCDCodecCCITTRLE.h>

#include <Imagepp/all/h/HCDCodecLZW.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>

#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPChannelOrgFloat.h>
#include <Imagepp/all/h/HRPChannelOrgGrayWhite.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>

#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgInt.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#define DEFAULT_STRIPSIZE                       (16384)     // 16k, represent 16k for an 8bits image
//      32k for an 16 bits image, .....



bool HRFTiffFile::s_BypassFileSharing = false;

//-----------------------------------------------------------------------------
// HRFTiffCodec1BitCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodec1BitCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCodec1BitCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFTiff1BitBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiff1BitBlockCapabilities()));

        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiff1BitBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiff1BitBlockCapabilities()));
#ifdef JBIG_SUPPORT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecJBIG::CLASS_ID,
                                   new HRFTiff1BitBlockCapabilities()));
#endif
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodecPaletteCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodecPaletteCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCodecPaletteCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        }
    };


//-----------------------------------------------------------------------------
// HRFTiffCodecTrueColorCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCMYKColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCMYKColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFTiffCodecTrueColorCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodecYCCColorCapabilities : public  HRFRasterFileCapabilities
    {
    public:
        // Constructor
        HRFTiffCodecYCCColorCapabilities()
            : HRFRasterFileCapabilities()
            {
            // YCC Jpeg is supported via v24R8G8B8 since jpeg decoder is doing the conversion internally.

            // Codec Identity
            Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                HCDCodecIdentity::CLASS_ID,
                new HRFTiffBlockCapabilities()));

            // Codec LZW
            Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                HCDCodecLZW::CLASS_ID,
                new HRFTiffBlockCapabilities()));
            }
    };



//-----------------------------------------------------------------------------
// HRFTiffCodecTrueColorCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodecTrueColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCodecTrueColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodecV32RGBAlphaCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodecV32RGBAlphaCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCodecV32RGBAlphaCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodec16BitsPerChannelCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCodec16BitsPerChannelCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------

class HRFTiffCodec16BitsPerChannelWithAlphaCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffCodec16BitsPerChannelWithAlphaCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodecFloatCapabilities
//-----------------------------------------------------------------------------

class HRFTiffLosslessCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffLosslessCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCapabilities
//-----------------------------------------------------------------------------
HRFTiffCapabilities::HRFTiffCapabilities()
    :HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFTiffCodec1BitCapabilities()));
    // PixelTypeV1Gray1
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFTiffCodec1BitCapabilities()));
    // PixelTypeV1GrayWhite1
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV1GrayWhite1::CLASS_ID,
                                   new HRFTiffCodec1BitCapabilities()));
    // PixelTypeV24R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFTiffCodecTrueColorCapabilities()));

    // PixelTypeV32R8G8B8A8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFTiffCodecV32RGBAlphaCapabilities()));

    // PixelTypeV32R8G8B8X8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV32R8G8B8X8::CLASS_ID,
                                   new HRFTiffCodecV32RGBAlphaCapabilities()));

    // HRPPixelTypeV32PR8PG8PB8A8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID,
                                   new HRFTiffCodecV32RGBAlphaCapabilities()));

    // HRPPixelTypeV32CMYK
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32CMYK::CLASS_ID,
                                   new HRFTiffCMYKColorCapabilities()));

    // PixelTypeV8Gray8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFTiffCodecTrueColorCapabilities()));
    // PixelTypeV8GrayWhite8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8GrayWhite8::CLASS_ID,
                                   new HRFTiffCodecTrueColorCapabilities()));
    // PixelTypeI4R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI4R8G8B8::CLASS_ID,
                                   new HRFTiffCodecPaletteCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFTiffCodecPaletteCapabilities()));
    // PixelTypeI8VA8R8G8B8
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8VA8R8G8B8::CLASS_ID,
                                   new HRFTiffCodecPaletteCapabilities()));

    // PixelTypeV48R16G16B16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFTiffCodec16BitsPerChannelCapabilities()));

    // PixelTypeV64R16G16B16A16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV64R16G16B16A16::CLASS_ID,
                                   new HRFTiffCodec16BitsPerChannelWithAlphaCapabilities()));

    // PixelTypeV64R16G16B16X16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV64R16G16B16X16::CLASS_ID,
                                   new HRFTiffCodec16BitsPerChannelCapabilities()));

    // PixelTypeV16Gray16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFTiffCodec16BitsPerChannelCapabilities()));

    // PixelTypeV16Int16
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFTiffLosslessCodecCapabilities()));

    // PixelTypeV96R32G32B32
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV96R32G32B32::CLASS_ID,
                                   new HRFTiffCodecPaletteCapabilities()));

    // PixelTypeV32Float32
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32Float32::CLASS_ID,
                                   new HRFTiffLosslessCodecCapabilities()));

    // PixelTypeV24PhotoYCC
    // Read/Write/Create capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24PhotoYCC::CLASS_ID,
                                   new HRFTiffCodecYCCColorCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // MultiResolution Capability
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_WRITE_CREATE, // AccessMode,
        true,                  // SinglePixelType,
        true,                  // SingleBlockType,
        false,                 // ArbitaryXRatio,
        false);                // ArbitaryYRatio);
    Add(pMultiResolutionCapability);

    // Multi Page capability
    Add(new HRFMultiPageCapability(HFC_READ_WRITE_CREATE));
    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Embeding capability
    Add(new HRFEmbedingCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMake));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeModel));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeInkNames));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeCopyright));

    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeImageSlo(4)));

    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMinSampleValue));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMaxSampleValue));

    //GPS tags
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSVersionID));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLatitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLatitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLongitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSLongitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSAltitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSAltitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSTimeStamp));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSSatellites));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSStatus));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSMeasureMode));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDOP));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSSpeedRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSSpeed));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSTrackRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSTrack));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSImgDirectionRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSImgDirection));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSMapDatum));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLatitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLatitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLongitudeRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestLongitude));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestBearingRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestBearing));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestDistanceRef));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDestDistance));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSProcessingMethod));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSAreaInformation));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDateStamp));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGPSDifferential));

    //Based on the EXIF tags
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFNumber));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureProgram));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSpectralSensitivity));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeISOSpeedRatings));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeOptoElectricConversionFactor));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExifVersion));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTimeOriginal));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTimeDigitized));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeComponentsConfiguration));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCompressedBitsPerPixel));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeShutterSpeedValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeApertureValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBrightnessValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureBiasValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMaxApertureValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectDistance));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMeteringMode));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeLightSource));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFlash));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalLength));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectArea));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMakerNote));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeUserComment));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubSecTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubSecTimeOriginal));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubSecTimeDigitized));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFlashpixVersion));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeColorSpace));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePixelXDimension));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePixelYDimension));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeRelatedSoundFile));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFlashEnergy));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSpatialFrequencyResponse));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalPlaneXResolution));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalPlaneYResolution));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalPlaneResolutionUnit));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectLocation));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureIndex));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSensingMethod));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFileSource));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSceneType));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCFAPattern));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCustomRendered));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeExposureMode));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeWhiteBalance));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDigitalZoomRatio));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeFocalLengthIn35mmFilm));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSceneCaptureType));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeGainControl));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeContrast));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSaturation));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSharpness));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDeviceSettingDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSubjectDistanceRange));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageUniqueID));
    }

HFC_IMPLEMENT_SINGLETON(HRFTiffCreator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFTiffCreator::HRFTiffCreator()
    : HRFRasterFileCreator(HRFTiffFile::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFTiffCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_Tiff());  // Tagged Image File Format (TIFF)
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFTiffCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFTiffCreator::GetExtensions() const
    {
    return WString(L"*.tif;*.tiff");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFTiffCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFTiffFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFTiffCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    HAutoPtr<HTIFFFile>  pTiff;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);

    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    (const_cast<HRFTiffCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if (pTiff->IsValid(&pErr) || ((pErr != 0) && !pErr->IsFatal()))
        {
        // the tiff was opened successfully, verify if it is
        // a real tiff, or a HMR file.
        bResult = true;

        // To detect if it is a HMR file, verify if the private tag is present
        // if so, set the result to false
        if ((pTiff->TagIsPresent (HMR_IMAGEINFORMATION)) ||
            (pTiff->TagIsPresent (HMR2_IMAGEINFORMATION)))
            bResult = false;
        else
            {
            // Check if this is not a Geo tiff
            double*    pMat4by4;
            double*    pTiePoint;
            double*    pScale;
            uint32_t    Count;
            unsigned short* pKeyDirectory;
            unsigned short*    pTagRagBag;
            uint32_t KeyCount;

            if (pTiff->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4))
                {
                // If the Matrix is valid..
                if (IsValidDoubleArray  (pMat4by4, Count))
                    bResult = false;
                }
            else if ((pTiff->GetField(GEOTRANSMATRIX, &Count, &pMat4by4)     ||
                      pTiff->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag) ||
                      pTiff->GetField(GEOTIEPOINTS, &Count, &pTiePoint)       ||
                      pTiff->GetField(GEOPIXELSCALE, &Count, &pScale)         ||
                      pTiff->GetField(GEOKEYDIRECTORY, &KeyCount, &pKeyDirectory)))
                bResult = false;
            else
                {
                // Declare the current pixel type and codec
                HFCPtr<HRPPixelType> CurrentPixelType;
                HFCPtr<HCDCodec>     CurrentCodec;
                try
                    {
                    // Create the current pixel type and codec
                    CurrentPixelType = HRFTiffFile::CreatePixelTypeFromFile(pTiff);
                    CurrentCodec     = HRFTiffFile::CreateCodecFromFile(pTiff);
                    }

                catch (...)
                    {
                    bResult = false;
                    }

                if (bResult)
                    {
                    // Create the codec list to be attach to the PixelType Capability.
                    HFCPtr<HRFRasterFileCapabilities> pCurrentCodecCapability = new HRFRasterFileCapabilities();
                    pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                        CurrentCodec->GetClassID(),
                                                                        new HRFTiffBlockCapabilities()));

                    // Create the capability for the current pixel type and codec
                    HFCPtr<HRFCapability> pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                                                            CurrentPixelType->GetClassID(),
                                                                                            pCurrentCodecCapability);

                    // Check if we support these pixel type and codec
                    bResult = ((HRFRasterFileCreator*)this)->GetCapabilities()->Supports(pPixelTypeCapability);
                    }
                }
            }

        }
    else
        bResult = false;

    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFTiffCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFTiffCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

// Create or get the singleton capabilities of tiff file.
const HFCPtr<HRFRasterFileCapabilities>& HRFTiffCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFTiffCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------

HRFTiffFile::~HRFTiffFile()
    {
    try
        {
        // Close the file
        SaveTiffFile(true);
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// Public CreateResolutionEditor
//-----------------------------------------------------------------------------

HRFResolutionEditor* HRFTiffFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                         unsigned short pi_Resolution,
                                                         HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE)
        pEditor = new HRFTiffTileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        pEditor = new HRFTiffStripEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }


//-----------------------------------------------------------------------------
// Public Save
//-----------------------------------------------------------------------------

void HRFTiffFile::Save()
    {
    SaveTiffFile(false);
    }

//-----------------------------------------------------------------------------
// Public AddPage
//-----------------------------------------------------------------------------

bool HRFTiffFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess );

    // Validation with the capabilities if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);

    HPRECONDITION(pi_pPage->CountResolutions() > 0 || pi_pPage->IsEmpty());

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    // add the main resolution for the specified page
    if ((CountPages()-1) == 0)
        AddResolutionToFile (0, 0);
    else
        AddResolutionToFile(CountPages() - 1, 0);

    // Add subresolution
    for (unsigned short Resolution=1; Resolution < pi_pPage->CountResolutions(); Resolution++)
        AddResolutionToFile(CountPages() - 1, Resolution);

    return true;
    }

//-----------------------------------------------------------------------------
// Public GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFTiffFile::GetCapabilities () const
    {
    return HRFTiffCreator::GetInstance()->GetCapabilities();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MarcBedard  8/2006
+---------------+---------------+---------------+---------------+---------------+------*/
HRFScanlineOrientation HRFTiffFile::GetScanLineOrientation() const
    {
    unsigned short Orientation;
    if (!GetFilePtr()->GetField(ORIENTATION, &Orientation))
        Orientation = 1;    // by default, UPPER LEFT HORIZONTAL (SLO4)

    // this convert is made by IRasB
    HRFScanlineOrientation scanLineOrientation;
    switch(Orientation)
        {
        case 1:
            scanLineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            break;
        case 2:
            scanLineOrientation = HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL;
            break;
        case 3:
            scanLineOrientation = HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL;
            break;
        case 4:
            scanLineOrientation = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;
            break;
        case 5:
            scanLineOrientation = HRFScanlineOrientation::UPPER_LEFT_VERTICAL;
            break;
        case 6:
            scanLineOrientation = HRFScanlineOrientation::UPPER_RIGHT_VERTICAL;
            break;
        case 7:
            scanLineOrientation = HRFScanlineOrientation::LOWER_RIGHT_VERTICAL;
            break;
        case 8:
            scanLineOrientation = HRFScanlineOrientation::LOWER_LEFT_VERTICAL;
            break;
        default:
            scanLineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
        }

    return scanLineOrientation;
    }


//-----------------------------------------------------------------------------
// Public ResetGeokeys
// Erase the geokey list.
//-----------------------------------------------------------------------------
void HRFTiffFile::ResetGeokeys(uint32_t pi_Page)
    {
    // Backup the current directory
    HTIFFFile::DirectoryID CurrentDir(GetFilePtr()->CurrentDirectory());

    // Select the page and reset the geokeys
    GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, GetIndexOfPage(pi_Page)));
    GetFilePtr()->GetGeoKeyInterpretation().Reset();

    // Restore the current directory
    SetDirectory(CurrentDir);
    }

//-----------------------------------------------------------------------------
// protected CalcNumberOfPage
//-----------------------------------------------------------------------------

uint32_t HRFTiffFile::CalcNumberOfPage () const
    {
    uint32_t i;
    uint32_t NumberPage (0);

    // Count the number of subImage with the Type PAGE.
    for (i=0; i<m_NumberDir; i++)
        {
        // Presently, we consider PAGE and FULLIMAGE at the same level
        if (m_pDirectories[i].Type == FULLIMAGE ||
            m_pDirectories[i].Type == PAGE ||
            m_pDirectories[i].Type == EMPTYPAGE)
            NumberPage++;
        }

    // if the number of pages is 0, return 1
    if (NumberPage == 0)
        NumberPage = 1;

    return (NumberPage);
    }

//-----------------------------------------------------------------------------
// protected CalcNumberOfSubResolution
//-----------------------------------------------------------------------------

unsigned short HRFTiffFile::CalcNumberOfSubResolution(uint32_t pi_IndexImage) const
    {
    unsigned short NbSubResolution (0);

    HPRECONDITION (pi_IndexImage < (uint32_t)m_NumberDir);

    // Set on the selected Page.
    if (pi_IndexImage < m_NumberDir)
        {
        pi_IndexImage++;
        // Scan the next directories
        for (; pi_IndexImage < m_NumberDir; pi_IndexImage++)
            {
            if (m_pDirectories[pi_IndexImage].Type == REDUCEDIMAGE)
                NbSubResolution++;
            else
                break;
            }
        }
    return NbSubResolution;
    }

//-----------------------------------------------------------------------------
// protected GetIndexOfPage
//-----------------------------------------------------------------------------

uint32_t HRFTiffFile::GetIndexOfPage(uint32_t pi_Page) const
    {
    uint32_t i;
    uint32_t PageNumber (0);

    for (i=0; i<m_NumberDir; i++)
        {
        // Presently, we consider PAGE and FULLIMAGE at the same level
        //
        if (m_pDirectories[i].Type == FULLIMAGE ||
            m_pDirectories[i].Type == PAGE      ||
            m_pDirectories[i].Type == EMPTYPAGE)
            {
            if (PageNumber == pi_Page)
                break;

            PageNumber++;
            }
        }
    return ((i >= m_NumberDir) ? ULONG_MAX : i);
    }

//-----------------------------------------------------------------------------
// protected SetImageOnSubImage
//-----------------------------------------------------------------------------


void HRFTiffFile::SetImageInSubImage  (uint32_t pi_IndexImage)
    {
    HPRECONDITION (m_pTiff != 0);

    // Check if outside of page.
    // Should be a precondition, cause the caller can check before calling
    if (pi_IndexImage > m_NumberDir)
        throw HRFBadPageNumberException(GetURL()->GetURL());

    // Change the sub-image only if we are not already in the wanted image
    HTIFFFile::DirectoryID NewDirID = HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, pi_IndexImage);
    if (m_pTiff->CurrentDirectory() != NewDirID)
        {
        // Set the Subimage (TIFF Directory)
        m_pTiff->SetDirectory(NewDirID);
        }
    }

//-----------------------------------------------------------------------------
// private Set1BitPhotometric
//-----------------------------------------------------------------------------
void HRFTiffFile::Set1BitPhotometric(bool pi_MinIsWhite)
    {
    if (pi_MinIsWhite)
        GetFilePtr()->SetField(PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISWHITE);
    else
        GetFilePtr()->SetField(PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
    }

//-----------------------------------------------------------------------------
// private CompactITIFF
//-----------------------------------------------------------------------------
bool HRFTiffFile::CompactITIFF()
    {
    return GetFilePtr()->CompactITIFF();
    }

//-----------------------------------------------------------------------------
// private ScanDirectories
//-----------------------------------------------------------------------------

void HRFTiffFile::ScanDirectories ()
    {
    uint32_t SubImage;
    uint32_t PreviousDirectory;
    uint32_t ImageType;

    HPRECONDITION (m_pTiff != 0);

    PreviousDirectory = m_pTiff->CurrentDirectory();

    // Alloc an entry for each Directory
    m_NumberDir = m_pTiff->NumberOfDirectory();
    m_pDirectories = new DirectoryEntries[m_NumberDir];

    // Scan all directory, keep Offset and type.
    for (SubImage=0L; SubImage<m_NumberDir; SubImage++)
        {
        m_pTiff->SetDirectory(SubImage);

        if (m_pTiff->GetField (SUBFILETYPE, &ImageType))
            {
            switch (ImageType)
                {
                case FILETYPE_REDUCEDIMAGE:
                    m_pDirectories[SubImage].Type = REDUCEDIMAGE;
                    break;

                case FILETYPE_MASK:
                    m_pDirectories[SubImage].Type = MASK;
                    break;

                case FILETYPE_PAGE:
                case FILETYPE_PAGE+FILETYPE_REDUCEDIMAGE:
                    m_pDirectories[SubImage].Type = PAGE;

                default:
                    m_pDirectories[SubImage].Type = FULLIMAGE;
                    break;
                }
            }
        else
            m_pDirectories[SubImage].Type = FULLIMAGE;
        }

    // Reset Directory
    m_pTiff->SetDirectory(PreviousDirectory);
    }

//-----------------------------------------------------------------------------
// Protected GetSystemDateTime -
// Return current Date and hour in a string(len 20)
//-----------------------------------------------------------------------------

void HRFTiffFile::GetSystemDateTime (char* datetime)
    {
    time_t timer;
    struct tm gm;

    time (&timer);
    gm = *localtime (&timer);

    sprintf ((char*)datetime, "%4d:%02d:%02d %02d:%02d:%02d",1900+gm.tm_year,
             1+gm.tm_mon,
             gm.tm_mday,
             gm.tm_hour,
             gm.tm_min,
             gm.tm_sec);
    }

//-----------------------------------------------------------------------------
// protected
// AddResolutionToFile -
//-----------------------------------------------------------------------------
void HRFTiffFile::AddResolutionToFile(uint32_t pi_Page,
                                      unsigned short pi_Resolution)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess );

    char aDateTime[20];
    uint32_t ResolutionType=0;
    HFCPtr<HRFPageDescriptor>       pPageDescriptor       = GetPageDescriptor(pi_Page);

    // Find the resolution type with the specified page and resolution number
    if (pPageDescriptor->IsEmpty())
        {
        HPRECONDITION(pi_Resolution == 0);
        ResolutionType = FILETYPE_EMPTYPAGE;
        }
    else if ((pi_Page == 0) && (pi_Resolution == 0))
        ResolutionType = 0; // First Page Added
    else if ((pi_Page != 0) && (pi_Resolution == 0))
        ResolutionType = FILETYPE_PAGE;
    else if (pi_Resolution != 0)
        ResolutionType = FILETYPE_REDUCEDIMAGE;

    if (!m_pTiff->AppendDirectory())
        {
        HASSERT(false);
        }

    // Info standard
    m_pTiff->SetField (SUBFILETYPE, (uint32_t)ResolutionType);
    m_pTiff->SetField (PLANARCONFIG, (unsigned short)PLANARCONFIG_CONTIG);

    if (ResolutionType == FILETYPE_EMPTYPAGE)
        {
        if (pi_Page == 0)
            {
            // Set the image size
            m_pTiff->SetField (IMAGEWIDTH, (uint32_t)0);
            m_pTiff->SetField (IMAGELENGTH, (uint32_t)0);

            GetSystemDateTime (aDateTime);
            m_pTiff->SetFieldA (DATETIME, aDateTime);
            }
        }
    else
        {
        HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(pi_Resolution);

        // Set DateTime if main image or Page
        if ((ResolutionType == 0) || ((ResolutionType & FILETYPE_PAGE) != 0))
            {
            GetSystemDateTime (aDateTime);
            m_pTiff->SetFieldA (DATETIME, aDateTime);
            }

        // Set the image size
        m_pTiff->SetField (IMAGEWIDTH, (uint32_t)pResolutionDescriptor->GetWidth());
        m_pTiff->SetField (IMAGELENGTH, (uint32_t)pResolutionDescriptor->GetHeight());

        // if the file is to be tiled, set the tile size now
        if (pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE)
            {
            m_pTiff->SetField (TILEWIDTH, pResolutionDescriptor->GetBlockWidth());
            m_pTiff->SetField (TILELENGTH, pResolutionDescriptor->GetBlockHeight());
            }
        else
            {
            // Strip, set row by strip
            m_pTiff->SetField (ROWSPERSTRIP, pResolutionDescriptor->GetBlockHeight());
            }


        // Set mandatory tags if present.
        HPMAttributeSet::HPMASiterator TagIterator;

        for (TagIterator  = pPageDescriptor->GetTags().begin();
             TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
            {
            HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

            // Sample Minimum Value Tag
            if (pTag->GetID() == HRFAttributeMinSampleValue::ATTRIBUTE_ID) 
                {
                WriteSampleLimitValueToDir(static_cast<HRFAttributeMinSampleValue*>(pTag.GetPtr())->GetData(),
                                           true,
                                           GetFilePtr());
                }

            // Sample Minimum Value Tag
            else if (pTag->GetID() == HRFAttributeMaxSampleValue::ATTRIBUTE_ID)
                {
                WriteSampleLimitValueToDir(static_cast<HRFAttributeMaxSampleValue*>(pTag.GetPtr())->GetData(),
                                           false,
                                           GetFilePtr());
                }
            }

        // set JPEG codec embeded
        // If we have a HCDCodecIJG we need to set its embeded.
        if (pResolutionDescriptor->GetCodec()->GetClassID() == HCDCodecIJG::CLASS_ID)
            ((HFCPtr<HCDCodecIJG>&)pResolutionDescriptor->GetCodec())->SetAbbreviateMode(true);

        // Write the pixel type and Palette to the file
        WritePixelTypeAndCodecToFile(pi_Page,
                                     pResolutionDescriptor->GetPixelType(),
                                     pResolutionDescriptor->GetCodec(),
                                     pResolutionDescriptor->GetBlockWidth(),
                                     pResolutionDescriptor->GetBlockHeight());

        // Set the TransfoModel on page
        if ((pi_Resolution == 0) && (pPageDescriptor->HasTransfoModel()))
            WriteTransfoModel(pPageDescriptor->GetTransfoModel());
        }

    // Build the directory list
    m_pDirectories = 0;
    ScanDirectories ();
    }

//-----------------------------------------------------------------------------
// protected
// AddResolutionToFile -
//-----------------------------------------------------------------------------
HFCPtr<HRFThumbnail> HRFTiffFile::ReadThumbnailFromFile(uint32_t pi_Page)
    {
    HFCPtr<HRFThumbnail> pThumbnail;

    // Get HMR directirie information.
    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();
    // Check if the private tag is present
    //
    // check if the Thumbnail is Present
    unsigned short IsThumbnailComposed = false;

    if (m_pTiff->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)) && m_pTiff->GetField(HMR_THUMBNAIL_COMPOSED, &IsThumbnailComposed))
        {
        // Read the thumbnail descriptor

        // DEBUG MODE ONLY
        // JPEG ISO Compression
        // Select the page
#if 0
        HDEBUGCODE(
            unsigned short TESTJPEGISOCompression =  = COMPRESSION_NONE;
            GetFilePtr()->GetField(COMPRESSION, &TESTJPEGISOCompression);

            if (TESTJPEGISOCompression == COMPRESSION_JPEG)
        {
        uint32_t TESTJPEGISOBlockWidth (0);
            uint32_t TESTJPEGISOBlockHeight (0);

            uint32_t TESTJPEGISOLength;
            GetFilePtr()->GetField(IMAGEWIDTH, &TESTJPEGISOBlockWidth);
            GetFilePtr()->GetField(ROWSPERSTRIP, &TESTJPEGISOBlockHeight);
            GetFilePtr()->GetField(IMAGELENGTH, &TESTJPEGISOLength);

            if (TESTJPEGISOBlockHeight < TESTJPEGISOLength)
                HASSERT(TESTJPEGISOBlockHeight % 16 == 0);

            HASSERT(TESTJPEGISOBlockWidth <= 65535);
            HASSERT(TESTJPEGISOBlockHeight <= 65535);
            }
        )
#endif
        // Pixel Type
        HFCPtr<HRPPixelType> pPixelType;

        try
            {
            pPixelType = CreatePixelTypeFromFile(GetFilePtr(), pi_Page);
            }

        catch (...)
            {
            }

        // if we have a pixel type in HMR DIR the thumbnail is present
        if (pPixelType != 0)
            {
            // resolution dimension
            uint32_t Width;
            uint32_t Height;
            GetFilePtr()->GetField(IMAGEWIDTH, &Width);
            GetFilePtr()->GetField(IMAGELENGTH, &Height);

            // Strip dimension
            GetFilePtr()->GetField(ROWSPERSTRIP, &Height);


            // Read the thumbnail data

            uint32_t BitsAlignment = 8;
            uint32_t BytesPerWidth = ((pPixelType->CountPixelRawDataBits() * Width) + (BitsAlignment-1)) / BitsAlignment * BitsAlignment / 8;
            uint32_t SizeInBytes   = BytesPerWidth * Height;
            HArrayAutoPtr<Byte> pPixels(new Byte[SizeInBytes]);

            // Lock the sister file for the GetField operation
            HFCLockMonitor SisterFileLock (GetLockManager());

            if (m_pTiff->StripRead(pPixels, 0) == H_SUCCESS)
                pThumbnail = new HRFThumbnail(Width, Height, pPixelType, pPixels, HFC_READ_WRITE, IsThumbnailComposed != 0);

            // Unlock the sister file
            SisterFileLock.ReleaseKey();
            }
        }

    // Reset Directory
    GetFilePtr()->SetDirectory(CurDir);

    return pThumbnail;
    }

//-----------------------------------------------------------------------------
// protected
// AddResolutionToFile -
// This function must be call only after the add resolution
//-----------------------------------------------------------------------------
void HRFTiffFile::AddThumbnailToFile(uint32_t pi_Page)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);
    HFCPtr<HRFPageDescriptor>  pPageDescriptor = GetPageDescriptor(pi_Page);

    if (pPageDescriptor->HasThumbnail())
        {
        HFCPtr<HRFThumbnail> pThumbnail = pPageDescriptor->GetThumbnail();

        // Get HMR directirie information.
        HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();
        // Check if the private tag is present
        //
        HTIFFFile::DirectoryID HMRDirID = HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page);
        if (m_pTiff->SetDirectory(HMRDirID))
            {

            //if (!m_pTiff->AppendDirectory())
            //    HASSERT(false);

            m_pTiff->SetField(HMR_THUMBNAIL_COMPOSED, (unsigned short)pThumbnail->IsComposed());

            // Write the thumbnail descriptor
            //UInt32 ResolutionType = FILETYPE_THUMBNAIL;

            // Info standard
            //m_pTiff->SetField (SUBFILETYPE, (UInt32)ResolutionType);
            m_pTiff->SetField (PLANARCONFIG, (unsigned short)PLANARCONFIG_CONTIG);

            // Set the image size
            m_pTiff->SetField (IMAGEWIDTH, pThumbnail->GetWidth());
            m_pTiff->SetField (IMAGELENGTH, pThumbnail->GetHeight());

            // Strip, set row by strip
            m_pTiff->SetField (ROWSPERSTRIP, pThumbnail->GetHeight());

            // Write the compression to the file
            // Write the pixel type and Palette to the file
            WritePixelTypeAndCodecToFile(pi_Page,
                                         pThumbnail->GetPixelType(),
                                         0,
                                         pThumbnail->GetWidth(),
                                         pThumbnail->GetHeight());

            // Build the directory list
            //m_pDirectories = 0;
            //ScanDirectories ();


            // Write the thumbnail data
            // Reset Directory
            GetFilePtr()->SetDirectory(CurDir);
            // Reset Directory
            GetFilePtr()->SetDirectory(HMRDirID);

            HArrayAutoPtr<Byte> pPixels(new Byte[pThumbnail->GetSizeInBytes()]);
            pThumbnail->Read(pPixels);

            // Lock the sister file for the GetField operation
            HFCLockMonitor SisterFileLock (GetLockManager());

            m_pTiff->StripWrite(pPixels, 0);

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        // Reset Directory
        GetFilePtr()->SetDirectory(CurDir);
        }
    }

//-----------------------------------------------------------------------------
// protected
// WritePixelTypeAndCodecToFile
//-----------------------------------------------------------------------------
void HRFTiffFile::WritePixelTypeAndCodecToFile(uint32_t                    pi_Page,
                                               const HFCPtr<HRPPixelType>& pi_rpPixelType,
                                               const HFCPtr<HCDCodec>&     pi_rpCodec,
                                               uint32_t                    pi_BlockWidth,
                                               uint32_t                    pi_BlockHeight)
    {
    // Find the compression tag that fit with the specified codec.
    unsigned short Compression = COMPRESSION_NONE;
    uint32_t Quality     = 0;

    if (pi_rpCodec != 0)
        {
        if (pi_rpCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
            {
            Compression = COMPRESSION_JPEG;
            Quality     = ((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetQuality();

            unsigned short HorizontalSub;
            unsigned short VerticalSub;
            if(((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetSubsamplingMode() == HCDCodecIJG::SNONE)
                {
                HorizontalSub = 1;
                VerticalSub = 1;
                m_pTiff->SetField(YCBCRSUBSAMPLING, HorizontalSub, VerticalSub);
                }
            else if(((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetSubsamplingMode() == HCDCodecIJG::S411)
                {
                HorizontalSub = 2;
                VerticalSub = 2;
                // Don't set the field, [2,2] is the default.
                }
            else
                {
                HorizontalSub = 1;
                VerticalSub = 2;
                m_pTiff->SetField(YCBCRSUBSAMPLING, HorizontalSub, VerticalSub);
                }

            if(((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetOptimizeCoding())
                {
                unsigned short ShortVal = 0;
                m_pTiff->SetField(COMPRESSION_JPEGOPTIMIZECODING, ShortVal);
                }

            // JPEG table Embeded or in a table ?
            if (((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->GetAbbreviateMode())
                {
                HFCPtr<HCDCodecIJG> pCodec((HCDCodecIJG*)((HFCPtr<HCDCodecIJG>&)pi_rpCodec)->Clone());
                pCodec->SetDimensions(pi_BlockWidth, pi_BlockHeight);

                // Save Table in file.
                HAutoPtr<Byte> pTable(new Byte[1024]);
                uint32_t        CodecTableSize;
                CodecTableSize = pCodec->CreateTables(pTable, 1024);

                if (CodecTableSize > 0)
                    m_pTiff->SetField (JPEGTABLES, CodecTableSize, pTable);
                }
            }
#ifdef JBIG_SUPPORT
        if (pi_rpCodec->GetClassID() == HCDCodecJBIG::CLASS_ID)
            Compression = COMPRESSION_JBIG;
#endif

        if (pi_rpCodec->GetClassID() == HCDCodecHMRCCITT::CLASS_ID)
            Compression = COMPRESSION_CCITTFAX4;

        if (pi_rpCodec->GetClassID() == HCDCodecZlib::CLASS_ID)
            Compression = COMPRESSION_DEFLATE;

        if (pi_rpCodec->GetClassID() == HCDCodecLZW::CLASS_ID)
            Compression = COMPRESSION_LZW;

        /*HChrMM if (pi_rpCodec->GetClassID() == HCDCodecHMRLZW::CLASS_ID)
            Compression = COMPRESSION_LZW;*/

        if (pi_rpCodec->GetClassID() == HCDCodecHMRPackBits::CLASS_ID)
            Compression = COMPRESSION_PACKBITS;

        if (pi_rpCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
            {
            HFCPtr<HCDCodecFlashpix> pCodec((HCDCodecFlashpix*)((HFCPtr<HCDCodecFlashpix>&)pi_rpCodec));

            Compression = COMPRESSION_HMR_FLASHPIX;
            Quality = pCodec->GetQuality();

            uint32_t EncoderTable = pCodec->GetEncoderTable();
            Byte* pTable = (Byte*)pCodec->GetTable(EncoderTable);
            uint32_t CodecTableSize = pCodec->GetTableSize(EncoderTable);

            if (CodecTableSize > 0)
                m_pTiff->SetField(JPEGTABLES, CodecTableSize, pTable);
            }

        if (pi_rpCodec->GetClassID() == HCDCodecHMRRLE1::CLASS_ID)
            Compression = COMPRESSION_HMR_RLE1;
        }

    // Set the compression
    HASSERT(Compression <= USHRT_MAX);
    m_pTiff->SetField (COMPRESSION, (unsigned short)Compression);
    if (Quality != 0)
        m_pTiff->SetField (COMPRESSION_QUALITY, Quality);

    // Set PixelType to the header file
    if (pi_rpPixelType->CountIndexBits() == 0)
        {
        // Image RGB
        m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_RGB);

        if (pi_rpPixelType->GetClassID() == HRPPixelTypeV24R8G8B8::CLASS_ID ||
            pi_rpPixelType->GetClassID() == HRPPixelTypeV24B8G8R8::CLASS_ID)
            {
            unsigned short BitPerSample[3];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;
            BitPerSample[2] = 8;

            if(Compression == COMPRESSION_JPEG || Compression == COMPRESSION_HMR_FLASHPIX)
                m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_YCBCR);

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 3, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)3);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV32R8G8B8A8::CLASS_ID ||
                 pi_rpPixelType->GetClassID() == HRPPixelTypeV32R8G8B8X8::CLASS_ID ||
                 pi_rpPixelType->GetClassID() == HRPPixelTypeV32B8G8R8X8::CLASS_ID)
            {
            unsigned short ExtraSample = EXTRASAMPLE_UNASSALPHA;
            if ((pi_rpPixelType->GetClassID() == HRPPixelTypeV32R8G8B8X8::CLASS_ID) ||
                (pi_rpPixelType->GetClassID() == HRPPixelTypeV32B8G8R8X8::CLASS_ID))
                ExtraSample = EXTRASAMPLE_UNSPECIFIED;

            unsigned short BitPerSample[4];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;
            BitPerSample[2] = 8;
            BitPerSample[3] = 8;

            if(Compression == COMPRESSION_JPEG || Compression == COMPRESSION_HMR_FLASHPIX)
                m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_YCBCR);

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 4, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            m_pTiff->SetField (EXTRASAMPLES, (uint32_t) 1, &ExtraSample);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV24PhotoYCC::CLASS_ID)
            {
            unsigned short BitPerSample[3];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;
            BitPerSample[2] = 8;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 3, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)3);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_YCBCR);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID)
            {
            unsigned short ExtraSample = EXTRASAMPLE_ASSOCALPHA;

            unsigned short BitPerSample[4];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;
            BitPerSample[2] = 8;
            BitPerSample[3] = 8;

            if(Compression == COMPRESSION_JPEG || Compression == COMPRESSION_HMR_FLASHPIX)
                m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_YCBCR);

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 4, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            m_pTiff->SetField (EXTRASAMPLES, (uint32_t) 1, &ExtraSample);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID)
            {
            unsigned short ExtraSample = EXTRASAMPLE_ASSOCALPHA;

            unsigned short BitPerSample[4];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;
            BitPerSample[2] = 8;
            BitPerSample[3] = 8;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 4, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_YCBCR);
            m_pTiff->SetField (EXTRASAMPLES, (uint32_t) 1, &ExtraSample);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV16B5G5R5::CLASS_ID)
            {
            unsigned short BitPerSample[4];
            BitPerSample[0] = 5;
            BitPerSample[1] = 5;
            BitPerSample[2] = 5;
            BitPerSample[3] = 1;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 4, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            }

        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV16R5G6B5::CLASS_ID)
            {
            unsigned short BitPerSample[3];
            BitPerSample[0] = 5;
            BitPerSample[1] = 6;
            BitPerSample[2] = 5;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 3, BitPerSample);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)3);
            }

        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID)
            {
            unsigned short BitPerSample = 8;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV8GrayWhite8::CLASS_ID)
            {
            unsigned short BitPerSample = 8;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISWHITE);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV16PRGray8A8::CLASS_ID)
            {
            unsigned short ExtraSample = EXTRASAMPLE_ASSOCALPHA;

            unsigned short BitPerSample[2];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 2, BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)2);
            m_pTiff->SetField (EXTRASAMPLES, (uint32_t) 1, &ExtraSample);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV1Gray1::CLASS_ID)
            {
            unsigned short BitPerSample = 1;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t) 1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV1GrayWhite1::CLASS_ID)
            {
            unsigned short BitPerSample = 1;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISWHITE);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV48R16G16B16::CLASS_ID)
            {
            unsigned short BitPerSample[3];
            BitPerSample[0] = 16;
            BitPerSample[1] = 16;
            BitPerSample[2] = 16;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)3, BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_RGB);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)3);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV64R16G16B16A16::CLASS_ID)
            {
            unsigned short ExtraSample = EXTRASAMPLE_UNASSALPHA;
            unsigned short BitPerSample[4];
            BitPerSample[0] = 16;
            BitPerSample[1] = 16;
            BitPerSample[2] = 16;
            BitPerSample[3] = 16;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)4, BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_RGB);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            m_pTiff->SetField (EXTRASAMPLES, (uint32_t) 1, &ExtraSample);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV64R16G16B16X16::CLASS_ID)
            {
            unsigned short ExtraSample = EXTRASAMPLE_UNSPECIFIED;
            unsigned short BitPerSample[4];
            BitPerSample[0] = 16;
            BitPerSample[1] = 16;
            BitPerSample[2] = 16;
            BitPerSample[3] = 16;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)4, BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_RGB);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            m_pTiff->SetField (EXTRASAMPLES, (uint32_t) 1, &ExtraSample);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV16Gray16::CLASS_ID)
            {
            unsigned short BitPerSample = 16;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV16Int16::CLASS_ID)
            {
            unsigned short BitPerSample = 16;
            unsigned short SampleFormat = SAMPLEFORMAT_INT;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            m_pTiff->SetField (SAMPLEFORMAT, (uint32_t)1, &SampleFormat);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV32Float32::CLASS_ID)
            {
            unsigned short BitPerSample = 32;
            unsigned short SampleFormat = SAMPLEFORMAT_IEEEFP;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)1, &BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_MINISBLACK);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            m_pTiff->SetField (SAMPLEFORMAT, (uint32_t)1, &SampleFormat);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV96R32G32B32::CLASS_ID)
            {
            unsigned short BitPerSample[3];
            BitPerSample[0] = 32;
            BitPerSample[1] = 32;
            BitPerSample[2] = 32;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)3, BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_RGB);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)3);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeV32CMYK::CLASS_ID)
            {
            unsigned short BitPerSample[4];
            BitPerSample[0] = 8;
            BitPerSample[1] = 8;
            BitPerSample[2] = 8;
            BitPerSample[3] = 8;

            m_pTiff->SetField (BITSPERSAMPLE, (uint32_t)4, BitPerSample);
            m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_SEPARATED);
            m_pTiff->SetField (INKSET, (unsigned short)1);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)4);
            }
        else
            {
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
            }
        }
    else
        {
        // Image Set Pixel Type
        m_pTiff->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_PALETTE);

        if (pi_rpPixelType->GetClassID() == HRPPixelTypeI8R8G8B8::CLASS_ID ||
            pi_rpPixelType->GetClassID() == HRPPixelTypeI8Gray8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)8);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI4R8G8B8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)4);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeId_I2R8G8B8)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)2);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI1R8G8B8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)1);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI1R8G8B8A8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)1);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI4R8G8B8A8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)4);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI8R8G8B8A8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)8);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)1);
            }
        else if (pi_rpPixelType->GetClassID() == HRPPixelTypeI8VA8R8G8B8::CLASS_ID)
            {
            m_pTiff->SetField (BITSPERSAMPLE, (unsigned short)8);
            m_pTiff->SetField (SAMPLESPERPIXEL, (unsigned short)2);
            }
        else
            {
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
            }

        WritePaletteToFile(pi_Page, pi_rpPixelType->GetPalette());
        }
    }


//-----------------------------------------------------------------------------
// protected
// WritePaletteToFile
//-----------------------------------------------------------------------------
void HRFTiffFile::WritePaletteToFile(uint32_t pi_Page, const HRPPixelPalette&  pi_rPalette)
    {
    HASSERT(pi_Page < CountPages());

    size_t          MaxPaletteEntries  = pi_rPalette.GetMaxEntries();
    bool            HasAlpha           = pi_rPalette.GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE;

    Byte*           pPaletteEntry;
    unsigned short*         rm = NULL;         // Store the Red Channel
    unsigned short*         gm = NULL;         // Store the Green Channel
    unsigned short*         bm = NULL;         // Store the Blue Channel
    Byte*           am = NULL;         // Store the Alpha Channel

    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess );

    // Validate the file access
    if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess )
        {
        // Create RGB buffer to convert the palette entry
        rm = new unsigned short[MaxPaletteEntries];
        gm = new unsigned short[MaxPaletteEntries];
        bm = new unsigned short[MaxPaletteEntries];
        if(HasAlpha)
            am = new Byte[MaxPaletteEntries];

        if ((rm) && (gm) && (bm))
            {
            if (MaxPaletteEntries != pi_rPalette.CountUsedEntries())
                {
                memset(rm, 0, MaxPaletteEntries * sizeof(unsigned short));
                memset(gm, 0, MaxPaletteEntries * sizeof(unsigned short));
                memset(bm, 0, MaxPaletteEntries * sizeof(unsigned short));
                if (HasAlpha)
                    memset(am, 0, MaxPaletteEntries * sizeof(Byte));
                }
            // Convert the palette entry to the RGB buffer
            for (uint32_t i=0; i<pi_rPalette.CountUsedEntries(); i++)
                {
                pPaletteEntry = (Byte*)pi_rPalette.GetCompositeValue(i);
                rm[i] = (unsigned short)(pPaletteEntry[0] * 257);
                gm[i] = (unsigned short)(pPaletteEntry[1] * 257);
                bm[i] = (unsigned short)(pPaletteEntry[2] * 257);
                if(HasAlpha)
                    am[i] = pPaletteEntry[3];
                }
            // Store the RGB buffer to TIFF
            m_pTiff->SetField (TCOLORMAP, rm, gm, bm);
            if(HasAlpha)
                {
                // Get HMR directorie information.
                HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();
                // Check if the private tag is present
                //
                if (m_pTiff->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
                    {
                    GetFilePtr()->SetField(HMR_TRANSPARENCY_PALETTE,
                                           pi_rPalette.GetMaxEntries(),
                                           am);

                    // Reset Directory
                    GetFilePtr()->SetDirectory(CurDir);
                    }
                }

            }

        // Free the RGB Buffer
        if (rm)
            delete rm;

        if (gm)
            delete gm;

        if (bm)
            delete bm;
        if (am)
            delete am;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Open
//-----------------------------------------------------------------------------
bool HRFTiffFile::Open(bool pi_CreateBigTifFormat)
    {
    // Not open and Creation mode
    if (!m_IsOpen && GetAccessMode().m_HasCreateAccess)
        Create(pi_CreateBigTifFormat);

    // Open the file
    else if (!m_IsOpen)
        {
        HFCAccessMode AccessMode = GetAccessMode() | HFC_READ_ONLY;

        // This method creates the sharing control sister file
        SharingControlCreate();

        // Lock the sister file for the GetField operation
        HFCLockMonitor SisterFileLock (GetLockManager());

        m_pTiff = new HTIFFFile (GetURL(), m_Offset, AccessMode);

        m_pTiff->GetFilePtr()->ThrowOnError(); 

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        HTIFFError* pErr;
        m_pTiff->IsValid(&pErr);
        if ((pErr != 0) && pErr->IsFatal())
            throw HRFTiffErrorException(GetURL()->GetURL(), *pErr);

        // Build the directory list
        m_pDirectories = 0;
        ScanDirectories ();
        m_IsOpen = true;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// Open
//-----------------------------------------------------------------------------
bool HRFTiffFile::Open(const HFCPtr<HFCURL>&  pi_rpURL)
    {
    // Not open and Creation mode
    bool Result = false;

    // Open the file
    if (!m_IsOpen)
        {
        HFCAccessMode AccessMode = GetAccessMode() | HFC_READ_ONLY;

        //  Open the file
        // This method creates the sharing control sister file
        SharingControlCreate();

        // Lock the sister file for the GetField operation
        HFCLockMonitor SisterFileLock (GetLockManager());

        m_pTiff = new HTIFFFile (pi_rpURL, m_Offset, AccessMode);

        if (m_pTiff->GetFilePtr() == 0)
            throw HFCCannotOpenFileException(pi_rpURL->GetURL());
        m_pTiff->GetFilePtr()->ThrowOnError(); 

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        HTIFFError* pErr;
        m_pTiff->IsValid(&pErr);
        if ((pErr != 0) && pErr->IsFatal())
            throw HRFTiffErrorException(GetURL()->GetURL(), *pErr);

        // Build the directory list
        m_pDirectories = 0;
        ScanDirectories ();
        m_IsOpen = true;
        Result = true;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Protected
// Returns a list of common tags for the TIFF family
//-----------------------------------------------------------------------------
void HRFTiffFile::GetBaselineTags(HPMAttributeSet* po_pTagList, const HRPPixelType& pi_rPixelType) const
    {
    HPRECONDITION(po_pTagList != 0);

    char*  pSystem;
    HFCPtr<HPMGenericAttribute> pTag;

    // DOCUMENTNAME Tag
    if (GetFilePtr()->GetField(DOCUMENTNAME, &pSystem))
        {
        pTag = new HRFAttributeDocumentName(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // IMAGEDESCRIPTION Tag
    if (GetFilePtr()->GetField(IMAGEDESCRIPTION, &pSystem))
        {
        pTag = new HRFAttributeImageDescription(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // MAKE Tag
    if (GetFilePtr()->GetField(MAKE, &pSystem))
        {
        pTag = new HRFAttributeMake(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // MODEL Tag
    if (GetFilePtr()->GetField(MODEL, &pSystem))
        {
        pTag = new HRFAttributeModel(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // PAGENAME Tag
    if (GetFilePtr()->GetField(PAGENAME, &pSystem))
        {
        pTag = new HRFAttributePageName(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // SOFTWARE Tag
    if (GetFilePtr()->GetField(SOFTWARE, &pSystem))
        {
        pTag = new HRFAttributeSoftware(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // DATETIME Tag
    if (GetFilePtr()->GetField(DATETIME, &pSystem))
        {
        pTag = new HRFAttributeDateTime(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // ARTIST Tag
    if (GetFilePtr()->GetField(ARTIST, &pSystem))
        {
        pTag = new HRFAttributeArtist(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // HOSTCOMPUTER Tag
    if (GetFilePtr()->GetField(HOSTCOMPUTER, &pSystem))
        {
        pTag = new HRFAttributeHostComputer(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // RESOLUTIONUNIT Tag
    unsigned short UnitValue;
    if (GetFilePtr()->GetField(RESOLUTIONUNIT, &UnitValue))
        {
        pTag = new HRFAttributeResolutionUnit(UnitValue);
        po_pTagList->Set(pTag);
        }
    // XRESOLUTION Tag
    RATIONAL XResolution;
    if (GetFilePtr()->GetField(XRESOLUTION, &XResolution))
        {
        pTag = new HRFAttributeXResolution(XResolution.Value);
        po_pTagList->Set(pTag);
        }
    // YRESOLUTION Tag
    RATIONAL YResolution;
    if (GetFilePtr()->GetField(YRESOLUTION, &YResolution))
        {
        pTag = new HRFAttributeYResolution(YResolution.Value);
        po_pTagList->Set(pTag);
        }

    // COPYRIGHT Tag
    if (GetFilePtr()->GetField(COPYRIGHT, &pSystem))
        {
        pTag = new HRFAttributeCopyright(WString(pSystem,false));
        po_pTagList->Set(pTag);
        }

    // Extended sample minimum value tag (prefer this tag to the baseline tag because it has better precision)
    bool IsSMinSampleValueFound = false;
        {
        vector<double> MinSampleValues;

        if (GetFilePtr()->GetConvertedField(SMINSAMPLEVALUE, MinSampleValues))
            {
            HASSERT_DATA(pi_rPixelType.CountIndexBits() != 0 ? MinSampleValues.size() == 1 : pi_rPixelType.GetChannelOrg().CountChannels() == MinSampleValues.size());

            pTag = new HRFAttributeMinSampleValue(MinSampleValues);
            po_pTagList->Set(pTag);

            IsSMinSampleValueFound = true;
            }
        }

    // Extended sample maximum value tag (prefer this tag to the baseline tag because it has better precision)
    bool IsSMaxSampleValueFound = false;
        {
        vector<double> MaxSampleValues;

        if (GetFilePtr()->GetConvertedField(SMAXSAMPLEVALUE, MaxSampleValues))
            {
            HASSERT_DATA(pi_rPixelType.CountIndexBits() != 0 ? MaxSampleValues.size() == 1 : pi_rPixelType.GetChannelOrg().CountChannels() == MaxSampleValues.size());

            pTag = new HRFAttributeMaxSampleValue(MaxSampleValues);
            po_pTagList->Set(pTag);

            IsSMaxSampleValueFound = true;
            }
        }

    // Sample Minimum Value Tag
    unsigned short* pSampleLimitValue;
    uint32_t NbLimitValues;

    if (!IsSMinSampleValueFound && GetFilePtr()->GetField(MINSAMPLEVALUE, &NbLimitValues, &pSampleLimitValue))
        {
        HASSERT_DATA(pi_rPixelType.CountIndexBits() != 0 ? NbLimitValues == 1 : pi_rPixelType.GetChannelOrg().CountChannels() == NbLimitValues);
        vector<double> MinSampleValue;

        FILL_VECTOR_WITH_PTR_VALS(MinSampleValue, NbLimitValues, pSampleLimitValue);

        pTag = new HRFAttributeMinSampleValue(MinSampleValue);
        po_pTagList->Set(pTag);
        }

    // Sample Maximum Value Tag
    if (!IsSMaxSampleValueFound && GetFilePtr()->GetField(MAXSAMPLEVALUE, &NbLimitValues, &pSampleLimitValue))
        {
        HASSERT_DATA(pi_rPixelType.CountIndexBits() != 0 ? NbLimitValues == 1 : pi_rPixelType.GetChannelOrg().CountChannels() == NbLimitValues);
        vector<double> MaxSampleValue;

        FILL_VECTOR_WITH_PTR_VALS(MaxSampleValue, NbLimitValues, pSampleLimitValue);

        pTag = new HRFAttributeMaxSampleValue(MaxSampleValue);
        po_pTagList->Set(pTag);
        }
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFTiffFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    HFCAccessMode AccessMode = GetAccessMode();
    uint32_t TESTJPEGISOCompression = COMPRESSION_NONE;
    GetFilePtr()->GetField(COMPRESSION, &TESTJPEGISOCompression);

    // !!!! HChkSebG !!!!
    // Cannot allow to create and or edit file with jpeg compression and data source has been stored as RGB
    // (should'nt but happen..)
    if (TESTJPEGISOCompression == COMPRESSION_JPEG)
        {
        unsigned short SamplePerPixel = 0;
        unsigned short Photometric    = 0;

        GetFilePtr()->GetField(SAMPLESPERPIXEL, &SamplePerPixel);
        GetFilePtr()->GetField(PHOTOMETRIC    , &Photometric);

        if (Photometric == PHOTOMETRIC_RGB && SamplePerPixel == 3)
            {
            if (AccessMode.m_HasCreateAccess || AccessMode.m_HasWriteAccess)
                {
                throw HRFAccessModeForCodeNotSupportedException(GetURL()->GetURL());
                }
            }
        }

    //
    // Support OneStripPackBitRGBA image in Readonly only.
    //
    if (GetFilePtr()->IsSimulateLine_OneStripPackBitRGBAFile() && (AccessMode.m_HasCreateAccess || AccessMode.m_HasWriteAccess))
        {
        throw HRFAccessModeForCodeNotSupportedException(GetURL()->GetURL());
        }

    uint32_t PageDirectoryIndex;
    // Create the descriptor for each resolution of each page
    for (uint32_t Page=0; Page < CalcNumberOfPage(); Page++)
        {
        //
        PageDirectoryIndex = GetIndexOfPage(Page);
        SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, PageDirectoryIndex));

        // Compression the GetCodecsList
        HFCPtr<HCDCodec> pCodec = CreateCodecFromFile(GetFilePtr(), Page);

        // Pixel Type
        HFCPtr<HRPPixelType> PixelType = CreatePixelTypeFromFile(GetFilePtr(), Page);

        // the main 1:1 width
        uint32_t MainWidth = 0;
        // MainWidth to calc the resolution ratio
        GetFilePtr()->GetField(IMAGEWIDTH, &MainWidth);


        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        unsigned short SubResCount = CalcNumberOfSubResolution(PageDirectoryIndex);
        for (unsigned short Resolution=0; Resolution <= SubResCount; Resolution++)
            {
            // Obtain Resolution Information

            // Select the page and resolution
            SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, PageDirectoryIndex + Resolution));

            // DEBUG MODE ONLY
            // JPEG ISO Compression
            // Select the page
#if 0
            HDEBUGCODE(
                unsigned short TESTJPEGISOCompression = COMPRESSION_NONE;
                GetFilePtr()->GetField(COMPRESSION, &TESTJPEGISOCompression);

                if (TESTJPEGISOCompression == COMPRESSION_JPEG)
            {
            uint32_t TESTJPEGISOBlockWidth (0);
                uint32_t TESTJPEGISOBlockHeight (0);

                if (GetFilePtr()->IsTiled())
                    {
                    GetFilePtr()->GetField(TILEWIDTH, &TESTJPEGISOBlockWidth);
                    GetFilePtr()->GetField(TILELENGTH, &TESTJPEGISOBlockHeight);
                    HASSERT(TESTJPEGISOBlockWidth % 16 == 0);
                    HASSERT(TESTJPEGISOBlockHeight % 16 == 0);
                    }
                else
                    {
                    uint32_t TESTJPEGISOLength;
                    GetFilePtr()->GetField(IMAGEWIDTH, &TESTJPEGISOBlockWidth);
                    GetFilePtr()->GetField(ROWSPERSTRIP, &TESTJPEGISOBlockHeight);
                    GetFilePtr()->GetField(IMAGELENGTH, &TESTJPEGISOLength);

                    if (TESTJPEGISOBlockHeight < TESTJPEGISOLength)
                        HASSERT(TESTJPEGISOBlockHeight % 16 == 0);
                    }
                HASSERT(TESTJPEGISOBlockWidth <= 65535);
                HASSERT(TESTJPEGISOBlockHeight <= 65535);
                }
            )
#endif
            // resolution dimension
            uint32_t Width;
            uint32_t Height;
            GetFilePtr()->GetField(IMAGEWIDTH, &Width);
            GetFilePtr()->GetField(IMAGELENGTH, &Height);

            uint32_t BlockWidth (0);
            uint32_t BlockHeight (0);
            HRFBlockType BlockType;

            if (GetFilePtr()->IsTiled())
                {
                // Tile dimension
                GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);
                GetFilePtr()->GetField(TILELENGTH, &BlockHeight);
                BlockType = HRFBlockType::TILE;
                }
            else
                {
                // Strip dimension
                BlockWidth = Width;
                GetFilePtr()->GetField(ROWSPERSTRIP, &BlockHeight);
                BlockType = HRFBlockType::STRIP;
                }

            double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(MainWidth, Width);

            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                GetSpecialAccessMode(AccessMode),                   // AccessMode,
                GetCapabilities(),                                  // Capabilities,
                Ratio,                                              // XResolutionRatio,
                Ratio,                                              // YResolutionRatio,
                PixelType,                                          // PixelType,
                pCodec,                                             // Codec,
                GetSpecialBlockAccessType(HRFBlockAccess::RANDOM),  // RBlockAccess,
                GetSpecialBlockAccessType(HRFBlockAccess::RANDOM),  // WBlockAccess,
                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
                HRFInterleaveType::PIXEL,                           // InterleaveType
                false,                                              // IsInterlace,
                Width,                                              // Width,
                Height,                                             // Height,
                BlockWidth,                                         // BlockWidth,
                BlockHeight,                                        // BlockHeight,
                0,                                                  // BlocksDataFlag
                BlockType);                                            // BlockType

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // Tag information
        char*  pSystem;
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;
        SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, PageDirectoryIndex));

        // Get common TIFF family tags
        GetBaselineTags(&TagList, *PixelType);

        // INKNAMES Tag
        if (GetFilePtr()->GetField(INKNAMES, &pSystem))
            {
            pTag = new HRFAttributeInkNames(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // SLO
        pTag = new HRFAttributeImageSlo((unsigned short)GetScanLineOrientation().m_ScanlineOrientation);
        TagList.Set(pTag);

        //Get the EXIF related GPS tags if any
        GetFilePtr()->GetEXIFDefinedGPSTags(PageDirectoryIndex, TagList);

        //Get the EXIF related GPS tags if any
        GetFilePtr()->GetEXIFTags(PageDirectoryIndex, TagList);

        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (AccessMode,
                                       GetCapabilities(),           // Capabilities,
                                       ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                       0,                           // RepresentativePalette,
                                       0,                           // Histogram,
                                       0,                           // Thumbnail,
                                       0,                           // ClipShape,
                                       0,                            // TransfoModel,
                                       0,                           // Filters
                                       &TagList);                   // Defined Tag

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }


//-----------------------------------------------------------------------------
// Protected
// CreateTransfoModelFromTiffMatrix
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFTiffFile::CreateTransfoModelFromTiffMatrix() const
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel;
    double*                  pMat4by4;
    uint32_t                  Count;

    // Read the matrix from file
    if (GetFilePtr()->GetField(GEOTRANSMATRIX, &Count, &pMat4by4))
        {
        pTransfoModel = new HGF2DProjective();

        HFCMatrix<3, 3> TheMatrix;

        TheMatrix[0][0] = pMat4by4[0];
        TheMatrix[0][1] = pMat4by4[1];
        TheMatrix[0][2] = pMat4by4[3];
        TheMatrix[1][0] = pMat4by4[4];
        TheMatrix[1][1] = pMat4by4[5];
        TheMatrix[1][2] = pMat4by4[7];
        TheMatrix[2][0] = pMat4by4[12];
        TheMatrix[2][1] = pMat4by4[13];
        TheMatrix[2][2] = pMat4by4[15];

        ((HFCPtr<HGF2DProjective>&)pTransfoModel)->SetByMatrix(TheMatrix);

        // Get the simplest model possible.
        HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

        if (pTempTransfoModel != 0)
            pTransfoModel = pTempTransfoModel;
        }

    return pTransfoModel;
    }

//-----------------------------------------------------------------------------
// private
// Create
//-----------------------------------------------------------------------------
bool HRFTiffFile::Create(bool pi_CreateBigTifFormat)
    {
    // Create the file

    // Set no directories
    m_pDirectories = 0;

    // This method creates the sharing control sister file
    SharingControlCreate();

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Create the tiff
    m_pTiff = new HTIFFFile (GetURL(), 0/*offset*/, HFC_READ_WRITE_CREATE, pi_CreateBigTifFormat);

    m_pTiff->GetFilePtr()->ThrowOnError(); 

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    HTIFFError* pErr;
    m_pTiff->IsValid(&pErr);
    if ((pErr != 0) && pErr->IsFatal())
        throw HRFTiffErrorException(GetURL()->GetURL(), *pErr);

    m_IsOpen = true;

    return true;
    }


//-----------------------------------------------------------------------------
// Public
// Return the current size of the file, which might not be accurate in the
// case of a file access by a third party library due to internal cache in
// such a library.
//-----------------------------------------------------------------------------
uint64_t HRFTiffFile::GetFileCurrentSize() const
    {
    uint64_t        CurrentFileSize = 0;
    HFCBinStream* pFile = GetFilePtr()->GetFilePtr();

    if (pFile != 0)
        {
        CurrentFileSize = pFile->GetSize();
        }

    return CurrentFileSize;
    }

//-----------------------------------------------------------------------------
// Protected
// WriteTransfoModel
// Used to call different fonction depending of the file type.
//-----------------------------------------------------------------------------
bool HRFTiffFile::WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    return WriteTransfoModelToTiffMatrix(pi_rpTransfoModel);
    }

//-----------------------------------------------------------------------------
// Protected
// WriteTransfoModelToTiffMatrix
//-----------------------------------------------------------------------------
bool HRFTiffFile::WriteTransfoModelToTiffMatrix(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess  || GetAccessMode().m_HasCreateAccess);
    HPRECONDITION(pi_rpTransfoModel != 0);

    bool Ret = true;

    // Check if the transformation can be represented by a matrix.
    if (pi_rpTransfoModel->CanBeRepresentedByAMatrix())
        {
        double  aMat[16];
        double* pMat4by4;
        uint32_t Count;

        // Get the values to put into the matrix.
        HFCMatrix<3, 3> TheMatrix = pi_rpTransfoModel->GetMatrix();

        // Fill the affine matrix.
        aMat[0]  = TheMatrix[0][0];
        aMat[1]  = TheMatrix[0][1];
        aMat[2]  = 0.0;
        aMat[3]  = TheMatrix[0][2];
        aMat[4]  = TheMatrix[1][0];
        aMat[5]  = TheMatrix[1][1];
        aMat[6]  = 0.0;
        aMat[7]  = TheMatrix[1][2];
        aMat[8]  = 0.0;
        aMat[9]  = 0.0;
        aMat[10] = 1.0;
        aMat[11] = 0.0;
        aMat[12] = TheMatrix[2][0];
        aMat[13] = TheMatrix[2][1];
        aMat[14] = 0.0;
        aMat[15] = TheMatrix[2][2];

        // Don't update the file if we don't change de Matrix.
        //
        // if the file don't have a Matrix, or
        // the new Matrix is different
        if (!GetFilePtr()->GetField(GEOTRANSMATRIX, &Count, &pMat4by4) ||
            memcmp((double*)aMat, pMat4by4, sizeof(double)*16))
            {
            // Set the Matrix in the file
            Ret = GetFilePtr()->SetField(GEOTRANSMATRIX, 16, (double*)aMat);
            }
        }
    else
        Ret = false;

    return Ret;
    }

//-----------------------------------------------------------------------------
// protected
// CreatePixelTypeFromFile for the current resolution
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFTiffFile::CreatePixelTypeFromFile(HTIFFFile*                        pi_pTIFFFile,
                                                          uint32_t                         pi_Page,
                                                          Byte*                             pi_pAlphaPalette,
                                                          unsigned short                    pi_HMRPixelTypeSpec,
                                                          const ListOfChannelIndex*         pi_pChannelsWithNoDataValue,
                                                          const ListOfChannelNoDataValue*   pi_pChannelsNoDataValue)
    {
    unsigned short*                pBitPerSample;
    unsigned short         BitPerSample (1);
    unsigned short         SamplePerPixel (1);
    unsigned short         Photometric (0);
    unsigned short*                pExtraSample;
    HFCPtr<HRPPixelType>    pPixelType;

    bool             HasNoDataValues  = (0 != pi_pChannelsWithNoDataValue &&
                                         0 != pi_pChannelsNoDataValue &&
                                         0 < pi_pChannelsWithNoDataValue->size());
    HPRECONDITION(pi_pTIFFFile != 0);
    HPRECONDITION((HasNoDataValues) ? pi_pChannelsWithNoDataValue->size() == pi_pChannelsNoDataValue->size() : true);

    HTIFFFile::DirectoryID CurDir = pi_pTIFFFile->CurrentDirectory();

    if (pi_pTIFFFile->SetPage(pi_Page))
        {
        // 1, 4, 8, 16...
        pi_pTIFFFile->GetField(SAMPLESPERPIXEL, &SamplePerPixel);
        pi_pTIFFFile->GetField(PHOTOMETRIC, &Photometric);

        // Get the nb of bit per pixel (FOR EACH SAMPLE)
        uint32_t NbSample = SamplePerPixel;
        pi_pTIFFFile->GetField(BITSPERSAMPLE, &NbSample, &pBitPerSample);

        BitPerSample = pBitPerSample[0];

        //DMx CYMK
        //
        // HChkSebG TR#: 131143  : Patch to better support non-standard Tiff.
        //                         Some Tiff provide non valid Photometric interpretation value
        //                         considering their pixeltype.

        // PLF      TR#: 131143  : With the previous patch, files with 2 channels with Photometric
        //                         MinIsWhite and MinIsBlack were treated as RGB and therefore couldn't
        //                         be opened. The NbSample value in the condition has ben changed from 1 to 2
        //                         to solve this.
        if(NbSample > 2 && (Photometric == PHOTOMETRIC_MINISWHITE || Photometric == PHOTOMETRIC_MINISBLACK))
            Photometric = PHOTOMETRIC_RGB;

        // Build the PixelType
        switch (Photometric)
            {
            case PHOTOMETRIC_MINISWHITE:
                // HCHK dg
                // Detect if it is a HMR file, verify if the private tag is present
                // It is a patch to because some old HMR file have
                // PhotometricInterpretation=min-is-white even if it should be set to Palette.
                if (pi_pTIFFFile->TagIsPresent (HMR_IMAGEINFORMATION))
                    {
                    int32_t             Index;
                    HRPPixelPalette*    pPalette;
                    Byte               Value[3];
                    unsigned short*             rm = 0;
                    unsigned short*             gm = 0;
                    unsigned short*             bm = 0;
                    bool               HasPalette = 0;

                    HasPalette = pi_pTIFFFile->GetField(TCOLORMAP, &rm, &gm, &bm);

                    HASSERT(BitPerSample == 1);
                    HASSERT(HasPalette);

                    pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB (8, 8, 8, 0,
                                                                                              HRPChannelType::UNUSED,
                                                                                              HRPChannelType::VOID_CH,
                                                                                              0),
                                                                            BitPerSample);

                    pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                    // Convert Palette 16 bits to 8 bits.
                    for (Index=0; Index<2; Index++)
                        {
                        Value[0]  = (unsigned char)(rm[Index] / 257);
                        Value[1]  = (unsigned char)(gm[Index] / 257);
                        Value[2]  = (unsigned char)(bm[Index] / 257);
                        pPalette->SetCompositeValue(Index, Value);
                        }
                    }
                else
                    {
                    if(SamplePerPixel == 1)
                        {
                        pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgGrayWhite (BitPerSample,
                                                                                                        HRPChannelType::UNUSED,
                                                                                                        HRPChannelType::VOID_CH,
                                                                                                        0),
                                                                                0);
                        }
                    else if (SamplePerPixel == 2)
                        {
                        uint32_t NbSample = 1;
                        pi_pTIFFFile->GetField(EXTRASAMPLES, &NbSample, &pExtraSample);

                        switch (*pExtraSample)
                            {
                                // Extra sample contain alpha value.
                            case EXTRASAMPLE_UNASSALPHA:
                                // TO DO
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                                // Extra sample contain pre-multiplied alpha value.
                            case EXTRASAMPLE_ASSOCALPHA:
                                // TO DO
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                                // Extra sample does not contain alpha value .
                            case EXTRASAMPLE_UNSPECIFIED:
                                // TO DO
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                            default:
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                            }
                        }
                    else
                        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                    }
                break;

            case PHOTOMETRIC_MINISBLACK:
                // HCHK dg
                // Detect if it is a HMR file, verify if the private tag is present
                // It is a patch to because some old HMR file have
                // PhotometricInterpretation=min-is-black even if it should be set to Palette.
                if (pi_pTIFFFile->TagIsPresent (HMR_IMAGEINFORMATION))
                    {
                    int32_t             Index;
                    HRPPixelPalette*    pPalette;
                    Byte               Value[3];
                    unsigned short*             rm = 0;
                    unsigned short*             gm = 0;
                    unsigned short*             bm = 0;
                    bool               HasPalette = 0;

                    HasPalette = pi_pTIFFFile->GetField(TCOLORMAP, &rm, &gm, &bm);

                    HASSERT(BitPerSample == 1);
                    HASSERT(HasPalette);

                    pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB (8, 8, 8, 0,
                                                                                              HRPChannelType::UNUSED,
                                                                                              HRPChannelType::VOID_CH,
                                                                                              0),
                                                                            BitPerSample);

                    pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                    // Convert Palette 16 bits to 8 bits.
                    for (Index=0; Index<2; Index++)
                        {
                        Value[0]  = (unsigned char)(rm[Index] / 257);
                        Value[1]  = (unsigned char)(gm[Index] / 257);
                        Value[2]  = (unsigned char)(bm[Index] / 257);
                        pPalette->SetCompositeValue(Index, Value);
                        }
                    }
                else
                    {
                    if(SamplePerPixel == 1)
                        {
                        HASSERT ((HasNoDataValues) ? 1 == pi_pChannelsWithNoDataValue->size() : true);
                        HASSERT ((HasNoDataValues) ? CHANNEL_INDEX_ALL_CHANNELS == pi_pChannelsWithNoDataValue->at(0) : true);

                        double noDataValue = DBL_MIN;

                        if (HasNoDataValues)
                            {
                            noDataValue = pi_pChannelsNoDataValue->at(0);
                            }
                        else if (pi_pTIFFFile->TagIsPresent (GDALNODATA))
                            {
                            char* noDataValueTag;   
                            bool  hasTag = pi_pTIFFFile->GetField (GDALNODATA, &noDataValueTag);
                            assert(hasTag == true);

                            noDataValue = atof(noDataValueTag);                                                            
                            HasNoDataValues = true;
                            }

                        // Get the nb of bit per pixel (FOR EACH SAMPLE)
                        unsigned short SampleFormat;
                        bool   IsSampleFormatTag = false;

                        IsSampleFormatTag = pi_pTIFFFile->GetField(SAMPLEFORMAT, &SampleFormat);

                        if ((IsSampleFormatTag == true) && (SampleFormat == SAMPLEFORMAT_IEEEFP))
                            {
                            if (32 != BitPerSample)
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());

                            if (HasNoDataValues)
                                {
                                HASSERT((-FLT_MAX) <= noDataValue && FLT_MAX >= noDataValue);
                                float NoDataValue = static_cast<float>(noDataValue);
                                pPixelType = new HRPPixelTypeV32Float32(HRPChannelType::USER, &NoDataValue);
                                }
                            else
                                {
                                pPixelType = new HRPPixelTypeV32Float32(HRPChannelType::USER);                                
                                }
                            }
                        else if ((IsSampleFormatTag == true) && (SampleFormat == SAMPLEFORMAT_INT))
                            {
                            if (16 != BitPerSample)
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());

                            if (HasNoDataValues)
                                {
                                HASSERT((numeric_limits<int16_t>::min()) <= noDataValue && (numeric_limits<int16_t>::max()) >= noDataValue);
                                int16_t NoDataValue = static_cast<int16_t>(noDataValue);
                                pPixelType = new HRPPixelTypeV16Int16(HRPChannelType::USER, &NoDataValue);
                                }
                            else
                                {
                                pPixelType = new HRPPixelTypeV16Int16(HRPChannelType::USER);
                                }
                            }
                        else
                            {
                            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgGray ( BitPerSample,
                                                                                                        HRPChannelType::UNUSED,
                                                                                                        HRPChannelType::VOID_CH,
                                                                                                        0),
                                                                                    0);
                                                                                    
                            if (HasNoDataValues && 0 != pPixelType)
                                {
                                HRPChannelType* channelTypeP = const_cast<HRPChannelType*>(pPixelType->GetChannelOrg().GetChannelPtr(0));
                                HASSERT(0 != channelTypeP);

                                if (8 == BitPerSample)
                                    {
                                    HASSERT((numeric_limits<uint8_t>::max)() >= noDataValue);
                                    channelTypeP->SetNoDataValue(noDataValue);
                                    }

                                else if (16 == BitPerSample)
                                    {
                                    HASSERT((numeric_limits<uint16_t>::max)() >= noDataValue);
                                    channelTypeP->SetNoDataValue(noDataValue);
                                    }
                                else if (32 == BitPerSample)
                                    {
                                    HASSERT((numeric_limits<uint32_t>::max)() >= noDataValue);
                                    channelTypeP->SetNoDataValue(noDataValue);
                                    }
                                }
                            }
                        }
                    else if (SamplePerPixel == 2)
                        {
                        uint32_t NbSample = 1;

                        pi_pTIFFFile->GetField(EXTRASAMPLES, &NbSample, &pExtraSample);

                        switch (*pExtraSample)
                            {
                                // Extra sample contain alpha value.
                            case EXTRASAMPLE_UNASSALPHA:
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                                // Extra sample contain pre-multiplied alpha value.
                            case EXTRASAMPLE_ASSOCALPHA:
                                pPixelType = new HRPPixelTypeV16PRGray8A8();
                                break;
                                // Extra sample does not contain alpha value .
                            case EXTRASAMPLE_UNSPECIFIED:
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                            default:
                                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                break;
                            }
                        }
                    else
                        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                    }
                    break;

            case PHOTOMETRIC_YCBCR:
                {
                // According to TIFF6 documentation: "Minimum Requirements for YCbCr Images" section 21.
                // SamplePerPixel = 3
                // BitPerSample = 8,8,8
                // Compression : none, LZW, JPEG.
                // *** iTiff variance: Added Compression=FlashPix and SamplePerPixel=4 and ??
                // Note that JPEG/flashpix decoder will automatically convert YCbCr to RGB so they are handled like PHOTOMETRIC_RGB. 
                // Others compression mode do not so me must handle them as true YCC.
                uint32_t Compression;
                if (SamplePerPixel == 3 && NbSample == 3 && pBitPerSample[0] == 8 && pBitPerSample[1] == 8 && pBitPerSample[2] == 8 &&
                    pi_pTIFFFile->GetField(COMPRESSION, &Compression) && (COMPRESSION_NONE == Compression || COMPRESSION_LZW == Compression))
                    {
                    // TFS#8838 for now, disable YCbCr with none and LZW compression if subsampling tag != 1:1.
                    // If an YCBCR raster have the YCBCRsubsampling tag other than 1:1, it will create stripe in the resulting image.
                    // the problem is coming from the two levels of decompression, for example if the subsampling tag is 4:2:0 there will be four Y for one Cb and one Cr.
                    // the data packing in the stream is Y_0_0, Y_0_1, Y_1_0, Y_1_1, Cb, Cr... So those pixels are shape like a small window.
                    // To fix this, we first need decompress the data (lzw, jpeg..) to half the final size of a strip. And then, we place each Y, Cb, Cb to their respective place so the final count of a strip is correct.
                    unsigned short val1, val2;
                    if (pi_pTIFFFile->GetField(YCBCRSUBSAMPLING, &val1, &val2) && (val1 != 1 || val2 != 1))
                        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());

                    pPixelType = new HRPPixelTypeV24PhotoYCC();
                    break;
                    }
                }
// ************* FALL THROUGH **** PHOTOMETRIC_RGB since jpeg and flashpix are handled as RGB since the decoder is doing the conversion to RGB.
            case PHOTOMETRIC_RGB:
                if (SamplePerPixel == 3)
                    {
                    if (NbSample == 3)
                        {
                        if (pi_HMRPixelTypeSpec == PIXELTYPESPEC_BGR)
                            pPixelType = new HRPPixelTypeV24B8G8R8();
                        else if (pBitPerSample[0] == 5 && pBitPerSample[1] == 6 && pBitPerSample[2] == 5)
                            pPixelType = new HRPPixelTypeV16R5G6B5();
                        else if (pBitPerSample[0] == 32 && pBitPerSample[1] == 32 && pBitPerSample[2] == 32)
                            pPixelType = new HRPPixelTypeV96R32G32B32();
                        else if (pBitPerSample[0] != pBitPerSample[1] || pBitPerSample[0] != pBitPerSample[2] &&
                            (pBitPerSample[0] != 8 || pBitPerSample[0] != 16))
                            throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                        else
                            {
                            HRPChannelOrgRGB ChannelOrg(pBitPerSample[0],
                                pBitPerSample[1],
                                pBitPerSample[2],
                                0,
                                HRPChannelType::UNUSED,
                                HRPChannelType::VOID_CH,
                                0);

                            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(ChannelOrg, 0);
                            }
                        }
                    else if(NbSample == 1)
                        {
                        if(BitPerSample != 8)
                            throw HRFPixelTypeNotSupportedException( pi_pTIFFFile->GetURL()->GetURL());
                        else
                            {
                            HRPChannelOrgRGB ChannelOrg(BitPerSample,
                                                        BitPerSample,
                                                        BitPerSample,
                                                        0,
                                                        HRPChannelType::UNUSED,
                                                        HRPChannelType::VOID_CH,
                                                        0);

                            pPixelType = HRPPixelTypeFactory::GetInstance()->Create (ChannelOrg,
                                                                                     0);
                            }
                        }
                    else
                        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                    }
                else if (SamplePerPixel == 4)
                    {
                    if (BitPerSample == 5)
                        {
                        pPixelType = new HRPPixelTypeV16B5G5R5();
                        }
                    else if(BitPerSample == 8)
                        {
                        uint32_t NbSample = 1;
                        if (pi_pTIFFFile->GetField(EXTRASAMPLES, &NbSample, &pExtraSample))
                            {
                            switch (*pExtraSample)
                                {
                                    // Extra sample contain alpha value.
                                case EXTRASAMPLE_UNASSALPHA:
                                    pPixelType = new HRPPixelTypeV32R8G8B8A8();
                                    break;
                                    // Extra sample contain pre-multiplied alpha value.
                                case EXTRASAMPLE_ASSOCALPHA:
                                    pPixelType = new HRPPixelTypeV32PR8PG8PB8A8();
                                    break;
                                    // By default we use V32R8G8B8X8 for unspecified extra sample value.
                                case EXTRASAMPLE_UNSPECIFIED:
                                    if (pi_HMRPixelTypeSpec == PIXELTYPESPEC_BGR)
                                        {
                                        pPixelType = new HRPPixelTypeV32B8G8R8X8();
                                        }
                                    else
                                        {
                                        pPixelType = new HRPPixelTypeV32R8G8B8X8();
                                        }
                                    break;
                                default:
                                    throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                    break;
                                }
                            }
                        else
                            {
                            // We don't know what the Last channel stand for. (Alpha, Infra-Red, .. ??)
                            // Assume for now it's Alpha like MicroSoft do.
                            pPixelType = new HRPPixelTypeV32R8G8B8A8();
                            }

                        }
                    else if(BitPerSample == 16)
                        {
                        uint32_t NbSample = 1;
                        if (pi_pTIFFFile->GetField(EXTRASAMPLES, &NbSample, &pExtraSample))
                            {
                            switch (*pExtraSample)
                                {
                                case EXTRASAMPLE_UNASSALPHA:
                                    pPixelType = new HRPPixelTypeV64R16G16B16A16();
                                    break;
                                case EXTRASAMPLE_UNSPECIFIED:      //unspecified extra sample
                                    pPixelType = new HRPPixelTypeV64R16G16B16X16();
                                    break;
                                case EXTRASAMPLE_ASSOCALPHA:       // Extra sample contain pre-multiplied alpha value.
                                default:
                                    throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                                    break;
                                }
                            }
                        else
                            {
                            // We don't know what the Last channel stand for. (Alpha, Infra-Red, .. ??)
                            // Assume for now it's unspecified.
                            pPixelType = new HRPPixelTypeV64R16G16B16X16();
                            }
                        }
                    else
                        {
                        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                        }
                    }
                else
                    throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                break;

            case PHOTOMETRIC_SEPARATED:
                if (SamplePerPixel == 4 && NbSample == 4)
                    {
                    unsigned short InkSet = 0;
                    bool   IsInkSetTag = 0;
                    IsInkSetTag = pi_pTIFFFile->GetField(INKSET, &InkSet);

                    if((pBitPerSample[0] == 8) &&
                       (pBitPerSample[1] == 8) &&
                       (pBitPerSample[2] == 8) &&
                       (pBitPerSample[3] == 8) &&
                       ((IsInkSetTag == false) || (InkSet == INKSET_CMYK)))
                        {
                        //get feild inkset tag should be 1
                        pPixelType = new HRPPixelTypeV32CMYK();
                        }
                    else
                        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                    }
                else
                    throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());

                break;

            case PHOTOMETRIC_PALETTE:
                // Pixel type is a I8VA8R8G8B8
                if(SamplePerPixel == 2)
                    {
                    pPixelType = new HRPPixelTypeI8VA8R8G8B8();

                    // Set the palette
                    int32_t             Index;
                    HRPPixelPalette*    pPalette;
                    Byte               Value[3];
                    unsigned short*             rm, *gm, *bm;
                    int32_t             MaxColor;

                    pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                    MaxColor = (1 << BitPerSample);

                    // The TIFF palette is on 16 bits
                    pi_pTIFFFile->GetField(TCOLORMAP, &rm, &gm, &bm);

                    // Convert Palette 16 bits to 8 bits.
                    for (Index=0; Index<MaxColor; Index++)
                        {
                        Value[0]  = (unsigned char)(rm[Index] / 257);
                        Value[1]  = (unsigned char)(gm[Index] / 257);
                        Value[2]  = (unsigned char)(bm[Index] / 257);
                        pPalette->SetCompositeValue(Index, Value);
                        }

                    }
                else
                    {
                    if(pi_pAlphaPalette != 0)
                        {
                        pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB (8, 8, 8, 8,
                                                                                                  HRPChannelType::UNUSED,
                                                                                                  HRPChannelType::VOID_CH,
                                                                                                  0),
                                                                                BitPerSample);
                        // Set the palette
                        int32_t             Index;
                        HRPPixelPalette*    pPalette;
                        Byte               Value[4];
                        unsigned short*             rm, *gm, *bm;
                        int32_t             MaxColor;

                        pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                        MaxColor = (1 << BitPerSample);

                        /*
                        ** The TIFF palette is on 16 bits
                        */
                        pi_pTIFFFile->GetField(TCOLORMAP, &rm, &gm, &bm);

                        /*
                        ** Convert Palette 16 bits to 8 bits.
                        */
                        for (Index=0; Index<MaxColor; Index++)
                            {
                            Value[0]  = (unsigned char)(rm[Index] / 257);
                            Value[1]  = (unsigned char)(gm[Index] / 257);
                            Value[2]  = (unsigned char)(bm[Index] / 257);
                            Value[3]  = pi_pAlphaPalette[Index];
                            pPalette->SetCompositeValue(Index, Value);
                            }
                        }
                    else
                        {
                        pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB (8, 8, 8, 0,
                                                                                                  HRPChannelType::UNUSED,
                                                                                                  HRPChannelType::VOID_CH,
                                                                                                  0),
                                                                                BitPerSample);
                        // Set the palette
                        int32_t             Index;
                        HRPPixelPalette*    pPalette;
                        Byte               Value[3];
                        unsigned short*             rm, *gm, *bm;
                        int32_t             MaxColor;

                        pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                        MaxColor = (1 << BitPerSample);

                        /*
                        ** The TIFF palette is NORMALLY on 16 bits
                        */
                        pi_pTIFFFile->GetField(TCOLORMAP, &rm, &gm, &bm);

                        // Some bad tiff have a palette on 8 bits only...
                        // We are trying to support both.
                        bool IsReal16BitsWide = false;
                        for (Index=0; Index < MaxColor; Index++)
                            {
                            // As soon as we encounter a value greater than 8 bits we assume the palette is a normal 16 bits per entry palette.
                            if(rm[Index] > 255 || gm[Index] > 255 || bm[Index] > 255)
                                {
                                IsReal16BitsWide = true;
                                break;
                                }
                            }

                        if(IsReal16BitsWide)
                            {
                            for (Index=0; Index < MaxColor; ++Index)
                                {
                                Value[0]  = (Byte)(rm[Index] / 257);
                                Value[1]  = (Byte)(gm[Index] / 257);
                                Value[2]  = (Byte)(bm[Index] / 257);
                                pPalette->SetCompositeValue(Index, Value);
                                }
                            }
                        else
                            {
                            for (Index=0; Index < MaxColor; ++Index)
                                {
                                Value[0]  = (Byte)rm[Index];
                                Value[1]  = (Byte)gm[Index];
                                Value[2]  = (Byte)bm[Index];
                                pPalette->SetCompositeValue(Index, Value);
                                }
                            }
                        }
                    }
                break;

            default:
                throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                break;
            }
        }

    pi_pTIFFFile->SetDirectory(CurDir);

    if(pPixelType == 0)
        throw HRFPixelTypeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());

    return (pPixelType);
    }

//-----------------------------------------------------------------------------
// Protected
// SaveTiffFile
//-----------------------------------------------------------------------------
void HRFTiffFile::SaveTiffFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen)
        {
        if (m_ListOfPageDescriptor.size() > 0)
            {
            // Validate the file access
            if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
                {
                // Update the modification to the file
                for (uint32_t Page=0; Page<CountPages(); Page++)
                    {
                    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

                    if (!pPageDescriptor->IsEmpty())
                        {
                        // Select the page
                        SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, GetIndexOfPage(Page)));

                        // Update the palette
                        if (pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType()->CountIndexBits() > 0)
                            {
                            for (uint32_t Res = 0; Res < pPageDescriptor->CountResolutions(); Res ++)
                                {
                                if (pPageDescriptor->GetResolutionDescriptor((unsigned short)Res)->PaletteHasChanged())
                                    {
                                    WritePaletteToFile(Page,
                                                       pPageDescriptor->GetResolutionDescriptor((unsigned short)Res)->GetPixelType()->GetPalette());
                                    }
                                pPageDescriptor->GetResolutionDescriptor((unsigned short)Res)->Saved();
                                }
                            }

                        // Display each tag.
                        HPMAttributeSet::HPMASiterator TagIterator; 
                        bool isMinSampleTagPresent = false;
                        bool isMaxSampleTagPresent = false;
   
                        for (TagIterator  = GetPageDescriptor(Page)->GetTags().begin();
                             TagIterator != GetPageDescriptor(Page)->GetTags().end(); TagIterator++)
                            {
                            HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

                            if (GetPageDescriptor(Page)->TagHasChanged(*pTag) || GetAccessMode().m_HasCreateAccess)
                                {
                                // DOCUMENTNAME Tag
                                if (pTag->GetID() == HRFAttributeDocumentName::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(DOCUMENTNAME, AString(((HFCPtr<HRFAttributeDocumentName>&)pTag)->GetData().c_str()).c_str());

                                // IMAGEDESCRIPTION Tag
                                else if (pTag->GetID() == HRFAttributeImageDescription::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(IMAGEDESCRIPTION, AString(((HFCPtr<HRFAttributeImageDescription>&)pTag)->GetData().c_str()).c_str());

                                // MAKE Tag
                                else if (pTag->GetID() == HRFAttributeMake::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(MAKE, AString(((HFCPtr<HRFAttributeMake>&)pTag)->GetData().c_str()).c_str());

                                // MODEL Tag
                                else if (pTag->GetID() == HRFAttributeModel::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(MODEL, AString(((HFCPtr<HRFAttributeModel>&)pTag)->GetData().c_str()).c_str());

                                // PAGENAME Tag
                                else if (pTag->GetID() == HRFAttributePageName::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(PAGENAME, AString(((HFCPtr<HRFAttributePageName>&)pTag)->GetData().c_str()).c_str());

                                // SOFTWARE Tag
                                else if (pTag->GetID() == HRFAttributeSoftware::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(SOFTWARE, AString(((HFCPtr<HRFAttributeSoftware>&)pTag)->GetData().c_str()).c_str());

                                // DATETIME Tag
                                else if (pTag->GetID() == HRFAttributeDateTime::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(DATETIME, AString(((HFCPtr<HRFAttributeDateTime>&)pTag)->GetData().c_str()).c_str());

                                // ARTIST Tag
                                else if (pTag->GetID() == HRFAttributeArtist::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(ARTIST, AString(((HFCPtr<HRFAttributeArtist>&)pTag)->GetData().c_str()).c_str());

                                // HOSTCOMPUTER Tag
                                else if (pTag->GetID() == HRFAttributeHostComputer::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(HOSTCOMPUTER, AString(((HFCPtr<HRFAttributeHostComputer>&)pTag)->GetData().c_str()).c_str());

                                // INKNAMES Tag
                                else if (pTag->GetID() == HRFAttributeInkNames::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(INKNAMES, AString(((HFCPtr<HRFAttributeInkNames>&)pTag)->GetData().c_str()).c_str());

                                // RESOLUTIONUNIT Tag
                                else if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
                                    GetFilePtr()->SetField(RESOLUTIONUNIT, ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData());

                                // XRESOLUTION Tag
                                else if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
                                    {
                                    RATIONAL XResolution;
                                    XResolution.Value = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
                                    GetFilePtr()->SetField(XRESOLUTION, XResolution);
                                    }
                                // YRESOLUTION Tag
                                else if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
                                    {
                                    RATIONAL YResolution;
                                    YResolution.Value = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
                                    GetFilePtr()->SetField(YRESOLUTION, YResolution);
                                    }

                                // COPYRIGHT Tag
                                else if (pTag->GetID() == HRFAttributeCopyright::ATTRIBUTE_ID)
                                    GetFilePtr()->SetFieldA(COPYRIGHT,
                                                           AString(((HFCPtr<HRFAttributeCopyright>&)pTag)->GetData().c_str()).c_str());

                                // Sample Minimum Value Tag
                                else if (pTag->GetID() == HRFAttributeMinSampleValue::ATTRIBUTE_ID)
                                    {
                                    isMinSampleTagPresent = true;
                                    WriteSampleLimitValueToDir(static_cast<HRFAttributeMinSampleValue*>(pTag.GetPtr())->GetData(),
                                                               true,
                                                               GetFilePtr());
                                    }

                                // Sample Minimum Value Tag
                                else if (pTag->GetID() == HRFAttributeMaxSampleValue::ATTRIBUTE_ID)
                                    {
                                    isMaxSampleTagPresent = true;
                                    WriteSampleLimitValueToDir(static_cast<HRFAttributeMaxSampleValue*>(pTag.GetPtr())->GetData(),
                                                               false,
                                                               GetFilePtr());
                                    }
                                }
                            }

                        // Clean-up min/max sample in file if they were removed from the page descriptor
                        if (GetFilePtr()->TagIsPresent(MINSAMPLEVALUE) && !isMinSampleTagPresent)
                            GetFilePtr()->RemoveTag(MINSAMPLEVALUE);

                        if (GetFilePtr()->TagIsPresent(MAXSAMPLEVALUE) && !isMaxSampleTagPresent)
                            GetFilePtr()->RemoveTag(MAXSAMPLEVALUE);
        
                        if (GetFilePtr()->TagIsPresent(SMINSAMPLEVALUE) && !isMinSampleTagPresent)
                            GetFilePtr()->RemoveTag(SMINSAMPLEVALUE);

                        if (GetFilePtr()->TagIsPresent(SMAXSAMPLEVALUE) && !isMaxSampleTagPresent)
                            GetFilePtr()->RemoveTag(SMAXSAMPLEVALUE);

                        RasterFileGeocoding const& fileGeocoding = pPageDescriptor->GetRasterFileGeocoding();
                        HCPGeoTiffKeys const& geoTiffKeys = fileGeocoding.GetGeoTiffKeys();

                        IGeoTiffKeysList::GeoKeyItem GeoTiffKey;

                        if (pPageDescriptor->GeocodingHasChanged())
                            {
                            GetFilePtr()->GetGeoKeyInterpretation().Reset();
                            if (!geoTiffKeys.HasValidGeoTIFFKeysList ())
                                {
                                // the list of geotiffkeys is not valid, write minimum values
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(GTModelType, (unsigned short)TIFFGeo_ModelTypeProjected);
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeographicType, (unsigned short)TIFFGeo_UserDefined);
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjectedCSType, (unsigned short)TIFFGeo_UserDefined);
                                GetFilePtr()->GetGeoKeyInterpretation().SetValues(PCSCitation, "User defined (Default value)");
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(Projection, (unsigned short)TIFFGeo_UserDefined);
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCoordTrans, (unsigned short)TIFFGeo_UserDefined);

                                uint32_t UnitsKey = 0;
                                geoTiffKeys.GetValue (ProjLinearUnits, &UnitsKey);
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjLinearUnits, (unsigned short)UnitsKey);
                                }
                            else if (geoTiffKeys.GetFirstKey(&GeoTiffKey) == true)
                                {
                                do
                                    {
                                    // GTModelType
                                    if (GeoTiffKey.KeyID == GTModelType)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GTModelType, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GTRasterType
                                    else if (GeoTiffKey.KeyID == GTRasterType)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GTRasterType, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // PCSCitation
                                    else if (GeoTiffKey.KeyID == PCSCitation)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValues(PCSCitation, GeoTiffKey.KeyValue.StringVal);

                                    // ProjectedCSType
                                    else if (GeoTiffKey.KeyID == ProjectedCSType)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjectedCSType, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GTCitation
                                    else if (GeoTiffKey.KeyID == GTCitation)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValues(GTCitation, GeoTiffKey.KeyValue.StringVal);

                                    // Projection
                                    else if (GeoTiffKey.KeyID == Projection)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(Projection, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // ProjCoordTrans
                                    else if (GeoTiffKey.KeyID == ProjCoordTrans)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCoordTrans, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // ProjLinearUnits
                                    else if (GeoTiffKey.KeyID == ProjLinearUnits)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjLinearUnits, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // ProjLinearUnitSize
                                    else if (GeoTiffKey.KeyID == ProjLinearUnitSize)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjLinearUnitSize, GeoTiffKey.KeyValue.DoubleVal);

                                    // GeographicType
                                    else if (GeoTiffKey.KeyID == GeographicType)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeographicType, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogCitation
                                    else if (GeoTiffKey.KeyID == GeogCitation)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValues(GeogCitation, GeoTiffKey.KeyValue.StringVal);

                                    // GeogGeodeticDatum
                                    else if (GeoTiffKey.KeyID == GeogGeodeticDatum)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogGeodeticDatum, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogPrimeMeridian
                                    else if (GeoTiffKey.KeyID == GeogPrimeMeridian)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogPrimeMeridian, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogLinearUnits
                                    else if (GeoTiffKey.KeyID == GeogLinearUnits)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogLinearUnits, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogLinearUnitSize
                                    else if (GeoTiffKey.KeyID == GeogLinearUnitSize)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogLinearUnitSize, GeoTiffKey.KeyValue.DoubleVal);

                                    // GeogAngularUnits
                                    else if (GeoTiffKey.KeyID == GeogAngularUnits)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogAngularUnits, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogAngularUnitSize
                                    else if (GeoTiffKey.KeyID == GeogAngularUnitSize)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogAngularUnitSize, GeoTiffKey.KeyValue.DoubleVal);

                                    // GeogEllipsoid
                                    else if (GeoTiffKey.KeyID == GeogEllipsoid)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogEllipsoid, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogSemiMajorAxis
                                    else if (GeoTiffKey.KeyID == GeogSemiMajorAxis)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogSemiMajorAxis, GeoTiffKey.KeyValue.DoubleVal);

                                    // GeogSemiMinorAxis
                                    else if (GeoTiffKey.KeyID == GeogSemiMinorAxis)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogSemiMinorAxis, GeoTiffKey.KeyValue.DoubleVal);

                                    // GeogInvFlattening
                                    else if (GeoTiffKey.KeyID == GeogInvFlattening)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogInvFlattening, GeoTiffKey.KeyValue.DoubleVal);

                                    // GeogAzimuthUnits
                                    else if (GeoTiffKey.KeyID == GeogAzimuthUnits)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogAzimuthUnits, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // GeogPrimeMeridianLong
                                    else if (GeoTiffKey.KeyID == GeogPrimeMeridianLong)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeogPrimeMeridianLong, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjStdParallel1
                                    else if (GeoTiffKey.KeyID == ProjStdParallel1)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjStdParallel1, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjStdParallel2
                                    else if (GeoTiffKey.KeyID == ProjStdParallel2)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjStdParallel2, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjNatOriginLong
                                    else if (GeoTiffKey.KeyID == ProjNatOriginLong)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjNatOriginLong, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjNatOriginLat
                                    else if (GeoTiffKey.KeyID == ProjNatOriginLat)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjNatOriginLat, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjFalseEasting
                                    else if (GeoTiffKey.KeyID == ProjFalseEasting)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjFalseEasting, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjFalseNorthing
                                    else if (GeoTiffKey.KeyID == ProjFalseNorthing)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjFalseNorthing, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjFalseOriginLong
                                    else if (GeoTiffKey.KeyID == ProjFalseOriginLong)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjFalseOriginLong, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjFalseOriginLat
                                    else if (GeoTiffKey.KeyID == ProjFalseOriginLat)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjFalseOriginLat, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjFalseOriginEasting
                                    else if (GeoTiffKey.KeyID == ProjFalseOriginEasting)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjFalseOriginEasting, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjFalseOriginNorthing
                                    else if (GeoTiffKey.KeyID == ProjFalseOriginNorthing)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjFalseOriginNorthing, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjCenterLong
                                    else if (GeoTiffKey.KeyID == ProjCenterLong)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCenterLong, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjCenterLat
                                    else if (GeoTiffKey.KeyID == ProjCenterLat)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCenterLat, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjCenterEasting
                                    else if (GeoTiffKey.KeyID == ProjCenterEasting)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCenterEasting, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjCenterNorthing
                                    else if (GeoTiffKey.KeyID == ProjCenterEasting)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCenterNorthing, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjScaleAtNatOrigin
                                    else if (GeoTiffKey.KeyID == ProjScaleAtNatOrigin)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjScaleAtNatOrigin, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjScaleAtCenter
                                    else if (GeoTiffKey.KeyID == ProjScaleAtCenter)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjScaleAtCenter, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjAzimuthAngle
                                    else if (GeoTiffKey.KeyID == ProjAzimuthAngle)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjAzimuthAngle, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjStraightVertPoleLong
                                    else if (GeoTiffKey.KeyID == ProjStraightVertPoleLong)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjStraightVertPoleLong, GeoTiffKey.KeyValue.DoubleVal);

                                    // ProjRectifiedGridAngle
                                    else if (GeoTiffKey.KeyID == ProjRectifiedGridAngle)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjRectifiedGridAngle, GeoTiffKey.KeyValue.DoubleVal);

                                    // VerticalCSType
                                    else if (GeoTiffKey.KeyID == VerticalCSType)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(VerticalCSType, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // VerticalCitation
                                    else if (GeoTiffKey.KeyID == VerticalCitation)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValues(VerticalCitation, GeoTiffKey.KeyValue.StringVal);

                                    // VerticalDatum
                                    else if (GeoTiffKey.KeyID == VerticalDatum)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(VerticalDatum, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    // VerticalUnits
                                    else if (GeoTiffKey.KeyID == VerticalUnits)
                                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(VerticalUnits, (unsigned short)GeoTiffKey.KeyValue.LongVal);

                                    }
                                while(geoTiffKeys.GetNextKey(&GeoTiffKey));
                                }
                            }

                        pPageDescriptor->Saved();
                        }


                    bool  IsTagPresent = false;

                    if (m_pTiff->IsTiff64() == true)
                        {
                        uint64_t DirOffset;
                        IsTagPresent = m_pTiff->GetField (HMR2_IMAGEINFORMATION, &DirOffset);
                        }
                    else
                        {
                        uint32_t DirOffset;
                        IsTagPresent = m_pTiff->GetField (HMR2_IMAGEINFORMATION, &DirOffset);
                        }

                    // Fill the empty blocks.... Needed for Dcartes
                    // Do not call this function if it is a iTiff file, verify if the private tag is present
                    if (!IsTagPresent && !m_IsCreateCancel)
                        {
                        // Lock the sister file for the GetField operation
                        HFCLockMonitor SisterFileLock (GetLockManager());

                        GetFilePtr()->FillAllEmptyDataBlock();

                        // Unlock the sister file
                        SisterFileLock.ReleaseKey();
                        }
                    }
                }
            }

        if (pi_CloseFile)
            {
            // Lock the sister file
            HFCLockMonitor SisterFileLock (GetLockManager());

            m_pTiff  = 0;

            // Unlock the sister file
            SisterFileLock.ReleaseKey();

            m_IsOpen = false;
            }
        else
            {
            m_pTiff->Save();
            }
        }
    }

//-----------------------------------------------------------------------------
// private
// CreateCodecFromFile for the specified Compression
//-----------------------------------------------------------------------------
HFCPtr<HCDCodec> HRFTiffFile::CreateCodecFromFile(HTIFFFile* pi_pTIFFFile,
                                                  uint32_t   pi_Page)
    {
    HPRECONDITION(pi_pTIFFFile != 0);

    HFCPtr<HCDCodec> pCodec;

    HTIFFFile::DirectoryID CurDir = pi_pTIFFFile->CurrentDirectory();

    if (pi_pTIFFFile->SetPage(pi_Page))
        {
        uint32_t Compression = COMPRESSION_NONE;
        pi_pTIFFFile->GetField(COMPRESSION, &Compression);

        // map tiff codec id to HRP codec
        switch (Compression)
            {
            case COMPRESSION_NONE:
                pCodec = new HCDCodecIdentity();
                break;

            case COMPRESSION_HMR_RLE1:
                pCodec = new HCDCodecHMRRLE1();
                break;

            case COMPRESSION_HMR_FLASHPIX:
                pCodec = new HCDCodecFlashpix();
                break;

            case COMPRESSION_OJPEG :
            case COMPRESSION_JPEG:
                {
                unsigned short*  pBitPerSample = 0;
                uint32_t  NbSample=0;
                pi_pTIFFFile->GetField(BITSPERSAMPLE, &NbSample, &pBitPerSample);

                // if the file is a 16bits by sample (48 or 64 bits) and Jpeg compression.
                // We support that only in ReadOnly.
                // Tiff 12bits is in the case.
                //
                if (pBitPerSample[0] == 16 && (NbSample == 3 || NbSample == 4))
                    {
                    // disable the JPEG 12 bits in write/create access
                    if (pi_pTIFFFile->GetFilePtr()->GetAccessMode().m_HasWriteAccess || pi_pTIFFFile->GetFilePtr()->GetAccessMode().m_HasCreateAccess )
                        throw HRFAccessModeForCodeNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                    }

                pCodec = new HCDCodecIJG();

                unsigned short SamplePerPixel = 0;
                unsigned short Photometric    = 0;

                pi_pTIFFFile->GetField(SAMPLESPERPIXEL, &SamplePerPixel);
                pi_pTIFFFile->GetField(PHOTOMETRIC    , &Photometric);

                // !!!! HChkSebG !!!! => must take care if jpeg data source has been stored as RGB (should'nt but happen..) or YCbCr...
                if ( Photometric == PHOTOMETRIC_RGB && SamplePerPixel == 3)
                    ((HFCPtr<HCDCodecIJG>&)pCodec)->SetSourceColorMode(HCDCodecIJG::RGB);

                // UShort Quality;
                // pi_pTIFFFile->GetField(COMPRESSION_QUALITY, &Quality);
                // ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality(Quality);
                }
            break;
#ifdef JBIG_SUPPORT
            case COMPRESSION_JBIG:
                {
                pCodec = new HCDCodecJBIG();
                }
            break;
#endif
            case COMPRESSION_CCITTRLE:
            case COMPRESSION_CCITTFAX4:
                pCodec = new HCDCodecHMRCCITT();
                ((HFCPtr<HCDCodecHMRCCITT>&)pCodec)->SetCCITT3(false);
                break;
            case COMPRESSION_CCITTFAX3:
                pCodec = new HCDCodecHMRCCITT();
                ((HFCPtr<HCDCodecHMRCCITT>&)pCodec)->SetCCITT3(true);
                break;

            case COMPRESSION_DEFLATE:
            case COMPRESSION_ADOBE_DEFLATE:
                pCodec = new HCDCodecZlib();
                break;

        case COMPRESSION_LZW:         
                // Create codec for LZW (compress) compression.
                pCodec = new HCDCodecLZW();
                break;

            case COMPRESSION_PACKBITS:
                pCodec = new HCDCodecHMRPackBits();
                break;
            default:
                pi_pTIFFFile->SetDirectory(CurDir);
                throw HRFCodecNotSupportedException(pi_pTIFFFile->GetURL()->GetURL());
                break;
            }
        }
    else
        {
        pi_pTIFFFile->SetDirectory(CurDir);
        throw HRFBadPageNumberException(pi_pTIFFFile->GetURL()->GetURL());
        }

    pi_pTIFFFile->SetDirectory(CurDir);

    HPOSTCONDITION(pCodec != 0);
    return pCodec;
    }

//-----------------------------------------------------------------------------
// protected
// Reload the directories of the tiff file format
//-----------------------------------------------------------------------------
void HRFTiffFile::ReloadDescriptors()
    {
    HPRECONDITION(GetFilePtr() != 0);

    // Lock the sister file for the Write PrivateDirectory method
    HFCLockMonitor SisterFileLock (GetLockManager());

    GetFilePtr()->ReadDirectories();

    // Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// protected
// Save the directories of the tiff file format
//-----------------------------------------------------------------------------
void HRFTiffFile::SaveDescriptors(uint32_t pi_Page)
    {
    HPRECONDITION (GetFilePtr() != 0);

    // Lock the sister file for the Write PrivateDirectory method
    HFCLockMonitor SisterFileLock (GetLockManager());

    GetFilePtr()->WriteDirectories();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    }

//-----------------------------------------------------------------------------
// private
// Return the AccessMode, the current access mode is modified only for some
// speical tiff file.
//-----------------------------------------------------------------------------
HFCAccessMode HRFTiffFile::GetSpecialAccessMode (const HFCAccessMode& pi_rCurrentAccessMode) const
    {
    HFCAccessMode CurAccessMode = pi_rCurrentAccessMode;
    if (GetFilePtr()->IsSimulateLine_OneStripPackBitRGBAFile())
        {
        CurAccessMode = HFC_READ_ONLY;
        }

    return CurAccessMode;
    }

//-----------------------------------------------------------------------------
// private
// Return the BlockAccessType, the current block access tyoe is modified only for some
// speical tiff file.
//-----------------------------------------------------------------------------
HRFBlockAccess HRFTiffFile::GetSpecialBlockAccessType (const HRFBlockAccess& pi_rCurrentBlockAccessType) const
    {
    HRFBlockAccess CurBlockAccess = pi_rCurrentBlockAccessType;
    if (GetFilePtr()->IsSimulateLine_OneStripPackBitRGBAFile())
        {
        CurBlockAccess = HRFBlockAccess::SEQUENTIAL;
        }

    return CurBlockAccess;
    }

//-----------------------------------------------------------------------------
// private
// Write the MINSAMPLEVALUE or MAXSAMPLEVALUE TIFF tag to the current TIFF
// directory.
//-----------------------------------------------------------------------------
void HRFTiffFile::WriteSampleLimitValueToDir(vector<double> const&  pi_rSampleValues,
                                             bool                   pi_IsMinLimit,
                                             HTIFFFile*             po_pTIFFFile)
    {
    HPRECONDITION(po_pTIFFFile != 0);
    HPRECONDITION(!pi_rSampleValues.empty());

    const double Max = *max_element(pi_rSampleValues.begin(), pi_rSampleValues.end());
    const double Min = *min_element(pi_rSampleValues.begin(), pi_rSampleValues.end());

    // Determine whether a case to short is possible
    const bool IsCastToShortPossible = (Max <= USHRT_MAX) || (Min >= 0);


    // Determine whether values contain a fractional part
    bool ContainFractionalPart = false;
    for (vector<double>::const_iterator It = pi_rSampleValues.begin(); It != pi_rSampleValues.end(); ++It)
        {
        if (!HNumeric<double>::EQUAL_EPSILON(fmod(*It, 1.0), 0.0))
            {
            ContainFractionalPart = true;
            break;
            }
        }


    const TIFFTag ExtendedTagID = (pi_IsMinLimit) ? SMINSAMPLEVALUE : SMAXSAMPLEVALUE;

    // Ensure that any previous tag is removed
    if (po_pTIFFFile->TagIsPresent(ExtendedTagID))
        po_pTIFFFile->RemoveTag(ExtendedTagID);

    // Write to extended tag when the value contain a fractional part or when no cast to baseline tag data type is possible
    if (ContainFractionalPart || !IsCastToShortPossible)
        {
        po_pTIFFFile->SetField(ExtendedTagID, (uint32_t)pi_rSampleValues.size(), &pi_rSampleValues[0]);
        }


    const TIFFTag BaselineTagID = (pi_IsMinLimit) ? MINSAMPLEVALUE : MAXSAMPLEVALUE;

    // Ensure that any previous tag is removed
    if (po_pTIFFFile->TagIsPresent(BaselineTagID))
        po_pTIFFFile->RemoveTag(BaselineTagID);

    // Write to baseline tag only when cast to baseline tag data type is possible
    if (IsCastToShortPossible)
        {
        // Cast values to MINSAMPLEVALUE's/MAXSAMPLEVALUE's storage type to an array
        HAutoPtr<unsigned short> pUShortSampleLimitValues(new unsigned short[pi_rSampleValues.size()]);
        transform(pi_rSampleValues.begin(), pi_rSampleValues.end(), pUShortSampleLimitValues.get(), StaticCast<unsigned short>());


        po_pTIFFFile->SetField(BaselineTagID,
                               (uint32_t)pi_rSampleValues.size(),
                               pUShortSampleLimitValues.get());
        }

    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFTiffFile::HRFTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                HFCAccessMode         pi_AccessMode,
                                uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset), m_NumberDir(0)
    {
    // The ancestor store the access mode
    // To do ....
    m_pTiff  = 0;
    m_IsOpen = false;

    // if Open success and it is not a new file
    if (Open() && !GetAccessMode().m_HasCreateAccess)
        {
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFTiffFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }




//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFTiffFile::HRFTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                HFCAccessMode         pi_AccessMode,
                                uint64_t             pi_Offset,
                                bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset), m_NumberDir(0)
    {
    // The ancestor store the access mode
    // To do ....
    m_pTiff  = 0;
    m_IsOpen = false;
    }


//-----------------------------------------------------------------------------
// protected
// SetStandardDirectory
//-----------------------------------------------------------------------------
void HRFTiffFile::SetDirectory(HTIFFFile::DirectoryID pi_DirID)
    {
    if (!m_pTiff->SetDirectory(pi_DirID))
        throw HRFBadPageNumberException(GetURL()->GetURL());
    }

//-----------------------------------------------------------------------------
// protected
// GetReadPtr   - Get the TIFF file pointer.
//-----------------------------------------------------------------------------
HTIFFFile* HRFTiffFile::GetFilePtr  () const
    {
    HPRECONDITION (m_pTiff != 0);

    return (m_pTiff);
    }

//-----------------------------------------------------------------------------
// public
// Instanciation of the Sharing Control object.
//-----------------------------------------------------------------------------
void HRFTiffFile::SharingControlCreate()
    {
    if (1) // Disable GetFilePtr()->GetSynchroOffsetInFile() == 0)
        {
        // There is no Sharing Control counter in the file, we must create a sister file
        HASSERT (GetURL() != 0);
        if (m_pSharingControl == 0)
            {
            if (!s_BypassFileSharing)
                m_pSharingControl = new HRFSisterFileSharing(GetURL(), GetAccessMode());
            else
                m_pSharingControl = new HRFSisterFileSharing(GetURL(), GetAccessMode(), true);
            }
        }
#if 0 // Disable
    else
        {
        // There is a sharing control counter in the file, we will use it instead of
        // a sister file.
        HASSERT (GetFilePtr()->GetFilePtr() != 0);
        if (m_pSharingControl == 0)
            m_pSharingControl = new HRFCacheFileSharing(GetFilePtr()->GetFilePtr(),
                                                        GetFilePtr()->GetLockManager(),
                                                        GetFilePtr()->GetSynchroOffsetInFile(),
                                                        GetAccessMode());
        }
#endif
    }
