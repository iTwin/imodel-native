//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DSimpleShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DSimpleShape::HGF2DSimpleShape()
    : HGF2DShape()
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DSimpleShape object.
//-----------------------------------------------------------------------------
inline HGF2DSimpleShape::HGF2DSimpleShape(const HGF2DSimpleShape& pi_rObj)
    : HGF2DShape(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DSimpleShape::~HGF2DSimpleShape()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another simple shape object.
//-----------------------------------------------------------------------------
inline HGF2DSimpleShape& HGF2DSimpleShape::operator=(const HGF2DSimpleShape& pi_rObj)
    {
    HGF2DShape::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// IsSimple
// This method returns true since a simple shape may not be complex
//-----------------------------------------------------------------------------
inline bool   HGF2DSimpleShape::IsSimple() const
    {
    return (true);
    }


//-----------------------------------------------------------------------------
// IsComplex
// This method returns false since a simple shape may not be complex
//-----------------------------------------------------------------------------
inline bool   HGF2DSimpleShape::IsComplex() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// GetShapeList
// The method returns a constant reference to the list of shapes
// Since there are no such list of shapes in simple shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HGF2DShape::ShapeList& HGF2DSimpleShape::GetShapeList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HGF2DShape::ShapeList* pPtr = 0;
    return (*pPtr);
    }

//-----------------------------------------------------------------------------
// HasHoles
// This method indicates if there are any holes in the shape.
// Since a polygon does not contain any holes, false is always returned.
//-----------------------------------------------------------------------------
inline bool   HGF2DSimpleShape::HasHoles() const
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// GetHoleList
// The method returns a constant reference to the list of holes
// Since there are no such list of holes in simple shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HGF2DShape::HoleList& HGF2DSimpleShape::GetHoleList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HGF2DShape::HoleList* pPtr = 0;
    return (*pPtr);
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of simple shape in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HGF2DSimpleShape::Drop(HGF2DPositionCollection* po_pPoints,
                                 double                   pi_rTolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // This , however non-performant, will work for any simple shape
    GetLinear()->Drop(po_pPoints, pi_rTolerance, HGF2DLinear::INCLUDE_END_POINT);
    }

END_IMAGEPP_NAMESPACE