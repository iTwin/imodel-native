//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI8R8G8B8A8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>+--------------------------------------------------------------------------
// Methods for class HRPPixelTypeI8R8G8B8A8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8A8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeI8R8G8B8A8, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

static Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
static Byte s_SrcMask[9]    = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
static Byte s_DestMask[9]   = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };

//-----------------------------------------------------------------------------
//  s_I8R8G8B8A8_I8R8G8B8A8 - Converter between HRPPixelTypeI8R8G8B8A8
//                          and HRPPixelTypeI8R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8A8_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI8R8G8B8A8_I8R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            *pDest = EntryConversion[*pSrc];

            pSrc++;
            pDest++;
            pi_PixelsCount--;
            }
        };

   virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPalette = GetDestinationPixelType()->GetPalette();
        Byte Blend[4];
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);
            Byte* pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

            if (pDestComposite[3] == 0 ||
                pSourceComposite[3] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                *((uint32_t*)Blend) = *((uint32_t*)pSourceComposite);
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                Blend[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                Blend[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                Blend[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                Blend[3] = 255 - pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3]));
                }

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(
                         Blend[0],
                         Blend[1],
                         Blend[2],
                         Blend[3]);

            pi_PixelsCount -= 1;
            pDest++;
            pSrc++;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8A8_I8R8G8B8A8(*this));
        }

protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries.
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            {
            Byte* pComposite = (Byte*) rPalette.GetCompositeValue(Index);
            m_QuantizedPalette.AddCompositeValue(pComposite,
                                                 (Byte)Index);
            }

        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(unsigned short Index = 0; Index < 256; Index++)
                EntryConversion[Index] =static_cast<Byte>(Index);
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            int32_t NbIndex(rSrcPixelPalette.CountUsedEntries());
            for(int32_t Index = 0; Index < NbIndex; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                EntryConversion[Index] = m_QuantizedPalette.GetIndex(pSrcValue[0], pSrcValue[1], pSrcValue[2], pSrcValue[3]);
                }
            }
        }

private:

    // conversion table
    Byte EntryConversion[256];
    };
static ConverterI8R8G8B8A8_I8R8G8B8A8        s_I8R8G8B8A8_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V8Gray8_I8R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV8Gray8_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV8Gray8_I8R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // get a good index for the R,G,B source values
            *pDest        = m_QuantizedPalette.GetIndex(
                                *pSrc,
                                *pSrc,
                                *pSrc,
                                0xff);

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc++;

            // increment the destination
            pDest++;

            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV8Gray8_I8R8G8B8A8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
        }

    };
static ConverterV8Gray8_I8R8G8B8A8 s_V8Gray8_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8A8_V8Gray8 - Converter between HRPPixelTypeI8R8G8B8A8
//                           and HRPPixelTypeV8Gray8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8A8_V8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte* pSourceComposite = 0;

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(GetSourcePixelType()->GetPalette().GetCompositeValue(*pSrc));

            pDest[0] = Byte(pSourceComposite[0] * REDFACTOR +
                              pSourceComposite[1] * GREENFACTOR +
                              pSourceComposite[2] * BLUEFACTOR);
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source
            pSrc++;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        Byte Gray;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            // calculate a gray for the RGB value
            Gray = Byte(pSourceComposite[0] * REDFACTOR +
                          pSourceComposite[1] * GREENFACTOR +
                          pSourceComposite[2] * BLUEFACTOR);

            // alpha * (S - D) + D
            *pDestComposite = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (Gray - *pDestComposite)) + *pDestComposite;

            pi_PixelsCount -= 1;
            pDestComposite++;
            pSrc++;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8A8_V8Gray8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterI8R8G8B8A8_V8Gray8::m_LostChannels[] = {3, -1};
static ConverterI8R8G8B8A8_V8Gray8 s_I8R8G8B8A8_V8Gray8;

//-----------------------------------------------------------------------------
//  s_V1Gray1_I8R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV1Gray1_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV1Gray1_I8R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        Byte Index = 0;
        Byte Color;

        // convert all pixels
        while(pi_PixelsCount)
            {
            if(*pSrc & s_BitMask[Index])
                Color = 1;
            else
                Color = 0;

            *pDest = EntryConversion[Color];

            Index++;

            if(Index == 8)
                {
                Index = 0;

                // increment the pointer to the next 24-bit (3 bytes) composite value
                pSrc++;
                }

            // increment the destination
            pDest++;

            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV1Gray1_I8R8G8B8A8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);

        EntryConversion[0] = m_QuantizedPalette.GetIndex(0, 0, 0, 0xff);
        EntryConversion[1] = m_QuantizedPalette.GetIndex(0xff, 0xff, 0xff, 0xff);
        }

private:

    // conversion table
    Byte EntryConversion[2];
    };
