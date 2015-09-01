//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DComplexShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DComplexShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HGF2DComplexShape.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>

HPM_REGISTER_CLASS(HVE2DComplexShape, HVE2DShape)


#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>


/** -----------------------------------------------------------------------------
    Constructor for a complex shape.
    The interpretation coordinate system is the coordinate system of
    the first shape contained in the list

    This constructor enables specification of the initial complex shape
    spatial definition by a list of simple shapes none of which may
    intersect the other, nor be contiguous to any other.

    @param pi_rList List of shapes that must contain at least one shape
                    from which is obtained the interpretation coordinate system.
                    None of the shapes in the list can intersect nor be
                    contiguous to any other.


    Example:
    @code
    @end

    @see Intersects()
    @see AreContiguous()
    -----------------------------------------------------------------------------
*/
HVE2DComplexShape::HVE2DComplexShape(const HVE2DShape::SimpleShapeList& pi_rList)
// If the list is empty ... the following will crash
    : HVE2DShape((*pi_rList.begin())->GetCoordSys())
    {
    // The given list may not be empty
    HPRECONDITION(pi_rList.size() > 0);

    try
        {
        // We copy all shapes to current list
        HVE2DShape::SimpleShapeList::const_iterator    MyIterator;
        for (MyIterator = pi_rList.begin() ; MyIterator != pi_rList.end() ; ++MyIterator)
            {
            // All shapes in list must be disjoint (flirting is tolerated)
            HASSERTDUMP2(CalculateSpatialPositionOf(**MyIterator) == HVE2DShape::S_OUT,
                         *this,
                         **MyIterator);
            HASSERTDUMP2(!AreContiguous(**MyIterator), *this, **MyIterator);

            // All shape in list must share the same coordinate system
            HASSERT((*MyIterator)->GetCoordSys() == GetCoordSys());

            // We add shape
            m_ShapeList.push_back((HVE2DShape*)(*MyIterator)->Clone());
            }
        }
    catch(...)
        {
        MakeEmpty();
        throw;
        }
    }


/** -----------------------------------------------------------------------------
    Constructor for a complex shape from a ligh complex shape
    -----------------------------------------------------------------------------
*/
HVE2DComplexShape::HVE2DComplexShape(const HGF2DComplexShape& pi_rShape, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DShape(pi_rpCoordSys)
    {
    // We copy all shapes to current list
    const HGF2DShape::ShapeList& rShapeList = pi_rShape.GetShapeList();
    HGF2DShape::ShapeList::const_iterator   shapeItr = rShapeList.begin();
    for ( ; shapeItr != rShapeList.end() ; ++shapeItr)
        {
        // We add shape
        m_ShapeList.push_back(HVE2DShape::fCreateShapeFromLightShape(**shapeItr, pi_rpCoordSys));
        }
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another complex shape object.
//-----------------------------------------------------------------------------
HVE2DComplexShape& HVE2DComplexShape::operator=(const HVE2DComplexShape& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Empty self
        MakeEmpty();

        // Invoque ancester operator
        HVE2DShape::operator=(pi_rObj);

        // Make a copy of the list of shapes
        HVE2DShape::ShapeList::const_iterator Itr;
        for (Itr = pi_rObj.m_ShapeList.begin() ; Itr != pi_rObj.m_ShapeList.end() ; ++Itr)
            m_ShapeList.push_back((HVE2DShape*)(*Itr)->Clone());
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on complex shape boundary to given point.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DComplexShape::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    HGF2DLocation       ClosestPoint(GetCoordSys());
    HGF2DLocation       WorkPoint(GetCoordSys());
    double              WorkDistance;
    double              TheMinimalDistance = DBL_MAX;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator;

    // Loop till all shapes have been processed
    for (MyIterator = m_ShapeList.begin() ; MyIterator != m_ShapeList.end() ; ++MyIterator)
        {
        // Obtain closest point to current shape
        WorkPoint = (*MyIterator)->CalculateClosestPoint(pi_rPoint);

        // Check if the distance to this point is smaller than previous point
        if (TheMinimalDistance > (WorkDistance = (pi_rPoint - WorkPoint).CalculateLength()))
            {
            // This work point is closer ... it becomes the new point
            TheMinimalDistance = WorkDistance;
            ClosestPoint = WorkPoint;
            }
        }

    return (ClosestPoint);
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// This method returns the bearing of point on complex shape
//-----------------------------------------------------------------------------
HGFBearing HVE2DComplexShape::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                               HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on complex shape
    HPRECONDITION(IsPointOn(pi_rPoint));

    bool       BearingFound = false;
    HGFBearing  ReturnValue;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till bearing is found
    while (!BearingFound)
        {
        // Check if point is on specified shape
        if ((*MyIterator)->IsPointOn(pi_rPoint))
            {
            // Point is on component shape ... obtain bearing
            ReturnValue = (*MyIterator)->CalculateBearing(pi_rPoint, pi_Direction);
            BearingFound = true;
            }

        ++MyIterator;

        // Either the bearing was found or there are some shapes yet to process
        HASSERT(BearingFound || MyIterator != m_ShapeList.end());
        }

    return (ReturnValue);
    }

//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// This method sets the interpretation coordinate system
//-----------------------------------------------------------------------------
void HVE2DComplexShape::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Call ancester method
    HVE2DShape::SetCoordSysImplementation(pi_rpCoordSys);

    // For each shape in complex shape
    HVE2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Set interpretation coordinate system of component shape
        (*MyIterator)->SetCoordSys(GetCoordSys());
        ++MyIterator;
        }
    }


