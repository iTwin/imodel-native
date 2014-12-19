//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSVoidSurface.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> inline method for HGSVoidSurface
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

HGS_BEGIN_SURFACECAPABILITIES_REGISTRATION(HGSVoidSurface)
HGS_END_SURFACECAPABILITIES_REGISTRATION()

/**----------------------------------------------------------------------------
 Constructor for this class
-----------------------------------------------------------------------------*/
inline HGSVoidSurface::HGSVoidSurface(const HFCPtr<HGSSurfaceDescriptor>&   pi_rpDescriptor)
    : HGSSurfaceImplementation(pi_rpDescriptor)
    {
    HPRECONDITION(pi_rpDescriptor != 0);
    }

/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
inline HGSVoidSurface::~HGSVoidSurface()
    {
    }

