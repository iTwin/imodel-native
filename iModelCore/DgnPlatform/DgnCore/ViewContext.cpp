/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ViewContext.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "UpdateLogging.h"

#include <DgnPlatform/DgnCore/XGraphicsCache.h>

#define ELEINVISIBLE    0x0080      /* set if element is invisible */ // from scanner.h

IDrawRasterAttachment*   ViewContext::s_pRasterAttInterface = NULL;

enum FrustCorners
    {
    FRUST_Org    = 0,
    FRUST_X      = 1,
    FRUST_Y      = 2,
    FRUST_Z      = 3,
    FRUST_COUNT  = 4,
    };

static DPoint3d const s_frustPts[FRUST_COUNT] =
    {
    { 0.0, 0.0, 0.0 },  // FRUST_Org
    { 1.0, 0.0, 0.0 },  // FRUST_X
    { 0.0, 1.0, 0.0 },  // FRUST_Y
    { 0.0, 0.0, 1.0 },  // FRUST_Z
    };

static DRange3d const s_fullNpcRange =
    {
    {0.0, 0.0, 0.0},
    {1.0, 1.0, 1.0}
    };

static const int MAX_Alpha = (250 << 24);   // Limit for alpha to prevent complete transparency. In highest byte of a 32-bit integer.

enum
    {
    BG_COLOR_INDEX = 255,
    };

