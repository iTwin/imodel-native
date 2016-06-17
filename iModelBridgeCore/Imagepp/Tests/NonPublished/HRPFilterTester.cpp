//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/HRPFilterTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include "../imagepptestpch.h"

#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <ImagePP/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HRPFunctionFilters.h>
#include <ImagePP/all/h/HRPFilter.h>
#include <ImagePP/all/h/HRPDitherFilter.h>
#include <ImagePP/all/h/HRPOrFilter.h>

#include <ImagePP/all/h/HRPChannelType.h>
#include <ImagePP/all/h/HRPConvFilterV24R8G8B8.h>
#include <ImagePP/all/h/HRPLigthnessContrastStretch8.h>
#include <ImagePP/all/h/HRPLigthnessContrastStretch16.h>
#include <ImagePP/all/h/HRPLigthnessDensitySlicingFilter8.h>
#include <ImagePP/all/h/HRPLigthnessDensitySlicingFilter16.h>
#include <ImagePP/all/h/HRPDensitySlicingFilter8.h>
#include <ImagePP/all/h/HRPDensitySlicingFilter16.h>
#include <ImagePP/all/h/HRPMapFilters8.h>
#include <ImagePP/all/h/HRPMapFilters16.h>


typedef std::vector<HFCPtr<HRPPixelType>>           HRPPixelTypeVector;

static HFCPtr<HGF2DWorldCluster> s_pHMRWorld(new HGFHMRStdWorldCluster());

/*---------------------------------------------------------------------------------**//**
* Creator
*   All m_parameters used by child in base struct "Creator"
*   All members have to be initialized in "RunCreateFilterCopyFrom"
+---------------+---------------+---------------+---------------+---------------+------*/
struct Creator
    {
    virtual HFCPtr<HRPFilter> CreateFilter() = 0;

    Byte m_byteValue;
    };

/*---------------------------------------------------------------------------------**//**
* FilterCreator
+---------------+---------------+---------------+---------------+---------------+------*/
 template <typename Filter_T>
struct FilterCreator : Creator
    {
    HFCPtr<HRPFilter> CreateFilter() override
        {
        return new Filter_T();
        }
    };

/*---------------------------------------------------------------------------------**//**
* FilterCreatorByte
+---------------+---------------+---------------+---------------+---------------+------*/
 template <typename Filter_T>
struct FilterCreatorByte : Creator
    {
    HFCPtr<HRPFilter> CreateFilter() override
        {
        return new Filter_T(m_byteValue);
        }
    };

// creatorVectorNoArgs: all pixel type should comply with these constructors
std::vector<Creator*> creatorVectorNoArgs = 
    {   new FilterCreator<HRPAlphaComposer>(),
        new FilterCreator<HRPAlphaReplacer>(),
        new FilterCreator<HRPColortwistFilter>(),
        new FilterCreator<HRPComplexFilter>(),
        new FilterCreator<HRPContrastStretchFilter8>(),
        new FilterCreator<HRPDensitySlicingFilter8>(),
        new FilterCreator<HRPLigthnessContrastStretch8>(),
        new FilterCreator<HRPLigthnessDensitySlicingFilter8>(),
        new FilterCreator<HRPColorBalanceFilter>(),
        new FilterCreator<HRPContrastFilter>(),
        new FilterCreator<HRPHistogramScalingFilter>(),
        new FilterCreator<HRPGammaFilter>(),
        new FilterCreator<HRPInvertFilter>(),
        new FilterCreator<HRPTintFilter>(),
        new FilterCreator<HRPDensitySlicingFilter16>(),
        new FilterCreator<HRPLigthnessContrastStretch16>(),
        new FilterCreator<HRPLigthnessDensitySlicingFilter16>(),
        new FilterCreator<HRPCustomMap16Filter>(),
        new FilterCreator<HRPColorBalanceFilter16>(),
        new FilterCreator<HRPContrastFilter16>(),
        new FilterCreator<HRPHistogramScalingFilter16>(),
        new FilterCreator<HRPGammaFilter16>(),
        new FilterCreator<HRPInvertFilter16>(),
        new FilterCreator<HRPTintFilter16>(),
        new FilterCreator<HRPOrFilter>()
    };

// creatorVectorBytes: all pixel type should comply with these constructors
static std::vector<Creator*> creatorVectorBytes =
    {   new FilterCreatorByte<HRPAlphaComposer>(),
        new FilterCreatorByte<HRPAlphaReplacer>()
    };


/*=================================================================================**//**
* @bsiclass                         Alexandre.Gagnon                              11/2014           
+===============+===============+===============+===============+===============+======*/
class HRPFilterTester : public ::testing::TestWithParam< ::std::tr1::tuple<HFCPtr<HRPPixelType>, uint32_t, uint32_t> >
    {   
    protected:

    HRPFilterTester()
        {
        };

    virtual ~HRPFilterTester()
        {
        }

    virtual void SetUp() override
        {
        ImagePPTestConfig::GetConfig().SetUp();
        }

    HFCPtr<HRPPixelType> const& GetPixelType()  { return ::std::tr1::get<0>(GetParam()); }
    uint32_t const& GetWidth()                    { return ::std::tr1::get<1>(GetParam()); }
    uint32_t const& GetHeight()                   { return ::std::tr1::get<2>(GetParam()); }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRARaster> CreateRaster(HFCPtr<HRPPixelType> pPixelType, uint32_t width, uint32_t height)
        {
        //Creates a valid empty raster
        HFCPtr<HRABitmap> pRaster = HRABitmap::Create(width, height, NULL, s_pHMRWorld->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD),
                                                        pPixelType, pPixelType->CountPixelRawDataBits());
        return pRaster.GetPtr();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ImagePPStatus RunCreateFilterCopyFrom(HFCPtr<HRPPixelType> pPixelType, Creator* filterCreator, uint32_t width, uint32_t height, bool alphaBlend)
        {
        // Create source and sink rasters
        HFCPtr<HRARaster> pSourceRaster = CreateRaster(pPixelType, width, height);
        pSourceRaster->Clear();
        HFCPtr<HRARaster> pSinkRaster =   CreateRaster(pPixelType, width, height);
        pSinkRaster->Clear();

        // Initialize parameters HERE
        filterCreator->m_byteValue = 192;   //used by AlphaComposer and AlphaReplacer
        
        // Create filter
        HFCPtr<HRPFilter> pFilter = filterCreator->CreateFilter();

        HFCPtr<HIMFilteredImage> pFilteredRaster = new HIMFilteredImage(pSourceRaster, pFilter, pPixelType);

        HRACopyFromOptions copyFromOpts;
        copyFromOpts.SetAlphaBlend(alphaBlend);

        return pSinkRaster->CopyFrom(*pFilteredRaster, copyFromOpts);
        }

}; //END CLASS HRPFilterTester


/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gagnon                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRPFilterTester, CreateFilterCopyFrom)
    { 
    HFCPtr<HRPPixelType> pPixelType = GetPixelType();
    uint32_t width = GetWidth();
    uint32_t height = GetHeight();


    // Generic tests (any pixel type)
    for (size_t i = 0; i < creatorVectorNoArgs.size(); ++i)
        {
        ASSERT_EQ(IMAGEPP_STATUS_Success, RunCreateFilterCopyFrom(pPixelType, creatorVectorNoArgs[i], width, height, false));
        }

    // AlphaComposer and AlphaReplacer with byte argument
    for (size_t i = 0; i < creatorVectorBytes.size(); ++i)
        {
        ASSERT_EQ(IMAGEPP_STATUS_Success, RunCreateFilterCopyFrom(pPixelType, creatorVectorBytes[i], width, height, false));
        }

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gagnon                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static HRPPixelTypeVector s_GetPixelTypeVector ()
    {
    HRPPixelTypeVector pixelTypeVector;

    pixelTypeVector.push_back(new HRPPixelTypeI1R8G8B8());
    pixelTypeVector.push_back(new HRPPixelTypeI1R8G8B8A8());
    pixelTypeVector.push_back(new HRPPixelTypeV1Gray1());
    pixelTypeVector.push_back(new HRPPixelTypeV1GrayWhite1());

    pixelTypeVector.push_back(new HRPPixelTypeI8Gray8());
    pixelTypeVector.push_back(new HRPPixelTypeV8Gray8());
    pixelTypeVector.push_back(new HRPPixelTypeV8GrayWhite8());

    pixelTypeVector.push_back(new HRPPixelTypeV16Gray16());
    pixelTypeVector.push_back(new HRPPixelTypeV16Int16());
    pixelTypeVector.push_back(new HRPPixelTypeV16B5G5R5());
    pixelTypeVector.push_back(new HRPPixelTypeV16R5G6B5());

    pixelTypeVector.push_back(new HRPPixelTypeV24B8G8R8());
    pixelTypeVector.push_back(new HRPPixelTypeV24PhotoYCC());
    pixelTypeVector.push_back(new HRPPixelTypeV24R8G8B8());

    pixelTypeVector.push_back(new HRPPixelTypeV32PR8PG8PB8A8());
    pixelTypeVector.push_back(new HRPPixelTypeV32A8R8G8B8());
    pixelTypeVector.push_back(new HRPPixelTypeV32PRPhotoYCCA8());
    pixelTypeVector.push_back(new HRPPixelTypeV32R8G8B8A8());
    pixelTypeVector.push_back(new HRPPixelTypeV32B8G8R8X8());
    pixelTypeVector.push_back(new HRPPixelTypeV32R8G8B8X8());
    pixelTypeVector.push_back(new HRPPixelTypeV32CMYK());
    pixelTypeVector.push_back(new HRPPixelTypeV32Float32());

    pixelTypeVector.push_back(new HRPPixelTypeV48R16G16B16());
    pixelTypeVector.push_back(new HRPPixelTypeV64R16G16B16A16());
    pixelTypeVector.push_back(new HRPPixelTypeV64R16G16B16X16());
    pixelTypeVector.push_back(new HRPPixelTypeV96R32G32B32());

    return pixelTypeVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                            
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(HRPFilterTests,
                        HRPFilterTester,
                        ::testing::Combine(::testing::ValuesIn(s_GetPixelTypeVector()),
                                           ::testing::Values(static_cast<uint32_t>(11)),
                                           ::testing::Values(static_cast<uint32_t>(8))));

#else

#pragma message("Warining: Disabling HRPFilterTester because TEST_P/INSTANTIATE_TEST_CASE_P are not available")

#endif