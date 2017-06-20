/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/DwgDbUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

DPoint3d    Util::DPoint3dFrom (DWGGE_TypeCR(Point3d) gePoint) { return DPoint3d::From(gePoint.x, gePoint.y, gePoint.z); }
DPoint3d    Util::DPoint3dFrom (DWGGE_TypeCR(Point2d) gePoint) { return DPoint3d::From(gePoint.x, gePoint.y, 0.0); }
DPoint3d    Util::DPoint3dFrom (DWGGE_TypeCR(Vector3d) geVector) { return DPoint3d::From(geVector.x, geVector.y, geVector.z); }
DPoint3dCP  Util::DPoint3dCPFrom (DWGGE_TypeCP(Point3d) gePoints) { return reinterpret_cast<DPoint3dCP>(gePoints); }
DPoint2d    Util::DPoint2dFrom (DWGGE_TypeCR(Point2d) gePoint) { return DPoint2d::From(gePoint.x, gePoint.y); }
DPoint2d    Util::DPoint2dFrom (DWGGE_TypeCR(Vector2d) geVector) { return DPoint2d::From(geVector.x, geVector.y); }
DPoint2dCP  Util::DPoint2dCPFrom (DWGGE_TypeCP(Point2d) gePoints) { return reinterpret_cast<DPoint2dCP>(gePoints); }
DVec3d      Util::DVec3dFrom (DWGGE_TypeCR(Vector3d) geVector) { return DVec3d::From(geVector.x, geVector.y, geVector.z); }
DVec3d      Util::DVec3dFrom (DWGGE_TypeCR(Vector2d) geVector) { return DVec3d::From(geVector.x, geVector.y, 0.0); }
DVec3dCP    Util::DVec3dCPFrom (DWGGE_TypeCP(Vector3d) geVectors) { return reinterpret_cast<DVec3dCP>(geVectors); }
DVec2d      Util::DVec2dFrom (DWGGE_TypeCR(Vector2d) geVector) { return DVec2d::From(geVector.x, geVector.y); }
DRange3d    Util::DRange3dFrom (DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents) const& extents) { return DRange3d::From (Util::DPoint3dFrom(extents.minPoint()), Util::DPoint3dFrom(extents.maxPoint())); }
DRange2d    Util::DRange2dFrom (DWGGE_TypeCR(Point2d) extents) { return DRange2d::From (Util::DPoint2dFrom(extents)); }

