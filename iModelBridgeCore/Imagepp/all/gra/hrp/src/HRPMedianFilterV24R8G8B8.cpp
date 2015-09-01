//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPMedianFilterV24R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPTypedFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPMedianFilterV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HGFLuvColorSpace.h>


#define CHANNELS 3
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPMedianFilterV24R8G8B8
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilterV24R8G8B8::HRPMedianFilterV24R8G8B8()
    : HRPMedianFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_pColorSpaceConverter = new HGFLuvColorSpace(DEFAULT_GAMMA_FACTOR, 8);

    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilterV24R8G8B8::HRPMedianFilterV24R8G8B8(const HRPPixelNeighbourhood& pi_rNeighbourhood)
    : HRPMedianFilter(new HRPPixelTypeV24R8G8B8(), pi_rNeighbourhood)
    {
    m_pColorSpaceConverter = new HGFLuvColorSpace(DEFAULT_GAMMA_FACTOR, 8);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilterV24R8G8B8::HRPMedianFilterV24R8G8B8(const HRPMedianFilterV24R8G8B8& pi_rFilter)
    : HRPMedianFilter(pi_rFilter)
    {
    m_pColorSpaceConverter = new HGFLuvColorSpace(DEFAULT_GAMMA_FACTOR, 8);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilterV24R8G8B8::~HRPMedianFilterV24R8G8B8()
    {
    delete m_pColorSpaceConverter;

    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPMedianFilterV24R8G8B8::Clone() const
    {
    return new HRPMedianFilterV24R8G8B8(*this);
    }

//-----------------------------------------------------------------------------
// private
// Convoluate
//
// Note : This filter dont support the pixel positions. We do nothing with
//        pi_pPositionsX and pi_pPositionsY parameters.
//-----------------------------------------------------------------------------
void HRPMedianFilterV24R8G8B8::Convoluate(const void*     pi_pSrcRawData[],
                                          void*           po_pDestRawData,
                                          uint32_t        pi_Width,
                                          const double*  pi_pPositionsX,
                                          const double*  pi_pPositionsY) const
    {
    // get information about the width and height of the Neighbourhood
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();

    uint32_t NeighbourhoodHeight = rNeighbourhood.GetHeight();
    uint32_t NeighbourhoodWidth = rNeighbourhood.GetWidth();
    uint32_t MedianIndex = (NeighbourhoodHeight * NeighbourhoodWidth - 1) / 2;

    uint32_t PixelChannels = pi_Width * CHANNELS;
    Byte* pDestRawData = (Byte*) po_pDestRawData;

    std::list<double> list_L (NeighbourhoodHeight * NeighbourhoodWidth - 1);
    std::list<double>::iterator Iter0;
    double L;
    double U=0.0;
    double V=0.0;
    double Uoriginal;
    double Voriginal;


    for(uint32_t PixelChannel = 0; PixelChannel < PixelChannels; PixelChannel+=CHANNELS)
        {
        list_L.clear();

        for(uint32_t ElementY = 0; ElementY < NeighbourhoodHeight; ElementY++)
            {
            Byte* pSrcRawData = (Byte*)pi_pSrcRawData[ElementY] + PixelChannel;

            bool isOnOriginY =  ElementY == rNeighbourhood.GetYOrigin() ? true : false;

            for(uint32_t ElementX = 0; ElementX < NeighbourhoodWidth; ElementX++ , pSrcRawData+=CHANNELS)
                {
                //------------------------------------------
                // Convert to L*u*v
                if (isOnOriginY && ElementX == rNeighbourhood.GetXOrigin())
                    {
                    m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *(pSrcRawData + 1), *(pSrcRawData + 2), &L, &Uoriginal, &Voriginal);
                    }
                else
                    {
                    m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *(pSrcRawData + 1), *(pSrcRawData + 2), &L, &U, &V);
                    }

                list_L.push_back (L);
                }
            }

        list_L.sort();

        // use iterator to get the middle value of the sorted list for each channel
        Iter0 = list_L.begin();
        for (uint32_t i (0); i < MedianIndex; i++)
            {
            ++Iter0;
            }

        //------------------------------------------
        // Convert back to original pixeltype (RGB or RGBA)
        m_pColorSpaceConverter->ConvertToRGB (*Iter0/*L*/, U, V, &pDestRawData[PixelChannel], &pDestRawData[PixelChannel+1], &pDestRawData[PixelChannel+2]);
        }
    }