static ConverterV1Gray1_I8R8G8B8A8 s_V1Gray1_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8A8_I8R8G8B8 - Converter between HRPPixelTypeI8R8G8B8A8
//                          and HRPPixelTypeI8R8G8B8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8A8_I8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI8R8G8B8A8_I8R8G8B8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            *pDest = EntryConversion[*pSrc];

            pSrc++;
            pDest++;
            pi_PixelsCount--;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPalette = GetDestinationPixelType()->GetPalette();
        Byte Blend[3];

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);
            Byte* pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

            // alpha * (S - D) + D
            Blend[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
            Blend[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            Blend[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(
                         Blend[0],
                         Blend[1],
                         Blend[2]);

            pi_PixelsCount -= 1;
            pDest++;
            pSrc++;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8A8_I8R8G8B8(*this));
        }

protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);

        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // if src and dest palettes are the same, make an index-index map
            for(unsigned short Index = 0; Index < 256; Index++)
                EntryConversion[Index] = static_cast<Byte>(Index);
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            int32_t NbIndex(rSrcPixelPalette.CountUsedEntries());
            for(int32_t Index = 0; Index < NbIndex; Index++)
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

        };

    // conversion table
    Byte EntryConversion[256];

    static short m_LostChannels[];
    };
short ConverterI8R8G8B8A8_I8R8G8B8::m_LostChannels[] = {3, -1};
static ConverterI8R8G8B8A8_I8R8G8B8        s_I8R8G8B8A8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8_I8R8G8B8A8 - Converter between HRPPixelTypeI8R8G8B8
//                          and HRPPixelTypeI8R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8_I8R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            *pDest = EntryConversion[*pSrc];

            pSrc++;
            pDest++;
            pi_PixelsCount--;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8_I8R8G8B8A8(*this));
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
            for(unsigned short Index = 0; Index < 256; Index++)
                EntryConversion[Index] = static_cast<Byte>(Index);
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            int32_t NbIndex(rSrcPixelPalette.CountUsedEntries());
            for(int32_t Index = 0; Index < NbIndex; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // find the closest entry index in the destination palette
                EntryConversion[Index] = (Byte)GetClosestEntryIndexInPalette(pSrcValue,rDestPixelPalette);
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

        };

    // conversion table
    Byte EntryConversion[256];
    };
static ConverterI8R8G8B8_I8R8G8B8A8 s_I8R8G8B8_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8A8_V24B8G8R8 - Converter between HRPPixelTypeI8R8G8B8A8
//                           and HRPPixelTypeV24B8G8R8
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8A8_V24B8G8R8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue(*pSrc));

            pDest[2] = pSourceComposite[0];
            pDest[1] = pSourceComposite[1];
            pDest[0] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            // Increment Destination by 1 byte (index) !
            pSrc++;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            // alpha * (S - D) + D
            pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[2])) + pDestComposite[2];
            pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[0])) + pDestComposite[0];

            --pi_PixelsCount;
            pDestComposite += 3;
            ++pSrc;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8A8_V24B8G8R8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterI8R8G8B8A8_V24B8G8R8::m_LostChannels[] = {3, -1};
static struct ConverterI8R8G8B8A8_V24B8G8R8 s_I8R8G8B8A8_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8A8_V24R8G8B8 - Converter between HRPPixelTypeI8R8G8B8A8
//                           and HRPPixelTypeV24R8G8B8
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8A8_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue(*pSrc));

            pDest[0] = pSourceComposite[0];
            pDest[1] = pSourceComposite[1];
            pDest[2] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            // Increment Destination by 1 byte (index) !
            pSrc++;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            // alpha * (S - D) + D
            pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
            pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

            --pi_PixelsCount;
            pDestComposite += 3;
            ++pSrc;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8A8_V24R8G8B8(*this));
        }

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

private:

    static short m_LostChannels[];
    };
short ConverterI8R8G8B8A8_V24R8G8B8::m_LostChannels[] = {3, -1};
static struct ConverterI8R8G8B8A8_V24R8G8B8 s_I8R8G8B8A8_V24R8G8B8;


//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I8R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24R8G8B8_I8R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // get a good index for the R,G,B source values
            *pDest        = m_QuantizedPalette.GetIndex(
                                pSrc[0],
                                pSrc[1],
                                pSrc[2],
                                0xff);

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=3;

            // increment the destination
            pDest++;

            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_I8R8G8B8A8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
        }

    };
static ConverterV24R8G8B8_I8R8G8B8A8 s_V24R8G8B8_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V24B8G8R8_I8R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV24B8G8R8_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24B8G8R8_I8R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // get a good index for the R,G,B source values
            *pDest        = m_QuantizedPalette.GetIndex(
                                pSrc[2],
                                pSrc[1],
                                pSrc[0],
                                0xff);

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=3;

            // increment the destination
            pDest++;

            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24B8G8R8_I8R8G8B8A8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
        }

    };
static ConverterV24B8G8R8_I8R8G8B8A8 s_V24B8G8R8_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8A8_V32R8G8B8A8 - Converter between HRPPixelTypeI8R8G8B8A8
//                           and HRPPixelTypeV32R8G8B8A8
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8A8_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        Byte* pSourceComposite;
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            if (pDestComposite[3] == 0 ||
                pSourceComposite[3] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor =  pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                pDestComposite[3] = 255 - pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3]));
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            ++pSrc;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        Byte* pSourceComposite;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);

            pi_PixelsCount -= 1;
            pDestComposite+=4;
            pSrc+=1;
            }
        };

    virtual void ConvertLostChannel(const void*  pi_pSourceRawData,
                         void*        pio_pDestRawData,
                         size_t        pi_PixelsCount,
                         const bool* pi_pChannelsMask) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            if(pi_pChannelsMask[0])
                pDestComposite[0] = pSourceComposite[0];
            if(pi_pChannelsMask[1])
                pDestComposite[1] = pSourceComposite[1];
            if(pi_pChannelsMask[2])
                pDestComposite[2] = pSourceComposite[2];
            if(pi_pChannelsMask[3])
                pDestComposite[3] = pSourceComposite[3];

            pi_PixelsCount -= 1;
            pDestComposite+=4;
            pSrc+=1;
            }
        }

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8A8_V32R8G8B8A8(*this));
        }
    };
static struct ConverterI8R8G8B8A8_V32R8G8B8A8 s_I8R8G8B8A8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I8R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32R8G8B8A8_I8R8G8B8A8()
        {
        };

    virtual void Compose(const void* pi_pSourceRawDataV32, void* pio_pDestRawDataI8, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite = (Byte*)pi_pSourceRawDataV32;
        Byte* pDest = (Byte*)pio_pDestRawDataI8;
        const HRPPixelPalette& rDestPalette = GetDestinationPixelType()->GetPalette();
        Byte Blend[4];
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            Byte* pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

            if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                *((uint32_t*)Blend) = *((uint32_t*)pSourceComposite);
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                Blend[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                Blend[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                Blend[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                Blend[3] = 255 - pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3]));
                }

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2], Blend[3]);

            --pi_PixelsCount;
            ++pDest;
            pSourceComposite+=4;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // convert all pixels
        while(pi_PixelsCount)
            {
            *pDest = m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2], pSrc[3]);

            pSrc+=4;
            pDest++;
            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_I8R8G8B8A8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries.
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            {
            Byte* pComposite = (Byte*) rPalette.GetCompositeValue(Index);
            m_QuantizedPalette.AddCompositeValue(pComposite,
                                                 (Byte)Index);
            }
        }

    };
static ConverterV32R8G8B8A8_I8R8G8B8A8 s_V32R8G8B8A8_I8R8G8B8A8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8R8G8B8A8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I8R8G8B8A8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8A8::CLASS_ID, &s_I8R8G8B8A8_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V8Gray8_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID, &s_V1Gray1_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V24B8G8R8_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I8R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8R8G8B8A8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    I8R8G8B8A8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        // XXX to implement -> V1Gray1
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8A8::CLASS_ID, &s_I8R8G8B8A8_I8R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_I8R8G8B8A8_V8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8A8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_I8R8G8B8A8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I8R8G8B8A8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I8R8G8B8A8_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB and 8 bit
// alpha.
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8A8::HRPPixelTypeI8R8G8B8A8()
    : HRPPixelTypeRGB(8,8,8,8,8)
    {
    HRPPixelPalette& rPixelPalette = (HRPPixelPalette&) GetPalette();
    uint32_t Value;

    // create a grayscale palette
    int32_t NbIndex(rPixelPalette.GetMaxEntries());
    for(int gray = 0; gray < NbIndex; gray++)
        {
        Value = (0xFF << 24) | (gray << 16) | (gray << 8) | gray;
        rPixelPalette.AddEntry(&Value);
        }
    }


//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB and 8 bit
// alpha with a palette
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8A8::HRPPixelTypeI8R8G8B8A8 (const HRPPixelPalette& pi_Palette)
    : HRPPixelTypeRGB(8,8,8,8,8)
    {
    HRPPixelPalette*    pDstPalette = (HRPPixelPalette*)&GetPalette();

    for (uint32_t Index=0; Index < pi_Palette.CountUsedEntries (); Index++)
        pDstPalette->AddEntry (pi_Palette.GetCompositeValue (Index));
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8A8::HRPPixelTypeI8R8G8B8A8(const HRPPixelTypeI8R8G8B8A8& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8A8::~HRPPixelTypeI8R8G8B8A8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI8R8G8B8A8::Clone() const
    {
    return new HRPPixelTypeI8R8G8B8A8(*this);
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
unsigned short HRPPixelTypeI8R8G8B8A8::CountValueBits() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8R8G8B8A8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8R8G8B8A8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8R8G8B8A8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8R8G8B8A8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

