//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV16PRGray8A8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV16PRGray8A8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPChannelOrgPRGray8A8.h>
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV16PRGray8A8, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V16PRGray8A8_V16PRGray8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV16PRGray8A8_V16PRGray8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte DemultipliedSource;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if (pSourceComposite[1] != 0)
                {
                if (pDestComposite[1] == 0 ||
                    pSourceComposite[1] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel
                    pDestComposite[0] = pSourceComposite[0];
                    pDestComposite[1] = pSourceComposite[1];
                    }
                else
                    {
                    HFCMath (*pQuotients) (HFCMath::GetInstance());

                    DemultipliedSource = pSourceComposite[0] * 255 / pSourceComposite[1];

                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[1] * (DemultipliedSource - pDestComposite[0])) + pDestComposite[0];

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[1] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[1]) * (255 - pDestComposite[1])));

                    // Premultiply result
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[1]);
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 2;
            pSourceComposite += 2;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            memcpy(pDestComposite, pSourceComposite, 2);

            pi_PixelsCount -= 1;
            pDestComposite +=2;
            pSourceComposite+=2;
            }
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pi_pChannelsMask[1])
                {
                pDestComposite[1] = pSourceComposite[1];

                // premultiplied values are changed
                pDestComposite[0] = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[1]);
                }

            pi_PixelsCount -= 1;
            pDestComposite +=2;
            pSourceComposite+=2;
            }
        };


    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16PRGray8A8_V16PRGray8A8(*this));
        }
    };
static ConverterV16PRGray8A8_V16PRGray8A8        s_V16PRGray8A8_V16PRGray8A8;

//-----------------------------------------------------------------------------
//  s_V16PRGray8A8_V8Gray8 - Converter
//-----------------------------------------------------------------------------
class ConverterV16PRGray8A8_V8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte InverseSourceAlpha;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[1] != 0)
                {
                InverseSourceAlpha = 255 - pSourceComposite[1];

                // (S * alpha) + (D * (1 - alpha))
                pDestComposite[0] = pSourceComposite[0] + pQuotients->DivideBy255ToByte((pDestComposite[0] * InverseSourceAlpha));
                }

            --pi_PixelsCount;
            ++pDestComposite;
            pSourceComposite += 2;
            }
        };


    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            *pDestComposite = *pSourceComposite;

            pi_PixelsCount -= 1;
            pDestComposite ++;
            pSourceComposite+=2;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16PRGray8A8_V8Gray8(*this));
        }
    };
static ConverterV16PRGray8A8_V8Gray8        s_V16PRGray8A8_V8Gray8;

//-----------------------------------------------------------------------------
//  s_V16PRGray8A8_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV16PRGray8A8_I8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV16PRGray8A8_I8R8G8B8()
        {
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        const HRPPixelPalette& rDestPalette = GetDestinationPixelType()->GetPalette();
        Byte Blend[3];
        Byte InverseSourceAlpha;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pDestComposite = (Byte*)rDestPalette.GetCompositeValue(*pDest);

            InverseSourceAlpha = 255 - pSourceComposite[1];

            // (S * alpha) + (D * (1 - alpha))
            Blend[0] = pSourceComposite[0] +
                       pQuotients->DivideBy255ToByte(pDestComposite[0] * InverseSourceAlpha);
            Blend[1] = pSourceComposite[0] +
                       pQuotients->DivideBy255ToByte(pDestComposite[1] * InverseSourceAlpha);
            Blend[2] = pSourceComposite[0] +
                       pQuotients->DivideBy255ToByte(pDestComposite[2] * InverseSourceAlpha);

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(
                         Blend[0],
                         Blend[1],
                         Blend[2]);

            pi_PixelsCount -= 1;
            pDest++;
            pSourceComposite+=2;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pSourceComposite[1] != 0)
                // get a good index for the R,G,B blend values
                *pDest = m_QuantizedPalette.GetIndex(
                             pSourceComposite[0] * 255 / pSourceComposite[1],
                             pSourceComposite[0] * 255 / pSourceComposite[1],
                             pSourceComposite[0] * 255 / pSourceComposite[1]);
            else
                // get a good index for the R,G,B blend values
                *pDest = m_QuantizedPalette.GetIndex(0,0,0);

            pi_PixelsCount -= 1;
            pDest++;
            pSourceComposite+=2;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }


    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16PRGray8A8_I8R8G8B8(*this));
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

