//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFRaster
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGFRaster.h>

HPM_REGISTER_ABSTRACT_CLASS(HGFRaster, HGFGraphicObject)


//-----------------------------------------------------------------------------
// Default constructor
//-----------------------------------------------------------------------------
HGFRaster::HGFRaster()
    : HGFGraphicObject()
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HGFRaster::HGFRaster(const HGFRaster& pi_rObj)
    : HGFGraphicObject(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HGFRaster::HGFRaster(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HGFGraphicObject (pi_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
HGFRaster::~HGFRaster()
    {
    }

//-----------------------------------------------------------------------------
// Equal operator.
//-----------------------------------------------------------------------------
HGFRaster& HGFRaster::operator=(const HGFRaster& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGFGraphicObject::operator=(pi_rObj);
        }

    return(*this);
    }

