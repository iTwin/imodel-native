//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRPPixelTypeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include "../imagepptestpch.h"

#define TEST_NAME ::testing::UnitTest::GetInstance()->current_test_info()->name()
#define TEST_CASE_NAME ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name()
#define TEST_NAME_PRINT L"TEST: (" << TEST_CASE_NAME << ", " << TEST_NAME << ")"

typedef std::pair<HFCPtr<HRPPixelType>, WString>    HRPPixelTypeNamePair;
typedef std::vector<HRPPixelTypeNamePair>           HRPPixelTypeVector;

//UInt8   UInt8Seeds[]   = {0, 1, 64, 127, 128, 191, 254, 255};
uint8_t UInt8Seeds[]   = {0, 1, 127, 254, 255};
uint16_t UInt16Seeds[]  = {0, 16384, 32767, 32768, 49151, USHRT_MAX};
int16_t Int16Seeds[]   = {SHRT_MIN, SHRT_MIN/2, SHRT_MIN/4, -1, 0, 1, SHRT_MAX/4, SHRT_MAX/2, SHRT_MAX};
uint32_t UInt32Seeds[]  = {0, 500, 1078, 1689, 500001, 2147483647, 2147483648, ULONG_MAX};
float   FloatSeeds[]   = {FLT_MIN, -32767.0, -100.0, 1, 0, -1, 120, 1258, 6897, 3659811, FLT_MAX};

#define SEEDS_COUNT(s) (sizeof(s) / sizeof(s[0]))

/*=================================================================================**//**
* @bsiclass                                                   
+===============+===============+===============+===============+===============+======*/
class HRPPixelTypeCoverage :public ::testing::TestWithParam< ::std::tr1::tuple<HRPPixelTypeNamePair, HRPPixelTypeNamePair> >
    {   
public:
    HRPPixelTypeCoverage()
        {
        };

    virtual void SetUp() 
        {
        // Code here will be called immediately after the constructor (right
        // before each test).
        }

    size_t  ComputeMaxBufferSize(uint32_t seedsCount, HRPPixelType const& pixelType) const
        {
        uint32_t pixelCount = (uint32_t)pow((double)seedsCount, (double)pixelType.GetChannelOrg().CountChannels());

        size_t pixelSizeBytes = ((pixelType.CountPixelRawDataBits()*pixelCount) + 7) / 8;

        return pixelSizeBytes;
        }

    HRPPixelTypeNamePair const& GetInputPixelType() {return ::std::tr1::get<0>(GetParam());}
    
    HRPPixelTypeNamePair const& GetOutputPixelType() {return ::std::tr1::get<1>(GetParam());}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    uint32_t FillBinaryBuffer(vector<Byte>& buffer)
        {
        buffer.resize(8, 0);
                            
        buffer[0] = 0x00;   
        buffer[1] = 0x01;   
        buffer[2] = 0x80;   
        buffer[3] = 0x40;   
        buffer[4] = 0x20;   
        buffer[5] = 0xFF;   
        buffer[6] = 0xAA;   
        buffer[7] = 0xFF;   

        return 57;  // N.B. not aligned to byte boundary.
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    uint32_t FillRLEBuffer(vector<Byte>& buffer)
        {
        buffer.resize(9*sizeof(uint16_t), 0);

        uint16_t* pRLEBuffer = (uint16_t*)&buffer[0];
        pRLEBuffer[0] = 0;
        pRLEBuffer[1] = 3;
        pRLEBuffer[2] = 0;
        pRLEBuffer[3] = 7;
        pRLEBuffer[4] = 5;
        pRLEBuffer[5] = 1;
        pRLEBuffer[6] = 4;
        pRLEBuffer[7] = 8; 
        pRLEBuffer[8] = 3; 

        return 31;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    uint32_t FillBuffer1_T(Byte* pBuffer, Data_T const* seeds, uint32_t seedsCount)
        {
        Data_T* pDataBuffer = ( Data_T*)pBuffer;

        for(uint32_t index=0; index < seedsCount; ++index)
            {
            pDataBuffer[index] = seeds[index];
            }

        return seedsCount;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    uint32_t FillBuffer2_T(Byte* pBuffer, Data_T const* seeds, uint32_t seedsCount)
        {
        Data_T* pDataBuffer = ( Data_T*)pBuffer;

        uint32_t pixelIndex = 0;
        // First channel
        for(uint32_t index1=0; index1 < seedsCount; ++index1)
            {
            // second channel
            for(uint32_t index2=0; index2 < seedsCount; ++index2)
                {
                pDataBuffer[pixelIndex*3 + 0] = seeds[index1];
                pDataBuffer[pixelIndex*3 + 1] = seeds[index2];
                ++pixelIndex;
                }
            }

        return pixelIndex;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    uint32_t FillBuffer3_T(Byte* pBuffer, Data_T const* seeds, uint32_t seedsCount)
        {
        Data_T* pDataBuffer = ( Data_T*)pBuffer;

        uint32_t pixelIndex = 0;
        // First channel
        for(uint32_t index1=0; index1 < seedsCount; ++index1)
            {
            // second channel
            for(uint32_t index2=0; index2 < seedsCount; ++index2)
                {
                // Third channel
                for(uint32_t index3=0; index3 < seedsCount; ++index3)
                    {
                    pDataBuffer[pixelIndex*3 + 0] = seeds[index1];
                    pDataBuffer[pixelIndex*3 + 1] = seeds[index2];
                    pDataBuffer[pixelIndex*3 + 2] = seeds[index3];
                    ++pixelIndex;
                    }
                }
            }

        return pixelIndex;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<class Data_T>
    uint32_t FillBuffer4_T(Byte* pBuffer, Data_T const* seeds, uint32_t seedsCount)
        {
        Data_T* pDataBuffer = ( Data_T*)pBuffer;

        uint32_t pixelIndex = 0;

        // First channel
        for(uint32_t index1=0; index1 < seedsCount; ++index1)
            {
            // second channel
            for(uint32_t index2=0; index2 < seedsCount; ++index2)
                {
                // Third channel
                for(uint32_t index3=0; index3 < seedsCount; ++index3)
                    {
                    // fourth channel
                    for(uint32_t index4=0; index4 < seedsCount; ++index4)
                        {
                        pDataBuffer[pixelIndex*4 + 0] = seeds[index1];
                        pDataBuffer[pixelIndex*4 + 1] = seeds[index2];
                        pDataBuffer[pixelIndex*4 + 2] = seeds[index3];
                        pDataBuffer[pixelIndex*4 + 3] = seeds[index4];
                        ++pixelIndex;
                        }
                    }
                }
            }
       
        return pixelIndex;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  11/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    uint32_t FillBufferFor(vector<Byte>& buffer, HRPPixelType const& pixelType)
        {
        uint32_t pixelCount = 0;

        switch(pixelType.GetClassID())
            {
            case HRPPixelTypeId_V1Gray1:
            case HRPPixelTypeId_V1GrayWhite1:
            case HRPPixelTypeId_I1R8G8B8:
            case HRPPixelTypeId_I1R8G8B8A8:
                pixelCount = FillBinaryBuffer(buffer);
                break;

            case HRPPixelTypeId_I1R8G8B8RLE:
            case HRPPixelTypeId_I1R8G8B8A8RLE:
                pixelCount = FillRLEBuffer(buffer);
                break;

            case HRPPixelTypeId_I8R8G8B8A8:
            case HRPPixelTypeId_I8R8G8B8:
            case HRPPixelTypeId_I8Gray8:
            case HRPPixelTypeId_V8Gray8:
            case HRPPixelTypeId_V8GrayWhite8:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt8Seeds), pixelType));
                pixelCount = FillBuffer1_T<uint8_t>(&buffer[0], UInt8Seeds, SEEDS_COUNT(UInt8Seeds));
                break;

            case HRPPixelTypeId_I8VA8R8G8B8:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt8Seeds), pixelType));
                pixelCount = FillBuffer2_T<uint8_t>(&buffer[0], UInt8Seeds, SEEDS_COUNT(UInt8Seeds));
                break;

            case HRPPixelTypeId_V16Int16:   
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(Int16Seeds), pixelType));
                pixelCount = FillBuffer1_T<int16_t>(&buffer[0], Int16Seeds, SEEDS_COUNT(Int16Seeds));
                break;

            case HRPPixelTypeId_V16Gray16:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt16Seeds), pixelType));
                pixelCount = FillBuffer1_T<uint16_t>(&buffer[0], UInt16Seeds, SEEDS_COUNT(UInt16Seeds));
                break;
       
            case HRPPixelTypeId_V32Float32:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(FloatSeeds), pixelType));
                pixelCount = FillBuffer1_T<float>(&buffer[0], FloatSeeds, SEEDS_COUNT(FloatSeeds));
                break;

            case HRPPixelTypeId_V24PhotoYCC:
            case HRPPixelTypeId_V24R8G8B8:
            case HRPPixelTypeId_V24B8G8R8:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt8Seeds), pixelType));
                pixelCount = FillBuffer3_T<uint8_t>(&buffer[0], UInt8Seeds, SEEDS_COUNT(UInt8Seeds));
                break;

            case HRPPixelTypeId_V48R16G16B16:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt16Seeds), pixelType));
                pixelCount = FillBuffer3_T<uint16_t>(&buffer[0], UInt16Seeds, SEEDS_COUNT(UInt16Seeds));
                break;

            case HRPPixelTypeId_V96R32G32B32:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt32Seeds), pixelType));
                pixelCount = FillBuffer3_T<uint32_t>(&buffer[0], UInt32Seeds, SEEDS_COUNT(UInt32Seeds));
                break;

            
            case HRPPixelTypeId_V32R8G8B8A8:
            case HRPPixelTypeId_V32B8G8R8X8:
            case HRPPixelTypeId_V32R8G8B8X8:
            case HRPPixelTypeId_V32A8R8G8B8:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt8Seeds), pixelType));
                pixelCount = FillBuffer4_T<uint8_t>(&buffer[0], UInt8Seeds, SEEDS_COUNT(UInt8Seeds));
                break;

            case HRPPixelTypeId_V64R16G16B16A16:
            case HRPPixelTypeId_V64R16G16B16X16:
                buffer.resize(ComputeMaxBufferSize(SEEDS_COUNT(UInt16Seeds), pixelType));
                pixelCount = FillBuffer4_T<uint16_t>(&buffer[0], UInt16Seeds, SEEDS_COUNT(UInt16Seeds));
                break;

            // Format that we generate from V24RGB
            case HRPPixelTypeId_V16B5G5R5:
            case HRPPixelTypeId_V16R5G6B5:
            case HRPPixelTypeId_V32CMYK:
                {
                vector<Byte> tempRGB(ComputeMaxBufferSize(SEEDS_COUNT(UInt8Seeds), HRPPixelTypeV24R8G8B8()));
                pixelCount = FillBuffer3_T<uint8_t>(&tempRGB[0], UInt8Seeds, SEEDS_COUNT(UInt8Seeds));
                buffer.resize((pixelCount*pixelType.CountPixelRawDataBits() + 7) / 8);
                HRPPixelTypeV24R8G8B8().GetConverterTo(&pixelType)->Convert(&tempRGB[0], &buffer[0], pixelCount);
                }
                break;

            // Format that we generate from V32RGBA
            case HRPPixelTypeId_V16PRGray8A8:
            case HRPPixelTypeId_V32PRPhotoYCCA8:
            case HRPPixelTypeId_V32PR8PG8PB8A8:
                {
                vector<Byte> tempRGBA(ComputeMaxBufferSize(SEEDS_COUNT(UInt8Seeds), HRPPixelTypeV32R8G8B8A8()));
                pixelCount = FillBuffer4_T<uint8_t>(&tempRGBA[0], UInt8Seeds, SEEDS_COUNT(UInt8Seeds));
                buffer.resize((pixelCount*pixelType.CountPixelRawDataBits() + 7) / 8);
                HRPPixelTypeV32R8G8B8A8().GetConverterTo(&pixelType)->Convert(&tempRGBA[0], &buffer[0], pixelCount);
                }
                break;

            
            case HRPPixelTypeId_I4R8G8B8:
            case HRPPixelTypeId_I4R8G8B8A8:
                {
                buffer.clear();

                for(uint32_t i=0; i < 16; i+=2)
                    {
                    buffer.push_back((Byte)(i | ((i+1) << 4)) );
                    }
                pixelCount = (uint32_t)buffer.size()*2;
                }

            case HRPPixelTypeId_I2R8G8B8: // Disabled.
            default:
                break;
            }
        return pixelCount;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static HRPPixelTypeVector s_GetPixelTypeVector ()
    {
    //TODO We should ask the factory to get the list.
    HRPPixelTypeVector pixelTypeVector;

    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV8Gray8(),           L"V8Gray8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV8GrayWhite8(),      L"V8GrayWhite8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16PRGray8A8(),      L"V16PRGray8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16R5G6B5(),         L"V16R5G6B5"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV16B5G5R5(),         L"V16B5G5R5"));    
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
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV1Gray1(),           L"V1Gray1"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeV1GrayWhite1(),      L"V1GrayWhite1"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8(),          L"I1R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8RLE(),       L"I1R8G8B8RLE"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8(),        L"I1R8G8B8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8RLE(),     L"I1R8G8B8A8RLE"));
    // Disabled in IPP.  pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI2R8G8B8(),          L"I2R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI4R8G8B8(),          L"I4R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI4R8G8B8A8(),        L"I4R8G8B8A8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8(),          L"I8R8G8B8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8A8(),        L"I8R8G8B8A8"));
    //Do not test. Only used by DC Mask. pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8Mask(),      L"I8R8G8B8Mask"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8Gray8(),           L"I8Gray8"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8VA8R8G8B8(),       L"I8VA8R8G8B8"));

    Byte color[4];  //at most RGBA

    // I1RGB / I1RGBA
    HRPPixelPalette paletteI1RGB (2, HRPChannelOrgRGB(8, 8, 8, 0, HRPChannelType::UNUSED, HRPChannelType::VOID_CH, 0));
    HRPPixelPalette paletteI1RGBA(2, HRPChannelOrgRGB(8, 8, 8, 8, HRPChannelType::UNUSED, HRPChannelType::VOID_CH, 0));
    color[0] = 3; 
    color[1] = 0; 
    color[2] = 187; 
    color[3] = 0;
    paletteI1RGB.AddEntry(color);   // alpha ignored
    paletteI1RGBA.AddEntry(color);
    color[0] = 0; 
    color[1] = 250; 
    color[2] = 120; 
    color[3] = 187;
    paletteI1RGB.AddEntry(color);   // alpha ignored
    paletteI1RGBA.AddEntry(color);

    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8(paletteI1RGB),      L"I1R8G8B8_CustomPalette"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8RLE(paletteI1RGB),   L"I1R8G8B8RLE_CustomPalette"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8(paletteI1RGBA),   L"I1R8G8B8A8_CustomPalette"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI1R8G8B8A8RLE(paletteI1RGBA),L"I1R8G8B8A8RLE_CustomPalette"));

    // I8RGB / I8RGBA
    HRPPixelPalette paletteI8RGB (256, HRPChannelOrgRGB(8, 8, 8, 0, HRPChannelType::UNUSED, HRPChannelType::VOID_CH, 0));
    HRPPixelPalette paletteI8RGBA(256, HRPChannelOrgRGB(8, 8, 8, 8, HRPChannelType::UNUSED, HRPChannelType::VOID_CH, 0));
    for(uint32_t i=0; i < 256; ++i)
        {
        color[0] = (Byte)i; 
        color[1] = (Byte)(255-i); 
        color[2] = (Byte)(i%64 + 32); 
        color[3] = (Byte)i;
        paletteI8RGB.AddEntry(color);   // alpha ignored
        paletteI8RGBA.AddEntry(color);
        }
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8(paletteI8RGB),    L"I8R8G8B8_CustomPalette"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8VA8R8G8B8(paletteI8RGB), L"I8VA8R8G8B8_CustomPalette"));
    pixelTypeVector.push_back(HRPPixelTypeNamePair(new HRPPixelTypeI8R8G8B8A8(paletteI8RGBA), L"I8R8G8B8A8_CustomPalette"));

    return pixelTypeVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRPPixelTypeCoverage, DetectDuplicateTest)
    { 
    HFCPtr<HRPPixelType> pSrcPixelType = GetInputPixelType().first;
    HFCPtr<HRPPixelType> pDstPixelType = GetOutputPixelType().first;

    if(pSrcPixelType->IsCompatibleWith(pDstPixelType->GetClassID()))
        return;

    ASSERT_FALSE(pSrcPixelType->HasConverterTo(*pDstPixelType) && pDstPixelType->HasConverterFrom(*pSrcPixelType));
    ASSERT_FALSE(pDstPixelType->HasConverterTo(*pSrcPixelType) && pSrcPixelType->HasConverterFrom(*pDstPixelType)); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRPPixelTypeCoverage, ConvertAndComposeTest)
    {    
    HFCPtr<HRPPixelType> pSrcPixelType = GetInputPixelType().first;
    HFCPtr<HRPPixelType> pDstPixelType = GetOutputPixelType().first;
        
    HFCPtr<HRPPixelConverter> pConverter = pSrcPixelType->GetConverterTo(pDstPixelType);

    if(dynamic_cast<HRPComplexConverter*>(pConverter.GetPtr()) != NULL)
        return; // Skip complex conversion, they will be test individually.

    vector<Byte> srcBuffer;
    uint32_t pixelCount = FillBufferFor(srcBuffer, *pSrcPixelType);

    ASSERT_NE(0UL, pixelCount);

    bool isRLEDestination = false;
    size_t outSizeBytes;
    if(pDstPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || 
       pDstPixelType->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        {
        outSizeBytes = (pixelCount+2)*sizeof(uint16_t);
        isRLEDestination = true;
        }
    else
        {
        outSizeBytes = ((pDstPixelType->CountPixelRawDataBits()*pixelCount + 7) / 8);
        }

    bool sourceHasAlpha = pSrcPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE;
    
    vector<Byte> dstBuffer;
    if(dstBuffer.size() < outSizeBytes)
        dstBuffer.resize(outSizeBytes, 0);

    dstBuffer.push_back(0xAB/*171*/);  // marker for overflow.
    dstBuffer.push_back(0xFA/*250*/);  // marker for overflow.
    
    // Pixel per Pixel converage operation
    if(pSrcPixelType->CountPixelRawDataBits() % 8 == 0)
        {
        uint32_t bytesPerPixel = pSrcPixelType->CountPixelRawDataBits() / 8;
        for(uint32_t pixel=0; pixel < pixelCount; ++pixel)
            {
            Byte resultconvert[HRPPixelType::MAX_PIXEL_BYTES];
            pConverter->Convert(&srcBuffer[0] + bytesPerPixel*pixel, resultconvert, 1);

            // Only source with alpha need composition.
            if(sourceHasAlpha)
                {
                // Compose this result with every possible input.
                for(uint32_t pixelCompose=0; pixelCompose < pixelCount; ++pixelCompose)
                    {
                    Byte resultCompose[HRPPixelType::MAX_PIXEL_BYTES];
                    memcpy(resultCompose, resultconvert, sizeof(resultCompose));

                    pConverter->Compose(&srcBuffer[0] + bytesPerPixel*pixelCompose, resultCompose, 1);
                    }
                }
            }
        }
   
        
    ASSERT_EQ(0xAB, dstBuffer[dstBuffer.size()-2]);
    ASSERT_EQ(0xFA, dstBuffer[dstBuffer.size()-1]);

    // Pixel buffer operations
    pConverter->Convert(&srcBuffer[0], &dstBuffer[0], pixelCount);
    if(sourceHasAlpha)
        pConverter->Compose(&srcBuffer[0], &dstBuffer[0], pixelCount);

    ASSERT_EQ(0xAB, dstBuffer[dstBuffer.size()-2]);
    ASSERT_EQ(0xFA, dstBuffer[dstBuffer.size()-1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(HRPPixelTypeCoverage, ComposeSimpleTest)
    {    
    HFCPtr<HRPPixelType> pSrcPixelType = GetInputPixelType().first;
    HFCPtr<HRPPixelType> pDstPixelType = GetOutputPixelType().first;

    if(pSrcPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE ||
       pSrcPixelType->CountIndexBits() != 0 || pSrcPixelType->CountValueBits() < 8 ||
       pDstPixelType->CountIndexBits() != 0)
        return;

    // Skip for now. Compose not implemented.
    if(pSrcPixelType->IsCompatibleWith(HRPPixelTypeV24PhotoYCC::CLASS_ID) ||
       pSrcPixelType->IsCompatibleWith(HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID) || 
       pDstPixelType->IsCompatibleWith(HRPPixelTypeV24PhotoYCC::CLASS_ID) ||
       pDstPixelType->IsCompatibleWith(HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID))
       return;

    HFCPtr<HRPPixelConverter> pConverter = pSrcPixelType->GetConverterTo(pDstPixelType);

    if(dynamic_cast<HRPComplexConverter*>(pConverter.GetPtr()) != NULL)
        return; // Skip complex conversion, they will be test individually.

    HFCPtr<HRPPixelConverter> pRGBAToSrc  = HRPPixelTypeV32R8G8B8A8().GetConverterTo(pSrcPixelType);
    HFCPtr<HRPPixelConverter> pRGBAToDest = HRPPixelTypeV32R8G8B8A8().GetConverterTo(pDstPixelType);


    Byte srcRGBA[4];
    Byte destRGBA[4];

    vector<Byte> srcPixel((pSrcPixelType->CountPixelRawDataBits() + 7) / 8);
    vector<Byte> dstPixel((pDstPixelType->CountPixelRawDataBits() + 7) / 8);
       
    // A transparent src must not alter dest.
    srcRGBA[0] = srcRGBA[1] = srcRGBA[2] = 255;
    srcRGBA[3] = 0;
    destRGBA[0] = 27;
    destRGBA[1] = 255;
    destRGBA[2] = 167;
    destRGBA[3] = 244;

    pRGBAToSrc->Convert(srcRGBA, &srcPixel[0], 1);
    pRGBAToDest->Convert(destRGBA, &dstPixel[0], 1);
    
    vector<Byte> dstPixelBase(dstPixel);
    pConverter->Compose(&srcPixel[0], &dstPixel[0], 1);

    ASSERT_TRUE(memcmp(&dstPixel[0], &dstPixelBase[0], dstPixel.size()) == 0);

    // A opaque src must mask dest completely
    srcRGBA[0] = 0;
    srcRGBA[1] = 255;
    srcRGBA[2] = 58;
    srcRGBA[3] = 255;
    destRGBA[0] = 255;
    destRGBA[1] = 0;
    destRGBA[2] = 67;
    destRGBA[3] = 127;

    pRGBAToSrc->Convert(srcRGBA, &srcPixel[0], 1);
    pRGBAToDest->Convert(destRGBA, &dstPixel[0], 1);

    pConverter->Convert(&srcPixel[0], &dstPixelBase[0], 1);
    pConverter->Compose(&srcPixel[0], &dstPixel[0], 1);

    ASSERT_TRUE(memcmp(&dstPixel[0], &dstPixelBase[0], dstPixel.size()) == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(HRPPixelTypeCoverageTests,
                        HRPPixelTypeCoverage,
                        ::testing::Combine(::testing::ValuesIn(s_GetPixelTypeVector ()), ::testing::ValuesIn(s_GetPixelTypeVector ())));
