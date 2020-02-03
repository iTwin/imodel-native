//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPComplexFilter
//-----------------------------------------------------------------------------
// This class describes a complex filter.
//-----------------------------------------------------------------------------
#pragma once


#include "HRPFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPComplexFilter : public HRPFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Complex, HRPFilter)

public:

    typedef list<HRPFilter*, allocator<HRPFilter*> >
    ListFilters;


    // Primary methods

    IMAGEPP_EXPORT                 HRPComplexFilter();

    IMAGEPP_EXPORT                 HRPComplexFilter(
        const HRPComplexFilter& pi_rComplexFilter);

    IMAGEPP_EXPORT virtual            ~HRPComplexFilter();


    // Cloning

    virtual HRPFilter* Clone() const override;


    // Composing
    HRPFilter*      ComposeWith(const HRPFilter* pi_pFilter) override;

    HRPFilter*      Reduce() const;

    // Conversion

    void            Convert(   HRPPixelBuffer* pi_pInputBuffer,
                               HRPPixelBuffer* pio_pOutputBuffer) override;

    // Decapsulation

    bool           IsAComplexFilter() const;

    // List of filters

    void            Clear();

    IMAGEPP_EXPORT void            Insert(const HRPFilter* pi_pFilter);

    IMAGEPP_EXPORT const ListFilters&
    GetList() const;

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
