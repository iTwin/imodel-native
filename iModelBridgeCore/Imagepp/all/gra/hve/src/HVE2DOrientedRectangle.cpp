//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DOrientedRectangle.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HVE2DOrientedRectangle
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DOrientedRectangle.h>

HPM_REGISTER_CLASS(HVE2DOrientedRectangle, HVE2DSimpleShape)


#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HVEShapeRasterLine.h>


//-----------------------------------------------------------------------------
// GenerateScanLines
// This method creates the scan lines for the present rectangle
//-----------------------------------------------------------------------------
void HVE2DOrientedRectangle::GenerateScanLines(HVEShapeRasterLine&  pio_rScanlines,
                                               double               pi_rYRef,
                                               double               pi_rYStep,
                                               bool                 pi_rAdd) const
    {
    XScanline MyScanline;

    // The step must be positive
    HPRECONDITION(pi_rYStep > 0.0);

    // Obtain extent of shape
    HGF2DExtent ShapeExtent = GetExtent();

    // Get parameter in the CoordSys of the Shape.
    double YRef  = pi_rYRef;
    double YStep = pi_rYStep;

    double YStart;
    // Calculate YStart for scan lines
    YStart = YRef - (YStep * (round((YRef - (ShapeExtent.GetYMin() - YStep)) / YStep)));
    for (; HDOUBLE_SMALLER_EPSILON(YStart, ShapeExtent.GetYMin()) ; YStart+=YStep);

    // Append scanlines
    // For each valid Y position

    // Set scanline
    MyScanline.XBegin = m_XMin;
//    MyScanline.XEnd   = m_XMax - 1.0;
    MyScanline.XEnd   = m_XMax - (2 * HGLOBAL_EPSILON);
    for (double Y = YStart ; HDOUBLE_SMALLER_OR_EQUAL_EPSILON(Y, ShapeExtent.GetYMax())  ; Y += YStep)
        {
        // Add this scan line
        if (pi_rAdd)
            pio_rScanlines.AddScanLine(Y, MyScanline);
        else
            pio_rScanlines.RemoveScanLine(Y, MyScanline);
        }
    }



//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DOrientedRectangle::GetLinear() const
    {
    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    // Add required parts
    MyComplexLinear.AppendLinear(HVE2DSegment(m_FirstPoint, m_SecondPoint));
    MyComplexLinear.AppendLinear(HVE2DSegment(m_SecondPoint, m_ThirdPoint));
    MyComplexLinear.AppendLinear(HVE2DSegment(m_ThirdPoint, m_FourthPoint));
    MyComplexLinear.AppendLinear(HVE2DSegment(m_FourthPoint, m_FirstPoint));

    return(MyComplexLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DOrientedRectangle::GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
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

    return(MyComplexLinear);
    }

//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear* HVE2DOrientedRectangle::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HVE2DComplexLinear*    pMyComplexLinear = new HVE2DComplexLinear(GetCoordSys());

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

    return(pMyComplexLinear);
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DOrientedRectangle::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
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
HVE2DVector* HVE2DOrientedRectangle::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pMyResultVector;

    // Obtain the transformation model to the given coordinate system
    HFCPtr<HGF2DTransfoModel> pMyModel = GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys);

    // Check if this model preserves direction
    if (pMyModel->PreservesDirection() || pMyModel->IsStretchable())
        {
        // The model concerves non-oriented rectangle shape ... we can transform the points directly
        pMyResultVector = new HVE2DOrientedRectangle(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()).ExpressedIn(pi_rpCoordSys),
                                                     HGF2DLocation(m_XMax, m_YMax, GetCoordSys()).ExpressedIn(pi_rpCoordSys));
        }
    else
        {
        // The model either does not concerve the rectangle shape or is unknown
        // We transform the rectangle into a polygon, and pass and tell the polygon to
        // process itself
        HVE2DPolygonOfSegments    ThePolygon(*this);
        pMyResultVector = ThePolygon.AllocateCopyInCoordSys(pi_rpCoordSys);
        }

    return(pMyResultVector);
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DOrientedRectangle::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
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

    HASSERT(m_XMax >= m_XMin);
    HASSERT(m_YMax >= m_YMin);
    }


//-----------------------------------------------------------------------------
// UnifyRectangleSCS
// This method create a new shape as the union between self and given.
// The two rectangles must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DOrientedRectangle::UnifyRectangleSCS(const HVE2DOrientedRectangle& pi_rRectangle) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rRectangle.GetCoordSys());

    HVE2DShape*     pMyResultShape;

    // For the result to be a rectangle, it is required that the given rectangle
    // either fits exactly one of the sides of the other, or that either one be
    // completely included in the other

    // Check if both rectangles are empty
    if (IsEmpty() && pi_rRectangle.IsEmpty())
        {
        // Since both are empty, then the result is also empty
        pMyResultShape = new HVE2DOrientedRectangle(GetCoordSys());
        }
    else if (IsEmpty())
        {
        // Since self is empty, then the result is given
        pMyResultShape = new HVE2DOrientedRectangle(pi_rRectangle);
        }
    else if (pi_rRectangle.IsEmpty())
        {
        // Since given is empty, then the result is self
        pMyResultShape = new HVE2DOrientedRectangle(*this);
        }
    // Check if given englobs self
    else if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        {
        // Since the given contains self, the result is a copy of given shape
        pMyResultShape = new HVE2DOrientedRectangle(pi_rRectangle);
        }
    // Check if self englobs given
    else if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        {
        // Since self contains given shape, the result is a copy of self
        pMyResultShape = new HVE2DOrientedRectangle(*this);
        }
    // Check is adjoint and perfectly fitted on the left side
    else if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(pi_rRectangle.m_XMin, m_YMin, m_XMax, m_YMax, GetCoordSys());
    // Check is adjoint and perfectly fitted on the right side
    else if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(m_XMin, m_YMin, pi_rRectangle.m_XMax, m_YMax, GetCoordSys());
    // Check is adjoint and perfectly fitted on the lower side
    else if (HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(m_XMin, pi_rRectangle.m_YMin, m_XMax, m_YMax, GetCoordSys());
    // Check is adjoint and perfectly fitted on the upper side
    else if (HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(m_XMin, m_YMin, m_XMax, pi_rRectangle.m_YMax, GetCoordSys());
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER_EPSILON(pi_rRectangle.m_XMin, m_XMax) ||
             HDOUBLE_SMALLER_EPSILON(pi_rRectangle.m_XMax, m_XMin) ||
             HDOUBLE_GREATER_EPSILON(pi_rRectangle.m_YMin, m_YMax) ||
             HDOUBLE_SMALLER_EPSILON(pi_rRectangle.m_YMax, m_YMin))
        {
        // Since rectangles are disjoint, the result is a complex shape
        HVE2DComplexShape* pMyResultComplexShape = new HVE2DComplexShape(GetCoordSys());

        // Add copy of self and given to complex shape
        pMyResultComplexShape->AddShape(*this);
        pMyResultComplexShape->AddShape(pi_rRectangle);

        pMyResultShape = pMyResultComplexShape;
        }
    else
        {
        // In all other case, the result will not be a rectangle nor a complex shape
        // We transform rectangles into polygons and relenquish control to them
        pMyResultShape = HVE2DPolygonOfSegments(*this).UnifyShapeSCS(HVE2DPolygonOfSegments(pi_rRectangle));
//        pMyResultShape = HVE2DSimpleShape::UnifyShapeSCS(pi_rRectangle);
        }

    return (pMyResultShape);
    }

//-----------------------------------------------------------------------------
// DifferentiateRectangleSCS
// This method create a new shape as the difference between self and given rectangle.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DOrientedRectangle::DifferentiateRectangleSCS(const HVE2DOrientedRectangle& pi_rRectangle) const
    {
    // The two shapes must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rRectangle.GetCoordSys());

    HVE2DShape*     pMyResultShape;

    // For the result to be a rectangle, it is required that the given rectangle
    // Chunks out a whole side of self.

    // Check if both rectangles are empty
    if (pi_rRectangle.IsEmpty())
        {
        // Since given is empty, then the result is self
        pMyResultShape = new HVE2DOrientedRectangle(*this);
        }
    else if (IsEmpty())
        {
        // Since self is empty, then the result is empty
        pMyResultShape = new HVE2DOrientedRectangle(GetCoordSys());
        }
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMax) ||
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMin) ||
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMax) ||
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMin))
        {
        // Since rectangle are disjoint, the result is self
        pMyResultShape = new HVE2DOrientedRectangle(*this);
        }
    // Check if given englobs self
    else if (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMin, m_XMin) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_XMax, m_XMax) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMin, m_YMin) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(pi_rRectangle.m_YMax, m_YMax))
        {
        // Since given contains self, the result is an empty rectangle
        pMyResultShape = new HVE2DOrientedRectangle(GetCoordSys());
        }
    // Check if self englobs (COMPLETELY) given
    else if (HDOUBLE_SMALLER_EPSILON(m_XMin, pi_rRectangle.m_XMin) &&
             HDOUBLE_GREATER_EPSILON(m_XMax, pi_rRectangle.m_XMax) &&
             HDOUBLE_SMALLER_EPSILON(m_YMin, pi_rRectangle.m_YMin) &&
             HDOUBLE_GREATER_EPSILON(m_YMax, pi_rRectangle.m_YMax))
        {
        // Since self contains given completely (no contiguous borders), the result is
        // a holed shape with self as outter shape and given as hole
        HVE2DHoledShape* pResultHoledShape = new HVE2DHoledShape(*this);

        pResultHoledShape->AddHole(pi_rRectangle);

        pMyResultShape = pResultHoledShape;
        }
    // Check if Left side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_XMin, pi_rRectangle.m_XMin) &&
             HDOUBLE_GREATER_EPSILON(m_XMax, pi_rRectangle.m_XMax) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_YMin, pi_rRectangle.m_YMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_YMax, pi_rRectangle.m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(pi_rRectangle.m_XMax, m_YMin, m_XMax, m_YMax, GetCoordSys());
    // Check if Right side is no more
    else if (HDOUBLE_SMALLER_EPSILON(m_XMin, pi_rRectangle.m_XMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_XMax, pi_rRectangle.m_XMax) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_YMin, pi_rRectangle.m_YMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_YMax, pi_rRectangle.m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(m_XMin, m_YMin, pi_rRectangle.m_XMin, m_YMax, GetCoordSys());
    // Check if Lower side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_XMin, pi_rRectangle.m_XMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_XMax, pi_rRectangle.m_XMax) &&
             HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_YMin, pi_rRectangle.m_YMin) &&
             HDOUBLE_GREATER_EPSILON(m_YMax, pi_rRectangle.m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(m_XMin, pi_rRectangle.m_YMax, m_XMax, m_YMax, GetCoordSys());
    // Check if Upper side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_XMin, pi_rRectangle.m_XMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_XMax, pi_rRectangle.m_XMax) &&
             HDOUBLE_SMALLER_EPSILON(m_YMin, pi_rRectangle.m_YMin) &&
             HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_YMax, pi_rRectangle.m_YMax))
        pMyResultShape = new HVE2DOrientedRectangle(m_XMin, m_YMin, m_XMax, pi_rRectangle.m_YMin, GetCoordSys());
    else
        {
        // In all other case, the result will not be a rectangle
        // We transform rectangles into polygons and relenquish control to them
        pMyResultShape = HVE2DPolygonOfSegments(*this).DifferentiateShapeSCS(HVE2DPolygonOfSegments(pi_rRectangle));
//        pMyResultShape = HVE2DSimpleShape::DifferentiateShapeSCS(pi_rRectangle);
        }

    return (pMyResultShape);
    }




//-----------------------------------------------------------------------------
// IntersectRectangleSCS
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DOrientedRectangle::IntersectRectangleSCS(const HVE2DOrientedRectangle& pi_rRectangle) const
    {
    // The two rectangle must have the same coordinate system
    HPRECONDITION (GetCoordSys() == pi_rRectangle.GetCoordSys());

    HVE2DShape* pMyResultShape;

    // Check if both rectangles are empty
    if (IsEmpty() || pi_rRectangle.IsEmpty())
        {
        // Since both are empty, then the result is also empty
        pMyResultShape = new HVE2DOrientedRectangle(GetCoordSys());
        }
    else
        {
        // The intersection of two rectangles always yields a rectangle
        // Allocate copy of this to obtain result
        HVE2DOrientedRectangle*     pMyNewRectangle = new HVE2DOrientedRectangle(*this);

        if (pMyNewRectangle->m_XMin < pi_rRectangle.m_XMin)
            pMyNewRectangle->m_XMin = pi_rRectangle.m_XMin;

        if (pMyNewRectangle->m_XMax > pi_rRectangle.m_XMax)
            pMyNewRectangle->m_XMax = pi_rRectangle.m_XMax;

        if (pMyNewRectangle->m_YMin < pi_rRectangle.m_YMin)
            pMyNewRectangle->m_YMin = pi_rRectangle.m_YMin;

        if (pMyNewRectangle->m_YMax > pi_rRectangle.m_YMax)
            pMyNewRectangle->m_YMax = pi_rRectangle.m_YMax;

        // Note that as a result of the previous operations, the result may be
        // an empty rectangle

        if (pMyNewRectangle->m_XMax < pMyNewRectangle->m_XMin ||
            pMyNewRectangle->m_YMax < pMyNewRectangle->m_YMin)
            {
            // Set to indicate empty rectangle
            pMyNewRectangle->m_XMax = pMyNewRectangle->m_XMin;
            pMyNewRectangle->m_YMax = pMyNewRectangle->m_YMin;
            }

        pMyResultShape = pMyNewRectangle;
        }

    return (pMyResultShape);
    }


#if (0)
//-----------------------------------------------------------------------------
// Flirts
// This method checks if the rectangle flirts with given vector.
//-----------------------------------------------------------------------------
bool HVE2DOrientedRectangle::Flirts(const HVE2DVector& pi_rVector) const
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
bool HVE2DOrientedRectangle::Crosses(const HVE2DVector& pi_rVector) const
    {
    // Since crossing computations are so complicated, we tel HVE2DPolygonOfSegments to perform
    // the operation
    return (HVE2DPolygonOfSegments(*this).Crosses(pi_rVector));
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
int HVE2DOrientedRectangle::Intersect(const HVE2DVector& pi_rVector,
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
bool HVE2DOrientedRectangle::AreContiguousAt(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));


    bool   AreContiguousAtPoint = false;

    // Create first segment
    HVE2DSegment    MySegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMax, GetCoordSys()));

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
void HVE2DOrientedRectangle::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
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
bool HVE2DOrientedRectangle::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                         HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return (HVE2DPolygonOfSegments(*this).ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints));
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
double HVE2DOrientedRectangle::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
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
HGFBearing HVE2DOrientedRectangle::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return (HVE2DPolygonOfSegments(*this).CalculateBearing(pi_rPoint, pi_Direction));
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DOrientedRectangle::CalculateSpatialPositionOfSingleComponentVector(const HVE2DVector& pi_rVector) const
    {
    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    HGF2DExtent VectorExtent(pi_rVector.GetExtent());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(VectorExtent))
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
            if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(VectorXMin, m_XMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(VectorXMax, m_XMax) &&
                HDOUBLE_GREATER_OR_EQUAL_EPSILON(VectorYMin, m_YMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(VectorYMax, m_YMax))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL_EPSILON(VectorXMin, m_XMin) ||
                    HDOUBLE_EQUAL_EPSILON(VectorXMax, m_XMax) ||
                    HDOUBLE_EQUAL_EPSILON(VectorYMin, m_YMin) ||
                    HDOUBLE_EQUAL_EPSILON(VectorYMax, m_YMax))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                    }
                else
                    {
                    // Extent is completely inside of rectangle... the only possible answer is S_IN
                    ThePosition = HVE2DShape::S_IN;
                    }
                }
            else
                {
                // The extents outter overlap, but vector is not completely in rectangle ...
                // Check if extents inner overlap ..
                if (!GetExtent().InnerOverlaps(VectorExtent))
                    {
                    // The extents do outter overlap, but not inner overlap ...
                    // We cannot conclude ...
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                    }
                else
                    {
                    // Check if the extent overlaps only one rectangle side
                    // or two opposite sides
                    if (((HDOUBLE_SMALLER_EPSILON(VectorXMin, m_XMin) || HDOUBLE_GREATER_EPSILON(VectorXMax, m_XMax)) &&
                         (!HDOUBLE_SMALLER_EPSILON(VectorYMin, m_YMin) && !HDOUBLE_GREATER_EPSILON(VectorYMax, m_YMax))) ||
                        ((HDOUBLE_SMALLER_EPSILON(VectorYMin, m_YMin) || HDOUBLE_GREATER_EPSILON(VectorYMax, m_YMax)) &&
                         (!HDOUBLE_SMALLER_EPSILON(VectorXMin, m_XMin) && !HDOUBLE_GREATER_EPSILON(VectorXMax, m_XMax))))
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
            if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(VectorXMin, m_XMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(VectorXMax, m_XMax) &&
                HDOUBLE_GREATER_OR_EQUAL_EPSILON(VectorYMin, m_YMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(VectorYMax, m_YMax))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL_EPSILON(VectorXMin, m_XMin) ||
                    HDOUBLE_EQUAL_EPSILON(VectorXMax, m_XMax) ||
                    HDOUBLE_EQUAL_EPSILON(VectorYMin, m_YMin) ||
                    HDOUBLE_EQUAL_EPSILON(VectorYMax, m_YMax))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOf(pi_rVector);
                    }
                else
                    {
                    // Extent is completely inside of rectangle... the only possible answer is S_IN
                    ThePosition = HVE2DShape::S_IN;
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
HVE2DShape::SpatialPosition HVE2DOrientedRectangle::CalculateSpatialPositionOfNonCrossingLinear(const HVE2DLinear& pi_rLinear) const
    {
    // The two vectors must not cross
    HPRECONDITION(!Crosses(pi_rLinear));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    HGF2DExtent LinearExtent(pi_rLinear.GetExtent());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(LinearExtent))
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
            if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(LinearXMin, m_XMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(LinearXMax, m_XMax) &&
                HDOUBLE_GREATER_OR_EQUAL_EPSILON(LinearYMin, m_YMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(LinearYMax, m_YMax))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL_EPSILON(LinearXMin, m_XMin) ||
                    HDOUBLE_EQUAL_EPSILON(LinearXMax, m_XMax) ||
                    HDOUBLE_EQUAL_EPSILON(LinearYMin, m_YMin) ||
                    HDOUBLE_EQUAL_EPSILON(LinearYMax, m_YMax))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                else
                    {
                    // Extent is completely inside of rectangle... the only possible answer is S_IN
                    ThePosition = HVE2DShape::S_IN;
                    }
                }
            else
                {
                // Check if extents inner overlap ..
                if (!GetExtent().InnerOverlaps(LinearExtent))
                    {
                    // The extents do outter overlap, but not inner overlap ...
                    // We cannot conclude ...
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                else
                    {
                    // Check if the extent overlaps only one rectangle side
                    // or two opposite sides
                    if (((HDOUBLE_SMALLER_EPSILON(LinearXMin, m_XMin) || HDOUBLE_GREATER_EPSILON(LinearXMax, m_XMax)) &&
                         (!HDOUBLE_SMALLER_EPSILON(LinearYMin, m_YMin) && !HDOUBLE_GREATER_EPSILON(LinearYMax, m_YMax))) ||
                        ((HDOUBLE_SMALLER_EPSILON(LinearYMin, m_YMin) || HDOUBLE_GREATER_EPSILON(LinearYMax, m_YMax)) &&
                         (!HDOUBLE_SMALLER_EPSILON(LinearXMin, m_XMin) && !HDOUBLE_GREATER_EPSILON(LinearXMax, m_XMax))))
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
            if (HDOUBLE_GREATER_OR_EQUAL_EPSILON(LinearXMin, m_XMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(LinearXMax, m_XMax) &&
                HDOUBLE_GREATER_OR_EQUAL_EPSILON(LinearYMin, m_YMin) &&
                HDOUBLE_SMALLER_OR_EQUAL_EPSILON(LinearYMax, m_YMax))
                {
                // The vector is completely IN or ON rectangle
                // Check if possible ON
                if (HDOUBLE_EQUAL_EPSILON(LinearXMin, m_XMin) ||
                    HDOUBLE_EQUAL_EPSILON(LinearXMax, m_XMax) ||
                    HDOUBLE_EQUAL_EPSILON(LinearYMin, m_YMin) ||
                    HDOUBLE_EQUAL_EPSILON(LinearYMax, m_YMax))
                    {
                    // The vector is quite possibly ON the rectangle ... need more precise operation
                    ThePosition = HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                else
                    {
                    // Extent is completely inside of rectangle... the only possible answer is S_IN
                    ThePosition = HVE2DShape::S_IN;
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
inline bool HVE2DOrientedRectangle::AreAdjacent(const HVE2DVector& pi_rVector) const
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
inline bool HVE2DOrientedRectangle::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return (HVE2DSegment(HGF2DLocation(m_XMin, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMax, GetCoordSys())).AreContiguous(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMin, m_YMax, GetCoordSys()), HGF2DLocation(m_XMax, m_YMax, GetCoordSys())).AreContiguous(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMax, m_YMax, GetCoordSys()), HGF2DLocation(m_XMax, m_YMin, GetCoordSys())).AreContiguous(pi_rVector) ||
            HVE2DSegment(HGF2DLocation(m_XMax, m_YMin, GetCoordSys()), HGF2DLocation(m_XMin, m_YMin, GetCoordSys())).AreContiguous(pi_rVector));
    }

