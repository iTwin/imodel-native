//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageOpFunctionFiltersTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include <ImagePP\all\h\HRAImageOpFunctionFilters.h>

//-----------------------------------------------------------------------------
// Class : HRAImageOpFunctionFiltersTester
//-----------------------------------------------------------------------------

// Preparation of required environment
class HRAImageOpFunctionFiltersTester : public testing::Test
    {
    protected:
        HRAImageOpFunctionFiltersTester();
        ~HRAImageOpFunctionFiltersTester();

        void TestFiltering(HRAImageOpPtr imageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected);
        void TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        void TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        static const ::testing::TestInfo* TestInfo();

        void CreateImage_1(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType, Byte alphaValue = 0);
        void CreateImage_2_AlphaWithRanges(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType, Byte alphaValue, const ImagePP::VectorHRPAlphaRange& pi_rRanges);
        void CreateImage_3_RGBA_Specified(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType, Byte redValue, Byte greenValue, Byte blueValue, Byte alphaValue);
        void Create_AlphaRangeList_1(ImagePP::VectorHRPAlphaRange& listAlphaRange, Byte alphaValue);
        void Create_AlphaRangeList_2(ImagePP::VectorHRPAlphaRange& listAlphaRange);

        void CreateImage_ColorReplacer(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType);
        void CreateImageExpected_ColorReplacer(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType);

        IImageAllocator& GetMemoryManager();

    private:
        ImageAllocatorPool m_allocatorPool;
        ImageAllocatorRefPtr m_allocator;
    };


//==================================================================================
// Test Colortwist filter
//==================================================================================
TEST_F(HRAImageOpFunctionFiltersTester, ColortwistFilterTest)
    {
    double pColortwistMatrix[4][4] = {.3333, .3333, .3333,    0,
                                          0,     1,     0,    0,
                                          0,     0,     1,    0,
                                          0,     0,     0,    1};
    HRAImageOpPtr pNewOp = HRAImageOpColortwistFilter::CreateColortwistFilter(pColortwistMatrix);

    // Test available input with no output set
    //Not Compatible
    //TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, IMAGEPP_STATUS_Success);

    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24PhotoYCC::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24PhotoYCC::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);

    // Test available output with no input set
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV24PhotoYCC::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, 0, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24PhotoYCC())); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEPP_STATUS_Success,           pNewOp->SetOutputPixelType(new HRPPixelTypeV24PhotoYCC()));

    // Output is set to HRPPixelTypeV24PhotoYCC; available input should be also HRPPixelTypeV24PhotoYCC (even if index varies)
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24PhotoYCC::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24PhotoYCC::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Recreate filter
    pNewOp = HRAImageOpColortwistFilter::CreateColortwistFilter(pColortwistMatrix);
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Set input and output pixel types
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24PhotoYCC()));
    ASSERT_TRUE(pNewOp->GetInputPixelType()->IsCompatibleWith(HRPPixelTypeV24PhotoYCC::CLASS_ID)); 
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV24PhotoYCC())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV24PhotoYCC::CLASS_ID)); 

    //Test Filtering #JB
    }

//==================================================================================
// Test Color replacer filter
//==================================================================================
TEST_F(HRAImageOpFunctionFiltersTester, ColorReplacerFilterTest)
    {
    Byte newRGB[3] = {0, 255, 0};
    HGFRGBSet rgbSet;
    rgbSet.Add(255, 0, 0);
    rgbSet.Add(0, 0, 255);
    bvector<HGFRGBSet> rgbSetList;
    rgbSetList.push_back(rgbSet);
    bvector<HGFRGBSet> emptySetList;
    HRAImageOpPtr pNewOp = HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(newRGB, rgbSetList, emptySetList);

    // Test available input with no output set
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);
    TestAvailableInputPixelType(pNewOp, 0, 2, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test available output with no input set
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, 0, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success,  pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));

    // Output is set to HRPPixelTypeV24R8G8B8; available input should be also HRPPixelTypeV24R8G8B8 (even if index varies)
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Recreate filter
    pNewOp = HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(newRGB, rgbSetList, emptySetList);
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Set input and output pixel types
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(pNewOp->GetInputPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID)); 
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID)); 

    // Create image and expected output image
    HFCPtr<HRPPixelType> pixelTypeV24R8G8B8 = new HRPPixelTypeV24R8G8B8();
    HRAImageSamplePtr pImageIn;
    CreateImage_ColorReplacer(pImageIn, 1024, 1024, pixelTypeV24R8G8B8);
    HRAImageSamplePtr pImageOutExpected;
    CreateImageExpected_ColorReplacer(pImageOutExpected, 1024, 1024, pixelTypeV24R8G8B8);
    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_ColorReplacer(pImageIn, 137, 46, pixelTypeV24R8G8B8);
    CreateImageExpected_ColorReplacer(pImageOutExpected, 137, 46, pixelTypeV24R8G8B8);
    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_ColorReplacer(pImageIn, 1, 1, pixelTypeV24R8G8B8);
    CreateImageExpected_ColorReplacer(pImageOutExpected, 1, 1, pixelTypeV24R8G8B8);
    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    // Recreate filter
    pNewOp = HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(newRGB, rgbSetList, emptySetList);
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Set input and output pixel types
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(pNewOp->GetInputPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID)); 
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID)); 

    // Create image and expected output image
    HFCPtr<HRPPixelType> pixelTypeV32R8G8B8A8 = new HRPPixelTypeV24R8G8B8();
    CreateImage_ColorReplacer(pImageIn, 1024, 1024, pixelTypeV32R8G8B8A8);
    CreateImageExpected_ColorReplacer(pImageOutExpected, 1024, 1024, pixelTypeV32R8G8B8A8);
    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_ColorReplacer(pImageIn, 137, 46, pixelTypeV32R8G8B8A8);
    CreateImageExpected_ColorReplacer(pImageOutExpected, 137, 46, pixelTypeV32R8G8B8A8);
    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_ColorReplacer(pImageIn, 1, 1, pixelTypeV32R8G8B8A8);
    CreateImageExpected_ColorReplacer(pImageOutExpected, 1, 1, pixelTypeV32R8G8B8A8);
    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);
    }

//==================================================================================
// Test Alpha Replacer filter
//==================================================================================
TEST_F(HRAImageOpFunctionFiltersTester, AlphaReplacerFilterTest)
    {
    Byte defaultAlpha = 50;
    const VectorHRPAlphaRange  emptyRanges;
    HRAImageOpPtr pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, emptyRanges);

    // Test available input with no output set
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV32R8G8B8A8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, 0, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test available output with no input set
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, 0, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); // No alpha channel
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEPP_STATUS_Success,           pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));

    // Output is set to HRPPixelTypeV32R8G8B8A8; available input should be also HRPPixelTypeV32R8G8B8A8 (even if index varies)
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);

    // Recreate filter
    pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, emptyRanges);
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Set input and output pixel types
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(pNewOp->GetInputPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID)); 
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID)); 

    // Create image and expected output image
    HFCPtr<HRPPixelType> pixelTypeV24R8G8B8 = new HRPPixelTypeV24R8G8B8();
    HRAImageSamplePtr pImageIn;
    CreateImage_1(pImageIn, 256, 256, pixelTypeV24R8G8B8);
    HFCPtr<HRPPixelType> pixelTypeV32R8G8B8A8 = new HRPPixelTypeV32R8G8B8A8();
    HRAImageSamplePtr pImageOutExpected;
    CreateImage_1(pImageOutExpected, 256, 256, pixelTypeV32R8G8B8A8, defaultAlpha);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);
 
    // Try with different image sizes
    CreateImage_1(pImageIn, 100, 200, pixelTypeV24R8G8B8);
    CreateImage_1(pImageOutExpected, 100, 200, pixelTypeV32R8G8B8A8, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_1(pImageIn, 1000, 256, pixelTypeV24R8G8B8);
    CreateImage_1(pImageOutExpected, 1000, 256, pixelTypeV32R8G8B8A8, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_1(pImageIn, 1, 1, pixelTypeV24R8G8B8);
    CreateImage_1(pImageOutExpected, 1, 1, pixelTypeV32R8G8B8A8, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    // Recreate filter to test BGR as input pixel type
    defaultAlpha = 122;
    pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, emptyRanges);

    // Set input and output pixel types
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24B8G8R8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 

    HFCPtr<HRPPixelType> pixelTypeV24B8G8R8 = new HRPPixelTypeV24B8G8R8();
    CreateImage_3_RGBA_Specified(pImageIn, 50, 60, pixelTypeV24B8G8R8, 75, 150, 200, 0);
    CreateImage_3_RGBA_Specified(pImageOutExpected, 50, 60, pixelTypeV32R8G8B8A8, 75, 150, 200, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Recreate filter and test ranges
    //
    defaultAlpha = 100;

    // Create ranges to assign alpha values
    VectorHRPAlphaRange listAlphaRanges;
    Create_AlphaRangeList_1(listAlphaRanges, 60);

    // Create filter
    pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, listAlphaRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID)); 

    // Create image and expected output image
    CreateImage_1(pImageIn, 1, 200, pixelTypeV24R8G8B8);
    CreateImage_2_AlphaWithRanges(pImageOutExpected, 1, 200, pixelTypeV32R8G8B8A8, defaultAlpha, listAlphaRanges);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Test more ranges
    //
    defaultAlpha = 222;

    // Create ranges to assign alpha values
    Create_AlphaRangeList_2(listAlphaRanges);

    // Create filter
    pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, listAlphaRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID)); 

    // Create image and expected output image
    CreateImage_1(pImageIn, 200, 400, pixelTypeV24R8G8B8);
    CreateImage_2_AlphaWithRanges(pImageOutExpected,    200, 400, pixelTypeV32R8G8B8A8, defaultAlpha, listAlphaRanges);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Test input image with alpha channel
    //
    defaultAlpha = 80;

    // Create ranges to assign alpha values
    Create_AlphaRangeList_2(listAlphaRanges);

    // Create filter
    pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, listAlphaRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 

    // Create image and expected output image
    CreateImage_2_AlphaWithRanges(pImageIn, 100, 200, pixelTypeV32R8G8B8A8, defaultAlpha, emptyRanges);
    CreateImage_2_AlphaWithRanges(pImageOutExpected, 100, 200, pixelTypeV32R8G8B8A8, defaultAlpha, listAlphaRanges);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);
    }

