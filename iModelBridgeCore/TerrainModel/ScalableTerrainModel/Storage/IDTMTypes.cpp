//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMTypes.h>

namespace IDTMFile {

bool Extent2d64f::operator== (const Extent2d64f& pi_rRight) const
    {
    return equal(&xMin, &xMin + 4, &pi_rRight.xMin, &HNumeric<double>::EQUAL_EPSILON);
    }


bool Extent3d64f::operator== (const Extent3d64f& pi_rRight) const
    {
    return equal(&xMin, &xMin + 6, &pi_rRight.xMin, &HNumeric<double>::EQUAL_EPSILON);
    }




} //End namespace IDTMFile