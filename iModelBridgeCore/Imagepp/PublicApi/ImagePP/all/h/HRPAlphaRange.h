//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPAlphaRange.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPAlphaRange
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HGFColorSet.h"

class HRPAlphaRange : public HFCShareableObject<HRPAlphaRange>
    {
    HDECLARE_SEALEDCLASS_ID(1212)

public:

    // primary methods
    _HDLLg HRPAlphaRange();
    _HDLLg HRPAlphaRange(  Byte pi_RedMin,    // constructor kept for compatibility purposes
                           Byte pi_RedMax,
                           Byte pi_GreenMin,
                           Byte pi_GreenMax,
                           Byte pi_BlueMin,
                           Byte pi_BlueMax,
                           Byte pi_AlphaValue);
    _HDLLg HRPAlphaRange(HFCPtr<HGFColorSet>& pi_pSet, Byte pi_AlphaValue);
    _HDLLg HRPAlphaRange(const HRPAlphaRange& pi_rObj);

    // Overriden methods
    _HDLLg HRPAlphaRange&  operator=(const HRPAlphaRange& pi_rObj);
    HRPAlphaRange* Clone() const;

    // added methods
    bool           IsIn(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const;
    Byte          GetAlphaValue() const;

    bool           operator==(const HRPAlphaRange&) const {
        return false;
        }
    bool           operator!=(const HRPAlphaRange&) const {
        return false;
        }
    bool           operator<(const HRPAlphaRange&) const {
        return false;
        }
    bool           operator>(const HRPAlphaRange&) const {
        return false;
        }

private:

    void            DeepCopy(const HRPAlphaRange& pi_rObj);

    HFCPtr<HGFColorSet>
    m_pSet;
    Byte          m_AlphaValue;

    };

typedef vector<HRPAlphaRange, allocator<HRPAlphaRange> >
ListHRPAlphaRange;

#include "HRPAlphaRange.hpp"

