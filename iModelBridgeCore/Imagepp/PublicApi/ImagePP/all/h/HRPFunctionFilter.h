//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPFunctionFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilter
//-----------------------------------------------------------------------------
// This class describes a function filter.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPTypedFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPFunctionFilter : public HRPTypedFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Function, HRPTypedFilter)

public:

    // Primary methods
    IMAGEPP_EXPORT virtual         ~HRPFunctionFilter();

    // Conversion
    IMAGEPP_EXPORT void             Convert(HRPPixelBuffer* pi_pInputBuffer,
                                    HRPPixelBuffer* pio_pOutputBuffer);

    // Cloning
    virtual HRPFilter* Clone() const override = 0;

protected:

    // Primary methods
    IMAGEPP_EXPORT HRPFunctionFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT HRPFunctionFilter(const HRPFunctionFilter& pi_rFilter);

    // Function called by this class
    virtual void    Function(const void* pi_pSrcRawData,
                             void* po_pDestRawData,
                             uint32_t PixelsCount) const = 0;

private:
    };
END_IMAGEPP_NAMESPACE

