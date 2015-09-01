//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DBasicLinear.hpp $
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
inline HGF2DBasicLinear::HGF2DBasicLinear()
    : HGF2DLinear()
    {
    }

/** -----------------------------------------------------------------------------
    Creates a basic linear by specifying the start and end points.
    The interpretation coordinate system is taken from the start point.

    @param pi_rStartPoint The location of the start point of the linear.

    @param pi_rEndPoint The location of the end point of the linear.

    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline HGF2DBasicLinear::HGF2DBasicLinear(const HGF2DPosition& pi_rStartPoint,
                                      const HGF2DPosition& pi_rEndPoint)
    : HGF2DLinear(pi_rStartPoint, pi_rEndPoint)
    {
    }


/** -----------------------------------------------------------------------------
    Copy constructor.
    @param pi_rObj Reference to basic linear to duplicate.

    -----------------------------------------------------------------------------
*/
inline HGF2DBasicLinear::HGF2DBasicLinear(const HGF2DBasicLinear& pi_rObj)
    : HGF2DLinear(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destroyer.
//-----------------------------------------------------------------------------
inline HGF2DBasicLinear::~HGF2DBasicLinear()
    {
    }

/** -----------------------------------------------------------------------------
    Assignment operator.

    @return Reference to self to be used as an l-value.
    -----------------------------------------------------------------------------
*/
inline HGF2DBasicLinear& HGF2DBasicLinear::operator=(const HGF2DBasicLinear& pi_rBasicLinear)
    {
    // Invoque ancester operator
    HGF2DLinear::operator=(pi_rBasicLinear);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// IsABasicLinear
// Indicates if the linear is a basic linear
//-----------------------------------------------------------------------------
inline bool HGF2DBasicLinear::IsABasicLinear() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// IsComplex
// Indicates if the present linear is complex
//-----------------------------------------------------------------------------
inline bool HGF2DBasicLinear::IsComplex() const
    {
    return (false);
    }


END_IMAGEPP_NAMESPACE