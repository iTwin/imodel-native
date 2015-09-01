//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPConvFilterV24PhotoYCC.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPConvFilterV24PhotoYCC
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPConvFilterV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPConvFilterV24PhotoYCC::HRPConvFilterV24PhotoYCC()
    : HRPConvFilter(new HRPPixelTypeV24PhotoYCC())
    {
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRPConvFilterV24PhotoYCC::HRPConvFilterV24PhotoYCC(const HRPPixelNeighbourhood& pi_rNeighbourhood,
                                                   const int32_t* pi_pWeightMatrix,
                                                   int32_t pi_DivisionFactor) :
    HRPConvFilter(new HRPPixelTypeV24PhotoYCC(),
                  pi_rNeighbourhood,
                  pi_pWeightMatrix,
                  pi_DivisionFactor)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPConvFilterV24PhotoYCC::HRPConvFilterV24PhotoYCC(
    const HRPConvFilterV24PhotoYCC& pi_rFilter)
    : HRPConvFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRPConvFilterV24PhotoYCC::~HRPConvFilterV24PhotoYCC()
    {
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPConvFilterV24PhotoYCC::Convert(  HRPPixelBuffer* pi_pInputBuffer,
                                         HRPPixelBuffer* pio_pOutputBuffer)
    {
    uint32_t Index;

    // alocate an array of pointers to premultiplied tables for each unique coefficient
    // in the weight table
    // also create a temporary line for the result before saturating
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    uint32_t SizeOfMatrix = rNeighbourhood.GetWidth() * rNeighbourhood.GetHeight();
    m_pResultLine = new double[pi_pInputBuffer->GetWidth()];
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
    if(m_pResultLine != 0)
        delete m_pResultLine;
    }

//-----------------------------------------------------------------------------
// private
// Convoluate
//
// Note : This filter dont support the pixel positions. We do nothing with
//        pi_pPositionsX and pi_pPositionsY parameters.
//-----------------------------------------------------------------------------
void HRPConvFilterV24PhotoYCC::Convoluate(const void*   pi_pSrcRawData[],
                                          void*          po_pDestRawData,
                                          uint32_t       pi_Width,
                                          const double* pi_pPositionsX,
                                          const double* pi_pPositionsY) const
    {
    double* pResultLine;
    Byte*  pSrcRawData;
    double* pPreMulTable;
    uint32_t ElementY;
    uint32_t ElementX;
    uint32_t Pixel;
    uint32_t ElementXPtr;

    // get information about the width and height of the Neighbourhood
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    uint32_t NeighbourhoodHeight = rNeighbourhood.GetHeight();
    uint32_t NeighbourhoodWidth = rNeighbourhood.GetWidth();

    uint32_t Pixels = pi_Width;

    // reset the line of temporary results
    memset(m_pResultLine, 0, Pixels * sizeof(int32_t));

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
                pResultLine = m_pResultLine;

                // choose the line having the data appropriated for the element considered
                pSrcRawData = (Byte*)pi_pSrcRawData[ElementY] +
                              ElementXPtr;

                // sum each pixel channel value looked up in the table
                for(Pixel = 0; Pixel < Pixels; Pixel++)
                    {
                    *pResultLine += pPreMulTable[*pSrcRawData];

                    pResultLine++;
                    pSrcRawData+=3;
                    }
                }

            WeightIndex++;
            ElementXPtr += 3;
            }
        }


    // copy the original line
    Byte* pDestRawData = (Byte*) po_pDestRawData;

    memcpy(pDestRawData, ((Byte*)(pi_pSrcRawData[rNeighbourhood.GetYOrigin()])
                          + rNeighbourhood.GetXOrigin() * 3), Pixels * 3);

    // saturate the filtered results between 0 and 255 for each pixel channel
    // and store the results in the destination buffer
    pResultLine = m_pResultLine;
    for(Pixel = 0; Pixel < Pixels; Pixel++)
        {
        if(*pResultLine < 0)
            *pDestRawData = 0;
        else if(*pResultLine > 255)
            *pDestRawData = 255;
        else
            *pDestRawData = (Byte)*pResultLine;

        pDestRawData+=3;
        pResultLine++;
        }
    }
