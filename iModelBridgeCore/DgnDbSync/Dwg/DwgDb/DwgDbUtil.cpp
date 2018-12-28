/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/DwgDbUtil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
::HMODULE       Util::GetOrLoadToolkitDll (DwgStringCR dllPrefix)
    {
    // append the version suffix to dll name:
    DwgString   dllName = dllPrefix + DWGDB_ToolkitDllSuffix + L".dll";
    // try finding or loading the dll
    ::HMODULE   dllHandle = ::GetModuleHandle (dllName);
    if (nullptr == dllHandle)
        dllHandle = ::LoadLibrary (dllName);
    BeAssert (nullptr != dllHandle && "A toolkit DLL cannot be loaded (is the suffix up to date?)");
    return  dllHandle;
    }

DPoint3d    Util::DPoint3dFrom (DWGGE_TypeCR(Point3d) gePoint) { return DPoint3d::From(gePoint.x, gePoint.y, gePoint.z); }
DPoint3d    Util::DPoint3dFrom (DWGGE_TypeCR(Point2d) gePoint) { return DPoint3d::From(gePoint.x, gePoint.y, 0.0); }
DPoint3d    Util::DPoint3dFrom (DWGGE_TypeCR(Vector3d) geVector) { return DPoint3d::From(geVector.x, geVector.y, geVector.z); }
DPoint3dCP  Util::DPoint3dCPFrom (DWGGE_TypeCP(Point3d) gePoints) { return reinterpret_cast<DPoint3dCP>(gePoints); }
DPoint2d    Util::DPoint2dFrom (DWGGE_TypeCR(Point2d) gePoint) { return DPoint2d::From(gePoint.x, gePoint.y); }
DPoint2d    Util::DPoint2dFrom (DWGGE_TypeCR(Vector2d) geVector) { return DPoint2d::From(geVector.x, geVector.y); }
DPoint2dCP  Util::DPoint2dCPFrom (DWGGE_TypeCP(Point2d) gePoints) { return reinterpret_cast<DPoint2dCP>(gePoints); }
DVec3d      Util::DVec3dFrom (DWGGE_TypeCR(Vector3d) geVector) { return DVec3d::From(geVector.x, geVector.y, geVector.z); }
DVec3d      Util::DVec3dFrom (DWGGE_TypeCR(Vector2d) geVector) { return DVec3d::From(geVector.x, geVector.y, 0.0); }
DVec3d      Util::DVec3dFrom (DWGGE_TypeCR(Scale3d) geScale) { return DVec3d::From(geScale.sx, geScale.sy, geScale.sz); }
DVec3dCP    Util::DVec3dCPFrom (DWGGE_TypeCP(Vector3d) geVectors) { return reinterpret_cast<DVec3dCP>(geVectors); }
DVec2d      Util::DVec2dFrom (DWGGE_TypeCR(Vector2d) geVector) { return DVec2d::From(geVector.x, geVector.y); }
DRange3d    Util::DRange3dFrom (DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents) const& extents) { return DRange3d::From (Util::DPoint3dFrom(extents.minPoint()), Util::DPoint3dFrom(extents.maxPoint())); }
DRange2d    Util::DRange2dFrom (DWGGE_TypeCR(Point2d) extents) { return DRange2d::From (Util::DPoint2dFrom(extents)); }

