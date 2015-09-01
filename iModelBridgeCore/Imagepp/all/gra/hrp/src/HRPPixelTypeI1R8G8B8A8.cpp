//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI1R8G8B8A8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeI1R8G8B8A8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8A8.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HFCMath.h>

static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };

HPM_REGISTER_CLASS(HRPPixelTypeI1R8G8B8A8, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8_I1R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8_I1R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8_I1R8G8B8A8()
        {
        };

    virtual ~ConverterI1R8G8B8A8_I1R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte const* pSrc = (Byte const*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            if(m_ConvertEntryConversion[((*pSrc) >> (7 - Index)) & 0x01] == 1)
                *pDest |= s_BitMask[Index];
            else
                *pDest &= s_NotBitMask[Index];

            ++Index;

            if(Index == 8)
                {
                Index = 0;
                ++pSrc;
                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        Byte Index = 0;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pi_pChannelsMask[3])
                {
                // we do nothing with the alpha
                }

            Index++;

            if(Index == 8)
                {
                Index = 0;
                pDest ++;
                pSrc++;
                }

            pi_PixelsCount --;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte const* pSrc = (Byte const*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            if(m_ComposeEntryConversion[((*pSrc) >> (7 - Index)) & 0x01][((*pDest) >> (7 - Index)) & 0x01]/* == 1*/)
                *pDest |= s_BitMask[Index];
            else
                *pDest &= s_NotBitMask[Index];

            ++Index;

            if(Index == 8)
                {
                Index = 0;
                ++pSrc;
                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    virtual HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8_I1R8G8B8A8(*this));
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
static ConverterI1R8G8B8A8_I1R8G8B8A8 s_I1R8G8B8A8_I1R8G8B8A8;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8_I1R8G8B8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8_I1R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8_I1R8G8B8()
        {
        };

    virtual ~ConverterI1R8G8B8A8_I1R8G8B8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte const* pSrc = (Byte const*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            if(m_ConvertEntryConversion[((*pSrc) >> (7 - Index)) & 0x01] == 1)
                *pDest |= s_BitMask[Index];
            else
                *pDest &= s_NotBitMask[Index];

            ++Index;

            if(Index == 8)
                {
                Index = 0;
                ++pSrc;
                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        Byte Index = 0;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pi_pChannelsMask[3])
                {
                // we do nothing with the alpha
                }

            Index++;

            if(Index == 8)
                {
                Index = 0;
                pDest ++;
                pSrc++;
                }

            pi_PixelsCount --;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte const* pSrc = (Byte const*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            if(m_ComposeEntryConversion[((*pSrc) >> (7 - Index)) & 0x01][((*pDest) >> (7 - Index)) & 0x01]/* == 1*/)
                *pDest |= s_BitMask[Index];
            else
                *pDest &= s_NotBitMask[Index];

            ++Index;

            if(Index == 8)
                {
                Index = 0;
                ++pSrc;
                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };



    virtual HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8_I1R8G8B8(*this));
        };

protected:

    // this function pre-calculates a transformation table for fast conversion between two bitmap rasters
    virtual void Update() override
        {
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        HRPPaletteOctreeR8G8B8 destPalOctree;
        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(0), 0);
        destPalOctree.AddCompositeValue(rDestPixelPalette.GetCompositeValue(1), 1);

        // *** Build m_composeEntryConversion
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
                m_ConvertEntryConversion[Index] = destPalOctree.GetIndex(pSrcValue[0], pSrcValue[1], pSrcValue[2]);
                }
            }

        // *** Build m_composeEntryConversion
        Byte* pSrcValue;
        Byte* pDestValue;
        Byte TmpDst[3];

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
                    if (pSrcValue[3] == 255)
                        {
                        // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                        TmpDst[0] = pSrcValue[0];
                        TmpDst[1] = pSrcValue[1];
                        TmpDst[2] = pSrcValue[2];
                        }
                    else
                        {
                        TmpDst[0] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[0] - pDestValue[0])) + pDestValue[0];
                        TmpDst[1] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[1] - pDestValue[1])) + pDestValue[1];
                        TmpDst[2] = pQuotients->DivideBy255ToByte(pSrcValue[3] * (pSrcValue[2] - pDestValue[2])) + pDestValue[2];
                        }
                    }
                else
                    {
                    // Source is completely transparent.
                    TmpDst[0] = pDestValue[0];
                    TmpDst[1] = pDestValue[1];
                    TmpDst[2] = pDestValue[2];
                    }

                // find the closest entry index in the destination palette
                m_ComposeEntryConversion[SrcIndex][DstIndex] = destPalOctree.GetIndex(TmpDst[0], TmpDst[1], TmpDst[2]);
                }
            }
        };

