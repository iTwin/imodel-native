//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFRasterUtils.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>




#include <Imagepp/all/h/HGFRasterUtils.h>
#include <Imagepp/all/h/HGF2DExtent.h>
#include <Imagepp/all/h/HGF2DGrid.h>



// bit utilities... to be packaged elsewhere in a class

Byte Mask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

//*****************************************************************************
// public
// BitCopy - Copy bits from one buffer to another
//*****************************************************************************
void BitCopy(Byte*       po_pBufferOut,
              size_t        pi_OffsetOut,
              const Byte* pi_pBufferIn,
              size_t        pi_OffsetIn,
              size_t        pi_BitCount)
    {
    size_t i;

    for(i=0; i<pi_BitCount; i++)
        {
        BitSet(po_pBufferOut,pi_OffsetOut,BitGet(pi_pBufferIn,pi_OffsetIn));
        pi_OffsetOut++;
        pi_OffsetIn++;
        }
    }

//*****************************************************************************
// public
// BitCompare - Comapres two bit buffers
//
// Returns: -1 : Buffer1  < Buffer2
//           0 : Buffer1 == Buffer2
//           1 : Buffer1  > Buffer2
//*****************************************************************************
int32_t BitCompare(const Byte* pi_pBuffer1,
                 size_t        pi_Offset1,
                 const Byte* pi_pBuffer2,
                 size_t        pi_Offset2,
                 size_t        pi_BitCount)
    {
    size_t i;
    bool Bit1;
    bool Bit2;

    for(i=0; i<pi_BitCount; i++)
        {
        if((Bit1=BitGet(pi_pBuffer1,pi_Offset1)) < (Bit2=BitGet(pi_pBuffer2,pi_Offset2)))
            return(-1); // Buffer1 < Buffer2

        if((Bit1=BitGet(pi_pBuffer1,pi_Offset1)) > (Bit2=BitGet(pi_pBuffer2,pi_Offset2)))
            return(1); // Buffer1 > Buffer2

        pi_Offset1++;
        pi_Offset2++;
        }

    return(0); // Bit patterns are equal
    }

//*****************************************************************************
// public
// BitSet - Sets a bit in a buffer
//*****************************************************************************
bool BitGet(const Byte* pi_pBuffer,
             size_t        pi_Offset)
    {
    return((pi_pBuffer[pi_Offset / 8] & Mask[pi_Offset % 8]) != 0);
    }

//*****************************************************************************
// public
// BitGet - Returns a bit from a buffer
//*****************************************************************************
void BitSet(Byte* po_pBuffer,
             size_t  pi_Offset,
             bool   pi_On)
    {
    if(pi_On)
        po_pBuffer[pi_Offset / 8] |= Mask[pi_Offset % 8];
    else
        po_pBuffer[pi_Offset / 8] &= (~Mask[pi_Offset % 8]);
    }




//-----------------------------------------------------------------------------
// public
// fCopyMemoryBloc
//
//  We suppose that Extent are in Physical CoordSys.
//-----------------------------------------------------------------------------

void fCopyMemoryBloc   (void*               po_pBufDst,
                        const HGF2DExtent&  pi_rExtentDst,
                        uint32_t            pi_BytePerLineDst,
                        const void*         pi_pBufSrc,
                        const HGF2DExtent&  pi_rExtentSrc,
                        uint32_t            pi_BytePerLineSrc,
                        uint32_t            pi_BitByPixel)
    {
    HGF2DGrid SrcGrid(pi_rExtentSrc);
    HGF2DGrid DstGrid(pi_rExtentDst);

    // Get MIN of each Extent
    //
    int32_t DstMinX = DstGrid.GetXMin();
    int32_t DstMinY = DstGrid.GetYMin();
    int32_t SrcMinX = SrcGrid.GetXMin();
    int32_t SrcMinY = SrcGrid.GetYMin();

    // Find the intersect rectangle.
    //
    int32_t InterMinX = (int32_t)MAX (DstMinX, SrcMinX);
    int32_t InterMaxX = (int32_t)MIN (DstGrid.GetXMax(), SrcGrid.GetXMax());
    int32_t InterMinY = (int32_t)MAX (DstMinY, SrcMinY);
    int32_t InterMaxY = (int32_t)MIN (DstGrid.GetYMax(), SrcGrid.GetYMax());
    int32_t InterWidth= (InterMaxX - InterMinX + 1);

    // Empty overlap
    if ((InterMinX > InterMaxX) || (InterMinY > InterMaxY))
        goto WRAPUP;


    // Check if Pixel is n8
    //
    if (pi_BitByPixel >= 8)
        {
        int32_t ByteByPixel = (pi_BitByPixel / 8);

        // Set start position in the buffer
        Byte*       pDst = ((Byte*)po_pBufDst) + (((InterMinY - DstMinY) * pi_BytePerLineDst) +
                                                    ((InterMinX - DstMinX) * ByteByPixel));
        const Byte* pSrc = ((const Byte*)pi_pBufSrc) + (((InterMinY - SrcMinY) * pi_BytePerLineSrc) +
                                                          ((InterMinX - SrcMinX) * ByteByPixel));
        // Recompute Widht from BitByPixel
        InterWidth *= ByteByPixel;

        // Copy each lines
        for (; InterMinY <= InterMaxY; InterMinY++)
            {
            memcpy (pDst, pSrc, InterWidth);

            pDst += pi_BytePerLineDst;
            pSrc += pi_BytePerLineSrc;
            }
        }
    else
        // Pixel is 1,2,4 bits
        {
        int32_t DstPosX = InterMinX - DstMinX;
        int32_t SrcPosX = InterMinX - SrcMinX;

        // Set start position in the buffer
        Byte*       pDst = ((Byte*)po_pBufDst) + ((InterMinY - DstMinY) * pi_BytePerLineDst);
        const Byte* pSrc = ((const Byte*)pi_pBufSrc) + ((InterMinY - SrcMinY) * pi_BytePerLineSrc);

        // Copy each lines
        for (; InterMinY <= InterMaxY; InterMinY++)
            {
            BitCopy (pDst, DstPosX, pSrc, SrcPosX, InterWidth);

            pDst += pi_BytePerLineDst;
            pSrc += pi_BytePerLineSrc;
            }
        }

WRAPUP:
    return;
    }