DWGGE_Type(Point3d)     Util::GePoint3dFrom (DPoint3dCR p) { return DWGGE_Type(Point3d)(p.x, p.y, p.z); }
DWGGE_Type(Point2d)     Util::GePoint2dFrom (DPoint2dCR p) { return DWGGE_Type(Point2d)(p.x, p.y); }
DWGGE_Type(Vector3d)    Util::GeVector3dFrom (DVec3dCR v) { return DWGGE_Type(Vector3d)(v.x, v.y, v.z); }
DWGGE_Type(Vector2d)    Util::GeVector2dFrom (DVec2dCR v) { return DWGGE_Type(Vector2d)(v.x, v.y); }
DWGGE_Type(Scale3d)     Util::GeScale3dFrom (DVec3dCR v) { return DWGGE_Type(Scale3d)(v.x, v.y, v.z); }

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
size_t      Util::GetVectorArray (DVector3dArrayR vectorsOut, DWGGE_TypeCR(Vector3dArray) vectorsIn)
    {
    int     nVectors = vectorsIn.length ();
    for (int i = 0; i < nVectors; i++)
        vectorsOut.push_back (DVec3d::From(vectorsIn[i].x, vectorsIn[i].y));
    return  nVectors;
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
size_t      Util::GetObjectIdArray (DWGDB_TypeR(ObjectIdArray) idsOut, DwgDbObjectIdArrayCR idsIn)
    {
#ifdef DWGTOOLKIT_OpenDwg
    idsOut.clear ();
#elif DWGTOOLKIT_RealDwg
    idsOut.removeAll ();
#endif
    for (auto id : idsIn)
        idsOut.append (id);
    return  idsOut.length ();
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
    curve.Zero ();
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

    for (int i = 0; i < curve.params.numKnots; i++)
        curve.knots[i] = geSpline.knotAt (i);

    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     Util::GetMSBsplineCurve (MSBsplineCurveR curve, DWGGE_TypeCR(SplineEnt2d) geSpline)
    {
    curve.Zero ();
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

    for (int i = 0; i < curve.params.numKnots; i++)
        curve.knots[i] = geSpline.knotAt (i);

    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     Util::GetCurveVector (CurveVectorR curveVector, const TkArray<DWGGE_TypeP(Curve2d)>& curves)
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


#undef  BUILDING_REALDWG2017
#if defined(DWGTOOLKIT_RealDwg)
    #if VendorVersion < 2018
        #define BUILDING_REALDWG2017
    #endif
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<TkDbVersion, DwgFileVersion>   MapDwgVersionFromToolkit ()
    {
    bmap<TkDbVersion, DwgFileVersion>  versionMap;

    versionMap[TkDbVersion::kDHL_AC1_2]    = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_AC1_40]   = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_AC1_50]   = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_AC2_20]   = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_AC2_10]   = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_AC2_21]   = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_AC2_22]   = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_1001]     = DwgFileVersion::Unknown;
    versionMap[TkDbVersion::kDHL_1002]     = DwgFileVersion::R2_5;
    versionMap[TkDbVersion::kDHL_1003]     = DwgFileVersion::R2_6;
    versionMap[TkDbVersion::kDHL_1004]     = DwgFileVersion::R9;
    versionMap[TkDbVersion::kDHL_1005]     = DwgFileVersion::R9;
    versionMap[TkDbVersion::kDHL_1006]     = DwgFileVersion::R10;
    versionMap[TkDbVersion::kDHL_1007]     = DwgFileVersion::R10;
    versionMap[TkDbVersion::kDHL_1008]     = DwgFileVersion::R10;
    versionMap[TkDbVersion::kDHL_1009]     = DwgFileVersion::R11;
    versionMap[TkDbVersion::kDHL_1010]     = DwgFileVersion::R11;
    versionMap[TkDbVersion::kDHL_1011]     = DwgFileVersion::R11;
    versionMap[TkDbVersion::kDHL_1012]     = DwgFileVersion::R13;
    versionMap[TkDbVersion::kDHL_1014]     = DwgFileVersion::R14;     // default for R14
    versionMap[TkDbVersion::kDHL_1013]     = DwgFileVersion::R14;
    versionMap[TkDbVersion::kDHL_1015]     = DwgFileVersion::R2000;   // default for R2000
    versionMap[TkDbVersion::kDHL_1500]     = DwgFileVersion::R2000;
    versionMap[TkDbVersion::kDHL_1800]     = DwgFileVersion::R2004;   // default R2004
    versionMap[TkDbVersion::kDHL_1800a]    = DwgFileVersion::R2004;
    versionMap[TkDbVersion::kDHL_1021]     = DwgFileVersion::R2007;   // default for R2007
    versionMap[TkDbVersion::kDHL_2100a]    = DwgFileVersion::R2007;
    versionMap[TkDbVersion::kDHL_1024]     = DwgFileVersion::R2010;
    versionMap[TkDbVersion::kDHL_1027]     = DwgFileVersion::R2013;
#ifndef BUILDING_REALDWG2017
    versionMap[TkDbVersion::kDHL_1032]     = DwgFileVersion::R2018;
#endif
    versionMap[TkDbVersion::kDHL_CURRENT]  = DwgFileVersion::Current;

    return  versionMap;
    }
