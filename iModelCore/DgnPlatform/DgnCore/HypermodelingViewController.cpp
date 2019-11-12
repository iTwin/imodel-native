/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Geom/eigensys3d.fdf>

#if defined(NEEDS_WORK_CONTINUOUS_RENDER)
    static int s_flatten = 2; // 0 = don't flatten, 1 = flatten and draw on natural plane, 2 = flatten and draw on foremost section plane
#endif

#if defined (NEEDS_WORK_ELEMENTS_API)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
double static getSizeofPixelInDrawing (ViewContextR context, DrawingViewControllerCR drawing)
    {
    double onePixelInSpatialView = context.GetPixelSizeAtPoint (NULL);  
    DPoint3d vec = DPoint3d::From (onePixelInSpatialView, 0, 0);      

#ifdef NOT_RIGHT
    // TRICKY: The view is currently set up based on the physical view controller and so the view<->world transform is based on that. 
    //          What I want it is a delta in the DRAWING's local coordinate system. In the crazy setup we now have, the drawing's 
    //          local coordinate system may contain a scaling transform, so I have to convert.

    Transform toLCS;
    toLCS.InverseOf (drawing.GetTransformToWorld());
    RotMatrix scaling;
    toLCS.GetMatrix (scaling);
    scaling.Multiply (vec);                                          
#endif

    return vec.Magnitude();                          
    }
