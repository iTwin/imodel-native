//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/DownSamplingTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include <ImagePP/all/h/HRAImageOp.h>

//&&SP Adapt to DownSampler changes. 
#if 0

#define TEST_NAME ::testing::UnitTest::GetInstance()->current_test_info()->name()
#define TEST_CASE_NAME ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name()
#define TEST_NAME_PRINT L"TEST: (" << TEST_CASE_NAME << ", " << TEST_NAME << ")"



typedef std::vector<HFCPtr<HRPPixelType>>           HRPPixelTypeVector;

/*=================================================================================**//**
* @bsiclass                         Alexandre.Gagnon                              11/2014           
+===============+===============+===============+===============+===============+======*/
class DSTestBufferAllocator: public RefCounted<IImageAllocator>
    {
    public:
        DSTestBufferAllocator()
        {
        m_pBuffer = NULL;
        m_size = 0;
        }

        virtual ~DSTestBufferAllocator()
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
* @bsiclass                         Alexandre.Gagnon                              11/2014           
+===============+===============+===============+===============+===============+======*/
class DownSamplingTester : public ::testing::TestWithParam< ::std::tr1::tuple<HFCPtr<HRPPixelType>, uint32_t> >
    {   
    protected:
    DownSamplingTester()
        {
        };

    virtual void SetUp() 
        {
        }

    HFCPtr<HRPPixelType> const& GetPixelType()  { return ::std::tr1::get<0>(GetParam()); }
    uint32_t const& GetTileSquareSize()           { return ::std::tr1::get<1>(GetParam()); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    void FillBuffer1_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
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
    void FillBuffer3_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
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
    void FillBuffer4_T(Byte* pBuffer, uint32_t width, uint32_t height, size_t pitch, Data_T const* seeds)
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
    * @bsimethod                                                  
    +---------------+---------------+---------------+---------------+---------------+------*/
    void FillInputImage(HRAImageSamplePtr& pImageSample, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& pPixelType)
        {
        size_t pitch;
        Byte* pBuffer = pImageSample->GetBufferP()->GetDataP(pitch);

        // Seed size is prime in length to help prevent constant patterns
        uint8_t SeedsUInt8[] = { 255, 255, 255, 255, 255, 255, 0, 0, 0, 255, 255, 0, 0};
        uint16_t SeedsUInt16[] = { 0, USHRT_MAX / 3, USHRT_MAX / 3 * 2, USHRT_MAX, USHRT_MAX / 2 };
        int16_t SeedsInt16[] = { SHRT_MIN, 0, -(SHRT_MAX-1) / 2, SHRT_MAX, -SHRT_MAX / 3 };
        uint32_t SeedsUInt32[] = { 0, ULONG_MAX / 3, ULONG_MAX / 3 * 2, ULONG_MAX, ULONG_MAX / 2 };

        switch (pPixelType->GetClassID())
            {
            case HRPPixelTypeId_I8Gray8:
                FillBuffer1_T<uint8_t>(pBuffer, width, height, pitch, SeedsUInt8);
                break;

            case HRPPixelTypeId_V16Int16:
                FillBuffer1_T<int16_t>(pBuffer, width, height, pitch, SeedsInt16);
                break;

            case HRPPixelTypeId_V24R8G8B8:
                FillBuffer3_T<uint8_t>(pBuffer, width, height, pitch, SeedsUInt8);
                break;

            case HRPPixelTypeId_V32R8G8B8A8:
                FillBuffer4_T<uint8_t>(pBuffer, width, height, pitch, SeedsUInt8);
                break;

            case HRPPixelTypeId_V48R16G16B16:
                FillBuffer3_T<uint16_t>(pBuffer, width, height, pitch, SeedsUInt16);
                break;
            
            case HRPPixelTypeId_V64R16G16B16A16:
                FillBuffer4_T<uint16_t>(pBuffer, width, height, pitch, SeedsUInt16);
                break;

            case HRPPixelTypeId_V96R32G32B32:
                FillBuffer3_T<uint32_t>(pBuffer, width, height, pitch, SeedsUInt32);
                break;

            default:
                FAIL() << "Unknown PixelTypeId. We shouldn't get here";
                break;
            }

        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    WString CompareImage(const HRAImageSamplePtr pImage1, const HRAImageSamplePtr pImage2, HFCPtr<HRPPixelType>& pPixelType,
                         uint32_t compareWidth, uint32_t compareHeight)
        {
        size_t pitchB1;
        size_t pitchB2;
        const Byte* pBuffer1 = pImage1->GetBufferP()->GetDataCP(pitchB1);
        const Byte* pBuffer2 = pImage2->GetBufferP()->GetDataCP(pitchB2);
        size_t lineWidthBytes = (pPixelType->CountPixelRawDataBits()*compareWidth + 7) / 8;
        
        for(uint32_t line = 0; line < compareHeight; ++line)
            {
            // Line-by-line memory comparison
            if (0 != memcmp(pBuffer1 + line * pitchB1, pBuffer2 + line * pitchB2, lineWidthBytes))
                {
                WString result;
                result.Sprintf(L"Difference at line %i", line);
                return result;
                }
            }
        return L"";
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ComputeOptimizedDownSampler(HRAImageSamplePtr pImageIn, uint32_t width, uint32_t height,
                                     HRAImageSamplePtr& pImageOutDownSize, HFCPtr<HRPPixelType>& pPixelType)
        {
        std::unique_ptr<OptimizedDownSampler> pDownSampler;
        pDownSampler.reset(CreateOptimizedDownSampler(width, height, *pPixelType.GetPtr(), HGSResampling(HGSResampling::NEAREST_NEIGHBOUR)));

        size_t inPitch;
        Byte* pInBuffer = pImageIn->GetBufferP()->GetDataP(inPitch);

        size_t outPitch;
        Byte* pOutBuffer = pImageOutDownSize->GetBufferP()->GetDataP(outPitch);
        pDownSampler->_Compute(pOutBuffer, pInBuffer);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ComputeBoundOptimizedDownSampler(HRAImageSamplePtr pImageIn, uint32_t width, uint32_t height,
                                     HRAImageSamplePtr& pImageOutDownSize, HFCPtr<HRPPixelType>& pPixelType)
        {
        std::unique_ptr<OptimizedDownSampler> pDownSampler;
        pDownSampler.reset(CreateOptimizedDownSampler(width, height, *pPixelType.GetPtr(), HGSResampling(HGSResampling::NEAREST_NEIGHBOUR)));

        size_t inPitch;
        Byte* pInBuffer = pImageIn->GetBufferP()->GetDataP(inPitch);

        size_t outPitch;
        Byte* pOutBuffer = pImageOutDownSize->GetBufferP()->GetDataP(outPitch);
        pDownSampler->_ComputeBound(pOutBuffer, pInBuffer, width, height);
        }


    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ComputeGenericDownSampler(HRAImageSamplePtr pImageIn, uint32_t width, uint32_t height,
                                   HRAImageSamplePtr& pImageOutDownSize, HFCPtr<HRPPixelType>& pPixelType)
        {
        std::unique_ptr<GenericDownSampler> pDownSampler;
        pDownSampler.reset(CreateGenericDownSampler(*pPixelType, HGSResampling(HGSResampling::NEAREST_NEIGHBOUR)));

        size_t inPitch;
        Byte* pInBuffer = pImageIn->GetBufferP()->GetDataP(inPitch);

        size_t outPitch;
        Byte* pOutBuffer = pImageOutDownSize->GetBufferP()->GetDataP(outPitch);

        //parameter SrcTileWidth = width
        pDownSampler->_Compute(width, height, pOutBuffer, width, width, height, pInBuffer);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    WString RunOptimizedVSGenericDownSampler(HRAImageSamplePtr pImageIn, uint32_t width, uint32_t height, HFCPtr<HRPPixelType>& pPixelType)
        {
        // If PixelType is indexed, will call Nearest and NearestGeneric.
        // If not, call AverageOptimized and AverageGeneric 
        DSTestBufferAllocator allocatorOptimized;
        ImagePPStatus status;
        HRAImageSamplePtr pImageOptimized = HRAImageSample::CreateSample(status, width, height, pPixelType, allocatorOptimized);

        DSTestBufferAllocator allocatorGeneric;
        HRAImageSamplePtr pImageGeneric = HRAImageSample::CreateSample(status, width, height, pPixelType, allocatorGeneric);

        DSTestBufferAllocator allocatorOptimizedBound;
        HRAImageSamplePtr pImageOptimizedBound = HRAImageSample::CreateSample(status, width, height, pPixelType, allocatorOptimizedBound);

        ComputeOptimizedDownSampler(pImageIn, width, height, pImageOptimized, pPixelType);
        ComputeGenericDownSampler(pImageIn, width, height, pImageGeneric, pPixelType);
        ComputeBoundOptimizedDownSampler(pImageIn, width, height, pImageOptimizedBound, pPixelType);

        EXPECT_STREQ(L"", CompareImage(pImageOptimized, pImageOptimizedBound, pPixelType, width/2, height/2).c_str());

        return CompareImage(pImageOptimized, pImageGeneric, pPixelType, width/2, height/2);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ComputeGenericStretcherN(HRAImageSamplePtr pImageIn, uint32_t width, uint32_t height,
                                     HRAImageSamplePtr& pImageOutDownSize, HFCPtr<HRPPixelType>& pPixelType, uint32_t stretchFactor)
        {
        std::unique_ptr<StretcherN> pDownSampler;
        pDownSampler.reset(CreateGenericStretcherN(*pPixelType, HGSResampling(HGSResampling::NEAREST_NEIGHBOUR)));

        size_t inPitch;
        Byte* pInBuffer = pImageIn->GetBufferP()->GetDataP(inPitch);

        size_t outPitch;
        Byte* pOutBuffer = pImageOutDownSize->GetBufferP()->GetDataP(outPitch);

        pDownSampler->_Compute(pOutBuffer, outPitch, width, height, pInBuffer, inPitch, stretchFactor);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                               Alexandre.Gagnon                      11/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    WString RunOptimizedVSGenericStretcherN(HRAImageSamplePtr pImageIn, uint32_t width, uint32_t height, uint32_t stretchFactor, HFCPtr<HRPPixelType>& pPixelType)
        {
        DSTestBufferAllocator allocatorStretcherN;
        ImagePPStatus status;
        HRAImageSamplePtr pImageStretched = HRAImageSample::CreateSample(status, width, height, pPixelType, allocatorStretcherN);

        DSTestBufferAllocator allocatorOptimized;
        HRAImageSamplePtr pImageOptimized = HRAImageSample::CreateSample(status, width, height, pPixelType, allocatorOptimized);

        ComputeOptimizedDownSampler(pImageIn, width, height, pImageOptimized, pPixelType);
        ComputeGenericStretcherN(pImageIn, width, height, pImageStretched, pPixelType, stretchFactor);

        return CompareImage(pImageOptimized, pImageStretched, pPixelType, width/2, height/2);
        }


}; //END CLASS HRPFilterTester


/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gagnon                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(DownSamplingTester, DownSamplingTestRunner)
    { 
    HFCPtr<HRPPixelType> pPixelType = GetPixelType();
    uint32_t width = GetTileSquareSize();
    uint32_t height = GetTileSquareSize();
    uint32_t stretchFactor = 2; // for StretcherN

    // Create input image
    DSTestBufferAllocator srcAllocator;
    ImagePPStatus status;
    HRAImageSamplePtr pImageIn = HRAImageSample::CreateSample(status, width, height, pPixelType, srcAllocator);
    FillInputImage(pImageIn, width, height, pPixelType);


    ASSERT_STREQ(L"", RunOptimizedVSGenericDownSampler(pImageIn, width, height, pPixelType).c_str());

    ASSERT_STREQ(L"", RunOptimizedVSGenericStretcherN(pImageIn, width, height, stretchFactor, pPixelType).c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Alexandre.Gagnon                      11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static HRPPixelTypeVector s_GetPixelTypeVector ()
    {
    HRPPixelTypeVector pixelTypeVector;

    pixelTypeVector.push_back(new HRPPixelTypeI8Gray8());
    pixelTypeVector.push_back(new HRPPixelTypeV16Int16());
    pixelTypeVector.push_back(new HRPPixelTypeV24R8G8B8());
    pixelTypeVector.push_back(new HRPPixelTypeV32R8G8B8A8());
    pixelTypeVector.push_back(new HRPPixelTypeV48R16G16B16());
    pixelTypeVector.push_back(new HRPPixelTypeV64R16G16B16A16());
    pixelTypeVector.push_back(new HRPPixelTypeV96R32G32B32());

    return pixelTypeVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                            
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(DownSamplingTests,
                        DownSamplingTester,
                        ::testing::Combine(::testing::ValuesIn(s_GetPixelTypeVector()),
                                           ::testing::Values(256, 512, 1024, 2048)));

#endif