//-----------------------------------------------------------------------------
// CalculateAngularAccceleration
// This method returns the angular acceleration of point on complex shape
//-----------------------------------------------------------------------------
double HVE2DComplexShape::CalculateAngularAcceleration(
    const HGF2DLocation& pi_rPoint,
    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on the complex shape boundary
    HPRECONDITION(IsPointOn(pi_rPoint));

    bool                   AccelerationFound = false;
    double  ReturnValue=0.0;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till acceleration found
    while (!AccelerationFound)
        {
        // Check if point is on specified shape
        if ((*MyIterator)->IsPointOn(pi_rPoint))
            {
            // Point is on component shape ... obtain acceleration
            ReturnValue = (*MyIterator)->CalculateAngularAcceleration(pi_rPoint, pi_Direction);

            // Indicate acceleration was found
            AccelerationFound = true;
            }

        ++MyIterator;

        // Either the acceleration was found or there are some shapes yet to process
        HASSERT(AccelerationFound || MyIterator != m_ShapeList.end());
        }

    return (ReturnValue);
    }


//-----------------------------------------------------------------------------
// Crosses
// This method checks if the complex shape crosses with given vector.
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::Crosses(const HVE2DVector& pi_rVector) const
    {
    bool           IsCrossing = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or one is crossed
    while (MyIterator != m_ShapeList.end() && !(IsCrossing = (*MyIterator)->Crosses(pi_rVector)))
        ++MyIterator;

    return (IsCrossing);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the complex shape is adjacent with given vector.
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    bool           DoAreAdjacent = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or one is adjacent
    while (MyIterator != m_ShapeList.end() &&
           !(DoAreAdjacent = (*MyIterator)->AreAdjacent(pi_rVector)))
        ++MyIterator;

    return (DoAreAdjacent);
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the complex shape is contiguous with given vector.
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    bool           DoAreContiguous = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or one is contiguous
    while (MyIterator != m_ShapeList.end() &&
           !(DoAreContiguous = (*MyIterator)->AreContiguous(pi_rVector)))
        ++MyIterator;

    return (DoAreContiguous);
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the complex shape is contiguous with given vector
// at specified position
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::AreContiguousAt(const HVE2DVector& pi_rVector,
                                         const HGF2DLocation& pi_rPoint) const
    {
    // The point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool           DoAreContiguous = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or one is contiguous
    while (MyIterator != m_ShapeList.end() && !(DoAreContiguous))
        {
        // Check if point is on shape
        if ((*MyIterator)->IsPointOn(pi_rPoint))
            {
            // Point is on component shape ... find out is they are contiguous
            // at this point
            DoAreContiguous = (*MyIterator)->AreContiguousAt(pi_rVector, pi_rPoint);
            }
        ++MyIterator;
        }

    return (DoAreContiguous);
    }

#if (0)
//-----------------------------------------------------------------------------
// Flirts
// This method checks if the complex shape flirts with given vector.
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::Flirts(const HVE2DVector& pi_rVector) const
    {
    bool           DoesFlirt = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or one flirts
    while (MyIterator != m_ShapeList.end() && !(DoesFlirt = (*MyIterator)->Flirts(pi_rVector)))
        ++MyIterator;

    return (DoesFlirt);
    }
#endif

//-----------------------------------------------------------------------------
// Intersect
// This method intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
size_t HVE2DComplexShape::Intersect(const HVE2DVector& pi_rVector,
                                    HGF2DLocationCollection* po_pCrossPoints) const
    {
    HPRECONDITION(po_pCrossPoints != 0);

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pCrossPoints->size();

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Add intersection points with componenet shape
        (*MyIterator)->Intersect(pi_rVector, po_pCrossPoints);
        ++MyIterator;
        }

    // Return additional number of points
    return (po_pCrossPoints->size() - InitialNumberOfPoints);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns all contiguousness points between complex shape and given
// vector
//-----------------------------------------------------------------------------
size_t HVE2DComplexShape::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                     HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The two objects must be contiguous one to the other
    HPRECONDITION(AreContiguous(pi_rVector));

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Check if this component is contiguous with vector
        if ((*MyIterator)->AreContiguous(pi_rVector))
            {
            // They do are contiguous ... obtain contiguousness points
            (*MyIterator)->ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints);
            }

        ++MyIterator;
        }

    // Return additional number of points
    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method, returns the contiguousness points of which the given point
