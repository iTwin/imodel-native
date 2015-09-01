//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageOpContrastStretchTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include <ImagePP\all\h\HRAImageOpContrastStretchFilter.h>


// Preparation of required environment
class HRAImageOpContrastStretchTester : public testing::Test
    {
    protected:
        HRAImageOpContrastStretchTester();

        ~HRAImageOpContrastStretchTester();

        void TestFiltering(HRAImageOpPtr imageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected);
        void TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        void TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        static const ::testing::TestInfo* TestInfo();

        IImageAllocator& GetMemoryManager();

    private:
        ImageAllocatorPool m_allocatorPool;
        ImageAllocatorRefPtr m_allocator;
    };


//==================================================================================
// LightnessContrastStretchTest
//==================================================================================
TEST_F(HRAImageOpContrastStretchTester, LightnessContrastStretchTest)
    {
    HRAImageOpLightnessContrastStretchFilterPtr pNewOp = HRAImageOpLightnessContrastStretchFilter::CreateLightnessContrastStretchFilter();
    pNewOp->SetInterval(20, 87);
    pNewOp->SetContrastInterval(0, 85);
    pNewOp->SetGammaFactor(1);

    // Test available input with no output set
    // Pixel type to match with no alpha
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeI8Gray8(), IMAGEPP_STATUS_Success);
    // Pixel depth > 8
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV48R16G16B16::CLASS_ID, 0, new HRPPixelTypeV16Gray16(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV48R16G16B16::CLASS_ID, 0, new HRPPixelTypeV48R16G16B16(), IMAGEPP_STATUS_Success);

    // Pixel type to match with alpha
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV32R8G8B8A8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV16PRGray8A8(), IMAGEPP_STATUS_Success);
    // Pixel depth > 8
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV64R16G16B16A16(), IMAGEPP_STATUS_Success);

    // Test available output with no input set
    // Pixel type to match with no alpha
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeI8Gray8(), IMAGEPP_STATUS_Success);
    // Pixel depth > 8
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV48R16G16B16::CLASS_ID, 0, new HRPPixelTypeV16Gray16(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV48R16G16B16::CLASS_ID, 0, new HRPPixelTypeV48R16G16B16(), IMAGEPP_STATUS_Success);

    // Pixel type to match with alpha
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV32R8G8B8A8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV16PRGray8A8(), IMAGEPP_STATUS_Success);
    // Pixel depth > 8
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV64R16G16B16A16(), IMAGEPP_STATUS_Success);

    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 1, new HRPPixelTypeV64R16G16B16A16(), IMAGEOP_STATUS_NoMorePixelType);

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(NULL)); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeI1R8G8B8A8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV64R16G16B16A16()));

    // Test available output with HRPPixelTypeV64R16G16B16A16 as input
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Recreate filter
    pNewOp = HRAImageOpLightnessContrastStretchFilter::CreateLightnessContrastStretchFilter();
    pNewOp->SetInterval(20, 87);
    pNewOp->SetContrastInterval(0, 99);
    pNewOp->SetGammaFactor(1);
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(NULL)); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeI1R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV64R16G16B16A16()));

    // Test available input with HRPPixelTypeV64R16G16B16A16 as output
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    //Test Get/Set
    float MinValue, MaxValue, MinContrastValue, MaxContrastValue;
    double GammaFactor;
    pNewOp->GetInterval(MinValue, MaxValue);
    ASSERT_FLOAT_EQ(20, MinValue);
    ASSERT_FLOAT_EQ(87, MaxValue);
    pNewOp->GetContrastInterval(MinContrastValue, MaxContrastValue);
    ASSERT_FLOAT_EQ(0, MinContrastValue);
    ASSERT_FLOAT_EQ(99, MaxContrastValue);
    pNewOp->GetGammaFactor(GammaFactor);
    ASSERT_DOUBLE_EQ(1, GammaFactor);

    //TODO #JB
    //TestFiltering
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastStretchTester::HRAImageOpContrastStretchTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpContrastStretchTester::~HRAImageOpContrastStretchTester()
    {
    m_allocator = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IImageAllocator& HRAImageOpContrastStretchTester::GetMemoryManager()
    {
    if (m_allocator.IsNull())
        m_allocator = m_allocatorPool.GetAllocatorRef();
    return m_allocator->GetAllocator();
    }
    
//==================================================================================
// Test if "GetAvailableInputPixelType" returns the expected pixel types and status.
//==================================================================================
void HRAImageOpContrastStretchTester::TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex,  HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
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
void HRAImageOpContrastStretchTester::TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
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
void HRAImageOpContrastStretchTester::TestFiltering(HRAImageOpPtr pImageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected)
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
// Utility method that returns TestInfo
//==================================================================================
const ::testing::TestInfo* HRAImageOpContrastStretchTester::TestInfo()
    {
    return ::testing::UnitTest::GetInstance()->current_test_info();
    }


