//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV32PR8PG8PB8A8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV32PR8PG8PB8A8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPChannelOrgPR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV32PR8PG8PB8A8, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> > MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_V32PR8PG8PB8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32PR8PG8PB8A8_V32PR8PG8PB8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

   virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel,
                    *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);
                    }
                else
                    {
                    register unsigned NewValue;

                    // Cdst' = Csrc + (1 - Asrc) * Cdst
                    NewValue = pSourceComposite[0] + pQuotients->UnsignedDivideBy255((255-pSourceComposite[3])*pDestComposite[0]);
                    pDestComposite[0] = (Byte)MIN(NewValue, 255);
                    NewValue = pSourceComposite[1] + pQuotients->UnsignedDivideBy255((255-pSourceComposite[3])*pDestComposite[1]);
                    pDestComposite[1] = (Byte)MIN(NewValue, 255);
                    NewValue = pSourceComposite[2] + pQuotients->UnsignedDivideBy255((255-pSourceComposite[3])*pDestComposite[2]);
                    pDestComposite[2] = (Byte)MIN(NewValue, 255);
                    NewValue = pSourceComposite[3] + pQuotients->UnsignedDivideBy255((255-pSourceComposite[3])*pDestComposite[3]);
                    pDestComposite[3] = (Byte)MIN(NewValue, 255);
                    }
                }

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, 4*pi_PixelsCount);
        };

    virtual void ConvertLostChannel(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount,
                         const bool* pi_pChannelsMask) const override
        {
        if(!pi_pChannelsMask[3])
            return;

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If pixel if fully transparent we cannot recover original color value.
            if(pDestComposite[3] != 0)
                {
                // Cdest  = (PRCdest * 255) / Adest
                // PRCdest' = (Cdest * Asrc) / 255
                pDestComposite[0] = MIN(255, (pDestComposite[0] * pSourceComposite[3]) / pDestComposite[3]);
                pDestComposite[1] = MIN(255, (pDestComposite[1] * pSourceComposite[3]) / pDestComposite[3]);
                pDestComposite[2] = MIN(255, (pDestComposite[2] * pSourceComposite[3]) / pDestComposite[3]);
                }
            pDestComposite[3] = pSourceComposite[3];

            --pi_PixelsCount;
            pDestComposite +=4;
            pSourceComposite+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PR8PG8PB8A8_V32PR8PG8PB8A8(*this));
        }
    };
static ConverterV32PR8PG8PB8A8_V32PR8PG8PB8A8        s_V32PR8PG8PB8A8_V32PR8PG8PB8A8;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32PR8PG8PB8A8_I8R8G8B8 : public HRPPixelConverter
    {
    Byte                 m_GoodIndexForBlack;
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32PR8PG8PB8A8_I8R8G8B8()
        {
        };
    
    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rDestPalette = GetDestinationPixelType()->GetPalette();
        Byte Blend[3];

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                Byte* pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

                register unsigned NewValue;

                // alpha * (S - D) + D = (alpha*s) - (alpha*D) + D --> PR_S - (alpha*D) + D
                NewValue = pSourceComposite[0] - pQuotients->UnsignedDivideBy255(pDestComposite[0]*pSourceComposite[3]) + pDestComposite[0];
                Blend[0] = (Byte)MIN(NewValue, 255);
                NewValue = pSourceComposite[1] - pQuotients->UnsignedDivideBy255(pDestComposite[1]*pSourceComposite[3]) + pDestComposite[1];
                Blend[1] = (Byte)MIN(NewValue, 255);
                NewValue = pSourceComposite[2] - pQuotients->UnsignedDivideBy255(pDestComposite[2]*pSourceComposite[3]) + pDestComposite[2];
                Blend[2] = (Byte)MIN(NewValue, 255);


                // get a good index for the R,G,B blend values
                *pDest = m_QuantizedPalette.GetIndex(Blend[0], Blend[1], Blend[2]);
                }

            --pi_PixelsCount;
            ++pDest;
            pSourceComposite+=4;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pSourceComposite[3] != 0)
                {
                // get a good index for the R,G,B blend values
                *pDest = m_QuantizedPalette.GetIndex(MIN(255, pSourceComposite[0] * 255 / pSourceComposite[3]),
                                                     MIN(255, pSourceComposite[1] * 255 / pSourceComposite[3]),
                                                     MIN(255, pSourceComposite[2] * 255 / pSourceComposite[3]));
                }
            else
                {
                *pDest = m_GoodIndexForBlack;
                }

            --pi_PixelsCount;
            ++pDest;
            pSourceComposite+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PR8PG8PB8A8_I8R8G8B8(*this));
        }


protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),(Byte) Index);

        m_GoodIndexForBlack = m_QuantizedPalette.GetIndex(0,0,0);
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32PR8PG8PB8A8_I8R8G8B8::m_LostChannels[] = {3, -1};
static ConverterV32PR8PG8PB8A8_I8R8G8B8        s_V32PR8PG8PB8A8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PR8PG8PB8A8_V24B8G8R8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                register unsigned NewValue;

                // alpha * (S - D) + D = ((alpha*s) - (alpha*D)) + D --> (PR_S - (alpha*D)) + D
                NewValue = pSourceComposite[2] - pQuotients->UnsignedDivideBy255(pDestComposite[0]*pSourceComposite[3]) + pDestComposite[0];
                pDestComposite[0] = (Byte)MIN(NewValue, 255);
                NewValue = pSourceComposite[1] - pQuotients->UnsignedDivideBy255(pDestComposite[1]*pSourceComposite[3]) + pDestComposite[1];
                pDestComposite[1] = (Byte)MIN(NewValue, 255);
                NewValue = pSourceComposite[0] - pQuotients->UnsignedDivideBy255(pDestComposite[2]*pSourceComposite[3]) + pDestComposite[2];
                pDestComposite[2] = (Byte)MIN(NewValue, 255);
                }

            --pi_PixelsCount;
            pDestComposite+=3;
            pSourceComposite+=4;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // Extract original color from PR color. If transparent, we cannot retrieve original value.
            if(pSourceComposite[3] != 0)
                {
                pDestComposite[0] = MIN(255, pSourceComposite[2] * 255 / pSourceComposite[3]);
                pDestComposite[1] = MIN(255, pSourceComposite[1] * 255 / pSourceComposite[3]);
                pDestComposite[2] = MIN(255, pSourceComposite[0] * 255 / pSourceComposite[3]);
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            --pi_PixelsCount;
            pDestComposite+=3;
            pSourceComposite+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PR8PG8PB8A8_V24B8G8R8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32PR8PG8PB8A8_V24B8G8R8::m_LostChannels[] = {3, -1};
static struct ConverterV32PR8PG8PB8A8_V24B8G8R8        s_V32PR8PG8PB8A8_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PR8PG8PB8A8_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                register unsigned NewValue;

                // alpha * (S - D) + D = (alpha*s) - (alpha*D) + D --> PR_S - (alpha*D) + D
                NewValue = pSourceComposite[0] - pQuotients->UnsignedDivideBy255(pDestComposite[0]*pSourceComposite[3]) + pDestComposite[0];
                pDestComposite[0] = (Byte)MIN(NewValue, 255);
                NewValue = pSourceComposite[1] - pQuotients->UnsignedDivideBy255(pDestComposite[1]*pSourceComposite[3]) + pDestComposite[1];
                pDestComposite[1] = (Byte)MIN(NewValue, 255);
                NewValue = pSourceComposite[2] - pQuotients->UnsignedDivideBy255(pDestComposite[2]*pSourceComposite[3]) + pDestComposite[2];
                pDestComposite[2] = (Byte)MIN(NewValue, 255);
                }

            --pi_PixelsCount;
            pDestComposite+=3;
            pSourceComposite+=4;
            }
        };

   virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // Extract original color from PR color. If transparent, we cannot retrieve original value.
            if(pSourceComposite[3] != 0)
                {
                pDestComposite[0] = MIN(255, pSourceComposite[0] * 255 / pSourceComposite[3]);
                pDestComposite[1] = MIN(255, pSourceComposite[1] * 255 / pSourceComposite[3]);
                pDestComposite[2] = MIN(255, pSourceComposite[2] * 255 / pSourceComposite[3]);
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            --pi_PixelsCount;
            pDestComposite+=3;
            pSourceComposite+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PR8PG8PB8A8_V24R8G8B8(*this));
        }

private:
    static short m_LostChannels[];
    };
short ConverterV32PR8PG8PB8A8_V24R8G8B8::m_LostChannels[] = {3, -1};
static struct ConverterV32PR8PG8PB8A8_V24R8G8B8        s_V32PR8PG8PB8A8_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V32PR8PG8PB8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V32PR8PG8PB8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

   virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            memcpy(pDestComposite, pSourceComposite, 3);

            pDestComposite[3] = 0xff; // opaque

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V32PR8PG8PB8A8(*this));
        }
    };
static struct ConverterV24R8G8B8_V32PR8PG8PB8A8        s_V24R8G8B8_V32PR8PG8PB8A8;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PR8PG8PB8A8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

   virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        HFCMath const* pQuotients = HFCMath::GetInstance();

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // if the source is fully transparent the destination remain unchanged
            if(pSourceComposite[3] != 0)
                {
                if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel,
                    pDestComposite[0] = MIN(255, pSourceComposite[0] * 255 / pSourceComposite[3]);
                    pDestComposite[1] = MIN(255, pSourceComposite[1] * 255 / pSourceComposite[3]);
                    pDestComposite[2] = MIN(255, pSourceComposite[2] * 255 / pSourceComposite[3]);
                    pDestComposite[3] = pSourceComposite[3];
                    }
                else
                    {
                    uint32_t ComposedDestAlpha = (255 - pSourceComposite[3]) * pDestComposite[3];
                    uint32_t TotalWeight = pSourceComposite[3] + ComposedDestAlpha;

                    register unsigned NewValue;

                    // Cdst' = Csrc * Asrc + ( (1 - Asrc) * Adst * Cdst ) / (Asrc + ((1 - Asrc) * Adst))
                    // Both alphas need to be taken into account, and weight the result
                    NewValue = (pSourceComposite[0] * 255 + (ComposedDestAlpha * pDestComposite[0])) / TotalWeight;
                    pDestComposite[0] = (Byte)MIN(255, NewValue);
                    NewValue = (pSourceComposite[1] * 255 + (ComposedDestAlpha * pDestComposite[1])) / TotalWeight;
                    pDestComposite[1] = (Byte)MIN(255, NewValue);
                    NewValue = (pSourceComposite[2] * 255 + (ComposedDestAlpha * pDestComposite[2])) / TotalWeight;
                    pDestComposite[2] = (Byte)MIN(255, NewValue);

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[3] = 255 - pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3]));
                    }
                }

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Extract original color from PR color. If transparent, we cannot retrieve original value.
            if(pSourceComposite[3] != 0)
                {
                pDestComposite[0] = MIN(255, pSourceComposite[0] * 255 / pSourceComposite[3]);
                pDestComposite[1] = MIN(255, pSourceComposite[1] * 255 / pSourceComposite[3]);
                pDestComposite[2] = MIN(255, pSourceComposite[2] * 255 / pSourceComposite[3]);
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            pDestComposite[3] = pSourceComposite[3];

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PR8PG8PB8A8_V32R8G8B8A8(*this));
        }
    };
static struct ConverterV32PR8PG8PB8A8_V32R8G8B8A8    s_V32PR8PG8PB8A8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V32PR8PG8PB8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V32PR8PG8PB8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            // if the source is fully transparent the destination remain unchanged
            if(pSourceComposite[3] != 0)
                {
                if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel,
                    pDestComposite[0] = pQuotients->UnsignedDivideBy255(pSourceComposite[0] * pSourceComposite[3]);
                    pDestComposite[1] = pQuotients->UnsignedDivideBy255(pSourceComposite[1] * pSourceComposite[3]);
                    pDestComposite[2] = pQuotients->UnsignedDivideBy255(pSourceComposite[2] * pSourceComposite[3]);
                    pDestComposite[3] = pSourceComposite[3];
                    }
                else
                    {
                    // if all PR => Cdst' = Csrc + (1 - Asrc) * Cdst
                    // in our case src is not PR =>  Cdst' = Csrc*Asrc + (1 - Asrc) * Cdst
                    pDestComposite[0] = pQuotients->UnsignedDivideBy255(pSourceComposite[0] * pSourceComposite[3]) + pQuotients->UnsignedDivideBy255((255 - pSourceComposite[3]) * pDestComposite[0]);
                    pDestComposite[1] = pQuotients->UnsignedDivideBy255(pSourceComposite[1] * pSourceComposite[3]) + pQuotients->UnsignedDivideBy255((255 - pSourceComposite[3]) * pDestComposite[1]);
                    pDestComposite[2] = pQuotients->UnsignedDivideBy255(pSourceComposite[2] * pSourceComposite[3]) + pQuotients->UnsignedDivideBy255((255 - pSourceComposite[3]) * pDestComposite[2]);
                    pDestComposite[3] = pSourceComposite[3] + pQuotients->UnsignedDivideBy255((255 - pSourceComposite[3]) * pDestComposite[3]);
                    }
                }

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

   virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite[0] = pQuotients->UnsignedDivideBy255(pSourceComposite[0] * pSourceComposite[3]);
            pDestComposite[1] = pQuotients->UnsignedDivideBy255(pSourceComposite[1] * pSourceComposite[3]);
            pDestComposite[2] = pQuotients->UnsignedDivideBy255(pSourceComposite[2] * pSourceComposite[3]);
            pDestComposite[3] = pSourceComposite[3];

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V32PR8PG8PB8A8(*this));
        }
    };
static struct ConverterV32R8G8B8A8_V32PR8PG8PB8A8        s_V32R8G8B8A8_V32PR8PG8PB8A8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32PR8PG8PB8A8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V32PR8PG8PB8A8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID, &s_V32PR8PG8PB8A8_V32PR8PG8PB8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V32PR8PG8PB8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V32PR8PG8PB8A8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32PR8PG8PB8A8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V32PR8PG8PB8A8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID, &s_V32PR8PG8PB8A8_V32PR8PG8PB8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V32PR8PG8PB8A8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V32PR8PG8PB8A8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_V32PR8PG8PB8A8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32PR8PG8PB8A8_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV32PR8PG8PB8A8::HRPPixelTypeV32PR8PG8PB8A8()
    : HRPPixelType(HRPChannelOrgPR8PG8PB8A8(), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32PR8PG8PB8A8::HRPPixelTypeV32PR8PG8PB8A8(const HRPPixelTypeV32PR8PG8PB8A8& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32PR8PG8PB8A8::~HRPPixelTypeV32PR8PG8PB8A8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV32PR8PG8PB8A8::Clone() const
    {
    return new HRPPixelTypeV32PR8PG8PB8A8(*this);
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
unsigned short HRPPixelTypeV32PR8PG8PB8A8::CountValueBits() const
    {
    return 32;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32PR8PG8PB8A8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32PR8PG8PB8A8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32PR8PG8PB8A8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32PR8PG8PB8A8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }
