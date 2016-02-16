/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RangeContext.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*=================================================================================**//**
* Context to caclulate the range of all elements either within a view or from the view's non-range criteria.
* @bsiclass                                                     RayBentley    09/06
+===============+===============+===============+===============+===============+======*/
struct Dgn::FitContext : NullContext
{
    DEFINE_T_SUPER(NullContext)
    FitViewParams   m_params;
    Transform       m_trans;        // usually view transform 
    DRange3d        m_fitRange;     // union of all view-aligned element ranges
    DRange3d        m_lastRange;    // last view-aligned range tested

    void AcceptRangeElement(DgnElementId id);
    bool IsRangeContained(DRange3dCR range);
    virtual StatusInt _InitContextForView() override;
    virtual StatusInt _VisitGeometry(GeometrySourceCR source) override;
    virtual bool _ScanRangeFromPolyhedron() override;
    virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR criteria, DRange3dCR range, bool is3d) override;
    FitContext(FitViewParams const& params) : m_params(params) {m_fitRange.Init();}
};

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
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool FitContext::IsRangeContained(DRange3dCR range)
    {
    Frustum box(range);
    box.Multiply(m_trans);
    m_lastRange = box.ToRange();

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
* @bsimethod                                                    RayBentley      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FitContext::_VisitGeometry(GeometrySourceCR source) 
    {
    // NOTE: Can just use element aligned box instead of drawing element geometry...
    bool is3d = (nullptr != source.ToGeometrySource3d());
    ElementAlignedBox3d elementBox = (is3d ? source.ToGeometrySource3d()->GetPlacement().GetElementBox() : ElementAlignedBox3d(source.ToGeometrySource2d()->GetPlacement().GetElementBox()));

    Frustum box(elementBox);
    box.Multiply(source.GetPlacementTransform());
    box.Multiply(m_trans);

    m_fitRange.Extend(box.m_pts, 8);
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
struct FitQuery : DgnQueryView::SpatialQuery
    {
    FitContextR m_context;
    virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;

public:
    FitQuery(DgnQueryView::SpecialElements const* special, FitContextR context) : DgnQueryView::SpatialQuery(special), m_context(context) 
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

    // if we're limiting the elements we test by the view's range (to find the range of the currently visible elements vs. find all elements)
    // then we compare against the Frustum volume and (potentially)
    if (m_context.m_params.m_limitByVolume)
        {
        Frustum box(*testRange);
        auto rangeTest = TestVolume(box, testRange);
        if (RTreeMatchFunction::Within::Outside == rangeTest)
            {
            info.m_within = RTreeMatchFunction::Within::Outside;
            return  BE_SQLITE_OK;
            }
        }

    DRange3d thisRange = testRange->ToRange();
    if (m_context.IsRangeContained(thisRange))
        info.m_within = RTreeMatchFunction::Within::Outside; // If this range is entirely contained there is no reason to continue (it cannot contribute to the fit)
    else
        {
        info.m_within = RTreeMatchFunction::Within::Partly; 
        info.m_score  = info.m_level; // to get depth-first traversal
        }

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
ViewController::FitComplete DgnQueryView::_ComputeFitRange(FitContextR context)
    {
    FitQuery filter(&m_special, context);
    filter.Start(*this);

    DgnElementId thisId;
    while ((thisId = filter.StepRtree()).IsValid())
        {
        if (filter.TestElement(thisId))
            context.AcceptRangeElement(thisId);
        }
    return FitComplete::Yes;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController::FitComplete DgnQueryView::_ComputeFitRange(FitContextR context)
    {
    range = GetViewedExtents();
    Transform  transform;
    transform.InitFrom((nullptr == params.m_rMatrix) ? vp.GetRotMatrix() : *params.m_rMatrix);
    transform.Multiply(range, range);

    return FitComplete::Yes;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/06
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
