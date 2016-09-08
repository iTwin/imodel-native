/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/FitContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FitContext::_InitContextForView() 
    {
    if (SUCCESS != T_Super::_InitContextForView())
        return ERROR;

    BeAssert(m_viewport); // must call Attach!
    m_trans.InitFrom(nullptr != m_params.m_rMatrix ? *m_params.m_rMatrix : m_viewport->GetRotMatrix());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool FitContext::IsRangeContained(DRange3dCR range)
    {
    Frustum box(range);
    box.Multiply(m_trans);
    m_lastRange = box.ToRange(); // view aligned range

    if (m_fitRange.IsNull())  // NOTE: test this AFTER setting m_lastRange!
        return false;

    if (m_params.m_fitDepthOnly)
        return (m_lastRange.low.z > m_fitRange.low.z) && (m_lastRange.high.z < m_fitRange.high.z);

    return m_lastRange.IsContained(m_fitRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Result FitContext::_CheckNodeRange(ScanCriteriaCR criteria, DRange3dCR range, bool is3d) 
    {
    if (ScanCriteria::Result::Fail == T_Super::_CheckNodeRange(criteria, range, is3d))
        return ScanCriteria::Result::Fail;

    return IsRangeContained(range) ? ScanCriteria::Result::Fail : ScanCriteria::Result::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* convert the view context polyhedron to scan parameters in the scanCriteria.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool FitContext::_ScanRangeFromPolyhedron() 
    {
    if (!m_params.m_limitByVolume)
        {
        // Rather than no range test - use a big range so range tree still gets used (and we can reject nodes).
        DRange3d bigRange;
        
        bigRange.low.x = bigRange.low.y = bigRange.low.z = -1.0e20;
        bigRange.high.x = bigRange.high.y = bigRange.high.z = 1.0e20;
        m_scanCriteria.SetRangeTest(&bigRange);

        return true;
        }

    return T_Super::_ScanRangeFromPolyhedron();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FitContext::ExtendFitRange(ElementAlignedBox3dCR elementBox, TransformCR placement)
    {
    Frustum box(elementBox);
    box.Multiply(Transform::FromProduct(m_trans, placement)); // put 8 corners from element coordintes to view coordinates
    m_fitRange.Extend(box.m_pts, 8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FitContext::_VisitGeometry(GeometrySourceCR source) 
    {
    // NOTE: Just use element aligned box instead of drawing geometry.
    bool is3d = (nullptr != source.ToGeometrySource3d());
    ExtendFitRange(is3d ? source.ToGeometrySource3d()->GetPlacement().GetElementBox() : 
                          ElementAlignedBox3d(source.ToGeometrySource2d()->GetPlacement().GetElementBox()),
                   source.GetPlacementTransform());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FitContext::AcceptRangeElement(DgnElementId id)
    {
    if (!m_params.m_useElementAlignedBox)
        m_fitRange.Extend(m_lastRange);
    else
        VisitElement(id, true);
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct FitQuery : QueryViewController::SpatialQuery
    {
    FitContextR m_context;
    virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;

public:
    FitQuery(QueryViewController::SpecialElements const* special, FitContextR context, ClipPrimitiveCP volume) : QueryViewController::SpatialQuery(special, volume), m_context(context)
        {
        if (context.m_params.m_limitByVolume)
            SetFrustum(context.GetFrustum());
        }
    };

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
int FitQuery::_TestRTree(RTreeMatchFunction::QueryInfo const& info)
    {
    RTree3dValCP testRange = (RTree3dValCP) info.m_coords;
    
    info.m_within = RTreeMatchFunction::Within::Outside;

    // if we're limiting the elements we test by the view's range (to find the range of the currently visible elements vs. find all elements)
    // then we compare against the Frustum volume and potentially reject elements that are outside the view volume or active volume.
    if (m_context.m_params.m_limitByVolume)
        {
        Frustum box(*testRange);
        auto rangeTest = TestVolume(box, testRange);
        if (RTreeMatchFunction::Within::Outside == rangeTest)
            return  BE_SQLITE_OK;
        }
    else if (m_activeVolume.IsValid())
        {
        Frustum box(*testRange);
        if (ClipPlaneContainment_StronglyOutside == m_activeVolume->ClassifyPointContainment(box.m_pts, 8))
            return  BE_SQLITE_OK;
        }

    DRange3d thisRange = testRange->ToRange();
    if (m_context.IsRangeContained(thisRange))
        return BE_SQLITE_OK; // If range is entirely contained, there's no reason to continue with it (or its children, if this is a node)

    info.m_within = RTreeMatchFunction::Within::Partly; 
    info.m_score  = info.m_level; // to get depth-first traversal
    return  BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::FitComplete ViewController::_ComputeFitRange(FitContextR context)
    {
    context._VisitAllModelElements();
    return FitComplete::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::FitComplete QueryViewController::_ComputeFitRange(FitContextR context)
    {
    FitQuery filter(&m_special, context, m_activeVolume.get());
    filter.Start(*this);

    if (m_noQuery)
        {
        AutoRestore<bool> saveElemRange(&context.m_params.m_useElementAlignedBox, true); // m_lastRange won't be initialized from scan callback...

        // we're only showing a fixed set of elements. Don't perform a query, just get the results (created in ctor of RangeQuery)
        for (auto const& curr : m_special.m_always)
            {
            if (filter.TestElement(curr))
                context.AcceptRangeElement(curr);
            }

        return FitComplete::Yes;
        }    

    DgnElementId thisId;
    while ((thisId = filter.StepRtree()).IsValid())
        {
        if (filter.TestElement(thisId))
            context.AcceptRangeElement(thisId);
        }
    
    // Allow models to participate in fit
    for (DgnModelId modelId : GetViewedModels())
        {
        DgnModelPtr model = GetDgnDb().Models().GetModel(modelId);
        auto geomModel = model.IsValid() ? model->ToGeometricModelP() : nullptr;

        if (nullptr != geomModel)
            geomModel->_OnFitView(context);
        }

    return FitComplete::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::ComputeViewRange(DRange3dR range, FitViewParams& params) 
    {
    if (ViewportStatus::Success != SetupFromViewController()) // can't proceed if viewport isn't valid (e.g. not active)
        return ERROR;

    FitContext context(params);
    if (SUCCESS != context.Attach(this, DrawPurpose::FitView))
        return ERROR;

    m_viewController->_ComputeFitRange(context);

    range = context.m_fitRange;
    return range.IsNull() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::ComputeFittedElementRange(DRange3dR range, DgnElementIdSet const& elements, RotMatrixCP rMatrix)
    {
    FitViewParams params;
    params.m_rMatrix = rMatrix; // Old function had this feature. So retaining it

    FitContext context(params);

    if (SUCCESS != context.Attach(this, DrawPurpose::FitView))
        return ERROR;

    for (DgnElementId elemId : elements)
        {
        DgnElementCPtr elem = context.GetDgnDb().Elements().GetElement(elemId);

        if (!elem.IsValid())
            continue;

        GeometrySourceCP geomElem = elem->ToGeometrySource();
        if (nullptr != geomElem)
            context.VisitGeometry(*geomElem);
        }
    
    range = context.m_fitRange;
    return range.IsNull() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::DetermineVisibleDepthNpc(double& lowNpc, double& highNpc, DRange3dCP subRectNpc)
    {
    FitViewParams params;
    params.m_fitDepthOnly = true;
    params.m_limitByVolume = true;

    FitContext context(params);
    if (subRectNpc)
        context.SetSubRectNpc(*subRectNpc);

    if (SUCCESS != context.Attach(this, DrawPurpose::FitView))
        return ERROR;

    m_viewController->_ComputeFitRange(context);

    DRange3d range = context.m_fitRange;
    if (range.IsNull())
        {
        lowNpc = 0.0;
        highNpc = 1.0;
        return ERROR;
        }

    Frustum corner(range);

    DPoint3d orgView;
    m_rotMatrix.Multiply(&orgView, &m_viewOrg, 1);

    // limit values between front plane and back plane. Otherwise NPC doesn't work in camera views.
    double min = orgView.z;
    double max = min + m_viewDelta.z;

    for (int i=0; i<8; ++i)
        LIMIT_RANGE(min, max, corner.m_pts[i].z);

    m_rotMatrix.MultiplyTranspose(corner.m_pts, corner.m_pts, 8);
    WorldToNpc(corner.m_pts, corner.m_pts, 8);

    range = corner.ToRange();
    lowNpc = range.low.z;
    highNpc = range.high.z;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnViewport::ComputeVisibleDepthRange(double& minDepth, double& maxDepth, bool ignoreViewExtent)
    {
    FitViewParams params;
    params.m_fitDepthOnly = true;
    params.m_limitByVolume = !ignoreViewExtent;

    FitContext context(params);
    if (SUCCESS != context.Attach(this, DrawPurpose::FitView))
        return ERROR;

    m_viewController->_ComputeFitRange(context);

    DRange3d range = context.m_fitRange;
    if (range.IsNull())
        return ERROR;

    minDepth = range.low.z;
    maxDepth = range.high.z;

    return SUCCESS;
    }
