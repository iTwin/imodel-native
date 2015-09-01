//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI4R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeI4R8G8B8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

static const Byte s_BitMask[4]    = {0xf0, 0x0f};

HPM_REGISTER_CLASS(HRPPixelTypeI4R8G8B8, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_I4R8G8B8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterI4R8G8B8_V24R8G8B8 : public HRPPixelConverter
    {
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
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (4 - Index)) & 0x0f));

            pDest[0] = pSourceComposite[0];
            pDest[1] = pSourceComposite[1];
            pDest[2] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            Index+=4;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterI4R8G8B8_V24R8G8B8(*this));
        }
    };
static ConverterI4R8G8B8_V24R8G8B8 s_I4R8G8B8_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_I4R8G8B8_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
class ConverterI4R8G8B8_V24B8G8R8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue((*pSrc >> (4 - Index)) & 0x0f));

            pDest[2] = pSourceComposite[0];
            pDest[1] = pSourceComposite[1];
            pDest[0] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            Index+=4;
            if(Index == 8)
                {
                // Increment Destination by 1 byte (index) !
                pSrc++;
                Index = 0;
                }
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI4R8G8B8_V24B8G8R8(*this));
        }
    };
static ConverterI4R8G8B8_V24B8G8R8 s_I4R8G8B8_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I4R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I4R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24R8G8B8_I4R8G8B8()
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
            *pDest &= ~s_BitMask[(Index / 4)];

            *pDest |= (m_QuantizedPalette.GetIndex(
                           pSrc[0],
                           pSrc[1],
                           pSrc[2]) << (4 - Index));

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=3;

            //+Increment Source
            Index += 4;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                pDest++;
                }

            pi_PixelsCount--;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= 0xF0;
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_I4R8G8B8(*this));
        }

protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        for(Byte Index = 0; Index < 16; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 Index);
        }

    };
static ConverterV24R8G8B8_I4R8G8B8 s_V24R8G8B8_I4R8G8B8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I4R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I4R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32R8G8B8A8_I4R8G8B8()
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
            *pDest &= ~s_BitMask[(Index / 4)];
            *pDest |= (m_QuantizedPalette.GetIndex(
                           pSrc[0],
                           pSrc[1],
                           pSrc[2]) << (4 - Index));

            // increment the pointer to the next 32-bits(4 bytes) composite value
            pSrc+=4;

            //+Increment Source
            Index += 4;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                pDest++;
                }

            pi_PixelsCount--;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= 0xF0;
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        Byte* pSourceComposite  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;
        Byte ComposedRGB[3];

        while(pi_PixelsCount)
            {
            Byte const* pDestComposite = (Byte const*)(rPalette.GetCompositeValue((*pDest >> (4 - Index)) & 0x0f));

            // alpha * (S - D) + D
            ComposedRGB[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[0])) + pDestComposite[0];
            ComposedRGB[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            ComposedRGB[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[2])) + pDestComposite[2];

            *pDest &= ~s_BitMask[(Index / 4)];
            *pDest |= (m_QuantizedPalette.GetIndex(ComposedRGB[0], ComposedRGB[1], ComposedRGB[2]) << (4 - Index));

            // increment the pointer to the next 32-bits(4 bytes) composite value
            pSourceComposite+=4;

            //+Increment Source
            Index += 4;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                pDest++;
                }

            --pi_PixelsCount;
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= 0xF0;
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_I4R8G8B8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        for(Byte Index = 0; Index < 16; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 Index);
        }


private:

    static short m_LostChannels[];

    };
short ConverterV32R8G8B8A8_I4R8G8B8::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_I4R8G8B8 s_V32R8G8B8A8_I4R8G8B8;


//-----------------------------------------------------------------------------
//  s_I4R8G8B8_I8R8G8B8 - Converter between HRPPixelTypeI8R8G8B8
//-----------------------------------------------------------------------------
class ConverterI4R8G8B8_I8R8G8B8 : public HRPPixelConverter
    {
    // Conversion table
    Byte EntryConversion[16];

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            // copy the pre-calculated destination index that fit the best
            *pDest = EntryConversion[((*pSrc) >> (4 - Index)) & 0x0f];

            Index+=4;

            if(Index == 8)
                {
                Index = 0;
                pSrc++;
                }

            pDest++;
            pi_PixelsCount--;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI4R8G8B8_I8R8G8B8(*this));
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
            for(Byte Index = 0; Index < 16; Index++)
                EntryConversion[Index] = Index;
            }
        else
            {
            Byte* pSrcValue;

            // for each entry in the source palette, calculate the closest entry
            // in the destination palette
            for(uint32_t Index = 0; Index < 16; Index++)
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
    };
static ConverterI4R8G8B8_I8R8G8B8 s_I4R8G8B8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I4R8G8B8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I4R8G8B8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I4R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I4R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I4R8G8B8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    I4R8G8B8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I4R8G8B8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_I4R8G8B8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I4R8G8B8_I8R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeI4R8G8B8::HRPPixelTypeI4R8G8B8()
    : HRPPixelTypeRGB(8,8,8,0,4)
    {
    HRPPixelPalette& rPixelPalette = LockPalette();

    // by default, create a gray palette
    Byte Color[3];
    for(uint32_t EntryIndex = 0; EntryIndex < 16; EntryIndex++)
        {
        Color[0] = Color[1] = Color[2] = (Byte)(255 * EntryIndex / 15);
        rPixelPalette.AddEntry(&Color);
        }

    UnlockPalette();
    }


//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
// with a palette
//-----------------------------------------------------------------------------
HRPPixelTypeI4R8G8B8::HRPPixelTypeI4R8G8B8 (const HRPPixelPalette& pi_Palette)
    : HRPPixelTypeRGB(8,8,8,0,4)
    {
    HRPPixelPalette*    pDstPalette = (HRPPixelPalette*)&GetPalette();

    for (uint32_t Index=0; Index < pi_Palette.CountUsedEntries (); Index++)
        pDstPalette->AddEntry (pi_Palette.GetCompositeValue (Index));
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI4R8G8B8::HRPPixelTypeI4R8G8B8(const HRPPixelTypeI4R8G8B8& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI4R8G8B8::~HRPPixelTypeI4R8G8B8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI4R8G8B8::Clone() const
    {
    return new HRPPixelTypeI4R8G8B8(*this);
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
unsigned short HRPPixelTypeI4R8G8B8::CountValueBits() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI4R8G8B8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I4R8G8B8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI4R8G8B8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I4R8G8B8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }
