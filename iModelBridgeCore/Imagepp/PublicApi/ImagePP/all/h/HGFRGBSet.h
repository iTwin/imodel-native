//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRGBSet.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HGFRGBSet
//---------------------------------------------------------------------------

#pragma once

#include "HGFColorSet.h"

BEGIN_IMAGEPP_NAMESPACE
class HGFRGBSet : public HGFColorSet
    {
    HDECLARE_CLASS_ID(HGFRGBId_Set, HGFColorSet)

public:

    IMAGEPP_EXPORT HGFRGBSet();
    IMAGEPP_EXPORT HGFRGBSet(const HGFRGBSet& pi_rSrc);
    IMAGEPP_EXPORT virtual         ~HGFRGBSet();


    HGFRGBSet& operator=(const HGFRGBSet& pi_rSrc);

    IMAGEPP_EXPORT void            Add(Byte pi_R, Byte pi_G, Byte pi_B);
    IMAGEPP_EXPORT virtual bool   IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const;
    IMAGEPP_EXPORT void            Remove(Byte pi_R, Byte pi_G, Byte pi_B);

protected:

    // pi_Level varies from 7 to zero.

    bool           IsIn(Byte pi_R, Byte pi_G, Byte pi_B, int8_t pi_Level) const;
    bool           Add(Byte pi_R, Byte pi_G, Byte pi_B, int8_t pi_Level);
    bool           Remove(Byte pi_R, Byte pi_G, Byte pi_B, int8_t pi_Level);

private:
    void            LevelDeepCopy(const HGFRGBSet* pi_pChildArray, int8_t pi_Level);

    // Members
    HGFRGBSet*      m_pChildren;
    size_t          m_Count;    // count of terminal nodes in children

    };

END_IMAGEPP_NAMESPACE
#include "HGFRGBSet.hpp"

