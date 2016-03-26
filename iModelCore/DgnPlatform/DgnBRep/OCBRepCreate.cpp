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
static double s_defaultEqualPointTolerance = 1.0e-10;   // should be "like" PSD resabs
                                                        // BUT ... it seems to be good to make this SMALLER so that 
                                                        //  we call in "move the endpoints" machinery to REALLY close the gaps instead of
                                                        //  just hoping we "really" understand what PSD will close up.
static double s_defaultMaxDirectAdjust     = 1.0e-4;    // gaps this large can be closed by just moving endpoints (i.e. without gap segment).
                                                        // (And this adjustemnt can be away from the curve direction)
static double s_defaultMaxAdjustAlongCurve = 1.0e-3;    // motion along the curve by this much is permitted.

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
static bool triangulatedBodyFromNonPlanarPolygon(TopoDS_Shape& shape, CurveVectorCR curves)
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
//    return PSolidUtil::BodyFromPolyface (bodyTag, builder->GetClientMeshR(), uorToBodyTransform);
    return false;
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
* @bsimethod                                                    Ray.Bentley     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus shapeFromCurveVector(TopoDS_Shape& shape, CurveVectorCR curves, TransformCR dgnToSolid, bool coverClosed)
    {
    if (1 > curves.size())
        return ERROR;

#if defined (NOT_NOW)
    if (curves.IsUnionRegion())
        return shapeFromUnionRegion(shape, curves, dgnToSolid);
    else if (curves.IsParityRegion())
        return shapeFromParityRegion(shape, curves, dgnToSolid);
#endif

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
                curve = ICurvePrimitive::CreateLineString (&curve->GetLineCP()->point[0], 2);

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

                    BRepBuilderAPI_MakeEdge edgeBuilder(OCBRepUtil::ToGpPnt(segment.point[0]), OCBRepUtil::ToGpPnt(segment.point[1]));

                    if (!edgeBuilder.IsDone())
                        {
                        BeAssert (false && "Failure to create edge from segment");
                        return ERROR;
                        }

                    wireBuilder.Add(edgeBuilder);

                    if (!wireBuilder.IsDone())
                        {
                        BeAssert (false && "Failure to add edge to wire");
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
                    gp_Circ gpCirc = OCBRepUtil::ToGpCirc(start, end, ellipse);
                    edgeBuilder = BRepBuilderAPI_MakeEdge(gpCirc, start, end);
                    }
                else
                    {
                    gp_Elips gpEllipse = OCBRepUtil::ToGpElips(start, end, ellipse);
                    edgeBuilder = BRepBuilderAPI_MakeEdge(gpEllipse, start, end);
                    }

                if (!edgeBuilder.IsDone())
                    {
                    BeAssert (false && "Failure to create edge from arc");
                    return ERROR;
                    }

                wireBuilder.Add(edgeBuilder);

                if (!wireBuilder.IsDone())
                    {
                    BeAssert (false && "Failure to add edge to wire");
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
                    BeAssert (false && "Unexpected entry in CurveVector");
                    return ERROR;
                    }

                Handle(Geom_BSplineCurve) geomBCurve = OCBRepUtil::ToGeomBSplineCurve(*bcurve, &dgnToSolid);

                if (geomBCurve.IsNull())
                    {
                    BeAssert (false && "Failure to create edge from bcurve");
                    return ERROR;
                    }

                wireBuilder.Add(BRepBuilderAPI_MakeEdge(geomBCurve));

                if (!wireBuilder.IsDone())
                    {
                    BeAssert (false && "Failure to add edge to wire");
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
            BeAssert (false && "Error creating shape from wire");
            return ERROR;
            }

        shape = faceBuilder;

        return SUCCESS;
        }

    shape = wireBuilder;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus OCBRepUtil::Create::ShapeFromCurveVector(TopoDS_Shape& shape, CurveVectorCR rawCurves)
    {
    if (1 > rawCurves.size())
        return ERROR;

    CurveVectorPtr  curvesNoGaps;
    CurveVectorCP   curves = (resolveCurveVectorGaps(curvesNoGaps, rawCurves) ? curvesNoGaps.get() : &rawCurves);

    if (triangulatedBodyFromNonPlanarPolygon(shape, *curves))
        return SUCCESS;

    DPoint3d    startPt;

    if (!rawCurves.GetStartPoint(startPt))
        return ERROR;

    Transform   solidToDgn = Transform::From(startPt);
    Transform   dgnToSolid;

    dgnToSolid.InverseOf(solidToDgn);

    Transform   compositeTransform = dgnToSolid;
    bool        coverClosed = testClosedPathFlattening(compositeTransform, *curves);       

    if (SUCCESS != shapeFromCurveVector(shape, *curves, compositeTransform, coverClosed))
        return ERROR;
    
    if (!solidToDgn.IsIdentity())
        shape.Location(TopLoc_Location(OCBRepUtil::ToGpTrsf(solidToDgn)));

    return SUCCESS;
    }
