//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelBuffer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class: HRPPixelBuffer
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelBuffer.h>
#include <Imagepp/all/h/HRPPixelType.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPPixelBuffer::HRPPixelBuffer(const HRPPixelNeighbourhood& pi_rNeighbourhood,
                               const HRPPixelType&          pi_rPixelType,
                               void*                        pio_pRawDataPtr,
                               uint32_t                     pi_Width,
                               uint32_t                     pi_Height,
                               uint32_t                     pi_PaddingBytes,
                               bool                        pi_InternalBorders,
                               const double*               pi_pPositionsX,
                               const double*               pi_pPositionsY)
    {

    Initialize(pi_rNeighbourhood, pi_rPixelType);

    SetRawData(pio_pRawDataPtr,
               pi_Width,
               pi_Height,
               pi_PaddingBytes,
               pi_InternalBorders,
               pi_pPositionsX,
               pi_pPositionsY);
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPPixelBuffer::HRPPixelBuffer(const HRPPixelType&  pi_rPixelType,
                               void*                pio_pRawDataPtr,
                               uint32_t             pi_Width,
                               uint32_t             pi_Height,
                               uint32_t             pi_PaddingBytes,
                               const double*       pi_pPositionsX,
                               const double*       pi_pPositionsY)
    {
    Initialize(HRPPixelNeighbourhood(1, 1, 0, 0), pi_rPixelType);

    SetRawData(pio_pRawDataPtr,
               pi_Width,
               pi_Height,
               pi_PaddingBytes,
               false,
               pi_pPositionsX,
               pi_pPositionsY);
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPPixelBuffer::HRPPixelBuffer(const HRPPixelNeighbourhood& pi_rNeighbourhood,
                               const HRPPixelType&          pi_rPixelType)
    {
    Initialize(pi_rNeighbourhood, pi_rPixelType);
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPPixelBuffer::HRPPixelBuffer(const HRPPixelType&  pi_rPixelType)
    {
    Initialize(HRPPixelNeighbourhood(1, 1, 0, 0), pi_rPixelType);
    }

//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HRPPixelBuffer::~HRPPixelBuffer()
    {
    }

//-----------------------------------------------------------------------------
// public
// Extract
//-----------------------------------------------------------------------------
Byte* HRPPixelBuffer::Extract() const
    {
    uint32_t BytesPerLine = ((m_Width + (m_Neighbourhood.GetWidth() - 1)) * m_BitsPerPixel + 7) / 8;

    // allocate a new buffer
    Byte* pBuffer = new Byte[ BytesPerLine *
                                (m_Height + (m_Neighbourhood.GetHeight() - 1))];

    if(pBuffer != 0)
        {
        Byte* pLine = pBuffer;

        // copy the content of the pixel buffer in the byte buffer
        for(int32_t Y = (int32_t)(-1 * m_Neighbourhood.GetYOrigin());
            Y < (int32_t)(m_Height + (m_Neighbourhood.GetHeight() - 1 - m_Neighbourhood.GetYOrigin()));
            Y++)
            {
            GetLine(Y, pLine);

            pLine += BytesPerLine;
            }
        }

    return(pBuffer);
    }

//-----------------------------------------------------------------------------
// public
// SetRawData
//-----------------------------------------------------------------------------
void HRPPixelBuffer::SetRawData(void*               pio_pRawDataPtr,
                                uint32_t            pi_Width,
                                uint32_t            pi_Height,
                                uint32_t            pi_PaddingBytes,
                                bool               pi_InternalBorders,
                                const double*      pi_pPositionsX,
                                const double*      pi_pPositionsY)
    {
    HPRECONDITION(pio_pRawDataPtr != 0);

    // set the dimensions
    m_Width = pi_Width;
    m_Height = pi_Height;

    // keep the array of position
    m_pPositionsX = pi_pPositionsX;
    m_pPositionsY = pi_pPositionsY;

    if(!pi_InternalBorders)
        {
        m_pBuffer = (Byte*)pio_pRawDataPtr;
        m_PaddingBytes = pi_PaddingBytes;
        }
    else
        {
        uint32_t BytesPerLine = ((pi_Width + (m_Neighbourhood.GetWidth() - 1)) * m_BitsPerPixel + 7) / 8 +
                              pi_PaddingBytes;

        m_pTopBorder    = (Byte*)pio_pRawDataPtr;
        m_pLeftBorder   = m_pTopBorder + (m_Neighbourhood.GetYOrigin() * BytesPerLine);
        m_pRightBorder  = m_pLeftBorder + ((pi_Width + m_Neighbourhood.GetXOrigin()) * m_BitsPerPixel + 7) / 8;
        m_pBottomBorder = m_pLeftBorder + (pi_Height * BytesPerLine);
        m_pBuffer       = m_pLeftBorder + (m_Neighbourhood.GetXOrigin() * m_BitsPerPixel + 7) / 8;

        m_TopBorderPaddingBytes = pi_PaddingBytes;
        m_LeftBorderPaddingBytes = BytesPerLine -
                                   (m_Neighbourhood.GetXOrigin() * m_BitsPerPixel + 7) / 8;
        m_RightBorderPaddingBytes = BytesPerLine -
                                    ((m_Neighbourhood.GetWidth() - 1 - m_Neighbourhood.GetXOrigin()) * m_BitsPerPixel + 7) / 8;
        m_BottomBorderPaddingBytes = pi_PaddingBytes;
        m_PaddingBytes = BytesPerLine - (pi_Width * m_BitsPerPixel + 7) / 8;
        }
    }

//-----------------------------------------------------------------------------
// public
// SetBorders
//-----------------------------------------------------------------------------
void HRPPixelBuffer::SetBorders(const HRPPixelNeighbourhood&    pi_rNeighbourhood,
                                const void*                     pi_pTopBorder,
                                const void*                     pi_pLeftBorder,
                                const void*                     pi_pRightBorder,
                                const void*                     pi_pBottomBorder,
                                uint32_t                        pi_TopBorderPaddingBytes,
                                uint32_t                        pi_LeftBorderPaddingBytes,
                                uint32_t                        pi_RightBorderPaddingBytes,
                                uint32_t                        pi_BottomBorderPaddingBytes)
    {
    m_Neighbourhood = pi_rNeighbourhood;

    m_pTopBorder = (Byte*)pi_pTopBorder;
    m_pLeftBorder = (Byte*)pi_pLeftBorder;
    m_pRightBorder = (Byte*)pi_pRightBorder;
    m_pBottomBorder = (Byte*)pi_pBottomBorder;

    m_TopBorderPaddingBytes = pi_TopBorderPaddingBytes;
    m_LeftBorderPaddingBytes = pi_LeftBorderPaddingBytes,
    m_RightBorderPaddingBytes = pi_RightBorderPaddingBytes;
    m_BottomBorderPaddingBytes = pi_BottomBorderPaddingBytes;
    }

//-----------------------------------------------------------------------------
// Private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// private
// Initialize
//-----------------------------------------------------------------------------
void HRPPixelBuffer::Initialize(const HRPPixelNeighbourhood& pi_rNeighbourhood,
                                const HRPPixelType& pi_rPixelType)
    {
    m_Neighbourhood = pi_rNeighbourhood;

    // retrieve the number of bits per pixel
    m_BitsPerPixel = pi_rPixelType.CountPixelRawDataBits();

    }



