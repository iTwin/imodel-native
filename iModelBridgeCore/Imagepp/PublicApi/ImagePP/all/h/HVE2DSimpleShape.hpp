//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DSimpleShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DSimpleShape::HVE2DSimpleShape()
    : HVE2DShape()
    {
    }



//-----------------------------------------------------------------------------
// Constructor with coordinate system
//-----------------------------------------------------------------------------
inline HVE2DSimpleShape::HVE2DSimpleShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DShape(pi_rpCoordSys)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DSimpleShape object.
//-----------------------------------------------------------------------------
inline HVE2DSimpleShape::HVE2DSimpleShape(const HVE2DSimpleShape& pi_rObj)
    : HVE2DShape(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DSimpleShape::~HVE2DSimpleShape()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another simple shape object.
//-----------------------------------------------------------------------------
inline HVE2DSimpleShape& HVE2DSimpleShape::operator=(const HVE2DSimpleShape& pi_rObj)
    {
    HVE2DShape::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// IsSimple
// This method returns true since a simple shape may not be complex
//-----------------------------------------------------------------------------
inline bool   HVE2DSimpleShape::IsSimple() const
    {
    return (true);
    }


//-----------------------------------------------------------------------------
// IsComplex
// This method returns false since a simple shape may not be complex
//-----------------------------------------------------------------------------
inline bool   HVE2DSimpleShape::IsComplex() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// GetShapeList
// The method returns a constant reference to the list of shapes
// Since there are no such list of shapes in simple shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HVE2DShape::ShapeList& HVE2DSimpleShape::GetShapeList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HVE2DShape::ShapeList* pPtr = 0;
    return (*pPtr);
    }

//-----------------------------------------------------------------------------
// HasHoles
// This method indicates if there are any holes in the shape.
// Since a polygon does not contain any holes, false is always returned.
//-----------------------------------------------------------------------------
inline bool   HVE2DSimpleShape::HasHoles() const
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// GetHoleList
// The method returns a constant reference to the list of holes
// Since there are no such list of holes in simple shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HVE2DShape::HoleList& HVE2DSimpleShape::GetHoleList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HVE2DShape::HoleList* pPtr = 0;
    return (*pPtr);
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of simple shape in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HVE2DSimpleShape::Drop(HGF2DLocationCollection* po_pPoints,
                                   double                   pi_Tolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // This , however non-performant, will work for any simple shape
    GetLinear().Drop(po_pPoints, pi_Tolerance, HVE2DLinear::INCLUDE_END_POINT);
    }

END_IMAGEPP_NAMESPACE
