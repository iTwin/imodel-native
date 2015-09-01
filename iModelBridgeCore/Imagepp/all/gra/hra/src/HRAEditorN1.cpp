//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAEditorN1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
// Class HRAEditorN1
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAEditorN1.h>

#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HGFScanlineOrientation.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRATransaction.h>
#include <Imagepp/all/h/HFCMath.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAEditorN1::HRAEditorN1(HGSMemorySurfaceDescriptor& pi_rDescriptor)
    : HRAGenEditor()
    {
    HPRECONDITION(pi_rDescriptor.GetPacket() != 0);
    HPRECONDITION(pi_rDescriptor.GetPacket()->GetCodec() == 0 ||
                  (pi_rDescriptor.GetPacket()->GetCodec() != 0 &&
                   pi_rDescriptor.GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID)));
    HPRECONDITION(pi_rDescriptor.GetPacket()->GetBufferAddress() != 0);

    // compute some useful information

    m_pPacket       = pi_rDescriptor.GetPacket();
    HPRECONDITION(pi_rDescriptor.GetPixelType()->CountPixelRawDataBits() < 8);
    m_BitsPerPixel  = (Byte)pi_rDescriptor.GetPixelType()->CountPixelRawDataBits();
    m_PixelsPerByte = 8 / m_BitsPerPixel;
    m_BytesPerLine  = pi_rDescriptor.GetBytesPerRow();
    m_Width         = pi_rDescriptor.GetWidth();
    m_Height        = pi_rDescriptor.GetHeight();
    pi_rDescriptor.GetOffsets(&m_XPosInRaster, &m_YPosInRaster);
    m_SLO4          = pi_rDescriptor.GetSLO() == HGF_UPPER_LEFT_HORIZONTAL;

    // compute a mask to extract pixels
    m_Mask = CONVERT_TO_BYTE(0xff << (8 - m_BitsPerPixel));

    // create a working buffer
    m_pTmpRun = new Byte[m_BytesPerLine];

    HPOSTCONDITION((8 % m_BitsPerPixel) == 0);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAEditorN1::~HRAEditorN1()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
void HRAEditorN1::GetPixels(const HUINTX*   pi_pPositionsX,
                            const HUINTX*   pi_pPositionsY,
                            size_t          pi_PixelCount,
                            void*           po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);

    // get the individual pixels
    Byte* pDstBuffer = (Byte*)po_pBuffer;
    Byte DstBitIndex = 0;

    Byte* pPixelValue;

    // initialize the output buffer to 0
    size_t NbBytes = pi_PixelCount / m_PixelsPerByte;
    memset(po_pBuffer, 0, NbBytes);

    Byte Remainder = (Byte)(pi_PixelCount % m_PixelsPerByte);
    if (Remainder != 0)
        *((Byte*)po_pBuffer + NbBytes) |= (0xFF >> Remainder * m_BitsPerPixel);

    for (uint32_t PixelIndex = 0; PixelIndex < pi_PixelCount; PixelIndex++)
        {
        HPOSTCONDITION(pi_pPositionsX[PixelIndex] >= 0L);
        HPOSTCONDITION(pi_pPositionsY[PixelIndex] >= 0L);

        // get the pixel value at the position requested
        pPixelValue = (Byte*)GetPixel(pi_pPositionsX[PixelIndex],
                                        pi_pPositionsY[PixelIndex]);

        // copy the pixel value in the destination buffer
        *pDstBuffer |= ((*pPixelValue & m_Mask) >> DstBitIndex);

        // goto to next position in the destination byte
        DstBitIndex += (Byte)m_BitsPerPixel;

        // if we reach the end of the byte, go to next byte
        if (DstBitIndex == 8)
            {
            // go to the next position in the destination buffer
            pDstBuffer++;
            DstBitIndex = 0;
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
void HRAEditorN1::GetPixels(HUINTX  pi_PosX,
                            HUINTX  pi_PosY,
                            HSINTX  pi_DeltaX,
                            HSINTX  pi_DeltaY,
                            size_t  pi_PixelCount,
                            void*   po_pBuffer) const
    {
    HPRECONDITION(po_pBuffer != 0);

    // get the individual pixels
    Byte* pDstBuffer = (Byte*)po_pBuffer;
    Byte DstBitIndex = 0;

    Byte* pPixelValue;

    // initialize the output buffer to 0
    size_t NbBytes = pi_PixelCount / m_PixelsPerByte;
    memset(po_pBuffer, 0, NbBytes);

    Byte Remainder = (Byte)(pi_PixelCount % m_PixelsPerByte);
    if (Remainder != 0)
        *((Byte*)po_pBuffer + NbBytes) |= (0xFF >> Remainder * m_BitsPerPixel);

    if (pi_DeltaX == 1 && pi_DeltaY == 0)
        {
        HPRECONDITION(pi_PosX + pi_PixelCount <= m_Width);

        GetRun(pi_PosX, pi_PosY, pi_PixelCount, (Byte*)po_pBuffer);
        }
    else
        {
        while (pi_PixelCount > 0)
            {
            HPRECONDITION(pi_PosX < m_Width);
            HPRECONDITION(pi_PosY < m_Height);

            // get the pixel value at the position requested
            pPixelValue = (Byte*)GetPixel(pi_PosX,
                                            pi_PosY);

            // copy the pixel value in the destination buffer
            *pDstBuffer |= ((*pPixelValue & m_Mask) >> DstBitIndex);

            // goto to next position in the destination byte
            DstBitIndex += (Byte)m_BitsPerPixel;

            // if we reach the end of the byte, go to next byte
            if(DstBitIndex == 8)
                {
                // go to the next position in the destination buffer
                pDstBuffer++;
                DstBitIndex = 0;
                }

            --pi_PixelCount;
            pi_PosX += pi_DeltaX;
            pi_PosY += pi_DeltaY;
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// GetRun
//-----------------------------------------------------------------------------
void* HRAEditorN1::GetRun(HUINTX    pi_StartPosX,
                          HUINTX    pi_StartPosY,
                          size_t    pi_PixelCount,
                          void*     pi_pTransaction) const
    {
    Byte* pOutData;
    Byte* pRawData;
    Byte  BitIndex;

    // compute the address
    ComputeAddress(pi_StartPosX,
                   pi_StartPosY,
                   &pRawData,
                   &BitIndex);

    // test if the bit index is 0
    if (BitIndex != 0)
        {
        if (pi_pTransaction != 0)
            {
            // compute to write 8 bits entry
            HUINTX StartPosX = pi_StartPosX - BitIndex;
            size_t PixelCount = pi_PixelCount + (pi_StartPosX - StartPosX);

            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + StartPosX,
                                                          m_YPosInRaster + pi_StartPosY,
                                                          (uint32_t)PixelCount,
                                                          1,
                                                          ((PixelCount * m_BitsPerPixel) + m_BitsPerPixel) / 8,
                                                          pRawData);
            }

        // fill the run with pixels
        CopyBits(m_pTmpRun,
                 0,
                 pRawData,
                 BitIndex * m_BitsPerPixel, 
                 pi_PixelCount * m_BitsPerPixel);
        m_PixelCount = pi_PixelCount;

        pOutData = m_pTmpRun;
        }
    else
        {
        if (pi_pTransaction != 0)
            {
            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_StartPosX,
                                                          m_YPosInRaster + pi_StartPosY,
                                                          (uint32_t)pi_PixelCount,
                                                          1,
                                                          ((pi_PixelCount * m_BitsPerPixel) + m_BitsPerPixel) / 8,
                                                          pRawData);
            }
        pOutData = pRawData;
        }

    return pOutData;
    }


//-----------------------------------------------------------------------------
// public
// SetRun
//-----------------------------------------------------------------------------
void HRAEditorN1::SetRun(HUINTX       pi_StartPosX,
                         HUINTX       pi_StartPosY,
                         size_t       pi_PixelCount,
                         const void*  pi_pRun,
                         void*        pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX >= 0);
    HPRECONDITION(pi_StartPosY >= 0);
    HPRECONDITION(pi_pRun != 0);

    Byte* pRawData;
    Byte  BitIndex;

    ComputeAddress(pi_StartPosX,
                   pi_StartPosY,
                   &pRawData,
                   &BitIndex);

    if (pi_pTransaction != 0)
        {
        if (BitIndex == 0)
            {
            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_StartPosX,
                                                          m_YPosInRaster + pi_StartPosY,
                                                          (uint32_t)pi_PixelCount,
                                                          1,
                                                          ((pi_PixelCount * m_BitsPerPixel) + m_BitsPerPixel) / 8,
                                                          pRawData);
            }
        else
            {
            // compute to write 8 bits entry
            HUINTX StartPosX = pi_StartPosX - BitIndex;
            size_t PixelCount = pi_PixelCount + (pi_StartPosX - StartPosX);

            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + StartPosX,
                                                          m_YPosInRaster + pi_StartPosY,
                                                          (uint32_t)PixelCount,
                                                          1,
                                                          ((PixelCount * m_BitsPerPixel) + m_BitsPerPixel) / 8,
                                                          pRawData);
            }
        }

    CopyBits(pRawData,
             (Byte)(BitIndex * m_BitsPerPixel),
             (const Byte*)pi_pRun,
             0,
             pi_PixelCount * m_BitsPerPixel);
    }

//-----------------------------------------------------------------------------
// public
// GetPixel
//-----------------------------------------------------------------------------
void* HRAEditorN1::GetPixel(HUINTX  pi_PosX,
                            HUINTX  pi_PosY) const
    {
    HPRECONDITION(pi_PosX >= 0L);
    HPRECONDITION(pi_PosY >= 0L);

    // compute the address
    ComputeAddress(pi_PosX,
                   pi_PosY,
                   &m_pRawData,
                   &m_BitIndex);

    // copy the pixel value in the destination buffer
    m_TmpValue = CONVERT_TO_BYTE(*m_pRawData << (m_BitIndex * m_BitsPerPixel));

    return &m_TmpValue;
    }


//-----------------------------------------------------------------------------
// public
// SetPixel
//-----------------------------------------------------------------------------
void HRAEditorN1::SetPixel(HUINTX       pi_PosX,
                           HUINTX       pi_PosY,
                           const void*  pi_pValue)
    {

    Byte* pRawData;
    Byte  BitIndex;
    // compute the address
    ComputeAddress(pi_PosX,
                   pi_PosY,
                   &pRawData,
                   &BitIndex);

    *pRawData |= ((m_Mask & *((Byte*)pi_pValue)) >> (BitIndex * m_BitsPerPixel));
    }

//-----------------------------------------------------------------------------
// private
// GetNextPixel
//-----------------------------------------------------------------------------
void* HRAEditorN1::GetNextPixel() const
    {
    // compute the address
    m_BitIndex++;

    if (m_BitIndex >= m_PixelsPerByte)
        {
        m_pRawData++;
        m_BitIndex = 0;
        }

    m_TmpValue = *m_pRawData << (m_BitIndex * m_BitsPerPixel);

    return &m_TmpValue;
    }

