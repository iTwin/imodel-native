/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SimplifyGraphic.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnRscFontStructures.h>

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SimplifyCurveCollector : IGeometryProcessor
{
protected:

CurveVectorPtr  m_curves;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic) override
    {
    CurveVectorPtr childCurve = curves.Clone(); // NOTE: Don't apply local to world...want geometry in local coords of parent graphic...

    if (m_curves.IsNull())
        m_curves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    m_curves->Add(childCurve);

    return true;
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetCurveVector() {return m_curves;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process(Render::GraphicCR graphic, ISolidPrimitiveCR geom, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    SimplifyCurveCollector    processor;
    Render::GraphicBuilderPtr builder = new SimplifyGraphic(Render::Graphic::CreateParams(graphic.GetViewport(), graphic.GetLocalToWorldTransform(), graphic.GetPixelSize()), processor, context);

    WireframeGeomUtil::Draw(*builder, geom, &context, includeEdges, includeFaceIso);

    return processor.GetCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process(Render::GraphicCR graphic, MSBsplineSurfaceCR geom, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    SimplifyCurveCollector    processor;
    Render::GraphicBuilderPtr builder = new SimplifyGraphic(Render::Graphic::CreateParams(graphic.GetViewport(), graphic.GetLocalToWorldTransform(), graphic.GetPixelSize()), processor, context);

    WireframeGeomUtil::Draw(*builder, geom, &context, includeEdges, includeFaceIso);

    return processor.GetCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process(Render::GraphicCR graphic, IBRepEntityCR geom, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    SimplifyCurveCollector    processor;
    Render::GraphicBuilderPtr builder = new SimplifyGraphic(Render::Graphic::CreateParams(graphic.GetViewport(), graphic.GetLocalToWorldTransform(), graphic.GetPixelSize()), processor, context);

    WireframeGeomUtil::Draw(*builder, geom, &context, includeEdges, includeFaceIso);

    return processor.GetCurveVector();
    }

}; // SimplifyCurveCollector

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/15
+===============+===============+===============+===============+===============+======*/
struct SimplifyCurveClipper
{
struct IntersectLocationDetail
    {
    size_t m_index;
    double m_fraction;

    IntersectLocationDetail(size_t index, double fraction) {m_index = index; m_fraction = fraction;}
    };

bvector<CurveVectorPtr> m_output;
        
SimplifyCurveClipper() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int CompareCurveIntersections(IntersectLocationDetail const* detail0, IntersectLocationDetail const* detail1)
    {
    if (detail0->m_index < detail1->m_index)
        {
        return -1;
        }
    else if (detail0->m_index > detail1->m_index)
        {
        return 1;
        }
    else
        {
        if (detail0->m_fraction < detail1->m_fraction)
            return -1;
        else if (detail0->m_fraction > detail1->m_fraction)
            return 1;
        else
            return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ComputeInteriorPoint(DPoint3dR midPoint, CurveVectorCR curves, IntersectLocationDetail& startDetail, IntersectLocationDetail& endDetail)
    {
    size_t      midIndex;
    double      midFraction;

    if (startDetail.m_index == endDetail.m_index)
        {
        midIndex = startDetail.m_index;
        midFraction = (startDetail.m_fraction + endDetail.m_fraction) / 2.0;
        }
    else if (endDetail.m_index > startDetail.m_index+1)
        {
        midIndex = startDetail.m_index+1;
        midFraction = 0.5;
        }
    else
        {
        midIndex = endDetail.m_index;
        midFraction = 0.0;
        }

   return curves.at(midIndex)->FractionToPoint(midFraction, midPoint);
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipAsOpenCurveVector(CurveVectorCR curves, ClipVectorCR clip)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        bool    clipped = false;

        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsNull() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType())
                clipped |= ClipAsOpenCurveVector(*curve->GetChildCurveVectorCP (), clip);
            }

        return clipped;
        }

    DRange3d  curveRange;
                             
    if (!curves.GetRange(curveRange))
        return false;

    curveRange.Extend(DgnUnits::OneMillimeter());

    bvector<IntersectLocationDetail> intersectDetails;

    for (ClipPrimitivePtr const& thisClip: clip)
        {
        DRange3d  clipRange;                                                                                             

        // Optimization for TR#300934. Don't intersect with planes from clips that are disjoint from the current GPA.
        if (thisClip->GetRange(clipRange, NULL, true) && !curveRange.IntersectsWith(clipRange))
            continue;

        ClipPlaneSetCP      clipPlaneSet;

        if (NULL != (clipPlaneSet = thisClip->GetMaskOrClipPlanes()))       // Use mask planes if they exist.
            {
            for (ConvexClipPlaneSetCR convexPlaneSet: *clipPlaneSet)
                {
                for (ClipPlaneCR plane: convexPlaneSet)
                    {
                    // NOTE: It would seem that it would not be necessary to calculate intersections with "interior" planes. However, we use 
                    //       that designation to mean any plane that should not generate cut geometry so "interior" is not really correct
                    //       and we need to intersect with these planes as well. Interior intersections do not really cause a problem as we 
                    //       discard them below if insidedness does not change. (RayB TR#244943)
                    bvector<CurveLocationDetailPair> intersections;
                
                    curves.AppendCurvePlaneIntersections(plane.GetDPlane3d(), intersections); // NOTE: Method calls clear in output vector!!!

                    // Get curve index for sorting, can disregard 2nd detail in pair as both should be identical...
                    for (CurveLocationDetailPair pair: intersections)
                        intersectDetails.push_back(IntersectLocationDetail(curves.CurveLocationDetailIndex(pair.detailA), pair.detailA.fraction));
                    }
                }
            }
        }

    if (0 == intersectDetails.size())
        {
        DPoint3d  testPoint;

        if (!curves.front()->FractionToPoint(0.5, testPoint))
            return false;

        return !clip.PointInside(testPoint, 1.0e-5);
        }

    qsort(&intersectDetails.front(), intersectDetails.size(), sizeof (IntersectLocationDetail), (int (*) (void const*, void const*)) CompareCurveIntersections);

    // Add final point for last curve...
    intersectDetails.push_back(IntersectLocationDetail(curves.size()-1, 1.0));

    bool lastInside = false;
    IntersectLocationDetail insideStartDetail(0, 0.0);
    IntersectLocationDetail lastDetail(0, 0.0);

    for (IntersectLocationDetail thisDetail: intersectDetails)
        {
        if (thisDetail.m_index == lastDetail.m_index && (thisDetail.m_fraction - lastDetail.m_fraction) < 1.0e-4)
            continue;

        bool        thisInside = false;
        DPoint3d    midPoint;

        if (ComputeInteriorPoint(midPoint, curves, lastDetail, thisDetail))
            thisInside = clip.PointInside(midPoint, 1.0e-5);

        if (thisInside)
            {
            if (!lastInside)
                insideStartDetail = lastDetail;
            }
        else
            {
            if (lastInside)
                {
                CurveVectorPtr  partialCurve = curves.CloneBetweenDirectedFractions((int) insideStartDetail.m_index, insideStartDetail.m_fraction, (int) lastDetail.m_index, lastDetail.m_fraction, false);

                m_output.push_back(partialCurve);
                }
            }

        lastDetail = thisDetail;
        lastInside = thisInside;
        }

    if (lastInside)
        {
        CurveVectorPtr  partialCurve = curves.CloneBetweenDirectedFractions((int) insideStartDetail.m_index, insideStartDetail.m_fraction, (int) lastDetail.m_index, lastDetail.m_fraction, false);

        m_output.push_back(partialCurve);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPointStringCurvePrimitive(ICurvePrimitiveCR primtive, ClipVectorCR clip)
    {
    bvector<DPoint3d> const* points = primtive.GetPointStringCP();
    bvector<DPoint3d>        insidePts;

    for (size_t iPt = 0; iPt < points->size(); ++iPt)
        {
        if (!clip.PointInside(points->at(iPt), 1.0E-5))
            continue;

        insidePts.push_back(points->at(iPt));
        }

    if (insidePts.empty() || insidePts.size() == points->size())
        return false;

    CurveVectorPtr  tmpCurve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    
    tmpCurve->push_back(ICurvePrimitive::CreatePointString(&insidePts.front(), insidePts.size()));
    m_output.push_back(tmpCurve);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipUnclassifiedCurveVector(CurveVectorCR curves, ClipVectorCR clip)
    {
    if (1 > curves.size())
        return false;

    bool    clipped = false;

    for (ICurvePrimitivePtr curve : curves)
        {
        if (curve.IsNull())
            continue;

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                {
                clipped |= ClipPointStringCurvePrimitive(*curve, clip);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                clipped |= ClipAsOpenCurveVector(*curve->GetChildCurveVectorCP(), clip);
                break;
                }

            default:
                {
                clipped |= ClipAsOpenCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), clip);
                break;
                }
            }
        }

    return clipped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CurveVectorPtr>& ClipCurveVector(CurveVectorCR curve, ClipVectorCR clip)
    {
    if (CurveVector::BOUNDARY_TYPE_None == curve.GetBoundaryType())
        ClipUnclassifiedCurveVector(curve, clip);
    else
        ClipAsOpenCurveVector(curve, clip);

    return m_output;
    }

}; // SimplifyCurveClipper

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/15
+===============+===============+===============+===============+===============+======*/
struct SimplifyPolyfaceClipper : PolyfaceQuery::IClipToPlaneSetOutput
{
bool m_unclipped;
bvector<PolyfaceHeaderPtr> m_output;
        
SimplifyPolyfaceClipper() : m_unclipped(false) {}
StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR) override {m_unclipped = true; return SUCCESS;}
StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override {PolyfaceHeaderPtr meshPtr = &mesh; m_output.push_back(meshPtr); return SUCCESS;}
bvector<PolyfaceHeaderPtr>& ClipPolyface(PolyfaceQueryCR mesh, ClipVectorCR clip, bool triangulate) {clip.ClipPolyface(mesh, *this, triangulate); return m_output;}
bool IsUnclipped() {return m_unclipped;}

}; // SimplifyPolyfaceClipper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
SimplifyGraphic::SimplifyGraphic(Render::Graphic::CreateParams const& params, IGeometryProcessorR processor, ViewContextR context) : T_Super(params), m_processor(processor), m_context(context)
    {
    m_facetOptions = m_processor._GetFacetOptionsP();

    if (!m_facetOptions.IsValid())
        {
        m_facetOptions = IFacetOptions::Create();
    
        m_facetOptions->SetMaxPerFace(5000/*MAX_VERTICES*/);
        m_facetOptions->SetAngleTolerance(0.25 * Angle::Pi());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicBuilderPtr SimplifyGraphic::_CreateSubGraphic(TransformCR subToGraphic, ClipVectorCP clip) const
    {
    SimplifyGraphic* subGraphic = new SimplifyGraphic(Render::Graphic::CreateParams(m_vp, Transform::FromProduct(m_localToWorldTransform, subToGraphic), m_pixelSize), m_processor, m_context);

    subGraphic->m_currGraphicParams  = m_currGraphicParams;
    subGraphic->m_currGeometryParams = m_currGeometryParams;
    subGraphic->m_currGeomEntryId    = m_currGeomEntryId;
    subGraphic->m_currClip           = (nullptr != clip ? clip->Clone(&m_localToWorldTransform) : nullptr);

    return subGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d SimplifyGraphic::GetLocalToView() const
    {
    DMatrix4d   localToWorld = DMatrix4d::From(m_localToWorldTransform);
    DMatrix4d   worldToView = m_context.GetWorldToView().M0;
    DMatrix4d   localToView;

    localToView.InitProduct(worldToView, localToWorld);

    return localToView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d SimplifyGraphic::GetViewToLocal() const
    {
    Transform   worldToLocalTrans;

    worldToLocalTrans.InverseOf(m_localToWorldTransform);

    DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
    DMatrix4d   viewToWorld = m_context.GetWorldToView().M1;
    DMatrix4d   viewToLocal;

    viewToLocal.InitProduct(worldToLocal, viewToWorld);

    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    GetLocalToView().Multiply(viewPts, localPts, nullptr, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    DMatrix4dCR localToView = GetLocalToView();

    if (nullptr != m_context.GetViewport() && m_context.GetViewport()->IsCameraOn())
        localToView.MultiplyAndRenormalize(viewPts, localPts, nPts);
    else
        localToView.MultiplyAffine(viewPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const
    {
    Transform   worldToLocal;

    worldToLocal.InverseOf(m_localToWorldTransform);
    m_context.ViewToWorld(localPts, viewPts, nPts);
    worldToLocal.Multiply(localPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const
    {
    Transform   worldToLocal;

    worldToLocal.InverseOf(m_localToWorldTransform);
    m_context.ViewToWorld(localPts, viewPts, nPts);
    worldToLocal.Multiply(localPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::IsRangeTotallyInside(DRange3dCR range) const
    {
    Frustum box(range);
    return m_context.GetFrustumPlanes().Contains(box.m_pts, 8) == FrustumPlanes::Contained::Inside;
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::IsRangeTotallyInsideClip(DRange3dCR range) const
    { 
    DPoint3d    corners[8];
    range.Get8Corners(corners);
    return ArePointsTotallyInsideClip(corners, 8);
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::ArePointsTotallyInsideClip(DPoint3dCP points, int nPoints) const
    { 
    if (nullptr == GetCurrentClip())
        return true;

    for (ClipPrimitivePtr const& primitive : *GetCurrentClip())
        if (ClipPlaneContainment_StronglyInside != primitive->ClassifyPointContainment(points, nPoints))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**  
* @bsimethod                                                    RayBentley      12/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool SimplifyGraphic::ArePointsTotallyOutsideClip(DPoint3dCP points, int nPoints) const
    { 
    if (nullptr == GetCurrentClip())
        return false;

    for (ClipPrimitivePtr const& primitive : *GetCurrentClip())
        if (ClipPlaneContainment_StronglyOutside == primitive->ClassifyPointContainment(points, nPoints))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessAsLinearSegments(CurveVectorCR geom, bool filled)
    {
    bvector<DPoint3d> points;

    geom.AddStrokePoints(points, *m_facetOptions);
    m_processor._ProcessLinearSegments(&points.front(), points.size(), geom.IsAnyRegionType(), filled, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void processCurvePrimitives(IGeometryProcessorR processor, CurveVectorCR curves, bool filled, SimplifyGraphic& graphic)
    {
    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                {
                BeAssert(true && "Unexpected entry in region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region)...
                }

            processCurvePrimitives(processor, *curve->GetChildCurveVectorCP (), filled && curves.IsUnionRegion(), graphic); // Don't pass filled when spewing parity region loops...
            }
        }
    else
        {
        bool isSingleEntry = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive());
        bool isClosed = curves.IsClosedPath();
        bool isOpen = curves.IsOpenPath();
        bool isComplex = ((isClosed || isOpen) && !isSingleEntry);

        for (ICurvePrimitivePtr curve : curves)
            {
            if (!curve.IsValid())
                continue;

            if (processor._ProcessCurvePrimitive(*curve, !isComplex && isClosed, !isComplex && filled, graphic)) // Don't pass filled when spewing primitives...
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curve->GetCurvePrimitiveType())
                processCurvePrimitives(processor, *curve->GetChildCurveVectorCP (), filled, graphic);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ProcessAsCurvePrimitives(CurveVectorCR geom, bool filled)
    {
    processCurvePrimitives(m_processor, geom, filled, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessCurveVector(CurveVectorCR geom, bool filled)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    if (m_currGeomEntryId.IsValid())
        CurveTopologyId::AddCurveVectorIds(geom, CurvePrimitiveId::Type::CurveVector, CurveTopologyId::FromCurveVector(), m_currGeomEntryId.GetIndex(), m_currGeomEntryId.GetPartIndex());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessCurveVectorClipped(geom, filled, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessCurveVector(geom, filled, *this))
            return;
        }

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);
    bool isAutoClipPref = false;

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Auto & unhandled))
        {
        if (geom.IsAnyRegionType())
            {
            if (!geom.ContainsNonLinearPrimitive())
                unhandled = IGeometryProcessor::UnhandledPreference::Facet; // BRep is expensive - facets will represent this geometry exactly.
            else
                unhandled = IGeometryProcessor::UnhandledPreference::BRep | IGeometryProcessor::UnhandledPreference::Facet; // Try BRep first...

            isAutoClipPref = doClipping;
            }
        else
            {
            unhandled = IGeometryProcessor::UnhandledPreference::Curve; // Clip as open curves...
            }
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::BRep & unhandled))
        {
        if (isAutoClipPref)
            {
            bvector<CurveVectorPtr> insideCurves;

            if (SUCCESS == BRepUtil::ClipCurveVector(insideCurves, geom, *GetCurrentClip(), &m_localToWorldTransform))
                {
                for (CurveVectorPtr tmpCurves : insideCurves)
                    m_processor._ProcessCurveVector(*tmpCurves, filled, *this);

                return;
                }
            }
        else if (!doClipping || geom.IsAnyRegionType()) // ClipBody doesn't support wire bodies...
            {
            IBRepEntityPtr entityPtr;

            if (SUCCESS == BRepUtil::Create::BodyFromCurveVector(entityPtr, geom))
                {
                if (!doClipping)
                    {
                    m_processor._ProcessBody(*entityPtr, *this);
                    return;
                    }

                bool clipped;
                bvector<IBRepEntityPtr> clippedBodies;

                if (SUCCESS == BRepUtil::ClipBody(clippedBodies, clipped, *entityPtr, *GetCurrentClip()) && clipped)
                    {
                    for (IBRepEntityPtr entityOut : clippedBodies)
                        m_processor._ProcessBody(*entityOut, *this);
                    }
                else if (!m_processor._ProcessCurveVector(geom, filled, *this))
                    {
                    m_processor._ProcessBody(*entityPtr, *this);
                    }
                return;
                }
            }

        // If conversion to BRep wasn't possible, check if conversion to another type is requested...
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled) && geom.IsAnyRegionType()) // Can only facet regions...
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*m_facetOptions);

        builder->AddRegion(geom);

        if (!doClipping)
            {
            m_processor._ProcessPolyface(builder->GetClientMeshR(), filled, *this);
            return;
            }

        SimplifyPolyfaceClipper     polyfaceClipper;
        bvector<PolyfaceHeaderPtr>& clippedPolyface = polyfaceClipper.ClipPolyface(builder->GetClientMeshR(), *GetCurrentClip(), m_facetOptions->GetMaxPerFace() <= 3);

        if (0 != clippedPolyface.size())
            {
            for (PolyfaceHeaderPtr meshOut : clippedPolyface)
                m_processor._ProcessPolyface(*meshOut, filled, *this);
            }
        else if (polyfaceClipper.IsUnclipped() && !m_processor._ProcessCurveVector(geom, filled, *this))
            {
            m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
            }
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled) && doClipping) // Already had a chance at un-clipped CurveVector...
        {
        SimplifyCurveClipper     curveClipper;
        bvector<CurveVectorPtr>& clippedCurves = curveClipper.ClipCurveVector(geom, *GetCurrentClip());

        if (0 != clippedCurves.size())
            {
            for (CurveVectorPtr curveOut : clippedCurves)
                m_processor._ProcessCurveVector(*curveOut, false, *this);
            }
        else
            {
            m_processor._ProcessCurveVector(geom, filled, *this); // Give chance to process un-clipped CurveVector...
            }
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessSolidPrimitive(ISolidPrimitiveCR geom)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessSolidPrimitiveClipped(geom, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessSolidPrimitive(geom, *this))
            return;
        }

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Auto & unhandled))
        {
        if (!geom.HasCurvedFaceOrEdge())
            unhandled = IGeometryProcessor::UnhandledPreference::Facet; // BRep is expensive - facets will represent this geometry exactly.
        else
            unhandled = IGeometryProcessor::UnhandledPreference::BRep | IGeometryProcessor::UnhandledPreference::Facet; // Try BRep first...
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::BRep & unhandled))
        {
        IBRepEntityPtr entityPtr;

        if (SUCCESS == BRepUtil::Create::BodyFromSolidPrimitive(entityPtr, geom))
            {
            if (!doClipping)
                {
                m_processor._ProcessBody(*entityPtr, *this);
                return;
                }

            bool clipped;
            bvector<IBRepEntityPtr> clippedBodies;

            if (SUCCESS == BRepUtil::ClipBody(clippedBodies, clipped, *entityPtr, *GetCurrentClip()) && clipped)
                {
                for (IBRepEntityPtr entityOut : clippedBodies)
                    m_processor._ProcessBody(*entityOut, *this);
                }
            else if (!m_processor._ProcessSolidPrimitive(geom, *this))
                {
                m_processor._ProcessBody(*entityPtr, *this);
                }
            return;
            }

        // If conversion to BRep wasn't possible, check if conversion to another type is requested...
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled))
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*m_facetOptions);

        builder->AddSolidPrimitive(geom);

        if (!doClipping)
            {
            m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
            return;
            }

        SimplifyPolyfaceClipper     polyfaceClipper;
        bvector<PolyfaceHeaderPtr>& clippedPolyface = polyfaceClipper.ClipPolyface(builder->GetClientMeshR(), *GetCurrentClip(), m_facetOptions->GetMaxPerFace() <= 3);

        if (0 != clippedPolyface.size())
            {
            for (PolyfaceHeaderPtr meshOut : clippedPolyface)
                m_processor._ProcessPolyface(*meshOut, false, *this);
            }
        else if (polyfaceClipper.IsUnclipped() && !m_processor._ProcessSolidPrimitive(geom, *this))
            {
            m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
            }
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        // NOTE: Can't use WireframeGeomUtil::CollectCurves because we require geometry relative to this graphic/context...
        CurveVectorPtr curves = SimplifyCurveCollector::Process(*this, geom, m_context, m_processor._IncludeWireframeEdges(), m_processor._IncludeWireframeFaceIso());

        if (!curves.IsValid())
            return;

        if (!doClipping)
            {
            m_processor._ProcessCurveVector(*curves, false, *this);
            return;
            }

        SimplifyCurveClipper     curveClipper;
        bvector<CurveVectorPtr>& clippedCurves = curveClipper.ClipCurveVector(*curves, *GetCurrentClip());

        if (0 != clippedCurves.size())
            {
            for (CurveVectorPtr curveOut : clippedCurves)
                m_processor._ProcessCurveVector(*curveOut, false, *this);
            }
        else if (!m_processor._ProcessSolidPrimitive(geom, *this))
            {
            m_processor._ProcessCurveVector(*curves, false, *this);
            }
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessSurface(MSBsplineSurfaceCR geom)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessSurfaceClipped(geom, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessSurface(geom, *this))
            return;
        }

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Auto & unhandled))
        {
        if (geom.IsPlanarBilinear())
            unhandled = IGeometryProcessor::UnhandledPreference::Facet; // BRep is expensive - facets will represent this geometry exactly.
        else
            unhandled = IGeometryProcessor::UnhandledPreference::BRep | IGeometryProcessor::UnhandledPreference::Facet; // Try BRep first...
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::BRep & unhandled))
        {
        IBRepEntityPtr entityPtr;

        if (SUCCESS == BRepUtil::Create::BodyFromBSurface(entityPtr, geom))
            {
            if (!doClipping)
                {
                m_processor._ProcessBody(*entityPtr, *this);
                return;
                }

            bool clipped;
            bvector<IBRepEntityPtr> clippedBodies;

            if (SUCCESS == BRepUtil::ClipBody(clippedBodies, clipped, *entityPtr, *GetCurrentClip()) && clipped)
                {
                for (IBRepEntityPtr entityOut : clippedBodies)
                    m_processor._ProcessBody(*entityOut, *this);
                }
            else if (!m_processor._ProcessSurface(geom, *this))
                {
                m_processor._ProcessBody(*entityPtr, *this);
                }
            return;
            }

        // If conversion to BRep wasn't possible, check if conversion to another type is requested...
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled))
        {
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*m_facetOptions);

        builder->Add(geom);

        if (!doClipping)
            {
            m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
            return;
            }

        SimplifyPolyfaceClipper     polyfaceClipper;
        bvector<PolyfaceHeaderPtr>& clippedPolyface = polyfaceClipper.ClipPolyface(builder->GetClientMeshR(), *GetCurrentClip(), m_facetOptions->GetMaxPerFace() <= 3);

        if (0 != clippedPolyface.size())
            {
            for (PolyfaceHeaderPtr meshOut : clippedPolyface)
                m_processor._ProcessPolyface(*meshOut, false, *this);
            }
        else if (polyfaceClipper.IsUnclipped() && !m_processor._ProcessSurface(geom, *this))
            {
            m_processor._ProcessPolyface(builder->GetClientMeshR(), false, *this);
            }
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        // NOTE: Can't use WireframeGeomUtil::CollectCurves because we require geometry relative to this graphic/context...
        CurveVectorPtr curves = SimplifyCurveCollector::Process(*this, geom, m_context, m_processor._IncludeWireframeEdges(), m_processor._IncludeWireframeFaceIso());

        if (!curves.IsValid())
            return;

        if (!doClipping)
            {
            m_processor._ProcessCurveVector(*curves, false, *this);
            return;
            }

        SimplifyCurveClipper     curveClipper;
        bvector<CurveVectorPtr>& clippedCurves = curveClipper.ClipCurveVector(*curves, *GetCurrentClip());

        if (0 != clippedCurves.size())
            {
            for (CurveVectorPtr curveOut : clippedCurves)
                m_processor._ProcessCurveVector(*curveOut, false, *this);
            }
        else if (!m_processor._ProcessSurface(geom, *this))
            {
            m_processor._ProcessCurveVector(*curves, false, *this);
            }
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessPolyface(PolyfaceQueryCR geom, bool filled)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessPolyfaceClipped(geom, filled, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessPolyface(geom, filled, *this))
            return;
        }

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Auto & unhandled))
        unhandled = IGeometryProcessor::UnhandledPreference::Facet;

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::BRep & unhandled))
        {
        IBRepEntityPtr entityPtr;

        if (SUCCESS == BRepUtil::Create::BodyFromPolyface(entityPtr, geom))
            {
            if (!doClipping)
                {
                m_processor._ProcessBody(*entityPtr, *this);
                return;
                }

            bool clipped;
            bvector<IBRepEntityPtr> clippedBodies;

            if (SUCCESS == BRepUtil::ClipBody(clippedBodies, clipped, *entityPtr, *GetCurrentClip()) && clipped)
                {
                for (IBRepEntityPtr entityOut : clippedBodies)
                    m_processor._ProcessBody(*entityOut, *this);
                }
            else if (!m_processor._ProcessPolyface(geom, filled, *this))
                {
                m_processor._ProcessBody(*entityPtr, *this);
                }
            return;
            }

        // If conversion to BRep wasn't possible, check if conversion to another type is requested...
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled) && doClipping) // Already had a chance at un-clipped Polyface...
        {
        SimplifyPolyfaceClipper     polyfaceClipper;
        bvector<PolyfaceHeaderPtr>& clippedPolyface = polyfaceClipper.ClipPolyface(geom, *GetCurrentClip(), m_facetOptions->GetMaxPerFace() <= 3);

        if (0 != clippedPolyface.size())
            {
            for (PolyfaceHeaderPtr meshOut : clippedPolyface)
                m_processor._ProcessPolyface(*meshOut, filled, *this);
            }
        else if (polyfaceClipper.IsUnclipped())
            {
            m_processor._ProcessPolyface(geom, filled, *this); // Give chance to process un-clipped Polyface...
            }
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        ClipAndProcessPolyfaceAsCurves(geom);
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessPolyfaceAsCurves(PolyfaceQueryCR geom)
    {
    int const*  vertIndex = geom.GetPointIndexCP();
    size_t      numIndices = geom.GetPointIndexCount();
    DPoint3dCP  verts = geom.GetPointCP();
    int         polySize = geom.GetNumPerFace();
    int         thisIndex, prevIndex=0, firstIndex=0;
    size_t      thisFaceSize = 0;

    if (!vertIndex)
        return;

    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    for (size_t readIndex = 0; readIndex < numIndices; readIndex++)
        {    
        // found face loop entry
        if (thisIndex = vertIndex[readIndex])
            {
            if (!thisFaceSize)
                {
                // remember first index in this face loop
                firstIndex = thisIndex;
                }
            else if (prevIndex > 0)
                {
                // draw visible edge (prevIndex, thisIndex)
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(thisIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));

                if (m_currGeomEntryId.IsValid())
                    {
                    CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type::PolyfaceEdge, CurveTopologyId(CurveTopologyId::Type::PolyfaceEdge, closeVertexId, segmentVertexId), m_currGeomEntryId.GetIndex(), m_currGeomEntryId.GetPartIndex());
                    curve->SetId(newId.get());
                    }

                CurveVectorPtr curvePtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve);

                if (!doClipping)
                    {
                    m_processor._ProcessCurveVector(*curvePtr, false, *this);
                    }
                else
                    {
                    SimplifyCurveClipper     curveClipper;
                    bvector<CurveVectorPtr>& clippedCurves = curveClipper.ClipCurveVector(*curvePtr, *GetCurrentClip());

                    if (0 != clippedCurves.size())
                        {
                        for (CurveVectorPtr curveOut : clippedCurves)
                            m_processor._ProcessCurveVector(*curveOut, false, *this);
                        }
                    else
                        {
                        m_processor._ProcessCurveVector(*curvePtr, false, *this);
                        }
                    }
                }

            prevIndex = thisIndex;
            thisFaceSize++;
            }

        // found end of face loop (found first pad/terminator or last index in fixed block)
        if (thisFaceSize && (!thisIndex || (polySize > 1 && polySize == thisFaceSize)))
            {
            // draw last visible edge (prevIndex, firstIndex)
            if (prevIndex > 0)
                {
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(firstIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));

                if (m_currGeomEntryId.IsValid())
                    {
                    CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type::PolyfaceEdge, CurveTopologyId(CurveTopologyId::Type::PolyfaceEdge, closeVertexId, segmentVertexId), m_currGeomEntryId.GetIndex(), m_currGeomEntryId.GetPartIndex());
                    curve->SetId(newId.get());
                    }

                CurveVectorPtr curvePtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve);

                if (!doClipping)
                    {
                    m_processor._ProcessCurveVector(*curvePtr, false, *this);
                    }
                else
                    {
                    SimplifyCurveClipper     curveClipper;
                    bvector<CurveVectorPtr>& clippedCurves = curveClipper.ClipCurveVector(*curvePtr, *GetCurrentClip());

                    if (0 != clippedCurves.size())
                        {
                        for (CurveVectorPtr curveOut : clippedCurves)
                            m_processor._ProcessCurveVector(*curveOut, false, *this);
                        }
                    else
                        {
                        m_processor._ProcessCurveVector(*curvePtr, false, *this);
                        }
                    }
                }

            thisFaceSize = 0;
            }

        if (0 == (readIndex % 100) && m_context.CheckStop())
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessBody(IBRepEntityCR geom)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessBodyClipped(geom, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessBody(geom, *this))
            return;
        }

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(geom, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Auto & unhandled))
        unhandled = IGeometryProcessor::UnhandledPreference::BRep;

    bool isSolidOrSheet = (IBRepEntity::EntityType::Sheet == geom.GetEntityType() || IBRepEntity::EntityType::Solid == geom.GetEntityType());

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::BRep & unhandled) && doClipping && isSolidOrSheet) // Already had a chance at un-clipped solid...
        {
        bool clipped;
        bvector<IBRepEntityPtr> clippedBodies;

        if (SUCCESS == BRepUtil::ClipBody(clippedBodies, clipped, geom, *GetCurrentClip()) && clipped)
            {
            for (IBRepEntityPtr entityOut : clippedBodies)
                m_processor._ProcessBody(*entityOut, *this);
            }
        else
            {
            m_processor._ProcessBody(geom, *this);
            }

        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Facet & unhandled) && isSolidOrSheet)
        {
        ClipAndProcessBodyAsPolyface(geom);
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        // NOTE: Can't use WireframeGeomUtil::CollectCurves because we require geometry relative to this graphic/context...
        CurveVectorPtr curves = SimplifyCurveCollector::Process(*this, geom, m_context, m_processor._IncludeWireframeEdges(), m_processor._IncludeWireframeFaceIso());

        if (!curves.IsValid())
            return;

        if (!doClipping)
            {
            m_processor._ProcessCurveVector(*curves, false, *this);
            return;
            }

        SimplifyCurveClipper     curveClipper;
        bvector<CurveVectorPtr>& clippedCurves = curveClipper.ClipCurveVector(*curves, *GetCurrentClip());

        if (0 != clippedCurves.size())
            {
            for (CurveVectorPtr curveOut : clippedCurves)
                m_processor._ProcessCurveVector(*curveOut, false, *this);
            }
        else if (!m_processor._ProcessBody(geom, *this))
            {
            m_processor._ProcessCurveVector(*curves, false, *this);
            }
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessBodyAsPolyface(IBRepEntityCR entity)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    if (nullptr != entity.GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr> polyfaces;
        bvector<FaceAttachment> params;

        if (BRepUtil::FacetEntity(entity, polyfaces, params, *m_facetOptions))
            {
            for (size_t i = 0; i < polyfaces.size(); i++)
                {
                if (0 == polyfaces[i]->GetPointCount())
                    continue;

                GraphicBuilder builder(*this);
                GeometryParams faceParams;

                params[i].ToGeometryParams(faceParams, m_currGeometryParams);
                m_context.CookGeometryParams(faceParams, builder);

                if (!doClipping)
                    {
                    m_processor._ProcessPolyface(*polyfaces[i], false, *this);
                    continue;
                    }

                SimplifyPolyfaceClipper     polyfaceClipper;
                bvector<PolyfaceHeaderPtr>& clippedPolyface = polyfaceClipper.ClipPolyface(*polyfaces[i], *GetCurrentClip(), m_facetOptions->GetMaxPerFace() <= 3);

                if (0 != clippedPolyface.size())
                    {
                    for (PolyfaceHeaderPtr meshOut : clippedPolyface)
                        m_processor._ProcessPolyface(*meshOut, false, *this);
                    }
                else if (polyfaceClipper.IsUnclipped())
                    {
                    m_processor._ProcessPolyface(*polyfaces[i], false, *this);
                    }
                }

            return;
            }
        }

    PolyfaceHeaderPtr meshPtr = BRepUtil::FacetEntity(entity, *m_facetOptions);

    if (!meshPtr.IsValid())
        return;

    if (!doClipping)
        {
        m_processor._ProcessPolyface(*meshPtr, false, *this);
        return;
        }

    SimplifyPolyfaceClipper     polyfaceClipper;
    bvector<PolyfaceHeaderPtr>& clippedPolyface = polyfaceClipper.ClipPolyface(*meshPtr, *GetCurrentClip(), m_facetOptions->GetMaxPerFace() <= 3);

    if (0 != clippedPolyface.size())
        {
        for (PolyfaceHeaderPtr meshOut : clippedPolyface)
            m_processor._ProcessPolyface(*meshOut, false, *this);
        }
    else if (polyfaceClipper.IsUnclipped())
        {
        m_processor._ProcessPolyface(*meshPtr, false, *this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessText(TextStringCR text)
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessTextStringClipped(text, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessTextString(text, *this))
            return;
        }

    IGeometryProcessor::UnhandledPreference unhandled = m_processor._GetUnhandledPreference(text, *this);

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Box & unhandled))
        {
        if (text.GetText().empty())
            return;
        
        DPoint3d points[5];

        text.ComputeBoundingShape(points);
        text.ComputeTransform().Multiply(points, _countof(points));

        Render::GraphicPtr graphic = _CreateSubGraphic(Transform::FromIdentity(), nullptr);
        SimplifyGraphic* sGraphic = static_cast<SimplifyGraphic*> (graphic.get());

        if (nullptr == sGraphic)
            return;

        CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, 5));
        sGraphic->ClipAndProcessCurveVector(*curve, false);
        return;
        }

    if (IGeometryProcessor::UnhandledPreference::Ignore != (IGeometryProcessor::UnhandledPreference::Curve & unhandled))
        {
        Render::GraphicBuilderPtr graphic = _CreateSubGraphic(text.ComputeTransform(), nullptr);
        SimplifyGraphic* sGraphic = static_cast<SimplifyGraphic*> (graphic.GetGraphic());

        if (nullptr == sGraphic)
            return;

        auto        numGlyphs = text.GetNumGlyphs();
        DPoint3dCP  glyphOrigins = text.GetGlyphOrigins();
        DVec3d      xVector, yVector;
        DgnGlyphCP const* glyphs = text.GetGlyphs();

        text.ComputeGlyphAxes(xVector, yVector);

        if (text.GetGlyphSymbology(sGraphic->m_currGeometryParams))
            m_context.CookGeometryParams(sGraphic->m_currGeometryParams, *graphic);

        Transform       rotationTransform = Transform::From (RotMatrix::From2Vectors(xVector, yVector));
        for (size_t iGlyph = 0; iGlyph < numGlyphs; ++iGlyph)
            {
            if (nullptr != glyphs[iGlyph])
                {
                bool            isFilled = false;
                CurveVectorPtr  curves = glyphs[iGlyph]->GetCurveVector (isFilled);

                curves->TransformInPlace (Transform::FromProduct (Transform::From(glyphOrigins[iGlyph]), rotationTransform));
                ClipAndProcessCurveVector(*curves, isFilled);
                }
            }

        text.AddUnderline(*graphic); // NOTE: Issue with supporting bold resource fonts, don't want underline bolded...
        return;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(int numPoints, DPoint3dP pts3d, DPoint2dCP pts2d, double zDepth)
    {
    for (int i=0; i<numPoints; i++, pts3d++, pts2d++)
        pts3d->Init(pts2d->x, pts2d->y, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddLineString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLineString(points, numPoints));
    
    ClipAndProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddLineString2d(int numPoints, DPoint2dCP points, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddLineString(numPoints, &localPointsBuf3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPointString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreatePointString(points, numPoints));

    ClipAndProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPointString2d(int numPoints, DPoint2dCP points, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddPointString(numPoints, &localPointsBuf3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddShape(int numPoints, DPoint3dCP points, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, numPoints));

    ClipAndProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddShape(numPoints, &localPointsBuf3d[0], filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
PUSH_MSVC_IGNORE(6386) // I can't figure out how to silence this static analysis warning in this function, so just ignoring.
void SimplifyGraphic::_AddTriStrip(int numPoints, DPoint3dCP points, int32_t usageFlags)
    {
    if (1 == usageFlags) // represents thickened line...
        {
        int         nPt = 0;
        DPoint3dP   tmpPtsP = (DPoint3dP) _alloca((numPoints+1) * sizeof (DPoint3d));

        for (int iPtS1=0; iPtS1 < numPoints; iPtS1 = iPtS1+2)
            tmpPtsP[nPt++] = points[iPtS1];

        for (int iPtS2=numPoints-1; iPtS2 > 0; iPtS2 = iPtS2-2)
            tmpPtsP[nPt++] = points[iPtS2];

        tmpPtsP[nPt] = tmpPtsP[0]; // Add closure point...simplifies drop of extrude thickness...

        _AddShape(numPoints+1, tmpPtsP, true);
        return;
        }

    // spew triangles
    for (int iPt=0; iPt < numPoints-2; iPt++)
        _AddShape(3, &points[iPt], true);
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth)
    {
    std::valarray<DPoint3d> localPointsBuf3d(numPoints);

    copy2dTo3d(numPoints, &localPointsBuf3d[0], points, m_processor._AdjustZDepth(zDepth));
    _AddTriStrip(numPoints, &localPointsBuf3d[0], usageFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled)
    {
    // NOTE: QVis closes arc ends and displays them filled (see outputCapArc for linestyle strokes)...
    CurveVectorPtr curve = CurveVector::Create((isEllipse || filled) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse));
    
    if (filled && !isEllipse && !ellipse.IsFullEllipse())
        {
        DSegment3d         segment;
        ICurvePrimitivePtr gapSegment;

        ellipse.EvaluateEndPoints(segment.point[1], segment.point[0]);
        gapSegment = ICurvePrimitive::CreateLine(segment);
        gapSegment->SetMarkerBit(ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
        curve->push_back(gapSegment);
        }

    ClipAndProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddArc(ellipse, isEllipse, filled);
        }
    else
        {
        auto ell = ellipse;
        ell.center.z = zDepth;
        _AddArc(ell, isEllipse, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBSplineCurve(MSBsplineCurveCR bcurve, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(bcurve));

    ClipAndProcessCurveVector(*curve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBSplineCurve2d(MSBsplineCurveCR bcurve, bool filled, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddBSplineCurve(bcurve, filled);
        }
    else
        {
        MSBsplineCurve bs;
        bs.CopyFrom(bcurve);
        int nPoles = bs.GetNumPoles();
        DPoint3d* poles = bs.GetPoleP();
        for (int i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurve(bs, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddCurveVector(CurveVectorCR curves, bool isFilled)
    {
    ClipAndProcessCurveVector(curves, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddCurveVector(curves, isFilled);
        }
    else
        {
        Transform tf = Transform::From(DPoint3d::FromXYZ(0.0, 0.0, zDepth));
        auto cv = curves.Clone(tf);
        _AddCurveVector(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddSolidPrimitive(ISolidPrimitiveCR geom)
    {
    ClipAndProcessSolidPrimitive(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBSplineSurface(MSBsplineSurfaceCR geom)
    {
    ClipAndProcessSurface(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPolyface(PolyfaceQueryCR geom, bool filled)
    {
    // See if we need to modify this polyface to conform to the processor's facet options...
    IGeometryProcessor::IncompatiblePolyfacePreference pref = m_processor._GetIncompatiblePolyfacePreference(geom, *this);

    if (IGeometryProcessor::IncompatiblePolyfacePreference::Original != pref)
        {
        size_t maxPerFace;

        if ((m_facetOptions->GetNormalsRequired() && 0 == geom.GetNormalCount()) ||
            (m_facetOptions->GetParamsRequired() && (0 == geom.GetParamCount() || 0 == geom.GetFaceCount())) ||
            (m_facetOptions->GetEdgeChainsRequired() && 0 == geom.GetEdgeChainCount()) ||
            (m_facetOptions->GetConvexFacetsRequired() && !geom.HasConvexFacets()) ||
            (geom.GetNumFacet(maxPerFace) > 0 && (int) maxPerFace > m_facetOptions->GetMaxPerFace()))
            {
            if (IGeometryProcessor::IncompatiblePolyfacePreference::Ignore == pref)
                return;

            IPolyfaceConstructionPtr builder = PolyfaceConstruction::New(*m_facetOptions);

            builder->AddPolyface(geom);
            ClipAndProcessPolyface(builder->GetClientMeshR(), filled);
            return;
            }
        }

    ClipAndProcessPolyface(geom, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTriMesh(TriMeshArgs const& args)
    {
    ClipAndProcessTriMesh(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::ClipAndProcessTriMesh(TriMeshArgs const& args) 
    {
    bool doClipping = (nullptr != GetCurrentClip() && m_processor._DoClipping());

    // Give output a chance to handle geometry directly...
    if (doClipping)
        {
        if (m_processor._ProcessTriMeshClipped(args, *this, *GetCurrentClip()))
            return;
        }
    else
        {
        if (m_processor._ProcessTriMesh(args, *this))
            return;
        }

    PolyfaceHeaderPtr           polyface = args.ToPolyface();

    if (m_processor._GetUnhandledPreference(args, *this) == IGeometryProcessor::UnhandledPreference::Ignore)
        return;     

    if (m_processor._GetUnhandledPreference(args, *this) == IGeometryProcessor::UnhandledPreference::Facet)
        {
        ClipAndProcessPolyface (*polyface, false);
        return;
        }

    SimplifyPolyfaceClipper     polyfaceClipper;
    bvector<PolyfaceHeaderPtr>& clippedPolyfaces = polyfaceClipper.ClipPolyface(*polyface, *GetCurrentClip(), true);

    if (polyfaceClipper.IsUnclipped())
        {
        m_processor._ProcessTriMesh(args, *this);
        }
    else 
        {
        for (PolyfaceHeaderPtr clippedPolyface : clippedPolyfaces)
            {
            if (!clippedPolyface->IsTriangulated())
                clippedPolyface->Triangulate();

            if ((0 != clippedPolyface->GetParamCount() && clippedPolyface->GetParamCount() != clippedPolyface->GetPointCount()) || 
                (0 != clippedPolyface->GetNormalCount() && clippedPolyface->GetNormalCount() != clippedPolyface->GetPointCount()))
                clippedPolyface = PolyfaceHeader::CreateUnifiedIndexMesh(*clippedPolyface);

            size_t              numFacets = clippedPolyface->GetNumFacet();
            size_t              numPoints = clippedPolyface->GetPointCount();
            bvector<int32_t>    indices;
            bvector<FPoint3d>   points(numPoints), normals(nullptr == clippedPolyface->GetNormalCP() ? 0 : numPoints);
            bvector<FPoint2d>   params(nullptr == clippedPolyface->GetParamCP() ? 0 : numPoints);

            for (size_t i=0; i<numPoints; i++)
                {
                bsiFPoint3d_initFromDPoint3d(&points[i], &clippedPolyface->GetPointCP()[i]);
                if (nullptr != clippedPolyface->GetNormalCP())
                    bsiFPoint3d_initFromDPoint3d(&normals[i], &clippedPolyface->GetNormalCP()[i]);

                if (nullptr != clippedPolyface->GetParamCP())
                    bsiFPoint2d_initFromDPoint2d(&params[i], &clippedPolyface->GetParamCP()[i]);
                }
            size_t  j = 0;
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*clippedPolyface, true);
            for (visitor->Reset(); visitor->AdvanceToNextFace();)
                {   
                indices.push_back(visitor->GetClientPointIndexCP()[0]);
                indices.push_back(visitor->GetClientPointIndexCP()[1]);
                indices.push_back(visitor->GetClientPointIndexCP()[2]);
                }

            TriMeshArgs     triMesh;

            triMesh.m_numIndices = 3 * clippedPolyface->GetNumFacet();
            triMesh.m_numPoints = numPoints;
            triMesh.m_vertIndex = &indices.front();
            triMesh.m_points  = &points.front();
            triMesh.m_normals = normals.empty() ? nullptr : &normals.front();
            triMesh.m_textureUV = params.empty() ? nullptr : &params.front();
            triMesh.m_texture = args.m_texture;
            triMesh.m_flags = args.m_flags;

            m_processor._ProcessTriMesh(triMesh, *this);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddBody(IBRepEntityCR geom)
    {
    ClipAndProcessBody(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTextString(TextStringCR text)
    {
    ClipAndProcessText(text);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddTextString2d(TextStringCR text, double zDepth)
    {
    if (0.0 == (zDepth = m_processor._AdjustZDepth(zDepth)))
        {
        _AddTextString(text);
        }
    else
        {
        TextStringPtr ts = text.Clone();
        auto origin = ts->GetOrigin();
        origin.z = zDepth;
        ts->SetOrigin(origin);
        _AddTextString(*ts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddDgnOle(DgnOleDraw* ole)
    {
    // NEEDSWORK...Draw box...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddPointCloud(int32_t numPoints, DPoint3dCR origin, FPoint3d const* points, ByteCP colors)
    {
    // NEEDSWORK...Provide option to handle/ignore...
    enum {MAX_POINTS_PER_BATCH = 300};
    if (0 == numPoints)
        return;

    // Don't risk stack overflow to get points buffer
    int32_t maxPointsPerIter = MAX_POINTS_PER_BATCH;

    if (numPoints < maxPointsPerIter)
        maxPointsPerIter = numPoints;

    DPoint3dP pointBuffer = (DPoint3dP)_alloca(maxPointsPerIter * sizeof (*pointBuffer));

    // Convert float points to DPoints and add offset
    FPoint3dCP currIn = points;
    while (numPoints > 0)
        {
        if (m_context.CheckStop())
            return;

        uint32_t pointsThisIter = numPoints > maxPointsPerIter ? maxPointsPerIter : numPoints;

        for (DPoint3dP  curr = pointBuffer; curr < pointBuffer + pointsThisIter; curr++, currIn++)
            {
            curr->x = currIn->x + origin.x;
            curr->y = currIn->y + origin.y;
            curr->z = currIn->z + origin.z;
            }

        _AddPointString(pointsThisIter, pointBuffer);
        numPoints -= pointsThisIter;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void SimplifyGraphic::_AddTile(Render::TextureCR tile, Render::IGraphicBuilder::TileCorners const& corners)
    {
    FPoint3d                        points[4];
    FPoint2d                        params[4];
    int32_t                         indices[6] = {0, 1, 2, 1, 3, 2};
    IGraphicBuilder::TriMeshArgs    triMesh;
    
    triMesh.m_numIndices = 6;
    triMesh.m_vertIndex = indices;
    triMesh.m_numPoints = 4;
    triMesh.m_points = points;
    triMesh.m_normals = nullptr;
    triMesh.m_textureUV = params;
    triMesh.m_texture = const_cast<Render::TextureP> (&tile);
    triMesh.m_flags = 0;

    for (size_t i=0; i<4; i++)
        {
        points[i].x = corners.m_pts[i].x;
        points[i].y = corners.m_pts[i].y;
        points[i].z = corners.m_pts[i].z;
        }
    
    params[0].x = params[2].x = 0.0;
    params[1].x = params[3].x = 1.0;
    params[0].y = params[1].y = 0.0;
    params[2].y = params[3].y = 1.0;

    _AddTriMesh(triMesh);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_AddSubGraphic(GraphicR, TransformCR, GraphicParamsCR, ClipVectorCP clip)
    {
    // Nothing to do...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyGraphic::_ActivateGraphicParams(GraphicParamsCR graphicParams, GeometryParamsCP geomParams)
    {
    m_currGraphicParams = graphicParams;

    if (nullptr != geomParams)
        m_currGeometryParams = *geomParams;
    else
        m_currGeometryParams = GeometryParams();
    }

#ifdef NEEDS_WORK_GEOMETRY_MAPS
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCP SimplifyGraphic::GetCurrentGeometryMap() const
    {
    MaterialCP      material;
    MaterialMapCP   map;

    if (! m_processingMaterialGeometryMap  &&
        NULL != m_context->GetViewFlags() &&
        (NULL == m_context->GetCurrentCookedDisplayStyle() || !m_context->GetCurrentCookedDisplayStyle()->m_flags.m_ignoreGeometryMaps) &&
        NULL != (material = GetCurrentMaterial()) &&
        NULL != (map = material->GetGeometryMap()) &&
        map->IsEnabled() &&
        _ProduceMaterialGeometryMaps(*material, *map))
        return map;

    return NULL;
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     03/2013
+===============+===============+===============+===============+===============+======*/
struct      GeometryMapFacetOptionsMark
{
    SimplifyGraphic&       m_drawGeom;
    bool                        m_saveParamsRequired;

    ~GeometryMapFacetOptionsMark() { m_drawGeom.GetFacetOptions()->SetParamsRequired(m_saveParamsRequired); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryMapFacetOptionsMark(SimplifyGraphic& drawGeom) : m_drawGeom(drawGeom)
    { 
    m_saveParamsRequired = drawGeom.GetFacetOptions()->GetParamsRequired();
    drawGeom.GetFacetOptions()->SetParamsRequired(true);
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static double calculateFacetParamArea(ElementProjectionInfo& projectionInfo, TransformCP currTrans, PolyfaceQueryCR facets, MaterialCR material, MaterialMapLayerCR layer)
    {
    double                  area = 0.0;
    bvector<DPoint2d>       facetParams;

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(facets); visitor->AdvanceToNextFace(); )
        {
        material.ComputeUVParams(facetParams, currTrans, projectionInfo, *visitor, layer);
        area += fabs(PolygonOps::Area(&facetParams[0], visitor->NumEdgesThisFace()));
        }

    return area;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyGraphic::ProcessGeometryMap(PolyfaceQueryCR facets)
    {
    MaterialCP          material;
    MaterialMapCP       geometryMap;

    if (NULL == (material = GetCurrentMaterial()) ||
        NULL == (geometryMap = GetCurrentGeometryMap()))
        return ERROR;

    StatusInt               status;
    EditElementHandle       definitionEh;
    bool                    useCellColors;
    MaterialMapLayerCR      layer = geometryMap->GetLayers().GetTopLayer();

    if (SUCCESS != (status = material->GetGeometryMapDefinition(definitionEh, useCellColors)))
        return status;


    // Set the level same as the parent so the definition children are never eliminated on level/scan criteria.
    ElementPropertiesSetter     levelSetter;

    levelSetter.SetCategory(m_context->GetCurrentGeometryParams()->GetCategory());
    levelSetter.SetChangeEntireElement(true);

    levelSetter.Apply(definitionEh);

    bvector<DPoint2d>       facetParams;
    ElementProjectionInfo   projectionInfo;
    CookedDisplayStyleP     displayStyle = const_cast <CookedDisplayStyleP> (m_context->GetCurrentCookedDisplayStyle());        // Changed temporarily....
    bool                    colorFromMaterial = NULL != displayStyle && displayStyle->m_flags.m_hLineMaterialColors;

    if (useCellColors && colorFromMaterial)
         displayStyle->m_flags.m_hLineMaterialColors = false;         // Else an assigned material may cause material from color to be assigned to geometry map.
        
    AutoRestore <ViewContext::T_DrawMethod>     saveDrawMethod(&m_context->m_callDrawMethod, &ViewContext::DrawElementNormal);
    AutoRestore <bool>                          saveNoRangeTestOnComponents(&m_context->m_noRangeTestOnComponents, true);
    AutoRestore <bool>                          saveProcessingGeometryMap(&m_processingMaterialGeometryMap, true);
    AutoRestore <GraphicParams>                   saveOutputGraphicParams(&m_currentMatSymb);
    AutoRestore <OvrGraphicParams>                    saveOutputOvrGraphicParams(&m_overrideMatSymb);
    AutoRestore <GraphicParams>                   saveContextGraphicParams(m_context->GetGraphicParams());
    AutoRestore <OvrGraphicParams>                    saveContextOvrGraphicParams(m_context->GetOverrideGraphicParams());
    AutoRestore <GeometryParams>             saveContextDisplayParams(m_context->GetCurrentGeometryParams());
    XGraphicsRecorder*                          xGraphicsRecorder = NULL;
    Transform                                   localToElement, elementToRoot;

    m_context->GetDisplayParamsIgnores().Set(*m_context->GetCurrentGeometryParams(), true, !useCellColors, false); // NOTE: Geometry map level is always inherited from base element...

    projectionInfo.CalculateForElement(GetCurrentElement(), SUCCESS == GetElementToRootTransform(elementToRoot) ? &elementToRoot : NULL, *material, geometryMap);

    // The parameter area is equivalent to the tile count.... 
    double          paramArea = calculateFacetParamArea(projectionInfo, SUCCESS == GetLocalToElementTransform(localToElement) ? &localToElement : NULL, facets, *material, layer);
    static double   s_minTileCount = (.01 * .01), s_maxTileCount = (1000.0 * 1000.0);
    
    if (paramArea < s_minTileCount || paramArea > s_maxTileCount)
        {
        //BeAssert (false);
        return ERROR;
        }

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(facets); visitor->AdvanceToNextFace(); )
        {
        if (ClipPlaneContainment_StronglyOutside != m_context->GetTransformClipStack().ClassifyPoints(visitor->GetPointCP(), visitor->NumEdgesThisFace()))
            {
            material->ComputeUVParams(facetParams, SUCCESS == GetLocalToElementTransform(localToElement) ? &localToElement : NULL, projectionInfo, *visitor, layer);
            ProcessGeometryMap(visitor->GetPointCP(), &facetParams[0], visitor->NumEdgesThisFace(), definitionEh, xGraphicsRecorder);
            }
        }

    DELETE_AND_CLEAR (xGraphicsRecorder);

    m_context->GetDisplayParamsIgnores().Clear();

    if (useCellColors && colorFromMaterial)
        displayStyle->m_flags.m_hLineMaterialColors = true;            // Restore if changed above.

    return SUCCESS;
    }

#define IS_SIGNIFICANT20(v)     (fabs(v) > 1.0e-20)

/*---------------------------------------------------------------------------------**//**
* solves linear system using Gaussian elimination with partial pivoting
* @bsimethod                                                    BJB             01/87
+---------------+---------------+---------------+---------------+---------------+------*/
////////////////////NEEDS_WORK... Find equivalent in geomlibs?  /////////////////////////////
static int solve_linear_system
(
double  *amatrix,       /* =>  matrix of coefficients */
int     dimensions,     /* =>  dimension of matrix */
double  *rhsides,       /* <=> input=right hand sides.  output=solutions */
int     numrhs          /* =>  number of right hand sides */
)
    {
    int         row, irow, column, i, j, pivotrow;
    double      *dp, *dp1, *dp2;
    double      temp, pivotel;

    for (row=0; row<dimensions; row++)
        {
        /* first find the largest element of column i and interchange
            that row with row row */
        for (j=pivotrow=row, dp=amatrix+(row*dimensions+row), temp = 0.0;
                j < dimensions;  j++, dp+=dimensions)
            {
            if (fabs(*dp) > temp)
                {
                temp = fabs(*dp);
                pivotrow = j;
                }
            }

        /* if we have an illconditioned matrix, quit */
        if (! IS_SIGNIFICANT20(temp)) return (1);
        else if (row != pivotrow)
            {
            /* interchange row and pivotrow of coefficient matrix & rhsides */
            for (j=row, dp=amatrix+(row*dimensions+j),
                      dp1=amatrix+(pivotrow*dimensions+j); j<dimensions;
                        j++, dp++, dp1++)
                {
                temp = *dp1;
                *dp1 = *dp;
                *dp = temp;
                }

            for (j=0, dp=rhsides+row, dp1=rhsides+pivotrow; j<numrhs;
                    j++, dp+=dimensions, dp1+=dimensions)
                {
                temp = *dp1;
                *dp1 = *dp;
                *dp = temp;
                }
            }

        /* now go through and do the elimination */
        for (irow=row+1, pivotel= *(amatrix+(row*dimensions+row));
                irow<dimensions; irow++)
            {
            for (column=row+1, dp=amatrix+(irow*dimensions+row),
                dp1=amatrix+(row*dimensions+column),
                temp= *dp++/pivotel; column<dimensions; column++, dp++, dp1++)
                {
                *dp -= temp * *dp1;
                }
            /* do the same operation on the right hand sides */
            for (column=0, dp=rhsides+irow, dp1=rhsides+row;
                 column<numrhs; column++, dp1+= dimensions, dp+=dimensions)
                {
                *dp -= temp * *dp1;
                }
            }
        }

    /* next go through and do the back substitution for all rh sides */
    for (i=0; i<numrhs; i++)
        {
        for (row=dimensions-1, dp=rhsides+i*dimensions+row; row>=0; row--, dp--)
            {
            for (j=dimensions-1, dp1=rhsides+i*dimensions+j,
                    dp2=amatrix+row*dimensions+j; j>row; j--, dp1--, dp2--)
                {
                *dp -= *dp1 * *dp2;
                }
            *dp /= *dp2;
            }
        }
    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt calculateParamToWorld(TransformR transform, DPoint2dCP params, DPoint3dCP points, size_t nPoints)
    {
    StatusInt   status;
    double      aMatrix[3][3];
    double      rhSides[3][3];

    /* a11 = sumxixi, a12 = sumxiyi, a14 = sumxi,
       a21 = sumxiyi, a22 = sumyiyi, a24 = sumyi,
       a41 = sumxi,   a42 = sumyi,   a44 = 1 */

    /* rhs11 = sumXixi, rhs12 = sumXiyi, rhs14 = sumXi,
       rhs21 = sumYixi, rhs22 = sumYiyi, rhs24 = sumYi,
       rhs31 = sumZixi, rhs32 = sumZiyi, rhs34 = sumZi */

    memset(aMatrix, 0, sizeof (aMatrix));
    memset(rhSides, 0, sizeof (rhSides));

    DRange3d        pointRange;

    pointRange.InitFrom(points, (int) nPoints);
    
    for (size_t i=0; i<nPoints; i++)
        {
        DPoint3d            point;

        point.DifferenceOf(points[i], pointRange.low);

        aMatrix[0][0] += params[i].x * params[i].x;
        aMatrix[0][1] += params[i].x * params[i].y;
        aMatrix[0][2] += params[i].x;
        aMatrix[1][1] += params[i].y * params[i].y;
        aMatrix[1][2] += params[i].y;

        rhSides[0][0] += point.x * params[i].x;
        rhSides[0][1] += point.x * params[i].y;
        rhSides[0][2] += point.x;
        rhSides[1][0] += point.y * params[i].x;
        rhSides[1][1] += point.y * params[i].y;
        rhSides[1][2] += point.y;
        rhSides[2][0] += point.z * params[i].x;
        rhSides[2][1] += point.z * params[i].y;
        rhSides[2][2] += point.z;

        }                                                                                                                                                                   
    aMatrix[1][0] = aMatrix[0][1];
    aMatrix[2][0] = aMatrix[0][2];
    aMatrix[2][1] = aMatrix[1][2];
    aMatrix[2][2] = (double) nPoints;

    if (SUCCESS == (status = solve_linear_system((double *)aMatrix, 3, (double *)rhSides, 3)))
        {
        DVec3d      xColumn, yColumn, zColumn;
        RotMatrix   rMatrix;

        xColumn.x = rhSides[0][0];
        xColumn.y = rhSides[1][0];
        xColumn.z = rhSides[2][0];

        yColumn.x = rhSides[0][1];
        yColumn.y = rhSides[1][1];
        yColumn.z = rhSides[2][1];
        zColumn.NormalizedCrossProduct(xColumn, yColumn);

        rMatrix.InitFromColumnVectors(xColumn, yColumn, zColumn);

        transform.InitFrom(rMatrix);
        transform.form3d[0][3] = rhSides[0][2] + pointRange.low.x;
        transform.form3d[1][3] = rhSides[1][2] + pointRange.low.y;
        transform.form3d[2][3] = rhSides[2][2] + pointRange.low.z;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
static double computePolygonNormal(DVec3dR normal, DPoint3dCP pXYZ, size_t numXYZ)
    {
    DVec3d      uVec, vVec, wVec;

    normal.Zero();
    uVec.DifferenceOf(pXYZ[1], pXYZ[0]);
    for (size_t i = 2 ; i < numXYZ; i++)
        {
        vVec.DifferenceOf(pXYZ[ i], pXYZ[0]);
        wVec.CrossProduct(uVec, vVec);
        normal.add(&wVec);
        uVec = vVec;
        }
    return normal.Normalize();
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      06/2010
+===============+===============+===============+===============+===============+======*/
struct FacetOutlineMeshGatherer : PolyfaceQuery::IClipToPlaneSetOutput
{
    IPolyfaceConstructionR  m_builder;
    TransformCR             m_transform;
        
    FacetOutlineMeshGatherer(IPolyfaceConstructionR builder, TransformCR transform) : m_builder(builder), m_transform(transform) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override
    {
    PolyfaceHeaderPtr   tempPolyface = PolyfaceHeader::New();

    tempPolyface->CopyFrom(polyfaceQuery);
    tempPolyface->Transform(m_transform);
    m_builder.AddPolyface(*tempPolyface);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override
    {
    polyfaceHeader.Transform(m_transform);
    m_builder.AddPolyface(polyfaceHeader);

    return SUCCESS;
    }

}; // FacetOutlineMeshGatherer

static double   s_facetTileAreaMinimum   = (.01 * .01);      // Minimum portion of tile within a single 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyGraphic::ProcessFacetTextureOutlines(IPolyfaceConstructionR builder, DPoint3dCP points, DPoint2dCP params, bool const* edgeVisible, size_t nPoints, bvector<DPoint3d>& outlinePoints, bvector<int32_t>& outlineIndices)
    {
    DRange2d            paramRange;
    DVec2d              paramDelta;
    static double       s_minParamRange = 1.0E-3;

    paramRange.InitFrom(params, (int) nPoints);
    paramDelta.DifferenceOf(paramRange.high, paramRange.low);

    if (paramDelta.x < s_minParamRange || paramDelta.y < s_minParamRange)
        return ERROR;

    Transform           paramToWorld;
    DVec3d              normal;
    DRange2d            tileRange;

    computePolygonNormal(normal, points, nPoints);
    if (PolygonOps::Area(params, (int) nPoints) < s_facetTileAreaMinimum ||
        SUCCESS != calculateParamToWorld(paramToWorld, params, points, nPoints))
        return ERROR;

    tileRange.InitFrom(floor(paramRange.low.x), floor(paramRange.low.y), ceil(paramRange.high.x), ceil(paramRange.high.y));

    ConvexClipPlaneSet  convexPlanes;
    bool                clockwise = params[1].crossProductToPoints(&params[0], &params[2]) > 0.0;

    for (size_t i=0; i<nPoints; i++)
        {
        DVec2d          delta;
        if (0.0 != delta.NormalizedDifference(params[(i+1) % nPoints], params[i]))
            {
            DVec3d          normal;
            double          distance;

            normal.Init(clockwise ? delta.y : -delta.y, clockwise ? -delta.x : delta.x, 0.0);
            distance = normal.x * params[i].x + normal.y * params[i].y;

            convexPlanes.push_back(ClipPlane(normal, distance, !edgeVisible[i]));
            }
        }

    ClipPlaneSet                clipPlaneSet(convexPlanes);
    bvector <DPoint3d>          tileOutline(outlinePoints.size());
    PolyfaceQueryCarrier        tileFacets(0, true, outlineIndices.size(), outlinePoints.size(), &tileOutline[0], &outlineIndices[0]);
    FacetOutlineMeshGatherer    outputGatherer(builder, paramToWorld);
    T_ClipPlaneSets             planeSets;

    planeSets.push_back(clipPlaneSet);
    for (int i = (int) tileRange.low.x; i < (int) tileRange.high.x; i++)
        {
        for (int j = (int) tileRange.low.y; j < (int) tileRange.high.y; j++)  
            {
            for (size_t iPoint = 0; iPoint < outlinePoints.size(); iPoint++)
                tileOutline[iPoint].Init((double) i + outlinePoints[iPoint].x, (double) j + outlinePoints[iPoint].y, 0.0);

            tileFacets.ClipToPlaneSetIntersection(planeSets, outputGatherer, false);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SimplifyGraphic::ProcessTextureOutlines(PolyfaceQueryCR facets)
    {
    MaterialCP              material;
    MaterialMapCP           patternMap;
    size_t                  numFacets = facets.GetNumFacet();
    bvector<DPoint3d>       outlinePoints;

    if (0 == facets.GetParamCount() ||             // Prevents recursion.
        !_ProduceTextureOutlines() ||
        NULL == (material = GetCurrentMaterial()) ||
        NULL == (patternMap = material->GetSettings().GetMaps().GetMapCP (MaterialMap::MAPTYPE_Pattern)) ||
        SUCCESS != T_HOST.GetGraphicsAdmin()._ExtractTextureOutline(outlinePoints, *material))
        return ERROR;

    MaterialMapLayerCR      layer = patternMap->GetLayers().GetTopLayer();
    ElementProjectionInfo   projectionInfo;
    Transform               localToElement, elementToRoot;

    projectionInfo.CalculateForElement(GetCurrentElement(), SUCCESS == GetElementToRootTransform(elementToRoot) ? &elementToRoot : NULL, *material, patternMap);

    double          paramArea = calculateFacetParamArea(projectionInfo, SUCCESS == GetLocalToElementTransform(localToElement) ? &localToElement : NULL, facets, *material, layer);
    double          pointMagnitude = (double) outlinePoints.size() * paramArea * (double) numFacets;
    static double   s_maxPointMagnitude = 1.0E8;

    if (pointMagnitude > s_maxPointMagnitude)
        return ERROR;
    
    PolyfaceHeaderPtr   triangulatedFacets = PolyfaceHeader::New();
    bvector<int32_t>    triangulatedOutlineIndices;
    StatusInt           status;

    triangulatedFacets->CopyFrom(const_cast <PolyfaceQueryR> (facets));
    triangulatedFacets->Triangulate();

    if (SUCCESS != (status = vu_triangulateSpacePolygonExt2(&triangulatedOutlineIndices, NULL, NULL, NULL, NULL, &outlinePoints[0], (int) outlinePoints.size(), 1.0E-6, TRUE)))
        return status;

    bvector<DPoint2d>           facetParams;
    IPolyfaceConstructionPtr    builder = IPolyfaceConstruction::New(*_GetFacetOptions());

    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*triangulatedFacets); visitor->AdvanceToNextFace(); )
        {
        size_t  nFacetPoints = visitor->NumEdgesThisFace();

        material->ComputeUVParams(facetParams, m_context->GetCurrLocalToWorldTransformCP(), projectionInfo, *visitor, layer);
        ProcessFacetTextureOutlines(*builder, visitor->GetPointCP(), &facetParams[0], visitor->GetVisibleCP(), nFacetPoints, outlinePoints, triangulatedOutlineIndices);
        }

    if (builder->GetClientMeshR().HasFacets())
        _ProcessFacetSet(builder->GetClientMeshR(), false);
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    SimplifyGraphic::StrokeGeometryMap(CurveVectorCR curves)
    {
    if (NULL == GetCurrentGeometryMap())
        return;

    GeometryMapFacetOptionsMark     facetOptionsMark(*this);
    IPolyfaceConstructionPtr        builder = GetPolyfaceBuilder();

    builder->AddRegion(curves);
    ProcessGeometryMap(builder->GetClientMeshR());
    }

#endif // NEEDS_WORK_GEOMETRY_MAPS

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/09
+===============+===============+===============+===============+===============+======*/
struct GeometryProcessorContext : NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

IGeometryProcessorR    m_processor;

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override
    {
    return new SimplifyGraphic(params, m_processor, *this);
    }

public:

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryProcessorContext(IGeometryProcessorR processor) : m_processor(processor)
    {
    m_purpose = m_processor._GetProcessPurpose();
    m_wantMaterials = true; // Setup material in GeometryParams in case processor needs it...do we still need to do this on DgnDbCR???
    }

}; // GeometryProcessorContext

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryProcessor::Process(IGeometryProcessorR processor, DgnDbR dgnDb)
    {
    GeometryProcessorContext context(processor);

    context.SetDgnDb(dgnDb);
    processor._OutputGraphics(context);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryProcessor::Process(IGeometryProcessorR processor, GeometrySourceCR source)
    {
    GeometryProcessorContext context(processor);

    context.SetDgnDb(source.GetSourceDgnDb());
    context.VisitGeometry(source);
    }    






















                                                                                                                                      