private:

    static short m_LostChannels[];

    };
short ConverterV16PRGray8A8_I8R8G8B8::m_LostChannels[] = {1, -1};
static ConverterV16PRGray8A8_I8R8G8B8        s_V16PRGray8A8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V16PRGray8A8_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
class ConverterV16PRGray8A8_V24B8G8R8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte InverseSourceAlpha;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            InverseSourceAlpha = 255 - pSourceComposite[1];

            // (S * alpha) + (D * (1 - alpha))
            pDestComposite[0] = pSourceComposite[0] +
                                pQuotients->DivideBy255ToByte(pDestComposite[0] * InverseSourceAlpha);
            pDestComposite[1] = pSourceComposite[0] +
                                pQuotients->DivideBy255ToByte(pDestComposite[1] * InverseSourceAlpha);
            pDestComposite[2] = pSourceComposite[0] +
                                pQuotients->DivideBy255ToByte(pDestComposite[2] * InverseSourceAlpha);

            pi_PixelsCount -= 1;
            pDestComposite+=3;
            pSourceComposite+=2;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pSourceComposite[1] != 0)
                {
                // Same value for 3 channels
                pDestComposite[0] = pSourceComposite[0] * 255 / pSourceComposite[1];
                pDestComposite[1] = pDestComposite[0];
                pDestComposite[2] = pDestComposite[0];
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            pi_PixelsCount -= 1;
            pDestComposite+=3;
            pSourceComposite+=2;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16PRGray8A8_V24B8G8R8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV16PRGray8A8_V24B8G8R8::m_LostChannels[] = {1, -1};
static ConverterV16PRGray8A8_V24B8G8R8        s_V16PRGray8A8_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_V16PRGray8A8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16PRGray8A8_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte InverseSourceAlpha;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            InverseSourceAlpha = 255 - pSourceComposite[1];

            // (S * alpha) + (D * (1 - alpha))
            pDestComposite[0] = pSourceComposite[0] +
                                pQuotients->DivideBy255ToByte(pDestComposite[0] * InverseSourceAlpha);
            pDestComposite[1] = pSourceComposite[0] +
                                pQuotients->DivideBy255ToByte(pDestComposite[1] * InverseSourceAlpha);
            pDestComposite[2] = pSourceComposite[0] +
                                pQuotients->DivideBy255ToByte(pDestComposite[2] * InverseSourceAlpha);

            pi_PixelsCount -= 1;
            pDestComposite+=3;
            pSourceComposite+=2;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pSourceComposite[1] != 0)
                {
                // Same value for 3 channels
                pDestComposite[0] = pSourceComposite[0] * 255 / pSourceComposite[1];
                pDestComposite[1] = pDestComposite[0];
                pDestComposite[2] = pDestComposite[0];
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            pi_PixelsCount -= 1;
            pDestComposite+=3;
            pSourceComposite+=2;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16PRGray8A8_V24R8G8B8(*this));
        }


private:

    static short m_LostChannels[];

    };
short ConverterV16PRGray8A8_V24R8G8B8::m_LostChannels[] = {1, -1};
static struct ConverterV16PRGray8A8_V24R8G8B8        s_V16PRGray8A8_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V16PRGray8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V16PRGray8A8 : public HRPPixelConverter
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
            pDestComposite[0] = Byte(pSourceComposite[0] * REDFACTOR +
                                       pSourceComposite[1] * GREENFACTOR +
                                       pSourceComposite[2] * BLUEFACTOR);
            pDestComposite[1] = 0xff; // opaque

            pi_PixelsCount -= 1;
            pDestComposite+=2;
            pSourceComposite+=3;
            }
        };


    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V16PRGray8A8(*this));
        }
    };
static struct ConverterV24R8G8B8_V16PRGray8A8        s_V24R8G8B8_V16PRGray8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V16PRGray8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V16PRGray8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte SourceGray;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                if (pDestComposite[1] == 0 ||
                    pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel
                    pDestComposite[0] = Byte(pSourceComposite[0] * REDFACTOR +
                                               pSourceComposite[1] * GREENFACTOR +
                                               pSourceComposite[2] * BLUEFACTOR);
                    pDestComposite[1] = pSourceComposite[3];
                    }
                else
                    {
                    SourceGray = Byte(pSourceComposite[0] * REDFACTOR +
                                        pSourceComposite[1] * GREENFACTOR +
                                        pSourceComposite[2] * BLUEFACTOR);

                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    pDestComposite[0] = (Byte)(pQuotients->DivideBy255(pSourceComposite[3] * (SourceGray - pDestComposite[0])) + pDestComposite[0]);

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[1] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[1])));

                    // Premultiply result
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[1]);
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 2;
            pSourceComposite += 4;
            }
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite[0] = Byte(pSourceComposite[0] * REDFACTOR +
                                       pSourceComposite[1] * GREENFACTOR +
                                       pSourceComposite[2] * BLUEFACTOR) * pSourceComposite[3] / 255;
            pDestComposite[1] = pSourceComposite[3];

            pi_PixelsCount -= 1;
            pDestComposite+=2;
            pSourceComposite+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V16PRGray8A8(*this));
        }

    };
static struct ConverterV32R8G8B8A8_V16PRGray8A8        s_V32R8G8B8A8_V16PRGray8A8;

//-----------------------------------------------------------------------------
//  s_V16PRGray8A8_V24R8G8B8A8- Converter
//-----------------------------------------------------------------------------
struct ConverterV16PRGray8A8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte PremultDestColor;
        Byte DemultipliedSource;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if (pSourceComposite[1] != 0)
                {
                DemultipliedSource = pSourceComposite[0] * 255 / pSourceComposite[1];

                if (pDestComposite[3] == 0 ||
                    pSourceComposite[1] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel,
                    pDestComposite[0] = DemultipliedSource;
                    pDestComposite[1] = DemultipliedSource;
                    pDestComposite[2] = DemultipliedSource;
                    pDestComposite[3] = pSourceComposite[1];
                    }
                else
                    {
                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[1] * (DemultipliedSource - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                    pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[1] * (DemultipliedSource - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                    pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[1] * (DemultipliedSource - PremultDestColor)) + PremultDestColor;

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[1]) * (255 - pDestComposite[3])));
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 2;
            }
        };


    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pSourceComposite[1] != 0)
                {
                // Same value for 3 channels
                pDestComposite[0] = pSourceComposite[0] * 255 / pSourceComposite[1];
                pDestComposite[1] = pDestComposite[0];
                pDestComposite[2] = pDestComposite[0];
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            pDestComposite[3] = pSourceComposite[1];

            pi_PixelsCount -= 1;
            pDestComposite+=4;
            pSourceComposite+=2;
            }
        };

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterV16PRGray8A8_V32R8G8B8A8(*this));
        }
    };
static struct ConverterV16PRGray8A8_V32R8G8B8A8        s_V16PRGray8A8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16PRGray8A8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V16PRGray8A8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16PRGray8A8::CLASS_ID, &s_V16PRGray8A8_V16PRGray8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V16PRGray8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V16PRGray8A8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16PRGray8A8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V16PRGray8A8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16PRGray8A8::CLASS_ID, &s_V16PRGray8A8_V16PRGray8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V16PRGray8A8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V16PRGray8A8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_V16PRGray8A8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V16PRGray8A8_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V16PRGray8A8_V8Gray8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV16PRGray8A8::HRPPixelTypeV16PRGray8A8()
    : HRPPixelType(HRPChannelOrgPRGray8A8(), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16PRGray8A8::HRPPixelTypeV16PRGray8A8(const HRPPixelTypeV16PRGray8A8& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16PRGray8A8::~HRPPixelTypeV16PRGray8A8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV16PRGray8A8::Clone() const
    {
    return new HRPPixelTypeV16PRGray8A8(*this);
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
unsigned short HRPPixelTypeV16PRGray8A8::CountValueBits() const
    {
    return 16;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16PRGray8A8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16PRGray8A8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16PRGray8A8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16PRGray8A8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }
