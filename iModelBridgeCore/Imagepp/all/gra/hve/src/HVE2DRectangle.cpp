//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DRectangle.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DRectangle
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DRectangle.h>

HPM_REGISTER_CLASS(HVE2DRectangle, HVE2DSimpleShape)


#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear describing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DRectangle::GetLinear() const
    {
    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    // Add required parts
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()),
                                              HGF2DLocation(m_XMin, m_YMax, GetCoordSys())));
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()),
                                              HGF2DLocation(m_XMax, m_YMax, GetCoordSys())));
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()),
                                              HGF2DLocation(m_XMax, m_YMin, GetCoordSys())));
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()),
                                              HGF2DLocation(m_XMin, m_YMin, GetCoordSys())));

    // Copy tolerance settings
    MyComplexLinear.SetTolerance(GetTolerance());
    MyComplexLinear.SetAutoToleranceActive(IsAutoToleranceActive());

    return(MyComplexLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DRectangle::GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    if (pi_DirectionDesired == HVE2DSimpleShape::CW)
        {
        // Create CW linear
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint(m_XMin, m_YMin);
        MySegment.SetRawEndPoint(m_XMin, m_YMax);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMin, m_YMax);
        MySegment.SetRawEndPoint(m_XMax, m_YMax);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMax);
        MySegment.SetRawEndPoint(m_XMax, m_YMin);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMin);
        MySegment.SetRawEndPoint(m_XMin, m_YMin);
        MyComplexLinear.AppendLinear(MySegment);
        }
    else
        {
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint(m_XMin, m_YMin);
        MySegment.SetRawEndPoint(m_XMax, m_YMin);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMin);
        MySegment.SetRawEndPoint(m_XMax, m_YMax);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMax);
        MySegment.SetRawEndPoint(m_XMin, m_YMax);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMin, m_YMax);
        MySegment.SetRawEndPoint(m_XMin, m_YMin);
        MyComplexLinear.AppendLinear(MySegment);
        }

    // Copy tolerance settings
    MyComplexLinear.SetTolerance(GetTolerance());
    MyComplexLinear.SetAutoToleranceActive(IsAutoToleranceActive());


    return(MyComplexLinear);
    }

//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear* HVE2DRectangle::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HAutoPtr<HVE2DComplexLinear>    pMyComplexLinear(new HVE2DComplexLinear(GetCoordSys()));

    if (pi_DirectionDesired == HVE2DSimpleShape::CW)
        {
        // Create CW linear
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint(m_XMin, m_YMin);
        MySegment.SetRawEndPoint(m_XMin, m_YMax);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMin, m_YMax);
        MySegment.SetRawEndPoint(m_XMax, m_YMax);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMax);
        MySegment.SetRawEndPoint(m_XMax, m_YMin);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMin);
        MySegment.SetRawEndPoint(m_XMin, m_YMin);
        pMyComplexLinear->AppendLinear(MySegment);
        }
    else
        {
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint(m_XMin, m_YMin);
        MySegment.SetRawEndPoint(m_XMax, m_YMin);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMin);
        MySegment.SetRawEndPoint(m_XMax, m_YMax);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMax, m_YMax);
        MySegment.SetRawEndPoint(m_XMin, m_YMax);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(m_XMin, m_YMax);
        MySegment.SetRawEndPoint(m_XMin, m_YMin);
        pMyComplexLinear->AppendLinear(MySegment);
        }

    // Copy tolerance settings
    pMyComplexLinear->SetTolerance(GetTolerance());
    pMyComplexLinear->SetAutoToleranceActive(IsAutoToleranceActive());

    return(pMyComplexLinear.release());
    }

/** -----------------------------------------------------------------------------
    This method sets the spatial definition of the rectangle.

    @param pi_rOrigin Location specifying the origin of rectangle.

    @param pi_rCorner Location specifying the corner of rectangle.

    Example
    @code
    @end

    @see GetRectangle()
    @see HGF2DLocation
    -----------------------------------------------------------------------------
*/
void HVE2DRectangle::SetRectangle(const HGF2DLocation& pi_rFirstPoint,
                                  const HGF2DLocation& pi_rSecondPoint)
    {
    // Create copies of given points converted in self coordinate system
    HGF2DLocation   MyFirstPoint(pi_rFirstPoint, GetCoordSys());
    HGF2DLocation   MySecondPoint(pi_rSecondPoint, GetCoordSys());

    // Set extreme values
    m_XMin = MIN(MyFirstPoint.GetX(), MySecondPoint.GetX());
    m_XMax = MAX(MyFirstPoint.GetX(), MySecondPoint.GetX());
    m_YMin = MIN(MyFirstPoint.GetY(), MySecondPoint.GetY());
    m_YMax = MAX(MyFirstPoint.GetY(), MySecondPoint.GetY());

    // Set tolerance if necessary
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DRectangle::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    // Obtain closest point for a first segment of rectangle
    HGF2DLocation   ClosestPoint(HVE2DSegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()),
                                              HGF2DLocation(m_XMin, m_YMax, GetCoordSys())).CalculateClosestPoint(pi_rPoint));

    // Obtain the closest point to another segment of rectangle
    HGF2DLocation   WorkPoint(HVE2DSegment(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()),
                                           HGF2DLocation(m_XMax, m_YMax, GetCoordSys())).CalculateClosestPoint(pi_rPoint));

    // Check which is the closest to test point
    if ((WorkPoint - pi_rPoint).CalculateLength() < (ClosestPoint - pi_rPoint).CalculateLength())
        {
        // The second is closest ... it is our temporay closest point
        ClosestPoint = WorkPoint;
        }

    // Obtain the closest point for a third segment of rectangle
    WorkPoint = HVE2DSegment(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()),
                             HGF2DLocation(m_XMax, m_YMin, GetCoordSys())).CalculateClosestPoint(pi_rPoint);

    // Check which is the closest (old one or third closest point) to test point
    if ((WorkPoint - pi_rPoint).CalculateLength() < (ClosestPoint - pi_rPoint).CalculateLength())
        {
        // The third is closest ... it is our temporay closest point
        ClosestPoint = WorkPoint;
        }

    // Obtain the closest point for a fourth and final segment of rectangle
    WorkPoint = HVE2DSegment(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()),
                             HGF2DLocation(m_XMin, m_YMin, GetCoordSys())).CalculateClosestPoint(pi_rPoint);

    // Check which is the closest (old one or fourth closest point) to test point
    if ((WorkPoint - pi_rPoint).CalculateLength() < (ClosestPoint - pi_rPoint).CalculateLength())
        {
        // The fourth is closest ... it is our final closest point
        ClosestPoint = WorkPoint;
        }

    return (ClosestPoint);
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DRectangle::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HAutoPtr<HVE2DVector>  pMyResultVector;

    // Check if rectangle is empty
    if (IsEmpty())
        {
        pMyResultVector = new HVE2DVoidShape(pi_rpCoordSys);
        }
    else
        {
        // Check if this model preserves direction
        if (GetCoordSys()->HasStretchRelationTo(pi_rpCoordSys) ||
            GetCoordSys()->HasDirectionPreservingRelationTo(pi_rpCoordSys))
            {
            // The model concerves non-oriented rectangle shape ... we can transform the points directly
            HAutoPtr<HVE2DRectangle> pTempRectangle(new HVE2DRectangle(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()).ExpressedIn(pi_rpCoordSys),
                                                                       HGF2DLocation(m_XMax, m_YMax, GetCoordSys()).ExpressedIn(pi_rpCoordSys)));
            // Set tolerance if necessary
            pTempRectangle->SetAutoToleranceActive(IsAutoToleranceActive());

            pTempRectangle->ResetTolerance();

            pMyResultVector = pTempRectangle.release();
            }
        else
            {
            // The model either does not concerve the rectangle shape or is unknown
            // We transform the rectangle into a polygon, and pass and tell the polygon to
            // process itself
            HVE2DPolygonOfSegments    ThePolygon(*this);
            ThePolygon.SetAutoToleranceActive(IsAutoToleranceActive());
            pMyResultVector = ThePolygon.AllocateCopyInCoordSys(pi_rpCoordSys);
            }
        }

    pMyResultVector->SetStrokeTolerance(m_pStrokeTolerance);

    return(pMyResultVector.release());
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DRectangle::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Obtain the location in the rectangle coordinate system
    HGF2DLocation   MyScaleOrigin(pi_rScaleOrigin, GetCoordSys());

    // Change values
    m_XMin = MyScaleOrigin.GetX() + ((m_XMin - MyScaleOrigin.GetX()) * pi_ScaleFactor);
    m_YMin = MyScaleOrigin.GetY() + ((m_YMin - MyScaleOrigin.GetY()) * pi_ScaleFactor);
    m_XMax = MyScaleOrigin.GetX() + ((m_XMax - MyScaleOrigin.GetX()) * pi_ScaleFactor);
    m_YMax = MyScaleOrigin.GetY() + ((m_YMax - MyScaleOrigin.GetY()) * pi_ScaleFactor);

    // Swap if XMin is now greater than XMax
    if (m_XMin > m_XMax)
        std::swap(m_XMin, m_XMax);

    // Swap if XMin is now greater than XMax
    if (m_YMin > m_YMax)
        std::swap(m_YMin, m_YMax);

    // Set tolerance if necessary
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factors
// around the given location
//-----------------------------------------------------------------------------
void HVE2DRectangle::Scale(double pi_ScaleFactorX,
                           double pi_ScaleFactorY,
                           const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factors must not be zero
    HPRECONDITION(pi_ScaleFactorX != 0.0);
    HPRECONDITION(pi_ScaleFactorY != 0.0);

    // Obtain the location in the rectangle coordinate system
    HGF2DLocation   MyScaleOrigin(pi_rScaleOrigin, GetCoordSys());

    // Change values
    m_XMin = MyScaleOrigin.GetX() + ((m_XMin - MyScaleOrigin.GetX()) * pi_ScaleFactorX);
    m_YMin = MyScaleOrigin.GetY() + ((m_YMin - MyScaleOrigin.GetY()) * pi_ScaleFactorY);
    m_XMax = MyScaleOrigin.GetX() + ((m_XMax - MyScaleOrigin.GetX()) * pi_ScaleFactorX);
    m_YMax = MyScaleOrigin.GetY() + ((m_YMax - MyScaleOrigin.GetY()) * pi_ScaleFactorY);

    // Swap if XMin is now greater than XMax
    if (m_XMin > m_XMax)
        std::swap(m_XMin, m_XMax);

    // Swap if XMin is now greater than XMax
    if (m_YMin > m_YMax)
        std::swap(m_YMin, m_YMax);

    // Set tolerance if necessary
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// UnifyRectangleSCS
// This method create a new shape as the union between self and given.
// The two rectangles must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::UnifyRectangleSCS(const HVE2DRectangle& pi_rRectangle) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rRectangle.GetCoordSys());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    double Tolerance = MIN(pi_rRectangle.GetTolerance(), GetTolerance());

    // For the result to be a rectangle, it is required that the given rectangle
    // either fits exactly one of the sides of the other, or that either one be
    // completely included in the other

    // Check if both rectangles are empty
    if (IsEmpty() && pi_rRectangle.IsEmpty())
        {
        // Since both are empty, then the result is also empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (IsEmpty())
        {
        // Since self is empty, then the result is given
        pMyResultShape = new HVE2DRectangle(pi_rRectangle);
        }
    else if (pi_rRectangle.IsEmpty())
        {
        // Since given is empty, then the result is self
        pMyResultShape = new HVE2DRectangle(*this);
        }
    // Check if given englobs self
    else if (HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        {
        // Since the given contains self, the result is a copy of given shape
        pMyResultShape = new HVE2DRectangle(pi_rRectangle);
        }
    // Check if self englobs given
    else if (HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        {
        // Since self contains given shape, the result is a copy of self
        pMyResultShape = new HVE2DRectangle(*this);
        }
    // Check is adjoint and perfectly fitted on the left or right side
    else if (HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMin, Tolerance) &&
             HDOUBLE_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        pMyResultShape = new HVE2DRectangle(MIN(pi_rRectangle.m_XMin, m_XMin), m_YMin, MAX(pi_rRectangle.m_XMax, m_XMax), m_YMax, GetCoordSys());
    // Check is adjoint and perfectly fitted on the lower side
    else if (HDOUBLE_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        pMyResultShape = new HVE2DRectangle(m_XMin, MIN(pi_rRectangle.m_YMin, m_YMin), m_XMax, MAX(pi_rRectangle.m_YMax, m_YMax), GetCoordSys());
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER(pi_rRectangle.m_XMin, m_XMax, Tolerance) ||
             HDOUBLE_SMALLER(pi_rRectangle.m_XMax, m_XMin, Tolerance) ||
             HDOUBLE_GREATER(pi_rRectangle.m_YMin, m_YMax, Tolerance) ||
             HDOUBLE_SMALLER(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        {
        // Since rectangles are disjoint, the result is a complex shape
        HAutoPtr<HVE2DComplexShape> pMyResultComplexShape(new HVE2DComplexShape(GetCoordSys()));

        // Add copy of self and given to complex shape
        pMyResultComplexShape->AddShape(*this);
        pMyResultComplexShape->AddShape(pi_rRectangle);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // In all other case, the result will not be a rectangle nor a complex shape
        // We transform rectangles into polygons and relenquish control to them
        pMyResultShape = HVE2DPolygonOfSegments(*this).UnifyShapeSCS(HVE2DPolygonOfSegments(pi_rRectangle));
        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateRectangleSCS
// This method create a new shape as the difference between self and given rectangle.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::DifferentiateRectangleSCS(const HVE2DRectangle& pi_rRectangle) const
    {
    // The two shapes must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rRectangle.GetCoordSys());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    double Tolerance = MIN(pi_rRectangle.GetTolerance(), GetTolerance());

    // For the result to be a rectangle, it is required that the given rectangle
    // Chunks out a whole side of self.

    // Check if both rectangles are empty
    if (pi_rRectangle.IsEmpty())
        {
        // Since given is empty, then the result is self
        pMyResultShape = new HVE2DRectangle(*this);
        }
    else if (IsEmpty())
        {
        // Since self is empty, then the result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMin, Tolerance) ||
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        {
        // Since rectangle are disjoint, the result is self
        pMyResultShape = new HVE2DRectangle(*this);
        }
    // Check if given englobs self
    else if (HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        {
        // Since given contains self, the result is an empty rectangle
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // Check if self englobs (COMPLETELY) given
    else if (HDOUBLE_SMALLER(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_GREATER(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_SMALLER(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_GREATER(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        {
        // Since self contains given completely (no contiguous borders), the result is
        // a holed shape with self as outter shape and given as hole
        HAutoPtr<HVE2DHoledShape> pResultHoledShape(new HVE2DHoledShape(*this));

        pResultHoledShape->AddHole(pi_rRectangle);

        pMyResultShape = pResultHoledShape.release();
        }
    // Check if Left side is no more
    else if (HDOUBLE_SMALLER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        pMyResultShape = new HVE2DRectangle(pi_rRectangle.m_XMax, m_YMin, m_XMax, m_YMax, GetCoordSys());
    // Check if Right side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        pMyResultShape = new HVE2DRectangle(m_XMin, m_YMin, pi_rRectangle.m_XMin, m_YMax, GetCoordSys());
    // Check if Lower side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance))
        pMyResultShape = new HVE2DRectangle(m_XMin, pi_rRectangle.m_YMax, m_XMax, m_YMax, GetCoordSys());
    // Check if Upper side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        pMyResultShape = new HVE2DRectangle(m_XMin, m_YMin, m_XMax, pi_rRectangle.m_YMin, GetCoordSys());
    // Check if given splits self in two parts (given is vertical)
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        {
        HAutoPtr<HVE2DComplexShape> pComplex(new HVE2DComplexShape(GetCoordSys()));

        pComplex->AddShape(HVE2DRectangle(m_XMin, m_YMin, pi_rRectangle.m_XMin, m_YMax, GetCoordSys()));
        pComplex->AddShape(HVE2DRectangle(pi_rRectangle.m_XMax, m_YMin, m_XMax, m_YMax, GetCoordSys()));

        pMyResultShape = pComplex.release();
        }
    // Check if given splits self in two parts (given is horizontal)
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance))
        {
        HAutoPtr<HVE2DComplexShape> pComplex(new HVE2DComplexShape(GetCoordSys()));

        pComplex->AddShape(HVE2DRectangle(m_XMin, m_YMin, m_XMax, pi_rRectangle.m_YMin, GetCoordSys()));
        pComplex->AddShape(HVE2DRectangle(m_XMin, pi_rRectangle.m_YMax, m_XMax, m_YMax, GetCoordSys()));

        pMyResultShape = pComplex.release();
        }
    else
        {
        // In all other case, the result will not be a rectangle
        // We transform rectangles into polygons and relenquish control to them
        pMyResultShape = HVE2DPolygonOfSegments(*this).DifferentiateShapeSCS(HVE2DPolygonOfSegments(pi_rRectangle));
        }

    return (pMyResultShape.release());
    }




//-----------------------------------------------------------------------------
// IntersectRectangleSCS
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::IntersectRectangleSCS(const HVE2DRectangle& pi_rRectangle) const
    {
    // The two rectangle must have the same coordinate system
    HPRECONDITION (GetCoordSys() == pi_rRectangle.GetCoordSys());

    HAutoPtr<HVE2DShape> pMyResultShape;

    double Tolerance = MIN(pi_rRectangle.GetTolerance(), GetTolerance());

    // Check if both rectangles are empty
    if (IsEmpty() || pi_rRectangle.IsEmpty())
        {
        // Since both are empty, then the result is also empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMin, Tolerance) ||
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        {
        // Since rectangle are disjoint, the result is void
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        // The intersection of two rectangles always yields a rectangle
        // Allocate copy of this to obtain result
        pMyResultShape = new HVE2DRectangle(MAX(m_XMin, pi_rRectangle.m_XMin),
                                            MAX(m_YMin, pi_rRectangle.m_YMin),
                                            MIN(m_XMax, pi_rRectangle.m_XMax),
                                            MIN(m_YMax, pi_rRectangle.m_YMax),
                                            GetCoordSys());
        }

    return (pMyResultShape.release());
    }


#if (0)
//-----------------------------------------------------------------------------
// Flirts
// This method checks if the rectangle flirts with given vector.
//-----------------------------------------------------------------------------
bool HVE2DRectangle::Flirts(const HVE2DVector& pi_rVector) const
    {
    // Since flirting computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygonOfSegments(*this).Flirts(pi_rVector));
    }
#endif

//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
bool HVE2DRectangle::Crosses(const HVE2DVector& pi_rVector) const
    {
#if (1)
    bool Answer = false;

    // Obtain vector extent
    HGF2DExtent VectorExtent(pi_rVector.GetExtent());

    // Obtain minimal tolerance
    double Tolerance = MIN(pi_rVector.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().InnerOverlaps(VectorExtent, Tolerance))
        {
        // Possible interaction ...
        // Check if extent is in self coordinate system
        if (VectorExtent.GetCoordSys() == GetCoordSys())
            {
            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double VectorXMin = VectorExtent.GetXMin();
            double VectorXMax = VectorExtent.GetXMax();
            double VectorYMin = VectorExtent.GetYMin();
            double VectorYMax = VectorExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (!(HDOUBLE_GREATER_OR_EQUAL(VectorXMin, m_XMin, Tolerance) &&
                  HDOUBLE_SMALLER_OR_EQUAL(VectorXMax, m_XMax, Tolerance) &&
                  HDOUBLE_GREATER_OR_EQUAL(VectorYMin, m_YMin, Tolerance) &&
                  HDOUBLE_SMALLER_OR_EQUAL(VectorYMax, m_YMax, Tolerance)))
                {
                // Check if the extent overlaps only one rectangle side
                // or two opposite sides
                if (((HDOUBLE_SMALLER(VectorXMin, m_XMin, Tolerance) || HDOUBLE_GREATER(VectorXMax, m_XMax, Tolerance)) &&
                     (!HDOUBLE_SMALLER(VectorYMin, m_YMin, Tolerance) && !HDOUBLE_GREATER(VectorYMax, m_YMax, Tolerance))) ||
                    ((HDOUBLE_SMALLER(VectorYMin, m_YMin, Tolerance) || HDOUBLE_GREATER(VectorYMax, m_YMax, Tolerance)) &&
                     (!HDOUBLE_SMALLER(VectorXMin, m_XMin, Tolerance) && !HDOUBLE_GREATER(VectorXMax, m_XMax, Tolerance))))
                    {
                    // The vector extent overlaps only non consecutive sides of rectangle
                    // If the vector is made of a single component ... it is partialy in

                    // The extents partialy overlap ... If the vector is a one part vector
                    // then the result is partialy in, otherwise ... need more complex operation
                    // Check if vector is made of multiple entities
                    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID ||
                        (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
                         ((HVE2DShape*)&pi_rVector)->IsSimple()))
                        {
                        Answer = true;
                        }
                    else
                        {
                        // Too complicated ...
                        Answer = HVE2DPolygonOfSegments(*this).Crosses(pi_rVector);
                        }
                    }
                else
                    {
                    // Check if vector is a rectangle
                    if (pi_rVector.GetClassID() == HVE2DRectangle::CLASS_ID)
                        {
                        // These are rectangles ...
                        // we must check if self is completely in given rectangle
                        // If not then they must cross
                        // Check if extent is COMPLETELY in given rectangle
                        Answer =  (!(HDOUBLE_GREATER_OR_EQUAL(m_XMin, VectorXMin, Tolerance) &&
                                     HDOUBLE_SMALLER_OR_EQUAL(m_XMax, VectorXMax, Tolerance) &&
                                     HDOUBLE_GREATER_OR_EQUAL(m_YMin, VectorYMin, Tolerance) &&
                                     HDOUBLE_SMALLER_OR_EQUAL(m_YMax, VectorYMax, Tolerance)));
                        }
                    else
                        {
                        // Too complicated ...
                        Answer = HVE2DPolygonOfSegments(*this).Crosses(pi_rVector);
                        }
                    }
                }
            }
        else
            {
            // The extents are not in the same coordinate systems ...

            // We transform the Vector extent to self coordinate system
            VectorExtent.ChangeCoordSys(GetCoordSys());

            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double VectorXMin = VectorExtent.GetXMin();
            double VectorXMax = VectorExtent.GetXMax();
            double VectorYMin = VectorExtent.GetYMin();
            double VectorYMax = VectorExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (!(HDOUBLE_GREATER_OR_EQUAL(VectorXMin, m_XMin, Tolerance) &&
                  HDOUBLE_SMALLER_OR_EQUAL(VectorXMax, m_XMax, Tolerance) &&
                  HDOUBLE_GREATER_OR_EQUAL(VectorYMin, m_YMin, Tolerance) &&
                  HDOUBLE_SMALLER_OR_EQUAL(VectorYMax, m_YMax, Tolerance)))
                {
                // Too complicated ...
                Answer = HVE2DPolygonOfSegments(*this).Crosses(pi_rVector);
                }
            }
        }

    return(Answer);
#else
    // Since crossing computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygonOfSegments(*this).Crosses(pi_rVector));
#endif
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
size_t HVE2DRectangle::Intersect(const HVE2DVector& pi_rVector,
                                 HGF2DLocationCollection* po_pCrossPoints) const
    {
    // Since intersect computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygonOfSegments(*this).Intersect(pi_rVector, po_pCrossPoints));
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
bool HVE2DRectangle::AreContiguousAt(const HVE2DVector& pi_rVector,
                                      const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));


    bool   AreContiguousAtPoint = false;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    // Create first segment
    HVE2DSegment    MySegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMax, GetCoordSys()));

    MySegment.SetAutoToleranceActive(false);
    MySegment.SetTolerance(Tolerance);

    // Check if point is on segment
    if (MySegment.IsPointOn(pi_rPoint))
        {
        // Point is on this segment ... check if they are contiguous
        AreContiguousAtPoint = MySegment.AreContiguousAt(pi_rVector, pi_rPoint);
        }

    // Check if answer has been found
    if (!AreContiguousAtPoint)
        {
        // Set second segment
        MySegment.SetStartPoint(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()));
        MySegment.SetEndPoint(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()));

        // Check if point is on segment
        if (MySegment.IsPointOn(pi_rPoint))
            {
            // Point is on this segment ... check if they are contiguous
            AreContiguousAtPoint = MySegment.AreContiguousAt(pi_rVector, pi_rPoint);
            }
        }

    // Check if answer has been found
    if (!AreContiguousAtPoint)
        {
        // Set third segment
        MySegment.SetStartPoint(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()));
        MySegment.SetEndPoint(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()));

        // Check if point is on segment
        if (MySegment.IsPointOn(pi_rPoint))
            {
            // Point is on this segment ... check if they are contiguous
            AreContiguousAtPoint = MySegment.AreContiguousAt(pi_rVector, pi_rPoint);
            }
        }

    // Check if answer has been found
    if (!AreContiguousAtPoint)
        {
        // Set fourth segment
        MySegment.SetStartPoint(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()));
        MySegment.SetEndPoint(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()));

        // Check if point is on segment
        if (MySegment.IsPointOn(pi_rPoint))
            {
            // Point is on this segment ... check if they are contiguous
            AreContiguousAtPoint = MySegment.AreContiguousAt(pi_rVector, pi_rPoint);
            }
        }

    return(AreContiguousAtPoint);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
void HVE2DRectangle::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                  const HGF2DLocation& pi_rPoint,
                                                  HGF2DLocation* po_pFirstContiguousnessPoint,
                                                  HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Create a polygon and process by it
    HVE2DPolygonOfSegments(*this).ObtainContiguousnessPointsAt(pi_rVector,
                                                               pi_rPoint,
                                                               po_pFirstContiguousnessPoint,
                                                               po_pSecondContiguousnessPoint);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
size_t HVE2DRectangle::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                  HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return (HVE2DPolygonOfSegments(*this).ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints));
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
double HVE2DRectangle::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on a rectangle is always 0.0
    return 0.0;
    }


//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of rectangle at specified point
//-----------------------------------------------------------------------------
HGFBearing HVE2DRectangle::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                            HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return (HVE2DPolygonOfSegments(*this).CalculateBearing(pi_rPoint, pi_Direction));
    }

//-----------------------------------------------------------------------------
// Overlaps
// This method indicates if the rectangle overlaps the given shape.
// This method is equivalent to obtaining a spatial position different from OUT
// without going into details concerning the exact position (IN ON PARTIALLY_IN)
// It is therefore faster to operate if the only answer needed is overlaps
//-----------------------------------------------------------------------------
bool HVE2DRectangle::Overlaps(const HVE2DShape& pi_rShape) const
    {
    bool DoesOverlap = false;

    HGF2DExtent ShapeExtent(pi_rShape.GetExtent());

    // Obtain minimal tolerance
    double Tolerance = MIN(pi_rShape.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().InnerOverlaps(ShapeExtent, Tolerance))
        {
        // Possible interaction ...
        // Check if extent is in self coordinate system
        if (ShapeExtent.GetCoordSys() == GetCoordSys())
            {
            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double ShapeXMin = ShapeExtent.GetXMin();
            double ShapeXMax = ShapeExtent.GetXMax();
            double ShapeYMin = ShapeExtent.GetYMin();
            double ShapeYMax = ShapeExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (HDOUBLE_GREATER_OR_EQUAL(ShapeXMin, m_XMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(ShapeXMax, m_XMax, Tolerance) &&
                HDOUBLE_GREATER_OR_EQUAL(ShapeYMin, m_YMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(ShapeYMax, m_YMax, Tolerance))
                {
                DoesOverlap = true;
                }
            else
                {
                // Check if the extent overlaps only one rectangle side
                // or two opposite sides
                if (((HDOUBLE_SMALLER(ShapeXMin, m_XMin, Tolerance) || HDOUBLE_GREATER(ShapeXMax, m_XMax, Tolerance)) &&
                     (!HDOUBLE_SMALLER(ShapeYMin, m_YMin, Tolerance) && !HDOUBLE_GREATER(ShapeYMax, m_YMax, Tolerance))) ||
                    ((HDOUBLE_SMALLER(ShapeYMin, m_YMin, Tolerance) || HDOUBLE_GREATER(ShapeYMax, m_YMax, Tolerance)) &&
                     (!HDOUBLE_SMALLER(ShapeXMin, m_XMin, Tolerance) && !HDOUBLE_GREATER(ShapeXMax, m_XMax, Tolerance))))
                    {
                    // The vector extent overlaps only non consecutive sides of rectangle
                    // If the vector is made of a single component ... it is partialy in

                    // The extents partialy overlap ... If the vector is a one part vector
                    // then the result is partialy in, otherwise ... need more complex operation
                    // Check if vector is made of multiple entities
                    if (pi_rShape.IsSimple())
                        {
                        DoesOverlap = true;
                        }
                    else
                        {
                        // Too complicated ...
                        DoesOverlap = (S_OUT != CalculateSpatialPositionOf(pi_rShape)) &&
                                      (S_OUT != pi_rShape.CalculateSpatialPositionOf(*this));
                        }
                    }
                else
                    {
                    // There is extent overlap but we do not know more ...
                    DoesOverlap = (S_OUT != CalculateSpatialPositionOf(pi_rShape)) &&
                                  (S_OUT != pi_rShape.CalculateSpatialPositionOf(*this));
                    }
                }
            }
        else
            {
            // The extents do not have the same coordinate system  ... check
            // We transform the Vector extent to self coordinate system
            ShapeExtent.ChangeCoordSys(GetCoordSys());

            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double ShapeXMin = ShapeExtent.GetXMin();
            double ShapeXMax = ShapeExtent.GetXMax();
            double ShapeYMin = ShapeExtent.GetYMin();
            double ShapeYMax = ShapeExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (HDOUBLE_GREATER_OR_EQUAL(ShapeXMin, m_XMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(ShapeXMax, m_XMax, Tolerance) &&
                HDOUBLE_GREATER_OR_EQUAL(ShapeYMin, m_YMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(ShapeYMax, m_YMax, Tolerance))
                {
                // Extent of vector is completely in ...
                DoesOverlap = true;
                }
            else
                {
                // Too complicated ...
                DoesOverlap = (S_OUT != CalculateSpatialPositionOf(pi_rShape)) &&
                              (S_OUT != pi_rShape.CalculateSpatialPositionOf(*this));
                }
            }
        }
    return(DoesOverlap);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DRectangle::CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const
    {
    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    HGF2DExtent VectorExtent(pi_rVector.GetExtent());

    // Obtain minimal tolerance
    double Tolerance = MIN(pi_rVector.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(VectorExtent, Tolerance))
        {
        // Possible interaction ...
        // Check if extent is in self coordinate system
        if (VectorExtent.GetCoordSys() == GetCoordSys())
            {
            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double VectorXMin = VectorExtent.GetXMin();
            double VectorXMax = VectorExtent.GetXMax();
            double VectorYMin = VectorExtent.GetYMin();
            double VectorYMax = VectorExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (HDOUBLE_GREATER_OR_EQUAL(VectorXMin, m_XMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(VectorXMax, m_XMax, Tolerance) &&
                HDOUBLE_GREATER_OR_EQUAL(VectorYMin, m_YMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(VectorYMax, m_YMax, Tolerance))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON ... To do this we check if extents are equal
                if (HDOUBLE_EQUAL(VectorXMin, m_XMin, Tolerance) &&
                    HDOUBLE_EQUAL(VectorXMax, m_XMax, Tolerance) &&
                    HDOUBLE_EQUAL(VectorYMin, m_YMin, Tolerance) &&
                    HDOUBLE_EQUAL(VectorYMax, m_YMax, Tolerance))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                    }
                else
                    {
                    // At this point, the vector is probably inside, but it may also be located on a boundary
                    // such as a horizontal or vertical segment upon a boundary ... we check if extent of
                    // vector is COMPLETELY in the inner extent of rectangle
                    if (HDOUBLE_GREATER(VectorXMin, m_XMin, Tolerance) &&
                        HDOUBLE_SMALLER(VectorXMax, m_XMax, Tolerance) &&
                        HDOUBLE_GREATER(VectorYMin, m_YMin, Tolerance) &&
                        HDOUBLE_SMALLER(VectorYMax, m_YMax, Tolerance))
                        {
                        // Extent is completely inside of inner rectangle... the only possible answer is S_IN
                        ThePosition = HVE2DShape::S_IN;
                        }
                    else
                        {
                        // The vector is quite possibly ON the rectangle ... need more precise operation
                        // We will attempt to discard cases where the extent of vector is completely located
                        // on ONE boundary of the rectangle (This implies that extent of vector is flat)
                        // IMPORTANT: The condition must remain as is. The fact VectorXMin == VectorXMax
                        // does NOT imply that if VectorXMin == XMin then VectorXMax will also == XMin
                        if ((HDOUBLE_EQUAL(VectorXMin, VectorXMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(VectorXMin, m_XMin, Tolerance) && HDOUBLE_EQUAL(VectorXMax, m_XMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(VectorXMin, m_XMax, Tolerance) && HDOUBLE_EQUAL(VectorXMax, m_XMax, Tolerance))
                             ))
                            ||
                            (HDOUBLE_EQUAL(VectorYMin, VectorYMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(VectorYMin, m_YMin, Tolerance) && HDOUBLE_EQUAL(VectorYMax, m_YMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(VectorYMin, m_YMax, Tolerance) && HDOUBLE_EQUAL(VectorYMax, m_YMax, Tolerance))
                             )))
                            {
                            ThePosition = HVE2DShape::S_ON;
                            }
                        else
                            {
                            // The vector has not a flat extent upon a boundary
                            ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                            }
                        }
                    }
                }
            else
                {
                // The extents outter overlap, but vector is not completely in rectangle ...
                // Check if extents inner overlap ..
                if (!GetExtent().InnerOverlaps(VectorExtent, Tolerance))
                    {
                    // The extents do outter overlap, but not inner overlap ...
                    // We cannot conclude ...
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                    }
                else
                    {
                    // Check if the extent overlaps only one rectangle side
                    // or two opposite sides
                    if (((HDOUBLE_SMALLER(VectorXMin, m_XMin, Tolerance) || HDOUBLE_GREATER(VectorXMax, m_XMax, Tolerance)) &&
                         (!HDOUBLE_SMALLER(VectorYMin, m_YMin, Tolerance) && !HDOUBLE_GREATER(VectorYMax, m_YMax, Tolerance))) ||
                        ((HDOUBLE_SMALLER(VectorYMin, m_YMin, Tolerance) || HDOUBLE_GREATER(VectorYMax, m_YMax, Tolerance)) &&
                         (!HDOUBLE_SMALLER(VectorXMin, m_XMin, Tolerance) && !HDOUBLE_GREATER(VectorXMax, m_XMax, Tolerance))))
                        {
                        // The vector extent overlaps only non consecutive sides of rectangle
                        // If the vector is made of a single component ... it is partialy in

                        // The extents partialy overlap ... If the vector is a one part vector
                        // then the result is partialy in, otherwise ... need more complex operation
                        ThePosition = HVE2DShape::S_PARTIALY_IN;
                        }
                    else
                        {
                        // The extent overlaps consecutive sides of rectangle ... we cannot conclude
                        ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                        }
                    }
                }
            }
        else
            {
            // The extents are not in the same coordinate systems ...

            // We transform the Vector extent to self coordinate system
            VectorExtent.ChangeCoordSys(GetCoordSys());

            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double VectorXMin = VectorExtent.GetXMin();
            double VectorXMax = VectorExtent.GetXMax();
            double VectorYMin = VectorExtent.GetYMin();
            double VectorYMax = VectorExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (HDOUBLE_GREATER_OR_EQUAL(VectorXMin, m_XMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(VectorXMax, m_XMax, Tolerance) &&
                HDOUBLE_GREATER_OR_EQUAL(VectorYMin, m_YMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(VectorYMax, m_YMax, Tolerance))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL(VectorXMin, m_XMin, Tolerance) &&
                    HDOUBLE_EQUAL(VectorXMax, m_XMax, Tolerance) &&
                    HDOUBLE_EQUAL(VectorYMin, m_YMin, Tolerance) &&
                    HDOUBLE_EQUAL(VectorYMax, m_YMax, Tolerance))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                    }
                else
                    {
                    // At this point, the vector is probably inside, but it may also be located on a boundary
                    // such as a horizontal or vertical segment upon a boundary ... we check if extent of
                    // vector is COMPLETELY in the inner extent of rectangle
                    if (HDOUBLE_GREATER(VectorXMin, m_XMin, Tolerance) &&
                        HDOUBLE_SMALLER(VectorXMax, m_XMax, Tolerance) &&
                        HDOUBLE_GREATER(VectorYMin, m_YMin, Tolerance) &&
                        HDOUBLE_SMALLER(VectorYMax, m_YMax, Tolerance))
                        {
                        // Extent is completely inside of rectangle... the only possible answer is S_IN
                        ThePosition = HVE2DShape::S_IN;
                        }
                    else
                        {
                        // The vector is quite possibly ON the rectangle ... need more precise operation
                        // We will attempt to discard cases where the extent of vector is completely located
                        // on ONE boundary of the rectangle (This implies that extent of vector is flat)
                        // IMPORTANT: The condition must remain as is. The fact VectorXMin == VectorXMax
                        // does NOT imply that if VectorXMin == XMin then VectorXMax will also == XMin
                        if ((HDOUBLE_EQUAL(VectorXMin, VectorXMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(VectorXMin, m_XMin, Tolerance) && HDOUBLE_EQUAL(VectorXMax, m_XMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(VectorXMin, m_XMax, Tolerance) && HDOUBLE_EQUAL(VectorXMax, m_XMax, Tolerance))
                             ))
                            ||
                            (HDOUBLE_EQUAL(VectorYMin, VectorYMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(VectorYMin, m_YMin, Tolerance) && HDOUBLE_EQUAL(VectorYMax, m_YMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(VectorYMin, m_YMax, Tolerance) && HDOUBLE_EQUAL(VectorYMax, m_YMax, Tolerance))
                             )))
                            {
                            ThePosition = HVE2DShape::S_ON;
                            }
                        else
                            {
                            // The vector is quite possibly ON the rectangle ... need more precise operation
                            ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                            }
                        }
                    }
                }
            else
                {
                // It is not completely in ... due to the fact we changed coordinate system ...
                // we must analyse further ...
                ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                }
            }
        }

    return (ThePosition);
    }



//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfNonCrossingLinear
// This method returns the spatial position relative to shape of given linear
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DRectangle::CalculateSpatialPositionOfNonCrossingLinear(const HVE2DLinear& pi_rLinear) const
    {
    // The two vectors must not cross
    HPRECONDITION(!Crosses(pi_rLinear));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    HGF2DExtent LinearExtent(pi_rLinear.GetExtent());

    double Tolerance = MIN(pi_rLinear.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(LinearExtent, Tolerance))
        {
        // Possible interaction ...
        // Check if extent is in self coordinate system
        if (LinearExtent.GetCoordSys() == GetCoordSys())
            {
            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double LinearXMin = LinearExtent.GetXMin();
            double LinearXMax = LinearExtent.GetXMax();
            double LinearYMin = LinearExtent.GetYMin();
            double LinearYMax = LinearExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (HDOUBLE_GREATER_OR_EQUAL(LinearXMin, m_XMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(LinearXMax, m_XMax, Tolerance) &&
                HDOUBLE_GREATER_OR_EQUAL(LinearYMin, m_YMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(LinearYMax, m_YMax, Tolerance))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL(LinearXMin, m_XMin, Tolerance) &&
                    HDOUBLE_EQUAL(LinearXMax, m_XMax, Tolerance) &&
                    HDOUBLE_EQUAL(LinearYMin, m_YMin, Tolerance) &&
                    HDOUBLE_EQUAL(LinearYMax, m_YMax, Tolerance))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                else
                    {
                    // At this point, the vector is probably inside, but it may also be located on a boundary
                    // such as a horizontal or vertical segment upon a boundary ... we check if extent of
                    // vector is COMPLETELY in the inner extent of rectangle
                    if (HDOUBLE_GREATER(LinearXMin, m_XMin, Tolerance) &&
                        HDOUBLE_SMALLER(LinearXMax, m_XMax, Tolerance) &&
                        HDOUBLE_GREATER(LinearYMin, m_YMin, Tolerance) &&
                        HDOUBLE_SMALLER(LinearYMax, m_YMax, Tolerance))
                        {
                        // Extent is completely inside of rectangle... the only possible answer is S_IN
                        ThePosition = HVE2DShape::S_IN;
                        }
                    else
                        {
                        // The vector is quite possibly ON the rectangle ... need more precise operation
                        // We will attempt to discard cases where the extent of vector is completely located
                        // on ONE boundary of the rectangle (This implies that extent of vector is flat)
                        // IMPORTANT: The condition must remain as is. The fact LinearXMin == LinearXMax
                        // does NOT imply that if LinearXMin == XMin then LinearXMax will also == XMin
                        if ((HDOUBLE_EQUAL(LinearXMin, LinearXMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(LinearXMin, m_XMin, Tolerance) && HDOUBLE_EQUAL(LinearXMax, m_XMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(LinearXMin, m_XMax, Tolerance) && HDOUBLE_EQUAL(LinearXMax, m_XMax, Tolerance))
                             ))
                            ||
                            (HDOUBLE_EQUAL(LinearYMin, LinearYMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(LinearYMin, m_YMin, Tolerance) && HDOUBLE_EQUAL(LinearYMax, m_YMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(LinearYMin, m_YMax, Tolerance) && HDOUBLE_EQUAL(LinearYMax, m_YMax, Tolerance))
                             )))
                            {
                            ThePosition = HVE2DShape::S_ON;
                            }
                        else
                            {
                            // The vector is quite possibly ON the rectangle ... need more precise operation
                            ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                            }
                        }
                    }
                }
            else
                {
                // Check if extents inner overlap ..
                if (!GetExtent().InnerOverlaps(LinearExtent, Tolerance))
                    {
                    // The extents do outter overlap, but not inner overlap ...
                    // We cannot conclude ...
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                else
                    {
                    // Check if the extent overlaps only one rectangle side
                    // or two opposite sides
                    if (((HDOUBLE_SMALLER(LinearXMin, m_XMin, Tolerance) || HDOUBLE_GREATER(LinearXMax, m_XMax, Tolerance)) &&
                         (!HDOUBLE_SMALLER(LinearYMin, m_YMin,Tolerance) && !HDOUBLE_GREATER(LinearYMax, m_YMax,Tolerance))) ||
                        ((HDOUBLE_SMALLER(LinearYMin, m_YMin, Tolerance) || HDOUBLE_GREATER(LinearYMax, m_YMax, Tolerance)) &&
                         (!HDOUBLE_SMALLER(LinearXMin, m_XMin,Tolerance) && !HDOUBLE_GREATER(LinearXMax, m_XMax,Tolerance))))
                        {
                        // The extents partialy overlap ... If the vector is a one part vector
                        // then the result is partialy in, otherwise ... need more complex operation
                        // Check if object is a linear
                        // The vector is a linear ... therefore is is partialy in
                        ThePosition = HVE2DShape::S_PARTIALY_IN;
                        }
                    else
                        {
                        // The extent overlaps consecutive sides of rectangle ... we cannot conclude
                        ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                        }
                    }
                }
            }
        else
            {
            // The extents are not in the same coordinate systems ...

            // We transform the Vector extent to self coordinate system
            LinearExtent.ChangeCoordSys(GetCoordSys());

            // Extent is in the same coordinate system as rectangle ...
            // We can refine the processing
            // Obtain extent values
            // We know it is defined since overlapped
            double LinearXMin = LinearExtent.GetXMin();
            double LinearXMax = LinearExtent.GetXMax();
            double LinearYMin = LinearExtent.GetYMin();
            double LinearYMax = LinearExtent.GetYMax();

            // Check if extent is COMPLETELY in rectangle or exactly ON
            if (HDOUBLE_GREATER_OR_EQUAL(LinearXMin, m_XMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(LinearXMax, m_XMax, Tolerance) &&
                HDOUBLE_GREATER_OR_EQUAL(LinearYMin, m_YMin, Tolerance) &&
                HDOUBLE_SMALLER_OR_EQUAL(LinearYMax, m_YMax, Tolerance))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL(LinearXMin, m_XMin, Tolerance) &&
                    HDOUBLE_EQUAL(LinearXMax, m_XMax, Tolerance) &&
                    HDOUBLE_EQUAL(LinearYMin, m_YMin, Tolerance) &&
                    HDOUBLE_EQUAL(LinearYMax, m_YMax, Tolerance))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                else
                    {
                    // At this point, the vector is probably inside, but it may also be located on a boundary
                    // such as a horizontal or vertical segment upon a boundary ... we check if extent of
                    // vector is COMPLETELY in the inner extent of rectangle
                    if (HDOUBLE_GREATER(LinearXMin, m_XMin, Tolerance) &&
                        HDOUBLE_SMALLER(LinearXMax, m_XMax, Tolerance) &&
                        HDOUBLE_GREATER(LinearYMin, m_YMin, Tolerance) &&
                        HDOUBLE_SMALLER(LinearYMax, m_YMax, Tolerance))
                        {
                        // Extent is completely inside of rectangle... the only possible answer is S_IN
                        ThePosition = HVE2DShape::S_IN;
                        }
                    else
                        {
                        // The vector is quite possibly ON the rectangle ... need more precise operation
                        // We will attempt to discard cases where the extent of vector is completely located
                        // on ONE boundary of the rectangle (This implies that extent of vector is flat)
                        // IMPORTANT: The condition must remain as is. The fact LinearXMin == LinearXMax
                        // does NOT imply that if LinearXMin == XMin then LinearXMax will also == XMin
                        if ((HDOUBLE_EQUAL(LinearXMin, LinearXMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(LinearXMin, m_XMin, Tolerance) && HDOUBLE_EQUAL(LinearXMax, m_XMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(LinearXMin, m_XMax, Tolerance) && HDOUBLE_EQUAL(LinearXMax, m_XMax, Tolerance))
                             ))
                            ||
                            (HDOUBLE_EQUAL(LinearYMin, LinearYMax, Tolerance) &&
                             ((HDOUBLE_EQUAL(LinearYMin, m_YMin, Tolerance) && HDOUBLE_EQUAL(LinearYMax, m_YMin, Tolerance)) ||
                              (HDOUBLE_EQUAL(LinearYMin, m_YMax, Tolerance) && HDOUBLE_EQUAL(LinearYMax, m_YMax, Tolerance))
                             )))
                            {
                            ThePosition = HVE2DShape::S_ON;
                            }
                        else
                            {
                            // The vector is quite possibly ON the rectangle ... need more precise operation
                            ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                            }
                        }
                    }
                }
            else
                {
                // It is not completely in ... due to the fact we changed coordinate system ...
                // we must analyse further ...
                ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                }
            }
        }

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the rectangle is adjacent with given vector.
//-----------------------------------------------------------------------------
bool HVE2DRectangle::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    return (HVE2DSegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMax, GetCoordSys())).AreAdjacent(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()), HGF2DLocation(m_XMax, m_YMax, GetCoordSys())).AreAdjacent(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()), HGF2DLocation(m_XMax, m_YMin, GetCoordSys())).AreAdjacent(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMin, GetCoordSys())).AreAdjacent(pi_rVector));
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the rectangle is contiguous with given vector.
//-----------------------------------------------------------------------------
bool HVE2DRectangle::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return (HVE2DSegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMax, GetCoordSys())).AreContiguous(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()), HGF2DLocation(m_XMax, m_YMax, GetCoordSys())).AreContiguous(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()), HGF2DLocation(m_XMax, m_YMin, GetCoordSys())).AreContiguous(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMin, GetCoordSys())).AreContiguous(pi_rVector));
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DRectangle::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char     DumString[256];

    HVE2DSimpleShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HVE2DRectangle" << endl;
    HDUMP0("Object is a HVE2DRectangle\n");

    sprintf(DumString, "XMin = %5.15lf", m_XMin);
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");

    sprintf(DumString, "XMax = %5.15lf", m_XMax);
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");

    sprintf(DumString, "YMin = %5.15lf", m_YMin);
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");

    sprintf(DumString, "YMax = %5.15lf", m_YMax);
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");

#endif
    }

//-----------------------------------------------------------------------------
// GetLightShape
// Returns the description of rectangle in the form of a light shape
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DRectangle::GetLightShape() const
{
    if (IsEmpty())
    {
        // Allocate a new polygon fence
        return new HGF2DPolygonOfSegments();
    }
    else
    {
        return new HGF2DRectangle(m_XMin, m_YMin, m_XMax, m_YMax);
    }
}




//-----------------------------------------------------------------------------
// Drop
// Returns the description of linear in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HVE2DRectangle::Drop(HGF2DLocationCollection* po_pPoints,
                          double                   pi_Tolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Check if rectangle is not empty
    if (!IsEmpty())
        {
        // Reserve space for 5 points
        po_pPoints->reserve(5);

        // Add points
        po_pPoints->push_back(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()));
        po_pPoints->push_back(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()));
        po_pPoints->push_back(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()));
        po_pPoints->push_back(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()));
        po_pPoints->push_back(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()));
        }
    }


//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // Declare recipient variable
    HAutoPtr<HVE2DShape> pResultShape;

    // Check if given is completely inside rectangle
    if (GetExtent().Contains(pi_rShape.GetExtent()))
        {
        // The given completely inside rectangle
        // The result will therefore be the rectangle itself
        pResultShape = new HVE2DRectangle(*this);
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID  ?
                        UnifyRectangleSCS(*((HVE2DRectangle*)(&pi_rShape)))  :
                        pi_rShape.UnifyShapeSCS(*this));
        }

    return(pResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // Declare recipient variable
    HAutoPtr<HVE2DShape> pResultShape;

    // Check if given is a simple shape and completely inside rectangle
    if (pi_rShape.IsSimple() &&
        GetExtent().InnerContains(pi_rShape.GetExtent(), MIN(GetTolerance(), pi_rShape.GetTolerance())))
        {
        // The given is a simple shape and completely inside rectangle
        // The result will therefore be a holed shape

        // Cast the shape into a simple shape
        const HVE2DSimpleShape& rHole = static_cast<const HVE2DSimpleShape&>(pi_rShape);

        // Create holed shape
        HAutoPtr<HVE2DHoledShape> pMyResultHoledShape(new HVE2DHoledShape(*this));

        // Add given as hole
        pMyResultHoledShape->AddHole(rHole);

        pResultShape = pMyResultHoledShape.release();
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID  ?
                        DifferentiateRectangleSCS(*((HVE2DRectangle*)(&pi_rShape)))  :
                        pi_rShape.DifferentiateFromShapeSCS(*this));
        }

    return(pResultShape.release());
    }


//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // Declare recipient variable
    HAutoPtr<HVE2DShape> pResultShape;

    // Check if given completely inside rectangle
    if (GetExtent().Contains(pi_rShape.GetExtent()))
        {
        // The given completely inside rectangle
        // The result will therefore be void
        pResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID  ?
                        ((HVE2DRectangle*)(&pi_rShape))->DifferentiateRectangleSCS(*this)  :
                        pi_rShape.DifferentiateShapeSCS(*this));
        }

    return(pResultShape.release());

    }


//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DRectangle::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shape must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    // Declare recipient variable
    HAutoPtr<HVE2DShape> pResultShape;

    // Check if given is completely inside rectangle
    if (GetExtent().Contains(pi_rShape.GetExtent()))
        {
        // The given is completely inside rectangle
        // The result will therefore be given shape
        pResultShape = static_cast<HVE2DShape*>(pi_rShape.Clone());
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID  ?
                        IntersectRectangleSCS(*((HVE2DRectangle*)(&pi_rShape)))  :
                        pi_rShape.IntersectShapeSCS(*this));
        }

    return(pResultShape.release());
    }

//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE
// This method recalculates the tolerance of rectangle if automatic tolerance is
// active
//-----------------------------------------------------------------------------
void HVE2DRectangle::ResetTolerance()
    {
    // Check if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        // Autotolerance active ... update tolerance

        // Set tolerance to minimum value accepted
        double Tolerance = HGLOBAL_EPSILON;

        // Check if a greater tolerance may be applicable
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_XMin));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_XMax));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_YMin));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_YMax));

        // Set tolerance
        SetTolerance(Tolerance);
        }
    }


