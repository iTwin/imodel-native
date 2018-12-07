/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/ImagePPInternal/gra/DownSampling.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;
class HGSResampling;
END_IMAGEPP_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* SumType_T : Deduce sum type from type
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename DataType_T> struct SumType_T {};
template <> struct SumType_T<uint8_t> { typedef uint8_t _MyType; typedef uint32_t _SumType; };
template <> struct SumType_T<uint16_t> { typedef uint16_t _MyType; typedef uint32_t _SumType; };
template <> struct SumType_T<uint32_t> { typedef uint32_t _MyType; typedef uint32_t _SumType; };
template <> struct SumType_T<int8_t> { typedef int8_t _MyType; typedef int32_t _SumType; };
template <> struct SumType_T<int16_t> { typedef int16_t _MyType; typedef int32_t _SumType; };
template <> struct SumType_T<int32_t> { typedef int32_t _MyType; typedef int32_t _SumType; };
template <> struct SumType_T<float> { typedef float _MyType; typedef double _SumType; };
template <> struct SumType_T<double> { typedef double _MyType; typedef double _SumType; };

/*---------------------------------------------------------------------------------**//**
* OptimizedDownSampler
+---------------+---------------+---------------+---------------+---------------+------*/
struct OptimizedDownSampler
    {
    OptimizedDownSampler(){}
    virtual ~OptimizedDownSampler(){}

    virtual void _Compute(Byte* pDest, Byte const* pSrc) const = 0;
    virtual void _ComputeBound(Byte* pDest, Byte const* pSrc, uint32_t srcValidCountX, uint32_t srcValidCountY) const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* AverageOptimizedDownSampler_T
* DownSample any image block by a factor of 2
+---------------+---------------+---------------+---------------+---------------+------*/
template <uint32_t TileSizeX_T, uint32_t TileSizeY_T, uint32_t ChannelCount_T, typename Data_T>
struct AverageOptimizedDownSampler_T : public OptimizedDownSampler
    {
    typedef typename SumType_T<Data_T>::_SumType _SumType;

    AverageOptimizedDownSampler_T(){}
    virtual ~AverageOptimizedDownSampler_T(){}

    virtual void _Compute(Byte* pDest, Byte const* pSrc) const override
        {
        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T*       bufOut = (Data_T*)pDest;

        for(uint64_t line=0; line < TileSizeY_T/2; ++line)
            {
            Data_T* pDestLine = bufOut + line*(TileSizeX_T*ChannelCount_T);
            Data_T const* pLine0 = bufIn + (line*2)*(TileSizeX_T*ChannelCount_T);
            Data_T const* pLine1 = bufIn + ((line*2)+1)*(TileSizeX_T*ChannelCount_T);
            for(uint64_t column=0; column < TileSizeX_T/2; ++column)
                {
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    {
                    pDestLine[column*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[(column * 2)*ChannelCount_T + channel] +
                        pLine0[((column*2)+1)*ChannelCount_T + channel] +
                        pLine1[(column*2)*ChannelCount_T + channel] + 
                        pLine1[((column*2)+1)*ChannelCount_T + channel]) / 4);
                    }
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ComputeBound(Byte* pDest, Byte const* pSrc, uint32_t srcValidCountX, uint32_t srcValidCountY) const override
        {
        BeAssert(srcValidCountX <= TileSizeX_T);
        BeAssert(srcValidCountY <= TileSizeY_T);
        BeAssert(srcValidCountX > 0);
        BeAssert(srcValidCountY > 0);

        // Assuming that we must fill 1/4 of the tile
        const uint32_t dstWidth = TileSizeX_T/2;
        const uint32_t dstHeight = TileSizeY_T/2;

        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T* const bufOut = (Data_T*)pDest;

        const uint32_t evenSrcHeight = (srcValidCountY % 2) ? srcValidCountY-1 : srcValidCountY;
        const uint32_t evenSrcWidth  = (srcValidCountX % 2)  ? srcValidCountX-1 : srcValidCountX;

        uint32_t inY=0, outY=0;
        uint32_t outX=0, inX=0;
        Data_T*       pDestLine = NULL;
        Data_T const* pLine0 = NULL;
        Data_T const* pLine1 = NULL;

        // process an even number of lines
        for(inY=0, outY=0; inY < evenSrcHeight; inY+=2, ++outY)
            {
            pDestLine = bufOut + outY*(TileSizeX_T*ChannelCount_T);
            pLine0 = bufIn + inY*(TileSizeX_T*ChannelCount_T);
            pLine1 = bufIn + (inY + 1)*(TileSizeX_T*ChannelCount_T);
            // process an even number of columns
            for(inX=0, outX=0; inX < evenSrcWidth; inX+=2, ++outX)
                {
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    {
                    pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] +
                                                                                pLine0[(inX+1)*ChannelCount_T + channel] +
                                                                                pLine1[inX*ChannelCount_T + channel] + 
                                                                                pLine1[(inX+1)*ChannelCount_T + channel]) / 4);
                    }
                }

            // process the last pixel of the src line
            if (evenSrcWidth != srcValidCountX)
                {
                // Average last 2 pixels
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] + pLine1[inX*ChannelCount_T + channel]) / 2);

                outX++;
                inX +=2;
                }

            // Init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, (size_t)(sizeof(Data_T)*ChannelCount_T*(dstWidth-outX)));
            }

        // process the last valid src line
        if (evenSrcHeight != srcValidCountY)
            {
            pDestLine = bufOut + outY*(TileSizeX_T*ChannelCount_T);
            pLine0 = bufIn + inY*(TileSizeX_T*ChannelCount_T);
            pLine1 = bufIn + (inY + 1)*(TileSizeX_T*ChannelCount_T);

            // average last line
            for(inX=0, outX=0; inX < evenSrcWidth; inX+=2, ++outX)
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = ((Data_T)pLine0[inX*ChannelCount_T + channel] + 
                                                                        pLine0[(inX+1)*ChannelCount_T + channel])/2;
            // last pixel
            if (evenSrcWidth != srcValidCountX)
                {
                // copy last pixel
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = pLine0[inX*ChannelCount_T + channel];

                outX++;
                inX +=2;
                }

            // Init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*(dstWidth-outX));
            
            inY+=2;
            ++outY;
            }

        // init the remaining lines to default value
        for(; outY < dstHeight; ++outY)
            memset((bufOut + outY*(TileSizeX_T*ChannelCount_T)), 0, sizeof(Data_T)*ChannelCount_T*(dstWidth));
        }
    };

