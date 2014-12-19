//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DGrid.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DGrid
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DExtent.h"
#include "HGF2DCoordSys.h"

class HGF2DGrid
    {
public:

    // Primary methods
    HGF2DGrid (const HGF2DExtent& pi_rExtent);
    HGF2DGrid (const HGF2DExtent& pi_rExtent,
               const HFCPtr<HGF2DCoordSys>&  pi_rpCoordSys);
    virtual         ~HGF2DGrid();

    HGF2DGrid(const HGF2DGrid& pi_rObj);
    HGF2DGrid&      operator=(const HGF2DGrid& pi_rObj);


    // Coordinate management
    int32_t        GetXMin() const;
    int32_t        GetYMin() const;
    int32_t        GetXMax() const;
    int32_t        GetYMax() const;

    // Dimension measurement
    int32_t        GetWidth() const;
    int32_t        GetHeight() const;

protected:

private:

    HGF2DExtent     m_Extent;
    };


#include "HGF2DGrid.hpp"

