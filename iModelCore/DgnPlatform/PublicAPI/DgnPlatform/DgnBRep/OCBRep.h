/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnBRep/OCBRep.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

// Ick.... The MSVC does not require rand_r and is implicitly multithread (according to MSDN).
#undef _REENTRANT
#include <NCollection_UBTreeFiller.hxx>
#define _REENTRANT

#include <BinTools.hxx>
#include <BinTools_ShapeSet.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <Standard_TypeDef.hxx>
#include <gp_Ax2.hxx>
#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_GTrsf.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Plane.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepAlgoAPI_BuilderAlgo.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimApi_MakePrism.hxx>
#include <BRepPrimApi_MakeRevol.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepFill.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <GeomLProp_SLProps.hxx>

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct OCBRep
{
// Initialize Open Cascade type from Dgn type...
static gp_Pnt ToGpPnt(DPoint3dCR point) {return gp_Pnt(point.x, point.y, point.z);}
static gp_Pnt2d ToGpPnt2d(DPoint2dCR point) {return gp_Pnt2d(point.x, point.y);}
static gp_Pnt2d ToGpPnt2d(DPoint3dCR point) {return gp_Pnt2d(point.x, point.y);}
static gp_XYZ ToGpXYZ(DPoint3dCR point) {return gp_XYZ(point.x, point.y, point.z);}
static gp_Dir ToGpDir(DVec3dCR vec) {return gp_Dir(vec.x, vec.y, vec.z);}
static gp_Vec ToGpVec(DVec3dCR vec) {return gp_Vec(vec.x, vec.y, vec.z);}
static gp_Ax1 ToGpAx1(DPoint3dCR point, DVec3dCR zVec) {return gp_Ax1(ToGpPnt(point), ToGpVec(zVec));}
static gp_Ax2 ToGpAx2(DPoint3dCR point, RotMatrixCR rMatrix) {DVec3d xVec, zVec; rMatrix.GetColumn(xVec, 0); rMatrix.GetColumn(zVec, 2); return gp_Ax2(ToGpPnt(point), ToGpDir(zVec), ToGpDir(xVec));}
static gp_Lin ToGpLin(DRay3dCR ray) {return gp_Lin(ToGpPnt(ray.origin), ToGpDir(ray.direction));}
static gp_Mat ToGpMat(RotMatrixCR rMatrix) {gp_Mat mat(rMatrix.form3d[0][0], rMatrix.form3d[0][1], rMatrix.form3d[0][2], rMatrix.form3d[1][0], rMatrix.form3d[1][1], rMatrix.form3d[1][2], rMatrix.form3d[2][0], rMatrix.form3d[2][1], rMatrix.form3d[2][2]); return mat;}
static gp_Trsf ToGpTrsf(TransformCR transform) {gp_Trsf trsf; trsf.SetValues(transform.form3d[0][0], transform.form3d[0][1], transform.form3d[0][2], transform.form3d[0][3], transform.form3d[1][0], transform.form3d[1][1], transform.form3d[1][2], transform.form3d[1][3], transform.form3d[2][0], transform.form3d[2][1], transform.form3d[2][2], transform.form3d[2][3]); return trsf;}
static gp_GTrsf ToGpGTrsf(TransformCR transform) {DPoint3d origin; RotMatrix rMatrix; transform.GetTranslation(origin); transform.GetMatrix(rMatrix); gp_GTrsf trsf(ToGpMat(rMatrix), ToGpXYZ(origin)); return trsf;}
DGNPLATFORM_EXPORT static gp_Circ ToGpCirc(double& start, double& end, DEllipse3dCR ellipse);
DGNPLATFORM_EXPORT static gp_Elips ToGpElips(double& start, double& end, DEllipse3dCR ellipse);
DGNPLATFORM_EXPORT static Handle(Geom_BSplineCurve) ToGeomBSplineCurve(MSBsplineCurveCR bCurve, TransformCP transform = nullptr);
DGNPLATFORM_EXPORT static Handle(Geom2d_BSplineCurve) ToGeom2dBSplineCurve(MSBsplineCurveCR bCurve, TransformCP transform = nullptr);
DGNPLATFORM_EXPORT static void GetOcctKnots(TColStd_Array1OfReal*& occtKnots, TColStd_Array1OfInteger*& occtMultiplicities, bvector<double> const& knots, int order);

// Initialize Dgn type from Open Cascade type...
static DPoint3d ToDPoint3d(gp_Pnt const& gpPoint) {return DPoint3d::From(gpPoint.X(), gpPoint.Y(), gpPoint.Z());}
static DPoint2d ToDPoint2d(gp_Pnt2d const& gpPoint) {return DPoint2d::From(gpPoint.X(), gpPoint.Y());}
static DPoint3d ToDPoint3d(gp_Pnt2d const& gpPoint) {return DPoint3d::From(gpPoint.X(), gpPoint.Y(), 0.0);}
static DPoint3d ToDPoint3d(gp_XYZ const& gpPoint) {return DPoint3d::From(gpPoint.X(), gpPoint.Y(), gpPoint.Z());}
static DVec3d ToDVec3d(gp_Dir const& dir) {return DVec3d::From(dir.X(), dir.Y(), dir.Z());}
static DVec3d ToDVec3d(gp_Dir2d const& dir) {return DVec3d::From(dir.X(), dir.Y(), 0.0);}
static Transform ToTransform(gp_Trsf const& trsf) {return Transform::From(ToRotMatrix(trsf.VectorialPart()), ToDPoint3d(trsf.TranslationPart()));}
static RotMatrix ToRotMatrix(gp_Mat const& mat) {RotMatrix rMatrix; for (int i=1; i <= 3; i++) for (int j=1; j <= 3; j++) rMatrix.form3d[i-1][j-1] = mat.Value(i, j); return rMatrix;}
static DRange3d ToDRange3d(Bnd_Box const& box) {DRange3d range; box.Get(range.low.x, range.low.y, range.low.z, range.high.x, range.high.y, range.high.z); return range;}
static DEllipse3d ToDEllipse3d(gp_Circ const& gpCirc, double start, double end) {return DEllipse3d::FromScaledVectors(ToDPoint3d(gpCirc.Location()), ToDVec3d(gpCirc.XAxis().Direction()), ToDVec3d(gpCirc.YAxis().Direction()), gpCirc.Radius(), gpCirc.Radius(), start, end - start);}
static DEllipse3d ToDEllipse3d(gp_Circ2d const& gpCirc, double start, double end) {return DEllipse3d::FromScaledVectors(ToDPoint3d(gpCirc.Location()), ToDVec3d(gpCirc.XAxis().Direction()), ToDVec3d(gpCirc.YAxis().Direction()), gpCirc.Radius(), gpCirc.Radius(), start, end - start);}
static DEllipse3d ToDEllipse3d(gp_Elips const& gpElips, double start, double end) {return DEllipse3d::FromScaledVectors(ToDPoint3d(gpElips.Location()), ToDVec3d(gpElips.XAxis().Direction()), ToDVec3d(gpElips.YAxis().Direction()), gpElips.MajorRadius(), gpElips.MinorRadius(), start, end - start);}
static DEllipse3d ToDEllipse3d(gp_Elips2d const& gpElips, double start, double end) {return DEllipse3d::FromScaledVectors(ToDPoint3d(gpElips.Location()), ToDVec3d(gpElips.XAxis().Direction()), ToDVec3d(gpElips.YAxis().Direction()), gpElips.MajorRadius(), gpElips.MinorRadius(), start, end - start);}

DGNPLATFORM_EXPORT static bool HasCurvedFaceOrEdge(TopoDS_Shape const&);
DGNPLATFORM_EXPORT static PolyfaceHeaderPtr IncrementalMesh(TopoDS_Shape const&, IFacetOptionsR, bool cleanShape=true); //! BRepTools::Clean is called after meshing when cleanShape is true
DGNPLATFORM_EXPORT static BentleyStatus ClipTopoShape(bvector<TopoDS_Shape>& output, bool& clipped, TopoDS_Shape const& shape, ClipVectorCR clip) {return ERROR;} // NEEDSWORK...
DGNPLATFORM_EXPORT static BentleyStatus ClipCurveVector(bvector<CurveVectorPtr>& output, CurveVectorCR input, ClipVectorCR clip, TransformCP localToWorld) {return ERROR;} // NEEDSWORK...

//! Support for the creation of new bodies from other types of geometry.
struct Create
    {
    //! Create a new wire or planar sheet from a CurveVector that represents an open path, closed path, region with holes, or union region.
    //! @param[out] shape The new shape.
    //! @param[in] curve The curve vector to create a shape from.
    //! @note The CurvePrimitives that define an open path or closed loop are expected to be connected head-to-tail and may not intersect except at a vertex. A vertex can be shared by at most 2 edges.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromCurveVector(TopoDS_Shape& shape, CurveVectorCR curve);

    //! Create a new sheet or solid from a Polyface.
    //! @param[out] out The new shape.
    //! @param[in] mesh The polyface to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromPolyface(TopoDS_Shape& shape, PolyfaceQueryCR mesh);

    //! Create a new sheet or solid from a DgnBoxDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnBoxDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromBox(TopoDS_Shape& shape, DgnBoxDetailCR detail);

    //! Create a new sheet or solid from a DgnConeDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnConeDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromCone(TopoDS_Shape& shape, DgnConeDetailCR detail);

    //! Create a new sheet or solid from a DgnSphereDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnSphereDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromSphere(TopoDS_Shape& shape, DgnSphereDetailCR detail);

    //! Create a new sheet or solid from a DgnTorusPipeDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnTorusPipeDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromTorus(TopoDS_Shape& shape, DgnTorusPipeDetailCR detail);

    //! Create a new sheet or solid from a DgnExtrusionDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnExtrusionDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromExtrusion(TopoDS_Shape& shape, DgnExtrusionDetailCR detail);

    //! Create a new sheet or solid from a DgnRotationalSweepDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnRotationalSweepDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromRotationalSweep(TopoDS_Shape& shape, DgnRotationalSweepDetailCR detail);

    //! Create a new sheet or solid from a DgnRuledSweepDetail.
    //! @param[out] out The new shape.
    //! @param[in] detail The DgnRuledSweepDetail to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromRuledSweep(TopoDS_Shape& shape, DgnRuledSweepDetailCR detail);

    //! Create a new sheet or solid from a ISolidPrimitive.
    //! @param[out] out The new shape.
    //! @param[in] primitive The ISolidPrimitive to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromSolidPrimitive(TopoDS_Shape& shape, ISolidPrimitiveCR primitive);

    //! Create a new sheet from a MSBsplineSurface.
    //! @param[out] out The new shape.
    //! @param[in] bSurface The MSBsplineSurface to create a shape from.
    //! @return SUCCESS if shape was created.
    DGNPLATFORM_EXPORT static BentleyStatus TopoShapeFromBSurface(TopoDS_Shape& shape, MSBsplineSurfaceCR bSurface);
    };

}; // OCBRep

END_BENTLEY_DGN_NAMESPACE
