//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HRAImageOpTester.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

struct ITestImageOpCreator;

typedef std::pair<HFCPtr<HRPPixelType>, WString>    HRPPixelTypeNamePair;
typedef std::vector<HRPPixelTypeNamePair>           HRPPixelTypeVector;
typedef std::vector<ITestImageOpCreator*>           TestImageOpCreatorVector;

#define PADDING_BYTE        0xB2        // Dummy value used for padding

static bool         m_timingIsStarted = false;
static clock_t      s_startTime;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ITestImageOpCreator : public RefCountedBase
    {
    virtual HRAImageOpPtr Create(HRPPixelType const& srcPixeltype) = 0;

    virtual WCharCP GetOperationName() = 0;
    };

/*=================================================================================**//**
* @bsiclass                                                     Eric.Paquet  11/2012
+===============+===============+===============+===============+===============+======*/
struct BufferAllocator : RefCounted<BentleyApi::ImagePP::IImageAllocator>
    {
    void            GetImageSize(uint32_t &width, uint32_t &height);
    uint32_t        GetPitch();
    static bool     VerifyBuffer(HRAImageSamplePtr& prImageSample, HFCPtr<HRPPixelType>& prPixelType, WStringR errorMsg);
    virtual Byte*   _AllocMemory(size_t pi_MemorySize) override;
    virtual void    _FreeMemory(Byte* pi_MemPtr) override;

private:
    static const uint32_t m_imageWidth = 16;
    static const uint32_t m_imageHeight = 32;
    static const uint32_t m_maxBytesPixelSize = 12; // Biggest pixel size (in bytes) seems to be HRPPixelTypeV96R32G32B32
    static const uint32_t m_paddingBytes = m_maxBytesPixelSize * 4; // Each side of the image is padded with 4 pixels
    static const uint32_t m_pitch = m_imageWidth * m_maxBytesPixelSize + m_paddingBytes * 2; // A buffer line has padding at beginning and end
    static const uint32_t m_nbFirstPaddingBytes = m_pitch + m_paddingBytes; // The returned buffer points to where the image must be written (skip one line and add padding)
    };

/*=================================================================================**//**
* @bsiclass                                                     Eric.Paquet  11/2012
+===============+===============+===============+===============+===============+======*/
class HRAImageOpCombinePixelTypes :public ::testing::TestWithParam< ::std::tr1::tuple<ITestImageOpCreator*, HRPPixelTypeNamePair, HRPPixelTypeNamePair> >
    {   
public:

protected :
    HRAImageOpCombinePixelTypes();

    virtual void SetUp() 
        {
        // Code here will be called immediately after the constructor (right
        // before each test).
        }
    
    ITestImageOpCreator& GetFilterCreator() {return *::std::tr1::get<0>(GetParam()) ;}

    HRPPixelTypeNamePair const& GetInputPixelType() {return ::std::tr1::get<1>(GetParam());}
    
    HRPPixelTypeNamePair const& GetOutputPixelType() {return ::std::tr1::get<2>(GetParam());}

    void CreateImage(HRAImageSamplePtr& prImageSample, HFCPtr<HRPPixelType>& prPixelType);

private:
    static void     StartTiming();

    BufferAllocator m_bufferAllocator;
    };


