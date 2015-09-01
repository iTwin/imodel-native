//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DBasicLinear.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default Constructor for a basic linear
    The interpretation coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
inline HVE2DBasicLinear::HVE2DBasicLinear()
    : HVE2DLinear()
    {
    }

/** -----------------------------------------------------------------------------
    Creates a basic linear by specifying the start and end points.
    The interpretation coordinate system is taken from the start point.

    @param pi_rStartPoint The location of the start point of the linear.

    @param pi_rEndPoint The location of the end point of the linear.

    @see HGF2DLocation
    -----------------------------------------------------------------------------
*/
inline HVE2DBasicLinear::HVE2DBasicLinear(const HGF2DLocation& pi_rStartPoint,
                                          const HGF2DLocation& pi_rEndPoint)
    : HVE2DLinear(pi_rStartPoint, pi_rEndPoint)
    {
    }

/** -----------------------------------------------------------------------------
    Creates a basic linear by specifying the start and end points.
    The interpretation coordinate system is the provided coordinate system.

    @param pi_rStartPoint The location of the start point of the linear.

    @param pi_rEndPoint The location of the end point of the linear.

    @param pi_rpCoordSys Reference to smart pointer to interpretation
                         coordinate system. This pointer may not be null.

    @see HGF2DLocation
    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HVE2DBasicLinear::HVE2DBasicLinear (const HGF2DPosition& pi_rStartPoint,
                                           const HGF2DPosition& pi_rEndPoint,
                                           const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DLinear(pi_rStartPoint, pi_rEndPoint, pi_rpCoordSys)
    {
    }



/** -----------------------------------------------------------------------------
    Creates a null basic linear
    The interpretation coordinate system is the provided coordinate system.

    @param pi_rpCoordSys Reference to smart pointer to interpretation
                         coordinate system. This pointer may not be null.

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HVE2DBasicLinear::HVE2DBasicLinear(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DLinear(pi_rpCoordSys)
    {
    }

/** -----------------------------------------------------------------------------
    Copy constructor.
    @param pi_rObj Reference to basic linear to duplicate.

    -----------------------------------------------------------------------------
*/
inline HVE2DBasicLinear::HVE2DBasicLinear(const HVE2DBasicLinear& pi_rObj)
    : HVE2DLinear(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destroyer.
//-----------------------------------------------------------------------------
inline HVE2DBasicLinear::~HVE2DBasicLinear()
    {
    }

/** -----------------------------------------------------------------------------
    Assignment operator.

    @return Reference to self to be used as an l-value.
    -----------------------------------------------------------------------------
*/
inline HVE2DBasicLinear& HVE2DBasicLinear::operator=(const HVE2DBasicLinear& pi_rBasicLinear)
    {
    // Invoque ancester operator
    HVE2DLinear::operator=(pi_rBasicLinear);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// IsABasicLinear
// Indicates if the linear is a basic linear
//-----------------------------------------------------------------------------
inline bool HVE2DBasicLinear::IsABasicLinear() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// IsComplex
// Indicates if the present linear is complex
//-----------------------------------------------------------------------------
inline bool HVE2DBasicLinear::IsComplex() const
    {
    return (false);
    }


END_IMAGEPP_NAMESPACE
