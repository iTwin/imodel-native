//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageSamplerTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include <ImagePP/all/h/HCDPacketRLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP/all/h/HRAImageOp.h>

// A was hoping to validate the stretch RLE with the old version but it doesn't work
// for 1:1 and it cannot handle clamping.
#ifdef OLD_SURFACE
#include <ImagePP/all/h/HGSMemoryRLESurfaceDescriptor.h>
#include <ImagePP/all/h/HRANearestSamplerRLE1.h>
#endif

#define TEST_NAME ::testing::UnitTest::GetInstance()->current_test_info()->name()
#define TEST_CASE_NAME ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name()
#define TEST_NAME_PRINT L"TEST: (" << TEST_CASE_NAME << ", " << TEST_NAME << ")"



typedef std::vector<HFCPtr<HRPPixelType>>           HRPPixelTypeVector;
typedef std::vector<double>                         ScalingsVector;
typedef std::vector<double>                         OffsetsVector;
typedef std::vector<uint32_t>                         WidthsVector;
typedef std::vector<uint32_t>                         HeightsVector;

// RLE Seed used in tests. make sure width = RLESeed[0]
static std::vector<std::vector<uint16_t> > RLESeed =
    {
    { 15 },
    { 0, 15, 0 },
    { 1, 14, 0 },
    { 1, 13, 1 },
    { 0, 1, 0, 13, 1 },
    { 1, 0, 13, 1, 0 },
    { 1, 1, 1, 2, 1, 1, 8 },
    { 8, 1, 1, 2, 1, 1, 1 }};

// Seed size is prime in length to help prevent constant patterns
uint8_t SeedsBinary[] = { 0, 255, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255 };
uint8_t SeedsUInt8[] = { 0, 255, 255, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255 };
uint16_t SeedsUInt16[]  = {0, USHRT_MAX/3, USHRT_MAX/3*2, USHRT_MAX, USHRT_MAX/2};
int16_t SeedsInt16[]   = {SHRT_MIN, 0, SHRT_MAX/2, SHRT_MAX, SHRT_MAX/3};
uint32_t SeedsUInt32[]  = {0, ULONG_MAX/3, ULONG_MAX/3*2, ULONG_MAX, ULONG_MAX/2};
float   SeedsFloat[]   = {FLT_MIN, 0, FLT_MAX/2, FLT_MAX, 0};



