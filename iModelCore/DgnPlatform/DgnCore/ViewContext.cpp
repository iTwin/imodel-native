/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "UpdateLogging.h"

enum FrustCorners
    {
    FRUST_Org    = 0,
    FRUST_X      = 1,
    FRUST_Y      = 2,
    FRUST_Z      = 3,
    FRUST_COUNT  = 4,
    };

static DRange3d const s_fullNpcRange =
    {
    {0.0, 0.0, 0.0},
    {1.0, 1.0, 1.0}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ViewContext()
    {
    m_viewport    = NULL;
    m_IViewDraw   = NULL;
    m_IDrawGeom   = NULL;
    m_ICachedDraw = NULL;

    m_scanCriteria  = NULL;
    m_purpose       = DrawPurpose::NotSpecified;
    m_arcTolerance  = .01;
    m_patternScale  = 1.0;
    m_minLOD        = DEFAULT_MINUMUM_LOD;

    m_isAttached                = false;
    m_blockAsyncs               = false;
    m_blockIntermediatePaints   = false;
    m_is3dView                  = true; // Changed default to 3d...
    m_creatingCacheElem         = false;
    m_useNpcSubRange            = false;
    m_filterLOD                 = FILTER_LOD_ShowRange;
    m_parentRangeResult         = RangeResult::Overlap;

    m_dgnDb = nullptr;

    m_ignoreScaleForDimensions  = false;
    m_ignoreScaleForMultilines  = false;
    m_applyRotationToDimView    = false;
    m_wantMaterials             = false;
    m_useCachedGraphics         = true;

    m_startTangent = m_endTangent = nullptr;

    m_drawingClipElements       = false;
    m_ignoreViewRange           = false;
    m_displayStyleStackMark     = 0;
    m_edgeMaskState             = EdgeMaskState_None;
    m_hiliteState               = DgnElement::Hilited::None;
    m_isCameraOn                = false;
    m_edgeMaskState             = EdgeMaskState_None;

    m_rasterDisplayParams.SetFlags(0);

    m_scanRangeValid        = false;
    m_levelOfDetail         = 1.0;

    m_worldToNpc.InitIdentity();
    m_worldToView.InitIdentity();

    // Draw any plane
    ResetRasterPlane();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::~ViewContext()
    {
    BeAssert(!m_isAttached);

    DELETE_AND_CLEAR(m_scanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP ViewContext::_GetViewTarget()
    {
    return NULL == GetViewport() ? NULL : GetViewport()->GetViewController().GetTargetModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToNpc(DPoint3dP npcVec, DPoint3dCP screenVec, int nPts) const
    {
    ViewToWorld(npcVec, screenVec, nPts);
    m_worldToNpc.M0.MultiplyAndRenormalize(npcVec, npcVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToView(DPoint3dP viewVec, DPoint3dCP npcVec, int nPts) const
    {
    NpcToWorld(viewVec, npcVec, nPts);
    WorldToView(viewVec, viewVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const
    {
    m_worldToNpc.M1.MultiplyAndRenormalize(worldPts, npcPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::InitDisplayPriorityRange()
    {
    m_displayPriorityRange[0] = (m_is3dView ? 0 : -MAX_HW_DISPLAYPRIORITY);
    m_displayPriorityRange[1] = (m_is3dView ? 0 : MAX_HW_DISPLAYPRIORITY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_InitContextForView()
    {
    BeAssert(0 == GetTransClipDepth());

    m_elemMatSymb.Init();
    m_ovrMatSymb.Clear();
    m_rasterDisplayParams.SetFlags(0);

    m_worldToNpc  = *m_viewport->GetWorldToNpcMap();
    m_worldToView = *m_viewport->GetWorldToViewMap();
    m_transformClipStack.Clear();
    m_scanRangeValid = false;

    _PushFrustumClip();

    // Note - Don't set CurrentDisplayStyle here (by calling RefreshCurrentDisplayStyle).
    // as it the root display style is only ever sent (and therefore stored in the
    // viewOutput) in DrawContext::_InitContextForView. Nor can we clear it here
    // (It is set to NULL in the constructor). as that would overwrite the value when
    // it is correctly set in DrawContext. TR# 329406 - The display style wth
    // a proxy handler would be used if CVE proxy was last thing displayed.
    m_displayStyleStackMark = 0;

    SetDgnDb(GetViewport()->GetViewController().GetDgnDb());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Frustum ViewContext::GetFrustum()
    {
    Frustum frustum;
    DPoint3dP frustPts = frustum.GetPtsP();

    if (m_useNpcSubRange)
        {
        m_npcSubRange.Get8Corners(frustPts);
        }
    else
        {
        if (NULL == m_viewport)
            s_fullNpcRange.Get8Corners(frustPts);
        else
            frustum = m_viewport->GetFrustum(DgnCoordSystem::Npc, true);
        }

    m_worldToNpc.M1.MultiplyAndRenormalize(frustPts, frustPts, NPC_CORNER_COUNT);
    DgnViewport::FixFrustumOrder(frustum);
    return frustum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushFrustumClip()
    {
    if (m_ignoreViewRange)
        return;

    int         nPlanes;
    ClipPlane   frustumPlanes[6];
    ViewFlagsCP viewFlags = GetViewFlags();

    Frustum polyhedron = GetFrustum();

    if (0 != (nPlanes = ClipUtil::RangePlanesFromPolyhedra(frustumPlanes, polyhedron.GetPts(), NULL != viewFlags && !viewFlags->noFrontClip, NULL != viewFlags && !viewFlags->noBackClip, 1.0E-6)))
        m_transformClipStack.PushClipPlanes(frustumPlanes, nPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_InitScanRangeAndPolyhedron()
    {
    // set up scanner search criteria
    _InitScanCriteria();
    _ScanRangeFromPolyhedron();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::PushClipPlanes(ClipPlaneSetCR clipPlanes)
    {
    _PushClip(*ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromClipPlanes(clipPlanes)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushClip(ClipVectorCR clip)
    {
    m_transformClipStack.PushClip(clip);

    for (ClipPrimitivePtr const& primitive: clip)
        {
        GetIDrawGeom()._PushTransClip(NULL, primitive->GetClipPlanes());
        m_transformClipStack.IncrementPushedToDrawGeom();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushTransform(TransformCR trans)
    {
    m_transformClipStack.PushTransform(trans);
    GetIDrawGeom()._PushTransClip(&trans , NULL);
    m_transformClipStack.IncrementPushedToDrawGeom();
    InvalidateScanRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushViewIndependentOrigin(DPoint3dCP origin)
    {
    Transform   viTrans;
    GetViewIndependentTransform(&viTrans, origin);
    _PushTransform(viTrans);
    m_transformClipStack.SetViewIndependent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PopTransformClip()
    {
    if (m_transformClipStack.IsEmpty())
        {
        BeAssert(false);
        return;
        }

    m_transformClipStack.Pop(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DirectPushTransClipOutput(IDrawGeomR drawGeom, TransformCP trans, ClipPlaneSetCP clip)
    {
    drawGeom._PushTransClip(trans, clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DirectPopTransClipOutput(IDrawGeomR drawGeom)
    {
    drawGeom._PopTransClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetCurrentElement(GeometricElementCP element)
    {
    if (nullptr == element)
        GetIDrawGeom().PopMethodState();
    else
        GetIDrawGeom().PushMethodState();

    m_currentElement = (DgnElementP) element;
    }

/*---------------------------------------------------------------------------------**//**
* Prepare this context to work on the given project when getting project from ViewController.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetDgnDb(DgnDbR dgnDb)
    {
    m_dgnDb = &dgnDb;
    InitDisplayPriorityRange();
    _SetupScanCriteria();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_InitScanCriteria()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetupScanCriteria()
    {
    if (NULL == m_scanCriteria)
        return;

    DgnViewportP vp = GetViewport();
    if (NULL != vp)
        m_scanCriteria->SetCategoryTest(vp->GetViewController().GetViewedCategories());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AllocateScanCriteria()
    {
    if (NULL == m_scanCriteria)
        m_scanCriteria = new ScanCriteria;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_Attach(DgnViewportP viewport, DrawPurpose purpose)
    {
    if (NULL == viewport)
        return  ERROR;

    if (IsAttached())
        {
        BeAssert(!IsAttached());
        return  ERROR;
        }

    m_isAttached = true;
    _AllocateScanCriteria();

    m_viewport = viewport;
    _SetupOutputs();

    m_purpose = purpose;
    ClearAborted();

    m_minLOD = viewport->GetMinimumLOD();
    m_filterLOD = FILTER_LOD_ShowRange;
    m_isCameraOn = viewport->IsCameraOn();

    m_is3dView = viewport->Is3dView();
    m_useCachedGraphics = true;

    SetViewFlags(viewport->GetViewFlags());

    m_arcTolerance = 0.1;
    m_parentRangeResult = RangeResult::Overlap;

    return _InitContextForView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_Detach()
    {
    BeAssert(IsAttached());

    m_isAttached = false;

    m_transformClipStack.PopAll(*this);
    m_currentElement = NULL;

    /* m_IDrawGeom and m_IViewDraw are not typically NULL so the Get methods return references.
       However, there is a hack in SymbolContext::_Detach that NULLs them out specifically
       to prevent this method from modifying them */
    if (NULL != m_IDrawGeom)
        m_IDrawGeom->ActivateOverrideMatSymb(NULL);     // clear any overrides

    // _EmptySymbolCache(); not yet in Graphite
    UpdateLogging::RecordDetach();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    GetLocalToView().Multiply(viewPts, localPts, NULL, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    DMatrix4dCR  localToView = GetLocalToView();

    if (m_isCameraOn)
        localToView.MultiplyAndRenormalize(viewPts, localPts, nPts);
    else
        localToView.MultiplyAffine(viewPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const
    {
    GetViewToLocal().MultiplyAndNormalize(localPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const
    {
    GetViewToLocal().MultiplyAndRenormalize(localPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToView.M0.Multiply(viewPts, worldPts, NULL, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    m_worldToView.M0.MultiplyAndRenormalize(viewPts, worldPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToView(Point2dP viewPts, DPoint3dCP worldPts, int nPts) const
    {
    DPoint3d  tPt;
    DPoint4d  t4dPt;

    for (int i=0; i<nPts; i++)
        {
        WorldToView(&t4dPt, worldPts+i, 1);

        t4dPt.GetProjectedXYZ (tPt);

        (viewPts+i)->x = (long) tPt.x;
        (viewPts+i)->y = (long) tPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const
    {
    m_worldToView.M1.MultiplyAndNormalize(worldPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const
    {
    m_worldToView.M1.MultiplyAndRenormalize(worldPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::LocalToWorld(DPoint3dP worldPts, DPoint3dCP localPts, int nPts) const
    {
    Transform   localToWorld;

    if (SUCCESS == m_transformClipStack.GetTransform(localToWorld))
        localToWorld.Multiply(worldPts, localPts, nPts);
    else
        memcpy(worldPts, localPts, nPts * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::WorldToLocal(DPoint3dP localPts, DPoint3dCP worldPts, int nPts) const
    {
    Transform   worldToLocal;

    if (SUCCESS == m_transformClipStack.GetInverseTransform(worldToLocal))
        worldToLocal.Multiply(localPts, worldPts, nPts);
    else
        memcpy(localPts, worldPts, nPts * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::GetViewIndependentTransform(TransformP trans, DPoint3dCP originLocal)
    {
    RotMatrix   rMatrix;
    DgnViewportP vp = GetViewport();
    if (NULL != vp)
        {
        // get two vectors from origin in VIEW x and y directions
        DPoint4d    screenPt[2];
        LocalToView(screenPt, originLocal, 1);

        DPoint3d viewSize;
        Frustum viewBox = vp->GetFrustum(DgnCoordSystem::View, true);
        viewSize.DifferenceOf(viewBox.GetCorner(NPC_111), viewBox.GetCorner(NPC_000));

        screenPt[1] = screenPt[0];
        screenPt[0].x += viewSize.x;
        screenPt[1].y += viewSize.y;

        // convert to local coordinates
        DPoint3d    localPt[2];
        ViewToLocal(localPt, screenPt, 2);

        // if we're in a 2d view, we remove any fuzz
        if (!m_is3dView)
            localPt[0].z = localPt[1].z = originLocal->z;

        DVec3d  u, v;
        u.NormalizedDifference(*localPt, *originLocal);
        v.NormalizedDifference(localPt[1], *originLocal);

        // convert to rmatrix
        rMatrix.InitFrom2Vectors(u, v);
        }
    else
        {
        rMatrix.InitIdentity();
        }

    // get transform about origin
    trans->InitFromMatrixAndFixedPoint(rMatrix, *originLocal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BSI                             04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::SetWantMaterials(bool wantMaterials)
    {
    bool    prevWantMaterials = m_wantMaterials;

    m_wantMaterials = wantMaterials;

    return prevWantMaterials;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool QvUnsizedKey::Matches(QvUnsizedKeyCR other) const
    {
    if (m_transformKey != other.m_transformKey || m_qvIndex != other.m_qvIndex)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool QvUnsizedKey::IsNull() const {return m_transformKey == 0;}

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
struct QvSizedKey
{
    QvUnsizedKey    m_unsizedKey;
    double          m_low;
    double          m_high;

public:
    QvSizedKey(double size, double dependentRatio, QvUnsizedKeyCR unsizedKey) : m_unsizedKey(unsizedKey)
        {
        if (0.0 == dependentRatio)
            {
            m_low = -1.0;
            m_high = 1.0E8;
            }
        else
            {
            m_low  = size / dependentRatio;
            m_high = size * dependentRatio;
            }

        }

    inline bool LessThan(QvSizedKey const& other) const { return m_low < other.m_low; }
    void DeleteQvElem(QvElem* qvElem) { if (m_unsizedKey.OwnsQvElem()) T_HOST.GetGraphicsAdmin()._DeleteQvElem(qvElem);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool Equal(QvSizedKey const& other) const
    {
    return other.m_low > m_low && other.m_high < m_high && m_unsizedKey.Matches(other.m_unsizedKey);
    }
};

static DgnElement::AppData::Key s_cacheSetKey;
/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
struct QvElemCacheSet : QvElemSet<QvSizedKey>
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
public: QvElemCacheSet(HeapZone& zone) : QvElemSet<QvSizedKey> (zone) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* Find(QvUnsizedKeyP* foundKey, double size, QvUnsizedKeyCR unsizedKey)
    {
    for (Entry* thisEntry = m_entry; NULL != thisEntry; thisEntry = thisEntry->m_next)
        {
        if (thisEntry->m_key.m_unsizedKey.Matches(unsizedKey) &&
            size >= thisEntry->m_key.m_low && size <= thisEntry->m_key.m_high)
            {
            if (NULL != foundKey)
                *foundKey = &thisEntry->m_key.m_unsizedKey;

            return thisEntry->m_qvElem;
            }
        }

    return nullptr;
    }

}; // QvElemCacheSet

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
*
*  Returns a key that is produced by transforming a (constant) random point and summing the
*   coordinates.  This should return the same key for identical localTransforms - it is
*   essentially a hash of the transform.  This is appropriate for use as part of a cache
*   key when we want to distinguish between two representations with different display
*   path transforms (as in more than one representation of a cell).  It is currently
*   used to distinguish XGraphics cache keys and also thematic keys.
*
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ViewContext::GetLocalTransformKey() const
    {
    Transform   localToWorld;

    if (SUCCESS != m_transformClipStack.GetTransform(localToWorld))
        return 0;

    static      DPoint3d  s_randomLocalToWorldTransformRefPoint = { 1234567.0, 7654321.0, 233425.0};
    DPoint3d    transformedPoint;

    localToWorld.Multiply(&transformedPoint, &s_randomLocalToWorldTransformRefPoint, 1);

    return (uint32_t) transformedPoint.x + (uint32_t) transformedPoint.y + (uint32_t) transformedPoint.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* StrokeElementForCache::_GetQvElem(double pixelSize) const
    {
    QvElemP         qvElem;
    QvUnsizedKey    unsizedKey = QvUnsizedKey(0, _GetQvIndex());
    QvElemCacheSet* cacheSet;

    // Search CacheSet first to find any conditional drawn elements.
    if (nullptr != (cacheSet = (QvElemCacheSet*) m_element.FindAppData(s_cacheSetKey)) &&
        nullptr != (qvElem = cacheSet->Find(NULL, pixelSize, unsizedKey)))
        return qvElem;

    return unsizedKey.IsNull() ? m_element.GetQvElem(_GetQvIndex()) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void StrokeElementForCache::_SaveQvElem(QvElemP qvElem, double pixelSize, double sizeDependentRatio) const
    {
    QvUnsizedKey    unsizedKey = QvUnsizedKey(0, _GetQvIndex());

    if (0.0 == sizeDependentRatio && unsizedKey.IsNull())
        {
        (const_cast <GeometricElementR> (m_element)).SetQvElem(qvElem, _GetQvIndex());
        return;
        }

    QvElemCacheSet* cacheSet = (QvElemCacheSet*) m_element.FindAppData(s_cacheSetKey);

    if (nullptr == cacheSet)
        {
        HeapZone& zone = m_element.GetHeapZone();
        cacheSet = new QvElemCacheSet(zone);
        m_element.AddAppData(s_cacheSetKey, cacheSet);
        }

    cacheSet->Add(QvSizedKey(pixelSize, sizeDependentRatio, unsizedKey), qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransientCachedGraphics::~TransientCachedGraphics() {if (nullptr != m_qvElem) T_HOST.GetGraphicsAdmin()._DeleteQvElem(m_qvElem);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILineStyleCP ViewContext::_GetCurrLineStyle(LineStyleSymbP* symb)
    {
    LineStyleSymbR  tSymb = (m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_Style) ? m_ovrMatSymb.GetMatSymbR().GetLineStyleSymbR() : m_elemMatSymb.GetLineStyleSymbR();

    if (symb)
        *symb = &tSymb;

    return tSymb.GetILineStyle();
    }

/*---------------------------------------------------------------------------------**//**
* convert the view context polyhedron to scan parameters in the scanCriteria.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_ScanRangeFromPolyhedron()
    {
    Frustum polyhedron = GetFrustum();

    WorldToLocal(polyhedron.GetPtsP(), polyhedron.GetPts(), 8);

    // get enclosing bounding box around polyhedron (outside scan range).
    DRange3d scanRange = polyhedron.ToRange();

    if (!Is3dView())
        {
        scanRange.low.z = -1;
        scanRange.high.z = 1;
        }

    if (m_scanCriteria)
        {
        if (RangeResult::Inside == m_parentRangeResult)
            m_scanCriteria->SetRangeTest(NULL);
        else
            {
            m_scanCriteria->SetRangeTest(&scanRange);

            // if we're doing a skew scan, get the skew parameters
            if (Is3dView())
                {
                DRange3d skewRange;

                // get bounding range of front plane of polyhedron
                skewRange.InitFrom(polyhedron.GetPts(), 4);

                // get unit bvector from front plane to back plane
                DVec3d      skewVec = DVec3d::FromStartEndNormalize(polyhedron.GetCorner(0), polyhedron.GetCorner(4));

                // check to see if it's worthwhile using skew scan (skew bvector not along one of the three major axes */
                int alongAxes = (fabs(skewVec.x) < 1e-8);
                alongAxes += (fabs(skewVec.y) < 1e-8);
                alongAxes += (fabs(skewVec.z) < 1e-8);

                if (alongAxes < 2)
                    m_scanCriteria->SetSkewRangeTest(&scanRange, &skewRange, &skewVec);
                }
            }
        }
    m_scanRangeValid = true;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* make sure the scan range and range planes are in synch with the polyhedron (if not, calculate them from the
* current polyhedron.
* @bsimethod                                                    KeithBentley    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::ValidateScanRange()
    {
    return m_scanRangeValid ? true : _ScanRangeFromPolyhedron();
    }

/*---------------------------------------------------------------------------------**//**
* mark the scan range and range planes as invalid (out-of-synch with current polyhedron).
* @bsimethod                                                    KeithBentley    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::InvalidateScanRange()     { m_scanRangeValid = false; }

/*---------------------------------------------------------------------------------**//**
* Test an element against the current scan range using the range planes.
* @return true if the element is outside the range and should be ignored.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_FilterRangeIntersection(GeometricElementCR element)
    {
    if (RangeResult::Inside == m_parentRangeResult)
        return false;

    if (RangeResult::Outside == m_parentRangeResult)
        return true;

    return ClipPlaneContainment_StronglyOutside == m_transformClipStack.ClassifyRange(element.CalculateRange3d(), element.Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_OutputElement(GeometricElementCR element)
    {
    ResetContextOverrides();

    if (m_viewport)
        return m_viewport->GetViewControllerR()._DrawElement(*this, element);

    element._Draw(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddViewOverrides(OvrMatSymbR ovrMatSymb)
    {
    // NOTE: ElemDisplayParams/ElemMatSymb ARE NOT setup at this point!
    ViewFlagsCP viewFlags = GetViewFlags();

    if (NULL == viewFlags)
        return;

    if (!viewFlags->weights)
        ovrMatSymb.SetWidth(1);

    if (!viewFlags->styles)
        ovrMatSymb.SetRasterPattern(DgnViewport::GetDefaultIndexedLinePattern(0));

    if (!viewFlags->transparency)
        {
        ovrMatSymb.SetLineTransparency(0);
        ovrMatSymb.SetFillTransparency(0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_AddContextOverrides(OvrMatSymbR ovrMatSymb)
    {
    // Modify m_ovrMatSymb for view flags...
    _AddViewOverrides(ovrMatSymb); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_CookDisplayParamsOverrides(ElemDisplayParamsR elParams, OvrMatSymbR ovrMatSymb)
    {
    // if no overrides are set there is nothing to do...
    if (MATSYMB_OVERRIDE_None == ovrMatSymb.GetFlags())
        return;

    _ModifyPreCook(elParams); // Allow context to modify elParams before cooking...

    // "cook" the display params into a OvrMatSymb
    ovrMatSymb.GetMatSymbR().FromResolvedElemDisplayParams(elParams, *this, NULL, NULL);

    // Add any overrides specific to the view/context...
    _AddContextOverrides(ovrMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::CookDisplayParamsOverrides()
    {
    _CookDisplayParamsOverrides(m_currDisplayParams, m_ovrMatSymb);

    // Activate the ovrMatsymb in the IDrawGeom
    GetIDrawGeom().ActivateOverrideMatSymb(&m_ovrMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_ModifyPreCook(ElemDisplayParamsR elParams)
    {
    elParams.Resolve(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_CookDisplayParams(ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb)
    {
    _ModifyPreCook(elParams); // Allow context to modify elParams before cooking...

    // "cook" the display params into a MatSymb
    elMatSymb.FromResolvedElemDisplayParams(elParams, *this, m_startTangent, m_endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::CookDisplayParams()
    {
    _CookDisplayParams(m_currDisplayParams, m_elemMatSymb);

    // Activate the matsymb in the IDrawGeom
    GetIDrawGeom().ActivateMatSymb(&m_elemMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ResetContextOverrides()
    {
    m_rasterDisplayParams.SetFlags(ViewContext::RasterDisplayParams::RASTER_PARAM_None); // NEEDSWORK_RASTER_DISPLAY - Not sure how this fits into new continuous update approach?!?

    // NOTE: Context overrides CAN NOT look at m_currDisplayParams or m_elemMatSymb as they are not valid.
    m_ovrMatSymb.Clear();
    _AddContextOverrides(m_ovrMatSymb);
    GetIDrawGeom().ActivateOverrideMatSymb(&m_ovrMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::ElementIsUndisplayed(GeometricElementCR element)
    {
    return (!_WantUndisplayed() && element.IsUndisplayed());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawBox(DPoint3dP box, bool is3d)
    {
    IDrawGeomR  drawGeom = GetIDrawGeom();
    DPoint3d    tmpPts[9];

    if (is3d)
        {
        tmpPts[0] = box[0];
        tmpPts[1] = box[1];
        tmpPts[2] = box[2];
        tmpPts[3] = box[3];

        tmpPts[4] = box[5];
        tmpPts[5] = box[6];
        tmpPts[6] = box[7];
        tmpPts[7] = box[4];

        tmpPts[8] = box[0];

        // Draw a "saddle" shape to accumulate correct dirty region, simple lines can be clipped out when zoomed in...
        drawGeom.DrawLineString3d(9, tmpPts, NULL);

        // Draw missing connecting lines to complete box...
        drawGeom.DrawLineString3d(2, DSegment3d::From(box[0], box[3]).point, NULL);
        drawGeom.DrawLineString3d(2, DSegment3d::From(box[4], box[5]).point, NULL);
        drawGeom.DrawLineString3d(2, DSegment3d::From(box[1], box[7]).point, NULL);
        drawGeom.DrawLineString3d(2, DSegment3d::From(box[2], box[6]).point, NULL);
        return;
        }

    tmpPts[0] = box[0];
    tmpPts[1] = box[1];
    tmpPts[2] = box[2];
    tmpPts[3] = box[3];
    tmpPts[4] = box[0];

    drawGeom.DrawLineString3d(5, tmpPts, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitElement(GeometricElementCR element)
    {
    if (_CheckStop())
        return ERROR;

    if (ElementIsUndisplayed(element))
        return SUCCESS;

    _SetCurrentElement(&element);
    _OutputElement(element);

    // Output element or local range for debugging if requested...
    switch (GetDrawPurpose())
        {
        case DrawPurpose::FitView:
        case DrawPurpose::CaptureGeometry:
            break; // Don't do this when trying to compute range or drop!

        default:
            {
            static int s_drawRange; // 0 - Host Setting (Bounding Box Debug), 1 - Bounding Box, 2 - Element Range
            if (m_creatingCacheElem || nullptr == m_viewport || (!s_drawRange && !T_HOST.GetGraphicsAdmin()._WantDebugElementRangeDisplay()))
                break;

            DPoint3d  p[8];
            BoundingBox3d  range = (2 == s_drawRange ? BoundingBox3d(element.CalculateRange3d()) : 
                                   (element.Is3d() ? BoundingBox3d(element.ToElement3d()->GetPlacement().GetElementBox()) : BoundingBox3d(element.ToElement2d()->GetPlacement().GetElementBox())));
            Transform placementTrans = (2 == s_drawRange ? Transform::FromIdentity() : (element.Is3d() ? element.ToElement3d()->GetPlacement().GetTransform() : element.ToElement2d()->GetPlacement().GetTransform()));

            p[0].x = p[3].x = p[4].x = p[5].x = range.low.x;
            p[1].x = p[2].x = p[6].x = p[7].x = range.high.x;
            p[0].y = p[1].y = p[4].y = p[7].y = range.low.y;
            p[2].y = p[3].y = p[5].y = p[6].y = range.high.y;
            p[0].z = p[1].z = p[2].z = p[3].z = range.low.z;
            p[4].z = p[5].z = p[6].z = p[7].z = range.high.z;

            m_ovrMatSymb.SetLineColor(m_viewport->MakeTransparentIfOpaque(m_viewport->AdjustColorForContrast(m_elemMatSymb.GetLineColor(), m_viewport->GetBackgroundColor()), 150));
            m_ovrMatSymb.SetWidth(1);
            m_ovrMatSymb.SetRasterPattern(0);
            _AddContextOverrides(m_ovrMatSymb);
            m_IDrawGeom->ActivateOverrideMatSymb(&m_ovrMatSymb);

            PushTransform(placementTrans);
            DrawBox(p, element.Is3d());
            PopTransformClip();

            ResetContextOverrides();
            break;
            }
        }

    _SetCurrentElement(nullptr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_VisitTransientGraphics(bool isPreUpdate)
    {
    IViewOutput*    output = (IsAttached() ? GetViewport()->GetIViewOutput() : NULL);
    bool            restoreZWrite = (output && isPreUpdate ? output->EnableZWriting(false) : false);

    T_HOST.GetGraphicsAdmin()._CallViewTransients(*this, isPreUpdate);

    if (restoreZWrite)
        output->EnableZWriting(true);
    }

/*---------------------------------------------------------------------------------**//**
* private callback (called from scanner)
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt visitElementFunc(DgnElementCR element, void* inContext, ScanCriteriaR sc)
    {
    GeometricElementCP geomElement = element.ToGeometricElement();
    if (nullptr == geomElement)
        return SUCCESS;
    
    ViewContextR context = *(ViewContext*)inContext;
    return context.VisitElement(*geomElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetScanReturn()
    {
    m_scanCriteria->SetRangeNodeCheck(this);
    m_scanCriteria->SetElementCallback(visitElementFunc, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Result  ViewContext::_CheckNodeRange(ScanCriteriaCR scanCriteria, DRange3dCR testRange, bool is3d)
    {
    return ClipPlaneContainment_StronglyOutside != m_transformClipStack.ClassifyElementRange(testRange, is3d, true) ? ScanCriteria::Result::Pass : ScanCriteria::Result::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsWorldPointVisible(DPoint3dCR worldPoint, bool boresite)
    {
    DPoint3d    localPoint;

    WorldToLocal(&localPoint, &worldPoint, 1);

    return IsLocalPointVisible(localPoint, boresite);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsLocalPointVisible(DPoint3dCR localPoint, bool boresite)
    {
    if (m_transformClipStack.IsEmpty())
        return true;

    if (!boresite)
        return m_transformClipStack.TestPoint(localPoint);

    DVec3d      localZVec;

    if (IsCameraOn())
        {
        DPoint3d        localCamera;
        
        WorldToLocal(&localCamera, &GetViewport()->GetCamera().GetEyePoint(), 1);
        localZVec.NormalizedDifference(localPoint, localCamera);
        }
    else
        {
        DPoint3d        zPoints[2];
        Transform       worldToLocal;

        zPoints[0].Zero();
        zPoints[1].Init(0.0, 0.0, 1.0);

        NpcToWorld(zPoints, zPoints, 2);

        localZVec.NormalizedDifference(zPoints[1], zPoints[0]);

        if (GetCurrWorldToLocalTrans(worldToLocal))
            {
            worldToLocal.MultiplyMatrixOnly(localZVec);
            localZVec.Normalize();
            }
        }

    return  m_transformClipStack.TestRay(localPoint, localZVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::PointInsideClip(DPoint3dCR point)
    {
    return m_transformClipStack.TestPoint(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::GetRayClipIntersection(double& distance, DPoint3dCR origin, DVec3dCR direction)
    {
    return  m_transformClipStack.GetRayIntersection(distance, origin, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_ScanDgnModel(DgnModelP model)
    {
    if (!ValidateScanRange())
        return ERROR;

    m_scanCriteria->SetDgnModel(model);

    return m_scanCriteria->Scan(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitDgnModel(DgnModelP modelRef)
    {
    if (CheckStop())
        return ERROR;

    return _ScanDgnModel(modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetSubRectFromViewRect(BSIRectCP viewRect)
    {
    if (NULL == viewRect)
        return;

    BSIRect tRect = *viewRect;
    tRect.Expand(1);

    DRange3d viewRange;
    viewRange.low.Init(tRect.origin.x, tRect.corner.y, 0.0);
    viewRange.high.Init(tRect.corner.x, tRect.origin.y, 0.0);

    GetViewport()->ViewToNpc(&viewRange.low, &viewRange.low, 2);

    // this is due to fact that y's can be reversed from view to npc
    DRange3d npcSubRect;
    npcSubRect.InitFrom(&viewRange.low, 2);
    SetSubRectNpc(npcSubRect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetSubRectNpc(DRange3dCR subRect)
    {
    m_npcSubRange = subRect;
    m_npcSubRange.low.z  = 0.0;                // make sure entire z range.
    m_npcSubRange.high.z = 1.0;
    m_useNpcSubRange = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::VisitAllViewElements(bool includeTransients, BSIRectCP updateRect)
    {
    ClearAborted();
    if (NULL != updateRect)
        SetSubRectFromViewRect(updateRect);

    _InitScanRangeAndPolyhedron();

    SetScanReturn();
    _VisitAllModelElements(includeTransients);

    m_transformClipStack.PopAll(*this);    // This will cause pushed clip elements to display correctly (outside their clip).

#ifdef WIP_VANCOUVER_MERGE // material
    m_materialAssignmentCache.clear();

    if (!WasAborted() && _WantDgnAttachmentBoundaryDisplay())
        AddAbortTest(drawAttachmentBoundaries(*this, _GetViewRoot(), includeList, includeRefs));
#endif
    return  WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_VisitAllModelElements(bool includeTransients)
    {
    if (includeTransients)
        _VisitTransientGraphics(true);

    PhysicalViewControllerCP physController = m_viewport->GetPhysicalViewControllerCP();
    ClipVectorPtr clipVector = physController ? physController->GetClipVector() : nullptr;
    if (clipVector.IsValid())
        PushClip(*clipVector);

    // The ViewController must orchestrate the display of all of the elements in the view.
    m_viewport->GetViewControllerR().DrawView(*this);

    if (clipVector.IsValid())
        PopTransformClip();

    if (includeTransients) // Give post-update IViewTransients a chance to display even if aborted the element draw...
        _VisitTransientGraphics(false);

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::VisitHit(HitDetailCR hit)
    {
    ClearAborted();
    _InitScanRangeAndPolyhedron();

    return m_viewport->GetViewController().VisitHit(hit, *this);
    }

/*---------------------------------------------------------------------------------**//**
* create a QvElem using an IStrokeForCache stroker.
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::CreateCacheElem(IStrokeForCache& stroker, QvCache* qvCache, double pixelSize, ICachedDrawP cachedDraw)
    {
    BeAssert(!m_creatingCacheElem || nullptr != cachedDraw);

    if (nullptr == cachedDraw)
        cachedDraw = m_ICachedDraw;

    if (nullptr == cachedDraw)
        return nullptr;

    if (nullptr == qvCache)
        qvCache = T_HOST.GetGraphicsAdmin()._GetTempElementCache();

    BeAssert(qvCache);
    cachedDraw->BeginCacheElement(qvCache, m_is3dView, m_is3dView ? 0.0 : stroker._GetDisplayPriority(*this));

    AutoRestore<IDrawGeomP> saveDrawGeom(&m_IDrawGeom, cachedDraw);
    AutoRestore<Byte> savefilter(&m_filterLOD, FILTER_LOD_Off);
    AutoRestore<bool> saveCreatingCache(&m_creatingCacheElem, true);

    try
        {
        stroker._StrokeForCache(*this, pixelSize);
        }
    catch (...)
        {
        }

    QvElem* result = cachedDraw->EndCacheElement();

    if (!WasAborted() || nullptr == result)
        return result;

    T_HOST.GetGraphicsAdmin()._DeleteQvElem(result);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::GetCachedGeometry(IStrokeForCache& stroker, bool& deleteQvElem)
    {
    static double   s_sizeDependentCacheRatio = 3.0;
    double          sizeDependentRatio = s_sizeDependentCacheRatio;
    double          pixelSize = 0.0;

    deleteQvElem = false;

    if (stroker._GetSizeDependentGeometryPossible())
        {
        DRange3d    localRange = stroker._GetRange();
        DPoint3d    localCenter;

        localCenter.Interpolate(localRange.low, 0.5, localRange.high);
        pixelSize = GetPixelSizeAtPoint(&localCenter);
        }

    if (m_creatingCacheElem)
        {
        stroker._StrokeForCache(*this, pixelSize);

        return nullptr;
        }

    bool     useCachedDisplay = _UseCachedDisplay();
    QvElem*  qvElem = nullptr;

    // if there is already a qvElem, use that instead of stroking.
    if (useCachedDisplay)
        qvElem = stroker._GetQvElem(pixelSize);

    if (nullptr == qvElem)
        {
        bool      saveQvElem = _WantSaveQvElem(stroker._GetDrawExpense());
        QvCache*  qvCache = (saveQvElem ? stroker._GetQVCache() : nullptr);

        if (nullptr == qvCache)
            saveQvElem = false;

        if (nullptr == (qvElem = CreateCacheElem(stroker, qvCache, pixelSize)))
            return nullptr;

        if (!stroker._GetSizeDependentGeometryStroked())
            sizeDependentRatio = 0.0;

        if (saveQvElem)
            stroker._SaveQvElem(qvElem, pixelSize, sizeDependentRatio); 
        else
            deleteQvElem = true;
        }

    return qvElem;
    }

/*---------------------------------------------------------------------------------**//**
* Output a displayable that uses QvElems and can be cached in QuickVision.
* Sometimes this function will find that the cached representation already exists, and then it
* simply draws that cached representation. Otherwise it calls the Draw
* method on the displayable and draws (and potentially saves) the resultant QvElem.
* @bsimethod                                                    KeithBentley    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::_DrawCached(IStrokeForCache& stroker)
    {
    bool    deleteQvElem;
    QvElem* qvElem = GetCachedGeometry(stroker, deleteQvElem);

    if (nullptr == qvElem)
        return nullptr;

    m_IViewDraw->DrawQvElem(qvElem);

    if (!deleteQvElem)
        return qvElem;

    T_HOST.GetGraphicsAdmin()._DeleteQvElem(qvElem);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed)
    {
    if (nPts < 1)
        return;

    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle && (nPts > 2 || !pts->IsEqual(pts[1])))
        {
        currLStyle->_GetComponent()->_StrokeLineString(this, currLsSymb, pts, nPts, closed);
        return;
        }

    if (closed)
        GetIDrawGeom().DrawShape3d(nPts, pts, false, range);
    else
        GetIDrawGeom().DrawLineString3d(nPts, pts, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString2d(int nPts, DPoint2dCP pts, double priority, DPoint2dCP range, bool closed)
    {
    if (nPts < 1)
        return;

    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle && (nPts > 2 || !pts->IsEqual(pts[1])))
        {
        currLStyle->_GetComponent()->_StrokeLineString2d(this, currLsSymb, pts, nPts, priority, closed);
        return;
        }

    if (closed)
        GetIDrawGeom().DrawShape2d(nPts, pts, false, priority, range);
    else
        GetIDrawGeom().DrawLineString2d(nPts, pts, priority, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        double      r0, r1, start, sweep;
        RotMatrix   rMatrix;
        DPoint3d    center;

        ellipse.GetScaledRotMatrix(center, rMatrix, r0, r1, start, sweep);
        currLStyle->_GetComponent()->_StrokeArc(this, currLsSymb, &center, &rMatrix, r0, r1, isEllipse ? NULL : &start, isEllipse ? NULL : &sweep, range);
        return;
        }

    GetIDrawGeom().DrawArc3d(ellipse, isEllipse, false, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        double      r0, r1, start, sweep;
        RotMatrix   rMatrix;
        DPoint3d    center;

        ellipse.GetScaledRotMatrix(center, rMatrix, r0, r1, start, sweep);
        center.z = zDepth; // Set priority on center...
        currLStyle->_GetComponent()->_StrokeArc(this, currLsSymb, &center, &rMatrix, r0, r1, isEllipse ? NULL : &start, isEllipse ? NULL : &sweep, NULL);
        return;
        }

    GetIDrawGeom().DrawArc2d(ellipse, isEllipse, false, zDepth, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve3d(MSBsplineCurveCR bcurve)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        currLStyle->_GetComponent()->_StrokeBSplineCurve(this, currLsSymb, &bcurve, NULL);
        return;
        }

    GetIDrawGeom().DrawBSplineCurve(bcurve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve2d(MSBsplineCurveCR bcurve, double zDepth)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        if (0.0 == zDepth)
            {
            currLStyle->_GetComponent()->_StrokeBSplineCurve(this, currLsSymb, &bcurve, NULL);
            return;
            }

        // NOTE: Copy curve and set priority on poles since we won't be drawing cached...
        MSBsplineCurvePtr tmpCurve = bcurve.CreateCopy();
        bool useWeights = tmpCurve->rational && NULL != tmpCurve->GetWeightCP();
        for (int iPoint = 0; iPoint < tmpCurve->params.numPoles; ++iPoint)
            {
            DPoint3d xyz = tmpCurve->GetPole(iPoint);
            if (useWeights)
                xyz.z = zDepth * tmpCurve->GetWeight(iPoint);
            else
                xyz.z = zDepth;
            tmpCurve->SetPole(iPoint, xyz);
            }

        currLStyle->_GetComponent()->_StrokeBSplineCurve(this, currLsSymb, tmpCurve.get(), NULL);
        return;
        }

    GetIDrawGeom().DrawBSplineCurve2d(bcurve, false, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d ViewContext::GetViewToLocal() const
    {
    DMatrix4d  viewToLocal = GetWorldToView().M1;
    Transform  worldToLocalTransform;

    if (SUCCESS == GetCurrWorldToLocalTrans(worldToLocalTransform) && !worldToLocalTransform.IsIdentity())
        {
        DMatrix4d worldToLocal = DMatrix4d::From(worldToLocalTransform);
        viewToLocal.InitProduct(worldToLocal, viewToLocal);
        }

    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d ViewContext::GetLocalToView() const
    {
    DMatrix4d localToView = GetWorldToView().M0;
    Transform localToWorldTransform;

    if (SUCCESS == GetCurrLocalToWorldTrans(localToWorldTransform) && !localToWorldTransform.IsIdentity())
        {
        DMatrix4d   localToWorld = DMatrix4d::From(localToWorldTransform);
        localToView.InitProduct(localToView, localToWorld);
        }

    return localToView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::GetPixelSizeAtPoint(DPoint3dCP inPoint) const
    {
    DPoint3d    vec[2];

    if (NULL != inPoint)
        LocalToView(vec, inPoint, 1); // convert point to pixels
    else
        {
        DPoint3d    center = {.5, .5, .5};   // if they didn't give a point, use center of view.
        NpcToView(vec, &center, 1);
        }

    vec[1] = vec[0];
    vec[1].x += 1.0;

    // Convert pixels back to local coordinates and use the length as tolerance
    ViewToLocal(vec, vec, 2);

    return vec[0].Distance(vec[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_ClearZ()
    {
    GetIViewDraw().ClearZ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::RasterDisplayParams::RasterDisplayParams()
    : m_flags(0), m_contrast(50), m_brightness(50), m_greyScale(false), m_applyBinaryWhiteOnWhiteReversal(false), m_quality(1.0)
    {
    m_backgroundColor = ColorDef::Black();      // Background color for binary image.
    m_foregroundColor = ColorDef::White();      // Foreground color for binary image.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::RasterDisplayParams::operator==(RasterDisplayParams const& rhs) const
    {
    if (this == &rhs)
        return true;

    return (memcmp(this, &rhs, sizeof(*this)) == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::RasterDisplayParams::operator!=(RasterDisplayParams const& rhs) const
    {
    return !(operator==(rhs));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetFlags(uint32_t flags)     
    {
    m_flags = flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetContrast(int8_t value)     
    {
    m_contrast = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Contrast;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetBrightness(int8_t value)   
    {
    m_brightness = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Brightness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetGreyscale(bool value)
    {
    m_greyScale = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_GreyScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetApplyBinaryWhiteOnWhiteReversal(bool value)
    {
    m_applyBinaryWhiteOnWhiteReversal = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_ApplyBinaryWhiteOnWhiteReversal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetEnableGrid(bool value)
    {
    m_enableGrid = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_EnableGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetBackgroundColor(ColorDef value)
    {
    m_backgroundColor = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_BackgroundColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetForegroundColor(ColorDef value)
    {
    m_foregroundColor = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_ForegroundColor;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetQualityFactor(double factor)
    {
    m_quality = factor;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Quality;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ContextMark::Pop()
    {
    if (NULL == m_context)
        return;

    while (m_context->GetTransClipDepth() > m_transClipMark)
        m_context->GetTransformClipStack().Pop(*m_context);

    if (m_pushedRange)
        {
        m_context->SetCurrParentRangeResult(m_parentRangeResult);
        m_pushedRange = false;
        }

    m_context->SetIgnoreScaleForDimensions(m_ignoreScaleForDimensions);
    m_context->SetIgnoreScaleForMultilines(m_ignoreScaleForMultilines);
    m_context->SetApplyRotationToDimView(m_applyRotationToDimView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
inline void ViewContext::ContextMark::SetNow()
    {
    m_transClipMark                 = m_context->GetTransClipDepth();
    m_parentRangeResult             = m_context->GetCurrParentRangeResult();
    m_displayStyleStackMark         = m_context->m_displayStyleStackMark;
    m_pushedRange                   = false;

    m_ignoreScaleForDimensions      = m_context->GetIgnoreScaleForDimensions();
    m_ignoreScaleForMultilines      = m_context->GetIgnoreScaleForMultilines();
    m_applyRotationToDimView        = m_context->GetApplyRotationToDimView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ContextMark::ContextMark(ViewContextP context)
    {
    if (NULL == (m_context = context))
        Init(context);
    else
        SetNow();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsMonochromeDisplayStyleActive()
    {
#if defined (NEEDS_WORK_DGNITEM)
    CookedDisplayStyleCP currDispStyle = _GetCurrentCookedDisplayStyle();

    return (currDispStyle && currDispStyle->StylePresent() && currDispStyle->m_flags.m_elementColor);
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/15
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ViewContext::ResolveNetDisplayPriority(int32_t geomPriority, DgnSubCategoryId subCategoryId, DgnCategories::SubCategory::Appearance* appearanceIn) const
    {
    if (m_is3dView)
        return 0;

    // SubCategory display priority is combined with element priority to compute net display priority. 
    int32_t netPriority = geomPriority;

    if (nullptr == appearanceIn)
        {
        if (!subCategoryId.IsValid())
            return netPriority;

        DgnCategories::SubCategory::Appearance appearance;

        if (nullptr != GetViewport())
            appearance = GetViewport()->GetViewController().GetSubCategoryAppearance(subCategoryId);
        else
            appearance = GetDgnDb().Categories().QuerySubCategory(subCategoryId).GetAppearance();

        netPriority += appearance.GetDisplayPriority();
        }
    else
        {
        netPriority += appearanceIn->GetDisplayPriority();
        }

    int32_t displayRange[2];

    if (GetDisplayPriorityRange(displayRange[0], displayRange[1]))
        LIMIT_RANGE (displayRange[0], displayRange[1], netPriority);

    return netPriority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymb::ElemMatSymb()
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::Init()
    {
    m_lineColor         = ColorDef::Black();
    m_fillColor         = ColorDef::Black();
    m_elementStyle      = 0;
    m_isFilled          = false;
    m_isBlankingRegion  = false;
    m_extSymbID         = 0;
    m_rasterWidth       = 1;
    m_rasterPat         = 0;
    m_patternParams     = nullptr;
    m_gradient          = nullptr;

    m_material.Invalidate();
    m_lStyleSymb.Clear();
    }

#if defined (WIP_PLOTTING)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static Byte screenColor(Byte color, double factor)
    {
    uint32_t tmp = color + (uint32_t) (factor * (255 - color));
    LIMIT_RANGE (0, 255, tmp);
    return  (Byte) tmp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AndrewEdge      08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     applyScreeningFactor(ColorDef* rgb, double screeningFactor)
    {
    rgb->SetRed(screenColor(rgb->GetRed(),   screeningFactor));
    rgb->SetGreen(screenColor(rgb->GetGreen(), screeningFactor));
    rgb->SetBlue(screenColor(rgb->GetBlue(),  screeningFactor));
    }
#endif

#define ACAD_LINEWEIGHT_SIGNATURE   (0x80000000)
#define LINEWEIGHT_SIGNATURE_BITS   (0xff000000)
#define ACAD_LINEWEIGHT_MAX         (211)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t   remapAcadLineWeight(uint32_t lineWeight)
    {
    if (ACAD_LINEWEIGHT_SIGNATURE != (lineWeight & LINEWEIGHT_SIGNATURE_BITS))
        return lineWeight;

    lineWeight = (lineWeight & ~LINEWEIGHT_SIGNATURE_BITS);

    // maximum autocad line weight is 211.
    if (lineWeight > ACAD_LINEWEIGHT_MAX)
        lineWeight = ACAD_LINEWEIGHT_MAX;

#ifdef BEIJING_DGNPLATFORM_WIP_DWG
    double          styleScale, weightScale;
    if (SUCCESS != dwgSaveSettings_getLineCodeAndWeightScale(&styleScale, &weightScale))
        weightScale = DEFAULT_DWG_WEIGHT_SCALE;

    return (uint32_t) (0.5 + 0.5 * weightScale * lineWeight);
#endif
    return lineWeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::FromResolvedElemDisplayParams(ElemDisplayParamsCR elParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
    Init();

    // We store the style index returned from LineStyleSymb::FromResolvedElemDisplayParams in order to
    // provide PlotContext with the information it needs to handle custom line styles that are
    // mapped to cosmetic line styles.  PlotContext is different from DrawContext in that it
    // uses QV extended symbologies for cosmetic line styles rather than pattern codes. TR 225586.
    m_elementStyle = m_lStyleSymb.FromResolvedElemDisplayParams(elParams, context, startTangent, endTangent);

    DgnViewportP vp = context.GetViewport();

    m_rasterPat = (nullptr != vp ? vp->GetIndexedLinePattern(m_elementStyle) : DgnViewport::GetDefaultIndexedLinePattern(m_elementStyle));
    m_rasterWidth = (nullptr != vp ? vp->GetIndexedLineWidth(remapAcadLineWeight(elParams.GetWeight())) : DgnViewport::GetDefaultIndexedLineWidth(remapAcadLineWeight(elParams.GetWeight())));
    m_lineColor = m_fillColor = elParams.GetLineColor(); // NOTE: In case no fill is defined it should be set the same as line color...

    double netElemTransparency = elParams.GetNetTransparency();
    double netFillTransparency = elParams.GetNetFillTransparency();

    if (FillDisplay::Never != elParams.GetFillDisplay())
        {
        if (NULL != elParams.GetGradient())
            {
            m_gradient = GradientSymb::Create();
            m_gradient->CopyFrom(*elParams.GetGradient());

            m_fillColor = ColorDef::White(); // Fill should be set to opaque white for gradient texture...

            if (0 == (m_gradient->GetFlags() & static_cast<int>(GradientFlags::Outline)))
                {
                m_lineColor.SetAlpha(0xff); // Qvis checks for this to disable auto-outline...
                netElemTransparency = 0.0;  // Don't override the fully transparent outline below...
                }
            }
        else
            {
            m_fillColor = elParams.GetFillColor();
            }

        m_isFilled = true;
        m_isBlankingRegion = (FillDisplay::Blanking == elParams.GetFillDisplay());
        }

    m_material = elParams.GetMaterial();

    if (0.0 != netElemTransparency)
        {
        Byte netTransparency = (Byte) (netElemTransparency * 255.0);

        if (netTransparency > 250)
            netTransparency = 250; // Don't allow complete transparency.

        m_lineColor.SetAlpha(netTransparency);
        }

    if (0.0 != netFillTransparency)
        {
        Byte netTransparency = (Byte) (netFillTransparency * 255.0);

        if (netTransparency > 250)
            netTransparency = 250; // Don't allow complete transparency.

        m_fillColor.SetAlpha(netTransparency);
        }

#if defined (WIP_PLOTTING)    
    if (elParams.IsScreeningSet() && (elParams.GetScreening() >= SCREENING_Full) && (elParams.GetScreening() < SCREENING_None))
        {
        double  screeningFactor = (SCREENING_None - elParams.GetScreening()) / SCREENING_None;

        applyScreeningFactor(&m_lineColor, screeningFactor);
        applyScreeningFactor(&m_fillColor, screeningFactor);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::FromNaturalElemDisplayParams(ElemDisplayParamsR elParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
    elParams.Resolve(context);
    FromResolvedElemDisplayParams(elParams, context, startTangent, endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* compare two ElemMatSymb's to see if they're the same.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemMatSymb::operator==(ElemMatSymbCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_lineColor        != m_lineColor        ||
        rhs.m_fillColor        != m_fillColor        ||
        rhs.m_elementStyle     != m_elementStyle     ||
        rhs.m_isFilled         != m_isFilled         ||
        rhs.m_isBlankingRegion != m_isBlankingRegion ||
        rhs.m_extSymbID        != m_extSymbID        ||
        rhs.m_material         != m_material         ||
        rhs.m_rasterWidth      != m_rasterWidth      ||
        rhs.m_rasterPat        != m_rasterPat)
        return false;

    if (!(rhs.m_gradient == m_gradient))
        return false;

#ifdef WIP_VANCOUVER_MERGE // linestyle
    if (!(rhs.m_lStyleSymb == m_lStyleSymb))
        return false;

    if (m_materialDetail.IsValid() && rhs.m_materialDetail.IsValid())
        {
        if (!(m_materialDetail->Equals(*rhs.m_materialDetail)))
            return false;
        }
    else if (m_materialDetail.IsNull() != rhs.m_materialDetail.IsNull())
        return false;
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::SetMaterial(DgnMaterialId material)
    {
    m_material = material;

#ifdef DGNPLATFORM_WIP_MATERIAL
    // Shouldn't need "seedContext" to create geometry map...from DgnGeomPart/GeomStream...not element...
    if (NULL != material &&
        NULL != seedContext &&
        material->NeedsQvGeometryTexture())
        {
        bool                useCellColors;
        EditElementHandle   eh;

        if (seedContext->GetIViewDraw().IsOutputQuickVision() &&
            SUCCESS == material->GetGeometryMapDefinition(eh, useCellColors))
            seedContext->GetIViewDraw().DefineQVGeometryMap(*material, eh, NULL, useCellColors, *seedContext);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymb::ElemMatSymb(ElemMatSymbCR rhs)
    {
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_elementStyle      = rhs.m_elementStyle;
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_extSymbID         = rhs.m_extSymbID;
    m_material          = rhs.m_material;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_rasterPat         = rhs.m_rasterPat;
    m_lStyleSymb        = rhs.m_lStyleSymb;
    m_gradient          = rhs.m_gradient;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymbR ElemMatSymb::operator=(ElemMatSymbCR rhs)
    {
    m_lineColor         = rhs.m_lineColor;
    m_fillColor         = rhs.m_fillColor;
    m_elementStyle      = rhs.m_elementStyle;
    m_isFilled          = rhs.m_isFilled;
    m_isBlankingRegion  = rhs.m_isBlankingRegion;
    m_extSymbID         = rhs.m_extSymbID;
    m_material          = rhs.m_material;
    m_rasterWidth       = rhs.m_rasterWidth;
    m_rasterPat         = rhs.m_rasterPat;
    m_lStyleSymb        = rhs.m_lStyleSymb;
    m_gradient          = rhs.m_gradient;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void OvrMatSymb::Clear()
    {
    SetFlags(MATSYMB_OVERRIDE_None);
    m_matSymb.Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  OvrMatSymb::SetLineStyle(int32_t styleNo, DgnModelR modelRef, DgnModelR styleDgnModel, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
#ifdef WIP_VANCOUVER_MERGE // linestyle
    m_matSymb.GetLineStyleSymbR().FromResolvedStyle(styleNo, modelRef, styleDgnModel, lStyleParams, context, startTangent, endTangent);
    m_flags |= MATSYMB_OVERRIDE_Style;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParams::ElemDisplayParams() {Init();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParams::ElemDisplayParams(ElemDisplayParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;       
    m_geometryClass         = rhs.m_geometryClass;       
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_material              = rhs.m_material;
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;
    m_plotInfo              = rhs.m_plotInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsR ElemDisplayParams::operator=(ElemDisplayParamsCR rhs)
    {
    m_appearanceOverrides   = rhs.m_appearanceOverrides;
    m_categoryId            = rhs.m_categoryId;
    m_subCategoryId         = rhs.m_subCategoryId;
    m_elmPriority           = rhs.m_elmPriority;
    m_netPriority           = rhs.m_netPriority;
    m_weight                = rhs.m_weight;
    m_geometryClass         = rhs.m_geometryClass;       
    m_lineColor             = rhs.m_lineColor;
    m_fillColor             = rhs.m_fillColor;
    m_fillDisplay           = rhs.m_fillDisplay;
    m_elmTransparency       = rhs.m_elmTransparency;
    m_netElmTransparency    = rhs.m_netElmTransparency;
    m_fillTransparency      = rhs.m_fillTransparency;
    m_netFillTransparency   = rhs.m_netFillTransparency;
    m_material              = rhs.m_material;
    m_styleInfo             = rhs.m_styleInfo;
    m_gradient              = rhs.m_gradient;
    m_pattern               = rhs.m_pattern;
    m_plotInfo              = rhs.m_plotInfo;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::Init()
    {
    memset(this, 0, offsetof(ElemDisplayParams, m_material));

    m_styleInfo = nullptr;
    m_gradient  = nullptr;
    m_pattern   = nullptr;
    m_plotInfo  = nullptr;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2015
//---------------------------------------------------------------------------------------
void ElemDisplayParams::ResetAppearance()
    {
    DgnCategoryId categoryId = m_categoryId;
    DgnSubCategoryId subCategoryId = m_subCategoryId;
    
    Init();
    
    SetCategoryId(categoryId);
    SetSubCategoryId(subCategoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemDisplayParams::operator==(ElemDisplayParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_categoryId    != m_categoryId ||
        rhs.m_subCategoryId != m_subCategoryId ||
        rhs.m_elmPriority   != m_elmPriority ||
        rhs.m_netPriority   != m_netPriority)
        return false;

    if (rhs.m_lineColor     != m_lineColor ||
        rhs.m_weight        != m_weight ||
        rhs.m_geometryClass != m_geometryClass)
        return false;

    if (rhs.m_fillColor             != m_fillColor ||
        rhs.m_fillDisplay           != m_fillDisplay ||
        rhs.m_elmTransparency       != m_elmTransparency ||
        rhs.m_netElmTransparency    != m_netElmTransparency ||
        rhs.m_fillTransparency      != m_fillTransparency ||
        rhs.m_netFillTransparency   != m_netFillTransparency)
        return false;

    if (0 != memcmp(&rhs.m_appearanceOverrides, &m_appearanceOverrides, sizeof (m_appearanceOverrides)))
        return false;

    if (!(m_material == rhs.m_material))
        return false;

    if (!(m_gradient == rhs.m_gradient))
        return false;

    if (!(m_pattern == rhs.m_pattern))
        return false;

    if (!(m_styleInfo == rhs.m_styleInfo))
        return false;

    if (!(m_plotInfo == rhs.m_plotInfo))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::Resolve(ViewContextR context)
    {
    DgnSubCategoryId subCategoryId = GetSubCategoryId();

    BeAssert(subCategoryId.IsValid());
    if (!subCategoryId.IsValid())
        return;

    // Setup from SubCategory appearance...
    DgnCategories::SubCategory::Appearance appearance;

    if (nullptr != context.GetViewport())
        appearance = context.GetViewport()->GetViewController().GetSubCategoryAppearance(subCategoryId);
    else
        appearance = context.GetDgnDb().Categories().QuerySubCategory(subCategoryId).GetAppearance();

    if (!m_appearanceOverrides.m_color)
        m_lineColor = appearance.GetColor();

    if (!m_appearanceOverrides.m_fill)
        m_fillColor = appearance.GetColor();

    if (!m_appearanceOverrides.m_weight)
        m_weight = appearance.GetWeight();

    if (!m_appearanceOverrides.m_style)
        m_styleInfo = LineStyleInfo::Create(appearance.GetStyle(), nullptr).get(); // WIP_LINESTYLE - Need LineStyleParams...

    if (!m_appearanceOverrides.m_material)
        m_material = appearance.GetMaterial();

    // SubCategory transparency is combined with element transparency to compute net transparency. 
    if (0.0 != appearance.GetTransparency())
        {
        // combine transparencies by multiplying the opaqueness.
        // A 50% transparent element on a 50% transparent category should give a 75% transparent result.
        // (1 - ((1 - .5) * (1 - .5))
        double elementOpaque = 1.0 - m_elmTransparency;
        double fillOpaque = 1.0 - m_fillTransparency;
        double categoryOpaque = 1.0 - appearance.GetTransparency();

        m_netElmTransparency = (1.0 - (elementOpaque * categoryOpaque));
        m_netFillTransparency = (1.0 - (fillOpaque * categoryOpaque));
        }

    // SubCategory display priority is combined with element priority to compute net display priority. 
    m_netPriority = context.ResolveNetDisplayPriority(m_elmPriority, subCategoryId, &appearance);
    m_resolved = true;
    }

/*---------------------------------------------------------------------------------**//**
* draw a symbol into the current context
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawSymbol(IDisplaySymbol* symbol, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight)
    {
#if defined (DGNPLATFORM_WIP_SYMBOL_SYMBOLOGY)
    // We need to revisit "ignoreColor" and "ignoreWeight" and remove these args. The DgnGeomPart for the symbol doesn't 
    // store any symbology, so there should never be a reason to "ignore" anything, the correct symbology, either from the
    // symbol definition or element, should be setup prior to calling this method.
#endif
    QvCache* symbolCache = T_HOST.GetGraphicsAdmin()._GetSymbolCache();

    if (!symbolCache || CheckICachedDraw())
        {
        // if we're creating a cache elem already, we need to stroke the symbol into that elem by value
        IDrawGeomR output = GetIDrawGeom();

        output._PushTransClip(trans, clip);
        symbol->_Draw(*this);
        output._PopTransClip();

        return;
        }

    QvElem* qvElem = T_HOST.GetGraphicsAdmin()._LookupQvElemForSymbol(symbol);

    if (nullptr == qvElem)
        {
        SymbolContext symbolContext(*this);

        qvElem = symbolContext.DrawSymbolForCache(symbol, *symbolCache);

        if (nullptr == qvElem)
            return;

        T_HOST.GetGraphicsAdmin()._SaveQvElemForSymbol(symbol, qvElem); // save the qvelem in case we encounter this symbol again
        }

    // draw the symbol.
    IViewDrawR output = GetIViewDraw();

    output._PushTransClip(trans, clip);
    output.DrawQvElem(qvElem); // Display priority for symbols in 2d is incorporated into the transform.
    output._PopTransClip();
    }

/*---------------------------------------------------------------------------------**//**
* delete the cached representation of a symbol
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DeleteSymbol(IDisplaySymbol* symbol)
    {
    T_HOST.GetGraphicsAdmin()._DeleteSymbol(symbol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawAligned(DVec3dCR axis, DPoint3dCR origin, AlignmentMode alignmentMode, IStrokeAligned& stroker)
    {
    DPoint4d            zColumn4d;
    DVec3d              zColumn;
    DMatrix4d           toNpc = GetWorldToNpc().M0;
    Transform           localToWorldTransform;

    if (SUCCESS == GetCurrLocalToWorldTrans(localToWorldTransform))
        {
        DMatrix4d       localToWorld = DMatrix4d::From(localToWorldTransform);

        toNpc.InitProduct(toNpc, localToWorld);
        }

    DVec3d      axisLocal;

    switch (alignmentMode)
        {
        case AlignmentMode_AlongLocalInDrawing:
            axisLocal = axis;
            break;

        case AlignmentMode_AlongDrawing:
            axisLocal = axis;
            break;

        default:
            BeAssert(false);
            return;
        }
    axisLocal.Normalize();

    toNpc.GetRow(zColumn4d, 2);
    zColumn.XyzOf(zColumn4d);
    zColumn.Normalize();

    RotMatrix       rMatrix;

    if (zColumn.IsParallelTo(axisLocal))
        {
        rMatrix = RotMatrix::From1Vector(zColumn, 2, true);
        }
    else
        {
        DVec3d          xColumn, yColumn;

        yColumn.CrossProduct(zColumn, axisLocal);
        xColumn.CrossProduct(yColumn, zColumn);
        xColumn.Normalize();
        yColumn.Normalize();
        rMatrix = RotMatrix::FromColumnVectors(xColumn, yColumn, zColumn);
        }

    Transform       alignmentTransform = Transform::From(rMatrix, origin);

    PushTransform(alignmentTransform);
    m_transformClipStack.SetViewIndependent();
    stroker._StrokeAligned(*this);
    PopTransformClip();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawTextString(TextStringCR text)
    {
    text.GetGlyphSymbology(*GetCurrentDisplayParams());
    CookDisplayParams();

    double zDepth = GetCurrentDisplayParams()->GetNetDisplayPriority();
    GetIDrawGeom().DrawTextString(text, Is3dView() ? nullptr : &zDepth);                
    text.DrawTextAdornments(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetLinestyleTangents(DPoint3dCP start, DPoint3dCP end)
    {
    m_startTangent = start;
    m_endTangent = end;
    m_elemMatSymb.GetLineStyleSymbR().ClearContinuationData();
    m_ovrMatSymb.GetMatSymbR().GetLineStyleSymbR().ClearContinuationData();
    }

//  On tablets some of the raster information on the GPU is reset whenever QV is reset.
//  On tablets, QV is reset when the app is put into the background and when the device is
//  rotated.  That causes QV to reset some of the raster data that is kept in the GPU.
//  To compensate the RasterHandler needs to know if QV has been reset.
static uint32_t s_numQvInitCalls;
uint32_t ViewContext::GetCountQvInitCalls() {return s_numQvInitCalls;}
void ViewContext::IncrementCountQvInitCalls() { ++s_numQvInitCalls; }
