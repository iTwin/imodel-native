/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DisplayHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

enum
    {
    MAX_INFO_STRING_LEN     = 4096,
    };

struct XGraphicsContainerVector : bvector<XGraphicsContainer> {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_Draw (ElementHandleCR el, ViewContextR context)
    {
#if !defined (NOT_NOW_XGRAPHICS)
    XGraphicsContainer::Draw (context, el);
#else
    ElementHandle::XAttributeIter iterator (el, XAttributeHandlerId (XATTRIBUTEID_XGraphics, 10), 0);

    if (!iterator.IsValid ())
        return;

    ElementGraphics::Collection ((const UInt8 *) iterator.PeekData (), iterator.GetSize ()).Draw (context);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DisplayHandler::_OnTransformFinish (EditElementHandleR elemHandle, TransformInfoCR trans)
    {
    //  Apply base class logic
    if (SUCCESS != T_Super::_OnTransformFinish (elemHandle, trans))
        return ERROR;

    // Transform known graphics-specific linkages
    ElementUtil::ApplyTransformToLinkages (elemHandle, trans);

#if defined (NEEDS_WORK_DGNITEM)
    Transform           basisTransform;
    if (SUCCESS == BasisXAttributesUtil::GetTransform (&basisTransform, elemHandle))
        {
        basisTransform = Transform::FromProduct (*trans.GetTransform(), basisTransform);
        BasisXAttributesUtil::SetTransform (basisTransform, elemHandle);
        }
#endif

    // Update range. NOTE: calcRange also checks that element is within design range.
    return ValidateElementRange (elemHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DisplayHandler::_OnFenceStretchFinish (EditElementHandleR elemHandle, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options)
    {
    //  Apply base class logic
    StatusInt   status = Handler::_OnFenceStretchFinish (elemHandle, transform, fp, options);
    if (SUCCESS != status)
        return status;

#if defined (WHEN_WE_HAVE_KNOWN_LINKAGES_TO_STRETCH)
    // Fence stretch known linkages that are specific to graphic types
    DgnElementCP  el = elemHandle->GetElement ();
    BeAssert (el->hdr.IsGraphic());
    if (el->GetSizeWords() > el->GetAttributeOffset())
        {
        }
#endif

    // Update range. NOTE: ValidateElementRange also checks that element is within design range.
    return ValidateElementRange (elemHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
static int processAttributesDimensionChange (LinkageHeader* outLinkageP, void* argP, LinkageHeader* inLinkageP, DgnElementP elmP)
    {
    if (!inLinkageP->user)
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    switch (inLinkageP->primaryID)
        {
#if defined (NEEDS_WORK_DGNITEM)
        case PATTERN_ID:
            {
            PatternParams   hParams;
            DwgHatchDefLine hatchLines[MAX_DWG_EXPANDEDHATCH_LINES]; // huge local!

            PatternLinkageUtil::Extract (hParams, hatchLines, MAX_DWG_EXPANDEDHATCH_LINES, (HatchLinkage *) inLinkageP, !elmP->Is3d());
            PatternLinkageUtil::Create (*((HatchLinkage *) outLinkageP), hParams, hatchLines, elmP->Is3d());

            return PROCESS_ATTRIB_STATUS_REPLACE;
            }
#endif

        case STYLELINK_ID:
            {
            LineStyleParams      sParams;

            LineStyleLinkageUtil::ExtractRawLinkage (&sParams, (StyleLink *) inLinkageP, !elmP->Is3d());
            // Going from 2D to 3D, reinitilize rotation matrix in the case where it was bogus. TR #280435
            if (elmP->Is3d() && sParams.modifiers & STYLEMOD_RMATRIX && !bsiRotMatrix_isOrthogonal (&sParams.rMatrix))
                (sParams.rMatrix).InitIdentity ();
            int linkBytes = LineStyleLinkageUtil::CreateRawLinkage ((StyleLink *) outLinkageP, &sParams, elmP->Is3d());

            return linkBytes > 0 ? PROCESS_ATTRIB_STATUS_REPLACE : PROCESS_ATTRIB_STATUS_DELETE;
            }
        }

    return PROCESS_ATTRIB_STATUS_NOCHANGE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void postConvertAttributeDimension (EditElementHandleR eeh)
    {
    DgnElementCP elmCP = eeh.GetElementCP ();

    if (elmCP->GetSizeWords() <= elmCP->GetAttributeOffset())
        return;

    DgnV8ElementBlank   elm;

    elmCP->CopyTo (elm);
    mdlElement_processLinkages (processAttributesDimensionChange, NULL, &elm);

    eeh.ReplaceElement (&elm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    T_Super::_OnConvertTo3d (eeh, elevation);

    DgnElementP  elm = eeh.GetElementP ();

    elm->SetIs3d(true);
    elm->GetRangeR().low.x = elevation;
    elm->GetRangeR().high.z = elevation;

    postConvertAttributeDimension (eeh);

    ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);

    DgnElementP  elm = eeh.GetElementP ();

    elm->SetIs3d(false);
    elm->GetRangeR().low.z = elm->GetRangeR().high.z = 0.0;

    postConvertAttributeDimension (eeh);

    ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_DrawFiltered (ElementHandleCR thisElm, ViewContextR context, DPoint3dCP pts, double size)
    {
    if (FILTER_LOD_ShowNothing == context.GetFilterLODFlag ())
        return;

    // Setup symbology to use for filtered draw...
    ElemDisplayParamsP  currDisplayParams = context.GetCurrentDisplayParams ();

    thisElm.GetDisplayHandler ()->GetElemDisplayParams (thisElm, *currDisplayParams, context.GetWantMaterials ());

    // Don't cook/activate gradient as it won't be used. Display gradient w/o outline as opaque fill using first color...
    if (NULL != currDisplayParams->GetGradient ())
        {
        double      value;
        IntColorDef color;

        currDisplayParams->GetGradient ()->GetKey (color.m_rgb, value, 0);

        if (0 == static_cast<int>((currDisplayParams->GetGradient ()->GetFlags ()) & static_cast<int> (GradientFlags::Outline)))
            currDisplayParams->SetLineColorTBGR (color.m_int);

        currDisplayParams->SetFillColorTBGR (color.m_int); // Won't be used for display...but best to keep things in a valid state...
        currDisplayParams->SetGradient (NULL);
        }

    context.CookDisplayParams ();
    context.CookDisplayParamsOverrides ();

    UInt32  info = context.GetDisplayInfo (currDisplayParams->IsRenderable ());

    if (DISPLAY_INFO_None == info)
        return;

    IDrawGeomR   output = context.GetIDrawGeom ();
    DgnElementCP el = thisElm.GetElementCP ();

    // Ignore line codes and activate overrides for hilite, etc.
    context.GetOverrideMatSymb ()->SetRasterPattern (0xffffffff);
    context.ActivateOverrideMatSymb ();

    if (DisplayHandler::Is3dElem (el))
        {
        DPoint3d    pts3d[4];

        if (0 == (info & DISPLAY_INFO_Edge))
            {
            pts3d[0] = pts3d[3] = pts[0];

            for (int iPoly = 0; iPoly < 3; iPoly++)
                {
                pts3d[1] = (0 == iPoly ? pts[5] : pts[3]);
                pts3d[2] = (2 == iPoly ? pts[5] : pts[6]);

                output.DrawTriStrip3d (3, pts3d, 0, NULL);
                }
            }
        else
            {
            pts3d[0] = pts[0];
            pts3d[1] = pts[7];
            pts3d[2] = pts[5];
            pts3d[3] = pts[6];

            output.DrawLineString3d (4, pts3d, NULL);
            }

        return;
        }

    DPoint2d    pts2d[4];

    DataConvert::Points3dTo2d (pts2d, pts, 3);
    pts2d[3] = pts2d[0];

    if (0 == (info & DISPLAY_INFO_Edge))
        output.DrawTriStrip2d (3, pts2d, 0, context.GetDisplayPriority(), NULL);
    else
        output.DrawLineString2d (4, pts2d, context.GetDisplayPriority(), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Test an element to see if the "level of detail" threshold makes it is too small to be useful
* in this view. If the element is too small, just draw a dot, otherwise, call the "Draw" method.
* @return true if the element was filtered and drawn with minimum LOD.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayHandler::_FilterLevelOfDetail (ElementHandleCR thisElm, ViewContextR context)
    {
#ifdef NO_LOD
    return false;
#endif

    DRange2d    viewRange;
    DPoint3d    localPts[8];
    DgnElementCP elmCP = thisElm.GetElementCP ();

    if (elmCP->Is3d())
        {
        DPoint3d    viewPts[8];
        thisElm.GetIndexRange()->get8Corners (localPts);
        context.LocalToView (viewPts, localPts, 8);
        viewRange.initFrom (viewPts, 8);
        }
    else
        {
        DRange2d    range;
        DPoint3d    viewPts[4];
        DPoint2d    localPts2d[4];

        GetDPRange ((DPoint2dP) &range, thisElm.GetIndexRange());
        range.get4Corners (localPts2d);
        DataConvert::Points2dTo3d (localPts, localPts2d, 4, 0.0);
        context.LocalToView (viewPts, localPts, 4);

        viewRange.initFrom (viewPts, 4);
        }

    double      minLOD = context.GetMinLOD ();
    double      size = viewRange.extentSquared ();

    if (size >= minLOD)
        return false;

    _DrawFiltered (thisElm, context, localPts, size);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    DgnElementCP el = thisElm.GetElementCP ();

    params.Init ();
    params.SetSubLevelId (SubLevelId(el->GetLevel()));
    params.SetElementClass (el->GetElementClass());
    params.SetIsRenderable (IsRenderable (thisElm));

    params.SetLineColor (el->GetSymbology().color);
    params.SetLineStyle (el->GetSymbology().style);
    params.SetWeight (el->GetSymbology().weight);

    // Set element priority and compute net priority using model and level (2d only)...
    params.SetElementDisplayPriority (el->GetDisplayPriority(), thisElm.GetDgnModelP()->Is3d(), /*facetAppearance*/NULL);

#if defined (NEEDS_WORK_MATERIAL)
    if (wantMaterials && params.IsRenderable ())
        {
        MaterialUVDetailPtr detail = MaterialUVDetail::Create ();

        detail->SetElementHandle (thisElm);
        params.SetMaterialUVDetail (detail.get ());
        }
#endif

    if (el->GetSizeWords() <= el->GetAttributeOffset())
        return;

    #ifdef WIP_VANCOUVER_MERGE // material
    if (wantMaterials)
        {
        MaterialCP          material = MaterialManager::GetManagerR ().FindMaterialAttachment (NULL, thisElm, *thisElm.GetDgnModelP (), false);

        params.SetMaterial (material, true);
        }
    #endif

    LineStyleParams  styleParams;

    if (SUCCESS == LineStyleLinkageUtil::GetParamsFromElement (&styleParams, el))
        params.SetLineStyle (params.GetLineStyle (), &styleParams);

    Display_attribute  dispAttr;

    if (mdlElement_displayAttributePresent (el, TRANSPARENCY_ATTRIBUTE, &dispAttr))
        params.SetTransparency (dispAttr.attr_data.transparency.transparency);

    if (params.IsRenderable ())
        {
        if (mdlElement_displayAttributePresent (el, GRADIENT_ATTRIBUTE, &dispAttr))
            {
            GradientSymbPtr  gradient = GradientSymb::Create();

            gradient->FromDisplayAttribute (&dispAttr.attr_data.gradient);
            params.SetFillDisplay ((dispAttr.attr_data.gradient.flags & static_cast<int>(GradientFlags::AlwaysFilled)) ? FillDisplay::Always : FillDisplay::ByView);
            params.SetGradient (gradient.get());
            }
        else if (mdlElement_displayAttributePresent (el, FILL_ATTRIBUTE, &dispAttr))
            {
            params.SetFillDisplay (dispAttr.attr_data.fill.alwaysFilled ? FillDisplay::Always : FillDisplay::ByView);
            params.SetFillColor (dispAttr.attr_data.fill.color);
            }
        }

    bool    isCapped, alwaysUseDirection;
    double  thickness;
    DVec3d  defaultDirection;

    if (SUCCESS == ElementUtil::ExtractThickness (thickness, defaultDirection, isCapped, alwaysUseDirection, thisElm))
        {
        DVec3d  thicknessVector = defaultDirection;

        if (alwaysUseDirection || IsPlanar (thisElm, &thicknessVector, NULL, &defaultDirection))
            {
            if (thicknessVector.DotProduct (defaultDirection) < 0.0)
                thickness = -thickness;

            thicknessVector.Scale (thicknessVector, thickness);
            }

        if (NULL != params.GetLineStyleParams ())
            {
            // NOTE: These are always treated as capped in DWG...ignore capped flag from linkage...
            if ((params.GetLineStyleParams ()->modifiers & STYLEMOD_SWIDTH && 0.0 != params.GetLineStyleParams ()->startWidth) ||
                (params.GetLineStyleParams ()->modifiers & STYLEMOD_EWIDTH && 0.0 != params.GetLineStyleParams ()->endWidth))
                isCapped = true;
            }

        params.SetThickness (&thicknessVector, isCapped);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayHandler::_IsPlanar (ElementHandleCR thisElm, DVec3dP normalP, DPoint3dP pointP, DVec3dCP inputDefaultNormalP)
    {
    if (!Is3dElem (thisElm.GetElementCP ()))
        {
        if (normalP)
            normalP->Init (0.0, 0.0, 1.0);

        if (pointP)
            _GetTransformOrigin (thisElm, *pointP);

        return true;
        }

    CurveVectorPtr  curves = ICurvePathQuery::ElementToCurveVector (thisElm);
    Transform       localToWorld, worldToLocal;
    DRange3d        localRange;

    if (!curves.IsValid () || !curves->IsPlanarWithDefaultNormal (localToWorld, worldToLocal, localRange, inputDefaultNormalP))
        return false; 

    if (normalP)
        {
        normalP->Init (0.0, 0.0, 1.0);
        localToWorld.MultiplyMatrixOnly (*normalP);
        normalP->Normalize ();
        }

    if (pointP)
        {
        pointP->Zero ();
        localToWorld.Multiply (*pointP);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus DisplayHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayHandler::_IsVisible (ElementHandleCR elHandle, ViewContextR context, bool testRange, bool testLevel, bool testClass)
    {
    if (testRange && context.FilterRangeIntersection (elHandle))
        return false;

    ScanCriteriaCP scanCrit = context.GetScanCriteria();

    if (NULL == scanCrit)
        return true;

    UInt32  classMask = scanCrit->GetClassMask ();

    if (ScanTestResult::Pass != _DoScannerTests (elHandle, testLevel ? scanCrit->GetLevelBitMask() : NULL, testClass ? &classMask : NULL, &context))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult DisplayHandler::_DoScannerTests (ElementHandleCR eh, BitMaskCP levels, UInt32 const* classMask, ViewContextP context)
    {
    if ((NULL != classMask) && (!(*classMask & (1 << eh.GetElementCP()->GetElementClassValue()))))
        return ScanTestResult::Fail;

    return  T_Super::_DoScannerTests (eh, levels, classMask, context);
    }

// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                    Sam.Wilson                      10/2007
// +---------------+---------------+---------------+---------------+---------------+------*/
// static bool liesInPlane (DisplayHandler& handler, ElementHandleCR thisElm, ICutPlaneR cutPlane, ViewContextR context)
//     {
//     DVec3d      normal;
//     DPoint3d    point;
// 
//     if (!handler.IsPlanar (thisElm, &normal, &point, NULL))
//         return false;
// 
//     DPlane3d    plane;
// 
//     cutPlane._GetPlane (plane);
// 
//     return fabs (plane.evaluate (&point)) <= mgds_fc_epsilon && bsiDPoint3d_areParallel (&plane.normal, &normal);
//     }
// static void InitCutXGraphicsFacetOptions (IFacetOptionsR options)
//     {
//     options.SetMaxPerFace (MAX_VERTICES);
//     options.SetChordTolerance (1.0);
//     options.SetAngleTolerance (0.0);
//     options.SetNormalsRequired (false);
//     options.SetParamsRequired (false);
//     }
// 
// /*=================================================================================**//**
// * @bsiclass                                                     RayBentley      06/2008
// +===============+===============+===============+===============+===============+======*/
// struct          CutToXGraphicsDrawGeom : SimplifyViewDrawGeom
// {
// XGraphicsRecorder           m_xgRecorder;
// ICutPlaneR                  m_cutPlane;
// ElementHandleCR             m_theElem;
// IFacetOptionsPtr            m_facetOptions;
// 
// virtual bool                _ProcessAsBody (bool isCurved) const override {return true;}
// virtual IFacetOptions*      _GetFacetOptions () override {return m_facetOptions.get ();}
// 
// XGraphicsContainer const&   GetXGraphicsContainer () {return m_xgRecorder.GetContainer();}
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     06/2008
// +---------------+---------------+---------------+---------------+---------------+------*/
// CutToXGraphicsDrawGeom (ElementHandleCR thisElm, ICutPlaneR cutPlane, ViewContextR context) :
//     m_xgRecorder (thisElm.GetDgnModelP()),
//     m_cutPlane (cutPlane),
//     m_theElem (thisElm)
//     {
//     m_xgRecorder.GetContext()->CookElemDisplayParams (thisElm);
//     m_xgRecorder.SetUseCache (true);
//     m_facetOptions = IFacetOptions::New ();
//     InitCutXGraphicsFacetOptions (*m_facetOptions);
//     m_viewFlags.SetRenderMode (MSRenderMode::SmoothShade);
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Brien.Bastings  04/12
// +---------------+---------------+---------------+---------------+---------------+------*/
// virtual StatusInt _ProcessCurveVector (CurveVectorCR curves, bool filled) override
//     {
//     GPArraySmartP  gpa;
// 
//     gpa->Add (curves); // NOTE: Would be nice to eliminate this conversion but SS3 has a dependency on the gpa point count...
//     gpa->Transform (m_context->GetCurrLocalToFrustumTransformCP ()); // TR# 313860 - CurrTrans not applied in shared cells for closed planars.
// 
//     UInt64  edgeId[3];
// 
//     edgeId[0] = m_theElem.GetElementCP ()->ehdr.uniqueId; // If this is a cell component, we must distinguish it from other components.
//     edgeId[1] = gpa->GetCount (); // To make it fragile, we make the association depend on the original shape's topology
// 
//     IDrawGeomR      drawGeom = m_xgRecorder.GetContext ()->GetIDrawGeom ();
//     DPlane3d        plane;
//     GPArraySmartP   hatchGPA;
// 
//     m_cutPlane._GetPlane (plane);
//     hatchGPA->AddPlaneIntersectionEdges (*gpa, plane);
// 
//     if (curves.IsAnyRegionType ())
//         {
//         DSegment3d  segment;
// 
//         for (size_t nextReadIndex, i = 0; hatchGPA->GetDSegment3d (i, nextReadIndex, segment); i = nextReadIndex)
//             {
//             CurveVectorPtr      curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
//             ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (segment);
//             
//             edgeId[2] = (int) i;
// 
//             CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create (CurvePrimitiveId::Type_CutGeometry, &edgeId, sizeof (edgeId), m_context->GetCompoundDrawState().get());
//             primitive->SetId (newId.get());
//             curve->push_back (primitive);
//             drawGeom.DrawCurveVector (*curve, false);
//             }
//         }
//     else
//         {
//         DPoint3d  point;
// 
//         for (int i = 0; NULL != hatchGPA->GetPoint (point, i); i++)
//             {
//             CurveVectorPtr      curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
//             ICurvePrimitivePtr  primitive = ICurvePrimitive::CreatePointString (&point, 1);
// 
//             edgeId[2] = i;
// 
//             CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create (CurvePrimitiveId::Type_CutGeometry, &edgeId, sizeof (edgeId), m_context->GetCompoundDrawState().get());
//             primitive->SetId (newId.get());
//             curve->push_back (primitive);
//             drawGeom.DrawCurveVector (*curve, false);
//             }
//         }
// 
//     return SUCCESS;                                                                                                                                                                                                       
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     06/2008
// +---------------+---------------+---------------+---------------+---------------+------*/
// virtual StatusInt _ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP) override
//     {
//     ClipMask    clipMask;
//     DPlane3d    plane;
//     DRange2d    range;
//     RotMatrix   matrix;
// 
//     m_cutPlane._GetClipRange (clipMask, range, matrix);
//     m_cutPlane._GetPlane (plane);
//    
//     return T_HOST.GetSolidsKernelAdmin()._OutputBodyCut (entity, m_context->GetCurrLocalToFrustumTransformCP(), *m_xgRecorder.GetContext (), plane, range, matrix, clipMask, m_context->GetCompoundDrawState().get());
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     06/2008
// +---------------+---------------+---------------+---------------+---------------+------*/
// virtual StatusInt _ProcessFacetSet (PolyfaceQueryCR facets, bool filled) override
//     {
//     bool        isClosed = facets.IsClosedByEdgePairing ();
//     DPlane3d    plane;
// 
//     m_cutPlane._GetPlane (plane);
// 
//     CurveVectorPtr  slice = facets.PlaneSlice (plane, isClosed, true);
// 
//     if (!slice.IsValid ())
//         return ERROR;
// 
//     CurveTopologyId::AddPolyfaceCutIds (*slice, m_context->GetCompoundDrawState().get());
// 
//     m_xgRecorder.GetContext ()->GetIDrawGeom ().DrawCurveVector (*slice, isClosed);
// 
//     return SUCCESS;
//     }
// 
// }; // CutToXGraphicsDrawGeom
// 
// /*=================================================================================**//**
// * @bsiclass                                                     RayBentley      06/2008
// +===============+===============+===============+===============+===============+======*/
// struct          CutToXGraphicsContext : public NullContext
// {
// DEFINE_T_SUPER(NullContext)
// CutToXGraphicsDrawGeom*     m_cutToXGraphicsDrawGeom;
// TransformClipStackR         m_modelTransformClipStack;
// 
// QvElem*         _DrawCached (ElementHandleCR elHandle, IStrokeForCache& stroker, Int32) override { stroker._StrokeForCache (elHandle, *this); return NULL; }
// virtual void    _SetupOutputs () override {SetIViewDraw (*m_cutToXGraphicsDrawGeom);}
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     06/2008
// +---------------+---------------+---------------+---------------+---------------+------*/
// CutToXGraphicsContext (CutToXGraphicsDrawGeom* drawGeom, DgnModelP modelRef, TransformClipStackR rangeTest) : m_modelTransformClipStack (rangeTest)
//     {
//     m_cutToXGraphicsDrawGeom = drawGeom;
// 
//     SetBlockAsynchs (true);
//     _SetupOutputs ();
//     SetPathRoot (modelRef);
// 
//     m_purpose = DrawPurpose::CutXGraphicsCreate;
//     drawGeom->SetViewContext (this);
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     02/2011
// +---------------+---------------+---------------+---------------+---------------+------*/
// StatusInt _VisitElemHandle (ElementHandleCR eh, bool checkRange, bool checkScanCriteria) override
//     {
// 
// #ifdef NEEDS_WORK_CHECK_FOR_SHAREDCELL_DEFINITIONS
//     DgnElementCP el;
// 
//     // TR#311423 - If this is a complex component, test it against the modelTransformClipStack to reject on range
//     // Else, since we handle the cutCaching from the outermost element every child of a shared cell
//     // will be tested explicitly and expsensivel for cut.
//     if (checkRange && NULL != (el = eh.GetElementCP ()) && !m_modelTransformClipStack.IsEmpty() && el->IsComplexComponent())
//         {
//         if (ClipPlaneContainment_StronglyOutside == m_modelTransformClipStack.TestRange (NULL, el->hdr.dhdr.range, el->Is3d()))
//             return SUCCESS;
//         }
// #endif
// 
//     return T_Super::_VisitElemHandle (eh, checkRange, checkScanCriteria);
//     }
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     06/2008
// +---------------+---------------+---------------+---------------+---------------+------*/
// void _CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) override
//     {
//     m_cutToXGraphicsDrawGeom->m_xgRecorder.SetElemDisplayParams (&elParams);
//     }
// 
// }; // CutToXGraphicsContext
// 
// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Ray.Bentley     06/2008
// +---------------+---------------+---------------+---------------+---------------+------*/
// StatusInt DisplayHandler::_DrawCut (ElementHandleCR thisElm, ICutPlaneR cutPlane, ViewContextR context)
//     {
//     //  If element is planar and does lie in the cut plane, then draw it normally.
//     IDrawCutInfo*   drawCutInfo = context.GetIDrawCutInfo ();
// 
//     if (NULL != drawCutInfo)
//         drawCutInfo->_SetWasNotCut (true);
// 
//     if (liesInPlane (*this, thisElm, cutPlane, context))
//         {
//         ICutPlaneP      wasCutPlane = context.GetCuttingPlane ();
// 
//         context.SetCuttingPlane (NULL);
//         _Draw (thisElm, context);
//         context.SetCuttingPlane (wasCutPlane);
// 
//         return SUCCESS;
//         }
// 
// #ifdef IGNORE_WIRE_INTERSECTIONS
//     // In 8.11.7  we did not intersect non-renderable (wire) geometry with the cut plane.
//     // The Civil group requires this for their profile tools and it seems correct to provide the point intersections. (TR# 318074).
//     if (!IsRenderable (thisElm) && !IsSupportedOperation (&thisElm, SupportOperation::CacheCutGraphics))
//         return SUCCESS;
// #endif
// 
//     // if element is not persistent, there's nothing on which to hang the cutgraphics, so bail
//     if (!thisElm.GetElementRef())
//         return ERROR;
// 
//     if (NULL != drawCutInfo)
//         drawCutInfo->_SetWasNotCut (false);
// 
//     CutGraphicsContainerCP      cachedCutGraphics;
// 
//     if (NULL == (cachedCutGraphics = context.GetCutGraphicsCache (thisElm, context.GetCurrDisplayPath(), cutPlane)))
//         {
//         if (!cutPlane._OverlapsElement (thisElm.GetElementCP()))
//             return SUCCESS;         // Should we fail permanently?
// 
//         context.BeginAddToCutGraphicsCache ();
//         CutToXGraphicsDrawGeom cutToXGraphicsDrawGeom (thisElm, cutPlane, context);
// 
//         CutToXGraphicsContext (&cutToXGraphicsDrawGeom, context.GetCurrentModel(), context.GetTransformClipStack()).VisitElemHandle (thisElm, false, false);
// 
//         // Section could result in multiple solid regions, want one solid region per-container so split it up...
//         XGraphicsContainer const& xgContainer = cutToXGraphicsDrawGeom.GetXGraphicsContainer();
//         XGraphicsContainerVector  xgContainerVector;
// 
//         xgContainer.ExtractPrimitives (xgContainerVector);
// 
//         for (XGraphicsContainerVector::const_iterator region = xgContainerVector.begin(); region != xgContainerVector.end(); ++region)
//             context.AddToCutGraphicsCache (thisElm, const_cast<XGraphicsContainer&>(*region), cutPlane);
// 
//         if (NULL == (cachedCutGraphics = context.EndAddToCutGraphicsCache (thisElm, cutPlane)))
//             {
//             BeAssert (false);
//             return ERROR;
//             }
//         }
//         
//     context.DrawCutGraphicsCache (thisElm, context.GetCurrDisplayPath(), *cachedCutGraphics, cutPlane);
//     return SUCCESS;
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool doSegmentFlash (DisplayPathCR path)
    {
    if (path.GetPathType () < DisplayPathType::Snap)
        return true;

    switch (static_cast <SnapPathCR> (path).GetSnapMode ())
        {
        case SnapMode::Center:
        case SnapMode::Origin:
        case SnapMode::Bisector:
            return false; // Snap point for these is computed using entire linestring, not just the hit segment...

        default:
            return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DisplayHandler::_DrawPath (DisplayPathCR path, ViewContextR context)
    {
    if (DrawPurpose::Flash != context.GetDrawPurpose () || path.GetPathType () < DisplayPathType::Hit)
        return ERROR;

    HitPathCR hitPath = static_cast <HitPathCR> (path);

    if (!hitPath.GetComponentMode ())
        return ERROR;

    ICurvePrimitiveCP primitive = hitPath.GetGeomDetail ().GetCurvePrimitive ();

    if (NULL == primitive)
        return ERROR;

    ElementRefP   elemRef = hitPath.GetCursorElem ();
    ElementHandle eh (elemRef);

    if (!IsVisible (eh, context, false, true, true))
        return SUCCESS; // No point calling VisitElemRef...

    context.SetCurrentElement (elemRef);

    bool        pushedtrans = false;
    Transform   hitLocalToContextLocal;

    // NOTE: GeomDetail::LocalToWorld includes pushed transforms...
    if (SUCCESS == hitPath.GetHitLocalToContextLocal (hitLocalToContextLocal, context) && !hitLocalToContextLocal.IsIdentity ())
        {
        context.PushTransform (hitLocalToContextLocal);
        pushedtrans = true;
        }

    context.CookElemDisplayParams (eh);
    context.ActivateOverrideMatSymb ();

    DSegment3d      segment;
    CurveVectorPtr  curve;

    // Flash only the selected segment of linestrings/shapes based on snap mode...
    if (doSegmentFlash (hitPath) && hitPath.GetGeomDetail ().GetSegment (segment))
        curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine (segment));
    else
        curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open, primitive->Clone ());

    if (DisplayHandler::Is3dElem (eh.GetElementCP ()))
        context.GetIDrawGeom ().DrawCurveVector (*curve, false);
    else
        context.GetIDrawGeom ().DrawCurveVector2d (*curve, false, context.GetDisplayPriority ());

    if (pushedtrans)
        context.PopTransformClip ();

    context.SetCurrentElement (NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    DgnElementCP elmCP = eh.GetElementCP ();

    HitPathCP   path = context.GetQueryPath ();

    if (path && QueryPropertyPurpose::Match == context.GetIQueryPropertiesP ()->_GetQueryPropertiesPurpose ()) // Report properties for hit detail...
        {
        HatchLinkage*       hatchLinkP;
        GeomDetail const&   detail = path->GetGeomDetail ();

        if (HIT_DETAIL_Pattern & detail.GetDetailType () && NULL != (hatchLinkP = (HatchLinkage *) linkage_extractLinkageByIndex (NULL, elmCP, PATTERN_ID, NULL, detail.GetPatternIndex ())))
            {
#if defined (NEEDS_WORK_DGNITEM)
            PatternParams*  patParamsP = (PatternParams *) alloca (sizeof (PatternParams));

            // NOTE: In this case announce pattern symbology as the base id!
            patParamsP->Init();
            PatternLinkageUtil::Extract (*patParamsP, NULL, 0, hatchLinkP, elmCP->Is3d());

            if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                context.DoColorCallback (NULL, EachColorArg (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Color) ? patParamsP->color : elmCP->GetSymbology().color, PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                context.DoWeightCallback (NULL, EachWeightArg (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Weight) ? patParamsP->weight : elmCP->GetSymbology().weight, PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                context.DoLineStyleCallback (NULL, EachLineStyleArg (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Style) ? patParamsP->style : elmCP->GetSymbology().style, NULL, PROPSCALLBACK_FLAGS_IsBaseID, context));
#endif

            // Report info from element inherited by pattern...
            if (0 != (ELEMENT_PROPERTY_Level & context.GetElementPropertiesMask ()))
                context.DoLevelCallback (NULL, EachLevelArg (context.GetCurrentLevelID().IsValid() ? context.GetCurrentLevelID () : LevelId(elmCP->GetLevel()), PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_DisplayPriority & context.GetElementPropertiesMask ()) && !elmCP->Is3d())
                context.DoDisplayPriorityCallback (NULL, EachDisplayPriorityArg (elmCP->GetDisplayPriority(), PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_ElementClass & context.GetElementPropertiesMask ()))
                context.DoElementClassCallback (NULL, EachElementClassArg ((DgnElementClass) elmCP->GetElementClass(), PROPSCALLBACK_FLAGS_IsBaseID, context));

            if (0 != (ELEMENT_PROPERTY_Transparency & context.GetElementPropertiesMask ()))
                {
                Display_attribute   dispAttr;

                context.DoTransparencyCallback (NULL, EachTransparencyArg (mdlElement_displayAttributePresent (elmCP, TRANSPARENCY_ATTRIBUTE, &dispAttr) ? dispAttr.attr_data.transparency.transparency : 0, PROPSCALLBACK_FLAGS_IsBaseID, context));
                }

#ifdef WIP_VANCOUVER_MERGE // material
            if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
                {
                IMaterialPropertiesExtension    *mExtension = IMaterialPropertiesExtension::Cast (eh.GetHandler ());
                DgnMaterialId                    materialId;
                
                if (mExtension && SUCCESS ==  mExtension->StoresAttachmentInfo (eh, materialId))
                    context.DoMaterialCallback (NULL, EachMaterialArg (materialId, PROPSCALLBACK_FLAGS_IsBaseID, context));
                }
#endif

            return;
            }
        }

    if (0 != (ELEMENT_PROPERTY_Level & context.GetElementPropertiesMask ()))
        context.DoLevelCallback (NULL, EachLevelArg (elmCP->GetLevel(), PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_DisplayPriority & context.GetElementPropertiesMask ()) && !elmCP->Is3d())
        context.DoDisplayPriorityCallback (NULL, EachDisplayPriorityArg (elmCP->GetDisplayPriority(), PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_ElementClass & context.GetElementPropertiesMask ()))
        context.DoElementClassCallback (NULL, EachElementClassArg ((DgnElementClass) elmCP->GetElementClass(), PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
        {
        Display_attribute   attribute;

        if (mdlElement_displayAttributePresent (elmCP, FILL_ATTRIBUTE, &attribute))
            context.DoColorCallback (NULL, EachColorArg (attribute.attr_data.fill.color, PROPSCALLBACK_FLAGS_IsBackgroundID, context));

        context.DoColorCallback (NULL, EachColorArg (elmCP->GetSymbology().color, PROPSCALLBACK_FLAGS_IsBaseID, context));
        }

    if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
        context.DoWeightCallback (NULL, EachWeightArg (elmCP->GetSymbology().weight, PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
        {
        LineStyleParams lsParams;
        memset (&lsParams, 0, sizeof (lsParams));

        LineStyleLinkageUtil::ExtractParams (&lsParams, elmCP);
        context.DoLineStyleCallback (NULL, EachLineStyleArg (elmCP->GetSymbology().style, &lsParams, PROPSCALLBACK_FLAGS_IsBaseID, context));
        }

#if defined (NEEDS_WORK_DGNITEM)
    if (0 != ((ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Weight) & context.GetElementPropertiesMask ()) && mdlElement_attributePresent (elmCP, PATTERN_ID, NULL))
        {
        int             patIndex = 0;
        HatchLinkage*   hatchLinkP;
        PatternParams*  patParamsP = (PatternParams *) alloca (sizeof (PatternParams));

        while (NULL != (hatchLinkP = (HatchLinkage *) linkage_extractLinkageByIndex (NULL, elmCP, PATTERN_ID, NULL, patIndex)))
            {
            patParamsP->Init();
            PatternLinkageUtil::Extract (*patParamsP, NULL, 0, hatchLinkP, elmCP->Is3d());

            if (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Color) && 0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                context.DoColorCallback (NULL, EachColorArg (patParamsP->color, PROPSCALLBACK_FLAGS_IsDecorationID, context));

            if (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Weight) && 0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                context.DoWeightCallback (NULL, EachWeightArg (patParamsP->weight, PROPSCALLBACK_FLAGS_IsDecorationID, context));

            if (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Style) && 0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                context.DoLineStyleCallback (NULL, EachLineStyleArg (patParamsP->style, NULL, PROPSCALLBACK_FLAGS_IsDecorationID, context));

            patIndex++;
            }
        }
#endif

    if (0 != (ELEMENT_PROPERTY_Transparency & context.GetElementPropertiesMask ()))
        {
        Display_attribute   dispAttr;

        context.DoTransparencyCallback (NULL, EachTransparencyArg (mdlElement_displayAttributePresent (elmCP, TRANSPARENCY_ATTRIBUTE, &dispAttr) ? dispAttr.attr_data.transparency.transparency : 0, PROPSCALLBACK_FLAGS_IsBaseID, context));
        }

#ifdef WIP_VANCOUVER_MERGE // material
    if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
        {
        IMaterialPropertiesExtension    *mExtension = IMaterialPropertiesExtension::Cast (eh.GetHandler ());
        DgnMaterialId                    materialId;
        
        if (mExtension && SUCCESS == mExtension->StoresAttachmentInfo (eh, materialId))
            context.DoMaterialCallback (NULL, EachMaterialArg (materialId, PROPSCALLBACK_FLAGS_IsBaseID, context));
        }
#endif

#if defined (BEIJING_DGNPLATFORM_WIP_TEMPLATES)
    if (0 != (ELEMENT_PROPERTY_ElementTemplate & context.GetElementPropertiesMask ()))
        context.DoElementTemplateCallback (NULL, EachElementTemplateArg (TemplateRefAttributes::GetReferencedTemplateIDFromHandle (eh), PROPSCALLBACK_FLAGS_IsBaseID, context));
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    DgnElementP  elmP = eeh.GetElementP ();

    if (0 != (ELEMENT_PROPERTY_Level & context.GetElementPropertiesMask ()))
        context.DoLevelCallback ((LevelId*) &elmP->GetLevelR(), EachLevelArg (elmP->GetLevel(), PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_DisplayPriority & context.GetElementPropertiesMask ()) && !elmP->Is3d())
        context.DoDisplayPriorityCallback (&elmP->GetDisplayPriorityR(), EachDisplayPriorityArg (elmP->GetDisplayPriority(), PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_ElementClass & context.GetElementPropertiesMask ()))
        {
        DgnElementClass    elmClass;

        if (context.DoElementClassCallback (&elmClass, EachElementClassArg ((DgnElementClass) elmP->GetElementClass(), PROPSCALLBACK_FLAGS_IsBaseID, context)))
            {
            switch (static_cast<DgnElementClass>(elmP->GetElementClass())) // Don't rely on IEditProperties to preserve rule class...
                {
                case DgnElementClass::PrimaryRule:
                case DgnElementClass::ConstructionRule:
                    {
                    switch (elmClass)
                        {
                        case DgnElementClass::PrimaryRule:
                        case DgnElementClass::Primary:
                            elmP->SetElementClass(DgnElementClass::PrimaryRule);
                            break;

                        case DgnElementClass::ConstructionRule:
                        case DgnElementClass::Construction:
                            elmP->SetElementClass(DgnElementClass::ConstructionRule);
                            break;
                        }
                    break;
                    }

                default:
                    {
                    elmP->SetElementClass(elmClass);
                    break;
                    }
                }
            }
        }

    bool    preserveMatchingPatternColor = false;
    UInt32  oldColor = elmP->GetSymbology().color;

    if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
        {
        EachColorArg    colorArg (elmP->GetSymbology().color, PROPSCALLBACK_FLAGS_IsBaseID, context);

        context.DoColorCallback (&elmP->GetSymbologyR().color, colorArg);

        preserveMatchingPatternColor = (0 != (colorArg.GetPropertyCallerFlags () & PROPSCALLER_FLAGS_PreserveMatchingDecorationColor));

        // NOTE: Only allow remap of fill, add/removal of fill/gradient/pattern should be handled elsewhere!
        Display_attribute   attribute;

        if (mdlElement_displayAttributePresent (elmP, FILL_ATTRIBUTE, &attribute))
            {
            if (0 != (colorArg.GetPropertyCallerFlags () & PROPSCALLER_FLAGS_PreserveOpaqueFill) && elmP->GetSymbology().color != oldColor && oldColor == attribute.attr_data.fill.color)
                {
                attribute.attr_data.fill.color = elmP->GetSymbology().color;
                mdlElement_displayAttributeReplace (elmP, FILL_ATTRIBUTE, &attribute);
                }
            else
                {
                if (context.DoColorCallback (&attribute.attr_data.fill.color, EachColorArg (attribute.attr_data.fill.color, PROPSCALLBACK_FLAGS_IsBackgroundID, context)))
                    mdlElement_displayAttributeReplace (elmP, FILL_ATTRIBUTE, &attribute);
                }
            }
        }

    if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
        context.DoWeightCallback (&elmP->GetSymbologyR().weight, EachWeightArg (elmP->GetSymbology().weight, PROPSCALLBACK_FLAGS_IsBaseID, context));

    if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
        {
        LineStyleParams lsParams;

        LineStyleLinkageUtil::ExtractParams (&lsParams, elmP);

        EachLineStyleArg arg (elmP->GetSymbology().style, &lsParams, PROPSCALLBACK_FLAGS_IsBaseID, context);

        long    oldStyle = elmP->GetSymbology().style;

        if (context.DoLineStyleCallback (&elmP->GetSymbologyR().style, arg))
            {
            bool    paramsChanged = (arg.GetParamsChanged () && _IsSupportedOperation (&eeh, SupportOperation::LineStyle));

            if (IS_LINECODE (elmP->GetSymbology().style) || (paramsChanged && 0 == arg.GetParams ()->modifiers))
                {
                LineStyleLinkageUtil::ClearElementStyle (elmP, true, 0, 0);
                }
            else if (paramsChanged && 0 != arg.GetParams ()->modifiers) // POSSIBLE SIZE CHANGE!
                {
                DgnElementP  tmpElmP = (DgnElementP) alloca (elmP->Size () + (2*sizeof (StyleLink)));

                elmP->CopyTo (*tmpElmP);
                LineStyleLinkageUtil::SetStyleParams (tmpElmP, (LineStyleParams *) (arg.GetParams ()));
                eeh.ReplaceElement (tmpElmP);

                elmP = eeh.GetElementP ();
                }

            // If purpose is change instead of just a simple id remap we need to recompute range...
            if (EditPropertyPurpose::Change == context.GetIEditPropertiesP ()->_GetEditPropertiesPurpose ())
                {
                // Header may be processed BEFORE children...validate will take care of updating it's range...
                if ((paramsChanged || (IS_LINECODE (oldStyle) != IS_LINECODE (elmP->GetSymbology().style))))
                    _ValidateElementRange (eeh);
                }
            }
        }

    if (0 != ((ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Weight) & context.GetElementPropertiesMask ()) && mdlElement_attributePresent (elmP, PATTERN_ID, NULL))
        {
#if defined (NEEDS_WORK_DGNITEM)
        int                 patIndex = 0;
        HatchLinkage*       hatchLinkP;
        PatternParams*      patParamsP = (PatternParams *) alloca (sizeof (PatternParams));
        DwgHatchDefLine*    hatchLinesP = (DwgHatchDefLine *) alloca (MAX_DWG_EXPANDEDHATCH_LINES * sizeof (DwgHatchDefLine));

        while (NULL != (hatchLinkP = (HatchLinkage *) linkage_extractLinkageByIndex (NULL, elmP, PATTERN_ID, NULL, patIndex)))
            {
            bool    patternChanged = false;

            patParamsP->Init();
            PatternLinkageUtil::Extract (*patParamsP, hatchLinesP, MAX_DWG_EXPANDEDHATCH_LINES, hatchLinkP, elmP->Is3d());

            if (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Color) && 0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                {
                if (preserveMatchingPatternColor && elmP->GetSymbology().color != oldColor && oldColor == patParamsP->color)
                    {
                    patParamsP->color = elmP->GetSymbology().color;
                    patternChanged = true;
                    }
                else
                    {
                    if (context.DoColorCallback (&patParamsP->color, EachColorArg (patParamsP->color, PROPSCALLBACK_FLAGS_IsDecorationID, context)))
                        patternChanged = true;
                    }
                }

            if (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Weight) && 0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                if (context.DoWeightCallback (&patParamsP->weight, EachWeightArg (patParamsP->weight, PROPSCALLBACK_FLAGS_IsDecorationID, context)))
                    patternChanged = true;

            if (PatternParamsModifierFlags::None != (patParamsP->modifiers & PatternParamsModifierFlags::Style) && 0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                if (context.DoLineStyleCallback ((Int32 *) &patParamsP->style, EachLineStyleArg (patParamsP->style, NULL, PROPSCALLBACK_FLAGS_IsDecorationID, context)))
                    patternChanged = true;

            if (patternChanged)
                PatternLinkageUtil::Create (*hatchLinkP, *patParamsP, hatchLinesP, elmP->Is3d());

            patIndex++;
            }
#endif
        }

    if (0 != (ELEMENT_PROPERTY_Transparency & context.GetElementPropertiesMask ()))
        {
        Display_attribute   dispAttr;
        double              transparency = mdlElement_displayAttributePresent (elmP, TRANSPARENCY_ATTRIBUTE, &dispAttr) ? dispAttr.attr_data.transparency.transparency : 0.0;
        bool                haveDisplayAttr = (0.0 != transparency);

        if (context.DoTransparencyCallback (&transparency, EachTransparencyArg (transparency, PROPSCALLBACK_FLAGS_IsBaseID, context) ))
            {
            // NOTE: Allow linkage to be added, transparency is still a property with value of 0.0 if linkage not present...
            if (!haveDisplayAttr && 0.0 != transparency)
                {
                DgnElementP  tmpElmP = (DgnElementP) alloca (elmP->Size () + (2*sizeof (Display_attribute)));

                elmP->CopyTo (*tmpElmP);

                Display_attribute_transparency  transparencyData;

                transparencyData.transparency = transparency;

                if (SUCCESS == mdlElement_displayAttributeCreate (&dispAttr, TRANSPARENCY_ATTRIBUTE, sizeof (transparencyData), (UShort *) &transparencyData))
                    {
                    mdlElement_displayAttributeAdd (tmpElmP, &dispAttr);
                    eeh.ReplaceElement (tmpElmP);

                    elmP = eeh.GetElementP ();
                    }
                }
            else if (0.0 == transparency)
                {
                mdlElement_displayAttributeRemove (elmP, TRANSPARENCY_ATTRIBUTE);
                }
            else
                {
                dispAttr.attr_data.transparency.transparency = transparency;
                mdlElement_displayAttributeReplace (elmP, TRANSPARENCY_ATTRIBUTE, &dispAttr);
                }
            }
        }

#ifdef WIP_VANCOUVER_MERGE // material
    if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
        {
        IMaterialPropertiesExtension    *mExtension = IMaterialPropertiesExtension::Cast (eeh.GetHandler ());
        DgnMaterialId                    newMaterialId, materialId;
        
        if (mExtension && (SUCCESS == mExtension->StoresAttachmentInfo (eeh, materialId)) &&
            context.DoMaterialCallback (&newMaterialId, EachMaterialArg (materialId, PROPSCALLBACK_FLAGS_IsBaseID, context)))
            mExtension->AddMaterialAttachment (eeh, newMaterialId);
        }
#endif

#ifdef WIP_VANCOUVER_MERGE // template
    if (0 != (ELEMENT_PROPERTY_ElementTemplate & context.GetElementPropertiesMask ()))
        {
        ElementId  templateId = TemplateRefAttributes::GetReferencedTemplateIDFromHandle (eeh);
        EachElementTemplateArg                       etArg (templateId, PROPSCALLBACK_FLAGS_IsBaseID, context);

        if (context.DoElementTemplateCallback (&templateId, etArg))
            TemplateRefAttributes::AttatchToHandle (eeh, templateId, etArg.GetApplyDefaultSymbology()); 
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::QueryThicknessProperty (ElementHandleCR eh, PropertyContextR context)
    {
    if (0 == (ELEMENT_PROPERTY_Thickness & context.GetElementPropertiesMask ()))
        return;

    DgnElementCP elmCP = eh.GetElementCP ();

    if (!elmCP->Is3d())
        return;

    double      thickness = 0.0;
    DVec3d      direction;
    bool        capped = false, alwaysUseDirection = false;

    direction.zero ();
    ElementUtil::ExtractThickness (thickness, direction, capped, alwaysUseDirection, eh);

    context.DoThicknessCallback (NULL, EachThicknessArg (thickness, &direction, capped, alwaysUseDirection, PROPSCALLBACK_FLAGS_IsBaseID, context));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::EditThicknessProperty (EditElementHandleR eeh, PropertyContextR context)
    {
    if (0 == (ELEMENT_PROPERTY_Thickness & context.GetElementPropertiesMask ()))
        return;

    DgnElementCP elmP = eeh.GetElementP ();

    if (!elmP->Is3d())
        return;

    double      thickness = 0.0;
    DVec3d      direction;
    bool        capped = false, alwaysUseDirection = false;

    direction.zero ();
    ElementUtil::ExtractThickness (thickness, direction, capped, alwaysUseDirection, eeh);

    EachThicknessArg    arg (thickness, &direction, capped, alwaysUseDirection, PROPSCALLBACK_FLAGS_IsBaseID, context);

    if (!context.DoThicknessCallback (&thickness, arg))
        return;

    if (SUCCESS != ElementUtil::AddThickness (eeh, thickness, SUCCESS == arg.GetDirection (direction) ? &direction : NULL, arg.GetCapped (), arg.GetAlwaysUseDirection ()))
        return;

    DisplayHandlerP dHandler = eeh.GetDisplayHandler ();

    if (dHandler)
        dHandler->ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
#if defined (NEEDS_WORK_DGNITEM)
    DgnElementCP elmCP = (eh ? eh->GetElementCP () : NULL);

    switch (stype)
        {
        case SupportOperation::Selection:
            return (elmCP ? (!elmCP->IsDictionary() && !elmCP->IsInvisible()) : true);

        case SupportOperation::CellGroup:
            return (elmCP ? (!elmCP->IsDictionary() && !elmCP->IsInvisible()) : false);
        }
#endif

    return Handler::_IsSupportedOperation (eh, stype);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (elHandle);
    double          pathLength;

    if (pathCurve.IsValid () && pathCurve->WireCentroid (pathLength, origin))
        return;

    GetRangeCenter (elHandle, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_GetSnapOrigin (ElementHandleCR elHandle, DPoint3dR origin)
    {
    CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (elHandle);
    DPoint3d        endPt;

    if (pathCurve.IsValid () && pathCurve->GetStartEnd (origin, endPt))
        return;

    GetRangeCenter (elHandle, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation)
    {
    CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (elHandle);

    if (pathCurve.IsValid ())
        {
        Transform   trans;
        if (pathCurve->GetAnyFrenetFrame (trans))
            {
            trans.GetMatrix (orientation);
            return;
            }

        DPoint3d        startPt, endPt;
        DVec3d          startTan, endTan;

        if (pathCurve->GetStartEnd (startPt, endPt, startTan, endTan))
            {
            if (startTan.isParallelTo (&endTan))
                orientation.initFrom1Vector (&startTan, 0, false);
            else
                orientation.initFrom2Vectors (&startTan, &endTan);

            orientation.squareAndNormalizeColumns (&orientation, 0, 2);

            return;
            }
        }

    orientation.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void    appendToDescr (WStringR descr, WCharCP dataStr, WCharCP prefix)
    {
    if (NULL == dataStr || 0 == dataStr[0])
        return;

    descr.append(prefix).append(dataStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayHandler::_GetPathDescription (ElementHandleCR el, WStringR descr, DisplayPathCP path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr)
    {
    _GetDescription (el, descr, 100);  // start with element's description

    if (path && (1 < path->GetCount()))
        {
        ElementHandle inner (path->GetPathElem (-1));
        DisplayHandlerP h = inner.GetDisplayHandler ();
        if (h)
            {
            WString innerStr;
            h->GetDescription (inner, innerStr, 100);
            if (!innerStr.empty())
                descr.append (L" \\ ").append (innerStr);
            }
        }

    if (levelStr && '\0' != levelStr[0]) appendToDescr (descr, levelStr, delimiterStr);
    if (modelStr && '\0' != modelStr[0]) appendToDescr (descr, modelStr, delimiterStr);
    if (groupStr && '\0' != groupStr[0]) appendToDescr (descr, groupStr, delimiterStr);
    }

/*---------------------------------------------------------------------------------**//**
* Default implementation of CalulateRange for DisplayHandler.
* Calculates the range by calling the Draw method into a "RangeContext"
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DisplayHandler::_ValidateElementRange (EditElementHandleR elHandle)
    {
    return CalculateDefaultRange (elHandle);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DisplayHandler::_OnGeoCoordinateReprojection (EditElementHandleR eeh, IGeoCoordinateReprojectionHelper& gcrH, bool inChain)
    {
    DPoint3d        origin;
    TransformInfo   transform;

    _GetTransformOrigin (eeh, origin);
    ReprojectStatus status = gcrH.GetLocalTransform (&transform.GetTransformR(), origin, NULL, true, true);
    if ( (REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != status) )
        return status;

    // if can't transform, don't change what we have.
    if (SUCCESS != _ApplyTransform (eeh, transform))
        return REPROJECT_NoChange;

    return REPROJECT_Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DisplayHandler::_GetBasisTransform (ElementHandleCR eh, TransformR transform) 
    {
    if (SUCCESS == BasisXAttributesUtil::GetTransform (&transform, eh))
        return true;

    DPoint3d        origin;
    RotMatrix       rMatrix;

    GetOrientation (eh, rMatrix);
    GetTransformOrigin (eh, origin);

    transform.InitFrom (rMatrix, origin);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DisplayHandler::_GetBasisRange (ElementHandleCR eh, DRange3dR range)
    {
    if (SUCCESS == BasisXAttributesUtil::GetRange (&range, eh))
        return true;

    Transform       basisTransform, inverseBasisTransform;

    if (!GetBasisTransform (eh, basisTransform))
        {
        range = *eh.GetIndexRange();
        return false;
        }

    return  inverseBasisTransform.InverseOf (basisTransform) && 
            SUCCESS == DisplayHandler::CalcElementRange (eh, range, &inverseBasisTransform);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
bool            ExtendedElementHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayHandler::_SetBasisRange (EditElementHandleR eh, DRange3dCR range)
    {
    if (SUCCESS == BasisXAttributesUtil::GetRange (NULL, eh))
        BasisXAttributesUtil::SetRange (range, eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayHandler::_SetBasisTransform (EditElementHandleR eh, TransformCR transform)
    {
    if (SUCCESS == BasisXAttributesUtil::GetTransform (NULL, eh))
        BasisXAttributesUtil::SetTransform (transform, eh);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
ElementHandlerId OgreLeadHandler::GetElemHandlerId() // added in graphite
    {
    return ElementHandlerId (22830, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
ElementHandlerId OgreFollowerHandler::GetElemHandlerId() // added in graphite
    {
    return ElementHandlerId (22831, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
void OgreLeadHandler::AttachToElement (EditElementHandleR eeh) // added in graphite
    {
    eeh.GetElementDescrP()->SetElementHandler (&ELEMENTHANDLER_INSTANCE(OgreLeadHandler));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
void OgreFollowerHandler::AttachToElement (EditElementHandleR eeh) // added in graphite
    {
    eeh.GetElementDescrP()->SetElementHandler (&ELEMENTHANDLER_INSTANCE(OgreFollowerHandler));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
void OgreLeadHandler::_GetTypeName (WStringR string, UInt32 desiredLength) // added in graphite
    {
    string = DgnHandlersMessage::GetStringW (DgnHandlersMessage::IDS_TYPENAMES_OGRE_LEADER);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
void OgreFollowerHandler::_GetTypeName (WStringR string, UInt32 desiredLength) // added in graphite
    {
    string = DgnHandlersMessage::GetStringW (DgnHandlersMessage::IDS_TYPENAMES_OGRE_MEMBER);
    }
#endif

//  DGNV10FORMAT_CHANGES_WIP dynamic_cast kludge -- These methods provide a temporary workaround to the 
//  Android dynamic_cast problem.
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2012
//--------------+------------------------------------------------------------------------
ICurvePathQuery* DisplayHandler::_CastToCurvePathQuery() { return dynamic_cast<ICurvePathQuery*> (this); } // added in graphite
