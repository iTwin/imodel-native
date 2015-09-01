//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAEditorN8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRAEditorN8
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAEditorN8.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRATransaction.h>
#include <Imagepp/all/h/HCDPacket.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAEditorN8::HRAEditorN8(HGSMemorySurfaceDescriptor& pi_rDescriptor)
    : HRAGenEditor()
    {
    // compute some useful information
    HPRECONDITION(pi_rDescriptor.GetPixelType() != 0);
    HPRECONDITION((pi_rDescriptor.GetPixelType()->CountPixelRawDataBits() % 8) == 0);
    m_BytesPerPixel = pi_rDescriptor.GetPixelType()->CountPixelRawDataBits() / 8;
    m_BytesPerLine  = pi_rDescriptor.GetBytesPerRow();
    m_Width         = pi_rDescriptor.GetWidth();
    m_Height        = pi_rDescriptor.GetHeight();
    pi_rDescriptor.GetOffsets(&m_XPosInRaster, &m_YPosInRaster);
    m_SLO4          = pi_rDescriptor.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;

    HPRECONDITION(pi_rDescriptor.GetPacket() != 0);
    m_pPacket = pi_rDescriptor.GetPacket();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAEditorN8::~HRAEditorN8()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
void HRAEditorN8::GetPixels(const HUINTX*   pi_pPositionsX,
                            const HUINTX*   pi_pPositionsY,
                            size_t          pi_PixelCount,
                            void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);

    // get the individual pixels
    Byte* pBuffer = (Byte*)po_pBuffer;
    Byte* pRawData;
    for(uint32_t PixelIndex = 0; PixelIndex < pi_PixelCount; PixelIndex++)
        {
        HPRECONDITION(pi_pPositionsX[PixelIndex] < m_Width);
        HPRECONDITION(pi_pPositionsY[PixelIndex] < m_Height);

        // get the raw data
        pRawData = ComputeAddress(pi_pPositionsX[PixelIndex],
                                  pi_pPositionsY[PixelIndex]);

        // copy each pixel
        memcpy(pBuffer, pRawData, m_BytesPerPixel);


        // go to the next position in the destination buffer
        pBuffer += m_BytesPerPixel;
        }
    }

//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
void HRAEditorN8::GetPixels(HUINTX  pi_PosX,
                            HUINTX  pi_PosY,
                            HSINTX  pi_DeltaX,
                            HSINTX  pi_DeltaY,
                            size_t  pi_PixelCount,
                            void*   po_pBuffer) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);
    HPRECONDITION(po_pBuffer != 0);

    const Byte* pRawData = ComputeAddress(pi_PosX, pi_PosY);
    Byte* pOut = (Byte*)po_pBuffer;

    if (pi_DeltaX != 0 && pi_DeltaY != 0)
        {
        while (pi_PixelCount > 0)
            {
            HPRECONDITION(pRawData + m_BytesPerPixel <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
            memcpy(pOut, pRawData, m_BytesPerPixel);

            pRawData += (pi_DeltaY * m_BytesPerLine) + (pi_DeltaX * m_BytesPerPixel);
            pOut += m_BytesPerPixel;
            pi_PixelCount--;
            }
        }
    else if (pi_DeltaX != 0)
        {
        while (pi_PixelCount > 0)
            {
            HPRECONDITION(pRawData + m_BytesPerPixel <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
            memcpy(pOut, pRawData, m_BytesPerPixel);

            pRawData += pi_DeltaX * m_BytesPerPixel;
            pOut += m_BytesPerPixel;
            pi_PixelCount--;
            }
        }
    else
        {
        while (pi_PixelCount > 0)
            {
            HPRECONDITION(pRawData + m_BytesPerPixel <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
            memcpy(pOut, pRawData, m_BytesPerPixel);

            pRawData += pi_DeltaY * m_BytesPerLine;
            pOut += m_BytesPerPixel;
            pi_PixelCount--;
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// GetRun
//-----------------------------------------------------------------------------
void* HRAEditorN8::GetRun(HUINTX    pi_StartPosX,
                          HUINTX    pi_StartPosY,
                          size_t    pi_PixelCount,
                          void*     pi_pTransaction) const
    {
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);

    // compute the address
    Byte* pRawData = ComputeAddress(pi_StartPosX, pi_StartPosY);

    if (pi_pTransaction)
        {
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_StartPosX,
                                                      m_YPosInRaster + pi_StartPosY,
                                                      (uint32_t)(pi_PixelCount),
                                                      1,
                                                      pi_PixelCount * m_BytesPerPixel,
                                                      pRawData);
        }

    return pRawData;
    }

//-----------------------------------------------------------------------------
// public
// SetRun
//-----------------------------------------------------------------------------
void HRAEditorN8::SetRun(HUINTX       pi_StartPosX,
                         HUINTX       pi_StartPosY,
                         size_t       pi_PixelCount,
                         const void*  pi_pRun,
                         void*        pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);
    HPRECONDITION(pi_pRun != 0);

    Byte* pRawData = ComputeAddress(pi_StartPosX, pi_StartPosY);

    if (pi_pTransaction != 0)
        {
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_StartPosX,
                                                      m_YPosInRaster + pi_StartPosY,
                                                      (uint32_t)pi_PixelCount,
                                                      1,
                                                      pi_PixelCount * m_BytesPerPixel,
                                                      pRawData);
        }

    memcpy(pRawData,
           pi_pRun,
           pi_PixelCount * m_BytesPerPixel);
    }


//-----------------------------------------------------------------------------
// public
// GetPixel
//-----------------------------------------------------------------------------
void* HRAEditorN8::GetPixel(HUINTX  pi_PosX,
                            HUINTX  pi_PosY) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);

    // compute the address
    m_pRawData = ComputeAddress(pi_PosX,
                                pi_PosY);
    m_RawDataPosX = pi_PosX;
    m_RawDataPosY = pi_PosY;

    return m_pRawData;
    }

