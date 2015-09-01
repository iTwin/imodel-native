//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPFunctionFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPFunctionFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPFunctionFilter.h>

//-----------------------------------------------------------------------------
// protected
// Constructor.
//-----------------------------------------------------------------------------
HRPFunctionFilter::HRPFunctionFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :    HRPTypedFilter(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
// protected
// Copy constructor
//-----------------------------------------------------------------------------
HRPFunctionFilter::HRPFunctionFilter(const HRPFunctionFilter& pi_rFilter)
    : HRPTypedFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPFunctionFilter::~HRPFunctionFilter()
    {
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPFunctionFilter::Convert(  HRPPixelBuffer* pi_pInputBuffer,
                                  HRPPixelBuffer* pio_pOutputBuffer)
    {
    HPRECONDITION(GetInputPixelType().GetPtr() != NULL);
    HPRECONDITION(GetOutputPixelType().GetPtr() != NULL);

    Byte* pSrcRawData = (Byte*)pi_pInputBuffer->GetBufferPtr();
    Byte* pDestRawData = (Byte*)pio_pOutputBuffer->GetBufferPtr();
    const HFCPtr<HRPPixelConverter>& pInputConverter = GetInputConverter();
    const HFCPtr<HRPPixelConverter>& pOutputConverter = GetOutputConverter();
    Byte* pFilterSrcRawData;
    Byte* pFilterDestRawData;

    Byte* pFilterBuffer = NULL;

    // if there is a converter, create a temporay line input buffer
    if((pInputConverter != NULL) || (pOutputConverter != NULL))
        {
        pFilterBuffer = new Byte[pi_pInputBuffer->GetWidth() *
                                   (GetFilterPixelType()->CountPixelRawDataBits() / 8)];
        }

    // associate the input of the filter to the right buffer (real or temporary)
    if(pInputConverter != NULL)
        pFilterSrcRawData = pFilterBuffer;
    else
        pFilterSrcRawData = (Byte*)pi_pInputBuffer->GetBufferPtr();

    // associate the output of the buffer to the right buffer (real or temporary)
    if(pOutputConverter != NULL)
        pFilterDestRawData = pFilterBuffer;
    else
        pFilterDestRawData = (Byte*)pio_pOutputBuffer->GetBufferPtr();


    // process normal conversions without alpha or nondefined channels
    for(uint32_t Line = 0; Line < pi_pInputBuffer->GetHeight(); Line++)
        {
        // convert first the input raw data if required
        if(pInputConverter != NULL)
            {
            pInputConverter->Convert(   pSrcRawData,
                                        pFilterSrcRawData,
                                        pi_pInputBuffer->GetWidth());
            }

        // call the function
        Function(pFilterSrcRawData, pFilterDestRawData, pi_pInputBuffer->GetWidth());

        // convert the output to the real output pixel type if required
        if(pOutputConverter != NULL)
            {
            pOutputConverter->Convert(  pFilterDestRawData,
                                        pDestRawData,
                                        pi_pInputBuffer->GetWidth());

            // copy the lost channels if there are
            if(AreThereLostChannels())
                {
                GetInOutConverter()->ConvertLostChannel(   pSrcRawData,
                                                pDestRawData,
                                                pi_pInputBuffer->GetWidth(),
                                                GetLostChannelsMask()
                                            );
                }

            }

        // increment the pointers to the real buffer
        pSrcRawData += pi_pInputBuffer->GetBytesPerLine();
        pDestRawData += pio_pOutputBuffer->GetBytesPerLine();


        // update the filter pointers
        if(pInputConverter == NULL)
            pFilterSrcRawData = pSrcRawData;
        if(pOutputConverter == NULL)
            pFilterDestRawData = pDestRawData;
        }

    // delete the temporary buffer if created
    if(pFilterBuffer != NULL)
        delete pFilterBuffer;
    }