//==================================================================================
// Test Alpha Composer filter
//==================================================================================
TEST_F(HRAImageOpFunctionFiltersTester, AlphaComposerFilterTest)
    {
    Byte defaultAlpha = 50;
    const VectorHRPAlphaRange  emptyRanges;
    HRAImageOpPtr pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter(defaultAlpha, emptyRanges);

    // Test available input with no output set
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV32R8G8B8A8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, 0, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test available output with no input set
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, 0, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); // No alpha channel
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType,  pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEPP_STATUS_Success,           pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));

    // Output is set to HRPPixelTypeV32R8G8B8A8; available input should be also HRPPixelTypeV32R8G8B8A8 (even if index varies)
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);

    // Recreate filter
    pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter (defaultAlpha, emptyRanges);
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Set input and output pixel types
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(pNewOp->GetInputPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID)); 
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID)); 

    // Create image and expected output image
    HFCPtr<HRPPixelType> pixelTypeV24R8G8B8 = new HRPPixelTypeV24R8G8B8();
    HRAImageSamplePtr pImageIn;
    CreateImage_1(pImageIn, 256, 256, pixelTypeV24R8G8B8);
    HFCPtr<HRPPixelType> pixelTypeV32R8G8B8A8 = new HRPPixelTypeV32R8G8B8A8();
    HRAImageSamplePtr pImageOutExpected;
    CreateImage_1(pImageOutExpected, 256, 256, pixelTypeV32R8G8B8A8, defaultAlpha);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);
 
    // Try with different image sizes
    CreateImage_1(pImageIn, 100, 200, pixelTypeV24R8G8B8);
    CreateImage_1(pImageOutExpected, 100, 200, pixelTypeV32R8G8B8A8, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_1(pImageIn, 1000, 256, pixelTypeV24R8G8B8);
    CreateImage_1(pImageOutExpected, 1000, 256, pixelTypeV32R8G8B8A8, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    CreateImage_1(pImageIn, 1, 1, pixelTypeV24R8G8B8);
    CreateImage_1(pImageOutExpected, 1, 1, pixelTypeV32R8G8B8A8, defaultAlpha);
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Recreate filter and test ranges
    //
    defaultAlpha = 100;

    // Create ranges to assign alpha values
    VectorHRPAlphaRange  listAlphaRanges;
    Create_AlphaRangeList_1(listAlphaRanges, 35);

    // Create filter
    pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter (defaultAlpha, listAlphaRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID)); 

    // Create image and expected output image
    CreateImage_1(pImageIn, 1, 200, pixelTypeV24R8G8B8);
    CreateImage_2_AlphaWithRanges(pImageOutExpected, 1, 200, pixelTypeV32R8G8B8A8, defaultAlpha, listAlphaRanges);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Test more ranges
    //
    defaultAlpha = 222;

    // Create ranges to assign alpha values
    Create_AlphaRangeList_2(listAlphaRanges);

    // Create filter
    pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter (defaultAlpha, listAlphaRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType()->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID)); 

    // Create image and expected output image
    CreateImage_1(pImageIn, 200, 400, pixelTypeV24R8G8B8);
    CreateImage_2_AlphaWithRanges(pImageOutExpected,    200, 400, pixelTypeV32R8G8B8A8, defaultAlpha, listAlphaRanges);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Test input image with alpha channel
    //
    Byte defaultAlphaInputImage = 100;
    defaultAlpha = 80;

    // Create filter
    pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter (defaultAlpha, emptyRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 

    // Create image and expected output image
    CreateImage_2_AlphaWithRanges(pImageIn, 1, 200, pixelTypeV32R8G8B8A8, defaultAlphaInputImage, emptyRanges);

    // Expected alpha is 31: (100 * 80 / 255)
    CreateImage_2_AlphaWithRanges(pImageOutExpected, 1, 200, pixelTypeV32R8G8B8A8, 31, emptyRanges);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);

    //
    // Test input image with alpha channel; filter with ranges
    //
    defaultAlphaInputImage = 100;
    Byte filterDefaultAlpha = 200;
    Byte alphaValueFilterRange = 70;

    // Create ranges for filter
    Create_AlphaRangeList_1(listAlphaRanges, alphaValueFilterRange);

    // Compute composed alpha value for ranges
    Byte composedAlpha = ((uint32_t)defaultAlphaInputImage * (uint32_t)alphaValueFilterRange) / 255;
    VectorHRPAlphaRange  listAlphaRangesComposed;
    Create_AlphaRangeList_1(listAlphaRangesComposed, composedAlpha);

    // Create filter
    pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter (filterDefaultAlpha, listAlphaRanges);
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_TRUE(IMAGEPP_STATUS_Success == pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8())); 

    // Create image and expected output image
    CreateImage_2_AlphaWithRanges(pImageIn, 1, 200, pixelTypeV32R8G8B8A8, defaultAlphaInputImage, emptyRanges);

    // Compute expected composed default alpha
    composedAlpha = ((uint32_t)defaultAlphaInputImage * (uint32_t)filterDefaultAlpha) / 255;
    CreateImage_2_AlphaWithRanges(pImageOutExpected, 1, 200, pixelTypeV32R8G8B8A8, composedAlpha, listAlphaRangesComposed);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpFunctionFiltersTester::HRAImageOpFunctionFiltersTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpFunctionFiltersTester::~HRAImageOpFunctionFiltersTester()
    {
    m_allocator = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IImageAllocator& HRAImageOpFunctionFiltersTester::GetMemoryManager()
    {
    if (m_allocator.IsNull())
        m_allocator = m_allocatorPool.GetAllocatorRef();
    return m_allocator->GetAllocator();
    }
    
//==================================================================================
// Test if "GetAvailableInputPixelType" returns the expected pixel types and status.
//==================================================================================
void HRAImageOpFunctionFiltersTester::TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
    {
    ImagePPStatus imageOpStatus;
    HFCPtr<HRPPixelType> pixelType;
    imageOpStatus = pImageOp->GetAvailableInputPixelType(pixelType, pixelTypeIndex, pixelTypeToMatch);

    if (imageOpStatus != expectedStatus)
        {
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableInputPixelType: ImagePPStatus:%d expectedStatus:%d", 
                                      TestInfo()->test_case_name(), TestInfo()->name(), imageOpStatus, expectedStatus);
        FAIL() << errorMsg;
        }

    // Don't check pixelTypeId if == 0
    if (pixelTypeId > 0 && pixelType != NULL)
        {
        if (!pixelType->IsCompatibleWith(pixelTypeId))
            {
            wchar_t errorMsg[512];
            BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableInputPixelType: pixel type not compatible with:%d", 
                                          TestInfo()->test_case_name(), TestInfo()->name(), pixelTypeId);
            FAIL() << errorMsg;
            }
        }
    }

//==================================================================================
// Test if "GetAvailableOutputPixelType" returns the expected pixel types and status.
//==================================================================================
void HRAImageOpFunctionFiltersTester::TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
    {
    ImagePPStatus imageOpStatus;
    HFCPtr<HRPPixelType> pixelType;
    imageOpStatus = pImageOp->GetAvailableOutputPixelType(pixelType, pixelTypeIndex, pixelTypeToMatch);

    if (imageOpStatus != expectedStatus)
        {
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableOutputPixelType: ImagePPStatus:%d expectedStatus:%d", 
                                      TestInfo()->test_case_name(), TestInfo()->name(), imageOpStatus, expectedStatus);
        FAIL() << errorMsg;
        }

    // Don't check pixelTypeId if == 0
    if (pixelTypeId > 0 && pixelType != NULL)
        {
        if (!pixelType->IsCompatibleWith(pixelTypeId))
            {
            wchar_t errorMsg[512];
            BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableOutputPixelType: pixel type not compatible with:%d", 
                                          TestInfo()->test_case_name(), TestInfo()->name(), pixelTypeId);
            FAIL() << errorMsg;
            }
        }
    }

//==================================================================================
// Apply filtering to an input image and verify if the result is equal to the
// expected output image.
//==================================================================================
void HRAImageOpFunctionFiltersTester::TestFiltering(HRAImageOpPtr pImageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected)
    {
    ASSERT_EQ(imageSampleIn.GetWidth(), imageSampleOutExpected.GetWidth());
    ASSERT_EQ(imageSampleIn.GetHeight(), imageSampleOutExpected.GetHeight());

    // Create output image - it will contain the result of filtering imageSampleIn
    uint32_t width = imageSampleOutExpected.GetWidth();
    uint32_t height = imageSampleOutExpected.GetHeight();

    ImagePPStatus status;
    HRAImageSamplePtr pImageOut = HRAImageSample::CreateSample(status, width, height, pImageOp->GetOutputPixelType(), GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    size_t pixelSize = (pImageOp->GetOutputPixelType()->CountPixelRawDataBits() +7) /8;
    
    HGF2DIdentity identity;
    ImagepOpParams imageOpParams(identity);
    imageOpParams.SetOffset(0, 0);

    pImageOp->Process(*pImageOut, imageSampleIn, imageOpParams);

    // Compare filtered image (pImageOut) with expected image (imageSampleOutExpected)
    size_t pitch;
    Byte* pOutBuffer = pImageOut->GetBufferP()->GetDataP(pitch);
    size_t pitchExpected;
    const Byte* pOutBufferExpected = imageSampleOutExpected.GetBufferCP()->GetDataCP(pitchExpected);
    ASSERT_EQ(pitch, pitchExpected) << L"TEST: (" << TestInfo()->test_case_name() << ", " << TestInfo()->name() << ") - In TestFiltering.";

    uint32_t posBuffer = 0;
    for(uint32_t row=0; row < height; ++row)
        {
        for(uint32_t column=0; column < width; ++column)
            {
            for(uint32_t channel=0; channel < pixelSize; ++channel)
                {
                if (pOutBuffer[posBuffer] != pOutBufferExpected[posBuffer])
                    {
                    wchar_t errorMsg[512];
                    BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestFiltering: pOutBuffer[%d]:%d pOutBufferExpected[%d]:%d", 
                                                  TestInfo()->test_case_name(), TestInfo()->name(),
                                                  posBuffer, pOutBuffer[posBuffer], posBuffer, pOutBufferExpected[posBuffer]);
                    FAIL() << errorMsg;
                    }
                posBuffer++;
                }
            }
        }
    }

//==================================================================================
// Creation of a test image
//==================================================================================
void HRAImageOpFunctionFiltersTester::CreateImage_1(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType, Byte alphaValue)
    {
    ImagePPStatus status;
    prImageSample = HRAImageSample::CreateSample(status, width, height, prPixelType, GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);

    if (prPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                }
            }
        }
    else if (prPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = alphaValue;
                }
            }
        }
    else
        {
        // Pixel type not supported yet by this method
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In CreateImage_1: pixel type not supported.", 
                                      TestInfo()->test_case_name(), TestInfo()->name());
        FAIL() << errorMsg;
        }
    }

//==================================================================================
// Creation of a test image that contains alpha channel adjusted with ranges.
//==================================================================================
void HRAImageOpFunctionFiltersTester::CreateImage_2_AlphaWithRanges(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType, Byte alphaValue, const VectorHRPAlphaRange& ranges)
    {
    ImagePPStatus status;
    prImageSample = HRAImageSample::CreateSample(status, width, height, prPixelType, GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);
    uint32_t nbRanges = (uint32_t)ranges.size();

    if (prPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = (Byte) (row % 256);
                pBufferLine[posBuffer++] = alphaValue;

                // If the color is in some specified range list, the alpha channel is assigned the value specified by this range.
                uint32_t posBuffer2 = posBuffer - 4;
                for(uint32_t rangeIdx = 0; rangeIdx < nbRanges; rangeIdx++)
                    if(ranges[rangeIdx].IsIn(pBufferLine[posBuffer2], 
                                                 pBufferLine[posBuffer2 + 1], 
                                                 pBufferLine[posBuffer2 + 2]))
                        // if yes, set the new alpha value
                        pBufferLine[posBuffer2 + 3] = ranges[rangeIdx].GetAlphaValue();
                }
            }
        }
    else
        {
        // Pixel type not supported yet by this method
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In CreateImage_2_AlphaWithRanges: pixel type not supported.", 
                                      TestInfo()->test_case_name(), TestInfo()->name());
        FAIL() << errorMsg;
        }
    }

//==================================================================================
// Creation of a list of alpha ranges.
//==================================================================================
void HRAImageOpFunctionFiltersTester::Create_AlphaRangeList_1(VectorHRPAlphaRange& listAlphaRange, Byte alphaValue)
    {
    listAlphaRange.clear();

    HGFRGBSet rgbSet;
    rgbSet.Add(0, 0, 0);
    rgbSet.Add(5, 5, 5);
    rgbSet.Add(75, 75, 75);
    rgbSet.Add(10, 20, 30);
    HFCPtr<HGFRGBSet> pRgbSet(new HGFRGBSet(rgbSet));
    HRPAlphaRange alphaRange ((HFCPtr<HGFColorSet>&)pRgbSet, alphaValue);

    listAlphaRange.push_back(alphaRange);
    }

//==================================================================================
// Creation of a list of alpha ranges. The list contains many ranges.
//==================================================================================
void HRAImageOpFunctionFiltersTester::Create_AlphaRangeList_2(VectorHRPAlphaRange& listAlphaRange)
    {
    listAlphaRange.clear();

    HGFRGBSet rgbSet;
    rgbSet.Add(8, 10, 0);
    rgbSet.Add(0, 5, 5);
    rgbSet.Add(175, 175, 175);
    rgbSet.Add(10, 20, 30);
    HFCPtr<HGFRGBSet> pRgbSet(new HGFRGBSet(rgbSet));
    HRPAlphaRange alphaRange ((HFCPtr<HGFColorSet>&)pRgbSet, 35);
    listAlphaRange.push_back(alphaRange);

    HGFRGBSet rgbSet2;
    rgbSet2.Add(8, 8, 8);
    rgbSet2.Add(15, 15, 15);
    rgbSet2.Add(255, 255, 255);
    HFCPtr<HGFRGBSet> pRgbSet2(new HGFRGBSet(rgbSet2));
    HRPAlphaRange alphaRange2 ((HFCPtr<HGFColorSet>&)pRgbSet2, 200);
    listAlphaRange.push_back(alphaRange2);

    HGFRGBSet rgbSet3;
    rgbSet3.Add(2, 2, 2);
    rgbSet3.Add(3, 3, 3);
    rgbSet3.Add(4, 4, 4);
    HFCPtr<HGFRGBSet> pRgbSet3(new HGFRGBSet(rgbSet3));
    HRPAlphaRange alphaRange3 ((HFCPtr<HGFColorSet>&)pRgbSet3, 150);
    listAlphaRange.push_back(alphaRange3);
    }

//==================================================================================
// Creation of a test image. RGBA values can be specified (they're the same for all pixels)
//==================================================================================
void HRAImageOpFunctionFiltersTester::CreateImage_3_RGBA_Specified
    (HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType, Byte redValue, Byte greenValue, Byte blueValue, Byte alphaValue)
    {
    ImagePPStatus status;
    prImageSample = HRAImageSample::CreateSample(status, width, height, prPixelType, GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);

    if (prPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = redValue;
                pBufferLine[posBuffer++] = greenValue;
                pBufferLine[posBuffer++] = blueValue;
                }
            }
        }
    else if (prPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = redValue;
                pBufferLine[posBuffer++] = greenValue;
                pBufferLine[posBuffer++] = blueValue;
                pBufferLine[posBuffer++] = alphaValue;
                }
            }
        }
    else if (prPixelType->IsCompatibleWith(HRPPixelTypeV24B8G8R8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = blueValue;
                pBufferLine[posBuffer++] = greenValue;
                pBufferLine[posBuffer++] = redValue;
                }
            }
        }
    else
        {
        // Pixel type not supported yet by this method
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In CreateImage_3_RGBA_Specified: pixel type not supported.", 
                                      TestInfo()->test_case_name(), TestInfo()->name());
        FAIL() << errorMsg;
        }
    }

//==================================================================================
// Creation of a test image
//==================================================================================
void HRAImageOpFunctionFiltersTester::CreateImage_ColorReplacer(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType)
    {
    ImagePPStatus status;
    prImageSample = HRAImageSample::CreateSample(status, width, height, prPixelType, GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);

    if (prPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                //Even row pixel is red
                if(row % 2 == 0)
                    {
                    pBufferLine[posBuffer++] = (Byte) (255);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    }
                //Odd row pixel is green
                else
                    {
                    pBufferLine[posBuffer++] = (Byte) (0);
                    pBufferLine[posBuffer++] = (Byte) (255);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    }
                }
            }
        }
    else if (prPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                //Even row pixel is red
                if(row % 2 == 0)
                    {
                    pBufferLine[posBuffer++] = (Byte) (255);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    }
                //Odd row pixel is green
                else
                    {
                    pBufferLine[posBuffer++] = (Byte) (0);
                    pBufferLine[posBuffer++] = (Byte) (255);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    pBufferLine[posBuffer++] = (Byte) (0);
                    }
                }
            }
        }
    else
        {
        // Pixel type not supported yet by this method
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In CreateImage_1: pixel type not supported.", 
                                      TestInfo()->test_case_name(), TestInfo()->name());
        FAIL() << errorMsg;
        }
    }

//==================================================================================
// Creation of a test image
//==================================================================================
void HRAImageOpFunctionFiltersTester::CreateImageExpected_ColorReplacer(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType)
    {
    ImagePPStatus status;
    prImageSample = HRAImageSample::CreateSample(status, width, height, prPixelType, GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);

    if (prPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                //Even row pixel is expected to be green also...
                pBufferLine[posBuffer++] = (Byte) (0);
                pBufferLine[posBuffer++] = (Byte) (255);
                pBufferLine[posBuffer++] = (Byte) (0);
                }
            }
        }
    else if (prPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                //Even row pixel is expected to be green also...
                pBufferLine[posBuffer++] = (Byte) (0);
                pBufferLine[posBuffer++] = (Byte) (255);
                pBufferLine[posBuffer++] = (Byte) (0);
                }
            }
        }
    else
        {
        // Pixel type not supported yet by this method
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In CreateImage_1: pixel type not supported.", 
                                      TestInfo()->test_case_name(), TestInfo()->name());
        FAIL() << errorMsg;
        }
    }

//==================================================================================
// Utility method that returns TestInfo
//==================================================================================
const ::testing::TestInfo* HRAImageOpFunctionFiltersTester::TestInfo()
    {
    return ::testing::UnitTest::GetInstance()->current_test_info();
    }


