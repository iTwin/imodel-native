//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageOpShapingTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"

#if 0  //&&MM TODO shaping has move to OutputMerger.  Need to find a way to test this. Maybe a full copyFrom operation?
#include <ImagePP\all\h\HRAImageOpShaping.h>





// Preparation of required environment
class HRAImageOpShapingTester : public testing::Test
    {
    protected:
        HRAImageOpShapingTester();
        ~HRAImageOpShapingTester();

        void TestFiltering(HRAImageOpPtr imageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected);
        void TestClipping(HRAImageOpPtr imageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected);
        void TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);
        void TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus);

        //Testing the shaping algorithm
        void CreateImage(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType);
        HFCPtr<HVEShape> CreateShape();
        void CreateExpectedImage(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType);

        //Testing the clipping of images
        void CreateRGBImage(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, uint32_t r, uint32_t g, uint32_t b);
        HVEShape CreateShape_RectInside(double width, double height);
        HVEShape CreateShape_RectOutside(double width, double height);
        HVEShape CreateShape_RectCorner(double width, double height);
        HVEShape CreateShape_RectDoubleIntersect(double width, double height);
        HVEShape CreateShape_RectIdentical(double width, double height);
        HVEShape CreateShape_RectWhole(double width, double height);
        void CreateExpectedImageWithShape(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HVEShape& prShape);

        static const ::testing::TestInfo* TestInfo();
        IImageAllocator& GetMemoryManager();

    private:
        ImageAllocatorPool m_allocatorPool;
        IImageAllocatorPtr m_allocator;
    };

//==================================================================================
// ShapingTest - Red/Blue
//==================================================================================
TEST_F(HRAImageOpShapingTester, ClippingTest)
    {
    // Create image and expected output image
    HRAImageSamplePtr pImageIn;
    CreateRGBImage(pImageIn, 1000, 1000, 255, 0, 0);
    HVEShape shape = CreateShape_RectInside(1000, 1000);
    HRAImageOpPtr pNewOp = HRAImageOpShaper::CreateShaper(shape);
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
    HRAImageSamplePtr pImageOutExpected;
    CreateExpectedImageWithShape(pImageOutExpected, 1000, 1000, shape);
    TestClipping(pNewOp, *pImageIn, *pImageOutExpected);

    // Recreate filter - GetEffectiveShape() is a void shape. Will assert
//     shape = CreateShape_RectOutside(1000, 1000);
//     pNewOp = HRAImageOpShaper::CreateShaper(shape);
//     CreateExpectedImageWithShape(pImageOutExpected, 1000, 1000, shape);
//     TestClipping(pNewOp, *pImageIn, *pImageOutExpected);    

    // Recreate filter
    shape = CreateShape_RectCorner(1000, 1000);
    pNewOp = HRAImageOpShaper::CreateShaper(shape);
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
    CreateExpectedImageWithShape(pImageOutExpected, 1000, 1000, shape);
    TestClipping(pNewOp, *pImageIn, *pImageOutExpected);    

    // Recreate filter -Shape origin is negative
//     shape = CreateShape_RectDoubleIntersect(1000, 1000); 
//     pNewOp = HRAImageOpShaper::CreateShaper(shape);
//     ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
//     ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
//     CreateExpectedImageWithShape(pImageOutExpected, 1000, 1000, shape);
//     TestClipping(pNewOp, *pImageIn, *pImageOutExpected);    

    // Recreate filter - Shape origin is negative
    shape = CreateShape_RectIdentical(1000, 1000);
    pNewOp = HRAImageOpShaper::CreateShaper(shape);
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
    CreateExpectedImageWithShape(pImageOutExpected, 1000, 1000, shape);
    TestClipping(pNewOp, *pImageIn, *pImageOutExpected); 

    // Recreate filter - Shape origin is negative
//     shape = CreateShape_RectWhole(1000, 1000);
//     pNewOp = HRAImageOpShaper::CreateShaper(shape);
//     ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
//     ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8())); 
//     CreateExpectedImageWithShape(pImageOutExpected, 1000, 1000, shape);
//     TestClipping(pNewOp, *pImageIn, *pImageOutExpected);  
    }

//==================================================================================
// ShapingTest - Basic
//==================================================================================
TEST_F(HRAImageOpShapingTester, ShapingTest)
    {
    HRAImageOpPtr pNewOp = HRAImageOpShaper::CreateShaper(*CreateShape());

    // Test available input with no output set
    TestAvailableInputPixelType(pNewOp, 0, 0, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test available output with no input set
    TestAvailableOutputPixelType(pNewOp, 0, 0, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Test SetInputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(NULL)); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV1GrayWhite1())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeI1R8G8B8A8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8())); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV64R16G16B16A16()));

    // Test available output with HRPPixelTypeV64R16G16B16A16 as input
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableOutputPixelType(pNewOp, HRPPixelTypeV64R16G16B16A16::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    // Recreate filter
    pNewOp = HRAImageOpShaper::CreateShaper(*CreateShape());
    ASSERT_TRUE(pNewOp->GetInputPixelType() == NULL); 
    ASSERT_TRUE(pNewOp->GetOutputPixelType() == NULL); 

    // Test SetOutputPixelType
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(NULL)); 
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV1GrayWhite1()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeI1R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV32R8G8B8A8()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV48R16G16B16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV64R16G16B16A16()));
    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetOutputPixelType(new HRPPixelTypeV24R8G8B8()));

    // Test available input with HRPPixelTypeV24R8G8B8 as output
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 0, new HRPPixelTypeV24R8G8B8(), IMAGEPP_STATUS_Success);
    TestAvailableInputPixelType(pNewOp, HRPPixelTypeV24R8G8B8::CLASS_ID, 1, new HRPPixelTypeV24R8G8B8(), IMAGEOP_STATUS_NoMorePixelType);

    ASSERT_EQ(IMAGEPP_STATUS_Success, pNewOp->SetInputPixelType(new HRPPixelTypeV24R8G8B8()));

    // Create image and expected output image
    HFCPtr<HRPPixelType> pixelTypeV24R8G8B8 = new HRPPixelTypeV24R8G8B8();
    HRAImageSamplePtr pImageIn;
    CreateImage(pImageIn, 1024, 1024, pixelTypeV24R8G8B8);
    HRAImageSamplePtr pImageOutExpected;
    CreateExpectedImage(pImageOutExpected, 1024, 1024, pixelTypeV24R8G8B8);

    // Apply filtering and verify if resulting image is as expected
    TestFiltering(pNewOp, *pImageIn, *pImageOutExpected);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpShapingTester::HRAImageOpShapingTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageOpShapingTester::~HRAImageOpShapingTester()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IHPMMemoryManager& HRAImageOpShapingTester::GetMemoryManager()
    {
    if (m_allocator.IsNull())
        m_allocator = m_allocatorPool.GetAllocator();
    return *m_allocator;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Apply shaping to an input image and verify if the result is equal to the
* expected output image.
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::TestFiltering(HRAImageOpPtr pImageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected)
    {
    ASSERT_EQ(imageSampleIn.GetWidth(), imageSampleOutExpected.GetWidth());
    ASSERT_EQ(imageSampleIn.GetHeight(), imageSampleOutExpected.GetHeight());

    // Create output image - it will contain the result of filtering imageSampleIn
    uint32_t width = imageSampleOutExpected.GetWidth();
    uint32_t height = imageSampleOutExpected.GetHeight();
    
    size_t pixelSize = (pImageOp->GetOutputPixelType()->CountPixelRawDataBits() +7) /8;

    ImagePPStatus status;
    HRAImageSamplePtr pImageOut = HRAImageSample::CreateSample(status, width, height, pImageOp->GetOutputPixelType(), GetMemoryManager());
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // Compare filtered image (pImageOut) with expected image (imageSampleOutExpected)
    size_t pitch;
    Byte* pOutBuffer = pImageOut->GetBufferP()->GetDataP(pitch);
    size_t pitchExpected;
    const Byte* pOutBufferExpected = imageSampleOutExpected.GetBufferCP()->GetDataCP(pitchExpected);
    ASSERT_EQ(pitch, pitchExpected) << L"TEST: (" << TestInfo()->test_case_name() << ", " << TestInfo()->name() << ") - In TestFiltering.";

    // Clean the outbuffer
    memset(pImageOut->GetBufferP()->GetDataP(pitch), 255, pImageOut->GetWidth()*pImageOut->GetHeight()*pixelSize);

    HGF2DIdentity identity;
    ImagepOpParams imageOpParams(identity);
    imageOpParams.SetOffset(0, 0);

    pImageOp->Process(*pImageOut, imageSampleIn, imageOpParams);
    
    uint32_t posBuffer = 0;
    for(uint32_t row=0; row < height; ++row)
        {
        for(uint32_t column=0; column < width; ++column)
            {
            for(uint32_t channel=0; channel < pixelSize; ++channel)
                {
                uint32_t outnow = pOutBuffer[posBuffer];
                uint32_t outexpected = pOutBufferExpected[posBuffer];
                if (outnow != outexpected)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
* 
* Apply clipping to an input image and verify if the result is equal to the
* expected output image.
* Basically, we take a red image and apply the shaping algorithm on it. With a blue
* image as the output the result should be a red shape within the blue image.
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::TestClipping(HRAImageOpPtr pImageOp, HRAImageSampleCR imageSampleIn, HRAImageSampleCR imageSampleOutExpected)
    {
    ASSERT_EQ(imageSampleIn.GetWidth(), imageSampleOutExpected.GetWidth());
    ASSERT_EQ(imageSampleIn.GetHeight(), imageSampleOutExpected.GetHeight());

    // Create output image - it will contain the result of clipping imageSampleIn
    uint32_t width = imageSampleOutExpected.GetWidth();
    uint32_t height = imageSampleOutExpected.GetHeight();
    size_t pixelSize = (pImageOp->GetOutputPixelType()->CountPixelRawDataBits() +7) /8;

    // Create image and expected output image
    HRAImageSamplePtr pImageOut;
    CreateRGBImage(pImageOut, width, height, 0, 0, 255);
    HRAImageBufferPtr pBuffer;
    
    HGF2DIdentity identity;
    ImagepOpParams imageOpParams(identity);
    imageOpParams.SetOffset(0, 0);

    pImageOp->Process(*pImageOut, imageSampleIn, imageOpParams);

    // Compare filtered image (pImageOut) with expected image (imageSampleOutExpected)
    size_t pitch;
    Byte* pOutBuffer = pImageOut->GetBufferP()->GetDataP(pitch);
    size_t pitchExpected;
    const Byte* pOutBufferExpected = imageSampleOutExpected.GetBufferCP()->GetDataCP(pitchExpected);
    ASSERT_EQ(pitch, pitchExpected) << L"TEST: (" << TestInfo()->test_case_name() << ", " << TestInfo()->name() << ") - In TestClipping.";

    uint32_t posBuffer = 0;
    for(uint32_t row=0; row < height; ++row)
        {
        for(uint32_t column=0; column < width; ++column)
            {
            for(uint32_t channel=0; channel < pixelSize; ++channel)
                {
                uint32_t outnow = pOutBuffer[posBuffer];
                uint32_t outexpected = pOutBufferExpected[posBuffer];
                if (outnow != outexpected)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Test if "GetAvailableInputPixelType" returns the expected pixel types and status.
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::TestAvailableInputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Test if "GetAvailableOutputPixelType" returns the expected pixel types and status.
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::TestAvailableOutputPixelType(HRAImageOpPtr pImageOp, uint32_t pixelTypeId, uint32_t pixelTypeIndex, HFCPtr<HRPPixelType> pixelTypeToMatch, uint32_t expectedStatus)
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
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Creation of a test image
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::CreateImage(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType)
    {
    prImageSample = HRAImageSample::CreateSample(width, height, prPixelType);
    HRAImageBufferPtr pNewBuffer;
    size_t pixelSize = (prPixelType->CountPixelRawDataBits() +7) /8;

    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    pNewBuffer = HRAImageBufferMemory::CreateMemoryBuffer(status, prImageSample->GetWidth()*prImageSample->GetHeight()*pixelSize, prImageSample->GetWidth()*pixelSize, GetMemoryManager());
    prImageSample->SetBuffer(pNewBuffer);


    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);

    //33 is the default value that indicates there is data in this pixel
    if (prPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = (Byte) (33);
                pBufferLine[posBuffer++] = (Byte) (33);
                pBufferLine[posBuffer++] = (Byte) (33);
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
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HVEShape> HRAImageOpShapingTester::CreateShape()
    {
    //The testing Shape looks like this
    //  _1______________________ x ->
    // |  ______________________|
    // | |______________________|
    // |  ______________________|
    // | |______________________|
    // |  ______________________|
    // | |______________________|
    // |  ______________________|
    // | |______________________|
    // |________________________|
    // y                        (1024, 1024)
    //So basically, every even row the buffer is valid for 1024 pixels
    //and every odd row the buffer is valid only for 1 pixel

    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVE2DPolySegment AddPolySegment(pCoordSys);
    AddPolySegment.AppendPoint(HGF2DLocation(0, 0, pCoordSys));
    AddPolySegment.AppendPoint(HGF2DLocation(0, 1024, pCoordSys));
    AddPolySegment.AppendPoint(HGF2DLocation(1024, 1024, pCoordSys));
    for(uint32_t i=1024; i > 0; i=i-2)
        {
        AddPolySegment.AppendPoint(HGF2DLocation(1024, i, pCoordSys));
        AddPolySegment.AppendPoint(HGF2DLocation(1, i, pCoordSys));
        AddPolySegment.AppendPoint(HGF2DLocation(1, i-1, pCoordSys));
        AddPolySegment.AppendPoint(HGF2DLocation(1024, i-1, pCoordSys));
        }
    AddPolySegment.AppendPoint(HGF2DLocation(1024, 0, pCoordSys));
    AddPolySegment.AppendPoint(HGF2DLocation(0, 0, pCoordSys));

    HVE2DShape* p2DShape;
    p2DShape = new HVE2DPolygonOfSegments(AddPolySegment);
    HFCPtr<HVEShape> pShape = new HVEShape(*p2DShape);
    return pShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Creation of a test image
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::CreateExpectedImage(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& prPixelType)
    {
    prImageSample = HRAImageSample::CreateSample(width, height, prPixelType);
    HRAImageBufferPtr pNewBuffer;
    size_t pixelSize = (prPixelType->CountPixelRawDataBits() +7) /8;

    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    pNewBuffer = HRAImageBufferMemory::CreateMemoryBuffer(status, prImageSample->GetWidth()*prImageSample->GetHeight()*pixelSize, prImageSample->GetWidth()*pixelSize, GetMemoryManager());
    prImageSample->SetBuffer(pNewBuffer);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);
    //So basically, every even row the buffer is valid (value of 33) for 1024 pixels
    //and every odd row the buffer is valid only for 1 pixel (33 for the first pixel and then 255 for the other)
    if (prPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                if(row % 2 != 0 && column > 0)
                    {
                    pBufferLine[posBuffer++] = 255;
                    pBufferLine[posBuffer++] = 255;
                    pBufferLine[posBuffer++] = 255;
                    }
                else
                    {
                    pBufferLine[posBuffer++] = (Byte) (33);
                    pBufferLine[posBuffer++] = (Byte) (33);
                    pBufferLine[posBuffer++] = (Byte) (33);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Creation of a RGB test image
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::CreateRGBImage(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, uint32_t r, uint32_t g, uint32_t b)
    {
    HFCPtr<HRPPixelType> pixelTypeV24R8G8B8 = new HRPPixelTypeV24R8G8B8();
    prImageSample = HRAImageSample::CreateSample(width, height, pixelTypeV24R8G8B8);
    HRAImageBufferPtr pNewBuffer;
    size_t pixelSize = (pixelTypeV24R8G8B8->CountPixelRawDataBits() +7) /8;

    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    pNewBuffer = HRAImageBufferMemory::CreateMemoryBuffer(status, prImageSample->GetWidth()*prImageSample->GetHeight()*pixelSize, prImageSample->GetWidth()*pixelSize, GetMemoryManager());
    prImageSample->SetBuffer(pNewBuffer);

    // Input buffer
    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);
    if (pixelTypeV24R8G8B8->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        for(uint32_t row=0; row < height; ++row)
            {
            Byte* pBufferLine = pBuffer+row*pitch;
            uint32_t posBuffer = 0;
            for(uint32_t column=0; column < width; ++column)
                {
                pBufferLine[posBuffer++] = (Byte) (r);
                pBufferLine[posBuffer++] = (Byte) (g);
                pBufferLine[posBuffer++] = (Byte) (b);
                }
            }
        }
    else
        {
        // Pixel type not supported yet by this method
        wchar_t errorMsg[512];
        BeStringUtilities::Snwprintf (errorMsg, L"TEST: (%hs, %hs) - In CreateRGBImage: pixel type not supported.", 
            TestInfo()->test_case_name(), TestInfo()->name());
        FAIL() << errorMsg;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HVEShape HRAImageOpShapingTester::CreateShape_RectInside(double width, double height)
    {
    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVEShape RectShape(width*0.1, height*0.1, width*0.9, height*0.9, pCoordSys);
    return RectShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HVEShape HRAImageOpShapingTester::CreateShape_RectOutside(double width, double height)
    {
    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVEShape RectShape(width*2, height*0.25, width*4, height*0.75, pCoordSys);
    return RectShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HVEShape HRAImageOpShapingTester::CreateShape_RectCorner(double width, double height)
    {
    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVEShape RectShape(width*0.5, height*.5, width*1.5, height*1.5, pCoordSys);
    return RectShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HVEShape HRAImageOpShapingTester::CreateShape_RectDoubleIntersect(double width, double height)
    {
    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVEShape RectShape(width*0.25, -height*.5, width*0.75, height*1.5, pCoordSys);
    return RectShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HVEShape HRAImageOpShapingTester::CreateShape_RectIdentical(double width, double height)
    {
    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVEShape RectShape(0, 0, width, height, pCoordSys);
    return RectShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HVEShape HRAImageOpShapingTester::CreateShape_RectWhole(double width, double height)
    {
    HFCPtr<HGF2DCoordSys> pCoordSys = new HGF2DCoordSys();
    HVEShape RectShape(-width*0.5, -height*.5, width*1.5, height*1.5, pCoordSys);
    return RectShape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Creation of a blue image with a red shape
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAImageOpShapingTester::CreateExpectedImageWithShape(HRAImageSamplePtr& prImageSample, uint32_t width, uint32_t height, HVEShape& prShape)
    {
    HFCPtr<HRPPixelType> pixelTypeV24R8G8B8 = new HRPPixelTypeV24R8G8B8();
    HFCPtr<HRABitmap> outputBitmapExpected = new HRABitmap (width, 
                                                            height, 
                                                            0, 
                                                            prShape.GetCoordSys(),
                                                            pixelTypeV24R8G8B8,
                                                            8,
                                                            HRABitmap::UPPER_LEFT_HORIZONTAL);

    uint32_t red;
    ((Byte*)&red)[0] = 255;
    ((Byte*)&red)[1] = 0;
    ((Byte*)&red)[2] = 0;
    uint32_t blue;
    ((Byte*)&blue)[0] = 0;
    ((Byte*)&blue)[1] = 0;
    ((Byte*)&blue)[2] = 255;

    HRAClearOptions clearOpts;
    clearOpts.SetRawDataValue(&blue);
    outputBitmapExpected->Clear(clearOpts);

    HRAClearOptions clearOptsShape = HRAClearOptions();
    clearOptsShape.SetShape(&prShape);
    clearOptsShape.SetRawDataValue(&red);
    clearOptsShape.SetApplyRasterClipping(true);
    outputBitmapExpected->Clear(clearOptsShape);

    // Output buffer
    prImageSample = HRAImageSample::CreateSample(width, height, pixelTypeV24R8G8B8);
    HRAImageBufferPtr pNewBuffer;
    size_t pixelSize = (pixelTypeV24R8G8B8->CountPixelRawDataBits() +7) /8;

    ImagePPStatus status = IMAGEPP_STATUS_UnknownError;

    pNewBuffer = HRAImageBufferMemory::CreateMemoryBuffer(status, prImageSample->GetWidth()*prImageSample->GetHeight()*pixelSize, prImageSample->GetWidth()*pixelSize, GetMemoryManager());
    prImageSample->SetBuffer(pNewBuffer);

    size_t pitch;
    Byte* pBuffer = prImageSample->GetBufferP()->GetDataP(pitch);
    Byte* pDestBuffer = outputBitmapExpected->GetPacket()->GetBufferAddress();
    uint32_t posBuffer = 0;
    for(uint32_t row=0; row < height; ++row)
        {
        for(uint32_t column=0; column < width; ++column)
            {
            for(uint32_t channel=0; channel < pixelSize; ++channel)
                {
                pBuffer[posBuffer] = pDestBuffer[posBuffer];
                posBuffer++;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jonathan.Bernier  02/2013
*
* Utility method that returns TestInfo
+---------------+---------------+---------------+---------------+---------------+------*/
const ::testing::TestInfo* HRAImageOpShapingTester::TestInfo()
    {
    return ::testing::UnitTest::GetInstance()->current_test_info();
    }

#endif