//-----------------------------------------------------------------------------
// public
// SetPixel
//-----------------------------------------------------------------------------
void HRAEditorN8::SetPixel(HUINTX       pi_PosX,
                           HUINTX       pi_PosY,
                           const void*  pi_pValue)
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);
    HPRECONDITION(pi_pValue != 0);

    Byte* pRawData = ComputeAddress(pi_PosX, pi_PosY);

    memcpy(pRawData,
           pi_pValue,
           m_BytesPerPixel);
    }

//-----------------------------------------------------------------------------
// public
// GetNextPixel
//-----------------------------------------------------------------------------
void* HRAEditorN8::GetNextPixel() const
    {
    // compute the address
    m_pRawData += m_BytesPerPixel;

    return m_pRawData;
    }

//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAEditorN8::Clear(const void* pi_pValue,
                        void*       pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);

    // cases where a pixel is 8, 16, 24 or 32 bits long
    Byte* pData = (Byte*)m_pPacket->GetBufferAddress();
    Byte* pDataPtr = pData;

    if (pi_pTransaction != 0)
        {
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                      m_YPosInRaster,
                                                      m_Width,
                                                      m_Height,
                                                      m_pPacket->GetDataSize(),
                                                      pData);
        }

    // Check if Each bytes are equal
    // then we can use a memset.
    // if BytesPerPacket == 1 then use memset (8bits)
    //
    bool PixelEqual = true;
    for (size_t i=1; PixelEqual && (i < m_BytesPerPixel); i++)
        {
        if (((Byte*)pi_pValue)[i-1] != ((Byte*)pi_pValue)[i])
            PixelEqual = false;
        }

    HPRECONDITION(m_pPacket->GetBufferSize() >= m_BytesPerLine * m_Height);

    if (PixelEqual)
        {
        memset (pDataPtr, *((Byte*)pi_pValue), (m_BytesPerLine * m_Height));
        }
    else
        {
        // Each bytes are not equal, move it seperatly
        //
        // calculate the number of packets in the bitmap
        size_t PaddingBytes = m_BytesPerLine - (m_Width * m_BytesPerPixel);

        // clear the first line with the default composite value
        for(uint32_t PixelIndex = 0; PixelIndex < m_Width; PixelIndex++)
            {
            memcpy(pDataPtr, pi_pValue, m_BytesPerPixel);
            pDataPtr += m_BytesPerPixel;
            }

        pDataPtr += PaddingBytes;

        // set the entire bitmap with the default composite value
        // use the first line to clear all others lines
        for(uint32_t LineIndex = 1; LineIndex < m_Height; LineIndex++)
            {
            memcpy(pDataPtr, pData, m_BytesPerLine);
            pDataPtr += m_BytesPerLine;
            }

        }

    m_pPacket->SetDataSize(m_BytesPerLine * m_Height);
    }


//-----------------------------------------------------------------------------
// public
// ClearRun
//-----------------------------------------------------------------------------
void HRAEditorN8::ClearRun(HUINTX       pi_PosX,
                           HUINTX       pi_PosY,
                           size_t       pi_PixelCount,
                           const void*  pi_pValue,
                           void*        pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);

    // cases where a pixel is 8, 16, 24 or 32 bits long
    Byte* pData = ComputeAddress(pi_PosX,
                                   pi_PosY);

    if (pi_pTransaction != 0)
        {
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_PosX,
                                                      m_YPosInRaster + pi_PosY,
                                                      (uint32_t)pi_PixelCount,
                                                      1,
                                                      pi_PixelCount * m_BytesPerPixel,
                                                      pData);
        }

    // Check if Each bytes are equal
    // then we can use a memset.
    // if BytesPerPacket == 1 then use memset (8bits)
    //
    bool PixelEqual = true;
    for (size_t i=1; PixelEqual && (i < m_BytesPerPixel); i++)
        {
        if (((Byte*)pi_pValue)[i-1] != ((Byte*)pi_pValue)[i])
            PixelEqual = false;
        }

    if (PixelEqual)
        {
        memset (pData, *((Byte*)pi_pValue), pi_PixelCount * m_BytesPerPixel);
        }
    else
        {
        switch(m_BytesPerPixel)
            {
            case 2:
                {
                unsigned short* pPixels = (unsigned short*)pData;
                unsigned short Value   = *(unsigned short const*)pi_pValue;
                for (size_t i = 0; i < pi_PixelCount; ++i)
                    pPixels[i] = Value;
                }
            break;

            case 4:
                {
                uint32_t* pPixels = (uint32_t*)pData;
                uint32_t Value   = *(uint32_t const*)pi_pValue;
                for (size_t i = 0; i < pi_PixelCount; ++i)
                    pPixels[i] = Value;
                }
            break;

            default:
                {
                for (size_t i = 0; i < pi_PixelCount; ++i)
                    {
                    memcpy(pData, pi_pValue, m_BytesPerPixel);
                    pData += m_BytesPerPixel;
                    }
                }
            break;
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// MergeRun
//
// Note : The position must be relative to the image
//-----------------------------------------------------------------------------
void HRAEditorN8::MergeRuns(HUINTX       pi_StartPosX,
                            HUINTX       pi_StartPosY,
                            size_t       pi_Width,
                            size_t       pi_Height,
                            const void*  pi_pRun,
                            void*        pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX < m_XPosInRaster + m_Width  && pi_StartPosX + pi_Width  > m_XPosInRaster);
    HPRECONDITION(pi_StartPosY < m_YPosInRaster + m_Height && pi_StartPosY + pi_Height > m_YPosInRaster);

    if (pi_StartPosX == m_XPosInRaster && pi_Width == m_Width)
        {
        // compute how many lines we need to merge

        HUINTX FirstLine = MAX(pi_StartPosY, m_YPosInRaster) - m_YPosInRaster;
        uint32_t Height = (uint32_t)MIN(pi_Height, m_Height - FirstLine);

        Byte* pRun = ComputeAddress(0, FirstLine);
        if (pi_pTransaction != 0)
            {
            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                          m_YPosInRaster + FirstLine,
                                                          m_Width,
                                                          Height,
                                                          Height * m_BytesPerLine,
                                                          pRun);
            }

        memcpy(pRun, pi_pRun, Height * m_BytesPerLine);
        }
    else if (pi_StartPosX >= m_XPosInRaster) // the position is relative to the image
        {
        HUINTX PosX = pi_StartPosX - m_XPosInRaster;
        HUINTX PosY = pi_StartPosY - m_YPosInRaster;
        size_t PixelCount = m_Width - PosX;
        PixelCount = MIN(pi_Width, PixelCount);
        while (pi_Height != 0)
            {
            SetRun(PosX,
                   PosY++,
                   MIN(PixelCount, pi_Width),
                   pi_pRun,
                   pi_pTransaction);
            --pi_Height;
            }
        }
    else
        {
        size_t PixelToSkip = m_XPosInRaster - pi_StartPosX;
        Byte* pRun = (Byte*)pi_pRun + PixelToSkip * m_BytesPerPixel;
        size_t PixelCount = pi_Width - PixelToSkip;
        HUINTX PosY = pi_StartPosY - m_YPosInRaster;
        while (pi_Height != 0)
            {
            SetRun(0,
                   PosY++,
                   MIN(PixelCount, m_Width),
                   pRun,
                   pi_pTransaction);
            --pi_Height;
            }
        }
    }




//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// ComputeAddress
//-----------------------------------------------------------------------------
Byte* HRAEditorN8::ComputeAddress(HUINTX  pi_PosX,
                                    HUINTX  pi_PosY) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);

    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);

    Byte* pRawData = m_pPacket->GetBufferAddress();

    // compute the address

    // test if we are in SLO4 or SLO6
    if(m_SLO4)
        pRawData += pi_PosY * m_BytesPerLine;
    else
        pRawData += (m_Height - pi_PosY - 1) * m_BytesPerLine;

    pRawData += pi_PosX * m_BytesPerPixel;

    //:> This post condition is use to verify if we read a initialized data
    //:> call Clear() to initialize the buffer
    HPOSTCONDITION(pRawData <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
    return pRawData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HRAEditorN8::HasData() const
    {
    return m_pPacket->GetDataSize() != 0;
    }

