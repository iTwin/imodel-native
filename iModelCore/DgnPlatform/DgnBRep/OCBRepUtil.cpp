/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/OCBRepUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/OCBRep.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TopAbs_ShapeEnum OCBRepUtil::GetShapeType(TopoDS_Shape const& shape)
    {
    TopAbs_ShapeEnum shapeType = shape.ShapeType();

    if (TopAbs_COMPOUND != shapeType)
        return shapeType;

    // NOTE: Ignore empty child compound shapes from Parasolid import (Mantis Issue #27733)...
    for (TopoDS_Iterator shapeIter(shape); shapeIter.More(); shapeIter.Next())
        {
        TopAbs_ShapeEnum childShapeType = GetShapeType(shapeIter.Value());

        if (TopAbs_COMPOUND == shapeType)
            shapeType = childShapeType;
        else if (shapeType != childShapeType)
            return TopAbs_COMPOUND;
        }

    return shapeType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void OCBRepUtil::GetOcctKnots(TColStd_Array1OfReal*& occtKnots, TColStd_Array1OfInteger*& occtMultiplicities, bvector<double> const& knots, int order)
    {
    size_t          lowIndex, highIndex;
    bvector<double> compressedKnots;
    bvector<size_t> multiplicities;

    MSBsplineCurve::CompressKnots(knots, order, compressedKnots, multiplicities, lowIndex, highIndex);

    occtKnots = new TColStd_Array1OfReal(1, (int) compressedKnots.size());
    occtMultiplicities = new TColStd_Array1OfInteger (1, (int) compressedKnots.size());

    for (int i=0; i < (int) compressedKnots.size(); i++)
        {
        occtKnots->ChangeValue(i+1) = compressedKnots[i];
        occtMultiplicities->ChangeValue(i+1) = (int) multiplicities[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Handle(Geom_BSplineCurve) OCBRep::ToGeomBSplineCurve(MSBsplineCurveCR bCurveIn, TransformCP transform)
    {
    MSBsplineCurvePtr bCurve = (nullptr == transform) ? bCurveIn.CreateCopy() : bCurveIn.CreateCopyTransformed(*transform);

    if (bCurve->IsClosed())
        bCurve->MakeOpen(0.0); // Needs work -- Can we somehow send periodic knot vector intact?? -- for now ignore periodic knot nonsense.

    int                      numPoles = (int) bCurve->GetNumPoles();
    TColgp_Array1OfPnt       occtPoles(1, numPoles);
    TColStd_Array1OfReal*    occtKnots = nullptr;
    TColStd_Array1OfInteger* occtMultiplicities = nullptr;
    bvector <double>         knots;

    bCurve->GetKnots(knots);
    OCBRepUtil::GetOcctKnots(occtKnots, occtMultiplicities, knots, bCurve->GetIntOrder());

    for (int i=0; i<numPoles; i++)
        occtPoles.ChangeValue(i+1) = ToGpPnt(bCurve->GetUnWeightedPole(i));
        
    Handle(Geom_BSplineCurve) geomCurve;

    if (bCurve->HasWeights())                          
        {
        TColStd_Array1OfReal occtWeights(1, numPoles);

        for (int i=0; i<numPoles; i++)
            occtWeights.ChangeValue(i+1) = bCurve->GetWeight(i);

        geomCurve = new Geom_BSplineCurve(occtPoles, occtWeights, *occtKnots, *occtMultiplicities, bCurve->GetIntOrder()-1, false, false);
        }
    else
        {
        geomCurve = new Geom_BSplineCurve(occtPoles, *occtKnots, *occtMultiplicities, bCurve->GetIntOrder()-1, false);
        }

    DELETE_AND_CLEAR(occtKnots)
    DELETE_AND_CLEAR(occtMultiplicities);

    return geomCurve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Handle(Geom2d_BSplineCurve) OCBRep::ToGeom2dBSplineCurve(MSBsplineCurveCR bCurveIn, TransformCP transform)
    {
    MSBsplineCurvePtr bCurve = (nullptr == transform) ? bCurveIn.CreateCopy() : bCurveIn.CreateCopyTransformed(*transform);

    if (bCurve->IsClosed())
        bCurve->MakeOpen(0.0);      // Needs work -- Can we somehow send periodic knot vector intact??  -- for now ignore periodic knot nonsense.

    int                      numPoles = (int) bCurve->GetNumPoles();
    TColgp_Array1OfPnt2d     occtPoles(1, numPoles);
    TColStd_Array1OfReal*    occtKnots = nullptr;
    TColStd_Array1OfInteger* occtMultiplicities = nullptr;
    bvector <double>         knots;

    bCurve->GetKnots(knots);
    OCBRepUtil::GetOcctKnots(occtKnots, occtMultiplicities, knots, bCurve->GetIntOrder());

    for (int i=0; i<numPoles; i++)
        occtPoles.ChangeValue(i+1) = ToGpPnt2d(bCurve->GetUnWeightedPole(i));
        
    Handle(Geom2d_BSplineCurve) geomCurve;

    if (bCurve->HasWeights())                          
        {
        TColStd_Array1OfReal occtWeights(1, numPoles);

        for (int i=0; i<numPoles; i++)
            occtWeights.ChangeValue(i+1) = bCurve->GetWeight(i);

        geomCurve = new Geom2d_BSplineCurve(occtPoles, occtWeights, *occtKnots, *occtMultiplicities, bCurve->GetIntOrder()-1, false);
        }
    else
        {
        geomCurve = new Geom2d_BSplineCurve(occtPoles, *occtKnots, *occtMultiplicities, bCurve->GetIntOrder()-1, false);
        }

    DELETE_AND_CLEAR (occtKnots)
    DELETE_AND_CLEAR (occtMultiplicities);

    return geomCurve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
gp_Circ OCBRep::ToGpCirc(double& start, double& end, DEllipse3dCR ellipse)
    {
    double      r0, r1, startAngle, sweepAngle;
    DPoint3d    center;
    DVec3d      xVec, zVec;
    RotMatrix   rMatrix;

    ellipse.GetScaledRotMatrix(center, rMatrix, r0, r1, startAngle, sweepAngle);
    BeAssert(DoubleOps::WithinTolerance(r0, r1, Precision::Confusion()));

    rMatrix.GetColumn(xVec, 0);
    rMatrix.GetColumn(zVec, 2);

    if (sweepAngle != msGeomConst_2pi)
        {
        start = startAngle;
        end = startAngle + sweepAngle;
        }
    else
        {
        start = 0.0;
        end = sweepAngle;
        }

    if (sweepAngle < 0.0)
        {
        start = -start;
        end = -end;
        zVec.Negate();
        }

    if (start > end)
        end += msGeomConst_2pi;
    
    return gp_Circ(gp_Ax2(ToGpPnt(center), ToGpDir(zVec), ToGpDir(xVec)), r0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
gp_Elips OCBRep::ToGpElips(double& start, double& end, DEllipse3dCR ellipse)
    {
    double      r0, r1, startAngle, sweepAngle;
    DPoint3d    center;
    DVec3d      xVec, yVec, zVec;
    RotMatrix   rMatrix;

    ellipse.GetScaledRotMatrix(center, rMatrix, r0, r1, startAngle, sweepAngle);
    rMatrix.GetColumns(xVec, yVec, zVec);

    if (sweepAngle != msGeomConst_2pi)
        {
        start = startAngle;
        end = startAngle + sweepAngle;
        }
    else
        {
        start = 0.0;
        end = sweepAngle;
        }

    if (sweepAngle < 0.0)
        {
        start = -start;
        end = -end;
        zVec.Negate();
        }

    if (start > end)
        end += msGeomConst_2pi;

    if (r1 > r0)
        {
        start -= msGeomConst_piOver2;
        end -= msGeomConst_piOver2;
        xVec = yVec;

        double rTmp = r0;

        r0 = r1;
        r1 = rTmp;
        }
    
    return gp_Elips(gp_Ax2(ToGpPnt(center), ToGpDir(zVec), ToGpDir(xVec)), r0, r1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool OCBRep::HasCurvedFaceOrEdge(TopoDS_Shape const& shape)
    {
    for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next())
        {
        TopoDS_Face const& face = TopoDS::Face(ex.Current()); 
        TopLoc_Location location;
        Handle(Geom_Surface) const& surface = BRep_Tool::Surface(face, location);

        if (surface.IsNull())
            continue;

        if (!surface->IsKind(STANDARD_TYPE(Geom_Plane)))
            return true;
        }

    TopTools_IndexedMapOfShape edgeMap;

    TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);

    for (int iEdge=1; iEdge <= edgeMap.Extent(); iEdge++)
        {
        TopoDS_Edge const& edge = TopoDS::Edge(edgeMap(iEdge));
        TopLoc_Location location;
        Standard_Real first, last;
        Handle(Geom_Curve) const& curve = BRep_Tool::Curve(edge, location, first, last);

        if (curve.IsNull())
            continue;

        if (!curve->IsKind(STANDARD_TYPE(Geom_Line)))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void extractKnots(bvector<double>& knots, TColStd_Array1OfReal const& knotSequence, int nPoles, int order, bool periodic)
    {
    if (periodic)
        {
        knots.resize(nPoles + order);

        // Per Earlin....and this sort of matches the parasolid conversion code...
        memcpy (&knots.front(), &knotSequence.Value(1), knotSequence.Length() * sizeof (double));
        knots[0] = knots[1];
        knots[knots.size()-1] = knots[knots.size()-2];
        knots.push_back (knots.front());
        }
    else
        {
        for (int i=1; i<=knotSequence.Length(); i++)
            knots.push_back(knotSequence.Value(i));
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr toCurvePrimitiveFromNonPeriodic(Handle(Geom_BSplineCurve) const& geomBCurve)
    {
    if (0 != geomBCurve->IsPeriodic())
        {
        BeAssert(false && "Unexpected Periodic curve");
        return nullptr;
        }

    bvector<double>   knots, weights;
    bvector<DPoint3d> poles;
    MSBsplineCurvePtr bCurve = MSBsplineCurve::CreatePtr();

    for (int i=1, nPoles = geomBCurve->NbPoles(); i <= nPoles; i++)
        poles.push_back(OCBRep::ToDPoint3d(geomBCurve->Pole(i)));

    if (geomBCurve->IsRational())
        for (int i=1; i <= geomBCurve->Weights()->Length(); i++)
            weights.push_back(geomBCurve->Weights()->Value(i));

    extractKnots(knots, geomBCurve->KnotSequence(), geomBCurve->NbPoles(), geomBCurve->Degree()+1, false);

    if (SUCCESS != bCurve->Populate(&poles.front(), geomBCurve->IsRational() ? &weights.front() : nullptr, (int) poles.size(), &knots.front(), (int) knots.size(), geomBCurve->Degree()+1, false, false))
        {
        BeAssert(false && "Bspline populate failure");
        return nullptr;
        }
        
    return ICurvePrimitive::CreateBsplineCurve(*bCurve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr OCBRep::ToCurvePrimitive(Handle(Geom_BSplineCurve) const& geomBCurveIn, double first, double last)
    {
    if (0 != geomBCurveIn->IsPeriodic() || first != geomBCurveIn->FirstParameter() || last != geomBCurveIn->LastParameter())
        {
        static double s_parametricTolerance = 1.0e-08;
        Handle(Geom_BSplineCurve) geomBCurve = GeomConvert::SplitBSplineCurve(geomBCurveIn, first, last, s_parametricTolerance);

        return toCurvePrimitiveFromNonPeriodic(geomBCurve);
        }

    return toCurvePrimitiveFromNonPeriodic(geomBCurveIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr OCBRep::ToCurvePrimitive(Handle(Geom_Curve) const& curve, double first, double last)
    {
    if (curve.IsNull())
        return nullptr;

    Handle(Standard_Type) kindOfCurve = curve->DynamicType();

    if (STANDARD_TYPE(Geom_Line) == kindOfCurve)
        {
        DSegment3d segment = DSegment3d::From(OCBRep::ToDPoint3d(curve->Value(first)), OCBRep::ToDPoint3d(curve->Value(last)));

        return ICurvePrimitive::CreateLine(segment);
        }
    else if (STANDARD_TYPE(Geom_Ellipse) == kindOfCurve)
        {
        Handle(Geom_Ellipse) ellipse = Handle(Geom_Ellipse)::DownCast(curve);

        return ICurvePrimitive::CreateArc(OCBRep::ToDEllipse3d(ellipse->Elips(), first, last));
        }
    else if (STANDARD_TYPE(Geom_Circle) == kindOfCurve)
        {
        Handle(Geom_Circle) circle = Handle(Geom_Circle)::DownCast(curve);

        return ICurvePrimitive::CreateArc(OCBRep::ToDEllipse3d(circle->Circ(), first, last));
        }
    else if (STANDARD_TYPE(Geom_TrimmedCurve) == kindOfCurve)
        {
        Handle(Geom_TrimmedCurve) trimmedCurve = Handle(Geom_TrimmedCurve)::DownCast(curve);

        return OCBRep::ToCurvePrimitive(trimmedCurve->BasisCurve(), first, last);
        }
    else if (STANDARD_TYPE(Geom_BSplineCurve) == kindOfCurve)
        {
        Handle(Geom_BSplineCurve) geomBCurve = Handle(Geom_BSplineCurve)::DownCast(curve);

        return OCBRep::ToCurvePrimitive(geomBCurve, first, last);
        }
    else
        {
        Handle(Geom_BSplineCurve) geomBCurve = GeomConvert::CurveToBSplineCurve(curve); // Handle everything else (Beziers etc.) by conversion to bSpline.

        return OCBRep::ToCurvePrimitive(geomBCurve, first, last);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr OCBRep::ToCurvePrimitive(TopoDS_Edge const& edge)
    {
    Standard_Real      first, last;
    TopLoc_Location    location;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, location, first, last);
    ICurvePrimitivePtr curvePrimitive = OCBRep::ToCurvePrimitive(curve, first, last);

    if (curvePrimitive.IsNull())
        return nullptr;

    if (TopAbs_REVERSED == edge.Orientation())
        curvePrimitive->ReverseCurvesInPlace();

    if (!location.IsIdentity())
        curvePrimitive->TransformInPlace(OCBRep::ToTransform(location.Transformation()));

    return curvePrimitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isSurfaceValidForSolidPrimitive (Handle(Geom_Surface) const& surface)
    {
    Handle(Standard_Type)       kindOfSurface = surface->DynamicType();

    return STANDARD_TYPE (Geom_CylindricalSurface)          == kindOfSurface ||
           STANDARD_TYPE (Geom_ConicalSurface)              == kindOfSurface ||
           STANDARD_TYPE (Geom_SphericalSurface)            == kindOfSurface ||
           STANDARD_TYPE (Geom_ToroidalSurface)             == kindOfSurface ||
           STANDARD_TYPE (Geom_SurfaceOfLinearExtrusion)    == kindOfSurface ||
           STANDARD_TYPE (Geom_SurfaceOfRevolution)         == kindOfSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr OCBRep::ToCurvePrimitive(Handle(Geom2d_Curve) const& curve, double first, double last)
    {
    Handle(Standard_Type) kindOfCurve = curve->DynamicType();

    if (STANDARD_TYPE (Geom2d_Line) == kindOfCurve)
        {
        DSegment3d      segment;

        segment.point[0] = ToDPoint3d (curve->Value (first));
        segment.point[1] = ToDPoint3d (curve->Value (last));
        return ICurvePrimitive::CreateLine (segment);
        }
        
    if (STANDARD_TYPE (Geom2d_Ellipse) == kindOfCurve)
        {
        Handle(Geom2d_Ellipse) ellipse = Handle(Geom2d_Ellipse)::DownCast (curve);

        return ICurvePrimitive::CreateArc (ToDEllipse3d (ellipse->Elips2d(), first, last));
        }
   
    if (STANDARD_TYPE (Geom2d_Circle) == kindOfCurve)
        {
        Handle(Geom2d_Circle) circle = Handle(Geom2d_Circle)::DownCast (curve);

        return ICurvePrimitive::CreateArc (ToDEllipse3d (circle->Circ2d(), first, last));
        }

    if (STANDARD_TYPE (Geom2d_TrimmedCurve) == kindOfCurve)
        {
        Handle(Geom2d_TrimmedCurve)   trimmedCurve = Handle(Geom2d_TrimmedCurve)::DownCast (curve);
        return ToCurvePrimitive(trimmedCurve->BasisCurve(), first, last);
        }

    Handle(Geom2d_BSplineCurve) bCurve;

    if (STANDARD_TYPE (Geom2d_BSplineCurve) == kindOfCurve)
        bCurve = Handle(Geom2d_BSplineCurve)::DownCast (curve);
    else
        bCurve = Geom2dConvert::CurveToBSplineCurve (curve);


    if (0 != bCurve->IsPeriodic() ||
        first != bCurve->FirstParameter() || last != bCurve->LastParameter())
        {
        static double s_parametricTolerance = 1.0E-08;

        bCurve = Geom2dConvert::SplitBSplineCurve (bCurve, first, last, s_parametricTolerance);
        BeAssert (0 == bCurve->IsPeriodic());
        }

    MSBsplineCurvePtr           msBCurve = MSBsplineCurve::CreatePtr();
    bvector<DPoint3d>           poles;
    bvector<double>             knots, weights;
    
    for (int i=1, nPoles = bCurve->NbPoles(); i <= nPoles; i++)
        poles.push_back (ToDPoint3d (bCurve->Pole(i)));

    if (bCurve->IsRational())
        for (int i = 1; i <= bCurve->Weights()->Length(); i++)
            weights.push_back (bCurve->Weights()->Value(i));

    extractKnots (knots, bCurve->KnotSequence(), bCurve->NbPoles(), bCurve->Degree() + 1, false);

    if (SUCCESS != msBCurve->Populate (&poles.front(), bCurve->IsRational() ? &weights.front() : NULL, (int) poles.size(), &knots.front(), (int) knots.size(), bCurve->Degree() + 1, false, false))
        {
        BeAssert(false && "Bspline populate failure");
        return nullptr;
        }

    return ICurvePrimitive::CreateBsplineCurve (*msBCurve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr OCBRep::ToCurvePrimitive(TopoDS_Edge const& edge, TopoDS_Face const& face)
    {
    Standard_Real           first, last;
    Handle(Geom2d_Curve)    curve = BRep_Tool::CurveOnSurface (edge, face, first, last);

    BeAssert(!curve.IsNull() && "Null Edge Curve");
    if (curve.IsNull())
        return nullptr;
        
    auto curvePrimitive = ToCurvePrimitive(curve, first, last);
    BeAssert(curvePrimitive.IsValid() && "Curve primitive extraction failure");
    if (curvePrimitive.IsValid() && TopAbs_REVERSED == edge.Orientation())
        curvePrimitive->ReverseCurvesInPlace();

    return curvePrimitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::ParametricBoundaryCurveVectorFromFace(CurveVectorPtr& boundaries, TopoDS_Face const& face, DRange2dCR range)
    {
    for (TopExp_Explorer wireExplorer (face, TopAbs_WIRE); wireExplorer.More(); wireExplorer.Next())
        {
        CurveVectorPtr              loop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

        for (BRepTools_WireExplorer edgeExplorer ((TopoDS_Wire const&) wireExplorer.Current()); edgeExplorer.More(); edgeExplorer.Next())
            {
            ICurvePrimitivePtr edgePrimitive = ToCurvePrimitive(TopoDS::Edge(edgeExplorer.Current()), face);
            BeAssert(edgePrimitive.IsValid() && "Curve Primitive From Edge Failure");
            if (edgePrimitive.IsNull())
                return ERROR;

            loop->push_back (edgePrimitive);
            }

        DPoint3d    centroid;
        DVec3d      normal;
        double      area;

        if (!loop->CentroidNormalArea (centroid, normal, area))
            {
            BeAssert(false && "Error calculating loop normal");
            return ERROR;
            }
    
        if (normal.z < 0.0)
            loop->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Inner);
    
        if (boundaries.IsNull())
            {
            boundaries = loop;
            }
        else
            {
            if (CurveVector::BOUNDARY_TYPE_ParityRegion != boundaries->GetBoundaryType())
                {
                CurveVectorPtr  firstLoop = boundaries;

                boundaries = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
                boundaries->Add (firstLoop);
                }
            boundaries->Add (loop);
            }
        }
    Transform       rangeTransform;

    if (boundaries.IsValid() &&
        Transform::TryRangeMapping (range, DRange2d::From (0.0, 0.0, 1.0, 1.0), rangeTransform))
        boundaries->TransformInPlace (rangeTransform);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void extractPosition (DPoint3dR location, DVec3dR xVector, DVec3dR yVector, DVec3dR zVector, gp_Ax3 const& position, double zRotation = 0.0)
    {
    location = OCBRep::ToDPoint3d (position.Location());
    zVector  = OCBRep::ToDVec3d (position.Axis().Direction());
    xVector  = OCBRep::ToDVec3d (position.XDirection());
    yVector  = OCBRep::ToDVec3d (position.YDirection());
    
    if (0.0 != zRotation)
        {
        RotMatrix    rMatrix, rotateRMatrix;

        rMatrix.InitFromColumnVectors (xVector, yVector, zVector);
        rotateRMatrix.InitFromAxisAndRotationAngle (2, zRotation);
        rMatrix.InitProduct (rMatrix, rotateRMatrix);

        rMatrix.GetColumn (xVector, 0);
        rMatrix.GetColumn (yVector, 1);
        rMatrix.GetColumn (zVector, 2);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void flipBoundaries (CurveVectorPtr* uvBoundaries, bool flipU, bool flipV)
    {
    if (nullptr == uvBoundaries || !uvBoundaries->IsValid() || (!flipU && !flipV))
        return;

    Transform       transform = Transform::FromIdentity();

    if (flipU)
        {
        transform.ScaleMatrixColumns (transform, -1.0, 1.0, 1.0);
        transform.TranslateInLocalCoordinates (transform, -1.0, 0.0, 0.0);
        }

    if (flipV)
        {
        transform.ScaleMatrixColumns (transform, 1.0, -1.0, 1.0);
        transform.TranslateInLocalCoordinates (transform, 0.0, -1.0, 0.0);
        }
    
    (*uvBoundaries)->TransformInPlace (transform);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void     swapBoundariesXY (CurveVectorPtr* uvBoundaries)
    {
    if (NULL == uvBoundaries || !uvBoundaries->IsValid())
        return;

    Transform swapXY = Transform::From (RotMatrix::From2Vectors (DVec3d::From (0.0, 1.0, 0.0), DVec3d::From (1.0, 0.0, 0.0)));
    
    (*uvBoundaries)->TransformInPlace (swapXY);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidPrimitivePtr solidPrimitiveFromConeFace (gp_Ax3 const& position, double radius, double semiAngle, DRange2dCR uvRange, CurveVectorPtr* uvBoundaries, bool reversed)
    {
    double          radius0, radius1, startAngle = uvRange.low.x, sweepAngle = uvRange.high.x - uvRange.low.x;
    DPoint3d        center0, center1, location;
    DVec3d          xVector, yVector, zVector;

    extractPosition (location, xVector, yVector, zVector, position);

    radius0  = radius + uvRange.low.y  * tan (semiAngle);
    radius1 =  radius + uvRange.high.y * tan (semiAngle);

    center0 = DPoint3d::FromSumOf (location, zVector, uvRange.low.y);
    center1 = DPoint3d::FromSumOf (location, zVector, uvRange.high.y);
        
    if (sweepAngle >= msGeomConst_2pi)
        return ISolidPrimitive::CreateDgnCone (DgnConeDetail (center0, center1, xVector, yVector, radius0, radius1, false));


    if (reversed)
        {
        startAngle = uvRange.low.x;
        }
    else
        {
        startAngle = uvRange.high.x;
        sweepAngle = -sweepAngle;
        }
    flipBoundaries (uvBoundaries, !reversed, true);      // Always flip V to account for topPoint mismatch retained from SS3.  U flipped only if not reversed.  Ick.

    DEllipse3d  baseEllipse, topEllipse;

    baseEllipse.InitFromDGNFields3d (center0, xVector, yVector, radius0, radius0, startAngle, sweepAngle);
    topEllipse.InitFromDGNFields3d (center1, xVector, yVector, radius1, radius1, startAngle, sweepAngle);

    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc (baseEllipse));
    CurveVectorPtr  topCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc (topEllipse));

    return ISolidPrimitive::CreateDgnRuledSweep (DgnRuledSweepDetail (baseCurve, topCurve, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr  solidPrimitiveFromSphereFace (gp_Ax3 const& position, double radius, DRange2dCR uvRange, CurveVectorPtr* uvBoundaries, bool reversed)
    {
    double          sweepAngle = sweepAngle = uvRange.high.x - uvRange.low.x;
    double          arcStart = uvRange.low.y, arcEnd = uvRange.high.y, arcSweep;
    DPoint3d        location;
    DVec3d          xVector, yVector, zVector;

    extractPosition (location, xVector, yVector, zVector, position, uvRange.low.x);

    swapBoundariesXY (uvBoundaries);
    if (reversed)
        {
        arcSweep = arcEnd - arcStart;
        }
    else
        {
        arcSweep = arcStart - arcEnd;
        arcStart = arcEnd;
        flipBoundaries (uvBoundaries, true, false);
        }

    DEllipse3d      ellipse;
    ellipse.InitFromDGNFields3d (location, xVector, zVector, radius, radius, arcStart, arcSweep);

    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc (ellipse));

    DgnRotationalSweepDetail  detail (baseCurve,location, zVector, sweepAngle, false);
    return ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetail (baseCurve, location, zVector, sweepAngle, false));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr  solidPrimitiveFromTorusFace (gp_Ax3 const& position, double majorRadius, double minorRadius, DRange2dCR uvRange, CurveVectorPtr* uvBoundaries, bool reversed)
    {
    double          sweepAngle = sweepAngle = uvRange.high.x - uvRange.low.x;
    double          arcStart = uvRange.low.y, arcEnd = uvRange.high.y, arcSweep;
    DPoint3d        location;
    DVec3d          xVector, yVector, zVector;

    extractPosition (location, xVector, yVector, zVector, position, uvRange.low.x);

    swapBoundariesXY (uvBoundaries);
    if (!reversed)
        {
        arcSweep = arcEnd - arcStart;
        }
    else
        {
        arcSweep = arcStart - arcEnd;
        arcStart = arcEnd;
        flipBoundaries (uvBoundaries, true, false);
        }

    if (arcSweep >= msGeomConst_2pi && (NULL == uvBoundaries || !uvBoundaries->IsValid()))
        return ISolidPrimitive::CreateDgnTorusPipe (DgnTorusPipeDetail (location, xVector, yVector, majorRadius, minorRadius, sweepAngle, false));

    DPoint3d    center;
    DEllipse3d      ellipse;

    center.SumOf (location, xVector, majorRadius);
    ellipse.InitFromDGNFields3d (center, xVector, zVector, minorRadius, minorRadius, arcStart, arcSweep);

    CurveVectorPtr  baseCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc (ellipse));

    return  ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetail (baseCurve, location, zVector, sweepAngle, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr  solidPrimitiveFromExtrusionSurf (Handle(Geom_SurfaceOfLinearExtrusion) const& extrusionSurf, DRange2dCR uvRange, CurveVectorPtr* uvBoundaries, bool reversed)
    {
    ICurvePrimitivePtr basisPrimitive = OCBRep::ToCurvePrimitive(extrusionSurf->BasisCurve(), uvRange.low.x, uvRange.high.x);
    BeAssert(basisPrimitive.IsValid() && "Error Extracting extrusion basis curve");
    if (basisPrimitive.IsNull())
        return nullptr;

    DVec3d  direction = OCBRep::ToDVec3d (extrusionSurf->Direction());

    if (0.0 != uvRange.low.y)
        {
        DVec3d      delta;

        delta.Scale (direction, uvRange.low.y);
        basisPrimitive->TransformInPlace (Transform::From (delta.x, delta.y, delta.z));
        }

    if (reversed)
        {
        basisPrimitive->ReverseCurvesInPlace();
        flipBoundaries (uvBoundaries, true, false);
        }

    DVec3d      extrusionVector;
    
    extrusionVector.Scale (direction, uvRange.high.y - uvRange.low.y);
    CurveVectorPtr     basisCurveVector = CurveVector::Create (extrusionSurf->IsUClosed() ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, basisPrimitive);
    return  ISolidPrimitive::CreateDgnExtrusion (DgnExtrusionDetail (basisCurveVector, extrusionVector, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ISolidPrimitivePtr  solidPrimitiveFromRevolutionSurf (Handle(Geom_SurfaceOfRevolution) const& revolutionSurf, DRange2dCR uvRange, CurveVectorPtr* uvBoundaries, bool reversed)
    {
    ICurvePrimitivePtr basisPrimitive = OCBRep::ToCurvePrimitive(revolutionSurf->BasisCurve(), uvRange.low.y, uvRange.high.y);
    BeAssert(basisPrimitive.IsValid() && "Error Extracting Revolution basis curve");
    if (basisPrimitive.IsNull())
        return nullptr;

    DRay3d      axis       = DRay3d::FromOriginAndVector (OCBRep::ToDPoint3d (revolutionSurf->Location()),  OCBRep::ToDVec3d (revolutionSurf->Direction()));
    double      sweepAngle = uvRange.high.x - uvRange.low.x;

    if (0.0 != uvRange.low.x)
        basisPrimitive->TransformInPlace (Transform::FromAxisAndRotationAngle (axis, uvRange.low.x));

    if (reversed)
        {
        basisPrimitive->ReverseCurvesInPlace();
        flipBoundaries (uvBoundaries, true, false);
        }

    CurveVectorPtr     basisCurveVector = CurveVector::Create (revolutionSurf->IsUClosed() ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, basisPrimitive);
    return  ISolidPrimitive::CreateDgnRotationalSweep (DgnRotationalSweepDetail (basisCurveVector, axis.origin, axis.direction, sweepAngle, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidPrimitivePtr OCBRep::ToSolidPrimitive(CurveVectorPtr* boundaries, Handle(Geom_Surface) const& surface, DRange2dCR uvRange, bool reversed)
    {
    Handle(Standard_Type)       kindOfSurface = surface->DynamicType();

    if (STANDARD_TYPE (Geom_CylindricalSurface) == kindOfSurface)
        {
        Handle(Geom_CylindricalSurface) cylSurf = Handle(Geom_CylindricalSurface)::DownCast (surface);
        return solidPrimitiveFromConeFace (cylSurf->Position(), cylSurf->Radius(), 0.0, uvRange, boundaries, reversed);
        }
    else if (STANDARD_TYPE (Geom_ConicalSurface) == kindOfSurface)
        {
        Handle(Geom_ConicalSurface) coneSurf = Handle(Geom_ConicalSurface)::DownCast (surface);
        return solidPrimitiveFromConeFace (coneSurf->Position(), coneSurf->RefRadius(), coneSurf->SemiAngle(), uvRange, boundaries, reversed);
        }
    else if (STANDARD_TYPE (Geom_SphericalSurface) == kindOfSurface)
        {
        Handle(Geom_SphericalSurface) sphereSurf = Handle(Geom_SphericalSurface)::DownCast (surface);
        return solidPrimitiveFromSphereFace (sphereSurf->Position(), sphereSurf->Radius(), uvRange, boundaries, reversed);
        }
    else if (STANDARD_TYPE (Geom_ToroidalSurface) == kindOfSurface)
        {
        Handle(Geom_ToroidalSurface) torusSurf = Handle(Geom_ToroidalSurface)::DownCast (surface);
        return solidPrimitiveFromTorusFace (torusSurf->Position(), torusSurf->MajorRadius(), torusSurf->MinorRadius(), uvRange, boundaries, reversed);
        }
    else if (STANDARD_TYPE (Geom_SurfaceOfLinearExtrusion) == kindOfSurface)
        {
        Handle(Geom_SurfaceOfLinearExtrusion) extrusionSurf = Handle(Geom_SurfaceOfLinearExtrusion)::DownCast (surface);
        return solidPrimitiveFromExtrusionSurf (extrusionSurf, uvRange, boundaries, reversed);
        }
    else if (STANDARD_TYPE (Geom_SurfaceOfRevolution) == kindOfSurface)
        {
        Handle(Geom_SurfaceOfRevolution) revolutionSurf = Handle(Geom_SurfaceOfRevolution)::DownCast (surface);
        return solidPrimitiveFromRevolutionSurf (revolutionSurf, uvRange, boundaries, reversed);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolidPrimitivePtr OCBRep::ToSolidPrimitive(CurveVectorPtr& boundaries, TopoDS_Face const& face)
    {
    TopLoc_Location             location;
    Handle(Geom_Surface)        surface = BRep_Tool::Surface (face, location);
    Handle(Standard_Type)       kindOfSurface = surface->DynamicType();

    if (STANDARD_TYPE (Geom_RectangularTrimmedSurface) == kindOfSurface)
        surface = Handle(Geom_RectangularTrimmedSurface)::DownCast (surface)->BasisSurface();

    if (!isSurfaceValidForSolidPrimitive (surface))
        return nullptr;

    DRange2d                    uvRange;

    BRepTools::UVBounds (face, uvRange.low.x, uvRange.high.x, uvRange.low.y, uvRange.high.y);

    if (SUCCESS != ParametricBoundaryCurveVectorFromFace (boundaries, face, uvRange))
        {
        BeAssert(false && "Parametric Curve extraction failure");
        return nullptr;
        }

    ISolidPrimitivePtr solidPrimitive = ToSolidPrimitive(&boundaries, surface, uvRange, TopAbs_REVERSED == face.Orientation());
    if (solidPrimitive.IsNull())
        return nullptr;

    if (!location.IsIdentity())
        solidPrimitive->TransformInPlace (ToTransform (location.Transformation()));

    DPoint3d            centroid;
    DVec3d              normal;
    double              area;

    static double s_nearOne = .99999;

    if (boundaries.IsValid() && 
        boundaries->CentroidNormalArea (centroid, normal, area) &&
        area >  s_nearOne)
        boundaries = NULL;

    return solidPrimitive;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr OCBRep::ToCurveVector(TopoDS_Face const& face)
    {
    CurveVectorPtr cv;
    CurveVectorFromPlanarFace(cv, face);
    return cv;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   OCBRep::CurveVectorFromPlanarFace (CurveVectorPtr& curveVector, TopoDS_Face const& face)
    {
    TopLoc_Location     location;

    Handle(Geom_Surface) const& surface = BRep_Tool::Surface (face, location);

    if (!surface->IsKind (STANDARD_TYPE (Geom_Plane)))
        return ERROR;

    Handle(Geom_Plane) geomPlane = Handle(Geom_Plane)::DownCast (surface);
    
    DVec3d      planeNormal = ToDVec3d (geomPlane->Pln().Axis().Direction());

    if (TopAbs_REVERSED == face.Orientation())
        planeNormal.Negate();
    
    for (TopExp_Explorer wireExplorer (face, TopAbs_WIRE); wireExplorer.More(); wireExplorer.Next())
        {
        CurveVectorPtr loop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

        for (BRepTools_WireExplorer edgeExplorer (TopoDS::Wire(wireExplorer.Current())); edgeExplorer.More(); edgeExplorer.Next())
            {
            ICurvePrimitivePtr edgePrimitive = ToCurvePrimitive(TopoDS::Edge(edgeExplorer.Current()));
            BeAssert(edgePrimitive.IsValid() && "Curve Primitive From Edge Failure");
            if (edgePrimitive.IsNull())
                return ERROR;

            loop->push_back (edgePrimitive);
            }

        DPoint3d    centroid;
        DVec3d      normal;
        double      area;

        if (!loop->CentroidNormalArea (centroid, normal, area))
            {
            BeAssert(false && "Error calculating loop normal");
            return ERROR;
            }
    
        if (normal.DotProduct (planeNormal) < 0.0)
            loop->SetBoundaryType (CurveVector::BOUNDARY_TYPE_Inner);
    
        if (curveVector.IsNull())
            {
            curveVector = loop;
            }
        else
            {
            if (CurveVector::BOUNDARY_TYPE_ParityRegion != curveVector->GetBoundaryType())
                {
                CurveVectorPtr  firstLoop = curveVector;

                curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
                curveVector->Add (firstLoop);
                }
            curveVector->Add (loop);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineSurfacePtr OCBRep::ToBsplineSurface(CurveVectorPtr& boundaries, TopoDS_Face const& face, double tolerance)
    {
    TopLoc_Location             location;
    Handle(Geom_Surface)        surface = BRep_Tool::Surface (face, location);

    if (STANDARD_TYPE (Geom_RectangularTrimmedSurface) == surface->DynamicType())
        surface = Handle(Geom_RectangularTrimmedSurface)::DownCast (surface)->BasisSurface();

    if (STANDARD_TYPE (Geom_BSplineSurface) != surface->DynamicType())
        return nullptr;

    Handle(Geom_BSplineSurface) bSurface = Handle(Geom_BSplineSurface)::DownCast (surface);

    bvector<DPoint3d>           poles;
    bvector<double>             uKnots, vKnots, weights;
    bool                        isRational = bSurface->IsURational() || bSurface->IsVRational();
    int                         nUPoles = bSurface->NbUPoles(), nVPoles = bSurface->NbVPoles();
    DRange2d                    uvRange;

    BRepTools::UVBounds (face, uvRange.low.x, uvRange.high.x, uvRange.low.y, uvRange.high.y);

    if (SUCCESS != ParametricBoundaryCurveVectorFromFace (boundaries, face, uvRange))
        {
        BeAssert (false && "Parametric Curve extraction failure");
        return nullptr;
        }

    for (int i=1; i <= nVPoles; i++)
        for (int j=1; j <= nUPoles; j++)               
            poles.push_back (ToDPoint3d (bSurface->Pole(j, i)));

    if (isRational)
        for (int i=1; i <= nVPoles; i++)
            for (int j=1; j <= nUPoles; j++)               
                weights.push_back (bSurface->Weight (j,i));

    extractKnots (uKnots, bSurface->UKnotSequence(), nUPoles, bSurface->UDegree() + 1, 0 != bSurface->IsUPeriodic());
    extractKnots (vKnots, bSurface->VKnotSequence(), nVPoles, bSurface->VDegree() + 1, 0 != bSurface->IsVPeriodic());

    if (0 != bSurface->IsUPeriodic())
        {
        // Append duplicate first column of poles.
        bvector<DPoint3d>   savePoles = poles;
        bvector<double >    saveWeights = weights;

        poles.clear();
        weights.clear();
        for (int i=0; i < nVPoles; i++)
            {
            int     row = i * bSurface->NbUPoles();
            for (int j=0; j < nUPoles; j++)
                {
                poles.push_back (savePoles.at (row + j));
                if (isRational)
                    weights.push_back (saveWeights.at (row+j));
                }

            poles.push_back (savePoles.at (row));
            if (isRational)
                weights.push_back (saveWeights.at (row));
            }
        nUPoles++;
        }

    if (0 != bSurface->IsVPeriodic())
        {
        // Append duplicate of first row of poles.
        for (int j=0, nUPoles = bSurface->NbUPoles(); j < nUPoles; j++)
            {
            poles.push_back (poles.at(j));
            if (isRational)
                weights.push_back (weights.at(j));
            }

        nVPoles++;
        }

    auto msBSurface = MSBsplineSurface::CreatePtr();

    if (SUCCESS != msBSurface->Populate (poles, isRational ? &weights : NULL, &uKnots, 1 + bSurface->UDegree(), nUPoles, false,
                                                                              &vKnots, 1 + bSurface->VDegree(), nVPoles, false, false))
        {
        BeAssert(false && "Bspline surface populate failure");
        return nullptr;
        }

    if (!location.IsIdentity())
        msBSurface->TransformSurface (OCBRep::ToTransform (location.Transformation()));

    return msBSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool OCBRep::Locate::Faces(TopoDS_Shape const& shape, DRay3dCR boresite, bvector<TopoDS_Face>& faces, bvector<DPoint2d>& params, bvector<DPoint3d>& points, uint32_t maxFace)
    {
    IntCurvesFace_ShapeIntersector shint;

    shint.Load(shape, Precision::Confusion());
    shint.Perform(OCBRep::ToGpLin(boresite), -RealLast(), RealLast());

    if (!shint.IsDone() || 0 == shint.NbPnt())
        return false;

    int nHits = ((int) maxFace < shint.NbPnt() ? (int) maxFace : shint.NbPnt());

    for (int iHit=1; iHit <= nHits; ++iHit)
        {
        faces.push_back(shint.Face(iHit));
        params.push_back(DPoint2d::From(shint.UParameter(iHit), shint.VParameter(iHit)));
        points.push_back(OCBRep::ToDPoint3d(shint.Pnt(iHit)));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool OCBRep::Locate::Edges(TopoDS_Shape const& shape, DRay3dCR boresite, double maxDistance, bvector<TopoDS_Edge>& edges, bvector<double>& params, uint32_t maxEdge)
    {
    TopTools_IndexedMapOfShape allEdge;
    TopTools_IndexedMapOfShape allVert;

    TopExp::MapShapes(shape, TopAbs_EDGE, allEdge);
    TopExp::MapShapes(shape, TopAbs_VERTEX, allVert);

    PrimitivePicker picker(allVert, allEdge, PrimitivePicker::PrimitivePicker_TypeOfAlgo::PrimitivePicker_BRUTE_FORCE);

    NCollection_Array1<Standard_Character> pickedEdge(1, allEdge.Size());
    NCollection_Array1<Standard_Character> pickedVert(1, allVert.Size());
    NCollection_Array1<Standard_Real> curveParams(1, allEdge.Size());

    picker.PickEdges(OCBRep::ToGpPnt(boresite.origin), OCBRep::ToGpDir(boresite.direction), maxDistance, pickedEdge, curveParams);

    for (Standard_Integer iEdge = 1; iEdge <= pickedEdge.Size(); ++iEdge)
        {
        if (1 != pickedEdge.Value(iEdge))
            continue;

        edges.push_back(TopoDS::Edge(allEdge.FindKey(iEdge)));
        params.push_back(curveParams.Value(iEdge));

        if (edges.size() == maxEdge)
            break;
        }    

    return !edges.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool OCBRep::Locate::Vertices(TopoDS_Shape const& shape, DRay3dCR boresite, double maxDistance, bvector<TopoDS_Vertex>& vertices, uint32_t maxVertex)
    {
    TopTools_IndexedMapOfShape allEdge;
    TopTools_IndexedMapOfShape allVert;

    TopExp::MapShapes(shape, TopAbs_EDGE, allEdge);
    TopExp::MapShapes(shape, TopAbs_VERTEX, allVert);

    PrimitivePicker picker(allVert, allEdge, PrimitivePicker::PrimitivePicker_TypeOfAlgo::PrimitivePicker_BRUTE_FORCE);

    NCollection_Array1<Standard_Character> pickedEdge(1, allEdge.Size());
    NCollection_Array1<Standard_Character> pickedVert(1, allVert.Size());
    NCollection_Array1<Standard_Real> curveParams(1, allEdge.Size());

    picker.PickVertices(OCBRep::ToGpPnt(boresite.origin), OCBRep::ToGpDir(boresite.direction), maxDistance, pickedVert);

    for (Standard_Integer iVert = 1; iVert <= pickedVert.Size(); ++iVert)
        {
        if (1 != pickedVert.Value(iVert))
            continue;

        vertices.push_back(TopoDS::Vertex(allVert.FindKey(iVert)));

        if (vertices.size() == maxVertex)
            break;
        }    

    return !vertices.empty();
    }