/*=================================================================================**//**
* @bsiclass                         Alexandre.Gagnon                              11/2014           
+===============+===============+===============+===============+===============+======*/
class SamplerTestBufferAllocator: public RefCounted<IImageAllocator>
    {
    public:
    SamplerTestBufferAllocator()
        {
        m_pBuffer = NULL;
        m_size = 0;
        }

    virtual ~SamplerTestBufferAllocator()
        {  
        BeAssert(NULL == m_pBuffer);
        }

    virtual Byte* _AllocMemory(size_t size) override
        {
        BeAssert(NULL == m_pBuffer);
        m_size = size + 4;
        m_pBuffer = new Byte[m_size];
        
        m_pBuffer[0] = 0x65;    //101
        m_pBuffer[1] = 0x29;    //41
        m_pBuffer[m_size-2] = 0x39;//57
        m_pBuffer[m_size-1] = 0xf3;//243

        return m_pBuffer + 2;
        }

    virtual void _FreeMemory(Byte* pi_MemPtr) override
        {
        BeAssert(NULL != m_pBuffer);
        HASSERT(pi_MemPtr - 2 == m_pBuffer);
        HASSERT(m_pBuffer[0] == 0x65);
        HASSERT(m_pBuffer[1] == 0x29);
        HASSERT(m_pBuffer[m_size - 2] == 0x39);
        HASSERT(m_pBuffer[m_size - 1] == 0xf3);

        delete[] m_pBuffer;
        m_pBuffer = NULL;
        }

    Byte*  m_pBuffer;
    size_t m_size;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ImageSampleCreator
    {
    public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FillBufferRLE(HRAImageBufferRleP pImageBuffer, uint32_t width, uint32_t height)
        {

        size_t pitch;
        Byte* pByteBuffer = pImageBuffer->GetDataP(pitch);
        for (uint32_t line = 0; line < RLESeed.size(); line++)
            {
            pImageBuffer->SetLineDataSize(line, RLESeed[line].size()*sizeof(uint16_t));
            uint16_t* pLineBuffer = (uint16_t*)(pByteBuffer + line * pitch);
            
            for (uint32_t column = 0; column < RLESeed[line].size(); ++column)
                {
                pLineBuffer[column] = RLESeed[line][column];
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FillBufferBinary(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, uint8_t const* seeds)
        {
        // We assume BITSPERPIXEL_N1 == 8
        size_t seedsCount = sizeof(seeds) / sizeof(seeds[0]);

        for (uint32_t line = 0; line < height; ++line)
            {
            Byte* pLineBuffer = pBuffer + line * pitch;

            for (uint32_t column = 0; column < width; ++column)
                {
                const uint32_t index =   column / BITSPERPIXEL_N1;
                const uint32_t outBit =  column % BITSPERPIXEL_N1;

                pLineBuffer[index] = (seeds[(line + column) % seedsCount] > 0) ?
                    pLineBuffer[index] |  (0x80 >> outBit) : // Set bit
                    pLineBuffer[index] & ~(0x80 >> outBit);  // Clear bit
                }
            }
        }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    static void FillBuffer1_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
        {
        size_t seedsCount = sizeof(seeds) / sizeof(seeds[0]);

        for (uint32_t line = 0; line < height; ++line)
            {
            Data_T* pLineBuffer = reinterpret_cast<Data_T*>(pBuffer + line * pitch);

            for (uint32_t column = 0; column < width; ++column)
                {
                pLineBuffer[column] = seeds[(line + column) % seedsCount];
                }

            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    static void FillBuffer2_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
        {
        size_t seedsCount = sizeof(seeds) / sizeof(seeds[0]);

        for (uint32_t line = 0; line < height; ++line)
            {
            Data_T* pLineBuffer = reinterpret_cast<Data_T*>(pBuffer + line * pitch);

            for (uint32_t column = 0; column < width; ++column)
                {
                pLineBuffer[column * 2 + 0] = seeds[(line + column * 1) % seedsCount];
                pLineBuffer[column * 2 + 1] = seeds[(line + column * 2) % seedsCount];
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    static void FillBuffer3_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
        {
        size_t seedsCount = sizeof(seeds) / sizeof(seeds[0]);

        for (uint32_t line = 0; line < height; ++line)
            {
            Data_T* pLineBuffer = reinterpret_cast<Data_T*>(pBuffer + line * pitch);

            for (uint32_t column = 0; column < width; ++column)
                {
                pLineBuffer[column * 3 + 0] = seeds[(line + column * 1) % seedsCount];
                pLineBuffer[column * 3 + 1] = seeds[(line + column * 2) % seedsCount];
                pLineBuffer[column * 3 + 2] = seeds[(line + column * 3) % seedsCount];
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    static void FillBuffer4_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
        {
        size_t seedsCount = sizeof(seeds) / sizeof(seeds[0]);

        for (uint32_t line = 0; line < height; ++line)
            {
            Data_T* pLineBuffer = reinterpret_cast<Data_T*>(pBuffer + line * pitch);

            for (uint32_t column = 0; column < width; ++column)
                {
                pLineBuffer[column * 4 + 0] = seeds[(line + column * 1) % seedsCount];
                pLineBuffer[column * 4 + 1] = seeds[(line + column * 2) % seedsCount];
                pLineBuffer[column * 4 + 2] = seeds[(line + column * 3) % seedsCount];
                pLineBuffer[column * 4 + 3] = seeds[(line + column * 4) % seedsCount];
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FillBufferFor(Byte* buffer, HRPPixelType const& pixelType, uint32_t width, uint32_t height, size_t pitch)
        {

        switch(pixelType.GetClassID())
            {
            case HRPPixelTypeId_V1Gray1:
            case HRPPixelTypeId_V1GrayWhite1:
            case HRPPixelTypeId_I1R8G8B8:
            case HRPPixelTypeId_I1R8G8B8A8:
                FillBufferBinary(buffer, width, height, pitch, SeedsBinary);
                break;

            case HRPPixelTypeId_I8Gray8:
            case HRPPixelTypeId_V8Gray8:
            case HRPPixelTypeId_V8GrayWhite8:
            case HRPPixelTypeId_I8R8G8B8Mask:
                FillBuffer1_T<uint8_t>(buffer, width, height, pitch, SeedsUInt8);
                break;

            case HRPPixelTypeId_V16Gray16:
            case HRPPixelTypeId_V16B5G5R5:
            case HRPPixelTypeId_V16R5G6B5:
                FillBuffer1_T<uint16_t>(buffer, width, height, pitch, SeedsUInt16);
                break;

            case HRPPixelTypeId_V16Int16:   
                FillBuffer1_T<int16_t>(buffer, width, height, pitch, SeedsInt16);
                break;

            case HRPPixelTypeId_V16PRGray8A8:
                FillBuffer2_T<uint8_t>(buffer, width, height, pitch, SeedsUInt8);
                break;

            case HRPPixelTypeId_V24B8G8R8:
            case HRPPixelTypeId_V24PhotoYCC:
            case HRPPixelTypeId_V24R8G8B8:
                FillBuffer3_T<uint8_t>(buffer, width, height, pitch, SeedsUInt8);
                break;
            
            case HRPPixelTypeId_V32A8R8G8B8:
            case HRPPixelTypeId_V32PR8PG8PB8A8:
            case HRPPixelTypeId_V32PRPhotoYCCA8:
            case HRPPixelTypeId_V32R8G8B8A8:
            case HRPPixelTypeId_V32B8G8R8X8:
            case HRPPixelTypeId_V32R8G8B8X8:
            case HRPPixelTypeId_V32CMYK:
                FillBuffer4_T<uint8_t>(buffer, width, height, pitch, SeedsUInt8);
                break;

            case HRPPixelTypeId_V48R16G16B16:
                FillBuffer3_T<uint16_t>(buffer, width, height, pitch, SeedsUInt16);
                break;

            case HRPPixelTypeId_V64R16G16B16A16:
            case HRPPixelTypeId_V64R16G16B16X16:
                FillBuffer4_T<uint16_t>(buffer, width, height, pitch, SeedsUInt16);
                break;

            case HRPPixelTypeId_V32Float32:
                FillBuffer1_T<float>(buffer, width, height, pitch, SeedsFloat);
                break;

            case HRPPixelTypeId_V96R32G32B32:
                FillBuffer3_T<uint32_t>(buffer, width, height, pitch, SeedsUInt32);
                break;

            default:
                FAIL() << "Unknown PixelTypeId. We shouldn't get here";
                break;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                  
    +---------------+---------------+---------------+---------------+---------------+------*/
    static HRAImageSamplePtr CreateImage (ImagePPStatus& status, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& pPixelType, SamplerTestBufferAllocator& srcAllocator)
        {
        HRAImageSamplePtr pImageSample = HRAImageSample::CreateSample(status, width, height, pPixelType, srcAllocator);
        if(status != IMAGEPP_STATUS_Success)
            return NULL;        

        if (pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
            pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
            {
            FillBufferRLE(pImageSample->GetBufferRleP(), width, height);
            }
        else
            {
            size_t pitch;
            Byte* pBuffer = pImageSample->GetBufferP()->GetDataP(pitch);
            FillBufferFor(pBuffer, *pPixelType, width, height, pitch);
            }

        return pImageSample;
        }       
    };
    
/*=================================================================================**//**
* @bsiclass                                                   
+===============+===============+===============+===============+===============+======*/
class HRAImageSamplerTester : public ::testing::TestWithParam< ::std::tr1::tuple<HFCPtr<HRPPixelType>, double, double, double, uint32_t, uint32_t> >
    {   
    public:
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static WString CompareImageRle (HRAImageSampleCR image1, HRAImageSampleCR image2)
        {
        HRAImageBufferRleCP pImageBuffer1 = image1.GetBufferRleCP();
        HRAImageBufferRleCP pImageBuffer2 = image2.GetBufferRleCP();

        size_t pitchB1;
        size_t pitchB2;
        const Byte* pBuffer1 = pImageBuffer1->GetDataCP(pitchB1);
        const Byte* pBuffer2 = pImageBuffer2->GetDataCP(pitchB2);
                
        for(uint32_t line = 0; line < image1.GetHeight(); ++line)
            {
            size_t lineDataSize1 = pImageBuffer1->GetLineDataSize(line);
            size_t lineDataSize2 = pImageBuffer2->GetLineDataSize(line);
            if(lineDataSize1 != lineDataSize2)
                {
                WString result;
                result.Sprintf(L"Line dataSize difference at line %i", line);
                return result;
                }
                
            if (0 != memcmp(pBuffer1 + line * pitchB1, pBuffer2 + line * pitchB2, lineDataSize1))
                {
                WString result;
                result.Sprintf(L"Difference at line %i, pixel_ID %i", line, image1.GetPixelType().GetClassID());
                return result;
                }
            }
        return L"";
        }

    protected:
    HRAImageSamplerTester()
        {
        ImagePP::ImageppLib::Initialize(m_imagePPHost);
        };

    virtual ~HRAImageSamplerTester()
        {
        ImagePP::ImageppLib::GetHost().Terminate(false);
        }

    virtual void SetUp() 
        {
        }

    HFCPtr<HRPPixelType> const& GetPixelType() {return ::std::tr1::get<0>(GetParam());}
    
    double const& GetScaling()   {return ::std::tr1::get<1>(GetParam());}
    double const& GetOffsetIn()  {return ::std::tr1::get<2>(GetParam());}
    double const& GetOffsetOut() {return ::std::tr1::get<3>(GetParam());}
    uint32_t const& GetWidth()     {return ::std::tr1::get<4>(GetParam());}
    uint32_t const& GetHeight()    {return ::std::tr1::get<5>(GetParam());}
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    WString CompareImageBinary(HRAImageSampleCR image1, HRAImageSampleCR image2)
        {
        uint32_t width = image1.GetWidth();
        uint32_t height = image2.GetHeight();
        
        size_t pitchB1;
        size_t pitchB2;
        const Byte* pBuffer1 = image1.GetBufferCP()->GetDataCP(pitchB1);
        const Byte* pBuffer2 = image2.GetBufferCP()->GetDataCP(pitchB2);
        
        uint32_t fullBytes = width / BITSPERPIXEL_N1;
        for(uint32_t line = 0; line < height; ++line)
            {
            const Byte* pLineBuffer1 = pBuffer1 + line * pitchB1;
            const Byte* pLineBuffer2 = pBuffer2 + line * pitchB2;
            
            if (0 != memcmp(pLineBuffer1, pLineBuffer2, fullBytes))
                {
                WString result;
                result.Sprintf(L"Difference at line %i", line);
                return result;
                }

            // Remainder
            if (width % BITSPERPIXEL_N1)
                {
                const uint32_t index = fullBytes;
                const uint32_t bitMask = ~(0xFF >> (width - (fullBytes*BITSPERPIXEL_N1)));
                if ((pLineBuffer1[index] & bitMask) != (pLineBuffer2[index] & bitMask))
                    {
                    WString result;
                    result.Sprintf(L"Difference on last Byte of line %i, pixel_ID %i", line, image1.GetPixelType().GetClassID());
                    return result;
                    }
                }
            }
        return L"";
        }
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    WString CompareImage(HRAImageSampleCR image1, HRAImageSampleCR image2)
        {
        uint32_t width = image1.GetWidth();
        uint32_t height = image2.GetHeight();
        
        size_t pitchB1;
        size_t pitchB2;
        const Byte* pBuffer1 = image1.GetBufferCP()->GetDataCP(pitchB1);
        const Byte* pBuffer2 = image2.GetBufferCP()->GetDataCP(pitchB2);
        size_t lineWidthBytes = (image1.GetPixelType().CountPixelRawDataBits()*width + 7) / 8;
        
        for(uint32_t line = 0; line < height; ++line)
            {
            // Line-by-line memory comparison
            if (0 != memcmp(pBuffer1 + line * pitchB1, pBuffer2 + line * pitchB2, lineWidthBytes))
                {
                WString result;
                result.Sprintf(L"Difference at line %i, pixel_ID %i", line, image1.GetPixelType().GetClassID());
                return result;
                }
            }
        return L"";
        }

#ifdef OLD_SURFACE
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  1/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HGSMemoryRLESurfaceDescriptor> CreateMemoryRLESurface(HRAImageSampleR sample)
        {
        HFCPtr<HCDPacketRLE> pPacketRLE = new HCDPacketRLE(sample.GetWidth(), sample.GetHeight());

        size_t pitch;
        Byte const* pData = sample.GetBufferCP()->GetDataCP(pitch);

        for(uint32_t line=0; line < sample.GetHeight(); ++line)
            pPacketRLE->SetLineData(line, pData + line*pitch, sample.GetBufferRleCP()->GetLineDataSize(line));
            
        return new HGSMemoryRLESurfaceDescriptor(sample.GetWidth(), sample.GetHeight(), sample.GetPixelTypePtr(), pPacketRLE, HGF_UPPER_LEFT_HORIZONTAL);
        }
#endif
    

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    WString RunStretchWarpComparison(HRAImageSurfaceR surfIn,
                                  std::unique_ptr<HRAImageSampler>& pSampler, double scaling, double offsetIn, double offsetOut)
        {
        // Create output image
        uint32_t outWidth =  (uint32_t)abs(surfIn.GetWidth()/scaling);
        uint32_t outHeight = (uint32_t)abs(surfIn.GetHeight()/scaling);

        SamplerTestBufferAllocator destAllocatorStretch;
        SamplerTestBufferAllocator destAllocatorWarp;

        // Allocate sample buffers (Normally allocated by ComputeSample before calling stretch/warp)
        ImagePPStatus status;
        HRAImageSamplePtr pImageOutStretch = HRAImageSample::CreateSample(status, outWidth, outHeight, surfIn.GetPixelTypePtr(), destAllocatorStretch);
        if(IMAGEPP_STATUS_Success != status)
            {
            WString result;
            result.Sprintf(L"Stretch CreateSample returned with ERROR=%d", status);
            return result;
            }
        HRAImageSamplePtr pImageOutWarp = HRAImageSample::CreateSample(status, outWidth, outHeight, surfIn.GetPixelTypePtr(), destAllocatorWarp);
        if(IMAGEPP_STATUS_Success != status)
            {
            WString result;
            result.Sprintf(L"Warp CreateSample returned with ERROR=%d", status);
            return result;
            }

        EXPECT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Stretch(*pImageOutStretch, PixelOffset(offsetOut, offsetOut),
                                                                 surfIn,            PixelOffset(offsetIn, offsetIn)));


        EXPECT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Warp(*pImageOutWarp, PixelOffset(offsetOut, offsetOut),
                                                              surfIn,       PixelOffset(offsetIn, offsetIn)));

        // RLE not expected
        BeAssert(!(surfIn.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || surfIn.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID)));               
//         if (surfIn.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || surfIn.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
//             return CompareImageRle(*pImageOutStretch, *pImageOutWarp);

        if(1 == surfIn.GetPixelType().CountPixelRawDataBits())
            return CompareImageBinary(*pImageOutStretch, *pImageOutWarp);
        else
            return CompareImage(*pImageOutStretch, *pImageOutWarp);
        }

    TestImageppLibHost m_imagePPHost;

    }; //END CLASS HRAImageSamplerTester


/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gagnon                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static HRPPixelTypeVector s_GetPixelTypeVector ()
    {
    HRPPixelTypeVector pixelTypeVector;

//  No binary! They need their own test
//     pixelTypeVector.push_back(new HRPPixelTypeI1R8G8B8RLE());
//     pixelTypeVector.push_back(new HRPPixelTypeI1R8G8B8A8RLE());
    pixelTypeVector.push_back(new HRPPixelTypeI1R8G8B8());    //NEEDSWORK stretch N1
    pixelTypeVector.push_back(new HRPPixelTypeI1R8G8B8A8());  //NEEDSWORK stretch N1
    pixelTypeVector.push_back(new HRPPixelTypeV1Gray1());
    pixelTypeVector.push_back(new HRPPixelTypeV1GrayWhite1());

    pixelTypeVector.push_back(new HRPPixelTypeI8Gray8());
    pixelTypeVector.push_back(new HRPPixelTypeI8R8G8B8Mask());
    pixelTypeVector.push_back(new HRPPixelTypeV8Gray8());
    pixelTypeVector.push_back(new HRPPixelTypeV8GrayWhite8());

    pixelTypeVector.push_back(new HRPPixelTypeV16Gray16());
    pixelTypeVector.push_back(new HRPPixelTypeV16Int16());
    pixelTypeVector.push_back(new HRPPixelTypeV16B5G5R5());
    pixelTypeVector.push_back(new HRPPixelTypeV16R5G6B5());
    pixelTypeVector.push_back(new HRPPixelTypeV16PRGray8A8());

    pixelTypeVector.push_back(new HRPPixelTypeV24B8G8R8());
    pixelTypeVector.push_back(new HRPPixelTypeV24PhotoYCC());
    pixelTypeVector.push_back(new HRPPixelTypeV24R8G8B8());

    pixelTypeVector.push_back(new HRPPixelTypeV32A8R8G8B8());
    pixelTypeVector.push_back(new HRPPixelTypeV32PR8PG8PB8A8());
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
* @bsimethod                          Alexandre.Gagnon                            11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRAImageSamplerTester, StretchWarpComparison)
    {
    HFCPtr<HRPPixelType> pPixelType = GetPixelType();

    double scaling = GetScaling();
    double offsetIn = GetOffsetIn();
    double offsetOut = GetOffsetOut();

    uint32_t width = GetWidth();
    uint32_t height = GetHeight();

    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DStretch(HGF2DDisplacement(0, 0), scaling, scaling);

    std::unique_ptr<HRAImageSampler> pSampler;
    
    // no RLE here.
    ASSERT_TRUE(!(pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || pPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID)));
    
    if (pPixelType->CountPixelRawDataBits() == 1)
        {
        pSampler.reset(HRAImageSampler::CreateNearestN1(pModel, *pPixelType));
        }
    else if (pPixelType->CountPixelRawDataBits() % 8 == 0)
        {
        pSampler.reset(HRAImageSampler::CreateNearestN8(pModel, *pPixelType));
        }
    
    ASSERT_TRUE(pSampler.get() != NULL);

    // Create input image
    SamplerTestBufferAllocator srcAllocator;
    ImagePPStatus status;
    HRAImageSamplePtr pImageIn = ImageSampleCreator::CreateImage(status, width, height, pPixelType, srcAllocator);
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    HRAImageSurfacePtr pSurfIn = HRASampleSurface::Create(*pImageIn);

    ASSERT_STREQ(L"", RunStretchWarpComparison(*pSurfIn, pSampler, scaling, offsetIn, offsetOut).c_str());

    // Try bilinear and bicubic using HGF2DStretch transformation model.
    if (pPixelType->CountIndexBits() == 0 && pPixelType->CountPixelRawDataBits() % 8 == 0)
        {
        //// *** bilinear
        pSampler.reset(HRAImageSampler::CreateBilinear(pModel, *pPixelType));
        if (pSampler != NULL)
            ASSERT_STREQ(L"", RunStretchWarpComparison(*pSurfIn, pSampler, scaling, offsetIn, offsetOut).c_str());

        // *** bicubic
        pSampler.reset(HRAImageSampler::CreateBicubic(pModel, *pPixelType));
        if (pSampler != NULL)     // Pixeltypes with nodata value are not supported.
            ASSERT_STREQ(L"", RunStretchWarpComparison(*pSurfIn, pSampler, scaling, offsetIn, offsetOut).c_str());
        }

    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                          Alexandre.Gagnon                            11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(HRAImageSamplerTests,
                        HRAImageSamplerTester,
                        ::testing::Combine(::testing::ValuesIn(s_GetPixelTypeVector ()),
                                           ::testing::Values(-1.0, 0.5, 1.0, 2.0),
                                           ::testing::Values(-15.0, 0.0, 15.0),     // in offset
                                           ::testing::Values(-15.0, 0.0, 15.0),     // out offset
                                           ::testing::Values<uint32_t>(3, 18),        // width
                                           ::testing::Values<uint32_t>(2, 23)         // height
                                           ));

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                   Mathieu.Marchand  1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
class HRAImageSamplerRleTester : public ::testing::TestWithParam<std::tr1::tuple<double, double, double, bool> >
    {
    public:
        HRAImageSamplerRleTester()
        {
        ImagePP::ImageppLib::Initialize(m_imagePPHost);
        };

    virtual ~HRAImageSamplerRleTester()
        {
        ImagePP::ImageppLib::GetHost().Terminate(false);
        }

    double const& GetScaling()   {return ::std::tr1::get<0>(GetParam());}
    double const& GetOffsetIn()  {return ::std::tr1::get<1>(GetParam());}
    double const& GetOffsetOut() {return ::std::tr1::get<2>(GetParam());}
    bool          DoEqualityTest() {return ::std::tr1::get<3>(GetParam());}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  1/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSampleP GetSource()
        {
        if(m_pSource == NULL)
            {
            uint32_t width = RLESeed[0][0]; 
            uint32_t height = (uint32_t)RLESeed.size();

            HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeI1R8G8B8RLE());
            
            // Create input image and surface
            ImagePPStatus status;
            m_pSource = ImageSampleCreator::CreateImage(status, width, height, pPixelType, m_allocator);
            if(status != IMAGEPP_STATUS_Success)
                m_pSource = NULL;
            }

        return m_pSource.get();
        }

#ifdef OLD_SURFACE
//         HRASampleRleSurface* pSamplerRleSurface = dynamic_cast<HRASampleRleSurface*>(&surfIn);
//         if(pSamplerRleSurface != NULL)
//             {
//             HFCPtr<HGSMemoryRLESurfaceDescriptor> pSrc = CreateMemoryRLESurface(pSamplerRleSurface->GetSampleR());
// 
//             HRANearestSamplerRLE1Line legacySampler(*pSrc, HGF2DRectangle(0,0,1,1), scaling, 0.0);
// 
//             size_t outPitch;
//             byte const* pOutData = pImageOutWarp->GetBufferCP()->GetDataCP(outPitch);
// 
//             for(UInt32 line=0; line < pImageOutWarp->GetHeight(); ++line)
//                 {
//                 double xOut, yOut;
//                 pSampler->GetTransfoModel().ConvertDirect(offsetOut + 0.5, line + offsetOut + 0.5, &xOut, &yOut);
//                 xOut -= offsetIn;
//                 yOut -= offsetIn;
// 
//                 legacySampler.GetPixels(xOut, yOut, pImageOutWarp->GetWidth(), (void*)(pOutData + line*outPitch));
//                 // Assume equal to original but we should count.
//                 pImageOutWarp->GetBufferRleP()->SetLineDataSize(line, pImageOutStretch->GetBufferRleCP()->GetLineDataSize(line));
//                 }
//             }
//         else
#endif            
    TestImageppLibHost m_imagePPHost;
    SamplerTestBufferAllocator m_allocator;
    HRAImageSamplePtr m_pSource;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  1/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRAImageSamplerRleTester, RleSamplerTest)
    {
    HRAImageSampleP pSource = GetSource();
    ASSERT_TRUE(pSource != NULL);

    double scaling = GetScaling();
    double offsetIn = GetOffsetIn();
    double offsetOut = GetOffsetOut();

    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DStretch(HGF2DDisplacement(0, 0), scaling, scaling);

    HRAImageSurfacePtr pSrcSurface = HRASampleSurface::Create(*pSource);

    // Create output image
    uint32_t outWidth =  (uint32_t)abs(pSrcSurface->GetWidth()/scaling);
    uint32_t outHeight = (uint32_t)abs(pSrcSurface->GetHeight()/scaling);

    SamplerTestBufferAllocator destAllocatorStretch;
    SamplerTestBufferAllocator destAllocatorWarp;

    // Allocate sample buffers (Normally allocated by ComputeSample before calling stretch/warp)
    ImagePPStatus status;
    HRAImageSamplePtr pOutputStretch = HRAImageSample::CreateSample(status, outWidth, outHeight, pSrcSurface->GetPixelTypePtr(), destAllocatorStretch);
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);
    
    HRAImageSamplePtr pOutputWarp = HRAImageSample::CreateSample(status, outWidth, outHeight, pSrcSurface->GetPixelTypePtr(), destAllocatorWarp);
    ASSERT_EQ(IMAGEPP_STATUS_Success, status);

    // BIAS_EqualWeight 
        {
        std::unique_ptr<HRAImageSampler> pSampler(HRAImageSampler::CreateNearestRle(pModel, pSource->GetPixelType()));
        ASSERT_TRUE(pSampler.get() != NULL);

        // We do not set y offset here the real sampling logic is line by line.
        ASSERT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Stretch(*pOutputStretch, PixelOffset(offsetOut, 0),
                                                                 *pSrcSurface, PixelOffset(offsetIn, 0)));

        ASSERT_TRUE(pOutputStretch->ValidateIntegrity());

        ASSERT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Warp(*pOutputWarp, PixelOffset(offsetOut, 0),
                                                              *pSrcSurface, PixelOffset(offsetIn, 0)));        
        
        ASSERT_TRUE(pOutputWarp->ValidateIntegrity());

        if(DoEqualityTest())
            ASSERT_STREQ(L"", HRAImageSamplerTester::CompareImageRle(*pOutputStretch, *pOutputWarp).c_str());
        } 

    // BIAS_Black. Only supported by stretch 
        {
        HFCPtr<HRPPixelType> pBiasPixelType = (HRPPixelType*)pSource->GetPixelType().Clone();
        ASSERT_TRUE(NULL != pBiasPixelType->Get1BitInterface());
        pBiasPixelType->Get1BitInterface()->SetForegroundState(false);

        std::unique_ptr<HRAImageSampler> pSampler(HRAImageSampler::CreateNearestRle(pModel, *pBiasPixelType));
        ASSERT_TRUE(pSampler.get() != NULL);

        ASSERT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Stretch(*pOutputStretch, PixelOffset(offsetOut, offsetOut),
                                                                 *pSrcSurface, PixelOffset(offsetIn, offsetIn)));

        ASSERT_TRUE(pOutputStretch->ValidateIntegrity());
        } 
    
    // BIAS_White. Only supported by stretch.
        {
        HFCPtr<HRPPixelType> pBiasPixelType = (HRPPixelType*)pSource->GetPixelType().Clone();
        ASSERT_TRUE(NULL != pBiasPixelType->Get1BitInterface());
        pBiasPixelType->Get1BitInterface()->SetForegroundState(true);

        std::unique_ptr<HRAImageSampler> pSampler(HRAImageSampler::CreateNearestRle(pModel, *pBiasPixelType));
        ASSERT_TRUE(pSampler.get() != NULL);

        ASSERT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Stretch(*pOutputStretch, PixelOffset(offsetOut, offsetOut),
                                                                 *pSrcSurface, PixelOffset(offsetIn, offsetIn)));

        ASSERT_TRUE(pOutputStretch->ValidateIntegrity());
        } 
    }
// Some values to do sampler coverage.  No validation is done besides RLE buffer validation.
INSTANTIATE_TEST_CASE_P(Coverage,
                        HRAImageSamplerRleTester,
                        ::testing::Combine(::testing::Values(-2.2, -1.1, -1.8, -0.7, 0.5, 0.25, 0.1, 0.2, 0.3, 0.4, 0.8, 0.9, 1.1, 1.2, 1.5, 1.6,1.9,2.0, 2.5),        // scaling
                                           ::testing::Values(-15.0, -0.8, 0.0, 1, 2.9, 15.0),           // in offset
                                           ::testing::Values(-15.0, -1, 0.0, 1, 4, 15.0),   // out offset
                                           ::testing::Values(false)         // Equality Test 
                                           )); 

// Some tests where stretch and warp will be equal
INSTANTIATE_TEST_CASE_P(Equality1,
                        HRAImageSamplerRleTester,
                        ::testing::Combine(::testing::Values(1.5),        // scaling
                                           ::testing::Values(1.0),           // in offset
                                           ::testing::Values(-2, -1),   // out offset
                                           ::testing::Values(true)         // Equality Test 
                                           )); 

// Some tests where stretch and warp will be equal
INSTANTIATE_TEST_CASE_P(Equality2,
                        HRAImageSamplerRleTester,
                        ::testing::Combine(::testing::Values(1.0, -1.0),        // scaling
                                           ::testing::Values(-15, -14, -13, 13, 14, 15),           // in offset
                                           ::testing::Values(0.0),         // out offset
                                           ::testing::Values(true)         // Equality Test 
                                           )); 

// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  1/2015
// +---------------+---------------+---------------+---------------+---------------+------*/
// class SamplerRleTester : public testing::Test
//     {};
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                   Mathieu.Marchand  1/2015
// +---------------+---------------+---------------+---------------+---------------+------*/
// TEST_F(SamplerRleTester, IdentityTest)
//     {
//     HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeI1R8G8B8A8RLE();
// 
//     HFCPtr<HGF2DTransfoModel> pModel = new HGF2DIdentity();
// 
//     UInt32 width = RLESeed[0][0]; 
//     UInt32 height = (UInt32)RLESeed.size();
// 
//     std::unique_ptr<HRAImageSampler> pSampler(HRAImageSampler::CreateNearestRle(pModel, *pPixelType));
//     ASSERT_TRUE(pSampler.get() != NULL);
// 
//     ImagePPStatus status;
// 
//     // Create input image and surface
//     SamplerTestBufferAllocator srcAllocator;
//     HRAImageSamplePtr pSource = ImageSampleCreator::CreateImage(status, width, height, pPixelType, srcAllocator);
//     ASSERT_EQ(IMAGEPP_STATUS_Success, status);
//     HRAImageSurfacePtr pSourceSurface = HRASampleSurface::Create(*pSource);
//     
//     // Allocate destination sample buffers (Normally allocated by ComputeSample before calling stretch/warp)
//     SamplerTestBufferAllocator destAllocator;    
//     HRAImageSamplePtr pOutput = HRAImageSample::CreateSample(status, width, height, pSourceSurface->GetPixelTypePtr(), destAllocator);
//     ASSERT_EQ(IMAGEPP_STATUS_Success, status);
// 
// 
//     ASSERT_EQ(IMAGEPP_STATUS_Success, pSampler->test_Stretch(*pOutput, PixelOffset(0, 0), *pSourceSurface, PixelOffset(0, 0)));
// 
//     ASSERT_STREQ(L"", HRAImageSamplerTester::CompareImageRle(*pSource, *pOutput).c_str());
//     }



struct RleN1Base
{
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    RleN1Base() {m_allocatorRef = m_allocatorPool.GetAllocatorRef();}

    ~RleN1Base() {m_allocatorRef = NULL;}

    /*---------------------------------------------------------------------------------**//**
    * Create a gray 8 ImageSample.
    * @bsimethod                                    Stephane.Poulin                 09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSamplePtr CreateGrayTestSample(uint32_t srcWidth, uint32_t srcHeight, Byte defaultValue)
        {
        HFCPtr<HRPPixelType> pGrayPixelType = new HRPPixelTypeV8Gray8;
        ImagePPStatus status;
        HRAImageSamplePtr pOutGray = HRAImageSample::CreateSample(status, srcWidth, srcHeight, pGrayPixelType, GetAllocator());
        
        if(IMAGEPP_STATUS_Success != status)
            return NULL;
        
        size_t outPitch;
        Byte* pOutBuffer = pOutGray->GetBufferP()->GetDataP(outPitch);

        for (uint64_t line = 0; line < srcHeight; ++line)
            memset(pOutBuffer + (line*outPitch), defaultValue, srcWidth*sizeof(Byte));

        return pOutGray;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSamplePtr ConvertToGray(HRAImageSampleCR srcSample)
        {
        HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeV8Gray8;
        ImagePPStatus status;
        HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, srcSample.GetWidth(), srcSample.GetHeight(), pPixelType, GetAllocator());

        if(IMAGEPP_STATUS_Success != status)
            return NULL;

        HFCPtr<HRPPixelConverter> pConverter = srcSample.GetPixelType().GetConverterTo(&pOutSample->GetPixelType());

        size_t inPitch;
        Byte const* pInData = srcSample.GetBufferCP()->GetDataCP(inPitch);
        size_t outPitch;
        Byte* pOutData = pOutSample->GetBufferP()->GetDataP(outPitch);
        for (uint64_t y = 0; y < srcSample.GetHeight(); ++y)
            {
            pConverter->Convert(pInData+y*inPitch, pOutData+y*outPitch, srcSample.GetWidth());
            }
        return pOutSample;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSamplePtr ConvertToRle(HRAImageSampleCR srcSample)
        {
        HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeI1R8G8B8RLE;
        ImagePPStatus status;
        HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, srcSample.GetWidth(), srcSample.GetHeight(), pPixelType, GetAllocator());

        if(IMAGEPP_STATUS_Success != status)
            return NULL;

        HFCPtr<HRPPixelConverter> pConverter = srcSample.GetPixelType().GetConverterTo(&pOutSample->GetPixelType());

        size_t inPitch;
        Byte const* pInData = srcSample.GetBufferCP()->GetDataCP(inPitch);
        size_t outPitch;
        Byte* pOutData = pOutSample->GetBufferP()->GetDataP(outPitch);
        HRAImageBufferRleP pBufRle = dynamic_cast<HRAImageBufferRleP>(pOutSample->GetBufferP());

        for (uint32_t y = 0; y < srcSample.GetHeight(); ++y)
            {
            pConverter->Convert(pInData+y*inPitch, pOutData+y*outPitch, srcSample.GetWidth());

            // Compute size
            uint16_t* pBuf = (uint16_t*)(pOutData + y*outPitch);
            uint16_t* pRun = (uint16_t*)(pOutData + y*outPitch);
            uint32_t count = 0;
            while (count < srcSample.GetWidth())
                {
                count += *pRun;
                ++pRun;
                }

            pBufRle->SetLineDataSize(y, (pRun - pBuf)*sizeof(uint16_t));
            }
        return pOutSample;
        }


    /*---------------------------------------------------------------------------------**//**
    * Compare 2 8 bits buffer (gray 8)
    * @bsimethod                                    Stephane.Poulin                 09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool CompareGrayBuffer(std::wstring& errorMsg, Byte const* pBase, size_t basePitch, Byte const* pBuffer, size_t bufferPitch, uint32_t width, uint32_t height)
        {
        errorMsg.clear();

        for (uint32_t line = 0; line < height; ++line)
            {
            Byte const* pBaseLine = (pBase + basePitch*line);
            Byte const* pLineBuffer = (pBuffer + bufferPitch*line);

            for (uint32_t col = 0; col < width; ++col)
                {
                if (pBaseLine[col] != pLineBuffer[col])
                    {
                    wchar_t tempErrorMsg[512];
                    BeStringUtilities::Snwprintf(tempErrorMsg, L"Pixel at (%d, %d):\n  Actual: %d\n  Expected: %d",
                        line, col, pLineBuffer[col], pBaseLine[col]);

                    errorMsg = tempErrorMsg;
                    return false;
                    }
                }
            }

        return true;
        }


protected:
    IImageAllocatorR GetAllocator()          { return m_allocatorRef->GetAllocator(); }

private:
    ImageAllocatorPool m_allocatorPool;
    ImageAllocatorRefPtr m_allocatorRef;

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::vector<std::vector<uint16_t> > ImageVector;
class CopyPixelsRLETester : /*public testing::Test*/
                            public ::testing::TestWithParam< ::std::tr1::tuple< int64_t, int64_t, uint32_t, uint32_t, ImageVector> >,
                            public RleN1Base
    {
    public:
        struct LineReader
            {
            LineReader(HFCPtr<HCDPacketRLE> const& packet) : m_packet(packet){}

            Byte const* GetLineDataCP(uint32_t line) const { return m_packet->GetLineBuffer(line); }
            Byte const* GetLineDataCP(uint32_t line, size_t& dataSize) const { dataSize = m_packet->GetLineDataSize(line); return m_packet->GetLineBuffer(line); }

            HFCPtr<HCDPacketRLE> m_packet;
            };

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        static std::vector<ImageVector> GetTestBuffers()
            {
            std::vector<ImageVector> testImages;
            ImageVector RLETestBuf =
                {
                    { 15 },
                    { 0, 15, 0 },
                    { 1, 14, 0 },
                    { 1, 13, 1 },
                    { 0, 1, 0, 13, 1 },
                    { 1, 0, 13, 1, 0 },
                    { 1, 1, 1, 2, 1, 1, 8 },
                    { 8, 1, 1, 2, 1, 1, 1 }
                };


            ImageVector RLETestBuf1 =
                {
                    { 32767 },
                    { 0, 32767, 0 },
                    { 1, 32766, 0 },
                    { 1, 32765, 1 },
                    { 0, 1, 0, 32765, 1 },
                    { 1, 0, 32765, 1, 0 },
                    { 1, 1, 1, 1, 1, 1, 32761 },
                    { 10, 2, 5, 1, 3, 7, 32739 }
                };

            testImages.push_back(RLETestBuf);
            testImages.push_back(RLETestBuf1);

            return testImages;
            }

    protected:
        CopyPixelsRLETester() {};
        ~CopyPixelsRLETester(){};

        PixelOffset64      GetInputOffset() const  { return PixelOffset64(::std::tr1::get<0>(GetParam()), ::std::tr1::get<1>(GetParam()));}
        uint32_t           GetOutputWidth() const  { return ::std::tr1::get<2>(GetParam()); }
        uint32_t           GetOutputHeight() const { return ::std::tr1::get<3>(GetParam()); }
        ImageVector const& GetTestBuffer() const   { return ::std::tr1::get<4>(GetParam()); }

        /*---------------------------------------------------------------------------------**//**
        * Convert RLE line buffer to  gray 8.
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        HRAImageSamplePtr ConvertRleToGray(uint32_t inWidth, uint32_t inHeight, HCDPacketRLE const& pInData)
            {
            HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeV8Gray8;
            ImagePPStatus status;
            HRAImageSamplePtr pOut = HRAImageSample::CreateSample(status, inWidth, inHeight, pPixelType, GetAllocator());

            size_t outPitch;
            Byte* pOutBuffer = pOut->GetBufferP()->GetDataP(outPitch);

            for (uint32_t line = 0; line < inHeight; ++line)
                {
                Byte* pOutLine = pOutBuffer + line * outPitch;
                uint16_t const* pInLine = (uint16_t*)pInData.GetLineBuffer(line);

                uint64_t idx = 0;
                uint64_t count = 0;
                while (count < inWidth)
                    {
                    Byte color = (idx & 0x00000001) ? 255 : 0;

                    memset (pOutLine+count, color, pInLine[idx]);
                    count += pInLine[idx++];
                    }
                }

            return pOut;
            }

        /*---------------------------------------------------------------------------------**//**
        * Convert RLE (constant pitch) buffer to  gray 8.
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        HRAImageSamplePtr ConvertRleToGray(uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch)
            {
            HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeV8Gray8;
            ImagePPStatus status;
            HRAImageSamplePtr pOut = HRAImageSample::CreateSample(status, inWidth, inHeight, pPixelType, GetAllocator());

            size_t outPitch;
            Byte* pOutBuffer = pOut->GetBufferP()->GetDataP(outPitch);

            for (uint32_t line = 0; line < inHeight; ++line)
                {
                Byte* pOutLine = pOutBuffer + line * outPitch;
                uint16_t const* pInLine = (uint16_t const*)(pInData + line * inPitch);

                uint64_t idx = 0;
                uint64_t count = 0;
                while (count < inWidth)
                    {
                    Byte color = (idx & 0x00000001) ? 255 : 0;

                    memset (pOutLine+count, color, pInLine[idx]);
                    count += pInLine[idx++];
                    }
                }

            return pOut;
            }

        /*---------------------------------------------------------------------------------**//**
        * Create and initialize a RLE (constant pitch) ImageSample.
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        HRAImageSamplePtr CreateRlePitchTestSample(uint32_t srcWidth, uint32_t srcHeight)
            {
            HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeI1R8G8B8RLE();
            ImagePPStatus status;
            HRAImageSamplePtr pOut = HRAImageSample::CreateSample(status, srcWidth, srcHeight, pPixelType, GetAllocator());
                        
            size_t outPitch;
            Byte* pOutBuffer = pOut->GetBufferP()->GetDataP(outPitch);
            for (uint64_t line = 0; line < srcHeight; ++line)
                memset (pOutBuffer + line*outPitch, 0xFF, outPitch);

            return pOut;
            }

        /*---------------------------------------------------------------------------------**//**
        * Create and initialize a RLE (line) packet.
        * @bsimethod                                    Stephane.Poulin                 09/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        HFCPtr<HCDPacketRLE> CreatePacketRleTestSample(uint32_t srcWidth, uint32_t srcHeight, ImageVector const& buffer)
            {
            HFCPtr<HCDPacketRLE> pPacketRle = new HCDPacketRLE(srcWidth, srcHeight);
            pPacketRle->SetBufferOwnership(true);
            for (uint32_t line = 0; line < srcHeight; ++line)
                pPacketRle->SetLineData(line, (Byte*)&(buffer[line][0]), buffer[line].size()*sizeof(uint16_t));

            return pPacketRle;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t ComputeWidth(std::vector<uint16_t> const& line) const
            {
            uint32_t sum=0;
            for (uint16_t value : line)
                sum = sum + value;

            return sum;
            }

    private:
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(CopyPixelsRLETester, CopyPixels_identityB)
    {
    ImageVector const& testBuf = GetTestBuffer();
    uint32_t srcWidth = ComputeWidth(testBuf[0]);
    uint32_t srcHeight = (uint32_t)testBuf.size();

    // Allocate and init source
    HFCPtr<HCDPacketRLE> pInPacketRle = CreatePacketRleTestSample(srcWidth, srcHeight, testBuf);
    // Convert the RLE1 source to gray
    HRAImageSamplePtr pInGray = ConvertRleToGray(srcWidth, srcHeight, *pInPacketRle);

    // Allocate and init destination
    uint32_t outWidth = GetOutputWidth();
    uint32_t outHeight = GetOutputHeight();
    HRAImageSamplePtr pOutRleWithPitch = CreateRlePitchTestSample(outWidth, outHeight); 
    HRAImageBufferRleP pOutRleBuf = dynamic_cast<HRAImageBufferRleP>(pOutRleWithPitch->GetBufferP()); 

    // Execute copy pixels with RLE1->RLEPitch
    LineReader lineReader(pInPacketRle);
    PixelOffset64 const offset(GetInputOffset());
    CopyPixelsRle_T(pOutRleWithPitch->GetWidth(), pOutRleWithPitch->GetHeight(), *pOutRleBuf, srcWidth, srcHeight, lineReader, offset);
    pInPacketRle = 0;

    // Repeat the same test with the same input but converted in gray
    size_t inGrayPitch;
    Byte* inGrayBuf = pInGray->GetBufferP()->GetDataP(inGrayPitch);

    // Allocate output
    HRAImageSamplePtr pOutGray = CreateGrayTestSample(outWidth, outHeight, 0x00);
    size_t outGrayPitch;
    Byte* outGrayBuf = pOutGray->GetBufferP()->GetDataP(outGrayPitch);

    // Execute copy pixels with gray->gray
    ASSERT_EQ(IMAGEPP_STATUS_Success, CopyPixelsN8(pOutGray->GetWidth(), pOutGray->GetHeight(), outGrayBuf, outGrayPitch, pInGray->GetWidth(), pInGray->GetHeight(), inGrayBuf, inGrayPitch, 1, offset));
    pInGray = 0;

    // Generate 8 bits version of the RLEPitch output
    size_t pitch;
    Byte* buf = pOutRleWithPitch->GetBufferP()->GetDataP(pitch);
    HRAImageSamplePtr pRlePitchToGray = ConvertRleToGray(outWidth, outHeight, buf, pitch);

    // Compare both output
    std::wstring errorMsg;
    size_t pitch1;
    Byte* buf1 = NULL;
    buf1 = pRlePitchToGray->GetBufferP()->GetDataP(pitch1);
    buf  = pOutGray->GetBufferP()->GetDataP(pitch);

    ASSERT_TRUE(CompareGrayBuffer(errorMsg, buf1, pitch1, buf, pitch, pRlePitchToGray->GetWidth(), pRlePitchToGray->GetHeight())) << errorMsg.c_str();
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                            
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(CopyPixelsRLETests,
                        CopyPixelsRLETester,
                        ::testing::Combine(
                            ::testing::Values(-1000, -1.1, -1.0, -0.9, /*-0.5,*/-0.25, 0, 0.9, 1, 239, 240, 555), // X input Offset
                            ::testing::Values(-300,-1, 0, 1, 300),                                                // Y input Offset
                            ::testing::Values(1, 255, 256), // Output width
                            ::testing::Values(1, 255, 256), // Output height
                            ::testing::ValuesIn(CopyPixelsRLETester::GetTestBuffers())
                        ));


#define BitMask(bit) (0x01 << (bit))

class EditorN1Tester : public testing::Test, RleN1Base
    {
protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSamplePtr CreateSampleGray8(uint32_t srcWidth, uint32_t srcHeight, Byte defaultValue)
        {
        HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeV8Gray8;
        ImagePPStatus status;
        HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, srcWidth, srcHeight, pPixelType, GetAllocator());
        
        size_t pitch;
        Byte* pData = pOutSample->GetBufferP()->GetDataP(pitch);
        memset(pData, defaultValue, pitch*pOutSample->GetHeight());

        return pOutSample;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSamplePtr CreateSampleN1(uint32_t srcWidth, uint32_t srcHeight, Byte defaultValue)
        {
        HFCPtr<HRPPixelType> pPixelType = new HRPPixelTypeI1R8G8B8;
        ImagePPStatus status;
        HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, srcWidth, srcHeight, pPixelType, GetAllocator());
        
        size_t pitch;
        Byte* pData = pOutSample->GetBufferP()->GetDataP(pitch);
        memset(pData, defaultValue, pitch*pOutSample->GetHeight());
        return pOutSample;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HRAImageSamplePtr ClearSample(HRAImageSampleR srcSample)
        {
        HRASampleSurfacePtr pSurf = HRASampleSurface::Create(srcSample);
        pSurf->Clear();

        return &pSurf->GetSampleR();
        }


    public:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void BitPatternTest()
            {
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 0>::value) == 0);
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 1>::value) == BitMask(0));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 2>::value) == (BitMask(0) | BitMask(1)));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 3>::value) == (BitMask(0) | BitMask(1) | BitMask(2)));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 4>::value) == (BitMask(0) | BitMask(1) | BitMask(2) | BitMask(3)));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 5>::value) == (BitMask(0) | BitMask(1) | BitMask(2) | BitMask(3) | BitMask(4)));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 6>::value) == (BitMask(0) | BitMask(1) | BitMask(2) | BitMask(3) | BitMask(4) | BitMask(5)));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 7>::value) == (BitMask(0) | BitMask(1) | BitMask(2) | BitMask(3) | BitMask(4) | BitMask(5) | BitMask(6)));
            ASSERT_TRUE((N1Manip::RightBitPattern<Byte, 8>::value) == (BitMask(0) | BitMask(1) | BitMask(2) | BitMask(3) | BitMask(4) | BitMask(5) | BitMask(6) | BitMask(7)));


            ASSERT_TRUE(N1Manip::GetLeftBitPattern(1) == (BitMask(7)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(2) == (BitMask(7) | BitMask(6)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(3) == (BitMask(7) | BitMask(6) | BitMask(5)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(4) == (BitMask(7) | BitMask(6) | BitMask(5) | BitMask(4)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(5) == (BitMask(7) | BitMask(6) | BitMask(5) | BitMask(4) | BitMask(3)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(6) == (BitMask(7) | BitMask(6) | BitMask(5) | BitMask(4) | BitMask(3) | BitMask(2)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(7) == (BitMask(7) | BitMask(6) | BitMask(5) | BitMask(4) | BitMask(3) | BitMask(2) | BitMask(1)));
            ASSERT_TRUE(N1Manip::GetLeftBitPattern(8) == (BitMask(7) | BitMask(6) | BitMask(5) | BitMask(4) | BitMask(3) | BitMask(2) | BitMask(1) | BitMask(0)));
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void GetPixelTest()
            {
            uint32_t width = 31;
            uint32_t height = 31;
            uint32_t outHeight = 1;

            std::vector<uint32_t> offsets = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
            std::vector <uint32_t> outWidths = { 1, 2, 3, 4, 5, 6, 7, 8, 31, 127 };

            for (auto offset : offsets)
                {
                for (auto outWidth : outWidths)
                    {
                    outWidth = MIN(outWidth, width - (uint32_t)offset);
                    HRAImageSamplePtr srcSampleN1 = CreateSampleN1(width, height, 0xAA);
                    HRASampleN1SurfacePtr srcSurface = dynamic_cast<HRASampleN1SurfaceP>(HRASampleSurface::Create(*srcSampleN1).get());

                    size_t datasize;
                    HRASampleN1Surface::ImageEditor editorN1(*srcSurface);
                    Byte const* pixels = editorN1.GetPixels(datasize, offset, offset, outWidth);

                    HRAImageSamplePtr outSampleN1 = CreateSampleN1(outWidth, outHeight, 0x00);
                    size_t outPitchN1;
                    Byte* pOutBuffer = outSampleN1->GetBufferP()->GetDataP(outPitchN1);

                    memcpy(pOutBuffer, pixels, datasize);

                    HRAImageSamplePtr outGraySampleFromN1 = ConvertToGray(*outSampleN1);
                    size_t outGraySamplePitchFromN1;
                    Byte* pOutGrayBufferFromN1 = outGraySampleFromN1->GetBufferP()->GetDataP(outGraySamplePitchFromN1);

                    // Generate base (Gray) from srcN1
                    HRAImageSamplePtr baseGraySample = ConvertToGray(*srcSampleN1);
                    HRAImageSamplePtr outGraySample = CreateSampleGray8(outWidth, outHeight, 0x00);

                    size_t outPitchGray;
                    Byte* pOutBufferGray = outGraySample->GetBufferP()->GetDataP(outPitchGray);

                    HRASampleN8Surface::ImageEditor editorN8(*baseGraySample);
                    pixels = editorN8.GetPixels(datasize, offset, offset, outWidth);
                    memcpy(pOutBufferGray, pixels, datasize);

                    // Compare both gray output 
                    std::wstring errorMsg;
                    ASSERT_TRUE(CompareGrayBuffer(errorMsg, pOutBufferGray, outPitchGray, pOutGrayBufferFromN1, outGraySamplePitchFromN1, outSampleN1->GetWidth(), outSampleN1->GetHeight()));
                    }
                }
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetPixelTest()
            {
            uint32_t srcWidth = 15;
            uint32_t srcHeight = 1;
            HRAImageSamplePtr pSrcSampleN1 = CreateSampleN1(srcWidth, srcHeight, 0xFF);
            HRAImageSamplePtr pSrcSampleGray = ConvertToGray(*pSrcSampleN1);

            size_t srcPitchN1;
            Byte const* pSrcBufN1 = pSrcSampleN1->GetBufferCP()->GetDataCP(srcPitchN1);
            size_t srcPitchGray;
            Byte const* pSrcBufGray = pSrcSampleGray->GetBufferCP()->GetDataCP(srcPitchGray);


            std::vector<uint32_t> offsets = {0,1,2,3,4,5,6,7,9};
            for (auto offset : offsets)
                {
                uint32_t width = 32;
                uint32_t height = 1;

                HRAImageSamplePtr pOutSampleN1 = CreateSampleN1(width, height, 0x00);
                HRASampleN1SurfacePtr pOutSurfaceN1 = dynamic_cast<HRASampleN1SurfaceP>(HRASampleSurface::Create(*pOutSampleN1.get()).get());
                ImageEditorN1 editorN1(*pOutSurfaceN1);

                uint32_t pixelCount = pSrcSampleN1->GetWidth();
                editorN1.SetPixels(offset, 0, pixelCount, pSrcBufN1, (pixelCount + 7) / 8);

                // Convert the result to Gray
                HRAImageSamplePtr pOutSampleN1ToGray = ConvertToGray(*pOutSampleN1);
                
                // Repeat the operation using Gray sample
                HRAImageSamplePtr pOutSampleGray = CreateGrayTestSample(width, height, 0x00);
                HRASampleN8SurfacePtr pOutSurfaceGray = dynamic_cast<HRASampleN8SurfaceP>(HRASampleSurface::Create(*pOutSampleGray.get()).get());
                ImageEditorN8 editorN8(*pOutSurfaceGray);

                editorN8.SetPixels(offset, 0, pixelCount, pSrcBufGray, pixelCount);

                // Compare the result
                std::wstring errorMsg;
                size_t pitch1, pitch2;
                Byte const* pBuf1 = pOutSampleN1ToGray->GetBufferCP()->GetDataCP(pitch1);
                Byte const* pBuf2 = pOutSampleGray->GetBufferP()->GetDataP(pitch2);

                ASSERT_TRUE(CompareGrayBuffer(errorMsg, pBuf1, pitch1, pBuf2, pitch2, pOutSampleN1->GetWidth(), pOutSampleN1->GetHeight()));
                }
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Stephane.Poulin                 10/2014
        +---------------+---------------+---------------+---------------+---------------+------*/
        void CopyPixelN1Test()
            {
            uint32_t width = 31;
            uint32_t height = 31;

            std::vector<uint32_t> outSizes = { 1, 2, 3, 7, 9, 13, 15, 31, 127, 128, 256, 257 };
            std::vector<PixelOffset64> offsets = { { 0, 0 }, { 1, 1 }, { -1, 0 }, { -1, -1 }, { 30, 0 }, { 30, 30 }, { -30, -30 } };

            for (auto size : outSizes)
                {
                uint32_t outWidth = size;
                uint32_t outHeight = size;
                for (auto offset : offsets)
                    {
                    HRAImageSamplePtr srcSampleN1 = CreateSampleN1(width, height, 0xAA);
                    size_t srcPitchN1;
                    Byte const* pInBuffer = srcSampleN1->GetBufferP()->GetDataP(srcPitchN1);

                    HRAImageSamplePtr outSampleN1 = CreateSampleN1(outWidth, outHeight, 0x00);
                    size_t outPitchN1;
                    Byte* pOutBuffer = outSampleN1->GetBufferP()->GetDataP(outPitchN1);

                    CopyPixelsN1(outSampleN1->GetWidth(), outSampleN1->GetHeight(), pOutBuffer, outPitchN1, srcSampleN1->GetWidth(), srcSampleN1->GetHeight(), pInBuffer, srcPitchN1, offset);

                    HRAImageSamplePtr outGraySampleFromN1 = ConvertToGray(*outSampleN1);
                    size_t outPitchFromN1;
                    Byte* pOutBufferFromN1 = outGraySampleFromN1->GetBufferP()->GetDataP(outPitchFromN1);

                    // Generate base (Gray) from srcN1
                    HRAImageSamplePtr baseGraySample = ConvertToGray(*srcSampleN1);
                    HRAImageSamplePtr outGraySample = CreateSampleGray8(outWidth, outHeight, 0x00);

                    size_t inPitchGray, outPitchGray;
                    Byte const* pInBufferGray = baseGraySample->GetBufferCP()->GetDataCP(inPitchGray);
                    Byte* pOutBufferGray = outGraySample->GetBufferP()->GetDataP(outPitchGray);

                    CopyPixelsN8(outGraySample->GetWidth(), outGraySample->GetHeight(), pOutBufferGray, outPitchGray, baseGraySample->GetWidth(), baseGraySample->GetHeight(), pInBufferGray, inPitchGray, 1, offset);

                    // Compare both gray output 
                    std::wstring errorMsg;
                    ASSERT_TRUE(CompareGrayBuffer(errorMsg, pOutBufferGray, outPitchGray, pOutBufferFromN1, outPitchFromN1, outSampleN1->GetWidth(), outSampleN1->GetHeight()));
                    }
                }
            }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 10/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ClearPixelsTest()
        {
        std::vector<uint32_t> sizes = { 1, 7, 9, 13, 15, 127, 128, 256, 257 };
        std::vector<Byte> initValues = { 0x00, 0xFF, 0xAA, 0x55, 0x81 };

        for (auto size : sizes)
            {
            for (auto value : initValues)
                {
                uint32_t width = size;
                uint32_t height = size;

                // Create input sample
                HRAImageSamplePtr srcSampleN1 = CreateSampleN1(width, height, value);
                HRAImageSamplePtr srcSampleN8 = ConvertToGray(*srcSampleN1);
                HRAImageSamplePtr srcSampleRle = ConvertToRle(*srcSampleN8);

                // Test Clear N1
                ClearSample(*srcSampleN1);

                // test Clear N8
                ClearSample(*srcSampleN8);

                // Test Clear Rle1
                ClearSample(*srcSampleRle);

                // Convert the results to Gray
                HRAImageSamplePtr srcSampleN1ToGray = ConvertToGray(*srcSampleN1);
                HRAImageSamplePtr srcSampleN8ToGray = ConvertToGray(*srcSampleN8);
                HRAImageSamplePtr srcSampleRleToGray = ConvertToGray(*srcSampleRle);

                std::wstring errorMsg;
                size_t pitch1, pitch2;
                Byte const* pBuf1 = srcSampleN1ToGray->GetBufferCP()->GetDataCP(pitch1);
                Byte const* pBuf2 = srcSampleN8ToGray->GetBufferP()->GetDataP(pitch2);

                ASSERT_TRUE(CompareGrayBuffer(errorMsg, pBuf1, pitch1, pBuf2, pitch2, srcSampleN1->GetWidth(), srcSampleN1->GetHeight()));

                pBuf2 = srcSampleRleToGray->GetBufferP()->GetDataP(pitch2);
                ASSERT_TRUE(CompareGrayBuffer(errorMsg, pBuf1, pitch1, pBuf2, pitch2, srcSampleN1->GetWidth(), srcSampleN1->GetHeight()));
                }
            }
        }


    EditorN1Tester() {}
    ~EditorN1Tester() {}

    private:
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(EditorN1Tester, EditorN1TesterTest)
    {
    BitPatternTest();
    GetPixelTest();
    SetPixelTest();
    CopyPixelN1Test();
    ClearPixelsTest();
    }

