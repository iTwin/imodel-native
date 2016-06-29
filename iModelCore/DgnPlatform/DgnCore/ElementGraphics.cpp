/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGraphics.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>
#if defined (BENTLEYCONFIG_OPENCASCADE)
#include <DgnPlatform/DgnBRep/OCBRep.h>
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static double wireframe_getTolerance(CurveVectorCR curves)
    {
    static double s_minRuleLineTolerance = 1.0e-8;
   
    return curves.ResolveTolerance(s_minRuleLineTolerance); // TFS# 24423 - Length calculation can be very slow for B-Curves. s_defaultLengthRelTol * curves.Length ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void wireframe_addRulePoints(bvector<DPoint3d>& pts, bvector<bool>& interior, DPoint3dCR pt, bool isVertex, double ruleTolerance, bool checkDistance)
    {
    if (checkDistance && 0 != pts.size())
        {
        if (pt.Distance(pts.back()) <= ruleTolerance)
            return; // Don't duplicate previous point...
        else if (pt.Distance(pts.front()) <= ruleTolerance)
            return; // Don't duplicate first point...
        }

    pts.push_back(pt);
    interior.push_back(!isVertex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectPoints(CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, ViewContextP context)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            if (context && context->CheckStop())
                return true;

            wireframe_collectPoints(*curve->GetChildCurveVectorCP(), pts, interior, ruleTolerance, checkDistance, context);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid())
                continue;

            if (context && context->CheckStop())
                return true;

            switch (curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP  segment = curve->GetLineCP();

                    wireframe_addRulePoints(pts, interior, segment->point[0], true, ruleTolerance, checkDistance);
                    wireframe_addRulePoints(pts, interior, segment->point[1], true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> const* points = curve->GetLineStringCP();

                    for (size_t iPt = 0; iPt < points->size(); ++iPt)
                        wireframe_addRulePoints(pts, interior, points->at(iPt), true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP();

                    if (ellipse->IsFullEllipse())
                        {
                        for (size_t iRule = 0; iRule < 4; ++iRule)
                            {
                            double    theta = iRule * Angle::PiOver2();
                            DPoint3d  point;

                            ellipse->Evaluate(point, theta);
                            wireframe_addRulePoints(pts, interior, point, false, ruleTolerance, checkDistance);
                            }
                        break;
                        }

                    double      start, sweep;
                    DPoint3d    startPt, endPt;

                    ellipse->GetSweep(start, sweep);
                    ellipse->EvaluateEndPoints(startPt, endPt);

                    int  interiorPts = (int) (fabs(sweep) / (0.5 * Angle::Pi()));

                    wireframe_addRulePoints(pts, interior, startPt, true, ruleTolerance, checkDistance);

                    if (interiorPts > 0)
                        {
                        int     i;
                        double  theta, delta = sweep / (double) (interiorPts);

                        for (i=0, theta = start + delta; i < interiorPts; i++, theta += delta)
                            {
                            DPoint3d  point;

                            ellipse->Evaluate(point, theta);
                            wireframe_addRulePoints(pts, interior, point, DoubleOps::AlmostEqual(theta, start + sweep), ruleTolerance, checkDistance);
                            }
                        }

                    wireframe_addRulePoints(pts, interior, endPt, true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                    {
                    bvector<DPoint3d> const* points = curve->GetAkimaCurveCP();

                    for (size_t iPt = 2; iPt < points->size()-2; ++iPt)
                        wireframe_addRulePoints(pts, interior, points->at(iPt), 2 == iPt || points->size()-2 == iPt+1, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();
                    int              iPt, numPoints = bcurve->params.numPoles;
                    double           r, delta = 1.0 / (bcurve->params.closed ? bcurve->params.numPoles : bcurve->params.numPoles-1);

                    for (iPt = 0, r = 0.0; iPt < numPoints; r += delta, iPt++)
                        {
                        DPoint3d    point;

                        bcurve->FractionToPoint(point, r);
                        wireframe_addRulePoints(pts, interior, point, bcurve->params.closed ? false : (0 == iPt || numPoints == iPt+1), ruleTolerance, checkDistance);
                        }
                    break;
                    }

                default:
                    break;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectArcMidPoints(CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, ViewContextP context)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            if (context && context->CheckStop())
                return true;

            wireframe_collectArcMidPoints(*curve->GetChildCurveVectorCP(), pts, interior, ruleTolerance, checkDistance, context);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != curve->GetCurvePrimitiveType())
                continue;

            if (context && context->CheckStop())
                return true;

            DPoint3d    point;

            curve->FractionToPoint(0.5, point);
            wireframe_addRulePoints(pts, interior, point, true, ruleTolerance, checkDistance);
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_computeArc(DEllipse3dR ellipse, DPoint3dCR startPt, DPoint3dCR originPt, double sweepAngle, TransformCR transform, RotMatrixCR axes, RotMatrixCR invAxes, double ruleTolerance)
    {
    DPoint3d    endPt, centerPt, tmpPt;

    transform.Multiply(&endPt, &startPt, 1);
    centerPt = originPt;

    tmpPt = startPt;
    axes.Multiply(tmpPt);
    axes.Multiply(centerPt);
    centerPt.z = tmpPt.z;

    DVec3d      xVec, yVec, zVec;
    RotMatrix   rMatrix;

    zVec.Init(0.0, 0.0, 1.0);
    xVec.NormalizedDifference(tmpPt, centerPt);
    yVec.CrossProduct(zVec, xVec);
    rMatrix.InitFromColumnVectors(xVec, yVec, zVec);
    rMatrix.InitProduct(invAxes, rMatrix);
    axes.MultiplyTranspose(centerPt);

    double  radius = centerPt.Distance(startPt);

    if (radius < ruleTolerance)
        return false;

    ellipse.InitFromScaledRotMatrix(centerPt, rMatrix, radius, radius, 0.0, sweepAngle);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct StrokeSurfaceCurvesInfo
    {
    ViewContextR                m_context;
    Render::GraphicBuilderR     m_graphic;
    MSBsplineSurfaceCR          m_surface;
    bool                        m_includeEdges;
    bool                        m_includeFaceIso;

    StrokeSurfaceCurvesInfo(ViewContextR context, Render::GraphicBuilderR graphic, MSBsplineSurfaceCR surface, bool includeEdges, bool includeFaceIso) : m_context(context), m_graphic(graphic), m_surface(surface), m_includeEdges(includeEdges), m_includeFaceIso(includeFaceIso) {}
    };

#define MAX_CLIPBATCH   200

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static int wireframe_drawSurfaceCurveCallback(void* userArg, MSBsplineCurveP bcurve, double u0, double u1, double v0, double v1)
    {
    StrokeSurfaceCurvesInfo* info = (StrokeSurfaceCurvesInfo*) userArg;

    // Special case when full boundary isn't displayed...process as strokes...
    if (info->m_includeEdges && !info->m_surface.holeOrigin && (u0 == u1 && u0 == v0 && u0 == v1))
        {
        BsurfBoundary  *boundP = &info->m_surface.boundaries[abs((int) u0)];
        int            lastEdge, thisEdge, numStrokes;
        DPoint2d       *bP, *endP;
        DPoint3d       strokeBuffer[MAX_CLIPBATCH+1];
        DSegment3d     chord;
 
        numStrokes = 0;
        bP = endP = boundP->points;
        info->m_surface.EvaluatePoint(chord.point[1], bP->x, bP->y);
        thisEdge = bsputil_edgeCode(bP, 0.0);
 
        for (endP += boundP->numPoints, bP++; bP < endP; bP++)
            {
            chord.point[0] = chord.point[1];
            lastEdge = thisEdge;
            info->m_surface.EvaluatePoint(chord.point[1], bP->x, bP->y);
            thisEdge = bsputil_edgeCode(bP, 0.0);
 
            // If both points are on the same edge then do not stroke this segment...
            if (!(lastEdge & thisEdge))
                {
                /* Leave this test in as it supports discontinuity in a B-spline (which arises from the conversion of group holes). */
                if (numStrokes && !LegacyMath::RpntEqual(&chord.point[0], strokeBuffer + numStrokes - 1))
                    {
                    info->m_graphic.AddLineString(numStrokes, strokeBuffer);
                    numStrokes = 0;
                    }
 
                /* If the buffer is empty ... */
                if (!numStrokes)
                    {
                    strokeBuffer[0] = chord.point[0];
                    numStrokes = 1;
                    }
 
                strokeBuffer[numStrokes] = chord.point[1];
                numStrokes += 1;
 
                if (numStrokes >= MAX_CLIPBATCH-1)
                    {
                    info->m_graphic.AddLineString(numStrokes, strokeBuffer);
                    strokeBuffer[0] = strokeBuffer[numStrokes - 1];
                    numStrokes = 1;
                    }
                }
            }
 
        info->m_graphic.AddLineString(numStrokes, strokeBuffer);
 
        return (info->m_context.CheckStop() ? ERROR : SUCCESS);
        }

    bool  showThisRule = true;

    if (!info->m_includeEdges || !info->m_includeFaceIso)
        {
        if ((DoubleOps::AlmostEqual(u0, u1) && (DoubleOps::AlmostEqual(u0, 0.0) || DoubleOps::AlmostEqual(u0, 1.0))) ||
            (DoubleOps::AlmostEqual(v0, v1) && (DoubleOps::AlmostEqual(v0, 0.0) || DoubleOps::AlmostEqual(v0, 1.0))))
            showThisRule = info->m_includeEdges;
        else
            showThisRule = info->m_includeFaceIso;            
        }

    if (showThisRule)
        info->m_graphic.AddBSplineCurve(*bcurve, false);

    return (info->m_context.CheckStop() ? ERROR : SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnExtrusionDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints(*detail.m_baseCurve, pts, interior, wireframe_getTolerance(*detail.m_baseCurve), true, context))
        return true;

    for (size_t iPt = 0; iPt < pts.size(); ++iPt)
        {
        DSegment3d  segment = DSegment3d::FromOriginAndDirection(pts[iPt], detail.m_extrusionVector);

        rules.push_back(segment);
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnRotationalSweepDetailR detail, bvector<DEllipse3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    double ruleTolerance = wireframe_getTolerance(*detail.m_baseCurve);
    bvector<bool> tmpInterior;
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints(*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, context))
        return true;

    RotMatrix   axes, invAxes, tmpRMatrix;
    Transform   transform;

    invAxes.InitFrom1Vector(detail.m_axisOfRotation.direction, 2, true);
    axes.TransposeOf(invAxes);

    tmpRMatrix.InitFromPrincipleAxisRotations(axes, 0.0, 0.0, detail.m_sweepAngle);
    tmpRMatrix.InitProduct(invAxes, tmpRMatrix);
    transform.From(tmpRMatrix, detail.m_axisOfRotation.origin);

    for (size_t iRule = 0; iRule < pts.size(); ++iRule)
        {
        DEllipse3d  ellipse;

        if (!wireframe_computeArc(ellipse, pts.at(iRule), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, ruleTolerance))
            continue;

        rules.push_back(ellipse);
        interior.push_back(tmpInterior.at(iRule));
        }

    if (0 == rules.size()) // TR#115152 - Problem generating rule arc from arc profile with end point on axis of revolution and small sweep...
        {    
        pts.clear();
        tmpInterior.clear();

        if (wireframe_collectArcMidPoints(*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, context))
            return true;

        for (size_t iRule = 0; iRule < pts.size(); ++iRule)
            {
            DEllipse3d  ellipse;

            if (!wireframe_computeArc(ellipse, pts.at(iRule), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, ruleTolerance))
                continue;

            rules.push_back(ellipse);
            interior.push_back(tmpInterior.at(iRule));
            }
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnRuledSweepDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
        {
        bvector<bool> interior1;
        bvector<bool> interior2;
        bvector<DPoint3d> rulePts1;
        bvector<DPoint3d> rulePts2;

        if (wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile), rulePts1, interior1, wireframe_getTolerance(*detail.m_sectionCurves.at(iProfile)), true, context) ||
            wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, interior2, wireframe_getTolerance(*detail.m_sectionCurves.at(iProfile+1)), true, context))
            return true;

        if (rulePts1.size() != rulePts2.size())
            {
            if (1 == rulePts2.size())
                {
                // Special case to handle zero scale in both XY...
                rulePts2.insert(rulePts2.end(), rulePts1.size()-1, rulePts2.front());
                interior2.insert(interior2.end(), interior1.size()-1, interior2.front());
                }
            else
                {
                rulePts1.clear(); rulePts2.clear(); interior1.clear(); interior2.clear();

                // In case of zero scale in only X or Y...we have no choice but to re-collect without excluding any points...
                if (wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile), rulePts1, interior1, 0.0, false, context) ||
                    wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, interior2, 0.0, false, context))
                    return true;

                if (rulePts1.size() != rulePts2.size())
                    return true;
                }
            }

        for (size_t iRule = 0; iRule < rulePts1.size(); ++iRule)
            {
            DSegment3d  segment;

            segment.Init(rulePts1.at(iRule), rulePts2.at(iRule));
            rules.push_back(segment);
            interior.push_back(interior1.at(iRule));
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearCurveVectorIds(CurveVectorCR curveVector)
    {
    for (ICurvePrimitivePtr curve: curveVector)
        {
        curve->SetId(NULL);

        if (curve->GetChildCurveVectorP ().IsValid())
            clearCurveVectorIds(*curve->GetChildCurveVectorP ());
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curveVector, CurveTopologyIdCR topologyId, GeometryStreamEntryIdCR entryId)
    {
    if (!entryId.IsValid())
        {
        WireframeGeomUtil::DrawOutline(curveVector, graphic); // Always output as open profile...
        return;
        }

    clearCurveVectorIds(curveVector);
    CurveTopologyId::AddCurveVectorIds(curveVector, CurvePrimitiveId::Type::SolidPrimitive, topologyId, entryId.GetIndex(), entryId.GetPartIndex());
    WireframeGeomUtil::DrawOutline(curveVector, graphic); // Always output as open profile...
    clearCurveVectorIds(curveVector); // Best not to leave our curve ids on the curve primitives...
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurve(Render::GraphicBuilderR graphic, ICurvePrimitivePtr primitive, CurveTopologyIdCR topologyId, GeometryStreamEntryIdCR entryId)
    {
    if (!entryId.IsValid())
        {
        graphic.AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive), false);
        return;
        }

    CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type::SolidPrimitive, topologyId, entryId.GetIndex(), entryId.GetPartIndex());
    primitive->SetId(newId.get());

    graphic.AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive), false);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(Render::GraphicBuilderR graphic, ISolidPrimitiveCR primitive, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    GeometryStreamEntryId entryId = context.GetGeometryStreamEntryId();

    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail  detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return;

            int     maxURules = 4;
            bool    showCap0 = (Angle::IsFullCircle(detail.m_sweepAngle) ? includeFaceIso : includeEdges);
            bool    showCap1 = (Angle::IsFullCircle(detail.m_sweepAngle) ? false : includeEdges);

            if (showCap0)
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(0.0)), CurveTopologyId::FromSweepProfile(0), entryId);

            if (showCap1)
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(1.0)), CurveTopologyId::FromSweepProfile(1), entryId);

            if (!includeFaceIso)
                return;

            size_t  numVRules = DgnRotationalSweepDetail::ComputeVRuleCount(detail.m_sweepAngle);

            for (size_t vRule = 1; vRule < numVRules; ++vRule)
                {
                double  vFraction = (1.0 / numVRules) * vRule;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction)), CurveTopologyId::FromSweepProfile(vRule + 1), entryId);
                }

            for (int uRule = 0; uRule < maxURules; ++uRule)
                {
                double  uFraction = (1.0 / maxURules) * uRule;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction)), CurveTopologyId::FromSweepLateral(uRule), entryId);
                }
            return;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail  detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return;

            if (detail.m_radiusA > 0.0 && includeEdges)
                {
                DEllipse3d  ellipse;

                ellipse.InitFromDGNFields3d(detail.m_centerA, detail.m_vector0, detail.m_vector90, detail.m_radiusA, detail.m_radiusA, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(ellipse), CurveTopologyId::FromSweepProfile(0), entryId);
                }

            if (detail.m_radiusB > 0.0 && includeEdges)
                {
                DEllipse3d  ellipse;
    
                ellipse.InitFromDGNFields3d(detail.m_centerB, detail.m_vector0, detail.m_vector90, detail.m_radiusB, detail.m_radiusB, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(ellipse), CurveTopologyId::FromSweepProfile(1), entryId);
                }

            if (!includeFaceIso || nullptr == context.GetViewport())
                return; // QVis handles cone silhouette display...viewport will be nullptr in this case...

            Transform   worldToLocalTrans;

            worldToLocalTrans.InverseOf(graphic.GetLocalToWorldTransform());

            DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
            DMatrix4d   viewToWorld = context.GetWorldToView().M1;
            DMatrix4d   viewToLocal;

            viewToLocal.InitProduct(worldToLocal, viewToWorld);

            DSegment3d  silhouettes[2];

            if (!detail.GetSilhouettes(silhouettes[0], silhouettes[1], viewToLocal))
                return; // NOTE: This is expected to fail for TopologyCurveGenerator::CurveByIdCollector, don't allow associations to silhouettes!!!

            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(silhouettes[0]), CurveTopologyId::FromSweepSilhouette(0), entryId);
            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(silhouettes[1]), CurveTopologyId::FromSweepSilhouette(1), entryId);
            return;
            }

        case SolidPrimitiveType_DgnBox:
            {
            if (!includeEdges)
                return; // No face iso to display...

            DgnBoxDetail  detail;

            if (!primitive.TryGetDgnBoxDetail(detail))
                return;

            bvector<DPoint3d> corners;

            detail.GetCorners(corners);

            DPoint3d  baseRectangle[5];

            baseRectangle[0] = corners[0];
            baseRectangle[1] = corners[1];
            baseRectangle[2] = corners[3];
            baseRectangle[3] = corners[2];
            baseRectangle[4] = corners[0];

            DPoint3d  topRectangle[5];

            topRectangle[0] = corners[4];
            topRectangle[1] = corners[5];
            topRectangle[2] = corners[7];
            topRectangle[3] = corners[6];
            topRectangle[4] = corners[4];

            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLineString(baseRectangle, 5), CurveTopologyId::FromSweepProfile(0), entryId);
            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLineString(topRectangle,  5), CurveTopologyId::FromSweepProfile(1), entryId);

            for (uint32_t iRule = 0; iRule < 4; ++iRule)
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(DSegment3d::From(baseRectangle[iRule], topRectangle[iRule])), CurveTopologyId::FromSweepLateral(iRule), entryId);
            return;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail  detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return;

            if (includeFaceIso)
                {
                int     maxURules = 4;
            
                for (int uRule = 0; uRule < maxURules; ++uRule)
                    {
                    double  uFraction = (1.0 / maxURules) * uRule;

                    drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction)), CurveTopologyId::FromSweepLateral(uRule), entryId);
                    }
                }

            if (!includeEdges)
                return;

            // Draw merdian if latitude sweep > 90 and includes 0.0 sweep latitude...
            if (fabs(detail.m_latitudeSweep) < Angle::PiOver2())
                return;

            double  vFraction = detail.LatitudeToVFraction(0.0);

            if (vFraction <= 0.0 || vFraction >= 1.0)
                return;

            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction)), CurveTopologyId::FromSweepProfile(0), entryId);
            return;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail  detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return;

            if (includeEdges)
                {
                drawSolidPrimitiveCurveVector(graphic, *detail.m_baseCurve, CurveTopologyId::FromSweepProfile(0), entryId);

                if (context.CheckStop())
                    return;

                CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone();

                tmpCurve->TransformInPlace(Transform::From(detail.m_extrusionVector));
                drawSolidPrimitiveCurveVector(graphic, *tmpCurve, CurveTopologyId::FromSweepProfile(1), entryId);

                if (context.CheckStop())
                    return;
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (wireframe_collectRules(detail, rules, interior, &context))
                return;

            for (uint32_t iRule = 0; iRule < rules.size(); ++iRule)
                {
                if (!(interior.at(iRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(rules.at(iRule)), CurveTopologyId::FromSweepLateral(iRule), entryId);
                }
            return;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail  detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return;

            bool    showCap0 = (Angle::IsFullCircle(detail.m_sweepAngle) ? includeFaceIso : includeEdges);
            bool    showCap1 = (Angle::IsFullCircle(detail.m_sweepAngle) ? false : includeEdges);

            if (showCap0)
                {
                drawSolidPrimitiveCurveVector(graphic, *detail.m_baseCurve, CurveTopologyId::FromSweepProfile(0), entryId);

                if (context.CheckStop())
                    return;
                }

            if (showCap1)
                {
                DPoint3d    axisPoint;
                Transform   transform;

                axisPoint.SumOf(detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction);
                transform.InitFromLineAndRotationAngle(detail.m_axisOfRotation.origin, axisPoint, detail.m_sweepAngle);

                CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone();

                tmpCurve->TransformInPlace(transform);
                drawSolidPrimitiveCurveVector(graphic, *tmpCurve, CurveTopologyId::FromSweepProfile(1), entryId);

                if (context.CheckStop())
                    return;
                }

            if (includeFaceIso)
                {
                // Draw V rules (if any needed) in addition to end caps...
                size_t  numVRules = detail.GetVRuleCount();

                for (size_t vRule = 1; vRule < numVRules; ++vRule)
                    {
                    double      vFraction = (1.0 / numVRules) * vRule;
                    Transform   transform, derivativeTransform;

                    if (!detail.GetVFractionTransform(vFraction, transform, derivativeTransform))
                        continue;

                    CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone();

                    tmpCurve->TransformInPlace(transform);
                    drawSolidPrimitiveCurveVector(graphic, *tmpCurve, CurveTopologyId::FromSweepProfile(vRule + 1), entryId);

                    if (context.CheckStop())
                        return;
                    }
                }

            // Draw U rule arcs based on profile geometry...
            bvector<bool> interior;
            bvector<DEllipse3d> rules;

            if (wireframe_collectRules(detail, rules, interior, &context))
                return;

            for (uint32_t uRule = 0; uRule < rules.size(); ++uRule)
                {
                if (!(interior.at(uRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(rules.at(uRule)), CurveTopologyId::FromSweepLateral(uRule), entryId);
                }
            return;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
    
            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return;

            if (includeEdges)
                {
                uint32_t curveIndex = 0;

                for (CurveVectorPtr curves: detail.m_sectionCurves)
                    {
                    drawSolidPrimitiveCurveVector(graphic, *curves, CurveTopologyId::FromSweepProfile(curveIndex++), entryId);

                    if (context.CheckStop())
                        return;
                    }
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (wireframe_collectRules(detail, rules, interior, &context))
                return;

            for (size_t uRule = 0; uRule < rules.size(); ++uRule)
                {
                if (!(interior.at(uRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(rules.at(uRule)), CurveTopologyId::FromSweepLateral(uRule), entryId);
                }
            return;
            }

        default:
            return;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(Render::GraphicBuilderR graphic, MSBsplineSurfaceCR surface, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    if (includeEdges)
        {
        CurveVectorPtr  curves = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

        if (curves.IsValid())
            {
            // NOTE: This should be BOUNDARY_TYPE_None with bcurve primitives. Output each curve separately so callers don't have to deal with nesting...
            for (ICurvePrimitivePtr curve : *curves)
                {
                if (!curve.IsValid())
                    continue;

                graphic.AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
                }
            }
        }

    if (includeFaceIso)
        {
        StrokeSurfaceCurvesInfo info(context, graphic, surface, false, true);

        bspproc_surfaceWireframeByCurves(&surface, wireframe_drawSurfaceCurveCallback, &info, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(Render::GraphicBuilderR graphic, ISolidKernelEntityCR entity, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
#if defined (BENTLEYCONFIG_PARASOLIDS)
    T_HOST.GetSolidsKernelAdmin()._OutputBodyAsWireframe(graphic, entity, context, includeEdges, includeFaceIso);
#elif defined (BENTLEYCONFIG_OPENCASCADE)
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);

    if (nullptr == shape)
        return;    
    
    if (includeEdges)
        {
        TopTools_IndexedMapOfShape edgeMap;

        TopExp::MapShapes(*shape, TopAbs_EDGE, edgeMap);

        for (int iEdge=1; iEdge <= edgeMap.Extent(); iEdge++)
            {
            TopoDS_Edge const& edge = TopoDS::Edge(edgeMap(iEdge));
            ICurvePrimitivePtr curve = OCBRep::ToCurvePrimitive(edge);

            if (!curve.IsValid())
                continue;

            graphic.AddCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);

            if (context.CheckStop())
                return;
            }
        }

    if (includeFaceIso)
        {
        Geom2dHatch_Hatcher hatcher = Geom2dHatch_Hatcher(Geom2dHatch_Intersector(1.e-10, 1.e-10), 1.e-8, 1.e-8);

        for (TopExp_Explorer ex(*shape, TopAbs_FACE); ex.More(); ex.Next())
            {
            TopoDS_Face const& face = TopoDS::Face(ex.Current());

            OCBRepUtil::HatchFace(graphic, hatcher, face);

            if (context.CheckStop())
                return;
            }
        }
#endif
    }

BEGIN_UNNAMED_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RuleCollector : IGeometryProcessor
{
protected:

MSBsplineSurfaceCP      m_surface;
ISolidPrimitiveCP       m_primitive;
ISolidKernelEntityCP    m_entity;

bool                    m_includeEdges;
bool                    m_includeFaceIso;

ViewContextP            m_sourceContext;
CurveVectorPtr          m_curves;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit RuleCollector(bool includeEdges, bool includeFaceIso, ViewContextP sourceContext = nullptr)
    {
    m_surface   = nullptr;
    m_primitive = nullptr;
    m_entity    = nullptr;

    m_includeEdges   = includeEdges;
    m_includeFaceIso = includeFaceIso;

    m_sourceContext = sourceContext;
    }

virtual ~RuleCollector() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DrawPurpose _GetProcessPurpose() const override {return (nullptr != m_sourceContext ? m_sourceContext->GetDrawPurpose() : DrawPurpose::CaptureGeometry);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic) override
    {
    if (m_curves.IsNull())
        m_curves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    CurveVectorPtr  childCurve = curves.Clone();

    if (!graphic.GetLocalToWorldTransform().IsIdentity())
        childCurve->TransformInPlace(graphic.GetLocalToWorldTransform());

    m_curves->Add(childCurve);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics(ViewContextR context) override
    {
    if (nullptr != m_sourceContext)
        {
        DgnViewportP vp = m_sourceContext->GetViewport();

        if (nullptr != vp && SUCCESS != context.Attach(vp, context.GetDrawPurpose())) // For locate of cone silhouettes...
            return;

        context.GetGeometryStreamEntryIdR() = m_sourceContext->GetGeometryStreamEntryId(); // For CurvePrimitiveId...
        }

    Render::GraphicBuilderPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport()));

    if (m_surface)
        WireframeGeomUtil::Draw(*graphic, *m_surface, context, m_includeEdges, m_includeFaceIso);
    else if (m_primitive)
        WireframeGeomUtil::Draw(*graphic, *m_primitive, context, m_includeEdges, m_includeFaceIso);
    else if (m_entity)
        WireframeGeomUtil::Draw(*graphic, *m_entity, context, m_includeEdges, m_includeFaceIso);

    graphic->Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SetBsplineSurface(MSBsplineSurfaceCR surface) {m_surface = &surface;}
void SetSolidPrimitive(ISolidPrimitiveCR primitive) {m_primitive = &primitive;}
void SetSolidEntity(ISolidKernelEntityCR entity) {m_entity = &entity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetCurveVector() {return m_curves;}

}; // RuleCollector

END_UNNAMED_NAMESPACE

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(ISolidPrimitiveCR primitive, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso, ViewContextP context)
    {
    RuleCollector   rules(includeEdges, includeFaceIso, context);

    rules.SetSolidPrimitive(primitive);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(MSBsplineSurfaceCR surface, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso, ViewContextP context)
    {
    RuleCollector   rules(includeEdges, includeFaceIso, context);

    rules.SetBsplineSurface(surface);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(ISolidKernelEntityCR entity, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso, ViewContextP context)
    {
    RuleCollector   rules(includeEdges, includeFaceIso, context);

    rules.SetSolidEntity(entity);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

#if defined (BENTLEYCONFIG_PARASOLIDS)
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FaceAttachmentRuleCollector : IGeometryProcessor
{
protected:

ISolidKernelEntityCR                m_entity;
bool                                m_includeEdges;
bool                                m_includeFaceIso;
bmap<FaceAttachment, CurveVectorP>  m_uniqueAttachments;

bvector<CurveVectorPtr>&            m_curves;
bvector<GeometryParams>&            m_params;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit FaceAttachmentRuleCollector(ISolidKernelEntityCR entity, bvector<CurveVectorPtr>& curves, bvector<GeometryParams>& params, bool includeEdges, bool includeFaceIso) : m_entity(entity), m_curves(curves), m_params(params)
    {
    m_includeEdges   = includeEdges;
    m_includeFaceIso = includeFaceIso;
    }

virtual ~FaceAttachmentRuleCollector() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual UnhandledPreference _GetUnhandledPreference(ISolidKernelEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic) override
    {
    bmap<FaceAttachment, CurveVectorP>::iterator found = m_uniqueAttachments.find(graphic.GetCurrentGeometryParams());

    if (found == m_uniqueAttachments.end())
        return true;

    CurveVectorPtr  childCurve = curves.Clone();

    if (!graphic.GetLocalToWorldTransform().IsIdentity())
        childCurve->TransformInPlace(graphic.GetLocalToWorldTransform());

    found->second->Add(childCurve);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics(ViewContextR context) override
    {
    T_FaceAttachmentsVec const& faceAttachmentsVec = m_entity.GetFaceMaterialAttachments()->_GetFaceAttachmentsVec();

    for (FaceAttachment attachment : faceAttachmentsVec)
        {
        CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
        GeometryParams  faceParams;

        attachment.ToGeometryParams(faceParams);
        m_params.push_back(faceParams);
        m_curves.push_back(curve);
        m_uniqueAttachments[attachment] = curve.get();
        }

    Render::GraphicBuilderPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport()));

    WireframeGeomUtil::Draw(*graphic, m_entity, context, m_includeEdges, m_includeFaceIso);
    graphic->Close();
    }

}; // FaceAttachmentRuleCollector

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::CollectCurves(ISolidKernelEntityCR entity, DgnDbR dgnDb, bvector<CurveVectorPtr>& curves, bvector<GeometryParams>& params, bool includeEdges, bool includeFaceIso)
    {
    if (nullptr == entity.GetFaceMaterialAttachments())
        return; // No reason to call this method when there aren't attachments...

    FaceAttachmentRuleCollector rules(entity, curves, params, includeEdges, includeFaceIso);

    GeometryProcessor::Process(rules, dgnDb);
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr WireframeGeomUtil::CollectPolyface(ISolidKernelEntityCR entity, DgnDbR dgnDb, IFacetOptionsR options)
    {
#if defined (BENTLEYCONFIG_PARASOLIDS)
    IFacetTopologyTablePtr facetsPtr;

    if (SUCCESS != DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._FacetBody(facetsPtr, entity, options))
        return nullptr;

    PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*polyface, *facetsPtr, options))
        return nullptr;

    polyface->SetTwoSided(ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());
    polyface->Transform(entity.GetEntityTransform());

    return polyface;
#elif defined (BENTLEYCONFIG_OPENCASCADE)
    TopoDS_Shape const* shape = SolidKernelUtil::GetShape(entity);

    if (nullptr == shape)
        return nullptr;

    return OCBRep::IncrementalMesh(*shape, options);
#else
    return nullptr;
#endif
    }

#if defined (BENTLEYCONFIG_PARASOLIDS)
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::CollectPolyfaces(ISolidKernelEntityCR entity, DgnDbR dgnDb, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<GeometryParams>& params, IFacetOptionsR options)
    {
    if (nullptr == entity.GetFaceMaterialAttachments())
        return; // No reason to call this method when there aren't attachments...

    IFacetTopologyTablePtr facetsPtr;

    if (SUCCESS != DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._FacetBody(facetsPtr, entity, options))
        return;

    T_FaceToSubElemIdMap const& faceToSubElemIdMap = entity.GetFaceMaterialAttachments()->_GetFaceToSubElemIdMap();
    T_FaceAttachmentsVec const& faceAttachmentsVec = entity.GetFaceMaterialAttachments()->_GetFaceAttachmentsVec();
    bmap<int, PolyfaceHeaderCP> faceToPolyfaces;
    bmap<FaceAttachment, PolyfaceHeaderCP> uniqueFaceAttachments;

    for (T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMap.begin(); curr != faceToSubElemIdMap.end(); ++curr)
        {
        FaceAttachment faceAttachment = faceAttachmentsVec.at(curr->second.second);
        bmap<FaceAttachment, PolyfaceHeaderCP>::iterator found = uniqueFaceAttachments.find(faceAttachment);

        if (found == uniqueFaceAttachments.end())
            {
            PolyfaceHeaderPtr polyface = PolyfaceHeader::New();
            GeometryParams faceParams;

            faceAttachment.ToGeometryParams(faceParams);
            params.push_back(faceParams);
            polyfaces.push_back(polyface);
            faceToPolyfaces[curr->first] = uniqueFaceAttachments[faceAttachment] = polyface.get();
            }
        else
            {
            faceToPolyfaces[curr->first] = found->second;
            }
        }

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyfaces(polyfaces, faceToPolyfaces, *facetsPtr, options))
        return;

    for (size_t i=0; i<polyfaces.size(); i++)
        {
        polyfaces[i]->SetTwoSided(ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());
        polyfaces[i]->Transform(entity.GetEntityTransform());
        }
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawOutline(CurveVectorCR curves, GraphicBuilderR graphic)
    {
    if (1 > curves.size())
        return;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            DrawOutline(*curve->GetChildCurveVectorCP(), graphic);
            }
        }
    else if (curves.IsClosedPath())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType();

        const_cast <CurveVectorR> (curves).SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        graphic.AddCurveVector(curves, false);
        const_cast <CurveVectorR> (curves).SetBoundaryType(saveType);
        }
    else
        {
        // Open and none path types ok...
        graphic.AddCurveVector(curves, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawOutline2d(CurveVectorCR curves, GraphicBuilderR graphic, double zDepth)
    {
    if (1 > curves.size())
        return;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            DrawOutline2d(*curve->GetChildCurveVectorCP(), graphic, zDepth);
            }
        }
    else if (curves.IsClosedPath())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType();

        const_cast <CurveVectorR> (curves).SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        graphic.AddCurveVector2d(curves, false, zDepth);
        const_cast <CurveVectorR> (curves).SetBoundaryType(saveType);
        }
    else
        {
        // Open and none path types ok...
        graphic.AddCurveVector2d(curves, false, zDepth);
        }
    }

#if defined (WIP_NEEDSWORK_ELEMENT)
static const double TOLERANCE_ChainMiterCosLimit = .707;

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorStroker : GraphicStroker
{
CurveVectorCR   m_curves;

CurveVectorStroker(CurveVectorCR curves) : m_curves(curves) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AddCurveVector(ElementHandleCR eh, ViewContextR context, bool isFilled)
    {
    // NOTE: Always send outline to QVis as open profiles, avoids expensive QV topological analysis and problems with non-planar geometry...
    if (!isFilled && m_curves.IsAnyRegionType() && context.GetIViewDraw().IsOutputQuickVision())
        {
        if (eh.GetDgnModelP ()->Is3d())
            WireframeGeomUtil::DrawOutline(m_curves, context.GetCurrentGraphicR());
        else
            WireframeGeomUtil::DrawOutline2d(m_curves, context.GetCurrentGraphicR(), context.GetDisplayPriority());
        return;
        }

    if (eh.GetDgnModelP ()->Is3d())
        context.GetCurrentGraphicR().AddCurveVector(m_curves, isFilled);
    else
        context.GetCurrentGraphicR().AddCurveVector2d(m_curves, isFilled, context.GetDisplayPriority());
    }

}; // CurveVectorStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorFillStroker : CurveVectorStroker
{
CurveVectorFillStroker(CurveVectorCR curves) : CurveVectorStroker(curves) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache(CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();

    AddCurveVector(eh, context, true);
    }

}; // CurveVectorFillStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorOutlineStroker : CurveVectorStroker
{
/*----------------------------------------------------------------------------------*//**
* @bsiclass                                                     Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChainTangentInfo
    {
    private:

    DPoint3d    m_point;
    DVec3d      m_tangent;
    bool        m_isValid;

    public:

    ChainTangentInfo() {m_isValid = false;}

    bool        IsValid() {return m_isValid;}
    DPoint3dCR  GetPoint() {return m_point;}
    DVec3dCR    GetTangent() {return m_tangent;}
    void        SetValid(bool yesNo) {m_isValid = yesNo;}
    void        Init(DPoint3dCR point, DVec3dCR tangent) {m_point = point; m_tangent = tangent; m_isValid = true;}

    }; // ChainTangentInfo

CurveVectorOutlineStroker(CurveVectorCR curves) : CurveVectorStroker(curves) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache(CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();

    AddCurveVector(eh, context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetChainTangents(ChainTangentInfo* startInfo, ChainTangentInfo* endInfo, ICurvePrimitiveCR curvePrimitive)
    {
    bool        isPoint = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == curvePrimitive.GetCurvePrimitiveType() && 0.0 == curvePrimitive.GetLineCP()->Length());
    DVec3d      tangents[2]; 
    DPoint3d    points[2];

    if (isPoint || !curvePrimitive.GetStartEnd(points[0], points[1], tangents[0], tangents[1]))
        {
        if (startInfo)
            startInfo->SetValid(false);

        if (endInfo)
            endInfo->SetValid(false);

        return;
        }

    tangents[0].Negate();

    if (startInfo)
        startInfo->Init(points[0], tangents[0]);

    if (endInfo)
        endInfo->Init(points[1], tangents[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyled(ViewContextR context, CurveVectorCR curves, bool is3d, double zDepth)
    {
    if (1 > curves.size())
        return;

    bool              isClosed  = curves.IsClosedPath();
    bool              isComplex = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive());
    ChainTangentInfo  currEnd, prevEnd, nextEnd, currStart, nextStart, chainStart;

    if (isComplex) // Support start/end tangents for linestyle w/thickness...
        {
        GetChainTangents(&currStart, &currEnd, *curves.front());

        if (isClosed)
            GetChainTangents(NULL, &prevEnd, *curves.back());

        chainStart = currStart;
        }

    DPoint3d    startTangent, endTangent;
    DPoint3d    *pStartTangent, *pEndTangent;
    size_t      nCmpns = curves.size();

    for (size_t iCmpn = 0; iCmpn < nCmpns; ++iCmpn)
        {
        ICurvePrimitivePtr curve = curves.at(iCmpn);

        if (!curve.IsValid())
            continue;

        if (isComplex) // Support start/end tangents for linestyle w/thickness...
            {
            ICurvePrimitivePtr nextCurve = iCmpn < nCmpns-1 ? curves.at(iCmpn+1) : NULL;

            if (!nextCurve.IsValid())
                {
                if (isClosed)
                    {
                    nextStart = chainStart;
                    }
                else
                    {
                    nextStart.SetValid(false);
                    nextEnd.SetValid(false);
                    }
                }
            else
                {
                GetChainTangents(&nextStart, &nextEnd, *nextCurve);
                }
            }

        pStartTangent = pEndTangent = NULL;

        if (prevEnd.IsValid() && currStart.IsValid())
            {
            if (currStart.GetTangent().DotProduct(prevEnd.GetTangent()) < TOLERANCE_ChainMiterCosLimit)
                {
                startTangent.DifferenceOf(currStart.GetTangent(), prevEnd.GetTangent());

                if (0.0 != startTangent.Normalize())
                    pStartTangent = &startTangent;
                }
            }

        if (currEnd.IsValid() && nextStart.IsValid())
            {
            if (currEnd.GetTangent().DotProduct(nextStart.GetTangent()) < TOLERANCE_ChainMiterCosLimit)
                {
                endTangent.DifferenceOf(currEnd.GetTangent(), nextStart.GetTangent());

                if (0.0 != endTangent.Normalize())
                    pEndTangent = &endTangent;
                }
            }

        context.SetLinestyleTangents(pStartTangent, pEndTangent); // NOTE: This needs to happen before CookGeometryParams to setup modifiers!

        if (isComplex)
            context.CookGeometryParams(); // Set/Clear linestyle start/end tangent modifiers. (needed for constant width change...)

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3d  segment = *curve->GetLineCP();

                if (is3d)
                    {
                    context.DrawStyledLineString3d(2, segment.point, NULL);
                    break;
                    }

                DPoint2d    points[2];

                points[0].Init(segment.point[0]);
                points[1].Init(segment.point[1]);
                    
                context.DrawStyledLineString2d(2, points, zDepth, NULL);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d> const* points = curve->GetLineStringCP();

                if (is3d)
                    {
                    context.DrawStyledLineString3d((int) points->size(), &points->front(), NULL, !isComplex && isClosed);
                    break;
                    }

                int                      nPts = (int) points->size();
                std::valarray<DPoint2d>  localPoints2dBuf(nPts);

                for (int iPt = 0; iPt < nPts; ++iPt)
                    {
                    DPoint3dCP  tmpPt = &points->front()+iPt;

                    localPoints2dBuf[iPt].x = tmpPt->x;
                    localPoints2dBuf[iPt].y = tmpPt->y;
                    }

                context.DrawStyledLineString2d(nPts, &localPoints2dBuf[0], zDepth, NULL, !isComplex && isClosed);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3d  ellipse = *curve->GetArcCP();

                if (is3d)
                    {
                    context.DrawStyledArc3d(ellipse, !isComplex && isClosed, NULL);
                    break;
                    }

                context.DrawStyledArc2d(ellipse, !isComplex && isClosed, zDepth, NULL);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();
        
                if (is3d)
                    {
                    context.DrawStyledBSplineCurve3d(*bcurve);
                    break;
                    }

                context.DrawStyledBSplineCurve2d(*bcurve, zDepth);
                break;
                }

            default:
                {
                BeAssert(false && "Unexpected entry in CurveVector.");
                break;
                }
            }

        prevEnd   = currEnd;
        currStart = nextStart;
        currEnd   = nextEnd;
        }

    context.SetLinestyleTangents(NULL, NULL); // Make sure we clear linestyle tangents...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyledCurveVector2d(ViewContextR context, CurveVectorCR curve, double zDepth)
    {
    if (NULL == context.GetCurrLineStyle(NULL))
        {
        if (context.GetIViewDraw().IsOutputQuickVision())
            WireframeGeomUtil::DrawOutline2d(curve, context.GetCurrentGraphicR(), zDepth);
        else
            context.GetCurrentGraphicR().AddCurveVector2d(curve, false, zDepth);
            
        return;
        }

    DrawStyled(context, curve, false, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawStyledCurveVector(ElementHandleCR eh, ViewContextR context)
    {
    // Only open/closed paths are valid for linestyle display. (ex. exclude point strings)
    if (CurveVector::BOUNDARY_TYPE_None == m_curves.GetBoundaryType())
        {
        AddCurveVector(eh, context, false);
        return;
        }

    bool    is3d = eh.GetDgnModelP ()->Is3d();

    DrawStyled(context, m_curves, is3d, is3d ? 0.0 : context.GetDisplayPriority());
    }

}; // CurveVectorOutlineStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddCurveVector(ElementHandleCR eh, CurveVectorCR curves, GeomRepresentations info, bool allowCachedOutline)
    {
    if (0 != (info & DISPLAY_INFO_Thickness))
        {
        CurveVectorThicknessStroker  stroker(curves);

        DrawWithThickness(eh, stroker, 2);
        }

    if (0 != (info & (DISPLAY_INFO_Fill | DISPLAY_INFO_Surface)))
        {
        CurveVectorFillStroker  stroker(curves);

        DrawCached(eh, stroker, 1);
        }

    if (0 != (info & DISPLAY_INFO_Edge))
        {
        CurveVectorOutlineStroker  stroker(curves);

        if (allowCachedOutline && NULL == GetCurrLineStyle(NULL))
            {
            if (allowCachedOutline)
                DrawCached(eh, stroker, 0);
            else
                stroker.AddCurveVector(eh, *this, false);
            }
        else
            {
            stroker.DrawStyledCurveVector(eh, *this);
            }
        }

    if (0 != (info & DISPLAY_INFO_Pattern))
        {
        CurveVectorFillStroker           stroker(curves);
        ViewContext::ClipStencil         clipStencil(stroker, 1);
        ViewContext::PatternParamSource  patParamSrc;
        
        DrawAreaPattern(eh, clipStencil, patParamSrc); // NOTE: Changes current matsymb...should do this last...
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawStyledCurveVector2d(CurveVectorCR curve, double zDepth)
    {
#if defined (WIP_NEEDSWORK_ELEMENT)
    CurveVectorOutlineStroker::DrawStyledCurveVector2d(*this, curve, zDepth);
//    GetCurrentGraphicR().AddCurveVector2d(curve, false, zDepth);
#endif
    }
