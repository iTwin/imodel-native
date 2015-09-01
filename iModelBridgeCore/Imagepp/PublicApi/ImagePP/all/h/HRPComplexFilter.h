//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPComplexFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    HRPFilter*      ComposeWith(const HRPFilter* pi_pFilter);

    HRPFilter*      Reduce() const;

    // Conversion

    void            Convert(   HRPPixelBuffer* pi_pInputBuffer,
                               HRPPixelBuffer* pio_pOutputBuffer);

    // Decapsulation

    bool           IsAComplexFilter() const;

    // List of filters

    void            Clear();

    IMAGEPP_EXPORT void            Insert(const HRPFilter* pi_pFilter);

    IMAGEPP_EXPORT const ListFilters&
    GetList() const;

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
END_IMAGEPP_NAMESPACE
