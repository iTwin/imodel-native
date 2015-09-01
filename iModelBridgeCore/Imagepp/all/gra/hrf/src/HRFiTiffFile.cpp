//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFiTiffFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Class HRFitiffFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLEmbedFile.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HTIFFDirectory.h>
#include <ImagePP/all/h/HTIFFGeoKey.h>

#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFiTiffTileEditor.h>
#include <Imagepp/all/h/HRFiTiffStripEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

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

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>

#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>




//-----------------------------------------------------------------------------
// HRFiTiffStripBlockCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffStripBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffStripBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   32,                     // MinHeight
                                   LONG_MAX,               // MaxHeight
                                   16));                   // HeightIncrement
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE,        // AccessMode
                                  LONG_MAX,              // MaxSizeInBytes
                                  256,                   // MinWidth
                                  256,                   // MaxWidth
                                  0,                     // WidthIncrement
                                  256,                   // MinHeight
                                  256,                   // MaxHeight
                                  0,                     // HeightIncrement
                                  false));               // Not Square
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffTileBlockCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffTileBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffTileBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE, // AccessMode
                                  LONG_MAX,              // MaxSizeInBytes
                                  256,                   // MinWidth
                                  256,                   // MaxWidth
                                  0,                     // WidthIncrement
                                  256,                   // MinHeight
                                  256,                   // MaxHeight
                                  0,                     // HeightIncrement
                                  false));               // Not Square
        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE,       // AccessMode
                                   LONG_MAX,             // MaxSizeInBytes
                                   32,                   // MinHeight
                                   LONG_MAX,             // MaxHeight
                                   16));                 // HeightIncrement
        }
    };


//-----------------------------------------------------------------------------
// HRFiTiffCodec1BitCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffCodec1BitCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffCodec1BitCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMR RLE1
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRRLE1::CLASS_ID,
                                   new HRFiTiffStripBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFiTiffStripBlockCapabilities()));
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFiTiffStripBlockCapabilities()));
        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFiTiffStripBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFiTiffStripBlockCapabilities()));

        // Codec is LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFiTiffStripBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffCodecPaletteCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffCodecPaletteCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffCodecPaletteCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        // Codec is LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffCodec16BitsPerChannelCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffCodec16BitsPerChannelCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffCodecTrueColorCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffCodecTrueColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffCodecTrueColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        // Codec Flashpix
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecFlashpix::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        // Codec is LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffCodecV32RGBAlphaCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffCodecV32RGBAlphaCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffCodecV32RGBAlphaCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Flashpix
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecFlashpix::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        // Codec is LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffCodecFloatCapabilities
//-----------------------------------------------------------------------------
class HRFiTiffCodecFloatCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFiTiffCodecFloatCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));

        // Codec LZW (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFiTiffTileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFiTiffCapabilities
//-----------------------------------------------------------------------------
HRFiTiffCapabilities::HRFiTiffCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI1R8G8B8;
    pPixelTypeI1R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI1R8G8B8::CLASS_ID,
                                                    new HRFiTiffCodec1BitCapabilities());
    pPixelTypeI1R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI1R8G8B8);

    // PixelTypeI1R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI1R8G8B8A8;
    pPixelTypeI1R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeI1R8G8B8A8::CLASS_ID,
                                                      new HRFiTiffCodec1BitCapabilities());
    pPixelTypeI1R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI1R8G8B8A8);

    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1Gray1;
    pPixelTypeV1Gray1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                                   new HRFiTiffCodec1BitCapabilities());
    pPixelTypeV1Gray1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1Gray1);

    // PixelTypeV1GrayWhite1
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1GrayWhite1;
    pPixelTypeV1GrayWhite1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV1GrayWhite1::CLASS_ID,
                                                        new HRFiTiffCodec1BitCapabilities());
    pPixelTypeV1GrayWhite1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1GrayWhite1);

    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8;
    pPixelTypeI8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI8R8G8B8::CLASS_ID,
                                                    new HRFiTiffCodecPaletteCapabilities());
    pPixelTypeI8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8);

    // PixelTypeI8R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8A8;
    pPixelTypeI8R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeI8R8G8B8A8::CLASS_ID,
                                                      new HRFiTiffCodecPaletteCapabilities());
    pPixelTypeI8R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8A8);

    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8Gray8;
    pPixelTypeV8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                                   new HRFiTiffCodecTrueColorCapabilities());
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8Gray8);

    // PixelTypeV8GrayWhite8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8GrayWhite8;
    pPixelTypeV8GrayWhite8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                        HRPPixelTypeV8GrayWhite8::CLASS_ID,
                                                        new HRFiTiffCodecTrueColorCapabilities());
    pPixelTypeV8GrayWhite8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8GrayWhite8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8GrayWhite8);

    // PixelTypeV16Int16
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFiTiffCodec16BitsPerChannelCapabilities()));

    // PixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV24R8G8B8;
    pPixelTypeV24R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                                     new HRFiTiffCodecTrueColorCapabilities());
    pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV24R8G8B8);


    // PixelTypeV32R8G8B8A8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32R8G8B8A8;
    pPixelTypeV32R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                       HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                                       new HRFiTiffCodecV32RGBAlphaCapabilities());
    pPixelTypeV32R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32R8G8B8A8);


    // PixelTypeV32Float32
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV32Float32;
    pPixelTypeV32Float32 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeV32Float32::CLASS_ID,
                                                      new HRFiTiffCodecFloatCapabilities());
    pPixelTypeV32Float32->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV32Float32->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV32Float32);



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

    // MultiResolution Capability
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_WRITE_CREATE, // AccessMode,
        true,                 // SinglePixelType,
        true,                  // SingleBlockType,
        false,                 // ArbitaryXRatio,
        false);                // ArbitaryYRatio);
    Add(pMultiResolutionCapability);

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

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
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMake));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeModel));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));

    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeCopyright));

    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMinSampleValue));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMaxSampleValue));


    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_WRITE_CREATE));

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
    }

//-----------------------------------------------------------------------------
// HRFiTiffCreatorBase
// This is the constructor of the base creator for both iTiff and
// iTiff64 formats.
//-----------------------------------------------------------------------------
HRFiTiffCreatorBase::HRFiTiffCreatorBase(HCLASS_ID pi_ClassID)
    : HRFRasterFileCreator(pi_ClassID)
    {
    }

//-----------------------------------------------------------------------------
// HRFiTiffCreatorBase
// This is the destructor of the base creator.
//-----------------------------------------------------------------------------
HRFiTiffCreatorBase::~HRFiTiffCreatorBase()
    {
    }


HFC_IMPLEMENT_SINGLETON(HRFiTiff64Creator)

//-----------------------------------------------------------------------------
// HRFiTiffCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFiTiff64Creator::HRFiTiff64Creator()
    : HRFiTiffCreatorBase(HRFiTiff64File::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFiTiff64Creator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_iTiff64());  // Internet TIFF 64 File Format
    }

// Identification information
WString HRFiTiff64Creator::GetExtensions() const
    {
    return WString(L"*.itiff64");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFiTiff64Creator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                HFCAccessMode         pi_AccessMode,
                                                uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFiTiff64File(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFiTiff64Creator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                      uint64_t             pi_Offset) const
    {
    return HRFiTiffCreatorBase::IsKindOfFile(pi_rpURL, pi_Offset, true);
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of HMR++ file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFiTiff64Creator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        {
        m_pCapabilities = new HRFiTiffCapabilities();

        //Add specific iTiff64 capabilities here
        m_pCapabilities->Add(new HRFMaxFileSizeCapability(HFC_READ_WRITE_CREATE, (uint64_t)UINT64_MAX));
        m_pCapabilities->Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttribute3DTransformationMatrix));
        }

    return m_pCapabilities;
    }

HFC_IMPLEMENT_SINGLETON(HRFiTiffCreator)

//-----------------------------------------------------------------------------
// HRFiTiffCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFiTiffCreator::HRFiTiffCreator()
    : HRFiTiffCreatorBase(HRFiTiffFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFiTiffCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_iTiff());  // Internet TIFF File Format
    }

// Identification information
WString HRFiTiffCreator::GetExtensions() const
    {
    return WString(L"*.itiff");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFiTiffCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                              HFCAccessMode         pi_AccessMode,
                                              uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFiTiffFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFiTiffCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                    uint64_t             pi_Offset) const
    {
    return HRFiTiffCreatorBase::IsKindOfFile(pi_rpURL, pi_Offset, false);
    }

//-----------------------------------------------------------------------------
// Return the scheme(s) supported by the file format.
//-----------------------------------------------------------------------------

WString HRFiTiffCreatorBase::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFiTiffCreatorBase::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset,
                                        bool                 pi_IsItiff64) const
    {
    HAutoPtr<HTIFFFile>  pTiff;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);

    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    if (pi_IsItiff64 == true)
        {
        ((HRFiTiff64Creator*)this)->SharingControlCreate(pi_rpURL);
        }
    else
        {
        ((HRFiTiffCreator*)this)->SharingControlCreate(pi_rpURL);
        }

    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if ((pTiff->IsValid(&pErr) || ((pErr != 0) && !pErr->IsFatal())) && (pTiff->IsTiff64() == pi_IsItiff64))
        {
        // set the result to true, it will be false if an error
        // occurs
        bResult = true;

        uint32_t PageCount = pTiff->NumberOfPages();
        if (PageCount == 0)
            {
            bResult = ValidatePageDirectory(pTiff, 0);
            }
        else
            {
            for (uint32_t Page = 0; Page < PageCount && bResult; Page++)
                bResult = ValidatePageDirectory(pTiff, Page);
            }


        for (uint32_t Page = 0; Page < PageCount && bResult; Page++)
            {
            if (pTiff->SetPage(Page))
                {
                // To detect if it is a iTiff file, verify if the private tag is present
                if (pTiff->TagIsPresent (HMR2_IMAGEINFORMATION))
                    {
                    Byte* pTransPalette = 0;
                    bResult = ValidateHMRDirectory(pTiff, Page, &pTransPalette);

                    if (bResult)
                        {
                        // Declare the current pixel type and codec
                        HFCPtr<HRPPixelType> CurrentPixelType;
                        HFCPtr<HCDCodec>     CurrentCodec;
                        try
                            {
                            if (bResult)
                                {
                                // Create the current pixel type and codec
                                CurrentPixelType = HRFTiffFile::CreatePixelTypeFromFile(pTiff,
                                                                                        Page,
                                                                                        pTransPalette);
                                CurrentCodec     = HRFTiffFile::CreateCodecFromFile(pTiff,
                                                                                    Page);
                                }
                            }

                        catch (...)
                            {
                            bResult = false;
                            }

                        if (bResult)
                            {
                            // Create the codec list to be attach to the PixelType Capability.
                            HFCPtr<HRFRasterFileCapabilities> pCurrentCodecCapability = new HRFRasterFileCapabilities();

                            // All iTiff 1 bit are striped. All other Pixel Type are tiled.
                            if(CurrentPixelType->CountPixelRawDataBits() == 1)
                                {
                                pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                                    CurrentCodec->GetClassID(),
                                                                                    new HRFiTiffStripBlockCapabilities()));
                                }
                            else
                                {
                                pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                                    CurrentCodec->GetClassID(),
                                                                                    new HRFiTiffTileBlockCapabilities()));
                                }

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
            }
        }
    else
        bResult = false;

    SisterFileLock.ReleaseKey();

    if (pi_IsItiff64 == true)
        {
        HASSERT(!((HRFiTiff64Creator*)this)->m_pSharingControl->IsLocked());
        ((HRFiTiff64Creator*)this)->m_pSharingControl = 0;
        }
    else
        {
        HASSERT(!((HRFiTiffCreator*)this)->m_pSharingControl->IsLocked());
        ((HRFiTiffCreator*)this)->m_pSharingControl = 0;
        }

    return bResult;
    }

bool HRFiTiffCreatorBase::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    if (HRFRasterFileCreator::SupportsURL(pi_rpURL))
        return true;
    else
        return (pi_rpURL->GetSchemeType() == HFCURLEmbedFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Private
// Validate if the field contain in the HMR directoty is valid.
//
//-----------------------------------------------------------------------------
bool HRFiTiffCreatorBase::ValidateHMRDirectory(HTIFFFile*  pi_pTiffFilePtr,
                                                uint32_t    pi_Page,
                                                Byte**    pTransPalette) const
    {
    HPRECONDITION (pi_pTiffFilePtr != 0);

    // Get HMR directirie information.
    uint32_t Version;
    unsigned short PixelSpec;
    bool    IsValid = true;

    HTIFFFile::DirectoryID CurDir = pi_pTiffFilePtr->CurrentDirectory();

    // Check if the private tag is present
    if (pi_pTiffFilePtr->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
        {
        // we support all minor version and all version before
        if (!pi_pTiffFilePtr->GetField(HMR_VERSION, &Version))
            {
            IsValid = false;
            }
        else if (Version > ITIFF_LATEST_VERSION)
            {   //A new version of iTiff cannot be opened with an old version of Imagepp.
            throw HRFUnsupportedITiffVersionException(pi_pTiffFilePtr->GetURL()->GetURL());
            }

        // Check Pixel Type Spec.
        if (pi_pTiffFilePtr->GetField(HMR_PIXEL_TYPE_SPEC, &PixelSpec) && PixelSpec == PIXELTYPESPEC_BGR)
            IsValid = false;

        // Check transparency palette .
        uint32_t Count;
        pi_pTiffFilePtr->GetField(HMR_TRANSPARENCY_PALETTE, &Count, pTransPalette);

        // Reset Directory
        pi_pTiffFilePtr->SetDirectory(CurDir);
        }

    return IsValid;
    }


//-----------------------------------------------------------------------------
// Private
// Validate the page directory
//-----------------------------------------------------------------------------
bool HRFiTiffCreatorBase::ValidatePageDirectory(HTIFFFile* pi_pTiffFilePtr, uint32_t pi_Page) const
    {
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
        else if ((ImageType & FILETYPE_PAGE) != 0)
            bResult = false;

        bResult = pi_pTiffFilePtr->SetPage(pi_Page);
        }

    if (bResult)
        {
        // To detect if it is a iTiff file, verify if the private tag is present
        if (pi_pTiffFilePtr->TagIsPresent (HMR2_IMAGEINFORMATION))
            {
            Byte* pTransPalette = 0;
            bResult = ValidateHMRDirectory(pi_pTiffFilePtr, pi_Page, &pTransPalette);

            if (bResult)
                {
                // Declare the current pixel type and codec
                HFCPtr<HRPPixelType> CurrentPixelType;
                HFCPtr<HCDCodec>     CurrentCodec;
                try
                    {
                    if (bResult)
                        {
                        // Create the current pixel type and codec
                        CurrentPixelType = HRFTiffFile::CreatePixelTypeFromFile(pi_pTiffFilePtr,
                                                                                pi_Page,
                                                                                pTransPalette);
                        CurrentCodec     = HRFTiffFile::CreateCodecFromFile(pi_pTiffFilePtr,
                                                                            pi_Page);
                        }
                    }

                catch (...)
                    {
                    bResult = false;
                    }

                if (bResult)
                    {
                    // Create the codec list to be attach to the PixelType Capability.
                    HFCPtr<HRFRasterFileCapabilities> pCurrentCodecCapability = new HRFRasterFileCapabilities();

                    // All iTiff 1 bit are striped. All other Pixel Type are tiled.
                    if (CurrentPixelType->CountPixelRawDataBits() == 1)
                        {
                        pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                            CurrentCodec->GetClassID(),
                                                                            new HRFiTiffStripBlockCapabilities()));
                        }
                    else
                        {
                        pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                            CurrentCodec->GetClassID(),
                                                                            new HRFiTiffTileBlockCapabilities()));
                        }

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
// Create or get the singleton capabilities of HMR++ file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFiTiffCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFiTiffCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFiTiffFile::HRFiTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                           HFCAccessMode         pi_AccessMode,
                           uint64_t             pi_Offset)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, true)
    {
//    InitPrivateTagDefault();
//    m_HMRDirDirty = false;

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
HRFiTiffFile::HRFiTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                  HFCAccessMode         pi_AccessMode,
                                  uint64_t             pi_Offset,
                                  bool                 pi_DontOpenFile)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFiTiff64File::HRFiTiff64File(const HFCPtr<HFCURL>& pi_rURL,
                                      HFCAccessMode         pi_AccessMode,
                                      uint64_t             pi_Offset)
    : HRFiTiffFile(pi_rURL, pi_AccessMode, pi_Offset, true)
    {
    //    InitPrivateTagDefault();
    //    m_HMRDirDirty = false;

    // if Open success and it is not a new file
    if (Open() && !GetAccessMode().m_HasCreateAccess)
        {
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRFiTiffFile::~HRFiTiffFile()
    {
    try
        {
        SaveiTiffFile();
        // The tiff ancestor close the file
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// Public
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFiTiffFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                          uint32_t pi_Page,
                                          bool   pi_CheckSpecificUnitSpec,
                                          bool   pi_InterpretUnitINTGR)
    {
    //The iTiff file format and the derivated cTiff and iTiff64 formats
    //always use meter as the linear unit.
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFiTiffFile::Save()
    {
    SaveiTiffFile();
    HRFTiffFile::Save();
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFiTiffFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                          unsigned short pi_Resolution,
                                                          HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE)
        {
        pEditor = new HRFiTiffTileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        }
    else
        {
        pEditor = new HRFiTiffStripEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        }

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFiTiffFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    // Validate the page and all resolutions with the HMR capabilities
    // Create other resolution if necessary
    HPRECONDITION(pi_pPage->CountResolutions() > 0);

    // Change the flags to empty for each each resolution
    for (unsigned short Resolution=0; Resolution < pi_pPage->CountResolutions(); Resolution++)
        {
        // Obtain the resolution descriptor
        HFCPtr<HRFResolutionDescriptor> pResolution = pi_pPage->GetResolutionDescriptor(Resolution);
        // We set in creation for the new page all flags to empty
        if (!pResolution->HasBlocksDataFlag())
            {
            HASSERT(pResolution->CountBlocks() <= SIZE_MAX);
            HASSERT(pResolution->CountBlocks() * sizeof(HRFDataFlag) <= SIZE_MAX);
            HArrayAutoPtr<HRFDataFlag> pBlocksDataFlag;
            pBlocksDataFlag = new HRFDataFlag[(size_t)pResolution->CountBlocks()];

            memset(pBlocksDataFlag, HRFDATAFLAG_EMPTY | HRFDATAFLAG_DIRTYFORSUBRES , (size_t)(pResolution->CountBlocks() * sizeof(HRFDataFlag)));
            pResolution->SetBlocksDataFlag(pBlocksDataFlag);
            }

        // set JPEG codec embeded
        // If we have a HFCCodecIJG we need to set its embeded.
        if (pResolution->GetCodec()->GetClassID() == HCDCodecIJG::CLASS_ID)
            ((HFCPtr<HCDCodecIJG>&)pResolution->GetCodec())->SetAbbreviateMode(true);
        }

    // allways we have a transfo model with HMR
    if (!pi_pPage->HasTransfoModel())
        {
        HFCPtr<HGF2DTransfoModel> NullModel;
        NullModel = new HGF2DIdentity();
        pi_pPage->SetTransfoModel(*NullModel);
        }

    // realloc the array of HMR Header
    uint32_t PageCount = CountPages();
    HArrayAutoPtr<HAutoPtr<HMRHeader> > ppHMRHeaders(new HAutoPtr<HMRHeader>[PageCount + 1]);
    for (uint32_t i = 0; i < PageCount; i++)
        ppHMRHeaders[i] = m_ppHMRHeaders[i].release();

    m_ppHMRHeaders = ppHMRHeaders.release();
    HAutoPtr<HMRHeader> pHMRHeader(new HMRHeader);
    InitPrivateTagDefault(pHMRHeader);
    pHMRHeader->m_HMRDirDirty = true;

    m_ppHMRHeaders[PageCount] = pHMRHeader.release();

    // Add the page descriptor to the list
    return HRFTiffFile::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFiTiffFile::GetCapabilities () const
    {
    return HRFiTiffCreator::GetInstance()->GetCapabilities();
    }


#if 0
disable, GT 27/09/2004
//-----------------------------------------------------------------------------
// AltaPhoto project
//
//-----------------------------------------------------------------------------
// Public
// Read_AltaPhotoBlob
//-----------------------------------------------------------------------------
bool HRFiTiffFile::Read_AltaPhotoBlob (vector<Byte>* po_pData) const
    {
    uint32_t BlobSize = 0;
    bool  Ret      = true;

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock(const_cast<HRFiTiffFile*>(this)->GetLockManager());

    GetFilePtr()->ReadAltaPhotoBlob(0, &BlobSize);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    if (BlobSize != 0)
        {
        po_pData->resize(BlobSize);

        // Lock the sister file for the GetField operation
        HFCLockMonitor SisterFileLock(const_cast<HRFiTiffFile*>(this)->GetLockManager());

        Ret = GetFilePtr()->ReadAltaPhotoBlob(&((*po_pData)[0]), 0);

        SisterFileLock.ReleaseKey();
        }
    else
        Ret = false;

    return Ret;
    }

//-----------------------------------------------------------------------------
// Public
// Write_AltaPhotoBlob
//-----------------------------------------------------------------------------
bool HRFiTiffFile::Write_AltaPhotoBlob   (const vector<Byte>& pi_pData)
    {
    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock (GetLockManager());

    return GetFilePtr()->WriteAltaPhotoBlob(&(pi_pData[0]), pi_pData.size());

    // The sister file is automatically unlock at the destruction of
    // SisterFileLock
    }

// AltaPhoto project End
//-----------------------------------------------------------------------------
#endif


//-----------------------------------------------------------------------------
// Protected
// Open
// File manipulation
//-----------------------------------------------------------------------------
bool HRFiTiffFile::Open(bool pi_CreateBigTifFormat)
    {
    return HRFTiffFile::Open(pi_CreateBigTifFormat);
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFiTiffFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);
    uint32_t PageIndex;
    HAutoPtr<HMRHeader> pHMRHeader;

    uint32_t PageCount = CalcNumberOfPage();

    m_ppHMRHeaders = new HAutoPtr<HMRHeader>[PageCount];

    // Create the descriptor for each resolution of each page
    for (uint32_t Page = 0; Page < PageCount; Page++)
        {
        // first, read the private directory
        pHMRHeader = new HMRHeader;
        // Init. default value for the Private Tags
        InitPrivateTagDefault (pHMRHeader);

        if (!ReadPrivateDirectory(Page, pHMRHeader))
            throw HFCFileNotSupportedException(GetURL()->GetURL());

        // Select the page
        PageIndex = GetIndexOfPage(Page);
        SetImageInSubImage (PageIndex);

        HRFDownSamplingMethod       DownSamplingMethod;

        //HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();

        if (pHMRHeader->m_HasDecimationMethod && pHMRHeader->m_DecimationMethodCount == 1)
            DownSamplingMethod = GetDownSamplingMethod(pHMRHeader->m_pDecimationMethod[0]);

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

        // MainWidth help to calc the resolution ratio
        GetFilePtr()->GetField(IMAGEWIDTH, &MainWidth);

        // Determindec the storage type
        GetFilePtr()->GetField(IMAGEWIDTH, &Width);
        GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);

        HRFBlockType StoragePage = ((GetFilePtr()->IsTiled()) ? HRFBlockType::TILE : HRFBlockType::STRIP);
        HFCPtr<HRPPixelType> PixelType;
        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < CalcNumberOfSubResolution(GetIndexOfPage(Page))+1; Resolution++)
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
            // In version 3.0, we add the multi pixeltype capabilities,
            // before version 3.0, all resolution has the main image pixeltype
            // Select the page and resolution

            if (pHMRHeader->m_Version >= ITIFF_VERSION_1BIT_GRAYSCALE || Resolution == 0)
                {
                PixelType = CreatePixelTypeFromFile(GetFilePtr(), Page,
                                                    pHMRHeader->m_pTransPalette,
                                                    PIXELTYPESPEC_RGB,
                                                    &pHMRHeader->m_ChannelsWithNoDataValue,
                                                    &pHMRHeader->m_ChannelsNoDataValue);
                }

            // Compression 
            HFCPtr<HCDCodec> pCodec = CreateCodecFromFile(GetFilePtr(), Page);

            // convert PHOTOMETRIC value
            // if the compression is FLASHPIX or JPEG, the PHOTOMETRIC must be set to YCbCr
            unsigned short Compression;
            GetFilePtr()->GetField(COMPRESSION, &Compression);
            if (GetAccessMode().m_HasWriteAccess)
                {
                if (Compression == COMPRESSION_HMR_FLASHPIX && PixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
                    throw HRFAccessModeForCodeNotSupportedException(GetURL()->GetURL());

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
            if(!pHMRHeader->m_HasDecimationMethod)
                {
                // Set the default down sampling methode depending of the Pixel Type.
                if ((PixelType->CountPixelRawDataBits() >= 24) ||
                    ((PixelType->CountIndexBits() == 0) && (PixelType->CountPixelRawDataBits() == 8)))
                    DownSamplingMethod = HRFDownSamplingMethod::AVERAGE;
                else
                    DownSamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
                }
            else if (pHMRHeader->m_DecimationMethodCount != 1)
                DownSamplingMethod = GetDownSamplingMethod(pHMRHeader->m_pDecimationMethod[Resolution]);
            // Else, m_DecimationMethodCount == 1, we already put the rigth DownSamplingMethod in
            // at the beginning of this function.

            double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(MainWidth, Width);
            HFCPtr<HRFResolutionDescriptor> pResolution =
                new HRFResolutionDescriptor(GetAccessMode(),                               // AccessMode,
                                            GetCapabilities(),                             // Capabilities,
                                            Ratio,                                         // XResolutionRatio,
                                            Ratio,                                         // YResolutionRatio,
                                            PixelType,                                     // PixelType,
                                            pCodec,                                        // Codec,
                                            HRFBlockAccess::RANDOM,                        // RStorageAccess,
                                            HRFBlockAccess::RANDOM,                        // WStorageAccess,
                                            HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL, // ScanLineOrientation,
                                            HRFInterleaveType::PIXEL,                      // InterleaveType
                                            false,                                         // IsInterlace,
                                            Width,                                         // Width,
                                            Height,                                        // Height,
                                            BlockWidth,                                    // BlockWidth,
                                            BlockHeight,                                   // BlockHeight,
                                            (pHMRHeader->m_iTiffTileFlagsLength == 0 ?     // BlocksDataFlag
                                             0 :
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

        // TranfoModel
        // Select the page and resolution and read the image height, all
        // other Resolution are computed from this value.
        SetImageInSubImage (PageIndex);
        GetFilePtr()->GetField(IMAGELENGTH, &Height);

        HFCPtr<HGF2DTransfoModel> pTransfoModel = CreateTransfoModelFromTiffMatrix();

        HFCPtr<HRPHistogram> pHistogram;
        if (pHMRHeader->m_HistogramLength == 768)
            {
            uint32_t** pEntryFrequencies;
            pEntryFrequencies = new uint32_t*[3];
            for (int ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
                {
                pEntryFrequencies   [ChannelIndex]  = new uint32_t[256];
                memcpy(pEntryFrequencies[ChannelIndex],
                       pHMRHeader->m_pHistogram + (ChannelIndex * 256),
                       256 * sizeof(uint32_t));
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
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;

        // Get common TIFF family tags
        HRFTiffFile::GetBaselineTags(&TagList, *PixelType);

        // 3D_TRANSFO_MATRIX Tag
        if (pHMRHeader->m_p3DTranfoMatrix != 0)
            {
            HASSERT(GetFilePtr()->IsTiff64() == true);
            pTag = new HRFAttribute3DTransformationMatrix(*(pHMRHeader->m_p3DTranfoMatrix.get()));
            TagList.Set(pTag);
            }

        //Geocoding Information
        IRasterBaseGcsPtr pBaseGCS;

        if (!pHMRHeader->m_WellKnownText.empty())
            {
            HASSERT(GetFilePtr()->IsTiff64() == true);

            WString WktGeocode;
            BeStringUtilities::CurrentLocaleCharToWChar(WktGeocode, pHMRHeader->m_WellKnownText.c_str());

            pBaseGCS = HRFGeoCoordinateProvider::CreateRasterGcsFromWKT(NULL, NULL, IRasterGeoCoordinateServices::WktFlavorUnknown, WktGeocode.c_str());

//            pGeoTiffKeys = ExtractGeocodingInformationFromWKT (pHMRHeader->m_WellKnownText);
            }

        // set keys since WKT was not used

        HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys = 0;

        if (pBaseGCS == 0)
            {
            char*  pString;
            unsigned short GeoShortValue;
            unsigned short GTModelTypeValue;
            double GeoDoubleValue;

            pGeoTiffKeys = new HCPGeoTiffKeys();

            // GTModelType
            if (!GetFilePtr()->GetGeoKeyInterpretation().GetValue(GTModelType, &GTModelTypeValue))
                {
                GTModelTypeValue = TIFFGeo_ModelTypeProjected;
                }

            pGeoTiffKeys->AddKey(GTModelType, (uint32_t)GTModelTypeValue);

            // GTRasterType
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GTRasterType, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GTRasterType, (uint32_t)GeoShortValue);
                }
            else
                {
                pGeoTiffKeys->AddKey(GTRasterType, (uint32_t)TIFFGeo_RasterPixelIsArea);
                }

            // PCSCitation
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(PCSCitation, &pString))
                {
                pGeoTiffKeys->AddKey(PCSCitation, pString);
                }

            // ProjectedCSType
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(ProjectedCSType, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)GeoShortValue);
                }
            else
                {
                //Add ProjectedCSType only if the model type is projected.
                if (GTModelTypeValue == TIFFGeo_ModelTypeProjected)
                    {
                    pGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)PrivateBaseGeoKey);
                    }
                }

            // GTCitation
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GTCitation, &pString))
                {
                pGeoTiffKeys->AddKey(GTCitation, pString);
                }

            // Projection
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(Projection, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(Projection, (uint32_t)GeoShortValue);
                }

            // ProjCoordTrans
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(ProjCoordTrans, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(ProjCoordTrans, (uint32_t)GeoShortValue);
                }

            // ProjLinearUnits
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(ProjLinearUnits, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(ProjLinearUnits,(uint32_t)GeoShortValue);
                }

            // ProjLinearUnitSize
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjLinearUnitSize, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjLinearUnitSize, GeoDoubleValue);
                }

            // GeographicType
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeographicType, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeographicType, (uint32_t)GeoShortValue);
                }

            // GeogCitation
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogCitation, &pString))
                {
                pGeoTiffKeys->AddKey(GeogCitation, pString);
                }

            // GeogGeodeticDatum
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogGeodeticDatum, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeogGeodeticDatum, (uint32_t)GeoShortValue);
                }

            // GeogPrimeMeridian
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogPrimeMeridian, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeogPrimeMeridian, (uint32_t)GeoShortValue);
                }

            // GeogLinearUnits
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogLinearUnits, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeogLinearUnits, (uint32_t)GeoShortValue);
                }

            // GeogLinearUnitSize
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogLinearUnitSize, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(GeogLinearUnitSize, GeoDoubleValue);
                }

            // GeogAngularUnits
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogAngularUnits, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeogAngularUnits, (uint32_t)GeoShortValue);
                }

            // GeogAngularUnitSize
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogAngularUnitSize, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(GeogAngularUnitSize, GeoDoubleValue);
                }

            // GeogEllipsoid
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogEllipsoid, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeogEllipsoid, (uint32_t)GeoShortValue);
                }

            // GeogSemiMajorAxis
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogSemiMajorAxis, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(GeogSemiMajorAxis, GeoDoubleValue);
                }

            // GeogSemiMinorAxis
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogSemiMinorAxis, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(GeogSemiMinorAxis, GeoDoubleValue);
                }

            // GeogInvFlattening
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogInvFlattening, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(GeogInvFlattening, GeoDoubleValue);
                }

            // GeogAzimuthUnits
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogAzimuthUnits, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(GeogAzimuthUnits, (uint32_t)GeoShortValue);
                }

            // GeogPrimeMeridianLong
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogPrimeMeridianLong, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(GeogPrimeMeridianLong, GeoDoubleValue);
                }

            // ProjStdParallel1
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjStdParallel1, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjStdParallel1, GeoDoubleValue);
                }

            // ProjStdParallel2
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjStdParallel2, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjStdParallel2, GeoDoubleValue);
                }

            // ProjNatOriginLong
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjNatOriginLong, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjNatOriginLong, GeoDoubleValue);
                }

            // ProjNatOriginLat
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjNatOriginLat, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjNatOriginLat, GeoDoubleValue);
                }

            // ProjFalseEasting
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseEasting, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjFalseEasting, GeoDoubleValue);
                }

            // ProjFalseNorthing
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseNorthing, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjFalseNorthing, GeoDoubleValue);
                }

            // ProjFalseOriginLong
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginLong, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjFalseOriginLong, GeoDoubleValue);
                }

            // ProjFalseOriginLat
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginLat, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjFalseOriginLat, GeoDoubleValue);
                }

            // ProjFalseOriginEasting
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginEasting, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjFalseOriginEasting, GeoDoubleValue);
                }

            // ProjFalseOriginNorthing
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginNorthing, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjFalseOriginNorthing, GeoDoubleValue);
                }

            // ProjCenterLong
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterLong, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjCenterLong, GeoDoubleValue);
                }

            // ProjCenterLat
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterLat, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjCenterLat, GeoDoubleValue);
                }

            // ProjCenterEasting
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterEasting, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjCenterEasting, GeoDoubleValue);
                }

            // ProjCenterNorthing
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterNorthing, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjCenterEasting, GeoDoubleValue);
                }

            // ProjScaleAtNatOrigin
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjScaleAtNatOrigin, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjScaleAtNatOrigin, GeoDoubleValue);
                }

            // ProjScaleAtCenter
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjScaleAtCenter, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjScaleAtCenter, GeoDoubleValue);
                }

            // ProjAzimuthAngle
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjAzimuthAngle, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjAzimuthAngle, GeoDoubleValue);
                }

            // ProjStraightVertPoleLong
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjStraightVertPoleLong, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjStraightVertPoleLong, GeoDoubleValue);
                }

            // ProjRectifiedGridAngle
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjRectifiedGridAngle, &GeoDoubleValue))
                {
                pGeoTiffKeys->AddKey(ProjRectifiedGridAngle, GeoDoubleValue);
                }

            // VerticalCSType
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(VerticalCSType, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(VerticalCSType, (uint32_t)GeoShortValue);
                }

            // VerticalCitation
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(VerticalCitation, &pString))
                {
                pGeoTiffKeys->AddKey(VerticalCitation, pString);
                }

            // VerticalDatum
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(VerticalDatum, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(VerticalDatum, (uint32_t)GeoShortValue);
                }

            // VerticalUnits
            if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(VerticalUnits, &GeoShortValue))
                {
                pGeoTiffKeys->AddKey(VerticalUnits, (uint32_t)GeoShortValue);
                }

            }

        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (GetAccessMode(),
                                       GetCapabilities(),           // Capabilities,
                                       ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                       0,                           // RepresentativePalette,
                                       pHistogram,                  // Histogram,
                                       0,                           // Thumbnail,
                                       pClipShape,                  // ClipShape,
                                       pTransfoModel,               // TransfoModel,
                                       0,                           // Filters
                                       &TagList,                    // Tag
                                       0);                          // Duration

        if (pBaseGCS==0)
            {
            pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys));
            }
        else
            {
            pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pBaseGCS.get()));
            }


        m_ListOfPageDescriptor.push_back(pPage);

        // the page is created, add the HMR Header into the list
        m_ppHMRHeaders[Page] = pHMRHeader.release();
        }
    }

//-----------------------------------------------------------------------------
// Public
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFiTiffFile::GetFileCurrentSize() const
    {
    return HRFTiffFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// protected
// Saves the file
//-----------------------------------------------------------------------------
void HRFiTiffFile::SaveiTiffFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        // Write Information
        if (GetAccessMode().m_HasWriteAccess  || GetAccessMode().m_HasCreateAccess)
            {
            // Update the modification to the file
            HFCPtr<HRFPageDescriptor> pPageDescriptor;
            HMRHeader* pHMRHeader;
            for (uint32_t Page = 0; Page < CountPages(); Page++)
                {
                // Select the page
                SetImageInSubImage (GetIndexOfPage(Page));

                pPageDescriptor = GetPageDescriptor(Page);
                if (!pPageDescriptor->IsEmpty())
                    {
                    pHMRHeader = m_ppHMRHeaders[Page];

                    // Save for the first page only
                    if (pPageDescriptor->HasClipShape())
                        if (pPageDescriptor->ClipShapeHasChanged() || (GetAccessMode().m_HasCreateAccess))
                            SetClipShape(pHMRHeader, *pPageDescriptor->GetClipShape());

                    if (pPageDescriptor->HasHistogram())
                        if (pPageDescriptor->HistogramHasChanged() || (GetAccessMode().m_HasCreateAccess))
                            SetHistogram(pHMRHeader, *pPageDescriptor->GetHistogram());

                    // Update the TransfoModel
                    if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                        WriteTransfoModel(pPageDescriptor->GetTransfoModel());

                    HRFAttribute3DTransformationMatrix const* p3DMatrixAttr = pPageDescriptor->FindTagCP<HRFAttribute3DTransformationMatrix>();
                    if (p3DMatrixAttr != 0) 
                        {
                        // Update the 3D transformation matrix, if any.                          
                        if (pHMRHeader->m_p3DTranfoMatrix == 0)
                            {
                            pHMRHeader->m_HMRDirDirty = true;
                            pHMRHeader->m_p3DTranfoMatrix = new HFCMatrix<4, 4>(p3DMatrixAttr->GetData());
                            }
                        else if (*(pHMRHeader->m_p3DTranfoMatrix) != p3DMatrixAttr->GetData())
                            {
                            pHMRHeader->m_HMRDirDirty = true;
                            *(pHMRHeader->m_p3DTranfoMatrix) = p3DMatrixAttr->GetData();
                            }
                        }
                    else if (pHMRHeader->m_p3DTranfoMatrix != 0)
                        {
                        pHMRHeader->m_HMRDirDirty = true;
                        pHMRHeader->m_p3DTranfoMatrix = 0;
                        }
                    
                    HRFAttributeOnDemandRastersInfo const* pODRastersInfoAttr = pPageDescriptor->FindTagCP<HRFAttributeOnDemandRastersInfo>();
                    if (pODRastersInfoAttr != 0) 
                        {
                        // Update the on demand rasters' info, if any.                     
                        if ((pHMRHeader->m_OnDemandRastersInfo.empty() == true) ||
                            (pHMRHeader->m_OnDemandRastersInfo != pODRastersInfoAttr->GetData()))
                            {
                            pHMRHeader->m_HMRDirDirty = true;
                            pHMRHeader->m_OnDemandRastersInfo = pODRastersInfoAttr->GetData();
                            }
                        }

                    // Update header with channels no data values
                    HASSERT(IsSamePixelTypeForAllResolutions(pPageDescriptor));
                    const HRPChannelOrg rChannelOrg = pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType()->GetChannelOrg();
                    GenerateNoDataValuesTagData(&pHMRHeader->m_ChannelsWithNoDataValue,
                                                &pHMRHeader->m_ChannelsNoDataValue,
                                                rChannelOrg);


                    if ((GetFilePtr()->IsTiff64()) && (pPageDescriptor->GeocodingHasChanged() == true))
                        {
                        RasterFileGeocoding const& fileGeocoding = pPageDescriptor->GetRasterFileGeocoding();

                        pHMRHeader->m_HMRDirDirty = true;

                        if (fileGeocoding.GetGeocodingCP() == NULL || !(fileGeocoding.GetGeocodingCP()->IsValid()))
                            {
                            pHMRHeader->m_WellKnownText = ""; //set to empty
                            }
                        else
                            {
                            // The first reaction is to store the geo key list as it is defined originally.
                            if (fileGeocoding.GetGeocodingCP() != NULL)
                                {
                                WString WellKnownText;
                                fileGeocoding.GetGeocodingCP()->GetWellKnownText(WellKnownText, IRasterGeoCoordinateServices::WktFlavorOGC);

                                if (WellKnownText == L"")
                                    fileGeocoding.GetGeocodingCP()->GetWellKnownText(WellKnownText, IRasterGeoCoordinateServices::WktFlavorESRI);

                                size_t  destinationBuffSize = WellKnownText.GetMaxLocaleCharBytes();
                                char*  WellKnownTextMBS= (char*)_alloca (destinationBuffSize);
                                BeStringUtilities::WCharToCurrentLocaleChar(WellKnownTextMBS,WellKnownText.c_str(),destinationBuffSize);

                                pHMRHeader->m_WellKnownText = string(WellKnownTextMBS);
                                }
                            else
                                {   // we want to read the Keys and not the WKT
                                pHMRHeader->m_WellKnownText = ""; //set to empty
                                }
                            }
                        }

                    // SamplingMethod (will be write in the private directory)
                    // Used only in creation mode.
                    // We set the buffer but we don't set the HMRDirectory to Dirty.
                    // Because we can not know if the Decimation Method has change.
                    // In fact, it can not change because it is set on the res descriptor and
                    // the res descriptor can not be change !!!
                    // Some old itiff files seem to have "m_DecimationMethodCount == 1", we will rewrite the file
                    // in this case.
                    if (!pHMRHeader->m_HasDecimationMethod || pHMRHeader->m_DecimationMethodCount == 1)
                        {
                        pHMRHeader->m_DecimationMethodCount = pPageDescriptor->CountResolutions();
                        pHMRHeader->m_pDecimationMethod     = new Byte[pHMRHeader->m_DecimationMethodCount];
                        }
                    HFCPtr<HRFResolutionDescriptor> pResDesc;
                    for (uint32_t Res=0; Res < pPageDescriptor->CountResolutions(); Res++)
                        {
                        pResDesc = pPageDescriptor->GetResolutionDescriptor((unsigned short)Res);
                        pHMRHeader->m_pDecimationMethod[Res] = GetDownSamplingMethodCode(pResDesc->GetDownSamplingMethod());

                        pHMRHeader->m_HMRDirDirty = pHMRHeader->m_HMRDirDirty ||
//                            GetAccessMode().m_HasCreateAccess ||
                                                    pResDesc->DownSamplingMethodHasChanged();
                        }
                    WritePrivateDirectory (Page);
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Write3DMatrixToTiffTag
//-----------------------------------------------------------------------------
bool HRFiTiffFile::Write3DMatrixToTiffTag(HFCMatrix<4, 4>& pi_r3DMatrix)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess  || GetAccessMode().m_HasCreateAccess);

    bool Ret = true;

    double  aMat[16];
    double* pMat4by4;
    uint32_t Count;

    // Fill the affine matrix.
    aMat[0]  = pi_r3DMatrix[0][0];
    aMat[1]  = pi_r3DMatrix[0][1];
    aMat[2]  = pi_r3DMatrix[0][2];
    aMat[3]  = pi_r3DMatrix[0][3];
    aMat[4]  = pi_r3DMatrix[1][0];
    aMat[5]  = pi_r3DMatrix[1][1];
    aMat[6]  = pi_r3DMatrix[1][2];
    aMat[7]  = pi_r3DMatrix[1][3];
    aMat[8]  = pi_r3DMatrix[2][0];
    aMat[9]  = pi_r3DMatrix[2][1];
    aMat[10] = pi_r3DMatrix[2][2];
    aMat[11] = pi_r3DMatrix[2][3];
    aMat[12] = pi_r3DMatrix[3][0];
    aMat[13] = pi_r3DMatrix[3][1];
    aMat[14] = pi_r3DMatrix[3][2];
    aMat[15] = pi_r3DMatrix[3][3];

    // Don't update the file if we don't change de Matrix.
    //
    if (!GetFilePtr()->GetField(HMR2_3D_TRANSFO_MATRIX, &Count, &pMat4by4) ||
        memcmp((double*)aMat, pMat4by4, sizeof(double)*16))
        {
        // Set the Matrix in the file
        Ret = GetFilePtr()->SetField(HMR2_3D_TRANSFO_MATRIX, 16, (double*)aMat);
        }

    return Ret;
    }


//-----------------------------------------------------------------------------
// Protected
// Create3DMatrixFromiTiffTag
//-----------------------------------------------------------------------------
RasterFileGeocodingPtr  HRFiTiffFile::ExtractGeocodingInformationFromWKT(string& pi_wkt) const
    {
    RasterFileGeocodingPtr pGeocoding(RasterFileGeocoding::Create());

    if (!pi_wkt.empty() && GCSServices->_IsAvailable())
        {

            WString WellKnownText;
            BeStringUtilities::CurrentLocaleCharToWChar( WellKnownText,pi_wkt.c_str());
                                                                                                             
            IRasterBaseGcsPtr pBaseGCS = HRFGeoCoordinateProvider::CreateRasterGcsFromWKT(NULL, NULL, IRasterGeoCoordinateServices::WktFlavorUnknown, WellKnownText.c_str());
            pGeocoding = RasterFileGeocoding::Create(pBaseGCS.get());
            
        }

    return pGeocoding;
    }


//-----------------------------------------------------------------------------
// Protected
// Create3DMatrixFromiTiffTag
//-----------------------------------------------------------------------------
HFCMatrix<4, 4>* HRFiTiffFile::Create3DMatrixFromiTiffTag() const
    {
    HFCMatrix<4, 4>*    p3DMatrix = 0;
    double*            pMat4by4;
    uint32_t            Count;

    // Read the matrix from file
    if (GetFilePtr()->GetField(HMR2_3D_TRANSFO_MATRIX, &Count, &pMat4by4))
        {
        p3DMatrix = new HFCMatrix<4, 4>;

        (*p3DMatrix)[0][0] = pMat4by4[0];
        (*p3DMatrix)[0][1] = pMat4by4[1];
        (*p3DMatrix)[0][2] = pMat4by4[2];
        (*p3DMatrix)[0][3] = pMat4by4[3];
        (*p3DMatrix)[1][0] = pMat4by4[4];
        (*p3DMatrix)[1][1] = pMat4by4[5];
        (*p3DMatrix)[1][2] = pMat4by4[6];
        (*p3DMatrix)[1][3] = pMat4by4[7];
        (*p3DMatrix)[2][0] = pMat4by4[8];
        (*p3DMatrix)[2][1] = pMat4by4[9];
        (*p3DMatrix)[2][2] = pMat4by4[10];
        (*p3DMatrix)[2][3] = pMat4by4[11];
        (*p3DMatrix)[3][0] = pMat4by4[12];
        (*p3DMatrix)[3][1] = pMat4by4[13];
        (*p3DMatrix)[3][2] = pMat4by4[14];
        (*p3DMatrix)[3][3] = pMat4by4[15];
        }

    return p3DMatrix;
    }



//-----------------------------------------------------------------------------
// Private
// InitPrivateTagDefault
//-----------------------------------------------------------------------------
void HRFiTiffFile::InitPrivateTagDefault (HMRHeader* po_pHMRHeader)
    {
    HPRECONDITION(po_pHMRHeader != 0);

    // Init. default value for the Private Tags
    //
    HFCPtr<HRFMultiResolutionCapability> pMultiResCapability;
    pMultiResCapability = static_cast<HRFMultiResolutionCapability* >(GetCapabilities()->
                           GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());
    // if we are not in create mode, the version will be read into the file
    if (GetFilePtr()->IsTiff64() == true)
        po_pHMRHeader->m_Version = ITIFF_VERSION_64BITS;
    else if (pMultiResCapability->IsSinglePixelType())
        po_pHMRHeader->m_Version = HMR_VERSION_TILE_SLO4;
    else
        po_pHMRHeader->m_Version = ITIFF_VERSION_1BIT_GRAYSCALE;

    po_pHMRHeader->m_MinorVersion  = 0;
    memset (po_pHMRHeader->m_SystemCoord, ' ', HMR_LgStringSystemCoord);
    po_pHMRHeader->m_SystemCoord[HMR_LgStringSystemCoord-1] = '\0';

    memset (po_pHMRHeader->m_HistoDateTime, ' ', HMR_LgStringDateTime);
    po_pHMRHeader->m_HistoDateTime[HMR_LgStringDateTime-1] = '\0';
    po_pHMRHeader->m_HistogramLength = 256;
    po_pHMRHeader->m_pHistogram = new uint32_t[po_pHMRHeader->m_HistogramLength];
    for (size_t i = 0; i < po_pHMRHeader->m_HistogramLength; i++)
        po_pHMRHeader->m_pHistogram[i] = 1L;

    po_pHMRHeader->m_HMRClipShapeLength     = 0L;
    po_pHMRHeader->m_pHMRClipShape          = 0;
    po_pHMRHeader->m_HMRClipShapeInFile     = false;

    po_pHMRHeader->m_iTiffTileFlagsLength   = 0L;
    po_pHMRHeader->m_piTiffTileFlags        = 0;

    po_pHMRHeader->m_HasDecimationMethod    = false;
    po_pHMRHeader->m_pDecimationMethod      = 0;

    po_pHMRHeader->m_p3DTranfoMatrix        = 0;

    po_pHMRHeader->m_WellKnownText          = "";

    po_pHMRHeader->m_OnDemandRastersInfo    = "";

    po_pHMRHeader->m_ChannelsWithNoDataValue.clear();
    po_pHMRHeader->m_ChannelsNoDataValue.clear();

    po_pHMRHeader->m_HMRDirDirty            = false;
    }

//-----------------------------------------------------------------------------
// private
// ReadPrivateDirectories - Read the HMR private Directory on the TIFF file.
//
//-----------------------------------------------------------------------------
bool HRFiTiffFile::ReadPrivateDirectory(uint32_t pi_Page, HMRHeader* po_pHMRHeader)
    {
    HPRECONDITION(po_pHMRHeader != 0);
    HPRECONDITION(GetFilePtr() != 0);

    bool    Ret = true;

    double* pData;
    char*   pTileFlags;

    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();

    // Check if the private tag is present
    //
    if (GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
        {
        char*  pSystem;
        uint32_t* pHisto;

        po_pHMRHeader->m_HMRDirDirty   = false;

        // Read System Coord.
        if (GetFilePtr()->GetField(HMR_IMAGECOORDINATESYSTEM,  &pSystem))
            {
            if (strlen (pSystem) >= HMR_LgStringSystemCoord)
                memcpy (po_pHMRHeader->m_SystemCoord, pSystem, HMR_LgStringSystemCoord);
            else
                memcpy (po_pHMRHeader->m_SystemCoord, pSystem, strlen(pSystem));
            po_pHMRHeader->m_SystemCoord[HMR_LgStringSystemCoord-1] = '\0';
            }

        // DateTime Histogram
        memset (po_pHMRHeader->m_HistoDateTime, ' ', HMR_LgStringDateTime);
        po_pHMRHeader->m_HistoDateTime[HMR_LgStringDateTime-1] = '\0';
        if (GetFilePtr()->GetField(HMR_HISTOGRAMDATETIME,  &pSystem))
            memcpy (po_pHMRHeader->m_HistoDateTime, pSystem, strlen(pSystem));

        // Read Histogram
        uint32_t Count;
        if (GetFilePtr()->GetField(HMR_HISTOGRAM, &Count, &pHisto))
            {
            po_pHMRHeader->m_HistogramLength = Count;
            po_pHMRHeader->m_pHistogram = new uint32_t[po_pHMRHeader->m_HistogramLength];

            for (size_t i = 0; i < po_pHMRHeader->m_HistogramLength; i++)
                po_pHMRHeader->m_pHistogram[i] = pHisto[i];
            }

        // Read File version
        GetFilePtr()->GetField(HMR_VERSION, &po_pHMRHeader->m_Version);
        GetFilePtr()->GetField(HMR_VERSION_MINOR, &po_pHMRHeader->m_MinorVersion);

        // Read
        if (GetFilePtr()->IsTiff64() == true)
            {
            po_pHMRHeader->m_p3DTranfoMatrix = Create3DMatrixFromiTiffTag();

            char*  pWKT;
            if (GetFilePtr()->GetField(HMR2_WELLKNOWNTEXT,  &pWKT) == true)

                po_pHMRHeader->m_WellKnownText = pWKT;
            }

        Byte* pOnDemandRastersInfo;
        uint32_t NbBytes;

        // Read OnDemandRastersInfo
        if (GetFilePtr()->GetField(HMR2_ONDEMANDRASTERS_INFO, &NbBytes, &pOnDemandRastersInfo) == true)
            {
            HASSERT(this->IsCompatibleWith(HRFcTiffFile::CLASS_ID));

            po_pHMRHeader->m_OnDemandRastersInfo.insert(0, (char*)pOnDemandRastersInfo, NbBytes);
            }

        // Read "No Data Values"
        uint32_t* pChannelsWithNoDataValueBuffer;
        if (GetFilePtr()->GetField(HMR2_CHANNELS_WITH_NODATAVALUE, &Count, &pChannelsWithNoDataValueBuffer) == true)
            {
            po_pHMRHeader->m_ChannelsWithNoDataValue.assign(pChannelsWithNoDataValueBuffer, pChannelsWithNoDataValueBuffer + Count);

            double* pChannelsNoDataValueBuffer;
            if (GetFilePtr()->GetField(HMR2_CHANNELS_NODATAVALUE, &Count, &pChannelsNoDataValueBuffer) == true)
                {
                po_pHMRHeader->m_ChannelsNoDataValue.assign(pChannelsNoDataValueBuffer, pChannelsNoDataValueBuffer + Count);

                HASSERT(po_pHMRHeader->m_ChannelsWithNoDataValue.size() == po_pHMRHeader->m_ChannelsNoDataValue.size());
                }
            else
                {
                HASSERT(0); // The two tag are supposed to come as a pair
                }
            }

        // Get logical shape
        GetFilePtr()->GetField(HMR_LOGICALSHAPE, (uint32_t*)&po_pHMRHeader->m_HMRClipShapeLength, &pData);
        if (po_pHMRHeader->m_HMRClipShapeLength > 0)
            {
            po_pHMRHeader->m_pHMRClipShape = new double[po_pHMRHeader->m_HMRClipShapeLength];
            memcpy(po_pHMRHeader->m_pHMRClipShape, pData, po_pHMRHeader->m_HMRClipShapeLength * sizeof(double));

            po_pHMRHeader->m_HMRClipShapeInFile = true;
            }

        // Get TileFlags
        if (GetFilePtr()->GetField(HMR2_TILEFLAG, &pTileFlags))
            {
            po_pHMRHeader->m_iTiffTileFlagsLength = (uint32_t)strlen(pTileFlags)+1;
            po_pHMRHeader->m_piTiffTileFlags = new Byte[po_pHMRHeader->m_iTiffTileFlagsLength];
            memcpy(po_pHMRHeader->m_piTiffTileFlags, pTileFlags, po_pHMRHeader->m_iTiffTileFlagsLength * sizeof(char));
            }
        else
            {
            // File have no Flags
            // Estimated number of DataBlock...
            //
            GetFilePtr()->SetDirectory(GetIndexOfPage(pi_Page));
            if (GetFilePtr()->IsTiled())
                po_pHMRHeader->m_iTiffTileFlagsLength = GetFilePtr()->NumberOfTiles() * 4;
            else
                po_pHMRHeader->m_iTiffTileFlagsLength = GetFilePtr()->NumberOfStrips() * 4;

            po_pHMRHeader->m_iTiffTileFlagsLength += (CalcNumberOfSubResolution(GetIndexOfPage(pi_Page)) * 2);
            po_pHMRHeader->m_piTiffTileFlags = new Byte[po_pHMRHeader->m_iTiffTileFlagsLength];


            if (GetAccessMode().m_HasCreateAccess)
                memset(po_pHMRHeader->m_piTiffTileFlags, HMR2_DATAFLAG_EMPTY, po_pHMRHeader->m_iTiffTileFlagsLength);
            else
                memset(po_pHMRHeader->m_piTiffTileFlags, HMR2_DATAFLAG_LOADED, po_pHMRHeader->m_iTiffTileFlagsLength);
            }

        Byte* pBuffer = 0;
        po_pHMRHeader->m_HasDecimationMethod = false;
        if (GetFilePtr()->GetField(HMR_DECIMATION_METHOD, &po_pHMRHeader->m_DecimationMethodCount, &pBuffer))
            {
            po_pHMRHeader->m_HasDecimationMethod = true;
            po_pHMRHeader->m_pDecimationMethod = new Byte[po_pHMRHeader->m_DecimationMethodCount];
            memcpy(po_pHMRHeader->m_pDecimationMethod, pBuffer, po_pHMRHeader->m_DecimationMethodCount * sizeof(Byte));
            }
        else
            po_pHMRHeader->m_DecimationMethodCount = 1;

        // HMRPixelTypeSpec
        unsigned short PixelSpec;
        if (GetFilePtr()->GetField(HMR_PIXEL_TYPE_SPEC, &PixelSpec))
            po_pHMRHeader->m_HMRPixelTypeSpec = PixelSpec;
        else
            po_pHMRHeader->m_HMRPixelTypeSpec = PIXELTYPESPEC_RGB;

        // Read transparency palette .
        if (GetFilePtr()->GetField(HMR_TRANSPARENCY_PALETTE, &Count, &pBuffer))
            {
            po_pHMRHeader->m_TransPaletteCount = Count;
            po_pHMRHeader->m_pTransPalette = new Byte[po_pHMRHeader->m_TransPaletteCount];
            memcpy(po_pHMRHeader->m_pTransPalette, pBuffer, po_pHMRHeader->m_TransPaletteCount);
            }
        else
            po_pHMRHeader->m_pTransPalette = 0;

        // Get SourceFile creation date, if present
        // That information is only keep for back compatible between V8i, on Windows platform.
        char* p_SrcFileTime=0;
        GetFilePtr()->GetField(HMR_SOURCEFILE_CREATIONDATE,  &p_SrcFileTime);
        if (p_SrcFileTime != 0)
            po_pHMRHeader->m_SourceFileCreationTime = p_SrcFileTime;

        // Reset Directory
        GetFilePtr()->SetDirectory(CurDir);
        }
    else
        {
        po_pHMRHeader->m_HMRDirDirty = true;
        Ret = false;
        }

    return (Ret);
    }

//-----------------------------------------------------------------------------
// private
// WritePrivateDirectory
//
// Write the HMR Directory into the current tiff page
//-----------------------------------------------------------------------------
void HRFiTiffFile::WritePrivateDirectory (uint32_t pi_Page)
    {
    HPRECONDITION(GetFilePtr() != 0);
    HPRECONDITION(pi_Page < CountPages());
    HASSERT(sizeof(HRFDataFlag) == sizeof(Byte));

    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();
    HFCPtr<HRFPageDescriptor> pPageDesc = GetPageDescriptor(pi_Page);
    HMRHeader* pHMRHeader = m_ppHMRHeaders[pi_Page];
    HPOSTCONDITION(pHMRHeader != 0);

    uint32_t TiffPageIndex = GetIndexOfPage(pi_Page);
    GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, TiffPageIndex));

    if (pi_Page == 0 && pPageDesc->IsEmpty())
        {
        GetFilePtr()->SetField(HMR_VERSION,       pHMRHeader->m_Version);
        GetFilePtr()->SetField(HMR_VERSION_MINOR, pHMRHeader->m_MinorVersion);
        }
    else if (!pPageDesc->IsEmpty())
        {
        // Check if TilesFlag are changed
        //
        // File already have the Flags ?
        if (pHMRHeader->m_iTiffTileFlagsLength == 0)
            {
            // Estimated number of DataBlock...
            if (pPageDesc->GetResolutionDescriptor(0)->GetBlockType() == HRFBlockType::TILE)
                pHMRHeader->m_iTiffTileFlagsLength = GetFilePtr()->NumberOfTiles() * 4;
            else
                pHMRHeader->m_iTiffTileFlagsLength = GetFilePtr()->NumberOfStrips() * 4;
            pHMRHeader->m_iTiffTileFlagsLength += (pPageDesc->CountResolutions() * 2);

            pHMRHeader->m_piTiffTileFlags      = new Byte[pHMRHeader->m_iTiffTileFlagsLength];
            memset(pHMRHeader->m_piTiffTileFlags, HMR2_DATAFLAG_EMPTY, pHMRHeader->m_iTiffTileFlagsLength);
            }

        uint32_t                        CountFlags = 0;
        HFCPtr<HRFResolutionDescriptor> pResDescriptor;

        // For each Resolution...
        for (uint32_t i = 0; i < pPageDesc->CountResolutions(); i++)
            {
            pResDescriptor            = pPageDesc->GetResolutionDescriptor((unsigned short)i);
            const HRFDataFlag* pFlags = pResDescriptor->GetBlocksDataFlag();
            uint32_t NbResFlags         = (uint32_t)pResDescriptor->GetBlocksPerWidth() *
                                        (uint32_t)pResDescriptor->GetBlocksPerHeight();

#ifdef __HMR_DEBUG
            // Checking....
            if (pHMRHeader->m_iTiffTileFlagsLength < (CountFlags+NbResFlags+1))
                HASSERT(0);
#endif

            // Check if flags are changed
            if (memcmp (&(pHMRHeader->m_piTiffTileFlags[CountFlags]), pFlags, NbResFlags) != 0)
                {
                memcpy(&(pHMRHeader->m_piTiffTileFlags[CountFlags]), pFlags, NbResFlags);
                pHMRHeader->m_HMRDirDirty = true;
                }
            CountFlags += NbResFlags;
            // dn Resolution.
            pHMRHeader->m_piTiffTileFlags[CountFlags] = HMR2_TILEFLAG_ENDRESOLUTION;
            CountFlags++;
            }
        pHMRHeader->m_piTiffTileFlags[CountFlags-1] = 0;

        // HMR Directory is changed ?
        if (pHMRHeader->m_HMRDirDirty)
            {
            HFCPtr<HRPPixelType> pPixelType = pPageDesc->GetResolutionDescriptor(0)->GetPixelType();

            // If directory not present, Add it.
            if (!GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
                GetFilePtr()->AddHMRDirectory(HMR2_IMAGEINFORMATION);

            // Tag HMR
            GetFilePtr()->SetField(HMR_VERSION,                pHMRHeader->m_Version);
            GetFilePtr()->SetField(HMR_VERSION_MINOR,          pHMRHeader->m_MinorVersion);
            GetFilePtr()->SetFieldA(HMR_IMAGECOORDINATESYSTEM,  pHMRHeader->m_SystemCoord);
            GetFilePtr()->SetField(HMR_HISTOGRAM, pHMRHeader->m_HistogramLength, pHMRHeader->m_pHistogram);

            // DateTime
            GetSystemDateTime (pHMRHeader->m_HistoDateTime);
            GetFilePtr()->SetFieldA(HMR_HISTOGRAMDATETIME,  pHMRHeader->m_HistoDateTime);


            // 3D Transformation Model
            if (pHMRHeader->m_p3DTranfoMatrix != 0)
                {
                HASSERT(GetFilePtr()->IsTiff64() == true);
                bool Result = Write3DMatrixToTiffTag(*(pHMRHeader->m_p3DTranfoMatrix.get()));
                HASSERT(Result != false);
                }
            else
                {
                GetFilePtr()->RemoveTag(HMR2_3D_TRANSFO_MATRIX);
                }

            // OnDemand Rasters Info
            if (!pHMRHeader->m_OnDemandRastersInfo.empty())
                {
                HAutoPtr<Byte> pInfo(new Byte[pHMRHeader->m_OnDemandRastersInfo.size()]);

                GetFilePtr()->SetField(HMR2_ONDEMANDRASTERS_INFO,
                                       (uint32_t)pHMRHeader->m_OnDemandRastersInfo.size(),
                                       (Byte*)pHMRHeader->m_OnDemandRastersInfo.data());
                }
            else
                {
                GetFilePtr()->RemoveTag(HMR2_ONDEMANDRASTERS_INFO);
                }


            if (!pHMRHeader->m_ChannelsWithNoDataValue.empty())
                {
                HASSERT(pHMRHeader->m_ChannelsWithNoDataValue.size() == pHMRHeader->m_ChannelsNoDataValue.size());

                GetFilePtr()->SetField( HMR2_CHANNELS_WITH_NODATAVALUE,
                                        (uint32_t)pHMRHeader->m_ChannelsWithNoDataValue.size(),
                                        (uint32_t*)&pHMRHeader->m_ChannelsWithNoDataValue[0]);
                GetFilePtr()->SetField( HMR2_CHANNELS_NODATAVALUE,
                                        (uint32_t)pHMRHeader->m_ChannelsNoDataValue.size(),
                                        &pHMRHeader->m_ChannelsNoDataValue[0]);

                }
            else
                {
                GetFilePtr()->RemoveTag(HMR2_CHANNELS_WITH_NODATAVALUE);
                GetFilePtr()->RemoveTag(HMR2_CHANNELS_NODATAVALUE);
                }


            // Set logical shape
            if ((pHMRHeader->m_HMRClipShapeLength > 0) || pHMRHeader->m_HMRClipShapeInFile)
                GetFilePtr()->SetField(HMR_LOGICALSHAPE,
                                       pHMRHeader->m_HMRClipShapeLength,
                                       pHMRHeader->m_pHMRClipShape);

            // Set TilesFlag...
            if (pHMRHeader->m_iTiffTileFlagsLength > 0)
                GetFilePtr()->SetFieldA(HMR2_TILEFLAG, (char*)pHMRHeader->m_piTiffTileFlags.get());

            // Down Sampling Method
            GetFilePtr()->SetField(HMR_DECIMATION_METHOD,
                                   pHMRHeader->m_DecimationMethodCount,
                                   (Byte*)pHMRHeader->m_pDecimationMethod.get());

            // Set transparency palette.
            if (pPixelType->CountIndexBits() != 0 &&
                pPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
                {
                HArrayAutoPtr<Byte> pTransPalette(new Byte[pPixelType->GetPalette().GetMaxEntries()]);
                memset(pTransPalette, 0, pPixelType->GetPalette().GetMaxEntries());
                Byte* pSourceComposite;

                for (uint32_t i = 0; i < pPixelType->GetPalette().CountUsedEntries(); ++i)
                    {
                    pSourceComposite = (Byte*) (pPixelType->GetPalette().GetCompositeValue(i));
                    pTransPalette[i] = (Byte) pSourceComposite[3];
                    }
                GetFilePtr()->SetField(HMR_TRANSPARENCY_PALETTE,
                                       pPixelType->GetPalette().GetMaxEntries(),
                                       pTransPalette);
                }

            if (GetFilePtr()->IsTiff64())
                {
                if (!pHMRHeader->m_WellKnownText.empty ())
                    GetFilePtr()->SetFieldA(HMR2_WELLKNOWNTEXT, pHMRHeader->m_WellKnownText.c_str());
                else
                    GetFilePtr()->RemoveTag(HMR2_WELLKNOWNTEXT);
                }

            // Reset Directory
            GetFilePtr()->SetDirectory(CurDir);
            }
        pHMRHeader->m_HMRDirDirty = false;
        }
    }

//-----------------------------------------------------------------------------
// Private
// GetClipShape
//-----------------------------------------------------------------------------
HRFClipShape* HRFiTiffFile::GetClipShape (const HMRHeader* pi_pHMRHeader)
    {
    HPRECONDITION(pi_pHMRHeader != 0);
    HRFClipShape* pClipShape = 0;

    // Create the shape from the file if available
    if (pi_pHMRHeader->m_HMRClipShapeLength > 3)
        {
        pClipShape =  ImportShapeFromArrayOfDouble(pi_pHMRHeader->m_pHMRClipShape, pi_pHMRHeader->m_HMRClipShapeLength);
        }

    return (pClipShape);
    }

//-----------------------------------------------------------------------------
// Private
// SetClipShape
//-----------------------------------------------------------------------------
void HRFiTiffFile::SetClipShape(HMRHeader*          pio_pHMRHeader,
                                const HRFClipShape& pi_rShape)
    {
    HPRECONDITION(pio_pHMRHeader != 0);

    size_t  NbPoints;
    pio_pHMRHeader->m_HMRDirDirty   = true;

    pio_pHMRHeader->m_pHMRClipShape = ExportClipShapeToArrayOfDouble(pi_rShape, &NbPoints);
    HASSERT_X64(NbPoints < LONG_MAX);
    pio_pHMRHeader->m_HMRClipShapeLength = (int32_t)NbPoints;
    }

//-----------------------------------------------------------------------------
// Private
// SetHistogram
//-----------------------------------------------------------------------------
void HRFiTiffFile::SetHistogram(HMRHeader*          pio_pHMRHeader,
                                const HRPHistogram& pi_rHistogram)
    {
    HPRECONDITION(pio_pHMRHeader != 0);
    HPRECONDITION(pi_rHistogram.GetEntryFrequenciesSize() == 256 || pi_rHistogram.GetEntryFrequenciesSize() == 65536);
    HPRECONDITION(pi_rHistogram.GetSamplingColorSpace() == HRPHistogram::NATIVE);


    // This file format support only FrequenciesSize == 256 or == 65536.
    if (pi_rHistogram.GetEntryFrequenciesSize() == 256 || pi_rHistogram.GetEntryFrequenciesSize() == 65536)
        {
        pio_pHMRHeader->m_HistogramLength = pi_rHistogram.GetEntryFrequenciesSize() * pi_rHistogram.GetChannelCount();
        pio_pHMRHeader->m_pHistogram = new uint32_t[pio_pHMRHeader->m_HistogramLength];

        // iTiff only support native histogram.
        if(pi_rHistogram.GetSamplingColorSpace() == HRPHistogram::NATIVE)
            {
            // This file format doesn't support more than 3 channel.
            HASSERT(pi_rHistogram.GetChannelCount() <= 3);  // Why? we already allocate ChannelCount in pio_pHMRHeader->m_pHistogram
            uint32_t ChannelCount = MIN(pi_rHistogram.GetChannelCount(), 3);
            for (uint32_t ChannelIndex = 0; ChannelIndex < ChannelCount; ChannelIndex++)
                {
                pi_rHistogram.GetEntryFrequencies(pio_pHMRHeader->m_pHistogram.get() + (ChannelIndex * pi_rHistogram.GetEntryFrequenciesSize()),
                                                  ChannelIndex);
                }
            }
        else
            {
            // We can't tell if the current histogram is up to date so restore to default "invalid" value
            for (size_t i = 0; i < pio_pHMRHeader->m_HistogramLength; i++)
                pio_pHMRHeader->m_pHistogram[i] = 1L;
            }

        pio_pHMRHeader->m_HMRDirDirty = true;
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
// (This fonction was previusly define in the hpp file but since HTiffFile.h is
//  no longer include in HRFiTiffFile we need define it in the cpp. file.)
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFiTiffFile::GetWorldIdentificator () const
    {
    return (HGF2DWorld_ITIFFWORLD);
    }

//-----------------------------------------------------------------------------
// Protected
// Return the byte code coresponding to the HRFDownSamplingMethod.
//-----------------------------------------------------------------------------
Byte HRFiTiffFile::GetDownSamplingMethodCode(HRFDownSamplingMethod pi_Sampling) const
    {
    Byte Result = HMR_UNDEFINED_DECIMATION;

    // Find a name that fit with the given sampling methode.
    if (pi_Sampling == HRFDownSamplingMethod::NEAREST_NEIGHBOUR)
        {
        Result = HMR_NEAREST_NEIGHBOUR_DECIMATION;
        }
    else if (pi_Sampling == HRFDownSamplingMethod::AVERAGE)
        {
        Result = HMR_AVERAGE_DECIMATION;
        }
    else if (pi_Sampling == HRFDownSamplingMethod::VECTOR_AWARENESS)
        {
        Result = HMR_VECTOR_AWARENESS_DECIMATION;
        }
    else if (pi_Sampling == HRFDownSamplingMethod::UNKOWN)
        {
        Result = HMR_UNDEFINED_DECIMATION;
        }
    else if (pi_Sampling == HRFDownSamplingMethod::ORING4)
        {
        Result = HMR_ORING4_DECIMATION;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Protected
// Return the HRFDownSamplingMethod coresponding to the byte code.
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFiTiffFile::GetDownSamplingMethod(Byte pi_Sampling) const
    {
    HRFDownSamplingMethod Result;

    // Find a name that fit with the given sampling methode.
    if (pi_Sampling == HMR_NEAREST_NEIGHBOUR_DECIMATION)
        {
        Result = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
        }
    else if (pi_Sampling == HMR_AVERAGE_DECIMATION)
        {
        Result = HRFDownSamplingMethod::AVERAGE;
        }
    else if (pi_Sampling == HMR_VECTOR_AWARENESS_DECIMATION)
        {
        Result = HRFDownSamplingMethod::VECTOR_AWARENESS;
        }
    else if (pi_Sampling == HMR_UNDEFINED_DECIMATION)
        {
        Result = HRFDownSamplingMethod::UNKOWN;
        }
    else if (pi_Sampling == HMR_ORING4_DECIMATION)
        {
        Result = HRFDownSamplingMethod::ORING4;
        }
    else
        {
        Result = HRFDownSamplingMethod::UNKOWN;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Protected
// Reload the directories for the iTiff file format
//-----------------------------------------------------------------------------
void HRFiTiffFile::ReloadDescriptors()
    {
    HRFTiffFile::ReloadDescriptors();

    char*   pTileFlags;

    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();
    HMRHeader* pHMRHeader;
    // Reload Flags to the descriptor for each page
    for (uint32_t Page=0; Page < CountPages(); Page++)
        {
        // If directory not present, Add it.
        if (!GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, Page)))
            GetFilePtr()->AddHMRDirectory(HMR2_IMAGEINFORMATION);

        // Get TileFlags
        if (GetFilePtr()->GetField(HMR2_TILEFLAG, &pTileFlags))
            {
            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);
            pHMRHeader = m_ppHMRHeaders[Page];
            HPOSTCONDITION(pHMRHeader != 0);
            pHMRHeader->m_iTiffTileFlagsLength = (uint32_t)strlen(pTileFlags)+1;
            if (memcmp (pHMRHeader->m_piTiffTileFlags, pTileFlags, pHMRHeader->m_iTiffTileFlagsLength) != 0)
                {

                pHMRHeader->m_piTiffTileFlags = new Byte[pHMRHeader->m_iTiffTileFlagsLength];
                memcpy(pHMRHeader->m_piTiffTileFlags, pTileFlags, pHMRHeader->m_iTiffTileFlagsLength * sizeof(char));
                uint32_t CurDataFlagsResolution = 0;

                // For each Resolution...
                for (uint32_t i=0; i<pPageDescriptor->CountResolutions(); i++)
                    {
                    // Select the page and resolution
                    SetImageInSubImage (GetIndexOfPage(Page)+i);

                    HFCPtr<HRFResolutionDescriptor> pResDescriptor = pPageDescriptor->GetResolutionDescriptor((unsigned short)i);
                    pResDescriptor->SetBlocksDataFlag((HRFDataFlag*)&(pHMRHeader->m_piTiffTileFlags[CurDataFlagsResolution]));

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
    GetFilePtr()->SetDirectory(CurDir);
    }

//-----------------------------------------------------------------------------
// Protected
// Save the directories for the iTiff file format
//-----------------------------------------------------------------------------
void HRFiTiffFile::SaveDescriptors(uint32_t pi_Page)
    {
    HPRECONDITION (GetFilePtr() != 0);
    HPRECONDITION(CountPages() > 0);
    HASSERT(sizeof(HRFDataFlag) == sizeof(Byte));

    if (pi_Page == -1)
        {
        for (uint32_t Page = 0; Page < CountPages(); Page++)
            SaveDescriptor(Page);
        }
    else
        SaveDescriptor(pi_Page);
    }


//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
void HRFiTiffFile::SaveDescriptor(uint32_t pi_Page)
    {
    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(pi_Page);
    if (!pPageDescriptor->IsEmpty())
        {
        HFCPtr<HRFResolutionDescriptor> pResDescriptor;
        uint32_t                        CountFlags = 0;
        HMRHeader*                      pHMRHeader = m_ppHMRHeaders[pi_Page];
        HPOSTCONDITION(pHMRHeader != 0);

        // For each Resolution...
        for (uint32_t Res = 0; Res < pPageDescriptor->CountResolutions(); Res++)
            {
            pResDescriptor            = pPageDescriptor->GetResolutionDescriptor((unsigned short)Res);
            const HRFDataFlag* pFlags = pResDescriptor->GetBlocksDataFlag();
            uint32_t NbResFlags         = (uint32_t)pResDescriptor->GetBlocksPerWidth() *
                                        (uint32_t)pResDescriptor->GetBlocksPerHeight();

#ifdef __HMR_DEBUG
            // Checking....
            if (pHMRHeader->m_iTiffTileFlagsLength < (CountFlags+NbResFlags+1))
                HASSERT(0);
#endif

            memcpy(&(pHMRHeader->m_piTiffTileFlags[CountFlags]), pFlags, NbResFlags);

            CountFlags += NbResFlags;
            // dn Resolution.
            pHMRHeader->m_piTiffTileFlags[CountFlags] = HMR2_TILEFLAG_ENDRESOLUTION;
            CountFlags++;
            }
        pHMRHeader->m_piTiffTileFlags[CountFlags-1] = 0;

        // If directory not present, Add it.
        if (!GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, pi_Page)))
            {
            HASSERT(false);
            }

        // Set TilesFlag...
        if (pHMRHeader->m_iTiffTileFlagsLength > 0)
            GetFilePtr()->SetFieldA(HMR2_TILEFLAG, (char*)pHMRHeader->m_piTiffTileFlags.get());
        }

    // Reset Directory
    GetFilePtr()->SetDirectory(CurDir);
    HRFTiffFile::SaveDescriptors();
    }


//-----------------------------------------------------------------------------
// Static Private
// true if all pixel type are homogeneous for all resolutions in the specified
// page.
// false otherwise.
//-----------------------------------------------------------------------------
bool HRFiTiffFile::IsSamePixelTypeForAllResolutions (HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    bool                       AreAllPixelTypesHomogeneous = true;
    const HFCPtr<HRPPixelType>  pReferencePixelType         = pi_pPage->GetResolutionDescriptor(0)->GetPixelType();

    for (unsigned short Resolution=0; Resolution < pi_pPage->CountResolutions(); Resolution++)
        {
        const HFCPtr<HRPPixelType> pCurrentPixelType = pi_pPage->GetResolutionDescriptor(Resolution)->GetPixelType();

        if (pCurrentPixelType->GetClassID() != pReferencePixelType->GetClassID())
            {
            AreAllPixelTypesHomogeneous = false;
            break;
            }
        }

    return AreAllPixelTypesHomogeneous;
    }

//-----------------------------------------------------------------------------
// Static Private
// Generate Tag Data for channel no data value
//-----------------------------------------------------------------------------
void HRFiTiffFile::GenerateNoDataValuesTagData (ListOfChannelIndex*         po_pChannelsWithNoDataValue,
                                                ListOfChannelNoDataValue*   po_pChannelsNoDataValue,
                                                const HRPChannelOrg&        pi_rChannelOrg)
    {
    HPRECONDITION(0 != po_pChannelsWithNoDataValue);
    HPRECONDITION(0 != po_pChannelsNoDataValue);

    po_pChannelsWithNoDataValue->clear();
    po_pChannelsNoDataValue->clear();

    const double*     pLastNoDataValue                    = pi_rChannelOrg.GetChannelPtr(0)->GetNoDataValue();
    uint32_t            ChannelsWithNoDataValueCount        = 0;
    bool               AreAllNoDataValuesHomogeneous       = true;

    const uint32_t      ChannelCount                        = pi_rChannelOrg.CountChannels();
    for (uint32_t ChannelIndex = 0; ChannelIndex < ChannelCount; ++ChannelIndex)
        {
        const double* pNoDataValue = pi_rChannelOrg.GetChannelPtr(ChannelIndex)->GetNoDataValue();

        if (0 != pNoDataValue)
            {
            // Append the current "No Data Value" to the output
            po_pChannelsWithNoDataValue->push_back(ChannelIndex);
            po_pChannelsNoDataValue->push_back(*pNoDataValue);

            // Check if the current "No Data Value" is different from the preceding
            if (0 != pLastNoDataValue    &&
                (*pNoDataValue) != (*pLastNoDataValue))
                {
                AreAllNoDataValuesHomogeneous = false;
                }

            // Count channels that have a "No Data Value"
            ++ChannelsWithNoDataValueCount;
            }

        pLastNoDataValue = pNoDataValue;
        }

    // Special case when all channels have a "No Data Value" and that these
    // are set to the same value.
    if (AreAllNoDataValuesHomogeneous &&
        ChannelCount == ChannelsWithNoDataValueCount)
        {
        // Clear the output
        po_pChannelsWithNoDataValue->clear();
        po_pChannelsNoDataValue->clear();


        // Append a special index value to the output that indicates that
        // all channel have the same exact "No Data Value".
        po_pChannelsWithNoDataValue->push_back(CHANNEL_INDEX_ALL_CHANNELS);
        po_pChannelsNoDataValue->push_back(*pi_rChannelOrg.GetChannelPtr(0)->GetNoDataValue());
        }

    }


//-----------------------------------------------------------------------------
// Public
// HRFiTiff64File
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
HRFiTiff64File::~HRFiTiff64File()
    {

    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFiTiff64File::GetCapabilities () const
    {
    return HRFiTiff64Creator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Protected
// Open
// Open the file.
//-----------------------------------------------------------------------------
bool HRFiTiff64File::Open(bool pi_CreateBigTifFormat)
    {
    HASSERT(pi_CreateBigTifFormat==true);
    return HRFTiffFile::Open(true);
    }
