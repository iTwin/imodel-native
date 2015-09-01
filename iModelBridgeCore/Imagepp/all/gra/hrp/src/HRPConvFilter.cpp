//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPConvFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPConvFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPConvFilter.h>

//-----------------------------------------------------------------------------
// protected
// Constructor.
//-----------------------------------------------------------------------------
HRPConvFilter::HRPConvFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    : HRPTypedFilter(pi_pFilterPixelType)
    {
    m_WeightMatrixSize = 0;
    m_pWeightMatrix = 0;

    // optimization
    m_FilterBytesPerPixel = GetFilterPixelType()->CountPixelRawDataBits() / 8;
    }

//-----------------------------------------------------------------------------
// protected
// Constructor.
//-----------------------------------------------------------------------------
HRPConvFilter::HRPConvFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType,
                             const HRPPixelNeighbourhood& pi_rNeighbourhood,
                             const int32_t* pi_pWeightMatrix,
                             int32_t pi_DivisionFactor)
    : HRPTypedFilter(pi_pFilterPixelType, pi_rNeighbourhood),
      m_DivisionFactor(pi_DivisionFactor)
    {
    if (pi_pWeightMatrix != 0)
        {
        // allocate memory for the Weight matrix
        m_WeightMatrixSize = pi_rNeighbourhood.GetWidth() *
                             pi_rNeighbourhood.GetHeight();
        m_pWeightMatrix = new int32_t[m_WeightMatrixSize];

        HASSERT(m_pWeightMatrix != 0);

        if (pi_pWeightMatrix != 0)
            SetWeightMatrix(pi_pWeightMatrix, pi_DivisionFactor);
        }
    else
        {
        m_WeightMatrixSize = 0;
        }

    // optimization
    m_FilterBytesPerPixel = GetFilterPixelType()->CountPixelRawDataBits() / 8;
    }

