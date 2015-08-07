//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DHoledShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DHoledShape
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HGF2DHoledShape.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>

HPM_REGISTER_CLASS(HVE2DHoledShape, HVE2DShape)


#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>


/** -----------------------------------------------------------------------------
    Default constructor for a holed shape. This constructor creates an empty
    holed shape that does not contain any holes nor has a defined outter base shape.
    The interpretation coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
HVE2DHoledShape::HVE2DHoledShape()
    : HVE2DShape()
    {
    // Set base shape to en empty shape
    m_pBaseShape = new HVE2DVoidShape(GetCoordSys());
    }


/** -----------------------------------------------------------------------------
    Constructor for an empty holed shape. This constructor creates an empty
    holed shape that does not contain any holes nor has a defined outter base shape.
    The interpretation coordinate system is the one provided.

    @param pi_rpCoordSys Reference to smart pointer to interpretation coordinate
                         system to use for the holed shape.

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
HVE2DHoledShape::HVE2DHoledShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DShape(pi_rpCoordSys)
    {
    // Set base shape to an empty shape
    m_pBaseShape = new HVE2DVoidShape(pi_rpCoordSys);
    }


//-----------------------------------------------------------------------------
/** -----------------------------------------------------------------------------
    This method sets the base shape of the holed shape. All holes which may
    have been previously defined are removed. The coordinate system used
    in the interpretation of the holed shape is unchanged.

    @param pi_rBaseShape Constant reference to an HVE2DSimpleShape which
                         specifies the new base shape.

    Example:
    @code
    @end

    @see HVE2DSimpleShape
    -----------------------------------------------------------------------------
*/
void HVE2DHoledShape::SetBaseShape(const HVE2DSimpleShape& pi_rBaseShape)
    {
    // Empty of all holes
    MakeEmpty();

    // Delete previous base shape
    m_pBaseShape = 0;

    // Copy given shape
    m_pBaseShape = (HVE2DSimpleShape*)pi_rBaseShape.AllocateCopyInCoordSys(GetCoordSys());

    // Dispend tolerance setting
    m_pBaseShape->SetAutoToleranceActive(IsAutoToleranceActive());
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the holed shape
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DHoledShape::AllocateCopyInCoordSys(
    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pResultVector;

    // Check if it is allready in appropriate coordinate system
    if (GetCoordSys() == pi_rpCoordSys)
        {
        // Clone it instead since faster
        pResultVector = (HVE2DVector*)Clone();
        }
    else
        {
        // We first allocate a copy of the base shape in coordinate system
        HAutoPtr<HVE2DShape>  pNewOutterShape((HVE2DShape*)(m_pBaseShape->AllocateCopyInCoordSys(
                                                                pi_rpCoordSys)));

        // We then check if there are any holes
        if (m_HoleList.size() > 0)
            {
            // There are some holes ... we generate a complex shape from those holes
            HVE2DComplexShape    HoleComplexShape(m_HoleList);

            // We differentiate this complex shape from new outter shape
            pResultVector = pNewOutterShape->DifferentiateShape(HoleComplexShape);

            // We destroy the temporary outter shape
            delete pNewOutterShape.release();
            }
        else
            {
            // There is no holes ... therefore the new outter shape is the result
            pResultVector = pNewOutterShape.release();
            }

        pResultVector->SetStrokeTolerance(m_pStrokeTolerance);
        }

    return (pResultVector);
    }


/** -----------------------------------------------------------------------------
    This method adds a hole to the holed shape. The given hole must not
    intersect nor be contiguous to any holes neither to the base shape.

    @param pi_rHole Constant reference to an HVE2DSimpleShape, specifying
                    the new hole to add.

    Example:
    @code
    @end

    @see AreContiguous()
    @see Intersect()
    -----------------------------------------------------------------------------
*/
void HVE2DHoledShape::AddHole(const HVE2DSimpleShape& pi_rSimpleShape)
    {
    // The given shape must be in
    HPRECONDITION(CalculateSpatialPositionOf(pi_rSimpleShape) == HVE2DShape::S_IN);

    // The given shape must not be contiguous
    HPRECONDITION(!AreContiguous(pi_rSimpleShape));

    // Create copy of shape to list
    HAutoPtr<HVE2DSimpleShape> pMyNewHole((HVE2DSimpleShape*)pi_rSimpleShape.AllocateCopyInCoordSys(
                                              GetCoordSys()));

    // Set tolerance setting to new hole
    pMyNewHole->SetAutoToleranceActive(IsAutoToleranceActive());

    // Add hole to list
    m_HoleList.push_back(pMyNewHole.release());
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another holed shape.
//-----------------------------------------------------------------------------
HVE2DHoledShape& HVE2DHoledShape::operator=(const HVE2DHoledShape& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Empty the holed shape
        MakeEmpty();

        // Invoque ancester copy
        HVE2DShape::operator=(pi_rObj);

        // Set new outter shape
        m_pBaseShape = (HVE2DSimpleShape*)pi_rObj.m_pBaseShape->Clone();

        // We copy the holes
        for (HVE2DShape::HoleList::const_iterator Itr = pi_rObj.m_HoleList.begin();
             Itr != pi_rObj.m_HoleList.end() ; ++Itr)
            {
            m_HoleList.push_back((HVE2DSimpleShape*)((*Itr)->Clone()));
            }
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// This method sets the interpretation coordinate system
//-----------------------------------------------------------------------------
void HVE2DHoledShape::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Call ancester set
    HVE2DShape::SetCoordSysImplementation(pi_rpCoordSys);

    // Set system of outter shape
    m_pBaseShape->SetCoordSys(GetCoordSys());

    // For each hole in holed polygon
    HVE2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

    // Set coordinate system of every hole
    while (MyIterator != m_HoleList.end())
        {
        (*MyIterator)->SetCoordSys(GetCoordSys());

        ++MyIterator;
        }
    }



//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on holed polygon boundary to given point.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DHoledShape::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    // Find the closest point on outter boundary
    HGF2DLocation       ClosestPoint(m_pBaseShape->CalculateClosestPoint(pi_rPoint));
    HGF2DLocation       WorkPoint(GetCoordSys());
    double              WorkDistance;

    // Compute distance to temporary closest point
    double              TheMinimalDistance((pi_rPoint - ClosestPoint).CalculateLength());

    // For each hole in holed polygon
    HVE2DShape::HoleList::const_iterator  MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Obtain closest point to hole
        WorkPoint = (*MyIterator)->CalculateClosestPoint(pi_rPoint);

        // Calculate distance to new point
        WorkDistance = (pi_rPoint - WorkPoint).CalculateLength();

        // Check if the distance to this point is smaller than previous point
        if (TheMinimalDistance > WorkDistance)
            {
            // This work point is closer ... it becomes the new point
            TheMinimalDistance = WorkDistance;
            ClosestPoint = WorkPoint;
            }

        // Advance to next hole
        ++MyIterator;
        }

    return (ClosestPoint);
    }

//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the holed shape is adjacent with given vector.
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    // Find if the vector is adjacent to the outter boundary
    bool           DoAreAdjacent(m_pBaseShape->AreAdjacent(pi_rVector));

    // If they are not adjacent ... check with holes
    if (!DoAreAdjacent)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one is adjacent
        while (MyIterator != m_HoleList.end() &&
               !(DoAreAdjacent = (*MyIterator)->AreAdjacent(pi_rVector)))
            ++MyIterator;
        }

    return (DoAreAdjacent);
    }



//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the holed shape is contiguous with given vector.
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    // Find if the vector is contiguous the outter boundary
    bool           DoAreContiguous(m_pBaseShape->AreContiguous(pi_rVector));

    // If they are not contiguous ... check with holes
    if (!DoAreContiguous)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one is contiguous
        while (MyIterator != m_HoleList.end() &&
               !(DoAreContiguous = (*MyIterator)->AreContiguous(pi_rVector)))
            ++MyIterator;
        }

    return (DoAreContiguous);
    }



//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the holed shape is contiguous with given vector
// at specified position
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::AreContiguousAt(const HVE2DVector& pi_rVector,
                                       const HGF2DLocation& pi_rPoint) const
    {
    // The point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool DoAreContiguous=false;

    // Obtain tolerance
    double Tolerance = min(GetTolerance(), pi_rVector.GetTolerance());

    // Find if the point is on the outter boundary
    if (m_pBaseShape->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
        {
        // The point is on aoutter boundary ... check if contiguous
        DoAreContiguous = m_pBaseShape->AreContiguousAt(pi_rVector, pi_rPoint);
        }

    // If it was not contiguous at specified point on outter shape ... check with holes
    if (!DoAreContiguous)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one is contiguous at specified point
        while (MyIterator != m_HoleList.end() && !(DoAreContiguous))
            {
            // Check if point is on hole
            if ((*MyIterator)->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                // The point is located on hole boundary ... check if contiguous
                DoAreContiguous = (*MyIterator)->AreContiguousAt(pi_rVector, pi_rPoint);
                }

            ++MyIterator;
            }
        }

    return (DoAreContiguous);
    }



//-----------------------------------------------------------------------------
// Intersect
// This method checks if the holed shape intersects with given and
// returns the cross points
//-----------------------------------------------------------------------------
size_t HVE2DHoledShape::Intersect(const HVE2DVector& pi_rVector,
                                  HGF2DLocationCollection* po_pCrossPoints) const
    {
    HPRECONDITION(po_pCrossPoints != 0);

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pCrossPoints->size();

    // Find the intersection points with base shape
    m_pBaseShape->Intersect(pi_rVector, po_pCrossPoints);

    // For each hole in holed polygon
    HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Add intersection points with hole
        (*MyIterator)->Intersect(pi_rVector, po_pCrossPoints);

        ++MyIterator;
        }

    // Return number of new points
    return (po_pCrossPoints->size() - InitialNumberOfPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method checks if the holed shape intersects with given and
// returns the cross points
//-----------------------------------------------------------------------------
size_t HVE2DHoledShape::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                   HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The two objects must be contiguous
    HPRECONDITION(AreContiguous(pi_rVector));

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Find the contiguousness points with base shape (if contiguous)
    if (m_pBaseShape->AreContiguous(pi_rVector))
        m_pBaseShape->ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints);

    // For each hole in holed polygon
    HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Find the contiguousness points with hole (if contiguous)
        if ((*MyIterator)->AreContiguous(pi_rVector))
            (*MyIterator)->ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints);

        ++MyIterator;
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method obtains the two contiguousness points of which given
// point is a part of
//-----------------------------------------------------------------------------
void HVE2DHoledShape::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                   const HGF2DLocation& pi_rPoint,
                                                   HGF2DLocation* po_pFirstContiguousnessPoint,
                                                   HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The two objects must be contiguous
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Obtain tolerance
    double Tolerance = min(GetTolerance(), pi_rVector.GetTolerance());

    // If the given point is on base shape and they are contiguous at this point ...
    if (m_pBaseShape->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
        AreContiguousAt(pi_rVector, pi_rPoint))
        {
        // Find the contiguousness points with base shape
        m_pBaseShape->ObtainContiguousnessPointsAt(pi_rVector,
                                                   pi_rPoint,
                                                   po_pFirstContiguousnessPoint,
                                                   po_pSecondContiguousnessPoint);
        }
    else
        {
        // The base shape is not contiguous at point...
        bool ContiguousnessPointsFound = false;

        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or contiguousness points are found
        while (MyIterator != m_HoleList.end() && !(ContiguousnessPointsFound))
            {
            // If the given point is on hole and they are contiguous at this point ...
            if((*MyIterator)->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
               (*MyIterator)->AreContiguousAt(pi_rVector, pi_rPoint))
                {
                // Find the contiguousness points with hole
                (*MyIterator)->ObtainContiguousnessPointsAt(pi_rVector,
                                                            pi_rPoint,
                                                            po_pFirstContiguousnessPoint,
                                                            po_pSecondContiguousnessPoint);

                // Indicate the contiguousness points were found
                ContiguousnessPointsFound = true;
                }

            ++MyIterator;
            }
        }
    }



#if (0)
//-----------------------------------------------------------------------------
// Flirts
// This method checks if the holed shape flirts with given vector.
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::Flirts(const HVE2DVector& pi_rVector) const
    {
    // Find if the vector flirts the outter boundary
    bool           DoesFlirt(m_pBaseShape->Flirts(pi_rVector));

    // If they do not flirt ... check with holes
    if (!DoesFlirt)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one flirts
        while (MyIterator != m_HoleList.end() && !(DoesFlirt = (*MyIterator)->Flirts(pi_rVector)))
            ++MyIterator;
        }

    return (DoesFlirt);
    }
#endif


//-----------------------------------------------------------------------------
// Crosses
// This method checks if the holed shape crosses with given vector.
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::Crosses(const HVE2DVector& pi_rVector) const
    {
    // Find if the point crosses the outter boundary
    bool           IsCrossing(m_pBaseShape->Crosses(pi_rVector));

    // If it does not cross ... check with holes
    if (!IsCrossing)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one crossed
        while (MyIterator != m_HoleList.end() && !(IsCrossing = (*MyIterator)->Crosses(pi_rVector)))
            ++MyIterator;
        }

    return (IsCrossing);
    }


//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the holed shape boundary
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::IsPointOn(const HGF2DLocation& pi_rPoint,
                                 HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                 double pi_Tolerance) const
    {
    // The extremity processing parameter is ignored since a holed shape has no extremity

    // Find out if the point is on the outter boundary
    bool   IsOn(m_pBaseShape->IsPointOn(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance));

    // If it is not on ... check with holes
    if (!IsOn)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or or the point is on a hole boundary
        while (MyIterator != m_HoleList.end() && !(IsOn = (*MyIterator)->IsPointOn(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance)))
            ++MyIterator;
        }

    return (IsOn);
    }


//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                    HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                    double pi_Tolerance) const
    {
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    // The extremity processing parameter is ignored since a holed shape has no extremity

    // Find out if the point is on the outter boundary
    bool   IsOn(m_pBaseShape->IsPointOnSCS(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance));

    // If it is not on ... check with holes
    if (!IsOn)
        {
        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or or the point is on a hole boundary
        while (MyIterator != m_HoleList.end() && !(IsOn = (*MyIterator)->IsPointOnSCS(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance)))
            ++MyIterator;
        }

    return (IsOn);
    }



//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method create a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape> pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, the result is given
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, the result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // We separate the process of differentiation depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.DifferentiateShapeSCS(*this);
        }
    else
        {
        // Either the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = DifferentiateFromSimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = DifferentiateFromHoledShapeSCS((*(HVE2DHoledShape*)(&pi_rShape)));
            }
        }

    return (pMyResultShape.release());
    }



//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape> pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, then result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, the result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    // We separate the process of differentiation depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.DifferentiateFromShapeSCS(*this);
        }
    else
        {
        // Either if the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = DifferentiateSimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = DifferentiateHoledShapeSCS((*(HVE2DHoledShape*)(&pi_rShape)));
            }
        }

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape> pMyResultShape;

    if (IsEmpty() || pi_rShape.IsEmpty())
        {
        // Since at least one of the shape is empty, result is also empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // We separate the process of intersection depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.IntersectShapeSCS(*this);
        }
    else
        {
        // Either the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = IntersectSimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = IntersectHoledShapeSCS((*(HVE2DHoledShape*)(&pi_rShape)));
            }
        }

    return (pMyResultShape.release());
    }



//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape> pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, the result is given
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty ... result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    // We separate the process of unification depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.UnifyShapeSCS(*this);
        }
    else
        {
        // Either the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = UnifySimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = UnifyHoledShapeSCS((*(HVE2DHoledShape*)(&pi_rShape)));
            }
        }

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the holed shape
//-----------------------------------------------------------------------------
double HVE2DHoledShape::CalculateArea() const
    {
    // Compute the outter polygon area
    double     MyTotalArea(m_pBaseShape->CalculateArea());

    // For each hole in holed polygon
    HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Remove from area the area occupied by hole
        MyTotalArea -= (*MyIterator)->CalculateArea();

        ++MyIterator;
        }

    return(MyTotalArea);
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the holed shape
//-----------------------------------------------------------------------------
double HVE2DHoledShape::CalculatePerimeter() const
    {
    // Compute the outter perimeter
    double     MyTotalPerimeter(m_pBaseShape->CalculatePerimeter());

    // For each hole in holed polygon
    HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Add hole perimeter to total perimeter
        MyTotalPerimeter += (*MyIterator)->CalculatePerimeter();

        ++MyIterator;
        }

    return(MyTotalPerimeter);
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the holed shape
//-----------------------------------------------------------------------------
bool HVE2DHoledShape::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    // Set Tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // Find out if the point is in the outter boundary
    bool   IsIn(m_pBaseShape->IsPointIn(pi_rPoint, Tolerance));

    // If it is IN ... check if it is outside of the holes area
    if (IsIn)
        {
        // For each hole in holed shape
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or the point is in or on one
        while (MyIterator != m_HoleList.end() && IsIn)
            {
            IsIn = !(*MyIterator)->IsPointIn(pi_rPoint, Tolerance) &&
                   !(*MyIterator)->IsPointOn(pi_rPoint, INCLUDE_EXTREMITIES, Tolerance);

            ++MyIterator;
            }
        }

    return (IsIn);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the holed shape
//-----------------------------------------------------------------------------
void HVE2DHoledShape::MakeEmpty()
    {
    // Empty the outter shape
    m_pBaseShape->MakeEmpty();

    // For each hole in holed polygon
    HVE2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Destroy hole
        delete *MyIterator;

        ++MyIterator;
        }

    // Clear list of holes
    m_HoleList.clear();
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the holed shape by the specified displacement
//-----------------------------------------------------------------------------
void HVE2DHoledShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // Move the base shape
    m_pBaseShape->Move(pi_rDisplacement);

    // For each hole in holed polygon
    HVE2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Move Hole
        (*MyIterator)->Move(pi_rDisplacement);

        ++MyIterator;
        }
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the holed shape by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DHoledShape::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The scale factor must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // We first scale the base shape
    m_pBaseShape->Scale(pi_ScaleFactor, pi_rScaleOrigin);

    // For each hole in holed polygon
    HVE2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

    // Loop till all holes have been processed
    while (MyIterator != m_HoleList.end())
        {
        // Scale Hole
        (*MyIterator)->Scale(pi_ScaleFactor, pi_rScaleOrigin);

        // Advance to next hole
        ++MyIterator;
        }
    }



//-----------------------------------------------------------------------------
// CalculateBearing
// This method returns the bearing at specified point
//-----------------------------------------------------------------------------
HGFBearing HVE2DHoledShape::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                             HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on vector
    HPRECONDITION(IsPointOn(pi_rPoint));

    HGFBearing  ReturnValue;

    // Find if the point is on the outter boundary
    if (m_pBaseShape->IsPointOn(pi_rPoint))
        {
        // Point is on outter shape ... obtain bearing
        ReturnValue = m_pBaseShape->CalculateBearing(pi_rPoint, pi_Direction);
        }
    else
        // If point is not on ... check with holes
        {
        bool       ReturnValueSet = false;

        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till the bearing is found
        while (!ReturnValueSet)
            {
            // Check if point is on
            if ((*MyIterator)->IsPointOn(pi_rPoint))
                {
                // Point is on hole ... obtain bearing
                ReturnValue = (*MyIterator)->CalculateBearing(pi_rPoint, pi_Direction);

                // Indicate bearing was found
                ReturnValueSet = true;
                }

            ++MyIterator;

            // The value must have been obtained if at end of list
            HASSERT(ReturnValueSet || (MyIterator != m_HoleList.end()));
            }
        }

    return (ReturnValue);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// This method returns the angular acceleration at specified point
//-----------------------------------------------------------------------------
double HVE2DHoledShape::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                     HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on vector
    HPRECONDITION(IsPointOn(pi_rPoint));

    double  ReturnValue=0.0;

    // Find if the point is on the outter boundary
    if (m_pBaseShape->IsPointOn(pi_rPoint))
        {
        // Point is on outter shape ... obtain angular acceleration
        ReturnValue = m_pBaseShape->CalculateAngularAcceleration(pi_rPoint, pi_Direction);
        }
    else
        // If it is not on ... check with holes
        {
        bool                   ReturnValueSet = false;

        // For each hole in holed polygon
        HVE2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one is touched
        while (!ReturnValueSet)
            {
            // Check if point is on hole
            if ((*MyIterator)->IsPointOn(pi_rPoint))
                {
                // Point is on hole ... obtain angular acceleration
                ReturnValue = (*MyIterator)->CalculateAngularAcceleration(pi_rPoint, pi_Direction);

                // Indicate acceleration was found
                ReturnValueSet = true;
                }

            ++MyIterator;

            // The value must have been obtained if at end of list
            HASSERT(ReturnValueSet || (MyIterator != m_HoleList.end()));
            }
        }

    return (ReturnValue);
    }

//-----------------------------------------------------------------------------
// SetAutoToleranceActive
// Sets the auto tolerance active to the components
//-----------------------------------------------------------------------------
void HVE2DHoledShape::SetAutoToleranceActive(bool pi_AutoToleranceActive)
    {
    // Set auto tolerance of outter shape
    m_pBaseShape->SetAutoToleranceActive(pi_AutoToleranceActive);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HVE2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Set auto tolerance of hole
            (*HoleItr)->SetAutoToleranceActive(pi_AutoToleranceActive);
            }
        }

    // Call ancester
    HVE2DVector::SetAutoToleranceActive(pi_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance to the component
//-----------------------------------------------------------------------------
void HVE2DHoledShape::SetTolerance(double pi_Tolerance)
    {
    // The tolerance may not be null of negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // Set tolerance of outter shape
    m_pBaseShape->SetTolerance(pi_Tolerance);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HVE2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Set tolerance of hole
            (*HoleItr)->SetTolerance(pi_Tolerance);
            }
        }

    // Call ancester
    HVE2DVector::SetTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetTolerance
// Sets the stroke tolerance to the component
//-----------------------------------------------------------------------------
void HVE2DHoledShape::SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance)
    {
    // Set tolerance of outter shape
    m_pBaseShape->SetStrokeTolerance(pi_Tolerance);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HVE2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Set tolerance of hole
            (*HoleItr)->SetStrokeTolerance(pi_Tolerance);
            }
        }

    // Call ancester
    HVE2DVector::SetStrokeTolerance(pi_Tolerance);
    }



