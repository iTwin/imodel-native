//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DShape::HGF2DShape()
    : HGF2DVector()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DShape object.
//-----------------------------------------------------------------------------
inline HGF2DShape::HGF2DShape(const HGF2DShape& pi_rObj)
    : HGF2DVector(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DShape::~HGF2DShape()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another shape object.
//-----------------------------------------------------------------------------
inline HGF2DShape& HGF2DShape::operator=(const HGF2DShape& pi_rObj)
    {
    HGF2DVector::operator=(pi_rObj);

    // Return reference to self
    return(*this);
    }


/** -----------------------------------------------------------------------------
    This method calculates and returns the spatial position of given
    location relative to the area defined by the
    shape. There are three possible answer resulting from
    operation: (HGF2DShape::S_OUT, HGF2DShape::S_IN, HGF2DShape::S_ON).
    The value HGF2DShape::S_PARTIALY_IN is of course impossible.
    The first one indicates that the given location
    is located outside the area defined by the shape.
    If HGF2DShape::S_IN is returned, then the location is located completely
    inside the area enclosed by the shape. Finally HGF2DShape::S_ON
    is returned is the path of the given location is
    located on the shape boundary.

    @param pi_rLocation The HGF2DPosition of which the spatial
                        position of must be determined.

    @return The spatial position of location.

    Example:
    @code
    @end

    @see HGF2DPosition
    @see Locate()
    @see IsPointIn()
    @see IsPointOn()
    -----------------------------------------------------------------------------
*/
inline HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOf(
    const HGF2DPosition& pi_rPoint) const
    {
    return(IsPointOn(pi_rPoint) ? S_ON : (IsPointIn(pi_rPoint) ? S_IN : S_OUT));
    }

//-----------------------------------------------------------------------------
// IsAtAnExtremity
// Determines if the given point is located at an extremity
//-----------------------------------------------------------------------------
inline bool HGF2DShape::IsAtAnExtremity(const HGF2DPosition& pi_rLocation,
                                      double pi_Tolerance) const
    {
    // A shape has no extremity
    return(false);
    }

//-----------------------------------------------------------------------------
// GetMainVectorType
// Returns the vector type
//-----------------------------------------------------------------------------
inline HGF2DVectorTypeId HGF2DShape::GetMainVectorType() const
    {
    return(HGF2DShape::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the shape is null (no length)
//-----------------------------------------------------------------------------
inline bool HGF2DShape::IsNull() const
    {
    return(IsEmpty());
    }

//-----------------------------------------------------------------------------
// public
// Locate
//-----------------------------------------------------------------------------
inline HGF2DVector::Location HGF2DShape::Locate(const HGF2DPosition& pi_rPoint) const
    {
    SpatialPosition MyPosition = CalculateSpatialPositionOf(pi_rPoint);

    return((MyPosition == S_ON) ?  S_ON_BOUNDARY : ((MyPosition == S_IN) ? S_INSIDE : S_OUTSIDE));
    }

END_IMAGEPP_NAMESPACE