//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines) for the rectangle.
//-----------------------------------------------------------------------------
void HVE2DRectangle::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    if (pio_rScanlines.GetScanlinesCoordSys() == 0)
        {
        pio_rScanlines.SetScanlinesCoordSys(GetCoordSys());
        }
    HASSERT(pio_rScanlines.GetScanlinesCoordSys() == GetCoordSys());

    // Check if shape is not empty
    if (!IsEmpty())
        {
        if (!pio_rScanlines.LimitsAreSet())
            {
            pio_rScanlines.SetLimits(m_YMin, m_YMax, 2, true);

            }

        if (pio_rScanlines.IsRectangle())
            {
            // Add the two vertical segments X position.
            pio_rScanlines.AddCrossingPoint((int32_t)pio_rScanlines.GetFirstScanlinePosition(), m_XMin);
            pio_rScanlines.AddCrossingPoint((int32_t)pio_rScanlines.GetFirstScanlinePosition(), m_XMax);
            }
        else
            {
            // The result will not be rectangular. Compute intersection points
            // for all scanlines. (Compute 2 vertical segments)
            pio_rScanlines.AddCrossingPointsForSegment(HGF2DPosition(m_XMin, m_YMin), HGF2DPosition(m_XMin, m_YMax));
            pio_rScanlines.AddCrossingPointsForSegment(HGF2DPosition(m_XMax, m_YMin), HGF2DPosition(m_XMax, m_YMax));
            }
        }
    }
