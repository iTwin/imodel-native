//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPQuantizedPalette
//-----------------------------------------------------------------------------
// Quantize palette definition.
//-----------------------------------------------------------------------------

#pragma once

#include "HRPPixelPalette.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPHistogram;

class HNOVTABLEINIT HRPQuantizedPalette
    {
public:
    // Primary methods
    HRPQuantizedPalette();
    HRPQuantizedPalette(uint16_t pi_MaxEntries);

    virtual         ~HRPQuantizedPalette();

    virtual bool   AddCompositeValue(  const void* pi_pValue,
                                        uint32_t    pi_Count=1) = 0;

    virtual uint16_t GetPalette(HRPPixelPalette* po_pPixelPalette,
                               HRPHistogram*    po_pHistogram = 0) const = 0;

    virtual void    FlushEntries() = 0;

    uint16_t GetMaxEntries() const;

    virtual void    AddIgnoreValue (const void* pi_pValue) = 0;


protected:

    HRPQuantizedPalette(const HRPQuantizedPalette& pi_rObj);
    HRPQuantizedPalette& operator=(const HRPQuantizedPalette& pi_rObj);

private:

    uint16_t  m_MaxEntries;
    };
END_IMAGEPP_NAMESPACE


#include "HRPQuantizedPalette.hpp"