//-----------------------------------------------------------------------------
// protected
// Copy constructor.
//-----------------------------------------------------------------------------
HRPConvFilter::HRPConvFilter(const HRPConvFilter& pi_rFilter)
    : HRPTypedFilter(pi_rFilter)
    {
    m_WeightMatrixSize = pi_rFilter.m_WeightMatrixSize;

    if (m_WeightMatrixSize > 0)
        {
        // copy the weight matrix
        m_pWeightMatrix = new int32_t[m_WeightMatrixSize];
        HASSERT(m_pWeightMatrix != 0);
        const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
        for(uint32_t Y = 0; Y < rNeighbourhood.GetHeight(); Y++)
            for(uint32_t X = 0; X < rNeighbourhood.GetWidth(); X++)
                m_pWeightMatrix[Y * rNeighbourhood.GetWidth() + X] =
                    pi_rFilter.m_pWeightMatrix[Y * rNeighbourhood.GetWidth() + X];
        }
    m_DivisionFactor = pi_rFilter.m_DivisionFactor;

    // optimization
    m_FilterBytesPerPixel = GetFilterPixelType()->CountPixelRawDataBits() / 8;

    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPConvFilter::~HRPConvFilter()
    {

    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPConvFilter::Convert(const void*     pi_pInputData[],
                            void*           po_pOutputData,
                            const double*  pi_pPosX,
                            const double*  pi_pPosY)
    {
    HPRECONDITION(pi_pInputData != 0);
    HPRECONDITION(po_pOutputData != 0);
    HPRECONDITION(pi_pPosX != 0);
    HPRECONDITION(pi_pPosY != 0);

    // convertors
    const HFCPtr<HRPPixelConverter>& pInputConverter = GetInputConverter();
    const HFCPtr<HRPPixelConverter>& pOutputConverter = GetOutputConverter();

    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    HArrayAutoPtr<Byte*> pSrcLinesBuffer;
    HArrayAutoPtr<Byte>  pFilterSrcBuffer;
    Byte** pSrcLinesPtr;

    // convert the input data in necessary
    if (pInputConverter != 0)
        {

        uint32_t FilterBytesPerLine = rNeighbourhood.GetWidth() * m_FilterBytesPerPixel;
        pFilterSrcBuffer = new Byte[FilterBytesPerLine * rNeighbourhood.GetHeight()];
        pSrcLinesBuffer = new Byte*[rNeighbourhood.GetHeight()];

        Byte* pTmpLinePtr = pFilterSrcBuffer;
        for(uint32_t j = 0; j < rNeighbourhood.GetHeight(); j++)
            {
            pSrcLinesBuffer[j] = pTmpLinePtr;
            pInputConverter->Convert(pi_pInputData[j],
                                     pSrcLinesBuffer[j],
                                     rNeighbourhood.GetWidth());
            pTmpLinePtr += FilterBytesPerLine;
            }
        pSrcLinesPtr = (Byte**)pSrcLinesBuffer.get();
        }
    else
        pSrcLinesPtr = (Byte**)pi_pInputData;

    HArrayAutoPtr<Byte> pDestRawDataBuffer;
    Byte* pDestRawDataPtr;
    // if we have an output converter, create a tempory buffer for the convolution
    if (pOutputConverter != 0)
        {
        pDestRawDataBuffer = new Byte[m_FilterBytesPerPixel];
        pDestRawDataPtr = pDestRawDataBuffer;
        }
    else
        pDestRawDataPtr = (Byte*)po_pOutputData;

    // now, we can apply the convolution
    Convoluate((const void**)pSrcLinesPtr,
               pDestRawDataPtr,
               1,
               pi_pPosX,
               pi_pPosY);

    // if there is an output converter, we must convert the filtered values
    if(pOutputConverter != 0)
        {
        pOutputConverter->Convert(pDestRawDataPtr,
                                  po_pOutputData,
                                  1);

        // copy the lost channels if there are
        if(AreThereLostChannels())
            {
            GetInOutConverter()->ConvertLostChannel(pSrcLinesPtr[rNeighbourhood.GetYOrigin()],
                                         pDestRawDataPtr,
                                         1,
                                         GetLostChannelsMask());
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPConvFilter::Convert(HRPPixelBuffer* pi_pInputBuffer,
                            HRPPixelBuffer* pio_pOutputBuffer)
    {
    HPRECONDITION(GetInputPixelType().GetPtr() != 0);
    HPRECONDITION(GetOutputPixelType().GetPtr() != 0);

    // convertors
    const HFCPtr<HRPPixelConverter>& pInputConverter = GetInputConverter();
    const HFCPtr<HRPPixelConverter>& pOutputConverter = GetOutputConverter();

    // source buffer (buffer filled by the input HRPPixelBuffer object)
    HArrayAutoPtr<Byte> pSrcRawData;
    // intermediate source buffer (when there is an input convertor)
    HArrayAutoPtr<Byte> pFilterSrcBuffer;
    // intermediate destination buffer (when there is an output converter)
    HArrayAutoPtr<Byte> pFilterDestBuffer;
    // final destination of the filtered data (HRPPixelBuffer output object)
    Byte* pDestRawData = (Byte*)pio_pOutputBuffer->GetBufferPtr();

    // general pointer indicating the source and destination buffer before calling
    // the Convoluate method
    Byte** pLinesPtr;
    Byte* pFilterDestRawData;

    // Neighbourhood information
    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    uint32_t LastLineFromOrigin = (rNeighbourhood.GetHeight() - 1) -
                                rNeighbourhood.GetYOrigin();

    // general information
    uint32_t Width = (rNeighbourhood.GetWidth() - 1) + pi_pInputBuffer->GetWidth();

    // temporary variables
    Byte* pTmpLinePtr;


    // create & initialize the source buffer and associated parameters
    uint32_t SrcBytesPerLine = (uint32_t)ceil (Width * GetInputPixelType()->CountPixelRawDataBits() / 8.0);
    pSrcRawData = new Byte[SrcBytesPerLine * rNeighbourhood.GetHeight()];
    HArrayAutoPtr<Byte*> pSrcLinesPtr(new Byte*[rNeighbourhood.GetHeight()]);
    // the first iteration, will set the entry [0].
    pSrcLinesPtr[0] = pSrcRawData;
    pTmpLinePtr = pSrcRawData;
    int32_t Line = -1 * rNeighbourhood.GetYOrigin();
    for(uint32_t j=1; j<rNeighbourhood.GetHeight(); j++)
        {
        pTmpLinePtr += SrcBytesPerLine;
        pSrcLinesPtr[j] = pTmpLinePtr;
        pi_pInputBuffer->GetLine(Line, pTmpLinePtr);
        Line++;
        }

    uint32_t FilterBytesPerPixel = (uint32_t)ceil (GetFilterPixelType()->CountPixelRawDataBits() / 8.0);
    HArrayAutoPtr<Byte*> pFilterSrcLinesPtr;

    // create & initialize the source filter buffer if required by a conversion
    if(pInputConverter != NULL)
        {
        uint32_t FilterBytesPerLine = Width * FilterBytesPerPixel;
        pFilterSrcBuffer = new Byte[FilterBytesPerLine * rNeighbourhood.GetHeight()];
        pFilterSrcLinesPtr = new Byte*[rNeighbourhood.GetHeight()];

        pFilterSrcLinesPtr[0] = pFilterSrcBuffer;
        pTmpLinePtr = pFilterSrcBuffer;
        for(uint32_t j = 0; j< rNeighbourhood.GetHeight(); j++)
            {
            pFilterSrcLinesPtr[j] = pTmpLinePtr;
            pInputConverter->Convert(   pSrcLinesPtr[j],
                                        pFilterSrcLinesPtr[j],
                                        Width);
            pTmpLinePtr += FilterBytesPerLine;
            }

        pLinesPtr = pFilterSrcLinesPtr;
        }
    else
        {
        pLinesPtr = pSrcLinesPtr;
        }

    // is there an output conversion requiring a temporary line buffer?
    if(pOutputConverter != NULL)
        {
        // yes, create one
        pFilterDestBuffer = new Byte[pi_pInputBuffer->GetWidth() *
                                       FilterBytesPerPixel];

        pFilterDestRawData = pFilterDestBuffer;
        }
    else
        pFilterDestRawData = pDestRawData;

    // filter all the lines in the buffer
    for(uint32_t Y = 0; Y < pi_pInputBuffer->GetHeight(); Y++)
        {
        // shift vertically the pointers to lines of the source buffer
        // and get the new line
        pTmpLinePtr = pSrcLinesPtr[0];
        uint32_t j;
        for(j=0; j < (rNeighbourhood.GetHeight() - 1); j++)
            pSrcLinesPtr[j] = pSrcLinesPtr[j+1];
        pSrcLinesPtr[j] = pTmpLinePtr;

        pi_pInputBuffer->GetLine(Y + LastLineFromOrigin, pSrcLinesPtr[j]);

        // update the temporary source filter pointers
        if((pInputConverter != NULL) && (Y < (pi_pInputBuffer->GetHeight() - 1)))
            {
            // shift vertically the pointer to lines of the source filter buffer
            pTmpLinePtr = pFilterSrcLinesPtr[0];
            for(j=0; j < (rNeighbourhood.GetHeight() - 1); j++)
                pFilterSrcLinesPtr[j] = pFilterSrcLinesPtr[j+1];
            pFilterSrcLinesPtr[j] = pTmpLinePtr;
            pInputConverter->Convert(pSrcLinesPtr[j], pFilterSrcLinesPtr[j], Width);
            }

        // do the convolution
        Convoluate((const void**)pLinesPtr,
                   pFilterDestRawData,
                   pi_pInputBuffer->GetWidth(),
                   pi_pInputBuffer->GetPositionsX(),
                   pi_pInputBuffer->GetPositionsY());

        // if there is an output converter, we must convert the filtered values
        if(pOutputConverter != NULL)
            {
            pOutputConverter->Convert(  pFilterDestRawData,
                                        pDestRawData,
                                        pi_pInputBuffer->GetWidth());

            // copy the lost channels if there are
            if(AreThereLostChannels())
                {
                GetInOutConverter()->ConvertLostChannel(   pSrcLinesPtr[rNeighbourhood.GetYOrigin()],
                                                pDestRawData,
                                                pi_pInputBuffer->GetWidth(),
                                                GetLostChannelsMask()
                                            );
                }
            }

        pDestRawData += pio_pOutputBuffer->GetBytesPerLine();

        if(pOutputConverter == NULL)
            pFilterDestRawData = pDestRawData;
        }
    }

//-----------------------------------------------------------------------------
// public
// ComposeWith
//-----------------------------------------------------------------------------
HRPFilter* HRPConvFilter::ComposeWith(const HRPFilter* pi_pFilter)
    {
    HPRECONDITION(pi_pFilter != 0);

    HRPFilter* pFilter;

    // verify if the parameter is a convolution filter
    if(IsAConvolutionFilter() == false ||
       !pi_pFilter->IsCompatibleWith(HRPConvFilter::CLASS_ID) ||
       ((HRPConvFilter*)pi_pFilter)->IsAConvolutionFilter() == false ||
       !((HRPTypedFilter*)pi_pFilter)->GetFilterPixelType()->HasSamePixelInterpretation(*GetFilterPixelType()))
        {
        // if not, call the parent method
        pFilter = HRPTypedFilter::ComposeWith(pi_pFilter);
        }
    else
        {
        HPRECONDITION(m_pWeightMatrix != 0);
        HASSERT(((HRPConvFilter*)pi_pFilter)->m_pWeightMatrix != 0);

        // compose the two filters in a single convolution filter

        // clone the current filter
        HRPConvFilter* pConvFilter = (HRPConvFilter*)Clone();

        // set the division factor
        pConvFilter->m_DivisionFactor = m_DivisionFactor * ((HRPConvFilter*)pi_pFilter)->m_DivisionFactor;

        // get the Neighbourhood1 (current object)
        const HRPPixelNeighbourhood& rNeighbourhood1 = GetNeighbourhood();
        // get the Neighbourhood2 (parameter)
        const HRPPixelNeighbourhood& rNeighbourhood2 = pi_pFilter->GetNeighbourhood();

        // set the neighbourhood and get it
        pConvFilter->SetNeighbourhood(rNeighbourhood1 + rNeighbourhood2);
        const HRPPixelNeighbourhood& rNeighbourhood = pConvFilter->GetNeighbourhood();

        // get the two Weight matrix
        const int32_t* pWeightMatrix1 = m_pWeightMatrix;
        const int32_t* pWeightMatrix2 = ((HRPConvFilter*)pi_pFilter)->m_pWeightMatrix;

        // allocate a new Weight matrix and set the elements to 0
        pConvFilter->m_WeightMatrixSize =  rNeighbourhood.GetWidth() *
                                           rNeighbourhood.GetHeight();
        pConvFilter->m_pWeightMatrix = new int32_t[pConvFilter->m_WeightMatrixSize];

        memset(pConvFilter->m_pWeightMatrix, 0, rNeighbourhood.GetWidth() *
               rNeighbourhood.GetHeight() * sizeof(int32_t));

        // convoluate the two filters
        for(uint32_t Y2 = 0; Y2 < rNeighbourhood2.GetHeight(); Y2++)
            for(uint32_t X2 = 0; X2 < rNeighbourhood2.GetWidth(); X2++)
                for(uint32_t Y1 = 0; Y1 < rNeighbourhood1.GetHeight(); Y1++)
                    for(uint32_t X1 = 0; X1 < rNeighbourhood1.GetWidth(); X1++)
                        pConvFilter->m_pWeightMatrix[
                            (Y1 + Y2) * rNeighbourhood.GetWidth() + (X1 + X2)] +=
                                pWeightMatrix2[Y2 * rNeighbourhood2.GetWidth() + X2] *
                                pWeightMatrix1[Y1 * rNeighbourhood1.GetWidth() + X1];


        pFilter = pConvFilter;

        uint32_t Divisor;

        // try to reduce the constants in the matrix
        // for that, we try to find a common divisor for all weights in the matrix
        for(Divisor = pConvFilter->m_DivisionFactor;  Divisor > 1; Divisor--)
            {
            if((pConvFilter->m_DivisionFactor % Divisor) != 0)
                break;

            for(uint32_t Y = 0; Y < rNeighbourhood.GetHeight(); Y++)
                for(uint32_t X = 0; X < rNeighbourhood.GetWidth(); X++)
                    if((pConvFilter->m_pWeightMatrix[Y * rNeighbourhood.GetWidth() + X]
                        % Divisor) != 0)
                        break;
            }

        // we found a common divisor; reduce the matrix
        if(Divisor == 1 && Divisor < pConvFilter->m_DivisionFactor)
            {
            Divisor++;

            pConvFilter->m_DivisionFactor /= Divisor;

            for(uint32_t Y = 0; Y < rNeighbourhood.GetHeight(); Y++)
                for(uint32_t X = 0; X < rNeighbourhood.GetWidth(); X++)
                    pConvFilter->m_pWeightMatrix[Y * rNeighbourhood.GetWidth() + X] /= Divisor;
            }

        }

    return pFilter;
    }

//-----------------------------------------------------------------------------
// protected
// SetWeightMatrix
//-----------------------------------------------------------------------------
void HRPConvFilter::SetWeightMatrix(const int32_t* pi_pWeightMatrix,
                                    int32_t pi_DivisionFactor)
    {
    HPRECONDITION(pi_pWeightMatrix != 0);

    const HRPPixelNeighbourhood& rNeighbourhood = GetNeighbourhood();
    if (m_pWeightMatrix == 0)
        {
        // allocate memory for the Weight matrix
        m_WeightMatrixSize = rNeighbourhood.GetWidth() * rNeighbourhood.GetHeight();
        m_pWeightMatrix = new int32_t[m_WeightMatrixSize];

        HASSERT(m_pWeightMatrix != 0);
        }


    // copy the Weight matrix parameter
    for(uint32_t Y = 0; Y < rNeighbourhood.GetHeight(); Y++)
        for(uint32_t X = 0; X < rNeighbourhood.GetWidth(); X++)
            m_pWeightMatrix[Y * rNeighbourhood.GetWidth() + X] =
                pi_pWeightMatrix[Y * rNeighbourhood.GetWidth() + X];

    m_DivisionFactor = pi_DivisionFactor;
    }
