//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DDisplacement.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DDisplacement
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFAngle.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>


/** -----------------------------------------------------------------------------
    This method permits to extract the bearing of the displacement.

    @return The bearing of the displacement.

    @code
        HGF2DDisplacement Vector2(10.2, 45.3);
        HGFBearing Direction = Vector2.CalculateBearing();
    @end

    @see CalculateLength()
    @see GetDeltaX()
    @see GetDeltaY()
    -----------------------------------------------------------------------------
*/
HGFBearing HGF2DDisplacement::CalculateBearing () const
    {
#define RIGHT_ANGLE (0.5 * PI)
    HGFBearing  TempBearing;

    // Check if that delta X is different from 0.0
    if (m_DeltaXDist == 0.0)
        {
        // The X offset being NULL, we must check sign of Y offset
        if (m_DeltaYDist < 0.0)
            {
            // Goes SOUTH
            TempBearing.SetAngle(3.0 * RIGHT_ANGLE);
            }
        else
            {
            // Goes NORTH
            TempBearing.SetAngle(RIGHT_ANGLE);
            }
        }
    else
        {
        // Convert distances to meters
        double MyDistYMeters = 0.0;
        double MyDistXMeters = 0.0;

        MyDistYMeters += m_DeltaYDist;
        MyDistXMeters += m_DeltaXDist;

        // Now that the two distance are expressed in the same units we can use atan2
        TempBearing.SetAngle(atan2 (MyDistYMeters, MyDistXMeters));
        }

    return (TempBearing);
    }


/** -----------------------------------------------------------------------------
    This method permits to extract the length of the displacement.

    @return The length of the displacement.

    @code
        HGF2DDisplacement Vector2(12.3, 45.6);

        double Length = Vector2.CalculateLength();
    @end

    @see CalculateBearing()
    @see GetXOffset()
    @see GetYOffset()
    -----------------------------------------------------------------------------
*/
double HGF2DDisplacement::CalculateLength () const
    {

    // Compute in meters squared the intermediate value
    double TempOpVal = ((m_DeltaXDist * m_DeltaXDist) + (m_DeltaYDist * m_DeltaYDist));

    // Create distance in meters as sqrt of temp area
    double  Length = sqrt(TempOpVal);

    // Since the previous calculations are notably imprecise
    // when numbers get large ... we trap strange results
    // Of course if either deltas are infinity, then distance is indeed infinity
    if (!BeNumerical::BeFinite(Length) && BeNumerical::BeFinite(m_DeltaXDist) && BeNumerical::BeFinite(m_DeltaYDist))
        {
        // We use the greatest of the two deltas (in absolute value)
        // This notably prevents multiplication by 0.0
        if (fabs(m_DeltaXDist) >= fabs(m_DeltaYDist))
            {
            // We use trigonometric function which are more precise
            Length =  cos(CalculateNormalizedTrigoValue(CalculateBearing().GetAngle()) * m_DeltaXDist);
            }
        else
            {
            // We use trigonometric function which are more precise
            Length = sin(CalculateNormalizedTrigoValue(CalculateBearing().GetAngle()) * m_DeltaYDist);
            }
        }

    return (Length);
    }




