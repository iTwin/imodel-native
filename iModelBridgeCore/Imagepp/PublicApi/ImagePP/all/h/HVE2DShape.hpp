//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DShape::HVE2DShape()
    : HVE2DVector()
    {
    }

//-----------------------------------------------------------------------------
// Constructor (with default coordinate system)
//-----------------------------------------------------------------------------
inline HVE2DShape::HVE2DShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DVector(pi_rpCoordSys)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DShape object.
//-----------------------------------------------------------------------------
inline HVE2DShape::HVE2DShape(const HVE2DShape& pi_rObj)
    : HVE2DVector(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DShape::~HVE2DShape()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another shape object.
//-----------------------------------------------------------------------------
inline HVE2DShape& HVE2DShape::operator=(const HVE2DShape& pi_rObj)
    {
    HVE2DVector::operator=(pi_rObj);

    // Return reference to self
    return(*this);
    }


/** -----------------------------------------------------------------------------
    This method calculates and returns the spatial position of given
    location relative to the area defined by the
    shape. There are three possible answer resulting from
    operation: (HVE2DShape::S_OUT, HVE2DShape::S_IN, HVE2DShape::S_ON).
    The value HVE2DShape::S_PARTIALY_IN is of course impossible.
    The first one indicates that the given location
    is located outside the area defined by the shape.
    If HVE2DShape::S_IN is returned, then the location is located completely
    inside the area enclosed by the shape. Finally HVE2DShape::S_ON
    is returned is the path of the given location is
    located on the shape boundary.

    @param pi_rLocation The HGF2DLocation of which the spatial
                        position of must be determined.

    @return The spatial position of location.

    Example:
    @code
    @end

    @see HGF2DLocation
    @see Locate()
    @see IsPointIn()
    @see IsPointOn()
    -----------------------------------------------------------------------------
*/
inline HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOf(
    const HGF2DLocation& pi_rPoint) const
    {
    return(IsPointOn(pi_rPoint) ? S_ON : (IsPointIn(pi_rPoint) ? S_IN : S_OUT));
    }

//-----------------------------------------------------------------------------
// IsAtAnExtremity
// Determines if the given point is located at an extremity
//-----------------------------------------------------------------------------
inline bool HVE2DShape::IsAtAnExtremity(const HGF2DLocation& pi_rLocation,
                                         double pi_Tolerance) const
    {
    // A shape has no extremity
    return(false);
    }

//-----------------------------------------------------------------------------
// GetMainVectorType
// Returns the vector type
//-----------------------------------------------------------------------------
inline HVE2DVectorTypeId HVE2DShape::GetMainVectorType() const
    {
    return(HVE2DShape::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the shape is null (no length)
//-----------------------------------------------------------------------------
inline bool HVE2DShape::IsNull() const
    {
    return(IsEmpty());
    }

//-----------------------------------------------------------------------------
// public
// Locate
//-----------------------------------------------------------------------------
inline HGFGraphicObject::Location HVE2DShape::Locate(const HGF2DLocation& pi_rPoint) const
    {
    SpatialPosition MyPosition = CalculateSpatialPositionOf(pi_rPoint);

    return((MyPosition == S_ON) ?  S_ON_BOUNDARY : ((MyPosition == S_IN) ? S_INSIDE : S_OUTSIDE));
    }

END_IMAGEPP_NAMESPACE
