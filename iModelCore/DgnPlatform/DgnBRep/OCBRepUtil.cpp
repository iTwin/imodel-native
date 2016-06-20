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
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void OCBRep::GetOcctKnots(TColStd_Array1OfReal*& occtKnots, TColStd_Array1OfInteger*& occtMultiplicities, bvector<double> const& knots, int order)
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
    GetOcctKnots(occtKnots, occtMultiplicities, knots, bCurve->GetIntOrder());

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
    GetOcctKnots(occtKnots, occtMultiplicities, knots, bCurve->GetIntOrder());

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
static ICurvePrimitivePtr toCurvePrimitive(Handle(Geom_BSplineCurve) const& geomBCurveIn, double first, double last)
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

        return toCurvePrimitive(geomBCurve, first, last);
        }
    else
        {
        Handle(Geom_BSplineCurve) geomBCurve = GeomConvert::CurveToBSplineCurve(curve); // Handle everything else (Beziers etc.) by conversion to bSpline.

        return toCurvePrimitive(geomBCurve, first, last);
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
