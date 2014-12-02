/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DropContext.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DropToElementDrawGeom::_ClipPreservesRegions () const
    {
    if (!m_preserveClosedCurves)
        return false;

    // When dropping patterns with surfaces requested, preserve closed elements...
    if (!m_inSymbolDraw || (0 == (DropGraphics::OPTION_Patterns & m_graphics.GetOptions ())))
        return true;

    return (DropGeometry::SOLID_Surfaces == m_geometry.GetSolidsOptions ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
DropToElementDrawGeom::DropToElementDrawGeom (ElementAgendaP agenda)
    {
    m_agenda = agenda;
    m_nonsnappable = false;
    m_preserveClosedCurves = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
DropToElementDrawGeom::~DropToElementDrawGeom () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
void    DropToElementDrawGeom::Init (ViewContextP context, DropGeometryCR geometry, DropGraphicsCR graphics)
    {
    SetViewContext (context);

    m_geometry = geometry;
    m_graphics = graphics;

    // Make sure linestyles drawn for drop...esp. when dropping linestyles!
    m_viewFlags.inhibitLineStyles = false;

    // Make sure to display fill so that fill/gradient can be added to output...
    m_viewFlags.fill = true;

    // Make sure to display patterns so that pattern linkage can be added to output...
    m_viewFlags.patterns = true;

    // Don't draw text node number/cross when dropping text!
    if (0 != (geometry.GetOptions() & DropGeometry::OPTION_Text))
        m_viewFlags.text_nodes = false;

    m_viewFlags.renderMode = static_cast<UInt32>(DropGeometry::SOLID_Surfaces == GetDropGeometry ().GetSolidsOptions () ? MSRenderMode::SmoothShade : MSRenderMode::Wireframe);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DropToElementDrawGeom::_OnElementCreated (EditElementHandleR eeh)
    {
    // When dropping patterns we only want the pattern geometry!
    if (DropGraphics::BOUNDARY_Include != m_graphics.GetPatternBoundary () && !m_inPatternDraw)
        {
        eeh.Invalidate ();

        return;
        }

    m_agenda->InsertElemDescr (eeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::_OnOutputElement (ElementHandleCR eh)
    {
    if (SUCCESS != T_Super::_OnOutputElement (eh))
        return ERROR;

    if (!m_inPatternDraw && !m_inSymbolDraw)
        {
        DgnElementCP elmP = eh.GetElementCP ();

        if (CMPLX_SHAPE_ELM == elmP->GetLegacyType() || CMPLX_STRING_ELM == elmP->GetLegacyType())
            {
            ChildElemIter   childIter (eh, ExposeChildrenReason::Count);

            if (!childIter.IsValid ())
                SetElementNonSnappable (false);
            else
                SetElementNonSnappable (childIter.GetElementCP()->IsGraphic() ? childIter.GetElementCP()->IsSnappable() : false);
            }
        else
            {
            SetElementNonSnappable (elmP->IsGraphic() ? elmP->IsSnappable() : false);
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     01/07
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCP     DropToElementDrawGeom::GetCurrTransformToWorld (TransformR tmpTransform)
    {
    DMatrix4d   localToWorld;

    if (SUCCESS != GetViewContext ()->GetCurrLocalToWorldTrans (localToWorld))
        return NULL;

    return (tmpTransform.InitFrom (localToWorld) && !tmpTransform.IsIdentity () ? &tmpTransform : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2011
*
* This will find an index color if there is an exact match - else it will return an
* return the raw color (for an RGB extended color.
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 getElementColorFromRGBA (UInt32 suggestedColor, UInt32 rgbaColor, DgnModelP modelRef, ViewportP viewport)
    {
    IntColorDef inColorDef, indexedColor, elementColor;
    UInt32      outputColor = suggestedColor;
    DgnProjectR dgnFile = modelRef->GetDgnProject();

    inColorDef.m_int = rgbaColor;

    if (SUCCESS == dgnFile.Colors().Extract (&elementColor, NULL, NULL, NULL, NULL, suggestedColor) &&
        elementColor.m_int != inColorDef.m_int)
        {
        static IntColorDef  s_white = IntColorDef (0xff, 0xff, 0xff);
        IntColorDef         testColor;
        UInt32              rgbColor = rgbaColor & 0xffffff;
        bool                isBackground = NULL != viewport && rgbaColor == viewport->GetBackgroundColor();
        bool                isBlack = (rgbColor == 0);
        bool                isWhite = (rgbColor == 0xffffff);

        if ((isBackground || isBlack || isWhite) &&                                                       
            SUCCESS == dgnFile.Colors().Extract (&testColor, NULL, NULL, NULL, NULL, 0) && // Color 0 is white - use this as it will autoInvert.
            testColor.m_int == s_white.m_int)
            {
            outputColor = 0;
            }
        else
            {
            UInt32          colorIndex;
            DgnColorMapP    colorMap = dgnFile.Colors().GetDgnColorMapP();

            // First look for an exact match indexed color;
            colorIndex = colorMap->FindClosestMatch (inColorDef, NULL);

            // Use the closest match if it matches exactly - else create a true color.
            if (SUCCESS == dgnFile.Colors().Extract (&indexedColor, NULL, NULL, NULL, NULL, colorIndex) && indexedColor.m_int == inColorDef.m_int)
                return colorIndex;

            return dgnFile.Colors().CreateElementColor (inColorDef, NULL, NULL);
            }
        }

    return outputColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 getElementColor (ElementHandleCR eh)
    {
    if (!eh.IsValid () || !eh.GetElementCP ()->IsGraphic())
        return 0;

    return eh.GetElementCP ()->GetSymbology().color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void applyGradientFill (EditElementHandleR eeh, GradientSymbCR symb)
    {
    IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetHandler ());

    if (areaObj)
        {
        areaObj->AddGradientFill (eeh, symb);
        return;
        }

    // Apply to public children (ex. some dropped text glyphs)...
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        applyGradientFill (childElm, symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void applySolidFill (EditElementHandleR eeh, UInt32* fillColor = NULL, bool* alwaysFilled = NULL)
    {
    IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetHandler ());

    if (areaObj)
        {
        areaObj->AddSolidFill (eeh, fillColor, alwaysFilled);
        return;
        }

    // Apply to public children (ex. some dropped text glyphs)...
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        applySolidFill (childElm, fillColor, alwaysFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void setUnsnappable (MSElementDescrP edP)
    {
    if (edP->Element().IsGraphic())
        edP->ElementR().SetSnappable(true);

    for (auto& child : edP->Components())
        setUnsnappable(child.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DropToElementDrawGeom::_ApplyRenderMode (EditElementHandleR eeh, bool isClosed, bool isFilled)
    {
    if (0 == (m_graphics.GetOptions () & DropGraphics::OPTION_ApplyRenderMode))
        return false;

    ViewFlagsCP           viewFlags = m_context->GetViewFlags ();
    CookedDisplayStyleCP  displayStyle = m_context->GetCurrentCookedDisplayStyle ();

    if (NULL == viewFlags || NULL == displayStyle)
        return false;

    DgnModelP            modelRef = eeh.GetDgnModelP ();
    ElementPropertiesSetter renderModeRemapper;
         
    if ((isClosed || displayStyle->m_flags.m_applyEdgeStyleToLines) && viewFlags->renderMode > static_cast<UInt32>(MSRenderMode::SolidFill))
        {
        UInt32  suggestedColor = getElementColor (eeh);
        UInt32  fillColor = suggestedColor;

        if (displayStyle->m_flags.m_elementColor)
            {
            if (INVALID_COLOR == (fillColor = displayStyle->m_elementColorIndex))
                fillColor = getElementColorFromRGBA (suggestedColor, displayStyle->m_elementColor, modelRef, m_context->GetViewport ()); 

            if (255 == fillColor) // TR#303204 - background fill - treat this as unfilled. 
                isClosed = false;
            }

        // Use a non-reversable white (rgb) for filled hidden line...
        IntColorDef colorDef;

        if (SUCCESS == modelRef->GetDgnProject().Colors().Extract (&colorDef, NULL, NULL, NULL, NULL, fillColor))
            fillColor = modelRef->GetDgnProject().Colors().CreateElementColor (colorDef, NULL, NULL);

        if (isClosed)
            applySolidFill (eeh, &fillColor); // Should this be done after calling Apply?!?

        if (displayStyle->m_flags.m_edgeColor)
            renderModeRemapper.SetColor (getElementColorFromRGBA (suggestedColor, displayStyle->m_edgeColor, modelRef, m_context->GetViewport ()));
        else
            renderModeRemapper.SetColor (getElementColorFromRGBA (suggestedColor, m_context->GetViewport ()->GetSolidFillEdgeColor (fillColor, modelRef), modelRef, m_context->GetViewport ()));
        }
    else
        {
        if (displayStyle->m_flags.m_elementColor)
            {
            if (INVALID_COLOR == displayStyle->m_elementColorIndex)
                renderModeRemapper.SetColor (getElementColorFromRGBA (getElementColor (eeh), displayStyle->m_elementColor, modelRef, m_context->GetViewport ())); 
            else
                renderModeRemapper.SetColor (displayStyle->m_elementColorIndex);
            }

        if (displayStyle->m_flags.m_edgeColor && viewFlags->renderMode > static_cast<int>(MSRenderMode::Wireframe))
            renderModeRemapper.SetColor (getElementColorFromRGBA (getElementColor (eeh), displayStyle->m_edgeColor, modelRef, m_context->GetViewport ()));

        if (MSRenderMode::Wireframe == viewFlags->renderMode && isClosed && isFilled) // TR#324271. Fill Color??
           applySolidFill (eeh); // Should this be done after calling Apply?!?
        }

    bool    displayEdges = (MSRenderMode::SolidFill == viewFlags->renderMode || MSRenderMode::HiddenLine == viewFlags->renderMode || (MSRenderMode::SmoothShade == viewFlags->renderMode && viewFlags->renderDisplayEdges));

    if (displayEdges && (isClosed || displayStyle->m_flags.m_applyEdgeStyleToLines))
        {
        for (UInt32 weight = 0; weight < 32; weight++)
            {
            if (m_context->GetViewport ()->GetIndexedLineWidth (weight) == displayStyle->m_visibleEdgeWidth)
                {
                renderModeRemapper.SetWeight (weight);
                break;
                }
            }
        }

    renderModeRemapper.Apply (eeh);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DropToElementDrawGeom::_ApplyCurrentAreaParams (EditElementHandleR eeh)
    {
    ElemDisplayParamsR  elParams = *m_context->GetCurrentDisplayParams ();

    if (NULL != elParams.GetGradient ())
        {
        applyGradientFill (eeh, *elParams.GetGradient ());
        }
    else if (FillDisplay::Never != elParams.GetFillDisplay ())
        {
        DgnModelP     modelRef = eeh.GetDgnModelP ();
        ElemMatSymb   matSymb;

        GetCurrentMatSymb (matSymb);

        UInt32        ovrFlags = (0 == (m_graphics.GetOptions () & DropGraphics::OPTION_ApplyOverrides)) ? 0 : GetCurrentOverrideFlags ();
        bool          alwaysFill = ((FillDisplay::Always == elParams.GetFillDisplay () || FillDisplay::Blanking == elParams.GetFillDisplay ()) ? true : false);
        UInt32        fillColor;

        if (0 != (ovrFlags & MATSYMB_OVERRIDE_FillColor))
            {
            if (matSymb.GetFillColorIndex () > DgnColorMap::INDEX_Invalid && matSymb.GetFillColorIndex () <= DgnColorMap::INDEX_Background)
                fillColor = matSymb.GetFillColorIndex ();
            else
                fillColor = getElementColorFromRGBA (getElementColor (eeh), matSymb.GetFillColorTBGR (), modelRef, m_context->GetViewport ());
            }
        else if (INVALID_COLOR == elParams.GetFillColor ())
            {
            fillColor = getElementColorFromRGBA (getElementColor (eeh), elParams.GetFillColorTBGR (), modelRef, m_context->GetViewport ());
            }
        else
            {
            fillColor = elParams.GetFillColor ();
            }

        applySolidFill (eeh, &fillColor, &alwaysFill);
        }
    else if (m_inTextDraw || 0 != (m_graphics.GetOptions() & DropGraphics::OPTION_LineStyles))
        {
        // NOTE: Filled shape w/continuous lstyle displays based on current display params...i.e. handled by above cases...
        //       For text and linestyle symbols/strokes, filled means always opaque filled...
        applySolidFill (eeh, NULL, &m_inTextDraw);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DropToElementDrawGeom::_ApplyCurrentDisplayParams (EditElementHandleR eeh)
    {
    DgnModelP           modelRef = eeh.GetDgnModelP ();
    ElemDisplayParamsR  elParams = *m_context->GetCurrentDisplayParams ();
    ElemMatSymb         matSymb;

    GetCurrentMatSymb (matSymb);

    UInt32              ovrFlags = (0 == (m_graphics.GetOptions () & DropGraphics::OPTION_ApplyOverrides)) ? 0 : GetCurrentOverrideFlags ();

    ElementPropertiesSetter remapper;

    remapper.SetLevel (elParams.GetLevelSubLevelId().GetLevel());
    remapper.SetDisplayPriority (elParams.GetElementDisplayPriority ());
    remapper.SetElementClass (elParams.GetElementClass ());
    remapper.SetWeight (elParams.GetWeight ());

    bool    useStyle = !m_inThicknessDraw && (0 == (m_graphics.GetOptions () & DropGraphics::OPTION_LineStyles) || IS_LINECODE (elParams.GetLineStyle ()));

    if (!useStyle)
        {
        remapper.SetLinestyle (0, NULL);
        }
    else if (IS_LINECODE (elParams.GetLineStyle ()) || NULL == elParams.GetLineStyleParams ())
        {
        remapper.SetLinestyle (elParams.GetLineStyle (), NULL);
        }
    else
        {
        Transform   transform;

        // NOTE: GetCurrTransformToWorld includes things like, solid->uor scale, that should not be applied to the ElemDisplayParams...
        if (SUCCESS == GetElementToWorldTransform (transform))
            {
            LineStyleParams styleParams = *elParams.GetLineStyleParams ();

            styleParams.ApplyTransform (transform);
            remapper.SetLinestyle (elParams.GetLineStyle (), &styleParams);
            }
        else
            {
            remapper.SetLinestyle (elParams.GetLineStyle (), elParams.GetLineStyleParams ());
            }
        }

    if (0 != (ovrFlags & MATSYMB_OVERRIDE_Color))
        {
        if (matSymb.GetLineColorIndex () > DgnColorMap::INDEX_Invalid && matSymb.GetLineColorIndex () <= DgnColorMap::INDEX_Background)
            remapper.SetColor (matSymb.GetLineColorIndex ());
        else
            remapper.SetColor (getElementColorFromRGBA (elParams.GetLineColor (), matSymb.GetLineColorTBGR (), modelRef, m_context->GetViewport ()));
        }
    else if (INVALID_COLOR == elParams.GetLineColor ())
        {
        UInt32      colorIndex;
        IntColorDef colorDef;

        colorDef.m_int = elParams.GetLineColorTBGR ();

        if (INVALID_COLOR != (colorIndex = modelRef->GetDgnProject().Colors().CreateElementColor (colorDef, NULL, NULL)))
            remapper.SetColor (colorIndex);
        }
    else
        {
        remapper.SetColor (elParams.GetLineColor ());
        }

    if (0 != (ovrFlags & MATSYMB_OVERRIDE_Style)) 
        {
        for (int i=0; i<=MAX_LINECODE; i++)
            {
            if (m_context->GetIndexedLinePattern (i) == matSymb.GetRasterPattern ())
                {
                remapper.SetLinestyle (i, NULL);
                break;
                }
            }
        }

    if (0 != (ovrFlags & MATSYMB_OVERRIDE_RastWidth) && m_context->GetIndexedLineWidth (elParams.GetWeight ()) != matSymb.GetWidth ())
        {
        for (int i=0; i<32 /* MAX_LINEWEIGHTS */; i++)
            {
            if (m_context->GetIndexedLineWidth (i) == matSymb.GetWidth ())
                {
                remapper.SetWeight (i);
                break;
                }
            }
        }

    if (0.0 != elParams.GetTransparency ())
        remapper.SetTransparency (elParams.GetTransparency ());

    remapper.Apply (eeh);

    // NOTE: Only need to make un-snappable as snappable is default...
    if (m_nonsnappable)
        setUnsnappable (eeh.GetElementDescrP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelP getSourceDgnModel (ViewContextR context)
    {
    // NEEDSWORK: DropToElementDrawGeom should maybe require destination DgnModel for new elements...
    if (NULL != context.GetCurrentElement ())
        return context.GetCurrentElement ()->GetDgnModelP ();
    else if (NULL != context.GetViewport ())
        return context.GetViewport ()->GetViewController ().GetTargetModel ();
    else
        return context.GetDgnProject ().Models ().GetModelById (context.GetDgnProject ().Models ().GetFirstModelId ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::_ProcessCurveVector (CurveVectorCR curves, bool isFilled)
    {
    DgnModelP   modelRef = getSourceDgnModel (*m_context);
    bool        create3d = modelRef->Is3d();

    if (!create3d)
        {
        // If we are converting a 3D curve vector into a 2d model (as in MVE from proxy model), create 3d and then
        // use ConvertTo2d so we get the handlers conversion (flattened arcs to lines etc.)
        DRange3d    range;

        curves.GetRange (range);

        if (range.low.z != 0.0 || range.high.z != 0.0)
            create3d = true;
        }

    Transform       tmpTransform;
    TransformCP     placementTransP = GetCurrTransformToWorld (tmpTransform);
    CurveVectorPtr  tmpCurves = curves.Clone ();

    if (placementTransP && !placementTransP->IsIdentity ())
        tmpCurves->TransformInPlace (*placementTransP);

    EditElementHandle   eeh;

    if (SUCCESS != DraftingElementSchema::ToElement (eeh, *tmpCurves, NULL, create3d, *modelRef))
        return ERROR;
        
    if (create3d && !modelRef->Is3d())
        {
        DVec3d              flattenDir;
        Transform           flattenTrans;

        flattenDir.init (0.0, 0.0, 1.0);
        flattenTrans.initIdentity ();
        flattenTrans.form3d[2][2] = 0.0;

        eeh.GetHandler ().ConvertTo2d (eeh, flattenTrans, flattenDir);
        }

    _ApplyCurrentDisplayParams (eeh);

    if (!_ApplyRenderMode (eeh, curves.IsAnyRegionType (), isFilled) && isFilled)
        _ApplyCurrentAreaParams (eeh);

    _OnElementCreated (eeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::_ProcessSolidPrimitive (ISolidPrimitiveCR primitive)
    {
    Transform           tmpTransform;
    TransformCP         placementTransP = GetCurrTransformToWorld (tmpTransform);
    ISolidPrimitivePtr  geomPtr = primitive.Clone ();

    if (placementTransP && !placementTransP->IsIdentity ())
        geomPtr->TransformInPlace (*placementTransP);

    DgnModelP           modelRef = getSourceDgnModel (*m_context);
    EditElementHandle   eeh;

    if (SUCCESS != DraftingElementSchema::ToElement (eeh, *geomPtr, NULL, *modelRef))
        return ERROR;

    _ApplyCurrentDisplayParams (eeh);
    _ApplyRenderMode (eeh);

    _OnElementCreated (eeh);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::_ProcessSurface (MSBsplineSurfaceCR surface)
    {
    Transform           tmpTransform;
    TransformCP         placementTransP = GetCurrTransformToWorld (tmpTransform);
    MSBsplineSurfacePtr tmpSurface = MSBsplineSurface::CreatePtr ();

    // Note - we always need to copy the surface as the create method will unweight the poles
    // and then reweight them. - The XGraphics draw code is drawing directly from memory that
    // is write-locked and this will cause a crash (TR#279340).
    tmpSurface->CopyFrom (surface);

    if (placementTransP)
        tmpSurface->TransformSurface (placementTransP);

    DgnModelP           modelRef = getSourceDgnModel (*m_context);
    EditElementHandle   eeh;

    if (BSPLINE_STATUS_Success != BSplineSurfaceHandler::CreateBSplineSurfaceElement (eeh, NULL, *tmpSurface, *modelRef))
        return ERROR;

    _ApplyCurrentDisplayParams (eeh);
    _ApplyRenderMode (eeh);

    _OnElementCreated (eeh);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::_ProcessBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP)
    {
    EditElementHandle   eeh;
    Transform           tmpTransform;
    TransformCP         placementTransP = GetCurrTransformToWorld (tmpTransform);

    if (SUCCESS != BrepCellHeaderHandler::CreateBRepCellElement (eeh, NULL, entity, *getSourceDgnModel (*m_context)))
        return ERROR;

    if (NULL != placementTransP)
        eeh.GetHandler().ApplyTransform (eeh, TransformInfo (*placementTransP));

    _ApplyCurrentDisplayParams (eeh);
    _ApplyRenderMode (eeh);

    _OnElementCreated (eeh);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     01/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::_ProcessFacetSet (PolyfaceQueryCR facets, bool filled)
    {
    PolyfaceHeaderPtr   meshData = PolyfaceHeader::New ();
    Transform           tmpTransform;
    TransformCP         placementTransP = GetCurrTransformToWorld (tmpTransform);

    meshData->CopyFrom (facets);

    if (NULL != placementTransP)
        meshData->Transform (*placementTransP);

    DgnModelP           modelRef = getSourceDgnModel (*m_context);
    EditElementHandle   eeh;

    // NEEDSWORK: Is it better to create lots-o-shapes for filled mesh or just ignore fill?!?
    if (SUCCESS != MeshHeaderHandler::CreateMeshElement (eeh, NULL, *meshData, modelRef->Is3d (), *modelRef))
        return SUCCESS;

    _ApplyCurrentDisplayParams (eeh);
    _ApplyRenderMode (eeh);

    _OnElementCreated (eeh);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAgendaP  DropToElementDrawGeom::GetDropGeometryAgenda ()
    {
    return m_agenda;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DropToElementDrawGeom::_DrawTextString (TextStringCR text, double* zDepth)
    {
    if (_DoTextGeometry ())
        {
        T_Super::_DrawTextString (text, zDepth);

        return;
        }

    DPoint3d    points[5];

    if (SUCCESS == text.GenerateBoundingShape (points) && ArePointsTotallyOutsideClip (points, 5))
        return; 

    DgnModelP       modelRef = getSourceDgnModel (*m_context);
    TextParamWide   textParam;

    text.GetProperties ().ToElementData (textParam, modelRef->GetDgnProject());

    EditElementHandle   eeh;

    if (SUCCESS != TextElemHandler::CreateElement (eeh, NULL, text.GetOrigin (), text.GetRotMatrix (), text.GetProperties ().GetFontSize (), textParam, text.GetString().c_str(), modelRef->Is3d (), *modelRef))
        return;

    Transform       tmpTransform;
    TransformCP     placementTransP = GetCurrTransformToWorld (tmpTransform);

    if (NULL != placementTransP)
        eeh.GetHandler ().ApplyTransform (eeh, TransformInfo (*placementTransP));

    _ApplyCurrentDisplayParams (eeh);
    _ApplyRenderMode (eeh);

    _OnElementCreated (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DropToElementDrawGeom::_ProcessAreaPattern (ElementHandleCR thisElm, ViewContext::ClipStencil& boundary, ViewContext::PatternParamSource& source)
    {
    DPoint3d         origin;
    PatternParamsP   params;
    DwgHatchDefLineP hatchLines;
    DgnModelP        modelRef = thisElm.GetDgnModelP ();

    if (NULL == (params = source.GetParams (thisElm, &origin, &hatchLines, NULL, m_context)))
        return;

    CurveVectorPtr  boundCurve = boundary.GetCurveVector (thisElm);

    if (!boundCurve.IsValid ())
        return;

    EditElementHandle  boundEeh;

    if (SUCCESS != DraftingElementSchema::ToElement (boundEeh, *boundCurve, NULL, modelRef->Is3d (), *modelRef))
        return;

    _ApplyCurrentDisplayParams (boundEeh);
    _ApplyRenderMode (boundEeh);

    EditElementHandle   eeh;

    // Create region w/invisible boundary instead of trying to figure out what element to add pattern to...
    if (DropGraphics::BOUNDARY_Only != m_graphics.GetPatternBoundary ())
        {
        ElementAgenda   agenda;
        RegionParams    regionParams;

        agenda.Insert (boundEeh);
        regionParams.SetInvisibleBoundary (true);

        if (SUCCESS != AssocRegionCellHeaderHandler::CreateAssocRegionElement (eeh, agenda, NULL, 0, NULL, 0, regionParams, L"Associative Region"))
            return;

        AutoRestore <PatternParamsModifierFlags> saveModifiers (&params->modifiers);

        params->modifiers = params->modifiers & ~PatternParamsModifierFlags::Origin; // Re-compute offset for region element...
        params->modifiers = params->modifiers & ~PatternParamsModifierFlags::Offset;

        ViewFlags   viewFlags = *m_context->GetViewFlags();

        if (viewFlags.renderMode >= static_cast<UInt32>(MSRenderMode::SolidFill) && (0 != (m_graphics.GetOptions() & DropGraphics::OPTION_ApplyRenderMode)))
            {
            if (MSRenderMode::SolidFill == viewFlags.renderMode || viewFlags.renderDisplayEdges)
                {
                CookedDisplayStyleCP  displayStyle;

                if (NULL != (displayStyle = m_context->GetCurrentCookedDisplayStyle()))
                    {
                    for (UInt32 weight = 0; weight < 32; weight++)
                        {
                        if (m_context->GetViewport()->GetIndexedLineWidth (weight) == displayStyle->m_visibleEdgeWidth)
                            {
                            params->weight = weight;
                            params->modifiers = params->modifiers | PatternParamsModifierFlags::Weight;
                            break;
                            }
                        }

                    if (displayStyle->m_flags.m_edgeColor)
                        {
                        IntColorDef colorDef;

                        colorDef.m_int = displayStyle->m_edgeColor;

                        if (INVALID_COLOR == (params->color = modelRef->GetDgnProject().Colors().CreateElementColor (colorDef, NULL, NULL)))
                            params->color = 0;
                        }
                    else
                        {
                        params->color = 0;
                        }

                    params->modifiers = params->modifiers | PatternParamsModifierFlags::Color;
                    }
                }
            }

        IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetHandler ());

        if (NULL != areaObj)
            {
            DVec3d      zVec;
            DPoint3d    patOrigin, elmOrigin;

            PatternLinkageUtil::GetHatchOrigin (elmOrigin, *eeh.GetElementCP ());
            eeh.GetDisplayHandler ()->IsPlanar (eeh, &zVec, NULL, NULL);

            DPoint3d    diff, tmp;

            diff.DifferenceOf (elmOrigin, origin);
            tmp.Scale (zVec, diff.DotProduct (zVec));
            patOrigin.SumOf (tmp, origin);

            // Compute new offset for region...
            if (!elmOrigin.IsEqual (patOrigin, 1.0e-8))
                {
                params->offset.DifferenceOf (patOrigin, elmOrigin);
                params->modifiers = params->modifiers | PatternParamsModifierFlags::Offset;
                }

            areaObj->AddPattern (eeh, *params, hatchLines);
            }
        }
    else
        {
        // Just return pattern boundary loops...
        eeh.SetElementDescr (boundEeh.ExtractElementDescr().get(), false);
        }

    Transform   tmpTransform;
    TransformCP placementTransP = GetCurrTransformToWorld (tmpTransform);

    if (_DoClipping () && NULL != GetCurrClip () && NULL != m_context->GetViewport ())
        {
        FenceParams fp;

        fp.SetViewParams (m_context->GetViewport ()); // WIP_V10_FENCE - Fences require a viewport (no longer creates "fake" viewport)...
        fp.SetClipMode (FenceClipMode::Original);
        fp.SetClip (*GetCurrClip());

        ElementAgenda inside;

        if (SUCCESS == eeh.GetHandler ().FenceClip (&inside, NULL, eeh, &fp, FenceClipFlags::Optimized))
            {
            for (EditElementHandleP curr = inside.GetFirstP (), end = curr + inside.GetCount (); curr < end ; curr++)
                {
                if (placementTransP)
                    eeh.GetHandler ().ApplyTransform (*curr, TransformInfo (*placementTransP));

                _OnElementCreated (*curr);
                }
            }

        return;
        }

    if (placementTransP)
        eeh.GetHandler ().ApplyTransform (eeh, TransformInfo (*placementTransP));

    _OnElementCreated (eeh);
    }

/*=================================================================================**//**
* Context to drop an element
* @bsiclass                                                     Brien.Bastings  06/05
+===============+===============+===============+===============+===============+======*/
struct DropContext : public NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

    DropToElementDrawGeom&      m_output;

virtual void _SetupOutputs () override {SetIViewDraw (m_output);}

public:

DropContext (ViewportP vp, DropGeometryCR geometry, DropGraphicsCR graphics, DropToElementDrawGeom& output) : m_output (output)
    {
    if (vp)
        {
        m_setupScan = true;

        Attach (vp, DrawPurpose::CaptureGeometry);
        }
    else
        {
        m_purpose = DrawPurpose::CaptureGeometry;
        }

    SetBlockAsynchs (true);

    m_output.Init (this, geometry, graphics);
    _SetupOutputs ();
    }

~DropContext () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) override
    {
    // Drop must preserve ByLevel values but still needs to create ElemMatSymb for _GetCurrLineStyle check. Update ignored values before saving state...
    m_ignores.Apply (elParams);
    ElemDisplayParamsStateSaver saveState (elParams, false, COLOR_BYLEVEL == elParams.GetLineColor (), COLOR_BYLEVEL == elParams.GetFillColor (), STYLE_BYLEVEL == elParams.GetLineStyle (), WEIGHT_BYLEVEL == elParams.GetWeight ());

    // Fully resolve and cook ElemMatSymb...
    elParams.Resolve (*this);
    elMatSymb.FromResolvedElemDisplayParams (elParams, *this, m_startTangent, m_endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CookDisplayParamsOverrides () override
    {
    // Apply header overrides only. Drop does not want to consider views flags...
    m_ovrMatSymb.SetFlags (MATSYMB_OVERRIDE_None);
    m_currDisplayParams.ApplyParentOverrides (GetHeaderOvr ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2007
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ILineStyleCP _GetCurrLineStyle (LineStyleSymbP* symb) override
    {
    // Don't want style to draw when not dropping lstyle...only need in current display params...
    if (m_output.GetInThicknessDraw () || 0 != (m_output.GetDropGraphics ().GetOptions () & DropGraphics::OPTION_LineStyles))
        return T_Super::_GetCurrLineStyle (symb);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _DrawWithThickness (ElementHandleCR thisElm, IStrokeForCache& stroker, Int32 qvIndex)
    {
    m_output.SetInThicknessDraw (true);
    T_Super::_DrawWithThickness (thisElm, stroker, qvIndex);
    m_output.SetInThicknessDraw (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _DrawSymbol (IDisplaySymbol* symbolDefP, Transform const* transP, ClipPlaneSetP clipPlaneSetP, bool ignoreColor, bool ignoreWeight) override
    {
    m_output.ClipAndProcessSymbol (symbolDefP, transP, clipPlaneSetP, ignoreColor, ignoreWeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _DrawAreaPattern (ElementHandleCR thisElm, ClipStencil& boundary, PatternParamSource& source)
    {
    int              patternIndex;
    PatternParamsP   params;

    if (NULL == (params = source.GetParams (thisElm, NULL, NULL, &patternIndex, this)))
        return;

    int         dropPatternIndex = m_output.GetDropGraphics ().GetPatternIndex ();

    // Test pattern index if we're dropping a specific pattern (mlines)...
    if (-1 != dropPatternIndex && dropPatternIndex != patternIndex)
        return;

    if (0 != (m_output.GetDropGraphics ().GetOptions() & DropGraphics::OPTION_Patterns) &&
        DropGraphics::BOUNDARY_Only != m_output.GetDropGraphics ().GetPatternBoundary ())
        {
        m_output.SetElementNonSnappable (PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::Snap));

        m_output.SetInPatternDraw (true);
        T_Super::_DrawAreaPattern (thisElm, boundary, source);
        m_output.SetInPatternDraw (false);

        return;
        }

    m_output.SetInPatternDraw (true);
    m_output.ProcessAreaPattern (thisElm, boundary, source);
    m_output.SetInPatternDraw (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
virtual UInt32  _GetDisplayInfo (bool isRenderable) override
    {
    UInt32      info = T_Super::_GetDisplayInfo (isRenderable);

    // Dropping linestyle...ignore thickness...
    if (0 != (m_output.GetDropGraphics ().GetOptions() & DropGraphics::OPTION_LineStyles))
        {
        if (0 != (info & (DISPLAY_INFO_Thickness)))
            {
            info &= ~(DISPLAY_INFO_Thickness);
            info |= DISPLAY_INFO_Edge;
            }
        }

    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OutputElement (ElementHandleCR elHandle) override
    {
    if (SUCCESS != m_output.OnOutputElement (elHandle))
        return;

    return T_Super::_OutputElement (elHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DoDrop (ElementHandleCR elHandle)
    {
    SetDgnProject (*elHandle.GetDgnProject ());

    return VisitElemHandle (elHandle, false, false);
    }

}; // DropContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
DropGeometry::DropGeometry ()
    {
    m_options = OPTION_None;

    m_dimensions    = DIMENSION_Geometry;
    m_sharedCells   = SHAREDCELL_NormalCell;
    m_solids        = SOLID_Wireframe;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
DropGeometry::DropGeometry (Options options)
    {
    m_options = options;

    m_dimensions    = DIMENSION_Geometry;
    m_sharedCells   = SHAREDCELL_NormalCell;
    m_solids        = SOLID_Wireframe;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DropGeometry::SetOptions (Options options) {m_options = options;}
DropGeometry::Options DropGeometry::GetOptions () const {return m_options;}

void DropGeometry::SetDimensionOptions (Dimensions option) {m_dimensions = option;}
DropGeometry::Dimensions DropGeometry::GetDimensionOptions () const {return m_dimensions;}

void DropGeometry::SetSharedCellOptions (SharedCells option) {m_sharedCells = option;}
DropGeometry::SharedCells DropGeometry::GetSharedCellOptions () const {return m_sharedCells;}

void DropGeometry::SetSolidsOptions (Solids option) {m_solids = option;}
DropGeometry::Solids DropGeometry::GetSolidsOptions () const {return m_solids;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DropGeometryPtr DropGeometry::Create () {return new DropGeometry ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
DropGraphics::DropGraphics ()
    {
    m_options = OPTION_None;

    m_patternBoundary = BOUNDARY_Include;
    m_patternIndex    = -1;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
DropGraphics::DropGraphics (Options options)
    {
    m_options = options;

    m_patternBoundary = BOUNDARY_Include;
    m_patternIndex    = -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DropGraphics::SetOptions (Options options) {m_options = options;}
DropGraphics::Options DropGraphics::GetOptions () const {return m_options;}

void DropGraphics::SetPatternIndex (int index) {m_patternIndex = index;}
int DropGraphics::GetPatternIndex () const {return m_patternIndex;}

void DropGraphics::SetPatternBoundary (PatternBoundary boundary) {m_patternBoundary = boundary;}
DropGraphics::PatternBoundary DropGraphics::GetPatternBoundary () const {return m_patternBoundary;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
DropGraphicsPtr DropGraphics::Create () {return new DropGraphics ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropToElementDrawGeom::DoDrop (ElementHandleCR elHandle, ElementAgendaR agenda, DropGeometryCR geometry, DropGraphicsCR graphics)
    {
    DropToElementDrawGeom output (&agenda);
    DropContext context (NULL, geometry, graphics, output);

    return context.DoDrop (elHandle);
    }

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     01/2012
+===============+===============+===============+===============+===============+======*/
struct CaptureGraphicsDrawGeom : DropToElementDrawGeom 
{
    bool                    m_destinationIs3d;
    
    CaptureGraphicsDrawGeom (ElementAgendaR agenda, bool destinationIs3d) : DropToElementDrawGeom (&agenda), m_destinationIs3d (destinationIs3d) 
        { 
        if (!destinationIs3d)
            m_geometry.SetSolidsOptions (DropGeometry::SOLID_Wireframe);
        }

};  // CaptureGraphicsDrawGeom
    
/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      09/2007
+===============+===============+===============+===============+===============+======*/
struct CaptureGraphicsContext : DropContext
{
    DEFINE_T_SUPER(DropContext)

private:
    CaptureGraphicsContext& operator=(CaptureGraphicsContext const&) {return *this;}           // NO!

protected:
    FenceParamsCP           m_fence;

public:
            ~CaptureGraphicsContext () { Detach (); }
bool        _ScanRangeFromPolyhedron() { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
CaptureGraphicsContext (ViewportP vp, DropGeometryCR geometry, DropGraphicsCR graphics, bool preserveClosedGPA, CaptureGraphicsDrawGeom& drawGeom, FenceParamsCP fence) : 
        DropContext (vp, geometry, graphics, drawGeom), m_fence (fence)
    {
    m_output.SetPreserveClosedCurves (preserveClosedGPA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            _OutputElement (ElementHandleCR eh) override
    {
    if (SUCCESS != m_output.OnOutputElement (eh))
        return;

    T_Super::_OutputElement (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CaptureDgnModel (DgnModelR model, bool includingRefs)
    {
    _SetScanReturn ();

    // set up scanner search criteria
    _InitScanCriteria ();
    m_ignoreViewRange = true;       // TR# 329490  - Capture independent of current view.
    m_useNpcSubRange = false;

    SetDgnProject (model.GetDgnProject ());

    if (NULL != m_fence)
        {
        Transform   transform;

        m_fence->PushClip (*this, m_output.GetCurrTransformToWorld (transform));
        }

    _VisitDgnModel (&model);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CaptureDisplayPath (DisplayPathCR displayPath)
    {
    _SetScanReturn ();
    _InitScanCriteria ();

    m_ignoreViewRange = true;       // TR# 329490  - Capture independent of current view.
    m_useNpcSubRange = false;
    
    return VisitPath (&displayPath, NULL);
    }

}; // CaptureGraphicsContext

/*---------------------------------------------------------------------------------**//**
* @remarks srcDgnModel must be currently displayed in vp
* @bsimethod                                                    Ray.Bentley     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt CaptureGraphicsToElement::CaptureViewGraphics (ElementAgendaR agenda, DgnModelR dest, DgnModelR source, ViewportP vp, DropGeometryCR geometry, DropGraphicsCR graphics, bool includeRefs, bool preserveClosedCurves, FenceParamsCP fence)
    {
    CaptureGraphicsDrawGeom     drawGeom (agenda, dest.Is3d());
    CaptureGraphicsContext      context (vp, geometry, graphics, preserveClosedCurves, drawGeom, fence); 

    return context.CaptureDgnModel (source, includeRefs);
    }

/*---------------------------------------------------------------------------------**//**
* @remarks srcDgnModel must be currently displayed in vp
* @bsimethod                                                    Ray.Bentley     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt   CaptureGraphicsToElement::CaptureDisplayPath (ElementAgendaR agenda, DgnModelR dest, DisplayPathCR displayPath, ViewportP vp, DropGeometryCR geometry, DropGraphicsCR graphics, bool preserveClosedCurves)
    {
    CaptureGraphicsDrawGeom     drawGeom (agenda, dest.Is3d());
    CaptureGraphicsContext      context (vp, geometry, graphics, preserveClosedCurves, drawGeom, NULL); 

    return context.CaptureDisplayPath (displayPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DraftingElementSchema::ToDroppedElements (ElementHandleCR eh, ElementAgendaR agenda, DropGraphicsCR graphics)
    {
    return (SUCCESS == DropToElementDrawGeom::DoDrop (eh, agenda, DropGeometry (), graphics) ? SUCCESS : ERROR);
    }

/*--------------------------------------------------------------------------------------+
| ANDROID_NONPORT_WIP 
|
| The functions below were formerly inlined in DgnHandlersDLLInlines.h.  Something about
| this class (and only this class) or its method of inlining caused problems for 
| optimized Android builds.  For now, we've taken the quick fix of never inlining these
| methods.
+---------------+---------------+---------------+---------------+---------------+------*/
DropGeometryR         DropToElementDrawGeom::GetDropGeometry () {return m_geometry;}
DropGraphicsR         DropToElementDrawGeom::GetDropGraphics () {return m_graphics;}
bool                  DropToElementDrawGeom::_DoTextGeometry () const {return (0 != (m_geometry.GetOptions() & DropGeometry::OPTION_Text)) || m_inSymbolDraw;}
bool                  DropToElementDrawGeom::_DoSymbolGeometry () const {return true;} // Shouldn't see pattern/linestyle symbols unless option is set to drop them...
bool                  DropToElementDrawGeom::_ProcessAsBody (bool isCurved) const {return m_geometry.GetSolidsOptions() != DropGeometry::SOLID_Wireframe;}
bool                  DropToElementDrawGeom::_DoClipping () const {return true;}
void                  DropToElementDrawGeom::SetElementNonSnappable (bool nonsnappable) {m_nonsnappable = nonsnappable;}
void                  DropToElementDrawGeom::SetPreserveClosedCurves (bool preserveClosed) {m_preserveClosedCurves = preserveClosed;}
void                  DropToElementDrawGeom::OnElementCreated (EditElementHandleR eeh) {_OnElementCreated (eeh);}
void                  DropToElementDrawGeom::ProcessAreaPattern (ElementHandleCR thisElm, DgnPlatform::ViewContext::ClipStencil& boundary, DgnPlatform::ViewContext::PatternParamSource& source) {_ProcessAreaPattern (thisElm, boundary, source);}