//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAEditorN1::Clear(const void*  pi_pValue,
                        void*        pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);


    if (pi_pTransaction)
        {
        ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                      m_YPosInRaster,
                                                      m_Width,
                                                      m_Height,
                                                      m_pPacket->GetDataSize(),
                                                      m_pPacket->GetBufferAddress());
        }

    // compute the byte to copy
    Byte ClearByte = 0;
    Byte Value = (m_Mask & *((Byte*)pi_pValue));
    for (uint32_t BitIndex = 0; BitIndex < 8; BitIndex += m_BitsPerPixel)
        {
        ClearByte |= (Value >> BitIndex);
        }

    // copy the byte
    size_t NumberOfBytes = m_BytesPerLine * m_Height;

    HPRECONDITION(m_pPacket->GetBufferSize() >= NumberOfBytes);
    memset(m_pPacket->GetBufferAddress(), ClearByte, NumberOfBytes);

    m_pPacket->SetDataSize(NumberOfBytes);
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAEditorN1::ClearRun(HUINTX       pi_PosX,
                           HUINTX       pi_PosY,
                           size_t       pi_PixelCount,
                           const void*  pi_pValue,
                           void*        pi_pTransaction)
    {
    HPRECONDITION(pi_pValue != 0);

    Byte Value = (m_Mask & *((Byte*)pi_pValue));

    Byte* pData;
    Byte  PixelIndex;

    ComputeAddress(pi_PosX,
                   pi_PosY,
                   &pData,
                   &PixelIndex);

    if (pi_pTransaction != 0)
        {
        if (PixelIndex == 0)
            {
            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_PosX,
                                                          m_YPosInRaster + pi_PosY,
                                                          (uint32_t)pi_PixelCount,
                                                          1,
                                                          ((pi_PixelCount * m_BitsPerPixel) + m_BitsPerPixel) / 8,
                                                          pData);
            }
        else
            {
            // compute to write 8 bits entry
            size_t PixelCount = pi_PixelCount + PixelIndex;

            ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster + pi_PosX - PixelIndex,
                                                          m_YPosInRaster + pi_PosY,
                                                          (uint32_t)PixelCount,
                                                          1,
                                                          ((PixelCount * m_BitsPerPixel) + m_BitsPerPixel) / 8,
                                                          pData);
            }
        }

    Byte ClearByte = 0;
    unsigned short BitIndex;

    // set the first byte if the beginning
    if (PixelIndex != 0)
        {
        unsigned short PixelPos = (unsigned short)(PixelIndex * m_BitsPerPixel);
        *pData &= ~(0xFF >> PixelIndex * m_BitsPerPixel);

        ClearByte = 0;
        for (BitIndex = PixelPos; BitIndex < 8; BitIndex += (unsigned short)m_BitsPerPixel)
            ClearByte |= (Value >> BitIndex);

        *pData |= ClearByte;
        pData++;
        pi_PixelCount -= (8 - PixelPos) / m_BitsPerPixel;
        }

    if (pi_PixelCount* m_BitsPerPixel >= 8)
        {
        // compute the byte to copy
        ClearByte = 0;
        for (BitIndex = 0; BitIndex < 8; BitIndex += (unsigned short)m_BitsPerPixel)
            {
            ClearByte |= (Value >> BitIndex);
            }

        size_t ByteCount = pi_PixelCount / m_PixelsPerByte;
        memset (pData, ClearByte, ByteCount);
        pi_PixelCount -= ByteCount * m_PixelsPerByte;
        pData += ByteCount;
        }

    if (pi_PixelCount != 0)
        {
        *pData &= 0xFF >> (pi_PixelCount * m_BitsPerPixel);

        ClearByte = 0;
        for (BitIndex = 0; BitIndex < pi_PixelCount * m_BitsPerPixel; BitIndex += (unsigned short)m_BitsPerPixel)
            ClearByte |= (Value >> BitIndex);

        *pData |= ClearByte;
        }
    }


