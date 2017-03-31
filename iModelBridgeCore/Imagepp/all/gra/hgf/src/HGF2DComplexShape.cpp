//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DComplexShape.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DComplexShape
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>



#include <ImagePP/all/h/HGF2DComplexShape.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>
#include <ImagePP/all/h/HGF2DVoidShape.h>
#include <ImagePP/all/h/HGFScanlines.h>


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
HGF2DComplexShape::HGF2DComplexShape(const HGF2DShape::SimpleShapeList& pi_rList)
    : HGF2DShape()
    {
    // The given list may not be empty
    HPRECONDITION(pi_rList.size() > 0);

    try
        {
        // We copy all shapes to current list
        HGF2DShape::SimpleShapeList::const_iterator    MyIterator;
        for (MyIterator = pi_rList.begin() ; MyIterator != pi_rList.end() ; ++MyIterator)
            {
            // All shapes in list must be disjoint (flirting is tolerated)
            HASSERTDUMP2(CalculateSpatialPositionOf(**MyIterator) == HGF2DShape::S_OUT,
                         *this,
                         **MyIterator);
            HASSERTDUMP2(!AreContiguous(**MyIterator), *this, **MyIterator);

            // We add shape
            m_ShapeList.push_back(static_cast<HGF2DShape*>((*MyIterator)->Clone()));
            }
        }
    catch(...)
        {
        MakeEmpty();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another complex shape object.
//-----------------------------------------------------------------------------
HGF2DComplexShape& HGF2DComplexShape::operator=(const HGF2DComplexShape& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        // Empty self
        MakeEmpty();

        // Invoque ancester operator
        HGF2DShape::operator=(pi_rObj);

        // Make a copy of the list of shapes
        HGF2DShape::ShapeList::const_iterator Itr;
        for (Itr = pi_rObj.m_ShapeList.begin() ; Itr != pi_rObj.m_ShapeList.end() ; ++Itr)
            m_ShapeList.push_back((HGF2DShape*)(*Itr)->Clone());
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on complex shape boundary to given point.
//-----------------------------------------------------------------------------
HGF2DPosition HGF2DComplexShape::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    HGF2DPosition       ClosestPoint;
    HGF2DPosition       WorkPoint;
    double              WorkDistance=0.0;
    double              TheMinimalDistance=0.0;
    bool                TheMinimalDistanceSet = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator;

    // Loop till all shapes have been processed
    for (MyIterator = m_ShapeList.begin() ; MyIterator != m_ShapeList.end() ; ++MyIterator)
        {
        // Obtain closest point to current shape
        WorkPoint = (*MyIterator)->CalculateClosestPoint(pi_rPoint);

        // Check if the distance to this point is smaller than previous point
        if ((!TheMinimalDistanceSet) ||
            (TheMinimalDistance > (WorkDistance = (pi_rPoint - WorkPoint).CalculateLength())))
            {
            // This work point is closer ... it becomes the new point
            TheMinimalDistance = WorkDistance;
            ClosestPoint = WorkPoint;
            TheMinimalDistanceSet = true;
            }
        }

    return (ClosestPoint);
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// This method returns the bearing of point on complex shape
//-----------------------------------------------------------------------------
HGFBearing HGF2DComplexShape::CalculateBearing(const HGF2DPosition& pi_rPoint,
                                             HGF2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on complex shape
    HPRECONDITION(IsPointOn(pi_rPoint));

    bool       BearingFound = false;
    HGFBearing     ReturnValue;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
// CalculateAngularAccceleration
// This method returns the angular acceleration of point on complex shape
//-----------------------------------------------------------------------------
double HGF2DComplexShape::CalculateAngularAcceleration(
    const HGF2DPosition& pi_rPoint,
    HGF2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on the complex shape boundary
    HPRECONDITION(IsPointOn(pi_rPoint));

    bool                   AccelerationFound = false;
    double  ReturnValue=0.0;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
bool HGF2DComplexShape::Crosses(const HGF2DVector& pi_rVector) const
    {
    bool           IsCrossing = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or one is crossed
    while (MyIterator != m_ShapeList.end() && !(IsCrossing = (*MyIterator)->Crosses(pi_rVector)))
        ++MyIterator;

    return (IsCrossing);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the complex shape is adjacent with given vector.
//-----------------------------------------------------------------------------
bool HGF2DComplexShape::AreAdjacent(const HGF2DVector& pi_rVector) const
    {
    bool           DoAreAdjacent = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
bool HGF2DComplexShape::AreContiguous(const HGF2DVector& pi_rVector) const
    {
    bool           DoAreContiguous = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
bool HGF2DComplexShape::AreContiguousAt(const HGF2DVector& pi_rVector,
                                      const HGF2DPosition& pi_rPoint) const
    {
    // The point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool           DoAreContiguous = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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


//-----------------------------------------------------------------------------
// Intersect
// This method intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
size_t HGF2DComplexShape::Intersect(const HGF2DVector& pi_rVector,
                                    HGF2DPositionCollection* po_pCrossPoints) const
    {
    HPRECONDITION(po_pCrossPoints != 0);

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pCrossPoints->size();

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
size_t HGF2DComplexShape::ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                   HGF2DPositionCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The two objects must be contiguous one to the other
    HPRECONDITION(AreContiguous(pi_rVector));

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
void HGF2DComplexShape::ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                   const HGF2DPosition& pi_rPoint,
                                                   HGF2DPosition* po_pFirstContiguousnessPoint,
                                                   HGF2DPosition* po_pSecondContiguousnessPoint) const
    {
    // The two objects must be contiguous at specified point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    bool   ContiguousnessPointsFound = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
bool HGF2DComplexShape::IsPointOn(const HGF2DPosition& pi_rPoint,
                                HGF2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                double pi_Tolerance) const
    {
    // Note that the extremity processing parameter is ignored, since a shape has no extremity

    bool   IsOn = false;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or the point is on
    while (MyIterator != m_ShapeList.end() && !(IsOn = (*MyIterator)->IsPointOn(pi_rPoint, pi_ExtremityProcessing, pi_Tolerance)))
        ++MyIterator;

    return (IsOn);
    }



//-----------------------------------------------------------------------------
// CalculateArea
// This method calculates the area of the complex shape
//-----------------------------------------------------------------------------
double HGF2DComplexShape::CalculateArea() const
    {
    // Initialise total area
    double     MyTotalArea = 0.0;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
double HGF2DComplexShape::CalculatePerimeter() const
    {
    // Initialize perimeter
    double     MyTotalPerimeter = 0.0;

    // For each shape in holed polygon
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

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
bool HGF2DComplexShape::IsPointIn(const HGF2DPosition& pi_rPoint, double pi_Tolerance) const
    {
    // Initialize default value
    bool   IsIn = false;

    // Set tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HGF_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed or the point is in one
    while (MyIterator != m_ShapeList.end() && !(IsIn = (*MyIterator)->IsPointIn(pi_rPoint, Tolerance)))
        ++MyIterator;

    return (IsIn);
    }



//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the complex shape
//-----------------------------------------------------------------------------
HGF2DLiteExtent HGF2DComplexShape::GetExtent() const
    {
    HGF2DLiteExtent     MyExtent;

    // For each shape in complex shape
    HGF2DShape::ShapeList::const_iterator  MyIterator = m_ShapeList.begin();

    if (MyIterator != m_ShapeList.end())
        {
        MyExtent = (*MyIterator)->GetExtent();
        ++MyIterator;
        }

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
void HGF2DComplexShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // For each shape in complex shape
    HGF2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

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
void HGF2DComplexShape::Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
    {
    // The scale factor must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // For each shape in complex shape
    HGF2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

    // Loop till all shapes have been processed
    while (MyIterator != m_ShapeList.end())
        {
        // Scale current shape
        (*MyIterator)->Scale(pi_ScaleFactor, pi_rScaleOrigin);

        ++MyIterator;
        }
    }



//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DComplexShape::DifferentiateFromShape(const HGF2DShape& pi_rShape) const
    {
    HGF2DShape* pMyResultShape;

    // Check if given shape is empty
    if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else if (IsEmpty())
        {
        // Self is empty ... nothing to remove
        pMyResultShape = (HGF2DShape*)pi_rShape.Clone();
        }
    else
        {
        // Start result is given shape
        pMyResultShape = (HGF2DShape*)pi_rShape.Clone();


        // For every part of the self complex shape ... or until nothing left
        HGF2DShape::ShapeList::const_iterator  MySelfIterator = m_ShapeList.begin();

        while (MySelfIterator != m_ShapeList.end() && !(pMyResultShape->IsEmpty()))
            {
            // Calculate spatial position of component relative to current result
            HGF2DShape::SpatialPosition MyComponentPosition;
            MyComponentPosition = pMyResultShape->CalculateSpatialPositionOf(**MySelfIterator);

            if (MyComponentPosition != HGF2DShape::S_OUT || pMyResultShape->GetExtent().OutterOverlaps((*MySelfIterator)->GetExtent()))
                {
                // Save pointer to previous result
                HGF2DShape* pMyPreviousResult = pMyResultShape;

                // We differentiate this part from given
                pMyResultShape = pMyPreviousResult->DifferentiateShape(**MySelfIterator);

                // Destroy previous result
                delete pMyPreviousResult;
                }

            ++MySelfIterator;
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);
    }

//-----------------------------------------------------------------------------
// DifferentiateShape
// This method create a new shape as the difference between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DComplexShape::DifferentiateShape(const HGF2DShape& pi_rShape) const
    {
    HGF2DShape* pMyResultShape;

    // Check if given is empty
    if (pi_rShape.IsEmpty())
        {
        // Since given shape is empty ... result is self
        pMyResultShape = (HGF2DShape*)Clone();
        }
    else if (IsEmpty())
        {
        // Self is empty ... result is of course empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else
        {
        // Start result is an empty one
        pMyResultShape = new HGF2DVoidShape();

        // For every part of the given complex shape ...
        HGF2DShape::ShapeList::const_iterator  MySelfIterator = m_ShapeList.begin();

        while (MySelfIterator != m_ShapeList.end())
            {
            // We differentiate the given from this part
            HGF2DShape* pThisPartDiff = pi_rShape.DifferentiateFromShape(**MySelfIterator);

            // Check if result is empty
            if (!(pThisPartDiff->IsEmpty()))
                {
                // Save pointer to previous result
                HGF2DShape* pMyPreviousResult = pMyResultShape;

                // Unify with the current part of given
                pMyResultShape = pMyPreviousResult->UnifyShape(*pThisPartDiff);

                // delete the previous result
                delete pMyPreviousResult;
                }

            delete pThisPartDiff;

            ++MySelfIterator;
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);
    }


//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DComplexShape::IntersectShape(const HGF2DShape& pi_rShape) const
    {
    HGF2DShape* pMyResultShape;

    // Check if self or given is empty
    if (pi_rShape.IsEmpty() || IsEmpty())
        {
        // Since one is empty, result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else
        {
        // Initial complex shape is an empty one
        pMyResultShape = new HGF2DVoidShape();

        // For every part of the  complex shape ...
        HGF2DShape::ShapeList::const_iterator  MySelfIterator = m_ShapeList.begin();

        while (MySelfIterator != m_ShapeList.end())
            {
            // We intersect the given with this part
            HGF2DShape*     pThisPartIntersection = pi_rShape.IntersectShape(**MySelfIterator);

            // Check if result is empty
            if (!(pThisPartIntersection->IsEmpty()))
                {


                // Save pointer to previous result
                HGF2DShape* pMyPreviousResult = pMyResultShape;

                // Unify with the current part of given
                pMyResultShape = pMyPreviousResult->UnifyShape(*pThisPartIntersection);

                // delete the previous result
                delete pMyPreviousResult;

                }

            delete pThisPartIntersection;

            ++MySelfIterator;
            }
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);
    }

//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DComplexShape::UnifyShape(const HGF2DShape& pi_rShape) const
    {
    HGF2DShape* pMyResultShape;

    // Check if given is empy
    if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is self
        pMyResultShape = (HGF2DShape*)Clone();
        }
    else if (IsEmpty())
        {
        // Self is empty ... result is given
        pMyResultShape = (HGF2DShape*)pi_rShape.Clone();
        }
    else
        {
        // Primary result is given shape
        HGF2DShape* pMyTempShape = (HGF2DShape*)pi_rShape.Clone();

        // Result shape is a complex shape
        HGF2DComplexShape* pMyResultComplexShape = new HGF2DComplexShape();


        // For each and every shape part of the complex shape
        HGF2DShape::ShapeList::const_iterator ComponentIterator;
        for (ComponentIterator = m_ShapeList.begin() ;
             ComponentIterator != m_ShapeList.end() ;
             ++ComponentIterator)
            {
            // Obtain spatial position of component relative to temporary result
            HGF2DShape::SpatialPosition ComponentSpatialPosition;
            ComponentSpatialPosition = pMyTempShape->CalculateSpatialPositionOf(
                                           **ComponentIterator);

            // Check if spatial position is out
            if (ComponentSpatialPosition == HGF2DShape::S_OUT)
                {
                // It is out ... we must obtain the spatial position relative to component
                HGF2DShape::SpatialPosition TempSpatialPosition;
                TempSpatialPosition = (*ComponentIterator)->CalculateSpatialPositionOf(
                                          *pMyTempShape);

                // Check if it is equal to out
                if (TempSpatialPosition == HGF2DShape::S_OUT)
                    {
                    // The temporary result and component are out of one another
                    // Check if they are contiguous
                    if (pMyTempShape->AreContiguous(**ComponentIterator))
                        {
                        // Shape and component are out of one another but contiguous
                        // Save pointer to previous result
                        HGF2DShape* pMyPreviousResult = pMyTempShape;

                        // Unify with the current part of given
                        pMyTempShape = pMyPreviousResult->UnifyShape(**ComponentIterator);

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
                    HGF2DShape* pMyPreviousResult = pMyTempShape;

                    // Unify with the current part of given
                    pMyTempShape = pMyPreviousResult->UnifyShape(**ComponentIterator);

                    // delete the previous result
                    delete pMyPreviousResult;
                    }
                }
            else
                {
                // Since not OUT, they may interact ...
                // Save pointer to previous result
                HGF2DShape* pMyPreviousResult = pMyTempShape;

                // Unify with the current part of given
                pMyTempShape = pMyPreviousResult->UnifyShape(**ComponentIterator);

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

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);
    }





//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DComplexShape::CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const
    {
    // The given vector must be composed of a single entity
    HPRECONDITION(pi_rVector.GetMainVectorType() == HGF2DLinear::CLASS_ID ||
                  (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
                   ((HGF2DShape*)&pi_rVector)->IsSimple()));

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    // For every component, until the relative position of vector to
    // component is different from OUT
    // or all components have been processed
    HGF2DShape::ShapeList::const_iterator    ComponentIterator;

    for (ComponentIterator = m_ShapeList.begin() ;
         ComponentIterator != m_ShapeList.end() && ThePosition == HGF2DShape::S_OUT ;
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
void HGF2DComplexShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HGF2DShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HGF2DComplexShape" << endl;
    HDUMP0("Object is a HGF2DComplexShape\n");

    po_rOutput << "The complex shape contains " << m_ShapeList.size() << " shapes" << endl;
    HDUMP1("The complex shape contains %" PRIu64 " shapes\n", (uint64_t)m_ShapeList.size());

    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    HGF2DShape::ShapeList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_ShapeList.begin() ;
         MyIterator != m_ShapeList.end() ; MyIterator++)
        (*MyIterator)->PrintState(po_rOutput);

    po_rOutput << "END OF COMPONENT LISTING" << endl;
    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }



//-----------------------------------------------------------------------------
// Drop
// Returns the description of linear in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HGF2DComplexShape::Drop(HGF2DPositionCollection* po_pPoints,
                           double                   pi_rTolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Drop every component
    HGF2DShape::ShapeList::const_iterator    Iterator;

    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        (*Iterator)->Drop(po_pPoints, pi_rTolerance);
        }

    }



//-----------------------------------------------------------------------------
// SetAutoToleranceActive
// Sets the auto tolerance active to the components
//-----------------------------------------------------------------------------
void HGF2DComplexShape::SetAutoToleranceActive(bool pi_AutoToleranceActive)
    {
    // For every component is complex shape
    HGF2DShape::ShapeList::const_iterator    Iterator;
    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        // Set auto tolerance of component
        (*Iterator)->SetAutoToleranceActive(pi_AutoToleranceActive);
        }

    // Call ancester
    HGF2DVector::SetAutoToleranceActive(pi_AutoToleranceActive);
    }


//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance to the component
//-----------------------------------------------------------------------------
void HGF2DComplexShape::SetTolerance(double pi_Tolerance)
    {
    // The tolerance may not be null of negative
    HPRECONDITION(pi_Tolerance > 0.0);

    // For every component in complex shape
    HGF2DShape::ShapeList::const_iterator    Iterator;
    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        // Set tolerance of component
        (*Iterator)->SetTolerance(pi_Tolerance);
        }

    // Call ancester
    HGF2DVector::SetTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetStrokeTolerance
// Sets the stroke tolerance to the component
//-----------------------------------------------------------------------------
void HGF2DComplexShape::SetStrokeTolerance(const HFCPtr<HGFLiteTolerance>& pi_Tolerance)
    {
    // For every component in complex shape
    HGF2DShape::ShapeList::const_iterator    Iterator;
    for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
        {
        // Set tolerance of component
        (*Iterator)->SetStrokeTolerance(pi_Tolerance);
        }

    // Call ancester
    HGF2DVector::SetStrokeTolerance(pi_Tolerance);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// PRIVATE
// This method empties the complex shape
//-----------------------------------------------------------------------------
void HGF2DComplexShape::MakeEmpty()
    {
    // For each shape in complex shape
    HGF2DShape::ShapeList::iterator   MyIterator = m_ShapeList.begin();

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
void HGF2DComplexShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {

//    if (pio_rScanlines.GetScanlinesCoordSys() == 0)
//        {
//        pio_rScanlines.SetScanlinesCoordSys(GetCoordSys());
//        }
//    HASSERT(pio_rScanlines.GetScanlinesCoordSys() == GetCoordSys());

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

        HGF2DShape::ShapeList::const_iterator    Iterator;
        for (Iterator = m_ShapeList.begin() ; Iterator != m_ShapeList.end() ; ++Iterator)
            {
            // Rasterize this component
            (*Iterator)->Rasterize(pio_rScanlines);
            }
        }
    }


//-----------------------------------------------------------------------------
// @bsimethod                                            Alain.Robert 2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DComplexShape::AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const
    {
    HFCPtr<HGF2DShape>    pResultShape;

    // We verify that the complex shape is not empty
    if (m_ShapeList.size() > 0)
        {
        // We check that the coordsys is not allready the good one
        if (pi_rModel.IsIdentity())
            {
            pResultShape = static_cast<HGF2DShape*>(Clone());
            }
        else
            {
            // The coord sys are different
            // Create an iterator to the list of shapes
            HGF2DShape::ShapeList::const_iterator MyIterator = m_ShapeList.begin();

            // Create the initial shape
            pResultShape = (*MyIterator)->AllocTransformDirect(pi_rModel);

            // Advance to next shape
            ++MyIterator;

            // For all the remaining shapes (if any)
            while (MyIterator != m_ShapeList.end())
                {
                // Save pointer to current result shape
                HFCPtr<HGF2DShape> pCurrentResult = pResultShape;

                // Transform current shape
                HFCPtr<HGF2DShape> pCurrentShape = (*MyIterator)->AllocTransformDirect(pi_rModel);

                // Obtain new result shape by unification of next shape
                pResultShape = pCurrentResult->UnifyShape(*pCurrentShape);

                // Go on to the next shape
                ++MyIterator;
                }
            }
        }
    else
        {
        // Since the complex shape is empty, we return an empty shape
        pResultShape = new HGF2DVoidShape();

        // &&AR Check if stroke tolerance is still pertinent
        HFCPtr<HGFLiteTolerance> pTol (GetStrokeTolerance());

        if (pTol != NULL)
            {
            pResultShape->SetStrokeTolerance(pTol);
            }
        }

    return (pResultShape);
    }

//-----------------------------------------------------------------------------
// @bsimethod                                            Alain.Robert 2014/06
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DComplexShape::CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const
    {
    if (m_ShapeList.size() == 0)
        return HGF2DShape::S_OUT;

    return HGF2DShape::CalculateSpatialPositionOf(pi_rVector);
    }

//-----------------------------------------------------------------------------
// @bsimethod                                            Alain.Robert 2014/06
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DComplexShape::CalculateSpatialPositionOf(const HGF2DPosition& pi_rPosition) const
{
    if (m_ShapeList.size() == 0)
        return HGF2DShape::S_OUT;

    return HGF2DShape::CalculateSpatialPositionOf(pi_rPosition);
}

//-----------------------------------------------------------------------------
// Compute the minimal bounding box in which a shape is contained. We can also visualize it
// as the oriented extent with the minimal area. i.e. the rotated rectangle contaning all the
// shape while having the minimal area.
//
// Returns the four corner of the best rectangle found and the points forming the convex hull of the shape
//
// Reference for the algorithm : https://geidav.wordpress.com/2014/01/23/computing-oriented-minimum-bounding-boxes-in-2d/
//
// Laurent.Robert-Veillette                                              03/2016
//-----------------------------------------------------------------------------
void HGF2DComplexShape::GetBestOrientedExtent(HGF2DPositionCollection* po_pMinimalBoxCorners, HGF2DPositionCollection* po_pConvexHull) const
    {
    HPRECONDITION(po_pMinimalBoxCorners != nullptr);
    HPRECONDITION(po_pConvexHull != nullptr);

    //Get the convex hull of the shape
    HGF2DPositionCollection ShapePoints;
    Drop(&ShapePoints, 0);
    GetConvexHull(&ShapePoints, po_pConvexHull);

    HGF2DShape::GetBestOrientedExtent(po_pMinimalBoxCorners, po_pConvexHull);

    }

