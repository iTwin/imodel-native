//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI1R8G8B8A8RLE.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>+--------------------------------------------------------------------------
// Methods for class HRPPixelTypeI1R8G8B8A8RLE
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8A8.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HFCMath.h>

static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };


HPM_REGISTER_CLASS(HRPPixelTypeI1R8G8B8A8RLE, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLER8G8B8A8RLE
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_I1R8G8B8A8RLE : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8RLE_I1R8G8B8A8RLE()
        {
        };

    virtual ~ConverterI1R8G8B8A8RLE_I1R8G8B8A8RLE()
        {
        };
     
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        bool DstOnState = false;
        bool SrcOnState = (m_ConvertEntryConversion[DstOnState] != 0);

        uint32_t NewValue;
        uint32_t PixelsCopied;

        *pDst = 0;

        while(pi_PixelsCount != 0)
            {
            if(*pSrc != 0)
                {
                if(DstOnState != SrcOnState)
                    {
                    pDst++;
                    *pDst = 0;
                    DstOnState = !DstOnState;
                    }

                // test if we reach the limit
                NewValue = *pDst + *pSrc;
                if(NewValue > 32767)
                    {
                    PixelsCopied = 32767 - *pDst;
                    *pDst = 32767;
                    pDst++;
                    *pDst = 0;
                    pDst++;
                    *pDst = (unsigned short)(*pSrc - PixelsCopied);
                    }
                else
                    {
                    *pDst = (unsigned short)NewValue;
                    }

                pi_PixelsCount -= MIN(*pSrc, pi_PixelsCount);
                }

            pSrc++;
            SrcOnState = !SrcOnState;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // To simplify RLE algo and avoid confusion between run len and run value
        // we will assume 0 to be black and 1 to be white. This is not the real pixel value
        // since the black and white position is determined by the pixel type.
        unsigned short const* pSrc = (unsigned short const*)pi_pSourceRawData;
        unsigned short*       pFinalDst = (unsigned short*)pio_pDestRawData;

        // Alloc a buffer able to hold the original destination.
        size_t RunCount = 0;
        for(size_t i(0); i < pi_PixelsCount; i+=pFinalDst[RunCount++]);
        HArrayAutoPtr<unsigned short> pTempDst(new unsigned short[RunCount]);

        // Copy the dest.
        unsigned short* pDst = pTempDst.get();
        memcpy(pDst, pFinalDst, sizeof(unsigned short)*RunCount);

        // We start on black
        bool DstOnState = false;
        bool SrcOnState = false;
        Byte FinalDstOnState = 0;

        uint32_t DstRunLen = *pDst;
        uint32_t SrcRunLen = *pSrc;
        uint32_t FinalDstRunLen = 0;

        while(pi_PixelsCount)
            {
            // Get more pixels from the destination
            while(DstRunLen == 0)
                {
                ++pDst;
                DstRunLen = *pDst;
                DstOnState = !DstOnState;
                }

            // Get more pixels from the source
            while(SrcRunLen == 0)
                {
                ++pSrc;
                SrcRunLen = *pSrc;
                SrcOnState = !SrcOnState;
                }

            // If the current pixel is not on the same state than the current one,
            // we write the current run and start a new one.
            if(FinalDstOnState != m_ComposeEntryConversion[SrcOnState][DstOnState])      // black runs are ON even number. 0,2,4,6...
                {
                // Write what is exceeding the MAX run len
                while(FinalDstRunLen > SHRT_MAX)
                    {
                    *pFinalDst++ = SHRT_MAX;
                    FinalDstRunLen -= SHRT_MAX;
                    *pFinalDst++ = 0;
                    }

                *pFinalDst++ = (unsigned short)FinalDstRunLen;
                // FinalDstRunLen = 0;    It should be 0 at this stage but we will set it to 1 below
                FinalDstOnState = !FinalDstOnState;

                // Add/subtract one pixel
                FinalDstRunLen = 1;
                --DstRunLen;
                --SrcRunLen;
                --pi_PixelsCount;
                }
            else
                {
                // The FinalDstOnState won't change until SrcOnState or DstOnState change so
                // we can safely skip the MIN pixel count between pSrc, pDst and pixelCount.
                uint32_t MinRunLen((uint32_t)MIN(DstRunLen, MIN(SrcRunLen, pi_PixelsCount)));

                FinalDstRunLen += MinRunLen;
                DstRunLen -= MinRunLen;
                SrcRunLen -= MinRunLen;
                pi_PixelsCount -= MinRunLen;
                }
            }

        // Write what is exceeding the MAX run len
        while(FinalDstRunLen > SHRT_MAX)
            {
            *pFinalDst++ = SHRT_MAX;
            FinalDstRunLen -= SHRT_MAX;
            *pFinalDst++ = 0;
            }

        // Write the last run
        *pFinalDst = (unsigned short)FinalDstRunLen;
        //++pFinalDst;
        //FinalDstOnState = !FinalDstOnState;
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        HASSERT(0);
        };

    virtual HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_I1R8G8B8A8RLE(*this));
        };

protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        HRPPaletteOctreeR8G8B8A8 destPalOctree;

        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(0), 0);
        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(1), 1);

        // *** Build m_ConvertEntryConversion
        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(Byte Index = 0; Index < 2; Index++)
                m_ConvertEntryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 2; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                m_ConvertEntryConversion[Index] = destPalOctree.GetIndex(pSrcValue[0], pSrcValue[1], pSrcValue[2], pSrcValue[3]);
                }
            }

        // *** Build m_composeEntryConversion
        Byte* pSrcValue;
        Byte* pDestValue;
        Byte TmpDst[4];

        // for each entry in the source palette, calculate the closest entry
        // in the destination palette
        for(uint32_t SrcIndex = 0; SrcIndex < 2; SrcIndex++)
            {
            pSrcValue = (Byte*)rSrcPixelPalette.GetCompositeValue(SrcIndex);

            for(uint32_t DstIndex = 0; DstIndex < 2; DstIndex++)
                {
                pDestValue = (Byte*)rDestPixelPalette.GetCompositeValue(DstIndex);

                // If source pixel is fully transparent, destination is unaltered
                if (pSrcValue[3] != 0)
                    {
                    if (pDestValue[3] == 0 || pSrcValue[3] == 255)
                        {
                        // Destination pixel is fully transparent, or source pixel
                        // is fully opaque. Copy source pixel,
                        *((uint32_t*)TmpDst) = *((uint32_t*)pSrcValue);
                        }
                    else
                        {
                        HFCMath (*pQuotients) (HFCMath::GetInstance());

                        // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                        // Alphas are in [0, 255].
                        Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[0]);
                        TmpDst[0] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[0] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[1]);
                        TmpDst[1] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[1] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[2]);
                        TmpDst[2] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[2] - PremultDestColor)) + PremultDestColor;

                        // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                        // --> Transparency percentages are multiplied
                        TmpDst[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSrcValue[3]) * (255 - pDestValue[3])));
                        }
                    }
                else
                    {
                    // Source is completely transparent.
                    *((uint32_t*)TmpDst) = *((uint32_t*)pDestValue);
                    }

                // find the closest entry index in the destination palette
                m_ComposeEntryConversion[SrcIndex][DstIndex] = destPalOctree.GetIndex(TmpDst[0], TmpDst[1], TmpDst[2], TmpDst[3]);
                }
            }
        };

