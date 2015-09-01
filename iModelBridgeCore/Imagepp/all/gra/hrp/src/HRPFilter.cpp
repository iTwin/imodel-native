//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPFilter.h>

#include <Imagepp/all/h/HRPComplexFilter.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>

//-----------------------------------------------------------------------------
// protected
// Default constructor.
//-----------------------------------------------------------------------------
HRPFilter::HRPFilter() : m_Neighbourhood(1,1,0,0)
    {
    }

//-----------------------------------------------------------------------------
// protected
// Constructor.
//-----------------------------------------------------------------------------
HRPFilter::HRPFilter(const HRPPixelNeighbourhood& pi_rNeighbourhood) :
    m_Neighbourhood(pi_rNeighbourhood)
    {
    }

//-----------------------------------------------------------------------------
// protected
// Copy constructor
//-----------------------------------------------------------------------------
HRPFilter::HRPFilter(const HRPFilter& pi_rFilter) :
    m_Neighbourhood(pi_rFilter.m_Neighbourhood)
    {
    m_pInputPixelType = pi_rFilter.m_pInputPixelType;
    m_pOutputPixelType = pi_rFilter.m_pOutputPixelType;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPFilter::~HRPFilter()
    {
    }

//-----------------------------------------------------------------------------
// public
// ComposeWith
//-----------------------------------------------------------------------------
HRPFilter* HRPFilter::ComposeWith(const HRPFilter* pi_pFilter)
    {
    HPRECONDITION(pi_pFilter != 0);

    // by default, we create a complex filter
    HAutoPtr<HRPComplexFilter> pComplexFilter(new HRPComplexFilter());
    pComplexFilter->Insert(this);
    pComplexFilter->Insert(pi_pFilter);

    return(pComplexFilter.release());
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPFilter::Convert(const HFCPtr<HRPPixelType>& pio_pPixelType)
    {
    HPRECONDITION(pio_pPixelType != 0);
    HPRECONDITION(pio_pPixelType->CountIndexBits() != 0);

    if(!m_Neighbourhood.IsUnity())
        return;

    SetInputPixelType(pio_pPixelType);
    SetOutputPixelType(HRPPixelTypeFactory::GetInstance()->Create(pio_pPixelType->GetChannelOrg(), 0));

    uint32_t Index;
    uint32_t CountIndexBits = pio_pPixelType->CountIndexBits();
    Byte SrcRawData;
    uint32_t ValueOut;

    // create the input and output buffers
    HRPPixelBuffer Input(*GetInputPixelType(), &SrcRawData, 1, 1, 0);
    HRPPixelBuffer Output(*GetOutputPixelType(), &ValueOut, 1, 1, 0);

    HRPPixelPalette& rPalette = pio_pPixelType->LockPalette();

    try
        {
        for(Index = 0; Index < rPalette.CountUsedEntries(); Index++)
            {
            // we shift the index to the left of the byte so it becomes the raw data
            // to convert
            SrcRawData = (Byte)(Index << (8 - CountIndexBits));

            // convert the palette composite value
            Convert(&Input, &Output);

            // change the palette composite value
            rPalette.SetCompositeValue(Index, &ValueOut);
            }
        }
    catch(...)
        {
        pio_pPixelType->UnlockPalette();
        throw;
        }

    pio_pPixelType->UnlockPalette();
    }
