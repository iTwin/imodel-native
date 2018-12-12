//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelBuffer.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPPixelBuffer
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetBufferPtr
//-----------------------------------------------------------------------------
inline const void* HRPPixelBuffer::GetBufferPtr() const
    {
    // return the the buffer pointer
    return m_pBuffer;
    }

//-----------------------------------------------------------------------------
// public
// GetBytesPerLine
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelBuffer::GetBytesPerLine() const
    {
    // return the number of bytes per line (including the padding)
    return (m_Width * m_BitsPerPixel + 7) / 8 + m_PaddingBytes;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelBuffer::GetHeight() const
    {
    // return the height of the buffer
    return m_Height;
    }

//-----------------------------------------------------------------------------
// public
// GetPositionsX
//-----------------------------------------------------------------------------
inline const double* HRPPixelBuffer::GetPositionsX() const
    {
    return m_pPositionsX;
    }

//-----------------------------------------------------------------------------
// public
// GetPositionsY
//-----------------------------------------------------------------------------
inline const double* HRPPixelBuffer::GetPositionsY() const
    {
    return m_pPositionsY;
    }

//-----------------------------------------------------------------------------
// public
// GetLine
// This method has been inlined in the code just in case it would be really
// inlined: this method is performance critical!
//-----------------------------------------------------------------------------
inline void HRPPixelBuffer::GetLine(int32_t pi_Y,
                                    void* po_pRawData) const
    {
    HPRECONDITION(po_pRawData != 0);
    HPRECONDITION(pi_Y >= (int32_t)(-1 * m_Neighbourhood.GetYOrigin()));
    HPRECONDITION(pi_Y < (int32_t)(m_Height + m_Neighbourhood.GetHeight() - 1
                                  - m_Neighbourhood.GetYOrigin()));
    Byte* pDestRawData = (Byte*)po_pRawData;

    uint32_t NumberOfBytesPerLine = ((m_Neighbourhood.GetWidth() - 1 + m_Width) *
                                   m_BitsPerPixel + 7) / 8;

    // check in which vertical third of the image structure we are
    // and copy the appropriate data in the output buffer
    if(pi_Y < 0)
        {
        // top border

        Byte* pTopBorder = m_pTopBorder +  (m_Neighbourhood.GetYOrigin() + pi_Y) *
                             (NumberOfBytesPerLine + m_TopBorderPaddingBytes);

        for(uint32_t aByte = 0; aByte < NumberOfBytesPerLine; aByte++)
            {
            *pDestRawData = *pTopBorder;
            pDestRawData++;
            pTopBorder++;
            }
        }
    else if(pi_Y >= (int32_t)m_Height)
        {
        // bottom border

        Byte* pBottomBorder = m_pBottomBorder + (pi_Y - m_Height) *
                                (NumberOfBytesPerLine + m_BottomBorderPaddingBytes);

        for(uint32_t aByte = 0; aByte < NumberOfBytesPerLine; aByte++)
            {
            *pDestRawData = *pBottomBorder;
            pDestRawData++;
            pBottomBorder++;
            }
        }
    else
        {
        // left border
        uint32_t NumberOfBytesInLeftBorder = (m_Neighbourhood.GetXOrigin() *
                                            m_BitsPerPixel + 7) / 8;

        if(NumberOfBytesInLeftBorder > 0)
            {
            Byte* pLeftBorder = m_pLeftBorder + pi_Y *
                                  (NumberOfBytesInLeftBorder + m_LeftBorderPaddingBytes);

            for(uint32_t aByte = 0; aByte < NumberOfBytesInLeftBorder; aByte++)
                {
                *pDestRawData = *pLeftBorder;
                pDestRawData++;
                pLeftBorder++;
                }
            }

        // buffer
        uint32_t NumberOfBytesInCenter = (m_Width * m_BitsPerPixel + 7) / 8;

        Byte* pBuffer = m_pBuffer + pi_Y *
                          (NumberOfBytesInCenter + m_PaddingBytes);

        for(uint32_t aByte = 0; aByte < NumberOfBytesInCenter; aByte++)
            {
            *pDestRawData = *pBuffer;
            pDestRawData++;
            pBuffer++;
            }

        uint32_t NumberOfBytesInRightBorder = ((m_Neighbourhood.GetWidth() - 1 -
                                              m_Neighbourhood.GetXOrigin()) * m_BitsPerPixel + 7) / 8;

        if(NumberOfBytesInRightBorder > 0)
            {
            // right border
            Byte* pRightBorder = m_pRightBorder + pi_Y *
                                   (NumberOfBytesInRightBorder + m_RightBorderPaddingBytes);


            for(uint32_t aByte = 0; aByte < NumberOfBytesInRightBorder; aByte++)
                {
                *pDestRawData = *pRightBorder;
                pDestRawData++;
                pRightBorder++;
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// GetPaddingBytes
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelBuffer::GetPaddingBytes() const
    {
    // return the padding bytes of the buffer
    return m_PaddingBytes;
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelBuffer::GetWidth() const
    {
    // return the width of the buffer
    return m_Width;
    }

END_IMAGEPP_NAMESPACE
