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

static BentleyStatus bodyFromTopLevelCurveVector(TopoDS_Shape& shape, CurveVectorCR rawCurves, TransformCR dgnToSolid, bool coverClosed, bool flattenClosed, bool triangulateClosed);
static BentleyStatus bodyFromCurveVector(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid, bool coverClosed);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus bodyFromPolyface(TopoDS_Shape& shape, PolyfaceQueryCR mesh, TransformCR dgnToSolid)
    {
    // NEEDSWORK: This is undoubtedly taking the brute force approach...hopefully Earlin can look into doing this the "right" way...whatever that is...
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(mesh, true);

    visitor->SetNumWrap (1);

    TopoDS_Builder shellBuilder;
    TopoDS_Shell   shell;

    shellBuilder.MakeShell(shell);

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

        shellBuilder.Add(shell, face);
        }

    shape = shell;

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
        Handle(Geom2d_BSplineCurve) boundaryGeomCurve;

        if (nullptr != curvePrimitive->GetProxyBsplineCurveCP())
            {
            boundaryGeomCurve = OCBRep::ToGeom2dBSplineCurve(*curvePrimitive->GetProxyBsplineCurveCP(), nullptr);
            }
        else
            {
            MSBsplineCurvePtr bCurve = curvePrimitive->GetMSBsplineCurvePtr(); // Handle linestring boundary...

            if (!bCurve.IsValid())
                {
                BeAssert(false && "Unexpected BSurf Boundary Primitive");
                return ERROR;
                }

            boundaryGeomCurve = OCBRep::ToGeom2dBSplineCurve(*bCurve, nullptr);
            }

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
    OCBRepUtil::GetOcctKnots(occtUKnots, occtUMultiplicities, uKnots, surface->GetIntUOrder());
    OCBRepUtil::GetOcctKnots(occtVKnots, occtVMultiplicities, vKnots, surface->GetIntVOrder());
    
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

        if (shape.IsNull())
            return ERROR;

        BRepLib::BuildCurves3d(shape);

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

    if (shape.IsNull())
        return ERROR;

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

    if (triangulateClosed && triangulatedBodyFromNonPlanarPolygon(shape, *curves, dgnToSolid))
        return SUCCESS;

    Transform   compositeTransform = dgnToSolid;

    if (flattenClosed && !testClosedPathFlattening(compositeTransform, *curves))
        coverClosed = false;

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

    if (SUCCESS != bodyFromTopLevelCurveVector(shape, curves, dgnToSolid, true, true, true))
        return ERROR;

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromBox(TopoDS_Shape& shape, DgnBoxDetailCR detail)
    {
    DPoint3d    origin;
    DVec3d      localDiagonal;
    RotMatrix   unitAxes;

    if (detail.IsBlock(origin, unitAxes, localDiagonal, 0.0, 0.0, 0.0))
        {
        BRepPrimAPI_MakeBox boxBuilder(OCBRep::ToGpAx2(DPoint3d::FromZero(), unitAxes), localDiagonal.x, localDiagonal.y, localDiagonal.z);

        boxBuilder.Build();

        if (!boxBuilder.IsDone())
            return ERROR;

        if (detail.m_capped)
            {
            shape = boxBuilder.Solid();
            }
        else
            {
            TopoDS_Builder shellBuilder;
            TopoDS_Shell   shell;

            shellBuilder.MakeShell(shell); // Make shell from lateral faces...
            shellBuilder.Add(shell, boxBuilder.BackFace());
            shellBuilder.Add(shell, boxBuilder.FrontFace());
            shellBuilder.Add(shell, boxBuilder.LeftFace());
            shellBuilder.Add(shell, boxBuilder.RightFace());
            shape = shell;
            }

        if (shape.IsNull())
            return ERROR;

        Transform solidToDgn = Transform::From(origin);

        if (!solidToDgn.IsIdentity())
            shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

        return SUCCESS;
        }

    bvector<DPoint3d> corners;

    detail.GetCorners(corners);

    DPoint3d baseRectangle[5];

    baseRectangle[0] = corners[0];
    baseRectangle[1] = corners[1];
    baseRectangle[2] = corners[3];
    baseRectangle[3] = corners[2];
    baseRectangle[4] = corners[0];

    DPoint3d topRectangle[5];

    topRectangle[0] = corners[4];
    topRectangle[1] = corners[5];
    topRectangle[2] = corners[7];
    topRectangle[3] = corners[6];
    topRectangle[4] = corners[4];

    CurveVectorPtr baseCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr topCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

    baseCurve->push_back(ICurvePrimitive::CreateLineString(baseRectangle, 5));
    topCurve->push_back(ICurvePrimitive::CreateLineString(topRectangle, 5));

    DgnRuledSweepDetail ruleDetail(baseCurve, topCurve, detail.m_capped);

    return TopoShapeFromRuledSweep(shape, ruleDetail);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromCone(TopoDS_Shape& shape, DgnConeDetailCR detail)
    {
    bool        capped;
    double      radiusA, radiusB;
    DPoint3d    centerA, centerB;
    RotMatrix   rMatrix;

    if (!detail.IsCircular(centerA, centerB, rMatrix, radiusA, radiusB, capped))
        {
        BeAssert(false && "Error noncircular cone"); // NEEDSWORK: Handle as ellipse ThruSections...
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
                shape = cylinderBuilder.Face(); // Only want lateral face...
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
                shape = coneBuilder.Face(); // Only want lateral face...
            }
        }

    if (shape.IsNull())
        return ERROR;

    Transform solidToDgn = Transform::From(centerA);

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromSphere(TopoDS_Shape& shape, DgnSphereDetailCR detail)
    {
    double      radius;
    DPoint3d    center;
    RotMatrix   axes;

    if (!detail.IsTrueSphere(center, axes, radius))
        {
        BeAssert(false && "Not a complete sphere"); // NEEDSWORK: Handle as rotational sweep...
        return ERROR;
        }

    BRepPrimAPI_MakeSphere sphereBuilder(OCBRep::ToGpAx2(DPoint3d::FromZero(), axes), radius);

    sphereBuilder.Build();

    if (!sphereBuilder.IsDone())
        return ERROR;

    shape = sphereBuilder.Solid(); // // Full sphere is always a solid...
    
    if (shape.IsNull())
        return ERROR;

    Transform solidToDgn = Transform::From(center);

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromTorus(TopoDS_Shape& shape, DgnTorusPipeDetailCR detail)
    {
    DPoint3d    center;
    RotMatrix   rMatrix;
    double      radiusA, radiusB, sweep;

    if (!detail.TryGetFrame(center, rMatrix, radiusA, radiusB, sweep))
        return ERROR;

    BRepPrimAPI_MakeTorus torusBuilder(OCBRep::ToGpAx2(DPoint3d::FromZero(), rMatrix), radiusA, radiusB, sweep);

    torusBuilder.Build();

    if (!torusBuilder.IsDone())
        return ERROR;

    if (detail.HasRealCaps())
        shape = torusBuilder.Solid();
    else
        shape = torusBuilder.Face(); // Only want lateral face...

    if (shape.IsNull())
        return ERROR;

    Transform solidToDgn = Transform::From(center);

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromExtrusion(TopoDS_Shape& shape, DgnExtrusionDetailCR detail)
    {
    DPoint3d    startPt;

    if (!detail.m_baseCurve->GetStartPoint(startPt))
        return ERROR;

    Transform   solidToDgn, dgnToSolid;

    solidToDgn = Transform::From(startPt);
    dgnToSolid.InverseOf(solidToDgn);

    TopoDS_Shape profileShape;

    if (SUCCESS != bodyFromTopLevelCurveVector(profileShape, *detail.m_baseCurve, dgnToSolid, detail.m_capped, detail.m_capped, false))
        return ERROR;

    DVec3d      extrusion = detail.m_extrusionVector;

    dgnToSolid.MultiplyMatrixOnly(extrusion);

    BRepPrimAPI_MakePrism extrusionBuilder(profileShape, OCBRep::ToGpVec(extrusion));

    extrusionBuilder.Build();

    if (!extrusionBuilder.IsDone())
        {
        BeAssert(false && "Failure to create extrusion");
        return ERROR;
        }

    shape = extrusionBuilder;

    if (shape.IsNull())
        return ERROR;

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromRotationalSweep(TopoDS_Shape& shape, DgnRotationalSweepDetailCR detail)
    {
    DPoint3d    origin;
    DVec3d      axis;
    double      sweep;

    if (!detail.TryGetRotationAxis(origin, axis, sweep))
        return ERROR;

    Transform   solidToDgn, dgnToSolid;

    solidToDgn = Transform::From(origin);
    dgnToSolid.InverseOf(solidToDgn);

    TopoDS_Shape profileShape;

    if (SUCCESS != bodyFromTopLevelCurveVector(profileShape, *detail.m_baseCurve, dgnToSolid, detail.m_capped, detail.m_capped, false))
        return ERROR;

    dgnToSolid.MultiplyMatrixOnly(axis);

    if (sweep > msGeomConst_2pi)
        sweep = msGeomConst_2pi;
    else if (sweep < -msGeomConst_2pi)
        sweep = -msGeomConst_2pi;

    BRepPrimAPI_MakeRevol revolutionBuilder(profileShape, OCBRep::ToGpAx1(DPoint3d::FromZero(), axis), sweep);

    revolutionBuilder.Build();

    if (!revolutionBuilder.IsDone())
        {
        BeAssert(false && "Failure to create revolution");
        return ERROR;
        }

    shape = revolutionBuilder;

    if (shape.IsNull())
        return ERROR;

    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getLateralRuledSweepFaces(bvector<TopoDS_Face>& faces, CurveVectorCR profile0, CurveVectorCR profile1, TransformCR dgnToSolid)
    {
    if (profile0.GetBoundaryType() != profile1.GetBoundaryType() || profile0.size() != profile1.size())
        {
        BeAssert(false && "Unexpected error getting lateral faces for ruled sweep");
        return ERROR;
        }

    if (profile0.IsParityRegion() || profile1.IsUnionRegion())
        {
        for (size_t i=0; i<profile0.size(); i++)
            {
            if (nullptr == profile0[i]->GetChildCurveVectorCP() || nullptr == profile1[i]->GetChildCurveVectorCP())
                {
                BeAssert(false && "Unexpected error getting lateral faces for ruled sweep");
                return ERROR;
                }

            if (SUCCESS != getLateralRuledSweepFaces(faces, *profile0[i]->GetChildCurveVectorCP(), *profile1[i]->GetChildCurveVectorCP(), dgnToSolid))
                return ERROR;
            }

        return SUCCESS;
        }

    TopoDS_Shape profileWire0, profileWire1;

    if (SUCCESS != bodyFromTopLevelCurveVector(profileWire0, profile0, dgnToSolid, false, true, false) || 
        SUCCESS != bodyFromTopLevelCurveVector(profileWire1, profile1, dgnToSolid, false, true, false))
        return ERROR;

    for (TopExp_Explorer ex0(profileWire0, TopAbs_EDGE), ex1(profileWire1, TopAbs_EDGE); ex0.More() && ex1.More(); ex0.Next(), ex1.Next())
        {
        TopoDS_Face face = BRepFill::Face(TopoDS::Edge(ex0.Current()), TopoDS::Edge(ex1.Current()));

        if (!face.IsNull())
            faces.push_back (face);
        }

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromRuledSweep(TopoDS_Shape& shape, DgnRuledSweepDetailCR detail)
    {
    DPoint3d    startPt;

    if (!detail.m_sectionCurves.front()->GetStartPoint(startPt))
        return ERROR;

    Transform   solidToDgn, dgnToSolid;

    solidToDgn = Transform::From(startPt);
    dgnToSolid.InverseOf(solidToDgn);

    DVec3d      translation;

    if (2 == detail.m_sectionCurves.size() && detail.GetSectionCurveTranslation(translation, 0, 1))
        {
        TopoDS_Shape profileShape;

        if (SUCCESS != bodyFromTopLevelCurveVector(profileShape, *detail.m_sectionCurves.front(), dgnToSolid, detail.m_capped, detail.m_capped, false))
            return ERROR;

        dgnToSolid.MultiplyMatrixOnly(translation);

        BRepPrimAPI_MakePrism extrusionBuilder(profileShape, OCBRep::ToGpVec(translation));

        extrusionBuilder.Build();

        if (!extrusionBuilder.IsDone())
            {
            BeAssert(false && "Failure to create extrusion");
            return ERROR;
            }

        shape = extrusionBuilder;

        if (shape.IsNull())
            return ERROR;

        if (!solidToDgn.IsIdentity())
            shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

        return SUCCESS;
        }
    else if (!detail.m_sectionCurves.front()->IsParityRegion() && !detail.m_sectionCurves.front()->IsUnionRegion())
        {
        BRepOffsetAPI_ThruSections sectionBuilder(detail.m_capped, true);

        sectionBuilder.CheckCompatibility(false);

        for (auto& profileCurve : detail.m_sectionCurves) 
            {
            TopoDS_Shape profileWire;

            if (SUCCESS != bodyFromTopLevelCurveVector(profileWire, *profileCurve, dgnToSolid, false, true, false))
                return ERROR;
                
            sectionBuilder.AddWire (TopoDS::Wire(profileWire));
            }
        
        sectionBuilder.Build();

        if (!sectionBuilder.IsDone())
            {
            BeAssert(false && "ThruSections failure");
            return ERROR;
            }

        shape = sectionBuilder;

        if (shape.IsNull())
            return ERROR;

        if (!solidToDgn.IsIdentity())
            shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

        return SUCCESS;
        }
    else
        {
        bvector<TopoDS_Face> faces;

        for (size_t i=0; i < detail.m_sectionCurves.size()-1; i++)
            if (SUCCESS != getLateralRuledSweepFaces(faces, *detail.m_sectionCurves[i], *detail.m_sectionCurves[i+1], dgnToSolid))
                return ERROR;

        if (detail.m_capped)
            {
            TopoDS_Shape    cap0, cap1;
            CurveVectorPtr  cap0Curves = detail.m_sectionCurves.front(), cap1Curves = detail.m_sectionCurves.back();

            if (SUCCESS != bodyFromTopLevelCurveVector(cap0, *cap0Curves, dgnToSolid, true, true, false) || 
                SUCCESS != bodyFromTopLevelCurveVector(cap1, *cap1Curves, dgnToSolid, true, true, false))
                return ERROR;

            for (TopExp_Explorer ex(cap0, TopAbs_FACE); ex.More(); ex.Next())
                faces.push_back(TopoDS::Face(ex.Current()));

            for (TopExp_Explorer ex(cap1, TopAbs_FACE); ex.More(); ex.Next())
                faces.push_back(TopoDS::Face(ex.Current()));
            }

        BRepBuilderAPI_Sewing sewingBuilder;

        for (auto& face : faces)
            sewingBuilder.Add(face);

        sewingBuilder.Perform();

        shape = sewingBuilder.SewedShape();

        if (shape.IsNull())
            {
            BeAssert(false && "Error sewing faces in ruled sweep");
            return ERROR;
            }

        if (detail.m_capped && TopAbs_SHELL == shape.ShapeType())
            {
            BRepBuilderAPI_MakeSolid solidBuilder;

            solidBuilder.Add(TopoDS::Shell(shape));
            solidBuilder.Build();

            if (solidBuilder.IsDone())
                shape = solidBuilder.Shape();
            }

        if (!solidToDgn.IsIdentity())
            shape.Location(TopLoc_Location(OCBRep::ToGpTrsf(solidToDgn)));

        return SUCCESS;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRep::Create::TopoShapeFromSolidPrimitive(TopoDS_Shape& shape, ISolidPrimitiveCR primitive)
    {
    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return ERROR;

            return TopoShapeFromTorus(shape, detail);
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return ERROR;

            return TopoShapeFromCone(shape, detail);
            }

        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail detail;

            if (!primitive.TryGetDgnBoxDetail(detail))
                return ERROR;

            return TopoShapeFromBox(shape, detail);
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return ERROR;

            return TopoShapeFromSphere(shape, detail);
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return ERROR;

            return TopoShapeFromExtrusion(shape, detail);
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return ERROR;

            return TopoShapeFromRotationalSweep(shape, detail);
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;

            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return ERROR;

            return TopoShapeFromRuledSweep(shape, detail);
            }

        default:
            {
            BeAssert(false && "Unimplemented solid primitive");
            return ERROR;
            }
        }
    }