private:

    // conversion table
    Byte m_ConvertEntryConversion[2];
    Byte m_ComposeEntryConversion[2][2];
    };
static ConverterI1R8G8B8A8_I1R8G8B8 s_I1R8G8B8A8_I1R8G8B8;


//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (7- Index)) & 0x01));

            // alpha * (S - D) + D
            pDest[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDest[0])) + pDest[0];
            pDest[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDest[1])) + pDest[1];
            pDest[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDest[2])) + pDest[2];

            // One less pixel to go
            --pi_PixelsCount;

            // Increment Destination by 3 bytes !
            pDest+=3;

            Index++;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (7- Index)) & 0x01));

            pDest[0] = pSourceComposite[0];
            pDest[1] = pSourceComposite[1];
            pDest[2] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            Index++;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterI1R8G8B8A8_V24R8G8B8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterI1R8G8B8A8_V24R8G8B8::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8_V24R8G8B8 s_I1R8G8B8A8_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();
        Byte* pSourceComposite;
        Byte Index = 0;

        Byte PremultDestColor;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (7- Index)) & 0x01));

            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                if (pDestComposite[3] == 0 ||
                    pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel,
                    *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);
                    }
                else
                    {
                    HFCMath (*pQuotients) (HFCMath::GetInstance());

                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                    pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                    pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3])));
                    }
                }

            --pi_PixelsCount;
            pDestComposite +=4;

            Index++;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (7- Index)) & 0x01));

            *((uint32_t*)pDest) = *((uint32_t*)pSourceComposite);

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=4;

            Index++;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        Byte* pSrc = (Byte*) pi_pSourceRawData;
        Byte* pDest = (Byte*) pio_pDestRawData;

        Byte Index = 0;

        while(pi_PixelsCount != 0)
            {
            if(pi_pChannelsMask[3])
                {
                pDest[3] = ((Byte*)GetSourcePixelType()->GetPalette().GetCompositeValue((*pSrc >> (7- Index)) & 0x01))[3];
                }

            Index++;

            if(Index == 8)
                {
                Index = 0;
                pSrc++;
                }

            pDest += 4;

            pi_PixelsCount--;
            }
        };

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterI1R8G8B8A8_V32R8G8B8A8(*this));
        }

private:

    };
static ConverterI1R8G8B8A8_V32R8G8B8A8 s_I1R8G8B8A8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8_V24B8G8R8 : public HRPPixelConverter
    {

    bool             m_BlackOpAndWhiteTr;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (7- Index)) & 0x01));

            pDest[2] = pSourceComposite[0];
            pDest[1] = pSourceComposite[1];
            pDest[0] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            Index++;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

  
    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            // Optimization
            if(m_BlackOpAndWhiteTr && Index == 0)
                {
                while(pi_PixelsCount >= 32)
                    {
                    if(*((uint32_t*)pSrc) == 0x00000000)
                        {
                        memset(pDest, 0x00, 3 * 32);

                        pSrc += 4;
                        pDest += 3 * 32;
                        pi_PixelsCount -= 32;
                        }
                    else if(*((uint32_t*)pSrc) == 0xFFFFFFFF)
                        {
                        pSrc += 4;
                        pDest += 3 * 32;
                        pi_PixelsCount -= 32;
                        }
                    else
                        break;
                    }

                if(pi_PixelsCount == 0)
                    break;
                }

            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (7- Index)) & 0x01));

            // Color is opaque ?
            if (pSourceComposite[3] == 255)
                {
                pDest[2] = pSourceComposite[0];
                pDest[1] = pSourceComposite[1];
                pDest[0] = pSourceComposite[2];
                }

            // Color is semi-transparent
            else if (pSourceComposite[3] > 0)
                {
                // alpha * (S - D) + D
                pDest[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDest[2])) + pDest[2];
                pDest[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDest[1])) + pDest[1];
                pDest[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDest[0])) + pDest[0];
                }

            // One less pixel to go
            --pi_PixelsCount;

            // Increment Destination by 3 bytes !
            pDest+=3;

            Index++;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8_V24B8G8R8(*this));
        }

protected:

    // this function test if we are in the special case
    // black opaque and white transparent (OPTIMIZATION)
    virtual void Update() override
        {
        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette = GetSourcePixelType()->GetPalette();

        if(    *((uint32_t*)(rSrcPixelPalette.GetCompositeValue(0))) == 0xFF000000 &&
               *((uint32_t*)(rSrcPixelPalette.GetCompositeValue(1))) == 0x00FFFFFF)
            m_BlackOpAndWhiteTr = true;
        else
            m_BlackOpAndWhiteTr = false;
        }

private:


    static short m_LostChannels[];
    };
short ConverterI1R8G8B8A8_V24B8G8R8::m_LostChannels[] = {3, -1};
static ConverterI1R8G8B8A8_V24B8G8R8 s_I1R8G8B8A8_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I1R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I1R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24R8G8B8_I1R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // get a good index for the R,G,B opaque source values
            if(m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2], 0xff) == 1)
                *pDest |= s_BitMask[Index];
            else
                *pDest &= s_NotBitMask[Index];

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=3;

            ++Index;

            if(Index == 8)
                {
                // Increment the destination
                Index = 0;

                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterV24R8G8B8_I1R8G8B8A8(*this));
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
static ConverterV24R8G8B8_I1R8G8B8A8 s_V24R8G8B8_I1R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I1R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I1R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32R8G8B8A8_I1R8G8B8A8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // Get a good index for the R,G,B,A source values
            if(m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2], pSrc[3]) /*== 1*/)
                *pDest |= s_BitMask[Index];
            else
                *pDest &= s_NotBitMask[Index];

            // Increment the pointer to the next 32-bit (4 bytes) composite value
            pSrc+=4;

            ++Index;

            if(Index == 8)
                {
                // Increment the destination
                Index = 0;

                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                Byte* pDestComposite = (Byte*)GetDestinationPixelType()->GetPalette().GetCompositeValue((*pDest >> (7-Index)) & 0x01);

                if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                    if(m_QuantizedPalette.GetIndex(pSourceComposite[0], pSourceComposite[1], pSourceComposite[2], pSourceComposite[3])/* == 1*/)
                        *pDest |= s_BitMask[Index];
                    else
                        *pDest &= s_NotBitMask[Index];
                    }
                else
                    {
                    HFCMath (*pQuotients) (HFCMath::GetInstance());
                    Byte TmpDst[4];

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

                    if(m_QuantizedPalette.GetIndex(TmpDst[0], TmpDst[1], TmpDst[2], TmpDst[3])/* == 1*/)
                        *pDest |= s_BitMask[Index];
                    else
                        *pDest &= s_NotBitMask[Index];
                    }
                }

            // Increment the pointer to the next 32-bit (4 bytes) composite value
            pSourceComposite+=4;

            ++Index;

            if(Index == 8)
                {
                // Increment the destination
                Index = 0;

                ++pDest;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };


    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterV32R8G8B8A8_I1R8G8B8A8(*this));
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
static ConverterV32R8G8B8A8_I1R8G8B8A8 s_V32R8G8B8A8_I1R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I1R8G8B8A8_I8R8G8B8 - Converter between g_PixelTypeFingerPrintI8R8G8B8
//-----------------------------------------------------------------------------
class ConverterI1R8G8B8A8_I8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8    m_QuantizedPalette;

    bool                      m_BlackOpAndWhiteTr;
    Byte                      m_BlackApproxEntry;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterI1R8G8B8A8_I8R8G8B8()
        {
        };

    virtual ~ConverterI1R8G8B8A8_I8R8G8B8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            *pDest = m_EntryConversion[((*pSrc) >> (7 - Index)) & 0x01];

            Index++;

            if(Index == 8)
                {
                Index = 0;
                pSrc++;
                }

            pDest++;
            pi_PixelsCount--;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte  Index = 0;
        Byte* pSourceComposite;
        Byte* pDestComposite;

        const HRPPixelPalette& rSrcPalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPalette = GetDestinationPixelType()->GetPalette();

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            if(m_BlackOpAndWhiteTr && Index == 0)
                {
                while(pi_PixelsCount >= 32)
                    {
                    if(*((uint32_t*)pSrc) == 0x00000000)
                        {
                        memset(pDest, m_BlackApproxEntry, 32);

                        pSrc += 4;
                        pDest += 32;
                        pi_PixelsCount -= 32;
                        }
                    else if(*((uint32_t*)pSrc) == 0xFFFFFFFF)
                        {
                        pSrc += 4;
                        pDest += 32;
                        pi_PixelsCount -= 32;
                        }
                    else
                        break;
                    }

                if(pi_PixelsCount == 0)
                    break;
                }


            // copy the pre-calculated destination index that fit the best
            pSourceComposite = (Byte*)rSrcPalette.GetCompositeValue(((*pSrc) >> (7 - Index)) & 0x01);

            // Color is opaque ?
            if (pSourceComposite[3] == 255)
                {
                // copy the pre-calculated destination index that fit the best
                *pDest = m_QuantizedPalette.GetIndex(pSourceComposite[0], pSourceComposite[1], pSourceComposite[2]);
                }

            // Color is transparent
            else if (pSourceComposite[3] == 0)
                {
                pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

                // copy the pre-calculated destination index that fit the best
                *pDest = m_QuantizedPalette.GetIndex(pDestComposite[0], pDestComposite[1], pDestComposite[2]);
                }
            else
                {
                uint32_t Blend[3];

                pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

                // alpha * (S - D) + D
                Blend[0] = pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
                Blend[1] = pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
                Blend[2] = pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

                // copy the pre-calculated destination index that fit the best
                *pDest = m_QuantizedPalette.GetIndex((Byte)Blend[0], (Byte)Blend[1], (Byte)Blend[2]);
                }

            Index++;

            if(Index == 8)
                {
                Index = 0;
                pSrc++;
                }

            pDest++;
            pi_PixelsCount--;
            }
        };


    virtual HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI1R8G8B8A8_I8R8G8B8(*this));
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
                m_EntryConversion[Index] = Index;
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
                m_EntryConversion[Index] = (Byte)GetClosestEntryIndexInPalette(pSrcValue,rDestPixelPalette);
                }
            }

        // fill the octree with the destination palette entries
        int32_t NbIndex(rDestPixelPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rDestPixelPalette.GetCompositeValue(Index),
                                                 (Byte)Index);


        //OPTIMIZATION ONLY
        if(    *((uint32_t*)(rSrcPixelPalette.GetCompositeValue(0))) == 0xFF000000 &&
               *((uint32_t*)(rSrcPixelPalette.GetCompositeValue(1))) == 0x00FFFFFF)
            {
            m_BlackOpAndWhiteTr = true;

            // Get an approximation for the black in the destination palette
            m_BlackApproxEntry = m_QuantizedPalette.GetIndex(0, 0, 0);
            }
        else
            {
            m_BlackOpAndWhiteTr = false;
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

    // conversion table
    Byte m_EntryConversion[2];
    };
