//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DRectangle.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DRectangle
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DRectangle.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>



#include <Imagepp/all/h/HGF2DSegment.h>
#include <Imagepp/all/h/HGF2DComplexShape.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>
#include <Imagepp/all/h/HGF2DHoledShape.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear describing the path of the rectangle
//-----------------------------------------------------------------------------
HFCPtr<HGF2DLinear> HGF2DRectangle::GetLinear() const
    {
    // Allocate linear
    HGF2DPolySegment*    pMyLinear = new HGF2DPolySegment();

    // Add required parts
    pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMin));
    pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMax));
    pMyLinear->AppendPoint(HGF2DPosition(m_XMax, m_YMax));
    pMyLinear->AppendPoint(HGF2DPosition(m_XMax, m_YMin));
    pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMin));

    // Copy tolerance settings
    pMyLinear->SetTolerance(GetTolerance());
    pMyLinear->SetAutoToleranceActive(IsAutoToleranceActive());

    return pMyLinear;
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HFCPtr<HGF2DLinear> HGF2DRectangle::GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HGF2DPolySegment*    pMyLinear = new HGF2DPolySegment();

    if (pi_DirectionDesired == HGF2DSimpleShape::CW)
        {

        // Create CW linear
        pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMin));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMax));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMax, m_YMax));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMax, m_YMin));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMin));
        }
    else
        {
        pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMin));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMax, m_YMin));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMax, m_YMax));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMax));
        pMyLinear->AppendPoint(HGF2DPosition(m_XMin, m_YMin));
        }

    // Copy tolerance settings
    pMyLinear->SetTolerance(GetTolerance());
    pMyLinear->SetAutoToleranceActive(IsAutoToleranceActive());


    return pMyLinear;
    }


