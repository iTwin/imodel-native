//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFScanLineOrientation.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFScanLineOrientation
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DTransfoModel.h"
#include "HGF2DPosition.h"

BEGIN_IMAGEPP_NAMESPACE
// The order of value in the enum HGFSLO is very important. Please do not change it.
// with this specific order you can examine the origine an the orientation easily .
// Bit 0 == 0 : Origin is at the left
// Bit 0 == 1 : Origin is at the right
// Bit 1 == 0 : Origin is at the upper
// Bit 1 == 1 : Origin is at the lower
// Bit 2 == 0 : Orientation is vertical
// Bit 2 == 1 : Orientation is horizontal

enum HGFSLO
    {
    HGF_UPPER_LEFT_VERTICAL = 0,
    HGF_UPPER_RIGHT_VERTICAL   ,
    HGF_LOWER_LEFT_VERTICAL    ,
    HGF_LOWER_RIGHT_VERTICAL   ,
    HGF_UPPER_LEFT_HORIZONTAL  ,
    HGF_UPPER_RIGHT_HORIZONTAL ,
    HGF_LOWER_LEFT_HORIZONTAL  ,
    HGF_LOWER_RIGHT_HORIZONTAL
    };

class HGFScanLineOrientation
    {
public:

    // Rpimary methods
    HGFScanLineOrientation(
        HGFSLO pi_ScanLineOrientation = HGF_UPPER_LEFT_HORIZONTAL);
    HGFScanLineOrientation(const HGFScanLineOrientation& pi_rObj);

    HGFScanLineOrientation&
    operator=(const HGFScanLineOrientation&);
    // Others
    HFCPtr<HGF2DTransfoModel>
    GetTransfoModelFrom(HGFSLO          pi_ScanLineOrientation,
                        uint32_t        pi_Width,
                        uint32_t        pi_Height,
                        HGF2DPosition&  pi_Origin);


    HGFSLO          GetScanLineOrigin() const;

protected:

private:

    // Attributes
    HGFSLO          m_ScanLineOrigin;

    // Private methods
    void            DeepCopy(const HGFScanLineOrientation& pi_rObj);
    void            DeepDelete();


    };

END_IMAGEPP_NAMESPACE