DWGGE_Type(Point3d)     Util::GePoint3dFrom (DPoint3dCR p) { return DWGGE_Type(Point3d)(p.x, p.y, p.z); }
DWGGE_Type(Point2d)     Util::GePoint2dFrom (DPoint2dCR p) { return DWGGE_Type(Point2d)(p.x, p.y); }
DWGGE_Type(Vector3d)    Util::GeVector3dFrom (DVec3dCR v) { return DWGGE_Type(Vector3d)(v.x, v.y, v.z); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        Util::GetDPoint3d (DPoint3dP points, DWGGE_TypeCP(Point3d) gePoint, uint32_t nPoints)
    {
    for (uint32_t i = 0; i < nPoints; i++)
        points[i].Init (gePoint[i].x, gePoint[i].y, gePoint[i].z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      Util::GetPointArray (DPoint3dArrayR pointsOut, DWGGE_TypeCR(Point3dArray) pointsIn)
    {
    int     nPoints = pointsIn.length ();
    for (int i = 0; i < nPoints; i++)
        pointsOut.push_back (DPoint3d::From(pointsIn[i].x, pointsIn[i].y, pointsIn[i].z));
    return  nPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      Util::GetPointArray (DPoint2dArrayR pointsOut, DWGGE_TypeCR(Point2dArray) pointsIn)
    {
    int     nPoints = pointsIn.length ();
    for (int i = 0; i < nPoints; i++)
        pointsOut.push_back (DPoint2d::From(pointsIn[i].x, pointsIn[i].y));
    return  nPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      Util::GetGePointArray (DWGGE_TypeR(Point3dArray) pointsOut, DPoint3dArrayCR pointsIn)
    {
    for (DPoint3dCR point : pointsIn)
        pointsOut.append (DWGGE_Type(Point3d)(point.x, point.y, point.z));
    return  pointsIn.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      Util::GetGePointArray (DWGGE_TypeR(Point2dArray) pointsOut, DPoint2dArrayCR pointsIn)
    {
    for (DPoint2dCR point : pointsIn)
        pointsOut.append (DWGGE_Type(Point2d)(point.x, point.y));
    return  pointsIn.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        Util::GetTransform (TransformR trans, DWGGE_TypeCR(Matrix3d) matrix)
    {
    trans.InitFromRowValues
        (
#ifdef DWGTOOLKIT_OpenDwg
        matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
        matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
        matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]
#elif DWGTOOLKIT_RealDwg
        matrix.entry[0][0], matrix.entry[0][1], matrix.entry[0][2], matrix.entry[0][3],
        matrix.entry[1][0], matrix.entry[1][1], matrix.entry[1][2], matrix.entry[1][3],
        matrix.entry[2][0], matrix.entry[2][1], matrix.entry[2][2], matrix.entry[2][3]
#endif
        );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        Util::GetRotMatrix (RotMatrixR rotMatrix, DWGGE_TypeCR(Matrix3d) matrix)
    {
    rotMatrix.InitFromRowValues
        (
#ifdef DWGTOOLKIT_OpenDwg
        matrix[0][0], matrix[0][1], matrix[0][2],
        matrix[1][0], matrix[1][1], matrix[1][2],
        matrix[2][0], matrix[2][1], matrix[2][2]
#elif DWGTOOLKIT_RealDwg
        matrix.entry[0][0], matrix.entry[0][1], matrix.entry[0][2],
        matrix.entry[1][0], matrix.entry[1][1], matrix.entry[1][2],
        matrix.entry[2][0], matrix.entry[2][1], matrix.entry[2][2]
#endif
        );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        Util::GetGeMatrix (DWGGE_TypeR(Matrix3d) matrix, TransformCR trans)
    {
    for (int i = 0; i < 3; i++)
        {
        for (int j = 0; j < 4; j++)
#ifdef DWGTOOLKIT_OpenDwg
            matrix[i][j] = trans.form3d[i][j];
#elif DWGTOOLKIT_RealDwg
            matrix.entry[i][j] = trans.form3d[i][j];
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DEllipse3d  Util::DEllipse3dFrom (DWGGE_TypeCR(EllipArc3d) geEllipse)
    {
    DPoint3d    center = Util::DPoint3dFrom (geEllipse.center());
    DVec3d      majorAxis = Util::DVec3dFrom (geEllipse.majorAxis());
    DVec3d      minorAxis = Util::DVec3dFrom (geEllipse.minorAxis());

    majorAxis.Normalize ();
    majorAxis.Scale (geEllipse.majorRadius());

    minorAxis.Normalize ();
    minorAxis.Scale (geEllipse.minorRadius());

    double      startAngle = geEllipse.startAng ();
    double      sweptAngle = geEllipse.endAng() - startAngle;
    if (!Angle::IsFullCircle(sweptAngle))
        sweptAngle = Angle::AdjustToSweep (sweptAngle, 0, msGeomConst_2pi);
    
    return  DEllipse3d::FromVectors (center, majorAxis, minorAxis, startAngle, sweptAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DEllipse3d  Util::DEllipse3dFrom (DWGGE_TypeCR(EllipArc2d) geEllipse)
    {
    DPoint3d    center = Util::DPoint3dFrom (geEllipse.center());
    DVec3d      majorAxis = Util::DVec3dFrom (geEllipse.majorAxis());
    DVec3d      minorAxis = Util::DVec3dFrom (geEllipse.minorAxis());

    majorAxis.Normalize ();
    majorAxis.Scale (geEllipse.majorRadius());

    minorAxis.Normalize ();
    minorAxis.Scale (geEllipse.minorRadius());

    double      startAngle = geEllipse.startAng ();
    double      sweptAngle = geEllipse.endAng() - startAngle;
    if (!Angle::IsFullCircle(sweptAngle))
        sweptAngle = Angle::AdjustToSweep (sweptAngle, 0, msGeomConst_2pi);
    
    return  DEllipse3d::FromVectors (center, majorAxis, minorAxis, startAngle, sweptAngle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DEllipse3d  Util::DEllipse3dFrom (DWGGE_TypeCR(CircArc2d) geArc)
    {
    DPoint3d    center = Util::DPoint3dFrom (geArc.center());
    DPoint3d    startPoint = Util::DPoint3dFrom (geArc.startPoint());
    DPoint3d    endPoint = Util::DPoint3dFrom (geArc.endPoint());
    
    return DEllipse3d::FromArcCenterStartEnd (center, startPoint, endPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     Util::GetMSBsplineCurve (MSBsplineCurveR curve, DWGGE_TypeCR(SplineEnt3d) geSpline)
    {
    curve.params.order = geSpline.degree() + 1;
    curve.rational = geSpline.isRational ();
    curve.params.closed = geSpline.isClosed ();
    curve.params.numPoles = geSpline.numControlPoints ();
    curve.params.numKnots = geSpline.numKnots ();
    curve.display.polygonDisplay = false;
    curve.display.curveDisplay = true;
    if (BSISUCCESS != curve.Allocate())
        return  DwgDbStatus::MemoryError;

    DWGGE_TypeCP(NurbCurve3d)   geNurbCurve = nullptr;
    if (curve.rational)
        geNurbCurve = static_cast<DWGGE_TypeCP(NurbCurve3d)> (&geSpline);
    if (nullptr == geNurbCurve || geNurbCurve->numWeights() < curve.params.numPoles)
        curve.rational = false;

    for (int i = 0; i < curve.params.numPoles; i++)
        {
        curve.poles[i] = Util::DPoint3dFrom (geSpline.controlPointAt(i));
        if (curve.rational)
            curve.weights[i] = geNurbCurve->weightAt (i);
        }

    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     Util::GetMSBsplineCurve (MSBsplineCurveR curve, DWGGE_TypeCR(SplineEnt2d) geSpline)
    {
    curve.params.order = geSpline.degree() + 1;
    curve.rational = geSpline.isRational ();
    curve.params.closed = geSpline.isClosed ();
    curve.params.numPoles = geSpline.numControlPoints ();
    curve.params.numKnots = geSpline.numKnots ();
    curve.display.polygonDisplay = false;
    curve.display.curveDisplay = true;
    if (BSISUCCESS != curve.Allocate())
        return  DwgDbStatus::MemoryError;

    DWGGE_TypeCP(NurbCurve2d)   geNurbCurve = nullptr;
    if (curve.rational)
        geNurbCurve = static_cast<DWGGE_TypeCP(NurbCurve2d)> (&geSpline);
    if (nullptr == geNurbCurve || geNurbCurve->numWeights() < curve.params.numPoles)
        curve.rational = false;

    for (int i = 0; i < curve.params.numPoles; i++)
        {
        curve.poles[i] = Util::DPoint3dFrom (geSpline.controlPointAt(i));
        if (curve.rational)
            curve.weights[i] = geNurbCurve->weightAt (i);
        }

    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     Util::GetCurveVector (CurveVectorR curveVector, const DwgDbArray<DWGGE_TypeP(Curve2d)>& curves)
    {
    if (curves.isEmpty())
        return  DwgDbStatus::InvalidData;

    int     numCurves = curves.length ();
    for (int i = 0; i < numCurves; i++)
        {
        DWGGE_TypeCP(Curve2d)   curve = curves.at (i);
        if (nullptr != curve)
            {
            ICurvePrimitivePtr  primitive;
            if (DwgDbStatus::Success == Util::GetCurvePrimitive(primitive, curve))
                curveVector.Add (primitive);
            }
        }

    return  curveVector.empty() ? DwgDbStatus::InvalidData : DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     Util::GetCurvePrimitive (ICurvePrimitivePtr& primitive, DWGGE_TypeCP(Curve2d) geCurve)
    {
    if (nullptr == geCurve)
        return  DwgDbStatus::InvalidInput;

    DWGGE_Type(Line2d)              line;
    DWGGE_TypeCP(CircArc2d)         arc;
    DWGGE_TypeCP(CompositeCurve2d)  compCurve;
    DWGGE_TypeCP(EllipArc2d)        ellip;
    DWGGE_TypeCP(ExternalCurve2d)   extCurve;
    DWGGE_TypeCP(OffsetCurve2d)     offset;
    DWGGE_TypeCP(SplineEnt2d)       spline;

    if (geCurve->isLinear(line))
        {
        DWGGE_Type(Point2d)     start, end;
        if (line.hasStartPoint(start) && line.hasEndPoint(end))
            primitive = ICurvePrimitive::CreateLine (DSegment3d::From(start.x, start.y, 0.0, end.x, end.y, 0.0));
        }
    else if (nullptr != (arc = static_cast<DWGGE_TypeCP(CircArc2d)>(geCurve)))
        {
        primitive = ICurvePrimitive::CreateArc (Util::DEllipse3dFrom(*arc));
        }
    else if (nullptr != (ellip = static_cast<DWGGE_TypeCP(EllipArc2d)>(geCurve)))
        {
        primitive = ICurvePrimitive::CreateArc (Util::DEllipse3dFrom(*ellip));
        }
    else if (nullptr != (compCurve = static_cast<DWGGE_TypeCP(CompositeCurve2d)>(geCurve)))
        {
        DWGDB_SDKNAME(OdGeCurve2dPtrArray,AcGeVoidPointerArray) nestedCurves;

        compCurve->getCurveList (nestedCurves);

        for (size_t i = 0; i < nestedCurves.length(); i++)
            {
            DWGGE_TypeP(Curve2d)    nestedCurve = static_cast<DWGGE_TypeP(Curve2d)> (nestedCurves.at(static_cast<unsigned int>(i)));
            if (nullptr != nestedCurve)
                Util::GetCurvePrimitive (primitive, nestedCurve);
            }
        }
    else if (nullptr != (extCurve = static_cast<DWGGE_TypeCP(ExternalCurve2d)>(geCurve)))
        {
        DWGGE_Type(NurbCurve2d)     nurb;
        if (extCurve->isDefined() && extCurve->isNurbCurve(nurb))
            {
            MSBsplineCurve          curve;
            if (DwgDbStatus::Success == Util::GetMSBsplineCurve(curve, nurb))
                primitive = ICurvePrimitive::CreateBsplineCurve (curve);
            }
        }
    else if (nullptr != (offset = static_cast<DWGGE_TypeCP(OffsetCurve2d)>(geCurve)))
        {
        // WIP - offset curve
        }
    else if (nullptr != (spline = static_cast<DWGGE_TypeCP(SplineEnt2d)>(geCurve)))
        {
        MSBsplineCurve  curve;
        if (DwgDbStatus::Success == Util::GetMSBsplineCurve(curve, *spline))
            primitive = ICurvePrimitive::CreateBsplineCurve (curve);
        }
    else
        {
        BeAssert (false && L"Unexpected DWG Curve2d type!");
        return  DwgDbStatus::InvalidData;
        }

    return  primitive.IsValid() ? DwgDbStatus::Success : DwgDbStatus::InvalidData;
    }
