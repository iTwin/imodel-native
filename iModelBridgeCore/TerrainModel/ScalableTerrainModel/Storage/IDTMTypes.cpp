//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMTypes.cpp $
//:>    $RCSfile: IDTMTypes.cpp,v $
//:>   $Revision: 1.5 $
//:>       $Date: 2011/01/10 15:26:51 $
//:>     $Author: Raymond.Gauthier $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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