//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPQuantizedPalette.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    HRPQuantizedPalette(unsigned short pi_MaxEntries);

    virtual         ~HRPQuantizedPalette();

    virtual bool   AddCompositeValue(  const void* pi_pValue,
                                        uint32_t    pi_Count=1) = 0;

    virtual unsigned short GetPalette(HRPPixelPalette* po_pPixelPalette,
                               HRPHistogram*    po_pHistogram = 0) const = 0;

    virtual void    FlushEntries() = 0;

    unsigned short GetMaxEntries() const;

    virtual void    AddIgnoreValue (const void* pi_pValue) = 0;


protected:

    HRPQuantizedPalette(const HRPQuantizedPalette& pi_rObj);
    HRPQuantizedPalette& operator=(const HRPQuantizedPalette& pi_rObj);

private:

    unsigned short  m_MaxEntries;
    };
END_IMAGEPP_NAMESPACE


#include "HRPQuantizedPalette.hpp"

