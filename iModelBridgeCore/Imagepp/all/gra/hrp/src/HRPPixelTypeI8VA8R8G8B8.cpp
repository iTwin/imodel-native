//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI8VA8R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeI8VA8R8G8B8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8A8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPQuantizedPaletteR8G8B8.h>
#include <Imagepp/all/h/HFCMath.h>


HPM_REGISTER_CLASS(HRPPixelTypeI8VA8R8G8B8, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Converter between HRPPixelTypeI8VA8R8G8B8 and HRPPixelTypeI8VA8R8G8B8.

    This converter is used static with the static object : s_I8R8G8B8A8_I8R8G8B8A8

    -----------------------------------------------------------------------------
 */
class ConverterI8VA8R8G8B8_I8VA8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    /** -----------------------------------------------------------------------------
        This is the public constructors ConverterI8VA8R8G8B8_I8VA8R8G8B8
        -----------------------------------------------------------------------------
     */
    ConverterI8VA8R8G8B8_I8VA8R8G8B8()
        {
        };

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  16 bits data pointer to a I8VA8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte const* pSrc = (Byte const*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // Copy the pre-calculated destination index that fit the best
            pDest[0] = EntryConversion[*pSrc];

            // Copy the alpha raw data directly.
            pDest[1] = pSrc[1];

            // Increment the source and the destination.
            pSrc  += 2;
            pDest += 2;

            pi_PixelsCount--;
            }
        };


    /** -----------------------------------------------------------------------------
        Compose pixels sources to destinations pixels. Compose take account of alpha
        chanel.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  16 bits data pointer to a I8VA8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPalette   = GetDestinationPixelType()->GetPalette();
        Byte Blend[3];
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);
            Byte* pDestComposite   = (Byte*)rDestPalette.GetCompositeValue(*pDest);

            if (pDest[1] == 0 ||
                pSrc[1] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                Blend[0] = pSourceComposite[0];
                Blend[1] = pSourceComposite[1];
                Blend[2] = pSourceComposite[2];

                pDest[1] = pSrc[1];
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255].
                PremultDestColor = pQuotients->DivideBy255ToByte(pDest[1] * pDestComposite[0]);
                Blend[0] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDest[1] * pDestComposite[1]);
                Blend[1] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDest[1] * pDestComposite[2]);
                Blend[2] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                pDest[1] = 255 - (pQuotients->DivideBy255ToByte((255 - pSrc[1]) * (255 - pDest[1])));
                }

            // Get a good index for the R,G,B blend values
            pDest[0] = m_QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2]);

            pDest += 2;
            pSrc  += 2;

            --pi_PixelsCount;
            }
        };

    /** -----------------------------------------------------------------------------
        Creates a new instance of the current object.

        @return A new instance of a pixel converter.
        -----------------------------------------------------------------------------
     */
    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterI8VA8R8G8B8_I8VA8R8G8B8(*this));
        }

protected:

    /** -----------------------------------------------------------------------------
        This method is called when the source and destination pixel types are both defined.
        This metod is used to set the Entry Conversion table.
        -----------------------------------------------------------------------------
     */
    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // Fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);

        // Get the palette of the source and destination pixel types
        const HRPPixelPalette& rSrcPixelPalette  = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        if(rSrcPixelPalette == rDestPixelPalette)
            {
            // If src and dest palettes are the same, make an index-index map
            for(unsigned short Index = 0; Index < 256; Index++)
                EntryConversion[Index] = static_cast<Byte>(Index);
            }
        else
            {
            Byte* pSrcValue;

            // For each entry in the source palette, calculate the closest entry
            // in the destination palette
            int32_t NbIndex(rSrcPixelPalette.CountUsedEntries());
            for(int32_t Index = 0; Index < NbIndex; Index++)
                {
                // Get the composite value associated to the source entry index
                // in the source palette
                pSrcValue=(Byte*)rSrcPixelPalette.GetCompositeValue(Index);

                // Find the closest entry index in the destination palette
                EntryConversion[Index] = (Byte)GetClosestEntryIndexInPalette(pSrcValue, rDestPixelPalette);
                }
            }
        }

private:

    /** -----------------------------------------------------------------------------
        Get the closest index in a palette from a given composite color.


        @param pi_pValue         Pixel value to find in the pi_rPixelPalette.
        @param pi_rPixelPalette  Palette used to find the closest entry.

        @return The index of the closest pixel found in the palette.
        -----------------------------------------------------------------------------
     */
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
static ConverterI8VA8R8G8B8_I8VA8R8G8B8  s_I8VA8R8G8B8_I8VA8R8G8B8;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Converter between HRPPixelTypeI8VA8R8G8B8 and HRPPixelTypeV32R8G8B8A8.

    This converter is used static with the static object : s_I8R8G8B8A8_V32R8G8B8A8

    -----------------------------------------------------------------------------
 */
class ConverterI8VA8R8G8B8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  32 bits data pointer to a V32R8G8B8A8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;

        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte* pSourceComposite;

        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(pSrc[0]);

            pDestComposite[0] = pSourceComposite[0];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[2];
            pDestComposite[3] = pSrc[1];

            pi_PixelsCount -= 1;
            pDestComposite += 4;
            pSrc           += 2;
            }
        };

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  32 bits data pointer to a V32R8G8B8A8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        @param pi_pChannelsMask  Channel to mask
        -----------------------------------------------------------------------------
     */
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
                pDestComposite[3] = pSrc[1];

            pi_PixelsCount -= 1;
            pDestComposite += 4;
            pSrc           += 2;
            }
        }

    /** -----------------------------------------------------------------------------
        Compose pixels sources to destinations pixels. Compose take account of alpha
        chanel.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  32 bits data pointer to a V32R8G8B8A8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;

        Byte* pSourceComposite;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            if (pDestComposite[3] == 0 ||
                pSrc[1] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                pDestComposite[0] = pSourceComposite[0];
                pDestComposite[1] = pSourceComposite[1];
                pDestComposite[2] = pSourceComposite[2];

                pDestComposite[3] = pSrc[1];
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                pDestComposite[0] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                pDestComposite[1] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                pDestComposite[2] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                pDestComposite[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSrc[1]) * (255 - pDestComposite[3])));
                }

            pi_PixelsCount -= 1;
            pDestComposite +=4;
            pSrc += 2;
            }
        };

    /** -----------------------------------------------------------------------------
        Creates a new instance of the current object.

        @return A new instance of a pixel converter.
        -----------------------------------------------------------------------------
     */
    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterI8VA8R8G8B8_V32R8G8B8A8(*this));
        }
    };
static ConverterI8VA8R8G8B8_V32R8G8B8A8 s_I8VA8R8G8B8_V32R8G8B8A8;



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Converter between HRPPixelTypeI8VA8R8G8B8 and HRPPixelTypeV24R8G8B8.

    This converter is used static with the static object : s_I8R8G8B8A8_V24R8G8B8

    -----------------------------------------------------------------------------
 */
class ConverterI8VA8R8G8B8_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  24 bits data pointer to a V24R8G8B8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;

        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte* pSourceComposite;

        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(pSrc[0]);

            pDestComposite[0] = pSourceComposite[0];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[2];

            pi_PixelsCount -= 1;
            pDestComposite += 3;
            pSrc           += 2;
            }
        };

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  32 bits data pointer to a V32R8G8B8A8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        @param pi_pChannelsMask  Channel to mask
        -----------------------------------------------------------------------------
     */
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

            pi_PixelsCount -= 1;
            pDestComposite += 3;
            pSrc           += 2;
            }
        }

    /** -----------------------------------------------------------------------------
        Compose pixels sources to destinations pixels. Compose take account of alpha
        chanel.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  24 bits data pointer to a V24R8G8B8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;

        Byte* pSourceComposite;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);

            // alpha * (S - D) + D
            pDestComposite[0] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
            pDestComposite[1] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            pDestComposite[2] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

            --pi_PixelsCount;
            pDestComposite += 3;
            pSrc += 2;
            }
        };

    /** -----------------------------------------------------------------------------
        Creates a new instance of the current object.

        @return A new instance of a pixel converter.
        -----------------------------------------------------------------------------
     */
    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterI8VA8R8G8B8_V24R8G8B8(*this));
        }
    };
static ConverterI8VA8R8G8B8_V24R8G8B8 s_I8VA8R8G8B8_V24R8G8B8;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Converter between HRPPixelTypeV32R8G8B8A8 and HRPPixelTypeI8VA8R8G8B8.

    This converter is used static with the static object : s_V32R8G8B8A8_I8R8G8B8A8

    -----------------------------------------------------------------------------
 */
class ConverterV32R8G8B8A8_I8VA8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    /** -----------------------------------------------------------------------------
        This is the public constructors for ConverterV32R8G8B8A8_I8VA8R8G8B8
        -----------------------------------------------------------------------------
     */
    ConverterV32R8G8B8A8_I8VA8R8G8B8()
        {
        };

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 32 bits data pointer to a V32R8G8B8A8 pixel
        @param pio_pDestRawData  16 bits data pointer to a I8VA8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // Convert all pixels
        while(pi_PixelsCount)
            {
            pDest[0] = m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2]);
            pDest[1] = pSrc[3];

            // Increment the pointer to the next 32-bit (4 bytes) composite value
            pSrc+=4;

            // Increment the destination
            pDest += 2;

            pi_PixelsCount--;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // IPP_TODO_COMPOSE
        Convert(pi_pSourceRawData, pio_pDestRawData, pi_PixelsCount);
        }

    /** -----------------------------------------------------------------------------
        Creates a new instance of the current object.

        @return A new instance of a pixel converter.
        -----------------------------------------------------------------------------
     */
    HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterV32R8G8B8A8_I8VA8R8G8B8(*this));
        }


protected:

    /** -----------------------------------------------------------------------------
        This method is called when the source and destination pixel types are both defined.
        This metod is used to set the Entry Conversion table.
        -----------------------------------------------------------------------------
     */
    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // Fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
        };
    };
static ConverterV32R8G8B8A8_I8VA8R8G8B8 s_V32R8G8B8A8_I8VA8R8G8B8;



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Converter between HRPPixelTypeV24R8G8B8 and HRPPixelTypeI8VA8R8G8B8.

    This converter is used static with the static object : s_V24R8G8B8_I8VA8R8G8B8

    -----------------------------------------------------------------------------
 */
class ConverterV24R8G8B8_I8VA8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    /** -----------------------------------------------------------------------------
        This is the public constructors for ConverterV24R8G8B8_I8VA8R8G8B8
        -----------------------------------------------------------------------------
     */
    ConverterV24R8G8B8_I8VA8R8G8B8()
        {
        };

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 24 bits data pointer to a V24R8G8B8 pixel
        @param pio_pDestRawData  16 bits data pointer to a I8VA8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // Convert all pixels
        while(pi_PixelsCount)
            {
            pDest[0] = m_QuantizedPalette.GetIndex(pSrc[0], pSrc[1], pSrc[2]);
            pDest[1] = 0xff;

            // Increment the pointer to the next 32-bit (4 bytes) composite value
            pSrc+=3;

            // Increment the destination
            pDest += 2;

            pi_PixelsCount--;
            }
        };

    /** -----------------------------------------------------------------------------
        Creates a new instance of the current object.

        @return A new instance of a pixel converter.
        -----------------------------------------------------------------------------
     */
    HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterV24R8G8B8_I8VA8R8G8B8(*this));
        }


protected:

    /** -----------------------------------------------------------------------------
        This method is called when the source and destination pixel types are both defined.
        This metod is used to set the Entry Conversion table.
        -----------------------------------------------------------------------------
     */
    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // Fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
        };
    };
static ConverterV24R8G8B8_I8VA8R8G8B8 s_V24R8G8B8_I8VA8R8G8B8;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    Converter between HRPPixelTypeI8VA8R8G8B8 and HRPPixelTypeI8R8G8B8A8.

    This converter is used static with the static object : s_I8VA8R8G8B8A8_I8R8G8B8A8

    -----------------------------------------------------------------------------
 */
class ConverterI8VA8R8G8B8_I8R8G8B8A8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8A8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    /** -----------------------------------------------------------------------------
        This is the public constructors ConverterI8VA8R8G8B8_I8R8G8B8A8
        -----------------------------------------------------------------------------
     */
    ConverterI8VA8R8G8B8_I8R8G8B8A8()
        {
        };

    /** -----------------------------------------------------------------------------
        Convert pixels to a others.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  8  bits data pointer to a I8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte* pSourceComposite;

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(GetSourcePixelType()->GetPalette().GetCompositeValue(pSrc[0]));

            *pDest = m_QuantizedPalette.GetIndex(pSourceComposite[0],
                                                 pSourceComposite[1],
                                                 pSourceComposite[2],
                                                 pSrc[1]);

            // Increment the source and the destination.
            pSrc += 2;
            pDest++;

            pi_PixelsCount--;
            }
        };

    /** -----------------------------------------------------------------------------
        Compose pixels sources to destinations pixels. Compose take account of alpha
        chanel.

        I8VA8Pointer[0] ==> Index data to a palette
        I8VA8Pointer[1] ==> 8 bits alpha raw data

        @param pi_pSourceRawData 16 bits data pointer to a I8VA8 pixel
        @param pio_pDestRawData  8  bits data pointer to a I8 pixel
        @param pi_PixelsCount    Number of pixel to convert
        -----------------------------------------------------------------------------
     */
    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        const HRPPixelPalette& rSourcePalette = GetSourcePixelType()->GetPalette();
        const HRPPixelPalette& rDestPalette   = GetDestinationPixelType()->GetPalette();
        Byte Blend[4];
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rSourcePalette.GetCompositeValue(*pSrc);
            Byte* pDestComposite   = (Byte*)rDestPalette.GetCompositeValue(*pDest);

            if (pDestComposite[3] == 0 ||
                pSrc[1] == 255)
                {
                // Destination pixel is fully transparent, or source pixel
                // is fully opaque. Copy source pixel,
                Blend[0] = pSourceComposite[0];
                Blend[1] = pSourceComposite[1];
                Blend[2] = pSourceComposite[2];
                Blend[3] = pSrc[1];
                }
            else
                {
                // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                // Alphas are in [0, 255].
                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                Blend[0] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                Blend[1] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                Blend[2] = pQuotients->DivideBy255ToByte(pSrc[1] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                // --> Transparency percentages are multiplied
                Blend[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSrc[1]) * (255 - pDestComposite[3])));
                }

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(
                         Blend[0],
                         Blend[1],
                         Blend[2],
                         Blend[3]);

            ++pDest;
            pSrc += 2;

            --pi_PixelsCount;
            }
        };

    /** -----------------------------------------------------------------------------
        Creates a new instance of the current object.

        @return A new instance of a pixel converter.
        -----------------------------------------------------------------------------
     */
    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterI8VA8R8G8B8_I8R8G8B8A8(*this));
        }

protected:

    /** -----------------------------------------------------------------------------
        This method is called when the source and destination pixel types are both defined.
        This metod is used to set the Entry Conversion table.
        -----------------------------------------------------------------------------
     */
    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries. Place the colors against
        // a black background to eliminate alpha. When composing, we will also place
        // the pixels against a black background, and this will give a common ground
        // for searching in an RGB octree. Blakc is used because its value is 0, so
        // the destination part isn't meaningful.
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            {
            Byte* pComposite = (Byte*) rPalette.GetCompositeValue(Index);
            m_QuantizedPalette.AddCompositeValue(pComposite,
                                                 (Byte)Index);
            }
        }
    };
static ConverterI8VA8R8G8B8_I8R8G8B8A8  s_I8VA8R8G8B8_I8R8G8B8A8;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Dictionnary of converters from other pixel types

    -----------------------------------------------------------------------------
 */
struct I8VA8R8G8B8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I8VA8R8G8B8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8VA8R8G8B8::CLASS_ID, &s_I8VA8R8G8B8_I8VA8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I8VA8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I8VA8R8G8B8));
        };
    };

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Dominic Gagné (${mailto:Dominic.Gagne@hmrinc.com})

    Dictionnary of converters to other pixel types

    -----------------------------------------------------------------------------
 */
struct I8VA8R8G8B8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    I8VA8R8G8B8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8VA8R8G8B8::CLASS_ID, &s_I8VA8R8G8B8_I8VA8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I8VA8R8G8B8_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I8VA8R8G8B8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8A8::CLASS_ID, &s_I8VA8R8G8B8_I8R8G8B8A8));
        };
    };

/** -----------------------------------------------------------------------------
    Constructor for Index 8 bits with palette true color 24 bits RGB and 8 bit
    alpha.
    -----------------------------------------------------------------------------
 */
HRPPixelTypeI8VA8R8G8B8::HRPPixelTypeI8VA8R8G8B8()
    : HRPPixelTypeRGB(8,8,8,8,8,8,8,8,0)
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


/** -----------------------------------------------------------------------------
    Constructor for Index 8 bits with palette true color 24 bits RGB and 8 bit
    alpha.

    @param pi_Palette      Pixel palette to set in the pixel type.
    -----------------------------------------------------------------------------
 */
HRPPixelTypeI8VA8R8G8B8::HRPPixelTypeI8VA8R8G8B8 (const HRPPixelPalette& pi_Palette)
    : HRPPixelTypeRGB(8,8,8,8,8,8,8,8,0)
    {
    HRPPixelPalette*    pDstPalette = (HRPPixelPalette*)&GetPalette();

    for (uint32_t Index=0; Index < pi_Palette.CountUsedEntries (); Index++)
        pDstPalette->AddEntry (pi_Palette.GetCompositeValue (Index));
    }

/** -----------------------------------------------------------------------------
    Copy Constructor.

    @param pi_rObj
    -----------------------------------------------------------------------------
 */
HRPPixelTypeI8VA8R8G8B8::HRPPixelTypeI8VA8R8G8B8(const HRPPixelTypeI8VA8R8G8B8& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
 */
HRPPixelTypeI8VA8R8G8B8::~HRPPixelTypeI8VA8R8G8B8()
    {
    }

/** -----------------------------------------------------------------------------
    Make a copy of self.
    -----------------------------------------------------------------------------
 */
HPMPersistentObject* HRPPixelTypeI8VA8R8G8B8::Clone() const
    {
    return new HRPPixelTypeI8VA8R8G8B8(*this);
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
unsigned short HRPPixelTypeI8VA8R8G8B8::CountValueBits() const
    {
    return 8;
    }

/** -----------------------------------------------------------------------------
    This function is used to set the default raw data with a composite value.

    @see HRPPixelType::SetDefaultCompositeValue()
    @see HRPPixelType::SetDefaultRawData()
    @end
    -----------------------------------------------------------------------------
 */
void HRPPixelTypeI8VA8R8G8B8::SetDefaultCompositeValue(const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);

    // the first 3 bytes was the RGB value
    uint32_t RawValue = FindNearestEntryInPalette(pi_pValue);
    RawValue |= (((Byte*)pi_pValue)[3] << 8);
    SetDefaultRawData(&RawValue);
    }

/** -----------------------------------------------------------------------------
    This method create a quantized palette.

    @note This method is overloaded from HRPPixelTypeRGB because the alpha channel
          must be not use into the quantized palette.

    @param pi_MaxEntries Max entries for the quantized palette.

    @see HRPPixelTypeRGB::CreateQuantizedPalette()
    @end
    -----------------------------------------------------------------------------
 */
HRPQuantizedPalette* HRPPixelTypeI8VA8R8G8B8::CreateQuantizedPalette(uint32_t pi_MaxEntries) const
    {
    HRPQuantizedPalette* pQuantizedPalette = 0;

    // return an appropriate quantized palette
    pQuantizedPalette = new HRPQuantizedPaletteR8G8B8((unsigned short)pi_MaxEntries, 8);

    return pQuantizedPalette;
    }

/** -----------------------------------------------------------------------------
    This method is used to find the best entry into the palette for the RGB value.

    @note This method is overloaded from HRPPixelTypeRGB because the alpha channel
          must be not use into the octree.

    @see HRPPixelTypeRGB::FindNearestEntryInPalette()
    @end
    -----------------------------------------------------------------------------
 */
uint32_t HRPPixelTypeI8VA8R8G8B8::FindNearestEntryInPalette(const void* pi_pValue) const
    {
    HRPPaletteOctreeR8G8B8 Octree;

    const HRPPixelPalette& rPalette = GetPalette();

    for(uint32_t EntryIndex = 0; EntryIndex < rPalette.CountUsedEntries(); EntryIndex++)
        Octree.AddCompositeValue(rPalette.GetCompositeValue(EntryIndex), (Byte)EntryIndex);

    return(Octree.GetIndex(((Byte*)pi_pValue)[0], ((Byte*)pi_pValue)[1], ((Byte*)pi_pValue)[2]));
    }




/** -----------------------------------------------------------------------------
    This return the converter from pi_pPixelTypeFrom to I8VA8R8G8B8 if it exist.

    @param pi_pPixelTypeFrom  Pixel type to find conterter for.
    -----------------------------------------------------------------------------
 */
const HRPPixelConverter* HRPPixelTypeI8VA8R8G8B8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8VA8R8G8B8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

/** -----------------------------------------------------------------------------
    This return the converter from I8VA8R8G8B8 to pi_pPixelTypeTo if it exist.

    @param pi_pPixelTypeFrom  Pixel type to find conterter for.
    -----------------------------------------------------------------------------
 */
const HRPPixelConverter* HRPPixelTypeI8VA8R8G8B8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8VA8R8G8B8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

