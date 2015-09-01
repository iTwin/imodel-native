//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPTypeAdaptFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPTypeAdaptFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPTypeAdaptFilter.h>

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPTypeAdaptFilter::HRPTypeAdaptFilter() :
    HRPFilter(),
    m_List()
    {
    m_Itr = m_List.begin();
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPTypeAdaptFilter::HRPTypeAdaptFilter(const HRPPixelNeighbourhood& pi_rNeighbourhood)
    :  HRPFilter(pi_rNeighbourhood),
       m_List()
    {
    m_Itr = m_List.begin();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPTypeAdaptFilter::HRPTypeAdaptFilter(const HRPTypeAdaptFilter& pi_rTypeAdaptFilter)
    :   HRPFilter(pi_rTypeAdaptFilter),
        m_List()
    {
    m_Itr = m_List.begin();

    // copy the list
    try
        {
        for(ListFilters::const_iterator Itr = pi_rTypeAdaptFilter.m_List.begin();
            Itr != pi_rTypeAdaptFilter.m_List.end();
            Itr++)
            Insert((HRPTypedFilter*)(*Itr));
        }
    catch(...)
        {
        // delete the list
        for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
            delete(*Itr);

        // Continue the exception propagation.
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPTypeAdaptFilter::~HRPTypeAdaptFilter()
    {
    // delete the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        delete(*Itr);
    }

//-----------------------------------------------------------------------------
// public
// Cloning
//-----------------------------------------------------------------------------
HRPFilter* HRPTypeAdaptFilter::Clone() const
    {
    return(new HRPTypeAdaptFilter(*this));
    }

/*
//-----------------------------------------------------------------------------
// public
// ComposeWith
//-----------------------------------------------------------------------------
HRPFilter* HRPTypeAdaptFilter::ComposeWith(const HRPFilter* pi_pFilter)
{

    HRPTypeAdaptFilter* pFilter = new HRPTypeAdaptFilter(*this);

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

        if(pComposedFilter->IsATypeAdaptFilter())
        {
            // if the result is a TypeAdapt filter, we simply add the
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
*/

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPTypeAdaptFilter::Convert( HRPPixelBuffer* pi_pInputBuffer,
                                  HRPPixelBuffer* pio_pOutputBuffer)
    {
    HPRECONDITION(GetInputPixelType() != 0);
    HPRECONDITION(GetOutputPixelType() != 0);

    HRPTypedFilter* pFilter = 0;

    // iterate through each filter in the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        if((*(*Itr)->GetFilterPixelType()).HasSamePixelInterpretation((*GetInputPixelType())))
            {
            pFilter = (*Itr);
            break;
            }

    if(pFilter == 0)
        pFilter = m_List.front();

    pFilter->Convert(pi_pInputBuffer, pio_pOutputBuffer);
    }

//-----------------------------------------------------------------------------
// public
// Insert
//-----------------------------------------------------------------------------
void HRPTypeAdaptFilter::Insert(const HRPTypedFilter* pi_pTypedFilter)
    {
//    HPRECONDITION(pi_pTypedFilter->GetNeighbourhood() == GetNeighbourhood());

    // clone the parameter filter
    HRPTypedFilter* pFilter = (HRPTypedFilter*)(pi_pTypedFilter->Clone());

    if(GetInputPixelType() != 0)
        pFilter->SetInputPixelType(GetInputPixelType());

    if(GetOutputPixelType() != 0)
        pFilter->SetOutputPixelType(GetOutputPixelType());

    // insert the element in the list
    m_Itr = m_List.insert(m_Itr, pFilter);
    m_Itr++;
    }

/*
//-----------------------------------------------------------------------------
// public
// Reduce
//-----------------------------------------------------------------------------
HRPFilter* HRPTypeAdaptFilter::Reduce() const
{
    HRPFilter* pNewFilter = 0;
    HRPFilter* pTmpFilter;

  // iterate through each filter in the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
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
*/

//-----------------------------------------------------------------------------
// public
// SetInputPixelType
//-----------------------------------------------------------------------------
void HRPTypeAdaptFilter::SetInputPixelType(
    const HFCPtr<HRPPixelType>& pi_pInputPixelType)
    {
    // iterate through each filter in the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        (*Itr)->SetInputPixelType(pi_pInputPixelType);

    HRPFilter::SetInputPixelType(pi_pInputPixelType);
    }

//-----------------------------------------------------------------------------
// public
// SetOutputPixelType
//-----------------------------------------------------------------------------
void HRPTypeAdaptFilter::SetOutputPixelType(
    const HFCPtr<HRPPixelType>& pi_pOutputPixelType)
    {
    // iterate through each filter in the list
    for(ListFilters::iterator Itr = m_List.begin(); Itr != m_List.end(); Itr++)
        (*Itr)->SetOutputPixelType(pi_pOutputPixelType);

    HRPFilter::SetOutputPixelType(pi_pOutputPixelType);
    }


//-----------------------------------------------------------------------------
// public
// Retrieve the filter that would be selected if the input pixeltype was
// set to pi_rpInputPixelType.
//-----------------------------------------------------------------------------
const HRPTypedFilter* HRPTypeAdaptFilter::GetPreferredFilterFor(const HFCPtr<HRPPixelType>& pi_rpInputPixelType) const
    {
    HRPTypedFilter* pFilter = 0;

    // iterate through each filter in the list
    for(ListFilters::const_iterator Itr(m_List.begin()); Itr != m_List.end(); Itr++)
        if((*(*Itr)->GetFilterPixelType()).HasSamePixelInterpretation(*pi_rpInputPixelType))
            {
            pFilter = (*Itr);
            break;
            }

    if(pFilter == 0 && m_List.size() > 0)
        pFilter = m_List.front();

    return pFilter;
    }