#endif

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
Transform static getTransformToForemostCutPlane (SectioningViewControllerCR section, SectionDrawingViewControllerCR drawing)
    {
#if defined (NEEDS_WORK_ELEMENTS_API)
    //  Get the foremost section cut plane as a DPlane3d
    DPlane3d cutPlane = section.GetForemostCutPlane();

    //  Get the drawing's origin (in world coordinates)
    auto drawingToWorld = drawing.GetTransformToWorld();
    DPoint3d drawingOriginWorld = DPoint3d::FromZero();
    drawingToWorld.Multiply (drawingOriginWorld);

    //  Get the distance FROM the drawing's origin TO the foremost cut plane
    double distanceToCutPlane = cutPlane.Evaluate (drawingOriginWorld);

    double tol = BeNumerical::ComputeComparisonTolerance (drawingOriginWorld.x, drawingOriginWorld.y);
    tol = std::max (tol, BeNumerical::ComputeComparisonTolerance (drawingOriginWorld.x, drawingOriginWorld.z));
    if (fabs (distanceToCutPlane) <= tol)
        return Transform::FromIdentity();

    //  Compose a transform that translates from drawing's origin to the foremost cut plane
    DPoint3d toCutPlane = DPoint3d::From (cutPlane.normal.x, cutPlane.normal.y, cutPlane.normal.z);
    toCutPlane.Scale (-distanceToCutPlane);
#endif

    Transform xlat = Transform::FromIdentity();
#if defined (NEEDS_WORK_ELEMENTS_API)
    xlat.SetTranslation (toCutPlane);
#endif

    return xlat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::PushClipsForSpatialView (ViewContextR context) const
    {
    for (auto drawing : m_drawings)
        context.PushClip (*drawing->GetProjectClipVector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::PopClipsForSpatialView (ViewContextR context) const
    {
    for (auto drawing : m_drawings)
        context.PopTransformClip ();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::PushClipsForInContextViewPass (ViewContextR context, SectionDrawingViewControllerCR drawing) const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    BeAssert (PASS_Hatch != m_pass && "The Hatch pass is drawing with just the toWorld transform -- do it yourself");

    //  3. Finally, translate section graphics to the foremost cut plane
    if (/*drawing.GetSectionHasDogLeg() &&*/ s_flatten > 1)  
        context.PushTransform (getTransformToForemostCutPlane (*drawing.GetSectioningViewController(), drawing));

#if defined (NEEDS_WORK_ELEMENTS_API)
    //  2. Next, transform section graphics from z-up drawing coordinates into 3D project space.
    context.PushTransform (drawing.GetTransformToWorld());
         
    //  1. First, flatten the section graphics onto a plane. (Yes, we store z-coordinates in the drawing model.)
    //          ... And optionally move them up or down, depending on the pass, to avoid bleed-through, since we cannot disable the z-buffer.
    if (s_flatten)
        {
        double onePixel = getSizeofPixelInDrawing (context, drawing);

        double pixelOffset = 0.0;
        if (m_pass & PASS_CutOrAnnotation)
            pixelOffset = +onePixel; // Cut and annotations draw in front of everything
        else if (m_pass & PASS_Forward)
            pixelOffset = -onePixel; // The "forward" graphics are a little behind the cuts and annotations, so as to avoid bleed-through.
        else if (m_pass & PASS_DrawingBackground)
            pixelOffset = -2*onePixel; // The drawing background must be behind everything else. Being filled, it would occlude everything else.

        context.PushTransform (drawing.GetFlatteningMatrix (pixelOffset));
        }
#endif
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::PopClipsForInContextViewPass (ViewContextR context, SectionDrawingViewControllerCR drawing) const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    BeAssert (PASS_Hatch != m_pass && "The Hatch pass is drawing with just the toWorld transform -- do it yourself");

    //  3. Finally, translate section graphics to the foremost cut plane
    if (/*drawing.GetSectionHasDogLeg() &&*/ s_flatten > 1)  
        context.PopTransformClip();

    //  2. Next, transform section graphics from z-up drawing coordinates into 3D project space.
    context.PopTransformClip();
         
    //  1. First, smash the section graphics onto a plane. (Yes, we store z-coordinates in the drawing model.)
    if (s_flatten)
        context.PopTransformClip();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
struct RangeDgnModelAppData : DgnModel::AppData
    {
    DRange3d m_range;
    };

static RangeDgnModelAppData::Key s_RangeDgnModelAppDataKey;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
DRange3d HypermodelingViewController::GetDrawingRange (DrawingViewControllerR drawing) const
    {
    DgnModelP model = drawing.GetTargetModel();
    if (NULL == model)
        return DRange3d::NullRange();

    auto appData = static_cast<RangeDgnModelAppData*>(model->FindOrAddAppData(s_RangeDgnModelAppDataKey, [&]()
        {
        DRange3d range = DRange3d::NullRange();
        for (auto const& el : *model)
            range.Extend(el.second->ToGeometrySource()->CalculateRange3d());

        range.ScaleAboutCenter (range, 1.10);

        appData = new RangeDgnModelAppData;
        appData->m_range = range;
        return appData;
        }).get());

    return appData->m_range;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
void HypermodelingViewController::DrawFakeSheetBorder (ViewContextR context, DrawingViewControllerR drawing) const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    ViewContext::ContextMark mark (&context);
    auto range3d = GetDrawingRange (drawing);
    auto range = DRange2d::From (DPoint2d::From(range3d.low.x,range3d.low.y), DPoint2d::From(range3d.high.x,range3d.high.y));
    DPoint2d box[4];
    range.Get4Corners (box);
    std::swap (box[2], box[3]);
    //SetOverrideGraphicParams (context);
    context.GetCurrentGraphicR().AddShape2d (_countof(box), box, /*filled*/true, 0, &range.low);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  03/14
//--------------+------------------------------------------------------------------------
void HypermodelingViewController::_DrawView (ViewContextR context) 
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_passesToDraw = (Pass)(PASS_Cut|PASS_Annotation|PASS_Hatch); // *** TEST TEST TEST

    //  Draw the embedded drawings
    for (auto drawingPtr : m_drawings)
        {
        if (drawingPtr == NULL)
            {
            BeAssert(false);
            continue;
            }

        m_currentViewController = drawingPtr.get();
        
        auto& drawing = *drawingPtr;

        if (ShouldDraw (PASS_Hatch) && drawing.GetSectionHasDogLeg()) // draw cuts only in their ORIGINAL PLANES to cover holes in the clipped 3d graphics
            {
#if defined (NEEDS_WORK_ELEMENTS_API)
            context.PushTransform (drawing.GetTransformToWorld());
#endif
            ViewContext::ContextMark mark (&context);
            m_pass = PASS_Hatch;
            SetOverrideGraphicParams (context);
            drawing.DrawView (context);
            context.PopTransformClip();
            if (context.WasAborted())
                break;
            }

        if (ShouldDraw (PASS_DrawingBackground))
            {
            m_pass = PASS_DrawingBackground;
            SetOverrideGraphicParams (context);
            PushClipsForInContextViewPass (context, drawing);
            DrawFakeSheetBorder (context, drawing);
            PopClipsForInContextViewPass (context, drawing);
            if (context.WasAborted())
                break;
            }

        if (ShouldDraw (PASS_Forward))
            {
            m_pass = PASS_Forward;
            ViewContext::ContextMark mark (&context);
            SetOverrideGraphicParams (context);
            PushClipsForInContextViewPass (context, drawing); 
            drawing.DrawView (context);
            PopClipsForInContextViewPass (context, drawing);

            if (context.WasAborted())
                break;
            }

        if (ShouldDraw (PASS_Cut) || ShouldDraw (PASS_Annotation))
            {
            m_pass = (Pass)(PASS_CutOrAnnotation & m_passesToDraw);
            ViewContext::ContextMark mark (&context);
            SetOverrideGraphicParams (context);
            ViewFlags flags = context.GetViewFlags();
            flags.hiddenEdges = flags.visibleEdges = true;
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
            context.SetViewFlags(flags);
#endif
            PushClipsForInContextViewPass (context, drawing); 
            drawing.DrawView (context);
            PopClipsForInContextViewPass (context, drawing);

            if (context.WasAborted())
                break;
            }

        if (context.WasAborted())
            break;
        }

    m_pass = PASS_None;
    SetOverrideGraphicParams (context);
    m_currentViewController = m_physical.get();

    //  Draw the clipped physical view
    m_currentViewController = m_physical.get();
    PushClipsForSpatialView (context);
    m_currentViewController->DrawView (context);
    PopClipsForSpatialView (context);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr HypermodelingViewController::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (m_pass != PASS_None && !ShouldDrawAnnotations() && !ProxyDisplayHandlerUtils::IsProxyDisplayHandler (elIter.GetHandler()))
/*<==*/ return nullptr;
#endif

    return T_Super::_StrokeGeometry(context, source, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
HypermodelingViewController::HypermodelingViewController (SpatialViewDefinition& def, SpatialViewControllerR p, bvector<SectionDrawingViewControllerPtr> const& d)
    :
    SpatialViewController (def),
    m_drawings (d),
    m_physical (&p),
    m_currentViewController (&p),
    m_pass (PASS_None),
    m_passesToDraw (PASS_All)
    {
    m_symbology.hatchColor = ColorDef::LightGrey();
    m_symbology.drawingBackgroundColor = ColorDef(0xcfff0808); // transparent blue
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SectionDrawingViewControllerPtr> HypermodelingViewController::GetSectionDrawingViews() const {return m_drawings;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
SectionDrawingViewControllerPtr HypermodelingViewController::FindSectionDrawingViewById (DgnViewId id) const 
    {
    for (auto c : m_drawings)
        {
        if (c->GetViewId() == id)
            return c;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HypermodelingViewController::AddDrawing (SectionDrawingViewControllerR d)
    {
    if (FindSectionDrawingViewById (d.GetViewId()) != NULL)
        return BSIERROR;
    m_drawings.push_back (&d);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HypermodelingViewController::RemoveDrawing (DgnViewId id)
    {
    for (auto i = m_drawings.begin(); i != m_drawings.end(); ++i)
        {
        if ((*i)->GetViewId() == id)
            {
            m_drawings.erase (i);
            return BSISUCCESS;
            }
        }
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialViewControllerR HypermodelingViewController::GetSpatialView() const {return *m_physical;}

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr HypermodelingViewController::_GetClipVector() const
    {
    return m_physical->GetClipVector();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef HypermodelingViewController::_GetBackgroundColor () const
    {
    return m_currentViewController->ResolveBGColor(); // TRICKY Must call ResolveBGColor, not GetBackgroundColor. 
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d HypermodelingViewController::_GetOrigin () const
    {
    return m_currentViewController->GetOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d HypermodelingViewController::_GetDelta () const
    {
    return m_currentViewController->GetDelta();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix HypermodelingViewController::_GetRotation() const
    {
    return m_currentViewController->GetRotation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::_SetOrigin (DPoint3dCR org)
    {
    T_Super::_SetOrigin (org);
    m_currentViewController->SetOrigin (org);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::_SetDelta (DVec3dCR delta)
    {
    T_Super::_SetDelta (delta);
    m_currentViewController->SetDelta (delta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::_AdjustAspectRatio (double aspect, bool expandView)
    {
    T_Super::_AdjustAspectRatio (aspect, expandView);
    m_currentViewController->AdjustAspectRatio (aspect, expandView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d HypermodelingViewController::_GetTargetPoint () const
    {
    return m_currentViewController->GetTargetPoint ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool HypermodelingViewController::_Allow3dManipulations () const {return m_currentViewController->Allow3dManipulations();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::_SetRotation (RotMatrixCR rot)
    {
    T_Super::_SetRotation (rot);
    m_currentViewController->SetRotation (rot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricModelP HypermodelingViewController::_GetTargetModel() const
    {
    return m_currentViewController->GetTargetModel();    // *** NEEDS WORK: Allow app to switch focus
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HypermodelingViewController::SetOverrideGraphicParams (ViewContextR context) const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (m_pass == PASS_Hatch)
        {
        OvrGraphicParamsP overrideMatSymb = context.GetOverrideGraphicParams();
        overrideMatSymb->SetLineColor (m_symbology.hatchColor);
        overrideMatSymb->SetFillColor (m_symbology.hatchColor);
        overrideMatSymb->SetFillTransparency (m_symbology.hatchColor.GetAlpha());
        overrideMatSymb->SetWidth (0);
        //auto pattern = PatternParams::Create ();
        //pattern->SetColor (0xffffff);
        //pattern->SetWeight (5);
        //pattern->SetStyle (1);
        //pattern->SetPrimarySpacing (GetDrawingRange(*m_drawings.front()).XLength()/100);
        //pattern->SetPrimaryAngle (msGeomConst_piOver4);
        //overrideMatSymb->SetPatternParams (pattern);
        context.GetCurrentGraphicR ().ActivateOverrideGraphicParams (overrideMatSymb);
        }
    else if (m_pass == PASS_DrawingBackground)
        {
        OvrGraphicParamsP overrideMatSymb = context.GetOverrideGraphicParams();
        overrideMatSymb->SetLineColor (m_symbology.drawingBackgroundColor);
        overrideMatSymb->SetFillColor (m_symbology.drawingBackgroundColor);
        overrideMatSymb->SetFlags (overrideMatSymb->GetFlags() | MATSYMB_OVERRIDE_FillColorTransparency);
        overrideMatSymb->SetWidth (0);
        }
        context.GetCurrentGraphicR ().ActivateOverrideGraphicParams (overrideMatSymb);
    else
        {
        OvrGraphicParamsP overrideMatSymb = context.GetOverrideGraphicParams();
        overrideMatSymb->SetFlags (MATSYMB_OVERRIDE_None);
        context.GetCurrentGraphicR ().ActivateOverrideGraphicParams (overrideMatSymb);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HypermodelingViewController::ShouldDrawProxyGraphics (ClipVolumePass proxyGraphicsType, int planeIndex) const
    {
    if (m_pass == PASS_Hatch) // if we are doing the Hatch proxyGraphicsType (all by itself)
        {                   // then display only cuts
        return (proxyGraphicsType == ClipVolumePass::Cut) && (planeIndex != m_nearestCutPlane);
        }

    if (0 != (m_pass & PASS_Cut))
        {
        return (proxyGraphicsType == ClipVolumePass::Cut);
        }

    if (0 != (m_pass & PASS_Forward))
        {
        return (proxyGraphicsType == ClipVolumePass::InsideForward);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HypermodelingViewController::ShouldDrawAnnotations () const
    {
    return 0 != (m_pass & PASS_Annotation);
    }
