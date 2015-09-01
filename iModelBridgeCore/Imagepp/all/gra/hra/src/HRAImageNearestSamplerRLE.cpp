//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageNearestSamplerRLE.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRAImageNearestSamplerRLE.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <ImagePP/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DCoordSys.h>
#include <ImagePP/all/h/HRPPixelNeighbourhood.h>
#include <ImagePP/all/h/HRPPixelType1bitInterface.h>
#include <ImagePP/all/h/HCDPacketRLE.h>
#include <ImagePP/all/h/HRABitmapRLE.h>
#include <ImagePPInternal/gra/HRAImageSurface.h>




/*---------------------------------------------------------------------------------**//**
* Generate LRS of white runs from RLE data.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RLEIteratorLRS
    {
    RLEIteratorLRS(uint16_t const* pLine, uint32_t lineWidth, int32_t startPosition)
        {
        m_pLine = pLine;
        m_index = 0;
        m_position = 0;
        m_lineWidth = lineWidth;
        GotoFirst((uint32_t)startPosition);
        }

    void GotoFirst(uint32_t startPosition)
        {
        BeAssert(startPosition >= 0 && startPosition <= (int32_t)m_lineWidth);

        // Read black and position index to a white state.
        m_position += m_pLine[m_index];
        ++m_index;
        
        // Goto the first non empty white runs that is beyond or equal to startPosition
        while (IsValid() && (m_pLine[m_index] == 0 || startPosition >= GetEnd()))
            Next();
        }

    void Next()
        {
        do
            {
            // read current white 
            m_position += m_pLine[m_index];
            ++m_index;

            // if all RLE lines end on black then we can read it always. it will be either zero or the remainder to close the line.
            // **BUT** to be safe we test for the end.
            if (m_position >= m_lineWidth)
                return; // no more. m_index will end on a black index. and last pixel is White.

            // skip black
            m_position += m_pLine[m_index];
            ++m_index; 
            }
        while(IsValid() && m_pLine[m_index] == 0);  // Skip empty white runs.

        // if invalid, m_index is on white and last pixel is black
        }

    inline void Invalidate() { m_position = m_lineWidth; }

    inline bool IsValid() const { return m_position < m_lineWidth; }

    inline uint32_t GetStart() const { BeAssert(IsValid()); return m_position; }

    inline uint32_t GetEnd() const { BeAssert(IsValid()); BeAssert(m_index > 0 && m_pLine[m_index] != 0); return m_position + m_pLine[m_index]; }

    //inline bool EndedOnBlack() const { return m_index & 0x1; }

    uint16_t const* m_pLine;
    uint32_t m_index;
    uint32_t m_lineWidth;
    uint32_t m_position;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct LRSRLELineWriter
    {
    LRSRLELineWriter(uint16_t* pLine, size_t buffSizeBytes)
        {
        m_pLine = pLine; 
        m_bufferSize = buffSizeBytes;
        m_index = 0; 
        m_writeCount = 0;
        m_currentStart = m_currentEnd = -1;
        // Cannot push zero here. We will know the first run fill count after we received the first LRS.
        }

    inline void PushLRS(int32_t start, int32_t end)
        {
        BeAssert(start < end);  // end must be one beyond so that end-start == run len. start == end is illegal since is gives an empty run.

        if (-1 == m_currentEnd)
            {
            m_currentStart = start;
            m_currentEnd = end;
            return;
            }

        if (start <= m_currentEnd)
            {
            // merge with current.
            m_currentEnd = end;
            }
        else
            {
            // New disjoint LRS.
            WriteCurrent();
            m_currentStart = start;
            m_currentEnd = end;
            }
        }

    inline void WriteCurrent()
        {
        BeAssert(m_currentStart >= (int32_t)m_writeCount);
        BeAssert(m_currentEnd >= (int32_t)m_writeCount);

        // Fill background.
        Write(m_currentStart - m_writeCount);

        // write foreground.
        Write(m_currentEnd - m_currentStart);
        m_writeCount = m_currentEnd;
        }

    inline void Write(uint32_t count)
        {
        // Write the current run to the destination 
        while (count > SHRT_MAX)
            {
            BeAssert(m_index*sizeof(uint16_t) < m_bufferSize);
            m_pLine[m_index] = SHRT_MAX;
            m_pLine[m_index + 1] = 0;
            m_index += 2;
            count -= SHRT_MAX;
            }

        BeAssert(m_index*sizeof(uint16_t) < m_bufferSize);
        m_pLine[m_index] = (uint16_t)count;
        ++m_index;
        }

    inline void WriteZero()
        {
        BeAssert(m_index*sizeof(uint16_t) < m_bufferSize);
        m_pLine[m_index] = 0;
        ++m_index;
        }

    inline void CloseClampWhite(uint32_t length)
        {
        if (m_currentStart == -1)
            m_currentStart = m_writeCount;

        m_currentEnd = length;
        
        WriteCurrent();
        WriteZero();
        }

    inline void CloseClampBlack(uint32_t length)
        {
        if (m_currentEnd != -1)
            {
            WriteCurrent();
            }

        BeAssert(length >= m_writeCount);

        // fill up to length             
        Write(length-m_writeCount);
        m_writeCount = length;
        }

    inline void ReverseLine()
        {
        BeAssert(m_index > 0 && m_writeCount != 0);

        std::reverse(&m_pLine[0], &m_pLine[m_index]);
        }

    // Black runs are ON even number. 0,2,4,6...
    inline uint32_t GetState() const { return m_index & 0x1; }

    inline size_t GetDataSize() { return m_index*sizeof(uint16_t); }
    int32_t m_currentStart;
    int32_t m_currentEnd;

    uint32_t m_writeCount;
    uint16_t* m_pLine;
    uint32_t m_index;
    size_t  m_bufferSize;       // In Bytes
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RLELineWriter
    {
    RLELineWriter(uint16_t* pLine, size_t buffSizeBytes)
        {
        m_pLine = pLine; 
        m_bufferSize = buffSizeBytes;
        m_index = 0; 
        }

    inline void Write(uint32_t count)
        {
        // Write the current run to the destination
        while (count > SHRT_MAX)
            {
            BeAssert(m_index*sizeof(uint16_t) < m_bufferSize);
            m_pLine[m_index] = SHRT_MAX;
            m_pLine[m_index + 1] = 0;
            m_index += 2;
            count -= SHRT_MAX;
            }

        BeAssert(m_index*sizeof(uint16_t) < m_bufferSize);
        m_pLine[m_index] = (uint16_t)count;
        ++m_index;
        }

    //! A slower version that will test the current state before writing.
    inline void WriteRun(uint32_t count, bool state)
        {
        if ((GetState() != 0) != state)
            WriteZero();

        Write(count);
        }

    inline void WriteZero()
        {
        BeAssert(m_index*sizeof(uint16_t) < m_bufferSize);
        m_pLine[m_index] = 0;
        ++m_index;
        }

    inline void CloseOnBlack() 
        {
        if (!GetState())
            WriteZero();
        }

    // Black runs are ON even number. 0,2,4,6...
    inline uint32_t GetState() const { return m_index & 0x1; }  

    inline size_t GetDataSize() { return m_index*sizeof(uint16_t); }
    uint16_t* m_pLine;
    uint32_t m_index;
    size_t  m_bufferSize;       // In Bytes
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RLELineReader
    {
    RLELineReader(uint16_t const* pLine)
        {
        m_pLine = pLine;
        m_index = 0;
        m_position = 0;
        }

    inline uint16_t GetRun() const { return m_pLine[m_index]; };

    inline uint16_t GetRunAndIncrement() { return m_pLine[m_index++]; };
    
    inline void NextRun() { ++m_index; }

    // Black runs are ON even number. 0,2,4,6...
    inline uint32_t GetState() const { return m_index & 0x1; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  09/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool GetStateAt(uint16_t const* pLine, uint32_t atPosition)
        {
        RLELineReader reader(pLine);

        uint32_t position = reader.GetRun();
        while (position <= atPosition)
            {
            reader.NextRun();
            position += reader.GetRun();
            }

        return reader.GetState() != 0;
        }
      
    
    uint16_t const* m_pLine;
    uint32_t m_index;
    uint32_t m_position;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Surface_T>
struct RLEPixelResolver
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RLEPixelResolver(Surface_T& inData, vector<HRAImageNearestSamplerRLE::RLEBufferPosition>& pPositionBuffer)
 :m_position(pPositionBuffer),
  m_lineReader(inData)
    {
    if (inData.GetHeight() > m_position.size())
        {
        m_position.resize(inData.GetHeight());
        }

    // Clear only the part that we will use.
    memset(&m_position[0], 0, sizeof(HRAImageNearestSamplerRLE::RLEBufferPosition)*inData.GetHeight());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t GetStateAt(uint32_t positionX, uint32_t positionY)
    {
    uint16_t const* pRun = (uint16_t const*)m_lineReader.GetLineDataCP(positionY);

    HRAImageNearestSamplerRLE::RLEBufferPosition& LastBufferPosition = m_position[positionY];

    if(LastBufferPosition.m_StartPosition <= positionX)
        {
        // Go foreword in RLE buffer
        while (LastBufferPosition.m_StartPosition <= positionX)
            {
            //HASSERT(LastBufferPosition.m_IndexInBuffer < GetLineDataSize(positionY) >> 1);
            LastBufferPosition.m_StartPosition += pRun[LastBufferPosition.m_IndexInBuffer];
            ++LastBufferPosition.m_IndexInBuffer;
            }

        //HASSERT(LastBufferPosition.m_IndexInBuffer-1 < GetLineDataSize(positionY) >> 1);

        // Black runs are ON even number. 0,2,4,6...
        return (LastBufferPosition.m_IndexInBuffer-1) & 0x00000001;
        }

    // Go backward in RLE buffer
    while (LastBufferPosition.m_StartPosition > positionX)
        {
        //HASSERT(LastBufferPosition.m_IndexInBuffer-1 < GetLineDataSize(positionY) >> 1);
        LastBufferPosition.m_StartPosition -= pRun[LastBufferPosition.m_IndexInBuffer-1];
        --LastBufferPosition.m_IndexInBuffer;
        }

    //HASSERT(LastBufferPosition.m_IndexInBuffer < GetLineDataSize(positionY) >> 1);

    // Black runs are ON even number. 0,2,4,6...
    return LastBufferPosition.m_IndexInBuffer & 0x00000001;
    }

private:
    vector<HRAImageNearestSamplerRLE::RLEBufferPosition>& m_position;
    typename Surface_T::LineReader m_lineReader;
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler* HRAImageSampler::CreateNearestRle(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType)
    {
    if (pixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || pixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        return new HRAImageNearestSamplerRLE(destToSrcTransfo, pixelType);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageNearestSamplerRLE::HRAImageNearestSamplerRLE(HFCPtr<HGF2DTransfoModel>& destToSrcTransfo, HRPPixelType const& pixelType)
    :HRAImageSampler(HRPPixelNeighbourhood(1, 1, 0, 0), destToSrcTransfo)
    {
    m_biasMode = BIAS_EqualWeight;

    HRPPixelType1BitInterface const* pPixelType1BitInterface = const_cast<HRPPixelType&>(pixelType).Get1BitInterface();
    BeAssert(pPixelType1BitInterface != NULL);  // What kind of pixel type is that? Only 1bit are supposed to end up here.
    
    if (pPixelType1BitInterface != NULL && pPixelType1BitInterface->IsForegroundStateDefined())
        m_biasMode = pPixelType1BitInterface->GetForegroundState() ? BIAS_White : BIAS_Black;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageNearestSamplerRLE::~HRAImageNearestSamplerRLE(){}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageNearestSamplerRLE::_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    BeAssert(outData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || outData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID));
    BeAssert(inData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || inData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID));
    BeAssert(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());
    BeAssert(m_isStretchable);
    //pops during gtest because we force stretch. Normally, 1:1 should not end-up here base class will intercept it. HPRECONDITION(!m_hasTranslationOnly);
    BeAssert(m_isStretchable);

    SamplerSurfaceVisitor<HRAImageNearestSamplerRLE, true/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageNearestSamplerRLE::_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    HPRECONDITION(outData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || outData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID));
    HPRECONDITION(inData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) || inData.GetPixelType().IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID));
    HPRECONDITION(outData.GetPixelType().CountPixelRawDataBits() == inData.GetPixelType().CountPixelRawDataBits());

    SamplerSurfaceVisitor<HRAImageNearestSamplerRLE, false/*IsStretch*/> visitor(*this, outData, outOffset, inOffset);

    return inData.Accept(visitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RunReader
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    RunReader(uint16_t const* pRuns, uint32_t pixelCount)
    :m_pRuns(pRuns), 
     m_pixelCount(pixelCount),
     m_index(0)
        {
        m_startPosition = 0;
        m_endPosition = m_pRuns[m_index];
        ++m_index;
        if(0 == m_endPosition)
            {
            m_endPosition = m_pRuns[m_index];
            ++m_index;
            }

        // Merge runs like 32767,0, 20, 9 ....
        while(m_endPosition < m_pixelCount && 0 == m_pRuns[m_index])
            {
            m_endPosition += m_pRuns[m_index + 1];
            m_index+=2;
            }
        }
    ~RunReader(){}

    uint32_t GetStartRun() const {return m_startPosition;}
    uint32_t GetEndRun() const {return m_endPosition;}
    bool GetState() const {return !(m_index & 0x1);}   // index is one beyond current.

    bool IsValid() const {return m_startPosition < m_pixelCount; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void MoveFoward(double position)
        {
        // false: Looks like the old implementation(V8i and below)
        // true: gives a more accurate(per pixel transformation wise) but sometimes produce uglier result.
        static bool s_equalMode = false; 
        if(s_equalMode)
            {
            while(IsValid() && (double)GetEndRun() <= position)
                Next();
            }
        else
            {
            while(IsValid() && (double)GetEndRun() < position)
                Next();
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Next()
        {
        // Disable assert because we want to avoid an extra check 
        // per run and we can handle call for Next when already at the end.
        // BeAssert(IsValid());
        
        if(m_endPosition >= m_pixelCount)
            {
            m_startPosition = m_pixelCount;
            return;
            }

        m_startPosition = m_endPosition;
        m_endPosition += m_pRuns[m_index];
        ++m_index;

        // Merge runs like 32767,0, 20, 9 ....
        while(m_endPosition < m_pixelCount && 0 == m_pRuns[m_index])
            {
            m_endPosition += m_pRuns[m_index + 1];
            m_index+=2;
            }
        }   

    uint32_t m_startPosition;
    uint32_t m_endPosition;
    uint32_t m_pixelCount;
    uint32_t m_index;
    uint16_t const* m_pRuns;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RunWriter
    {
    RunWriter(uint16_t* pRuns, uint32_t pixelCount)
    :m_pRuns(pRuns), 
     m_pixelCount(pixelCount),
     m_index(0),
     m_written(0)
        {
        m_pRuns[0] = 0;
        }

    bool GetState() const {return m_index & 0x1;}

    uint32_t WrittenCount() const {return m_written;}

    size_t GetDataSize() const {return (m_index+1)*sizeof(uint16_t);}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Write(uint32_t count, bool state)
        {
        m_written+=count;
        BeAssert(m_written <= m_pixelCount);

        // if same state append to current.
        if(state == GetState())
            {
            count += m_pRuns[m_index];
            Write(count);
            }
        else
            {
            ++m_index;
            Write(count);
            }            
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CloseClamp(bool state)
        {
        if(m_pixelCount-m_written)
            Write(m_pixelCount-m_written, state);        

        // end on black.
        if(GetState())
            {
            ++m_index;
            m_pRuns[m_index] = 0;
            }
        }

    inline void ReverseLine()
        {
        BeAssert(m_written != 0);

        //m_index   == last
        //m_index+1 == end
        std::reverse(&m_pRuns[0], &m_pRuns[m_index+1]);
        }

private:
    inline void Write(uint32_t count)
        {
        // Write the current run to the destination 
        while (count > SHRT_MAX)
            {
            //BeAssert(m_index*sizeof(UInt16) < m_bufferSize);
            m_pRuns[m_index] = SHRT_MAX;
            m_pRuns[m_index + 1] = 0;
            m_index += 2;
            count -= SHRT_MAX;
            }

        //BeAssert(m_index*sizeof(UInt16) < m_bufferSize);
        m_pRuns[m_index] = (uint16_t)count;
        }

    uint16_t* m_pRuns;
    uint32_t m_pixelCount;
    uint32_t m_index;
    uint32_t m_written;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Surface_T>
ImagePPStatus HRAImageNearestSamplerRLE::Stretch_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {        
    HRAImageBufferRleP pOutDataRle = outData.GetBufferRleP();
    if (NULL == pOutDataRle)
        return IMAGEPP_STATUS_NoImplementation;

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    size_t outPitch;
    Byte* pOutBuffer = pOutDataRle->GetDataP(outPitch);

    typename Surface_T::LineReader rleData(inData);

    // With a flip in X, we adjust the scaling and offset to remove x-flipping. We generate without flipping and then flip the result in place.
    bool isReverseLine = m_scaleX < 0.0;
    double deltaX = isReverseLine ? -m_scaleX : m_scaleX;
    double deltaY = m_scaleY;
    double offsetX = isReverseLine ? m_offsetX + m_scaleX*outWidth + m_scaleX*2*outOffset.x : m_offsetX;

    // We use floor in case we have negative values.
    double firstColumInSrcReel = (offsetX + ((0 + outOffset.x + 0.5)*deltaX)) - inOffset.x;
    int32_t firstColumnInSrc = (int32_t)floor(firstColumInSrcReel);
    int32_t LastColumnInSrc = (int32_t)floor((offsetX + (((outWidth-1) + outOffset.x + 0.5)*deltaX)) - inOffset.x);

    // MM: Disabled because of an epsilon error.
    //BeAssert(isReverseLine ? (Int32)((m_offsetX + ((0 + outOffset.x + 0.5)*m_scaleX)) - inOffset.x) == LastColumnInSrc : true);
    //BeAssert(isReverseLine ? (Int32)((m_offsetX + (((outWidth - 1) + outOffset.x + 0.5)*m_scaleX)) - inOffset.x) == firstColumnInSrc : true);
             
    // Add factor to change pixel selection strategy.
    double invScaleX = 1 / deltaX; 
    double translationXPrime;
    double translationEndXPrime;
    switch (m_biasMode)
        {
        case BIAS_Black:
            // A factor of 0.99 at the start will reject white pixels that fill less than 99%
            translationXPrime = (((-offsetX + inOffset.x) / deltaX) - outOffset.x) + 0.99;
            translationEndXPrime = (((-offsetX + inOffset.x) / deltaX) - outOffset.x);
            break;
        
        case BIAS_White:
            // A factor of 0.99 on the end will select pixels that fill at least 99% (aka foreground mode)
            translationXPrime = (((-offsetX + inOffset.x) / deltaX) - outOffset.x);
            translationEndXPrime = (((-offsetX + inOffset.x) / deltaX) - outOffset.x) + 0.99;
            break;

        case BIAS_EqualWeight: 
        default:
            // *** EqualWeight has now its own implementation since it did not look as good as the old stretch algo.
            //     So We kept code below as a reference only.
            // A factor of 0.499 will select pixels that fill more than 50%. (like warp)
            translationXPrime = (((-offsetX + inOffset.x) / deltaX) - outOffset.x) + (0.5-HGLOBAL_EPSILON);
            translationEndXPrime = (((-offsetX + inOffset.x) / deltaX) - outOffset.x) + (0.5-HGLOBAL_EPSILON);
            break;        
        }
        
    // Process all output lines.
    for (uint32_t row = 0; row < outHeight; ++row)
        {
        int32_t rowInSrc = (int32_t)((m_offsetY + ((row + outOffset.y + 0.5)*deltaY)) - inOffset.y);
        uint32_t effectiveRowInSrc = MAX(0, MIN(rowInSrc, (int32_t)(inHeight - 1)));

        //&&OPTIMIZATION: use inLineDataSize to read backward when appropriate. and validate than we are not reading beyond...
        uint16_t const* pSrcRuns = (uint16_t const*)rleData.GetLineDataCP(effectiveRowInSrc);

        if (LastColumnInSrc < 0)    // dest is completely at the left of the source.
            {
            LRSRLELineWriter writerLRS((uint16_t*)(pOutBuffer + row*outPitch), outPitch);

            if(RLELineReader::GetStateAt(pSrcRuns, 0))
                writerLRS.CloseClampWhite(outWidth);
            else
                writerLRS.CloseClampBlack(outWidth);

            pOutDataRle->SetLineDataSize(row, writerLRS.GetDataSize());  // Write line data size
            continue;
            }
        else if (firstColumnInSrc >= (int32_t)inWidth)    // dest is completely at the right of the source.
            {
            LRSRLELineWriter writerLRS((uint16_t*)(pOutBuffer + row*outPitch), outPitch);

            if(RLELineReader::GetStateAt(pSrcRuns, inWidth-1))
                writerLRS.CloseClampWhite(outWidth);
            else
                writerLRS.CloseClampBlack(outWidth);

            pOutDataRle->SetLineDataSize(row, writerLRS.GetDataSize());  // Write line data size
            continue;
            }

        if(BIAS_EqualWeight == m_biasMode)
            {
            RunReader reader(pSrcRuns, inWidth);
            RunWriter writer((uint16_t*)(pOutBuffer + row*outPitch), outWidth);  // validate that we are not going beyond.

            // If we have left clamping do it now.
            double positionInSrc = firstColumInSrcReel;
            if(positionInSrc < 0) 
                {
                // If dest is not touching the src use the first src pixel to find the first destination pixel that is fill by the src.
                BeAssert((((-offsetX + inOffset.x) / deltaX) - outOffset.x) > 0);
                uint32_t firstDestPixelFillBySrc = (uint32_t)((((-offsetX + inOffset.x) / deltaX) - outOffset.x)+0.5); // src first pixel to 
                positionInSrc =  (offsetX + ((firstDestPixelFillBySrc + outOffset.x + 0.5)*deltaX)) - inOffset.x;

                BeAssert(firstDestPixelFillBySrc <= outWidth);  // how that happen? we should have detect disjoint query and clamp above.
                writer.Write(MIN(firstDestPixelFillBySrc, outWidth), reader.GetState());
                }        
        
            // Do we need to floor for 1:1 case? What about other non aligned cases? If we do we loose the original offset. not good!
            //positionInSrc = floor(positionInSrc);
        
            // For all runs.
            for (;reader.IsValid(); reader.Next())
                {
                reader.MoveFoward(positionInSrc);   // Skip entry that we have scale over.


                double srcRunLen = (reader.GetEndRun() - positionInSrc);
                double dstRunLenReel = (srcRunLen * invScaleX);
                uint32_t dstRunLen = MAX((uint32_t)dstRunLenReel, 1);
                positionInSrc += dstRunLen*deltaX;  // move source position to the effective destRunLen.

                if(writer.WrittenCount() + dstRunLen >= outWidth)
                    {
                    writer.CloseClamp(reader.GetState());
                    break;
                    }

                writer.Write(dstRunLen, reader.GetState());
                } 

            writer.CloseClamp(reader.GetState());
            pOutDataRle->SetLineDataSize(row, writer.GetDataSize());

            if (isReverseLine)
                writer.ReverseLine();
            }
        else
            {
            LRSRLELineWriter writerLRS((uint16_t*)(pOutBuffer + row*outPitch), outPitch);

            // We do have an intersection between the source and the destination.
            RLEIteratorLRS itr(pSrcRuns, inWidth, MAX(0,firstColumnInSrc));        

            // No foreground runs intersect with the source.
            if (!itr.IsValid())
                {
                writerLRS.CloseClampBlack(outWidth);
                pOutDataRle->SetLineDataSize(row, writerLRS.GetDataSize());  // Write line data size
                continue;
                }

            // For all foreground runs.
            for (; itr.IsValid(); itr.Next())
                {
                int32_t destStartPos = (int32_t)(itr.GetStart() * invScaleX + translationXPrime);
                int32_t destEndPos = (int32_t)(itr.GetEnd() * invScaleX + translationEndXPrime);

                // The current scanline doesn't touch our dest get out.
                if (destStartPos >= (int32_t)outWidth)
                    {
                    if (itr.GetStart() == 0)    // Clamp white if this run is the first pixel.
                        writerLRS.PushLRS(0, outWidth);
                    break;
                    }               

                // * Start adjustement: clip if negative or clamp if first run.
                if (destStartPos < 0 || itr.GetStart() == 0)
                    destStartPos = 0;

                // * End adjustement: Clip if it goes beyond the dest or clamp if it is the last run.
                if (destEndPos >= (int32_t)outWidth || itr.GetEnd() == inWidth)
                    {
                    // We are done. We need to clamp till the end 
                    writerLRS.PushLRS(destStartPos, outWidth);
                    break;
                    }

                // Note that empty run test must be executed after adjustment because we may extend(clamp) or reduce(clip).
                if (destStartPos < destEndPos)     // Run must be at least one pixel.
                    writerLRS.PushLRS(destStartPos, destEndPos);
                }

            writerLRS.CloseClampBlack(outWidth);
            pOutDataRle->SetLineDataSize(row, writerLRS.GetDataSize());  // Write line data size

            if (isReverseLine)
                writerLRS.ReverseLine();
            }
        }
        
    return IMAGEPP_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Surface_T>
ImagePPStatus HRAImageNearestSamplerRLE::Warp_T(HRAImageSampleR outData, PixelOffset const& outOffset, Surface_T& inData, PixelOffset const& inOffset)
    {
    HRAImageBufferRleP pOutDataRle = outData.GetBufferRleP();
    if (NULL == pOutDataRle)
        return IMAGEPP_STATUS_NoImplementation;

    size_t outPitch;
    Byte* pOutBuffer = pOutDataRle->GetDataP(outPitch);

    uint32_t inWidth = inData.GetWidth();
    uint32_t inHeight = inData.GetHeight();

    uint32_t outWidth = outData.GetWidth();
    uint32_t outHeight = outData.GetHeight();

    RawBuffer_T<double, decltype(m_allocator)> positions(outWidth * 2, m_allocator);
    double* pXPositions = &positions.At(0);
    double* pYPositions = &positions.At(outWidth);

    //TR#259220: Allocated array that will remember the last position in RLE buffer. It gives a huge performance gains when resampling: 45 MIN -> 2 MIN
    RLEPixelResolver<Surface_T> pixelResolver(inData, m_bufferPosition);

    // Process all lines.
    for (uint32_t row = 0; row < outHeight; ++row)
        {
        // convert the destination run into the source coordinates
        // N.B. The Y component must be pass before X. How intuitive is that!
        m_pDestToSrcTransfo->ConvertDirect(row + outOffset.y + 0.5, outOffset.x + 0.5, outWidth, 1.0, pXPositions, pYPositions);

        RLELineWriter lineWriter((uint16_t*)(pOutBuffer + row*outPitch), outPitch);

        uint32_t runCount = 0;
        for (uint32_t column = 0; column < outWidth; ++column)
            {
            int32_t rowInSrc = MAX(0, MIN((int32_t)(pYPositions[column] - inOffset.y), (int32_t)(inHeight - 1)));
            int32_t columnInSrc = MAX(0, MIN((int32_t)(pXPositions[column] - inOffset.x), (int32_t)(inWidth - 1)));

            if (lineWriter.GetState() != pixelResolver.GetStateAt(columnInSrc, rowInSrc))
                {
                lineWriter.Write(runCount);
                runCount = 1;
                }
            else
                {
                ++runCount;
                }
            }

        if (runCount != 0)
            lineWriter.Write(runCount);

        lineWriter.CloseOnBlack();
        pOutDataRle->SetLineDataSize(row, lineWriter.GetDataSize());  // Write line data size
        }
    
    return IMAGEPP_STATUS_Success;
    }