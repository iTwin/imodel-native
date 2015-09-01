//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRaster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFRaster
//-----------------------------------------------------------------------------
// This class describes any graphical object of raster type (graphics that
// are composed of pixels).  Abstract class.
//-----------------------------------------------------------------------------

#pragma once

#include "HGFGraphicObject.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DCoordSys;
template<class T> class HFCPtr;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Donald Morissette

    HGFRaster is an abstract class which defines the ancestor of all raster
    objects. The class does not serve any other purpose than defining a
    common ancestor to all rasters within the scope of the Graphic
    Foundation library. No constructor is public, nor is any other
    methods exception made of the destructor.
    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGFRaster : public HGFGraphicObject
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HGFRasterId)

public:

    // Primary methods
    virtual         ~HGFRaster();

protected:

    // Primary methods
    HGFRaster();
    HGFRaster(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HGFRaster(const HGFRaster& pi_rObj);
    HGFRaster&      operator=(const HGFRaster& pi_rObj);

private:

    // Private methods
    };


END_IMAGEPP_NAMESPACE