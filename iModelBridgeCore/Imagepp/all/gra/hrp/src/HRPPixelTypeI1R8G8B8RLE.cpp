//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI1R8G8B8RLE.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeI1R8G8B8RLE
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };

HPM_REGISTER_CLASS(HRPPixelTypeI1R8G8B8RLE, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8RLER8G8B8RLE
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8RLE_I1R8G8B8RLE : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8RLE_I1R8G8B8RLE()
        {
        }

    virtual ~ConverterI1R8G8B8RLE_I1R8G8B8RLE()
        {
        }

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
        }

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        HASSERT(0);
        }


    virtual HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterI1R8G8B8RLE_I1R8G8B8RLE(*this));
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
        }

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
        }

    // conversion table
    Byte EntryConversion[2];
    };
static ConverterI1R8G8B8RLE_I1R8G8B8RLE s_I1R8G8B8RLE_I1R8G8B8RLE;



//-----------------------------------------------------------------------------
//  s_I1R8G8B8RLE_I1R8G8B8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8RLE_I1R8G8B8 : public HRPPixelConverter
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
        }

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterI1R8G8B8RLE_I1R8G8B8(*this));
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
        }

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
    };
short ConverterI1R8G8B8RLE_I1R8G8B8::m_LostChannels[] = {-1};
static ConverterI1R8G8B8RLE_I1R8G8B8 s_I1R8G8B8RLE_I1R8G8B8;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class ConverterI1R8G8B8_I1R8G8B8RLE : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        const Byte* pSrc = (const Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;
        size_t PixelCount(0);
        bool OnState(false);
        size_t Index = 0;

        while(PixelCount < pi_PixelsCount)
            {
            // Count bits
            uint32_t BitCount (0);
            while(PixelCount < pi_PixelsCount)
                {
                if((Byte)OnState != EntryConversion[(*pSrc & s_BitMask[Index]) >> (7 - Index)])
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

            while(BitCount > 32767)
                {
                pDest[0] = 32767;
                pDest[1] = 0;
                pDest+=2;
                BitCount -= 32767;
                }
            *pDest = (unsigned short)BitCount;
            ++pDest;
            OnState = !OnState;
            }

        // must end with black.
        if(!OnState)
            *pDest = 0;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI1R8G8B8_I1R8G8B8RLE(*this));
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
        }

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

    // conversion table
    Byte EntryConversion[2];
    };
static ConverterI1R8G8B8_I1R8G8B8RLE s_I1R8G8B8_I1R8G8B8RLE;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8RLE_I1R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8RLE_I1R8G8B8A8 : public HRPPixelConverter
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

            pSrcRun++;
            OnState = !OnState;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI1R8G8B8RLE_I1R8G8B8A8(*this));
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
        }

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

    // conversion table
    Byte EntryConversion[2];
    };
static ConverterI1R8G8B8RLE_I1R8G8B8A8 s_I1R8G8B8RLE_I1R8G8B8A8;



//-----------------------------------------------------------------------------
//  s_I1R8G8B8RLE_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8RLE_V32R8G8B8A8 : public HRPPixelConverter
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
                pDest[3] = 0xFF;

                pDest+=4;

                RunLen--;
                }

            // Increment Destination by 3 bytes !
            pSrcRun++;
            OnState = !OnState;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI1R8G8B8RLE_V32R8G8B8A8(*this));
        }

private:

    };
static ConverterI1R8G8B8RLE_V32R8G8B8A8 s_I1R8G8B8RLE_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8RLE_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8RLE_V24B8G8R8 : public HRPPixelConverter
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

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI1R8G8B8RLE_V24B8G8R8(*this));
        }
    };
