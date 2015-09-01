//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI8R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeI8R8G8B8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPQuantizedPaletteR8G8B8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>

HPM_REGISTER_CLASS(HRPPixelTypeI8R8G8B8, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

static Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
static Byte s_SrcMask[9]    = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
static Byte s_DestMask[9]   = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };

//-----------------------------------------------------------------------------
//  s_I8R8G8B8_I8R8G8B8 - Converter between HRPPixelTypeI8R8G8B8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8_I8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        if (!NeedConversion)
            memcpy(pDest, pSrc, pi_PixelsCount);
        else
            {
            while(pi_PixelsCount)
                {
                // copy the pre-calculated destination index that fit the best
                *pDest = EntryConversion[*pSrc];

                pSrc++;
                pDest++;
                pi_PixelsCount--;
                }
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8_I8R8G8B8(*this));
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
            NeedConversion = false;
        else
            {
            NeedConversion = true;
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
    bool  NeedConversion;
    };

static ConverterI8R8G8B8_I8R8G8B8 s_I8R8G8B8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8_V8Gray8 - Converter
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8_V8Gray8 : public HRPPixelConverter
    {
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

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8_V8Gray8(*this));
        }
    };
static struct ConverterI8R8G8B8_V8Gray8 s_I8R8G8B8_V8Gray8;

//-----------------------------------------------------------------------------
//  s_V8Gray8_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8Gray8_I8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV8Gray8_I8R8G8B8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceRawData = (Byte*)pi_pSourceRawData;
        Byte* pDestRawData = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            *pDestRawData = m_QuantizedPalette.GetIndex(
                                *pSourceRawData,
                                *pSourceRawData,
                                *pSourceRawData);

            pi_PixelsCount--;
            pSourceRawData++;
            pDestRawData++;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV8Gray8_I8R8G8B8(*this));
        }

protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            {
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
            }
        }

private:

    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;
    };
static struct ConverterV8Gray8_I8R8G8B8 s_V8Gray8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V1Gray1_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV1Gray1_I8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            // Set this output pixel
            *pDest = m_Map[(*pSrc & s_BitMask[Index] ? 1 : 0)];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest++;

            // Increment Source
            Index++;
            if(Index == 8)
                {
                // Start reading from a new source byte
                Index = 0;
                pSrc++;
                }
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV1Gray1_I8R8G8B8(*this));
        }

protected:

    // this function pre-calculates a transformation table for fast conversion
    // between two bitmap rasters
    virtual void Update() override
        {
        // Get the palette of the destination pixel types
        const HRPPixelPalette& rDestPixelPalette = GetDestinationPixelType()->GetPalette();

        Byte pSrcValue[3];

        // black
        pSrcValue[0] = pSrcValue[1] = pSrcValue[2] = 0x00;
        m_Map[0] = (Byte)GetClosestEntryIndexInPalette(pSrcValue, rDestPixelPalette);

        // white
        pSrcValue[0] = pSrcValue[1] = pSrcValue[2] = 0xff;
        m_Map[1] = (Byte)GetClosestEntryIndexInPalette(pSrcValue, rDestPixelPalette);
        };

private:

    Byte  m_Map[2];

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
static ConverterV1Gray1_I8R8G8B8 s_V1Gray1_I8R8G8B8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8R8G8B8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I8R8G8B8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V8Gray8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID, &s_V1Gray1_I8R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8R8G8B8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    I8R8G8B8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_I8R8G8B8_V8Gray8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8::HRPPixelTypeI8R8G8B8()
    : HRPPixelTypeRGB(8,8,8,0,8)
    {
    HRPPixelPalette& rPixelPalette = (HRPPixelPalette&) GetPalette();
    uint32_t Value;

    // create a grayscale palette
    int32_t NbIndex(rPixelPalette.GetMaxEntries());
    for(int gray = 0; gray < NbIndex; gray++)
        {
        Value = (gray << 16) | (gray << 8) | gray;
        rPixelPalette.AddEntry(&Value);
        }
    }


//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
// with a palette
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8::HRPPixelTypeI8R8G8B8 (const HRPPixelPalette& pi_Palette)
    : HRPPixelTypeRGB(8,8,8,0,8)
    {
    HRPPixelPalette*    pDstPalette = (HRPPixelPalette*)&GetPalette();

    for (uint32_t Index=0; Index < pi_Palette.CountUsedEntries (); Index++)
        pDstPalette->AddEntry (pi_Palette.GetCompositeValue (Index));
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8::HRPPixelTypeI8R8G8B8(const HRPPixelTypeI8R8G8B8& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8::~HRPPixelTypeI8R8G8B8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI8R8G8B8::Clone() const
    {
    return new HRPPixelTypeI8R8G8B8(*this);
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
unsigned short HRPPixelTypeI8R8G8B8::CountValueBits() const
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8R8G8B8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8R8G8B8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8R8G8B8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8R8G8B8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }



