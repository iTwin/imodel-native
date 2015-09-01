//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTClassIDDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HUTClassIDDescriptor
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HUTClassIDDescriptor.h>

// Codec headers files
#include <Imagepp/all/h/HCDCodec.h>
#include <Imagepp/all/h/HCDCodecDeflate.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecPackBits.h>
#include <Imagepp/all/h/HCDCodecCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecRLE8.h>
#include <Imagepp/all/h/HCDCodecRLE1.h>
#include <Imagepp/all/h/HCDCodecJPEG.h>
//#include "HCDCodecPCX.h"
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecFPXSingleColor.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecBMPRLE8.h>
#include <Imagepp/all/h/HCDCodecBMPRLE4.h>
#include <Imagepp/all/h/HCDCodecLZW.h>
#include <Imagepp/all/h/HCDCodecHMRGIF.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecTgaRLE.h>
#include <Imagepp/all/h/HCDCodecFLIRLE8.h>
#include <Imagepp/all/h/HCDCodecLRDRLE.h>
#include <Imagepp/all/h/HCDCodecJPEGAlpha.h>

#include <Imagepp/all/h/HCDCodecJBIG.h>

#include <Imagepp/all/h/HCDCodecJPEG2000.h>
#include <Imagepp/all/h/HCDCodecECW.h>
#include <Imagepp/all/h/HCDCodecCRL8.h>
#include <Imagepp/all/h/HCDCodecCCITTFax4.h>

#include <Imagepp/all/h/HCDCodecCCITTRLE.h>

// PixelType header files
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8Mask.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeRGB.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32A8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>

#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

//TransfoModel header files
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HGF2DLinearModelAdapter.h>
#include <Imagepp/all/h/HGF2DProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>
#include <Imagepp/all/h/HGF2DHelmert.h>

//Filter header files
#include <Imagepp/all/h/HRPFilter.h>
#include <Imagepp/all/h/HRPComplexFilter.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPTypeAdaptFilters.h>
#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <ImagePP/all/h/HRPCustomConvFilter.h>
#include <ImagePP/all/h/HRPMapFilters16.h>

//Scan line Orientation header files
#include <Imagepp/all/h/HRFTypes.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


HFC_IMPLEMENT_SINGLETON(HUTClassIDDescriptor)

//-----------------------------------------------------------------------------
// Default Constructor
//
// For multilingual purpose, we may pass a language descriptor to the
// constructor and use it to create the right string..
//-----------------------------------------------------------------------------


// Note:  All the visible strings to the user are now located in resource file Ipprsc.rc



HUTClassIDDescriptor::HUTClassIDDescriptor()
    : m_NotFound(ImagePPMessages::GetStringW(ImagePPMessages::IMAGEPP_NotFound()))
    {
    // Codec
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecIdentity::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::CODEC_Identity(),    L"00"))); //None
    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecCCITT::CLASS_ID              , HFCClassDescriptor(ImagePPMessages::CODEC_CCITT3(),    L"01"))); //CCITT3
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecHMRCCITT::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::CODEC_CCITT4(),    L"02"))); //CCITT4
    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecDeflate::CLASS_ID            , HFCClassDescriptor(ImagePPMessages::CODEC_Deflate(),    L"03"))); //Deflate
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecZlib::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::CODEC_Deflate(),    L"03"))); //Deflate
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecHMRPackBits::CLASS_ID        , HFCClassDescriptor(ImagePPMessages::CODEC_Packbits(),    L"04"))); //Packbits
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecLZW::CLASS_ID                , HFCClassDescriptor(ImagePPMessages::CODEC_LZW(),    L"05"))); //LZW
    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecJPEG::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::CODEC_JPEG(),    L"06"))); //Jpeg
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecIJG::CLASS_ID                , HFCClassDescriptor(ImagePPMessages::CODEC_JPEG(),    L"06"))); //Jpeg
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecFlashpix::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::CODEC_FlashPix(),    L"07"))); //FlashPix
    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecRLE1::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::CODEC_RLE1(),    L"08"))); //RLE 1
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecHMRRLE1::CLASS_ID            , HFCClassDescriptor(ImagePPMessages::CODEC_RLE1(),    L"08"))); //RLE 1
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecRLE8::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::CODEC_RLE8(),    L"09"))); //RLE 8
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecBMPRLE8::CLASS_ID            , HFCClassDescriptor(ImagePPMessages::CODEC_BMPRLE8(),    L"11"))); //BMPRLE8
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecBMPRLE4::CLASS_ID            , HFCClassDescriptor(ImagePPMessages::CODEC_BMPRLE4(),    L"12"))); //BMPRLE4
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecHMRGIF::CLASS_ID             , HFCClassDescriptor(ImagePPMessages::CODEC_Gif(),        L"13"))); //Gif
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecFPXSingleColor::CLASS_ID     , HFCClassDescriptor(ImagePPMessages::CODEC_FlashPixSingleColor(),    L"14"))); //FlashPix (single color)
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecCCITTRLE::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::CODEC_CCITTRLE(),    L"15"))); //CCITTRLE
    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodec????::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::CODEC_CCITTRLEW(),    L"16"))); //CCITTRLEW
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecTGARLE::CLASS_ID             , HFCClassDescriptor(ImagePPMessages::CODEC_TgaRLE(),    L"17"))); //Tga RLE
    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecPCX::CLASS_ID                , HFCClassDescriptor(ImagePPMessages::CODEC_PCX(),    L"19"))); //PCX
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecJPEGAlpha::CLASS_ID          , HFCClassDescriptor(ImagePPMessages::CODEC_JpegAlpha(),    L"20"))); //JpegAlpha

    //m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecTGARLE::CLASS_ID             , HFCClassDescriptor(ImagePPMessages::CODEC_TGARLE(),    L"23"))); //TGARLE
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecFLIRLE8::CLASS_ID            , HFCClassDescriptor(ImagePPMessages::CODEC_FLIRLE8(),    L"24")));
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecLRDRLE::CLASS_ID             , HFCClassDescriptor(ImagePPMessages::CODEC_LRDRLE1(),    L"25")));

    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecCRL8::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::CODEC_CRL8(),   L"27")));//CRL8
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecCCITTFax4::CLASS_ID          , HFCClassDescriptor(ImagePPMessages::CODEC_CCITTFax4(),   L"28")));//CCITTFax4

#ifdef JBIG_SUPPORT
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecJBIG::CLASS_ID                 , HFCClassDescriptor(ImagePPMessages::CODEC_JBIG(), L"29"))); //JBIG
#endif

    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecJPEG2000::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::CODEC_Jpeg2000(), L"30")));  //JPEG2000
    m_CodecClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCDCodecECW::CLASS_ID                , HFCClassDescriptor(ImagePPMessages::CODEC_ECW(),      L"31")));  //ECW

    // Pixel Type
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI1R8G8B8::CLASS_ID       , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_2ColorsPalette()     , L"1C")));     //2 colors palette
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI1R8G8B8RLE::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_2ColorsPaletteRLE()          , L"1CRLE"))); //2 colors palette RLE
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI1R8G8B8A8::CLASS_ID     , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_2ColorsPaletteAlpha()          , L"1A"))); //2 colors palette alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID  , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_2ColorsPaletteAlphaRLE()          , L"1ARLE"))); //2 colors palette alpha RLE
    //Disabled m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI2R8G8B8::CLASS_ID       , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_4Colors()          , L"2C"))); //4 colors
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI4R8G8B8::CLASS_ID       , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_16Colors()          , L"4C"))); //16 colors
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI4R8G8B8A8::CLASS_ID     , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_16ColorsAlpha()          , L"4A"))); //16 colors alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID       , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_256Colors()          , L"8C"))); //256 colors
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI8R8G8B8A8::CLASS_ID     , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_256ColorsAlpha()          , L"8A"))); //256 colors alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI8R8G8B8Mask::CLASS_ID   , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_256ColorsDCMask()          , L"8MSK"))); //256 colors DC Mask
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI8Gray8::CLASS_ID        , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Gray8Palette()          , L"GP"))); //Gray 8 Palette
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV1Gray1::CLASS_ID        , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_BlackAndWhite()          , L"1B"))); //Black & white
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV1GrayWhite1::CLASS_ID   , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_WhiteAndBlack()          , L"1W"))); //White & black
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV8Gray8::CLASS_ID        , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Gray8()          , L"8G"))); //Gray 8
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV8GrayWhite8::CLASS_ID   , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Gray8White()          , L"GI"))); //Gray 8 inv.
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV16PRGray8A8::CLASS_ID   , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Gray8Alpha()          , L"GA"))); //Gray 8 alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV16R5G6B5::CLASS_ID      , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors16()          , L"16V"))); //True colors 16
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV24PhotoYCC::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_YCC()          , L"YC"))); //YCC
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID, HFCClassDescriptor(ImagePPMessages::PIXELTYPE_YCCAlpha()          , L"YA"))); //YCC alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32CMYK::CLASS_ID        , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_CMYK()          , L"CK"))); //CMYK
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID      , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors24()          , L"TC"))); //True colors 24
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID      , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors24BGR()          , L"TCI"))); //True colors 24 BGR.
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32B8G8R8X8::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors32BGRX()         , L"TXI"))); //True colors 32 BGR
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors32()          , L"TA"))); //True colors 32
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32R8G8B8X8::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors32RGBX()          , L"TX"))); //True colors 32 no Alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors32PreMult()          , L"TP"))); //True colors 32 PR
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32A8R8G8B8::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors32ARGB()          , L"TAF"))); //True colors 32 ARGB
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeI8VA8R8G8B8::CLASS_ID    , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_256ColorsARGB()          , L"8V"))); //256 colors with alpha channel
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV16Gray16::CLASS_ID      , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Gray16()          , L"16G")));   //Gray 16
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV16Int16::CLASS_ID       , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Integer16()  , L"16INT"))); //Int 16
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV32Float32::CLASS_ID     , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_Float32(), L"32F"))); //Float 32
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID   , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors48()          , L"48V"))); //True colors 48
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors64()          , L"64A"))); //True colors 64
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV64R16G16B16X16::CLASS_ID, HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors64RGBX()          , L"64X"))); //True colors 64 no Alpha
    m_PixelClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPPixelTypeV96R32G32B32::CLASS_ID   , HFCClassDescriptor(ImagePPMessages::PIXELTYPE_TrueColors96()     , L"TC96"))); // True color 96

    // TransfoModel
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DStretch::CLASS_ID             , HFCClassDescriptor(ImagePPMessages::TRANSFO_Strech()            , L""))); //Stretch
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DTranslation::CLASS_ID         , HFCClassDescriptor(ImagePPMessages::TRANSFO_Translation()        , L""))); //Translation
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DIdentity::CLASS_ID            , HFCClassDescriptor(ImagePPMessages::TRANSFO_Identity()           , L""))); //Identity
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DSimilitude::CLASS_ID          , HFCClassDescriptor(ImagePPMessages::TRANSFO_Similitude()         , L""))); //Similitude
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DProjective::CLASS_ID          , HFCClassDescriptor(ImagePPMessages::TRANSFO_Projective()         , L""))); //Projective
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DAffine::CLASS_ID              , HFCClassDescriptor(ImagePPMessages::TRANSFO_Affine()             , L""))); //Affine
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HCPGCoordModel::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::TRANSFO_ProjectionCoordModel()         , L""))); //Coord Model
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DLinearModelAdapter::CLASS_ID  , HFCClassDescriptor(ImagePPMessages::TRANSFO_LinearModelAdapter() , L""))); //Linear Model Adapter
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DProjectiveGrid::CLASS_ID      , HFCClassDescriptor(ImagePPMessages::TRANSFO_ProjectiveGrid()     , L""))); //Projective Grid
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DComplexTransfoModel::CLASS_ID , HFCClassDescriptor(ImagePPMessages::TRANSFO_ComplexModel(), L""))); //Complex Transfo Model
    m_TransfoClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HGF2DHelmert::CLASS_ID             , HFCClassDescriptor(ImagePPMessages::TRANSFO_Helmert()            , L""))); //Helmert

    // Filter
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPBlurFilter::CLASS_ID                       , HFCClassDescriptor(ImagePPMessages::FILTER_Blur(),              L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPColorBalanceFilter::CLASS_ID               , HFCClassDescriptor(ImagePPMessages::FILTER_ColorBalance(),      L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPColortwistFilter::CLASS_ID                 , HFCClassDescriptor(ImagePPMessages::FILTER_ColorTwist(),        L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPComplexFilter::CLASS_ID                    , HFCClassDescriptor(ImagePPMessages::FILTER_Complex(),           L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPContrastFilter::CLASS_ID                   , HFCClassDescriptor(ImagePPMessages::FILTER_Contrast(),          L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPDetailFilter::CLASS_ID                     , HFCClassDescriptor(ImagePPMessages::FILTER_Detail(),            L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPEdgeEnhanceFilter::CLASS_ID                , HFCClassDescriptor(ImagePPMessages::FILTER_EdgeEnhance(),       L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPFindEdgesFilter::CLASS_ID                  , HFCClassDescriptor(ImagePPMessages::FILTER_FindEdges(),         L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPSharpenFilter::CLASS_ID                    , HFCClassDescriptor(ImagePPMessages::FILTER_Sharpen(),           L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPSmoothFilter::CLASS_ID                     , HFCClassDescriptor(ImagePPMessages::FILTER_Smooth(),            L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPAverageFilter::CLASS_ID                    , HFCClassDescriptor(ImagePPMessages::FILTER_Average(),           L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPCustomConvFilter::CLASS_ID                 , HFCClassDescriptor(ImagePPMessages::FILTER_CustomConvolution(), L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPAlphaReplacer::CLASS_ID                    , HFCClassDescriptor(ImagePPMessages::FILTER_AlphaReplacer(),     L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPAlphaComposer::CLASS_ID                    , HFCClassDescriptor(ImagePPMessages::FILTER_AlphaComposer(),     L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPColorReplacerFilter::CLASS_ID              , HFCClassDescriptor(ImagePPMessages::FILTER_ColorReplacer(),     L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPHistogramScalingFilter::CLASS_ID           , HFCClassDescriptor(ImagePPMessages::FILTER_HistogramScaling(),  L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPGammaFilter::CLASS_ID                      , HFCClassDescriptor(ImagePPMessages::FILTER_Gamma(),             L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPInvertFilter::CLASS_ID                     , HFCClassDescriptor(ImagePPMessages::FILTER_Invert(),            L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPTintFilter::CLASS_ID                       , HFCClassDescriptor(ImagePPMessages::FILTER_Tint(),              L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPCustomMap8Filter::CLASS_ID                 , HFCClassDescriptor(ImagePPMessages::FILTER_CustomMap8(),        L""))); 
    m_FilterClassIDDescriptorMap.insert(IDDescriptorMap::value_type(HRPCustomMap16Filter::CLASS_ID                , HFCClassDescriptor(ImagePPMessages::FILTER_CustomMap16(),       L""))); 
    }


//-----------------------------------------------------------------------------
// Class Destructor
//-----------------------------------------------------------------------------

HUTClassIDDescriptor::~HUTClassIDDescriptor()
    {
    // Clear the map...
    m_CodecClassIDDescriptorMap.clear();
    m_PixelClassIDDescriptorMap.clear();
    m_TransfoClassIDDescriptorMap.clear();
    m_FilterClassIDDescriptorMap.clear();
    }

//-----------------------------------------------------------------------------
//GetLabel for Codec by Class key
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelCodec(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_CodecClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_CodecClassIDDescriptorMap.end())
        return ImagePPMessages::GetStringW(static_cast<ImagePPMessages::StringId>((*itr).second.GetStringID()));
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//GetLabel for Codec by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelCodec(const HCDCodec& pi_rCodec) const
    {
    return GetClassLabelCodec(pi_rCodec.GetClassID());
    }
//-----------------------------------------------------------------------------
//GetLabel for Pixel Type by Class key
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelPixelType(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_PixelClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_PixelClassIDDescriptorMap.end())
        return ImagePPMessages::GetStringW(static_cast<ImagePPMessages::StringId>((*itr).second.GetStringID()));
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//GetLabel for Pixel Type by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelPixelType(const HRPPixelType& pi_rPixelType) const
    {
    return GetClassLabelPixelType(pi_rPixelType.GetClassID());
    }
//-----------------------------------------------------------------------------
//GetLabel for TransfoModel by Class key
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelTransfoModel(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_TransfoClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_TransfoClassIDDescriptorMap.end())
        return ImagePPMessages::GetStringW(static_cast<ImagePPMessages::StringId>((*itr).second.GetStringID()));
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//GetLabel for TransfoModel by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelTransfoModel(const HGF2DTransfoModel& pi_rTransfoModel) const
    {
    return GetClassLabelTransfoModel(pi_rTransfoModel.GetClassID());
    }
//-----------------------------------------------------------------------------
//GetLabel for Filter by Class key
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelFilter(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_FilterClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_FilterClassIDDescriptorMap.end())
        return ImagePPMessages::GetStringW(static_cast<ImagePPMessages::StringId>((*itr).second.GetStringID()));
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//GetLabel for Filter by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelFilter(const HRPFilter& pi_rFilter) const
    {
    return GetClassLabelFilter(pi_rFilter.GetClassID());
    }

//-----------------------------------------------------------------------------
//GetLabel for Scan line Orientation by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelSLO(const HRFScanlineOrientation& pi_rScanlineO) const
    {
    WString Result;

    if (pi_rScanlineO == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_UpperLeftHorizontal());

    else if (pi_rScanlineO == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_UpperRightHorizontal());

    else if (pi_rScanlineO == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_LowerLeftHorizontal());

    else if (pi_rScanlineO == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_LowerRightHorizontal());

    else if (pi_rScanlineO == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_UpperLeftVertial());

    else if (pi_rScanlineO == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_UpperRightVertial());

    else if (pi_rScanlineO == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_LowerLeftVertial());

    else if (pi_rScanlineO == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ORIENTATION_LowerRightVertial());
    else

        return m_NotFound;


    return Result;
    }

//-----------------------------------------------------------------------------
//GetLabel for Block Type by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelBlockType(const HRFBlockType& pi_rBlockT) const
    {
    WString Result;

    if (pi_rBlockT == HRFBlockType::TILE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::BLOCKTYPE_Tile());

    else if (pi_rBlockT == HRFBlockType::STRIP)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::BLOCKTYPE_Strip());

    else if (pi_rBlockT == HRFBlockType::LINE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::BLOCKTYPE_Line());

    else if (pi_rBlockT == HRFBlockType::IMAGE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::BLOCKTYPE_Image());

    else

        return m_NotFound;

    return Result;
    }

//-----------------------------------------------------------------------------
//GetLabel for Encoding Type by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelEncodingType(const HRFEncodingType& pi_rEncodingT) const
    {
    WString Result;

    if (pi_rEncodingT == HRFEncodingType::STANDARD)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ENCODING_Standard());

    else if (pi_rEncodingT == HRFEncodingType::MULTIRESOLUTION)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ENCODING_MultiResolution());

    else if (pi_rEncodingT == HRFEncodingType::PROGRESSIVE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::ENCODING_Progressive());

    else

        return m_NotFound;

    return Result;
    }

//-----------------------------------------------------------------------------
//GetLabel for Resampling by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelResampling(const HRFDownSamplingMethod& pi_rSampling) const
    {
    WString Result;

    if (pi_rSampling == HRFDownSamplingMethod::NEAREST_NEIGHBOUR)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::RESAMPLING_NearestNeighbour());

    else if (pi_rSampling == HRFDownSamplingMethod::AVERAGE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::RESAMPLING_Average());

    else if (pi_rSampling == HRFDownSamplingMethod::VECTOR_AWARENESS)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::RESAMPLING_VectorAwareness());

    else if (pi_rSampling == HRFDownSamplingMethod::UNKOWN)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::RESAMPLING_Unknown());

    else if (pi_rSampling == HRFDownSamplingMethod::ORING4)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::RESAMPLING_Oring4());

    else if (pi_rSampling == HRFDownSamplingMethod::NONE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::RESAMPLING_None());
    else

        return m_NotFound;

    return Result;
    }

//-----------------------------------------------------------------------------
//GetLabel for Geo Reference by objet
//-----------------------------------------------------------------------------

WString HUTClassIDDescriptor::GetClassLabelGeoRef(const HRFGeoreferenceFormat& pi_rGeoRef) const
    {
    WString Result;

    if (pi_rGeoRef == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::GEOREFERENCE_InImage());

    else if (pi_rGeoRef == HRFGeoreferenceFormat::GEOREFERENCE_IN_HGR)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::GEOREFERENCE_HGR());

    else if (pi_rGeoRef == HRFGeoreferenceFormat::GEOREFERENCE_IN_WORLD_FILE)

        Result = ImagePPMessages::GetStringW(ImagePPMessages::GEOREFERENCE_WorldFile());

    else

        return m_NotFound;

    return Result;
    }

//-----------------------------------------------------------------------------
//Get Class Code for Codec
//-----------------------------------------------------------------------------

const WString& HUTClassIDDescriptor::GetClassCodeCodec(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_CodecClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_CodecClassIDDescriptorMap.end())
        return (*itr).second.GetClassCode();
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//Get Class Code for PixelType
//-----------------------------------------------------------------------------

const WString& HUTClassIDDescriptor::GetClassCodePixelType(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_PixelClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_PixelClassIDDescriptorMap.end())
        return (*itr).second.GetClassCode();
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//Get Class Code for TransfoModel
//-----------------------------------------------------------------------------

const WString& HUTClassIDDescriptor::GetClassCodeTransfoModel(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_TransfoClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_TransfoClassIDDescriptorMap.end())
        return (*itr).second.GetClassCode();
    else
        return m_NotFound;
    }
//-----------------------------------------------------------------------------
//Get Class Code for Filter
//-----------------------------------------------------------------------------

const WString& HUTClassIDDescriptor::GetClassCodeFilter(HCLASS_ID pi_ClassID) const
    {
    IDDescriptorMap::const_iterator itr = m_FilterClassIDDescriptorMap.find(pi_ClassID);

    if (itr != m_FilterClassIDDescriptorMap.end())
        return (*itr).second.GetClassCode();
    else
        return m_NotFound;
    }

//-----------------------------------------------------------------------------
//  Inline Get the Notfound string.
//-----------------------------------------------------------------------------

const WString& HUTClassIDDescriptor::GetNotfoundString() const
    {
    return m_NotFound;
    }