static ConverterI1R8G8B8RLE_V24B8G8R8 s_I1R8G8B8RLE_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8RLE_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8RLE_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        uint16_t const* pSrcRun = (uint16_t const*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        const Byte* pColor[2];
        // we invert back/fore to match or mask op below.
        pColor[0] = (Byte*)(rPalette.GetCompositeValue(1));
        pColor[1] = (Byte*)(rPalette.GetCompositeValue(0));
        
        uint32_t pixelCount = (uint32_t)pi_PixelsCount;
        uint32_t srcRunIndex = 0;
        for (uint64_t pixelIndex = 0; pixelIndex < pixelCount;)
            {
            uint32_t copyToIndex = (uint32_t)MIN(pixelIndex + pSrcRun[srcRunIndex], pixelCount);
            ++srcRunIndex;

            for (; pixelIndex < copyToIndex; ++pixelIndex)
                {
                memcpy(pDest + pixelIndex * 3, pColor[srcRunIndex & 0x1], 3);
                }
            }
        }

    HRPPixelConverter* AllocateCopy() const override {return(new ConverterI1R8G8B8RLE_V24R8G8B8(*this));}
    };
static ConverterI1R8G8B8RLE_V24R8G8B8 s_I1R8G8B8RLE_V24R8G8B8;


//-----------------------------------------------------------------------------
//  s_I8R8G8B8_I1R8G8B8RLE - Converter
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8_I1R8G8B8RLE : public HRPPixelConverter
    {

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc  = (Byte*)pi_pSourceRawData;
        unsigned short* pDestRun = (unsigned short*)pio_pDestRawData;

        uint32_t CurrentRunLen = 0;
        bool  OnState = false;
        while(pi_PixelsCount != 0)
            {
            if(m_IndexMapping[*pSrc] == OnState)
                ++CurrentRunLen;
            else
                {
                // Write current run
                while(CurrentRunLen > 32767)
                    {
                    *pDestRun++ = 32767;
                    *pDestRun++ = 0;
                    CurrentRunLen -= 32767;
                    }
                *pDestRun++ = (unsigned short)CurrentRunLen;

                // Start new a new run.
                CurrentRunLen = 1;
                OnState = !OnState;
                }

            --pi_PixelsCount;
            ++pSrc;
            }

        // Write remaining pixels
        while(CurrentRunLen > 32767)
            {
            *pDestRun++ = 32767;
            *pDestRun++ = 0;
            CurrentRunLen -= 32767;
            }
        *pDestRun++ = (unsigned short)CurrentRunLen;

        // Must end with blacks.
        if(OnState)
            *pDestRun = 0;
        };

protected:

    virtual void Update() override
        {
        HPRECONDITION(GetDestinationPixelType()->GetPalette().CountUsedEntries() == 2);
        HPRECONDITION(GetSourcePixelType()->GetPalette().CountUsedEntries() <= 256);

        const HRPPixelPalette& rPaletteDest = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        HRPPaletteOctreeR8G8B8 QuantizedPalette;
        QuantizedPalette.AddCompositeValue(rPaletteDest.GetCompositeValue(0), 0);
        QuantizedPalette.AddCompositeValue(rPaletteDest.GetCompositeValue(1), 1);

        // Eval source palette index mapping
        const HRPPixelPalette& rPaletteSrc = GetSourcePixelType()->GetPalette();

        uint32_t NbIndex(rPaletteSrc.CountUsedEntries());
        for(uint32_t Index=0; Index < NbIndex; ++Index)
            {
            Byte* RGBValue = (Byte*)rPaletteSrc.GetCompositeValue(Index);
            m_IndexMapping[Index] = (QuantizedPalette.GetIndex(RGBValue[0],RGBValue[1],RGBValue[2]) != 0);
            }
        }

    bool    m_IndexMapping[256];

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8_I1R8G8B8RLE(*this));
        }
    };
static ConverterI8R8G8B8_I1R8G8B8RLE s_I8R8G8B8_I1R8G8B8RLE;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I1R8G8B8RLE - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I1R8G8B8RLE : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24R8G8B8_I1R8G8B8RLE()
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
                    pSrc[2]) == 1) == DstState)
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

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_I1R8G8B8RLE(*this));
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
static ConverterV24R8G8B8_I1R8G8B8RLE s_V24R8G8B8_I1R8G8B8RLE;


//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I1R8G8B8RLE - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I1R8G8B8RLE : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32R8G8B8A8_I1R8G8B8RLE()
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
            // If the source color is completly transparente we use the
            // destination transparent color index.
            ColorIndex = m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2]);

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
        HFCMath (*pQuotients) (HFCMath::GetInstance());

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
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            // Get more pixels from the destination
            while(pDst[DstIndex] == 0)
                ++DstIndex;

            Byte* pDestComposite = (Byte*)rPalette.GetCompositeValue(DstIndex & 0x00000001);       // black runs are ON even number. 0,2,4,6...

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
                Byte CurrentPixelState = (Byte)(DstIndex & 0x00000001);

                // If source pixel is fully transparent, destination is unaltered
                if (pSourceComposite[3] != 0)
                    {
                    if (pSourceComposite[3] == 255)
                        {
                        // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                        CurrentPixelState = m_QuantizedPalette.GetIndex(pSourceComposite[0], pSourceComposite[1], pSourceComposite[2]);
                        }
                    else
                        {
                        // get a good index for the R,G,B source values
                        CurrentPixelState = m_QuantizedPalette.GetIndex(pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0],
                                                                        pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1],
                                                                        pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2]);
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

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_I1R8G8B8RLE(*this));
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

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_I1R8G8B8RLE::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_I1R8G8B8RLE s_V32R8G8B8A8_I1R8G8B8RLE;



//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I1R8G8B8RLEConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I1R8G8B8RLEConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8::CLASS_ID, &s_I1R8G8B8_I1R8G8B8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8RLE::CLASS_ID, &s_I1R8G8B8RLE_I1R8G8B8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_I1R8G8B8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I1R8G8B8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I1R8G8B8RLE));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I1R8G8B8RLEConvertersTo : public MapHRPPixelTypeToConverter
    {
    I1R8G8B8RLEConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8RLE::CLASS_ID, &s_I1R8G8B8RLE_I1R8G8B8RLE));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8::CLASS_ID, &s_I1R8G8B8RLE_I1R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8::CLASS_ID, &s_I1R8G8B8RLE_I1R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I1R8G8B8RLE_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_I1R8G8B8RLE_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I1R8G8B8RLE_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8RLE::HRPPixelTypeI1R8G8B8RLE()
    : HRPPixelTypeRGB(8,8,8,0,1),
      HRPPixelType1BitInterface()
    {
    HRPPixelPalette& rPixelPalette = (HRPPixelPalette&) GetPalette();

    // by default, create a black and white palette
    uint32_t Value;

    Value = 0x000000;
    rPixelPalette.AddEntry(&Value);

    Value = 0xFFFFFF;
    rPixelPalette.AddEntry(&Value);
    }


//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
// with a palette
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8RLE::HRPPixelTypeI1R8G8B8RLE (const HRPPixelPalette& pi_Palette)
    : HRPPixelTypeRGB(8,8,8,0,1),
      HRPPixelType1BitInterface()
    {
    HRPPixelPalette*    pDstPalette = (HRPPixelPalette*)&GetPalette();

    for (uint32_t Index=0; Index < pi_Palette.CountUsedEntries (); Index++)
        pDstPalette->AddEntry (pi_Palette.GetCompositeValue (Index));
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8RLE::HRPPixelTypeI1R8G8B8RLE(const HRPPixelTypeI1R8G8B8RLE& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj),
      HRPPixelType1BitInterface(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8RLE::~HRPPixelTypeI1R8G8B8RLE()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI1R8G8B8RLE::Clone() const
    {
    return new HRPPixelTypeI1R8G8B8RLE(*this);
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
unsigned short HRPPixelTypeI1R8G8B8RLE::CountValueBits() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// Get1BitInterface
//-----------------------------------------------------------------------------
HRPPixelType1BitInterface* HRPPixelTypeI1R8G8B8RLE::Get1BitInterface()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI1R8G8B8RLE::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I1R8G8B8RLEConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI1R8G8B8RLE::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I1R8G8B8RLEConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }
