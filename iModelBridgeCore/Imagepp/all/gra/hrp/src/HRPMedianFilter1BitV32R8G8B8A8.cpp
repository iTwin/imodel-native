//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPMedianFilter1BitV32R8G8B8A8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPTypedFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPMedianFilter1BitV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>



#define CHANNELS 4
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPMedianFilter1BitV32R8G8B8A8
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter1BitV32R8G8B8A8::HRPMedianFilter1BitV32R8G8B8A8()
    : HRPMedianFilter(new HRPPixelTypeV32R8G8B8A8())
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter1BitV32R8G8B8A8::HRPMedianFilter1BitV32R8G8B8A8(const HRPPixelNeighbourhood& pi_rNeighbourhood)
    : HRPMedianFilter(new HRPPixelTypeV32R8G8B8A8(), pi_rNeighbourhood)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter1BitV32R8G8B8A8::HRPMedianFilter1BitV32R8G8B8A8(const HRPMedianFilter1BitV32R8G8B8A8& pi_rFilter)
    : HRPMedianFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter1BitV32R8G8B8A8::~HRPMedianFilter1BitV32R8G8B8A8()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPMedianFilter1BitV32R8G8B8A8::Clone() const
    {
    return new HRPMedianFilter1BitV32R8G8B8A8(*this);
    }

//-----------------------------------------------------------------------------
// private
// Convoluate
//
// Note : This filter dont support the pixel positions. We do nothing with
//        pi_pPositionsX and pi_pPositionsY parameters.
//-----------------------------------------------------------------------------
void HRPMedianFilter1BitV32R8G8B8A8::Convoluate(const void*     pi_pSrcRawData[],
                                                void*           po_pDestRawData,
                                                uint32_t        pi_Width,
                                                const double*  pi_pPositionsX,
                                                const double*  pi_pPositionsY) const
    {
    // get information about the width and height of the Neighbourhood
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();

    uint32_t NeighbourhoodHeight = rNeighbourhood.GetHeight();
    uint32_t NeighbourhoodWidth = rNeighbourhood.GetWidth();

    uint32_t PixelChannels = pi_Width * CHANNELS;
    Byte* pDestRawData = (Byte*) po_pDestRawData;

    for(uint32_t PixelChannel = 0; PixelChannel < PixelChannels; PixelChannel+=CHANNELS)
        {
        uint32_t totalOfWhite (0);
        uint32_t totalOfBlack (0);

        for(uint32_t ElementY = 0; ElementY < NeighbourhoodHeight; ElementY++)
            {
            Byte* pSrcRawData = (Byte*)pi_pSrcRawData[ElementY] + PixelChannel;

            for(uint32_t ElementX = 0; ElementX < NeighbourhoodWidth; ElementX++ , pSrcRawData+=CHANNELS)
                {
                //since the source is 1bit, only check the first channel
                if (pSrcRawData[0])
                    {
                    totalOfWhite++;
                    }
                else
                    {
                    totalOfBlack++;
                    }
                }
            }

        Byte newValue (0);
        // use most frequent color and set it in all channels
        if (totalOfBlack < totalOfWhite)
            newValue = 255;

        pDestRawData[PixelChannel] = newValue;
        pDestRawData[PixelChannel+1] = newValue;
        pDestRawData[PixelChannel+2] = newValue;
        pDestRawData[PixelChannel+3] = 255;
        }
    }