static ConverterI1R8G8B8A8_I8R8G8B8 s_I1R8G8B8A8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I1R8G8B8A8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I1R8G8B8A8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8_I1R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I1R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I1R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I1R8G8B8A8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    I1R8G8B8A8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8_I1R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI1R8G8B8::CLASS_ID, &s_I1R8G8B8A8_I1R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I1R8G8B8A8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_I1R8G8B8A8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I1R8G8B8A8_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I1R8G8B8A8_I8R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8A8::HRPPixelTypeI1R8G8B8A8()
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
HRPPixelTypeI1R8G8B8A8::HRPPixelTypeI1R8G8B8A8 (const HRPPixelPalette& pi_Palette)
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
HRPPixelTypeI1R8G8B8A8::HRPPixelTypeI1R8G8B8A8(const HRPPixelTypeI1R8G8B8A8& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj),
      HRPPixelType1BitInterface(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI1R8G8B8A8::~HRPPixelTypeI1R8G8B8A8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI1R8G8B8A8::Clone() const
    {
    return new HRPPixelTypeI1R8G8B8A8(*this);
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
unsigned short HRPPixelTypeI1R8G8B8A8::CountValueBits() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// Get1BitInterface
//-----------------------------------------------------------------------------
HRPPixelType1BitInterface* HRPPixelTypeI1R8G8B8A8::Get1BitInterface()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI1R8G8B8A8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I1R8G8B8A8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI1R8G8B8A8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I1R8G8B8A8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }
