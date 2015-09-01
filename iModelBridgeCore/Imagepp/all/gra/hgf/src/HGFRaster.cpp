//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFRaster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFRaster.h>

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