static bmap<TkDbVersion, DwgFileVersion> const s_dwgVersionFromToolkit = MapDwgVersionFromToolkit();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<DwgFileVersion, TkDbVersion>   MapDwgVersionToToolkit ()
    {
    bmap<DwgFileVersion, TkDbVersion>  versionMap;

    versionMap[DwgFileVersion::R2_5]     = TkDbVersion::kDHL_1002;
    versionMap[DwgFileVersion::R2_6]     = TkDbVersion::kDHL_1003;
    versionMap[DwgFileVersion::R9]       = TkDbVersion::kDHL_1005;
    versionMap[DwgFileVersion::R10]      = TkDbVersion::kDHL_1008;
    versionMap[DwgFileVersion::R11]      = TkDbVersion::kDHL_1011;
    versionMap[DwgFileVersion::R13]      = TkDbVersion::kDHL_1012;
    versionMap[DwgFileVersion::R14]      = TkDbVersion::kDHL_1013;
    versionMap[DwgFileVersion::R2000]    = TkDbVersion::kDHL_1500;
    versionMap[DwgFileVersion::R2004]    = TkDbVersion::kDHL_1800a;
    versionMap[DwgFileVersion::R2007]    = TkDbVersion::kDHL_2100a;
    versionMap[DwgFileVersion::R2010]    = TkDbVersion::kDHL_1024;
    versionMap[DwgFileVersion::R2013]    = TkDbVersion::kDHL_1027;
#ifndef BUILDING_REALDWG2017
    versionMap[DwgFileVersion::R2018]    = TkDbVersion::kDHL_1032;
#endif
    versionMap[DwgFileVersion::Current]  = TkDbVersion::kDHL_CURRENT;

    return  versionMap;
    }
static bmap<DwgFileVersion, TkDbVersion> const s_dwgVersionToToolkit = MapDwgVersionToToolkit();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgFileVersion  Util::GetDwgVersionFrom (TkDbVersion dwgVersion)
    {
    auto found = s_dwgVersionFromToolkit.find (dwgVersion);
    if (found != s_dwgVersionFromToolkit.end())
        return  found->second;

    return  DwgFileVersion::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TkDbVersion    Util::GetDwgVersionFrom (DwgFileVersion dwgVersion)
    {
    auto found = s_dwgVersionToToolkit.find (dwgVersion);
    if (found != s_dwgVersionToToolkit.end())
        return  found->second;

    return  TkDbVersion::kDHL_Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus Util::GetObjectArray (DwgDbObjectPArrayR out, TkObjectArray& in)
    {
#ifdef DWGTOOLKIT_OpenDwg
    for (OdUInt32 i = 0; i < in.length(); i++)
        {
        if (nullptr != DwgDbObject::Cast(in[i].get()))
            {
            DwgDbObjectP object = DwgDbObject::Cast (in[i].detach());
            if (nullptr != object)
                out.push_back (object);
            }
        }
    
#elif DWGTOOLKIT_RealDwg
    for (int i = 0; i < in.length(); i++)
        {
        AcDbObject* acObject = static_cast<AcDbObject*>(in[i]);
        if (nullptr != acObject)
            {
            DwgDbObjectP object = DwgDbObject::Cast (acObject);
            if (nullptr != object)
                out.push_back (object);
            else
                delete acObject;
            }
        }
#endif
    return out.empty() ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus Util::GetEntityArray (DwgDbEntityPArrayR out, TkEntityArray& in)
    {
#ifdef DWGTOOLKIT_OpenDwg
    for (OdUInt32 i = 0; i < in.length(); i++)
        {
        if (nullptr != DwgDbEntity::Cast(in[i].get()))
            {
            DwgDbEntityP entity = DwgDbEntity::Cast (in[i].detach());
            if (nullptr != entity)
                out.push_back (entity);
            }
        }
    
#elif DWGTOOLKIT_RealDwg
    for (int i = 0; i < in.length(); i++)
        {
        AcDbEntity* acObject = static_cast<AcDbEntity*>(in[i]);
        if (nullptr != acObject)
            {
            DwgDbEntityP entity = DwgDbEntity::Cast (acObject);
            if (nullptr != entity)
                out.push_back (entity);
            else
                delete acObject;
            }
        }
#endif
    return out.empty() ? DwgDbStatus::UnknownError : DwgDbStatus::Success;
    }

