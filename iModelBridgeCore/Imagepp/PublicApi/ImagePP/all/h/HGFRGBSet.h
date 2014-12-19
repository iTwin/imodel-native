//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRGBSet.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HGFRGBSet
//---------------------------------------------------------------------------

#pragma once

#include "HGFColorSet.h"

class HGFRGBSet : public HGFColorSet
    {
    HDECLARE_CLASS_ID(1538, HGFColorSet)

public:

    _HDLLg HGFRGBSet();
    _HDLLg HGFRGBSet(const HGFRGBSet& pi_rSrc);
    _HDLLg virtual         ~HGFRGBSet();


    HGFRGBSet& operator=(const HGFRGBSet& pi_rSrc);

    _HDLLg void            Add(Byte pi_R, Byte pi_G, Byte pi_B);
    _HDLLg virtual bool   IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const;
    _HDLLg void            Remove(Byte pi_R, Byte pi_G, Byte pi_B);

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

#include "HGFRGBSet.hpp"