private:

    // conversion table
    Byte m_ConvertEntryConversion[2];
    Byte m_ComposeEntryConversion[2][2];
    };
static ConverterI1R8G8B8A8RLE_I1R8G8B8A8RLE s_I1R8G8B8A8RLE_I1R8G8B8A8RLE;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLER8G8B8RLE
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_I1R8G8B8RLE : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8RLE_I1R8G8B8RLE()
        {
        };

    virtual ~ConverterI1R8G8B8A8RLE_I1R8G8B8RLE()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        bool DstOnState = false;
        bool SrcOnState = (EntryConversion[DstOnState] != 0);

        uint32_t NewValue;
        uint32_t PixelsCopied;

        *pDst = 0;

        while(pi_PixelsCount != 0)
            {
            if(*pSrc != 0)
                {
                if(DstOnState != SrcOnState)
                    {
                    pDst++;
                    *pDst = 0;
                    DstOnState = !DstOnState;
                    }

                // test if we reach the limit
                NewValue = *pDst + *pSrc;
                if(NewValue > 32767)
                    {
                    PixelsCopied = 32767 - *pDst;
                    *pDst = 32767;
                    pDst++;
                    *pDst = 0;
                    pDst++;
                    *pDst = (unsigned short)(*pSrc - PixelsCopied);
                    }
                else
                    {
                    *pDst = (unsigned short)NewValue;
                    }

                pi_PixelsCount -= MIN(*pSrc, pi_PixelsCount);
                }

            pSrc++;
            SrcOnState = !SrcOnState;
            }
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        HASSERT(0);
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // To simplify RLE algo and avoid confusion between run len and run value
        // we will assume 0 to be black and 1 to be white. This is not the real pixel value
        // since the black and white position is determined by the pixel type.

        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pFinalDst = (unsigned short*)pio_pDestRawData;

        // Alloc a buffer able to hold the original destination.
        size_t RunCount = 0;
        for(size_t i(0); i < pi_PixelsCount; i+=pFinalDst[RunCount++]);
        HArrayAutoPtr<unsigned short> pTempDst(new unsigned short[RunCount]);

        // Copy the dest.
        unsigned short* pDst = pTempDst.get();
        memcpy(pDst, pFinalDst, sizeof(unsigned short)*RunCount);

        // We start on black
        bool DstOnState = false;
        bool SrcOnState = false;
        Byte FinalDstOnState = 0;

        uint32_t DstRunLen = *pDst;
        uint32_t SrcRunLen = *pSrc;
        uint32_t FinalDstRunLen = 0;

        while(pi_PixelsCount)
            {
            // Get more pixels from the destination
            while(DstRunLen == 0)
                {
                ++pDst;
                DstRunLen = *pDst;
                DstOnState = !DstOnState;
                }

            // Get more pixels from the source
            while(SrcRunLen == 0)
                {
                ++pSrc;
                SrcRunLen = *pSrc;
                SrcOnState = !SrcOnState;
                }

            // If the current pixel is not on the same state than the current one,
            // we write the current run and start a new one.
            if(FinalDstOnState != m_AlphaEntryConversion[SrcOnState][DstOnState])
                {
                // Write what is exceeding the MAX run len
                while(FinalDstRunLen > SHRT_MAX)
                    {
                    *pFinalDst++ = SHRT_MAX;
                    FinalDstRunLen -= SHRT_MAX;
                    *pFinalDst++ = 0;
                    }

                *pFinalDst++ = (unsigned short)FinalDstRunLen;
                // FinalDstRunLen = 0;    It should be 0 at this stage but we will set it to 1 below
                FinalDstOnState = !FinalDstOnState;

                // Add/subtract one pixel
                FinalDstRunLen = 1;
                --DstRunLen;
                --SrcRunLen;
                --pi_PixelsCount;
                }
            else
                {
                // The FinalDstOnState won't change until SrcOnState or DstOnState change so
                // we can safely skip the MIN pixel count between pSrc, pDst and pixelCount.
                uint32_t MinRunLen((uint32_t)MIN(DstRunLen, MIN(SrcRunLen, pi_PixelsCount)));

                FinalDstRunLen += MinRunLen;
                DstRunLen -= MinRunLen;
                SrcRunLen -= MinRunLen;
                pi_PixelsCount -= MinRunLen;
                }
            }

        // Write what is exceeding the MAX run len
        while(FinalDstRunLen > SHRT_MAX)
            {
            *pFinalDst++ = SHRT_MAX;
            FinalDstRunLen -= SHRT_MAX;
            *pFinalDst++ = 0;
            }

        // Write the last run
        *pFinalDst = (unsigned short)FinalDstRunLen;
        //++pFinalDst;
        //FinalDstOnState = !FinalDstOnState;
        };


    virtual HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_I1R8G8B8RLE(*this));
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(Byte Index = 0; Index < 2; Index++)
                EntryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 2; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                EntryConversion[Index] = (Byte)GetClosestEntryIndexInPalette(pSrcValue,rDestPixelPalette);
                }
            }

        Byte* pSrcValue;
        Byte* pDestValue;
        Byte TmpDst[3];

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // for each entry in the source palette, calculate the closest entry
        // in the destination palette
        for(uint32_t SrcIndex = 0; SrcIndex < 2; SrcIndex++)
            {
            pSrcValue = (Byte*)rSrcPixelPalette.GetCompositeValue(SrcIndex);

            for(uint32_t DstIndex = 0; DstIndex < 2; DstIndex++)
                {
                pDestValue = (Byte*)rDestPixelPalette.GetCompositeValue(DstIndex);

                // alpha * (S - D) + D
                TmpDst[0] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[0] - pDestValue[0])) + pDestValue[0];
                TmpDst[1] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[1] - pDestValue[1])) + pDestValue[1];
                TmpDst[2] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[2] - pDestValue[2])) + pDestValue[2];

                // find the closest entry index in the destination palette
                m_AlphaEntryConversion[SrcIndex][DstIndex] = (Byte)GetClosestEntryIndexInPalette(TmpDst,rDestPixelPalette);
                }
            }
        };

private:

    // get the closest index in a palette from a given composite color
    size_t GetClosestEntryIndexInPalette(Byte* pi_pValue, const HRPPixelPalette& pi_rPixelPalette) const
        {
        Byte* pValue;
        int RDist, GDist, BDist;
        uint32_t Dist;
        size_t IndexForSmallerDist=0;

        // At the beginning, define the smaller distance to be larger
        // than the maximum distance
        uint32_t SmallerDist = 0xFFFFFFFF;

        // For each entry of the palette, verify if the
        // composite value associated is closer to the input composite value
        for(uint32_t Index = 0; Index < 2; Index++)
            {
            if (Index == pi_rPixelPalette.GetLockedEntryIndex())
                continue;//Skip locked entry

            // Get the composite value associated to the index in the palette
            pValue = (Byte*)pi_rPixelPalette.GetCompositeValue(Index);

            // Calculate the distance between the two composite values
            RDist = pValue[0] - pi_pValue[0];
            GDist = pValue[1] - pi_pValue[1];
            BDist = pValue[2] - pi_pValue[2];
            Dist = RDist * RDist + GDist * GDist + BDist * BDist;

            // Is the distance smaller than before?
            if(Dist < SmallerDist)
                {
                // copy the distance and the index in the palette associated
                SmallerDist = Dist;
                IndexForSmallerDist = Index;
                }
            }

        return IndexForSmallerDist;

        };

    static short m_LostChannels[];

    // conversion table
    Byte EntryConversion[2];
    Byte m_AlphaEntryConversion[2][2];
    };

short ConverterI1R8G8B8A8RLE_I1R8G8B8RLE::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8RLE_I1R8G8B8RLE s_I1R8G8B8A8RLE_I1R8G8B8RLE;



//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_I1R8G8B8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_I1R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        size_t RunLen;
        uint32_t Index = 0;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                if(EntryConversion[OnState] == 1)
                    *pDest = *pDest | s_BitMask[Index];
                else
                    *pDest = *pDest & s_NotBitMask[Index];

                RunLen--;

                // Increment Source
                Index++;
                if(Index == 8)
                    {
                    // Start writing to a new destination byte
                    Index = 0;
                    pDest++;
                    }
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        size_t RunLen;
        uint32_t Index = 0;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                if(AlphaEntryConversion[OnState][(*pDest & s_BitMask[Index]) >> (7 - Index)] == 1)
                    *pDest = *pDest | s_BitMask[Index];
                else
                    *pDest = *pDest & s_NotBitMask[Index];

                RunLen--;

                // Increment Source
                Index++;
                if(Index == 8)
                    {
                    // Start writing to a new destination byte
                    Index = 0;
                    pDest++;
                    }
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        }

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_I1R8G8B8(*this));
        }

protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(Byte Index = 0; Index < 2; Index++)
                EntryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 2; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                EntryConversion[Index] = (Byte)GetClosestEntryIndexInPalette(pSrcValue,rDestPixelPalette);
                }
            }

        Byte* pSrcValue;
        Byte* pDestValue;
        Byte TmpDst[3];

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // for each entry in the source palette, calculate the closest entry
        // in the destination palette
        for(uint32_t SrcIndex = 0; SrcIndex < 2; SrcIndex++)
            {
            pSrcValue = (Byte*)rSrcPixelPalette.GetCompositeValue(SrcIndex);

            for(uint32_t DstIndex = 0; DstIndex < 2; DstIndex++)
                {
                pDestValue = (Byte*)rDestPixelPalette.GetCompositeValue(DstIndex);

                // alpha * (S - D) + D
                TmpDst[0] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[0] - pDestValue[0])) + pDestValue[0];
                TmpDst[1] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[1] - pDestValue[1])) + pDestValue[1];
                TmpDst[2] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[2] - pDestValue[2])) + pDestValue[2];

                // find the closest entry index in the destination palette
                AlphaEntryConversion[SrcIndex][DstIndex] = (Byte)GetClosestEntryIndexInPalette(TmpDst,rDestPixelPalette);
                }
            }
        };

private:

    // get the closest index in a palette from a given composite color
    size_t GetClosestEntryIndexInPalette(Byte* pi_pValue, const HRPPixelPalette& pi_rPixelPalette) const
        {
        Byte* pValue;
        int RDist, GDist, BDist;
        uint32_t Dist;
        size_t IndexForSmallerDist=0;

        // At the beginning, define the smaller distance to be larger
        // than the maximum distance
        uint32_t SmallerDist = 0xFFFFFFFF;

        // For each entry of the palette, verify if the
        // composite value associated is closer to the input composite value
        for(uint32_t Index = 0; Index < 2; Index++)
            {
            if (Index == pi_rPixelPalette.GetLockedEntryIndex())
                continue;//Skip locked entry

            // Get the composite value associated to the index in the palette
            pValue = (Byte*)pi_rPixelPalette.GetCompositeValue(Index);

            // Calculate the distance between the two composite values
            RDist = pValue[0] - pi_pValue[0];
            GDist = pValue[1] - pi_pValue[1];
            BDist = pValue[2] - pi_pValue[2];
            Dist = RDist * RDist + GDist * GDist + BDist * BDist;

            // Is the distance smaller than before?
            if(Dist < SmallerDist)
                {
                // copy the distance and the index in the palette associated
                SmallerDist = Dist;
                IndexForSmallerDist = Index;
                }
            }

        return IndexForSmallerDist;

        };

    static short m_LostChannels[];
    // conversion table
    Byte EntryConversion[2];
    Byte AlphaEntryConversion[2][2];
    };
short ConverterI1R8G8B8A8RLE_I1R8G8B8::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8RLE_I1R8G8B8 s_I1R8G8B8A8RLE_I1R8G8B8;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_I1R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_I1R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        size_t RunLen;
        uint32_t Index = 0;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                if(m_entryConversion[OnState] == 1)
                    *pDest = *pDest | s_BitMask[Index];
                else
                    *pDest = *pDest & s_NotBitMask[Index];

                --RunLen;

                // Increment Source
                ++Index;
                if(Index == 8)
                    {
                    // Start writing to a new destination byte
                    Index = 0;
                    ++pDest;
                    }
                }

            pSrcRun++;
            OnState = !OnState;
            }
        
        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        size_t RunLen;
        uint32_t Index = 0;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                if(m_alphaEntryConversion[OnState][(*pDest & s_BitMask[Index]) >> (7 - Index)] == 1)
                    *pDest = *pDest | s_BitMask[Index];
                else
                    *pDest = *pDest & s_NotBitMask[Index];

                --RunLen;

                // Increment Source
                ++Index;
                if(Index == 8)
                    {
                    // Start writing to a new destination byte
                    Index = 0;
                    ++pDest;
                    }
                }

            pSrcRun++;
            OnState = !OnState;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_I1R8G8B8A8(*this));
        }

protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        HRPPaletteOctreeR8G8B8A8 destPalOctree;

        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(0), 0);
        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(1), 1);

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(Byte Index = 0; Index < 2; Index++)
                m_entryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 2; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                m_entryConversion[Index] = destPalOctree.GetIndex(pSrcValue[0], pSrcValue[1], pSrcValue[2], pSrcValue[3]);
                }
            }

        Byte* pSrcValue;
        Byte* pDestValue;
        Byte TmpDst[4];

        // for each entry in the source palette, calculate the closest entry
        // in the destination palette
        for(uint32_t SrcIndex = 0; SrcIndex < 2; SrcIndex++)
            {
            pSrcValue = (Byte*)rSrcPixelPalette.GetCompositeValue(SrcIndex);

            for(uint32_t DstIndex = 0; DstIndex < 2; DstIndex++)
                {
                pDestValue = (Byte*)rDestPixelPalette.GetCompositeValue(DstIndex);

                // If source pixel is fully transparent, destination is unaltered
                if (pSrcValue[3] != 0)
                    {
                    if (pDestValue[3] == 0 ||
                        pSrcValue[3] == 255)
                        {
                        // Destination pixel is fully transparent, or source pixel
                        // is fully opaque. Copy source pixel,
                        *((uint32_t*)TmpDst) = *((uint32_t*)pSrcValue);
                        }
                    else
                        {
                        HFCMath (*pQuotients) (HFCMath::GetInstance());

                        // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                        // Alphas are in [0, 255].
                        Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[0]);
                        TmpDst[0] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[0] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[1]);
                        TmpDst[1] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[1] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[2]);
                        TmpDst[2] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[2] - PremultDestColor)) + PremultDestColor;

                        // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                        // --> Transparency percentages are multiplied
                        TmpDst[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSrcValue[3]) * (255 - pDestValue[3])));
                        }
                    }
                else
                    {
                    // Source is completely transparent.
                    *((uint32_t*)TmpDst) = *((uint32_t*)pDestValue);
                    }

                // find the closest entry index in the destination palette
                m_alphaEntryConversion[SrcIndex][DstIndex] = destPalOctree.GetIndex(TmpDst[0], TmpDst[1], TmpDst[2], TmpDst[3]);
                }
            }
        };

private:

    // conversion table
    Byte m_entryConversion[2];
    Byte m_alphaEntryConversion[2][2];
    };
static ConverterI1R8G8B8A8RLE_I1R8G8B8A8 s_I1R8G8B8A8RLE_I1R8G8B8A8;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class ConverterI1R8G8B8A8_I1R8G8B8A8RLE : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        const Byte* pSrc = (const Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        // Optimize when everything map to the same entry. 
        // Anyways normal path goes into infinite loop!
        if(m_entryConversion[0] == m_entryConversion[1])
            {
            if(m_entryConversion[0])
                {
                *pDest = 0;
                ++pDest;
                }

            while(pi_PixelsCount > 32767)  
                {
                pi_PixelsCount -= 32767;
                pDest[0] = 32767;
                pDest[1] = 0;
                pDest+=2;
                }

            *pDest = (unsigned short)pi_PixelsCount;

            // end on FALSE(aka BLACK) state.
            if(m_entryConversion[0])
                pDest[1] = 0;
                
            return;
            }
        
        size_t PixelCount(0);
        bool OnState(false);
        size_t Index = 0;

        while (PixelCount < pi_PixelsCount)
            {
            // Count bits
            uint32_t BitCount(0);
            while (PixelCount < pi_PixelsCount)
                {
                if ((Byte)OnState != m_entryConversion[(*pSrc & s_BitMask[Index]) >> (7 - Index)])
                    break;

                ++BitCount;
                ++PixelCount;
                ++Index;
                if (Index == 8)
                    {
                    Index = 0;
                    ++pSrc;
                    }
                }

            while (BitCount > 32767)
                {
                pDest[0] = 32767;
                pDest[1] = 0;
                pDest += 2;
                BitCount -= 32767;
                }
            *pDest = (unsigned short)BitCount;
            ++pDest;
            OnState = !OnState;
            }

        // must end with black.
        if (!OnState)
            *pDest = 0;
        }

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // To simplify RLE algo and avoid confusion between run len and run value
        // we will assume 0 to be black and 1 to be white. This is not the real pixel value
        // since the black and white position is determined by the pixel type.
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pFinalDst = (unsigned short*)pio_pDestRawData;

        // Alloc a buffer able to hold the original destination.
        size_t RunCount = 0;
        for(size_t i(0); i < pi_PixelsCount; i+=pFinalDst[RunCount++]);
        HArrayAutoPtr<unsigned short> pTempDst(new unsigned short[RunCount]);

        // Copy the dest.
        unsigned short* pDst = pTempDst.get();
        memcpy(pDst, pFinalDst, sizeof(unsigned short)*RunCount);

        // We start on black
        uint32_t DstIndex = 0;
        Byte FinalDstOnState = 0;
        Byte SrcIndexInByte = 0;

        uint32_t DstRunLen = 0;
        uint32_t FinalDstRunLen = 0;

        while(pi_PixelsCount)
            {
            // Get more pixels from the destination
            while(pDst[DstIndex] == 0)
                ++DstIndex;

            // Make sure current run do not exceed the number of pixels to process.
            // Also subtract pi_PixelsCount now because we decrement DstRunLen
            if(pDst[DstIndex] < pi_PixelsCount)
                {
                DstRunLen = pDst[DstIndex];
                pi_PixelsCount-=DstRunLen;
                }
            else
                {
                DstRunLen = (uint32_t)pi_PixelsCount;
                pi_PixelsCount = 0;
                }

            while(DstRunLen)
                {
                // If the current pixel is not on the same state than the current one we write the current run and start a new one.
                if(FinalDstOnState == m_alphaEntryConversion[(*pSrc >> (7- SrcIndexInByte)) & 0x01][DstIndex & 0x00000001]) // black runs are ON even number. 0,2,4,6...
                    {
                    // Add one pixel to the current state.
                    ++FinalDstRunLen;
                    --DstRunLen;
                    }
                else
                    {
                    // Write what is exceeding the MAX run len
                    while(FinalDstRunLen > SHRT_MAX)
                        {
                        *pFinalDst++ = SHRT_MAX;
                        FinalDstRunLen -= SHRT_MAX;
                        *pFinalDst++ = 0;
                        }

                    *pFinalDst++ = (unsigned short)FinalDstRunLen;
                    // FinalDstRunLen = 0;    It should be 0 at this stage but we will set it to 1 below
                    FinalDstOnState = !FinalDstOnState;

                    // Add/subtract one pixel
                    FinalDstRunLen = 1;
                    --DstRunLen;
                    }

                // Next source pixel
                ++SrcIndexInByte;
                if(SrcIndexInByte == 8)
                    {
                    // Start reading to a new src byte
                    SrcIndexInByte = 0;
                    ++pSrc;
                    }
                }

            // Next RLE run.
            ++DstIndex;
            }

        // Write what is exceeding the MAX run len
        while(FinalDstRunLen > SHRT_MAX)
            {
            *pFinalDst++ = SHRT_MAX;
            FinalDstRunLen -= SHRT_MAX;
            *pFinalDst++ = 0;
            }

        // Write the last run
        *pFinalDst = (unsigned short)FinalDstRunLen;
        //++pFinalDst;
        //FinalDstOnState = !FinalDstOnState;
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8_I1R8G8B8A8RLE(*this));
        }

protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        HRPPaletteOctreeR8G8B8A8 destPalOctree;

        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(0), 0);
        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(1), 1);

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(Byte Index = 0; Index < 2; Index++)
                m_entryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 2; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                m_entryConversion[Index] = destPalOctree.GetIndex(pSrcValue[0], pSrcValue[1], pSrcValue[2], pSrcValue[3]);
                }
            }

        Byte* pSrcValue;
        Byte* pDestValue;
        Byte TmpDst[4];

        // for each entry in the source palette, calculate the closest entry
        // in the destination palette
        for(uint32_t SrcIndex = 0; SrcIndex < 2; SrcIndex++)
            {
            pSrcValue = (Byte*)rSrcPixelPalette.GetCompositeValue(SrcIndex);

            for(uint32_t DstIndex = 0; DstIndex < 2; DstIndex++)
                {
                pDestValue = (Byte*)rDestPixelPalette.GetCompositeValue(DstIndex);

                // If source pixel is fully transparent, destination is unaltered
                if (pSrcValue[3] != 0)
                    {
                    if (pDestValue[3] == 0 ||
                        pSrcValue[3] == 255)
                        {
                        // Destination pixel is fully transparent, or source pixel
                        // is fully opaque. Copy source pixel,
                        *((uint32_t*)TmpDst) = *((uint32_t*)pSrcValue);
                        }
                    else
                        {
                        HFCMath (*pQuotients) (HFCMath::GetInstance());

                        // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                        // Alphas are in [0, 255].
                        Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[0]);
                        TmpDst[0] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[0] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[1]);
                        TmpDst[1] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[1] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestValue[3] * pDestValue[2]);
                        TmpDst[2] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[2] - PremultDestColor)) + PremultDestColor;

                        // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                        // --> Transparency percentages are multiplied
                        TmpDst[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSrcValue[3]) * (255 - pDestValue[3])));
                        }
                    }
                else
                    {
                    // Source is completely transparent.
                    *((uint32_t*)TmpDst) = *((uint32_t*)pDestValue);
                    }

                // find the closest entry index in the destination palette
                m_alphaEntryConversion[SrcIndex][DstIndex] = destPalOctree.GetIndex(TmpDst[0], TmpDst[1], TmpDst[2], TmpDst[3]);
                }
            }
        };

private:

    // conversion table
    Byte m_entryConversion[2];
    Byte m_alphaEntryConversion[2][2];
    };
static ConverterI1R8G8B8A8_I1R8G8B8A8RLE s_I1R8G8B8A8_I1R8G8B8A8RLE;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_V32R8G8B8A8 : public HRPPixelConverter
    {

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const uint32_t* pColor[2];
        pColor[0] = (uint32_t*)(rPalette.GetCompositeValue(0));
        pColor[1] = (uint32_t*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                *((uint32_t*)pDest) = *(pColor[OnState]);

                pDest+=4;

                RunLen--;
                }

            pSrcRun++;
            OnState = !OnState;
            }
        };

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            // is the color opaque?
            if(pColor[OnState][3] == 255)
                {
                while(RunLen != 0)
                    {
                    *((uint32_t*)pDest) = *((uint32_t*)pColor[OnState]);

                    // Increment Destination by 4 bytes !
                    pDest+= 4;

                    RunLen--;
                    }
                }
            // color is semi-transparent
            else if (pColor[OnState][3] > 0)
                {
                uint32_t tempValue(255 - pColor[OnState][3]);
                while(RunLen != 0)
                    {
                    // if dest is fully transparent
                    if(pDest[3] == 0)
                        {
                        *((uint32_t*)pDest) = *((uint32_t*)pColor[OnState]);
                        }
                    else
                        {
                        // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                        // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                        Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDest[3] * pDest[0]);
                        pDest[0] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][0] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDest[3] * pDest[1]);
                        pDest[1] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][1] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDest[3] * pDest[2]);
                        pDest[2] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][2] - PremultDestColor)) + PremultDestColor;

                        // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                        // --> Transparency percentages are multiplied
                        pDest[3] = 255 - pQuotients->DivideBy255ToByte(tempValue * (255 - pDest[3]));
                        }

                    // Increment Destination by 4 bytes !
                    pDest+= 4;

                    RunLen--;
                    }
                }
            // transparent case
            else
                {
                // go to next section
                pDest += (4 * RunLen);
                }

            pSrcRun++;
            OnState = !OnState;
            }


        };

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_V32R8G8B8A8(*this));
        }

private:

    };
static ConverterI1R8G8B8A8RLE_V32R8G8B8A8 s_I1R8G8B8A8RLE_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_V24B8G8R8 : public HRPPixelConverter
    {

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                pDest[2] = pColor[OnState][0];
                pDest[1] = pColor[OnState][1];
                pDest[0] = pColor[OnState][2];

                pDest+=3;

                RunLen--;
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }
        };


    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            // is the color opaque?
            if(pColor[OnState][3] == 255)
                {
                while(RunLen != 0)
                    {
                    pDest[2] = pColor[OnState][0];
                    pDest[1] = pColor[OnState][1];
                    pDest[0] = pColor[OnState][2];

                    // Increment Destination by 3 bytes !
                    pDest+=3;

                    RunLen--;
                    }
                }
            // color is semi-transparent
            else if (pColor[OnState][3] > 0)
                {
                HFCMath (*pQuotients) (HFCMath::GetInstance());

                while(RunLen != 0)
                    {
                    // alpha * (S - D) + D
                    pDest[2] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][0] - pDest[2])) + pDest[2];
                    pDest[1] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][1] - pDest[1])) + pDest[1];
                    pDest[0] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][2] - pDest[0])) + pDest[0];

                    // Increment Destination by 3 bytes !
                    pDest+=3;

                    --RunLen;
                    }
                }
            // transparent case
            else
                {
                // go to next section
                pDest += (3 * RunLen);
                }

            pSrcRun++;
            OnState = !OnState;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_V24B8G8R8(*this));
        }

protected:


private:


    static short m_LostChannels[];
    };