// is a part of
//-----------------------------------------------------------------------------
void HVE2DComplexShape::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                     const HGF2DLocation& pi_rPoint,
                                                     HGF2DLocation* po_pFirstContiguousnessPoint,
                                                     HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The two objects must be contiguous at specified point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    bool   ContiguousnessPointsFound = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till contiguousness points were found
    while (!ContiguousnessPointsFound)
        {
        // Check if point is on shape component and component is contiguous at point
        if ((*MyIterator)->IsPointOn(pi_rPoint) && AreContiguousAt(pi_rVector, pi_rPoint))
            {
            // They are contiguous at given point ... obtain contiguousness points
            (*MyIterator)->ObtainContiguousnessPointsAt(pi_rVector,
                                                        pi_rPoint,
                                                        po_pFirstContiguousnessPoint,
                                                        po_pSecondContiguousnessPoint);

            // Indicate contiguousness points were found
            ContiguousnessPointsFound = true;
            }

        ++MyIterator;

        // Either the contiguousness points were found or there are some
        // shape components yet to process
        HASSERT(ContiguousnessPointsFound || MyIterator != m_ShapeList.end());
        }

    }


//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the complex shape boundary
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::IsPointOn(const HGF2DLocation& pi_rPoint,
                                   HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                   double pi_Tolerance) const
    {
    // Note that the extremity processing parameter is ignored, since a shape has no extremity

    bool   IsOn = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or the point is on
    while (MyIterator != m_ShapeList.end() && !(IsOn = (*MyIterator)->IsPointOn(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance)))
        ++MyIterator;

    return (IsOn);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the complex shape boundary
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                      HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                      double pi_Tolerance) const
    {
    // THe point must be expressed in the same coordinate system as complex shape
    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    // Note that the extremity processing parameter is ignored, since a shape has no extremity

    bool   IsOn = false;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or the point is on
    while (MyIterator != m_ShapeList.end() && !(IsOn = (*MyIterator)->IsPointOnSCS(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance)))
        ++MyIterator;

    return (IsOn);
    }



//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the complex shape
//-----------------------------------------------------------------------------
double HVE2DComplexShape::CalculateArea() const
    {
    // Initialise total area
    double     MyTotalArea = 0.0;

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Add to total area the area occupied by current shape
        MyTotalArea += (*MyIterator)->CalculateArea();

        ++MyIterator;
        }

    return(MyTotalArea);
    }

//-----------------------------------------------------------------------------
// CalculatePerimeter
// This method calculates the perimeter of the shape
//-----------------------------------------------------------------------------
double HVE2DComplexShape::CalculatePerimeter() const
    {
    // Initialize perimeter
    double     MyTotalPerimeter = 0.0;

    // For each shape in holed polygon
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Add perimeter of every shapes
        MyTotalPerimeter += (*MyIterator)->CalculatePerimeter();

        ++MyIterator;
        }

    return(MyTotalPerimeter);
    }

//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the complex shape area
//-----------------------------------------------------------------------------
bool HVE2DComplexShape::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    // Initialize default value
    bool   IsIn = false;

    // Set tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or the point is in one
    while (MyIterator != m_ShapeList.end() && !(IsIn = (*MyIterator)->IsPointIn(pi_rPoint, Tolerance)))
        ++MyIterator;

    return (IsIn);
    }



//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the complex shape
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DComplexShape::GetExtent() const
    {
    HGF2DExtent     MyExtent(GetCoordSys());

    // For each shape in complex shape
    HVE2DShape::ShapeList::const_iterator  MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Add current shape extent to global extent
        MyExtent.Add((*MyIterator)->GetExtent());

        ++MyIterator;
        }

    return (MyExtent);
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the complex shape by the specified displacement
//-----------------------------------------------------------------------------
void HVE2DComplexShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // For each shape in complex shape
    HVE2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Move current shape
        (*MyIterator)->Move(pi_rDisplacement);

        ++MyIterator;
        }
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the complex shape by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DComplexShape::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The scale factor must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // For each shape in complex shape
    HVE2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Scale current shape
        (*MyIterator)->Scale(pi_ScaleFactor, pi_rScaleOrigin);

        ++MyIterator;
        }
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the complex shape in specified
// coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DComplexShape::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pResultVector;

    // We verify that the complex shape is not empty
    if (m_ShapeList.size() > 0)
        {
        // We check that the coordsys is not allready the good one
        if (GetCoordSys() == pi_rpCoordSys)
            {
            pResultVector = (HVE2DVector*)Clone();
            }
        else
            {
            // The coord sys are different
            // Create an iterator to the list of shapes
            HVE2DShape::ShapeList::const_iterator MyIterator = m_ShapeList.begin();

            // Create the initial shape
            HVE2DShape*     pResultShape = (HVE2DShape*)((*MyIterator)->AllocateCopyInCoordSys(pi_rpCoordSys));

            // Advance to next shape
            ++MyIterator;

            // For all the remaining shapes (if any)
            while (MyIterator != m_ShapeList.end())
                {
                // Save pointer to current result shape
                HVE2DShape* pCurrentResult = pResultShape;

                // Transform current shape
                HVE2DShape* pCurrentShape = (HVE2DShape*) ((*MyIterator)->AllocateCopyInCoordSys(pi_rpCoordSys));

                // Obtain new result shape by unification of next shape
                pResultShape = pCurrentResult->UnifyShape(*pCurrentShape);

                // Destroy current transformed shape
                delete pCurrentShape;

                // We destroy the previous result
                delete pCurrentResult;

                // Go on to the next shape
                ++MyIterator;
                }

            pResultVector = pResultShape;
            }
        }
    else
        {
        // Since the complex shape is empty, we return an empty shape in given coordinate system
        pResultVector = new HVE2DVoidShape(pi_rpCoordSys);

        HFCPtr<HGFTolerance> pTol (GetStrokeTolerance());

        if (pTol != NULL)
            {
            pResultVector->SetStrokeTolerance(pTol);
            }
        }

    return (pResultVector);
    }

//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DComplexShape::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape* pMyResultShape;

    // Check if given shape is empty
    if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (IsEmpty())
        {
        // Self is empty ... nothing to remove
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();
        }
    else
        {
        // Start result is given shape
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();


        // For every part of the self complex shape ... or until nothing left
        HVE2DShape::ShapeList::const_iterator  MySelfIterator = m_ShapeList.begin();

        while (MySelfIterator != m_ShapeList.end() && !(pMyResultShape->IsEmpty()))
            {
            // Calculate spatial position of component relative to current result
            HVE2DShape::SpatialPosition MyComponentPosition;
            MyComponentPosition = pMyResultShape->CalculateSpatialPositionOf(**MySelfIterator);

            if (MyComponentPosition != HVE2DShape::S_OUT || pMyResultShape->GetExtent().OutterOverlaps((*MySelfIterator)->GetExtent()))
                {
                // Save pointer to previous result
                HVE2DShape* pMyPreviousResult = pMyResultShape;

                // We differentiate this part from given
                pMyResultShape = pMyPreviousResult->DifferentiateShapeSCS(**MySelfIterator);

                // Destroy previous result
                delete pMyPreviousResult;
                }

            ++MySelfIterator;
            }
        }

    return (pMyResultShape);
    }

