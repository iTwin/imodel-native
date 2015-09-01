//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPComplexFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPComplexFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPComplexFilter.h>

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPComplexFilter::HRPComplexFilter() :
    HRPFilter(),
    m_List()
    {
    m_Itr = m_List.begin();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPComplexFilter::HRPComplexFilter(const HRPComplexFilter& pi_rComplexFilter)
    :   HRPFilter(pi_rComplexFilter),
        m_List()
    {
    m_Itr = m_List.begin();

    // Reset the neighborhoud
    SetNeighbourhood(HRPPixelNeighbourhood());

    try
        {
        // copy the list
        for(ListFilters::const_iterator Itr = pi_rComplexFilter.m_List.begin();
            Itr != pi_rComplexFilter.m_List.end();
            Itr++)
            Insert(*Itr);
        }
    catch(...)
        {
        // delete the list
        for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
            delete(*Itr);

        // Continue the propagation of the exception.
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPComplexFilter::~HRPComplexFilter()
    {
    // delete the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        delete(*Itr);

    }

//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRPComplexFilter::Clear()
    {
    // delete the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        delete(*Itr);

    m_List.clear();

    // update the iterator
    m_Itr = m_List.begin();

    // reset the neighborhood
    SetNeighbourhood(HRPPixelNeighbourhood());
    }

//-----------------------------------------------------------------------------
// public
// Cloning
//-----------------------------------------------------------------------------
HRPFilter* HRPComplexFilter::Clone() const
    {
    return(new HRPComplexFilter(*this));
    }

//-----------------------------------------------------------------------------
// public
// ComposeWith
//-----------------------------------------------------------------------------
HRPFilter* HRPComplexFilter::ComposeWith(const HRPFilter* pi_pFilter)
    {
    HRPComplexFilter* pFilter = new HRPComplexFilter(*this);

    if((pFilter->m_List).empty())
        {
        // if the list of filters is empty, we simply insert the filter
        // in the list
        (pFilter->m_List).push_back(pi_pFilter->Clone());
        }
    else
        {
        HRPFilter* pLastFilter = (pFilter->m_List).back();

        // we compose the last filter in the list with the parameter
        // filter
        HRPFilter* pComposedFilter = pLastFilter->ComposeWith(pi_pFilter);

        // is it a complex filter
        if(pComposedFilter->GetClassID() == GetClassID())
            {
            // if the result is a complex filter, we simply add the
            // parameter filter and destroy the composed filter
            (pFilter->m_List).push_back(pi_pFilter->Clone());
            delete pComposedFilter;
            }
        else
            {
            // otherwise, remove the last filter of the list and
            // replace it by the new composed filter
            (pFilter->m_List).pop_back();
            delete pLastFilter;
            (pFilter->m_List).push_back(pComposedFilter);
            }
        }

    // update the position of the iterator at the end of the list
    (pFilter->m_Itr) = m_List.end();

    // return the current filter
    return pFilter;
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPComplexFilter::Convert( HRPPixelBuffer* pi_pInputBuffer,
                                HRPPixelBuffer* pio_pOutputBuffer)
    {
    HPRECONDITION(GetInputPixelType().GetPtr() != NULL);
    HPRECONDITION(GetOutputPixelType().GetPtr() != NULL);

    // if the list of filters is empty, we return immediately
    if(m_List.empty())
        return;

    HRPPixelBuffer*     pInPixelBuffer;
    HRPPixelBuffer*     pOutPixelBuffer;
    Byte*              pOutBuffer = NULL;

    // we must create a new continuous input buffer
    Byte* pInBuffer = pi_pInputBuffer->Extract();

    // calculate the width and height of the input buffer (including borders)
    const HRPPixelNeighbourhood& Neighbourhood = GetNeighbourhood();
    uint32_t Width  = pi_pInputBuffer->GetWidth() + Neighbourhood.GetWidth() - 1;
    uint32_t Height = pi_pInputBuffer->GetHeight() + Neighbourhood.GetHeight() - 1;

    // iterate through each filter in the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        {
        // get the Neighbourhood of the current filter
        const HRPPixelNeighbourhood& FilterNeighbourhood =
            (*Itr)->GetNeighbourhood();

        // update the width and height (extract the borders of the
        // current filter)
        Width  -= (FilterNeighbourhood.GetWidth() - 1);
        Height -= (FilterNeighbourhood.GetHeight() - 1);

        // we create a new input pixel buffer
        pInPixelBuffer = new HRPPixelBuffer((*Itr)->GetNeighbourhood(),
                                            *((*Itr)->GetInputPixelType()),
                                            pInBuffer,
                                            Width,
                                            Height,
                                            0,
                                            true);

        // Is it the last filter in the list?
        if((*Itr) == (m_List.back()))
            {
            // take directly the output pixel buffer
            // for the next filtering process
            pOutPixelBuffer = pio_pOutputBuffer;
            }
        else
            {
            // create a new output pixel buffer

            // get the output pixel type of the current filter
            const HFCPtr<HRPPixelType>& pOutType = (*Itr)->GetOutputPixelType();

            // create a new output buffer appropriated for the filter
            pOutBuffer = new Byte[Width * Height *
                                   (pOutType->CountPixelRawDataBits() / 8)];

            // create an output PIXEL buffer
            pOutPixelBuffer = new HRPPixelBuffer(*pOutType,
                                                 pOutBuffer,
                                                 Width,
                                                 Height);
            }

        // filter the data
        (*Itr)->Convert(pInPixelBuffer, pOutPixelBuffer);

        // we don't need anymore the input buffer
        delete pInPixelBuffer;
        delete pInBuffer;

        // if the output pixel buffer is not the last
        if(pOutPixelBuffer != pio_pOutputBuffer)
            {
            // we delete the pixel buffer object
            delete pOutPixelBuffer;

            // the output buffer become the input for
            // the following stage
            pInBuffer = pOutBuffer;
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// GetList
//-----------------------------------------------------------------------------
const HRPComplexFilter::ListFilters& HRPComplexFilter::GetList() const
    {
    // return an iterator on the list begin
    return(m_List);
    }

//-----------------------------------------------------------------------------
// public
// Insert
//-----------------------------------------------------------------------------
void HRPComplexFilter::Insert(const HRPFilter* pi_pFilter)
    {
    // clone the parameter filter
    HRPFilter* pFilter = pi_pFilter->Clone();

    // if the filter will not be the first element in the list
    if(m_Itr != m_List.begin())
        {
        // the input pixel type of the filter is the complex output type
        if(GetOutputPixelType().GetPtr() != NULL)
            pFilter->SetInputPixelType(GetOutputPixelType());
        }
    else
        {
        // otherwise, the input type is the complex input
        if(GetInputPixelType().GetPtr() != NULL)
            pFilter->SetInputPixelType(GetInputPixelType());
        }

    // the output pixel type of the filter is the complex output type
    if(GetOutputPixelType().GetPtr() != NULL)
        pFilter->SetOutputPixelType(GetOutputPixelType());

    // insert the element in the list
    m_Itr = m_List.insert(m_Itr, pFilter);
    m_Itr++;

    // update the Neighbourhood (sum the actual and the one
    // of the filter added)
    SetNeighbourhood(GetNeighbourhood() + pFilter->GetNeighbourhood());
    }

//-----------------------------------------------------------------------------
// public
// IsAComplexFilter
//-----------------------------------------------------------------------------
bool HRPComplexFilter::IsAComplexFilter() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// Reduce
//-----------------------------------------------------------------------------
HRPFilter* HRPComplexFilter::Reduce() const
    {
    HRPFilter* pNewFilter = 0;
    HRPFilter* pTmpFilter;

    // iterate through each filter in the list
    for(ListFilters::const_iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        {
        if(pNewFilter == 0)
            {
            pNewFilter = (*Itr)->Clone();
            }
        else
            {
            pTmpFilter = pNewFilter->ComposeWith(*Itr);
            delete pNewFilter;
            pNewFilter = pTmpFilter;
            }
        }

    if(pNewFilter == 0)
        pNewFilter = this->Clone();

    return pNewFilter;
    }

//-----------------------------------------------------------------------------
// public
// SetInputPixelType
//-----------------------------------------------------------------------------
void HRPComplexFilter::SetInputPixelType(
    const HFCPtr<HRPPixelType>& pi_pInputPixelType)
    {
    // set the input pixel type of the first filter in the list
    if(!m_List.empty())
        (m_List.front())->SetInputPixelType(pi_pInputPixelType);

    HRPFilter::SetInputPixelType(pi_pInputPixelType);
    }

//-----------------------------------------------------------------------------
// public
// SetOutputPixelType
//-----------------------------------------------------------------------------
void HRPComplexFilter::SetOutputPixelType(
    const HFCPtr<HRPPixelType>& pi_pOutputPixelType)
    {
    // iterate through each filter in the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        {
        // the first filter has its input pixel type equal to the complex input type
        // the others use the output pixel type of the complex filter
        if(Itr != m_List.begin())
            (*Itr)->SetInputPixelType(pi_pOutputPixelType);

        // the output pixel type of a filter in the list is always the
        // complex output type
        (*Itr)->SetOutputPixelType(pi_pOutputPixelType);
        }

    HRPFilter::SetOutputPixelType(pi_pOutputPixelType);
    }
