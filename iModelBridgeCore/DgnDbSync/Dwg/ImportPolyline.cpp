/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportPolyline.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    m_boundaryType = CurveVector::BOUNDARY_TYPE_None;
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

    polyline.GetEcs (m_ecs);
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
    m_boundaryType = CurveVector::BOUNDARY_TYPE_None;

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
    else if (m_numPoints < 3)
        m_isClosed = false;

    // if boundary type is not set, default to outer for closed polyline or open otherwise:
    if (m_boundaryType == CurveVector::BOUNDARY_TYPE_None)
        m_boundaryType = m_isClosed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open;

    size_t          numSegments = m_isClosed ? m_numPoints : m_numPoints - 1;

    CurveVectorPtr  curves = CurveVector::Create (m_boundaryType);
    if (!curves.IsValid())
        return  curves;

    bool hasEcs = !m_ecs.IsIdentity ();

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
            if (hasEcs)
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
                if (hasEcs)
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