//-----------------------------------------------------------------------------
// public
// MergeRun
//
// This method merge pi_pRun into the surface.
//
// Note : The position must be relative to the image
//-----------------------------------------------------------------------------
void HRAEditorN1::MergeRuns(HUINTX      pi_StartPosX,
                            HUINTX      pi_StartPosY,
                            size_t      pi_Width,
                            size_t      pi_Height,
                            const void* pi_pRun,
                            void*       pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX < m_XPosInRaster + m_Width  && pi_StartPosX + pi_Width  > m_XPosInRaster);
    HPRECONDITION(pi_StartPosY < m_YPosInRaster + m_Height && pi_StartPosY + pi_Height > m_YPosInRaster);


    if (pi_StartPosX == m_XPosInRaster && pi_Width == m_Width && m_BytesPerLine == ((pi_Width * m_BitsPerPixel) / 8))
        {
        if (pi_StartPosY < m_YPosInRaster)
            {
            size_t LineToSkip = (size_t)(m_YPosInRaster - pi_StartPosY);
            pi_pRun = (const Byte*)pi_pRun + LineToSkip * m_BytesPerLine;
            pi_StartPosY += (HUINTX)LineToSkip;
            pi_Height -= LineToSkip;
            }

        if (pi_StartPosY + pi_Height > m_YPosInRaster + m_Height)
            pi_Height = m_YPosInRaster + m_Height - pi_StartPosY;

        Byte* pData = m_pPacket->GetBufferAddress() + (pi_StartPosY - m_YPosInRaster) * m_BytesPerLine;
        if (pi_pTransaction)
            ((HRATransaction*)pi_pTransaction)->PushEntry(pi_StartPosX,
                                                          pi_StartPosY,
                                                          (uint32_t)pi_Width,
                                                          (uint32_t)pi_Height,
                                                          pi_Height * m_BytesPerLine,
                                                          pData);

        memcpy(pData, pi_pRun, m_BytesPerLine * pi_Height);
        }
    else
        {
        size_t SrcPixelToSkip = 0;

        if (pi_Height != 1)
            {
            if (pi_StartPosY < m_YPosInRaster)
                {
                size_t LineToSkip = (size_t)(m_YPosInRaster - pi_StartPosY);
                SrcPixelToSkip = LineToSkip * pi_Width;
                pi_Height -= LineToSkip;
                pi_StartPosY += (HUINTX)LineToSkip;
                }
            else
                {
                if (pi_StartPosY + pi_Height > m_YPosInRaster + m_Height)
                    pi_Height = pi_StartPosY + pi_Height - m_YPosInRaster;
                }
            }

        if (pi_StartPosX >= m_XPosInRaster)
            {
            // get buffer address into the surface
            Byte* pDstRawData;
            Byte  DstBitIndex;

            ComputeAddress(pi_StartPosX - m_XPosInRaster,
                           pi_StartPosY - m_YPosInRaster,
                           &pDstRawData,
                           &DstBitIndex);

            // compute how many pixels must be copied
            size_t PixelCount = m_Width - pi_StartPosX - m_XPosInRaster;
            PixelCount = MIN(pi_Width, PixelCount) - DstBitIndex;
            HPOSTCONDITION(PixelCount <= m_Width);
            size_t DataSize = ((PixelCount * m_BitsPerPixel) + 7) / 8;
            HPOSTCONDITION(DataSize <= m_BytesPerLine);

            while (pi_Height != 0)
                {
                if (pi_pTransaction != 0)
                    {
                    // record data before
                    ((HRATransaction*)pi_pTransaction)->PushEntry(pi_StartPosX - DstBitIndex,
                                                                  pi_StartPosY++,
                                                                  (uint32_t)(PixelCount + DstBitIndex),
                                                                  1,
                                                                  MAX(DataSize, 1),
                                                                  pDstRawData);
                    }

                size_t SrcBytesToSkip = (SrcPixelToSkip * m_BitsPerPixel) / 8;
                Byte SrcPixelIndex = (Byte)(SrcPixelToSkip - SrcBytesToSkip * m_PixelsPerByte);
                // copy the run into surface
                CopyBits(pDstRawData,
                         DstBitIndex,
                         (const Byte*)pi_pRun + SrcBytesToSkip,
                         SrcPixelIndex,
                         PixelCount * (size_t)m_BitsPerPixel);

                --pi_Height;
                SrcPixelToSkip += pi_Width;
                pDstRawData += m_BytesPerLine;
                }
            }
        else
            {
            // compute the position into the surface
            Byte* pDstRawData;
            Byte DstBitIndex;
            ComputeAddress(0,
                           m_YPosInRaster - pi_StartPosY,
                           &pDstRawData,
                           &DstBitIndex);

            HPOSTCONDITION(DstBitIndex == 0);

            // compute how many pixels must be copied
            size_t PixelCount = pi_Width - m_XPosInRaster - pi_StartPosX;
            PixelCount = MIN(PixelCount, m_Width);
            size_t DataSize = ((PixelCount * m_BitsPerPixel) + 7) / 8;
            HPOSTCONDITION(DataSize <= m_BytesPerLine);

            while (pi_Height != 0)
                {
                if (pi_pTransaction)
                    {
                    // record data before
                    ((HRATransaction*)pi_pTransaction)->PushEntry(m_XPosInRaster,
                                                                  pi_StartPosY++,
                                                                  (uint32_t)PixelCount,
                                                                  1,
                                                                  DataSize,
                                                                  pDstRawData);

                    }

                size_t SrcBytesToSkip = (SrcPixelToSkip * m_BitsPerPixel) / 8;
                Byte SrcPixelIndex = (Byte)(SrcPixelToSkip - SrcBytesToSkip * m_PixelsPerByte);
                // copy the run into surface
                CopyBits(pDstRawData,
                         DstBitIndex,
                         (const Byte*)pi_pRun + SrcBytesToSkip,
                         SrcPixelIndex,
                         PixelCount * (size_t)m_BitsPerPixel);

                --pi_Height;
                SrcPixelToSkip += pi_Width;
                pDstRawData += m_BytesPerLine;
                }

            }
        }
    }



//-----------------------------------------------------------------------------
// Private section
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// private
// ComputeAddress
//-----------------------------------------------------------------------------
void HRAEditorN1::ComputeAddress(HUINTX     pi_PosX,
                                 HUINTX     pi_PosY,
                                 Byte**   po_ppRawData,
                                 Byte*    po_pBitIndex) const
    {
    HPRECONDITION(po_ppRawData != 0);
    HPRECONDITION(po_pBitIndex != 0);
    HPRECONDITION(m_pPacket->GetBufferAddress() != 0);

    Byte* pRawData = m_pPacket->GetBufferAddress();

    // compute the address

    // test if we are in SLO4 or SLO6
    if(m_SLO4)
        pRawData += pi_PosY * m_BytesPerLine;
    else
        pRawData += (m_Height - pi_PosY - 1) * m_BytesPerLine;

    *po_ppRawData = pRawData + (pi_PosX / m_PixelsPerByte);

    *po_pBitIndex = (Byte)(pi_PosX % m_PixelsPerByte);

    //:> This post condition is use to verify if we read a initialized data
    //:> call Clear() to initialize the buffer
    HPOSTCONDITION(*po_ppRawData <= m_pPacket->GetBufferAddress() + m_pPacket->GetDataSize());
    }


//-----------------------------------------------------------------------------
// private
// CopyBits
//-----------------------------------------------------------------------------
void HRAEditorN1::CopyBits(Byte*         po_pDstBuffer,
                           Byte          pi_DstBitIndex,
                           const Byte*   pi_pSrcBuffer,
                           Byte          pi_SrcBitIndex,
                           size_t          pi_BitsToCopy) const
    {
    HPRECONDITION(po_pDstBuffer != 0);
    HPRECONDITION(pi_pSrcBuffer != 0);

    static Byte Mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    uint32_t      DstBitIndex   = pi_DstBitIndex;
    Byte*       pDstData      = po_pDstBuffer;
    uint32_t      SrcBitIndex   = pi_SrcBitIndex;
    const Byte* pSrcData      = pi_pSrcBuffer;
    size_t        BitsRemaining = pi_BitsToCopy;

    Byte Value;

    while(BitsRemaining != 0)
        {
        if((*pSrcData & Mask[SrcBitIndex]))
            Value = Mask[DstBitIndex];
        else
            Value = 0x00;

        *pDstData = (*pDstData & ~Mask[DstBitIndex]);
        *pDstData |= Value;

        SrcBitIndex++;

        if(SrcBitIndex == 8)
            {
            pSrcData++;
            SrcBitIndex = 0;
            }

        DstBitIndex++;

        if(DstBitIndex == 8)
            {
            pDstData++;
            DstBitIndex = 0;
            }

        BitsRemaining--;
        }
    }


//-----------------------------------------------------------------------------
// private
// GetRun
//-----------------------------------------------------------------------------
void HRAEditorN1::GetRun(HUINTX         pi_PosX,
                         HUINTX         pi_PosY,
                         size_t         pi_PixelCount,
                         Byte*        po_pBuffer) const
    {
    Byte* pRawData;
    Byte  BitIndex;

    // compute the address
    ComputeAddress(pi_PosX,
                   pi_PosY,
                   &pRawData,
                   &BitIndex);

    // fill the run with pixels
    CopyBits(po_pBuffer,
             0,
             pRawData,
             (Byte)(BitIndex * m_BitsPerPixel),
             pi_PixelCount * m_BitsPerPixel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HRAEditorN1::HasData() const
    {
    return m_pPacket->GetDataSize() != 0;
    }