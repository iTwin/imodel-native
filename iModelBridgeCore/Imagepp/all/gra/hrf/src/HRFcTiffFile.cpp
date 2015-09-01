//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFcTiffFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFcTiffFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HTIFFFile.h>

#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFiTiffTileEditor.h>
#include <Imagepp/all/h/HRFiTiffStripEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecLZW.h>

#include <Imagepp/all/h/HRPComplexFilter.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPTypeAdaptFilters.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>



// Defintion of the string used to identify the filters.
const string strBlur("BLUR");
const string strBrightness("BRIGHTNESS");
const string strColorTwist("COLORTWIST");
const string strContrast("CONTRAST");
const string strDetail("DETAIL");
const string strEdgeEnhance("EDGE_ENHANCE");
const string strFindEdges("FIND_EDGES");
const string strSharpen("SHARPEN");
const string strSmooth("SMOOTH");


//-----------------------------------------------------------------------------
// HRFcTiffBlockCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE, // AccessMode
                                  LONG_MAX,              // MaxSizeInBytes
                                  1,                     // MinWidth
                                  LONG_MAX,              // MaxWidth
                                  1,                     // WidthIncrement
                                  1,                     // MinHeight
                                  LONG_MAX,              // MaxHeight
                                  1,                     // HeightIncrement
                                  false));               // Not Square

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   1,                      // MinHeight
                                   LONG_MAX,               // MaxHeight
                                   1));                    // HeightIncrement

        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodecV24R8G8B8Capabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV24R8G8B8Capabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecV24R8G8B8Capabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodecV24PhotoYCCCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV24PhotoYCCCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecV24PhotoYCCCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // LZW Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodecV16R5G6B5Capabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV16R5G6B5Capabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecV16R5G6B5Capabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFcTiffCodecV32RGBAlphaCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV32RGBAlphaCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecV32RGBAlphaCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Flashpix
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecFlashpix::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        }
    };



//-----------------------------------------------------------------------------
// HRFcTiffCodecV32CMYK
//-----------------------------------------------------------------------------
class HRFcTiffCodecV32CMYKCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecV32CMYKCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        }
    };



//-----------------------------------------------------------------------------
// HRFcTiffCodec1BitCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodec1BitCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodec1BitCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMR RLE1
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRRLE1::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodecV8GrayCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV8GrayCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecV8GrayCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodecPaletteCapabilities
// Used by : PixelTypeV16PRGray8A8
//           PixelTypeI2R8G8B8
//           PixelTypeI4R8G8B8, PixelTypeI4R8G8B8A8
//           PixelTypeI8R8G8B8, PixelTypeI8R8G8B8A8
//-----------------------------------------------------------------------------
class HRFcTiffCodecPaletteCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecPaletteCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodec16BitsPerChannelCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodec16BitsPerChannelCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCodecFloatCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecFloatCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFcTiffCodecFloatCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));

        // Codec LZW (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFcTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFcTiffCapabilities
//-----------------------------------------------------------------------------
HRFcTiffCapabilities::HRFcTiffCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Pixel Type capabilities

    // PixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV24R8G8B8;
    pPixelTypeV24R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                                     new HRFcTiffCodecV24R8G8B8Capabilities());
    pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV24R8G8B8);

    // PixelTypeV24B8G8R8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV24B8G8R8;
    pPixelTypeV24B8G8R8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                     HRPPixelTypeV24B8G8R8::CLASS_ID,
                                                     new HRFcTiffCodecV24R8G8B8Capabilities());
    pPixelTypeV24B8G8R8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV24B8G8R8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV24B8G8R8);

    // PixelTypeV16R5G6B5
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV16R5G6B5;
    pPixelTypeV16R5G6B5 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                     HRPPixelTypeV16R5G6B5::CLASS_ID,
                                                     new HRFcTiffCodecV16R5G6B5Capabilities());
    // No sampler available
    //pPixelTypeV16R5G6B5->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV16R5G6B5->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV16R5G6B5);


    // PixelTypeV32R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32R8G8B8A8;
    pPixelTypeV32R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                       HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                                       new HRFcTiffCodecV32RGBAlphaCapabilities());
    pPixelTypeV32R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32R8G8B8A8);

    // PixelTypeV32R8G8B8X8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32R8G8B8X8;
    pPixelTypeV32R8G8B8X8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                       HRPPixelTypeV32R8G8B8X8::CLASS_ID,
                                                       new HRFcTiffCodecV32RGBAlphaCapabilities());
    pPixelTypeV32R8G8B8X8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32R8G8B8X8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32R8G8B8X8);

    // PixelTypeV32B8G8R8X8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32B8G8R8X8;
    pPixelTypeV32B8G8R8X8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                       HRPPixelTypeV32B8G8R8X8::CLASS_ID,
                                                       new HRFcTiffCodecV32RGBAlphaCapabilities());
    pPixelTypeV32B8G8R8X8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32B8G8R8X8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32B8G8R8X8);

    // PixelTypeV32PR8PG8PB8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32PR8PG8PB8A8;
    pPixelTypeV32PR8PG8PB8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                          HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID,
                                                          new HRFcTiffCodecV32RGBAlphaCapabilities());
    pPixelTypeV32PR8PG8PB8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32PR8PG8PB8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32PR8PG8PB8A8);

    // PixelTypeV32CMYK
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32CMYK;
    pPixelTypeV32CMYK = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV32CMYK::CLASS_ID,
                                                   new HRFcTiffCodecV32CMYKCapabilities());
    pPixelTypeV32CMYK->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32CMYK->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32CMYK);

    // PixelTypeV96R32G32B32
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV96R32G32B32;
    pPixelTypeV96R32G32B32 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV96R32G32B32::CLASS_ID,
                                                        new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeV96R32G32B32->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV96R32G32B32);

    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1Gray1;
    pPixelTypeV1Gray1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                                   new HRFcTiffCodec1BitCapabilities());
    pPixelTypeV1Gray1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1Gray1);

    // PixelTypeV1GrayWhite1
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1GrayWhite1;
    pPixelTypeV1GrayWhite1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV1GrayWhite1::CLASS_ID,
                                                        new HRFcTiffCodec1BitCapabilities());
    pPixelTypeV1GrayWhite1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1GrayWhite1);

    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8Gray8;
    pPixelTypeV8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                                   new HRFcTiffCodecV8GrayCapabilities());
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8Gray8);

    // PixelTypeV8GrayWhite8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8GrayWhite8;
    pPixelTypeV8GrayWhite8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV8GrayWhite8::CLASS_ID,
                                                        new HRFcTiffCodecV8GrayCapabilities());
    pPixelTypeV8GrayWhite8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8GrayWhite8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8GrayWhite8);

    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI1R8G8B8;
    pPixelTypeI1R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI1R8G8B8::CLASS_ID,
                                                    new HRFcTiffCodec1BitCapabilities());
    pPixelTypeI1R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI1R8G8B8);

    // PixelTypeI1R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI1R8G8B8A8;
    pPixelTypeI1R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeI1R8G8B8A8::CLASS_ID,
                                                      new HRFcTiffCodec1BitCapabilities());
    pPixelTypeI1R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI1R8G8B8A8);

    // PixelTypeI2R8G8B8
    // Read/Write/Create capabilities
    // HFCPtr<HRFPixelTypeCapability> pPixelTypeI2R8G8B8;
    // pPixelTypeI2R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
    //                                                 HRPPixelTypeI2R8G8B8::CLASS_ID,
    //                                                 new HRFcTiffCodecPaletteCapabilities());
    // pPixelTypeI2R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    // Add((HFCPtr<HRFCapability>&)pPixelTypeI2R8G8B8);

    HFCPtr<HRFPixelTypeCapability> pPixelTypeI4R8G8B8;
    pPixelTypeI4R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI4R8G8B8::CLASS_ID,
                                                    new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeI4R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI4R8G8B8);

    // PixelTypeI4R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI4R8G8B8A8;
    pPixelTypeI4R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeI4R8G8B8A8::CLASS_ID,
                                                      new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeI4R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI4R8G8B8A8);

    // PixelTypeI8Gray8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8Gray8;
    pPixelTypeI8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeI8Gray8::CLASS_ID,
                                                   new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeI8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8Gray8);

    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8;
    pPixelTypeI8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI8R8G8B8::CLASS_ID,
                                                    new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeI8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8);

    // PixelTypeI8R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8A8;
    pPixelTypeI8R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeI8R8G8B8A8::CLASS_ID,
                                                      new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeI8R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8A8);

    // PixelTypeI8VAR8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8VA8R8G8B8;
    pPixelTypeI8VA8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                       HRPPixelTypeI8VA8R8G8B8::CLASS_ID,
                                                       new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeI8VA8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8VA8R8G8B8);

    // PixelTypeV16PRGray8A8
    // Read/Write capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV16PRGray8A8;
    pPixelTypeV16PRGray8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV16PRGray8A8::CLASS_ID,
                                                        new HRFcTiffCodecPaletteCapabilities());
    pPixelTypeV16PRGray8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV16PRGray8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV16PRGray8A8);

    // PixelTypeV48R16B16G16
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV48R16G16B16;
    pPixelTypeV48R16G16B16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV48R16G16B16::CLASS_ID,
                                                        new HRFcTiffCodec16BitsPerChannelCapabilities());
    pPixelTypeV48R16G16B16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV48R16G16B16);

    // HRPPixelTypeV64R16G16B16A16
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV64R16G16B16A16;
    pPixelTypeV64R16G16B16A16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                           HRPPixelTypeV64R16G16B16A16::CLASS_ID,
                                                           new HRFcTiffCodec16BitsPerChannelCapabilities());
    pPixelTypeV64R16G16B16A16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV64R16G16B16A16);

    // HRPPixelTypeV64R16G16B16A16
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV64R16G16B16X16;
    pPixelTypeV64R16G16B16X16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                           HRPPixelTypeV64R16G16B16X16::CLASS_ID,
                                                           new HRFcTiffCodec16BitsPerChannelCapabilities());
    pPixelTypeV64R16G16B16X16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV64R16G16B16X16);

    // PixelTypeV16Gray16
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV16Gray16;
    pPixelTypeV16Gray16= new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeV16Gray16::CLASS_ID,
                                                    new HRFcTiffCodec16BitsPerChannelCapabilities());
    pPixelTypeV16Gray16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV16Gray16);

    // PixelTypeV16Int16
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV16Int16;
    pPixelTypeV16Int16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeV16Int16::CLASS_ID,
                                                    new HRFcTiffCodec16BitsPerChannelCapabilities());
    pPixelTypeV16Int16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);

    Add((HFCPtr<HRFCapability>&)pPixelTypeV16Int16);

    // PixelTypeV32Float32
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32Float32;
    pPixelTypeV32Float32 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeV32Float32::CLASS_ID,
                                                      new HRFcTiffCodecFloatCapabilities());
    pPixelTypeV32Float32->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);

    Add((HFCPtr<HRFCapability>&)pPixelTypeV32Float32);

    // PixelTypeV24PhotoYCC
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV24PhotoYCC;
    pPixelTypeV24PhotoYCC = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                       HRPPixelTypeV24PhotoYCC::CLASS_ID,
                                                       new HRFcTiffCodecV24PhotoYCCCapabilities());
    pPixelTypeV24PhotoYCC->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV24PhotoYCC);


    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));

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

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Multi Page capability
    Add(new HRFMultiPageCapability(HFC_READ_WRITE_CREATE));

    // Embeding capability
    Add(new HRFEmbedingCapability(HFC_READ_WRITE_CREATE));

    // Shape capability
    Add(new HRFClipShapeCapability(HFC_READ_WRITE_CREATE, HRFCoordinateType::PHYSICAL));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_WRITE_CREATE));

    // BlockDataFlag capability
    Add(new HRFBlocksDataFlagCapability(HFC_READ_WRITE_CREATE));

    // Sub sampling capability
    Add(new HRFSubSamplingCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMake));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeModel));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeInkNames));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeCopyright));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeOnDemandRastersInfo));

    // Add the supported filters.
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPBlurAdaptFilter::CLASS_ID));
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPContrastFilter::CLASS_ID));
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPDetailFilter::CLASS_ID));
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPEdgeEnhanceFilter::CLASS_ID));
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPFindEdgesFilter::CLASS_ID));
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPSharpenAdaptFilter::CLASS_ID));
    Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPSmoothFilter::CLASS_ID));

    Add(new HRFThumbnailCapability(HFC_READ_WRITE_CREATE,
                                   128,                   // pi_MinWidth
                                   128,                   // pi_MaxWidth
                                   0,                     // pi_WidthIncrement
                                   128,                   // pi_MinHeight
                                   128,                   // pi_MaxHeight
                                   0,                     // pi_HeightIncrement
                                   65535,                 // pi_MaxSizeInBytes
                                   false));               // pi_IsComposed

    }

HFC_IMPLEMENT_SINGLETON(HRFcTiffCreator)

//-----------------------------------------------------------------------------
// HRFcTiffCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFcTiffCreator::HRFcTiffCreator()
    : HRFRasterFileCreator(HRFcTiffFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFcTiffCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_cTiff()); //Cache TIFF File Format
    }

// Identification information
WString HRFcTiffCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFcTiffCreator::GetExtensions() const
    {
    return WString(L"*.cTIFF");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFcTiffCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                              HFCAccessMode         pi_AccessMode,
                                              uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFcTiffFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

// Opens the file and verifies if it is the right type
bool HRFcTiffCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                    uint64_t             pi_Offset) const
    {
    HAutoPtr<HTIFFFile>  pTiff;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);


    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    (const_cast<HRFcTiffCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (((pTiff->IsValid(&pErr)) || ((pErr != 0) && !pErr->IsFatal())) && (pTiff->IsTiff64() == false))
        {
        bResult = true;

        // validate each pages
        uint32_t PageCount = pTiff->NumberOfPages();
        if (PageCount == 0)
            {
            bResult = ValidatePageDirectory(pTiff, 0, CTIFF_VERSION);
            }
        else
            {
            for (uint32_t Page = 0; Page < PageCount && bResult; Page++)
                bResult = ValidatePageDirectory(pTiff, Page, CTIFF_VERSION);
            }
        }
    else
        bResult = false;


    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFcTiffCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFcTiffCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }
//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// ValidatePageDirectory
//
// Validate the page directory
//-----------------------------------------------------------------------------
bool HRFcTiffCreator::ValidatePageDirectory(HTIFFFile* pi_pTiffFilePtr, uint32_t pi_Page, uint32_t pi_HMRVersion) const
    {
    bool   EmptyPage = false;
    bool   bResult = true;

    if (pi_Page == 0)
        {
        bResult = pi_pTiffFilePtr->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, 0));
        }
    else
        {
        uint32_t ImageType;
        bResult = pi_pTiffFilePtr->SetPage(pi_Page);
        if (!pi_pTiffFilePtr->GetField(SUBFILETYPE, &ImageType))
            bResult = false;
        else if ((ImageType & FILETYPE_PAGE) != 0 && ImageType != FILETYPE_EMPTYPAGE)
            bResult = false;
        else
            EmptyPage = (ImageType == FILETYPE_EMPTYPAGE);
        }

    if (bResult && !EmptyPage)
        {
        uint32_t DirOffset;
        // To detect if it is a cTiff file, verify if the private tag is present
        if (pi_pTiffFilePtr->GetField (HMR2_IMAGEINFORMATION, &DirOffset))
            {
            // validate each pages
            Byte* pTransPalette = 0;
            unsigned short HMRPixelTypeSpec;
            bResult = ValidateHMRDirectory(pi_pTiffFilePtr, pi_Page, pi_HMRVersion, &pTransPalette, &HMRPixelTypeSpec);

            if (bResult)
                {
                // Declare the current pixel type and codec
                HFCPtr<HRPPixelType> CurrentPixelType;
                HFCPtr<HCDCodec>     CurrentCodec;
                try
                    {
                    // Create the current pixel type and codec
                    CurrentPixelType = HRFTiffFile::CreatePixelTypeFromFile(pi_pTiffFilePtr,
                                                                            pi_Page,
                                                                            pTransPalette,
                                                                            HMRPixelTypeSpec);
                    CurrentCodec     = HRFTiffFile::CreateCodecFromFile(pi_pTiffFilePtr);
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
                                                                        new HRFcTiffBlockCapabilities()));

                    // Create the capability for the current pixel type and codec
                    HFCPtr<HRFCapability> pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                                                            CurrentPixelType->GetClassID(),
                                                                                            pCurrentCodecCapability);

                    // Check if we support these pixel type and codec
                    bResult = ((HRFRasterFileCreator*)this)->GetCapabilities()->Supports(pPixelTypeCapability);
                    }
                }
            }
        else
            bResult = false;
        }
    return bResult;
    }
//-----------------------------------------------------------------------------
// protected
// ValidateHMRDirectory
//
// Validate if the field contain in the HMR directoty is valid.
//-----------------------------------------------------------------------------

