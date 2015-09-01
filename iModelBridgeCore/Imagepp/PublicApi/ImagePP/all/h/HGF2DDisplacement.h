//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DDisplacement.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DDisplacement
//-----------------------------------------------------------------------------
// Delta of location in two-dimensional coordinate systems.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

#include "HGF2DPosition.h"
#include "HGFBearing.h"

BEGIN_IMAGEPP_NAMESPACE
//typedef BentleyApi::ImagePP::HGF2DCoord<double> HGF2DPosition;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a displacement.  Displacements
    may be converted from one to the other. The same operations can be performed
    on a displacement as any other one dimension scalar entities. A displacement
    does not in any way understand nor use the concept of a coordinate system.
    A displacement object can however be used with object of types that do,
    such as HGF2DLocation objects.

    A displacement is a relative object. It requires to be used in the proper
    context in order to be properly interpreted, just like distances. Be careful
    when using such objects in conjunction with other absolute position
    objects (HGF2DLocation objects for example). Even if a displacement can be
    seen as a displacement vector, this vector does not have a position, and
    therefore transforming a displacement or changing its context of use if
    entirely left to the user.

    A displacement can be defined in two different ways. Either as two X
    and Y distance offsets, or as a bearing and a distance.

    IMPORTANT : The present class does not have a destructor. The class has
    only basic type members and members which do not have a destructors
    themselves. So no destructors has been implemented to prevent calling
    of this destructor which did nothing anyway. It follows that there should
    be no descendants from the present class since none could have a destructor.
    -----------------------------------------------------------------------------
*/
class HGF2DDisplacement
    {
    HDECLARE_SEALEDCLASS_ID(HGF2DDisplacementId_Base)

public:

    // Primary methods
                                HGF2DDisplacement();
                                HGF2DDisplacement(const HGFBearing&   pi_rDirection,
                                                  double              pi_rDistance);
                                HGF2DDisplacement(double  pi_rXOffset,
					                              double  pi_rYOffset);
                                HGF2DDisplacement(const HGF2DDisplacement& pi_rObj);
                                ~HGF2DDisplacement();
    HGF2DDisplacement&          operator=(const HGF2DDisplacement& pi_rObj);

    // Logical operations
    bool                        operator==(const HGF2DDisplacement& pi_rObj) const;
    bool                        operator!=(const HGF2DDisplacement& pi_rObj) const;

    // Displacement management
    double                      GetDeltaX() const;
    double                      GetDeltaY() const;
    void                        SetDeltaX(double pi_rNewDeltaX);
    void                        SetDeltaY(double pi_rNewDeltaY);
    IMAGEPP_EXPORT HGFBearing    CalculateBearing() const;
    IMAGEPP_EXPORT double        CalculateLength() const;
    void                        SetByBearing(const HGFBearing&  pi_rDirection,
                                             double             pi_NewLength);
    void                        SetBearing(const HGFBearing& pi_rDirection);
    void                        SetLength(double pi_NewLength);

    // Operations
    HGF2DDisplacement           operator+(const HGF2DDisplacement& pi_rDisp) const;
    HGF2DDisplacement           operator-(const HGF2DDisplacement& pi_rDisp) const;
    HGF2DDisplacement           operator-() const;
    HGF2DDisplacement&          operator+=(const HGF2DDisplacement& pi_rDisp);
    HGF2DDisplacement&          operator-=(const HGF2DDisplacement& pi_rDisp);

    HGF2DDisplacement           operator*(double pi_Multipli) const;
    HGF2DDisplacement           operator/(double pi_Divisor)const;
    HGF2DDisplacement&          operator*=(double pi_Multipli);
    HGF2DDisplacement&          operator/=(double pi_Divisor);


    friend HGF2DDisplacement    operator*(double   pi_Multipli,
                                          const HGF2DDisplacement& pi_rDisp);


protected:

private:

    // Private methods
    void                        Copy(const HGF2DDisplacement& pi_rObj);

    // Coordinates of this location
    double     m_DeltaXDist;
    double     m_DeltaYDist;
    };


inline HGF2DPosition& operator+=(HGF2DPosition& position, const HGF2DDisplacement& displacement)
    {
    position.SetX(position.GetX() + displacement.GetDeltaX());
    position.SetY(position.GetY() + displacement.GetDeltaY());
    return position;
    }

inline HGF2DPosition operator+(const HGF2DPosition& position, const HGF2DDisplacement& displacement)
    {
    return HGF2DPosition(position.GetX() + displacement.GetDeltaX(), position.GetY() + displacement.GetDeltaY());
    }

inline HGF2DDisplacement operator-(const HGF2DPosition& pi_rFirstPoint, const HGF2DPosition& pi_rSecondPoint)
    {
    HGF2DDisplacement result(pi_rFirstPoint.GetX() - pi_rSecondPoint.GetX(), pi_rFirstPoint.GetY()- pi_rSecondPoint.GetY());
    return result;
    }

END_IMAGEPP_NAMESPACE
#include "HGF2DDisplacement.hpp"