//-----------------------------------------------------------------------------
// UnifySimpleShapeSCS
// PRIVATE
// This method creates a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::UnifySimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    // The process requires that we first merge the outter shapes
    // This process may well result in a complex shape or a holed shape
    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_OUT &&
        !AreContiguous(pi_rSimpleShape))
        {
        // The two shapes are disjoint
        // completely disjoint
        HAutoPtr<HVE2DComplexShape> pMyResultComplexShape(new HVE2DComplexShape(GetCoordSys()));

        // Add two shapes
        pMyResultComplexShape->AddShape(pi_rSimpleShape);
        pMyResultComplexShape->AddShape(*this);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // We unify the shapes (result is either simple shape or holed shape)
        pMyResultShape = GetBaseShape().UnifyShapeSCS(pi_rSimpleShape);

        HASSERT(!pMyResultShape->IsComplex());


        // The second part consist in differentiating from the holes of self shape the base shape of the given shape.

        // We check if self has holes
        if (HasHoles())
            {
            HAutoPtr<HVE2DHoledShape> pNewHoledShape;

            // First check that previous result is not a simple shape !
            if (pMyResultShape->IsSimple())
                {
                // We must transform simple shape into a holed shape
                pNewHoledShape = new HVE2DHoledShape(*((HVE2DSimpleShape*)pMyResultShape.get()));

                delete pMyResultShape.release();
                }
            else
                pNewHoledShape = (HVE2DHoledShape*)pMyResultShape.release();

            // For each and every holes ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MyHoleIterator;

            for (MyHoleIterator = m_HoleList.begin() ; MyHoleIterator != m_HoleList.end() ; ++MyHoleIterator)
                {
                // Obtain spatial of hole relative to given shape
                HVE2DShape::SpatialPosition HolePosition = pi_rSimpleShape.CalculateSpatialPositionOf(**MyHoleIterator);

                // Check if OUT
                if ((HolePosition == HVE2DShape::S_OUT) &&
                    !((*MyHoleIterator)->AreContiguous(pi_rSimpleShape)))
                    {
                    // We already know from above that simple shape cannot be IN hole
                    HASSERT((*MyHoleIterator)->CalculateSpatialPositionOf(pi_rSimpleShape) != HVE2DShape::S_IN);

                    // The base shape and current hole are disjoint ... we add hole
                    pNewHoledShape->AddHole(**MyHoleIterator);
                    }
                else if ((HolePosition == HVE2DShape::S_PARTIALY_IN) ||
                         ((HolePosition == HVE2DShape::S_OUT) && (*MyHoleIterator)->AreContiguous(pi_rSimpleShape)))
                    {
                    // NOTE that when IN or ON the hole is not part of result ...
                    // here we have PARTIALLY_IN (or contiguous)
                    // We must differentiate simple shape from hole
                    HAutoPtr<HVE2DShape> pTempResult((*MyHoleIterator)->DifferentiateShapeSCS(pi_rSimpleShape));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HVE2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HVE2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HVE2DSimpleShape*)(*MyComplexIterator)));
                                }
                            }
                        }

                    delete pTempResult.release();
                    }
                }

            // If the holed shape has no holes then we revert it back to a simple shape
            // otherwise it is returned
            if (!pNewHoledShape->HasHoles())
                pMyResultShape = static_cast<HVE2DShape*>(pNewHoledShape->GetBaseShape().Clone());
            else
                pMyResultShape = pNewHoledShape.release();
            }

        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// UnifyHoledShapeSCS
// PRIVATE
// This method creates a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::UnifyHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const
    {
    // The shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rHoledShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    // The process requires that we first merge the outter shapes
    // This process may well result in a complex shape or a holed shape
    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rHoledShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rHoledShape);

    if (MySelfPosition == HVE2DShape::S_OUT &&
        MyGivenPosition == HVE2DShape::S_OUT && !AreContiguous(pi_rHoledShape))
        {
        // The two shapes are disjoint
        // completely disjoint
        HAutoPtr<HVE2DComplexShape> pMyResultComplexShape(new HVE2DComplexShape(GetCoordSys()));

        // Add two shapes
        pMyResultComplexShape->AddShape(pi_rHoledShape);
        pMyResultComplexShape->AddShape(*this);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // We unify the shapes (result is either simple shape or holed shape)
        pMyResultShape = GetBaseShape().UnifyShapeSCS(pi_rHoledShape.GetBaseShape());

        HASSERT(!pMyResultShape->IsComplex());

        HAutoPtr<HVE2DHoledShape> pNewHoledShape;

        // First check that previous result is not a simple shape !
        if (pMyResultShape->IsSimple())
            {
            // We must transform simple shape into a holed shape
            pNewHoledShape = new HVE2DHoledShape(*((HVE2DSimpleShape*)pMyResultShape.get()));

            delete pMyResultShape.release();

            }
        else
            pNewHoledShape = (HVE2DHoledShape*)pMyResultShape.release();



        // The second part consist in differentiating from the holes of self shape the base shape of the given shape.

        // We check if self has holes
        if (HasHoles())
            {
            // For each and every holes ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MyHoleIterator;

            for (MyHoleIterator = m_HoleList.begin() ; MyHoleIterator != m_HoleList.end() ; ++MyHoleIterator)
                {
                // Obtain spatial of hole relative to given holed shape base
                HVE2DShape::SpatialPosition HolePosition = pi_rHoledShape.GetBaseShape().CalculateSpatialPositionOf(**MyHoleIterator);

                // Check if OUT
                if (HolePosition == HVE2DShape::S_OUT)
                    {
                    // We already know from above that simple shape cannot be IN hole
                    HASSERT((*MyHoleIterator)->CalculateSpatialPositionOf(pi_rHoledShape.GetBaseShape()) != HVE2DShape::S_IN);

                    // The base shape and current hole are disjoint ... we add hole
                    pNewHoledShape->AddHole(**MyHoleIterator);
                    }
                else if (HolePosition == HVE2DShape::S_PARTIALY_IN)
                    {
                    // NOTE that when IN or ON the hole is not part of result ...
                    // here we have PARTIALLY_IN
                    // We must differentiate simple shape from hole
                    HAutoPtr<HVE2DShape> pTempResult((*MyHoleIterator)->DifferentiateShapeSCS(pi_rHoledShape.GetBaseShape()));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HVE2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HVE2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HVE2DSimpleShape*)(*MyComplexIterator)));
                                }
                            }
                        }

                    delete pTempResult.release();
                    }
                }
            }

        // We check if given has holes
        if (pi_rHoledShape.HasHoles())
            {
            // For each and every holes ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MyHoleIterator;

            for (MyHoleIterator = pi_rHoledShape.GetHoleList().begin() ;
                 MyHoleIterator != pi_rHoledShape.GetHoleList().end() ; ++MyHoleIterator)
                {
                // Obtain spatial of hole relative to simple shape
                HVE2DShape::SpatialPosition HolePosition = GetBaseShape().CalculateSpatialPositionOf(**MyHoleIterator);

                // Check if OUT
                if (HolePosition == HVE2DShape::S_OUT)
                    {
                    // We already know from above that simple shape cannot be IN hole
                    HASSERT((*MyHoleIterator)->CalculateSpatialPositionOf(GetBaseShape()) != HVE2DShape::S_IN);

                    // The base shape and current hole are disjoint ... we add hole
                    pNewHoledShape->AddHole(**MyHoleIterator);
                    }
                else if (HolePosition == HVE2DShape::S_PARTIALY_IN)
                    {
                    // NOTE that when IN or ON the hole is not part of result ...
                    // here we have PARTIALLY_IN
                    // We must differentiate simple shape from hole
                    HAutoPtr<HVE2DShape> pTempResult((*MyHoleIterator)->DifferentiateShapeSCS(GetBaseShape()));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HVE2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HVE2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HVE2DSimpleShape*)(*MyComplexIterator)));
                                }
                            }
                        }

                    delete pTempResult.release();
                    }
                }
            }

        // Next part consist in intersecting the holes of one with holes of the other ...
        if (HasHoles() && pi_rHoledShape.HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ; MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {

                // For each and every holes of given ... we check the spatial position
                HVE2DShape::HoleList::const_iterator MyGivenHoleIterator;

                for (MyGivenHoleIterator = pi_rHoledShape.GetHoleList().begin() ;
                     MyGivenHoleIterator != pi_rHoledShape.m_HoleList.end() ; ++MyGivenHoleIterator)
                    {
                    // Intersect current hole of self with current hole of given
                    HAutoPtr<HVE2DShape> pTempResult((*MySelfHoleIterator)->IntersectShapeSCS(**MyGivenHoleIterator));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HVE2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HVE2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HVE2DSimpleShape*)(*MyComplexIterator)));
                                }
                            }
                        }

                    delete pTempResult.release();

                    }

                }

            }

        pMyResultShape = pNewHoledShape.release();
        }

    return (pMyResultShape.release());

    }

//-----------------------------------------------------------------------------
// IntersectSimpleShapeSCS
// PRIVATE
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::IntersectSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two shapes must have the same coordinate systems
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {

        // The process requires that we first intersect the outter shapes
        // This process may well result in a complex shape
        pMyResultShape = GetBaseShape().IntersectShapeSCS(pi_rSimpleShape);

        // The second part consist in differentiating from the result shape, the holes.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ; MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) != HVE2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShapeSCS(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }
        }

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// IntersectHoledShapeSCS
// PRIVATE
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::IntersectHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const
    {
    // The two shapes must have the same coordinate systems
    HPRECONDITION(GetCoordSys() == pi_rHoledShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rHoledShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rHoledShape);

    if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (MySelfPosition == HVE2DShape::S_ON && MyGivenPosition == HVE2DShape::S_ON)
        {
        // The two shapes are identical ... intersection is either one
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else
        {

        // The process requires that we first intersect the outter shapes
        // This process may well result in a complex shape
        pMyResultShape = GetBaseShape().IntersectShapeSCS(pi_rHoledShape.GetBaseShape());

        // The second part consist in differentiating from the result shape, the holes.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ; MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) != HVE2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShapeSCS(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }

        // We do the same with the holes of given
        if (pi_rHoledShape.HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MyGivenHoleIterator;

            for (MyGivenHoleIterator = pi_rHoledShape.GetHoleList().begin() ;
                 MyGivenHoleIterator != pi_rHoledShape.GetHoleList().end() ; ++MyGivenHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MyGivenHoleIterator) !=
                    HVE2DShape::S_OUT)
                    {

                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShapeSCS(**MyGivenHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }
        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateSimpleShapeSCS
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::DifferentiateSimpleShapeSCS(
    const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two holed shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else if (MySelfPosition == HVE2DShape::S_OUT &&
             MyGivenPosition == HVE2DShape::S_IN && !AreContiguous(pi_rSimpleShape))
        {
        // The simple shape is completely in area of holed ... it is a new hole
        HAutoPtr<HVE2DHoledShape> pMyNewHoledShape((HVE2DHoledShape*)Clone());

        pMyNewHoledShape->AddHole(pi_rSimpleShape);

        pMyResultShape = pMyNewHoledShape.release();
        }
    else if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_ON)
        {
        // Given simple shape is equivalent to one of the holes....
        // Result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else if (MySelfPosition == HVE2DShape::S_OUT &&
             MyGivenPosition == HVE2DShape::S_IN && !(m_pBaseShape->AreContiguous(pi_rSimpleShape)))
        {
        // The simple shape is in but contiguous to one or more of the holes
        // The result can only be a holed shape

        // The current holed must have holes
        HASSERT(HasHoles());

        // Result is holed with the same base shape
        HAutoPtr<HVE2DHoledShape> pResultHoled(new HVE2DHoledShape(*m_pBaseShape));

        // For each and every holes of self ... we check the spatial position
        HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

        // Obtain a clone of simple shape
        HFCPtr<HVE2DShape> pSimpleShape = static_cast<HVE2DSimpleShape*>(pi_rSimpleShape.Clone());

        for (MySelfHoleIterator = m_HoleList.begin() ;
             MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
            {
            // We check if this hole is contiguous to simple shape
            if (pSimpleShape->AreContiguous(**MySelfHoleIterator))
                {
                // The current hole is contiguous to current simple shape

                // Let us unify them
                pSimpleShape = pSimpleShape->UnifyShape(**MySelfHoleIterator);

                // The result should always be simple
                HASSERT(pSimpleShape->IsSimple());
                }
            else if (pSimpleShape->CalculateSpatialPositionOf(**MySelfHoleIterator) == S_OUT)
                {
                // The hole is out of simple shape
                // Note that other posibility only include in or on and in both
                // cases, we simply eliminate the hole.
                // In the present case, we simply add it
                pResultHoled->AddHole(**MySelfHoleIterator);

                }

            }

        // Finaly we add as hole the final simple shape
        pResultHoled->AddHole(*(static_cast<HVE2DSimpleShape*>(&(*pSimpleShape))));

        pMyResultShape = pResultHoled.release();
        }
    else if (MyGivenPosition == HVE2DShape::S_PARTIALY_IN &&
             (m_pBaseShape->CalculateSpatialPositionOf(pi_rSimpleShape) == HVE2DShape::S_IN))
        {
        // The shape to remove is completely in base shape but partially in a hole
        // The result can be a holed shape or surface

        // The current holed must have holes
        HASSERT(HasHoles());

        // Partial result is holed with the same base shape
        HAutoPtr<HVE2DHoledShape> pResultHoled(new HVE2DHoledShape(*m_pBaseShape));

        // For each and every holes of self ... we check the spatial position
        HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

        // Obtain a clone of simple shape
        HFCPtr<HVE2DShape> pShape = static_cast<HVE2DShape*>(pi_rSimpleShape.Clone());

        for (MySelfHoleIterator = m_HoleList.begin() ;
             MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
            {
            // Obtain position of hole to current shape
            HVE2DShape::SpatialPosition ThePosition = pShape->CalculateSpatialPositionOf(**MySelfHoleIterator);

            // Check if this hole is partially in current result
            if (ThePosition == HVE2DShape::S_PARTIALY_IN)
                {
                // The current hole is partially in to current simple shape

                // Let us unify them .. the result can be holed!
                pShape = pShape->UnifyShape(**MySelfHoleIterator);
                }
            else if (ThePosition == S_OUT)
                {
                // The hole is out of simple shape
                // Note that other posibility only include in or on and in both
                // cases, we simply eliminate the hole.
                // In the present case, we simply add it
                pResultHoled->AddHole(**MySelfHoleIterator);
                }
            }

        // Finaly we add as hole the final shape if it is simple
        if (pShape->IsSimple())
            {
            pResultHoled->AddHole(*(static_cast<HVE2DSimpleShape*>(&(*pShape))));

            pMyResultShape = pResultHoled.release();
            }
        else
            {
            // It is not simple ... we take the long way home
            pMyResultShape = pResultHoled->DifferentiateShapeSCS(*pShape);
            }
        }
    else
        {
        // The process requires that we first differentate the outter shapes
        // This process may well result in a complex shape or a holed shape
        pMyResultShape = GetBaseShape().DifferentiateShapeSCS(pi_rSimpleShape);

        // The second part consist in differentiating from the result shape, the holes of the holed.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ;
                 MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) !=
                    HVE2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShapeSCS(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }

        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateHoledShapeSCS
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::DifferentiateHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const
    {
    // The two holed shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rHoledShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rHoledShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rHoledShape);

    if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else
        {
        // The process requires that we first differentate the outter shapes
        // This process may well result in a complex shape or a holed shape
        pMyResultShape = GetBaseShape().DifferentiateShapeSCS(pi_rHoledShape.GetBaseShape());

        // The second part consist in differentiating from the result shape, the holes of the holed.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ;
                 MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) !=
                    HVE2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShapeSCS(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }

        // The next part consist in intersecting the base shape of the first holed
        // with the holes of the second
        if (pi_rHoledShape.HasHoles())
            {
            // For each and every holes of given ...
            HVE2DShape::HoleList::const_iterator MyGivenHoleIterator;

            for (MyGivenHoleIterator = pi_rHoledShape.m_HoleList.begin() ;
                 MyGivenHoleIterator != pi_rHoledShape.m_HoleList.end() ; ++MyGivenHoleIterator)
                {
                // We obtain the result of intersecting
                HAutoPtr<HVE2DShape> pMyOtherShape(GetBaseShape().IntersectShapeSCS(**MyGivenHoleIterator));

                // From this shape, we remove the holes of the first polygon (if any)
                if (HasHoles())
                    {
                    // For each and every holes of self ... we check the spatial position
                    HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

                    for (MySelfHoleIterator = m_HoleList.begin() ;
                         MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                        {
                        // We maintain a copy of pointer to previous result
                        HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyOtherShape.release());

                        // We obtain the result of differentiating
                        pMyOtherShape = pMyPreviousShape->DifferentiateShapeSCS(**MySelfHoleIterator);

                        // We destroy the previous shape
                        delete pMyPreviousShape.release();
                        }
                    }

                // We maintain a copy of pointer to previous result
                HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                // We obtain the result of differentiating
                pMyResultShape = pMyPreviousShape->UnifyShapeSCS(*pMyOtherShape);

                // We then delete the temporary shapes
                delete pMyPreviousShape.release();
                delete pMyOtherShape.release();

                }
            }

        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateFromHoledShapeSCS
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::DifferentiateFromHoledShapeSCS(const HVE2DHoledShape& pi_rHoledShape) const
    {
    // The two holed shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rHoledShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    return(pi_rHoledShape.DifferentiateShapeSCS(*this));
    }

//-----------------------------------------------------------------------------
// DifferentiateFromSimpleShapeSCS
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DHoledShape::DifferentiateFromSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two holed shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    HVE2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HVE2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HVE2DShape::S_OUT && MyGivenPosition == HVE2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is self
        pMyResultShape = (HVE2DShape*)pi_rSimpleShape.Clone();
        }
    else
        {
        // The process requires that we first differentate the outter shapes
        // This process may well result in a complex shape or a holed shape
        pMyResultShape = pi_rSimpleShape.DifferentiateShapeSCS(GetBaseShape());

        // The second part consist in differentiating from the result shape, the holes of the holed.

        // The next part consist in intersecting the base shape of the first holed
        // with the holes of the second
        if (HasHoles())
            {
            // For each and every holes of given ...
            HVE2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ;
                 MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // We obtain the result of intersecting
                HAutoPtr<HVE2DShape> pMyOtherShape(pi_rSimpleShape.IntersectShapeSCS(**MySelfHoleIterator));

                // We maintain a copy of pointer to previous result
                HAutoPtr<HVE2DShape>     pMyPreviousShape(pMyResultShape.release());

                // We obtain the result of differentiating
                pMyResultShape = pMyPreviousShape->UnifyShapeSCS(*pMyOtherShape);

                // We then delete the temporary shapes
                delete pMyPreviousShape.release();
                delete pMyOtherShape.release();

                }
            }
        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DHoledShape::CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const
    {
    // The given vector must be composed of a single entity
    HPRECONDITION(pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID ||
                  (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID && ((HVE2DShape*)&pi_rVector)->IsSimple()));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    // Obtain spatial position relative to base shape
    ThePosition = m_pBaseShape->CalculateSpatialPositionOf(pi_rVector);

    // Check if the position is in ... further processing is required
    if (ThePosition == HVE2DShape::S_IN && HasHoles())
        {
        // The position is in

        // For every hole, until the relative position of vector to hole is different from OUT
        // or all holes have been processed
        HVE2DShape::HoleList::const_iterator    HoleIterator;
        HVE2DShape::SpatialPosition PositionToHole = HVE2DShape::S_OUT;

        for (HoleIterator = m_HoleList.begin() ;
             HoleIterator != m_HoleList.end() && PositionToHole == HVE2DShape::S_OUT ; ++HoleIterator)
            {
            // Obtain position of vector relative to hole
            PositionToHole = (*HoleIterator)->CalculateSpatialPositionOf(pi_rVector);
            }

        // If the final result is IN, then global position is OUT
        if (PositionToHole == HVE2DShape::S_IN)
            {
            ThePosition = HVE2DShape::S_OUT;
            }
        else if (PositionToHole == HVE2DShape::S_PARTIALY_IN || PositionToHole == HVE2DShape::S_ON)
            {
            // The vector was either partialy in or on
            ThePosition = PositionToHole;
            }
        // In other case (PositionToHole == HVE2DShape::S_OUT), the primary answer remains
        }

    return(ThePosition);
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DHoledShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HVE2DShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HVE2DHoledShape" << endl;
    HDUMP0("Object is a HVE2DHoledShape\n");

    po_rOutput << "The holed shape contains " << m_HoleList.size() << "holes" << endl;
    HDUMP1("The holed shape contains %lld holes", (uint64_t)m_HoleList.size());

    m_pBaseShape->PrintState(po_rOutput);

    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    HVE2DShape::HoleList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_HoleList.begin() ;
         MyIterator != m_HoleList.end() ; MyIterator++)
        (*MyIterator)->PrintState(po_rOutput);

    po_rOutput << "END OF COMPONENT LISTING" << endl;
    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }


//-----------------------------------------------------------------------------
// GetLightShape
// This method returns a dynamically allocated light shape
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DHoledShape::GetLightShape() const
{
    HAutoPtr<HGF2DShape > pResultFence;

    // Check if the holed contains any holes
    if (m_HoleList.size() > 0)
    {
        // There is at least one hole

        // Allocate a holed fence
        HAutoPtr<HGF2DHoledShape > pNewHoled(new HGF2DHoledShape());

        // Create fence for base shape
        HAutoPtr<HGF2DShape > pBaseFence(m_pBaseShape->GetLightShape());

        // Set it into holed
        pNewHoled->SetBaseShape(*(static_cast<HGF2DSimpleShape*>(&(*pBaseFence))));

        // Drop every component
        HVE2DShape::HoleList::const_iterator    Iterator;

        for (Iterator = m_HoleList.begin() ; Iterator != m_HoleList.end() ; ++Iterator)
        {
            // Obtain fence for component
            HAutoPtr<HGF2DShape > pTmpFence((*Iterator)->GetLightShape());

            // Add fence component to complex fence
            pNewHoled->AddHole(*(static_cast<HGF2DSimpleShape*>(&(*pTmpFence))));
        }
        
        // Copy fence pointer
        pResultFence.reset(pNewHoled.release());
    }
    else
    {
        // There is no holes ... we return a polygon of base
        pResultFence.reset(m_pBaseShape->GetLightShape());
    }

    return(pResultFence.release());
}



//-----------------------------------------------------------------------------
// Drop
// This method drops the holed shape
//-----------------------------------------------------------------------------
void HVE2DHoledShape::Drop(HGF2DLocationCollection* po_pPoints,
                           double                   pi_Tolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Generate outter shape scanlines
    m_pBaseShape->Drop(po_pPoints, pi_Tolerance);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HVE2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Generate (add or remove : reverse of pi_rAdd) scanlines of hole
            (*HoleItr)->Drop(po_pPoints, pi_Tolerance);
            }
        }
    }


//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines for) the holed shape.
//-----------------------------------------------------------------------------
void HVE2DHoledShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    if (pio_rScanlines.GetScanlinesCoordSys() == 0)
        {
        pio_rScanlines.SetScanlinesCoordSys(GetCoordSys());
        }
    HASSERT(pio_rScanlines.GetScanlinesCoordSys() == GetCoordSys());

    // Check if shape is not empty
    if (!IsEmpty())
        {
        // Obtain extent of shape
        HGF2DExtent ShapeExtent = GetExtent();

        if (!pio_rScanlines.LimitsAreSet())
            {
            // HChk MR: For now, 6 is arbitrary
            pio_rScanlines.SetLimits(ShapeExtent.GetYMin(), ShapeExtent.GetYMax(), 6);
            }

        // Rasterize outer shape
        m_pBaseShape->Rasterize(pio_rScanlines);

        // If the holed shape has holes
        if (HasHoles())
            {
            // For every hole in holed shape
            HVE2DShape::HoleList::const_iterator HoleItr;
            for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
                {
                // Generate scanlines of hole
                (*HoleItr)->Rasterize(pio_rScanlines);
                }
            }
        }
    }
