//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRGBCube.h $
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

class HGFRGBCube : public HGFColorSet
    {
    HDECLARE_CLASS_ID(HGFRGBId_Cube, HGFColorSet)

public:

    HGFRGBCube();
    HGFRGBCube(const HGFRGBCube& pi_rSrc);
    HGFRGBCube(Byte pi_Rmin, Byte pi_Rmax,
               Byte pi_Gmin, Byte pi_Gmax,
               Byte pi_Bmin, Byte pi_Bmax);
    virtual         ~HGFRGBCube();

    HGFRGBCube& operator=(const HGFRGBCube& pi_rSrc);

    virtual bool   IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const;

protected:

private:

    Byte m_Rmin;
    Byte m_Rmax;
    Byte m_Gmin;
    Byte m_Gmax;
    Byte m_Bmin;
    Byte m_Bmax;

    };

END_IMAGEPP_NAMESPACE

#include "HGFRGBCube.hpp"

