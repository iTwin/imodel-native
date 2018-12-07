//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE3DTriangle.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "hstdcpp.h"
#include "HVE3DTriangle.h"



//-----------------------------------------------------------------------------
// LOCAL FUNCTION DECLARATION
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments ComputeTriangleShape(const HGF3DPoint& pi_rFirstPoint,
                                            const HGF3DPoint& pi_rSecondPoint,
                                            const HGF3DPoint& pi_rThirdPoint);


//-----------------------------------------------------------------------------
// LOCAL FUNCTION DEFINITION
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 ComputeTriangleShape
 LOCAL FUNCTION
 This function calculates the triangle shape based on the provided three points.

 @param pi_rFirstPoint A HGF3DPoint specifying the first point of the triangle
                       definition.

 @param pi_rSecondPoint A HGF3DPoint specifying the second point of the triangle
                        definition.

 @param pi_rThirdPoint A HGF3DPoint specifying the third point of the triangle
                       definition.

  @param pi_rpCoordSys A reference to the interpretation coordinate system


  @return The shape
-----------------------------------------------------------------------------*/
HVE2DPolygonOfSegments ComputeTriangleShape(const HGF3DPoint& pi_rFirstPoint,
                                            const HGF3DPoint& pi_rSecondPoint,
                                            const HGF3DPoint& pi_rThirdPoint,
                                            const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Create linear
    HVE2DPolySegment MyTriangleContour(pi_rpCoordSys);

    // Add the four points (three plus closing point)
    MyTriangleContour.AppendPosition(HGF2DPosition(pi_rFirstPoint.GetX(), pi_rFirstPoint.GetY()));
    MyTriangleContour.AppendPosition(HGF2DPosition(pi_rSecondPoint.GetX(), pi_rSecondPoint.GetY()));
    MyTriangleContour.AppendPosition(HGF2DPosition(pi_rThirdPoint.GetX(), pi_rThirdPoint.GetY()));
    MyTriangleContour.AppendPosition(HGF2DPosition(pi_rFirstPoint.GetX(), pi_rFirstPoint.GetY()));

    // Create the shape
    HVE2DPolygonOfSegments MyTriangleShape(MyTriangleContour);

    return(MyTriangleShape);
    }



#if (0)

/**----------------------------------------------------------------------------
 Default constructor.

 The default plane is perpendicular to Z axis.

-----------------------------------------------------------------------------*/
HVE3DTriangle::HVE3DTriangle()
    : KGI2DTriangleFacet<HVE3DPlane>()
    {
    // Create the 3D points
    HGF3DPoint FirstPoint(0.0, 0.0, 0.0);
    HGF3DPoint SecondPoint(10.0, 0.0, 0.0);
    HGF3DPoint ThirdPoint(0.0, 10.0, 0.0);

    // Create the plane
    HVE3DPlane MyPlane(FirstPoint, SecondPoint, ThirdPoint);

    // Create the shape
    HVE2DPolygonOfSegments MyTriangleShape;

    MyTriangleShape.AppendPoint(HGF2DPosition(0.0, 0.0));
    MyTriangleShape.AppendPoint(HGF2DPosition(10.0, 0.0));
    MyTriangleShape.AppendPoint(HGF2DPosition(0.0, 10.0));
    MyTriangleShape.AppendPoint(HGF2DPosition(0.0, 0.0));

    // Set the shape
    SetTriangle(MyTriangleShape);

    // Set attribute
    SetAttribute(MyPlane);

    }

#endif


/**----------------------------------------------------------------------------
 Constructor.


 This constructor create a triangle from a plane facet

-----------------------------------------------------------------------------*/
HVE3DTriangle::HVE3DTriangle(const HVE2DTriangleFacet<HVE3DPlane>& pi_rpFacet)
    : HVE2DTriangleFacet<HVE3DPlane>(pi_rpFacet)
    {

    }



/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Plane to duplicate.
-----------------------------------------------------------------------------*/
HVE3DTriangle::HVE3DTriangle(const HVE3DTriangle& pi_rObj)
    : HVE2DTriangleFacet<HVE3DPlane>(pi_rObj)
    {
    }


