//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageOpTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "../imagepptestpch.h"
#include "HRAImageOpTester.h"
#include <Logging\bentleylogging.h>

#define TEST_NAME ::testing::UnitTest::GetInstance()->current_test_info()->name()
#define TEST_CASE_NAME ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name()
#define TEST_NAME_PRINT L"TEST: (" << TEST_CASE_NAME << ", " << TEST_NAME << ")"


USING_NAMESPACE_BENTLEY_LOGGING
using namespace ::testing;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlphaComposerTestImageOpCreator : ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        return HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter(50/*defaultAlpha*/, VectorHRPAlphaRange());
        }

    virtual WCharCP GetOperationName() {return L"AlphaComposerFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct AlphaReplacerTestImageOpCreator : ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        return HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter(55/*defaultAlpha*/, VectorHRPAlphaRange());
        }

    virtual WCharCP GetOperationName() {return L"AlphaReplacerFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ColorReplacerTestImageOpCreator : ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        Byte newRGB[3] = {100, 200, 150};
        HGFRGBSet rgbSet;
        rgbSet.Add(12, 15, 20);
        HRAImageOpColorReplacerFilter::RGBSetList rgbSetList;
        rgbSetList.push_back(rgbSet);
        HRAImageOpColorReplacerFilter::RGBSetList emptySetList;

        return HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(newRGB, rgbSetList, emptySetList);
        }

    virtual WCharCP GetOperationName() {return L"ColorReplacerFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ColorTwistTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        double colortwistMatrix[4][4] = {.3333, .3333, .3333,    0,
                                              0,     1,     0,    0,
                                              0,     0,     1,    0,
                                              0,     0,     0,    1};
        return HRAImageOpColortwistFilter::CreateColortwistFilter(colortwistMatrix);
        }

    virtual WCharCP GetOperationName() {return L"ColortwistFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct DensitySlicingTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        HRAImageOpDensitySlicingFilterPtr pImgOp = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_8bits);
        pImgOp->AddSlice(0, 221, 16711680, 65280, 0.5);
        pImgOp->SetDesaturationFactor(0.5);

        return pImgOp;
        }

    virtual WCharCP GetOperationName() {return L"DensitySlicingFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LightnessDensitySlicingTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        HRAImageOpLightnessDensitySlicingFilterPtr pImgOp = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_8bits);
        pImgOp->AddSlice(0, 80, 16711680, 65280, 0.5);
        pImgOp->SetDesaturationFactor(0.5);

        return pImgOp;
        }

    virtual WCharCP GetOperationName() {return L"LightnessDensitySlicingFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LightnessContrastStretchTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        HRAImageOpLightnessContrastStretchFilterPtr pImgOp = HRAImageOpLightnessContrastStretchFilter::CreateLightnessContrastStretchFilter();
        pImgOp->SetInterval(20, 86);
        pImgOp->SetContrastInterval(0, 100);
        pImgOp->SetGammaFactor(1);

        return pImgOp;
        }

    virtual WCharCP GetOperationName() {return L"LightnessContrastStretchFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct TintTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        Byte rgbColor[3] = {50, 75, 250};
        return HRAImageOpTintFilter::CreateTintFilter(rgbColor);
        }

    virtual WCharCP GetOperationName() {return L"TintFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InvertTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override {return HRAImageOpInvertFilter::CreateInvertFilter();}
    virtual WCharCP GetOperationName() {return L"TintFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct GammaTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override {return HRAImageOpGammaFilter::CreateGammaFilter(1.5);}
    virtual WCharCP GetOperationName() {return L"GammaFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct BrightnessTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override {return HRAImageOpBrightnessFilter::CreateBrightnessFilter(0.5);}
    virtual WCharCP GetOperationName() {return L"BrightnessFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContrastTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override {return HRAImageOpContrastFilter::CreateContrastFilter(-0.5);}
    virtual WCharCP GetOperationName() {return L"ContrastFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContrastStretchTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        HRAImageOpContrastStretchFilterPtr pContrastStretchFilter = HRAImageOpContrastStretchFilter::CreateContrastStretchFilter(HRPPixelTypeV24R8G8B8());
        for (uint16_t i = 0; i < 3; i++)
            {
            pContrastStretchFilter->SetInterval(i, 10, 30);
            pContrastStretchFilter->SetContrastInterval(i, 10, 30);
            pContrastStretchFilter->SetGammaFactor(i, 1.1);
            }
        return pContrastStretchFilter;
        }

    virtual WCharCP GetOperationName() {return L"ContrastStretchFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PixelReplacerTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        Byte oldRgb[3]; oldRgb[0] = oldRgb[1] = oldRgb[2] = 0;      // black
        Byte newRgb[3]; newRgb[0] = newRgb[1] = 0; newRgb[2] = 255; // blue

        // PixelReplacer only support 8 bytes aligned pixeltype.
        HRPPixelTypeV24R8G8B8 v24RGB;
        if(srcPixeltype.CountPixelRawDataBits() % 8 != 0)
            {
            HRAImageOpPixelReplacerFilterPtr pPixelReplacerFilter = HRAImageOpPixelReplacerFilter::CreatePixelReplacer(v24RGB);
            pPixelReplacerFilter->SetValue(oldRgb, 3);
            pPixelReplacerFilter->SetNewValue(newRgb, 3);    
            return pPixelReplacerFilter;
            }

        HRAImageOpPixelReplacerFilterPtr pPixelReplacerFilter = HRAImageOpPixelReplacerFilter::CreatePixelReplacer(srcPixeltype);
        
        size_t bytesPerPixel = srcPixeltype.CountPixelRawDataBits() / 8;
        Byte* pOldValue = (Byte*)alloca(bytesPerPixel);
        Byte* pNewValue = (Byte*)alloca(bytesPerPixel);

        v24RGB.GetConverterTo(&srcPixeltype)->Convert(oldRgb, pOldValue);
        v24RGB.GetConverterTo(&srcPixeltype)->Convert(newRgb, pNewValue);
        
        pPixelReplacerFilter->SetValue(pOldValue, bytesPerPixel);
        pPixelReplacerFilter->SetNewValue(pNewValue, bytesPerPixel);
        return pPixelReplacerFilter;
        }

    virtual WCharCP GetOperationName() {return L"PixelReplacerFilter";}
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct DEMAspectTestImageOpCreator: ITestImageOpCreator
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) override
        {
        HRPDEMFilter::UpperRangeValues rangeValues;
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(15,  HRPDEMFilter::RangeInfo(  0,   0, 255,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(45,  HRPDEMFilter::RangeInfo(127,   0, 255,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(75,  HRPDEMFilter::RangeInfo(255,   0, 255,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(105, HRPDEMFilter::RangeInfo(255,   0, 127,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(135, HRPDEMFilter::RangeInfo(255,   0,   0,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(165, HRPDEMFilter::RangeInfo(255, 127,   0,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(195, HRPDEMFilter::RangeInfo(255, 255,   0,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(225, HRPDEMFilter::RangeInfo(127, 255,   0,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(255, HRPDEMFilter::RangeInfo(  0, 255,   0,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(285, HRPDEMFilter::RangeInfo(  0, 255, 127,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(315, HRPDEMFilter::RangeInfo(  0, 255, 255,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(345, HRPDEMFilter::RangeInfo(  0, 127, 255,true)));
        rangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(360, HRPDEMFilter::RangeInfo(  0, 0,   255,true)));

        return HRAImageOpDEMFilter::CreateDEMFilter(HRPDEMFilter::Style_Aspect, rangeValues, 100.0, 100.0, HGF2DIdentity());
        }

    virtual WCharCP GetOperationName() {return L"DEMAspectFilter";}
    };


// *** Convolution: 2x2 matrix with origin at 0,0.
// NOT_NOW ImageOpTester cannot handle convolution filters.
// imageOpVector.push_back(HRAImageOpPtrVector::value_type(HRAImageOpConvolutionFilter::CreateAverageFilter(), L"AverageFilter"));
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static TestImageOpCreatorVector s_GetImageOpTestCreatorVector()
    {
    TestImageOpCreatorVector creators;

#define ADD_IMG_OP_CREATOR(name) \
    static name s_##name; \
    creators.push_back(&s_##name);

    ADD_IMG_OP_CREATOR(AlphaComposerTestImageOpCreator);
    ADD_IMG_OP_CREATOR(AlphaReplacerTestImageOpCreator);
    ADD_IMG_OP_CREATOR(ColorReplacerTestImageOpCreator);
    ADD_IMG_OP_CREATOR(ColorTwistTestImageOpCreator);
    ADD_IMG_OP_CREATOR(DensitySlicingTestImageOpCreator);
    ADD_IMG_OP_CREATOR(LightnessDensitySlicingTestImageOpCreator);
    ADD_IMG_OP_CREATOR(LightnessContrastStretchTestImageOpCreator);
    ADD_IMG_OP_CREATOR(TintTestImageOpCreator);
    ADD_IMG_OP_CREATOR(InvertTestImageOpCreator);
    ADD_IMG_OP_CREATOR(GammaTestImageOpCreator);
    ADD_IMG_OP_CREATOR(BrightnessTestImageOpCreator);
    ADD_IMG_OP_CREATOR(ContrastTestImageOpCreator);
    ADD_IMG_OP_CREATOR(ContrastStretchTestImageOpCreator);
    ADD_IMG_OP_CREATOR(PixelReplacerTestImageOpCreator);
    // Cannot handle imageOp with neighbourhood. ADD_IMG_OP_CREATOR(DEMAspectTestImageOpCreator);

#undef ADD_IMG_OP_CREATOR

    return creators;
    }


/*---------------------------------------------------------------------------------**//**
* Return a vector with all pixel types to test. 
* NOTE: this method can't be included in HRAImageOpCombinePixelTypes class, because 
* INSTANTIATE_TEST_CASE_P will not compile.
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static HRPPixelTypeVector s_GetPixelTypeVector ()
    {
    HRPPixelTypeVector pixelTypeVector;

    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV8Gray8(),           L"V8Gray8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV8GrayWhite8(),      L"V8GrayWhite8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16PRGray8A8(),      L"V16PRGray8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16R5G6B5(),         L"V16R5G6B5"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV24PhotoYCC(),       L"V24PhotoYCC"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32PRPhotoYCCA8(),   L"V32PRPhotoYCCA8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32CMYK(),           L"V32CMYK"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV24R8G8B8(),         L"V24R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV24B8G8R8(),         L"V24B8G8R8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32B8G8R8X8(),       L"V32B8G8R8X8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32R8G8B8A8(),       L"V32R8G8B8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32R8G8B8X8(),       L"V32R8G8B8X8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32PR8PG8PB8A8(),    L"V32PR8PG8PB8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32A8R8G8B8(),       L"V32A8R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16Gray16(),         L"V16Gray16"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16Int16(),          L"V16Int16"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32Float32(),        L"V32Float32"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV48R16G16B16(),      L"V48R16G16B16"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV64R16G16B16A16(),   L"V64R16G16B16A16"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV64R16G16B16X16(),   L"V64R16G16B16X16"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV96R32G32B32(),      L"V96R32G32B32"));

#if 0 //TODO the list that we used to treat as invalid
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV1Gray1(), L"V1Gray1"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV1GrayWhite1(), L"V1GrayWhite1"));

    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8(), L"I1R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8RLE(), L"I1R8G8B8RLE"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8(), L"I1R8G8B8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8RLE(), L"I1R8G8B8A8RLE"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI2R8G8B8(), L"I2R8G8B8"));

    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI4R8G8B8(), L"I4R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI4R8G8B8A8(), L"I4R8G8B8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8(), L"I8R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8A8(), L"I8R8G8B8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8Mask(), L"I8R8G8B8Mask"));

    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8Gray8(), L"I8Gray8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8VA8R8G8B8(), L"I8VA8R8G8B8"));
#endif

    return pixelTypeVector;
    }


/*---------------------------------------------------------------------------------**//**
* Value Parameterized test that validates the good working of all filters with all
* acceptable combinations of input/output pixel types. This test is instantiated by
* INSTANTIATE_TEST_CASE_P statements.
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRAImageOpCombinePixelTypes, TestFilterVsPixelTypes)
    {
    ITestImageOpCreator& filterCreator = GetFilterCreator();
    
    HFCPtr<HRPPixelType> pixelTypeIn = GetInputPixelType().first;
    WString pixelTypeInName = GetInputPixelType().second;
    HFCPtr<HRPPixelType> pixelTypeOut = GetOutputPixelType().first;
    WString pixelTypeOutName = GetOutputPixelType().second;

    // Extract test parameters (filter and pixel types)
    HRAImageOpPtr pImageOp = filterCreator.Create(*pixelTypeIn);
   
    //TODO: The code below cannot handle imageop with neighbourhood.
    ASSERT_TRUE(pImageOp->GetNeighbourhood().IsUnity());

    // Clean output/input pixel types, which may have been set by previous test
    ASSERT_EQ(IMAGEPP_STATUS_Success, pImageOp->SetInputPixelType(NULL));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pImageOp->SetOutputPixelType(NULL));

    // Check if this pixel types combination is valid for the filter
    if ((pImageOp->SetInputPixelType(pixelTypeIn) != IMAGEPP_STATUS_Success) ||
        pImageOp->SetOutputPixelType(pixelTypeOut) != IMAGEPP_STATUS_Success)
        {
        if (pImageOp->SetInputPixelType(pixelTypeIn) == IMAGEPP_STATUS_Success)
            {
            // Integrity test:
            // Check if the output pixel type is among the available output. If it's the case, it's an error, because
            // we tried to set the output pixel type with it and it failed.
            ImagePPStatus status = IMAGEPP_STATUS_Success;
            HFCPtr<HRPPixelType> availablePixelType;
            uint32_t pixelTypeIndex = 0;
            while (status != IMAGEOP_STATUS_NoMorePixelType)
                {
                status = pImageOp->GetAvailableOutputPixelType(availablePixelType, pixelTypeIndex, pixelTypeIn);
                if (status == IMAGEPP_STATUS_Success && availablePixelType->GetClassID() == pixelTypeOut->GetClassID())
                    {
                    WChar err[1024];
                    BeStringUtilities::Snwprintf (err, L"Could not set output to pixel type %ls, but it is among the available ones. Input pixel type: %ls, Filter:%ls", 
                                                       pixelTypeOutName, pixelTypeInName, filterCreator.GetOperationName());
                    FAIL() << err;
                    }
                pixelTypeIndex++;
                }
            }
        // Can't use this pixel type combination on the filter. Let's return.
        return;
        }

    // Log information about filter being tested
    ILogger* pLogger = LoggingManager::GetLogger ( L"TestFilterVsPixelTypes" );
    pLogger->infov(L"In TestFilterVsPixelTypes - Testing filter: %ls, pixel types in/out: %ls/%ls", filterCreator.GetOperationName(), pixelTypeInName, pixelTypeOutName);

    // Create input and output images
    HRAImageSamplePtr pImageIn;
    CreateImage(pImageIn, pixelTypeIn);
    HRAImageSamplePtr pImageOut;
    CreateImage(pImageOut, pixelTypeOut);

    HGF2DIdentity identity;
    ImagepOpParams imageOpParams(identity);
    imageOpParams.SetOffset(0, 0);

    // Filter pImageIn
    clock_t processStartTime = clock();  
    pImageOp->Process(*pImageOut, *pImageIn, imageOpParams);
    clock_t processEndTime = clock();  

    // Verify that the filtering process does not cause buffer overflow. 
    WString errorMsg;
    if (!BufferAllocator::VerifyBuffer(pImageOut, pixelTypeOut, errorMsg))
        {
        WChar err[1024];
        BeStringUtilities::Snwprintf (err, L"%ls Filter:%ls, input pixel type: %ls, output pixel type: %ls", 
                                      errorMsg, filterCreator.GetOperationName(), pixelTypeInName, pixelTypeOutName);
        FAIL() << err;
        }

    // Log filtering time and elapsed time since the beginning of filters testing
    clock_t clockTime = clock();  
    double elapsedClockTime = (double)(clockTime - s_startTime) / CLOCKS_PER_SEC;                  
    double processTime = (double)(processEndTime - processStartTime) / CLOCKS_PER_SEC;                  
    pLogger->infov(L"    Filtering time: %.4f - Elapsed time since beginning: %.4f", processTime, elapsedClockTime);
    }


/*---------------------------------------------------------------------------------**//**
* Instantiate HRAImageOpCombinePixelTypes parameterized tests ("TEST_P") so that all combinations of (ImageOp,
* PixelType, PixelType) will be tested.
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(HRAImageOpCombinePixelTypes_CombineFilters,
                        HRAImageOpCombinePixelTypes,
                        Combine(ValuesIn(s_GetImageOpTestCreatorVector ()), ValuesIn(s_GetPixelTypeVector ()), ValuesIn(s_GetPixelTypeVector ())));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpCombinePixelTypes::HRAImageOpCombinePixelTypes()
    {
    // Configure logging
    LoggingConfig::SetSeverity(L"TestFilterVsPixelTypes", LOG_INFO);

    // Specifiy the ouput file 
    LoggingConfig::SetOption ( CONFIG_OPTION_OUTPUT_FILE, L"HRAImageOpTester.log");

    // Activate the logging provider with the specified options
    LoggingConfig::ActivateProvider  ( SIMPLEFILE_LOGGING_PROVIDER );

    // Start timing for performance check
    HRAImageOpCombinePixelTypes::StartTiming();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpCombinePixelTypes::StartTiming()
    {
    // Start timing. Just start once for all tests; don't restart at every test.
    if (!m_timingIsStarted)
        {
        s_startTime = clock();
        m_timingIsStarted = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Creation of a test image
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpCombinePixelTypes::CreateImage(HRAImageSamplePtr& prImageSample, HFCPtr<HRPPixelType>& prPixelType)
    {
    uint32_t width, height;

    // Created images are always the same size
    m_bufferAllocator.GetImageSize(width, height);
    uint32_t pixelSize = (prPixelType->CountPixelRawDataBits() +7) /8;

    // Use pitch provided by the allocator, because we want to control how the data is written in the buffer, and thus verify it at the end.
    uint32_t pitch = m_bufferAllocator.GetPitch();

    // Create buffer for image. Notice that our special allocator, m_bufferAllocator, is used here.
    ImagePPStatus status;
    HRAImageBufferPtr pNewBuffer = HRAImageBufferMemory::CreateMemoryBuffer(status, width*height*pixelSize, pitch, m_bufferAllocator);
    prImageSample = HRAImageSample::internal_CreateSampleFromBuffer(status, width, height, prPixelType, *pNewBuffer);
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Fill image buffer: row 0: all 0s; row 1: all 1s; ...
    size_t imagePitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(imagePitch);
    ASSERT_EQ(imagePitch, m_bufferAllocator.GetPitch()) << "Pitch of m_bufferAllocator is not the same as created Image.";
    Byte* pBufferLine = pBuffer;
    uint32_t lineSize = pixelSize * width;
    for(uint32_t idxRow = 0; idxRow < height; ++idxRow)
        {
        memset (pBufferLine, idxRow % 256, lineSize);
        pBufferLine += pitch;   // jump to next line
        }
    }

/*=================================================================================**//**
* BufferAllocator
* @bsiclass                                                     Eric.Paquet  11/2012
+===============+===============+===============+===============+===============+======*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void BufferAllocator::GetImageSize(uint32_t &width, uint32_t &height)
    {
    width = m_imageWidth;
    height = m_imageHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t BufferAllocator::GetPitch()
    {
    return m_pitch;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Byte* BufferAllocator::_AllocMemory(size_t size)
        {   
        HPRECONDITION(size <= m_imageWidth * m_imageHeight * m_maxBytesPixelSize);    // Allow a max allocation size because the test buffer has a fixed size.

        // We keep a padding around the image to test for buffer overflow:
        //
        // <------------------------------------------- m_pitch --------------------------------------------->
        // <-m_paddingBytes-><------------ m_imageWidth * m_maxBytesPixelSize -------------><-m_paddingBytes->
        // 
        // |--------------------------------------------------------------------------------------------------|
        // |  |  |   ...     _______________________________________________________________     ...    |  |  |   -> padding line _____
        // |  |  |   ...     |  |  |                    ...                          |  |  |     ...    |  |  |                     ^
        // |  |  |   ...     |  |  |                    ...                          |  |  |     ...    |  |  |                     |
        // |  |  |   ...     |  |  |                    ...                          |  |  |     ...    |  |  |                     |
        // |                                            ...                                                   |               m_imageHeight  
        // |  |  |   ...     |  |  |                    ...                          |  |  |     ...    |  |  |                     |
        // |  |  |   ...     |  |  |                    ...                          |  |  |     ...    |  |  |                     |
        // |  |  |           |__|__|_________________________________________________|__|__|     ...    |  |  |                   __v__
        // |  |  |   ...                                                                         ...    |  |  |   -> padding line     
        // |--------------------------------------------------------------------------------------------------|
        //

        uint32_t sizeInternal = m_pitch * (m_imageHeight + 2); // There are two padding lines
        Byte* pBuffer = new Byte[sizeInternal];

        // Set the whole buffer with a dummy value. This is to verify that these values are still intact in the padding areas,
        // where the image should not be written.
        memset (pBuffer, PADDING_BYTE, sizeInternal);

        // The returned buffer points to where the image must be written (skip one line and add padding)
        // (m_nbFirstPaddingBytes = m_pitch + m_paddingBytes)
        return pBuffer + m_nbFirstPaddingBytes;
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void BufferAllocator::_FreeMemory(Byte* pi_MemPtr)
    {
    Byte* pBuffer = pi_MemPtr - m_nbFirstPaddingBytes;
    delete[] pBuffer;
    }

/*---------------------------------------------------------------------------------**//**
* Check that the padding around the allocated buffer is intact.
* @bsimethod                                                    Eric.Paquet     11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool BufferAllocator::VerifyBuffer(HRAImageSamplePtr& prImageSample, HFCPtr<HRPPixelType>& prPixelType, WStringR errorMsg)
    {
    WChar error[1024];
    uint32_t pixelSize = (prPixelType->CountPixelRawDataBits() +7) /8;
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);
    if (pitch != m_pitch)
        {
        errorMsg = L"In VerifyBuffer - Pitch of m_bufferAllocator is not the same as image pitch.";
        return false;
        }

    // check first line
    WString testCaseName(TEST_CASE_NAME, false);
    WString testName(TEST_NAME, false);
    Byte* pInternalBuffer = pBuffer - m_nbFirstPaddingBytes;
    for (uint32_t i = 0; i < m_pitch; i++)
        {
        if (PADDING_BYTE != pInternalBuffer[i])
            {
            BeStringUtilities::Snwprintf (error, L"TEST: (%ls, %ls) - In VerifyBuffer: pInternalBuffer[%d] == %d; it should be equal to %d.", 
                                            testCaseName, testName, i, (Byte) pInternalBuffer[i], PADDING_BYTE);
            errorMsg = WString (error);
            return false;
            }
        }

    // check padding at the beginning and end of other lines
    Byte* pLineBuffer = pInternalBuffer + m_pitch;
    for (uint32_t idxLine = 0; idxLine < m_imageHeight; idxLine++)
        {
        // check padding at the beginning of the line
        for (uint32_t i = 0; i < m_paddingBytes; i++)
            {
            if (PADDING_BYTE != pLineBuffer[i])
                {
                BeStringUtilities::Snwprintf (error, L"TEST: (%ls, %ls) - In VerifyBuffer: idxLine == %d, pLineBuffer[%d] == %d; it should be equal to %d.", 
                                              testCaseName, testName, idxLine, i, (Byte) pLineBuffer[i], PADDING_BYTE);
                errorMsg = WString (error);
                return false;
                }
            }

        // check padding at the end of the line
        Byte* pEndLinePadding = pLineBuffer + m_paddingBytes + pixelSize * m_imageWidth;
        uint32_t nbBytesToCheck = m_pitch - (m_paddingBytes + pixelSize * m_imageWidth);
        for (uint32_t i = 0; i < nbBytesToCheck; i++)
            {
            if (PADDING_BYTE != pEndLinePadding[i])
                {
                BeStringUtilities::Snwprintf (error, L"TEST: (%ls, %ls) - In VerifyBuffer: idxLine == %d, pEndLinePadding[%d] == %d; it should be equal to %d.", 
                                              testCaseName, testName, idxLine, i, (Byte) pEndLinePadding[i], PADDING_BYTE);
                errorMsg = WString (error);
                return false;
                }
            }

        pLineBuffer += m_pitch;
        }

    // check last line, which must contain all padding. (pLineBuffer now points to last line).
    for (uint32_t i = 0; i < m_pitch; i++)
        {
        if (PADDING_BYTE != pLineBuffer[i])
            {
            BeStringUtilities::Snwprintf (error, L"TEST: (%ls, %ls) - In VerifyBuffer: pLineBuffer[%d] == %d; it should be equal to %d.", 
                                            testCaseName, testName, i, (Byte) pLineBuffer[i], PADDING_BYTE);
            errorMsg = WString (error);
            return false;
            }
        }

    return true;
    }

#define USE_COPYFROM2 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConvolutionMatrixTestParam
    {
    ConvolutionMatrixTestParam(uint32_t width, uint32_t height, uint32_t originX, uint32_t originY)
        :m_width(width),
         m_height(height),
         m_originX(originX),
         m_originY(originY)
        {
        }

    ~ConvolutionMatrixTestParam(){};

    HFCPtr<HRPFilter> CreateConvolutionFilter() const
        {
        HPRECONDITION(m_originX < m_width);
        HPRECONDITION(m_originY < m_height);

        bvector<int32_t> matrix(m_width*m_height, 0);
        matrix[m_originY*m_width + m_originX] = 1;        

        return new HRPCustomConvFilter(m_width, m_height, m_originX, m_originY, &matrix[0]);
        }
    
    private:
        HFCPtr<HRPFilter> pFilter;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_originX;
        uint32_t m_originY;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
class ImageOpTest : public ::testing::TestWithParam<ConvolutionMatrixTestParam>
{
public:
    ImageOpTest()
        :m_width(10),
         m_height(9)
        {
        HASSERT(m_width*m_height < 256);        // We want a unique value for each pixel.
        
        HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
        HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeV8Gray8();
    
        m_srcBitmap = HRABitmap::Create(m_width, m_height, NULL, pCoordSys, pPixelType);

        // Fill source bitmap with a unique value for each pixels.
        Byte* pBuffer = m_srcBitmap->GetPacket()->GetBufferAddress();
        for(uint32_t line=0; line < m_height; ++line)
            for(uint32_t column=0; column < m_width; ++column)
                pBuffer[line*m_width + column] = (Byte)(line*m_width + column) + 1;
        }

    HFCPtr<HRABitmap> GetSource() {return m_srcBitmap;}

    bool ValidateResult(uint32_t& errPosX, uint32_t& errPosY, uint32_t& expectedVal, uint32_t& actualVal, HRABitmap const& bitmap) const
        {
        Byte const* pBaseBuffer = m_srcBitmap->GetPacket()->GetBufferAddress();
        Byte const* pBuffer = bitmap.GetPacket()->GetBufferAddress();
        for(uint32_t line=0; line < m_height; ++line)
            {          
            for(uint32_t column=0; column < m_width; ++column)
                {
#ifdef USE_COPYFROM2
                if(pBaseBuffer[line*m_width + column] != pBuffer[line*m_width + column])
#else
                // Old copy from use RGB and conversion to Gray introduce a slight diff.ex 43,43,43 gives 42.
                if(abs(pBaseBuffer[line*m_width + column] - pBuffer[line*m_width + column]) > 1)
#endif
                    {
                    expectedVal = pBaseBuffer[line*m_width + column];
                    actualVal = pBuffer[line*m_width + column];
                    errPosX = column;
                    errPosY = line;
                    return false;
                    }
                }
            }
        return true;
        }

    private:
        uint32_t m_width;
        uint32_t m_height;
        HFCPtr<HRABitmap> m_srcBitmap;
};


//==================================================================================
// ConvolutionAlignmentTest
// The purpose of this test is to validate pixel alignment when filtering with a neighbourhood.
//==================================================================================
TEST_P(ImageOpTest, ConvolutionAlignmentTest)
    {    
    HFCPtr<HRABitmap> pSrcBitmap = GetSource();
    
    HFCPtr<HRPFilter> pConvFilter = GetParam().CreateConvolutionFilter();

    HFCPtr<HRARaster> pFilteredRaster = new HIMFilteredImage(pSrcBitmap.GetPtr(), pConvFilter);

    HFCPtr<HRABitmap> pDestBitmap = (HRABitmap*)pSrcBitmap->Clone();
    pDestBitmap->Clear();

#ifdef USE_COPYFROM2
    HRACopyFromOptions opts;
    pDestBitmap->CopyFrom(*pFilteredRaster, opts);
#else
    HRACopyFromLegacyOptions opts;
    pDestBitmap->CopyFromLegacy(pFilteredRaster.GetPtr(), opts);
#endif

    uint32_t errPosX, errPosY;
    uint32_t expectedValue, actualValue;
    if(!ValidateResult(errPosX, errPosY, expectedValue, actualValue, *pDestBitmap))
        ASSERT_EQ(expectedValue, actualValue) << L"Difference at position(" << errPosX << L"," << errPosY << L")";
    }

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
typedef std::vector<std::list<HRAImageOpPtr>>  TestImageOpPipelineVector;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static HRPPixelTypeVector const& s_GetPipelineTestPixelTypeVector ()
    {
    static HRPPixelTypeVector pixelTypeVector;

    if(pixelTypeVector.empty())
        {
        //*** I removed some pixel type to reduce the amount of test cases.***

        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV8Gray8(),           L"V8Gray8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV8GrayWhite8(),      L"V8GrayWhite8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16PRGray8A8(),      L"V16PRGray8A8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV24PhotoYCC(),       L"V24PhotoYCC"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32PRPhotoYCCA8(),   L"V32PRPhotoYCCA8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32CMYK(),           L"V32CMYK"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV24R8G8B8(),         L"V24R8G8B8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV24B8G8R8(),         L"V24B8G8R8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32B8G8R8X8(),       L"V32B8G8R8X8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32R8G8B8A8(),       L"V32R8G8B8A8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32R8G8B8X8(),       L"V32R8G8B8X8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32PR8PG8PB8A8(),    L"V32PR8PG8PB8A8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32A8R8G8B8(),       L"V32A8R8G8B8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16Gray16(),         L"V16Gray16"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16Int16(),          L"V16Int16"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV32Float32(),        L"V32Float32"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV48R16G16B16(),      L"V48R16G16B16"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV64R16G16B16A16(),   L"V64R16G16B16A16"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV64R16G16B16X16(),   L"V64R16G16B16X16"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV96R32G32B32(),      L"V96R32G32B32"));

        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV1Gray1(), L"V1Gray1"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV1GrayWhite1(), L"V1GrayWhite1"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8(), L"I1R8G8B8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8(), L"I1R8G8B8A8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8(), L"I8R8G8B8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8A8(), L"I8R8G8B8A8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8Gray8(), L"I8Gray8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8VA8R8G8B8(), L"I8VA8R8G8B8"));
        //pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8RLE(), L"I1R8G8B8RLE"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8RLE(), L"I1R8G8B8A8RLE"));

    
        // These are not expected in the pipeline.
    #if 0
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI2R8G8B8(), L"I2R8G8B8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI4R8G8B8(), L"I4R8G8B8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI4R8G8B8A8(), L"I4R8G8B8A8"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8Mask(), L"I8R8G8B8Mask"));
        pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16R5G6B5(), L"V16R5G6B5"));
    #endif
        }

    return pixelTypeVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static TestImageOpPipelineVector const& s_GetImageOpPipelineTestVector()
    {
    static TestImageOpPipelineVector imageOps;

    if(!imageOps.empty())
        return imageOps;

    HRAImageOpPtr pDemFilter = HRAImageOpDEMFilter::CreateDEMFilter(HRPDEMFilter::Style_Elevation, HRPDEMFilter::UpperRangeValues(), 100.0, 100.0, HGF2DIdentity());

    HRAImageOpPtr pContrastStretchFilter = HRAImageOpLightnessContrastStretchFilter::CreateLightnessContrastStretchFilter();

    HRAImageOpPtr pBlur = HRAImageOpConvolutionFilter::CreateBlurFilter(127);

    HRAImageOpPtr pSharpen = HRAImageOpConvolutionFilter::CreateSharpenFilter(127);

    HRAImageOpPtr pDensitySlicing8 = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PIXELDEPTH_8bits);
        
    HRAImageOpPtr pDensitySlicing16 = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PIXELDEPTH_16bits);


    double colortwistMatrix[4][4] = {.3333, .3333, .3333,    0,
                                         0,     1,     0,    0,
                                         0,     0,     1,    0,
                                         0,     0,     0,    1};
    HRAImageOpPtr pColorTwist = HRAImageOpColortwistFilter::CreateColortwistFilter(colortwistMatrix);

    HRAImageOpPtr pPixelReplacer = HRAImageOpPixelReplacerFilter::CreatePixelReplacer(HRPPixelTypeV8GrayWhite8());

    Byte newRGB[3] = {100, 200, 150};
    HGFRGBSet rgbSet;
    bvector<HGFRGBSet> rgbSetList;
    rgbSetList.push_back(rgbSet);
    bvector<HGFRGBSet> emptySetList;
    HRAImageOpPtr pColorReplacer = HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(newRGB, HRAImageOpColorReplacerFilter::RGBSetList(), HRAImageOpColorReplacerFilter::RGBSetList());

    HRAImageOpPtr pContrastStretch = HRAImageOpContrastStretchFilter::CreateContrastStretchFilter(HRPPixelTypeV24B8G8R8());

    HRAImageOpPtr pTint = HRAImageOpTintFilter::CreateTintFilter(newRGB);

    HRAImageOpPtr pGama = HRAImageOpGammaFilter::CreateGammaFilter(0.5);

    HRAImageOpPtr pBright = HRAImageOpBrightnessFilter::CreateBrightnessFilter(0.5);
    HRAImageOpPtr pContrast = HRAImageOpContrastFilter::CreateContrastFilter(0.5);

    //-------------------------------
    
    // DEM doesn't support everything and thus failed with some pixel type. imageOps.push_back ( { pDemFilter });
    imageOps.push_back ( { pContrastStretchFilter });
    imageOps.push_back ( { pBlur });
    imageOps.push_back ( { pDensitySlicing8 });
    imageOps.push_back ( { pDensitySlicing16 });
    imageOps.push_back ( { pPixelReplacer });
    imageOps.push_back ( { pColorReplacer });
    imageOps.push_back ( { pContrastStretch });
    imageOps.push_back ( { pTint });
    imageOps.push_back ( { pGama });
    
    // Some random combinations. 
    // ***** Do not add too much since the number of combined case is exponential. **** 
    // imageOps.push_back ( { pColorTwist });
//     imageOps.push_back ( { pDensitySlicing8, pContrastStretch });
//     imageOps.push_back ( { pContrastStretchFilter, pDensitySlicing16 });
//     imageOps.push_back ( { pColorTwist, pDensitySlicing16 });
//     imageOps.push_back ( { pPixelReplacer, pDensitySlicing16 });
    imageOps.push_back ( { pContrastStretch, pDensitySlicing16 });
    imageOps.push_back ( { pTint, pGama });
//     imageOps.push_back ( { pBlur, pSharpen, pGama });
     imageOps.push_back ( { pPixelReplacer, pDensitySlicing16, pBlur, pColorReplacer});
    imageOps.push_back ( { pGama, pBright , pContrast, pTint, pDensitySlicing8});
    //imageOps.push_back ( { pGama, pContrast, pDensitySlicing8, pBlur});
    imageOps.push_back ( { pGama, pContrast});
    imageOps.push_back ( { pContrast, pTint});
    //imageOps.push_back ( { pTint, pDensitySlicing8});
    //imageOps.push_back ( { pDensitySlicing8, pGama});
    
    return imageOps;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
class ImageOpPipelineTest : public ::testing::TestWithParam< ::std::tr1::tuple<std::list<HRAImageOpPtr>, HRPPixelTypeNamePair, HRPPixelTypeNamePair> >
{
public:
    ImageOpPipelineTest(){}

    ~ImageOpPipelineTest(){};

    std::list<HRAImageOpPtr> const& GetOpList() {return ::std::tr1::get<0>(GetParam()) ;}

    HRPPixelTypeNamePair const& GetInputPixelType() {return ::std::tr1::get<1>(GetParam());}
    
    HRPPixelTypeNamePair const& GetOutputPixelType() {return ::std::tr1::get<2>(GetParam());}

private:
};

INSTANTIATE_TEST_CASE_P (ImageOpConnectionPipelineTest, ImageOpPipelineTest, 
                         Combine(ValuesIn(s_GetImageOpPipelineTestVector ()), ValuesIn(s_GetPipelineTestPixelTypeVector ()), ValuesIn(s_GetPipelineTestPixelTypeVector ())));

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ImageOpPipelineTest, PixelTypeConnection)
    {    
    HFCPtr<HRPPixelType> pIn = GetInputPixelType().first;
    HFCPtr<HRPPixelType> pOut = GetOutputPixelType().first;

    std::list<HRAImageOpPtr> const& opList = GetOpList();
    HRAImageOpPipeLine pipe;
    for(auto opItr : opList)
        {
        // Clean output/input pixel types, which may have been set by previous test
        ASSERT_EQ(IMAGEPP_STATUS_Success, opItr->SetInputPixelType(NULL));
        ASSERT_EQ(IMAGEPP_STATUS_Success, opItr->SetOutputPixelType(NULL));

        pipe.AddImageOp(opItr, false);
        }

    ASSERT_EQ(IMAGEPP_STATUS_Success, pipe.Prepare(pIn, pOut));

    // Connection validation
    HFCPtr<HRPPixelType> pPipeIn = pIn;
    for (auto opItr : pipe.GetImageOps())
        {
        ASSERT_TRUE(NULL != opItr->GetInputPixelType());
        ASSERT_TRUE(NULL != opItr->GetOutputPixelType());
        ASSERT_TRUE(opItr->GetInputPixelType()->HasSamePixelInterpretation(*pPipeIn));

        pPipeIn = opItr->GetOutputPixelType();
        }
    }

