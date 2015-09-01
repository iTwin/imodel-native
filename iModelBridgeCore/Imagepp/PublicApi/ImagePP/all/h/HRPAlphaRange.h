//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPAlphaRange.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPAlphaRange
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HGFColorSet.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPAlphaRange : public HFCShareableObject<HRPAlphaRange>
    {
    HDECLARE_SEALEDCLASS_ID(HRPAlphaRangeId_Base)

public:

    // primary methods
    IMAGEPP_EXPORT HRPAlphaRange();
    IMAGEPP_EXPORT HRPAlphaRange(  Byte pi_RedMin,    // constructor kept for compatibility purposes
                           Byte pi_RedMax,
                           Byte pi_GreenMin,
                           Byte pi_GreenMax,
                           Byte pi_BlueMin,
                           Byte pi_BlueMax,
                           Byte pi_AlphaValue);
    IMAGEPP_EXPORT HRPAlphaRange(HFCPtr<HGFColorSet>& pi_pSet, Byte pi_AlphaValue);
    IMAGEPP_EXPORT HRPAlphaRange(const HRPAlphaRange& pi_rObj);

    // Overriden methods
    IMAGEPP_EXPORT HRPAlphaRange&  operator=(const HRPAlphaRange& pi_rObj);
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
END_IMAGEPP_NAMESPACE

#include "HRPAlphaRange.hpp"