/**----------------------------------------------------------------------------
 Constructor for this class specifying three points. The three points provided
 must not be co-linear, nor define a plane parallel to the Z axis.

 @param pi_rFirstPoint A HGF3DPoint specifying the first point of the plane
                       definition.

 @param pi_rSecondPoint A HGF3DPoint specifying the second point of the plane
                        definition.

 @param pi_rThirdPoint A HGF3DPoint specifying the third point of the plane
                       definition.


-----------------------------------------------------------------------------*/
HVE3DTriangle::HVE3DTriangle(const HGF3DPoint& pi_rFirstPoint,
                             const HGF3DPoint& pi_rSecondPoint,
                             const HGF3DPoint& pi_rThirdPoint,
                             const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DTriangleFacet<HVE3DPlane>(ComputeTriangleShape(pi_rFirstPoint, pi_rSecondPoint, pi_rThirdPoint, pi_rpCoordSys),
                                     HVE3DPlane(pi_rFirstPoint, pi_rSecondPoint, pi_rThirdPoint))
    {
    // Make sure that the points are not colinear

    // Make sure the plane is not parallel to Z axis.
#if (0)
    // Create the plane
    HVE3DPlane MyPlane(pi_rFirstPoint, pi_rSecondPoint, pi_rThirdPoint);

    // Create the shape
    HVE2DPolygonOfSegments MyTriangleShape();

    MyTriangleShape.AppendPoint(HGF2DPosition(pi_rFirstPoint.GetX(), pi_rFirstPoint.GetY()));
    MyTriangleShape.AppendPoint(HGF2DPosition(pi_rSecondPoint.GetX(), pi_rSecondPoint.GetY()));
    MyTriangleShape.AppendPoint(HGF2DPosition(pi_rThirdPoint.GetX(), pi_rThirdPoint.GetY()));
    MyTriangleShape.AppendPoint(HGF2DPosition(pi_rFirstPoint.GetX(), pi_rFirstPoint.GetY()));

    // Set the shape
    SetTriangle(MyTriangleShape);

    // Set attribute
    SetAttribute(MyPlane);
#endif

    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HVE3DTriangle::~HVE3DTriangle()
    {
    }

/**----------------------------------------------------------------------------
  Assignment operator

  It duplicates a plane
  @param pi_rObj The plane to duplicate

  @return a reference to self to be used as a l-value.
-----------------------------------------------------------------------------*/
HVE3DTriangle& HVE3DTriangle::operator=(const HVE3DTriangle& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HVE2DTriangleFacet<HVE3DPlane>::operator=(pi_rObj);
        }

    return(*this);
    }


#if (0)


/**----------------------------------------------------------------------------
 This method indicates if point is in facet

 @param i_rPoint The point to check if it is in triangle

 @return true if point is in, false otherwise
-----------------------------------------------------------------------------*/
bool HVE3DTriangle::IsPointIn(const HGF2DLocation& i_rPoint) const
    {
#if (0)
    return(GetShape().IsPointIn(i_rPoint));
#else
    return(IsPointIn(i_rPoint.GetPosition()));
#endif
    }




/**----------------------------------------------------------------------------
 This method indicates if point is in facet

 @param i_rPoint The point to check if it is in triangle

 @return true if point is in, false otherwise
-----------------------------------------------------------------------------*/
bool HVE3DTriangle::IsPointIn(const HGF2DPosition& i_rPoint) const
    {
    bool IsIn = false;

    // There is a last hit ... check if point is in last hit ...
    // Obtain plane and extract plane definition points
    HGF3DPoint Point1(GetAttribute().GetFirstDefinitionPoint());
    HGF3DPoint Point2(GetAttribute().GetSecondDefinitionPoint());
    HGF3DPoint Point3(GetAttribute().GetThirdDefinitionPoint());


    double x  = i_rPoint.GetX();
    double y  = i_rPoint.GetY();
    double x1 = Point1.GetX();
    double y1 = Point1.GetY();

    double x2 = Point2.GetX();
    double y2 = Point2.GetY();

    double x3 = Point3.GetX();
    double y3 = Point3.GetY();


    double Det1 = (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1);
    double Det2 = (y - y2) * (x3 - x2) - (x - x2) * (y3 - y2);
    double Det3 = (y - y3) * (x1 - x3) - (x - x3) * (y1 - y3);


#if (0)
    if (HNumeric<double>::EQUAL_EPSILON(Det1, 0.0) ||
        HNumeric<double>::EQUAL_EPSILON(Det2, 0.0) ||
        HNumeric<double>::EQUAL_EPSILON(Det3, 0.0))
        {
        // Point is on boundary
        IsIn = true;
        }
    else
#endif
        if (Det1 > 0.0 && Det2 > 0.0 && Det3 > 0.0)
            {
            IsIn = true;
            }
        else if (Det1 < 0.0 && Det2 < 0.0 && Det3 < 0.0)
            {
            IsIn = true;
            }

    // The point must be inside if it is declared in
    HASSERT(!IsIn || (IsIn && (GetShape().IsPointIn(HGF2DLocation(i_rPoint, GetShape().GetCoordSys())) ||
                               GetShape().IsPointOn(HGF2DLocation(i_rPoint, GetShape().GetCoordSys())))));

#if (0)
    return(GetShape().IsPointIn(HGF2DLocation(i_rPoint, GetShape().GetCoordSys())));
#else
    return(IsIn);
#endif

    }

#endif


/**----------------------------------------------------------------------------
 This method allocates a transformed coy of the facet

 @param i_rTransformMatrix A 2D transformation matrix that contains the
                           transformation to apply to copy. The determinant of
                           the matrix may not be null.

 @return A newly allocated facet containing duplicates of all information and
         member. The new instance must be freed using delete when needed no more.
-----------------------------------------------------------------------------*/
HVE2DFacet<HVE3DPlane>* HVE3DTriangle::AllocateTransformed(const HFCMatrix<3, 3>& i_rTransformMatrix) const
    {
    HVE2DFacet<HVE3DPlane>* pResultFacet = 0;

    // Create 3D transfo matrix
    HFCMatrix<4, 4> Matrix3D;

    // Copy matrix into a 3D matrix
    Matrix3D[0][0] = i_rTransformMatrix[0][0];
    Matrix3D[0][1] = i_rTransformMatrix[0][1];
    Matrix3D[0][2] = 0.0;
    Matrix3D[0][3] = i_rTransformMatrix[0][2];

    Matrix3D[1][0] = i_rTransformMatrix[1][0];
    Matrix3D[1][1] = i_rTransformMatrix[1][1];
    Matrix3D[1][2] = 0.0;
    Matrix3D[1][3] = i_rTransformMatrix[1][2];

    Matrix3D[2][0] = 0.0;
    Matrix3D[2][1] = 0.0;
    Matrix3D[2][2] = 1.0;
    Matrix3D[2][3] = 0.0;

    Matrix3D[3][0] = i_rTransformMatrix[2][0];
    Matrix3D[3][1] = i_rTransformMatrix[2][1];
    Matrix3D[3][2] = 0.0;
    Matrix3D[3][3] = i_rTransformMatrix[2][2];

    // Extract plane definition points
    HGF3DPoint Point1;
    HGF3DPoint Point2;
    HGF3DPoint Point3;
    Point1 = GetAttribute().GetFirstDefinitionPoint();
    Point2 = GetAttribute().GetSecondDefinitionPoint();
    Point3 = GetAttribute().GetThirdDefinitionPoint();

    // Transform plane definition points
    Point1.Transform(Matrix3D);
    Point2.Transform(Matrix3D);
    Point3.Transform(Matrix3D);

    // Create linear
    HVE2DPolySegment MyTriangleContour(GetShape().GetCoordSys());

    // Add the four points (three plus closing point)
    MyTriangleContour.AppendPosition(HGF2DPosition(Point1.GetX(), Point1.GetY()));
    MyTriangleContour.AppendPosition(HGF2DPosition(Point2.GetX(), Point2.GetY()));
    MyTriangleContour.AppendPosition(HGF2DPosition(Point3.GetX(), Point3.GetY()));
    MyTriangleContour.AppendPosition(HGF2DPosition(Point1.GetX(), Point1.GetY()));

    if (!MyTriangleContour.IsAutoContiguous())
        {

        if (!HDOBLE_EQUAL(fabs(MyTriangleContour.CalculateRayArea(MyTriangleContour.GetStartPoint())),
                      0.0,
                      MIN(HMAX_EPSILON,
                          MyTriangleContour.GetTolerance() *
                          MyTriangleContour.GetTolerance())))
            {



            // Create a new triangle
            pResultFacet = new HVE3DTriangle(Point1, Point2, Point3, GetShape().GetCoordSys());
            }
        }

    return(pResultFacet);
    }


