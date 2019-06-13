/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Dwg/DwgDb/DwgDbCommon.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB

// do not publish this in PublicAPI
#ifdef DWGTOOLKIT_OpenDwg
#define TkArray         OdArray
#define TkDbVersion     OdDb::DwgVersion
#define TkObjectArray   OdRxObjectPtrArray
#define TkEntityArray   OdDbEntityPtrArray
#elif DWGTOOLKIT_RealDwg
#define TkArray         AcArray
#define TkDbVersion     AcDb::AcDbDwgVersion
#define TkObjectArray   AcDbVoidPtrArray
#define TkEntityArray   AcArray<AcDbEntity*>
#endif

struct Util : NonCopyableClass
    {
public:
    static DPoint3d     DPoint3dFrom (DWGGE_TypeCR(Point3d) gePoint);
    static DPoint3d     DPoint3dFrom (DWGGE_TypeCR(Point2d) gePoint);
    static DPoint3d     DPoint3dFrom (DWGGE_TypeCR(Vector3d) geVector);
    static DPoint3dCP   DPoint3dCPFrom (DWGGE_TypeCP(Point3d) gePoints);
    static DPoint2d     DPoint2dFrom (DWGGE_TypeCR(Point2d) gePoint);
    static DPoint2d     DPoint2dFrom (DWGGE_TypeCR(Vector2d) geVector);
    static DPoint2dCP   DPoint2dCPFrom (DWGGE_TypeCP(Point2d) gePoints);
    static DVec3d       DVec3dFrom (DWGGE_TypeCR(Vector3d) gePoint);
    static DVec3d       DVec3dFrom (DWGGE_TypeCR(Vector2d) gePoint);
    static DVec3d       DVec3dFrom (DWGGE_TypeCR(Scale3d) geScale);
    static DVec3dCP     DVec3dCPFrom (DWGGE_TypeCP(Vector3d) geVectors);
    static DVec2d       DVec2dFrom (DWGGE_TypeCR(Vector2d) gePoint);
    static DRange3d     DRange3dFrom (DWGDB_SDKNAME(OdGeExtents3d,AcDbExtents) const& extents);
    static DRange2d     DRange2dFrom (DWGGE_TypeCR(Point2d) extents);
    static DEllipse3d   DEllipse3dFrom (DWGGE_TypeCR(EllipArc3d) geEllipse);
    static DEllipse3d   DEllipse3dFrom (DWGGE_TypeCR(EllipArc2d) geEllipse);
    static DEllipse3d   DEllipse3dFrom (DWGGE_TypeCR(CircArc2d) geArc);
    static void         GetDPoint3d (DPoint3dP points, DWGGE_TypeCP(Point3d) gePoint, uint32_t nPoints);
    static size_t       GetPointArray (DPoint3dArrayR pointsOut, DWGGE_TypeCR(Point3dArray) pointsIn);
    static size_t       GetPointArray (DPoint2dArrayR pointsOut, DWGGE_TypeCR(Point2dArray) pointsIn);
    static size_t       GetVectorArray (DVector3dArrayR vectorsOut, DWGGE_TypeCR(Vector3dArray) vectorsIn);
    static void         GetTransform (TransformR trans, DWGGE_TypeCR(Matrix3d) matrix);
    static void         GetRotMatrix (RotMatrixR rotMatrix, DWGGE_TypeCR(Matrix3d) geMatrix);
    static DwgDbStatus  GetMSBsplineCurve (MSBsplineCurveR curve, DWGGE_TypeCR(SplineEnt3d) geSpline);
    static DwgDbStatus  GetMSBsplineCurve (MSBsplineCurveR curve, DWGGE_TypeCR(SplineEnt2d) geSpline);
    static DwgDbStatus  GetCurveVector (CurveVectorR curveVector, const TkArray<DWGGE_TypeP(Curve2d)>& curves);
    static DwgDbStatus  GetCurvePrimitive (ICurvePrimitivePtr& primitive, DWGGE_TypeCP(Curve2d) geCurve);

    static DWGGE_Type(Point3d)  GePoint3dFrom (DPoint3dCR p);
    static DWGGE_Type(Point2d)  GePoint2dFrom (DPoint2dCR p);
    static DWGGE_Type(Vector3d) GeVector3dFrom (DVec3dCR v);
    static DWGGE_Type(Vector2d) GeVector2dFrom (DVec2dCR v);
    static DWGGE_Type(Scale3d)  GeScale3dFrom (DVec3dCR v);
    static void         GetGeMatrix (DWGGE_TypeR(Matrix3d) matrix, TransformCR trans);
    static size_t       GetGePointArray (DWGGE_TypeR(Point3dArray) pointsOut, DPoint3dArrayCR pointsIn);
    static size_t       GetGePointArray (DWGGE_TypeR(Point2dArray) pointsOut, DPoint2dArrayCR pointsIn);
    static size_t       GetObjectIdArray (DWGDB_TypeR(ObjectIdArray) idsOut, DwgDbObjectIdArrayCR idsIn);
    static DwgDbStatus  GetObjectArray (DwgDbObjectPArrayR out, TkObjectArray& in);
    static DwgDbStatus  GetEntityArray (DwgDbEntityPArrayR out, TkEntityArray& in);

    static DwgFileVersion   GetDwgVersionFrom (TkDbVersion dwgVersion);
    static TkDbVersion      GetDwgVersionFrom (DwgFileVersion dwgVersion);
    static ::HMODULE        GetOrLoadToolkitDll (DwgStringCR dllPrefix);
    };  // Util
