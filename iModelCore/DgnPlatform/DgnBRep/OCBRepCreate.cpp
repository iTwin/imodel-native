/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/OCBRepCreate.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/OCBRep.h>

// Default values for gap closure options ...
static double s_defaultEqualPointTolerance = 1.0e-10;   // Similiar to PSD resabs...should this be SMALLER so that the "move the endpoints" machinery REALLY close the gaps???
static double s_defaultMaxDirectAdjust = 1.0e-4;        // Gaps this large can be closed by just moving endpoints (i.e. without gap segment). Adjustment can be away from the curve direction.
static double s_defaultMaxAdjustAlongCurve = 1.0e-3;    // Motion along the curve by this much is permitted.
static double s_cylinderRadiusTolerance = 1.0E-8;       // For skewed cone/cylinder classification

static BentleyStatus bodyFromTopLevelCurveVector(TopoDS_Shape& shape, CurveVectorCR rawCurves, TransformCR dgnToSolid, bool coverClosed = true, bool flattenClosed = true, bool triangulateClosed = true);
static BentleyStatus bodyFromCurveVector(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid, bool coverClosed);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromPolyface(TopoDS_Shape& shape, PolyfaceQueryCR mesh, TransformCR dgnToSolid)
    {
    // NEEDSWORK: This is undoubtedly taking the brute force approach...hopefully Earlin can look into doing this the "right" way...whatever that is...
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(mesh, true);

    visitor->SetNumWrap (1);

    BRepBuilderAPI_Sewing sewBuilder;

    for (visitor->Reset(); visitor->AdvanceToNextFace(); )
        {
        BRepBuilderAPI_MakePolygon polyBuilder;

        for (DPoint3d point : visitor->Point())
            {
            dgnToSolid.Multiply(point);
            polyBuilder.Add(OCBRep::ToGpPnt(point));
            }

        TopoDS_Wire wire = polyBuilder.Wire();

        if (wire.IsNull())
            continue;

        TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

        if (face.IsNull())
            continue;

        sewBuilder.Add(face);
        }

    sewBuilder.Perform();

    shape = sewBuilder.SewedShape();

    if (shape.IsNull())
        return ERROR;

    if (TopAbs_SHELL == shape.ShapeType() && mesh.IsClosedByEdgePairing())
        {
        BRepBuilderAPI_MakeSolid solidBuilder;

        solidBuilder.Add(TopoDS::Shell(shape));
        solidBuilder.Build();

        if (!solidBuilder.IsDone())
            return ERROR;

        shape = solidBuilder.Shape();
        }

    return (shape.IsNull() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromPolyface(TopoDS_Shape& shape, PolyfaceQueryCR mesh)
    {
    if (mesh.GetPointCount() < 3)
        return ERROR;

    Transform   solidToDgn, dgnToSolid;

    solidToDgn = Transform::From(*mesh.GetPointCP());
    dgnToSolid.InverseOf(solidToDgn);

    if (SUCCESS != bodyFromPolyface(shape, mesh, dgnToSolid))
        return ERROR;

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus createBSurfEdgeWireFromBoundary(TopoDS_Wire& wire, Handle(Geom_BSplineSurface) const& geomSurface, CurveVectorCR boundary)
    {
    if (CurveVector::BOUNDARY_TYPE_Outer != boundary.GetBoundaryType())
        {
        BeAssert(false && "Unexpected BSurf Boundary Type");
        return ERROR;
        }

    BRepBuilderAPI_MakeWire wireBuilder;

    for (auto& curvePrimitive : boundary)
        {
        if (nullptr == curvePrimitive->GetBsplineCurveCP())
            {
            BeAssert(false && "FUnexpected BSurf Boundary Primitive");
            return ERROR;
            }

        Handle(Geom2d_BSplineCurve) boundaryGeomCurve = OCBRep::ToGeom2dBSplineCurve(*curvePrimitive->GetBsplineCurveCP(), nullptr);

        if (boundaryGeomCurve.IsNull())
            {
            BeAssert(false && "BSurf Boundary Creation Error");
            return ERROR;
            }

        BRepBuilderAPI_MakeEdge edgeBuilder(boundaryGeomCurve, geomSurface);

        if (!edgeBuilder.IsDone())
            {
            BeAssert(false && "Edge Builder error on bSurf edge");
            return ERROR;
            }

        wireBuilder.Add(edgeBuilder);

        if (!wireBuilder.IsDone())
            {
            BeAssert(false && "Wire Builder error on bSurf edge");
            return ERROR;
            }
        }

    wire = wireBuilder.Wire();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromBSurface(TopoDS_Shape& shape, MSBsplineSurfaceCR bSurface, TransformCR dgnToSolid)
    {
    MSBsplineSurfacePtr surface = bSurface.CreateCopyTransformed(dgnToSolid);

    if (surface->GetIsUClosed())
        surface->MakeOpen(0.0, BSSURF_U); // Needs work -- Can we somehow send periodic knot vector intact??  -- for now ignore periodic knot nonsense.

    if (surface->GetIsVClosed())
        surface->MakeOpen(0.0, BSSURF_V); // Needs work -- Can we somehow send periodic knot vector intact??  -- for now ignore periodic knot nonsense.

    bvector<double> uKnots, vKnots;
    TColStd_Array1OfReal* occtUKnots = nullptr;
    TColStd_Array1OfReal* occtVKnots = nullptr;
    TColStd_Array1OfInteger* occtUMultiplicities = nullptr;
    TColStd_Array1OfInteger* occtVMultiplicities = nullptr;
    TColgp_Array2OfPnt occtPoles(1, (int) surface->GetIntNumUPoles(), 1, surface->GetIntNumVPoles());

    surface->GetUKnots(uKnots);
    surface->GetVKnots(vKnots);
    OCBRep::GetOcctKnots(occtUKnots, occtUMultiplicities, uKnots, surface->GetIntUOrder());
    OCBRep::GetOcctKnots(occtVKnots, occtVMultiplicities, vKnots, surface->GetIntVOrder());
    
    for (int i=0; i < surface->GetIntNumUPoles(); i++)
        for (int j=0; j < surface->GetIntNumVPoles(); j++)
            occtPoles.ChangeValue(i+1, j+1) = OCBRep::ToGpPnt(surface->GetUnWeightedPole(i, j));
       
    Handle(Geom_BSplineSurface) geomSurface;

    if (surface->HasWeights())                          
        {
        TColStd_Array2OfReal occtWeights(1, (int) surface->GetIntNumUPoles(), 1, surface->GetIntNumVPoles());

        for (int i=0; i < (int) surface->GetIntNumUPoles(); i++)
            for (int j=0; j < (int) surface->GetIntNumVPoles(); j++)
                occtWeights.ChangeValue(i+1, j+1) = surface->GetWeight(i, j);

        geomSurface = new Geom_BSplineSurface(occtPoles, occtWeights, *occtUKnots, *occtVKnots, *occtUMultiplicities, *occtVMultiplicities, surface->GetIntUOrder()-1, surface->GetIntVOrder()-1, false, false);
        }
    else
        {
        geomSurface = new Geom_BSplineSurface(occtPoles, *occtUKnots, *occtVKnots,  *occtUMultiplicities, *occtVMultiplicities, surface->GetIntUOrder()-1, surface->GetIntVOrder()-1, false, false);
        }

    DELETE_AND_CLEAR(occtUKnots)
    DELETE_AND_CLEAR(occtUMultiplicities);
    DELETE_AND_CLEAR(occtVKnots)
    DELETE_AND_CLEAR(occtVMultiplicities);
    
    if (0 == surface->GetNumBounds())
        {
        BRepBuilderAPI_MakeShell shellBuilder(geomSurface);
                                                      
        if (!shellBuilder.IsDone())
            return ERROR;

        shape = shellBuilder.Shell();
        BRepLib::BuildCurves3d (shape);

        return (shape.IsNull() ? ERROR : SUCCESS);
        }

    CurveVectorPtr uvBoundaries = surface->GetUVBoundaryCurves(true, true);
    
    switch (uvBoundaries->GetBoundaryType())
        {
        case CurveVector::BOUNDARY_TYPE_Outer:
            {
            TopoDS_Wire boundaryWire;

            if (SUCCESS == createBSurfEdgeWireFromBoundary(boundaryWire, geomSurface, *uvBoundaries))
                {
                BRepBuilderAPI_MakeFace faceBuilder(geomSurface, boundaryWire);

                if (!faceBuilder.IsDone())
                    {
                    BeAssert(false && "Face Builder Error");
                    return ERROR;
                    }

                shape = faceBuilder.Face();
                }
            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            TopoDS_Face face;

            for (auto& uvBoundary : *uvBoundaries)
                {
                TopoDS_Wire     boundaryWire;
                CurveVectorPtr  childCurveVector = uvBoundary->GetChildCurveVectorP();
                    
                if (childCurveVector.IsNull())
                    childCurveVector = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, uvBoundary);

                if (SUCCESS != createBSurfEdgeWireFromBoundary(boundaryWire, geomSurface, *childCurveVector))
                    {
                    BeAssert(false && "BSurf Boundary extraction error");
                    return ERROR;
                    }

                BRepBuilderAPI_MakeFace faceBuilder;

                if (face.IsNull())
                    faceBuilder = BRepBuilderAPI_MakeFace(geomSurface, boundaryWire);
                else
                    faceBuilder = BRepBuilderAPI_MakeFace(face, boundaryWire);

                if (!faceBuilder.IsDone())
                    {
                    BeAssert(false && "Face Builder Error");
                    return ERROR;
                    }

                face = faceBuilder.Face();
                }

            shape = face;
            break;
            }

        default:
            {
            BeAssert(false && "Unexpected boundary type");
            return ERROR;
            }
        }

    BRepLib::BuildCurves3d(shape);

    return (shape.IsNull() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromBSurface(TopoDS_Shape& shape, MSBsplineSurfaceCR bSurface)
    {
    Transform   solidToDgn, dgnToSolid;

    solidToDgn = Transform::From(bSurface.GetUnWeightedPole(0, 0));
    dgnToSolid.InverseOf(solidToDgn);

    if (SUCCESS != bodyFromBSurface(shape, bSurface, dgnToSolid))
        return ERROR;

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool resolveCurveVectorGaps(CurveVectorPtr& curvesNoGaps, CurveVectorCR rawCurves)
    {
    curvesNoGaps = rawCurves.CloneWithGapsClosed(CurveGapOptions(s_defaultEqualPointTolerance, s_defaultMaxDirectAdjust, s_defaultMaxAdjustAlongCurve));

    return (curvesNoGaps->MaxGapWithinPath () < s_defaultEqualPointTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool testClosedPathFlattening(TransformR compositeTransform, CurveVectorCR curves)
    {
    if (curves.IsClosedPath())
        {
        switch (curves.HasSingleCurvePrimitive())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                {
                Transform localToWorld, worldToLocal;

                if (!curves.GetAnyFrenetFrame(localToWorld) || !worldToLocal.InverseOf(localToWorld))
                    return false;

                DRange3d localRange;

                if (!curves.GetRange(localRange, worldToLocal))
                    return false;

                double planarTolerance = (localRange.low.DistanceXY(localRange.high)) * 1.0e-3;

                if (fabs(localRange.high.z - localRange.low.z) > planarTolerance)
                    return false; // Don't flatten or try to cover REALLY non-planar closed bcurves...

                break;
                }
            }
        }

    double   area;
    DVec3d   normal;
    DPoint3d centroid;

    if (curves.IsAnyRegionType() && curves.CentroidNormalArea(centroid, normal, area))
        {
        Transform flattenTransform;

        flattenTransform.InitFromProjectionToPlane(centroid, normal);
        compositeTransform.InitProduct(compositeTransform, flattenTransform);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool triangulatedBodyFromNonPlanarPolygon(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid)
    {
    if (!curves.IsAnyRegionType())
        return false;

    if (curves.ContainsNonLinearPrimitive())
        return false;

    Transform   localToWorld, worldToLocal;
    DRange3d    range;

    if (curves.IsPlanar(localToWorld, worldToLocal, range))
        return false;

    IFacetOptionsPtr         facetOptions = IFacetOptions::Create();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*facetOptions);

    builder->AddRegion(curves);

    return (SUCCESS == bodyFromPolyface(shape, builder->GetClientMeshR(), dgnToSolid) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromUnionRegion(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid)
    {
    TopTools_ListOfShape regions;

    for (ICurvePrimitivePtr curve : curves)
        {
        if (curve.IsNull())
            continue;

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
            {
            BeAssert(false && "Unexpected entry in union region");
            return ERROR; // Each loop must be a child curve bvector (a closed loop or parity region)...
            }

        CurveVectorCP childCurves = curve->GetChildCurveVectorCP();
        TopoDS_Shape  childShape;

        if (SUCCESS != bodyFromCurveVector(childShape, *childCurves, dgnToSolid, true))
            return ERROR;

        regions.Append(childShape);
        }

    if (1 == regions.Extent())
        {
        shape = regions.First();
        }
    else
        {
        BRepAlgoAPI_Fuse fuseBuilder;

        fuseBuilder.SetArguments(regions);
        fuseBuilder.Build();

        if (0 != fuseBuilder.ErrorStatus())
            {
            BeAssert(false && "Union region boolean failed");
            return ERROR;
            }

        shape = fuseBuilder.Shape();
        }

    return (shape.IsNull() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromParityRegion(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid)
    {
    TopTools_ListOfShape outer;
    TopTools_ListOfShape holes;

    for (ICurvePrimitivePtr curve : curves)
        {
        if (curve.IsNull())
            continue;

        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
            {
            BeAssert(false && "Unexpected entry in parity region");
            return ERROR; // Each loop must be a child curve bvector (a closed loop)...
            }

        CurveVectorCP childCurves = curve->GetChildCurveVectorCP();
        TopoDS_Shape  childShape;

        if (SUCCESS != bodyFromCurveVector(childShape, *childCurves, dgnToSolid, true))
            return ERROR;

        if (CurveVector::BOUNDARY_TYPE_Outer == childCurves->GetBoundaryType())
            outer.Append(childShape);
        else
            holes.Append(childShape);
        }

    if (1 != outer.Extent())
        {
        BeAssert(false && "Invalid Parity Region");
        return ERROR;
        }

    if (holes.IsEmpty())
        {
        shape = outer.First();
        }
    else
        {
        BRepAlgoAPI_Cut cutBuilder;

        cutBuilder.SetArguments(outer);
        cutBuilder.SetTools(holes);
        cutBuilder.Build();

        if (0 != cutBuilder.ErrorStatus())
            {
            BeAssert(false && "Subtract region boolean failed");
            return ERROR;
            }

        shape = cutBuilder.Shape();
        }

    return (shape.IsNull() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromCurveVector(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid, bool coverClosed)
    {
    if (curves.IsUnionRegion())
        return bodyFromUnionRegion(shape, curves, dgnToSolid);
    else if (curves.IsParityRegion())
        return bodyFromParityRegion(shape, curves, dgnToSolid);

    BRepBuilderAPI_MakeWire wireBuilder;

    // NOTE: Scribe curves in reverse order to match SS3 behavior (required for entity id assignment)...
    for (size_t iCurve = curves.size(); iCurve > 0; --iCurve)
        {
        ICurvePrimitivePtr curve = curves.at(iCurve-1);

        if (!curve.IsValid())
            continue;

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                curve = ICurvePrimitive::CreateLineString(&curve->GetLineCP()->point[0], 2);

                // FALL THROUGH...
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d> points = *curve->GetLineStringCP();

                dgnToSolid.Multiply(&points.front(), (int) points.size ());

                static double s_minimumSegmentLength = 1.0E-8;

                // NOTE: Scribe segments in reverse order to match SS3 behavior (required for entity id assignment)...
                for (size_t j = points.size()-1; j > 0; --j)
                    {
                    DSegment3d segment = DSegment3d::From(points[j-1], points[j]);

                    if (segment.Length() <= s_minimumSegmentLength)
                        {
                        // Make sure that the last (closure) point is used and that interior small segments are consolidated...
                        points[j-1] = points[j];
                        continue;
                        }

                    BRepBuilderAPI_MakeEdge edgeBuilder(OCBRep::ToGpPnt(segment.point[0]), OCBRep::ToGpPnt(segment.point[1]));

                    if (!edgeBuilder.IsDone())
                        {
                        BeAssert(false && "Failure to create edge from segment");
                        return ERROR;
                        }

                    wireBuilder.Add(edgeBuilder);

                    if (!wireBuilder.IsDone())
                        {
                        BeAssert(false && "Failure to add edge to wire");
                        return ERROR;
                        }
                    }
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                double start, end;
                DEllipse3d ellipse = *curve->GetArcCP();
                BRepBuilderAPI_MakeEdge edgeBuilder;

                dgnToSolid.Multiply(ellipse, ellipse);

                if (ellipse.IsCircular())
                    {
                    gp_Circ gpCirc = OCBRep::ToGpCirc(start, end, ellipse);
                    edgeBuilder = BRepBuilderAPI_MakeEdge(gpCirc, start, end);
                    }
                else
                    {
                    gp_Elips gpEllipse = OCBRep::ToGpElips(start, end, ellipse);
                    edgeBuilder = BRepBuilderAPI_MakeEdge(gpEllipse, start, end);
                    }

                if (!edgeBuilder.IsDone())
                    {
                    BeAssert(false && "Failure to create edge from arc");
                    return ERROR;
                    }

                wireBuilder.Add(edgeBuilder);

                if (!wireBuilder.IsDone())
                    {
                    BeAssert(false && "Failure to add edge to wire");
                    return ERROR;
                    }
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            default:
                {
                MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();

                if (nullptr == bcurve)
                    {
                    BeAssert(false && "Unexpected entry in CurveVector");
                    return ERROR;
                    }

                Handle(Geom_BSplineCurve) geomBCurve = OCBRep::ToGeomBSplineCurve(*bcurve, &dgnToSolid);

                if (geomBCurve.IsNull())
                    {
                    BeAssert(false && "Failure to create edge from bcurve");
                    return ERROR;
                    }

                wireBuilder.Add(BRepBuilderAPI_MakeEdge(geomBCurve));

                if (!wireBuilder.IsDone())
                    {
                    BeAssert(false && "Failure to add edge to wire");
                    return ERROR;
                    }
                break;
                }
            }
        }

    if (coverClosed && curves.IsClosedPath())
        {
        BRepBuilderAPI_MakeFace faceBuilder(wireBuilder);

        if (!faceBuilder.IsDone())
            {
            BeAssert(false && "Error creating shape from wire");
            return ERROR;
            }

        shape = faceBuilder;

        return SUCCESS;
        }

    shape = wireBuilder;

    return (shape.IsNull() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromTopLevelCurveVector(TopoDS_Shape& shape, CurveVectorCR rawCurves, TransformCR dgnToSolid, bool coverClosed, bool flattenClosed, bool triangulateClosed)
    {
    if (1 > rawCurves.size())
        return ERROR;

    CurveVectorPtr  curvesNoGaps;
    CurveVectorCP   curves = (resolveCurveVectorGaps(curvesNoGaps, rawCurves) ? curvesNoGaps.get() : &rawCurves);

    if (coverClosed && triangulateClosed && triangulatedBodyFromNonPlanarPolygon(shape, *curves, dgnToSolid))
        return SUCCESS;

    Transform   compositeTransform = dgnToSolid;

    if (coverClosed && flattenClosed)
        coverClosed = testClosedPathFlattening(compositeTransform, *curves);       

    return bodyFromCurveVector(shape, *curves, compositeTransform, coverClosed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromCurveVector(TopoDS_Shape& shape, CurveVectorCR curves)
    {
    if (1 > curves.size())
        return ERROR;

    DPoint3d    startPt;

    if (!curves.GetStartPoint(startPt))
        return ERROR;

    Transform   solidToDgn, dgnToSolid;

    solidToDgn = Transform::From(startPt);
    dgnToSolid.InverseOf(solidToDgn);

    if (SUCCESS != bodyFromTopLevelCurveVector(shape, curves, dgnToSolid))
        return ERROR;

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromCone(TopoDS_Shape& shape, DgnConeDetailCR cone)
    {
    bool        capped;
    double      radiusA, radiusB;
    DPoint3d    centerA, centerB;
    RotMatrix   rMatrix;

    if (!cone.IsCircular(centerA, centerB, rMatrix, radiusA, radiusB, capped))
        {
        BeAssert(false && "Error noncircular cone");
        return ERROR;
        }

    DVec3d baseNormal, delta, skewVector;

    rMatrix.GetColumn(baseNormal, 2);
    delta.DifferenceOf(centerB, centerA);
    skewVector.CrossProduct(delta, baseNormal);

    if (skewVector.Magnitude() > s_cylinderRadiusTolerance)
        {
        BRepOffsetAPI_ThruSections builder(capped, true);

        builder.AddWire(BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Circ(OCBRep::ToGpAx2(DPoint3d::FromZero(), rMatrix), radiusA))));
        builder.AddWire(BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(gp_Circ(OCBRep::ToGpAx2(DPoint3d::From(centerB.x-centerA.x, centerB.y-centerA.y, centerB.z-centerA.z), rMatrix), radiusB))));
        builder.Build();

        if (!builder.IsDone())
            {
            BeAssert(false && "ThruSections failure on skewed cone");
            return ERROR;
            }

        shape = builder.Shape();
        }
    else
        {
        if (baseNormal.DotProduct(delta) < 0.0)
            rMatrix.ScaleColumns(rMatrix, 1.0, -1.0, -1.0);
      
        if (DoubleOps::WithinTolerance(radiusA, radiusB, Precision::Confusion()))
            {
            BRepPrimAPI_MakeCylinder cylinderBuilder(OCBRep::ToGpAx2(DPoint3d::FromZero(), rMatrix), radiusA, delta.Magnitude());

            cylinderBuilder.Build();

            if (!cylinderBuilder.IsDone())
                return ERROR;

            if (capped)
                shape = cylinderBuilder.Solid();
            else
                shape = cylinderBuilder.Face();
            }
        else
            {
            BRepPrimAPI_MakeCone coneBuilder(OCBRep::ToGpAx2(DPoint3d::FromZero(), rMatrix), radiusA, radiusB, delta.Magnitude());

            coneBuilder.Build();

            if (!coneBuilder.IsDone())
                return ERROR;

            if (capped)
                shape = coneBuilder.Solid();
            else
                shape = coneBuilder.Face();
            }
        }

    if (shape.IsNull())
        return ERROR;

    Transform   solidToDgn = Transform::From(centerA);

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }
