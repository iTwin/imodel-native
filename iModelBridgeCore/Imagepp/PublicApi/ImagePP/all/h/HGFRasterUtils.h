//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRasterUtils.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


class HGF2DExtent;




void BitCopy(Byte* po_pBufferOut,
             size_t  pi_OffsetOut,
             const Byte* pi_pBufferIn,
             size_t  pi_OffsetIn,
             size_t  pi_BitCount);

int32_t BitCompare(const Byte* po_pBuffer1,
                 size_t  pi_Offset1,
                 const Byte* pi_pBuffer2,
                 size_t  pi_Offset2,
                 size_t  pi_BitCount);

bool BitGet(const Byte* pi_pBuffer,
             size_t  pi_Offset);

void BitSet(Byte* po_pBuffer,
            size_t  pi_Offset,
            bool   pi_On);



void fCopyMemoryBloc   (void*               po_pBufDst,
                        const HGF2DExtent&  pi_rExtentDst,
                        uint32_t            pi_BytePerLineDst,
                        const void*         pi_pBufSrc,
                        const HGF2DExtent&  pi_rExtentSrc,
                        uint32_t            pi_BytePerLineSrc,
                        size_t              pi_BitByPixel);



