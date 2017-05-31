/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGraphics.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
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
static bool wireframe_collectPoints(CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, CheckStop* stopTester)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            if (stopTester && stopTester->_CheckStop())
                return true;

            wireframe_collectPoints(*curve->GetChildCurveVectorCP(), pts, interior, ruleTolerance, checkDistance, stopTester);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid())
                continue;

            if (stopTester && stopTester->_CheckStop())
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
static bool wireframe_collectArcMidPoints(CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, CheckStop* stopTester)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            if (stopTester && stopTester->_CheckStop())
                return true;

            wireframe_collectArcMidPoints(*curve->GetChildCurveVectorCP(), pts, interior, ruleTolerance, checkDistance, stopTester);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != curve->GetCurvePrimitiveType())
                continue;

            if (stopTester && stopTester->_CheckStop())
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
    CheckStop*                  m_stopTester;
    Render::GraphicBuilderR     m_graphic;
    MSBsplineSurfaceCR          m_surface;
    bool                        m_includeEdges;
    bool                        m_includeFaceIso;

    StrokeSurfaceCurvesInfo(CheckStop* stopTester, Render::GraphicBuilderR graphic, MSBsplineSurfaceCR surface, bool includeEdges, bool includeFaceIso) : m_stopTester(stopTester), m_graphic(graphic), m_surface(surface), m_includeEdges(includeEdges), m_includeFaceIso(includeFaceIso) {}
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
 
        return (info->m_stopTester && info->m_stopTester->_CheckStop() ? ERROR : SUCCESS);
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

    return (info->m_stopTester && info->m_stopTester->_CheckStop() ? ERROR : SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnExtrusionDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, CheckStop* stopTester)
    {
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints(*detail.m_baseCurve, pts, interior, wireframe_getTolerance(*detail.m_baseCurve), true, stopTester))
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
static bool wireframe_collectRules(DgnRotationalSweepDetailR detail, bvector<DEllipse3d>& rules, bvector<bool>& interior,  CheckStop* stopTester)
    {
    double ruleTolerance = wireframe_getTolerance(*detail.m_baseCurve);
    bvector<bool> tmpInterior;
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints(*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, stopTester))
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

        if (wireframe_collectArcMidPoints(*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, stopTester))
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
static bool wireframe_collectRules(DgnRuledSweepDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, CheckStop* stopTester)
    {
    for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
        {
        bvector<bool> interior1;
        bvector<bool> interior2;
        bvector<DPoint3d> rulePts1;
        bvector<DPoint3d> rulePts2;

        if (wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile), rulePts1, interior1, wireframe_getTolerance(*detail.m_sectionCurves.at(iProfile)), true, stopTester) ||
            wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, interior2, wireframe_getTolerance(*detail.m_sectionCurves.at(iProfile+1)), true, stopTester))
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
                if (wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile), rulePts1, interior1, 0.0, false, stopTester) ||
                    wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, interior2, 0.0, false, stopTester))
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
static void drawSolidPrimitiveCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curveVector, CurveTopologyIdCR topologyId, GeometryStreamEntryIdCP entryId)
    {
    if (nullptr == entryId || !entryId->IsValid())
        {
        WireframeGeomUtil::DrawOutline(curveVector, graphic); // Always output as open profile...
        return;
        }

    clearCurveVectorIds(curveVector);
    CurveTopologyId::AddCurveVectorIds(curveVector, CurvePrimitiveId::Type::SolidPrimitive, topologyId, entryId->GetIndex(), entryId->GetPartIndex());
    WireframeGeomUtil::DrawOutline(curveVector, graphic); // Always output as open profile...
    clearCurveVectorIds(curveVector); // Best not to leave our curve ids on the curve primitives...
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurve(Render::GraphicBuilderR graphic, ICurvePrimitivePtr primitive, CurveTopologyIdCR topologyId, GeometryStreamEntryIdCP entryId)
    {
    if (nullptr == entryId || !entryId->IsValid())
        {
        graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive), false);
        return;
        }

    CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type::SolidPrimitive, topologyId, entryId->GetIndex(), entryId->GetPartIndex());
    primitive->SetId(newId.get());

    graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive), false);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(Render::GraphicBuilderR graphic, ISolidPrimitiveCR primitive, ViewContext* stopTester, bool includeEdges, bool includeFaceIso)
    {
    GeometryStreamEntryIdCP entryId = graphic.GetGeometryStreamEntryId();

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

            DgnViewportCP viewport = nullptr != stopTester ? stopTester->GetViewport() : nullptr;
            if (!includeFaceIso || nullptr == viewport)
                return;

            Transform   worldToLocalTrans;

            worldToLocalTrans.InverseOf(graphic.GetLocalToWorldTransform());

            DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
            DMatrix4d   viewToWorld = viewport->GetWorldToViewMap()->M1;
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

                if (stopTester && stopTester->_CheckStop())
                    return;

                CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone();

                tmpCurve->TransformInPlace(Transform::From(detail.m_extrusionVector));
                drawSolidPrimitiveCurveVector(graphic, *tmpCurve, CurveTopologyId::FromSweepProfile(1), entryId);

                if (stopTester && stopTester->_CheckStop())
                    return;
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (wireframe_collectRules(detail, rules, interior, stopTester))
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

                if (stopTester && stopTester->_CheckStop())
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

                if (stopTester && stopTester->_CheckStop())
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

                    if (stopTester && stopTester->_CheckStop())
                        return;
                    }
                }

            // Draw U rule arcs based on profile geometry...
            bvector<bool> interior;
            bvector<DEllipse3d> rules;

            if (wireframe_collectRules(detail, rules, interior, stopTester))
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

                    if (stopTester && stopTester->_CheckStop())
                        return;
                    }
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (wireframe_collectRules(detail, rules, interior, stopTester))
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
void WireframeGeomUtil::Draw(Render::GraphicBuilderR graphic, MSBsplineSurfaceCR surface, CheckStop* stopTester, bool includeEdges, bool includeFaceIso)
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

                graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
                }
            }
        }

    if (includeFaceIso)
        {
        StrokeSurfaceCurvesInfo info(stopTester, graphic, surface, false, true);

        bspproc_surfaceWireframeByCurves(&surface, wireframe_drawSurfaceCurveCallback, &info, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(Render::GraphicBuilderR graphic, IBRepEntityCR entity, CheckStop* stopTester, bool includeEdges, bool includeFaceIso)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag(entity);

    if (PK_ENTITY_null == entityTag)
        return;

    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();

    if (includeEdges)
        {
        int         nEdges = 0;
        PK_EDGE_t*  edgeTags = nullptr;

        PK_BODY_ask_edges(entityTag, &nEdges, &edgeTags);

        for (int iEdge = 0; iEdge < nEdges; ++iEdge)
            {
            if (stopTester && stopTester->_CheckStop())
                return;

            bool isHiddenEntity = false;

            if (SUCCESS == PSolidAttrib::GetHiddenAttribute(isHiddenEntity, edgeTags[iEdge]) && isHiddenEntity)
                continue;

            ICurvePrimitivePtr curve;

            if (SUCCESS != PSolidGeom::EdgeToCurvePrimitive(curve, edgeTags[iEdge]))
                break;

            PK_FACE_t faceTag = (attachments ? PSolidUtil::GetPreferredFaceAttachmentFaceForEdge(edgeTags[iEdge]) : PK_ENTITY_null);

            if (PK_ENTITY_null != faceTag)
                {
                T_FaceToSubElemIdMap const& faceToSubElemIdMap = attachments->_GetFaceToSubElemIdMap();
                T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();
                T_FaceToSubElemIdMap::const_iterator found = faceToSubElemIdMap.find(faceTag);

                if (found == faceToSubElemIdMap.end())
                    {
                    BeAssert(false); // ERROR - Face not represented in map...
                    }
                else
                    {
                    FaceAttachment faceAttachment = faceAttachmentsVec.at(found->second.second);
                    Render::GraphicParamsCP graphicParams = faceAttachment.GetGraphicParams();

                    if (nullptr != graphicParams)
                        graphic.ActivateGraphicParams(*graphicParams, nullptr); // Activate the pre-resolved face symbology...
                    }
                }

            curve->TransformInPlace(entity.GetEntityTransform());
            graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
            }

        PK_MEMORY_free(edgeTags);
        }

    if (includeFaceIso)
        {
        if (stopTester && stopTester->_CheckStop())
            return;

        // NEEDSWORK...Do something with face hatch lines???
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
IBRepEntityCP           m_entity;

bool                    m_includeEdges;
bool                    m_includeFaceIso;
GeometryStreamEntryIdCP m_entryId;

CurveVectorPtr          m_curves;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit RuleCollector(bool includeEdges, bool includeFaceIso, GeometryStreamEntryIdCP entryId)
    {
    m_surface   = nullptr;
    m_primitive = nullptr;
    m_entity    = nullptr;

    m_includeEdges   = includeEdges;
    m_includeFaceIso = includeFaceIso;
    m_entryId        = entryId;
    }

virtual ~RuleCollector() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurvePrimitive(ICurvePrimitiveCR curve, bool closed, bool filled, SimplifyGraphic& graphic) override
    {
    if (m_curves.IsNull())
        m_curves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    ICurvePrimitivePtr childCurve = curve.Clone();

    if (!graphic.GetLocalToWorldTransform().IsIdentity())
        childCurve->TransformInPlace(graphic.GetLocalToWorldTransform());

    m_curves->push_back(childCurve);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic) override
    {
    graphic.ProcessAsCurvePrimitives(curves, isFilled);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void _OutputGraphics(ViewContextR context) override
    {
    Render::GraphicBuilderPtr graphic = context.CreateGraphic(GraphicBuilder::CreateParams(context.GetDgnDb()));

    if (nullptr != m_entryId && m_entryId->IsValid())
        graphic->SetGeometryStreamEntryId(m_entryId);

    if (m_surface)
        WireframeGeomUtil::Draw(*graphic, *m_surface, &context, m_includeEdges, m_includeFaceIso);
    else if (m_primitive)
        WireframeGeomUtil::Draw(*graphic, *m_primitive, &context, m_includeEdges, m_includeFaceIso);
    else if (m_entity)
        WireframeGeomUtil::Draw(*graphic, *m_entity, &context, m_includeEdges, m_includeFaceIso);

    graphic->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SetBsplineSurface(MSBsplineSurfaceCR surface) {m_surface = &surface;}
void SetSolidPrimitive(ISolidPrimitiveCR primitive) {m_primitive = &primitive;}
void SetSolidEntity(IBRepEntityCR entity) {m_entity = &entity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetCurveVector() {return m_curves;}

}; // RuleCollector

END_UNNAMED_NAMESPACE

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(ISolidPrimitiveCR primitive, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso, GeometryStreamEntryIdCP entryId)
    {
    RuleCollector   rules(includeEdges, includeFaceIso, entryId);

    rules.SetSolidPrimitive(primitive);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(MSBsplineSurfaceCR surface, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso, GeometryStreamEntryIdCP entryId)
    {
    RuleCollector   rules(includeEdges, includeFaceIso, entryId);

    rules.SetBsplineSurface(surface);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(IBRepEntityCR entity, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso, GeometryStreamEntryIdCP entryId)
    {
    RuleCollector   rules(includeEdges, includeFaceIso, entryId);

    rules.SetSolidEntity(entity);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

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

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawControlPolygon(MSBsplineSurfaceCR surface, Render::GraphicBuilderR graphic, Render::GraphicParamsCR params)
    {
    bvector<DPoint3d> poles;

    surface.GetUnWeightedPoles(poles);

    if (0 == poles.size())
        return;

    Render::GraphicParams poleParams(params);

    poleParams.SetWidth(params.GetWidth()+5);
    graphic.ActivateGraphicParams(poleParams, nullptr);

    graphic.AddPointString((int) poles.size(), &poles.front());

    poleParams.SetWidth(1);
    poleParams.SetLinePixels(LinePixels::Code2);
    graphic.ActivateGraphicParams(poleParams, nullptr);

    size_t uNumPoles = surface.GetNumUPoles();
    size_t vNumPoles = surface.GetNumVPoles();

    bvector<DPoint3d> uPoles;
    bvector<DPoint3d> vPoles;

    uPoles.resize(surface.uParams.closed ? uNumPoles+1 : uNumPoles);
    vPoles.resize(surface.vParams.closed ? vNumPoles+1 : vNumPoles);

    for (size_t i=0; i < vNumPoles; i++)
        {
        memcpy(&uPoles[0], &poles.at(uNumPoles * i), uNumPoles * sizeof(DPoint3d));

        if (surface.uParams.closed)
            uPoles[uNumPoles] = uPoles[0];

        graphic.AddLineString((int) uPoles.size(), &uPoles.front());
        }

    for (size_t i=0; i < uNumPoles; i++)
        {
        for (size_t j=0; j < vNumPoles; j++)
            vPoles[j] = poles.at(i + j * uNumPoles);

        if (surface.vParams.closed)
            vPoles[vNumPoles] = vPoles[0];

        graphic.AddLineString((int) vPoles.size(), &vPoles.front());
        }

    graphic.ActivateGraphicParams(params, nullptr); // Restore params...
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void computeInterpolationCurveTangentPoints(DPoint3dR startTangentPt, DPoint3dR endTangentPt, MSInterpolationCurveCR curve)
    {
    startTangentPt.SumOf(curve.fitPoints[0], curve.startTangent, curve.fitPoints[0].Distance(curve.fitPoints[1]) * 0.5);
    endTangentPt.SumOf(curve.fitPoints[curve.params.numPoints-1], curve.endTangent, curve.fitPoints[curve.params.numPoints-1].Distance(curve.fitPoints[curve.params.numPoints-2]) * 0.5);
    }
    
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawControlPolygon(ICurvePrimitiveCR curve, Render::GraphicBuilderR graphic, Render::GraphicParamsCR params, bool is3d, double zDepth)
    {
    MSInterpolationCurveCP fitCurve = curve.GetInterpolationCurveCP();

    if (nullptr != fitCurve)
        {
        Render::GraphicParams poleParams(params);

        poleParams.SetWidth(params.GetWidth()+5);
        graphic.ActivateGraphicParams(poleParams, nullptr);

        if (!is3d)
            {
            bvector<DPoint3d> points;

            for (int32_t i=0; i < fitCurve->params.numPoints; i++)
                {
                DPoint3d pt = fitCurve->fitPoints[i];

                pt.z = zDepth;
                points.push_back(pt);
                }

            graphic.AddPointString((int) points.size(), &points.front());
            }
        else
            {
            graphic.AddPointString(fitCurve->params.numPoints, fitCurve->fitPoints);
            }

        if (!fitCurve->params.isPeriodic)
            {
            DPoint3d tangentPoints[4];

            // Compute interpolation curve tangent points...
            computeInterpolationCurveTangentPoints(tangentPoints[0], tangentPoints[2], *fitCurve);

            if (!is3d)
                {
                tangentPoints[0].z = zDepth;
                tangentPoints[2].z = zDepth;
                }

            // Display fat dots for start/end tangent points...
            graphic.AddPointString(1, &tangentPoints[0]);
            graphic.AddPointString(1, &tangentPoints[2]);

            // Display dotted style start/end tangent lines...
            poleParams.SetWidth(1);
            poleParams.SetLinePixels(LinePixels::Code2);
            graphic.ActivateGraphicParams(poleParams, nullptr);

            tangentPoints[1] = fitCurve->fitPoints[0];
            tangentPoints[3] = fitCurve->fitPoints[fitCurve->params.numPoints-1];

            if (!is3d)
                {
                tangentPoints[1].z = zDepth;
                tangentPoints[3].z = zDepth;
                }

            graphic.AddLineString(2, &tangentPoints[0]);
            graphic.AddLineString(2, &tangentPoints[2]);
            }

        graphic.ActivateGraphicParams(params, nullptr); // Restore params...
        return;
        }

    MSBsplineCurveCP bcurve = curve.GetProxyBsplineCurveCP();

    if (nullptr == bcurve || bcurve->GetIntOrder() < 1 || bcurve->GetIntNumPoles() < 1)
        return;

    bvector<DPoint3d> poles;

    bcurve->GetUnWeightedPoles(poles);

    if (0 == poles.size())
        return;

    if (!is3d)
        {
        for (DPoint3dR pt : poles)
            pt.z = zDepth;
        }

    Render::GraphicParams poleParams(params);

    poleParams.SetWidth(params.GetWidth()+5);
    graphic.ActivateGraphicParams(poleParams, nullptr);

    graphic.AddPointString((int) poles.size(), &poles.front());
    
    poleParams.SetWidth(1);
    poleParams.SetLinePixels(LinePixels::Code2);
    graphic.ActivateGraphicParams(poleParams, nullptr);

    if (bcurve->params.closed)
        poles.push_back(poles.front());

    graphic.AddLineString((int) poles.size(), &poles.front());

    graphic.ActivateGraphicParams(params, nullptr); // Restore params...
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorOutlineStroker
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
static void DrawStyled(CurveVectorCR curves, LineStyleContext& lsContext, ILineStyleCR currLStyle, LineStyleSymbCR lsSymbIn)
    {
    if (1 > curves.size())
        return;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                {
                BeAssert (false && "Unexpected entry in union/parity region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region for a union region)...
                }

            DrawStyled(*curve->GetChildCurveVectorCP(), lsContext, currLStyle, lsSymbIn);
            }
        }
    else
        {
        bool              isClosed  = curves.IsClosedPath();
        bool              isComplex = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive());
        bool              is3d = lsContext.GetViewContext().Is3dView();
        double            zDepth = (is3d ? 0.0 : lsContext.GetGeometryParams().GetNetDisplayPriority());
        ChainTangentInfo  currEnd, prevEnd, nextEnd, currStart, nextStart, chainStart;
        LineStyleSymb     lsSymb = lsSymbIn;
        bool              treatAsSingleSegment = lsSymb.IsTreatAsSingleSegment(); // Save initial value as this gets changed by arc/bcurve strokers...

        if (isComplex) // Support start/end tangents for linestyle w/thickness...
            {
            GetChainTangents(&currStart, &currEnd, *curves.front());

            if (isClosed)
                GetChainTangents(NULL, &prevEnd, *curves.back());

            chainStart = currStart;
            }

        static double chainMiterCosLimit = 0.707;
        DVec3d startTangent, endTangent;
        DVec3d *pStartTangent, *pEndTangent;
        size_t nCmpns = curves.size();

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
                if (currStart.GetTangent().DotProduct(prevEnd.GetTangent()) < chainMiterCosLimit)
                    {
                    startTangent.DifferenceOf(currStart.GetTangent(), prevEnd.GetTangent());

                    if (0.0 != startTangent.Normalize())
                        pStartTangent = &startTangent;
                    }
                }

            if (currEnd.IsValid() && nextStart.IsValid())
                {
                if (currEnd.GetTangent().DotProduct(nextStart.GetTangent()) < chainMiterCosLimit)
                    {
                    endTangent.DifferenceOf(currEnd.GetTangent(), nextStart.GetTangent());

                    if (0.0 != endTangent.Normalize())
                        pEndTangent = &endTangent;
                    }
                }

            lsSymb.SetTreatAsSingleSegment(treatAsSingleSegment); // Restore initial value in case arc/bcurve strokers changed it...
            lsSymb.SetTangents(pStartTangent, pEndTangent);
            lsSymb.CheckContinuationData();

            switch (curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3d segment = *curve->GetLineCP();

                    if (!is3d)
                        segment.point[0].z = segment.point[1].z = zDepth;

                    currLStyle._GetComponent()->_StrokeLineString(lsContext, lsSymb, segment.point, 2, false);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    if (!is3d)
                        {
                        bvector<DPoint3d> points = *curve->GetLineStringCP();

                        for (DPoint3dR pt : points)
                            pt.z = zDepth;

                        currLStyle._GetComponent()->_StrokeLineString(lsContext, lsSymb, &points.front(), (int) points.size(), !isComplex && isClosed);
                        break;
                        }

                    currLStyle._GetComponent()->_StrokeLineString(lsContext, lsSymb, &curve->GetLineStringCP()->front(), (int) curve->GetLineStringCP()->size(), !isComplex && isClosed);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    currLStyle._GetComponent()->_StrokeArc(lsContext, lsSymb, *curve->GetArcCP(), is3d, zDepth, !isComplex && isClosed);
                    break;
                    }

                default:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();

                    if (nullptr != bcurve && bcurve->GetIntOrder() > 0 && bcurve->GetIntNumPoles() > 0)
                        currLStyle._GetComponent()->_StrokeBSplineCurve(lsContext, lsSymb, *bcurve, is3d, zDepth);
                    else
                        BeAssert(false && "Unexpected entry in CurveVector.");
                    break;
                    }
                }

            prevEnd   = currEnd;
            currStart = nextStart;
            currEnd   = nextEnd;
            }
        }
    }

}; // CurveVectorOutlineStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_WantLineStyles()
    {
    return GetViewFlags().ShowStyles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool useLineStyleStroker(Render::GraphicBuilderR builder, LineStyleSymbCR lsSymb, IFacetOptionsPtr& facetOptions)
    {
    return lsSymb.GetUseStroker() && builder.WantStrokeLineStyle(lsSymb, facetOptions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curve, Render::GeometryParamsR params, bool doCook)
    {
    // NOTE: It is left up to the caller if they want to call WantLineStyles, some decorator might want to display a style regardless of the ViewFlags.
    if (doCook)
        CookGeometryParams(params, graphic);

    LineStyleInfoCP lsInfo = params.GetLineStyle();

    // Only open/closed paths are valid for linestyle display. (ex. exclude point strings)
    if (nullptr != lsInfo && CurveVector::BOUNDARY_TYPE_None != curve.GetBoundaryType())
        {
        IFacetOptionsPtr facetOptions;
        LineStyleSymbCR  lsSymb = lsInfo->GetLineStyleSymb();
        ILineStyleCP     currLStyle = (useLineStyleStroker(graphic, lsSymb, facetOptions) ? lsSymb.GetILineStyle() : nullptr);

        if (nullptr != currLStyle)
            {
            LineStyleContext lsContext(graphic, params, *this, facetOptions.get());

            CurveVectorOutlineStroker::DrawStyled(curve, lsContext, *currLStyle, lsSymb);

            if (!curve.IsAnyRegionType() || FillDisplay::Never == params.GetFillDisplay())
                return;

            Render::GeometryParams fillParams(params);

            if (nullptr == fillParams.GetGradient())
                {
                fillParams.SetFillDisplay(FillDisplay::Blanking);
                }
            else if (0 != (fillParams.GetGradient()->GetFlags() & GradientSymb::Flags::Outline))
                {
                GradientSymbPtr gradient = GradientSymb::Create();
                
                gradient->CopyFrom(*fillParams.GetGradient());
                gradient->SetFlags((GradientSymb::Flags) (((Byte) gradient->GetFlags()) & ~GradientSymb::Flags::Outline));

                fillParams.SetGradient(gradient.get());
                }

            CookGeometryParams(fillParams, graphic); // Activate fill with auto-outline disabled...

            if (Is3dView())
                graphic.AddCurveVector(curve, true);
            else
                graphic.AddCurveVector2d(curve, true, params.GetNetDisplayPriority());

            CookGeometryParams(params, graphic); // Restore original...
            return;
            }
        }

    if (Is3dView())
        graphic.AddCurveVector(curve, curve.IsAnyRegionType() && FillDisplay::Never != params.GetFillDisplay());
    else
        graphic.AddCurveVector2d(curve, curve.IsAnyRegionType() && FillDisplay::Never != params.GetFillDisplay(), params.GetNetDisplayPriority());
    }

