/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

#define MinBulgeFactor      1.0E-8
#define MaxBulgeFactor      572.96  // tangent of 89.9 degrees as a 1/4 of maximum arc angle(i.e. nearly full circle)

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineFactory::PolylineFactory ()
    {
    m_polyline = nullptr;
    m_numPoints = 0;
    m_points.clear ();
    m_widths.clear ();
    m_bulges.clear ();
    m_constantWidth = 0.0;
    m_hasBulges = false;
    m_hasConstantWidth = false;
    m_hasWidths = false;
    m_isClosed = false;
    m_ecs.InitIdentity ();
    m_hasEcs = false;
    m_boundaryType = CurveVector::BOUNDARY_TYPE_None;
    m_hasAppliedWidths = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineFactory::PolylineFactory (DwgDbPolylineCR polyline, bool allowClosed)
    {
    m_polyline = &polyline;
    m_isClosed = allowClosed && polyline.IsClosed ();

    m_numPoints = polyline.GetNumPoints ();
    m_constantWidth = 0.0;
    m_hasConstantWidth = polyline.GetConstantWidth (m_constantWidth);
    m_hasWidths = polyline.HasWidth ();
    m_hasBulges = polyline.HasBulges ();

    for (size_t i = 0; i < m_numPoints; i++)
        {
        m_points.push_back (polyline.GetPointAt(i));

        if (m_hasWidths)
            m_widths.push_back (polyline.GetWidthsAt(i));

        if (m_hasBulges)
            m_bulges.push_back (polyline.GetBulgeAt(i));
        }

    m_elevation = polyline.GetElevation ();
    m_thickness = polyline.GetThickness ();
    m_boundaryType = CurveVector::BOUNDARY_TYPE_None;
    m_hasAppliedWidths = false;

    polyline.GetEcs (m_ecs);
    m_hasEcs = !m_ecs.IsIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineFactory::PolylineFactory (size_t nPoints, DPoint3dCP points, bool closed)
    {
    m_polyline = nullptr;
    m_isClosed = closed;
    m_numPoints = nPoints;
    m_constantWidth = 0.0;
    m_hasConstantWidth = 0;
    m_hasWidths = false;
    m_hasBulges = false;
    m_elevation = 0.0;
    m_thickness = 0.0;
    m_ecs.InitIdentity ();
    m_hasEcs = false;
    m_boundaryType = CurveVector::BOUNDARY_TYPE_None;
    m_hasAppliedWidths = false;

    for (size_t i = 0; i < m_numPoints; i++)
        m_points.push_back (points[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void  PolylineFactory::SetBoundaryType (CurveVector::BoundaryType type)
    {
    m_boundaryType = type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  PolylineFactory::CreateCurveVector (bool useElevation)
    {
    m_numPoints = m_points.size ();
    if (m_numPoints < 2)
        return  CurveVectorPtr();

    // try to convert variable widths into a shape
    auto shape = this->CreateShapeFromVariableWidths(useElevation);
    if (shape.IsValid())
        return  shape;

    // if boundary type is not set, default to outer for closed polyline or open otherwise:
    if (m_boundaryType == CurveVector::BOUNDARY_TYPE_None)
        m_boundaryType = m_isClosed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open;

    size_t          numSegments = m_isClosed ? m_numPoints : m_numPoints - 1;

    CurveVectorPtr  curves = CurveVector::Create (m_boundaryType);
    if (!curves.IsValid())
        return  curves;

    ICurvePrimitivePtr  primitive;
    DPoint3dArray       noBulgePoints;

    for (size_t i = 0; i < numSegments; i++)
        {
        size_t      next = (i + 1) % m_numPoints;
        DPoint3d    startPoint = DPoint3d::From (m_points[i].x, m_points[i].y, useElevation ? m_elevation : m_points[i].z);
        DPoint3d    endPoint = DPoint3d::From (m_points[next].x, m_points[next].y, useElevation ? m_elevation : m_points[next].z);

        // WIP - width & thickness
        if (m_hasBulges && PolylineFactory::IsValidBulgeFactor(m_bulges[i]) && !startPoint.IsEqual(endPoint))
            {
            // create a linestring from previous points and flush away the point buffer:
            if (!noBulgePoints.empty())
                {
                primitive = ICurvePrimitive::CreateLineString (noBulgePoints);
                if (primitive.IsValid())
                    curves->Add (primitive);

                noBulgePoints.clear ();
                }

            // transform the points to ECS:
            if (m_hasEcs)
                {
                m_ecs.MultiplyTranspose (startPoint);
                m_ecs.MultiplyTranspose (endPoint);
                }

            // create an arc from the bulge factor, on polyline ECS:
            DEllipse3d  arc;
            DwgHelper::CreateArc2d (arc, startPoint, endPoint, m_bulges[i]);

            // translate the arc to the elevation:
            arc.center.z = m_elevation;

            primitive = ICurvePrimitive::CreateArc (arc);
            if (primitive.IsValid())
                {
                // transform the arc to WCS:
                if (m_hasEcs)
                    primitive->TransformInPlace (m_ecs);
                curves->Add (primitive);
                }

            continue;
            }

        // append the linear segment:
        if (noBulgePoints.size() == 0 || !noBulgePoints.back().IsEqual(startPoint))
            noBulgePoints.push_back (startPoint);
        if (!startPoint.IsEqual(endPoint))
            noBulgePoints.push_back (endPoint);
        }

    if (noBulgePoints.size() > 0)
        {
        primitive = ICurvePrimitive::CreateLineString (noBulgePoints);
        if (primitive.IsValid())
            curves->Add (primitive);
        }

    return  curves;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  PolylineFactory::CreateShapeFromVariableWidths(bool useElevation)
    {
    // leave constant width to ApplyConstantWidthTo made by the caller
    if (!m_hasWidths || m_hasConstantWidth || m_hasBulges || m_isClosed)
        return  nullptr;

    // collect line segment arrays in CW & CCW sequences on left & right sides of the path respectively
    bvector<DSegment3d> leftSegs, rightSegs;
    for (size_t i = 0; i < m_numPoints - 1; i++)
        {
        DSegment3d right, left;
        this->WidenLineToSides(right, left, i, useElevation);
        rightSegs.push_back(right);
        leftSegs.push_back(left);
        }

    // build a path in CCW from the widened and disconnected line segment arrays
    bvector<DPoint3d>   shapePoints;
    this->ConnectAndChainToPath(shapePoints, rightSegs, leftSegs);
    if (shapePoints.empty())
        return  nullptr;

    CurveVectorPtr  shape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
    if (shape.IsValid())
        {
        if (m_hasEcs)
            m_ecs.Multiply(shapePoints);

        auto linestring = ICurvePrimitive::CreateLineString(shapePoints);
        if (linestring.IsValid())
            {
            shape->Add(linestring);
            m_hasAppliedWidths = true;
            }
        }
    return  shape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineFactory::ConnectAndChainToPath(bvector<DPoint3d>& shapePoints, bvector<DSegment3d>const& rightSegs, bvector<DSegment3d>const& leftSegs) const
    {
    DPoint3d    point0, point1;
    size_t  numSegments = rightSegs.size();

    shapePoints.clear ();
    if (numSegments == 1)
        {
        // a simple shape - no intersection computation is needed
        rightSegs.front().GetStartPoint(point0);
        shapePoints.push_back(point0);
        rightSegs.front().GetEndPoint(point1);
        shapePoints.push_back(point1);

        DPoint3d    nextPoint;
        leftSegs.front().GetStartPoint(nextPoint);
        if (!nextPoint.IsEqual(point1))
            shapePoints.push_back(nextPoint);
        leftSegs.front().GetEndPoint(nextPoint);
        if (!nextPoint.IsEqual(point0))
            shapePoints.push_back(nextPoint);
        return;
        }

    // chain right segment array into a CCW path
    DPoint3d    previousPoint, intersectionPoint;
    for (size_t i = 0; i < numSegments - 1; i++)
        {
        size_t  next = i + 1;
        this->IntersectLines(intersectionPoint, rightSegs[i], rightSegs[next]);
        if (i == 0)
            rightSegs[0].GetStartPoint(previousPoint);
        shapePoints.push_back(previousPoint);
        shapePoints.push_back(intersectionPoint);
        previousPoint = intersectionPoint;
        }
    // add last right segment
    rightSegs.back().GetEndPoint(previousPoint);
    shapePoints.push_back(previousPoint);

    // chain left segment array in reversed order to continue the path in CCW
    for (size_t i = numSegments - 1; i > 0; i--)
        {
        size_t  next = i - 1;
        this->IntersectLines(intersectionPoint, leftSegs[i], leftSegs[next]);
        if (i == numSegments - 1)
            leftSegs[i].GetStartPoint(previousPoint);
        shapePoints.push_back(previousPoint);
        shapePoints.push_back(intersectionPoint);
        previousPoint = intersectionPoint;
        }
    // add the last point in the CCW path (i.e. the first segment in left array)
    leftSegs.front().GetEndPoint(previousPoint);
    shapePoints.push_back(previousPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineFactory::WidenLineToSides(DSegment3dR rightSeg, DSegment3dR leftSeg, size_t fromIndex, bool useElevation) const
    {
    size_t  toIndex = (fromIndex + 1) % m_numPoints;

    // start & end points of the center path
    DPoint3d    startPoint = DPoint3d::From(m_points[fromIndex].x, m_points[fromIndex].y, useElevation ? m_elevation : m_points[fromIndex].z);
    DPoint3d    endPoint = DPoint3d::From(m_points[toIndex].x, m_points[toIndex].y, useElevation ? m_elevation : m_points[toIndex].z);

    if (m_hasEcs)
        {
        m_ecs.MultiplyTranspose (startPoint);
        m_ecs.MultiplyTranspose (endPoint);
        }

    // path and width vectors of the center line
    auto fromStart2End = DVec3d::FromStartEnd(startPoint, endPoint);
    auto perpendicular = DVec3d::FromCCWPerpendicularXY(fromStart2End);

    // start width
    DPoint3d    right, left;
    WidenPointToSides(right, left, startPoint, perpendicular, m_widths[fromIndex].x);
    rightSeg.SetStartPoint(right);
    leftSeg.SetEndPoint(left);

    // end width
    WidenPointToSides(right, left, endPoint, perpendicular, m_widths[fromIndex].y);
    rightSeg.SetEndPoint(right);
    leftSeg.SetStartPoint(left);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineFactory::WidenPointToSides (DPoint3dR rightPoint, DPoint3dR leftPoint, DPoint3dCR origin, DVec3dCR right2Left, double width) const
    {
    double  halfWidth = 0.5 * width;
    if (halfWidth < 1.0e-6)
        {
        rightPoint = origin;
        leftPoint = origin;
        return;
        }

    DVec3d  widthVector = right2Left.ValidatedNormalize();
    widthVector.ScaleToLength(right2Left, halfWidth);
    rightPoint.SumOf(origin, widthVector);

    widthVector.Negate();
    leftPoint.SumOf(origin, widthVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineFactory::IntersectLines (DPoint3dR intersectionPoint, DSegment3dCR line1, DSegment3dCR line2) const
    {
    double  param1 = 0, param2 = 0;
    DPoint3d    point1, point2;
    if (DSegment3d::IntersectXY(param1, param2, point1, point2, line1, line2))
        intersectionPoint = point1;
    else
        line1.GetEndPoint(intersectionPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PolylineFactory::IsValidBulgeFactor (double bulge)
    {
    return  (fabs(bulge) > MinBulgeFactor) && (fabs(bulge) < MaxBulgeFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr   PolylineFactory::ApplyThicknessTo (CurveVectorPtr const& plineCurve)
    {
    GeometricPrimitivePtr   extruded;

    if (m_numPoints < 2 || !ISVALID_Thickness(m_thickness) || m_polyline == nullptr || !plineCurve.IsValid())
        return  extruded;

    DVec3d      normal = m_polyline->GetNormal ();

    DgnExtrusionDetail  profile(plineCurve, normal, m_isClosed);
    ISolidPrimitivePtr  solid = ISolidPrimitive::CreateDgnExtrusion (profile);

    if (solid.IsValid())
        extruded = GeometricPrimitive::Create (solid.get());

    return  extruded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  PolylineFactory::ApplyConstantWidthTo (CurveVectorPtr const& plineCurve)
    {
    if (m_constantWidth < 1.0e-5 || !m_hasConstantWidth || !m_ecs.IsIdentity())
        return  nullptr;

    double  halfWidth = 0.5 * m_constantWidth;

    auto shape = plineCurve->ThickenXYPathToArea (plineCurve, halfWidth, halfWidth);
    if (shape.IsValid())
        {
        m_hasAppliedWidths = true;
        m_isClosed = true;
        }

    return  shape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            PolylineFactory::HashAndAppendTo (BentleyApi::MD5& hashOut) const
    {
    // create a hash from raw data of this polyline and append it to output:
    if (m_numPoints < 1)
        return;

    hashOut.Add (&m_points.front(), m_points.size() * sizeof(DPoint3d));

    if (m_hasWidths)
        hashOut.Add (&m_widths.front(), m_widths.size() * sizeof(DPoint2d));
    if (m_hasBulges)
        hashOut.Add (&m_bulges.front(), m_bulges.size() * sizeof(double));
    if (m_hasConstantWidth)
        hashOut.Add (&m_constantWidth, sizeof m_constantWidth);

    hashOut.Add (&m_hasWidths, sizeof m_hasWidths);
    hashOut.Add (&m_hasBulges, sizeof m_hasBulges);
    hashOut.Add (&m_hasConstantWidth, sizeof m_hasConstantWidth);
    hashOut.Add (&m_elevation, sizeof m_elevation);
    hashOut.Add (&m_thickness, sizeof m_thickness);
    hashOut.Add (&m_isClosed, sizeof m_isClosed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void            PolylineFactory::TransformData (TransformCR transform)
    {
    if (transform.IsIdentity())
        return;

    for (auto& point : m_points)
        transform.Multiply (point);

    double  scale = 1.0;
    if (transform.IsRigidScale(scale) && fabs(scale - 1.0) > 0.01)
        {
        if (m_hasBulges)
            {
            for (auto& bulge : m_bulges)
                bulge *= scale;
            }

        if (m_hasWidths)
            {
            for (auto& width : m_widths)
                width.Scale (scale);
            }

        m_constantWidth *= scale;
        m_elevation *= scale;
        m_thickness *= scale;
        }
    }