bool HRFcTiffCreator::ValidateHMRDirectory(HTIFFFile*  pi_pTiffFilePtr,
                                            uint32_t    pi_Page,
                                            uint32_t    pi_HMRVersion,
                                            Byte**    po_pTransPalette,
                                            unsigned short*    po_pHMRPixelTypeSpec) const
    {
    HPRECONDITION (pi_pTiffFilePtr != 0);

    // Get HMR directirie information.
    HTIFFFile::DirectoryID CurDir = pi_pTiffFilePtr->CurrentDirectory();
    uint32_t Version;
    bool    IsValid = true;

    // Check if the private tag is present
    if (pi_pTiffFilePtr->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
        {
        // we support all minor version and all version before
        if (!pi_pTiffFilePtr->GetField(HMR_VERSION, &Version) || (Version != pi_HMRVersion))
            IsValid = false;

        // Check transparency palette .
        uint32_t Count;
        pi_pTiffFilePtr->GetField(HMR_TRANSPARENCY_PALETTE, &Count, po_pTransPalette);

        // Check Pixel Type Spec.
        if (!pi_pTiffFilePtr->GetField(HMR_PIXEL_TYPE_SPEC, po_pHMRPixelTypeSpec))
            po_pHMRPixelTypeSpec = PIXELTYPESPEC_RGB;

        // Reset Directory
        pi_pTiffFilePtr->SetDirectory(CurDir);
        }

    return IsValid;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of HMR++ file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFcTiffCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFcTiffCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFcTiffFile::HRFcTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                  HFCAccessMode         pi_AccessMode,
                                  uint64_t             pi_Offset)
    : HRFiTiffFile(pi_rURL, pi_AccessMode, pi_Offset, true)
    {
    m_OriginalFileAccessMode = HFC_NO_ACCESS;
    m_SourceFileCreationTimeChanged = false;

    // if Open success and it is not a new file
    if (Open() && !GetAccessMode().m_HasCreateAccess)
        {
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }


//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file without open
//-----------------------------------------------------------------------------
HRFcTiffFile::HRFcTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                  HFCAccessMode         pi_AccessMode,
                                  uint64_t             pi_Offset,
                                  bool                 pi_DontOpenFile)
    : HRFiTiffFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    m_OriginalFileAccessMode = HFC_NO_ACCESS;
    m_SourceFileCreationTimeChanged = false;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFcTiffFile::~HRFcTiffFile()
    {
    try
        {
        SavecTiffFile();
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }


//-----------------------------------------------------------------------------
// public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFcTiffFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // same code than HTFiTiffFile

    // Validation if it's possible to add a page
    // Validate the page and all resolutions with the HMR capabilities
    // Create other resolution if necessary
    HPRECONDITION(pi_pPage->CountResolutions() > 0 || pi_pPage->IsEmpty());


    if (!pi_pPage->IsEmpty())
        {
        // Change the flags to empty for each each resolution
        for (unsigned short Resolution=0; Resolution < pi_pPage->CountResolutions(); Resolution++)
            {
            // Obtain the resolution descriptor
            HFCPtr<HRFResolutionDescriptor> pResolution = pi_pPage->GetResolutionDescriptor(Resolution);
            // We set in creation for the new page all flags to empty
            if (!pResolution->HasBlocksDataFlag())
                {
                HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
                HPRECONDITION(pResolution->CountBlocks() <= SIZE_MAX);
                HPRECONDITION(pResolution->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);
                pBlocksDataFlag = new HRFDataFlag[(size_t)pResolution->CountBlocks()];

                memset(pBlocksDataFlag, HRFDATAFLAG_EMPTY | HRFDATAFLAG_DIRTYFORSUBRES , (size_t)(pResolution->CountBlocks() * sizeof(HRFDataFlag)));
                pResolution->SetBlocksDataFlag(pBlocksDataFlag);
                }

            // set JPEG codec embeded
            // If we have a HFCCodecIJG we need to set its embeded.
            if (pResolution->GetCodec()->GetClassID() == HCDCodecIJG::CLASS_ID)
                ((HFCPtr<HCDCodecIJG>&)pResolution->GetCodec())->SetAbbreviateMode(true);
            }

        // always we have a transfo model with HMR
        if (!pi_pPage->HasTransfoModel())
            {
            HFCPtr<HGF2DTransfoModel> NullModel;
            NullModel = new HGF2DIdentity();
            pi_pPage->SetTransfoModel(*NullModel);
            }
        }

    // Add the page descriptor to the list
    if (HRFTiffFile::AddPage(pi_pPage))
        {
        // realloc the array of HMR Header
        uint32_t PageCount = CountPages();
        HArrayAutoPtr<HAutoPtr<HMRHeader> > ppHMRHeaders(new HAutoPtr<HMRHeader>[PageCount]);
        for (uint32_t i = 0; i < PageCount - 1; i++)
            ppHMRHeaders[i] = m_ppHMRHeaders[i].release();

        m_ppHMRHeaders = ppHMRHeaders.release();
        HAutoPtr<HMRHeader> pHMRHeader(new HMRHeader);
        InitPrivateTagDefault(pHMRHeader);
        pHMRHeader->m_HMRDirDirty = true;

        m_ppHMRHeaders[PageCount - 1] = pHMRHeader.release();
        return true;
        }
    else
        return false;
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFcTiffFile::GetFileCurrentSize() const
    {
    return HRFiTiffFile::GetFileCurrentSize();
    }


//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFcTiffFile::GetCapabilities () const
    {
    return HRFcTiffCreator::GetInstance()->GetCapabilities();
    }


//-----------------------------------------------------------------------------
// Public
// SetSourceFile_CreationDateTime
// pi_DateTime : Must be the value return by ctime() function
//
// Note : This information is stored in the HMR private directory but it's a
//        global information. For this reason, the HMR_SOURCEFILE_CREATIONDATE
//        is always stored in the HMR directory in the first page
//
// *** That information is only keep for back compatible between V8i, on Windows platform.
//-----------------------------------------------------------------------------
void HRFcTiffFile::SetSourceFile_CreationDateTime(const char* pi_DateTime)
    {
    HPRECONDITION(pi_DateTime != 0);

    m_SourceFileCreationTime = pi_DateTime;
    m_SourceFileCreationTimeChanged = true;
    }

void HRFcTiffFile::Save()
    {
    SavecTiffFile();
    HRFiTiffFile::Save();
    }

//-----------------------------------------------------------------------------

//---------------------------------------------------------- Protected

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//
// Because the HRFiTIFFFile was created without opened it, we need to create
// all descriptor.
//-----------------------------------------------------------------------------
void HRFcTiffFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    uint32_t PageIndex;
    uint32_t ImageType;
    HAutoPtr<HRFiTiffFile::HMRHeader> pHMRHeader;

    // Create the descriptor for each resolution of each page
    uint32_t PagesCount = CalcNumberOfPage();

    m_ppHMRHeaders = new HAutoPtr<HMRHeader>[PagesCount];

    for (uint32_t Page=0; Page < PagesCount; Page++)
        {
        // Select the page
        PageIndex = GetIndexOfPage(Page);
        SetImageInSubImage(PageIndex);

        if (!GetFilePtr()->GetField (SUBFILETYPE, &ImageType))
            throw HFCCorruptedFileException(GetURL()->GetURL());

        if (ImageType != FILETYPE_EMPTYPAGE)
            {
            pHMRHeader = new HRFiTiffFile::HMRHeader;
            // Init. default value for the Private Tags
            InitPrivateTagDefault (pHMRHeader);

            if (!ReadPrivateDirectory(Page, pHMRHeader))
                throw HFCFileNotSupportedException(GetURL()->GetURL());

            // initialize the creation date
            // *** That information is only keep for back compatible between V8i, on Windows platform.
            if (Page == 0)
                {
                if (m_SourceFileCreationTime.empty())
                    {
                    m_SourceFileCreationTime = pHMRHeader->m_SourceFileCreationTime;
                    }
                else
                    {
                    HASSERT(m_SourceFileCreationTime == string(pHMRHeader->m_SourceFileCreationTime));
                    }
                }

            // Reset Directory
            SetImageInSubImage(PageIndex);

            // TileFlags
            // Convert FileTileFlag to HRFDataFlag
            HASSERT(sizeof(HRFDataFlag) == sizeof(Byte));      // not necessary, same type
            uint32_t CurDataFlagsResolution = 0;

            // the main 1:1 width
            uint32_t MainWidth = 0;
            uint32_t Width;
            uint32_t Height;
            uint32_t BlockWidth;
            uint32_t BlockHeight;

            HFCPtr<HRPPixelType> PixelType;

            // MainWidth help to calc the resolution ratio
            GetFilePtr()->GetField(IMAGEWIDTH, &MainWidth);

            // Determindec the storage type
            GetFilePtr()->GetField(IMAGEWIDTH, &Width);
            GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);

            HRFBlockType StoragePage = ((GetFilePtr()->IsTiled()) ? HRFBlockType::TILE : HRFBlockType::STRIP);
            HRFDownSamplingMethod DownSamplingMethod;

            // Instantiation of Resolution descriptor
            HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
            unsigned short ResCount = CalcNumberOfSubResolution(PageIndex);
            for (unsigned short Resolution=0; Resolution <= ResCount; Resolution++)
                {
                // Obtain Resolution Information

                // Select the page and resolution
                SetImageInSubImage (PageIndex + Resolution);

                // DEBUG MODE ONLY
                // JPEG ISO Compression
                // Select the page
#if 0
                HDEBUGCODE(
                    unsigned short TESTJPEGISOCompression;
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
                // Select the page and resolution
                PixelType = CreatePixelTypeFromFile(GetFilePtr(),
                                                    Page,
                                                    pHMRHeader->m_pTransPalette,
                                                    pHMRHeader->m_HMRPixelTypeSpec,
                                                    &pHMRHeader->m_ChannelsWithNoDataValue,
                                                    &pHMRHeader->m_ChannelsNoDataValue);

                // Compression the GetCodecsList
                HFCPtr<HCDCodec> pCodec = CreateCodecFromFile(GetFilePtr(), Page);

                // convert PHOTOMETRIC value
                // if the compression is FLASHPIX or JPEG, the PHOTOMETRIC must be set to YCbCr
                unsigned short Compression;
                GetFilePtr()->GetField(COMPRESSION, &Compression);
                if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess )
                    {
                    unsigned short Photometric;
                    GetFilePtr()->GetField(PHOTOMETRIC, &Photometric);
                    if ((Compression == COMPRESSION_HMR_FLASHPIX || Compression == COMPRESSION_JPEG) &&
                        (Photometric == PHOTOMETRIC_RGB))
                        GetFilePtr()->SetField (PHOTOMETRIC, (unsigned short)PHOTOMETRIC_YCBCR);
                    }

                // resolution dimension
                GetFilePtr()->GetField(IMAGEWIDTH, &Width);
                GetFilePtr()->GetField(IMAGELENGTH, &Height);

                // Block dimension
                if (StoragePage == HRFBlockType::TILE)
                    {
                    GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);
                    GetFilePtr()->GetField(TILELENGTH, &BlockHeight);
                    }
                else
                    {
                    // Strip dimension
                    BlockWidth = Width;
                    GetFilePtr()->GetField(ROWSPERSTRIP, &BlockHeight);
                    }

                // DownSamplingMethod.
                DownSamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
                if (pHMRHeader->m_HasDecimationMethod && (pHMRHeader->m_DecimationMethodCount != 1))
                    DownSamplingMethod = GetDownSamplingMethod(pHMRHeader->m_pDecimationMethod[Resolution]);

                double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(MainWidth, Width);
                HFCPtr<HRFResolutionDescriptor> pResolution =
                    new HRFResolutionDescriptor(GetAccessMode(),                               // AccessMode,
                                                GetCapabilities(),                             // Capabilities,
                                                Ratio,                                         // XResolutionRatio,
                                                Ratio,                                         // YResolutionRatio,
                                                PixelType,                                     // PixelType,
                                                pCodec,                                        // Codecs,
                                                HRFBlockAccess::RANDOM,                        // RStorageAccess,
                                                HRFBlockAccess::RANDOM,                        // WStorageAccess,
                                                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL, // ScanLineOrientation,
                                                HRFInterleaveType::PIXEL,                      // InterleaveType
                                                false,                                         // IsInterlace,
                                                Width,                                         // Width,
                                                Height,                                        // Height,
                                                BlockWidth,                                    // BlockWidth,
                                                BlockHeight,                                   // BlockHeight,
                                                (pHMRHeader->m_iTiffTileFlagsLength == 0 ? 0 :             // BlocksDataFlag
                                                 (HRFDataFlag*)&(pHMRHeader->m_piTiffTileFlags[CurDataFlagsResolution])),
                                                StoragePage,
                                                1,                                             // NumberOfPass
                                                8,                                             // PaddingBits
                                                DownSamplingMethod);

                ListOfResolutionDescriptor.push_back(pResolution);

                // Incr list of DataFlag
                // + 1 because we have a marker to separate each resolution.

                if (StoragePage == HRFBlockType::TILE)
                    CurDataFlagsResolution += GetFilePtr()->NumberOfTiles() + 1;
                else
                    // Strip case
                    CurDataFlagsResolution += GetFilePtr()->NumberOfStrips() + 1;
                }
            // Thumbnail interface
            HFCPtr<HRFThumbnail>  pThumbnail = ReadThumbnailFromFile(Page);

            // TranfoModel
            // Select the page and resolution and read the image height, all
            // other Resolution are computed from this value.
            SetImageInSubImage (PageIndex);
            GetFilePtr()->GetField(IMAGELENGTH, &Height);

            HFCPtr<HGF2DTransfoModel> pTransfoModel = CreateTransfoModelFromTiffMatrix();

            HFCPtr<HRPHistogram> pHistogram;
            if (pHMRHeader->m_HistogramLength == 768)
                {
                uint32_t**  pEntryFrequencies;
                pEntryFrequencies = new uint32_t*[3];
                for (int ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
                    {
                    pEntryFrequencies   [ChannelIndex]  = new uint32_t[256];
                    memcpy(pEntryFrequencies[ChannelIndex], pHMRHeader->m_pHistogram + (ChannelIndex * 256), 256 * sizeof(uint32_t));
                    }
                pHistogram = new HRPHistogram(pEntryFrequencies, 256, 3);

                for (int ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
                    delete pEntryFrequencies[ChannelIndex];
                delete pEntryFrequencies;
                }
            else
                pHistogram = new HRPHistogram(pHMRHeader->m_pHistogram, pHMRHeader->m_HistogramLength);

            HAutoPtr<HRFClipShape> pClipShape(GetClipShape(pHMRHeader));

            // Tag information
            char*  pSystem;
            HPMAttributeSet TagList;
            HFCPtr<HPMGenericAttribute> pTag;

            // DOCUMENTNAME Tag
            if (GetFilePtr()->GetField(DOCUMENTNAME, &pSystem))
                {
                pTag = new HRFAttributeDocumentName(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // IMAGEDESCRIPTION Tag
            if (GetFilePtr()->GetField(IMAGEDESCRIPTION, &pSystem))
                {
                pTag = new HRFAttributeImageDescription(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // PAGENAME Tag
            if (GetFilePtr()->GetField(PAGENAME, &pSystem))
                {
                pTag = new HRFAttributePageName(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // MAKE Tag
            if (GetFilePtr()->GetField(MAKE, &pSystem))
                {
                pTag = new HRFAttributeMake(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // MODEL Tag
            if (GetFilePtr()->GetField(MODEL, &pSystem))
                {
                pTag = new HRFAttributeModel(WString(pSystem,false));
                TagList.Set(pTag);
                }


            // SOFTWARE Tag
            if (GetFilePtr()->GetField(SOFTWARE, &pSystem))
                {
                pTag = new HRFAttributeSoftware(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // DATETIME Tag
            if (GetFilePtr()->GetField(DATETIME, &pSystem))
                {
                pTag = new HRFAttributeDateTime(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // ARTIST Tag
            if (GetFilePtr()->GetField(ARTIST, &pSystem))
                {
                pTag = new HRFAttributeArtist(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // HOSTCOMPUTER Tag
            if (GetFilePtr()->GetField(HOSTCOMPUTER, &pSystem))
                {
                pTag = new HRFAttributeHostComputer(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // INKNAMES Tag
            if (GetFilePtr()->GetField(INKNAMES, &pSystem))
                {
                pTag = new HRFAttributeInkNames(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // RESOLUTIONUNIT Tag
            unsigned short UnitValue;
            if (GetFilePtr()->GetField(RESOLUTIONUNIT, &UnitValue))
                {
                pTag = new HRFAttributeResolutionUnit(UnitValue);
                TagList.Set(pTag);
                }
            // XRESOLUTION Tag
            RATIONAL XResolution;
            if (GetFilePtr()->GetField(XRESOLUTION, &XResolution))
                {
                pTag = new HRFAttributeXResolution(XResolution.Value);
                TagList.Set(pTag);
                }
            // YRESOLUTION Tag
            RATIONAL YResolution;
            if (GetFilePtr()->GetField(YRESOLUTION, &YResolution))
                {
                pTag = new HRFAttributeYResolution(YResolution.Value);
                TagList.Set(pTag);
                }
            // COPYRIGHT Tag
            if (GetFilePtr()->GetField(COPYRIGHT, &pSystem))
                {
                pTag = new HRFAttributeCopyright(WString(pSystem,false));
                TagList.Set(pTag);
                }

            // ONDEMANDRASTERS_INFO Tag
            if (!pHMRHeader->m_OnDemandRastersInfo.empty())
                {
                pTag = new HRFAttributeOnDemandRastersInfo(pHMRHeader->m_OnDemandRastersInfo);
                TagList.Set(pTag);
                }

            HAutoPtr<HRPFilter> pFilters(GetFilter());

            HFCPtr<HRFPageDescriptor> pPage;
            pPage = new HRFPageDescriptor (GetAccessMode(),
                                           GetCapabilities(),           // Capabilities,
                                           ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                           0,                           // RepresentativePalette,
                                           pHistogram,                  // Histogram,
                                           pThumbnail,                  // Thumbnail,
                                           pClipShape,                  // ClipShape,
                                           pTransfoModel,               // TransfoModel,
                                           pFilters,                    // Filters
                                           &TagList);                   // Tag

            m_ListOfPageDescriptor.push_back(pPage);
            }
        else
            m_ListOfPageDescriptor.push_back(new HRFPageDescriptor(true));  // add an empty page

        // the page is created, add the HMR Header into the list
        m_ppHMRHeaders[Page] = pHMRHeader.release();
        }
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
/*
void HRFcTiffFile::ReloadDescriptors()
{
    // Lock the sister file for the Write PrivateDirectory method
    HRFLockManager SisterFileLock (GetSharingControl());

    GetFilePtr()->ReadDirectories();

    // Unlock the sister file
    SisterFileLock.Unlock();

    char*   pTileFlags;

    UInt32  PreviousDirectory = GetFilePtr()->CurrentDirectory();

    // Reload Flags to the descriptor for each page
    for (UInt32 Page=0; Page < CountPages(); Page++)
    {
        // If directory not present, Add it.
        if (!GetFilePtr()->SetDirectory(HTIFFFile::HMR_DIRECTORY))
            GetFilePtr()->AddHMRDirectory(HMR2_IMAGEINFORMATION);

        // Get TileFlags
        if (GetFilePtr()->GetField(HMR2_TILEFLAG, &pTileFlags))
        {
            m_iTiffTileFlagsLength = strlen(pTileFlags)+1;
            if (memcmp (m_piTiffTileFlags, pTileFlags, m_iTiffTileFlagsLength) != 0)
            {

                m_piTiffTileFlags = new Byte[m_iTiffTileFlagsLength];
                memcpy(m_piTiffTileFlags, pTileFlags, m_iTiffTileFlagsLength * sizeof(char));
                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);
                UInt32 CurDataFlagsResolution = 0;

                // For each Resolution...
                for (UInt32 i=0; i<pPageDescriptor->CountResolutions(); i++)
                {
                    // Select the page and resolution
                    SetImageInSubImage (GetIndexOfPage(Page)+i);

                    HFCPtr<HRFResolutionDescriptor> pResDescriptor = pPageDescriptor->GetResolutionDescriptor(i);
                    pResDescriptor->SetBlocksDataFlag((HRFDataFlag*)&(m_piTiffTileFlags[CurDataFlagsResolution]));

                    // Incr list of DataFlag
                    // + 1 because we have a marker to separate each resolution.
                    if (pResDescriptor->GetBlockType() == HRFBlockType::TILE)
                        CurDataFlagsResolution += GetFilePtr()->NumberOfTiles() + 1;
                    else
                        // Strip case
                        CurDataFlagsResolution += GetFilePtr()->NumberOfStrips() + 1;
                }
            }
        }
    }
    GetFilePtr()->SetDirectory(PreviousDirectory);

}
*/
//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
/*
void HRFcTiffFile::SaveDescriptors()
{
    HPRECONDITION (GetFilePtr());
    HPRECONDITION(CountPages() > 0);
    HASSERT(sizeof(HRFDataFlag) == sizeof(Byte));

    UInt32  PreviousDirectory = GetFilePtr()->CurrentDirectory();

    HFCPtr<HRFPageDescriptor>       pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResDescriptor;
    UInt32                          CountFlags = 0;

    // For each Resolution...
    for (UInt32 i=0; i<pPageDescriptor->CountResolutions(); i++)
    {
        pResDescriptor            = pPageDescriptor->GetResolutionDescriptor(i);
        const HRFDataFlag* pFlags = pResDescriptor->GetBlocksDataFlag();
        UInt32 NbResFlags         = pResDescriptor->GetBlocksPerWidth() *
                                    pResDescriptor->GetBlocksPerHeight();

        // Checking....
        if (m_iTiffTileFlagsLength < (CountFlags+NbResFlags+1))
            HASSERT(0);

        // Check if flags are changed
        if (memcmp (&(m_piTiffTileFlags[CountFlags]), pFlags, NbResFlags) != 0)
        {
            memcpy(&(m_piTiffTileFlags[CountFlags]), pFlags, NbResFlags);
            m_HMRDirDirty = true;
        }
        CountFlags += NbResFlags;
        // dn Resolution.
        m_piTiffTileFlags[CountFlags] = HMR2_TILEFLAG_ENDRESOLUTION;
        CountFlags++;
    }
    m_piTiffTileFlags[CountFlags-1] = 0;


    // HMR Directory is changed ?
    if (m_HMRDirDirty)
    {
        // If directory not present, Add it.
        if (!GetFilePtr()->SetDirectory(HTIFFFile::HMR_DIRECTORY))
            GetFilePtr()->AddHMRDirectory(HMR2_IMAGEINFORMATION);

        // Set TilesFlag...
        if (m_iTiffTileFlagsLength > 0)
            GetFilePtr()->SetField(HMR2_TILEFLAG, (char*)m_piTiffTileFlags.get());

        // Reset Directory
        GetFilePtr()->SetDirectory(PreviousDirectory);

        // Lock the sister file for the Write PrivateDirectory method
        HRFLockManager SisterFileLock (GetSharingControl());

        GetFilePtr()->WriteDirectories();

        // Unlock the sister file.
        SisterFileLock.Unlock();

        m_HMRDirDirty = false;
    }
}
*/
//---------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// InitPrivateTagDefault -
//-----------------------------------------------------------------------------
void HRFcTiffFile::InitPrivateTagDefault (HRFiTiffFile::HMRHeader* po_pHMRHeader)
    {
    HPRECONDITION(po_pHMRHeader != 0);

    // call the ancestor
    HRFiTiffFile::InitPrivateTagDefault(po_pHMRHeader);

    po_pHMRHeader->m_Version       = CTIFF_VERSION;
    po_pHMRHeader->m_MinorVersion  = 0;
    }

//-----------------------------------------------------------------------------
// private
// WritePrivateDirectory -
//
// For better performance, the file must be set to the HMR directory before
// calling this method
//-----------------------------------------------------------------------------
void HRFcTiffFile::WritePrivateDirectory (uint32_t pi_Page)
    {
    HPRECONDITION(GetFilePtr() != 0);
    HASSERT(sizeof(HRFDataFlag) == sizeof(Byte));

    HFCPtr<HRFPageDescriptor> pPageDesc(GetPageDescriptor(pi_Page));

    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();

    // set on the right page
    SetImageInSubImage(GetIndexOfPage(pi_Page));

    if (!GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
        GetFilePtr()->AddHMRDirectory(HMR2_IMAGEINFORMATION);

    // HMR_SOURCEFILE_CREATIONDATE is stored in the HMR private directory but it's a global information,
    // we stored in the first directory
    //
    // *** That information is only keep for back compatible between V8i, on Windows platform.
    if (pi_Page == 0 && m_SourceFileCreationTimeChanged)
        GetFilePtr()->SetFieldA(HMR_SOURCEFILE_CREATIONDATE,  m_SourceFileCreationTime.c_str());

    if (!pPageDesc->IsEmpty())
        {
        HFCPtr<HRPPixelType> pPixelType = pPageDesc->GetResolutionDescriptor(0)->GetPixelType();

        // Set pixel type specification.
        if ((pPixelType->GetClassID() == HRPPixelTypeV24B8G8R8::CLASS_ID) ||
            (pPixelType->GetClassID() == HRPPixelTypeV32B8G8R8X8::CLASS_ID))
            GetFilePtr()->SetField(HMR_PIXEL_TYPE_SPEC, (unsigned short)PIXELTYPESPEC_BGR);

        // Set transparency palette.
        if (pPixelType->CountIndexBits() != 0 &&
            pPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
            {
            Byte* pTransPalette = new Byte[pPixelType->GetPalette().GetMaxEntries()];
            Byte* pSourceComposite;

            for (uint32_t i = 0; i < pPixelType->GetPalette().GetMaxEntries(); ++i)
                {
                pSourceComposite = (Byte*) (pPixelType->GetPalette().GetCompositeValue(i));
                pTransPalette[i] = (Byte) pSourceComposite[3];
                }
            GetFilePtr()->SetField(HMR_TRANSPARENCY_PALETTE,
                                   pPixelType->GetPalette().GetMaxEntries(),
                                   pTransPalette);
            }
        }
    GetFilePtr()->SetDirectory(CurDir);
    }

void HRFcTiffFile::SavecTiffFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        // Write Information
        if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
            {
            // Update the modification to the file
            for (uint32_t Page = 0; Page < CountPages(); Page++)
                {
                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

                // when the first page is empty, the only data we need to save is the SourceFileCreationTime
                if (Page == 0 && pPageDescriptor->IsEmpty() && m_SourceFileCreationTimeChanged)
                    {
                    // the HMR private directory is always present in the page 0
                    WritePrivateDirectory(0);
                    }
                else if (!pPageDescriptor->IsEmpty())
                    {
                    if (pPageDescriptor->HasFilter() && pPageDescriptor->FiltersHasChanged())
                        SetFilter(pPageDescriptor->GetFilter());

                    HRFAttributeInkNames const* pTag = pPageDescriptor->FindTagCP<HRFAttributeInkNames>();
                    if (pTag != NULL && (pPageDescriptor->TagHasChanged(*pTag) || GetAccessMode().m_HasCreateAccess))
                        {
                        GetFilePtr()->SetFieldA(INKNAMES, AString(pTag->GetData().c_str()).c_str());
                        }

                    if (pPageDescriptor->ThumbnailHasChanged())
                        {
                        // if the page has a thumbnail we add it to the file
                        AddThumbnailToFile(Page);
                        }
                    HPRECONDITION(GetAccessMode().m_HasCreateAccess || m_ppHMRHeaders[Page] != 0);

                    if (m_ppHMRHeaders[Page]->m_HMRDirDirty)
                        WritePrivateDirectory (Page);
                    }
                else if (GetAccessMode().m_HasCreateAccess)
                    {
                    WritePrivateDirectory (Page);
                    }
                }
            }

        }
    // The itiff ancestor save the others tags,
    // write the private directory and close the file if needed.
    }
//-----------------------------------------------------------------------------
// Protected
// Return the filter(s) associate with the image.
//-----------------------------------------------------------------------------
HRPFilter* HRFcTiffFile::GetFilter()
    {
    HPRECONDITION (m_IsOpen);

    bool               HasComplexFilter = false;
    char*              pFilterDesc;
    HAutoPtr<HRPFilter> pFilter;

    // Get the filters value.
    if (GetFilePtr()->GetField(HMR_FILTERS, &pFilterDesc))
        {
        HAutoPtr<HRPFilter> pTmpFilter;
        istringstream       FiltersStream(pFilterDesc);

        // Extract filter as long as we have some thing to extract.
        while (pTmpFilter = UnstreamFilter(FiltersStream))
            {
            if (HasComplexFilter)
                {
                ((HRPComplexFilter*)pFilter.get())->Insert(pTmpFilter.release());
                }
            else if (pFilter)
                {
                // We don't have complex filter yet, but we have more than
                // one filter.  We need to create a complex filter and to add
                // the filters into it.
                HAutoPtr<HRPComplexFilter> pComplex(new HRPComplexFilter());
                pComplex->Insert(pFilter.release());
                pComplex->Insert(pTmpFilter.release());

                // Get the complex filter and indicate it's presence.
                pFilter          = pComplex;
                HasComplexFilter = true;
                }
            else
                {
                // We have our first filter.
                pFilter = pTmpFilter;
                }
            }
        }
    return pFilter.release();
    }

//-----------------------------------------------------------------------------
// Protected
// Extract a word from the specified stream.
//-----------------------------------------------------------------------------
string HRFcTiffFile::ReadString(istringstream& pi_rStream) const
    {
    char Dummy;
    char Buffer[128];

    // Read a word from the stream.
    pi_rStream.get(Buffer, 128, ' ');
    pi_rStream.get(Dummy);

    return Buffer;
    }

//-----------------------------------------------------------------------------
// Protected
// Set the filter(s) associate with the image.
//
//-----------------------------------------------------------------------------
void HRFcTiffFile::SetFilter(const HRPFilter& pi_rFilter)
    {
    HPRECONDITION (m_IsOpen);

    ostringstream FilterStream;

    // Check if the filter is a complex one.
    if (pi_rFilter.GetClassID() == HRPComplexFilter::CLASS_ID)
        {
        HRPComplexFilter* pComplexFilter;
        bool             AddSpace = false;

        // Stream all filters of the complex filter.
        pComplexFilter = (HRPComplexFilter*)&pi_rFilter;
        const HRPComplexFilter::ListFilters& Filters = pComplexFilter->GetList();
        for (HRPComplexFilter::ListFilters::const_iterator Itr = Filters.begin() ;
             Itr != Filters.end() ; Itr++)
            {
            if(AddSpace)
                FilterStream << " ";

            StreamFilter(FilterStream, *(*Itr));
            AddSpace = true;
            }
        }
    else
        {
        // Since we only have on filter we stream it.
        StreamFilter(FilterStream, pi_rFilter);
        }

    // If we have filters we save it.
    if (!FilterStream.str().empty())
        {
        GetFilePtr()->SetFieldA(HMR_FILTERS, FilterStream.str().c_str());
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Get the filter and stream it into a string.
//-----------------------------------------------------------------------------
void HRFcTiffFile::StreamFilter(ostringstream& po_rStream, const HRPFilter& pi_rFilter) const
    {
    HCLASS_ID ClassKey;

    // Get the class key of the specified filter.
    ClassKey = pi_rFilter.GetClassID();

    // Check if the specified filter is a supported one, if so we stream it.
    if (ClassKey == HRPBlurAdaptFilter::CLASS_ID)
        {
        // Stream out the blur filter information.
        po_rStream << strBlur << " " << ((HRPBlurAdaptFilter*)&pi_rFilter)->GetIntensity();
        }
    else if (ClassKey == HRPSharpenAdaptFilter::CLASS_ID)
        {
        // Stream out the sharpen filter information.
        po_rStream << strSharpen << " " << ((HRPSharpenAdaptFilter*)&pi_rFilter)->GetIntensity();
        }
    else if (ClassKey == HRPContrastFilter::CLASS_ID)
        {
        // Stream out the contrast filter information.
        po_rStream << strContrast << " " << ((HRPContrastFilter*)&pi_rFilter)->GetIntensity();
        }
    else if (ClassKey == HRPColortwistFilter::CLASS_ID)
        {
        double (*pMatrix)[4][4];

        // Stream out the color twist matrix filter information.
        pMatrix = (double(*)[4][4])((HRPColortwistFilter*)&pi_rFilter)->GetMatrix();
        po_rStream << strColorTwist << " " << (*pMatrix)[0][0] << " "
                   << (*pMatrix)[0][1] << " "
                   << (*pMatrix)[0][2] << " "
                   << (*pMatrix)[0][3] << " "
                   << (*pMatrix)[1][0] << " "
                   << (*pMatrix)[1][1] << " "
                   << (*pMatrix)[1][2] << " "
                   << (*pMatrix)[1][3] << " "
                   << (*pMatrix)[2][0] << " "
                   << (*pMatrix)[2][1] << " "
                   << (*pMatrix)[2][2] << " "
                   << (*pMatrix)[2][3] << " "
                   << (*pMatrix)[3][0] << " "
                   << (*pMatrix)[3][1] << " "
                   << (*pMatrix)[3][2] << " "
                   << (*pMatrix)[3][3];
        }
    else if (ClassKey == HRPColorBalanceFilter::CLASS_ID)
        {
        // Stream out the brightness filter information.
        po_rStream << strBrightness << " " << ((HRPColorBalanceFilter*)&pi_rFilter)->GetRedVariation() << " "
                   << ((HRPColorBalanceFilter*)&pi_rFilter)->GetGreenVariation() << " "
                   << ((HRPColorBalanceFilter*)&pi_rFilter)->GetBlueVariation();
        }
    else if (ClassKey == HRPSmoothFilter::CLASS_ID)
        {
        // Stream out the smooth filter information.
        po_rStream << strSmooth;
        }
    else if (ClassKey == HRPDetailFilter::CLASS_ID)
        {
        // Stream out the detail filter information.
        po_rStream << strDetail;
        }
    else if (ClassKey == HRPEdgeEnhanceFilter::CLASS_ID)
        {
        // Stream out the edge enhance filter information.
        po_rStream << strEdgeEnhance;
        }
    else if (ClassKey == HRPFindEdgesFilter::CLASS_ID)
        {
        // Stream out the find edges filter information.
        po_rStream << strFindEdges;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Create a filter that fit with the information found into the specified
// stream.
// Note:
//   This method only build one filter, if the stream contain more than one
//   filter you need to call this method as long as the return value is not 0.
//-----------------------------------------------------------------------------
HRPFilter* HRFcTiffFile::UnstreamFilter(istringstream& pi_rStream) const
    {
    HAutoPtr<HRPFilter> pFilter;
    string              Value;

    // Read the name of the filter.
    Value = ReadString(pi_rStream);

    if (!Value.empty())
        {
        // Check the kind of filter used.
        if (Value == strBlur)
            {
            // Read the blur amount and create a filter.
            Value   = ReadString(pi_rStream);
            pFilter = new HRPBlurAdaptFilter((Byte)strtoul(Value.c_str(), 0, 10));
            }
        else if (Value == strSharpen)
            {
            // Read the sharpen amount and create a filter.
            Value   = ReadString(pi_rStream);
            pFilter = new HRPSharpenAdaptFilter((Byte)strtoul(Value.c_str(), 0, 10));
            }
        else if (Value == strContrast)
            {
            // Read the contrast amount and create a filter.
            Value = ReadString(pi_rStream);
            pFilter = new HRPColorBalanceFilter((Byte)strtol(Value.c_str(), 0, 10));
            }
        else if (Value == strColorTwist)
            {
            // Read the color twist matrix and create the filter.
            double Matrix[4][4];
            Value = ReadString(pi_rStream);
            Matrix[0][0] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[0][1] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[0][2] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[0][3] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[1][0] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[1][1] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[1][2] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[1][3] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[2][0] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[2][1] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[2][2] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[2][3] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[3][0] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[3][1] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[3][2] = strtod(Value.c_str(), 0);
            Value = ReadString(pi_rStream);
            Matrix[3][3] = strtod(Value.c_str(), 0);
            pFilter = new HRPColortwistFilter(Matrix);
            }
        else if (Value == strBrightness)
            {
            // Read the brightness filter values and create it.
            string RedValue(ReadString(pi_rStream));

            // Assume a global value for brightess filter.
#if 0
            string GreenValue(ReadString(pi_rStream));
            string BlueValue(ReadString(pi_rStream));
            pFilter = new HRPColorBalanceFilter(strtol(RedValue.c_str(), 0, 10),
                                                strtol(GreenValue.c_str(), 0, 10),
                                                strtol(BlueValue.c_str(), 0, 10));
#else
            pFilter = new HRPColorBalanceFilter(strtol(RedValue.c_str(), 0, 10));
#endif


            }
        else if (Value == strSmooth)
            {
            // Create a smooth filter.
            pFilter = new HRPSmoothFilter();
            }
        else if (Value == strDetail)
            {
            // Create a detail filter.
            pFilter = new HRPDetailFilter();
            }
        else if (Value == strEdgeEnhance)
            {
            // Create a edge enhance filter.
            pFilter = new HRPEdgeEnhanceFilter();
            }
        else if (Value == strFindEdges)
            {
            // Create a find edges filter.
            pFilter = new HRPFindEdgesFilter();
            }
        }

    return pFilter.release();
    }

//-----------------------------------------------------------------------------
// Public
// SetOriginalFileAccessMode
// Store the access mode of the original file
//-----------------------------------------------------------------------------
void  HRFcTiffFile::SetOriginalFileAccessMode(HFCAccessMode pi_AccessMode)
    {
    m_OriginalFileAccessMode = pi_AccessMode;
    }

/*
//-----------------------------------------------------------------------------
// ProjectWise project
//

//-----------------------------------------------------------------------------
// Public
// Read_ProjectWiseBlob
//-----------------------------------------------------------------------------
bool HRFiTiffFile::Read_ProjectWiseBlob (vector<Byte>* po_pData) const
{
    UInt32 BlobSize = 0;
    bool  Ret      = true;

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock(const_cast<HRFiTiffFile*>(this)->GetLockManager());

    GetFilePtr()->ReadProjectWiseBlob(0, &BlobSize);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    if (BlobSize != 0)
    {
        po_pData->resize(BlobSize);

        // Lock the sister file for the GetField operation
        HFCLockMonitor SisterFileLock(const_cast<HRFiTiffFile*>(this)->GetLockManager());

        Ret = GetFilePtr()->ReadProjectWiseBlob(&((*po_pData)[0]), 0);

        SisterFileLock.ReleaseKey();
    }
    else
        Ret = false;

    return Ret;
}

//-----------------------------------------------------------------------------
// Public
// Write_ProjectWiseBlob
//-----------------------------------------------------------------------------
bool HRFiTiffFile::Write_WriteProjectBlob(const vector<Byte>& pi_pData)
{
    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock (GetLockManager());

    return GetFilePtr()->WriteProjectWiseBlob(&(pi_pData[0]), pi_pData.size());

    // The sister file is automatically unlock at the destruction of
    // SisterFileLock
}

// ProjectWise project End
//-----------------------------------------------------------------------------

*/
