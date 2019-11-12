//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGF2DCoordSys
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGF2DWorld.h>


HPM_REGISTER_CLASS(HGF2DWorld, HGF2DCoordSys)


//-----------------------------------------------------------------------------
// operator=
// PRIVATE
// Assignment operator.  It duplicates another world object.
// Desactivated method
//-----------------------------------------------------------------------------
HGF2DWorld& HGF2DWorld::operator=(const HGF2DWorld& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // call ancester operator (CANNOT BE CALLED)
        // HGF2DCoordSys::operator=(pi_rObj);

        // Copy identificator
        m_Identifier = pi_rObj.m_Identifier;
        }

    // Return reference to self
    return (*this);
    }

