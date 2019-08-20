/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
static void wireframe_addVertex(bvector<DPoint3d>& pts, DPoint3dCR pt, bool checkDistance)
    {
    if (checkDistance && !pts.empty())
        {
        if (pt.IsEqual(pts.back(), 1.0e-12))
            return; // Don't duplicate previous point...
        else if (pt.IsEqual(pts.front(), 1.0e-12))
            return; // Don't duplicate first point...
        }

    pts.push_back(pt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectVertices(CurveVectorCR curves, bvector<DPoint3d>& pts, bool checkDistance, CheckStop* stopTester)
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

            WireframeGeomUtil::CollectVertices(*curve->GetChildCurveVectorCP(), pts, checkDistance, stopTester);
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
                    DSegment3dCP segment = curve->GetLineCP();

                    wireframe_addVertex(pts, segment->point[0], checkDistance);
                    wireframe_addVertex(pts, segment->point[1], checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> const* points = curve->GetLineStringCP();

                    for (size_t iPt = 0; iPt < points->size(); ++iPt)
                        wireframe_addVertex(pts, points->at(iPt), checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP();

                    if (ellipse->IsFullEllipse())
                        break;

                    DPoint3d startPt, endPt;

                    ellipse->EvaluateEndPoints(startPt, endPt);

                    wireframe_addVertex(pts, startPt, checkDistance);
                    wireframe_addVertex(pts, endPt, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                    {
                    bvector<DPoint3d> const* points = curve->GetAkimaCurveCP();

                    wireframe_addVertex(pts, points->at(2), checkDistance);
                    wireframe_addVertex(pts, points->at(points->size()-3), checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                default:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();

                    if (!bcurve)
                        break;

                    if (2 == bcurve->params.order)
                        {
                        bvector<DPoint3d> poles;

                        bcurve->GetUnWeightedPoles(poles);

                        for (DPoint3dR pole : poles)
                            wireframe_addVertex(pts, pole, checkDistance);
                        break;
                        }

                    if (bcurve->params.closed)
                        break;

                    DPoint3d pointS, pointE;

                    bcurve->FractionToPoint(pointS, 0.0);
                    bcurve->FractionToPoint(pointE, 1.0);

                    wireframe_addVertex(pts, pointS, checkDistance);
                    wireframe_addVertex(pts, pointE, checkDistance);
                    break;
                    }
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectLateralRulePoints(CurveVectorCR curves, bvector<DPoint3d>& pts, int divisor, int closedDivisor, CheckStop* stopTester)
    {
    switch (curves.HasSingleCurvePrimitive())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP  ellipse = curves.front()->GetArcCP();
            bool          fullArc = ellipse->IsFullEllipse();

            if (fullArc)
                divisor = closedDivisor;

            for (int iRule = 0; iRule < divisor; ++iRule)
                {
                double    fraction = (1.0 / divisor) * iRule;
                DPoint3d  point;

                if (!fullArc && DoubleOps::AlmostEqual(fraction, 0.0))
                    continue;

                ellipse->FractionParameterToPoint(point, fraction);
                pts.push_back(point);
                }
            break;
            }

        default:
            {
            MSBsplineCurveCP bcurve = curves.front()->GetProxyBsplineCurveCP();

            if (!bcurve || 2 == bcurve->params.order)
                break;

            if (bcurve->IsClosed())
                divisor = closedDivisor;

            for (int iRule = 0; iRule < divisor; ++iRule)
                {
                double    fraction = (1.0 / divisor) * iRule;
                DPoint3d  point;

                if (!bcurve->IsClosed() && DoubleOps::AlmostEqual(fraction, 0.0))
                    continue;

                bcurve->FractionToPoint(point, fraction);
                pts.push_back(point);
                }
            break;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::ComputeRuleArc(DEllipse3dR ellipse, DPoint3dCR startPt, DPoint3dCR originPt, double sweepAngle, TransformCR transform, RotMatrixCR axes, RotMatrixCR invAxes, double ruleTolerance)
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

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectLateralEdges(DgnExtrusionDetailCR detail, bvector<DSegment3d>& edges, CheckStop* stopTester)
    {
    bvector<DPoint3d> pts;

    if (WireframeGeomUtil::CollectVertices(*detail.m_baseCurve, pts, true, stopTester))
        return true;

    for (size_t iPt = 0; iPt < pts.size(); ++iPt)
        {
        DSegment3d segment = DSegment3d::FromOriginAndDirection(pts[iPt], detail.m_extrusionVector);

        edges.push_back(segment);
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectLateralEdges(DgnRotationalSweepDetailCR detail, bvector<DEllipse3d>& edges, CheckStop* stopTester)
    {
    bvector<DPoint3d> pts;

    if (WireframeGeomUtil::CollectVertices(*detail.m_baseCurve, pts, true, stopTester))
        return true;

    RotMatrix axes, invAxes, tmpRMatrix;
    Transform transform;

    invAxes.InitFrom1Vector(detail.m_axisOfRotation.direction, 2, true);
    axes.TransposeOf(invAxes);

    tmpRMatrix.InitFromPrincipleAxisRotations(axes, 0.0, 0.0, detail.m_sweepAngle);
    tmpRMatrix.InitProduct(invAxes, tmpRMatrix);
    transform.From(tmpRMatrix, detail.m_axisOfRotation.origin);

    for (size_t iPt = 0; iPt < pts.size(); ++iPt)
        {
        DEllipse3d ellipse;

        if (!WireframeGeomUtil::ComputeRuleArc(ellipse, pts.at(iPt), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes))
            continue;

        edges.push_back(ellipse);
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool WireframeGeomUtil::CollectLateralEdges(DgnRuledSweepDetailCR detail, bvector<DSegment3d>& edges, CheckStop* stopTester)
    {
    for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
        {
        bvector<DPoint3d> rulePts1;
        bvector<DPoint3d> rulePts2;

        if (WireframeGeomUtil::CollectVertices(*detail.m_sectionCurves.at(iProfile), rulePts1, true, stopTester) ||
            WireframeGeomUtil::CollectVertices(*detail.m_sectionCurves.at(iProfile+1), rulePts2, true, stopTester))
            return true;

        if (rulePts1.size() != rulePts2.size())
            {
            if (1 == rulePts2.size())
                {
                // Special case to handle zero scale in both XY...
                rulePts2.insert(rulePts2.end(), rulePts1.size()-1, rulePts2.front());
                }
            else
                {
                rulePts1.clear(); rulePts2.clear();

                // In case of zero scale in only X or Y...we have no choice but to re-collect without excluding any points...
                if (WireframeGeomUtil::CollectVertices(*detail.m_sectionCurves.at(iProfile), rulePts1, false, stopTester) ||
                    WireframeGeomUtil::CollectVertices(*detail.m_sectionCurves.at(iProfile+1), rulePts2, false, stopTester))
                    return true;

                if (rulePts1.size() != rulePts2.size())
                    return true;
                }
            }

        for (size_t iRule = 0; iRule < rulePts1.size(); ++iRule)
            {
            DSegment3d segment;

            segment.Init(rulePts1.at(iRule), rulePts2.at(iRule));
            edges.push_back(segment);
            }
        }

    return false;
    }

#if defined (NOT_NOW_TOPOLOGYID)
// No point doing this now as it's not being used...
//   Also, when we switch to doing locate from depth buffer, PickContext and this code won't be involved so
//   we'll need to provide another method for getting the CurveTopologyId from an edge (ex. SnapGeometryHelper)...
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
#else
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curveVector)
    {
    WireframeGeomUtil::DrawOutline(curveVector, graphic); // Always output as open profile...
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurve(Render::GraphicBuilderR graphic, ICurvePrimitivePtr primitive)
    {
    graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive), false);
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(ISolidPrimitiveCR primitive, Render::GraphicBuilderR graphic, CheckStop* stopTester)
    {
#if defined (NOT_NOW_TOPOLOGYID)
    GeometryStreamEntryIdCP entryId = graphic.GetGeometryStreamEntryId();
#endif

    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return;

            if (Angle::IsFullCircle(detail.m_sweepAngle))
                return;

            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(0.0)));//, CurveTopologyId::FromSweepProfile(0), entryId);
            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(1.0)));//, CurveTopologyId::FromSweepProfile(1), entryId);
            return;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return;

            if (detail.m_radiusA > 0.0)
                {
                DEllipse3d ellipse;

                ellipse.InitFromDGNFields3d(detail.m_centerA, detail.m_vector0, detail.m_vector90, detail.m_radiusA, detail.m_radiusA, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(ellipse));//, CurveTopologyId::FromSweepProfile(0), entryId);
                }

            if (detail.m_radiusB > 0.0)
                {
                DEllipse3d ellipse;
    
                ellipse.InitFromDGNFields3d(detail.m_centerB, detail.m_vector0, detail.m_vector90, detail.m_radiusB, detail.m_radiusB, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(ellipse));//, CurveTopologyId::FromSweepProfile(1), entryId);
                }
            return;
            }

        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail  detail;

            if (!primitive.TryGetDgnBoxDetail(detail))
                return;

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

            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLineString(baseRectangle, 5));//, CurveTopologyId::FromSweepProfile(0), entryId);
            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLineString(topRectangle,  5));//, CurveTopologyId::FromSweepProfile(1), entryId);

            for (uint32_t iRule = 0; iRule < 4; ++iRule)
                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(DSegment3d::From(baseRectangle[iRule], topRectangle[iRule])));//, CurveTopologyId::FromSweepLateral(iRule), entryId);
            return;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return;

            double latitude0, latitude1, z0, z1;

            if (!detail.GetSweepLimits(latitude0, latitude1, z0, z1))
                return; // true sphere...

            double vFraction0 = detail.LatitudeToVFraction(latitude0);
            double vFraction1 = detail.LatitudeToVFraction(latitude1);

            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction0)));//, CurveTopologyId::FromSweepProfile(0), entryId);
            drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction1)));//, CurveTopologyId::FromSweepProfile(1), entryId);
            return;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return;

            drawSolidPrimitiveCurveVector(graphic, *detail.m_baseCurve);//, CurveTopologyId::FromSweepProfile(0), entryId);

            if (stopTester && stopTester->_CheckStop())
                return;

            CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone();

            tmpCurve->TransformInPlace(Transform::From(detail.m_extrusionVector));
            drawSolidPrimitiveCurveVector(graphic, *tmpCurve);//, CurveTopologyId::FromSweepProfile(1), entryId);

            if (stopTester && stopTester->_CheckStop())
                return;

            bvector<DSegment3d> edges;

            if (WireframeGeomUtil::CollectLateralEdges(detail, edges, stopTester))
                return;

            for (uint32_t iEdge = 0; iEdge < edges.size(); ++iEdge)
                {
                if (stopTester && stopTester->_CheckStop())
                    return;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(edges.at(iEdge)));//, CurveTopologyId::FromSweepLateral(iEdge), entryId);
                }
            return;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return;

            if (!Angle::IsFullCircle(detail.m_sweepAngle))
                {
                drawSolidPrimitiveCurveVector(graphic, *detail.m_baseCurve);//, CurveTopologyId::FromSweepProfile(0), entryId);

                if (stopTester && stopTester->_CheckStop())
                    return;

                DPoint3d  axisPoint;
                Transform transform;

                axisPoint.SumOf(detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction);
                transform.InitFromLineAndRotationAngle(detail.m_axisOfRotation.origin, axisPoint, detail.m_sweepAngle);

                CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone();

                tmpCurve->TransformInPlace(transform);
                drawSolidPrimitiveCurveVector(graphic, *tmpCurve);//, CurveTopologyId::FromSweepProfile(1), entryId);

                if (stopTester && stopTester->_CheckStop())
                    return;
                }

            bvector<DEllipse3d> edges;

            if (WireframeGeomUtil::CollectLateralEdges(detail, edges, stopTester))
                return;

            for (uint32_t iEdge = 0; iEdge < edges.size(); ++iEdge)
                {
                if (stopTester && stopTester->_CheckStop())
                    return;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateArc(edges.at(iEdge)));//, CurveTopologyId::FromSweepLateral(iEdge), entryId);
                }
            return;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
    
            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return;

            //uint32_t curveIndex = 0; //unused

            for (CurveVectorPtr curves: detail.m_sectionCurves)
                {
                drawSolidPrimitiveCurveVector(graphic, *curves);//, CurveTopologyId::FromSweepProfile(curveIndex++), entryId);

                if (stopTester && stopTester->_CheckStop())
                    return;
                }

            bvector<DSegment3d> edges;

            if (WireframeGeomUtil::CollectLateralEdges(detail, edges, stopTester))
                return;

            for (size_t iEdge = 0; iEdge < edges.size(); ++iEdge)
                {
                if (stopTester && stopTester->_CheckStop())
                    return;

                drawSolidPrimitiveCurve(graphic, ICurvePrimitive::CreateLine(edges.at(iEdge)));//, CurveTopologyId::FromSweepLateral(iEdge), entryId);
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
void WireframeGeomUtil::Draw(MSBsplineSurfaceCR surface, Render::GraphicBuilderR graphic, CheckStop* stopTester)
    {
    if (surface.uParams.closed && surface.vParams.closed)
        return;

    CurveVectorPtr curves = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

    if (!curves.IsValid())
        return;

    // NOTE: This should be BOUNDARY_TYPE_None with bcurve primitives. Output each curve separately so callers don't have to deal with nesting...
    for (ICurvePrimitivePtr curve : *curves)
        {
        if (!curve.IsValid())
            continue;

        graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(IBRepEntityCR entity, Render::GraphicBuilderR graphic, CheckStop* stopTester)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    PK_ENTITY_t entityTag = PSolidUtil::GetEntityTag(entity);

    if (PK_ENTITY_null == entityTag)
        return;

    bvector<PK_EDGE_t> edgeTags;

    if (SUCCESS != PSolidTopo::GetBodyEdges(edgeTags, entityTag))
        return;

#if defined (NOT_NOW_TOPOLOGYID)
    GeometryStreamEntryIdCP entryId = graphic.GetGeometryStreamEntryId();
#endif
    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();

    for (PK_EDGE_t edgeTag : edgeTags)
        {
        if (stopTester && stopTester->_CheckStop())
            return;

        bool isHiddenEntity = false;

        if (SUCCESS == PSolidAttrib::GetHiddenAttribute(isHiddenEntity, edgeTag) && isHiddenEntity)
            continue;

        ICurvePrimitivePtr curve;

        if (SUCCESS != PSolidGeom::EdgeToCurvePrimitive(curve, edgeTag))
            continue;

        PK_FACE_t faceTag = (attachments ? PSolidUtil::GetPreferredFaceAttachmentFaceForEdge(edgeTag) : PK_ENTITY_null);

        if (PK_ENTITY_null != faceTag)
            {
            T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();
            int32_t attachmentIndex = 0;

            if (SUCCESS != PSolidAttrib::GetFaceMaterialIndexAttribute(attachmentIndex, faceTag) || attachmentIndex < 0 || attachmentIndex >= faceAttachmentsVec.size())
                attachmentIndex = 0; // If face attrib not present, use base symbology...

            FaceAttachment faceAttachment = faceAttachmentsVec.at((size_t) attachmentIndex); 
            Render::GraphicParamsCP graphicParams = faceAttachment.GetGraphicParams();

            if (nullptr != graphicParams)
                graphic.ActivateGraphicParams(*graphicParams, nullptr); // Activate the pre-resolved face symbology...
            }

#if defined (NOT_NOW_TOPOLOGYID)
// No point doing this now as it's not being used...
//   Also, when we switch to doing locate from depth buffer, PickContext and this code won't be involved so
//   we'll need to provide another method for getting the CurveTopologyId from an edge (ex. SnapGeometryHelper)...
        if (nullptr != entryId && entryId->IsValid())
            {
            CurveTopologyId curveTopologyId;

            if (SUCCESS == PSolidTopoId::CurveTopologyIdFromEdge(curveTopologyId, edgeTag, true))
                {
                CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type::ParasolidBody, curveTopologyId);
                curve->SetId(newId.get());
                }
            }
#endif

        curve->TransformInPlace(entity.GetEntityTransform());
        graphic.AddCurveVectorR(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
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
GeometryStreamEntryIdCP m_entryId;
CurveVectorPtr          m_curves;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit RuleCollector(GeometryStreamEntryIdCP entryId)
    {
    m_surface   = nullptr;
    m_primitive = nullptr;
    m_entity    = nullptr;
    m_entryId   = entryId;
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
    auto graphic = context.CreateSceneGraphic();

    if (nullptr != m_entryId && m_entryId->IsValid())
        graphic->SetGeometryStreamEntryId(m_entryId);

    if (m_surface)
        WireframeGeomUtil::Draw(*m_surface, *graphic, &context);
    else if (m_primitive)
        WireframeGeomUtil::Draw(*m_primitive, *graphic, &context);
    else if (m_entity)
        WireframeGeomUtil::Draw(*m_entity, *graphic, &context);

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
CurveVectorPtr WireframeGeomUtil::CollectCurves(ISolidPrimitiveCR primitive, DgnDbR dgnDb, GeometryStreamEntryIdCP entryId)
    {
    RuleCollector rules(entryId);

    rules.SetSolidPrimitive(primitive);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(MSBsplineSurfaceCR surface, DgnDbR dgnDb, GeometryStreamEntryIdCP entryId)
    {
    RuleCollector rules(entryId);

    rules.SetBsplineSurface(surface);
    GeometryProcessor::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(IBRepEntityCR entity, DgnDbR dgnDb, GeometryStreamEntryIdCP entryId)
    {
    RuleCollector rules(entryId);

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

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct StrokeSurfaceCurvesInfo
    {
    CheckStop*                  m_stopTester;
    Render::GraphicBuilderR     m_graphic;
    MSBsplineSurfaceCR          m_surface;

    StrokeSurfaceCurvesInfo(CheckStop* stopTester, Render::GraphicBuilderR graphic, MSBsplineSurfaceCR surface) : m_stopTester(stopTester), m_graphic(graphic), m_surface(surface) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static int wireframe_drawSurfaceCurveCallback(void* userArg, MSBsplineCurveP bcurve, double u0, double u1, double v0, double v1)
    {
    StrokeSurfaceCurvesInfo* info = (StrokeSurfaceCurvesInfo*) userArg;

    if (!((DoubleOps::AlmostEqual(u0, u1) && (DoubleOps::AlmostEqual(u0, 0.0) || DoubleOps::AlmostEqual(u0, 1.0))) ||
          (DoubleOps::AlmostEqual(v0, v1) && (DoubleOps::AlmostEqual(v0, 0.0) || DoubleOps::AlmostEqual(v0, 1.0)))))
        info->m_graphic.AddBSplineCurve(*bcurve, false);

    return (info->m_stopTester && info->m_stopTester->_CheckStop() ? ERROR : SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawUVRules(MSBsplineSurfaceCR surface, Render::GraphicBuilderR graphic, Render::GraphicParamsCR params, CheckStop* stopTester)
    {
    StrokeSurfaceCurvesInfo info(stopTester, graphic, surface);
    Render::GraphicParams uvParams(params);

    uvParams.SetWidth(1);
    uvParams.SetLinePixels(LinePixels::Solid);
    graphic.ActivateGraphicParams(uvParams, nullptr);

    bspproc_surfaceWireframeByCurves(&surface, wireframe_drawSurfaceCurveCallback, &info, false);

    graphic.ActivateGraphicParams(params, nullptr); // Restore params...
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
        double            zDepth = (is3d ? 0.0 : Render::Target::DepthFromDisplayPriority(lsContext.GetGeometryParams().GetNetDisplayPriority()));
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