static bool s_drawRange;
#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
CookedDisplayStyle::CookedDisplayStyleFlags::CookedDisplayStyleFlags ()
    {
    memset (this, 0, sizeof (*this));

    // Initialize non-zero defaults.
    m_visibleEdgeLineStyle = true; // This is the original, default behavior (visible edges solid).
    m_hiddenEdgeLineStyle  = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
CookedDisplayStyle::CookedDisplayStyle (ViewFlagsCR viewFlags, CookedDisplayStyleCP parent)
    {
    m_visibleEdgeWidth = 1;
    m_hiddenEdgeWidth  = HiddenEdgeWidth_SameAsVisible;
    m_displayHandler = NULL;

    if (NULL != parent)
        Init (*parent);

    m_viewFlags = viewFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     10/07
+---------------+---------------+---------------+---------------+---------------+------*/
CookedDisplayStyle::CookedDisplayStyle (DisplayStyleCP style, DgnModelP modelRef, ViewContextP context, CookedDisplayStyleCP parent, bool inheritFromParent)
    {
    ViewFlagsCP viewFlags = context->GetViewFlags();

    m_displayHandler = NULL;

    if (!viewFlags)
        return;

    if (NULL == style)
        {
        if (NULL != parent)
            Init (*parent);

        m_viewFlags = *viewFlags;
        return;
        }

    m_flags.m_stylePresent = true;
    m_viewFlags = *viewFlags;

    if (!inheritFromParent)
        parent = NULL;

    style->ApplyTo (m_viewFlags);

    ViewportP               viewport = context->GetViewport();
    ViewDisplayOverridesCP  sourceOverrides = &style->GetOverrides();
    DgnProjectR             styleProject = style->GetProjectR();
    bool                    trueColor;
    IntColorDef             colorDef;

    if (NULL != parent && parent->m_flags.m_elementColor)
        {
        m_flags.m_elementColor  = true;
        m_elementColor          = parent->m_elementColor;
        m_elementColorIndex     = parent->m_elementColorIndex;
        }
    else
        {
        if (TO_BOOL (m_flags.m_elementColor = sourceOverrides->m_flags.m_elementColor))
            {

            m_elementColorIndex = sourceOverrides->m_elementColor;

                // Need to get background color from viewport...not colormap...
            if (NULL != viewport && DgnColorMap::INDEX_Background == m_elementColorIndex)
                {
                m_elementColor = viewport->GetBackgroundColor ();
                }
            else
                {
                if (SUCCESS != styleProject.Colors().Extract (&colorDef, NULL, &trueColor, NULL, NULL, sourceOverrides->m_elementColor) &&
                    SUCCESS != styleProject.Colors().Extract (&colorDef, NULL, &trueColor, NULL, NULL, (sourceOverrides->m_elementColor & 0xff))) // TFS# 65288 - Somehow extended colors without entries occur.   This matches SS3 (use index only).
                    {
                    BeAssert (false);
                    }
                else
                    {
                    m_elementColor = colorDef.m_int;

                    if (trueColor)
                        {
                        m_elementColorIndex = INVALID_COLOR;
                        }
                    else
                        {
                        // NOTE: White-on-white reversal isn't handled by QV for outline color override...element color used for hidden line...
                        if (MSRenderMode::HiddenLine == m_viewFlags.renderMode && NULL != viewport && viewport->GetMenuColor (WHITE_MENU_COLOR_INDEX) == m_elementColor && viewport->GetBackgroundColor () == m_elementColor)
                            {
                            m_elementColorIndex = INVALID_COLOR;
                            m_elementColor      = 0;
                            }
                        }
                    }
                }
            }
        else
            {
            m_elementColorIndex = INVALID_COLOR;
            m_elementColor      = 0;
            }
        }

    if (NULL != parent && parent->m_flags.m_lineStyle)
        {
        m_flags.m_lineStyle = true;
        m_lineStyleIndex = parent->m_lineStyleIndex;
        m_linePattern = parent->m_linePattern;
        }
    else
        {
        m_flags.m_lineStyle = sourceOverrides->m_flags.m_lineStyle;
        m_lineStyleIndex = sourceOverrides->m_lineStyle;
        m_linePattern = viewport ? viewport->GetIndexedLinePattern (sourceOverrides->m_lineStyle) : 0;
        }

    if (NULL != parent && parent->m_flags.m_lineWeight)
        {
        m_flags.m_lineWeight = true;
        m_lineWidth = parent->m_lineWidth;
        }
    else
        {
        m_flags.m_lineWeight = sourceOverrides->m_flags.m_lineWeight;
        m_lineWidth = viewport ? viewport->GetIndexedLineWidth (sourceOverrides->m_lineWeight) : 0;
        }

    if (NULL != parent && parent->m_flags.m_transparency)
        {
        m_flags.m_transparency = true;
        m_transparency = parent->m_transparency;
        }
    else
        {
        m_flags.m_transparency = sourceOverrides->m_flags.m_useTransparency;
        m_transparency = (UInt32)(255.0 * sourceOverrides->m_transparency);
        }

    if (NULL != parent && parent->m_flags.m_material)
        {
        m_flags.m_material = true;
        m_material = parent->m_material;
        }
    else
        {
        m_flags.m_material = sourceOverrides->m_flags.m_material;

        if (NULL == modelRef || !m_flags.m_material)
            m_material = NULL;
        else
            m_material = MaterialManager::GetManagerR().FindMaterial (DgnMaterialId (sourceOverrides->m_material.GetValue()), modelRef->GetDgnProject());
        }

    if (NULL != parent && parent->m_flags.m_edgeColor)
        {
        m_flags.m_edgeColor = true;
        m_edgeColor         = parent->m_edgeColor;
        }
    else
        {
        if (TO_BOOL (m_flags.m_edgeColor = sourceOverrides->m_flags.m_visibleEdgeColor))
            {
            m_edgeColorIndex = sourceOverrides->m_visibleEdgeColor;
            // Need to get background color from viewport...not colormap...
            if (DgnColorMap::INDEX_Background == sourceOverrides->m_visibleEdgeColor && NULL != viewport)
                {
                m_edgeColor = viewport->GetBackgroundColor ();
                }
            else if (SUCCESS != styleProject.Colors().Extract (&colorDef, NULL, &trueColor, NULL, NULL, sourceOverrides->m_visibleEdgeColor))
                {
                BeAssert (false);
                }
            else
                {
                m_edgeColor = colorDef.m_int;
                if (trueColor)
                    m_edgeColorIndex = INVALID_COLOR;
                else if (NULL != viewport && viewport->GetMenuColor (WHITE_MENU_COLOR_INDEX) == m_edgeColor && viewport->GetBackgroundColor () == m_edgeColor)
                    m_edgeColor = 0;// NOTE: White-on-white reversal isn't handled by QV for outline color override...
                }
            }
        else
            {
            m_edgeColor = 0;
            }
        }

    if (MSRenderMode::HiddenLine == m_viewFlags.renderMode || MSRenderMode::SolidFill == m_viewFlags.renderMode)            // Transparency is not supported in hidden line mode.
        {
        if (NULL != parent && parent->m_flags.m_hLineTransparency)
            {
            m_flags.m_hLineTransparency = parent->m_flags.m_hLineTransparency;
            m_hLineTransparencyThreshold = parent->m_hLineTransparencyThreshold;
            }
        else
            {
             m_flags.m_hLineTransparency  = style->GetOverrides().m_flags.m_hLineTransparency;
             m_hLineTransparencyThreshold = style->GetOverrides().m_hLineTransparencyThreshold;
            }
        if (NULL != parent && parent->m_flags.m_hLineMaterialColors)
            m_flags.m_hLineMaterialColors = parent->m_flags.m_hLineMaterialColors;
        else
            m_flags.m_hLineMaterialColors  = style->GetOverrides().m_flags.m_hLineMaterialColors;

        }

    if (NULL != m_displayHandler)
        m_displayHandler->PushStyle (m_displayHandlerSettings.get(), *context);

    if (NULL != parent && parent->m_flags.m_smoothIgnoreLights)
        m_flags.m_smoothIgnoreLights = parent->m_flags.m_smoothIgnoreLights;
    else
         m_flags.m_smoothIgnoreLights  = style->GetOverrides().m_flags.m_smoothIgnoreLights;

    m_visibleEdgeWidth                      = viewport ? viewport->GetIndexedLineWidth (sourceOverrides->m_visibleEdgeWeight) : 0; // No Overrides for this.
    m_flags.m_applyMonochromeOverrides      = context->GetIViewDraw().ApplyMonochromeOverrides (m_viewFlags);
    m_flags.m_perElementOverridesPresent    = m_flags.m_transparency || m_flags.m_lineWeight || m_flags.m_lineStyle || m_flags.m_material || (m_flags.m_elementColor && m_flags.m_applyMonochromeOverrides);
    m_flags.m_legacyDrawOrder               = MSRenderMode::Wireframe == m_viewFlags.renderMode && style->GetFlags ().m_legacyDrawOrder;
    m_flags.m_applyEdgeStyleToLines         = MSRenderMode::Wireframe != m_viewFlags.renderMode && style->GetFlags ().m_applyEdgeStyleToLines;
    m_flags.m_applyEdgeStyleToLines         = MSRenderMode::Wireframe != m_viewFlags.renderMode && style->GetFlags ().m_applyEdgeStyleToLines;
    m_flags.m_invisibleToCamera             = (NULL != parent && parent->m_flags.m_invisibleToCamera) || style->GetFlags ().m_invisibleToCamera;
    m_flags.m_ignoreGeometryMaps            = NULL != parent ? parent->m_flags.m_ignoreGeometryMaps : (MSRenderMode::Wireframe == m_viewFlags.renderMode || style->GetFlags().m_ignoreGeometryMaps);
    m_flags.m_displayEnvironment            = style->GetFlags().m_overrideBackgroundColor && style->GetEnvironmentTypeDisplayed() != EnvironmentDisplay::Color;
    m_environmentName                       = style->GetEnvironmentName();
    m_environmentTypeDisplayed              = style->GetEnvironmentTypeDisplayed ();

    if (sourceOverrides->HiddenEdgeWeightSameAsVisible() || NULL == viewport)
        m_hiddenEdgeWidth = HiddenEdgeWidth_SameAsVisible;
    else
        m_hiddenEdgeWidth = viewport->GetIndexedLineWidth (sourceOverrides->m_hiddenEdgeWeight);

    m_flags.m_displayGroundPlane            = style->GetFlags().m_displayGroundPlane;

    if (m_flags.m_displayGroundPlane)
        {
        m_groundPlaneColor                  = style->GetGroundPlane ().GetGroundColor ();
        m_groundPlaneHeight                 = style->GetGroundPlane ().GetHeight ();
        m_groundPlaneTransparency           = style->GetGroundPlane ().GetTransparency ();
        m_groundPlaneShowGroundFromBelow    = style->GetGroundPlane ().ShowGroundFromBelow ();
        m_groundPlaneHeight                 = style->GetGroundPlane().GetHeight () * 1000.0; //dgnModel_getUorPerMeter (modelRef);
        }

    if (NULL != parent && MSRenderMode::Wireframe != parent->m_viewFlags.renderMode)      // TR# 321060 - Dont use parent edge flags if parent is wirefrmae.
        {
        m_flags.m_visibleEdgeLineStyle      = parent->m_flags.m_visibleEdgeLineStyle;
        m_flags.m_visibleEdgeLineWeight     = parent->m_flags.m_visibleEdgeLineWeight;
        m_flags.m_hiddenEdgeLineStyle       = parent->m_flags.m_hiddenEdgeLineStyle;
        m_flags.m_hiddenEdgeLineWeightZero  = parent->m_flags.m_hiddenEdgeLineWeightZero;
        }
    else
        {
        m_flags.m_visibleEdgeLineStyle      = style->GetOverrides().m_flags.m_visibleEdgeStyle;
        m_flags.m_visibleEdgeLineWeight     = style->GetOverrides().m_flags.m_visibleEdgeWeight;
        m_flags.m_hiddenEdgeLineStyle       = style->GetOverrides().m_flags.m_hiddenEdgeStyle;
        m_flags.m_hiddenEdgeLineWeightZero  = style->GetOverrides().m_flags.m_hiddenEdgeWeightZero;
        }

    m_edgeMaskState = context->GetEdgeMaskState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
CookedDisplayStyle::CookedDisplayStyle (CookedDisplayStyleCR source)
    {
    Init (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void CookedDisplayStyle::Init (CookedDisplayStyle const& donor)
    {
    m_flags             = donor.m_flags;
    m_elementColorIndex = donor.m_elementColorIndex;
    m_elementColor      = donor.m_elementColor;
    m_linePattern       = donor.m_linePattern;
    m_lineStyleIndex    = donor.m_lineStyleIndex;
    m_lineWidth         = donor.m_lineWidth;
    m_transparency      = donor.m_transparency;
    m_edgeColorIndex    = donor.m_edgeColorIndex;
    m_edgeColor         = donor.m_edgeColor;
    m_visibleEdgeWidth  = donor.m_visibleEdgeWidth;
    m_hiddenEdgeWidth   = donor.m_hiddenEdgeWidth;
    m_material          = donor.m_material;
    m_viewFlags         = donor.m_viewFlags;
    m_edgeMaskState     = donor.m_edgeMaskState;
    m_environmentName   = donor.m_environmentName;
    m_displayHandler    = donor.m_displayHandler;
    m_displayHandlerSettings = donor.m_displayHandlerSettings;

    m_hLineTransparencyThreshold = donor.m_hLineTransparencyThreshold;

    m_environmentTypeDisplayed          = donor.m_environmentTypeDisplayed;
    m_groundPlaneColor                  = donor.m_groundPlaneColor;
    m_groundPlaneHeight                 = donor.m_groundPlaneHeight;
    m_groundPlaneTransparency           = donor.m_groundPlaneTransparency;
    m_groundPlaneShowGroundFromBelow    = donor.m_groundPlaneShowGroundFromBelow;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void    CookedDisplayStyle::OnFrustumChange (DgnModelR modelRef, ViewContextR viewContext) const
    {
    if (NULL != m_displayHandler && !m_displayHandlerSettings.IsNull())
        m_displayHandler->OnFrustumChange (*m_displayHandlerSettings, viewContext, modelRef);
    }
#endif

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

    m_dgnProject        = NULL;
    m_sourcePath        = NULL;

    m_ignoreScaleForDimensions  = false;
    m_ignoreScaleForMultilines  = false;
    m_applyRotationToDimView    = false;
    m_wantMaterials             = false;
    m_ignoreOpenElements        = false;
    m_useCachedGraphics         = true;

    m_levelClassMask.levelBitMaskP = NULL;
    m_levelClassMask.classMask     = 0;

    m_startTangent = m_endTangent = NULL;

    m_drawingClipElements       = false;
    m_currentDisplayStyle       = NULL;
    m_ignoreViewRange           = false;
    m_displayStyleStackMark     = 0;
    m_edgeMaskState             = EdgeMaskState_None;
    m_hiliteState               = HILITED_None;
    m_isCameraOn                = false;
    m_frustumTransClipDepth     = 0;
    m_edgeMaskState             = EdgeMaskState_None;
    m_currElemTopo              = NULL;
    m_conditionalBlockIndex     = 0;

    m_rasterDisplayParams.SetFlags (0);

    m_scanRangeValid        = false;
    m_levelOfDetail         = 1.0;

    m_frustumToNpc.InitIdentity();
    m_frustumToView.InitIdentity();

    // Draw any plane
    ResetRasterPlane();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/05
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::~ViewContext()
    {
    BeAssert (!m_isAttached);

    DELETE_AND_CLEAR (m_scanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP ViewContext::_GetViewTarget()
    {
    return NULL == GetViewport() ? NULL : GetViewport()->GetViewController().GetTargetModel (); // RangeContext may not set viewport.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
int             ViewContext::_GetViewNumber() const
    {
    return NULL != m_viewport ? m_viewport->GetViewNumber() : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewContext::GetCurrLocalToWorldTrans (DMatrix4dR localToWorld) const
    {
    Transform       localToFrustum;

    m_transformClipStack.GetTransform (localToFrustum); // NOTE: Returns ERROR if transform doesn't need to be pushed (identity)...so don't check status!
    localToWorld = DMatrix4d::From (localToFrustum);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::ViewToNpc (DPoint3dP npcVec, DPoint3dCP screenVec, int nPts) const
    {
    ViewToFrustum (npcVec, screenVec, nPts);
    m_frustumToNpc.M0.multiplyAndRenormalize (npcVec, npcVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::NpcToView (DPoint3dP viewVec, DPoint3dCP npcVec, int nPts) const
    {
    NpcToFrustum (viewVec, npcVec, nPts);
    FrustumToView (viewVec, viewVec, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::NpcToFrustum (DPoint3dP frustumPts, DPoint3dCP npcPts, int nPts) const
    {
    m_frustumToNpc.M1.multiplyAndRenormalize (frustumPts, npcPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::InitDisplayPriorityRange()
    {
    m_displayPriorityRange[0] = (m_is3dView ? 0 : -MAX_HW_DISPLAYPRIORITY);
    m_displayPriorityRange[1] = (m_is3dView ? 0 : MAX_HW_DISPLAYPRIORITY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ViewContext::_InitContextForView()
    {
    BeAssert (0 == GetTransClipDepth());

    m_elemMatSymb.Init ();
    m_ovrMatSymb.Clear();
    m_rasterDisplayParams.SetFlags(0);

    m_frustumTransClipDepth = 0;
    m_frustumToNpc  = *m_viewport->GetWorldToNpcMap();
    m_frustumToView = *m_viewport->GetWorldToViewMap();
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

    SetDgnProject (GetViewport ()->GetViewController().GetDgnProject ());

#if defined (NEEDS_WORK_DGNITEM)
    if (NULL != m_currentDisplayStyle)
        m_currentDisplayStyle->OnFrustumChange (*_GetViewTarget(), *this);
#endif

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
        m_npcSubRange.Get8Corners (frustPts);
        }
    else
        {

        if (NULL == m_viewport)
            s_fullNpcRange.get8Corners (frustPts);
        else
            frustum = m_viewport->GetFrustum(DgnCoordSystem::Npc, true);
        }

    m_frustumToNpc.M1.multiplyAndRenormalize (frustPts, frustPts, NPC_CORNER_COUNT);
    Viewport::FixFrustumOrder(frustum);
    return frustum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushFrustumClip ()
    {
    if (m_ignoreViewRange)
        return;

    int         nPlanes;
    ClipPlane   frustumPlanes[6];
    ViewFlagsCP viewFlags = GetViewFlags();

    Frustum polyhedron = GetFrustum();

    if (0 != (nPlanes = ClipUtil::RangePlanesFromPolyhedra(frustumPlanes, polyhedron.GetPts(), NULL != viewFlags && !viewFlags->noFrontClip, NULL != viewFlags && !viewFlags->noBackClip, 1.0E-6)))
        m_transformClipStack.PushClipPlanes (frustumPlanes, nPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_InitScanRangeAndPolyhedron ()
    {
    // set up scanner search criteria
    _InitScanCriteria();
    _ScanRangeFromPolyhedron();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::PushClipPlanes (ClipPlaneSetCR clipPlanes)
    {
    _PushClip (*ClipVector::CreateFromPrimitive (ClipPrimitive::CreateFromClipPlanes (clipPlanes)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushClip (ClipVectorCR clip)
    {
    m_transformClipStack.PushClip (clip);

    for (ClipPrimitivePtr const& primitive: clip)
        {
        GetIDrawGeom()._PushTransClip (NULL, primitive->GetClipPlanes ());
        m_transformClipStack.IncrementPushedToDrawGeom();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushTransform (TransformCR trans)
    {
    m_transformClipStack.PushTransform (trans);
    GetIDrawGeom()._PushTransClip (&trans , NULL);
    m_transformClipStack.IncrementPushedToDrawGeom();
    InvalidateScanRange ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_PushViewIndependentOrigin (DPoint3dCP origin)
    {
    Transform   viTrans;
    GetViewIndTransform (&viTrans, origin);
    _PushTransform (viTrans);
    m_transformClipStack.SetViewIndependent ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_PopTransformClip ()
    {
    if (m_transformClipStack.IsEmpty())
        {
        BeAssert (false);
        return;
        }

    m_transformClipStack.Pop (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::DirectPushTransClipOutput (IDrawGeomR drawGeom, TransformCP trans, ClipPlaneSetCP clip)
    {
    drawGeom._PushTransClip (trans, clip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::DirectPopTransClipOutput (IDrawGeomR drawGeom)
    {
    drawGeom._PopTransClip ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::PushOverrides (ElemHeaderOverrides* in)
    {
    int nOverrides = (int) m_headerOvr.size ();

    m_headerOvr.resize (nOverrides+1);
    ElemHeaderOverrides* newOvr = &m_headerOvr.at (nOverrides);
    newOvr->MergeFrom (in, nOverrides > 0 ? &m_headerOvr.at (nOverrides-1) : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::PopOverrides()
    {
    m_headerOvr.pop_back ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_SetCurrentElement (ElementRefP elemRef)
    {
    if (NULL == elemRef)
        GetIDrawGeom ().PopMethodState ();
    else
        GetIDrawGeom ().PushMethodState ();

    m_currentElement = elemRef;
    }

/*---------------------------------------------------------------------------------**//**
* when the frustum changes (only happens with Viewlets), reset frustum in m_IViewDraw
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_OnFrustumChange (bool is2d)
    {
    if (!is2d)
        {
        // Begin Viewlet.
        BeAssert (0 == m_frustumTransClipDepth);
        m_frustumTransClipDepth = GetTransClipDepth();
        m_transformClipStack.PushIdentity ();       // Clear pre-Frustum Clips
        }
    else
        {
        // End Viewlet.
        while (GetTransClipDepth() > m_frustumTransClipDepth)
            m_transformClipStack.Pop(*this);

        m_frustumTransClipDepth = 0;
        }

    InitDisplayPriorityRange();
    }

/*---------------------------------------------------------------------------------**//**
* Prepare this context to work on the given project when getting project from ViewController.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetDgnProject (DgnProjectR project)
    {
    m_dgnProject = &project;
    InitDisplayPriorityRange ();
    _SetupScanCriteria ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_InitScanCriteria()
    {
    // Set visible elements
    m_scanCriteria->SetPropertiesTest (0, ELEINVISIBLE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void classMaskFromFlags (UInt16& classMask, ViewFlagsCR flags)
    {
    // set properties/class masks for this view
    classMask = 0xffff;

    if (flags.patterns)
        {
        // turn off linear patterned elements
        classMask &= 0xffdf;
        classMask |= 2;       // turn on pattern elements
        }
    else
        {
        // turn off patterned elements
        classMask &= 0xfffd;
        classMask |= 0x20;    // turn on patterned lines
        }

    // set construction elements
    if (!flags.constructs)
        classMask &= 0xfffb;
    else
        classMask |= 4;

    // set dimension elements
    if (!flags.dimens)
        classMask &= 0xfff7;
    else
        classMask |= 8;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetupScanCriteria ()
    {
    if (NULL == m_scanCriteria)
        return;

    ViewFlagsCP  flags = GetViewFlags ();

    if (NULL != flags)
        {
        classMaskFromFlags (m_levelClassMask.classMask, *flags); // Was scanutil_classMaskFromFlags
        m_scanCriteria->SetClassTest(m_levelClassMask.classMask);

#if defined (NEEDS_WORK_DGNITEM)
        if (m_scanCriteria->GetScanType().testElementType) // make sure element type checking is enabled...
            {
            // if text off, don't bother to return text
            if (flags->fast_text)
                m_scanCriteria->RemoveSingleElementTypeTest (TEXT_ELM);
            else
                m_scanCriteria->AddSingleElementTypeTest (TEXT_ELM);
            }
#endif
        }

    ViewportP vp = GetViewport ();

    if (NULL != vp)
        {
        SetLevelBitMask (&vp->GetViewController().GetLevelDisplayMask());
        m_scanCriteria->SetLevelTest((BitMaskP)_GetLevelClassMask()->levelBitMaskP, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_AllocateScanCriteria()
    {
    if (NULL == m_scanCriteria)
        m_scanCriteria = new ScanCriteria;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_Attach (ViewportP viewport, DrawPurpose purpose)
    {
    if (NULL == viewport)
        return  ERROR;

    if (IsAttached())
        {
        BeAssert (!IsAttached());
        return  ERROR;
        }

    m_isAttached = true;
    _AllocateScanCriteria();

    m_viewport = viewport;
    _SetupOutputs ();

    m_purpose = purpose;
    ClearAborted();

    m_minLOD = viewport->GetMinimumLOD();
    m_filterLOD = FILTER_LOD_ShowRange;
    m_isCameraOn = viewport->IsCameraOn();

    m_is3dView = viewport->Is3dView();
    m_useCachedGraphics = true;

    SetViewFlags (viewport->GetViewFlags ());

    m_arcTolerance = 0.1;
    m_parentRangeResult = RangeResult::Overlap;
    m_currentDisplayStyle = NULL;

    return _InitContextForView();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_Detach()
    {
    BeAssert (IsAttached());

    m_isAttached = false;

    m_transformClipStack.PopAll(*this);
    m_headerOvr.resize (0);
    m_currentElement = NULL;

    /* m_IDrawGeom and m_IViewDraw are not typically NULL so the Get methods return references.
       However, there is a hack in SymbolContext::_Detach that NULLs them out specifically
       to prevent this method from modifying them */
    if (NULL != m_IDrawGeom)
        m_IDrawGeom->ActivateOverrideMatSymb (NULL);     // clear any overrides

    if (m_viewport)
        m_viewport->ResynchColorMap ();

    // _EmptySymbolCache(); not yet in Graphite
    UpdateLogging::RecordDetach();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::LocalToView (DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    GetLocalToView().multiply (viewPts, localPts, NULL, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::LocalToView (DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const
    {
    DMatrix4dCR  localToView = GetLocalToView();

    if (m_isCameraOn)
        localToView.multiplyAndRenormalize (viewPts, localPts, nPts);
    else
        localToView.multiplyAffine (viewPts, localPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::ViewToLocal (DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const
    {
    GetViewToLocal().MultiplyAndNormalize (localPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::ViewToLocal (DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const
    {
    GetViewToLocal ().multiplyAndRenormalize (localPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::FrustumToView (DPoint4dP viewPts, DPoint3dCP frustumPts, int nPts) const
    {
    m_frustumToView.M0.multiply (viewPts, frustumPts, NULL, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::FrustumToView (DPoint3dP viewPts, DPoint3dCP frustumPts, int nPts) const
    {
    m_frustumToView.M0.multiplyAndRenormalize (viewPts, frustumPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::FrustumToView (Point2dP viewPts, DPoint3dCP frustumPts, int  nPts) const
    {
    DPoint3d  tPt;
    DPoint4d  t4dPt;

    for (int i=0; i<nPts; i++)
        {
        FrustumToView (&t4dPt, frustumPts+i, 1);

        bsiDPoint4d_normalize (&t4dPt, &tPt);

        (viewPts+i)->x = (long) tPt.x;
        (viewPts+i)->y = (long) tPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::ViewToFrustum (DPoint3dP frustumPts, DPoint4dCP viewPts, int nPts) const
    {
    m_frustumToView.M1.MultiplyAndNormalize (frustumPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::ViewToFrustum (DPoint3dP frustumPts, DPoint3dCP  viewPts, int nPts) const
    {
    m_frustumToView.M1.multiplyAndRenormalize (frustumPts, viewPts, nPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::LocalToFrustum (DPoint3dP frustumPts, DPoint3dCP localPts, int nPts) const
    {
    Transform   localToFrustum;

    if (SUCCESS == m_transformClipStack.GetTransform (localToFrustum))
        localToFrustum.Multiply (frustumPts, localPts, nPts);
    else
        memcpy (frustumPts, localPts, nPts * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::FrustumToLocal (DPoint3dP localPts, DPoint3dCP frustumPts, int nPts) const
    {
    Transform   frustumToLocal;

    if (SUCCESS == m_transformClipStack.GetInverseTransform (frustumToLocal))
        frustumToLocal.Multiply (localPts, frustumPts, nPts);
    else
        memcpy (localPts, frustumPts, nPts * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::GetViewIndTransform (TransformP trans, DPoint3dCP originLocal)
    {
    RotMatrix   rMatrix;
    ViewportP vp = GetViewport();
    if (NULL != vp)
        {
        // get two vectors from origin in VIEW x and y directions
        DPoint4d    screenPt[2];
        LocalToView (screenPt, originLocal, 1);

        DPoint3d viewSize;
        Frustum viewBox = vp->GetFrustum (DgnCoordSystem::View, true);
        viewSize.DifferenceOf(viewBox.GetCorner(NPC_111), viewBox.GetCorner(NPC_000));

        screenPt[1] = screenPt[0];
        screenPt[0].x += viewSize.x;
        screenPt[1].y += viewSize.y;

        // convert to local coordinates
        DPoint3d    localPt[2];
        ViewToLocal (localPt, screenPt, 2);

        // if we're in a 2d view, we remove any fuzz
        if (!m_is3dView)
            localPt[0].z = localPt[1].z = originLocal->z;

        DVec3d  u, v;
        u.normalizedDifference (localPt,   originLocal);
        v.normalizedDifference (localPt+1, originLocal);

        // convert to rmatrix
        rMatrix.initFrom2Vectors (&u, &v);
        }
    else
        {
        rMatrix.initIdentity();
        }

    // get transform about origin
    bsiTransform_initFromMatrixAndFixedPoint (trans, &rMatrix, originLocal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ElemHeaderOverrides const*    ViewContext::GetHeaderOvr()
    {
    return m_headerOvr.empty() ? NULL : &m_headerOvr.back ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BSI                             04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ViewContext::SetWantMaterials (bool wantMaterials)
    {
    bool    prevWantMaterials = m_wantMaterials;

    m_wantMaterials = wantMaterials;

    return prevWantMaterials;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/13
//---------------------------------------------------------------------------------------
QvCache* ViewContext::GetQVCache (DgnProjectR project)
    {
    QvCache* result = project.Models().GetQvCache();
    if (NULL != result)
        return result;

    // create QvCache and save it on the DgnModel...
    project.Models().SetQvCache (result = T_HOST.GetGraphicsAdmin()._CreateQvCache ());

#ifdef WIP_MESH_SMOOTH_ANGLE
    if (tcb->maxMeshSmoothAngle > 0.0 && tcb->maxMeshSmoothAngle < msGeomConst_pi)
        QvOutput::SetMeshMaxSmoothAngle (result, tcb->maxMeshSmoothAngle);
#endif

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
QvCache* ViewContext::GetQVCache (CachedDrawHandleCR dh)
    {
    ElementRefP elRef = dh.GetElementRef();
    QvCache* qvCache = (elRef ? elRef->GetMyQvCache() : NULL);

    if (NULL == qvCache)
        {
        DgnProjectP dgnProject = const_cast<DgnProjectP>(dh.GetDgnProjectCP());

        // temp QvCache will be used for non-persistent elements...
        if (NULL == dgnProject)
            return NULL;

        qvCache = GetQVCache (*dgnProject);
        }

    return qvCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::GetCachedGeometry (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32 qvIndex, bool& deleteQvElem, bool allowQvElemSave)
    {
    double          pixelSize = 0.0;
    static double   s_sizeDependentCacheRatio = 3.0;
    double          sizeDependentRatio = s_sizeDependentCacheRatio;

    deleteQvElem = false;

#if defined (NEEDS_WORK_DGNITEM)
    if (NULL != m_currentDisplayStyle &&
        NULL != m_currentDisplayStyle->m_displayHandler)
        m_currentDisplayStyle->m_displayHandler->OverrideSizeDependence (sizeDependentRatio);
#endif

    if (stroker._GetSizeDependentGeometryPossible())
        {
        DPoint3d    localCenter;

        if (dh.IsGraphics())
            {
            DRange3d    localRange;
            ElementHandleCP thisElm = dh.GetElementHandleCP();

            if (NULL != thisElm)
                {
                DgnElementCP     el = thisElm->GetElementCP();
                localRange = el->GetRange();
                }
            else
                {
                dh.GetSymbolStampCP()->GetRange(localRange);
                }

            localCenter.sumOf (NULL, &localRange.low, .5, &localRange.high, .5);
            }
        else
            {
            localCenter.zero ();
            }

        pixelSize = GetPixelSizeAtPoint (&localCenter);
        }

    if (m_creatingCacheElem)
        {
        stroker._StrokeForCache (dh, *this, pixelSize);

        return NULL;
        }

    bool useCachedDisplay = allowQvElemSave && _UseCachedDisplay (dh);
    QvElem*         qvElem = NULL;

    // if we're given an elementRef, and if there is already a qvElem on it, use that instead of stroking
    if (useCachedDisplay)
        qvElem = GetQvCacheElem (dh, qvIndex, pixelSize);

    if (NULL != qvElem)
        return qvElem;

    bool        saveQvElem = allowQvElemSave && _WantSaveQvElem (stroker._GetDrawExpense());
    QvCache*    qvCache    = (saveQvElem ? GetQVCache (dh) : NULL);

    if (NULL == qvCache)
        {
        qvCache = GetViewport()->GetIViewOutput()->GetTempElementCache();
        saveQvElem = false;
        }

    BeAssert (qvCache);

#if defined (NEEDS_WORK_DGNITEM)
    m_displayFilterKey = DisplayFilterKey::Create();
#endif

    BeAssert(qvIndex >= 0);
#if defined (WIP_NEW_CACHE)
    XGraphicsSymbolP    xGraphicsSymbol = NULL;
    bool                saveSymbolQvElem = false;
    if (saveQvElem && qvIndex < 0 &&
        NULL != (xGraphicsSymbol  = XGraphicsSymbolCache::GetSymbol (thisElm, -qvIndex, stroker, pixelSize)))
        {
        qvElem = GetCachedSymbolGeometry (m_displayFilterKey, *xGraphicsSymbol, -qvIndex, pixelSize, &thisElm);
        saveSymbolQvElem = (NULL == qvElem);
        }
#endif

    if (NULL == qvElem && NULL == (qvElem = CreateCacheElem (dh, qvCache, stroker, NULL, pixelSize, NULL)))
        return NULL;

#if defined (NEEDS_WORK_DGNITEM)
    BeAssert(!stroker._GetSizeDependentGeometryStroked());
#endif
    if (!stroker._GetSizeDependentGeometryStroked())
        sizeDependentRatio = 0.0;

#if defined (WIP_NEW_CACHE)
    DisplayFilterKeyPtr  displayFilterKey = CacheDisplayFilterKey (*m_displayFilterKey, dh.GetDgnModelP());

    //  We only execute this code when qvIndex < 0.  I can't figure out how that ever happens.
    if (saveSymbolQvElem)
        SaveCachedSymbolGeometry (*xGraphicsSymbol, -qvIndex, qvElem, pixelSize,  sizeDependentRatio, *displayFilterKey, elRef);
#endif

    if (saveQvElem && dh.IsValid())
        SaveQvCacheElem (dh, qvIndex, qvElem, pixelSize, sizeDependentRatio, nullptr); 
    else
        deleteQvElem = true;

    m_displayFilterKey = NULL;

    return qvElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool QvUnsizedKey::MatchesConditionalDrawState (ViewContextR viewContext, ElementHandleCP element) const
    {
    return m_displayFilterKey.IsNull() || m_displayFilterKey->Matches (viewContext,element); // WIP_VANCOUVER_MERGE displayfilter
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool QvUnsizedKey::Matches (QvUnsizedKeyCR other) const
    {
    if (m_transformKey != other.m_transformKey || m_qvIndex != other.m_qvIndex)
        return false;

#if defined (NEEDS_WORK_DGNITEM)
    // Neither has an active display style handler (e. g. from thematic display)
    if (m_handlerKey.IsNull () && other.m_handlerKey.IsNull ())
        return true;

    // Only one has an active display style handler.
    if (m_handlerKey.IsNull () || other.m_handlerKey.IsNull ())
        return false;

    return m_handlerKey->Matches (*other.m_handlerKey);
#endif
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool QvUnsizedKey::IsNull () const
    {
    return m_transformKey == 0 && m_displayFilterKey.IsNull();
    }

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
struct QvSizedKey
{
    QvUnsizedKey    m_unsizedKey;
    double          m_low;
    double          m_high;

public:
    QvSizedKey (double size, double dependentRatio, QvUnsizedKeyCR unsizedKey) : m_unsizedKey (unsizedKey)
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

    inline bool LessThan (QvSizedKey const& other) const { return m_low < other.m_low; }
    void DeleteQvElem (QvElem* qvElem) { if (m_unsizedKey.OwnsQvElem()) T_HOST.GetGraphicsAdmin()._DeleteQvElem (qvElem);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool Equal (QvSizedKey const& other) const
    {
    return other.m_low > m_low && other.m_high < m_high && m_unsizedKey.Matches (other.m_unsizedKey);
    }
};

static ElementRefAppData::Key s_cacheSetKey;

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
struct QvElemCacheSet : QvElemSet<QvSizedKey>
    {
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
public: QvElemCacheSet (HeapZone& zone) : QvElemSet<QvSizedKey> (zone) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* Find (QvUnsizedKeyP* foundKey, double size, QvUnsizedKeyCR unsizedKey, ViewContextR viewContext, ElementHandleCP element)
    {
    for (Entry* thisEntry = m_entry; NULL != thisEntry; thisEntry = thisEntry->m_next)
        {
        if (thisEntry->m_key.m_unsizedKey.Matches (unsizedKey) &&
            size >= thisEntry->m_key.m_low && size <= thisEntry->m_key.m_high &&
            thisEntry->m_key.m_unsizedKey.MatchesConditionalDrawState (viewContext, element))
            {
            if (NULL != foundKey)
                *foundKey = &thisEntry->m_key.m_unsizedKey;

            return thisEntry->m_qvElem;
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void  _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
    {
#if defined (NEEDS_WORK_DGNITEM)
    for (Entry* thisEntry=m_entry; thisEntry; thisEntry=thisEntry->m_next)
        thisEntry->m_key.m_unsizedKey.ReleaseHandlerKey();
#endif

    QvElemSet<QvSizedKey>::_OnCleanup (host, unloadingCache, zone);
    }

}; // QvElemCacheSet

static DgnModelAppData::Key s_displayFilterSetKey;

struct CompareDisplayFilterKeys { bool operator () (const DisplayFilterKeyPtr& key0, const DisplayFilterKeyPtr& key1) const  { return *key0 < *key1; } };

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      08/2012
+===============+===============+===============+===============+===============+======*/
struct DisplayFilterKeyCache : DgnModelAppData
{

typedef bset <DisplayFilterKeyPtr, CompareDisplayFilterKeys>    T_DisplayFilterKeySet;

    T_DisplayFilterKeySet m_keys;

    virtual void  _OnCleanup (DgnModelR) override { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilterKeyPtr  CacheDisplayFilterKey (DisplayFilterKeyR key)
    {
    T_DisplayFilterKeySet::iterator     found = m_keys.find (&key);

    if (found != m_keys.end())
        return *found;

    m_keys.insert (&key);
    return &key;
    }
};  // DisplayFilterKeyCache

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilterKeyPtr  ViewContext::CacheDisplayFilterKey (DisplayFilterKeyR key, DgnModelP DgnModel)
    {
    // Display Filter Keys may be somewhat sizable and often repeated - so cache them with the model.

    DgnModelP       dgnModel;

    if (key.IsEmpty() || NULL == DgnModel || NULL == (dgnModel = DgnModel))
        return NULL;

    DisplayFilterKeyCache*      keyCache;

    if (NULL == (keyCache = (DisplayFilterKeyCache*) dgnModel->FindAppData (s_displayFilterSetKey)))
        dgnModel->AddAppData (s_displayFilterSetKey, keyCache = new DisplayFilterKeyCache ());

    return keyCache->CacheDisplayFilterKey (key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::GetCachedSymbolGeometry (DisplayFilterKeyPtr& filterKey, XGraphicsSymbolR symbol, Int32 qvIndex, double pixelSize, ElementHandleCP element)
    {
    QvElem*             qvElem = NULL;
    QvElemCacheSet*     cacheSet = NULL;
    QvUnsizedKeyP       foundKey = NULL;

    if (NULL != (cacheSet = (QvElemCacheSet *) symbol.GetQvElemSet ()) &&
        NULL != (qvElem =  cacheSet->Find (&foundKey, pixelSize, GetUnsizedKey(qvIndex), *this, element)))
        {
        filterKey = foundKey->GetDisplayFilterKey();
        return qvElem;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    ViewContext::SaveCachedSymbolGeometry (XGraphicsSymbolR symbol, Int32 qvIndex, QvElem* qvElem, double pixelSize, double sizeDependentRatio, DisplayFilterKeyR displayFilterKey, ElementRefP elementRef)
    {
    QvElemCacheSet*     cacheSet;

    if (NULL == (cacheSet = (QvElemCacheSet *) symbol.GetQvElemSet ()))
        {
        HeapZone& zone = elementRef->GetHeapZone();

        symbol.SetQvElemSet (cacheSet = new ((QvElemCacheSet*) zone.Alloc (sizeof(QvElemCacheSet))) QvElemCacheSet (zone));
        }

    cacheSet->Add (QvSizedKey (pixelSize, sizeDependentRatio, GetUnsizedKey (qvIndex, &displayFilterKey)), qvElem);
    }

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
UInt32          ViewContext::GetLocalTransformKey () const
    {
    Transform   localToFrustum;

    if (SUCCESS != m_transformClipStack.GetTransform (localToFrustum))
        return 0;

    static      DPoint3d  s_ramdomLocalToFrustTransformRefPoint = { 1234567.0, 7654321.0, 233425.0};
    DPoint3d    transformedPoint;

    localToFrustum.multiply (&transformedPoint, &s_ramdomLocalToFrustTransformRefPoint, 1);

    return   (UInt32) transformedPoint.x +  (UInt32) transformedPoint.y +  (UInt32) transformedPoint.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
QvUnsizedKey    ViewContext::GetUnsizedKey (Int32 qvIndex, DisplayFilterKeyP displayFilterKey)
    {
    return QvUnsizedKey (0, qvIndex, displayFilterKey);
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass                                                    John.Gooding    04/14
//=======================================================================================
struct StampQvElemMap
    {
private:
    typedef DgnStamps::StampData const* Key_T;
    struct QvElemIndexPair  
        { 
        QvElem*m_qvElem; 
        Int32 m_index; 
        QvElemIndexPair(QvElem*qvElem, Int32 index) : m_qvElem(qvElem), m_index(index) {}
        QvElemIndexPair(QvElemIndexPair const&source) : m_qvElem(source.m_qvElem), m_index(source.m_index) {}
        };


    HeapZone& m_zone;
    bmap<Key_T, bvector<QvElemIndexPair> > m_stampSimple;
    bmap<Key_T, QvElemCacheSet*> m_stampSet;

public:
    ~StampQvElemMap();
    StampQvElemMap(DgnProjectCR dgnProject) : m_zone(dgnProject.Models().GetHeapZone()) {}
    QvElem*Find(XGraphicsSymbolStampCR, Int32 index);
    void DeleteAll(XGraphicsSymbolStampCR);
    void Insert(XGraphicsSymbolStampCR, QvElem*, Int32 index);
    QvElemCacheSet* GetOrCreateCacheSet(XGraphicsSymbolStampCR);
    QvElemCacheSet* GetCacheSet(XGraphicsSymbolStampCR);
    };
END_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void ViewContext::FreeQvElems(XGraphicsSymbolStampCR symbolStamp)
    {
    //  Remove it from the QvElem cache even though it remains in the stamp cache.
    DgnProjectR dgnProject = const_cast<DgnProjectR>(symbolStamp.GetDgnProject());
    StampQvElemMapP qvElemMap = dgnProject.GetStampQvElemMapP();
    if (NULL != qvElemMap)
        qvElemMap->DeleteAll(symbolStamp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
StampQvElemMapP ViewContext::CreateSymbolStampMap(DgnProjectCR dgnProject)
    {
    return new StampQvElemMap(dgnProject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
StampQvElemMap::~StampQvElemMap()
    {
    DgnPlatformLib::Host::GraphicsAdmin& graphicsAdmin = T_HOST.GetGraphicsAdmin();
    for (auto& simpleMapEntry : m_stampSimple)
        {
        for (auto& indexPair : simpleMapEntry.second)
            graphicsAdmin._DeleteQvElem (indexPair.m_qvElem);
        }

    for (auto& setMapEntry : m_stampSet)
        {
        QvElemCacheSet* qvElemCacheSet = setMapEntry.second;
        qvElemCacheSet->_OnCleanup(NULL, true, m_zone);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void ViewContext::DeleteSymbolStampMap(StampQvElemMapP symbolStampMap)
    {
    //  This implementation assumes that the project is unloading and that it is appropriate to free all of the QvElems.
    delete symbolStampMap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
QvElem*StampQvElemMap::Find(XGraphicsSymbolStampCR stampRef, Int32 index)
    {
    Key_T stampKey = &stampRef.GetStampData();
    auto iter = m_stampSimple.find(stampKey);
    if (iter == m_stampSimple.end())
        return NULL;
    for (auto& pair : iter->second)
        {
        if (index == pair.m_index)
            return pair.m_qvElem;
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void StampQvElemMap::DeleteAll(XGraphicsSymbolStampCR stampRef)
    {
    Key_T stampKey = &stampRef.GetStampData();
    auto simpleMapIter = m_stampSimple.find(stampKey);
    if (simpleMapIter != m_stampSimple.end())
        {
        for (auto& pair : simpleMapIter->second)
            T_HOST.GetGraphicsAdmin()._DeleteQvElem(pair.m_qvElem);

        m_stampSimple.erase(simpleMapIter);
        }

    auto mapIter = m_stampSet.find(stampKey);
    if (mapIter == m_stampSet.end())
        return;

    QvElemCacheSet* cacheSet = mapIter->second;
    cacheSet->_OnCleanup(NULL, false, m_zone);
    
    m_stampSet.erase(mapIter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void StampQvElemMap::Insert(XGraphicsSymbolStampCR stampRef, QvElem*qvElem, Int32 index)
    {
    Key_T stampKey = &stampRef.GetStampData();
    auto mapIter = m_stampSimple.find(stampKey);
    if (mapIter == m_stampSimple.end())
        {
        bvector<QvElemIndexPair> list;
        list.push_back(QvElemIndexPair(qvElem, index));
        bpair<Key_T, bvector<QvElemIndexPair>> mapItem(stampKey, list);
        m_stampSimple.insert(mapItem);
        return;
        }

    for (bvector<StampQvElemMap::QvElemIndexPair>::iterator iter = mapIter->second.begin(); iter != mapIter->second.end(); ++iter)
        {
        if (index == iter->m_index)
            {
            T_HOST.GetGraphicsAdmin()._DeleteQvElem (iter->m_qvElem);
            iter->m_qvElem = qvElem;
            return;
            }
        }

    mapIter->second.push_back(QvElemIndexPair(qvElem, index));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
QvElemCacheSet* StampQvElemMap::GetOrCreateCacheSet(XGraphicsSymbolStampCR stampRef)
    {
    Key_T stampKey = &stampRef.GetStampData();
    auto mapIter = m_stampSet.find(stampKey);
    if (mapIter != m_stampSet.end())
        return mapIter->second;

    HeapZone& zone = stampRef.GetDgnProject().Models().GetHeapZone();
    QvElemCacheSet* newSet = new ((QvElemCacheSet*) zone.Alloc (sizeof(QvElemCacheSet))) QvElemCacheSet (zone);

    m_stampSet[stampKey] = newSet;
    return newSet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
QvElemCacheSet* StampQvElemMap::GetCacheSet(XGraphicsSymbolStampCR stampRef)
    {
    Key_T stampKey = &stampRef.GetStampData();
    auto mapIter = m_stampSet.find(stampKey);
    if (mapIter == m_stampSet.end())
        return NULL;

    return mapIter->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
static StampQvElemMapR GetStampQvElemMap(XGraphicsSymbolStampCR symbolStamp)
    {
    DgnProjectR dgnProject = const_cast<DgnProjectR>(symbolStamp.GetDgnProject());
    return dgnProject.GetStampQvElemMap();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::GetQvCacheElem (CachedDrawHandleCR dh, Int32 qvIndex, double pixelSize)
    {
    if (dh.GetElementHandleCP() != NULL)
        return GetQvCacheElemFromEH(*dh.GetElementHandleCP(), qvIndex, pixelSize);

    QvElemCacheSet*     cacheSet;
    QvElem*             qvElem;
    QvUnsizedKey        unsizedKey = GetUnsizedKey (qvIndex);

    XGraphicsSymbolStampCR symbolStamp = *dh.GetSymbolStampCP();
    StampQvElemMapR qvElemMap = GetStampQvElemMap(symbolStamp);
    // Search CacheSet first to find any conditional drawn elements.
    if (NULL != (cacheSet = (QvElemCacheSet *)qvElemMap.GetCacheSet(symbolStamp)) &&
        NULL != (qvElem =  cacheSet->Find (NULL, pixelSize, unsizedKey, *this, NULL)))
        return qvElem;

    return unsizedKey.IsNull () ? qvElemMap.Find (symbolStamp, qvIndex) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::GetQvCacheElemFromEH (ElementHandleCR eh, Int32 qvIndex, double pixelSize)
    {
    ElementRefP         elRef = eh.GetElementRef();
    if (NULL == elRef)
        return NULL;

    QvElemCacheSet*     cacheSet;
    QvElem*             qvElem;
    QvUnsizedKey        unsizedKey = GetUnsizedKey (qvIndex);

    // Search CacheSet first to find any conditional drawn elements.
    if (NULL != (cacheSet = (QvElemCacheSet *)elRef->FindAppData (s_cacheSetKey)) &&
        NULL != (qvElem =  cacheSet->Find (NULL, pixelSize, unsizedKey, *this, eh.IsValid() ? &eh : NULL)))
        return qvElem;

    return unsizedKey.IsNull () ? elRef->GetQvElem (qvIndex) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SaveQvCacheElem (CachedDrawHandleCR dh, Int32 qvIndex, QvElem* qvElem, double pixelSize, double sizeDependentRatio, DisplayFilterKeyP displayFilterKey)
    {
    XGraphicsSymbolStampCP stamp = dh.GetSymbolStampCP();
    if (NULL == stamp)
        {
        SaveQvCacheElemFromEH(*dh.GetElementHandleCP(), qvIndex, qvElem, pixelSize, sizeDependentRatio, displayFilterKey);
        return;
        }

    QvUnsizedKey        unsizedKey = GetUnsizedKey (qvIndex, displayFilterKey);
    StampQvElemMapR     qvElemMap = GetStampQvElemMap(*stamp);

    if (0.0 == sizeDependentRatio && unsizedKey.IsNull ())
        {
        qvElemMap.Insert(*stamp, qvElem, qvIndex);
        return;
        }

    QvElemCacheSet* qvElems = qvElemMap.GetOrCreateCacheSet(*stamp);
    qvElems->Add (QvSizedKey (pixelSize, sizeDependentRatio, unsizedKey), qvElem);

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SaveQvCacheElemFromEH(ElementHandleCR eh, Int32 qvIndex, QvElem* qvElem, double pixelSize, double sizeDependentRatio, DisplayFilterKeyP displayFilterKey)
    {
    ElementRefP elRef = eh.GetElementRef();
    BeAssert(NULL != elRef);

    QvUnsizedKey        unsizedKey = GetUnsizedKey (qvIndex, displayFilterKey);

    if (0.0 == sizeDependentRatio && unsizedKey.IsNull ())
        {
        elRef->SetQvElem (qvElem, qvIndex);
        return;
        }

    QvElemCacheSet* qvElems = (QvElemCacheSet*)elRef->FindAppData (s_cacheSetKey);

    if (NULL == qvElems)
        {
        HeapZone& zone = elRef->GetHeapZone();
        qvElems = new ((QvElemCacheSet*) zone.Alloc (sizeof(QvElemCacheSet))) QvElemCacheSet (zone);
        elRef->AddAppData (s_cacheSetKey, qvElems, zone);
        }

    qvElems->Add (QvSizedKey (pixelSize, sizeDependentRatio, unsizedKey), qvElem);
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ViewContext::GetIndexedColor (int index)
    {
#if defined (NEEDS_WORK_DGNITEM)
    BeAssert (NULL != m_dgnProject);
#endif

    return (NULL == m_dgnProject ? 0 : DgnColorMap::Get (*m_dgnProject)->GetTbgrColors()[index & 0xff]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ViewContext::GetIndexedLineWidth (int index)
    {
    ViewportP   viewport = GetViewport ();

    return (viewport ? viewport->GetIndexedLineWidth (index) : Viewport::GetDefaultIndexedLineWidth (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ViewContext::GetIndexedLinePattern (int index)
    {
    ViewportP   viewport = GetViewport();

    return (viewport ? viewport->GetIndexedLinePattern (index) : Viewport::GetDefaultIndexedLinePattern (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedLineColor (ElemMatSymbR elemMatSymb, int index)
    {
    elemMatSymb.SetIndexedLineColorTBGR (index, GetIndexedColor (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedFillColor (ElemMatSymbR elemMatSymb, int index)
    {
    elemMatSymb.SetIndexedFillColorTBGR (index, GetIndexedColor (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedLineWidth (ElemMatSymbR elemMatSymb, int index)
    {
    elemMatSymb.SetWidth (GetIndexedLineWidth (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedLinePattern (ElemMatSymbR elemMatSymb, int index)
    {
    elemMatSymb.SetIndexedRasterPattern (index, GetIndexedLinePattern (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedLineColor (OvrMatSymbR ovrMatSymb, int index)
    {
    ovrMatSymb.SetIndexedLineColorTBGR (index, GetIndexedColor (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedFillColor (OvrMatSymbR ovrMatSymb, int index)
    {
    ovrMatSymb.SetIndexedFillColorTBGR (index, GetIndexedColor (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedLineWidth (OvrMatSymbR ovrMatSymb, int index)
    {
    ovrMatSymb.SetWidth (GetIndexedLineWidth (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Andrew.Edge     03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::SetIndexedLinePattern (OvrMatSymbR ovrMatSymb, int index)
    {
    ovrMatSymb.SetIndexedRasterPattern (index, GetIndexedLinePattern (index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
ILineStyleCP    ViewContext::_GetCurrLineStyle (LineStyleSymbP* symb)
    {
    LineStyleSymbR  tSymb = (m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_Style) ? m_ovrMatSymb.GetMatSymbR ().GetLineStyleSymbR () : m_elemMatSymb.GetLineStyleSymbR ();

    if (symb)
        *symb = &tSymb;

    return tSymb.GetILineStyle ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ViewContext::GetCurrLineColor()
    {
    UInt32      lineColor = ((m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_Color) ? m_ovrMatSymb.GetLineColorTBGR() : m_elemMatSymb.GetLineColorTBGR()) & 0x00ffffff;
    UInt32      makeTrans = ((m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_ColorTransparency) ? m_ovrMatSymb.GetLineColorTBGR() : m_elemMatSymb.GetLineColorTBGR()) & 0xff000000;

    return lineColor | makeTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ViewContext::GetCurrFillColor()
    {
    UInt32      fillColor = ((m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_FillColor) ? m_ovrMatSymb.GetFillColorTBGR() : m_elemMatSymb.GetFillColorTBGR()) & 0x00ffffff;
    UInt32      makeTrans = ((m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_FillColorTransparency) ? m_ovrMatSymb.GetFillColorTBGR() : m_elemMatSymb.GetFillColorTBGR()) & 0xff000000;

    return fillColor | makeTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ViewContext::GetCurrWidth()
    {
    return (m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_RastWidth) ? m_ovrMatSymb.GetWidth() : m_elemMatSymb.GetWidth();
    }

/*---------------------------------------------------------------------------------**//**
* convert the view context polyhedron to scan parameters in the scanCriteria.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_ScanRangeFromPolyhedron()
    {
    Frustum polyhedron = GetFrustum();

    FrustumToLocal (polyhedron.GetPtsP(), polyhedron.GetPts(), 8);

    // get enclosing bounding box around polyhedron (outside scan range).
    DRange3d scanRange = polyhedron.ToRange();

    if (!Is3dView ())
        {
        scanRange.low.z = -1.0e20;
        scanRange.high.z = 1.0e20;
        }

    if (m_scanCriteria)
        {
        if (RangeResult::Inside == m_parentRangeResult)
            m_scanCriteria->SetRangeTest (NULL);
        else
            {
            m_scanCriteria->SetRangeTest (&scanRange);

            // if we're doing a skew scan, get the skew parameters
            if (Is3dView())
                {
                DRange3d skewRange;

                // get bounding range of front plane of polyhedron
                skewRange.initFrom (polyhedron.GetPts(), 4);

                // get unit bvector from front plane to back plane
                DPoint3d    skewVec;
                bsiDPoint3d_computeNormal (&skewVec, polyhedron.GetPts()+4, polyhedron.GetPts());

                // check to see if it's worthwhile using skew scan (skew bvector not along one of the three major axes */
                int alongAxes = (fabs (skewVec.x) < 1e-8);
                alongAxes += (fabs(skewVec.y) < 1e-8);
                alongAxes += (fabs(skewVec.z) < 1e-8);

                if (alongAxes < 2)
                    m_scanCriteria->SetSkewRangeTest (&scanRange, &skewRange, &skewVec);
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
void            ViewContext::InvalidateScanRange ()     { m_scanRangeValid = false; }

/*---------------------------------------------------------------------------------**//**
* Test an element against the current scan range using the range planes.
* @return true if the element is outside the range and should be ignored.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool         ViewContext::_FilterRangeIntersection (ElementHandleCR elIter)
    {
    if (RangeResult::Inside == m_parentRangeResult)
        return false;

    if (RangeResult::Outside == m_parentRangeResult)
        return true;

    DgnElementCP el = elIter.GetElementCP();

    return ClipPlaneContainment_StronglyOutside == m_transformClipStack.ClassifyRange (*elIter.GetIndexRange(), el->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::DrawBox (DPoint3dP box, bool is3d)
    {
    IDrawGeomR  drawGeom = GetIDrawGeom ();
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
        drawGeom.DrawLineString3d (9, tmpPts, NULL);

        // Draw missing connecting lines to complete box...
        drawGeom.DrawLineString3d (2, DSegment3d::From (box[0], box[3]).point, NULL);
        drawGeom.DrawLineString3d (2, DSegment3d::From (box[4], box[5]).point, NULL);
        drawGeom.DrawLineString3d (2, DSegment3d::From (box[1], box[7]).point, NULL);
        drawGeom.DrawLineString3d (2, DSegment3d::From (box[2], box[6]).point, NULL);
        return;
        }

    tmpPts[0] = box[0];
    tmpPts[1] = box[1];
    tmpPts[2] = box[2];
    tmpPts[3] = box[3];
    tmpPts[4] = box[0];

    drawGeom.DrawLineString3d (5, tmpPts, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawElementRange (DgnElementCP element)
    {
    DrawScanRange (element->GetRange(), element->Is3d(), GetElemMatSymb()->GetLineColorTBGR());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawScanRange (DRange3dCR range, bool is3d, UInt32 inputColor)
    {
    // Don't do this when trying to compute range or drop!
    switch (GetDrawPurpose ())
        {
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::FitView:
        case DrawPurpose::CaptureGeometry:
            return;
        }

    ViewportP vp = GetViewport();
    if (NULL != vp)
        {
        UInt32  color = vp->AdjustColorForContrast (inputColor, vp->GetBackgroundColor());

        m_ovrMatSymb.SetLineColorTBGR (vp->MakeTransparentIfOpaque (color, 150));
        m_ovrMatSymb.SetWidth (1);
        m_ovrMatSymb.SetRasterPattern (0);

        _AddContextOverrides ();
        ActivateOverrideMatSymb();
        }

    DPoint3d    p[8];

    p[0].x = p[3].x = p[4].x = p[5].x = range.low.x;
    p[1].x = p[2].x = p[6].x = p[7].x = range.high.x;
    p[0].y = p[1].y = p[4].y = p[7].y = range.low.y;
    p[2].y = p[3].y = p[5].y = p[6].y = range.high.y;
    p[0].z = p[1].z = p[2].z = p[3].z = range.low.z;
    p[4].z = p[5].z = p[6].z = p[7].z = range.high.z;

    DrawBox (p, is3d);

    m_ovrMatSymb.Clear(); // clear overrides

    _AddContextOverrides ();
    ActivateOverrideMatSymb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::VisitElemRef (ElementRefP elemRef, void* arg, bool checkScanCriteria)
    {
    if (NULL == elemRef)
        return  ERROR;

    ElementHandle  iter (elemRef);
    return  _VisitElemHandle (iter, false, checkScanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_OutputElement (ElementHandleCR elIter)
    {
    if (m_viewport)
        {
        m_viewport->GetViewControllerR()._DrawElement (*this, elIter);
        return;
        }

    CookElemDisplayParams (elIter);
    ActivateOverrideMatSymb ();
    elIter.GetDisplayHandler()->Draw (elIter, *this);
    }

/*---------------------------------------------------------------------------------**//**
* Apply additional modifications directly to the post-cook m_ovrMSymb. This method
* can NOT change the current ElemDisplayParams.
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_ModifyPostCookOverrides ()
    {
#if defined (NEEDS_WORK_DGNITEM)
    CookedDisplayStyleCP  displayStyle;

    if (NULL == (displayStyle = GetCurrentCookedDisplayStyle ()))
        {
        _AddSymbologyFilterOverrides ();
        return;
        }

    if (displayStyle->PerElementOverridesPresent ())
        {
        OvrMatSymbP  ovrMatSymb = GetOverrideMatSymb ();

        if (displayStyle->m_flags.m_elementColor && displayStyle->m_flags.m_applyMonochromeOverrides)
            {
            ovrMatSymb->SetLineColorTBGR (displayStyle->m_elementColor);
            ovrMatSymb->SetFillColorTBGR (displayStyle->m_elementColor);
            }

        if (displayStyle->m_flags.m_transparency)
            {
            ovrMatSymb->SetTransparentLineColor (displayStyle->m_transparency);
            ovrMatSymb->SetTransparentFillColor (displayStyle->m_transparency);
            }

        if (displayStyle->m_flags.m_lineStyle)
            {
            ovrMatSymb->SetIndexedRasterPattern (displayStyle->m_lineStyleIndex, displayStyle->m_linePattern);
            }

        if (displayStyle->m_flags.m_lineWeight)
            ovrMatSymb->SetWidth (displayStyle->m_lineWidth);

        if (displayStyle->m_flags.m_material)
            {
            ovrMatSymb->SetMaterial (displayStyle->m_material, this);
#if NEEDSWORK_MATERIALS
            if (elHandle.IsValid () && displayStyle->m_material)
                {
                MaterialUVDetailPtr detail;

                MaterialManager::GetManagerR ().GetMaterialUVDetailForElement (detail, *displayStyle->m_material, MaterialMap::MAPTYPE_Pattern, elHandle);
                if (detail.IsValid ())
                    ovrMatSymb->SetMaterialUVDetail (detail.get ());
                }
#endif
            }
        }

#ifdef WIP_VANCOUVER_MERGE // material
    MaterialCP material;

    if (!displayStyle->m_flags.m_material &&
        !GetDynamicViewStateStack().empty() &&
        ClipVolumePass::Cut == GetDynamicViewStateStack().back().GetPass() &&
        NULL != (material = GetElemMatSymb()->GetMaterial ()) &&
        material->GetSettings ().UseCutSectionMaterial ())
        {
        MaterialCP cutMaterial;

        if (NULL != (cutMaterial = material->GetCutSectionMaterial ()))
            {
            GetOverrideMatSymb ()->SetMaterial (cutMaterial, this);

#if NEEDSWORK_MATERIALS
            if (elHandle.IsValid ())
                {
                MaterialUVDetailPtr detail;

                MaterialManager::GetManagerR ().GetMaterialUVDetailForElement (detail, *cutMaterial, MaterialMap::MAPTYPE_Pattern, elHandle);
                if (detail.IsValid ())
                    ovrMatSymb->SetMaterialUVDetail (detail.get ());
                }
#endif
            }
        }
#endif

    if (NULL != displayStyle->m_displayHandler)
        displayStyle->m_displayHandler->ApplySymbologyOverrides (*this);

    _AddSymbologyFilterOverrides ();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* If any of the override flags are on, "cook" m_currDisplayParams into m_ovrMatSymb.
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_CookOverrideMatSymb ()
    {
    // if we overrode anything, we need to re-cook.
    if (MATSYMB_OVERRIDE_None == m_ovrMatSymb.GetFlags())
        return;

    m_currDisplayParams.ResolveColorTBGR (*this); // Make sure TBGR is valid for any color overrides...sensible overrides shouldn't need anything else resolved...
    m_ovrMatSymb.GetMatSymbR().FromResolvedElemDisplayParams (m_currDisplayParams, *this, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::ActivateOverrideMatSymb ()
    {
    // NOTE: This is a convenience method, there is nothing to prevent code from calling the output method directly and bypassing this method...
    GetIDrawGeom ().ActivateOverrideMatSymb (&m_ovrMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_AddPreCookParentOverrides ()
    {
    ElemHeaderOverridesCP  ovr = GetHeaderOvr ();

    if (NULL == ovr)
        return;

    UInt32      ovrFlags = MATSYMB_OVERRIDE_None;

    if (ovr->GetFlags ().color && !IsMonochromeDisplayStyleActive ()) // Monochrome style trumps hdr overrides...
        {
        m_currDisplayParams.SetLineColor (ovr->GetColor ());
        ovrFlags |= MATSYMB_OVERRIDE_Color;

        if (NULL == m_currDisplayParams.GetGradient ())
            {
            m_currDisplayParams.SetFillColor (ovr->GetColor ());
            ovrFlags |= MATSYMB_OVERRIDE_FillColor;
            }

#ifdef WIP_VANCOUVER_MERGE // material
        // NOTE1: If using materials...setup override so that new color/level assignment will be used.
        // NOTE2: Only do this if there is no material already ATTACHED.
        if (m_wantMaterials && !m_currDisplayParams.IsAttachedMaterial ())
            {
            m_currDisplayParams.SetMaterial (MaterialManager::GetManagerR ().FindMaterialBySymbology (NULL, m_currDisplayParams.GetLevel (), m_currDisplayParams.GetLineColor (), *GetCurrentModel (), true, false, this));
            ovrFlags |= MATSYMB_OVERRIDE_RenderMaterial;
            }
#endif
        }

    if (ovr->GetFlags ().style)
        {
        m_currDisplayParams.SetLineStyle (ovr->GetLineStyle (), ovr->GetLineStyleParams ());
        ovrFlags |= MATSYMB_OVERRIDE_Style;
        }

    if (ovr->GetFlags ().weight)
        {
        m_currDisplayParams.SetWeight (ovr->GetWeight ());
        ovrFlags |= MATSYMB_OVERRIDE_RastWidth;
        }

    m_ovrMatSymb.SetFlags (m_ovrMatSymb.GetFlags () | ovrFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_AddPreCookViewOverrides ()
    {
    ViewFlagsCP viewFlags = GetViewFlags ();

    if (NULL == viewFlags)
        return;

    UInt32          ovrFlags = MATSYMB_OVERRIDE_None;

    // Note: In DgnDb, we don't have the concept of "level override" symbology.

    // NOTE: Always set override even if m_currDisplayParams.m_symbology.weight = 0; needed for pattern cells/hatch lines...
    if (!viewFlags->line_wghts)
        {
        m_currDisplayParams.SetWeight (0);
        ovrFlags |= MATSYMB_OVERRIDE_RastWidth;
        }

    if (viewFlags->inhibitLineStyles && !IS_LINECODE (m_currDisplayParams.GetLineStyle ()))
        {
        m_currDisplayParams.SetLineStyle (0);
        ovrFlags |= MATSYMB_OVERRIDE_Style;
        }

    if (!viewFlags->transparency)
        {
        // Only setup element transparency override if currently transparent to avoid un-necessary overrides...
        if (0.0 != m_currDisplayParams.GetTransparency ())
            {
            m_currDisplayParams.SetTransparency (0.0);
            ovrFlags |= MATSYMB_OVERRIDE_ColorTransparency | MATSYMB_OVERRIDE_FillColorTransparency;
            }
        }
    else
        {
        DgnLevels::SubLevel::Appearance appear = m_viewport->GetViewController().GetSubLevelAppearance(m_currDisplayParams.GetSubLevelId());
        if (0.0 != appear.GetTransparency ())
            {
            // combine transparencies by multiplying the opaqueness.
            // A 50% transparent element on a 50% transparent level should give a 75% transparent result.
            // (1 - ((1 - .5) * (1 - .5))
            double      elementOpaque = 1.0 - m_currDisplayParams.GetTransparency();
            double      levelOpaque   = 1.0 - appear.GetTransparency ();

            m_currDisplayParams.SetTransparency (1.0 - (elementOpaque * levelOpaque));
            ovrFlags |= MATSYMB_OVERRIDE_ColorTransparency | MATSYMB_OVERRIDE_FillColorTransparency;
            }
        }

    m_ovrMatSymb.SetFlags (m_ovrMatSymb.GetFlags () | ovrFlags);
    }

/*---------------------------------------------------------------------------------**//**
* Apply any view-specific modifications to the current ElemDisplayParams. This method is called AFTER the displayParams have
* been cooked into the m_elemMatSym member of the context and therefore these modifications will <em>not</em> be saved in
* any cached representations. They will be "cooked" into the override matsymb.
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_ModifyPreCookOverrides ()
    {
    _AddPreCookParentOverrides ();
    _AddPreCookViewOverrides ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_CookDisplayParamsOverrides ()
    {
    m_rasterDisplayParams.SetFlags (ViewContext::RasterDisplayParams::RASTER_PARAM_None);
    m_ovrMatSymb.Clear();

    _ModifyPreCookOverrides (); // Modify m_currDisplayParams for anything specific to this instance, i.e. can't be cached...
    _CookOverrideMatSymb (); // Setup m_overrideMatSymb from cooked m_currDisplayParams...

    _ModifyPostCookOverrides (); // Apply direct overrides that don't require cooking m_currDisplayParams...
    _AddContextOverrides (); // Allow context to contribute to m_overrideMatSymb
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_ModifyPreCook (ElemDisplayParamsR elParams)
    {
    elParams.Resolve (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb)
    {
    _ModifyPreCook (elParams); // Allow context to modify elParams before cooking...

    // "cook" the display params into a MatSymb
    elMatSymb.FromResolvedElemDisplayParams (elParams, *this, m_startTangent, m_endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::CookDisplayParams ()
    {
    _CookDisplayParams (m_currDisplayParams, m_elemMatSymb);

    // Activate the matsymb in the IDrawGeom
    GetIDrawGeom ().ActivateMatSymb (&m_elemMatSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::CookElemDisplayParams (ElementHandleCR eh)
    {
    // NOTE: each of the following steps are kept separate so that subclasses can override them individually
    eh.GetDisplayHandler ()->GetElemDisplayParams (eh, m_currDisplayParams, m_wantMaterials); // Start by setting up m_currDisplayParams from the element we're working on.

    CookDisplayParams ();           // Cook m_currDisplayParams into m_elemMatSymb. This will be stored in any cached presentations.
    CookDisplayParamsOverrides ();  // Apply overrides to m_currDisplayParams and cook into override mat symb.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::ElementIsUndisplayed (ElementHandleCR inEl)
    {
    if (!_WantUndisplayed ())
        {
        ElementRefP elRef = inEl.GetElementRef();

        if (NULL != elRef && elRef->IsUndisplayed())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitElemHandle (ElementHandleCR inEl, bool checkRange, bool checkScanCriteria)
    {
    if (_CheckStop())
        return ERROR;

    DisplayHandlerP dHandler = inEl.GetDisplayHandler();
    if (NULL == dHandler)
        return  SUCCESS;

    if (ElementIsUndisplayed (inEl))
        return SUCCESS;

    // don't check range of children if we're creating cached representation or if handler has set m_noRangeTestOnComponents flag.
    if (checkRange && !CheckICachedDraw() && !dHandler->IsVisible (inEl, *this, true, false, false))
        return  SUCCESS;

    if (!checkScanCriteria || dHandler->IsVisible (inEl, *this, false, true, true))
        {
        ElementRefP elRef = inEl.GetElementRef();
        bool        pushPath = (NULL != elRef);

        if (pushPath)
            _SetCurrentElement (elRef);

        SetPresentationFormId (L"");
        _OutputElement (inEl);

        if (!m_creatingCacheElem && (s_drawRange || T_HOST.GetGraphicsAdmin()._WantDebugElementRangeDisplay ())) // this is really just for debugging...
            {
            DrawScanRange (*ElemRangeIndex::GetIndexRange(inEl), inEl.GetElementCP()->Is3d(), GetElemMatSymb()->GetLineColorTBGR());
            }

        if (pushPath)
            _SetCurrentElement (NULL);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  ViewContext::_VisitTransient (ElementHandleCR el, SymbologyOverridesP)
    {
    return _VisitElemHandle (el, true, true);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ViewContext::_VisitTransientElements (bool isPreUpdate)
    {
    IViewOutput*    output = (IsAttached () ? GetViewport()->GetIViewOutput() : NULL);
    bool            restoreZWrite = (output && isPreUpdate ? output->EnableZWriting (false) : false);

    T_HOST.GetGraphicsAdmin()._CallViewTransients (*this, isPreUpdate);

    if (restoreZWrite)
        output->EnableZWriting (true);
    }

/*---------------------------------------------------------------------------------**//**
* private callback (called from scanner)
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt visitElemRefFunc (ElementRefP elRef, ViewContextP context, ScanCriteriaP pSC)
    {
    StatusInt status = context->VisitElemRef (elRef, NULL, false);

    if (SUCCESS == status)      // revalidate because scanner is going to use it for future elements.
        context->ValidateScanRange();

    return context->WasAborted() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetScanReturn()
    {
    bool isScanRangeValid = (RangeResult::Inside != m_parentRangeResult);
    int  returnType = (isScanRangeValid ? _GetScanReturnType () : MSSCANCRIT_ITERATE_ELMREF);

    m_scanCriteria->SetRangeNodeCheck (this);
    m_scanCriteria->SetReturnType (returnType, false, true);
    m_scanCriteria->SetElemRefCallback ((PFScanElemRefCallback) visitElemRefFunc, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  ViewContext::_CheckNodeRange (ScanCriteriaCR scanCriteria, DRange3dCR testRange, bool is3d, bool isElement)
    {
    return ClipPlaneContainment_StronglyOutside != m_transformClipStack.ClassifyElementRange(testRange, is3d, true) ? ScanTestResult::Pass : ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsFrustumPointVisible (DPoint3dCR frustumPoint, bool boresite)
    {
    DPoint3d    localPoint;

    FrustumToLocal (&localPoint, &frustumPoint, 1);

    return IsLocalPointVisible (localPoint, boresite);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsLocalPointVisible (DPoint3dCR localPoint, bool boresite)
    {
    if (m_transformClipStack.IsEmpty())
        return true;

    if (!boresite)
        return m_transformClipStack.TestPoint (localPoint);

    DVec3d      localZVec;

    if (IsCameraOn())
        {
        DPoint3d        localCamera;
        
        FrustumToLocal (&localCamera, &GetViewport()->GetCamera().GetEyePoint(), 1);
        localZVec.NormalizedDifference (localPoint, localCamera);
        }
    else
        {
        DPoint3d        zPoints[2];
        Transform       frustumToLocal;

        zPoints[0].zero();
        zPoints[1].init (0.0, 0.0, 1.0);

        NpcToFrustum (zPoints, zPoints, 2);

        localZVec.NormalizedDifference (zPoints[1], zPoints[0]);

        if (GetCurrFrustumToLocalTrans (frustumToLocal))
            {
            frustumToLocal.MultiplyMatrixOnly (localZVec);
            localZVec.Normalize();
            }
        }

    return  m_transformClipStack.TestRay (localPoint, localZVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::PointInsideClip (DPoint3dCR point)
    {
    return m_transformClipStack.TestPoint (point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::GetRayClipIntersection (double& distance, DPoint3dCR origin, DVec3dCR direction)
    {
    return  m_transformClipStack.GetRayIntersection (distance, origin, direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_ScanDgnModel (DgnModelP model)
    {
    if (!ValidateScanRange ())
        return ERROR;

    m_scanCriteria->SetDgnModel (model);

    return m_scanCriteria->Scan (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::_VisitDgnModel (DgnModelP modelRef)
    {
    if (CheckStop ())
        return ERROR;

    return _ScanDgnModel (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetSubRectFromViewRect(BSIRectCP viewRect)
    {
    if (NULL == viewRect)
        return;

    BSIRect tRect = *viewRect;
    tRect.Expand (1);

    DRange3d viewRange;
    viewRange.low.init  (tRect.origin.x, tRect.corner.y, 0.0);
    viewRange.high.init (tRect.corner.x, tRect.origin.y, 0.0);

    GetViewport()->ViewToNpc (&viewRange.low, &viewRange.low, 2);

    // this is due to fact that y's can be reversed from view to npc
    DRange3d npcSubRect;
    npcSubRect.initFrom (&viewRange.low, 2);
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
bool ViewContext::VisitAllViewElements (bool includeTransients, BSIRectCP updateRect)
    {
    ClearAborted();
    if (NULL != updateRect)
        SetSubRectFromViewRect(updateRect);

    _InitScanRangeAndPolyhedron ();

    SetScanReturn();
    _VisitAllModelElements (includeTransients);

    m_transformClipStack.PopAll (*this);    // This will cause pushed clip elements to display correctly (outside their clip).

#ifdef WIP_VANCOUVER_MERGE // material
    m_materialAssignmentCache.clear ();

    if (!WasAborted() && _WantDgnAttachmentBoundaryDisplay ())
        AddAbortTest(drawAttachmentBoundaries (*this, _GetViewRoot (), includeList, includeRefs));
#endif
    return  WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_VisitAllModelElements (bool includeTransients)
    {
    if (includeTransients)
        _VisitTransientElements (true);

    PhysicalViewControllerCP physController = m_viewport->GetPhysicalViewControllerCP();
    ClipVectorPtr clipVector = physController ? physController->GetClipVector() : nullptr;
    if (clipVector.IsValid())
        PushClip(*clipVector);

    // The ViewController must orchestrate the display of all of the elements in the view.
    m_viewport->GetViewControllerR().DrawView(*this);

    if (clipVector.IsValid())
        PopTransformClip();

    if (includeTransients) // Give post-update IViewTransients a chance to display even if aborted the element draw...
        _VisitTransientElements (false);

    return WasAborted();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ViewContext::VisitPath (DisplayPathCP path, void* arg)
    {
    ClearAborted();
    AutoRestore <DisplayPathCP> savePath (&m_sourcePath, path);

    _InitScanRangeAndPolyhedron ();

    return m_viewport->GetViewController().VisitPath (path, arg, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::GetDisplayPriority() const
    {
    return m_currDisplayParams.GetNetDisplayPriority ();
    }

/*---------------------------------------------------------------------------------**//**
* Determine whether this context wants to use the cached presentation of this element (if it exists).
* Default to allow use of QvCached presentations.
* @bsimethod                                                    KeithBentley    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_UseCachedDisplay (CachedDrawHandleCR drawHandle)
    {
    return m_useCachedGraphics && drawHandle.IsPersistent();
    }

/*---------------------------------------------------------------------------------**//**
* create a QvElem from an ElementHandle using an IStrokeForCache stroker.
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::CreateCacheElem (CachedDrawHandleCR dh, QvCache* qvCache, IStrokeForCache& stroker, ViewFlagsCP viewFlags, double pixelSize, ICachedDrawP cachedDraw)
    {
    BeAssert (!m_creatingCacheElem || NULL != cachedDraw);

    if (NULL == cachedDraw)
        cachedDraw = m_ICachedDraw;

    if (NULL == cachedDraw)
        return NULL;

    cachedDraw->BeginCacheElement (!dh.IsGraphics() || dh.Is3d(), qvCache, viewFlags);

    AutoRestore<IDrawGeomP> saveDrawGeom (&m_IDrawGeom, cachedDraw);
    AutoRestore<byte>       savefilter (&m_filterLOD, FILTER_LOD_Off);
    AutoRestore<bool>       saveCreatingCache (&m_creatingCacheElem, true);

    // NOTE: Preserve current state of ElemDisplayParams for DrawQvElem. Level is needed to resolve DgnColorMap::INDEX_ByLevel and net display priority is needed for 2d...
    AutoRestore <ElemDisplayParams> saveCurrentDisplayParams (&m_currDisplayParams);

    try
        {
#if defined (NEEDS_WORK_DGNITEM)
        if (NULL == m_currentDisplayStyle ||
            NULL == m_currentDisplayStyle->m_displayHandler ||
            ! m_currentDisplayStyle->m_displayHandler->StrokeForCache (dh, *this, stroker, pixelSize))
#endif
            stroker._StrokeForCache (dh, *this, pixelSize);
        }
    catch (...)
        {
        }

    QvElem* result = cachedDraw->EndCacheElement();
    if (!WasAborted () || NULL == result)
        return result;

    T_HOST.GetGraphicsAdmin()._DeleteQvElem (result);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawQvElem (QvElem* qvElem, bool is3d)
    {
    if (is3d)
        GetIViewDraw().DrawQvElem3d (qvElem);
    else
        GetIViewDraw().DrawQvElem2d (qvElem, GetDisplayPriority());
    }

/*---------------------------------------------------------------------------------**//**
* Output a displayable that uses QvElems and can be cached in QuickVision.
* Sometimes this function will find that the cached representation already exists, and then it
* simply draws that cached representation. Otherwise it calls the Draw
* method on the displayable and draws (and potentially saves) the resultant QvElem.
* @bsimethod                                                    KeithBentley    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem*         ViewContext::_DrawCached (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32 qvIndex)
    {
    bool    deleteQvElem;
    QvElem* qvElem = GetCachedGeometry (dh, stroker, qvIndex, deleteQvElem, true);

    if (!qvElem)
        return NULL;

    _DrawQvElem (qvElem, dh.Is3d());
        //  DisplayHandler::Is3dElem (thisElm.GetElementCP()));

    if (!deleteQvElem)
        return qvElem;

    T_HOST.GetGraphicsAdmin()._DeleteQvElem (qvElem);
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
QvElem* ViewContext::DrawCached (ElementHandleCR eh, IStrokeForCache const& stroker, Int32 cacheIndex)
    {
    return DrawCached(CachedDrawHandle(&eh),const_cast<IStrokeForCache&>(stroker),cacheIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::CheckThicknessVector()
    {
    // Not attached to a view, thickness required...
    if (NULL == GetViewport())
        return true;

    // Thickness visible due to perspective...
    if (IsCameraOn())
        return true;

    bool        isCapped;
    DVec3dCP    thicknessDir = m_currDisplayParams.GetThickness (isCapped);

    if (NULL == thicknessDir)
        return false;

    DPoint3d    npcPt[2], frustumPt[2];
    DVec3d      zVector, testVec = *thicknessDir;

    npcPt[0] = s_frustPts[FRUST_Org];
    npcPt[1] = s_frustPts[FRUST_Z];

    NpcToFrustum (frustumPt, npcPt, 2);
    zVector.NormalizedDifference (frustumPt[0], frustumPt[1]);

    Transform   transform;

    if (SUCCESS == GetCurrLocalToFrustumTrans (transform))
        transform.MultiplyMatrixOnly (testVec, testVec);

    return (LegacyMath::Vec::AreParallel (&zVector, &testVec) ? false : true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ViewContext::_GetDisplayInfo (bool isRenderable)
    {
    UInt32      info = DISPLAY_INFO_None;
    ViewFlagsCP flags = GetViewFlags();
    bool        isFilled, isThickened, isCapped, isOutlined = false, isSurface = false;

    switch (m_currDisplayParams.GetFillDisplay ())
        {
        case FillDisplay::Always:
        case FillDisplay::Blanking:
            isFilled = true;
            break;

        case FillDisplay::ByView:
            isFilled = (!flags || flags->fill);
            break;

        default:
            isFilled = false;
            break;
        }

    isThickened = (NULL != m_currDisplayParams.GetThickness (isCapped));

    if (!flags || MSRenderMode::Wireframe == flags->renderMode)
        {
        if (isThickened)
            isThickened = CheckThicknessVector();

        isOutlined = (isFilled ? _CheckFillOutline() : true);
        }

    if (flags && (MSRenderMode::Wireframe != flags->renderMode))
        {
        if (isRenderable)
            isSurface = true;
        else if (! m_ignoreOpenElements)
            isOutlined = true;
        }

    if (isThickened)
        {
        // Thickness display handles edge/surface...never shown w/fill...
        info |= DISPLAY_INFO_Thickness;
        }
    else
        {
        if (isOutlined)
            info |= DISPLAY_INFO_Edge;

        if (isSurface)
            info |= DISPLAY_INFO_Surface;

        if (isFilled)
            info |= DISPLAY_INFO_Fill;

        if (isRenderable && WantAreaPatterns ())
            info |= DISPLAY_INFO_Pattern;
        }

    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawWithThickness (ElementHandleCR thisElm, IStrokeForCache& stroker, Int32 qvIndex)
    {
    AutoRestore<RangeResult> saveRangeResult (&m_parentRangeResult, RangeResult::Inside); // Don't filter cmpns on range!

    DrawCached (CachedDrawHandle(&thisElm), stroker, qvIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString3d (int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed)
    {
    if (nPts < 1)
        return;

    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle && (nPts > 2 || !pts->isEqual (pts+1)))
        {
        currLStyle->_GetComponent()->_StrokeLineString (this, currLsSymb, pts, nPts, closed);
        return;
        }

    if (closed)
        GetIDrawGeom().DrawShape3d (nPts, pts, false, range);
    else
        GetIDrawGeom().DrawLineString3d (nPts, pts, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledLineString2d (int nPts, DPoint2dCP pts, double priority, DPoint2dCP range, bool closed)
    {
    if (nPts < 1)
        return;

    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle && (nPts > 2 || !pts->isEqual (pts+1)))
        {
        currLStyle->_GetComponent()->_StrokeLineString2d (this, currLsSymb, pts, nPts, priority, closed);
        return;
        }

    if (closed)
        GetIDrawGeom().DrawShape2d (nPts, pts, false, priority, range);
    else
        GetIDrawGeom().DrawLineString2d (nPts, pts, priority, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc3d (DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        double      r0, r1, start, sweep;
        RotMatrix   rMatrix;
        DPoint3d    center;

        ellipse.GetScaledRotMatrix (center, rMatrix, r0, r1, start, sweep);
        currLStyle->_GetComponent()->_StrokeArc (this, currLsSymb, &center, &rMatrix, r0, r1, isEllipse ? NULL : &start, isEllipse ? NULL : &sweep, range);
        return;
        }

    GetIDrawGeom().DrawArc3d (ellipse, isEllipse, false, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledArc2d (DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        double      r0, r1, start, sweep;
        RotMatrix   rMatrix;
        DPoint3d    center;

        ellipse.GetScaledRotMatrix (center, rMatrix, r0, r1, start, sweep);
        center.z = zDepth; // Set priority on center...
        currLStyle->_GetComponent()->_StrokeArc (this, currLsSymb, &center, &rMatrix, r0, r1, isEllipse ? NULL : &start, isEllipse ? NULL : &sweep, NULL);
        return;
        }

    GetIDrawGeom().DrawArc2d (ellipse, isEllipse, false, zDepth, range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve3d (MSBsplineCurveCR bcurve)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        currLStyle->_GetComponent()->_StrokeBSplineCurve (this, currLsSymb, &bcurve, NULL);
        return;
        }

    GetIDrawGeom().DrawBSplineCurve (bcurve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawStyledBSplineCurve2d (MSBsplineCurveCR bcurve, double zDepth)
    {
    LineStyleSymbP  currLsSymb;
    ILineStyleCP    currLStyle = _GetCurrLineStyle(&currLsSymb);

    if (currLStyle)
        {
        if (0.0 == zDepth)
            {
            currLStyle->_GetComponent()->_StrokeBSplineCurve (this, currLsSymb, &bcurve, NULL);
            return;
            }

        // NOTE: Copy curve and set priority on poles since we won't be drawing cached (i.e. not handled by DrawQvElem2d)...
        MSBsplineCurvePtr tmpCurve = bcurve.CreateCopy ();
        bool useWeights = tmpCurve->rational && NULL != tmpCurve->GetWeightCP ();
        for (int iPoint = 0; iPoint < tmpCurve->params.numPoles; ++iPoint)
            {
            DPoint3d xyz = tmpCurve->GetPole (iPoint);
            if (useWeights)
                xyz.z = zDepth * tmpCurve->GetWeight (iPoint);
            else
                xyz.z = zDepth;
            tmpCurve->SetPole (iPoint, xyz);
            }

        currLStyle->_GetComponent()->_StrokeBSplineCurve (this, currLsSymb, tmpCurve.get (), NULL);
        return;
        }

    GetIDrawGeom().DrawBSplineCurve2d (bcurve, false, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d ViewContext::GetViewToLocal () const
    {
    DMatrix4d       viewToLocal = GetFrustumToView().M1;
    Transform       frustumToLocalTransform;

    if (SUCCESS == GetCurrFrustumToLocalTrans (frustumToLocalTransform) && !
        frustumToLocalTransform.IsIdentity())
        {
        DMatrix4d       frustumToLocal = DMatrix4d::From (frustumToLocalTransform);

        viewToLocal.productOf (&frustumToLocal, &viewToLocal);
        }
    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d ViewContext::GetLocalToView() const
    {
    DMatrix4d       localToView    = GetFrustumToView().M0;
    Transform       localToFrustumTransform;

    if (SUCCESS == GetCurrLocalToFrustumTrans (localToFrustumTransform) && !
        localToFrustumTransform.IsIdentity())
        {
        DMatrix4d   localToFrustum = DMatrix4d::From (localToFrustumTransform);

        localToView.productOf (&localToView, &localToFrustum);
        }
    return localToView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewContext::GetPixelSizeAtPoint (DPoint3dCP inPoint) const
    {
    DPoint3d    vec[2];

    if (NULL != inPoint)
        LocalToView (vec, inPoint, 1); // convert point to pixels
    else
        {
        DPoint3d    center = {.5, .5, .5};   // if they didn't give a point, use center of view.
        NpcToView (vec, &center, 1);
        }

    vec[1] = vec[0];
    vec[1].x += 1.0;

    // Convert pixels back to local coordinates and use the length as tolerance
    ViewToLocal (vec, vec, 2);

    return vec[0].distance (vec+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin    02/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetRasterAttInterface (IDrawRasterAttachment* pRasterAttInterface)
    {
    s_pRasterAttInterface = pRasterAttInterface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
IDrawRasterAttachment* ViewContext::GetRasterAttInterface ()
    {
    return s_pRasterAttInterface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PushDisplayStyle (DisplayStyleCP displayStyle, DgnModelP modelRef, bool inheritFromParent)
    {
#if defined (NEEDS_WORK_DGNITEM)
    IViewDrawR              output = GetIViewDraw ();
    CookedDisplayStyle      cookedStyle (displayStyle, modelRef, this, m_currentDisplayStyle, inheritFromParent);

    SetViewFlags (cookedStyle.GetViewFlags());
    output.PushRenderOverrides (*cookedStyle.GetViewFlags(), &cookedStyle);

    if (NULL == (m_currentDisplayStyle = output.GetDrawDisplayStyle()))         // Used to apply symbology overrides.
        return;     // This will occur if the DrawGeom does not maintain the display style call stack (NullContext).

    if (NULL != GetViewport ())
        m_currentDisplayStyle->OnFrustumChange(*_GetViewTarget(), *this);

    m_displayStyleStackMark++;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_PopDisplayStyle ()
    {
#if defined (NEEDS_WORK_DGNITEM)
    IViewDrawR  output = GetIViewDraw ();

    CookedDisplayStyleCP    currentStyle;
    if (NULL != (currentStyle =  output.GetDrawDisplayStyle()) &&
        NULL != currentStyle->m_displayHandler)
        currentStyle->m_displayHandler->PopStyle (currentStyle->m_displayHandlerSettings.get(), *this);

    output.PopRenderOverrides ();
    if (NULL != (m_currentDisplayStyle = output.GetDrawDisplayStyle()))
        SetViewFlags (m_currentDisplayStyle->GetViewFlags());

    if (m_displayStyleStackMark > 0)//m_displayStyleStackMark is a UInt32. don't go negative.
        m_displayStyleStackMark--;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_ClearZ ()
    {
    GetIViewDraw().ClearZ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jeff.Marker                     09/07
+---------------+---------------+---------------+---------------+---------------+------*/
CookedDisplayStyleCP ViewContext::_GetCurrentCookedDisplayStyle () const
    {
    return m_currentDisplayStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::RasterDisplayParams::RasterDisplayParams()
    : m_flags(0), m_contrast(50), m_brightness(50), m_greyScale(false), m_applyBinaryWhiteOnWhiteReversal(false), m_quality(1.0)
    {
    m_backgroundColor.red = m_backgroundColor.green = m_backgroundColor.blue = 0;        // Background color for binary image.
    m_foregroundColor.red = m_foregroundColor.green = m_foregroundColor.blue = 255;      // Foreground color for binary image.
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
void ViewContext::RasterDisplayParams::SetFlags(UInt32 flags)
    {
    m_flags = flags;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetContrast(Int8 value)
    {
    m_contrast = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_Contrast;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetBrightness(Int8 value)
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
void ViewContext::RasterDisplayParams::SetBackgroundColor(RgbColorDefCR value)
    {
    m_backgroundColor = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_BackgroundColor;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetForegroundColor(RgbColorDefCR value)
    {
    m_foregroundColor = value;
    m_flags |= ViewContext::RasterDisplayParams::RASTER_PARAM_ForegroundColor;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RasterDisplayParams::SetQualityFactor (double factor)
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
        m_context->GetTransformClipStack().Pop (*m_context);

    while (m_context->GetOverridesStackDepth() > m_hdrOvrMark)
        m_context->PopOverrides();

    if (m_pushedRange)
        {
        m_context->GetIDrawGeom().PopBoundingRange();
        m_context->SetCurrParentRangeResult (m_parentRangeResult);
        m_pushedRange = false;
        }

    while (m_context->m_displayStyleStackMark > m_displayStyleStackMark)
        m_context->PopDisplayStyle();

    m_context->SetIgnoreScaleForDimensions (m_ignoreScaleForDimensions);
    m_context->SetIgnoreScaleForMultilines (m_ignoreScaleForMultilines);
    m_context->SetApplyRotationToDimView (m_applyRotationToDimView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
inline void ViewContext::ContextMark::SetNow()
    {
    m_transClipMark                 = m_context->GetTransClipDepth();
    m_hdrOvrMark                    = m_context->GetOverridesStackDepth();
    m_parentRangeResult             = m_context->GetCurrParentRangeResult();
    m_displayStyleStackMark         = m_context->m_displayStyleStackMark;
    m_pushedRange                   = false;

    m_ignoreScaleForDimensions      = m_context->GetIgnoreScaleForDimensions ();
    m_ignoreScaleForMultilines      = m_context->GetIgnoreScaleForMultilines ();
    m_applyRotationToDimView        = m_context->GetApplyRotationToDimView ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ContextMark::ContextMark (ViewContextP context)
    {
    if (NULL == (m_context = context))
        Init (context);
    else
        SetNow();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ContextMark::ContextMark (ViewContextR context, ElementHandleCR testForRange)
    {
    m_context = &context;
    SetNow();

    IViewDrawR  output = context.GetIViewDraw ();

    DgnElementCP  el = testForRange.GetElementCP();
    DRange3dCP scanRange = ElemRangeIndex::GetIndexRange(testForRange);

    if (DisplayHandler::Is3dElem (el))
        {
        context.SetCurrParentRangeResult(output.PushBoundingRange3d(&scanRange->low));
        }
    else
        {
        DPoint2d    range[2];
        DisplayHandler::GetDPRange (range, scanRange);
        context.SetCurrParentRangeResult(output.PushBoundingRange2d (range, context.GetDisplayPriority()));
        }

    m_pushedRange = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElemHeaderOverrides::ElemHeaderOverrides ()
    {
    memset (&m_flags, 0, sizeof (m_flags));
    memset (&m_symb, 0, sizeof (m_symb));
    memset (&m_styleParams, 0, sizeof (m_styleParams));

    m_levelCodeDiff = 0;
    m_dispPriority  = 0;
    m_elementClass  = DgnElementClass::Primary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemHeaderOverrides::Init (SCOverride const* flags, LevelId baseLevel, int levelCodeDiff, UInt32 dispPriority, DgnElementClass elementClass, Symbology const* symb, LineStyleParamsCP styleParams)
    {
    // NOTE: Contructor has already initialized all fields...
    if (flags)
        m_flags = *flags;

    m_level         = baseLevel;
    m_levelCodeDiff = levelCodeDiff;
    m_dispPriority  = dispPriority;
    m_elementClass  = elementClass;

    if (symb)
        m_symb = *symb;

    if (styleParams)
        m_styleParams = *styleParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/05
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId ElemHeaderOverrides::AdjustLevel (LevelId inLevel) const
    {
    if (m_flags.level)
        return m_level;

    // if this element has "byCell" override, return the parent info
    return (inLevel.GetValueUnchecked() == LEVEL_BYCELL) ? m_level : inLevel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemHeaderOverrides::MergeFrom (ElemHeaderOverrides const* o1, ElemHeaderOverrides const* o2)
    {
    if (NULL == o1 && NULL == o2)
        return;

    if (NULL == o1)
        {
        memcpy (this, o2, sizeof(ElemHeaderOverrides));
        return;
        }

    // start out using o1
    memcpy (this, o1, sizeof(ElemHeaderOverrides));

    if (NULL == o2)
        return;

    // In the absence of overrides o2 values should be used for BYCELL values in o1...
    if (COLOR_BYCELL == m_symb.color)
        m_symb.color = o2->m_symb.color;

    if (STYLE_BYCELL == m_symb.style)
        m_symb.style = o2->m_symb.style;

    if (WEIGHT_BYCELL == m_symb.weight)
        m_symb.weight = o2->m_symb.weight;

    if (LEVEL_BYCELL == m_level.GetValueUnchecked())
        m_level = o2->m_level;

    // Account for override flags...preserve previous override from 02...
    if (o2->m_flags.level)
        {
        m_flags.level = 1;
        m_level = o2->m_level;
        }

    if (o2->m_flags.relative)
        {
        m_flags.relative = 1;
        m_levelCodeDiff = o2->m_levelCodeDiff;
        }

    if (o2->m_flags.color)
        {
        m_flags.color = 1;
        m_symb.color = o2->m_symb.color;
        }

    if (o2->m_flags.weight)
        {
        m_flags.weight = 1;
        m_symb.weight = o2->m_symb.weight;
        }

    if (o2->m_flags.style)
        {
        m_flags.style = 1;
        m_symb.style = o2->m_symb.style;
        memcpy (&m_styleParams, &o2->m_styleParams, sizeof(m_styleParams));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::IsMonochromeDisplayStyleActive ()
    {
#if defined (NEEDS_WORK_DGNITEM)
    CookedDisplayStyleCP currDispStyle = _GetCurrentCookedDisplayStyle ();

    return (currDispStyle && currDispStyle->StylePresent () && currDispStyle->m_flags.m_elementColor);
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymb::ElemMatSymb ()
    {
    Init ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::Init ()
    {
    m_lineColorTBGR     = 0;
    m_fillColorTBGR     = 0;
    m_lineColorIndex    = DgnColorMap::INDEX_Invalid;
    m_fillColorIndex    = DgnColorMap::INDEX_Invalid;
    m_elementStyle      = 0;
    m_isFilled          = false;
    m_isBlankingRegion  = false;
    m_extSymbID         = 0;
    m_material         = NULL;
    m_rasterWidth       = 1;
    m_rasterPat         = 0;
    m_patternParams     = NULL;

    m_lStyleSymb.Clear ();
    m_gradient = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::Init (ViewContextR context, UInt32 lineColor, UInt32 fillColor, UInt32 lineWeight, Int32 lineCode)
    {
    bool        isTrueColor;
    UInt32      colorIndex;
    IntColorDef colorDef;
    StatusInt   extractStatus;

    extractStatus = context.GetDgnProject().Colors().Extract (&colorDef, &colorIndex, &isTrueColor, NULL, NULL, lineColor);
    BeAssert (SUCCESS == extractStatus && "ERROR: ElemMatSymb::Init Expects valid color index! (0-255 or extended)");

    // NOTE: Get TBGR from a reference color map for non-extended colors...
    if (isTrueColor)
        {
        m_lineColorTBGR  = colorDef.m_int;
        m_lineColorIndex = DgnColorMap::INDEX_Invalid;
        }
    else
        {
        m_lineColorTBGR  = context.GetIndexedColor (lineColor);
        m_lineColorIndex = colorIndex;
        }

    // When fill is invalid (no fill) it's set the same as line color...
    if (INVALID_COLOR == fillColor)
        {
        m_fillColorTBGR  = m_lineColorTBGR;
        m_fillColorIndex = m_lineColorIndex;
        m_isFilled       = false;
        }
    else
        {
        extractStatus = context.GetDgnProject().Colors().Extract(&colorDef, &colorIndex, &isTrueColor, NULL, NULL, fillColor);
        BeAssert (SUCCESS == extractStatus && "ERROR: ElemMatSymb::Init Expects valid fill color index! (0-255 or extended)");

        // NOTE: Get TBGR from a reference color map for non-extended colors...
        if (isTrueColor)
            {
            m_fillColorTBGR  = colorDef.m_int;
            m_fillColorIndex = DgnColorMap::INDEX_Invalid;
            }
        else
            {
            m_fillColorTBGR  = context.GetIndexedColor (fillColor);
            m_fillColorIndex = colorIndex;
            }

        m_isFilled = true;
        }

    m_elementStyle = IS_LINECODE (lineCode) ? lineCode : 0;
    m_rasterPat    = context.GetIndexedLinePattern (m_elementStyle);
    m_rasterWidth  = context.GetIndexedLineWidth (lineWeight);

    m_extSymbID = 0;
    m_material = NULL;

    m_isBlankingRegion = false;
    m_gradient = NULL;

    m_lStyleSymb.Clear ();

    m_patternParams = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static byte     screenColor (byte color, double factor)
    {
    UInt32 tmp = color + (UInt32) (factor * (255 - color));
    LIMIT_RANGE (0, 255, tmp);
    return  (byte) tmp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AndrewEdge      08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     applyScreeningFactor (RgbaColorDef* rgb, double screeningFactor)
    {
    rgb->red   = screenColor (rgb->red,   screeningFactor);
    rgb->green = screenColor (rgb->green, screeningFactor);
    rgb->blue  = screenColor (rgb->blue,  screeningFactor);
    }

#define ACAD_LINEWEIGHT_SIGNATURE   (0x80000000)
#define LINEWEIGHT_SIGNATURE_BITS   (0xff000000)
#define ACAD_LINEWEIGHT_MAX         (211)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32   remapAcadLineWeight (UInt32 lineWeight)
    {
    if (ACAD_LINEWEIGHT_SIGNATURE != (lineWeight & LINEWEIGHT_SIGNATURE_BITS))
        return lineWeight;

    lineWeight = (lineWeight & ~LINEWEIGHT_SIGNATURE_BITS);

    // maximum autocad line weight is 211.
    if (lineWeight > ACAD_LINEWEIGHT_MAX)
        lineWeight = ACAD_LINEWEIGHT_MAX;

#ifdef BEIJING_DGNPLATFORM_WIP_DWG
    double          styleScale, weightScale;
    if (SUCCESS != dwgSaveSettings_getLineCodeAndWeightScale (&styleScale, &weightScale))
        weightScale = DEFAULT_DWG_WEIGHT_SCALE;

    return (UInt32) (0.5 + 0.5 * weightScale * lineWeight);
#endif
    return lineWeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::FromResolvedElemDisplayParams (ElemDisplayParamsCR elParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
    Init ();

    // We store the style index returned from LineStyleSymb::FromResolvedElemDisplayParams in order to
    // provide PlotContext with the information it needs to handle custom line styles that are
    // mapped to cosmetic line styles.  PlotContext is different from DrawContext in that it
    // uses QV extended symbologies for cosmetic line styles rather than pattern codes. TR 225586.
    m_elementStyle = m_lStyleSymb.FromResolvedElemDisplayParams (elParams, context, startTangent, endTangent);
    m_rasterPat    = context.GetIndexedLinePattern (m_elementStyle);
    m_rasterWidth  = context.GetIndexedLineWidth (remapAcadLineWeight (elParams.GetWeight ()));

    m_lineColorTBGR = m_fillColorTBGR = elParams.GetLineColorTBGR (); // NOTE: In case no fill is defined it should be set the same as line color...

    switch (elParams.GetLineColorIndex ())
        {
        case INVALID_COLOR: // Color defined by true TBGR value...
            m_lineColorIndex = m_fillColorIndex = DgnColorMap::INDEX_Invalid;
            break;

        case COLOR_BYCELL:  // Set special ByCell color index (CACHED_COLOR_INDEX)...
            m_lineColorIndex = m_fillColorIndex = DgnColorMap::INDEX_ByCell;
            break;

        case COLOR_BYLEVEL: // Set special ByLevel color index (CACHED_COLOR_INDEX)...(NOTE: Change to/from BG color invalidates QvElem!)
            m_lineColorIndex = m_fillColorIndex = DgnColorMap::INDEX_Background == elParams.GetLineColor() ? DgnColorMap::INDEX_Background : DgnColorMap::INDEX_ByLevel;
            break;

        default:            // Set 0-255 color index, invalid for extended colors...
            m_lineColorIndex = m_fillColorIndex = DgnColorMap::CanBeTrueColor(elParams.GetLineColor()) ? DgnColorMap::INDEX_Invalid : elParams.GetLineColor();
            break;
        }

    if (FillDisplay::Never != elParams.GetFillDisplay ())
        {
        if (NULL != elParams.GetGradient ())
            {
            m_gradient = GradientSymb::Create ();
            m_gradient->CopyFrom (*elParams.GetGradient());

            m_fillColorTBGR = 0x00ffffff; // Fill should be set to opaque white for gradient texture...
            m_fillColorIndex = DgnColorMap::INDEX_Invalid;
            }
        else
            {
            m_fillColorTBGR = elParams.GetFillColorTBGR ();

            switch (elParams.GetFillColorIndex ())
                {
                case INVALID_COLOR: // Color defined by true TBGR value...
                    m_fillColorIndex = DgnColorMap::INDEX_Invalid;
                    break;

                case COLOR_BYCELL:  // Set special ByCell color index (CACHED_COLOR_INDEX)...
                    m_fillColorIndex = DgnColorMap::INDEX_ByCell;
                    break;

                case COLOR_BYLEVEL: // Set special ByLevel color index (CACHED_COLOR_INDEX)...(NOTE: Change to/from BG color invalidates QvElem!)
                    m_fillColorIndex = DgnColorMap::INDEX_Background == elParams.GetFillColor () ? DgnColorMap::INDEX_Background : DgnColorMap::INDEX_ByLevel;
                    break;

                default:            // Set 0-255 color index, invalid for extended colors...
                    m_fillColorIndex = DgnColorMap::CanBeTrueColor(elParams.GetFillColor()) ? DgnColorMap::INDEX_Invalid : elParams.GetFillColor();
                    break;
                }
            }

        m_isFilled = true;
        m_isBlankingRegion = (FillDisplay::Blanking == elParams.GetFillDisplay ());
        }

    SetMaterial (elParams.GetMaterial (), &context); // Call method so that geometry map is created as needed...

    if (0.0 != elParams.GetTransparency ())
        {
        UInt32  netTransparency = (UInt32) (elParams.GetTransparency () * RMAXUI4);

        if (netTransparency > MAX_Alpha)
            netTransparency = MAX_Alpha; // Don't allow complete transparency.

        // discard the lowest 24 bits, since we only have 8 bits to work with.
        netTransparency = netTransparency & 0xff000000;

        m_lineColorTBGR |= netTransparency;
        m_fillColorTBGR |= netTransparency;
        }

    if (elParams.IsScreeningSet () && (elParams.GetScreening () >= SCREENING_Full) && (elParams.GetScreening () < SCREENING_None))
        {
        double  screeningFactor = (SCREENING_None - elParams.GetScreening ()) / SCREENING_None;

        applyScreeningFactor ((RgbaColorDef*) &m_lineColorTBGR, screeningFactor);
        applyScreeningFactor ((RgbaColorDef*) &m_fillColorTBGR, screeningFactor);
        m_lineColorIndex = m_fillColorIndex = DgnColorMap::INDEX_Invalid; // Always TBGR...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::FromNaturalElemDisplayParams (ElemDisplayParamsR elParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
    elParams.Resolve (context);
    FromResolvedElemDisplayParams (elParams, context, startTangent, endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* compare two ElemMatSymb's to see if they're the same.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemMatSymb::operator==(ElemMatSymbCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_lineColorTBGR    != m_lineColorTBGR    ||
        rhs.m_fillColorTBGR    != m_fillColorTBGR    ||
        rhs.m_lineColorIndex   != m_lineColorIndex   ||
        rhs.m_fillColorIndex   != m_fillColorIndex   ||
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

    if (m_materialDetail.IsValid () && rhs.m_materialDetail.IsValid ())
        {
        if (!(m_materialDetail->Equals (*rhs.m_materialDetail)))
            return false;
        }
    else if (m_materialDetail.IsNull () != rhs.m_materialDetail.IsNull ())
        return false;
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemMatSymb::SetMaterial (MaterialCP material, ViewContextP seedContext)
    {
#ifdef WIP_VANCOUVER_MERGE // material
    m_material = material;

    if (NULL != material &&
        NULL != seedContext &&
        material->NeedsQvGeometryTexture ())
        {
        bool                useCellColors;
        EditElementHandle   eh;

        if (seedContext->GetIViewDraw ().IsOutputQuickVision() &&
            SUCCESS == material->GetGeometryMapDefinition (eh, useCellColors))
            seedContext->GetIViewDraw ().DefineQVGeometryMap (*material, eh, NULL, useCellColors, *seedContext);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymb::ElemMatSymb (ElemMatSymbCR rhs)
    {
    m_lineColorTBGR     =       rhs.m_lineColorTBGR;
    m_fillColorTBGR     =       rhs.m_fillColorTBGR;
    m_lineColorIndex    =       rhs.m_lineColorIndex;
    m_fillColorIndex    =       rhs.m_fillColorIndex;
    m_elementStyle      =       rhs.m_elementStyle;
    m_isFilled          =       rhs.m_isFilled;
    m_isBlankingRegion  =       rhs.m_isBlankingRegion;
    m_extSymbID         =       rhs.m_extSymbID;
    m_material          =       rhs.m_material;
    m_rasterWidth       =       rhs.m_rasterWidth;
    m_rasterPat         =       rhs.m_rasterPat;
    m_lStyleSymb        =       rhs.m_lStyleSymb;
    m_gradient          =       rhs.m_gradient;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemMatSymbR ElemMatSymb::operator=(ElemMatSymbCR rhs)
    {
    m_lineColorTBGR     =       rhs.m_lineColorTBGR;
    m_fillColorTBGR     =       rhs.m_fillColorTBGR;
    m_lineColorIndex    =       rhs.m_lineColorIndex;
    m_fillColorIndex    =       rhs.m_fillColorIndex;
    m_elementStyle      =       rhs.m_elementStyle;
    m_isFilled          =       rhs.m_isFilled;
    m_isBlankingRegion  =       rhs.m_isBlankingRegion;
    m_extSymbID         =       rhs.m_extSymbID;
    m_material          =       rhs.m_material;
    m_rasterWidth       =       rhs.m_rasterWidth;
    m_rasterPat         =       rhs.m_rasterPat;
    m_lStyleSymb        =       rhs.m_lStyleSymb;
    m_gradient          =       rhs.m_gradient;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void OvrMatSymb::Clear ()
    {
    SetFlags (MATSYMB_OVERRIDE_None);
    m_matSymb.Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  OvrMatSymb::SetLineStyle (Int32 styleNo, DgnModelR modelRef, DgnModelR styleDgnModel, LineStyleParamsCP lStyleParams, ViewContextR context, DPoint3dCP startTangent, DPoint3dCP endTangent)
    {
#ifdef WIP_VANCOUVER_MERGE // linestyle
    m_matSymb.GetLineStyleSymbR().FromResolvedStyle (styleNo, modelRef, styleDgnModel, lStyleParams, context, startTangent, endTangent);
    m_flags |= MATSYMB_OVERRIDE_Style;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParams::ElemDisplayParams ()
    {
    Init ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParams::ElemDisplayParams (ElemDisplayParamsCR rhs)
    {
    m_isRenderable              = rhs.m_isRenderable;
    m_hasThickness              = rhs.m_hasThickness;
    m_isCapped                  = rhs.m_isCapped;
    m_hasScreening              = rhs.m_hasScreening;
    m_hasLineJoin               = rhs.m_hasLineJoin;
    m_hasLineCap                = rhs.m_hasLineCap;
    m_hasLineWeightMM           = rhs.m_hasLineWeightMM;
    m_ignoreLevelSymb           = rhs.m_ignoreLevelSymb;
    m_materialIsAttached        = rhs.m_materialIsAttached;
    m_isValidLineColorTBGR      = rhs.m_isValidLineColorTBGR;
    m_isValidFillColorTBGR      = rhs.m_isValidFillColorTBGR;
    m_subLevel                  = rhs.m_subLevel;
    m_elementClass              = rhs.m_elementClass;
    m_elmPriority               = rhs.m_elmPriority;
    m_netPriority               = rhs.m_netPriority;
    m_symbology                 = rhs.m_symbology;
    m_lineColorIndex            = rhs.m_lineColorIndex;
    m_lineColorTBGR             = rhs.m_lineColorTBGR;
    m_fillColor                 = rhs.m_fillColor;
    m_fillColorIndex            = rhs.m_fillColorIndex;
    m_fillColorTBGR             = rhs.m_fillColorTBGR;
    m_fillDisplay               = rhs.m_fillDisplay;
    m_transparency              = rhs.m_transparency;
    m_thicknessVector           = rhs.m_thicknessVector;
    m_lineCap                   = rhs.m_lineCap;
    m_lineJoin                  = rhs.m_lineJoin;
    m_screening                 = rhs.m_screening;
    m_widthMM                   = rhs.m_widthMM;
    m_material                  = rhs.m_material;
    m_styleParams               = rhs.m_styleParams;
    m_gradient                  = rhs.m_gradient;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsR ElemDisplayParams::operator=(ElemDisplayParamsCR rhs)
    {
    m_isRenderable              = rhs.m_isRenderable;
    m_hasThickness              = rhs.m_hasThickness;
    m_isCapped                  = rhs.m_isCapped;
    m_hasScreening              = rhs.m_hasScreening;
    m_hasLineJoin               = rhs.m_hasLineJoin;
    m_hasLineCap                = rhs.m_hasLineCap;
    m_hasLineWeightMM           = rhs.m_hasLineWeightMM;
    m_ignoreLevelSymb           = rhs.m_ignoreLevelSymb;
    m_materialIsAttached        = rhs.m_materialIsAttached;
    m_isValidLineColorTBGR      = rhs.m_isValidLineColorTBGR;
    m_isValidFillColorTBGR      = rhs.m_isValidFillColorTBGR;
    m_subLevel                  = rhs.m_subLevel;
    m_elementClass              = rhs.m_elementClass;
    m_elmPriority               = rhs.m_elmPriority;
    m_netPriority               = rhs.m_netPriority;
    m_symbology                 = rhs.m_symbology;
    m_lineColorIndex            = rhs.m_lineColorIndex;
    m_lineColorTBGR             = rhs.m_lineColorTBGR;
    m_fillColor                 = rhs.m_fillColor;
    m_fillColorIndex            = rhs.m_fillColorIndex;
    m_fillColorTBGR             = rhs.m_fillColorTBGR;
    m_fillDisplay               = rhs.m_fillDisplay;
    m_transparency              = rhs.m_transparency;
    m_thicknessVector           = rhs.m_thicknessVector;
    m_lineCap                   = rhs.m_lineCap;
    m_lineJoin                  = rhs.m_lineJoin;
    m_screening                 = rhs.m_screening;
    m_widthMM                   = rhs.m_widthMM;
    m_material                  = rhs.m_material;
    m_styleParams               = rhs.m_styleParams;
    m_gradient                  = rhs.m_gradient;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::Init ()
    {
    memset (this, 0, offsetof (ElemDisplayParams, m_styleParams));

    m_fillColor = INVALID_COLOR; // Default to invalid index not 0...
    m_styleParams.Init ();
    m_gradient = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemDisplayParams::operator==(ElemDisplayParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_isRenderable            != m_isRenderable            ||
        rhs.m_hasThickness            != m_hasThickness            ||
        rhs.m_isCapped                != m_isCapped                ||
        rhs.m_hasScreening            != m_hasScreening            ||
        rhs.m_hasLineJoin             != m_hasLineJoin             ||
        rhs.m_hasLineCap              != m_hasLineCap              ||
        rhs.m_hasLineWeightMM         != m_hasLineWeightMM         ||
        rhs.m_ignoreLevelSymb         != m_ignoreLevelSymb         ||
        rhs.m_materialIsAttached      != m_materialIsAttached      ||
        rhs.m_isValidLineColorTBGR    != m_isValidLineColorTBGR    ||
        rhs.m_isValidFillColorTBGR    != m_isValidFillColorTBGR)
        return false;

    if (rhs.m_subLevel     != m_subLevel     ||
        rhs.m_elementClass != m_elementClass ||
        rhs.m_elmPriority  != m_elmPriority  ||
        rhs.m_netPriority  != m_netPriority)
        return false;

    if (rhs.m_symbology.color  != m_symbology.color   ||
        rhs.m_symbology.style  != m_symbology.style   ||
        rhs.m_symbology.weight != m_symbology.weight)
        return false;

    if (rhs.m_lineColorIndex != m_lineColorIndex ||
        rhs.m_lineColorTBGR  != m_lineColorTBGR  ||
        rhs.m_fillColor      != m_fillColor      ||
        rhs.m_fillColorIndex != m_fillColorIndex ||
        rhs.m_fillColorTBGR  != m_fillColorTBGR  ||
        rhs.m_fillDisplay    != m_fillDisplay    ||
        rhs.m_transparency   != m_transparency)
        return false;

    if (!rhs.m_thicknessVector.IsEqual (m_thicknessVector))
        return false;

    if (rhs.m_lineCap   != m_lineCap   ||
        rhs.m_lineJoin  != m_lineJoin  ||
        rhs.m_screening != m_screening ||
        rhs.m_widthMM   != m_widthMM   ||
        rhs.m_material  != m_material)
        return false;

    if (!(m_gradient == rhs.m_gradient))
        return false;

    if (!(m_styleParams == rhs.m_styleParams))
        return false;

#if defined (NEEDS_WORK_MATERIAL)
    if (m_materialDetail.IsValid () && rhs.m_materialDetail.IsValid ())
        {
        if (!(m_materialDetail->Equals (*rhs.m_materialDetail)))
            return false;
        }
    else if (m_materialDetail.IsNull () != rhs.m_materialDetail.IsNull ())
        return false;
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::SetLineColor (UInt32 elementColor)
    {
    m_symbology.color = m_lineColorIndex = elementColor;
    m_isValidLineColorTBGR = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::SetLineColorTBGR (UInt32 colorTBGR)
    {
    m_symbology.color = m_lineColorIndex = INVALID_COLOR;
    m_lineColorTBGR = colorTBGR;
    m_isValidLineColorTBGR = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::SetFillColor (UInt32 elementColor)
    {
    m_fillColor = m_fillColorIndex = elementColor;
    m_isValidFillColorTBGR = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::SetFillColorTBGR (UInt32 colorTBGR)
    {
    m_fillColor = m_fillColorIndex = INVALID_COLOR;
    m_fillColorTBGR = colorTBGR;
    m_isValidFillColorTBGR = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemDisplayParams::ResolveByLevel (DgnLevels::SubLevel::Appearance const& appear)
    {
    // if there's no by-level stuff, stop so we don't have to get the level's symbology
    if ((COLOR_BYLEVEL == m_symbology.color) ||
        (FillDisplay::Never != m_fillDisplay && COLOR_BYLEVEL == m_fillColor) ||
        (STYLE_BYLEVEL == m_symbology.style) ||
        (WEIGHT_BYLEVEL == m_symbology.weight))
        {
        if (COLOR_BYLEVEL == m_symbology.color)
            {
            m_symbology.color = appear.GetColor();
            m_isValidLineColorTBGR = false;
            }

        if (FillDisplay::Never != m_fillDisplay && COLOR_BYLEVEL == m_fillColor)
            {
            m_fillColor = appear.GetColor ();
            m_isValidFillColorTBGR = false;
            }

        if (WEIGHT_BYLEVEL == m_symbology.weight)
            m_symbology.weight = appear.GetWeight();

        if (STYLE_BYLEVEL == m_symbology.style)
            {
            m_symbology.style = appear.GetStyle();
            }

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemDisplayParams::ResolveByCell (ElemHeaderOverridesCP ovr, DgnLevels::SubLevel::Appearance const& appear)
    {
    if (NULL == ovr)
        {
        // If BYCELL color and not in a cell...color is ALWAYS color 0 (white w/ACAD color table)
        if (COLOR_BYCELL == m_symbology.color)
            {
            m_symbology.color = 0;
            m_isValidLineColorTBGR = false;
            }

        if (COLOR_BYCELL == m_fillColor)
            {
            m_fillColor = 0;
            m_isValidFillColorTBGR = false;
            }

        // Use BYLEVEL style from default level (should be continuous for DWG...)
        if (STYLE_BYCELL == m_symbology.style)
            m_symbology.style = appear.GetStyle();

        // Don't leave these with BYCELL value...
        if (WEIGHT_BYCELL == m_symbology.weight)
            m_symbology.weight = 0;

        if (DISPLAYPRIORITY_BYCELL == m_elmPriority)
            m_netPriority = CalcDefaultNetDisplayPriority (0, &appear); // Assume we don't need to involve raster handler...

        return false;
        }

    m_subLevel = SubLevelId(ovr->AdjustLevel(m_subLevel.GetLevel()));

    if (COLOR_BYCELL == m_symbology.color)
        {
        m_symbology.color = ovr->GetColor ();
        m_isValidLineColorTBGR = false;
        }

    if (COLOR_BYCELL == m_fillColor)
        {
        m_fillColor = ovr->GetColor ();
        m_isValidFillColorTBGR = false;
        }

    if (WEIGHT_BYCELL == m_symbology.weight)
        m_symbology.weight = ovr->GetWeight ();

    if (STYLE_BYCELL == m_symbology.style)
        m_symbology.style = ovr->GetLineStyle (); // NOTE: Ignore style params...these always come from element for DWG...

    if (DISPLAYPRIORITY_BYCELL == m_elmPriority)
        m_netPriority = CalcDefaultNetDisplayPriority (ovr->GetDisplayPriority (), &appear); // Assume we don't need to call DisplayHandler::CalcDisplayPriority for rasters...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::ResolveColorTBGR (ViewContextR context)
    {
    IntColorDef     colorDef;

    // Compute TBGR from effective line color index, if index is invalid it must be a true rgb and needs to be left alone...
    if (!m_isValidLineColorTBGR && INVALID_COLOR != m_symbology.color)
        {
        if (DgnColorMap::CanBeTrueColor (m_symbology.color))
            {
            UInt32  closeIndex = 0;

            if (SUCCESS != context.GetDgnProject().Colors().Extract (&colorDef, &closeIndex, NULL, NULL, NULL, m_symbology.color))
                colorDef.m_int = context.GetIndexedColor (closeIndex); // Use closeIndex as this is what gets shown by Element Info, etc....

            m_lineColorTBGR = colorDef.m_int;
            }
        else
            {
            m_lineColorTBGR = context.GetIndexedColor (m_symbology.color); // Use context method as Plotting overrides colorMap...
            }

        m_isValidLineColorTBGR = true;
        }

    // Compute TBGR from effective fill color index, if index is invalid it must be a true rgb (or not filled) and needs to be left alone...
    if (!m_isValidFillColorTBGR && INVALID_COLOR != m_fillColor)
        {
        if (DgnColorMap::CanBeTrueColor (m_fillColor))
            {
            UInt32  closeIndex = 0;

            if (SUCCESS != context.GetDgnProject().Colors().Extract (&colorDef, &closeIndex, NULL, NULL, NULL, m_fillColor))
                colorDef.m_int = context.GetIndexedColor (closeIndex); // Use closeIndex as this is what gets shown by Element Info, etc....

            m_fillColorTBGR = colorDef.m_int;
            }
        else
            {
            m_fillColorTBGR = context.GetIndexedColor (m_fillColor); // Use context method as Plotting overrides colorMap...
            }

        m_isValidFillColorTBGR = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::Resolve (ViewContextR context)
    {
    // Restore ignored properties...
    context.GetDisplayParamsIgnores ().Apply (*this);

    // Get effective value for ByLevel and ByCell properties...
    DgnLevels::SubLevel::Appearance appearance;
    if (context.GetViewport() != NULL)
        appearance = context.GetViewport()->GetViewController().GetSubLevelAppearance(GetSubLevelId());

    // Resolve ByCell values and relative levels from parent.
    ResolveByCell (context.GetHeaderOvr (), appearance);

    // Resolve ByLevel values.
    ResolveByLevel (appearance);

    // Set TBGR color to use for draw...
    ResolveColorTBGR (context);

    Int32   displayRange[2];

    if (context.GetDisplayPriorityRange (displayRange[0], displayRange[1]))
        LIMIT_RANGE (displayRange[0], displayRange[1], m_netPriority);

#ifdef WIP_VANCOUVER_MERGE // material
    // If no material defined yet, look for assignment.
    if (context.GetWantMaterials () && NULL == m_material)
        SetMaterial (MaterialManager::GetManagerR ().FindMaterialBySymbology (NULL, m_level, m_symbology.color, *model, false, false, &context));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElemDisplayParams::ApplyParentOverrides (ElemHeaderOverridesCP ovr)
    {
    if (NULL == ovr)
        return false;

    if (ovr->GetFlags ().color)
        {
        SetLineColor (ovr->GetColor ());

        if (!m_gradient.IsValid ())
            SetFillColor (ovr->GetColor ());
        }

    if (ovr->GetFlags ().style)
        SetLineStyle (ovr->GetLineStyle (), ovr->GetLineStyleParams ());

    if (ovr->GetFlags ().weight)
        SetWeight (ovr->GetWeight ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32 getElementDisplayPriority (Int32 elValue)
    {
    return (DISPLAYPRIORITY_BYCELL == elValue) ? 0 : elValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 ElemDisplayParams::CalcDefaultNetDisplayPriority (Int32 priority, DgnLevels::SubLevel::Appearance const* appearance)
    {
    Int32           netPriority = getElementDisplayPriority (priority);

    if (appearance != NULL)
        netPriority += appearance->GetDisplayPriority();

    LIMIT_RANGE (-Viewport::GetDisplayPriorityFrontPlane(), Viewport::GetDisplayPriorityFrontPlane(), netPriority);

    return netPriority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParams::SetElementDisplayPriority (Int32 priority, bool is3d, DgnLevels::SubLevel::Appearance const* appearance)
    {
    m_elmPriority = is3d ? 0 : priority;
    m_netPriority = is3d ? 0 : CalcDefaultNetDisplayPriority (m_elmPriority, appearance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsStateSaver::ElemDisplayParamsStateSaver (ElemDisplayParamsR elParams, bool restoreLevel, bool restoreLineColor, bool restoreFillColor, 
                                    bool restoreLineStyle, bool restoreWeight) : m_elParams (elParams)
    {
    m_elParamsSaved    = m_elParams;
    m_restoreLevel     = restoreLevel;
    m_restoreLineColor = restoreLineColor;
    m_restoreFillColor = restoreFillColor;
    m_restoreLineStyle = restoreLineStyle;
    m_restoreWeight    = restoreWeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsStateSaver::~ElemDisplayParamsStateSaver ()
    {
    if (m_restoreLevel)
        {
        m_elParams.m_subLevel                = m_elParamsSaved.m_subLevel;
        }

    if (m_restoreLineColor)
        {
        m_elParams.m_symbology.color         = m_elParamsSaved.m_symbology.color;
        m_elParams.m_lineColorIndex          = m_elParamsSaved.m_lineColorIndex;
        m_elParams.m_lineColorTBGR           = m_elParamsSaved.m_lineColorTBGR;
        m_elParams.m_isValidLineColorTBGR    = m_elParamsSaved.m_isValidLineColorTBGR;
        }

    if (m_restoreFillColor)
        {
        m_elParams.m_fillColor               = m_elParamsSaved.m_fillColor;
        m_elParams.m_fillColorIndex          = m_elParamsSaved.m_fillColorIndex;
        m_elParams.m_fillColorTBGR           = m_elParamsSaved.m_fillColorTBGR;
        m_elParams.m_isValidFillColorTBGR    = m_elParamsSaved.m_isValidFillColorTBGR;
        m_elParams.m_fillDisplay             = m_elParamsSaved.m_fillDisplay;
        }

    if (m_restoreLineStyle)
        {
        m_elParams.m_symbology.style         = m_elParamsSaved.m_symbology.style;
        m_elParams.m_styleParams             = m_elParamsSaved.m_styleParams;
        }

    if (m_restoreWeight)
        {
        m_elParams.m_symbology.weight        = m_elParamsSaved.m_symbology.weight;
        }

    // Restore/Correct a material assignment change that resulted from a level/color change...
    if (!m_elParams.m_materialIsAttached && (m_elParams.m_material != m_elParamsSaved.m_material) && (m_restoreLevel || m_restoreLineColor))
        {
#ifdef WIP_VANCOUVER_MERGE // material
        bool    levelIsChanged = (m_elParams.m_level != m_elParamsSaved.m_level);
        bool    colorIsChanged = (m_elParams.m_symbology.color != m_elParamsSaved.m_symbology.color);

        if (levelIsChanged || colorIsChanged)
            m_elParams.m_material = MaterialManager::GetManagerR ().FindMaterialBySymbology (NULL, m_elParams.m_level, m_elParams.m_symbology.color, m_modelRef, true, false, NULL);
        else
#endif
            m_elParams.m_material = m_elParamsSaved.m_material; // Level and color are same as original state...can restore previous material assignment...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ElemDisplayParamsIgnores::ElemDisplayParamsIgnores ()
    {
    m_ignoreSubLevel = false;
    m_ignoreColor    = false;
    m_ignoreWeight   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParamsIgnores::Set (ElemDisplayParamsCR elParams, bool ignoreSubLevel, bool ignoreColor, bool ignoreWeight)
    {
    if (!m_ignoreSubLevel && ignoreSubLevel)
        {
        m_ignoreSubLevel = true;
        m_subLevel = elParams.m_subLevel;
        }

    if (!m_ignoreColor && ignoreColor)
        {
        m_ignoreColor = true;
        m_lineColor = elParams.m_symbology.color;
        m_lineColorIndex = elParams.m_lineColorIndex;
        m_lineColorTBGR = elParams.m_lineColorTBGR;
        m_isValidLineColorTBGR = elParams.m_isValidLineColorTBGR;
        }

    if (!m_ignoreWeight && ignoreWeight)
        {
        m_ignoreWeight = true;
        m_weight = elParams.m_symbology.weight;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParamsIgnores::Apply (ElemDisplayParamsR elParams)
    {
    if (m_ignoreSubLevel)
        {
        elParams.m_subLevel = m_subLevel;
        }

    if (m_ignoreColor)
        {
        elParams.m_symbology.color = elParams.m_fillColor = m_lineColor;
        elParams.m_lineColorIndex = elParams.m_fillColorIndex = m_lineColorIndex;
        elParams.m_lineColorTBGR = elParams.m_fillColorTBGR = m_lineColorTBGR;
        elParams.m_isValidLineColorTBGR = elParams.m_isValidFillColorTBGR = m_isValidLineColorTBGR;
        }

    if (m_ignoreWeight)
        {
        elParams.m_symbology.weight = m_weight;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemDisplayParamsIgnores::Clear ()
    {
    m_ignoreSubLevel = false;
    m_ignoreColor    = false;
    m_ignoreWeight   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawQvElem (QvElem* qvElem, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight, bool is3d)
    {
    IViewDrawR  output = GetIViewDraw ();

    output._PushTransClip (trans, clip);

    OvrMatSymb  ovrMatSymb = m_ovrMatSymb;
    UInt32      flags      = ovrMatSymb.GetFlags();

    if (ignoreColor && !IsMonochromeDisplayStyleActive ()) // Monochrome style trumps ignoreColor...
        {
        UInt32  color = GetCurrLineColor (); // Want current effective color w/overrides!
        int     colorIndex = ((m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_Color) ? m_ovrMatSymb.GetLineColorIndex() : m_elemMatSymb.GetLineColorIndex());

        if (0 == (flags & MATSYMB_OVERRIDE_Color))
            ovrMatSymb.SetIndexedLineColorTBGR (colorIndex, color);

        if (0 == (flags & MATSYMB_OVERRIDE_ColorTransparency))
            ovrMatSymb.SetTransparentLineColor (color>>24);

        if (0 == (flags & MATSYMB_OVERRIDE_FillColor))
            ovrMatSymb.SetIndexedFillColorTBGR (colorIndex, color); // Shapes in lstyle always use opaque fill using line color...not GetFillColor!

        if (0 == (flags & MATSYMB_OVERRIDE_FillColorTransparency))
            ovrMatSymb.SetTransparentFillColor (color>>24);
        }

    if (ignoreWeight && (0 == (flags & MATSYMB_OVERRIDE_RastWidth)))
        ovrMatSymb.SetWidth (m_elemMatSymb.GetWidth());
    else if (!ignoreWeight && (0 != (flags & MATSYMB_OVERRIDE_RastWidth)))
        ovrMatSymb.SetFlags (ovrMatSymb.GetFlags() & ~MATSYMB_OVERRIDE_RastWidth);

    if (MATSYMB_OVERRIDE_None != ovrMatSymb.GetFlags())
        output.ActivateOverrideMatSymb (&ovrMatSymb);

    _DrawQvElem (qvElem, is3d);

    // Clear or restore previous overrides...
    if (MATSYMB_OVERRIDE_None != ovrMatSymb.GetFlags())
        output.ActivateOverrideMatSymb (&m_ovrMatSymb);

    output._PopTransClip();
    }

/*---------------------------------------------------------------------------------**//**
* draw a symbol into the current context
* @bsimethod                                                    Keith.Bentley   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawSymbol (IDisplaySymbol* symbol, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight)
    {
    QvCache*    symbolCache = T_HOST.GetGraphicsAdmin()._GetSymbolCache ();

    if (!symbolCache || CheckICachedDraw())
        {
        // if we're creating a cache elem already, we need to stroke the symbol into that elem by value
        IDrawGeomR      output = GetIDrawGeom ();
        ElemMatSymb     saveElemMatSymb;

        saveElemMatSymb = m_elemMatSymb; // Save current mat symb in case symbol changes...
        m_ignores.Set (m_currDisplayParams, true, ignoreColor, ignoreWeight); // NOTE: Symbol level is always inherited from base element...

        output._PushTransClip (trans, clip);
        symbol->_Draw (*this);
        output._PopTransClip ();

        // Restore/Activate original mat symb if changed...
        if (!(m_elemMatSymb == saveElemMatSymb))
            {
            output.ActivateMatSymb (&saveElemMatSymb);
            m_elemMatSymb = saveElemMatSymb;
            }

        m_ignores.Clear ();

        return;
        }

    QvElem*     qvElem = T_HOST.GetGraphicsAdmin()._LookupQvElemForSymbol (symbol);

    if (NULL == qvElem)
        {
        SymbolContext symbolContext(*this);
        qvElem = symbolContext.DrawSymbolForCache (symbol, *symbolCache);

        if (NULL == qvElem)
            return;

        T_HOST.GetGraphicsAdmin()._SaveQvElemForSymbol (symbol, qvElem); // save the qvelem in case we encounter this symbol again
        }

    // draw the symbol. Always pass true for "is3d" since the z for display priority is incorporated into the transform.
    DrawQvElem (qvElem, trans, clip, ignoreColor, ignoreWeight, true);
    }

/*---------------------------------------------------------------------------------**//**
* delete the cached representation of a symbol
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DeleteSymbol (IDisplaySymbol* symbol)
    {
    T_HOST.GetGraphicsAdmin()._DeleteSymbol (symbol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::RefreshCurrentDisplayStyle ()
    {
    m_currentDisplayStyle = GetIViewDraw().GetDrawDisplayStyle();         // Used to apply symbology overrides..
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ViewContext::InConditionalDraw () 
    {
    return !m_conditionalDrawStates.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::TestConditionalDraw (DisplayFilterHandlerId handlerId, ElementHandleCP element, void const* data, size_t dataSize)
    {
#ifdef WIP_DISPLAYFILTER
    DisplayFilterHandlerP   handler;

    if (NULL == (handler = DisplayFilterHandlerManager::GetManager().GetHandler (handlerId)))
        {
        BeAssert (false);
        return true;
        }

    return  handler->DoConditionalDraw (*this, element, data, dataSize);
#endif
return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_IfConditionalDraw (DisplayFilterHandlerId handlerId, ElementHandleCP element, void const* data, size_t dataSize)
    {
    bool        state  = TestConditionalDraw (handlerId, element, data, dataSize);

    if (!m_displayFilterKey.IsNull())
        m_displayFilterKey->PushState (handlerId, data, dataSize, state);

    return PushConditionalDrawState (state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_ElseIfConditionalDraw (DisplayFilterHandlerId handlerId, ElementHandleCP element, void const* data, size_t dataSize)
    {
    if (m_conditionalDrawStates.empty())
        {
        BeAssert (false);
        return true;
        }

    if (m_conditionalDrawStates.back().m_state)
        return false;

    PopConditionalDrawState ();
    return _IfConditionalDraw (handlerId, element, data, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_ElseConditionalDraw ()
    {
    if (m_conditionalDrawStates.empty())
        {
        BeAssert (false);
        return true;
        }

    return PushConditionalDrawState (!PopConditionalDrawState());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_EndConditionalDraw ()
    {
    if (m_conditionalDrawStates.empty())
        {
        BeAssert (false);
        return;
        }
    PopConditionalDrawState ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::PushConditionalDrawState (bool state)
    {
    m_conditionalDrawStates.push_back (ConditionalDrawState (state, GetIDrawGeom().GetMethodIndex(), m_conditionalBlockIndex));
    GetIDrawGeom().PushMethodState ();
    m_conditionalBlockIndex = 0;

    return state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::PopConditionalDrawState ()
    {
    if (m_conditionalDrawStates.empty())
        {
        BeAssert (false);
        return false;
        }

    ConditionalDrawState    state = m_conditionalDrawStates.back();

    GetIDrawGeom().PopMethodState();
    m_conditionalBlockIndex = state.m_conditionalBlockIndex + 1;
    m_conditionalDrawStates.pop_back();

    return state.m_state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CompoundDrawStatePtr ViewContext::GetCompoundDrawState ()
    {
    size_t      drawGeomMethodIndex;

    if (0 == (drawGeomMethodIndex = GetIDrawGeom().GetMethodIndex()) && m_conditionalDrawStates.empty())
        return CompoundDrawStatePtr();

    bvector<size_t>      conditionalBlockIndices;

    for (ConditionalDrawState  state: m_conditionalDrawStates)
        conditionalBlockIndices.push_back (state.m_conditionalBlockIndex);

    return  CompoundDrawState::Create (drawGeomMethodIndex, conditionalBlockIndices);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawAligned (DVec3dCR axis, DPoint3dCR origin, AlignmentMode alignmentMode, IStrokeAligned& stroker)
    {
    DPoint4d            zColumn4d;
    DVec3d              zColumn;
    DMatrix4d           toNpc = GetFrustumToNpc().M0;
    Transform           localToFrustumTransform;

    if (SUCCESS == GetCurrLocalToFrustumTrans (localToFrustumTransform))
        {
        DMatrix4d       localToFrustum = DMatrix4d::From (localToFrustumTransform);

        toNpc.productOf (&toNpc, &localToFrustum);
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
            BeAssert (false);
            return;
        }
    axisLocal.Normalize();

    toNpc.GetRow (zColumn4d, 2);
    zColumn.XyzOf (zColumn4d);
    zColumn.Normalize ();

    RotMatrix       rMatrix;

    if (zColumn.IsParallelTo (axisLocal))
        {
        rMatrix = RotMatrix::From1Vector (zColumn, 2, true);
        }
    else
        {
        DVec3d          xColumn, yColumn;

        yColumn.CrossProduct (zColumn, axisLocal);
        xColumn.CrossProduct (yColumn, zColumn);
        xColumn.Normalize ();
        yColumn.Normalize ();
        rMatrix = RotMatrix::FromColumnVectors (xColumn, yColumn, zColumn);
        }

    Transform       alignmentTransform = Transform::From (rMatrix, origin);

    PushTransform (alignmentTransform);
    m_transformClipStack.SetViewIndependent ();
    stroker._StrokeAligned (*this);
    PopTransformClip ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetLocatePriority (int priority)
    {
    IPickGeomP      pickGeom;

    if (NULL != (pickGeom = GetIPickGeom()))
        pickGeom->GetGeomDetail().SetLocatePriority (static_cast<HitPriority> (priority));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_SetNonSnappable (bool unsnappable)
    {
    IPickGeomP      pickGeom;

    if (NULL != (pickGeom = GetIPickGeom()))
        pickGeom->GetGeomDetail().SetNonSnappable (unsnappable);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::SetLinestyleTangents (DPoint3dCP start, DPoint3dCP end)
    {
    m_startTangent = start;
    m_endTangent = end;
    m_elemMatSymb.GetLineStyleSymbR ().ClearContinuationData();
    m_ovrMatSymb.GetMatSymbR ().GetLineStyleSymbR ().ClearContinuationData();
    }

//  On tablets some of the raster information on the GPU is reset whenever QV is reset.
//  On tablets, QV is reset when the app is put into the background and when the device is
//  rotated.  That causes QV to reset some of the raster data that is kept in the GPU.
//  To compensate the RasterHandler needs to know if QV has been reset.
static UInt32 s_numQvInitCalls;
UInt32 ViewContext::GetCountQvInitCalls() {return s_numQvInitCalls;}
void ViewContext::IncrementCountQvInitCalls () { ++s_numQvInitCalls; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
DgnProjectCP CachedDrawHandle::GetDgnProjectCP() const
    {
    if (NULL != m_symbolStamp)
        return &m_symbolStamp->GetDgnProject();

    return m_elementHandle->GetDgnProject();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
CachedDrawHandle::CachedDrawHandle(ElementHandleCP elementHandle) : m_elementHandle(elementHandle), m_symbolStamp(NULL)
    {
    if (NULL != elementHandle)
        {
        m_is3d = DisplayHandler::Is3dElem (elementHandle->GetElementCP());
        m_isPersistent = elementHandle->IsPersistent();
        return;
        }

    m_isGraphics = true;
    m_isPersistent = false;
    m_is3d = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
CachedDrawHandle::CachedDrawHandle(XGraphicsSymbolStampCR symbolStamp) : m_elementHandle(NULL), m_symbolStamp(&symbolStamp)
    {
    m_is3d = symbolStamp.GetIs3d();
    m_isPersistent = true;  //  maybe false.  Does this really mean "is persistent" or does it mean "is persistent element"?
    m_isGraphics = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/13
//---------------------------------------------------------------------------------------
CachedGraphics::CachedGraphics (bool is3d) : m_cachedElem(NULL), m_is3d(is3d)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/13
//---------------------------------------------------------------------------------------
CachedGraphics::~CachedGraphics() // added in graphite
    {
    T_HOST.GetGraphicsAdmin()._DeleteQvElem (m_cachedElem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/13
//---------------------------------------------------------------------------------------
CachedGraphicsPtr CachedGraphics::Create (CachedGraphicsCreatorR creator) // added in graphite
    {
    CachedGraphicsP result = new CachedGraphics (creator._GetIs3d());

    creator._GetICachedDraw().BeginCacheElement (result->m_is3d, ViewContext::GetQVCache (creator._GetDgnProject()), NULL);

    creator._DrawGraphicsForCache (creator._GetICachedDraw());

    result->m_cachedElem = creator._GetICachedDraw().EndCacheElement();

    return NULL == result->m_cachedElem ? NULL : result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     10/13
//---------------------------------------------------------------------------------------
void CachedGraphics::Draw (IViewDrawR output, TransformCP trans, UInt32 displayPriority) // added in graphite
    {
    BeAssert (NULL != m_cachedElem); // API should prevent creation with NULL m_cachedElem

#ifdef WIP_VANCOUVER_MERGE // CachedGraphics
    output._PushTransClip (trans, NULL);
#endif
    BeAssert(false);

    if (m_is3d)
        output.DrawQvElem3d (m_cachedElem, 0);
    else
        output.DrawQvElem2d (m_cachedElem, 0, displayPriority);

#ifdef WIP_VANCOUVER_MERGE // CachedGraphics
    output._PopTransClip();
#endif
    }
