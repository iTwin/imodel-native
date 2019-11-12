//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
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

BEGIN_IMAGEPP_NAMESPACE
class HRPTypeAdaptFilter : public HRPFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_TypeAdapt, HRPFilter)

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
                               HRPPixelBuffer* pio_pOutputBuffer) override;

    // List of filters

    void            Insert(const HRPTypedFilter* pi_pFilter);

    const HRPTypedFilter*
    GetPreferredFilterFor(const HFCPtr<HRPPixelType>& pi_rpInputPixelType) const;

    // Pixel types

    void            SetInputPixelType(
        const HFCPtr<HRPPixelType>& pi_pInputPixelType) override;

    void            SetOutputPixelType(
        const HFCPtr<HRPPixelType>& pi_pOutputPixelType) override;

private:

    ListFilters     m_List;

    ListFilters::iterator
    m_Itr;
    };
END_IMAGEPP_NAMESPACE