//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DComplexShape::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape* pMyResultShape;

    // Check if given is empty
    if (pi_rShape.IsEmpty())
        {
        // Since given shape is empty ... result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else if (IsEmpty())
        {
        // Self is empty ... result is of course empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        // Start result is an empty one
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());

        // For every part of the given complex shape ...
        HVE2DShape::ShapeList::const_iterator  MySelfIterator = m_ShapeList.begin();

        while (MySelfIterator != m_ShapeList.end())
            {
            // We differentiate the given from this part
            HVE2DShape* pThisPartDiff = pi_rShape.DifferentiateFromShapeSCS(**MySelfIterator);

            // Check if result is empty
            if (!(pThisPartDiff->IsEmpty()))
                {
                // Save pointer to previous result
                HVE2DShape* pMyPreviousResult = pMyResultShape;

                // Unify with the current part of given
                pMyResultShape = pMyPreviousResult->UnifyShapeSCS(*pThisPartDiff);

                // delete the previous result
                delete pMyPreviousResult;
                }

            delete pThisPartDiff;

            ++MySelfIterator;
            }
        }

    return (pMyResultShape);
    }


//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DComplexShape::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape* pMyResultShape;

    // Check if self or given is empty
    if (pi_rShape.IsEmpty() || IsEmpty())
        {
        // Since one is empty, result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        // Initial complex shape is an empty one
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());

        // For every part of the  complex shape ...
        HVE2DShape::ShapeList::const_iterator  MySelfIterator = m_ShapeList.begin();

        while (MySelfIterator != m_ShapeList.end())
            {
            // We intersect the given with this part
            HVE2DShape*     pThisPartIntersection = pi_rShape.IntersectShapeSCS(**MySelfIterator);

            // Check if result is empty
            if (!(pThisPartIntersection->IsEmpty()))
                {
#if (0)
                // Check if current result is empty
                if (pMyResultShape->IsEmpty())
                    {
                    // Current result is empty ... copy this part
                    delete pMyResultShape;

                    pMyResultShape = (HVE2DShape*)pThisPartIntersection->Clone();
                    }
                else
                    {
                    // Current result is not empty ... is it a complex shape
                    if (!pMyResultShape->IsComplex())
                        {
                        // Current result is not complex, but is not empty ...
                        // We transform it into a complex shape
                        HVE2DComplexShape* pNewComplexShape = new HVE2DComplexShape(*pMyResultShape);

                        // Destroy previous result
                        delete pMyResultShape;

                        pMyResultShape = pNewComplexShape;
                        }

                    // It is a complex shape ... we add this part
                    ((HVE2DComplexShape*)pMyResultShape)->AddShape(*pThisPartIntersection);
                    }
#else

                // Save pointer to previous result
                HVE2DShape* pMyPreviousResult = pMyResultShape;

                // Unify with the current part of given
                pMyResultShape = pMyPreviousResult->UnifyShapeSCS(*pThisPartIntersection);

                // delete the previous result
                delete pMyPreviousResult;
#endif
                }

            delete pThisPartIntersection;

            ++MySelfIterator;
            }
        }

    return (pMyResultShape);
    }

//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DComplexShape::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape* pMyResultShape;

    // Check if given is empy
    if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else if (IsEmpty())
        {
        // Self is empty ... result is given
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();
        }
    else
        {
        // Primary result is given shape
        HVE2DShape* pMyTempShape = (HVE2DShape*)pi_rShape.Clone();

        // Result shape is a complex shape
        HVE2DComplexShape* pMyResultComplexShape = new HVE2DComplexShape(GetCoordSys());


        // For each and every shape part of the complex shape
        HVE2DShape::ShapeList::const_iterator ComponentIterator;
        for (ComponentIterator = m_ShapeList.begin() ;
             ComponentIterator != m_ShapeList.end() ;
             ++ComponentIterator)
            {
            // Obtain spatial position of component relative to temporary result
            HVE2DShape::SpatialPosition ComponentSpatialPosition;
            ComponentSpatialPosition = pMyTempShape->CalculateSpatialPositionOf(
                                           **ComponentIterator);

            // Check if spatial position is out
            if (ComponentSpatialPosition == HVE2DShape::S_OUT)
                {
                // It is out ... we must obtain the spatial position relative to component
                HVE2DShape::SpatialPosition TempSpatialPosition;
                TempSpatialPosition = (*ComponentIterator)->CalculateSpatialPositionOf(
                                          *pMyTempShape);

                // Check if it is equal to out
                if (TempSpatialPosition == HVE2DShape::S_OUT)
                    {
                    // The temporary result and component are out of one another
                    // Check if they are contiguous
                    if (pMyTempShape->AreContiguous(**ComponentIterator))
                        {
                        // Shape and component are out of one another but contiguous
                        // Save pointer to previous result
                        HVE2DShape* pMyPreviousResult = pMyTempShape;

                        // Unify with the current part of given
                        pMyTempShape = pMyPreviousResult->UnifyShapeSCS(**ComponentIterator);

                        // delete the previous result
                        delete pMyPreviousResult;
                        }
                    else
                        {
                        // Shapes are really disjoint ...
                        // we add a copy of component to result shape
                        pMyResultComplexShape->AddShape(**ComponentIterator);
                        }
                    }
                else
                    {
                    // Save pointer to previous result
                    HVE2DShape* pMyPreviousResult = pMyTempShape;

                    // Unify with the current part of given
                    pMyTempShape = pMyPreviousResult->UnifyShapeSCS(**ComponentIterator);

                    // delete the previous result
                    delete pMyPreviousResult;
                    }
                }
            else
                {
                // Since not OUT, they may interact ...
                // Save pointer to previous result
                HVE2DShape* pMyPreviousResult = pMyTempShape;

                // Unify with the current part of given
                pMyTempShape = pMyPreviousResult->UnifyShapeSCS(**ComponentIterator);

                // delete the previous result
                delete pMyPreviousResult;
                }
            }

        // Check if result complex shape is empty
        if (pMyResultComplexShape->IsEmpty())
            {
            // Resultcomplex shape is empty ... result is temporary result
            pMyResultShape = pMyTempShape;

            delete pMyResultComplexShape;
            }
        else
            {
            // complex shape is not empty ... we add temporary result to it
            pMyResultComplexShape->AddShape(*pMyTempShape);

            pMyResultShape = pMyResultComplexShape;

            delete pMyTempShape;
            }
        }

    return (pMyResultShape);
    }





//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DComplexShape::CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const
    {
    // The given vector must be composed of a single entity
    HPRECONDITION(pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID ||
                  (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
                   ((HVE2DShape*)&pi_rVector)->IsSimple()));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    // For every component, until the relative position of vector to
    // component is different from OUT
    // or all components have been processed
    HVE2DShape::ShapeList::const_iterator    ComponentIterator;

    for (ComponentIterator = m_ShapeList.begin() ;
         ComponentIterator != m_ShapeList.end() && ThePosition == HVE2DShape::S_OUT ;
         ++ComponentIterator)
        {
        // Obtain position of vector relative to component
        ThePosition = (*ComponentIterator)->CalculateSpatialPositionOf(pi_rVector);
        }

    return(ThePosition);
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DComplexShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HVE2DShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HVE2DComplexShape" << endl;
    HDUMP0("Object is a HVE2DComplexShape\n");

    po_rOutput << "The complex shape contains " << m_ShapeList.size() << " shapes" << endl;
    HDUMP1("The complex shape contains %" PRIu64 " shapes\n", (uint64_t)m_ShapeList.size());

    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    HVE2DShape::ShapeList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_ShapeList.begin() ;
         MyIterator != m_ShapeList.end() ; MyIterator++)
        (*MyIterator)->PrintState(po_rOutput);

    po_rOutput << "END OF COMPONENT LISTING" << endl;
    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }


//-----------------------------------------------------------------------------
// GetLightShape
// Allocates a light shape representing complex shape
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DComplexShape::GetLightShape() const
{
    HAutoPtr<HGF2DShape > pResultFence;

    // Check if the complex shape contains more than 1 fence
    if (m_ShapeList.size() > 1)
    {
        // Allocate a complex fence
        HAutoPtr<HGF2DComplexShape > pNewComplex(new HGF2DComplexShape());

        // Drop every component
        HVE2DShape::ShapeList::const_iterator    Iterator;

        for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
            // Obtain fence for component
            HAutoPtr<HGF2DShape > pComponent((*Iterator)->GetLightShape());

            // Add fence component to complex fence
            pNewComplex->AddShape(*pComponent);
        }
        
        // Copy fence pointer
        pResultFence.reset(pNewComplex.release());
    }
    else
    {
        // Check if there is one fence
        if (m_ShapeList.size() == 1)
        {
            // Since there is only one shape ... we ask it to allocate
            // instead
            pResultFence.reset((*m_ShapeList.begin())->GetLightShape());
        }
        else
        {
            // There is no shape ... we return an empty polygon
            pResultFence.reset(new HGF2DPolygonOfSegments());
        }
    }

    return(pResultFence.release());
}



//-----------------------------------------------------------------------------
// Drop
// Returns the description of linear in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HVE2DComplexShape::Drop(HGF2DLocationCollection* po_pPoints,
                             double                   pi_Tolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Drop every component
    HVE2DShape::ShapeList::const_iterator    Iterator;

    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        (*Iterator)->Drop(po_pPoints, pi_Tolerance);
        }

    }

//-----------------------------------------------------------------------------
// SetAutoToleranceActive
// Sets the auto tolerance active to the components
//-----------------------------------------------------------------------------
void HVE2DComplexShape::SetAutoToleranceActive(bool pi_AutoToleranceActive)
    {
    // For every component is complex shape
    HVE2DShape::ShapeList::const_iterator    Iterator;
    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        // Set auto tolerance of component
        (*Iterator)->SetAutoToleranceActive(pi_AutoToleranceActive);
        }

    // Call ancester
    HVE2DVector::SetAutoToleranceActive(pi_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance to the component
//-----------------------------------------------------------------------------
void HVE2DComplexShape::SetTolerance(double pi_Tolerance)
    {
    // The tolerance may not be null of negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // For every component in complex shape
    HVE2DShape::ShapeList::const_iterator    Iterator;
    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        // Set tolerance of component
        (*Iterator)->SetTolerance(pi_Tolerance);
        }

    // Call ancester
    HVE2DVector::SetTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetStrokeTolerance
// Sets the stroke tolerance to the component
//-----------------------------------------------------------------------------
void HVE2DComplexShape::SetStrokeTolerance(const HFCPtr<HGFTolerance>& pi_Tolerance)
    {
    // For every component in complex shape
    HVE2DShape::ShapeList::const_iterator    Iterator;
    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        // Set tolerance of component
        (*Iterator)->SetStrokeTolerance(pi_Tolerance);
        }

    // Call ancester
    HVE2DVector::SetStrokeTolerance(pi_Tolerance);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// PRIVATE
// This method empties the complex shape
//-----------------------------------------------------------------------------
void HVE2DComplexShape::MakeEmpty()
    {
    // For each shape in complex shape
    HVE2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been destroyed
    while (MyIterator != m_ShapeList.end())
        {
        // Destroy shape
        delete *MyIterator;

        ++MyIterator;
        }

    // Clear list of shapes
    m_ShapeList.clear();
    }


//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines for) the complex shape.
//-----------------------------------------------------------------------------
void HVE2DComplexShape::Rasterize(HGFScanLines& pio_rScanlines) const
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

        HVE2DShape::ShapeList::const_iterator    Iterator;
        for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
            {
            // Rasterize this component
            (*Iterator)->Rasterize(pio_rScanlines);
            }
        }
    }
