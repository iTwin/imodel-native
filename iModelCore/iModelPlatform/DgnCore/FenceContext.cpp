/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsP FenceGeometryProcessor::_GetFacetOptionsP ()
    {
    return (m_facetOptions.IsValid () ? m_facetOptions.get () : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceGeometryProcessor::SetWorldClipPlanes(ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive : clip)
        {
        if (primitive->IsMask())
            continue; // NEEDSWORK: Don't need to support this right now...doesn't seem to be in the form that the classify methods expect...

        ClipPlaneSetCP clipPlanes = primitive->GetClipPlanes();

        if (nullptr == clipPlanes)
            continue;

        for (ConvexClipPlaneSetCR convexSet : *clipPlanes)
            m_worldClipPlanes.push_back(convexSet);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceGeometryProcessor::UpdateLocalClipPlanes(TransformCR localToWorld)
    {
    if (!m_localClipPlanes.empty() || !m_localClipMasks.empty())
        {
        if (m_localToWorld.IsEqual(localToWorld))
            return; // local planes/masks still valid...

        m_localClipPlanes.clear();
        m_localClipMasks.clear();
        }

    for (ConvexClipPlaneSetCR convexSet : m_worldClipPlanes)
        m_localClipPlanes.push_back(convexSet);

    for (ConvexClipPlaneSetCR convexSet : m_worldClipMasks)
        m_localClipMasks.push_back(convexSet);

    Transform worldToLocal;

    worldToLocal.InverseOf(localToWorld);
    m_localToWorld = localToWorld;

    m_localClipPlanes.TransformInPlace(worldToLocal);
    m_localClipMasks.TransformInPlace(worldToLocal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment FenceGeometryProcessor::CheckRange (DRange3dCR localRange, TransformCR localToWorld)
    {
    if (m_worldClipMasks.empty())
        {
        TransformedDRange3dPtr tRange = TransformedDRange3d::Create(localRange, localToWorld);
        InOutStates state = tRange->Classify(m_worldClipPlanes);

        if (state.IsAllInside())
            return ClipPlaneContainment_StronglyInside;
        else if (state.IsAllOutside())
            return ClipPlaneContainment_StronglyOutside;

        return ClipPlaneContainment_Ambiguous;
        }

    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*facetOptions);
    DgnBoxDetail box = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::FromInterpolate(localRange.low, 0.5, localRange.high), DPoint3d::From(localRange.XLength(), localRange.YLength(), localRange.ZLength()), true);

    builder->Add(box);

    UpdateLocalClipPlanes(localToWorld);
    return ClipPlaneSet::ClassifyPolyfaceInSetDifference(builder->GetClientMeshR(), m_localClipPlanes, m_localClipMasks.empty() ? nullptr : &m_localClipMasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment FenceGeometryProcessor::OnNewGeometrySource (GeometrySourceCR source)
    {
    InitCurrentAccept();

    GeometrySource3dCP source3d = source.GetAsGeometrySource3d();
    DRange3d localRange = (nullptr != source3d ? source3d->GetPlacement().GetElementBox() : ElementAlignedBox3d(source.GetAsGeometrySource2d()->GetPlacement().GetElementBox()));
    Transform localToWorld = source.GetPlacementTransform();

    return CheckRange(localRange, localToWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::ProcessPoints(DPoint3dCP points, int numPoints, SimplifyGraphic& graphic)
    {
    UpdateLocalClipPlanes(graphic.GetLocalToWorldTransform());

    for (int iPoint = 0; iPoint < numPoints; iPoint++)
        {
        UpdateCurrentAccept(m_localClipPlanes.IsPointInside(points[iPoint]) ? ClipPlaneContainment_StronglyInside : ClipPlaneContainment_StronglyOutside);

        if (CheckStop())
            break;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::_ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled, SimplifyGraphic& graphic)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == primitive.GetCurvePrimitiveType())
        {
        bvector<DPoint3d> const* points = primitive.GetPointStringCP();

        ProcessPoints(&points->front(), (int) points->size(), graphic);

        return true;
        }

    return (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != primitive.GetCurvePrimitiveType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::_ProcessCurveVector (CurveVectorCR curves, bool filled, SimplifyGraphic& graphic)
    {
    if (CheckStop())
        return true;

    UpdateLocalClipPlanes(graphic.GetLocalToWorldTransform());
    UpdateCurrentAccept(ClipPlaneSet::ClassifyCurveVectorInSetDifference(curves, m_localClipPlanes, m_localClipMasks.empty() ? nullptr : &m_localClipMasks, !IsInsideMode()));

    if (!CheckStop())
        graphic.ProcessAsCurvePrimitives(curves, filled); // Check for point strings...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::_ProcessSolidPrimitive (ISolidPrimitiveCR primitive, SimplifyGraphic& graphic)
    {
    if (CheckStop())
        return true;

    return false; // Get containment by creating Polyface...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::_ProcessSurface (MSBsplineSurfaceCR surface, SimplifyGraphic& graphic)
    {
    if (CheckStop())
        return true;

    return false; // Get containment by creating Polyface...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::_ProcessPolyface (PolyfaceQueryCR polyface, bool filled, SimplifyGraphic& graphic)
    {
    if (CheckStop())
        return true;

    UpdateLocalClipPlanes(graphic.GetLocalToWorldTransform());
    UpdateCurrentAccept(ClipPlaneSet::ClassifyPolyfaceInSetDifference(polyface, m_localClipPlanes, m_localClipMasks.empty() ? nullptr : &m_localClipMasks));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::_ProcessBody (IBRepEntityCR entity, SimplifyGraphic& graphic)
    {
    if (CheckStop())
        return true;

    DRange3d range = entity.GetEntityRange(); // Worth checking range to potentially skip needing Polyface...
    ClipPlaneContainment status = CheckRange(range, graphic.GetLocalToWorldTransform());

    if (ClipPlaneContainment_Ambiguous != status)
        {
        UpdateCurrentAccept(status);
        return true;
        }

    return false; // Get containment by creating Polyface...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceGeometryProcessor::InitCurrentAccept()
    {
    m_hasOverlaps   = false;
    m_firstAccept   = true;
    m_currentAccept = false;
    m_accept        = false;
    m_earlyDecision = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceGeometryProcessor::UpdateCurrentAccept(ClipPlaneContainment status)
    {
    switch (status)
        {
        case ClipPlaneContainment_StronglyInside:
            m_currentAccept = true;
            break;

        case ClipPlaneContainment_Ambiguous:
            m_currentAccept = !IsInsideMode();
            m_hasOverlaps = true;
            break;

        case ClipPlaneContainment_StronglyOutside:
            m_currentAccept = false;
            break;
        }

    CheckCurrentAccept();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceGeometryProcessor::CheckCurrentAccept()
    {
    if (m_earlyDecision) // accept status has already been determined...
        return;

    bool insideMode = IsInsideMode();
    bool hasOverlap = !m_firstAccept && (m_currentAccept != m_accept);

    if (m_currentAccept)
        {
        m_accept = true;
        }
    else if (insideMode)
        {
        m_accept = false;
        m_earlyDecision = true;
        }

    if (!insideMode && m_accept)
        {
        // Fence can overlap an element that has disjoint geometric primitives w/o any single primitive having an overlap...
        if (hasOverlap)
            m_hasOverlaps = true;

        m_earlyDecision = HasOverlaps();
        }

    m_firstAccept = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FenceGeometryProcessor::CheckStop()
    {
    return m_earlyDecision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FenceContext::DoClassify(BeJsValue out, BeJsConst in, DgnDbR db, ICancellablePtr cancel)
    {
    FenceContext::Request input(in);
    FenceContext::Response output(out);
    output.SetStatus(ERROR);

    if (!input.IsValid())
        return;

    bvector<DgnElementId> candidates = input.GetCandidates();
    if (0 == candidates.size())
        return;

    bool allowOverlaps = input.GetAllowOverlaps();
    ClipVectorPtr clip = input.GetClip();

    if (!clip.IsValid())
        return;

    FenceGeometryProcessor processor(*clip, allowOverlaps);
    FenceContext context(processor);

    context.SetDgnDb(db);
    context.SetViewFlags(input.GetViewFlags());
    context.m_offSubCategories = input.GetOffSubCategories();

    output.SetStatus(SUCCESS); // Inputs were valid...
    bvector<ClipPlaneContainment> containments;
    uint32_t numInside = 0, numOutside = 0, numOverlap = 0;

    for (DgnElementId candidateId : candidates)
        {
        if (cancel.IsValid() && cancel->IsCanceled())
            break; // Return what we've processed so far...

        DgnElementCPtr candidateElement = db.Elements().GetElement(candidateId);
        GeometrySourceCP candidateSource = candidateElement.IsValid() ? candidateElement->ToGeometrySource() : nullptr;
        ClipPlaneContainment candiateStatus = ClipPlaneContainment_StronglyOutside;

        if (nullptr != candidateSource && SUCCESS == context.VisitGeometry(*candidateSource))
            {
            if (context.m_processor.m_accept)
                candiateStatus = (context.m_processor.HasOverlaps() ? ClipPlaneContainment_Ambiguous : ClipPlaneContainment_StronglyInside);
            }

        switch (candiateStatus)
            {
            case ClipPlaneContainment_StronglyInside:
                numInside++;
                break;
            case ClipPlaneContainment_StronglyOutside:
                numOutside++;
                break;
            case ClipPlaneContainment_Ambiguous:
                numOverlap++;
                break;
            }

        containments.push_back(candiateStatus);
        }

    output.SetCandiatesStatus(containments);
    output.SetNumInside(numInside);
    output.SetNumOutside(numOutside);
    output.SetNumOverlap(numOverlap);
    }
