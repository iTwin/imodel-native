//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DHoledShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DHoledShape
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGF2DHoledShape.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DComplexShape.h>
#include <Imagepp/all/h/HGF2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>


/** -----------------------------------------------------------------------------
    Default constructor for a holed shape. This constructor creates an empty
    holed shape that does not contain any holes nor has a defined outter base shape.
    The interpretation coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
HGF2DHoledShape::HGF2DHoledShape()
    : HGF2DShape()
    {
    // Set base shape to en empty shape
    m_pBaseShape = new HGF2DVoidShape();
    }




/** -----------------------------------------------------------------------------
    This method sets the base shape of the holed shape. All holes which may
    have been previously defined are removed. The coordinate system used
    in the interpretation of the holed shape is unchanged.

    @param pi_rBaseShape Constant reference to an HGF2DSimpleShape which
                         specifies the new base shape.

    Example:
    @code
    @end

    @see HGF2DSimpleShape
    -----------------------------------------------------------------------------
*/
void HGF2DHoledShape::SetBaseShape(const HGF2DSimpleShape& pi_rBaseShape)
    {
    // Empty of all holes
    MakeEmpty();

    // Delete previous base shape
    m_pBaseShape = 0;

    // Copy given shape
    m_pBaseShape = static_cast<HGF2DSimpleShape*>(pi_rBaseShape.Clone());

    // Dispend tolerance setting
    m_pBaseShape->SetAutoToleranceActive(IsAutoToleranceActive());
    }




/** -----------------------------------------------------------------------------
    This method adds a hole to the holed shape. The given hole must not
    intersect nor be contiguous to any holes neither to the base shape.

    @param pi_rHole Constant reference to an HGF2DSimpleShape, specifying
                    the new hole to add.

    Example:
    @code
    @end

    @see AreContiguous()
    @see Intersect()
    -----------------------------------------------------------------------------
*/
void HGF2DHoledShape::AddHole(const HGF2DSimpleShape& pi_rSimpleShape)
    {
    // The given shape must be in
    HPRECONDITION(CalculateSpatialPositionOf(pi_rSimpleShape) == HGF2DShape::S_IN);

    // The given shape must not be contiguous
    HPRECONDITION(!AreContiguous(pi_rSimpleShape));

    // Create copy of shape to list
    HAutoPtr<HGF2DSimpleShape> pMyNewHole(static_cast<HGF2DSimpleShape*>(pi_rSimpleShape.Clone()));

    // Set tolerance setting to new hole
    pMyNewHole->SetAutoToleranceActive(IsAutoToleranceActive());

    // Add hole to list
    m_HoleList.push_back(pMyNewHole.release());
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another holed shape.
//-----------------------------------------------------------------------------
HGF2DHoledShape& HGF2DHoledShape::operator=(const HGF2DHoledShape& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Empty the holed shape
        MakeEmpty();

        // Invoque ancester copy
        HGF2DShape::operator=(pi_rObj);

        // Set new outter shape
        m_pBaseShape = static_cast<HGF2DSimpleShape*>(pi_rObj.m_pBaseShape->Clone());

        // We copy the holes
        for (HGF2DShape::HoleList::const_iterator Itr = pi_rObj.m_HoleList.begin();
             Itr != pi_rObj.m_HoleList.end() ; ++Itr)
            {
            m_HoleList.push_back(static_cast<HGF2DSimpleShape*>(((*Itr)->Clone())));
            }
        }

    // Return reference to self
    return (*this);
    }




//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on holed polygon boundary to given point.
//-----------------------------------------------------------------------------
HGF2DPosition HGF2DHoledShape::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    // Find the closest point on outter boundary
    HGF2DPosition       ClosestPoint(m_pBaseShape->CalculateClosestPoint(pi_rPoint));
    HGF2DPosition       WorkPoint;
    double              WorkDistance;

    // Compute distance to temporary closest point
    double         TheMinimalDistance((pi_rPoint - ClosestPoint).CalculateLength());

    // For each hole in holed polygon
    HGF2DShape::HoleList::const_iterator  MyIterator = m_HoleList.begin();

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
bool HGF2DHoledShape::AreAdjacent(const HGF2DVector& pi_rVector) const
    {
    // Find if the vector is adjacent to the outter boundary
    bool           DoAreAdjacent = m_pBaseShape->AreAdjacent(pi_rVector);

    // If they are not adjacent ... check with holes
    if (!DoAreAdjacent)
        {
        // For each hole in holed polygon
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
bool HGF2DHoledShape::AreContiguous(const HGF2DVector& pi_rVector) const
    {
    // Find if the vector is contiguous the outter boundary
    bool           DoAreContiguous = m_pBaseShape->AreContiguous(pi_rVector);

    // If they are not contiguous ... check with holes
    if (!DoAreContiguous)
        {
        // For each hole in holed polygon
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
bool HGF2DHoledShape::AreContiguousAt(const HGF2DVector& pi_rVector,
                                    const HGF2DPosition& pi_rPoint) const
    {
    // The point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool           DoAreContiguous = false;

    // Obtain tolerance
    double Tolerance = min(GetTolerance(), pi_rVector.GetTolerance());

    // Find if the point is on the outter boundary
    if (m_pBaseShape->IsPointOn(pi_rPoint, HGF2DVector::INCLUDE_EXTREMITIES, Tolerance))
        {
        // The point is on aoutter boundary ... check if contiguous
        DoAreContiguous = m_pBaseShape->AreContiguousAt(pi_rVector, pi_rPoint);
        }

    // If it was not contiguous at specified point on outter shape ... check with holes
    if (!DoAreContiguous)
        {
        // For each hole in holed polygon
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or one is contiguous at specified point
        while (MyIterator != m_HoleList.end() && !(DoAreContiguous))
            {
            // Check if point is on hole
            if ((*MyIterator)->IsPointOn(pi_rPoint, HGF2DVector::INCLUDE_EXTREMITIES, Tolerance))
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
size_t HGF2DHoledShape::Intersect(const HGF2DVector& pi_rVector,
                                HGF2DPositionCollection* po_pCrossPoints) const
    {
    HPRECONDITION(po_pCrossPoints != 0);

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pCrossPoints->size();

    // Find the intersection points with base shape
    m_pBaseShape->Intersect(pi_rVector, po_pCrossPoints);

    // For each hole in holed polygon
    HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
size_t HGF2DHoledShape::ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                   HGF2DPositionCollection* po_pContiguousnessPoints) const
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
    HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
void HGF2DHoledShape::ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                 const HGF2DPosition& pi_rPoint,
                                                 HGF2DPosition* po_pFirstContiguousnessPoint,
                                                 HGF2DPosition* po_pSecondContiguousnessPoint) const
    {
    // The two objects must be contiguous
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Obtain tolerance
    double Tolerance = min(GetTolerance(), pi_rVector.GetTolerance());

    // If the given point is on base shape and they are contiguous at this point ...
    if (m_pBaseShape->IsPointOn(pi_rPoint, HGF2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
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
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or contiguousness points are found
        while (MyIterator != m_HoleList.end() && !(ContiguousnessPointsFound))
            {
            // If the given point is on hole and they are contiguous at this point ...
            if((*MyIterator)->IsPointOn(pi_rPoint, HGF2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
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





//-----------------------------------------------------------------------------
// Crosses
// This method checks if the holed shape crosses with given vector.
//-----------------------------------------------------------------------------
bool HGF2DHoledShape::Crosses(const HGF2DVector& pi_rVector) const
    {
    // Find if the point crosses the outter boundary
    bool           IsCrossing = m_pBaseShape->Crosses(pi_rVector);

    // If it does not cross ... check with holes
    if (!IsCrossing)
        {
        // For each hole in holed polygon
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
bool HGF2DHoledShape::IsPointOn(const HGF2DPosition& pi_rPoint,
                              HGF2DVector::ExtremityProcessing pi_ExtremityProcessing,
                              double pi_Tolerance) const
    {
    // The extremity processing parameter is ignored since a holed shape has no extremity

    // Find out if the point is on the outter boundary
    bool   IsOn = m_pBaseShape->IsPointOn(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance);

    // If it is not on ... check with holes
    if (!IsOn)
        {
        // For each hole in holed polygon
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

        // Loop till all holes have been processed or or the point is on a hole boundary
        while (MyIterator != m_HoleList.end() && !(IsOn = (*MyIterator)->IsPointOn(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance)))
            ++MyIterator;
        }

    return (IsOn);
    }




//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method create a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::DifferentiateFromShape(const HGF2DShape& pi_rShape) const
    {
    HAutoPtr<HGF2DShape> pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, the result is given
        pMyResultShape = static_cast<HGF2DShape*>(pi_rShape.Clone());
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, the result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    // We separate the process of differentiation depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.DifferentiateShape(*this);
        }
    else
        {
        // Either the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = DifferentiateFromSimpleShape((*(HGF2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = DifferentiateFromHoledShape((*(HGF2DHoledShape*)(&pi_rShape)));
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape.release());
    }



//-----------------------------------------------------------------------------
// DifferentiateShape
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::DifferentiateShape(const HGF2DShape& pi_rShape) const
    {
    HAutoPtr<HGF2DShape> pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, then result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, the result is self
        pMyResultShape = static_cast<HGF2DShape*>(Clone());
        }
    // We separate the process of differentiation depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.DifferentiateFromShape(*this);
        }
    else
        {
        // Either if the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = DifferentiateSimpleShape((*(HGF2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = DifferentiateHoledShape((*(HGF2DHoledShape*)(&pi_rShape)));
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::IntersectShape(const HGF2DShape& pi_rShape) const
    {
    HAutoPtr<HGF2DShape> pMyResultShape;

    if (IsEmpty() || pi_rShape.IsEmpty())
        {
        // Since at least one of the shape is empty, result is also empty
        pMyResultShape = new HGF2DVoidShape();
        }
    // We separate the process of intersection depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.IntersectShape(*this);
        }
    else
        {
        // Either the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = IntersectSimpleShape((*(HGF2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = IntersectHoledShape((*(HGF2DHoledShape*)(&pi_rShape)));
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape.release());
    }



//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::UnifyShape(const HGF2DShape& pi_rShape) const
    {
    HAutoPtr<HGF2DShape> pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, the result is given
        pMyResultShape = (HGF2DShape*)pi_rShape.Clone();
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty ... result is self
        pMyResultShape = (HGF2DShape*)Clone();
        }
    // We separate the process of unification depending on the complexity
    // of the given shape
    else if (pi_rShape.IsComplex())
        {
        // A complex shape will be responsible of the operation, we therefore
        // transfer control to it.
        pMyResultShape = pi_rShape.UnifyShape(*this);
        }
    else
        {
        // Either the given shape is a holed shape or a simple shape
        if (pi_rShape.IsSimple())
            {
            // Since we have a simple shape, we will perform the process for simple shapes
            pMyResultShape = UnifySimpleShape((*(HGF2DSimpleShape*)(&pi_rShape)));
            }
        else
            {
            // Since the given shape is neither a complex nor a simple shape
            // it is a holed shape.
            pMyResultShape = UnifyHoledShape((*(HGF2DHoledShape*)(&pi_rShape)));
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the holed shape
//-----------------------------------------------------------------------------
double HGF2DHoledShape::CalculateArea() const
    {
    // Compute the outter polygon area
    double     MyTotalArea = m_pBaseShape->CalculateArea();

    // For each hole in holed polygon
    HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
double HGF2DHoledShape::CalculatePerimeter() const
    {
    // Compute the outter perimeter
    double     MyTotalPerimeter = m_pBaseShape->CalculatePerimeter();

    // For each hole in holed polygon
    HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
bool HGF2DHoledShape::IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const
    {
    // Set Tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HGF_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // Find out if the point is in the outter boundary
    bool   IsIn(m_pBaseShape->IsPointIn(pi_rPoint, Tolerance));

    // If it is IN ... check if it is outside of the holes area
    if (IsIn)
        {
        // For each hole in holed shape
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
void HGF2DHoledShape::MakeEmpty()
    {
    // Empty the outter shape
    m_pBaseShape->MakeEmpty();

    // For each hole in holed polygon
    HGF2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

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
void HGF2DHoledShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // Move the base shape
    m_pBaseShape->Move(pi_rDisplacement);

    // For each hole in holed polygon
    HGF2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

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
void HGF2DHoledShape::Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
    {
    // The scale factor must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // We first scale the base shape
    m_pBaseShape->Scale(pi_ScaleFactor, pi_rScaleOrigin);

    // For each hole in holed polygon
    HGF2DShape::HoleList::iterator   MyIterator = m_HoleList.begin();

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
HGFBearing HGF2DHoledShape::CalculateBearing(const HGF2DPosition& pi_rPoint,
                                             HGF2DVector::ArbitraryDirection pi_Direction) const
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
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
double HGF2DHoledShape::CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                                                     HGF2DVector::ArbitraryDirection pi_Direction) const
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
        HGF2DShape::HoleList::const_iterator   MyIterator = m_HoleList.begin();

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
void HGF2DHoledShape::SetAutoToleranceActive(bool pi_AutoToleranceActive)
    {
    // Set auto tolerance of outter shape
    m_pBaseShape->SetAutoToleranceActive(pi_AutoToleranceActive);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HGF2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Set auto tolerance of hole
            (*HoleItr)->SetAutoToleranceActive(pi_AutoToleranceActive);
            }
        }

    // Call ancester
    HGF2DVector::SetAutoToleranceActive(pi_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance to the component
//-----------------------------------------------------------------------------
void HGF2DHoledShape::SetTolerance(double pi_Tolerance)
    {
    // The tolerance may not be null of negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // Set tolerance of outter shape
    m_pBaseShape->SetTolerance(pi_Tolerance);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HGF2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Set tolerance of hole
            (*HoleItr)->SetTolerance(pi_Tolerance);
            }
        }

    // Call ancester
    HGF2DVector::SetTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetTolerance
// Sets the stroke tolerance to the component
//-----------------------------------------------------------------------------
void HGF2DHoledShape::SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance)
    {
    // Set tolerance of outter shape
    m_pBaseShape->SetStrokeTolerance(pi_Tolerance);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HGF2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Set tolerance of hole
            (*HoleItr)->SetStrokeTolerance(pi_Tolerance);
            }
        }

    // Call ancester
    HGF2DVector::SetStrokeTolerance(pi_Tolerance);
    }



//-----------------------------------------------------------------------------
// UnifySimpleShape
// PRIVATE
// This method creates a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::UnifySimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    // The process requires that we first merge the outter shapes
    // This process may well result in a complex shape or a holed shape
    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_OUT &&
        !AreContiguous(pi_rSimpleShape))
        {
        // The two shapes are disjoint
        // completely disjoint
        HAutoPtr<HGF2DComplexShape> pMyResultComplexShape(new HGF2DComplexShape());

        // Add two shapes
        pMyResultComplexShape->AddShape(pi_rSimpleShape);
        pMyResultComplexShape->AddShape(*this);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // We unify the shapes (result is either simple shape or holed shape)
        pMyResultShape = GetBaseShape().UnifyShape(pi_rSimpleShape);

        HASSERT(!pMyResultShape->IsComplex());


        // The second part consist in differentiating from the holes of self shape the base shape of the given shape.

        // We check if self has holes
        if (HasHoles())
            {
            HAutoPtr<HGF2DHoledShape> pNewHoledShape;

            // First check that previous result is not a simple shape !
            if (pMyResultShape->IsSimple())
                {
                // We must transform simple shape into a holed shape
                pNewHoledShape = new HGF2DHoledShape(*((HGF2DSimpleShape*)pMyResultShape.get()));

                delete pMyResultShape.release();
                }
            else
                pNewHoledShape = (HGF2DHoledShape*)pMyResultShape.release();

            // For each and every holes ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MyHoleIterator;

            for (MyHoleIterator = m_HoleList.begin() ; MyHoleIterator != m_HoleList.end() ; ++MyHoleIterator)
                {
                // Obtain spatial of hole relative to given shape
                HGF2DShape::SpatialPosition HolePosition = pi_rSimpleShape.CalculateSpatialPositionOf(**MyHoleIterator);

                // Check if OUT
                if ((HolePosition == HGF2DShape::S_OUT) &&
                    !((*MyHoleIterator)->AreContiguous(pi_rSimpleShape)))
                    {
                    // We already know from above that simple shape cannot be IN hole
                    HASSERT((*MyHoleIterator)->CalculateSpatialPositionOf(pi_rSimpleShape) != HGF2DShape::S_IN);

                    // The base shape and current hole are disjoint ... we add hole
                    pNewHoledShape->AddHole(**MyHoleIterator);
                    }
                else if ((HolePosition == HGF2DShape::S_PARTIALY_IN) ||
                         ((HolePosition == HGF2DShape::S_OUT) && (*MyHoleIterator)->AreContiguous(pi_rSimpleShape)))
                    {
                    // NOTE that when IN or ON the hole is not part of result ...
                    // here we have PARTIALLY_IN (or contiguous)
                    // We must differentiate simple shape from hole
                    HAutoPtr<HGF2DShape> pTempResult((*MyHoleIterator)->DifferentiateShape(pi_rSimpleShape));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HGF2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HGF2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HGF2DSimpleShape*)(*MyComplexIterator)));
                                }
                            }
                        }

                    delete pTempResult.release();
                    }
                }

            // If the holed shape has no holes then we revert it back to a simple shape
            // otherwise it is returned
            if (!pNewHoledShape->HasHoles())
                pMyResultShape = static_cast<HGF2DShape*>(pNewHoledShape->GetBaseShape().Clone());
            else
                pMyResultShape = pNewHoledShape.release();
            }

        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// UnifyHoledShape
// PRIVATE
// This method creates a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::UnifyHoledShape(const HGF2DHoledShape& pi_rHoledShape) const
    {

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    // The process requires that we first merge the outter shapes
    // This process may well result in a complex shape or a holed shape
    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rHoledShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rHoledShape);

    if (MySelfPosition == HGF2DShape::S_OUT &&
        MyGivenPosition == HGF2DShape::S_OUT && !AreContiguous(pi_rHoledShape))
        {
        // The two shapes are disjoint
        // completely disjoint
        HAutoPtr<HGF2DComplexShape> pMyResultComplexShape(new HGF2DComplexShape());

        // Add two shapes
        pMyResultComplexShape->AddShape(pi_rHoledShape);
        pMyResultComplexShape->AddShape(*this);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // We unify the shapes (result is either simple shape or holed shape)
        pMyResultShape = GetBaseShape().UnifyShape(pi_rHoledShape.GetBaseShape());

        HASSERT(!pMyResultShape->IsComplex());

        HAutoPtr<HGF2DHoledShape> pNewHoledShape;

        // First check that previous result is not a simple shape !
        if (pMyResultShape->IsSimple())
            {
            // We must transform simple shape into a holed shape
            pNewHoledShape = new HGF2DHoledShape(*((HGF2DSimpleShape*)pMyResultShape.get()));

            delete pMyResultShape.release();

            }
        else
            pNewHoledShape = (HGF2DHoledShape*)pMyResultShape.release();



        // The second part consist in differentiating from the holes of self shape the base shape of the given shape.

        // We check if self has holes
        if (HasHoles())
            {
            // For each and every holes ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MyHoleIterator;

            for (MyHoleIterator = m_HoleList.begin() ; MyHoleIterator != m_HoleList.end() ; ++MyHoleIterator)
                {
                // Obtain spatial of hole relative to given holed shape base
                HGF2DShape::SpatialPosition HolePosition = pi_rHoledShape.GetBaseShape().CalculateSpatialPositionOf(**MyHoleIterator);

                // Check if OUT
                if (HolePosition == HGF2DShape::S_OUT)
                    {
                    // We already know from above that simple shape cannot be IN hole
                    HASSERT((*MyHoleIterator)->CalculateSpatialPositionOf(pi_rHoledShape.GetBaseShape()) != HGF2DShape::S_IN);

                    // The base shape and current hole are disjoint ... we add hole
                    pNewHoledShape->AddHole(**MyHoleIterator);
                    }
                else if (HolePosition == HGF2DShape::S_PARTIALY_IN)
                    {
                    // NOTE that when IN or ON the hole is not part of result ...
                    // here we have PARTIALLY_IN
                    // We must differentiate simple shape from hole
                    HAutoPtr<HGF2DShape> pTempResult((*MyHoleIterator)->DifferentiateShape(pi_rHoledShape.GetBaseShape()));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HGF2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HGF2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HGF2DSimpleShape*)(*MyComplexIterator)));
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
            HGF2DShape::HoleList::const_iterator MyHoleIterator;

            for (MyHoleIterator = pi_rHoledShape.GetHoleList().begin() ;
                 MyHoleIterator != pi_rHoledShape.GetHoleList().end() ; ++MyHoleIterator)
                {
                // Obtain spatial of hole relative to simple shape
                HGF2DShape::SpatialPosition HolePosition = GetBaseShape().CalculateSpatialPositionOf(**MyHoleIterator);

                // Check if OUT
                if (HolePosition == HGF2DShape::S_OUT)
                    {
                    // We already know from above that simple shape cannot be IN hole
                    HASSERT((*MyHoleIterator)->CalculateSpatialPositionOf(GetBaseShape()) != HGF2DShape::S_IN);

                    // The base shape and current hole are disjoint ... we add hole
                    pNewHoledShape->AddHole(**MyHoleIterator);
                    }
                else if (HolePosition == HGF2DShape::S_PARTIALY_IN)
                    {
                    // NOTE that when IN or ON the hole is not part of result ...
                    // here we have PARTIALLY_IN
                    // We must differentiate simple shape from hole
                    HAutoPtr<HGF2DShape> pTempResult((*MyHoleIterator)->DifferentiateShape(GetBaseShape()));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HGF2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HGF2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HGF2DSimpleShape*)(*MyComplexIterator)));
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
            HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ; MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {

                // For each and every holes of given ... we check the spatial position
                HGF2DShape::HoleList::const_iterator MyGivenHoleIterator;

                for (MyGivenHoleIterator = pi_rHoledShape.GetHoleList().begin() ;
                     MyGivenHoleIterator != pi_rHoledShape.m_HoleList.end() ; ++MyGivenHoleIterator)
                    {
                    // Intersect current hole of self with current hole of given
                    HAutoPtr<HGF2DShape> pTempResult((*MySelfHoleIterator)->IntersectShape(**MyGivenHoleIterator));

                    if (!pTempResult->IsEmpty())
                        {
                        // The result may be a simple shape or a complex shape made of simple shapes
                        if (pTempResult->IsSimple())
                            {
                            // The result is a simple shape ... add as hole
                            pNewHoledShape->AddHole(*((HGF2DSimpleShape*)pTempResult.get()));
                            }
                        else
                            {
                            // The shape is complex !!!
                            HASSERT(pTempResult->IsComplex());

                            // Add every component as hole
                            HGF2DShape::ShapeList::const_iterator MyComplexIterator;

                            for (MyComplexIterator = pTempResult->GetShapeList().begin() ;
                                 MyComplexIterator != pTempResult->GetShapeList().end() ;
                                 ++MyComplexIterator)
                                {
                                // Each component must be simple
                                HASSERT((*MyComplexIterator)->IsSimple());

                                // Add component as a hole
                                pNewHoledShape->AddHole(*((HGF2DSimpleShape*)(*MyComplexIterator)));
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
// IntersectSimpleShape
// PRIVATE
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::IntersectSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else
        {

        // The process requires that we first intersect the outter shapes
        // This process may well result in a complex shape
        pMyResultShape = GetBaseShape().IntersectShape(pi_rSimpleShape);

        // The second part consist in differentiating from the result shape, the holes.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ; MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) != HGF2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShape(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }
        }

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// IntersectHoledShape
// PRIVATE
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::IntersectHoledShape(const HGF2DHoledShape& pi_rHoledShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rHoledShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rHoledShape);

    if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else if (MySelfPosition == HGF2DShape::S_ON && MyGivenPosition == HGF2DShape::S_ON)
        {
        // The two shapes are identical ... intersection is either one
        pMyResultShape = (HGF2DShape*)Clone();
        }
    else
        {

        // The process requires that we first intersect the outter shapes
        // This process may well result in a complex shape
        pMyResultShape = GetBaseShape().IntersectShape(pi_rHoledShape.GetBaseShape());

        // The second part consist in differentiating from the result shape, the holes.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ; MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) != HGF2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShape(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }

        // We do the same with the holes of given
        if (pi_rHoledShape.HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MyGivenHoleIterator;

            for (MyGivenHoleIterator = pi_rHoledShape.GetHoleList().begin() ;
                 MyGivenHoleIterator != pi_rHoledShape.GetHoleList().end() ; ++MyGivenHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MyGivenHoleIterator) !=
                    HGF2DShape::S_OUT)
                    {

                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShape(**MyGivenHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }
        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateSimpleShape
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::DifferentiateSimpleShape(
    const HGF2DSimpleShape& pi_rSimpleShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is self
        pMyResultShape = (HGF2DShape*)Clone();
        }
    else if (MySelfPosition == HGF2DShape::S_OUT &&
             MyGivenPosition == HGF2DShape::S_IN && !AreContiguous(pi_rSimpleShape))
        {
        // The simple shape is completely in area of holed ... it is a new hole
        HAutoPtr<HGF2DHoledShape> pMyNewHoledShape((HGF2DHoledShape*)Clone());

        pMyNewHoledShape->AddHole(pi_rSimpleShape);

        pMyResultShape = pMyNewHoledShape.release();
        }
    else if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_ON)
        {
        // Given simple shape is equivalent to one of the holes....
        // Result is self
        pMyResultShape = (HGF2DShape*)Clone();
        }
    else if (MySelfPosition == HGF2DShape::S_OUT &&
             MyGivenPosition == HGF2DShape::S_IN && !(m_pBaseShape->AreContiguous(pi_rSimpleShape)))
        {
        // The simple shape is in but contiguous to one or more of the holes
        // The result can only be a holed shape

        // The current holed must have holes
        HASSERT(HasHoles());

        // Result is holed with the same base shape
        HAutoPtr<HGF2DHoledShape> pResultHoled(new HGF2DHoledShape(*m_pBaseShape));

        // For each and every holes of self ... we check the spatial position
        HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

        // Obtain a clone of simple shape
        HFCPtr<HGF2DShape> pSimpleShape = static_cast<HGF2DSimpleShape*>(pi_rSimpleShape.Clone());

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
        pResultHoled->AddHole(*(static_cast<HGF2DSimpleShape*>(&(*pSimpleShape))));

        pMyResultShape = pResultHoled.release();
        }
    else if (MyGivenPosition == HGF2DShape::S_PARTIALY_IN &&
             (m_pBaseShape->CalculateSpatialPositionOf(pi_rSimpleShape) == HGF2DShape::S_IN))
        {
        // The shape to remove is completely in base shape but partially in a hole
        // The result can be a holed shape or surface

        // The current holed must have holes
        HASSERT(HasHoles());

        // Partial result is holed with the same base shape
        HAutoPtr<HGF2DHoledShape> pResultHoled(new HGF2DHoledShape(*m_pBaseShape));

        // For each and every holes of self ... we check the spatial position
        HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

        // Obtain a clone of simple shape
        HFCPtr<HGF2DShape> pShape = static_cast<HGF2DShape*>(pi_rSimpleShape.Clone());

        for (MySelfHoleIterator = m_HoleList.begin() ;
             MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
            {
            // Obtain position of hole to current shape
            HGF2DShape::SpatialPosition ThePosition = pShape->CalculateSpatialPositionOf(**MySelfHoleIterator);

            // Check if this hole is partially in current result
            if (ThePosition == HGF2DShape::S_PARTIALY_IN)
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
            pResultHoled->AddHole(*(static_cast<HGF2DSimpleShape*>(&(*pShape))));

            pMyResultShape = pResultHoled.release();
            }
        else
            {
            // It is not simple ... we take the long way home
            pMyResultShape = pResultHoled->DifferentiateShape(*pShape);
            }
        }
    else
        {
        // The process requires that we first differentate the outter shapes
        // This process may well result in a complex shape or a holed shape
        pMyResultShape = GetBaseShape().DifferentiateShape(pi_rSimpleShape);

        // The second part consist in differentiating from the result shape, the holes of the holed.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ;
                 MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) !=
                    HGF2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShape(**MySelfHoleIterator);

                    // We destroy the previous shape
                    delete pMyPreviousShape.release();
                    }
                }
            }

        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateHoledShape
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::DifferentiateHoledShape(const HGF2DHoledShape& pi_rHoledShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rHoledShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rHoledShape);

    if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is self
        pMyResultShape = (HGF2DShape*)Clone();
        }
    else
        {
        // The process requires that we first differentate the outter shapes
        // This process may well result in a complex shape or a holed shape
        pMyResultShape = GetBaseShape().DifferentiateShape(pi_rHoledShape.GetBaseShape());

        // The second part consist in differentiating from the result shape, the holes of the holed.

        // We therefore check if self has holes
        if (HasHoles())
            {
            // For each and every holes of self ... we check the spatial position
            HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ;
                 MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // Obtain spatial position of hole
                if (pMyResultShape->CalculateSpatialPositionOf(**MySelfHoleIterator) !=
                    HGF2DShape::S_OUT)
                    {
                    // We maintain a copy of pointer to previous result
                    HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                    // We obtain the result of differentiating
                    pMyResultShape = pMyPreviousShape->DifferentiateShape(**MySelfHoleIterator);

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
            HGF2DShape::HoleList::const_iterator MyGivenHoleIterator;

            for (MyGivenHoleIterator = pi_rHoledShape.m_HoleList.begin() ;
                 MyGivenHoleIterator != pi_rHoledShape.m_HoleList.end() ; ++MyGivenHoleIterator)
                {
                // We obtain the result of intersecting
                HAutoPtr<HGF2DShape> pMyOtherShape(GetBaseShape().IntersectShape(**MyGivenHoleIterator));

                // From this shape, we remove the holes of the first polygon (if any)
                if (HasHoles())
                    {
                    // For each and every holes of self ... we check the spatial position
                    HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

                    for (MySelfHoleIterator = m_HoleList.begin() ;
                         MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                        {
                        // We maintain a copy of pointer to previous result
                        HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyOtherShape.release());

                        // We obtain the result of differentiating
                        pMyOtherShape = pMyPreviousShape->DifferentiateShape(**MySelfHoleIterator);

                        // We destroy the previous shape
                        delete pMyPreviousShape.release();
                        }
                    }

                // We maintain a copy of pointer to previous result
                HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                // We obtain the result of differentiating
                pMyResultShape = pMyPreviousShape->UnifyShape(*pMyOtherShape);

                // We then delete the temporary shapes
                delete pMyPreviousShape.release();
                delete pMyOtherShape.release();

                }
            }

        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateFromHoledShape
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::DifferentiateFromHoledShape(const HGF2DHoledShape& pi_rHoledShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rHoledShape.IsEmpty());

    return(pi_rHoledShape.DifferentiateShape(*this));
    }

//-----------------------------------------------------------------------------
// DifferentiateFromSimpleShape
// PRIVATE
// This method creates a new shape as the differentiation from self of given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DHoledShape::DifferentiateFromSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const
    {
    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HGF2DShape>     pMyResultShape;

    HGF2DShape::SpatialPosition MySelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);
    HGF2DShape::SpatialPosition MyGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

    if (MySelfPosition == HGF2DShape::S_OUT && MyGivenPosition == HGF2DShape::S_OUT)
        {
        // Shapes are disjoint ... result is self
        pMyResultShape = (HGF2DShape*)pi_rSimpleShape.Clone();
        }
    else
        {
        // The process requires that we first differentate the outter shapes
        // This process may well result in a complex shape or a holed shape
        pMyResultShape = pi_rSimpleShape.DifferentiateShape(GetBaseShape());

        // The second part consist in differentiating from the result shape, the holes of the holed.

        // The next part consist in intersecting the base shape of the first holed
        // with the holes of the second
        if (HasHoles())
            {
            // For each and every holes of given ...
            HGF2DShape::HoleList::const_iterator MySelfHoleIterator;

            for (MySelfHoleIterator = m_HoleList.begin() ;
                 MySelfHoleIterator != m_HoleList.end() ; ++MySelfHoleIterator)
                {
                // We obtain the result of intersecting
                HAutoPtr<HGF2DShape> pMyOtherShape(pi_rSimpleShape.IntersectShape(**MySelfHoleIterator));

                // We maintain a copy of pointer to previous result
                HAutoPtr<HGF2DShape>     pMyPreviousShape(pMyResultShape.release());

                // We obtain the result of differentiating
                pMyResultShape = pMyPreviousShape->UnifyShape(*pMyOtherShape);

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
HGF2DShape::SpatialPosition HGF2DHoledShape::CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const
    {
    // The given vector must be composed of a single entity
    HPRECONDITION(pi_rVector.GetMainVectorType() == HGF2DLinear::CLASS_ID ||
                  (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID && ((HGF2DShape*)&pi_rVector)->IsSimple()));

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    // Obtain spatial position relative to base shape
    ThePosition = m_pBaseShape->CalculateSpatialPositionOf(pi_rVector);

    // Check if the position is in ... further processing is required
    if (ThePosition == HGF2DShape::S_IN && HasHoles())
        {
        // The position is in

        // For every hole, until the relative position of vector to hole is different from OUT
        // or all holes have been processed
        HGF2DShape::HoleList::const_iterator    HoleIterator;
        HGF2DShape::SpatialPosition PositionToHole = HGF2DShape::S_OUT;

        for (HoleIterator = m_HoleList.begin() ;
             HoleIterator != m_HoleList.end() && PositionToHole == HGF2DShape::S_OUT ; ++HoleIterator)
            {
            // Obtain position of vector relative to hole
            PositionToHole = (*HoleIterator)->CalculateSpatialPositionOf(pi_rVector);
            }

        // If the final result is IN, then global position is OUT
        if (PositionToHole == HGF2DShape::S_IN)
            {
            ThePosition = HGF2DShape::S_OUT;
            }
        else if (PositionToHole == HGF2DShape::S_PARTIALY_IN || PositionToHole == HGF2DShape::S_ON)
            {
            // The vector was either partialy in or on
            ThePosition = PositionToHole;
            }
        // In other case (PositionToHole == HGF2DShape::S_OUT), the primary answer remains
        }

    return(ThePosition);
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DHoledShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HGF2DShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HGF2DHoledShape" << endl;
    HDUMP0("Object is a HGF2DHoledShape\n");

    po_rOutput << "The holed shape contains " << m_HoleList.size() << "holes" << endl;
    HDUMP1("The holed shape contains %lld holes", (uint64_t)m_HoleList.size());

    m_pBaseShape->PrintState(po_rOutput);

    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    HGF2DShape::HoleList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_HoleList.begin() ;
         MyIterator != m_HoleList.end() ; MyIterator++)
        (*MyIterator)->PrintState(po_rOutput);

    po_rOutput << "END OF COMPONENT LISTING" << endl;
    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }



//-----------------------------------------------------------------------------
// Drop
// This method drops the holed shape
//-----------------------------------------------------------------------------
void HGF2DHoledShape::Drop(HGF2DPositionCollection* po_pPoints,
                         double                   pi_rTolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Generate outter shape scanlines
    m_pBaseShape->Drop(po_pPoints, pi_rTolerance);

    // If the holed shape has holes
    if (HasHoles())
        {
        // For every hole in holed shape
        HGF2DShape::HoleList::const_iterator HoleItr;
        for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
            {
            // Generate (add or remove : reverse of pi_rAdd) scanlines of hole
            (*HoleItr)->Drop(po_pPoints, pi_rTolerance);
            }
        }
    }


//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines for) the holed shape.
//-----------------------------------------------------------------------------
void HGF2DHoledShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    // Check if shape is not empty
    if (!IsEmpty())
        {
        // Obtain extent of shape
        HGF2DLiteExtent ShapeExtent = GetExtent();

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
            HGF2DShape::HoleList::const_iterator HoleItr;
            for (HoleItr = m_HoleList.begin() ; HoleItr != m_HoleList.end() ; ++HoleItr)
                {
                // Generate scanlines of hole
                (*HoleItr)->Rasterize(pio_rScanlines);
                }
            }
        }
    }