short ConverterI1R8G8B8A8RLE_V24B8G8R8::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8RLE_V24B8G8R8 s_I1R8G8B8A8RLE_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_V24R8G8B8 : public HRPPixelConverter
    {

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                pDest[0] = pColor[OnState][0];
                pDest[1] = pColor[OnState][1];
                pDest[2] = pColor[OnState][2];

                pDest+=3;

                RunLen--;
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }
        };

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            // is the color opaque?
            if(pColor[OnState][3] == 255)
                {
                while(RunLen != 0)
                    {
                    pDest[0] = pColor[OnState][0];
                    pDest[1] = pColor[OnState][1];
                    pDest[2] = pColor[OnState][2];

                    // Increment Destination by 3 bytes !
                    pDest+=3;

                    RunLen--;
                    }
                }
            // color is semi-transparent
            else if (pColor[OnState][3] > 0)
                {
                while(RunLen != 0)
                    {
                    // alpha * (S - D) + D
                    pDest[0] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][0] - pDest[0])) + pDest[0];
                    pDest[1] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][1] - pDest[1])) + pDest[1];
                    pDest[2] = pQuotients->DivideBy255ToByte(pColor[OnState][3] * (pColor[OnState][2] - pDest[2])) + pDest[2];

                    // Increment Destination by 3 bytes !
                    pDest+=3;

                    RunLen--;
                    }
                }
            // transparent case
            else
                {
                // go to next section
                pDest += (3 * RunLen);
                }

            pSrcRun++;
            OnState = !OnState;
            }


        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8RLE_V24R8G8B8(*this));
        }


private:
    static short m_LostChannels[];
    };
short ConverterI1R8G8B8A8RLE_V24R8G8B8::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8RLE_V24R8G8B8 s_I1R8G8B8A8RLE_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I1R8G8B8A8RLE - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I1R8G8B8A8RLE : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24R8G8B8_I1R8G8B8A8RLE()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        *pDest = 0;
        bool DstState = false;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // get a good index for the R,G,B source values
            if((m_QuantizedPalette.GetIndex(
                    pSrc[0],
                    pSrc[1],
                    pSrc[2],
                    0xff) == 1) == DstState)
                {
                if(*pDest == 32767)
                    {
                    pDest++;
                    *pDest = 0;
                    pDest++;
                    *pDest = 1;
                    }
                else
                    {
                    (*pDest)++;
                    }
                }
            else
                {
                pDest++;
                *pDest = 1;
                DstState = !DstState;
                }

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=3;

            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterV24R8G8B8_I1R8G8B8A8RLE(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        for(Byte Index = 0; Index < 2; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 Index);
        }

    };
static ConverterV24R8G8B8_I1R8G8B8A8RLE s_V24R8G8B8_I1R8G8B8A8RLE;


//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I1R8G8B8A8RLE - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I1R8G8B8A8RLE : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32R8G8B8A8_I1R8G8B8A8RLE()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;
        Byte ColorIndex;

        *pDest = 0;
        bool DstState = false;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // Get a good index for the R,G,B,A source values
            ColorIndex = m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2], pSrc[3]);

            if((ColorIndex == 1) == DstState)
                {
                if(*pDest == 32767)
                    {
                    pDest++;
                    *pDest = 0;
                    pDest++;
                    *pDest = 1;

                    }
                else
                    {
                    (*pDest)++;
                    }
                }
            else
                {
                pDest++;
                *pDest = 1;
                DstState = !DstState;
                }

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=4;

            pi_PixelsCount--;
            }
        };

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // To simplify RLE algo and avoid confusion between run len and run value
        // we will assume 0 to be black and 1 to be white. This is not the real pixel value
        // since the black and white position is determined by the pixel type.
        Byte*  pSourceComposite = (Byte*)pi_pSourceRawData;
        unsigned short* pFinalDst = (unsigned short*)pio_pDestRawData;

        // Alloc a buffer able to hold the original destination.
        size_t RunCount = 0;
        for(size_t i(0); i < pi_PixelsCount; i+=pFinalDst[RunCount++]);
        HArrayAutoPtr<unsigned short> pTempDst(new unsigned short[RunCount]);

        // Copy the dest.
        unsigned short* pDst = pTempDst.get();
        memcpy(pDst, pFinalDst, sizeof(unsigned short)*RunCount);

        // We start on black
        uint32_t DstIndex = 0;
        Byte FinalDstOnState = 0;

        uint32_t DstRunLen = 0;
        uint32_t FinalDstRunLen = 0;

        Byte  TmpDst[4];

        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            // Get more pixels from the destination
            while(pDst[DstIndex] == 0)
                ++DstIndex;

            // Make sure current run do not exceed the number of pixels to process.
            // Also subtract pi_PixelsCount now because we decrement DstRunLen
            if(pDst[DstIndex] < pi_PixelsCount)
                {
                DstRunLen = pDst[DstIndex];
                pi_PixelsCount-=DstRunLen;
                }
            else
                {
                DstRunLen = (uint32_t)pi_PixelsCount;
                pi_PixelsCount = 0;
                }

            Byte* pDestComposite = (Byte*)rPalette.GetCompositeValue(DstIndex & 0x00000001); // black runs are ON even number. 0,2,4,6...

            while(DstRunLen)
                {
                Byte CurrentPixelState = (Byte)(DstIndex & 0x00000001);

                // If source pixel is fully transparent, destination is unaltered
                if (pSourceComposite[3] != 0)
                    {
                    if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                        {
                        // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                        CurrentPixelState = m_QuantizedPalette.GetIndex(pSourceComposite[0], pSourceComposite[1], pSourceComposite[2], pSourceComposite[3]);
                        }
                    else
                        {
                        HFCMath (*pQuotients) (HFCMath::GetInstance());

                        // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                        // Alphas are in [0, 255].
                        Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                        TmpDst[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                        TmpDst[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                        PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                        TmpDst[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                        // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                        // --> Transparency percentages are multiplied
                        TmpDst[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3])));

                        // get a good index for the R,G,B source values
                        CurrentPixelState = m_QuantizedPalette.GetIndex(TmpDst[0], TmpDst[1], TmpDst[2], TmpDst[3]);
                        }
                    }

                if(CurrentPixelState == FinalDstOnState)
                    {
                    // Add one pixel to the current state.
                    ++FinalDstRunLen;
                    }
                else
                    {
                    // Write what is exceeding the MAX run len
                    while(FinalDstRunLen > SHRT_MAX)
                        {
                        *pFinalDst++ = SHRT_MAX;
                        FinalDstRunLen -= SHRT_MAX;
                        *pFinalDst++ = 0;
                        }

                    *pFinalDst++ = (unsigned short)FinalDstRunLen;
                    // FinalDstRunLen = 0;    It should be 0 at this stage but we will set it to 1 below
                    FinalDstOnState = !FinalDstOnState;

                    // Add/subtract one pixel
                    FinalDstRunLen = 1;
                    }

                // Next source pixel
                pSourceComposite+=4;
                --DstRunLen;
                }

            // Next RLE run.
            ++DstIndex;
            }

        // Write what is exceeding the MAX run len
        while(FinalDstRunLen > SHRT_MAX)
            {
            *pFinalDst++ = SHRT_MAX;
            FinalDstRunLen -= SHRT_MAX;
            *pFinalDst++ = 0;
            }

        // Write the last run
        *pFinalDst = (unsigned short)FinalDstRunLen;
        //++pFinalDst;
        //FinalDstOnState = !FinalDstOnState;
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterV32R8G8B8A8_I1R8G8B8A8RLE(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // Fill the octree with the destination palette entries
        for(Byte Index = 0; Index < 2; Index++)
            {
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 Index);
            }
        }

    };
static ConverterV32R8G8B8A8_I1R8G8B8A8RLE s_V32R8G8B8A8_I1R8G8B8A8RLE;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_I8R8G8B8 - Converter between g_PixelTypeFingerPrintI8R8G8B8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_I8R8G8B8 : public HRPPixelConverter
    {
    static short m_LostChannels[];

    // conversion table (Convert)
    Byte EntryConversion[2];

    // conversion table (Compose)
    Byte m_EntryComposition[512];

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8RLE_I8R8G8B8()
        {
        };

    virtual ~ConverterI1R8G8B8A8RLE_I8R8G8B8()
        {
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                *pDest = EntryConversion[OnState];

                pDest++;

                RunLen--;
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun =  (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        size_t RunLen;

        bool OnState = false;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                *pDest = m_EntryComposition[OnState*256 + *pDest];

                pDest++;
                RunLen--;
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }


    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI1R8G8B8A8RLE_I8R8G8B8(*this));
        };


protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(Byte Index = 0; Index < 2; Index++)
                EntryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 2; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                EntryConversion[Index] = (Byte)GetClosestEntryIndexInPalette(pSrcValue,rDestPixelPalette);
                }
            }

        // fill the octree with the destination palette entries
        HRPPaletteOctreeR8G8B8 QuantizedPalette;
        uint32_t NbIndex(rDestPixelPalette.CountUsedEntries());
        for(uint32_t Index = 0; Index < NbIndex; Index++)
            QuantizedPalette.AddCompositeValue(rDestPixelPalette.GetCompositeValue(Index),
                                               (Byte)Index);

        // Pre compose each values
        Byte Blend[3];
        HFCMath (*pQuotients) (HFCMath::GetInstance());
        for(uint32_t Index = 0; Index < NbIndex; Index++)
            {
            Byte* pSourceComposite = (Byte*)rSrcPixelPalette.GetCompositeValue(0);
            Byte* pDestComposite = (Byte*)rDestPixelPalette.GetCompositeValue(Index);

            Blend[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
            Blend[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            Blend[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

            // get a good index for the R,G,B blend values
            m_EntryComposition[Index] = QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2]);

            pSourceComposite = (Byte*)rSrcPixelPalette.GetCompositeValue(1);

            Blend[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
            Blend[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            Blend[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

            // get a good index for the R,G,B blend values
            m_EntryComposition[256+Index] = QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2]);
            }
        };

private:

    // get the closest index in a palette from a given composite color
    size_t GetClosestEntryIndexInPalette(Byte* pi_pValue, const HRPPixelPalette& pi_rPixelPalette) const
        {
        Byte* pValue;
        int RDist, GDist, BDist;
        uint32_t Dist;
        size_t IndexForSmallerDist=0;

        // At the beginning, define the smaller distance to be larger
        // than the maximum distance
        uint32_t SmallerDist = 0xFFFFFFFF;

        // For each entry of the palette, verify if the
        // composite value associated is closer to the input composite value
        int32_t NbIndex(pi_rPixelPalette.CountUsedEntries());
        for(int32_t Index = 0; Index < NbIndex; Index++)
            {
            if (Index == pi_rPixelPalette.GetLockedEntryIndex())
                continue;//Skip locked entry

            // Get the composite value associated to the index in the palette
            pValue = (Byte*)pi_rPixelPalette.GetCompositeValue(Index);

            // Calculate the distance between the two composite values
            RDist = pValue[0] - pi_pValue[0];
            GDist = pValue[1] - pi_pValue[1];
            BDist = pValue[2] - pi_pValue[2];
            Dist = RDist * RDist + GDist * GDist + BDist * BDist;

            // Is the distance smaller than before?
            if(Dist < SmallerDist)
                {
                // copy the distance and the index in the palette associated
                SmallerDist = Dist;
                IndexForSmallerDist = Index;
                }
            }

        return IndexForSmallerDist;

        }
    };
short ConverterI1R8G8B8A8RLE_I8R8G8B8::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8RLE_I8R8G8B8 s_I1R8G8B8A8RLE_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8RLE_I8R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8RLE_I8R8G8B8A8 : public HRPPixelConverter
    {
    // conversion table (Convert)
    Byte EntryConversion[2];

    // conversion table (Compose)  0-255 --> src=0  256-511 --> src=1
    // result = m_EntryComposition[(srcIndex * 256) + dstIndex]
    Byte m_EntryComposition[512];

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8RLE_I8R8G8B8A8()
        {
        };

    virtual ~ConverterI1R8G8B8A8RLE_I8R8G8B8A8()
        {
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun  = (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        bool OnState = false;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        pColor[0]   = (Byte*)(rPalette.GetCompositeValue(0));
        pColor[1]   = (Byte*)(rPalette.GetCompositeValue(1));

        size_t RunLen;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                *pDest = EntryConversion[OnState];

                ++pDest;
                --RunLen;
                }

            // Increment
            ++pSrcRun;
            OnState = !OnState;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrcRun =  (unsigned short*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        size_t RunLen;

        bool OnState = false;

        while(pi_PixelsCount != 0)
            {
            // get the len
            RunLen = MIN(*pSrcRun, pi_PixelsCount);

            // Substract the len to the pixel count
            pi_PixelsCount -= RunLen;

            // copy the pixels
            while(RunLen != 0)
                {
                *pDest = m_EntryComposition[OnState*256 + *pDest];

                ++pDest;
                --RunLen;
                }

            // Increment
            ++pSrcRun;
            OnState = !OnState;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterI1R8G8B8A8RLE_I8R8G8B8A8(*this));
        };


protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        HRPPaletteOctreeR8G8B8A8 QuantizedPalette;
        uint32_t NbIndex(rDestPixelPalette.CountUsedEntries());
        for(uint32_t Index = 0; Index < NbIndex; Index++)
            QuantizedPalette.AddCompositeValue(rDestPixelPalette.GetCompositeValue(Index), (Byte)Index);

        Byte* pSrcCompositeEntry0 = (Byte*)rSrcPixelPalette.GetCompositeValue(0);
        Byte* pSrcCompositeEntry1 = (Byte*)rSrcPixelPalette.GetCompositeValue(1);

        // -----------------------------
        // COMPUTE EntryConversion table
        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            EntryConversion[0] = 0;
            EntryConversion[1] = 1;
            }
        else
            {
            // Get the composite value associated to the source entry index in the source palette and
            // get a good index for the R,G,B,A
            EntryConversion[0] = QuantizedPalette.GetIndex(pSrcCompositeEntry0[0], pSrcCompositeEntry0[1],
                                                           pSrcCompositeEntry0[2], pSrcCompositeEntry0[3]);
            EntryConversion[1] = QuantizedPalette.GetIndex(pSrcCompositeEntry1[0], pSrcCompositeEntry1[1],
                                                           pSrcCompositeEntry1[2], pSrcCompositeEntry1[3]);
            }

        // --------------------------------
        // COMPUTE m_EntryComposition table
        Byte Blend[4];
        for(uint32_t Index = 0; Index < NbIndex; Index++)
            {
            Byte* pDestComposite = (Byte*)rDestPixelPalette.GetCompositeValue(Index);

            // --------------------
            // ENTRY 0
            BlendCompositeValue(Blend, pSrcCompositeEntry0, pDestComposite);

            // get a good index for the R,G,B,A blend values
            m_EntryComposition[Index] = QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2], Blend[3]);

            // -----------------------
            // ENTRY 1
            BlendCompositeValue(Blend, pSrcCompositeEntry1, pDestComposite);

            // get a good index for the R,G,B,A blend values
            m_EntryComposition[256+Index] = QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2], Blend[3]);
            }
        };

private:
    void BlendCompositeValue(Byte* po_pBlendValue, Byte* pi_pSourceComposite, Byte* pi_pDestComposite)
        {
        if (pi_pDestComposite[3] == 0 || pi_pSourceComposite[3] == 255)
            {
            // Destination pixel is fully transparent, or source pixel
            // is fully opaque. Copy source pixel,
            *((uint32_t*)po_pBlendValue) = *((uint32_t*)pi_pSourceComposite);
            }
        else
            {
            HFCMath (*pQuotients) (HFCMath::GetInstance());

            // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
            // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
            Byte PremultDestColor(pQuotients->DivideBy255ToByte(pi_pDestComposite[3] * pi_pDestComposite[0]));
            po_pBlendValue[0] = pQuotients->DivideBy255ToByte(pi_pSourceComposite[3] * (pi_pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

            PremultDestColor = pQuotients->DivideBy255ToByte(pi_pDestComposite[3] * pi_pDestComposite[1]);
            po_pBlendValue[1] = pQuotients->DivideBy255ToByte(pi_pSourceComposite[3] * (pi_pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

            PremultDestColor = pQuotients->DivideBy255ToByte(pi_pDestComposite[3] * pi_pDestComposite[2]);
            po_pBlendValue[2] = pQuotients->DivideBy255ToByte(pi_pSourceComposite[3] * (pi_pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

            // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
            // --> Transparency percentages are multiplied
            po_pBlendValue[3] = 255 - pQuotients->DivideBy255ToByte((255 - pi_pSourceComposite[3]) * (255 - pi_pDestComposite[3]));
            }
        };
    };
static ConverterI1R8G8B8A8RLE_I8R8G8B8A8 s_I1R8G8B8A8RLE_I8R8G8B8A8;
//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I1R8G8B8A8RLEConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I1R8G8B8A8RLEConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8_I1R8G8B8A8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID, &s_I1R8G8B8A8RLE_I1R8G8B8A8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I1R8G8B8A8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I1R8G8B8A8RLE));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I1R8G8B8A8RLEConvertersTo : public MapHRPPixelTypeToConverter
    {
    I1R8G8B8A8RLEConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID, &s_I1R8G8B8A8RLE_I1R8G8B8A8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8RLE::CLASS_ID, &s_I1R8G8B8A8RLE_I1R8G8B8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8::CLASS_ID, &s_I1R8G8B8A8RLE_I1R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8RLE_I1R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I1R8G8B8A8RLE_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_I1R8G8B8A8RLE_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8RLE_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I1R8G8B8A8RLE_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8RLE_I8R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8A8RLE::HRPPixelTypeI1R8G8B8A8RLE()
    : HRPPixelTypeRGB(8,8,8,8,1),
      HRPPixelType1BitInterface()
    {
    HRPPixelPalette& rPixelPalette = (HRPPixelPalette&) GetPalette();

    // by default, create a black and white palette
    uint32_t Value;

    // the black is opqaue
    Value = 0xFF000000;
    rPixelPalette.AddEntry(&Value);

    // the white is transparent
    Value = 0xFFFFFFFF;
    rPixelPalette.AddEntry(&Value);
    }


//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
// with a palette
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8A8RLE::HRPPixelTypeI1R8G8B8A8RLE (const HRPPixelPalette& pi_Palette)
    : HRPPixelTypeRGB(8,8,8,8,1),
      HRPPixelType1BitInterface()
    {
    HRPPixelPalette*    pDstPalette = (HRPPixelPalette*)&GetPalette();

    for (uint32_t Index=0; Index < pi_Palette.CountUsedEntries (); Index++)
        pDstPalette->AddEntry (pi_Palette.GetCompositeValue (Index));
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8A8RLE::HRPPixelTypeI1R8G8B8A8RLE(const HRPPixelTypeI1R8G8B8A8RLE& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj),
      HRPPixelType1BitInterface(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8A8RLE::~HRPPixelTypeI1R8G8B8A8RLE()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI1R8G8B8A8RLE::Clone() const
    {
    return new HRPPixelTypeI1R8G8B8A8RLE(*this);
    }

/** -----------------------------------------------------------------------------
    This function is used to know the number of "value" bits contain in a pixel
    of this pixel type.

    @b{Example:} @list{HRPPixelTypeV32R8G8B8A8 should return 32.}
                 @list{HRPPixelTypeI8R8G8B8A8 should return 0.}
                 @list{HRPPixelTypeI8VA8R8G8B8 should return 8.}

    @return The number of "value" bits contain in a pixel of this pixel type.
    @end

    @see HRPPixelType::CountIndexBits()
    @see HRPPixelType::CountPixelRawData()
    @end
    -----------------------------------------------------------------------------
 */
unsigned short HRPPixelTypeI1R8G8B8A8RLE::CountValueBits() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// Get1BitInterface
//-----------------------------------------------------------------------------
HRPPixelType1BitInterface* HRPPixelTypeI1R8G8B8A8RLE::Get1BitInterface()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI1R8G8B8A8RLE::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I1R8G8B8A8RLEConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI1R8G8B8A8RLE::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I1R8G8B8A8RLEConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }
