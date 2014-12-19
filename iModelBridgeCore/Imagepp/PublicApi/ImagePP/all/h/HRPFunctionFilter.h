//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPFunctionFilter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilter
//-----------------------------------------------------------------------------
// This class describes a function filter.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPTypedFilter.h"

class HRPFunctionFilter : public HRPTypedFilter
    {
    HDECLARE_CLASS_ID(1134, HRPTypedFilter)

public:

    // Primary methods
    _HDLLg virtual         ~HRPFunctionFilter();

    // Conversion
    _HDLLg void             Convert(HRPPixelBuffer* pi_pInputBuffer,
                                    HRPPixelBuffer* pio_pOutputBuffer);

    // Cloning
    virtual HRPFilter* Clone() const override = 0;

protected:

    // Primary methods
    _HDLLg HRPFunctionFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    _HDLLg HRPFunctionFilter(const HRPFunctionFilter& pi_rFilter);

    // Function called by this class
    virtual void    Function(const void* pi_pSrcRawData,
                             void* po_pDestRawData,
                             uint32_t PixelsCount) const = 0;

private:
    };

