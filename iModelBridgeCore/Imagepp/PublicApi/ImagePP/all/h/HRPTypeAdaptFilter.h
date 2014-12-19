//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPTypeAdaptFilter.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPTypeAdaptFilter
//-----------------------------------------------------------------------------
// This class describes a TypeAdapt filter.
//-----------------------------------------------------------------------------

#pragma once


#include "HRPFilter.h"

#include "HRPTypedFilter.h"

class HRPTypeAdaptFilter : public HRPFilter
    {
    HDECLARE_CLASS_ID(1167, HRPFilter)

public:

    typedef list<HRPTypedFilter*, allocator<HRPTypedFilter*> >
    ListFilters;


    // Primary methods

    HRPTypeAdaptFilter();

    HRPTypeAdaptFilter(
        const HRPTypeAdaptFilter& pi_rTypeAdaptFilter);

    HRPTypeAdaptFilter(const HRPPixelNeighbourhood& pi_rNeighbourhood);

    virtual            ~HRPTypeAdaptFilter();


    // Cloning

    virtual HRPFilter* Clone() const override;


    // Composing
    //  HRPFilter*      ComposeWith(const HRPFilter* pi_pFilter);

    //HRPFilter*      Reduce() const;

    // Conversion

    void            Convert(   HRPPixelBuffer* pi_pInputBuffer,
                               HRPPixelBuffer* pio_pOutputBuffer);

    // List of filters

    void            Insert(const HRPTypedFilter* pi_pFilter);

    const HRPTypedFilter*
    GetPreferredFilterFor(const HFCPtr<HRPPixelType>& pi_rpInputPixelType) const;

    // Pixel types

    void            SetInputPixelType(
        const HFCPtr<HRPPixelType>& pi_pInputPixelType);

    void            SetOutputPixelType(
        const HFCPtr<HRPPixelType>& pi_pOutputPixelType);

private:

    ListFilters     m_List;

    ListFilters::iterator
    m_Itr;
    };

