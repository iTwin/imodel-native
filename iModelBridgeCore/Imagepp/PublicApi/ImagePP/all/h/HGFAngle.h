//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFAngle.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Description of an angle. An angle is a quantity with angular
// units.
//-----------------------------------------------------------------------------

#pragma once

inline double  CalculateNormalizedTrigoValue(double pi_Angle)
    {
    // The value must be finite
    HPRECONDITION(BeNumerical::BeFinite(pi_Angle));

    // Normalize it
    while (pi_Angle < 0.0)
        pi_Angle += 2*PI;
    while (pi_Angle > 2*PI)
        pi_Angle -= 2*PI;

    return pi_Angle;
    }

bool   ConvertDegMinSecToDeg(WString& pi_rDegMinSec,
                             double& po_rResultigDegreeValue);

