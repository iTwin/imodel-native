//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPConvFilterV24R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPConvFilterV24R8G8B8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPConvFilterV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPConvFilterV24R8G8B8::HRPConvFilterV24R8G8B8()
    : HRPConvFilter(new HRPPixelTypeV24R8G8B8())
    {
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRPConvFilterV24R8G8B8::HRPConvFilterV24R8G8B8(const HRPPixelNeighbourhood& pi_rNeighbourhood,
                                               const int32_t* pi_pWeightMatrix,
                                               int32_t pi_DivisionFactor) :
    HRPConvFilter(new HRPPixelTypeV24R8G8B8(),
                  pi_rNeighbourhood,
                  pi_pWeightMatrix,
                  pi_DivisionFactor)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPConvFilterV24R8G8B8::HRPConvFilterV24R8G8B8(
    const HRPConvFilterV24R8G8B8& pi_rFilter)
    : HRPConvFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRPConvFilterV24R8G8B8::~HRPConvFilterV24R8G8B8()
    {
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPConvFilterV24R8G8B8::Convert(  HRPPixelBuffer* pi_pInputBuffer,
                                       HRPPixelBuffer* pio_pOutputBuffer)
    {
    uint32_t Index;

    // alocate an array of pointers to premultiplied tables for each unique coefficient
    // in the weight table
    // also create a temporary line for the result before saturating
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    uint32_t SizeOfMatrix = rNeighbourhood.GetWidth() * rNeighbourhood.GetHeight();
    m_pPreMulTables = new double*[SizeOfMatrix];
    m_pAllocation = new bool[SizeOfMatrix];
    memset(m_pPreMulTables, 0, SizeOfMatrix * sizeof(double*));
    const int32_t* pMatrix = GetWeightMatrix();
    uint32_t TmpIndex;
    int32_t Weight;
    int32_t Division = GetDivisionFactor();
    double WeightDivided;

    for(Index = 0; Index < SizeOfMatrix; Index++)
        {
        if(m_pPreMulTables[Index] == 0)
            {
            Weight = pMatrix[Index];

            if(Weight != 0)
                {
                m_pPreMulTables[Index] = new double[256];
                m_pAllocation[Index] = true;

                WeightDivided = Weight / (double)Division;

                for(uint32_t I = 0; I < 256; I++)
                    m_pPreMulTables[Index][I] = (I * WeightDivided);

                for(TmpIndex = Index + 1; TmpIndex < SizeOfMatrix; TmpIndex++)
                    {
                    if(pMatrix[TmpIndex] == Weight)
                        {
                        m_pPreMulTables[TmpIndex] = m_pPreMulTables[Index];
                        m_pAllocation[TmpIndex] = false;
                        }
                    }
                }
            else
                {
                m_pAllocation[Index] = false;
                }
            }
        }

    // call the parent method
    HRPConvFilter::Convert(pi_pInputBuffer, pio_pOutputBuffer);

    // delete the buffers
    for(Index = 0; Index < SizeOfMatrix; Index++)
        if(m_pAllocation[Index] == true)
            delete m_pPreMulTables[Index];
    if(m_pPreMulTables != 0)
        delete m_pPreMulTables;
    if(m_pAllocation != 0)
        delete m_pAllocation;
    }

//-----------------------------------------------------------------------------
// private
// Convoluate
//
// Note : This filter dont support the pixel positions. We do nothing with
//        pi_pPositionsX and pi_pPositionsY parameters.
//-----------------------------------------------------------------------------
void HRPConvFilterV24R8G8B8::Convoluate(const void*     pi_pSrcRawData[],
                                        void*           po_pDestRawData,
                                        uint32_t        pi_Width,
                                        const double*  pi_pPositionsX,
                                        const double*  pi_pPositionsY) const
    {
    double* pResultLine;
    Byte*  pSrcRawData;
    double* pPreMulTable;
    uint32_t ElementY;
    uint32_t ElementX;
    uint32_t PixelChannel;
    uint32_t ElementXPtr;

    // get information about the width and height of the Neighbourhood
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    uint32_t NeighbourhoodHeight = rNeighbourhood.GetHeight();
    uint32_t NeighbourhoodWidth = rNeighbourhood.GetWidth();

    uint32_t PixelChannels = pi_Width * 3;

    double* pTempResultLine = new double[PixelChannels];

    // reset the line of temporary results
    memset(pTempResultLine, 0, PixelChannels * sizeof(double));

    uint32_t WeightIndex = 0;

    // for each element of the weight matrix, sum every channel values of the line
    // concerned (multiplied and divided in the lookup table).
    for(ElementY = 0; ElementY < NeighbourhoodHeight; ElementY++)
        {
        ElementXPtr = 0;

        for(ElementX = 0; ElementX < NeighbourhoodWidth; ElementX++)
            {
            // choose the appropriate premultiplied table for the coefficient
            pPreMulTable = m_pPreMulTables[WeightIndex];

            // if the coefficient is different than zero, we must do something
            if(pPreMulTable != 0)
                {
                // reset the pointer at the beginning of the result line
                pResultLine = pTempResultLine;

                // choose the line having the data appropriated for the element considered
                pSrcRawData = (Byte*)pi_pSrcRawData[ElementY] +
                              ElementXPtr;

                // sum each pixel channel value looked up in the table
                for(PixelChannel = 0; PixelChannel < PixelChannels; PixelChannel++)
                    {
                    *pResultLine += pPreMulTable[*pSrcRawData];

                    pResultLine++;
                    pSrcRawData++;
                    }
                }

            WeightIndex++;
            ElementXPtr += 3;
            }
        }


    // saturate the filtered results between 0 and 255 for each pixel channel
    // and store the results in the destination buffer
    pResultLine = pTempResultLine;
    Byte* pDestRawData = (Byte*) po_pDestRawData;
    for(PixelChannel = 0; PixelChannel < PixelChannels; PixelChannel++)
        {
        if(*pResultLine < 0)
            *pDestRawData = 0;
        else if(*pResultLine > 255)
            *pDestRawData = 255;
        else
            *pDestRawData = (Byte)*pResultLine;

        pDestRawData++;
        pResultLine++;
        }

    delete[] pTempResultLine;
    }