/*---------------------------------------------------------------------------------**//**
* NearestDownSampler_T
* DownSample any image block by a factor of 2
+---------------+---------------+---------------+---------------+---------------+------*/
template <uint32_t TileSizeX_T, uint32_t TileSizeY_T, uint32_t ChannelCount_T, typename Data_T>
struct NearestDownSampler_T : public OptimizedDownSampler
    {
    NearestDownSampler_T(){}
    virtual ~NearestDownSampler_T(){}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _Compute(Byte* pDest, Byte const* pSrc) const override
        {
        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T*       bufOut = (Data_T*)pDest;

        for(uint64_t line=0; line < TileSizeY_T/2; ++line)
            {
            Data_T* pDestLine = bufOut + line*(TileSizeX_T*ChannelCount_T);
            Data_T const* pLine0 = bufIn + (line*2)*(TileSizeX_T*ChannelCount_T);

            for(uint64_t column=0; column < TileSizeX_T/2; ++column)
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[column*ChannelCount_T + channel] = (Data_T)(pLine0[(column*2)*ChannelCount_T + channel]);
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _ComputeBound(Byte* pDest, Byte const* pSrc, uint32_t srcValidCountX, uint32_t srcValidCountY) const override
        {
        BeAssert(srcValidCountX <= TileSizeX_T);
        BeAssert(srcValidCountY <= TileSizeY_T);
        BeAssert(srcValidCountX > 0);
        BeAssert(srcValidCountY > 0);

        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T* const bufOut = (Data_T*)pDest;

        uint32_t inY=0, outY=0;

        // process lines
        for(inY=0, outY=0; inY < srcValidCountY; inY+=2, ++outY)
            {
            Data_T*      pDestLine = bufOut + outY*TileSizeX_T*ChannelCount_T;
            Data_T const*pLine0    = bufIn + inY*TileSizeX_T*ChannelCount_T;
            uint32_t outX=0, inX=0;

            // process columns
            for(inX=0, outX=0; inX < srcValidCountX; inX+=2, ++outX)
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = (Data_T)(pLine0[inX*ChannelCount_T + channel]);

            // Init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*((TileSizeX_T/2)-outX));
            }

        // init the remaining lines to default value
        for(; outY < TileSizeY_T/2; ++outY)
            memset((bufOut + outY*TileSizeX_T*ChannelCount_T), 0, sizeof(Data_T)*ChannelCount_T*(TileSizeX_T/2));
        }
    };

/*---------------------------------------------------------------------------------**//**
* GenericDownSampler
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDownSampler
    {
    GenericDownSampler(){}
    virtual ~GenericDownSampler(){}

    virtual void _Compute      (uint32_t dstTileWidth, uint32_t dstTileHeight, Byte* pDest, uint32_t srcTileWidth, uint32_t srcWidth, uint32_t srcHeight, Byte const* pSrc) const = 0;
    virtual void _ComputeNoData(uint32_t dstTileWidth, uint32_t dstTileHeight, Byte* pDest, uint32_t srcTileWidth, uint32_t srcWidth, uint32_t srcHeight, Byte const* pSrc, Byte const* noDataValue) const = 0;
};


/*----------------------------------------------------------------------------+
* AverageGenericDownSampler_T
* DownSample any image block by a factor of 2
* @param[in] dstTileWidth   Destination tile size
* @param[in] dstTileHeight  Destination tile size
* @param[in] srcTileWidth   Source tile size
* @param[in] srcWidth       Source valid tile size
* @param[in] srcHeight      Source valid tile size
* @bsimethod 
+----------------------------------------------------------------------------*/
template <uint32_t ChannelCount_T, typename Data_T>
struct AverageGenericDownSampler_T : public GenericDownSampler
    {
    typedef typename SumType_T<Data_T>::_SumType _SumType;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _Compute(uint32_t dstTileWidth, uint32_t dstTileHeight, Byte* pDest, uint32_t srcTileWidth, uint32_t srcWidth, uint32_t srcHeight, Byte const* pSrc) const override
        {
        BeAssert(((srcWidth+1)  >> 1) <= dstTileWidth);
        BeAssert(((srcHeight+1) >> 1) <= dstTileHeight);

        // Pitch is in Data_T, not in bytes
        const size_t srcPitch = srcTileWidth*ChannelCount_T;
        const size_t dstPitch = dstTileWidth*ChannelCount_T;

        // Assuming that we must fill 1/4 of the tile
        const uint32_t dstWidth = dstTileWidth/2;
        const uint32_t dstHeight = dstTileHeight/2;

        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T* const bufOut = (Data_T*)pDest;

        const uint32_t evenSrcHeight = (srcHeight % 2) ? srcHeight-1 : srcHeight;
        const uint32_t evenSrcWidth  = (srcWidth % 2)  ? srcWidth-1 : srcWidth;

        uint32_t inY=0, outY=0;
        uint32_t outX=0, inX=0;
        Data_T*       pDestLine = NULL;
        Data_T const* pLine0 = NULL;
        Data_T const* pLine1 = NULL;

        // process an even number of lines
        for(inY=0, outY=0; inY < evenSrcHeight; inY+=2, ++outY)
            {
            pDestLine = bufOut + outY*dstPitch;
            pLine0    = bufIn + inY*srcPitch;
            pLine1    = bufIn + (inY+1)*srcPitch;
            
            // process an even number of columns
            for(inX=0, outX=0; inX < evenSrcWidth; inX+=2, ++outX)
                {
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    {
                    pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] + 
                                                                                pLine0[(inX+1)*ChannelCount_T + channel] +
                                                                                pLine1[inX*ChannelCount_T + channel] + 
                                                                                pLine1[(inX+1)*ChannelCount_T + channel]) / 4);
                    }
                }

            // process the last pixel of the src line
            if (evenSrcWidth != srcWidth)
                {
                // Average last 2 pixels
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] + pLine1[inX*ChannelCount_T + channel]) / 2);

                outX++;
                inX +=2;
                }

            // Init the remaining part of the line to a default value
            BeAssert(dstWidth >= outX);
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*(dstWidth-outX));
            }

        // process the last valid src line
        if (evenSrcHeight != srcHeight)
            {
            pDestLine = bufOut + outY*dstPitch;
            pLine0    = bufIn + inY*srcPitch;
            pLine1    = bufIn + (inY+1)*srcPitch;

            // average last line
            for(inX=0, outX=0; inX < evenSrcWidth; inX+=2, ++outX)
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = ((Data_T)pLine0[inX*ChannelCount_T + channel] + 
                                                                        pLine0[(inX+1)*ChannelCount_T + channel])/2;
            // last pixel
            if (evenSrcWidth != srcWidth)
                {
                // copy last pixel
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = pLine0[inX*ChannelCount_T + channel];

                outX++;
                inX +=2;
                }

            // Init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*(dstWidth-outX));
            
            inY+=2;
            ++outY;
            }

        // init the remaining lines to default value
        for(; outY < dstHeight; ++outY)
            memset((bufOut + outY*dstPitch), 0, sizeof(Data_T)*ChannelCount_T*(dstWidth));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ComputeNoData(uint32_t dstTileWidth, uint32_t dstTileHeight, Byte* pDest, uint32_t srcTileWidth, uint32_t srcWidth, uint32_t srcHeight, Byte const* pSrc, Byte const* noDataValue) const override
        {
        BeAssert(((srcWidth+1)  >> 1) <= dstTileWidth);
        BeAssert(((srcHeight+1) >> 1) <= dstTileHeight);

        // Pitch is in Data_T, not in bytes
        const size_t srcPitch = srcTileWidth*ChannelCount_T;
        const size_t dstPitch = dstTileWidth*ChannelCount_T;

        // Assuming that we must fill 1/4 of the tile
        const uint32_t dstWidth = dstTileWidth/2;
        const uint32_t dstHeight = dstTileHeight/2;

        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T* const bufOut = (Data_T*)pDest;

        const uint32_t evenSrcHeight = (srcHeight % 2) ? srcHeight-1 : srcHeight;
        const uint32_t evenSrcWidth  = (srcWidth % 2)  ? srcWidth-1 : srcWidth;

        uint32_t inY=0, outY=0;
        uint32_t outX=0, inX=0;
        Data_T*       pDestLine = NULL;
        Data_T const* pLine0 = NULL;
        Data_T const* pLine1 = NULL;

        Data_T const* noData = (Data_T const*)noDataValue;

        // process an even number of lines
        for(inY=0, outY=0; inY < evenSrcHeight; inY+=2, ++outY)
            {
            pDestLine = bufOut + outY*dstPitch;
            pLine0    = bufIn + inY*srcPitch;
            pLine1    = bufIn + (inY+1)*srcPitch;

            // process an even number of columns
            for(inX=0, outX=0; inX < evenSrcWidth; inX+=2, ++outX)
                {
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    {
                    if(pLine0[inX*ChannelCount_T + channel]     == noData[channel] ||
                        pLine0[(inX+1)*ChannelCount_T + channel] == noData[channel] ||
                        pLine1[inX*ChannelCount_T + channel]     == noData[channel] ||
                        pLine1[(inX+1)*ChannelCount_T + channel] == noData[channel] )
                        {
                        _SumType  sum = 0;
                        uint32_t count = 0;

                        if(pLine0[inX*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine0[inX*ChannelCount_T + channel];
                            ++count;
                            }
                        if(pLine0[(inX+1)*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine0[(inX+1)*ChannelCount_T + channel];
                            ++count;
                            }
                        if(pLine1[inX*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine1[inX*ChannelCount_T + channel];
                            ++count;
                            }
                        if(pLine1[(inX+1)*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine1[(inX+1)*ChannelCount_T + channel];
                            ++count;
                            }

                        if(count == 0)
                            pDestLine[outX*ChannelCount_T + channel] = noData[channel];
                        else
                            pDestLine[outX*ChannelCount_T + channel] = (Data_T)(sum/count);
                        }
                    else
                        {
                        pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] +
                            pLine0[(inX+1)*ChannelCount_T + channel] +
                            pLine1[inX*ChannelCount_T + channel] + 
                            pLine1[(inX+1)*ChannelCount_T + channel]) / 4);
                        }
                    }
                }

            // process the last pixel of the src line
            if (evenSrcWidth != srcWidth)
                {
                // Average last 2 pixels
                for(uint64_t channel=0; channel < ChannelCount_T; ++channel)
                    {
                    if(pLine0[inX*ChannelCount_T + channel]     == noData[channel] ||
                        pLine1[inX*ChannelCount_T + channel]     == noData[channel])
                        {
                        _SumType  sum = 0;
                        uint32_t count = 0;

                        if(pLine0[inX*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine0[inX*ChannelCount_T + channel];
                            ++count;
                            }

                        if(pLine1[inX*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine1[inX*ChannelCount_T + channel];
                            ++count;
                            }

                        if(count == 0)
                            pDestLine[outX*ChannelCount_T + channel] = noData[channel];
                        else
                            pDestLine[outX*ChannelCount_T + channel] = (Data_T)(sum/count);
                        }
                    else
                        {
                        pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] +
                                                                                    pLine1[inX*ChannelCount_T + channel] ) / 2);
                        }
                    }

                outX++;
                inX +=2;
                }

            // Init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*(dstWidth-outX));
            }

        // process the last valid src line
        if (evenSrcHeight != srcHeight)
            {
            pDestLine = bufOut + outY*dstPitch;
            pLine0    = bufIn + inY*srcPitch;
            pLine1    = bufIn + (inY+1)*srcPitch;

            // average last line
            for(inX=0, outX=0; inX < evenSrcWidth; inX+=2, ++outX)
                {
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    {
                    if(pLine0[inX*ChannelCount_T + channel]     == noData[channel] ||
                        pLine0[(inX+1)*ChannelCount_T + channel] == noData[channel])
                        {
                        _SumType  sum = 0;
                        uint32_t count = 0;

                        if(pLine0[inX*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine0[inX*ChannelCount_T + channel];
                            ++count;
                            }
                        if(pLine0[(inX+1)*ChannelCount_T + channel] != noData[channel])
                            {
                            sum+=pLine0[(inX+1)*ChannelCount_T + channel];
                            ++count;
                            }

                        if(count == 0)
                            pDestLine[outX*ChannelCount_T + channel] = noData[channel];
                        else
                            pDestLine[outX*ChannelCount_T + channel] = (Data_T)(sum/count);
                        }
                    else
                        {
                        pDestLine[outX*ChannelCount_T + channel] = (Data_T)(((_SumType)pLine0[inX*ChannelCount_T + channel] +
                            pLine0[(inX+1)*ChannelCount_T + channel]) / 2);
                        }
                    }
                }

            // last pixel
            if (evenSrcWidth != srcWidth)
                {
                // copy last pixel
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = pLine0[inX*ChannelCount_T + channel];

                outX++;
                inX +=2;
                }

            // Init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*(dstWidth-outX));

            inY+=2;
            ++outY;
            }

        // init the remaining lines to default value
        for(; outY < dstHeight; ++outY)
            memset((bufOut + outY*dstPitch), 0, sizeof(Data_T)*ChannelCount_T*(dstWidth));
        }
    };

/*----------------------------------------------------------------------------+
* GenericNearestDownSampler_T
  DownSample any image block by a factor of 2
* @param[in] dstTileWidth   Destination tile size
* @param[in] dstTileHeight  Destination tile size
* @param[in] srcTileWidth   Source tile size
* @param[in] srcWidth       Source valid tile size
* @param[in] srcHeight      Source valid tile size
* @bsimethod 
+----------------------------------------------------------------------------*/
template <uint32_t ChannelCount_T, typename Data_T>
struct NearestGenericDownSampler_T : public GenericDownSampler
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _Compute(uint32_t dstTileWidth, uint32_t dstTileHeight, Byte* pDest, uint32_t srcTileWidth, uint32_t srcWidth, uint32_t srcHeight, Byte const* pSrc) const override
        {
        BeAssert(((srcWidth+1)  >> 1) <= dstTileWidth);
        BeAssert(((srcHeight+1) >> 1) <= dstTileHeight);

        // Pitch is in Data_T, not in bytes
        const size_t srcPitch = srcTileWidth*ChannelCount_T;
        const size_t dstPitch = dstTileWidth*ChannelCount_T;

        // assuming that we must fill 1/4 of the tile
        const uint32_t dstWidth = dstTileWidth/2;
        const uint32_t dstHeight = dstTileHeight/2;

        Data_T const* bufIn  = (Data_T const*)pSrc;
        Data_T* const bufOut = (Data_T*)pDest;

        uint32_t inY=0, outY=0;
        uint32_t outX=0, inX=0;

        // process each lines
        for(inY=0, outY=0; inY < srcHeight; inY+=2, ++outY)
            {
            Data_T* pDestLine = bufOut + outY*dstPitch;
            Data_T const* pLine0    = bufIn + inY*srcPitch;
            // process each columns
            for (inX = 0, outX = 0; inX < srcWidth; inX += 2, ++outX)
                for(uint32_t channel=0; channel < ChannelCount_T; ++channel)
                    pDestLine[outX*ChannelCount_T + channel] = pLine0[inX*ChannelCount_T + channel];

            // init the remaining part of the line to a default value
            memset(&pDestLine[outX*ChannelCount_T], 0, sizeof(Data_T)*ChannelCount_T*(dstWidth-outX));
            }

        // init the remaining lines to default value
        for(; outY < dstHeight; ++outY)
            memset((bufOut + outY*dstPitch), 0, sizeof(Data_T)*ChannelCount_T*(dstWidth));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Stephane.Poulin                 04/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ComputeNoData(uint32_t dstTileWidth, uint32_t dstTileHeight, Byte* pDest, uint32_t srcTileWidth, uint32_t srcWidth, uint32_t srcHeight, Byte const* pSrc, Byte const* noDataValue) const override
        {
        _Compute(dstTileWidth, dstTileHeight, pDest, srcTileWidth, srcWidth, srcHeight, pSrc);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <uint32_t ChannelCount_T, typename Data_T>
void NearestStretch_N(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch, uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch, uint32_t n)
    {
    BeAssert(n > 0);
    const uint64_t outXEnd = min(outWidth, (inWidth + n - 1) / n);
    const uint64_t outYEnd = min(outHeight, (inHeight + n - 1) / n);
    const uint64_t outXLast = outXEnd-1;
    const uint64_t outYLast = outYEnd-1;

    uint64_t inY, outY;
    uint64_t outX, inX;

    // process each lines
    for (inY = 0, outY = 0; outY < outYEnd; inY += n, ++outY)
        {
        BeAssert(outY < inHeight);
        Data_T* pDestLine = (Data_T*)(pOutData + outY*outPitch);
        Data_T const* pLine = (Data_T const*)(pInData + inY*inPitch);
        // process each columns
        for (inX = 0, outX = 0; outX < outXEnd; inX += n, ++outX)
            {
            BeAssert(inX < inWidth);
            for (uint64_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                pDestLine[outX*ChannelCount_T + channel] = pLine[inX*ChannelCount_T + channel];
                }
            }

        // Process remainder X
        memcpy(&pDestLine[outX*ChannelCount_T], &pDestLine[outXLast*ChannelCount_T], (size_t)(outWidth - outXEnd)*ChannelCount_T*sizeof(Data_T));
        }

    // Process Y remainder
    for (; outY < outHeight; ++outY)
        {
        Data_T* pDestLine = (Data_T*)(pOutData + outY*outPitch);
        Data_T const* pValidDestLine = (Data_T const*)(pOutData + outYLast*outPitch);
        memcpy(pDestLine, pValidDestLine, outWidth*ChannelCount_T*sizeof(Data_T));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <uint32_t ChannelCount_T, typename Data_T>
void AverageStretch_N(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch, uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch, uint32_t n)
    {
    BeAssert(n > 0);
    const typename SumType_T<Data_T>::_SumType windowSize = (typename SumType_T<Data_T>::_SumType)(n*n);

    const uint64_t srcLastX = inWidth - 1;
    const uint64_t srcLastY = inHeight - 1;

    const uint64_t outXEnd = min(outWidth, inWidth / n);
    const uint64_t outYEnd = min(outHeight, inHeight / n);

    Data_T* pDestLine;
    typename SumType_T<Data_T>::_SumType sum;

    uint64_t inY = 0, outY = 0;
    uint64_t inX = 0, outX = 0;

    // process each lines
    for (inY = 0, outY = 0; outY < outYEnd; inY += n, ++outY)
        {
        pDestLine = (Data_T*)(pOutData + outY*outPitch);
        // process each columns
        for (inX = 0, outX = 0; outX < outXEnd; inX += n, ++outX)
            {
            for (uint64_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                sum = 0;
                for (uint64_t line = 0; line < n; ++line)
                    {
                    BeAssert((inY + line) < inHeight);
                    Data_T const* pSrcLine = (Data_T*)(pInData + (inY + line)*inPitch);
                    for (uint64_t col = 0; col < n; ++col)
                        sum += pSrcLine[(inX + col)*ChannelCount_T + channel];
                    }
                pDestLine[outX*ChannelCount_T + channel] = (Data_T)(sum / windowSize);
                }
            }

        // Process remainder X
        for (; outX < outWidth; inX += n, ++outX)
            {
            for (uint64_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                sum = 0;
                for (uint64_t line = 0; line < n; ++line)
                    {
                    BeAssert((inY + line) < inHeight);
                    Data_T const* pSrcLine = (Data_T*)(pInData + (inY + line)*inPitch);
                    for (uint64_t col = 0; col < n; ++col)
                        sum += pSrcLine[(min(inX + col, srcLastX))*ChannelCount_T + channel];
                    }
                pDestLine[outX*ChannelCount_T + channel] = (Data_T)(sum / windowSize);
                }
            }
        }

    // Process Y remainder. In this loop we may not have enough pixels in the source 
    // to generate the destination. In that case, we use the last pixels.
    for (; outY < outHeight; inY += n, ++outY)
        {
        pDestLine = (Data_T*)(pOutData + outY*outPitch);
        // process each columns
        for (outX = 0; outX < outWidth; inX += n, ++outX)
            {
            for (uint64_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                sum = 0;
                for (uint64_t line = 0; line < n; ++line)
                    {
                    Data_T const* pSrcLine = (Data_T*)(pInData + (min(inY + line, srcLastY))*inPitch);
                    for (uint64_t col = 0; col < n; ++col)
                        sum += pSrcLine[(min(inX + col, srcLastX))*ChannelCount_T + channel];
                    }
                pDestLine[outX*ChannelCount_T + channel] = (Data_T)(sum / windowSize);
                }
            }
        }
    }

typedef void(*Stretch_1N_FunctionP)(uint32_t outWidth, uint32_t outHeight, Byte* pOutData, size_t outPitch, uint32_t inWidth, uint32_t inHeight, Byte const* pInData, size_t inPitch, uint32_t n);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
OptimizedDownSampler* CreateOptimizedDownSampler(uint64_t tileSizeX, uint64_t tileSizeY, ImagePP::HRPPixelType const& pixelType, ImagePP::HGSResampling const& method);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
GenericDownSampler* CreateGenericDownSampler(ImagePP::HRPPixelType const& pixelType, ImagePP::HGSResampling const& method);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Stretch_1N_FunctionP CreateGenericStretcherN(ImagePP::HRPPixelType const& pixelType, ImagePP::HGSResampling const& method);