/** -----------------------------------------------------------------------------
    This method sets the spatial definition of the rectangle.

    @param pi_rOrigin Location specifying the origin of rectangle.

    @param pi_rCorner Location specifying the corner of rectangle.

    Example
    @code
    @end

    @see GetRectangle()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
void HGF2DRectangle::SetRectangle(const HGF2DPosition& pi_rFirstPoint,
                                 const HGF2DPosition& pi_rSecondPoint)
    {
    // Set extreme values
    m_XMin = MIN(pi_rFirstPoint.GetX(), pi_rSecondPoint.GetX());
    m_XMax = MAX(pi_rFirstPoint.GetX(), pi_rSecondPoint.GetX());
    m_YMin = MIN(pi_rFirstPoint.GetY(), pi_rSecondPoint.GetY());
    m_YMax = MAX(pi_rFirstPoint.GetY(), pi_rSecondPoint.GetY());

    // Set tolerance if necessary
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
HGF2DPosition HGF2DRectangle::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    // Obtain closest point for a first segment of rectangle
    HGF2DPosition   ClosestPoint(HGF2DSegment(HGF2DPosition(m_XMin, m_YMin),
                                            HGF2DPosition(m_XMin, m_YMax)).CalculateClosestPoint(pi_rPoint));

    // Obtain the closest point to another segment of rectangle
    HGF2DPosition   WorkPoint(HGF2DSegment(HGF2DPosition(m_XMin, m_YMax),
                                         HGF2DPosition(m_XMax, m_YMax)).CalculateClosestPoint(pi_rPoint));

    // Check which is the closest to test point
    if ((WorkPoint - pi_rPoint).CalculateLength() < (ClosestPoint - pi_rPoint).CalculateLength())
        {
        // The second is closest ... it is our temporay closest point
        ClosestPoint = WorkPoint;
        }

    // Obtain the closest point for a third segment of rectangle
    WorkPoint = HGF2DSegment(HGF2DPosition(m_XMax, m_YMax),
                           HGF2DPosition(m_XMax, m_YMin)).CalculateClosestPoint(pi_rPoint);

    // Check which is the closest (old one or third closest point) to test point
    if ((WorkPoint - pi_rPoint).CalculateLength() < (ClosestPoint - pi_rPoint).CalculateLength())
        {
        // The third is closest ... it is our temporay closest point
        ClosestPoint = WorkPoint;
        }

    // Obtain the closest point for a fourth and final segment of rectangle
    WorkPoint = HGF2DSegment(HGF2DPosition(m_XMax, m_YMin),
                           HGF2DPosition(m_XMin, m_YMin)).CalculateClosestPoint(pi_rPoint);

    // Check which is the closest (old one or fourth closest point) to test point
    if ((WorkPoint - pi_rPoint).CalculateLength() < (ClosestPoint - pi_rPoint).CalculateLength())
        {
        // The fourth is closest ... it is our final closest point
        ClosestPoint = WorkPoint;
        }

    return (ClosestPoint);
    }



//-----------------------------------------------------------------------------
// Scale
// This method scales the rectangle by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HGF2DRectangle::Scale(double pi_ScaleFactor, const HGF2DPosition& pi_rScaleOrigin)
    {
    // The given scale factor must not be zero
    HPRECONDITION(pi_ScaleFactor != 0.0);
    // Change values
    m_XMin = pi_rScaleOrigin.GetX() + ((m_XMin - pi_rScaleOrigin.GetX()) * pi_ScaleFactor);
    m_YMin = pi_rScaleOrigin.GetY() + ((m_YMin - pi_rScaleOrigin.GetY()) * pi_ScaleFactor);
    m_XMax = pi_rScaleOrigin.GetX() + ((m_XMax - pi_rScaleOrigin.GetX()) * pi_ScaleFactor);
    m_YMax = pi_rScaleOrigin.GetY() + ((m_YMax - pi_rScaleOrigin.GetY()) * pi_ScaleFactor);

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
void HGF2DRectangle::Scale(double pi_ScaleFactorX,
                         double pi_ScaleFactorY,
                         const HGF2DPosition& pi_rScaleOrigin)
    {
    // The given scale factors must not be zero
    HPRECONDITION(pi_ScaleFactorX != 0.0);
    HPRECONDITION(pi_ScaleFactorY != 0.0);

    // Change values
    m_XMin = pi_rScaleOrigin.GetX() + ((m_XMin - pi_rScaleOrigin.GetX()) * pi_ScaleFactorX);
    m_YMin = pi_rScaleOrigin.GetY() + ((m_YMin - pi_rScaleOrigin.GetY()) * pi_ScaleFactorY);
    m_XMax = pi_rScaleOrigin.GetX() + ((m_XMax - pi_rScaleOrigin.GetX()) * pi_ScaleFactorX);
    m_YMax = pi_rScaleOrigin.GetY() + ((m_YMax - pi_rScaleOrigin.GetY()) * pi_ScaleFactorY);

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
// UnifyRectangle
// This method create a new shape as the union between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::UnifyRectangle(const HGF2DRectangle& pi_rRectangle) const
    {
    HAutoPtr<HGF2DShape>     pMyResultShape;

    double Tolerance = MIN(pi_rRectangle.GetTolerance(), GetTolerance());

    // For the result to be a rectangle, it is required that the given rectangle
    // either fits exactly one of the sides of the other, or that either one be
    // completely included in the other

    // Check if both rectangles are empty
    if (IsEmpty() && pi_rRectangle.IsEmpty())
        {
        // Since both are empty, then the result is also empty
        pMyResultShape = new HGF2DVoidShape();
        }
    else if (IsEmpty())
        {
        // Since self is empty, then the result is given
        pMyResultShape = new HGF2DRectangle(pi_rRectangle);
        }
    else if (pi_rRectangle.IsEmpty())
        {
        // Since given is empty, then the result is self
        pMyResultShape = new HGF2DRectangle(*this);
        }
    // Check if given englobs self
    else if (HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        {
        // Since the given contains self, the result is a copy of given shape
        pMyResultShape = new HGF2DRectangle(pi_rRectangle);
        }
    // Check if self englobs given
    else if (HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        {
        // Since self contains given shape, the result is a copy of self
        pMyResultShape = new HGF2DRectangle(*this);
        }
    // Check is adjoint and perfectly fitted on the left or right side
    else if (HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMin, Tolerance) &&
             HDOUBLE_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        pMyResultShape = new HGF2DRectangle(MIN(pi_rRectangle.m_XMin, m_XMin), m_YMin, MAX(pi_rRectangle.m_XMax, m_XMax), m_YMax);
    // Check is adjoint and perfectly fitted on the lower side
    else if (HDOUBLE_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        pMyResultShape = new HGF2DRectangle(m_XMin, MIN(pi_rRectangle.m_YMin, m_YMin), m_XMax, MAX(pi_rRectangle.m_YMax, m_YMax));
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER(pi_rRectangle.m_XMin, m_XMax, Tolerance) ||
             HDOUBLE_SMALLER(pi_rRectangle.m_XMax, m_XMin, Tolerance) ||
             HDOUBLE_GREATER(pi_rRectangle.m_YMin, m_YMax, Tolerance) ||
             HDOUBLE_SMALLER(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        {
        // Since rectangles are disjoint, the result is a complex shape
        HAutoPtr<HGF2DComplexShape> pMyResultComplexShape(new HGF2DComplexShape());

        // Add copy of self and given to complex shape
        pMyResultComplexShape->AddShape(*this);
        pMyResultComplexShape->AddShape(pi_rRectangle);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // In all other case, the result will not be a rectangle nor a complex shape
        // We transform rectangles into polygons and relenquish control to them
        pMyResultShape = HGF2DPolygonOfSegments(*this).UnifyShape(HGF2DPolygonOfSegments(pi_rRectangle));
        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateRectangle
// This method create a new shape as the difference between self and given rectangle.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::DifferentiateRectangle(const HGF2DRectangle& pi_rRectangle) const
    {
    HAutoPtr<HGF2DShape>     pMyResultShape;

    double Tolerance = MIN(pi_rRectangle.GetTolerance(), GetTolerance());

    // For the result to be a rectangle, it is required that the given rectangle
    // Chunks out a whole side of self.

    // Check if both rectangles are empty
    if (pi_rRectangle.IsEmpty())
        {
        // Since given is empty, then the result is self
        pMyResultShape = new HGF2DRectangle(*this);
        }
    else if (IsEmpty())
        {
        // Since self is empty, then the result is empty
        pMyResultShape = new HGF2DVoidShape();
        }
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMin, Tolerance) ||
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        {
        // Since rectangle are disjoint, the result is self
        pMyResultShape = new HGF2DRectangle(*this);
        }
    // Check if given englobs self
    else if (HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMax, Tolerance))
        {
        // Since given contains self, the result is an empty rectangle
        pMyResultShape = new HGF2DVoidShape();
        }
    // Check if self englobs (COMPLETELY) given
    else if (HDOUBLE_SMALLER(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_GREATER(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_SMALLER(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_GREATER(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        {
        // Since self contains given completely (no contiguous borders), the result is
        // a holed shape with self as outter shape and given as hole
        HAutoPtr<HGF2DHoledShape> pResultHoledShape(new HGF2DHoledShape(*this));

        pResultHoledShape->AddHole(pi_rRectangle);

        pMyResultShape = pResultHoledShape.release();
        }
    // Check if Left side is no more
    else if (HDOUBLE_SMALLER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        pMyResultShape = new HGF2DRectangle(pi_rRectangle.m_XMax, m_YMin, m_XMax, m_YMax);
    // Check if Right side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        pMyResultShape = new HGF2DRectangle(m_XMin, m_YMin, pi_rRectangle.m_XMin, m_YMax);
    // Check if Lower side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance))
        pMyResultShape = new HGF2DRectangle(m_XMin, pi_rRectangle.m_YMax, m_XMax, m_YMax);
    // Check if Upper side is no more
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        pMyResultShape = new HGF2DRectangle(m_XMin, m_YMin, m_XMax, pi_rRectangle.m_YMin);
    // Check if given splits self in two parts (given is vertical)
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance))
        {
        HAutoPtr<HGF2DComplexShape> pComplex(new HGF2DComplexShape());

        pComplex->AddShape(HGF2DRectangle(m_XMin, m_YMin, pi_rRectangle.m_XMin, m_YMax));
        pComplex->AddShape(HGF2DRectangle(pi_rRectangle.m_XMax, m_YMin, m_XMax, m_YMax));

        pMyResultShape = pComplex.release();
        }
    // Check if given splits self in two parts (given is horizontal)
    else if (HDOUBLE_GREATER_OR_EQUAL(m_XMin, pi_rRectangle.m_XMin, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_XMax, pi_rRectangle.m_XMax, Tolerance) &&
             HDOUBLE_GREATER_OR_EQUAL(m_YMax, pi_rRectangle.m_YMax, Tolerance) &&
             HDOUBLE_SMALLER_OR_EQUAL(m_YMin, pi_rRectangle.m_YMin, Tolerance))
        {
        HAutoPtr<HGF2DComplexShape> pComplex(new HGF2DComplexShape());

        pComplex->AddShape(HGF2DRectangle(m_XMin, m_YMin, m_XMax, pi_rRectangle.m_YMin));
        pComplex->AddShape(HGF2DRectangle(m_XMin, pi_rRectangle.m_YMax, m_XMax, m_YMax));

        pMyResultShape = pComplex.release();
        }
    else
        {
        // In all other case, the result will not be a rectangle
        // We transform rectangles into polygons and relenquish control to them
        pMyResultShape = HGF2DPolygonOfSegments(*this).DifferentiateShape(HGF2DPolygonOfSegments(pi_rRectangle));
        }

    return (pMyResultShape.release());
    }




//-----------------------------------------------------------------------------
// IntersectRectangle
// This method create a new shape as the intersection between self and given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::IntersectRectangle(const HGF2DRectangle& pi_rRectangle) const
    {
    HAutoPtr<HGF2DShape> pMyResultShape;

    double Tolerance = MIN(pi_rRectangle.GetTolerance(), GetTolerance());

    // Check if both rectangles are empty
    if (IsEmpty() || pi_rRectangle.IsEmpty())
        {
        // Since both are empty, then the result is also empty
        pMyResultShape = new HGF2DVoidShape();
        }
    // Check if rectangles are disjoint
    else if (HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_XMin, m_XMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_XMax, m_XMin, Tolerance) ||
             HDOUBLE_GREATER_OR_EQUAL(pi_rRectangle.m_YMin, m_YMax, Tolerance) ||
             HDOUBLE_SMALLER_OR_EQUAL(pi_rRectangle.m_YMax, m_YMin, Tolerance))
        {
        // Since rectangle are disjoint, the result is void
        pMyResultShape = new HGF2DVoidShape();
        }
    else
        {
        // The intersection of two rectangles always yields a rectangle
        // Allocate copy of this to obtain result
        pMyResultShape = new HGF2DRectangle(MAX(m_XMin, pi_rRectangle.m_XMin),
                                          MAX(m_YMin, pi_rRectangle.m_YMin),
                                          MIN(m_XMax, pi_rRectangle.m_XMax),
                                          MIN(m_YMax, pi_rRectangle.m_YMax));
        }

    return (pMyResultShape.release());
    }



//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
bool HGF2DRectangle::Crosses(const HGF2DVector& pi_rVector) const
    {
    bool Answer = false;

    // Obtain vector extent
    HGF2DLiteExtent VectorExtent(pi_rVector.GetExtent());

    // Obtain minimal tolerance
    double Tolerance = MIN(pi_rVector.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().InnerOverlaps(VectorExtent, Tolerance))
        {
        // Possible interaction ...
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
                if (pi_rVector.GetMainVectorType() == HGF2DLinear::CLASS_ID ||
                    (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
                     ((HGF2DShape*)&pi_rVector)->IsSimple()))
                    {
                    Answer = true;
                    }
                else
                    {
                    // Too complicated ...
                    Answer = HGF2DPolygonOfSegments(*this).Crosses(pi_rVector);
                    }
                }
            else
                {
                // Check if vector is a rectangle
                if (pi_rVector.GetClassID() == HGF2DRectangle::CLASS_ID)
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
                    Answer = HGF2DPolygonOfSegments(*this).Crosses(pi_rVector);
                    }
                }
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// Intersect
// This method checks if the intersects with given vector and returns cross points
//-----------------------------------------------------------------------------
size_t HGF2DRectangle::Intersect(const HGF2DVector& pi_rVector,
                               HGF2DPositionCollection* po_pCrossPoints) const
    {
    // Since intersect computations are so complicated, we tel HGF2DPolygonOfSegments to perform
    // the operation
    return (HGF2DPolygonOfSegments(*this).Intersect(pi_rVector, po_pCrossPoints));
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the rectangle is contiguous with given vector
// at specified point.
//-----------------------------------------------------------------------------
bool HGF2DRectangle::AreContiguousAt(const HGF2DVector& pi_rVector,
                                      const HGF2DPosition& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));


    bool   AreContiguousAtPoint = false;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    // Create first segment
    HGF2DSegment    MySegment(HGF2DPosition(m_XMin, m_YMin), HGF2DPosition(m_XMin, m_YMax));

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
        MySegment.SetStartPoint(HGF2DPosition(m_XMin, m_YMax));
        MySegment.SetEndPoint(HGF2DPosition(m_XMax, m_YMax));

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
        MySegment.SetStartPoint(HGF2DPosition(m_XMax, m_YMax));
        MySegment.SetEndPoint(HGF2DPosition(m_XMax, m_YMin));

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
        MySegment.SetStartPoint(HGF2DPosition(m_XMax, m_YMin));
        MySegment.SetEndPoint(HGF2DPosition(m_XMin, m_YMin));

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
void HGF2DRectangle::ObtainContiguousnessPointsAt(const HGF2DVector& pi_rVector,
                                                const HGF2DPosition& pi_rPoint,
                                                HGF2DPosition* po_pFirstContiguousnessPoint,
                                                HGF2DPosition* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Create a polygon and process by it
    HGF2DPolygonOfSegments(*this).ObtainContiguousnessPointsAt(pi_rVector,
                                                               pi_rPoint,
                                                               po_pFirstContiguousnessPoint,
                                                               po_pSecondContiguousnessPoint);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// This method returns the contiguousness points between rectangle and vector
//-----------------------------------------------------------------------------
size_t HGF2DRectangle::ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                  HGF2DPositionCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(AreContiguous(pi_rVector));

    return (HGF2DPolygonOfSegments(*this).ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints));
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of rectangle at specified point
//-----------------------------------------------------------------------------
double HGF2DRectangle::CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                                                    HGF2DVector::ArbitraryDirection pi_Direction) const
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
HGFBearing HGF2DRectangle::CalculateBearing(const HGF2DPosition& pi_rPoint,
                                            HGF2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    return (HGF2DPolygonOfSegments(*this).CalculateBearing(pi_rPoint, pi_Direction));
    }

//-----------------------------------------------------------------------------
// Overlaps
// This method indicates if the rectangle overlaps the given shape.
// This method is equivalent to obtaining a spatial position different from OUT
// without going into details concerning the exact position (IN ON PARTIALLY_IN)
// It is therefore faster to operate if the only answer needed is overlaps
//-----------------------------------------------------------------------------
bool HGF2DRectangle::Overlaps(const HGF2DShape& pi_rShape) const
    {
    bool DoesOverlap = false;

    HGF2DLiteExtent ShapeExtent(pi_rShape.GetExtent());

    // Obtain minimal tolerance
    double Tolerance = MIN(pi_rShape.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().InnerOverlaps(ShapeExtent, Tolerance))
        {
        // Possible interaction ...
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
        
    return(DoesOverlap);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DRectangle::CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const
    {
    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    HGF2DLiteExtent VectorExtent(pi_rVector.GetExtent());

    // Obtain minimal tolerance
    double Tolerance = MIN(pi_rVector.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(VectorExtent, Tolerance))
        {
        // Possible interaction ...
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
                ThePosition = HGF2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
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
                    ThePosition = HGF2DShape::S_IN;
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
                        ThePosition = HGF2DShape::S_ON;
                        }
                    else
                        {
                        // The vector has not a flat extent upon a boundary
                        ThePosition = HGF2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
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
                ThePosition = HGF2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
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
                    ThePosition = HGF2DShape::S_PARTIALY_IN;
                    }
                else
                    {
                    // The extent overlaps consecutive sides of rectangle ... we cannot conclude
                    ThePosition = HGF2DShape::CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
                    }
                }
            }
        }

    return (ThePosition);
    }



//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfNonCrossingLinear
// This method returns the spatial position relative to shape of given linear
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DRectangle::CalculateSpatialPositionOfNonCrossingLinear(const HGF2DLinear& pi_rLinear) const
    {
    // The two vectors must not cross
    HPRECONDITION(!Crosses(pi_rLinear));

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    HGF2DLiteExtent LinearExtent(pi_rLinear.GetExtent());

    double Tolerance = MIN(pi_rLinear.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(LinearExtent, Tolerance))
        {
        // Possible interaction ...
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
                ThePosition = HGF2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
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
                    ThePosition = HGF2DShape::S_IN;
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
                        ThePosition = HGF2DShape::S_ON;
                        }
                    else
                        {
                        // The vector is quite possibly ON the rectangle ... need more precise operation
                        ThePosition = HGF2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
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
                ThePosition = HGF2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
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
                    ThePosition = HGF2DShape::S_PARTIALY_IN;
                    }
                else
                    {
                    // The extent overlaps consecutive sides of rectangle ... we cannot conclude
                    ThePosition = HGF2DShape::CalculateSpatialPositionOfNonCrossingLinear(pi_rLinear);
                    }
                }
            }
        }
        

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the rectangle is adjacent with given vector.
//-----------------------------------------------------------------------------
bool HGF2DRectangle::AreAdjacent(const HGF2DVector& pi_rVector) const
    {
    return (HGF2DSegment(HGF2DPosition(m_XMin, m_YMin), HGF2DPosition(m_XMin, m_YMax)).AreAdjacent(pi_rVector) ||
            HGF2DSegment(HGF2DPosition(m_XMin, m_YMax), HGF2DPosition(m_XMax, m_YMax)).AreAdjacent(pi_rVector) ||
            HGF2DSegment(HGF2DPosition(m_XMax, m_YMax), HGF2DPosition(m_XMax, m_YMin)).AreAdjacent(pi_rVector) ||
            HGF2DSegment(HGF2DPosition(m_XMax, m_YMin), HGF2DPosition(m_XMin, m_YMin)).AreAdjacent(pi_rVector));
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the rectangle is contiguous with given vector.
//-----------------------------------------------------------------------------
bool HGF2DRectangle::AreContiguous(const HGF2DVector& pi_rVector) const
    {
    return (HGF2DSegment(HGF2DPosition(m_XMin, m_YMin), HGF2DPosition(m_XMin, m_YMax)).AreContiguous(pi_rVector) ||
            HGF2DSegment(HGF2DPosition(m_XMin, m_YMax), HGF2DPosition(m_XMax, m_YMax)).AreContiguous(pi_rVector) ||
            HGF2DSegment(HGF2DPosition(m_XMax, m_YMax), HGF2DPosition(m_XMax, m_YMin)).AreContiguous(pi_rVector) ||
            HGF2DSegment(HGF2DPosition(m_XMax, m_YMin), HGF2DPosition(m_XMin, m_YMin)).AreContiguous(pi_rVector));
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DRectangle::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char     DumString[256];

    HGF2DSimpleShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HGF2DRectangle" << endl;
    HDUMP0("Object is a HGF2DRectangle\n");

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
// Drop
// Returns the description of linear in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HGF2DRectangle::Drop(HGF2DPositionCollection* po_pPoints,
                        double pi_rTolerance) const
    {
    HPRECONDITION(po_pPoints != 0);

    // Check if rectangle is not empty
    if (!IsEmpty())
        {
        // Reserve space for 5 points
        po_pPoints->reserve(5);

        // Add points
        po_pPoints->push_back(HGF2DPosition(m_XMin, m_YMin));
        po_pPoints->push_back(HGF2DPosition(m_XMin, m_YMax));
        po_pPoints->push_back(HGF2DPosition(m_XMax, m_YMax));
        po_pPoints->push_back(HGF2DPosition(m_XMax, m_YMin));
        po_pPoints->push_back(HGF2DPosition(m_XMin, m_YMin));
        }
    }


//-----------------------------------------------------------------------------
// UnifyShape
// This method create a new shape as the union between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::UnifyShape(const HGF2DShape& pi_rShape) const
    {
    // Declare recipient variable
    HAutoPtr<HGF2DShape> pResultShape;

    // Check if given is completely inside rectangle
    if (GetExtent().Contains(pi_rShape.GetExtent()))
        {
        // The given completely inside rectangle
        // The result will therefore be the rectangle itself
        pResultShape = new HGF2DRectangle(*this);
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HGF2DRectangle::CLASS_ID  ?
                        UnifyRectangle(*((HGF2DRectangle*)(&pi_rShape)))  :
                        pi_rShape.UnifyShape(*this));
        }

    pResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return(pResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateShape
// This method create a new shape as the difference between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::DifferentiateShape(const HGF2DShape& pi_rShape) const
    {
    // Declare recipient variable
    HAutoPtr<HGF2DShape> pResultShape;

    // Check if given is a simple shape and completely inside rectangle
    if (pi_rShape.IsSimple() &&
        GetExtent().InnerContains(pi_rShape.GetExtent(), MIN(GetTolerance(), pi_rShape.GetTolerance())))
        {
        // The given is a simple shape and completely inside rectangle
        // The result will therefore be a holed shape

        // Cast the shape into a simple shape
        const HGF2DSimpleShape& rHole = static_cast<const HGF2DSimpleShape&>(pi_rShape);

        // Create holed shape
        HAutoPtr<HGF2DHoledShape> pMyResultHoledShape(new HGF2DHoledShape(*this));

        // Add given as hole
        pMyResultHoledShape->AddHole(rHole);

        pResultShape = pMyResultHoledShape.release();
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HGF2DRectangle::CLASS_ID  ?
                        DifferentiateRectangle(*((HGF2DRectangle*)(&pi_rShape)))  :
                        pi_rShape.DifferentiateFromShape(*this));
        }

    pResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return(pResultShape.release());
    }


//-----------------------------------------------------------------------------
// DifferentiateFromShape
// This method creates a new shape as the difference of self from given.
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::DifferentiateFromShape(const HGF2DShape& pi_rShape) const
    {
    // Declare recipient variable
    HAutoPtr<HGF2DShape> pResultShape;

    // Check if given completely inside rectangle
    if (GetExtent().Contains(pi_rShape.GetExtent()))
        {
        // The given completely inside rectangle
        // The result will therefore be void
        pResultShape = new HGF2DVoidShape();
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HGF2DRectangle::CLASS_ID  ?
                        ((HGF2DRectangle*)(&pi_rShape))->DifferentiateRectangle(*this)  :
                        pi_rShape.DifferentiateShape(*this));
        }

    pResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return(pResultShape.release());

    }


//-----------------------------------------------------------------------------
// IntersectShape
// This method create a new shape as the intersection between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DRectangle::IntersectShape(const HGF2DShape& pi_rShape) const
    {
    // Declare recipient variable
    HAutoPtr<HGF2DShape> pResultShape;

    // Check if given is completely inside rectangle
    if (GetExtent().Contains(pi_rShape.GetExtent()))
        {
        // The given is completely inside rectangle
        // The result will therefore be given shape
        pResultShape = static_cast<HGF2DShape*>(pi_rShape.Clone());
        }
    else
        {
        pResultShape = (pi_rShape.GetShapeType() == HGF2DRectangle::CLASS_ID  ?
                        IntersectRectangle(*((HGF2DRectangle*)(&pi_rShape)))  :
                        pi_rShape.IntersectShape(*this));
        }

    pResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return(pResultShape.release());
    }



//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE
// This method recalculates the tolerance of rectangle if automatic tolerance is
// active
//-----------------------------------------------------------------------------
void HGF2DRectangle::ResetTolerance()
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
void HGF2DRectangle::Rasterize(HGFScanLines& pio_rScanlines) const
    {
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


//-----------------------------------------------------------------------------
// @bsimethod                                                   2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DRectangle::AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const
    {
    HFCPtr<HGF2DShape>    pResultShape;

    // Check if rectangle is empty
    if (IsEmpty())
        {
        pResultShape = new HGF2DVoidShape();
        }
    else if (pi_rModel.IsIdentity())
        {
        pResultShape = new HGF2DRectangle(*this);
        }
    else
        {
        // Check if this model preserves direction
        if (pi_rModel.IsStretchable() || pi_rModel.PreservesDirection())
            {
            // The model concerves non-oriented rectangle shape ... we can transform the points directly
            double newXMin;
            double newYMin;
            double newXMax;
            double newYMax;
            pi_rModel.ConvertDirect(m_XMin, m_YMin, &newXMin, &newYMin);
            pi_rModel.ConvertDirect(m_XMax, m_YMax, &newXMax, &newYMax);

            HFCPtr<HGF2DRectangle> pResultRectangle = new HGF2DRectangle(HGF2DPosition(newXMin, newYMin), HGF2DPosition(newXMax, newYMax));

            // Set tolerance if necessary
            pResultRectangle->SetAutoToleranceActive(IsAutoToleranceActive());
            pResultRectangle->ResetTolerance();

            pResultShape = pResultRectangle;

            }
        else
            {
            // The model either does not preserve the rectangle shape or is unknown
            // We transform the rectangle into a polygon, and pass and tell the polygon to
            // process itself
            HGF2DPolygonOfSegments    ThePolygon(*this);
            ThePolygon.SetAutoToleranceActive(IsAutoToleranceActive());
            pResultShape = ThePolygon.AllocTransformDirect(pi_rModel);
            }
        }

    // &&AR ... Not sure the stroke tolerance shouldn't be transformed.
    pResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return pResultShape;
    }

