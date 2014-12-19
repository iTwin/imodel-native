//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPComplexFilter.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPComplexFilter
//-----------------------------------------------------------------------------
// This class describes a complex filter.
//-----------------------------------------------------------------------------
#pragma once


#include "HRPFilter.h"

class HRPComplexFilter : public HRPFilter
    {
    HDECLARE_CLASS_ID(1135, HRPFilter)

public:

    typedef list<HRPFilter*, allocator<HRPFilter*> >
    ListFilters;


    // Primary methods

    _HDLLg                 HRPComplexFilter();

    _HDLLg                 HRPComplexFilter(
        const HRPComplexFilter& pi_rComplexFilter);

    _HDLLg virtual            ~HRPComplexFilter();


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

    _HDLLg void            Insert(const HRPFilter* pi_pFilter);

    _HDLLg const ListFilters&
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
