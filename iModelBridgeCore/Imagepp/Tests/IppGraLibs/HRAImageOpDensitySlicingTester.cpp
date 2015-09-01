//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageOpDensitySlicingTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include <ImagePP\all\h\HRAImageOpDensitySlicingFilter.h>


// Preparation of required environment
class HRAImageOpDensitySlicingTester : public testing::Test
    {
    protected:
        HRAImageOpDensitySlicingTester();
        ~HRAImageOpDensitySlicingTester();

        void TestFiltering(HRAImageOpPtr imageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected);
        bool TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        bool TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        static const ::testing::TestInfo* TestInfo();

        IImageAllocator& GetMemoryManager();

    private:
        ImageAllocatorPool m_allocatorPool;
        ImageAllocatorRefPtr m_allocator;
    };



//==================================================================================
// DensitySlicingTest
//==================================================================================
TEST_F(HRAImageOpDensitySlicingTester, DensitySlicingTest)
    {
    HRAImageOpPtr pNewOp = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_8bits);
    HRAImageOpDensitySlicingFilter* pDensitySlicingFilter = static_cast<HRAImageOpDensitySlicingFilter*>(pNewOp.get());
    pDensitySlicingFilter->AddSlice(0, 255, 16711680, 65280, 0.5);
    pDensitySlicingFilter->SetDesaturationFactor(0.5);

    // Test available input with no output set
    ASSERT_TRUE(TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));

    // Test available output with no input set
    ASSERT_TRUE(TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(NULL)); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeI1R8G8B8A8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV64R16G16B16A16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));

    // Test available output with HRPPixelTypeV32R8G8B8A8 as input
    ASSERT_TRUE(TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));
    ASSERT_TRUE(TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType));

    // Recreate filter
    pNewOp = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_16bits);
    pDensitySlicingFilter = static_cast<HRAImageOpDensitySlicingFilter*>(pNewOp.get());
    pDensitySlicingFilter->AddSlice(0, 65385, 16711680, 65280, 0.5);
    pDensitySlicingFilter->SetDesaturationFactor(0.5);

    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(NULL)); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeI1R8G8B8A8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV64R16G16B16A16()));

    // Test available input with HRPPixelTypeV64R16G16B16A16 as output
    ASSERT_TRUE(TestAvailableInputPixelType(pNewOp,  HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));
    ASSERT_TRUE(TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType));

    //Test Slice functions
    pNewOp = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_8bits);
    pDensitySlicingFilter = static_cast<HRAImageOpDensitySlicingFilter*>(pNewOp.get());
    pDensitySlicingFilter->AddSlice(0, 255, 16711680, 65280, 0.5);
    pDensitySlicingFilter->SetDesaturationFactor(0.5);
    ASSERT_DOUBLE_EQ(0.5, pDensitySlicingFilter->GetDesaturationFactor()); 

    ASSERT_EQ(0, pDensitySlicingFilter->GetSlices()[0].m_StartValue); 
    ASSERT_EQ(255, pDensitySlicingFilter->GetSlices()[0].m_EndValue); 
    ASSERT_EQ(16711680, pDensitySlicingFilter->GetSlices()[0].m_StartColor); 
    ASSERT_EQ(65280, pDensitySlicingFilter->GetSlices()[0].m_EndColor); 
    ASSERT_DOUBLE_EQ(0.5, pDensitySlicingFilter->GetSlices()[0].m_Opacity);

    ASSERT_EQ(1, pDensitySlicingFilter->GetSlices().size());
    pDensitySlicingFilter->ClearSlices();
    ASSERT_EQ(0, pDensitySlicingFilter->GetSlices().size());

    pDensitySlicingFilter->AddSlice(0, 255, 16711680, 65280, 0.5);
    pDensitySlicingFilter->AddSlice(10, 210, 16711680, 65280, 0.5);
    pDensitySlicingFilter->AddSlice(20, 220, 16711680, 65280, 0.5);
    ASSERT_EQ(3, pDensitySlicingFilter->GetSlices().size());
    pDensitySlicingFilter->ClearSlices();
    ASSERT_EQ(0, pDensitySlicingFilter->GetSlices().size());

    pDensitySlicingFilter->AddSlice(0, 255, 16711680, 65280, 0.5);
    pDensitySlicingFilter->AddSlice(10, 210, 16711680, 65280, 0.5);
    ASSERT_EQ(2, pDensitySlicingFilter->GetSlices().size());
    ASSERT_EQ(0, pDensitySlicingFilter->GetSlices()[0].m_StartValue); 
    ASSERT_EQ(255, pDensitySlicingFilter->GetSlices()[0].m_EndValue); 
    ASSERT_EQ(16711680, pDensitySlicingFilter->GetSlices()[0].m_StartColor); 
    ASSERT_EQ(65280, pDensitySlicingFilter->GetSlices()[0].m_EndColor); 
    ASSERT_DOUBLE_EQ(0.5, pDensitySlicingFilter->GetSlices()[0].m_Opacity); 

    //TODO #JB
    //TestFiltering
    }

//==================================================================================
// LightnessDensitySlicingTest
//==================================================================================
TEST_F(HRAImageOpDensitySlicingTester, LightnessDensitySlicingTest)
    {
    HRAImageOpLightnessDensitySlicingFilterPtr pNewOp = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_8bits);
    pNewOp->AddSlice(0, 100, 16711680, 65280, 0.5);
    pNewOp->SetDesaturationFactor(0.5);

    // Test available input with no output set
    ASSERT_TRUE(TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));

    // Test available output with no input set
    ASSERT_TRUE(TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(NULL)); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeI1R8G8B8A8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetInputPixelType(new HRPPixelTypeV64R16G16B16A16()));

    // Test available output with HRPPixelTypeV32R8G8B8A8 as input
    ASSERT_TRUE(TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));
    ASSERT_TRUE(TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV32R8G8B8A8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType));

    // Recreate filter
    pNewOp = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_16bits);
    pNewOp->AddSlice(0, 100, 16711680, 65280, 0.5);
    pNewOp->SetDesaturationFactor(0.5);

    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(NULL)); 
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeI1R8G8B8A8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8()));
    ASSERT_EQ(IMAGEOP_STATUS_InvalidPixelType, pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV64R16G16B16A16()));

    // Test available input with HRPPixelTypeV64R16G16B16A16 as output
    ASSERT_TRUE(TestAvailableInputPixelType(pNewOp,  HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success));
    ASSERT_TRUE(TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType));

    //Test Slice functions
    pNewOp = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpDensitySlicingFilter::PIXELDEPTH_8bits);
    pNewOp->AddSlice(0, 100, 16711680, 65280, 0.5);
    pNewOp->SetDesaturationFactor(0.5);
    ASSERT_DOUBLE_EQ(0.5, pNewOp->GetDesaturationFactor()); 

    HRAImageOpLightnessDensitySlicingFilter::SliceList const& slices = pNewOp->GetSlices();

    ASSERT_EQ(1, pNewOp->GetSlices().size());

    ASSERT_FLOAT_EQ(0, slices[0].m_StartValue);
    ASSERT_FLOAT_EQ(100, slices[0].m_EndValue);
    ASSERT_EQ(16711680, slices[0].m_StartColor);
    ASSERT_EQ(65280, slices[0].m_EndColor);
    ASSERT_DOUBLE_EQ(0.5, slices[0].m_Opacity);

    pNewOp->AddSlice(0, 100, 16711680, 65280, 0.5);
    pNewOp->AddSlice(10, 88, 16711680, 65280, 0.5);
    pNewOp->AddSlice(20, 86, 16711680, 65280, 0.5);
    ASSERT_EQ(4, pNewOp->GetSlices().size());

    //TODO #JB
    //TestFiltering
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDensitySlicingTester::HRAImageOpDensitySlicingTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpDensitySlicingTester::~HRAImageOpDensitySlicingTester()
    {
    m_allocator = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IImageAllocator& HRAImageOpDensitySlicingTester::GetMemoryManager()
    {
    if (m_allocator.IsNull())
        m_allocator = m_allocatorPool.GetAllocatorRef();
    return m_allocator->GetAllocator();
    }
    
//==================================================================================
// Test if "GetAvailableInputPixelType" returns the expected pixel types and status.
//==================================================================================
bool HRAImageOpDensitySlicingTester::TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
    {
    ImagePPStatus imageOpStatus;
    HFCPtr<HRPPixelType> pixelType;
    imageOpStatus = pImageOp->GetAvailableInputPixelType(pixelType, pixelTypeIndex, pixelTypeToMatch);

    if (imageOpStatus != expectedStatus)
        {
// not now
//         wchar_t errorMsg[512];
//         BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableInputPixelType: ImagePPStatus:%d expectedStatus:%d", 
//                                       TestInfo()->test_case_name(), TestInfo()->name(), imageOpStatus, expectedStatus);
        return false;
        }

    // Don't check pixelTypeId if == 0
    if (pixelTypeId > 0 && pixelType != NULL)
        {
        if (!pixelType->IsCompatibleWith(pixelTypeId))
            {
// not now
//             wchar_t errorMsg[512];
//             BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableInputPixelType: pixel type not compatible with:%d", 
//                                           TestInfo()->test_case_name(), TestInfo()->name(), pixelTypeId);
            return false;
            }
        }
    return true;
    }

//==================================================================================
// Test if "GetAvailableOutputPixelType" returns the expected pixel types and status.
//==================================================================================
bool HRAImageOpDensitySlicingTester::TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
    {
    ImagePPStatus imageOpStatus;
    HFCPtr<HRPPixelType> pixelType;
    imageOpStatus = pImageOp->GetAvailableOutputPixelType(pixelType, pixelTypeIndex, pixelTypeToMatch);

    if (imageOpStatus != expectedStatus)
        {
        // not now
//         wchar_t errorMsg[512];
//         BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableOutputPixelType: ImagePPStatus:%d expectedStatus:%d", 
//                                       TestInfo()->test_case_name(), TestInfo()->name(), imageOpStatus, expectedStatus);
        return true;
        }

    // Don't check pixelTypeId if == 0
    if (pixelTypeId > 0 && pixelType != NULL)
        {
        if (!pixelType->IsCompatibleWith(pixelTypeId))
            {
            // not now
//             wchar_t errorMsg[512];
//             BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In TestAvailableOutputPixelType: pixel type not compatible with:%d", 
//                                           TestInfo()->test_case_name(), TestInfo()->name(), pixelTypeId);
//            FAIL() << errorMsg;

            return false;
            }
        }

    return true;
    }

//==================================================================================
// Apply filtering to an input image and verify if the result is equal to the
// expected output image.
//==================================================================================
void HRAImageOpDensitySlicingTester::TestFiltering(HRAImageOpPtr pImageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected)
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
const ::testing::TestInfo* HRAImageOpDensitySlicingTester::TestInfo()
    {
    return ::testing::UnitTest::GetInstance()->current_test_info();
    }


