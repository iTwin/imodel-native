/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextHandlers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

UShort  const ALONG_TEXT_DEPENDENCY_APP_ID      = DEPENDENCYAPPID_Text;
UShort  const ALONG_TEXT_DEPENDENCY_APP_VALUE   = TEXTDEPENDENCYAPPVALUE_AlongElement;
UInt32  const ALONG_TEXT_LINKAGE_SIZE           = sizeof (DependencyLinkage) + sizeof (AlongTextLinkageData::CustomDependencyData);
UInt16  const ALONG_TEXT_MAX_NUMBER_OF_ROOTS    = 1;

#define ISNOTEQUAL(x, y) (fabs (x - y) > mgds_fc_epsilon)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Helper Classes --------------------------------------------------------------------------------------------------------------- Helper Classes --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=======================================================================================
// @bsiclass                                                    Deepak.Malkan   09/07
//=======================================================================================
struct StrokeTextNodeNumber : IStrokeForCache
    {
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Keith.Bentley   09/03
    //---------------------------------------------------------------------------------------
    void DrawTextNodeCross (ViewContextR context, IDrawGeomR output, DPoint3dCP origin, DPoint2dCP scale, RotMatrixCP rMatrix)
        {
        Transform trans;
        trans.initFrom (rMatrix, origin);
        context.PushTransform (trans);

        DPoint3d vert[2];
        DPoint3d horz[2];

        memset (vert, 0, sizeof(vert));
        memset (horz, 0, sizeof(horz));

        horz[0].x = -scale->x * 0.5;
        vert[0].y = -scale->y * 0.5;
        vert[1].y =  scale->y * 0.5;
        horz[1].x =  scale->x * 0.5;

        output.DrawLineString3d (2, vert, NULL);
        output.DrawLineString3d (2, horz, NULL);
        context.PopTransformClip ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Keith.Bentley   09/03
    //---------------------------------------------------------------------------------------
    virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
        {
        ElementHandleCR elIter = *dh.GetElementHandleCP();
        DPoint3d origin;        if (SUCCESS != TextNodeHandler::GetUserOrigin (elIter, origin)) return;
        RotMatrix orientation;  if (SUCCESS != TextNodeHandler::GetOrientation (elIter, orientation)) return;
        DPoint2d textSize;      if (SUCCESS != TextNodeHandler::GetFontSize (elIter, textSize)) return;
        TextParamWide textParams; if (SUCCESS != TextNodeHandler::GetTextParams (elIter, textParams)) return;
        UInt32 nodeNumber;      if (SUCCESS != TextNodeHandler::GetNodeNumber (elIter, nodeNumber)) return;
        
        IDrawGeomR output = context.GetIDrawGeom();
        DrawTextNodeCross (context, output, &origin, &textSize, &orientation);

        WChar wString[32];
        BeStringUtilities::Snwprintf (wString, _countof(wString), L"%u", nodeNumber);

        DgnModelP modelRef = elIter.GetDgnModelP();
        
        TextString nodeNumString (wString, &origin, &orientation, TextStringProperties (textParams, textSize, modelRef->Is3d (), modelRef->GetDgnProject ()));
        nodeNumString.SetOriginFromUserOrigin (origin);

        double priority = context.GetDisplayPriority ();
        output.DrawTextString (nodeNumString, nodeNumString.Is3d () ? NULL : &priority);
        }
    }; // StrokeTextNodeNumber

/*=================================================================================**//**
* Text Node Element Stroker
* @bsiclass                                                     Deepak.Malkan   09/07
+===============+===============+===============+===============+===============+======*/
struct StrokeTextNodeElm : IAnnotationStrokeForCache
    {
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Deepak.Malkan                   09/07
    //---------------------------------------------------------------------------------------
    void StrokeScaledForCache (ElementHandleCR eh, ViewContextR context, AnnotationDisplayParameters const& params) override
        {
        // Since we are now allowing line-break length to be scaled as well (prior we were keeping line-break length
        //  the same and performing layout with scaled text), we can use a simple geometric scale for text elements.
        //  Note that we should scale about the user origin of the text node, so do not ask each child text element
        //  to draw, because they will scale about their user origin instead.
        
        DPoint3d originForScale;
        if (DisplayHandler::Is3dElem (eh.GetElementCP ()))
            {
            originForScale = eh.GetElementCP ()->ToText_node_3d().origin;
            }
        else
            {
            originForScale.x = eh.GetElementCP ()->ToText_node_2d().origin.x;
            originForScale.y = eh.GetElementCP ()->ToText_node_2d().origin.y;
            originForScale.z = 0.0;
            }
            
        ViewContext::ContextMark mark (context, eh);

        PushTransformToRescale (context, originForScale, params, true);    

        for (ChildElemIter childElement (eh, ExposeChildrenReason::Count); childElement.IsValid (); childElement = childElement.ToNext ())
            {
            BeAssert (TEXT_ELM == childElement.GetElementCP ()->GetLegacyType());
            if (TEXT_ELM == childElement.GetElementCP ()->GetLegacyType())
                {
                TextString textString;
                ((TextElemHandlerR)childElement.GetHandler ()).InitTextString (childElement, textString);
                context.DrawTextString (textString);
                }
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Keith.Bentley   09/03
    //---------------------------------------------------------------------------------------
    virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
        {
        ElementHandleCR eh = *dh.GetElementHandleCP();
        ((TextNodeHandler*)&eh.GetHandler())->VisitNodeChildren (eh, context);
        }
    
    }; // StrokeTextNodeElm

/*=================================================================================**//**
* @bsiclass                                                 
    Deepak.Malkan   09/07
+===============+===============+===============+===============+===============+======*/
struct StrokeTextElm : IAnnotationStrokeForCache
    {
    //---------------------------------------------------------------------------------------
    // This is used to re-scale for annotative text through references.
    // @bsimethod                                   Deepak.Malkan                   09/07
    //---------------------------------------------------------------------------------------
    void StrokeScaledForCache (ElementHandleCR eh, ViewContextR context, AnnotationDisplayParameters const& params) override
        {
        // Since we are now allowing line-break length to be scaled as well (prior we were keeping line-break length
        //  the same and performing layout with scaled text), we can use a simple geometric scale for text elements.
        //  Note that we should scale about the user origin.
        
        TextString          textString;
        TextElemHandlerP    handler     = dynamic_cast<TextElemHandlerP>(&eh.GetHandler ());
        
        if ((NULL == handler) || (SUCCESS != handler->InitTextString (eh, textString)))
            {
            BeAssert (false);
            return;
            }
        
        DPoint3d userOrigin;
        textString.ComputeUserOrigin (userOrigin);
        
        ViewContext::ContextMark mark (context, eh);

        PushTransformToRescale (context, userOrigin, params, true);

        context.DrawTextString (textString);
        }

    //---------------------------------------------------------------------------------------
    // This is used to draw text through the model it resides in (vs. through a reference).
    // @bsimethod                                                   Keith.Bentley   09/03
    //---------------------------------------------------------------------------------------
    virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
        {
        ElementHandleCR eh = *dh.GetElementHandleCP();
        // The V8 text element format bakes annotation scale into the element's properties.
        // Starting in Vancouver, all in-memory text data representations are normalized (e.g. scale is un-baked when encountered).
        // Thus, even through the file it was placed in, text needs to have its scale applied as a transform.
        
        TextString textString;
        if (SUCCESS != ((TextElemHandler&)eh.GetHandler ()).InitTextString (eh, textString))
            { BeAssert (false); return; }
        
        TextParamWide textParams;
        if (SUCCESS != TextElemHandler::GetTextParams (eh, textParams))
            { BeAssert (false); return; }

        if (!textParams.exFlags.annotationScale)
            {
            context.DrawTextString (textString);
            return;
            }
        
        // Similar to above, except PushTransformToRescale calls ReplaceAspectRatioSkewWithTranslationEffect, which I'm not convinced we want here.
        ViewContext::ContextMark mark (context, eh);
        
    #ifdef WIP_VANCOUVER_MERGE // text - the textstring is already scaled to match the annotation scale
        Transform annotationTransform = Transform::FromFixedPointAndScaleFactors(textString.GetOrigin (), textParams.annotationScale, textParams.annotationScale, textParams.annotationScale);
        context.PushTransform(annotationTransform);
    #endif

        context.DrawTextString (textString);
        }

    }; // StrokeTextElm

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Helper Functions ----------------------------------------------------------------------------------------------------------- Helper Functions --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
static bool shouldSetOverridesFromStyle (PropertyContextR context)
    {
    // Only allowed for edit (not query).
    if (NULL == context.GetIEditPropertiesP ())
        return false;
    
    // Only allowed if for change (strict remapping should not affect overrides).
    if (EditPropertyPurpose::Change != context.GetIEditPropertiesP ()->_GetEditPropertiesPurpose ())
        return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    06/06
//---------------------------------------------------------------------------------------
static PropsCallbackFlags getPropFlags (PropsCallbackFlags flags, bool usedID, bool displayedID)
    {
    if (!usedID)
        flags = (PropsCallbackFlags) (flags | PROPSCALLBACK_FLAGS_ElementIgnoresID);

    if (!displayedID)
        flags = (PropsCallbackFlags) (flags | PROPSCALLBACK_FLAGS_UndisplayedID);

    return flags;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BrienBastings   05/03
//---------------------------------------------------------------------------------------
static void pushTextNodeTransform (ElementHandleCR eh, ViewContextR context)
    {
    DgnElementCP elP = eh.GetElementCP();

    if (elP && elP->IsViewIndependent())
        {
        // get the origin of the text node element (not user origin)
        DPoint3d origin;
        TextNodeHandler::GetUserOrigin (eh, origin);

        // push the VI transform
        context.PushViewIndependentOrigin (&origin);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  02/04
//---------------------------------------------------------------------------------------
bool hasMirrorTransform (ElementHandleCR eh, TransformInfoCR transform)
    {
    if ((transform.GetOptions () & TRANSFORM_OPTIONS_DisableMirrorCharacters) && NULL != transform.GetMirrorPlane ())
        {
        double      determ;
        RotMatrix   rMatrix;

        rMatrix.InitFrom(*( transform.GetTransform ()));
        determ = rMatrix.Determinant ();

        if (determ < 0.0)
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   SunandSandurkar 06/06
//---------------------------------------------------------------------------------------
bool canScaleAnnotationText (ElementHandleCR eh, TransformInfoCR transform, IAnnotationHandler const* ah)
    {
    //  It's OK to scale any kind of text as long as the caller is not asking for the sizeMatchSource option
    if (!(transform.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource))
        return true;
    
    //  It's never OK to scale annotation text. 
    return !ah->GetAnnotationScale (NULL, eh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  07/06
//---------------------------------------------------------------------------------------
static bool canUseTransformGraphics (ElementHandleCR eh, TransformInfoCR transform, IAnnotationHandler const* ah)
    {
    // If text should not be mirrored, treat it as a special case
    if (hasMirrorTransform (eh, transform))
        return false;

    // Rotation/Mirror being applied to view independent text...
    if (eh.GetElementCP ()->IsViewIndependent() && (transform.GetTransform ()->isRotateAroundLine (NULL, NULL, NULL) || transform.GetTransform ()->isMirrorAboutPlane (NULL, NULL)))
        return false;

    // If annotation text size should not be scaled, treat it as a special case. It ultimately calls the
    // regular transform code, but we return false here in order to get the dynamics right.
    if (!canScaleAnnotationText (eh, transform, ah))
        return false;

    // This special rotation mode rotates the origin without modifying the underlying text orientation (and thus cannot be represented as a simple geometric transform).
    if (TRANSFORM_OPTIONS_DisableRotateCharacters == (TRANSFORM_OPTIONS_DisableRotateCharacters & transform.GetOptions ()))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BrienBastings   05/03
//---------------------------------------------------------------------------------------
static void pushVITextTransform (ElementHandleCR eh, ViewContextR context)
    {
    DgnElementCP elP = eh.GetElementCP();

    if (!elP->IsViewIndependent())
        return;

    DPoint3d origin;

    // get the origin of the text element (not user origin)
    TextElemHandler::GetElementOrigin (eh, origin);

    // push the VI transform
    context.PushViewIndependentOrigin (&origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JohnFerguson    04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void textStyle_validateStyle(bool* anyChangeOut, LegacyTextStyleP styleIn)
    {
    bool    anyChange = false;
    
    if (NULL == styleIn)
        return;

    // TR 86547 - the existence of text width/height < 0.0 is an artifact of 2D mirroring and
    // invalid style definitions and the text system doesn't support negative sizes.

    if (styleIn->height <= 0)
        {
        styleIn->height = - styleIn->height;
        anyChange = true;
        }

    if (styleIn->width <= 0)
        {
        styleIn->width = fabs (styleIn->width);
        anyChange = true;
        }

    if (!IN_RANGE(styleIn->fontNo, 0, 65535))
        {
        BeAssert(0);
        styleIn->fontNo = 3;
        anyChange = true;
        }

    #define CHECKRANGE(field,min,max)  if (!IN_RANGE (styleIn->field, min, max)) { styleIn->field = 0.0; anyChange = true; }

    CHECKRANGE (width,              0.0,        FLT_MAX)
    CHECKRANGE (height,             0.0,        FLT_MAX)
    CHECKRANGE (lineSpacing,        0.0,        FLT_MAX)
    CHECKRANGE (interCharSpacing,   0.0,        FLT_MAX)
    CHECKRANGE (underlineOffset,    0.0,        FLT_MAX)
    CHECKRANGE (overlineOffset,     0.0,        FLT_MAX)
    CHECKRANGE (lineOffset.x,       0.0,        FLT_MAX)
    CHECKRANGE (slant,              -1.0E+8,    1.0E+8)
    
    // We used to check for negative line offset as such:
    // CHECKRANGE (lineOffset.y,       0.0,        FLT_MAX)
    // However, we do diagonal fractions with negative line offsets, so we cannot indiscriminantly clamp it.

    if (NULL != anyChangeOut && anyChange)
        *anyChangeOut = anyChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int textstyle_getTextStyleFromTextParamWide (LegacyTextStyleP pTextStyle, DPoint2dP pScale, TextParamWideP pWide, int lineLength)
    {
    memset(pTextStyle, 0, sizeof (LegacyTextStyle));

    pTextStyle->lineLength  = static_cast<UInt16>(lineLength);
    pTextStyle->width       = pScale->x;
    pTextStyle->height      = pScale->y;

    pTextStyle->fontNo                      = pWide->font;
    pTextStyle->just                        = static_cast<UInt16>(pWide->just);
    pTextStyle->nodeJust                    = static_cast<UInt16>(pWide->just);
    pTextStyle->shxBigFont                  = pWide->shxBigFont;
    pTextStyle->backgroundFillColor         = pWide->backgroundFillColor;
    pTextStyle->lineSpacing                 = pWide->lineSpacing;
    pTextStyle->slant                       = pWide->slant;
    pTextStyle->interCharSpacing            = pWide->characterSpacing;
    pTextStyle->underlineOffset             = pWide->underlineSpacing;
    pTextStyle->backgroundBorder            = pWide->backgroundBorder;
    pTextStyle->backgroundStyle.color       = pWide->backgroundColor;
    pTextStyle->backgroundStyle.style       = pWide->backgroundStyle;
    pTextStyle->backgroundStyle.weight      = pWide->backgroundWeight;
    pTextStyle->underlineStyle.color        = pWide->underlineColor;
    pTextStyle->underlineStyle.style        = pWide->underlineStyle;
    pTextStyle->underlineStyle.weight       = pWide->underlineWeight;
    pTextStyle->overlineStyle.color         = pWide->overlineColor;
    pTextStyle->overlineStyle.style         = pWide->overlineStyle;
    pTextStyle->overlineStyle.weight        = pWide->overlineWeight;
    pTextStyle->overlineOffset              = pWide->overlineSpacing;
    pTextStyle->flags.fixedSpacing          = pWide->flags.fixedWidthSpacing;

    pTextStyle->flags.underline             = pWide->flags.underline;
    pTextStyle->flags.italics               = pWide->flags.slant;
    pTextStyle->flags.background            = pWide->flags.bgColor;
    pTextStyle->flags.subscript             = pWide->flags.subscript;
    pTextStyle->flags.superscript           = pWide->flags.superscript;
    pTextStyle->flags.overline              = pWide->exFlags.overline;
    pTextStyle->flags.bold                  = pWide->exFlags.bold;
    pTextStyle->flags.underlineStyle        = pWide->exFlags.underlineStyle;
    pTextStyle->flags.overlineStyle         = pWide->exFlags.overlineStyle;
    pTextStyle->flags.color                 = pWide->exFlags.color;
    pTextStyle->flags.fullJustification     = pWide->exFlags.fullJustification;
    pTextStyle->flags.color                 = pWide->exFlags.color;
    pTextStyle->flags.acadLineSpacingType   = pWide->exFlags.acadLineSpacingType;
    pTextStyle->flags.acadInterCharSpacing  = pWide->exFlags.acadInterCharSpacing;
    pTextStyle->color                       = pWide->color;
    if (pWide->flags.offset)
        pTextStyle->lineOffset               = pWide->lineOffset;

    pTextStyle->textDirection |= pWide->exFlags.backwards ? TXTDIR_BACKWARDS : 0;
    pTextStyle->textDirection |= pWide->exFlags.upsidedown ? TXTDIR_UPSIDEDOWN : 0;
    pTextStyle->textDirection |= pWide->flags.vertical ? TXTDIR_VERTICAL : 0;
    pTextStyle->textDirection |= pWide->flags.rightToLeft_deprecated ? TXTDIR_RIGHTLEFT : 0;

    if (pWide->exFlags.annotationScale)
        pTextStyle->Scale(1.0 / pWide->annotationScale);

    textStyle_validateStyle (NULL, pTextStyle);

    return  SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Abeesh.Basheer  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool areEqualWithinTolerance (double in1, double in2)
    {
    static const double precision = mgds_fc_epsilon;
    return (std::fabs (in1 - in2) < precision);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JohnFerguson    11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt textstyle_compareTextStyleValues(LegacyTextStyleOverrideFlagsP pFlags, LegacyTextStyleCP pTextStyle1, LegacyTextStyleCP pTextStyle2, bool userelativeValues)
    {
    /* individually compare items in the text style against the tcb style */

    if (pTextStyle1->fontNo != pTextStyle2->fontNo)
        pFlags->fontNo = 1;

    if (pTextStyle1->shxBigFont != pTextStyle2->shxBigFont)
        pFlags->shxBigFont = 1;

    if (!areEqualWithinTolerance (pTextStyle1->width, pTextStyle2->width))
        pFlags->width = 1;

    if (!areEqualWithinTolerance (pTextStyle1->height, pTextStyle2->height))
        pFlags->height = 1;

    if (!areEqualWithinTolerance (pTextStyle1->widthFactor, pTextStyle2->widthFactor))
        pFlags->widthFactor = 1;

    if (!areEqualWithinTolerance (pTextStyle1->slant, pTextStyle2->slant))
        pFlags->slant = 1;

    double heightfactor = 1.0;
    if (userelativeValues)
        {
        if (areEqualWithinTolerance (pTextStyle1->height, 0) || areEqualWithinTolerance (pTextStyle2->height, 0))
            heightfactor = 1.0;
        else
            heightfactor = pTextStyle1->height / pTextStyle2->height;
        }

    //For line spacing we have this additonal check for linespacing type
        {
        double lheightfactor = (DgnLineSpacingType::AtLeast == static_cast<DgnLineSpacingType>(pTextStyle1->flags.acadLineSpacingType)) ? 1.0 : heightfactor;
        if (!areEqualWithinTolerance (pTextStyle1->lineSpacing, pTextStyle2->lineSpacing * lheightfactor))
            pFlags->linespacing = 1;
        }

    if (!areEqualWithinTolerance (pTextStyle1->interCharSpacing, pTextStyle2->interCharSpacing * heightfactor))
        pFlags->interCharSpacing = 1;

    if (!areEqualWithinTolerance (pTextStyle1->underlineOffset, pTextStyle2->underlineOffset * heightfactor))
        pFlags->underlineOffset = 1;

    if (!areEqualWithinTolerance (pTextStyle1->overlineOffset, pTextStyle2->overlineOffset * heightfactor))
        pFlags->overlineOffset = 1;

    if (!areEqualWithinTolerance (pTextStyle1->lineOffset.x, pTextStyle2->lineOffset.x * heightfactor) ||
                                                !areEqualWithinTolerance (pTextStyle1->lineOffset.y, pTextStyle2->lineOffset.y * heightfactor))
        pFlags->lineOffset = 1;

    if (!areEqualWithinTolerance (pTextStyle1->backgroundBorder.x, pTextStyle2->backgroundBorder.x * heightfactor) ||
                                               !areEqualWithinTolerance (pTextStyle1->backgroundBorder.y, pTextStyle2->backgroundBorder.y * heightfactor))
        pFlags->backgroundborder = 1;

    if (pTextStyle1->just != pTextStyle2->just)
        pFlags->just = 1;

    if (pTextStyle1->nodeJust != pTextStyle2->nodeJust)
        pFlags->nodeJust = 1;

    if (pTextStyle1->lineLength != pTextStyle2->lineLength)
        pFlags->lineLength = 1;

    if (pTextStyle1->textDirection != pTextStyle2->textDirection)
        {
        if ((pTextStyle1->textDirection & TXTDIR_VERTICAL) != (pTextStyle2->textDirection & TXTDIR_VERTICAL))
            pFlags->direction = 1;
        if ((pTextStyle1->textDirection & TXTDIR_BACKWARDS) != (pTextStyle2->textDirection & TXTDIR_BACKWARDS))
            pFlags->backwards = 1;
        if ((pTextStyle1->textDirection & TXTDIR_UPSIDEDOWN) != (pTextStyle2->textDirection & TXTDIR_UPSIDEDOWN))
            pFlags->upsidedown = 1;
        }

    if (pTextStyle1->backgroundStyle.color != pTextStyle2->backgroundStyle.color)
        pFlags->backgroundcolor= 1;

    if (pTextStyle1->backgroundStyle.style != pTextStyle2->backgroundStyle.style)
        pFlags->backgroundstyle = 1;

    if (pTextStyle1->backgroundStyle.weight != pTextStyle2->backgroundStyle.weight)
        pFlags->backgroundweight = 1;

    if (pTextStyle1->backgroundFillColor != pTextStyle2->backgroundFillColor)
        pFlags->backgroundfillcolor = 1;

    if (pTextStyle1->underlineStyle.color != pTextStyle2->underlineStyle.color)
        pFlags->underlinecolor = 1;

    if (pTextStyle1->underlineStyle.style != pTextStyle2->underlineStyle.style)
        pFlags->underlinestyle = 1;

    if (pTextStyle1->underlineStyle.weight != pTextStyle2->underlineStyle.weight)
        pFlags->underlineweight = 1;

    if (pTextStyle1->overlineStyle.color != pTextStyle2->overlineStyle.color)
        pFlags->overlinecolor = 1;

    if (pTextStyle1->overlineStyle.style != pTextStyle2->overlineStyle.style)
        pFlags->overlinestyle = 1;

    if (pTextStyle1->overlineStyle.weight != pTextStyle2->overlineStyle.weight)
        pFlags->overlineweight = 1;

    if (pTextStyle1->flags.underline != pTextStyle2->flags.underline)
        pFlags->underline = 1;

    if (pTextStyle1->flags.overline != pTextStyle2->flags.overline)
        pFlags->overline = 1;

    if (pTextStyle1->flags.italics != pTextStyle2->flags.italics)
        pFlags->italics = 1;

    if (pTextStyle1->flags.bold != pTextStyle2->flags.bold)
        pFlags->bold = 1;

    if (pTextStyle1->flags.superscript != pTextStyle2->flags.superscript)
        pFlags->superscript = 1;

    if (pTextStyle1->flags.subscript != pTextStyle2->flags.subscript)
        pFlags->subscript = 1;

    if (pTextStyle1->flags.background != pTextStyle2->flags.background)
        pFlags->background = 1;

    if (pTextStyle1->flags.overlineStyle != pTextStyle2->flags.overlineStyle)
        pFlags->overlinestyleflag = 1;

    if (pTextStyle1->flags.underlineStyle != pTextStyle2->flags.underlineStyle)
        pFlags->underlinestyleflag = 1;

    if (pTextStyle1->flags.fixedSpacing != pTextStyle2->flags.fixedSpacing)
        pFlags->fixedSpacing = 1;

    if (pTextStyle1->flags.fractions != pTextStyle2->flags.fractions)
        pFlags->fractions = 1;

    if (pTextStyle1->flags.color != pTextStyle2->flags.color)
        pFlags->colorFlag = 1;

    if (pTextStyle1->color != pTextStyle2->color)
        pFlags->color = 1;

    if (pTextStyle1->flags.fullJustification != pTextStyle2->flags.fullJustification)
        pFlags->fullJustification = 1;

    if (pTextStyle1->flags.acadLineSpacingType != pTextStyle2->flags.acadLineSpacingType)
        pFlags->acadLineSpacingType = 1;

    if (pTextStyle1->flags.acadInterCharSpacing != pTextStyle2->flags.acadInterCharSpacing)
        pFlags->acadInterCharSpacing = 1;

    return  SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     09/08
+---------------+---------------+---------------+---------------+---------------+------*/
    static void correctHiddenOverrides(LegacyTextStyleOverrideFlagsR overrideFlags, LegacyTextStyleCR lhsTextStyle, LegacyTextStyleCR rhsTextStyle)
    {
    // Overrides should never be set on properties who's control flags are both (LHS and RHS) false.

    // Color (color flag)
    if (!lhsTextStyle.flags.color && !rhsTextStyle.flags.color)
        overrideFlags.color = false;


    // Underline offset (underline flag)
    if (!lhsTextStyle.flags.underline && !rhsTextStyle.flags.underline)
        overrideFlags.underlineOffset = false;

    bool underlineStyleCheckBit = ((!lhsTextStyle.flags.underline && !rhsTextStyle.flags.underline) || (!lhsTextStyle.flags.underlineStyle && !rhsTextStyle.flags.underlineStyle));

    // Underline color (underline flag + use style flag)
    if (underlineStyleCheckBit)
        overrideFlags.underlinecolor = false;

    // Underline style (underline flag + use style flag)
    if (underlineStyleCheckBit)
        overrideFlags.underlinestyle = false;

    // Underline weight (underline flag + use style flag)
    if (underlineStyleCheckBit)
        overrideFlags.underlineweight = false;


    // Overline offset (overline flag)
    if (!lhsTextStyle.flags.overline && !rhsTextStyle.flags.overline)
        overrideFlags.overlineOffset = false;

    bool overlineStyleCheckBit = ((!lhsTextStyle.flags.overline && !rhsTextStyle.flags.overline) || (!lhsTextStyle.flags.overlineStyle && !rhsTextStyle.flags.overlineStyle));

    // Overline color (overline flag + use style flag)
    if (overlineStyleCheckBit)
        overrideFlags.overlinecolor = false;

    // Overline style (overline flag + use style flag)
    if (overlineStyleCheckBit)
        overrideFlags.overlinestyle = false;

    // Overline weight (overline flag + use style flag)
    if (overlineStyleCheckBit)
        overrideFlags.overlineweight = false;


    bool backgroundStyleCheckBit = (!lhsTextStyle.flags.background && !rhsTextStyle.flags.background);

    // Background color (background flag)
    if (backgroundStyleCheckBit)
        overrideFlags.backgroundfillcolor = false;

    // Background border color (background flag)
    if (backgroundStyleCheckBit)
        overrideFlags.backgroundcolor = false;

    // Background border style (background flag)
    if (backgroundStyleCheckBit)
        overrideFlags.backgroundstyle = false;

    // Background border weight (background flag)
    if (backgroundStyleCheckBit)
        overrideFlags.backgroundweight = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      11/00
+---------------+---------------+---------------+---------------+---------------+------*/
static int textstyle_getOverrideFlagsForCompareWithStyle
(
LegacyTextStyleOverrideFlagsP pFlags,
UInt32                  textStyleId,
LegacyTextStyleP              pTextStyle,
DgnModelP            srcDgnModel,
DgnModelP            dstDgnModel
)
    {
    LegacyTextStyle textStyle;
    memset(pFlags, 0, sizeof (LegacyTextStyleOverrideFlags));

    DgnTextStylePtr fileStyle = srcDgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(textStyleId));
    if (!fileStyle.IsValid())
        return ERROR;

    textStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*fileStyle);
    
    if (0 == memcmp (&textStyle, pTextStyle, sizeof (textStyle)))
        return  SUCCESS;

    textstyle_compareTextStyleValues (pFlags, &textStyle, pTextStyle, false);

    // The point of this method is to compute text element overrides.
    // Overrides should never be set on properties who's control flags are both (LHS and RHS) false.
    correctHiddenOverrides (*pFlags, textStyle, *pTextStyle);

    // The 'fractions' value is not stored on text elements, therefore it should never have an override for it.
    pFlags->fractions = false;

    return  SUCCESS;
    }

//---------------------------------------------------------------------------------------
//! Based on the text style remapping action, applies the text style to the text parameters provided.
// @bsimethod                                                   Jeff.Marker     01/08
//---------------------------------------------------------------------------------------
static void    processTextStyleRemapping
(
EachTextStyleArg&   textStyleArg,
DPoint2dR           scale,
TextParamWideR      textParam,
TextBlockCR         textBlock,
UInt32              textStyleID,
PropertyContextR    context,
UInt32*             lineLength,
bool                forTextNode
)
    {
    switch (textStyleArg.GetRemappingAction ())
        {
        case StyleParamsRemapping::Override:
            {
            LegacyTextStyle               elementTextStyle;
            LegacyTextStyleOverrideFlags  newOverrides;
            
            // use the style id of the one with the same name in the file and
            // set the override flags on the text to match the changes in the file 
            textstyle_getTextStyleFromTextParamWide (&elementTextStyle, &scale, &textParam, textBlock.GetProperties ().GetMaxCharactersPerLine ());

            if (SUCCESS == textstyle_getOverrideFlagsForCompareWithStyle (&newOverrides, textStyleID, &elementTextStyle, context.GetDestinationDgnModel (), context.GetDestinationDgnModel ()))
                textParam.overridesFromStyle.ComputeLogicalOr (textParam.overridesFromStyle, newOverrides);

            if (textParam.overridesFromStyle.AreAnyFlagsSet ())
                textParam.exFlags.styleOverrides = true;

            break;
            }
        
        case StyleParamsRemapping::ApplyStyle:  //  Apply the style's parameters to the element.
            {
            DgnTextStylePtr textStyle = context.GetDestinationDgnModel()->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(textStyleID));
            if (textStyle.IsValid ())
                textParam = DgnTextStylePersistence::Legacy::ToTextParamWide(*textStyle, &scale, lineLength);
            
            break;
            }
        }
    }

#ifdef UNUSED_FUNCTION
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/09
//---------------------------------------------------------------------------------------
static void removeAllLinkagesWithIDFromEeh (EditElementHandleR eeh, UInt16 linkageId)
    {
    ElementLinkageIterator iter;
    
    while (eeh.EndElementLinkages () != (iter = eeh.BeginElementLinkages (linkageId)))
        eeh.RemoveElementLinkage (iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/09
//---------------------------------------------------------------------------------------
static LinkageHeaderCP getFirstLinkageWithIDFromEh (ElementHandleCR eh, UInt16 linkageId)
    {
    ConstElementLinkageIterator iter = eh.BeginElementLinkages (linkageId);
    if (eh.EndElementLinkages () == iter)
        return NULL;
    
    return &(*iter);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
static void copyNonTextLinkages (ElementHandleCR sourceEh, EditElementHandleR destinationEeh)
    {
    // Note that we are intentionally keeping the along-text linkage.
    //  If we cannot resolve the target element in this session, we will not attempt to re-write the along-text
    //  data, but we want to keep it around in case a future session can resolve it.
    
    for (ConstElementLinkageIterator sourceIter = sourceEh.BeginElementLinkages (0); sourceEh.EndElementLinkages () != sourceIter; ++sourceIter)
        {
        LinkageHeaderCR lHeader = *sourceIter;
        
        if (TextFormattingLinkage::GetLinkageID ()  == lHeader.primaryID    ||
            TextRenderingLinkage::GetLinkageID ()   == lHeader.primaryID    ||
            TextAnnotationLinkage::GetLinkageID ()  == lHeader.primaryID    ||
            TextIndentationLinkage::GetLinkageID () == lHeader.primaryID    ||
            ((LINKAGEID_BitMask == lHeader.primaryID) && (BITMASK_LINKAGE_KEY_TextWhiteSpace == ((MSBitMaskLinkage*)&lHeader)->data.linkageKey)))
            {
            continue;
            }
        
        destinationEeh.AppendElementLinkage (NULL, lHeader, sourceIter.GetData ());
        }
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Handler Get/Set Helpers --------------------------------------------------------------------------------------------- Handler Get/Set Helpers --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const double MULT_TO_TILE = (6.0 / 1000.0);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Keith.Bentley   09/03
//---------------------------------------------------------------------------------------
double TextHandlerBase::ConvertMultToScale (double mult)
    {
    return (mult * MULT_TO_TILE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Keith.Bentley   09/03
//---------------------------------------------------------------------------------------
double TextHandlerBase::ConvertScaleToMult (double scale)
    {
    return (scale / MULT_TO_TILE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void TextHandlerBase::FillTextParamsFromLinkages (ElementHandleCR eh, TextParamWideR textParams)
    {
    TextFormattingLinkage::FillTextParamsFromLinkage (eh, textParams);
    TextAnnotationLinkage::FillTextParamsFromLinkage (eh, textParams);
    TextRenderingLinkage::FillTextParamsFromLinkage (eh, textParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
void TextHandlerBase::AppendLinkagesFromTextParams (EditElementHandleR eeh, TextParamWideCR textParams)
    {
    TextFormattingLinkage::AppendLinkageFromTextParams (eeh, textParams);
    TextAnnotationLinkage::AppendLinkageFromTextParams (eeh, textParams);
    TextRenderingLinkage::AppendLinkageFromTextParams (eeh, textParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextHandlerBase::AppendLinkagesFromTextParams (DgnElementR el, TextParamWideCR textParams)
    {
    TextFormattingLinkage::AppendLinkageFromTextParams (el, textParams);
    TextAnnotationLinkage::AppendLinkageFromTextParams (el, textParams);
    TextRenderingLinkage::AppendLinkageFromTextParams (el, textParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2009
//---------------------------------------------------------------------------------------
void TextHandlerBase::RemoveTextParamLinkages (EditElementHandleR eeh)
    {
    TextFormattingLinkage::RemoveLinkages (eeh);
    TextRenderingLinkage::RemoveLinkages (eeh);
    TextAnnotationLinkage::RemoveLinkages (eeh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextHandlerBase::RemoveTextParamLinkages (DgnElementR el)
    {
    TextFormattingLinkage::RemoveLinkages (el);
    TextRenderingLinkage::RemoveLinkages (el);
    TextAnnotationLinkage::RemoveLinkages (el);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextHandlerBase ----------------------------------------------------------------------------------------------------------- TextHandlerBase --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                   Abeesh.Basheer                  05/2008
//---------------------------------------------------------------------------------------
bool TextHandlerBase::RemapRunProperties (RunPropertiesR runProperties, PropertyContextR context, TextBlockCR textBlock, TextParamWide const & textParams)
    {
    bool hasModifiedProperties = false;

    //-- Font -----------------------------------------------------------------------------
    DgnFontCP oldFont = &runProperties.GetFont ();
    if (NULL != oldFont)
        {
        UInt32 fontNumber;
        if (SUCCESS == context.GetSourceDgnModel()->GetDgnProject().Fonts().FindFontNumber (&fontNumber, *oldFont))
            {
            if (context.DoFontCallback (&fontNumber, EachFontArg (fontNumber, PROPSCALLBACK_FLAGS_NoFlagsSet, context)))
                {
                hasModifiedProperties = true;
            
                DgnFontCP remappedFont = context.GetDestinationDgnModel()->GetDgnProject().Fonts().FindFont (fontNumber);
                if (NULL != remappedFont)
                    runProperties.SetFont (*remappedFont);
                }
            }
        }
    
    DgnFontCP oldShxBigFont = runProperties.GetShxBigFontCP ();
    if (NULL != oldShxBigFont)
        {
        UInt32 shxBigFontNumber;
        if (SUCCESS == context.GetSourceDgnModel()->GetDgnProject().Fonts().FindFontNumber (&shxBigFontNumber, *oldShxBigFont))
            {
            if (context.DoFontCallback (&shxBigFontNumber, EachFontArg (shxBigFontNumber, PROPSCALLBACK_FLAGS_NoFlagsSet, context)))
                {
                hasModifiedProperties = true;
            
                DgnFontCP remappedFont = context.GetDestinationDgnModel()->GetDgnProject().Fonts().FindFont (shxBigFontNumber);
                if (NULL != remappedFont)
                    runProperties.SetShxBigFont (remappedFont);
                }
            }
        }
            
    //-- Style ----------------------------------------------------------------------------
    if (runProperties.HasTextStyle ())
        {
        UInt32              textStyleID = (UInt32)runProperties.GetTextStyleId ();
        EachTextStyleArg    textStyleArg (textStyleID, PROPSCALLBACK_FLAGS_NoFlagsSet, context);
        
        context.DoTextStyleCallback (&textStyleID, textStyleArg);
        
        if ((StyleParamsRemapping::ApplyStyle == textStyleArg.GetRemappingAction ()
            || StyleParamsRemapping::Override == textStyleArg.GetRemappingAction ())
            && 0 != textStyleID)
            {
            hasModifiedProperties = true;

            runProperties.ApplyTextStyle(*context.GetDestinationDgnModel()->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(textStyleID)), true);
            
            DPoint2d        textSize        = runProperties.GetFontSize ();
            TextParamWide   textParamsCopy  = textParams;
            
            runProperties.ToElementData (textParamsCopy, context.GetDestinationDgnModel()->GetDgnProject ());
            
            processTextStyleRemapping (textStyleArg, textSize, textParamsCopy, textBlock, textStyleID, context, NULL, false);
            
            runProperties.FromElementData (textParamsCopy, textSize, context.GetDestinationDgnModel ()->GetDgnProject ());
            }
        }
    return hasModifiedProperties;
    }

//---------------------------------------------------------------------------------------
//! Based on the element properties mask, will optionally create a TextBlock and apply layout-based properties to it, regenerating the element (potentially turning a text element into a text node).
// @bsimethod                                                   Jeff.Marker     01/08
//---------------------------------------------------------------------------------------
void TextHandlerBase::ProcessLayoutPropertiesByTextBlock (EditElementHandleR eeh, PropertyContextR context)
    {
    // If properties being edited don't affect layout we don't beed to regenerate...
    if (0 == ((ELEMENT_PROPERTY_Font | ELEMENT_PROPERTY_TextStyle | ELEMENT_PROPERTY_ElementTemplate) & context.GetElementPropertiesMask ()))
        return;
    
    eeh.GetXAttributeChangeSet()->LoadXasAsWrites (eeh.GetElementDescrP ());

    TextBlock       textBlock (eeh);
    bool            hasModifiedProperties   = false;
    TextParamWide   orginalTextParam;
    
    // We have decided, contrary to our ideals due to legacy behavior, that motions
    //  in text (e.g. size or justification changes) should not disturb the user origin.
    //  This also (just-so-happens) to coincide with our new DWG export mantra of never
    //  shifting user origin. This code used to be in TextBlock::PerformLayout, but the problem
    //  there is that justification has already been updated here before we get there,
    //  therefore text elements (which don't inherently have a user origin) would compute
    //  an updated user origin, and ensured it complied with that; therefore, transplanting
    //  it here to ensure we sync with the actual old user origin.
    DPoint3d        oldUserOrigin           = textBlock.GetUserOrigin ();
    
    // Under normal circumstances, asking a TextBlock to emit an element when it is empty
    //  generates an error, and it will not create an element. MicroStation must support
    //  the concept of empty text nodes, and propogate style to them, so we must detect
    //  this scenario and force TextBlock to emit an empty text node.
    if (TEXT_NODE_ELM == eeh.GetLegacyType() && textBlock.IsEmpty ())
        textBlock.SetForceTextNodeFlag (true);
    
    //-- Style for Text Block -------------------------------------------------------------
    {
    TextBlockProperties tbProps = textBlock.GetProperties ();
    TextParamWide       tbTextParam;
    DPoint2d            scale = { 0.0, 0.0 };
    UInt32              maxUnitsPerLine;
    
    tbProps.ToElementData (tbTextParam, &maxUnitsPerLine);
    
    if (NULL != textBlock.GetTextNodeRunProperties ())
        scale = textBlock.GetTextNodeRunProperties ()->GetFontSize ();
    
    orginalTextParam = tbTextParam;

    EachTextStyleArg tbTextStyleArg (tbTextParam.textStyleId, PROPSCALLBACK_FLAGS_NoFlagsSet, context);
    
    context.DoTextStyleCallback (&tbTextParam.textStyleId, tbTextStyleArg);
    
    if ((StyleParamsRemapping::ApplyStyle == tbTextStyleArg.GetRemappingAction ()
        || StyleParamsRemapping::Override == tbTextStyleArg.GetRemappingAction ())
        && 0 != tbTextParam.textStyleId)
        {
        hasModifiedProperties = true;
        processTextStyleRemapping (tbTextStyleArg, scale, tbTextParam, textBlock, tbTextParam.textStyleId, context, &maxUnitsPerLine, true);
        tbProps.FromElementData (tbTextParam, scale, maxUnitsPerLine);
        textBlock.SetProperties (tbProps);
        }
    }//end text block
    
    if (NULL != textBlock.GetTextNodeRunProperties ())
        {
        RunProperties textNodeProperties = *textBlock.GetTextNodeRunProperties ();
        hasModifiedProperties |= RemapRunProperties (textNodeProperties, context, textBlock, orginalTextParam);
        textBlock.SetTextNodeProperties (&textNodeProperties);
        }
    
    //-- Style for Paragraphs -------------------------------------------------------------
    if (!textBlock.IsEmpty ())
        {
        for (size_t i = 0; i < textBlock.GetParagraphCount (); i++)
            {
            ParagraphProperties paraProps = *textBlock.GetParagraphProperties (i);
            TextParamWide       paraTextParam = orginalTextParam;
            
            paraProps.ToElementData (paraTextParam);
            EachTextStyleArg paraTextStyleArg (paraTextParam.textStyleId, PROPSCALLBACK_FLAGS_NoFlagsSet, context);
            
            context.DoTextStyleCallback (&paraTextParam.textStyleId, paraTextStyleArg);
            
            if ((StyleParamsRemapping::ApplyStyle == paraTextStyleArg.GetRemappingAction ()
                || StyleParamsRemapping::Override == paraTextStyleArg.GetRemappingAction ())
                && 0 != paraTextParam.textStyleId)
                {
                hasModifiedProperties = true;
                
                DPoint2d dummyScale = { 1.0, 1.0 };
                processTextStyleRemapping (paraTextStyleArg, dummyScale, paraTextParam, textBlock, paraTextParam.textStyleId, context, NULL, false);
                paraProps.FromElementData (paraTextParam);
                
                Caret start = textBlock.Begin ();
                for (UInt32 j = 0; j < i; j++)
                    start.MoveToNextParagraph ();
                
                Caret end = start;
                end.MoveToNextParagraph ();

                ParagraphRange paragraphRange (start, end);
                textBlock.SetParagraphPropertiesForRange (paraProps, paragraphRange);
                }
            }
        }
    
    // The normal process properties mechanism did not change layout-dependent properties, do so here.
    if (!textBlock.IsEmpty ())
        {
        for (size_t i = 0; i < textBlock.GetRunPropertiesCount (); i++)
            {
            RunProperties runProps = *textBlock.GetRunProperties (i);
            hasModifiedProperties |= RemapRunProperties (runProps, context, textBlock, orginalTextParam);
            textBlock.ReplaceRunProperties (runProps, i);
            }
        }

    if (!hasModifiedProperties)
        return;

    /* NOTE: In the unlikely event of a different destination model with purpose of 
             change (not remap or clone); need to setup text block with destination model
             so correct font, etc. can be found as ids are now for destination. */
    textBlock.Reprocess ();
    
    DPoint3d newUserOrigin = textBlock.GetUserOrigin ();
    if (!oldUserOrigin.isEqual (&newUserOrigin))
        {
        DPoint3d diff = oldUserOrigin;
        diff.subtract (&newUserOrigin);
        
        DPoint3d textBlockPrimaryOrigin = textBlock.GetTextElementOrigin ();
        textBlockPrimaryOrigin.add (&diff);
        
        textBlock.SetTextElementOrigin (textBlockPrimaryOrigin);
        }
    
    EditElementHandle newEeh;
    textBlock.ToElement (newEeh, context.GetDestinationDgnModel (), &eeh);

    MSElementDescrPtr newEdP = newEeh.ExtractElementDescr();

    if (!newEdP.IsValid())
        {
        BeAssert (0); 
        return;
        }

    eeh.ReplaceElementDescr (newEdP.get());
    context.SetElementChanged ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
static BentleyStatus extractTextFormattingData (ElementHandleCR element, TextParamWideR textParams, DPoint2dR textSize, UInt16& maxLineLength)
    {
    if (TEXT_NODE_ELM == element.GetLegacyType())
        {
        if (SUCCESS != TextNodeHandler::GetTextParams       (element,   textParams))    { BeAssert (false); return ERROR; }
        if (SUCCESS != TextNodeHandler::GetFontSize         (element,   textSize))      { BeAssert (false); return ERROR; }
        if (SUCCESS != TextNodeHandler::GetMaxCharsPerLine  (element,   maxLineLength)) { BeAssert (false); return ERROR; }
        }
    else
        {
        if (SUCCESS != TextElemHandler::GetTextParams       (element,   textParams))    { BeAssert (false); return ERROR; }
        if (SUCCESS != TextElemHandler::GetFontSize         (element,   textSize))      { BeAssert (false); return ERROR; }
        
        maxLineLength = 0;
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
static BentleyStatus updateTextFormattingData (EditElementHandleR eeh, TextParamWideCR textParams, DPoint2dCR textSize, UInt16 const & maxLineLength)
    {
    if (TEXT_NODE_ELM == eeh.GetLegacyType())
        {
        if (SUCCESS != TextNodeHandler::SetTextParams       (eeh,   textParams,     false)) { BeAssert (false); return ERROR; }
        if (SUCCESS != TextNodeHandler::SetFontSize         (eeh,   textSize,       false))  { BeAssert (false); return ERROR; }
        if (SUCCESS != TextNodeHandler::SetMaxCharsPerLine  (eeh,   maxLineLength))         { BeAssert (false); return ERROR; }
        }
    else
        {
        if (SUCCESS != TextElemHandler::SetTextParams       (eeh,   textParams,     false)) { BeAssert (false); return ERROR; }
        if (SUCCESS != TextElemHandler::SetFontSize         (eeh,   textSize,       false))  { BeAssert (false); return ERROR; }
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//! A common method called to edit or query properties of text elements or text nodes. Note that this method will query all properties (layout and non layout-based properties), but will only edit non layout-based properties, leaving the caller to properly go through a TextBlock to properly edit layout-based properties.
// @bsimethod                                                   JoshSchifter    06/06
//---------------------------------------------------------------------------------------
ProcessTextPropertiesStatus TextHandlerBase::ProcessTextPropertiesByElement (PropertyContextR context, ElementHandleCR eh, EditElementHandleP eeh, UInt32* originalColor)
    {
    // Optimization when only processing level during add...
    if (ELEMENT_PROPERTY_Level == context.GetElementPropertiesMask ())
        return ProcessTextPropertiesStatus_None;
    
    bool            isTextNode      = (TEXT_NODE_ELM == eh.GetLegacyType());
    TextParamWide   textParams;
    DPoint2d        textSize;
    UInt16          maxLineLength;
    
    if (SUCCESS != extractTextFormattingData (eh, textParams, textSize, maxLineLength))
        return ProcessTextPropertiesStatus_None;
    
    if (originalColor)
        textParams.color = *originalColor; // NOTE: dhdr.symb.color already remapped by super!
    
    if (NULL == eeh) // query
        {
        TextHandlerBase::ProcessNonLayoutPropertiesDirect (textParams, maxLineLength, textSize, context, false);
        TextHandlerBase::ProcessLayoutPropertiesDirect (textParams, textSize, isTextNode, maxLineLength, context, false);
        
        return ProcessTextPropertiesStatus_All;
        }
    
    ProcessTextPropertiesStatus returnStatus    = ProcessTextPropertiesStatus_LayoutIndepedentOnly;
    bool                        wasChanged      = TextHandlerBase::ProcessNonLayoutPropertiesDirect (textParams, maxLineLength, textSize, context, true);
    
    if (EditPropertyPurpose::Change != context.GetIEditPropertiesP ()->_GetEditPropertiesPurpose ())
        {
        wasChanged      |= TextHandlerBase::ProcessLayoutPropertiesDirect (textParams, textSize, isTextNode, maxLineLength, context, true);
        returnStatus    = ProcessTextPropertiesStatus_All;
        }
    
    if (wasChanged)
        {
        // Update element data, only here if there were changes...
        // Also note that the MDL directCreate* methods can change element size because they attempt to audit and correct
        //  various parts of the element (e.g. code page). If we aren't allowed to change size, assume the methods above
        //  did not actually change size, and also assume all changes are localized to the TextParamWide, and rewrite the linkage.
        //  We should also ensure that the symbology color matches the TextParamWide color (also not a size change).
        if (SUCCESS != updateTextFormattingData (*eeh, textParams, textSize, maxLineLength))
            return ProcessTextPropertiesStatus_None;
        
        context.SetElementChanged ();
        }
    
    return returnStatus;
    }

//---------------------------------------------------------------------------------------
//! This is used to ensure that the text style overrides in the textParams match the style (or lack there of by erasing all overrides). This method should only be called when the element has already been marked for change; this method may change the element, but will not update the context saying it did.
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
void TextHandlerBase::SetOverridesFromStyle (PropertyContextR context, EditElementHandleR eeh)
    {
    // While the other override (what we chain to) also validates, short-circuit here to prevent extracting data unnecessarily.
    if (!shouldSetOverridesFromStyle (context))
        return;
    
    bool            isTextNode      = (TEXT_NODE_ELM == eeh.GetLegacyType());
    TextParamWide   textParams;
    DPoint2d        textSize;
    UInt16          maxLineLength;
    
    if (SUCCESS != extractTextFormattingData (eeh, textParams, textSize, maxLineLength))
        return;
    
    TextHandlerBase::SetOverridesFromStyle (context, textParams, textSize, maxLineLength, isTextNode);
    
    updateTextFormattingData (eeh, textParams, textSize, maxLineLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
bool TextHandlerBase::_IsTextElement (ElementHandleCR) const { return true; }
bool TextHandlerBase::_DoesSupportFields (ElementHandleCR) const { return true; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
ITextPartIdPtr TextHandlerBase::_GetTextPartId (ElementHandleCR, HitPathCR) const
    {
    // Text elements can only ever have a single text part, thus we don't need to squirrel any data away.
    return ITextPartId::Create ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
TextBlockPtr TextHandlerBase::_GetTextPart (ElementHandleCR eh, ITextPartIdCR) const
    {
    return TextBlock::Create (eh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
ITextEdit::ReplaceStatus TextHandlerBase::_ReplaceTextPart (EditElementHandleR eeh, ITextPartIdCR, TextBlockCR updatedText)
    {
    UInt32 nodeNumber = 0;
    if (TEXT_NODE_ELM == eeh.GetLegacyType())
        TextNodeHandler::GetNodeNumber (eeh, nodeNumber);
    
    EditElementHandle updatedTextEeh;
    
    TextBlock::ToElementResult toElementStatus = static_cast<TextBlock::ToElementResult>(TextHandlerBase::CreateElement (updatedTextEeh, &eeh, updatedText));
    
    if (TextBlock::TO_ELEMENT_RESULT_Empty == toElementStatus)
        return ReplaceStatus_Delete;
    
    if (TextBlock::TO_ELEMENT_RESULT_Success != toElementStatus)
        return ReplaceStatus_Error;
    
    eeh.ReplaceElementDescr (updatedTextEeh.ExtractElementDescr().get());
    
    if (TEXT_NODE_ELM == eeh.GetLegacyType())
        TextNodeHandler::SetNodeNumber (eeh, nodeNumber);

    return ReplaceStatus_Success;
    }

//---------------------------------------------------------------------------------------
//! Called to edit or query properties on a text element or text node, which handles all properties than CAN affect the layout of text.
//! Note that this is bad to call for editing (acceptable for queries) as layout-based properties should be set via a TextBlock. However, when a TextBlock cannot be used (e.g. for dimensions), this is the only alternative.
// @bsimethod                                                   Jeff.Marker     01/08
//---------------------------------------------------------------------------------------
bool TextHandlerBase::ProcessLayoutPropertiesDirect
(
TextParamWideR      textParams,
DPoint2dR           textSize,
bool                isTextNode,
int                 textNodeLineLength,
PropertyContextR    context,
bool                canChange
)
    {
    bool changed = false;
    
    // -- Fonts -----------------------------------------------------------------------------------
    changed |= context.DoFontCallback (canChange ? &textParams.font : NULL, EachFontArg (textParams.font, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
    
    if (0 != textParams.flags.shxBigFont)
        changed |= context.DoFontCallback (canChange ? &textParams.shxBigFont : NULL, EachFontArg (textParams.shxBigFont, PROPSCALLBACK_FLAGS_NoFlagsSet, context));

    // -- Style -----------------------------------------------------------------------------------
    EachTextStyleArg textStyleArg (textParams.textStyleId, PROPSCALLBACK_FLAGS_NoFlagsSet, context);

    changed |= context.DoTextStyleCallback (canChange ? &textParams.textStyleId : NULL, textStyleArg);

    if (!canChange || 0 == textParams.textStyleId)
        return changed;
    
    StyleParamsRemapping paramsRemapping = textStyleArg.GetRemappingAction ();

    //  The element has potentially been assigned to a new style.
    //  This change may have an impact on the parameters that are derived from the style.
    //  *** This step should be done after all ids are remapped ***

    DPoint2d        saveScale;
    TextParamWide   saveTextParam;

    memcpy (&saveTextParam, &textParams, sizeof(saveTextParam));
    memcpy (&saveScale, &textSize, sizeof(saveScale));

    switch (paramsRemapping)
        {
        case StyleParamsRemapping::Override:
            {
            LegacyTextStyle               elementTextStyle;
            DgnModelP            destDgnModel = context.GetDestinationDgnModel();
            LegacyTextStyleOverrideFlags  newOverrides;

            // use the style id of the one with the same name in the file and
            // set the override flags on the text to match the changes in the file 
            textstyle_getTextStyleFromTextParamWide (&elementTextStyle, &saveScale, &saveTextParam, textNodeLineLength);

            if (SUCCESS == textstyle_getOverrideFlagsForCompareWithStyle (&newOverrides, textParams.textStyleId, &elementTextStyle, destDgnModel, destDgnModel))
                textParams.overridesFromStyle.ComputeLogicalOr (textParams.overridesFromStyle, newOverrides);

            if (textParams.overridesFromStyle.AreAnyFlagsSet ())
                textParams.exFlags.styleOverrides = true;

            break;
            }
        
        case StyleParamsRemapping::ApplyStyle:  //  Apply the style's parameters to the element.
            {            
#ifdef DGNV10FORMAT_CHANGES_WIP
            int         lineLength;
            LegacyTextStyle   textStyle;

            DgnModelP defaultModel;
            StatusInt status = mdlDgnModel_getDefaultDgnModel (&defaultModel, NULL, context.GetSourceDgnModel (), true, false, false);
            BeAssert (SUCCESS == status);

            if (SUCCESS == textstyle_getResolvedStyle (&textStyle, NULL, textParams.textStyleId, defaultModel, context.GetDestinationDgnModel ()))
                textstyle_updateTextParamWideFromTextStyle (&textParams, &textSize, isTextNode, &lineLength, &textStyle, textParams.textStyleId, textParams.annotationScale, textParams.exFlags.annotationScale);
#endif
            
            break;
            }
        }
    
    if (!isTextNode)
        {
        textParams.overridesFromStyle.linespacing = saveTextParam.overridesFromStyle.linespacing;
        textParams.lineSpacing = saveTextParam.lineSpacing;
        }

    changed |= (0 != memcmp (&saveTextParam, &textParams,   sizeof(saveTextParam)));
    changed |= (0 != memcmp (&saveScale,     &textSize,     sizeof(saveScale)));

    return changed;
    }

//---------------------------------------------------------------------------------------
//! Called to edit or query properties on a text element or text node, which handles all properties than CANNOT affect the layout of text.
// @bsimethod                                                   JoshSchifter    06/06
//---------------------------------------------------------------------------------------
bool TextHandlerBase::ProcessNonLayoutPropertiesDirect (TextParamWideR textParams, int lineLength, DPoint2dR textSize, PropertyContextR context, bool canChange)
    {
    bool changed = false;
    
    // -- Colors ----------------------------------------------------------------------------------
    changed |= context.DoColorCallback (canChange ? &textParams.color               : NULL, EachColorArg (textParams.color,                 getPropFlags (PROPSCALLBACK_FLAGS_IsBaseID,         true,                               true),                          context));
    changed |= context.DoColorCallback (canChange ? &textParams.backgroundFillColor : NULL, EachColorArg (textParams.backgroundFillColor,   getPropFlags (PROPSCALLBACK_FLAGS_IsBackgroundID,   textParams.flags.bgColor,           true),                          context));
    changed |= context.DoColorCallback (canChange ? &textParams.underlineColor      : NULL, EachColorArg (textParams.underlineColor,        getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet,       textParams.exFlags.underlineStyle,  textParams.flags.underline),    context));
    changed |= context.DoColorCallback (canChange ? &textParams.overlineColor       : NULL, EachColorArg (textParams.overlineColor,         getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet,       textParams.exFlags.overlineStyle,   textParams.exFlags.overline),   context));
    changed |= context.DoColorCallback (canChange ? &textParams.backgroundColor     : NULL, EachColorArg (textParams.backgroundColor,       getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet,       textParams.flags.bgColor,           true),                          context));

    // -- Weights ---------------------------------------------------------------------------------
    changed |= context.DoWeightCallback (canChange ? &textParams.underlineWeight    : NULL, EachWeightArg (textParams.underlineWeight,  getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet, textParams.exFlags.underlineStyle,    textParams.flags.underline),    context));
    changed |= context.DoWeightCallback (canChange ? &textParams.overlineWeight     : NULL, EachWeightArg (textParams.overlineWeight,   getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet, textParams.exFlags.overlineStyle,     textParams.exFlags.overline),   context));
    changed |= context.DoWeightCallback (canChange ? &textParams.backgroundWeight   : NULL, EachWeightArg (textParams.backgroundWeight, getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet, textParams.flags.bgColor,             true),                          context));

    // -- Line Styles -----------------------------------------------------------------------------
    // Text elements don't support style params.
    changed |= context.DoLineStyleCallback (canChange ? &textParams.underlineStyle  : NULL, EachLineStyleArg (textParams.underlineStyle,   NULL,    getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet,   textParams.exFlags.underlineStyle,  textParams.flags.underline),    context));
    changed |= context.DoLineStyleCallback (canChange ? &textParams.overlineStyle   : NULL, EachLineStyleArg (textParams.overlineStyle,    NULL,    getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet,   textParams.exFlags.overlineStyle,   textParams.exFlags.overline),   context));
    changed |= context.DoLineStyleCallback (canChange ? &textParams.backgroundStyle : NULL, EachLineStyleArg (textParams.backgroundStyle,  NULL,    getPropFlags (PROPSCALLBACK_FLAGS_NoFlagsSet,   textParams.flags.bgColor,           true),                          context));

    return changed;
    }

//---------------------------------------------------------------------------------------
//! This is used to ensure that the text style overrides in the textParams match the style (or lack there of by erasing all overrides). This method should only be called when the element has already been marked for change; this method may change the element, but will not update the context saying it did.
// @bsimethod                                                   Jeff.Marker     08/08
//---------------------------------------------------------------------------------------
void TextHandlerBase::SetOverridesFromStyle (PropertyContextR context, TextParamWide& textParams, DPoint2d textSize, int lineLength, bool isTextNode)
    {
    // Called directly from dimension code, so make sure to validate here.
    if (!shouldSetOverridesFromStyle (context))
        return;
    
    if (0 != textParams.textStyleId)
        {
        LegacyTextStyle               elementTextStyle;
        DgnModelP            destDgnModel        = context.GetDestinationDgnModel();
        LegacyTextStyleOverrideFlags  newOverrides;

        textstyle_getTextStyleFromTextParamWide (&elementTextStyle, &textSize, &textParams, lineLength);

        if (SUCCESS == textstyle_getOverrideFlagsForCompareWithStyle (&newOverrides, textParams.textStyleId, &elementTextStyle, destDgnModel, destDgnModel))
            {
            textParams.overridesFromStyle.ComputeLogicalOr (textParams.overridesFromStyle, newOverrides);
            
            // If not a text node, then we can ignore differences for the following text node-only properties.
            if (!isTextNode)
                {
                textParams.overridesFromStyle.lineLength = 0;
                textParams.overridesFromStyle.linespacing = 0;
                // 'nodeJust' and its override flag must be maintained for backwards compatibility.
                }
            }

        textParams.exFlags.styleOverrides = textParams.overridesFromStyle.AreAnyFlagsSet ();
        }
    else
        {
        memset (&textParams.overridesFromStyle, 0, sizeof (textParams.overridesFromStyle));
        textParams.exFlags.styleOverrides = 0;
        }
    }

//---------------------------------------------------------------------------------------
//! Maps a text node justification to an equivalent simple text justification.
// @bsimethod                                                   Steve.Knipmeyer 05/88
//---------------------------------------------------------------------------------------
TextElementJustification TextHandlerBase::RemapNodeToTextJustification (TextElementJustification nodeJustification)
    {
    switch (nodeJustification)
        {
        case (TextElementJustification::LeftMarginTop):        return TextElementJustification::LeftTop;
        case (TextElementJustification::LeftMarginMiddle):     return TextElementJustification::LeftMiddle;
        case (TextElementJustification::LeftMarginBaseline):   return TextElementJustification::LeftBaseline;
        case (TextElementJustification::RightMarginTop):       return TextElementJustification::RightTop;
        case (TextElementJustification::RightMarginMiddle):    return TextElementJustification::RightMiddle;
        case (TextElementJustification::RightMarginBaseline):  return TextElementJustification::RightBaseline;
        
        default:
            return nodeJustification;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
static StatusInt appendAnyLinkageCallback (LinkageHeaderP, void* args, LinkageHeaderP linkage, DgnElementP inElement)
    {
    DgnElementP outElement = (DgnElementP)args;
    
    elemUtil_appendLinkage (outElement, linkage);
    
    return PROCESS_ATTRIB_STATUS_NOCHANGE;
    }

//---------------------------------------------------------------------------------------
//! Forms a template of a text element from a text node. The template is supposed to be used to generate the text elements of a node such that they have the same parameters.
// @bsimethod                                                   Steve.Knipmeyer 11/91
//---------------------------------------------------------------------------------------
void TextHandlerBase::CreateTemplateTextElemFromNode (EditElementHandleR outTextElementEeh, ElementHandleCR inTextNodeElementEh)
    {
    Text_3d const& inTextNodeElement = inTextNodeElementEh.GetElementCP()->ToText_3d();
    bool        is3d                = inTextNodeElement.Is3d();

    DgnV8ElementBlank   tmpElement;
    memset (&tmpElement, 0, sizeof(tmpElement));

    Text_2d& out2dEl = tmpElement.ToText_2dR();
    Text_3d& out3dEl = tmpElement.ToText_3dR();

    out2dEl.SetLegacyType(TEXT_ELM);
    out2dEl.SetLevel(inTextNodeElement.GetLevelValue());
    out2dEl.SetIsGraphic(true);
    out2dEl.SetIs3d(inTextNodeElement.Is3d());
    out2dEl.SetProperties(inTextNodeElement.GetProperties());
    out2dEl.SetSymbology(inTextNodeElement.GetSymbology());
    out2dEl.SetDisplayPriority(inTextNodeElement.GetDisplayPriority());

    if (is3d)
        {
        out3dEl.font             = inTextNodeElement.font;
        out3dEl.just             = static_cast<UInt16>(TextHandlerBase::RemapNodeToTextJustification (static_cast<TextElementJustification>(inTextNodeElement.just)));
        out3dEl.lngthmult        = inTextNodeElement.lngthmult;
        out3dEl.hghtmult         = inTextNodeElement.hghtmult;
        out3dEl.SetSizeWordsNoAttributes(sizeof(Text_3d) / 2);
        out3dEl.origin           = inTextNodeElement.origin;

        memcpy (out3dEl.quat, inTextNodeElement.quat, 4*sizeof (double));
        }
    else
        {
        Text_node_2d  const& in_2d = (Text_node_2d const&) inTextNodeElement;
        out2dEl.font             = in_2d.font;
        out2dEl.just             = static_cast<UInt16>(TextHandlerBase::RemapNodeToTextJustification (static_cast<TextElementJustification>(in_2d.just)));
        out2dEl.lngthmult        = in_2d.lngthmult;
        out2dEl.hghtmult         = in_2d.hghtmult;
        out2dEl.SetSizeWordsNoAttributes(sizeof(Text_2d) / 2);
        out2dEl.origin           = in_2d.origin;
        out2dEl.rotationAngle    = in_2d.rotationAngle;
        out2dEl.GetRangeR().low.z = out2dEl.GetRangeR().high.z = 0.0;
        }

    // Do we have attribute data attached?
    if (inTextNodeElement.GetSizeWords() > inTextNodeElement.GetAttributeOffset())
        mdlElement_processLinkages (appendAnyLinkageCallback, &tmpElement, (DgnElementP)(&inTextNodeElement));
    
    outTextElementEeh.SetElementDescr(new MSElementDescr(tmpElement,*inTextNodeElementEh.GetDgnModelP()), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Venkat.Kalyan                   12/2005
//---------------------------------------------------------------------------------------
void TextHandlerBase::CreateTemplateNodeFromTextElem (EditElementHandleR outTextNodeElementEeh, ElementHandleCR inTextElementEh)
    {
    DgnElementCR inTextElement           = *inTextElementEh.GetElementCP ();
    bool        is3d                    = DisplayHandler::Is3dElem (&inTextElement);
    size_t      outTextNodeElementSize  = is3d ? sizeof (Text_node_3d) : sizeof (Text_node_2d);
    DgnV8ElementBlank   outTextNodeElement;
    
    memset (&outTextNodeElement, 0, outTextNodeElementSize);
    outTextNodeElement.SetLegacyType(TEXT_NODE_ELM);

    DPoint3d userOrigin;
    TextElemHandler::GetElementOrigin (inTextElementEh, userOrigin);

    if (is3d)
        {
        Text_node_3d&   node    = outTextNodeElement.ToText_node_3dR();
        Text_3d const & text    = (Text_3d const &)inTextElement;

        node.SetSizeWordsNoAttributes(sizeof(Text_node_3d) / 2);
        
        node.origin = userOrigin;
        memcpy (node.quat, text.quat, 4 * sizeof (double));
        }
    else
        {
        outTextNodeElement.GetRangeR().low.z = outTextNodeElement.GetRangeR().high.z = 0.0;
        outTextNodeElement.SetSizeWordsNoAttributes(sizeof(Text_node_2d) / 2);
        outTextNodeElement.ToText_node_2dR().origin.x        = userOrigin.x;
        outTextNodeElement.ToText_node_2dR().origin.y        = userOrigin.y;
        outTextNodeElement.ToText_node_2dR().rotationAngle   = inTextElement.ToText_2d().rotationAngle;
        }

    outTextNodeElement.SetIs3d(inTextElement.Is3d());
    outTextNodeElement.SetIsGraphic(true);

    // Use the parameters directly which have analogs.
    outTextNodeElement.SetLevel(inTextElement.GetLevelValue());
    outTextNodeElement.SetDisplayPriority(inTextElement.GetDisplayPriority());

    outTextNodeElement.SetProperties(inTextElement.GetProperties());
    outTextNodeElement.SetSymbology(inTextElement.GetSymbology());
    outTextNodeElement.ToText_node_2dR().just        = inTextElement.ToText_2d().just;
    outTextNodeElement.ToText_node_2dR().font        = inTextElement.ToText_2d().font;
    outTextNodeElement.ToText_node_2dR().lngthmult   = inTextElement.ToText_2d().lngthmult;
    outTextNodeElement.ToText_node_2dR().hghtmult    = inTextElement.ToText_2d().hghtmult;

    // Do we have attribute data attached?
    if (inTextElement.GetSizeWords() > inTextElement.GetAttributeOffset())
        mdlElement_processLinkages (appendAnyLinkageCallback, &outTextNodeElement, const_cast<DgnElementP>(&inTextElement));

    outTextNodeElementEeh.SetElementDescr(new MSElementDescr(outTextNodeElement,*inTextElementEh.GetDgnModelP()), false);

    TextParamWide textParams;
    if (SUCCESS == TextElemHandler::GetTextParams (inTextElementEh, textParams))
        {
        // If we have text params then make sure to coerce them since they could be from text elements and not correct.
        //  Note that this is the only thing coerce_textParamsForTextNode did, so copying here instead of bothering to bring the function down.
        textParams.exFlags.acadFittedText = 0;
        
        TextNodeHandler::SetTextParams (outTextNodeElementEeh, textParams, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/04
//---------------------------------------------------------------------------------------
bool TextHandlerBase::RemoveWhiteSpaceLinkages (EditElementHandleR eeh)
    {
    // Delete all white space linkages from the element.  White space linkage includes tabs, line feeds and carriage returns.
    BitMaskP    pTabCRBitMask   = NULL;
    bool        foundAny        = false;

    // There may be more than one linkage. So, iterate until all are found and deleted.
    while (SUCCESS == BitMaskLinkage::ExtractBitMask (&pTabCRBitMask, eeh.GetElementCP (), BITMASK_LINKAGE_KEY_TextWhiteSpace, 0))
        {
        foundAny = true;
        
        int bitCount = pTabCRBitMask->GetCapacity ();

        for (int i = 0; i < bitCount; ++i)
            BitMaskLinkage::DeleteBitMask (eeh.GetElementP (), BITMASK_LINKAGE_KEY_TextWhiteSpace, 0);
        }
    
    return foundAny;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Venkat.Kalyan                   05/2005
//---------------------------------------------------------------------------------------
void TextHandlerBase::RemoveTextDataLinkages (EditElementHandleR eeh)
    {
    TextHandlerBase::RemoveWhiteSpaceLinkages (eeh);
    TextIndentationLinkage::RemoveLinkages (eeh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Venkat.Kalyan                   10/2005
//---------------------------------------------------------------------------------------
bool TextHandlerBase::HasMTextRenderingLinkage (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        { BeAssert (false); return false; }

    if (TEXT_ELM == eh.GetLegacyType())
        {
        TextParamWide textParams;
        TextRenderingLinkage::FillTextParamsFromLinkage (eh, textParams);
        
        return textParams.renderingFlags.alignEdge;
        }
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return false; }
    
    for (ChildElemIter childElemIter (eh, ExposeChildrenReason::Count); childElemIter.IsValid (); childElemIter = childElemIter.ToNext ())
        {
        if (TextHandlerBase::HasMTextRenderingLinkage (childElemIter))
            return true;
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
TextBlockToElementResult TextHandlerBase::CreateElement (EditElementHandleR eeh, ElementHandleCP templateEh, TextBlockCR textBlock)
    {
    return static_cast<TextBlockToElementResult> (textBlock.ToElement (eeh, &textBlock.GetDgnModelR (), templateEh));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
TextBlockPtr TextHandlerBase::GetFirstTextPartValue (ElementHandleCR eh)
    {
    ITextQueryCP textQuery = eh.GetITextQuery ();
    if (NULL == textQuery)
        return NULL;
    
    T_ITextPartIdPtrVector textParts;
    textQuery->GetTextPartIds (eh, *ITextQueryOptions::CreateDefault (), textParts);

    if (textParts.empty ())
        {
        BeAssert (false && L"You should only be calling this on text elements and nodes, which should by-definition always have a single text part.");
        return NULL;
        }

    if (1 != textParts.size ())
        BeAssert (false && L"You should use ITextQuery instead and deal with all text parts (not just the first one).");
    
    return textQuery->GetTextPart (eh, *textParts[0]);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextNodeHandler ------------------------------------------------------------------------------------------------------------- TextNodeHandler --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IAnnotationHandlerP     TextNodeHandler::_GetIAnnotationHandler         (ElementHandleCR)                               { return this; }
void                    TextNodeHandler::_GetSnapOrigin                 (ElementHandleCR el, DPoint3dR origin)          { _GetTransformOrigin (el, origin); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Keith.Bentley   09/03
//---------------------------------------------------------------------------------------
void TextNodeHandler::DrawNodeNumber (ElementHandleCR eh, ViewContextR context)
    {
    IPickGeom* pick = context.GetIPickGeom();

    if (NULL != pick)
        {
        DPoint3d origin;
        TextNodeHandler::GetUserOrigin (eh, origin);
        
        pick->SetHitPriorityOverride (HitPriority::Origin);
        context.GetIDrawGeom().DrawPointString3d (1, &origin, NULL);
        pick->SetHitPriorityOverride (HitPriority::Highest);
        }

    bool drawNode = (context.GetViewFlags() && context.GetViewFlags()->text_nodes && !context.CheckICachedDraw ()); // Should never bake node into QvElem...
    
    if (!drawNode)
        return;

    switch (context.GetDrawPurpose ())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::FitView:
        case DrawPurpose::FenceAccept:
            {
            Text_node_2d const* node = (Text_node_2d const*)eh.GetElementCP();

            // for picking and range calc, we don't draw the text node itself unless it's an empty text node.
            if (0 != node->componentCount)
                drawNode = false;
            
            break;
            }
        }

    if (!drawNode)
        return;

    // Always set mark in case of viewindependent text node: restored in destructor!
    ViewContext::ContextMark mark(&context);
    pushTextNodeTransform (eh, context);

    StrokeTextNodeNumber stroker;
    context.DrawCached (eh, stroker, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/07
//---------------------------------------------------------------------------------------
void TextNodeHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    UInt32 originalColor = eeh.GetElementCP ()->GetSymbology().color;

    T_Super::_EditProperties (eeh, context);

    ProcessTextPropertiesStatus processStatus = TextHandlerBase::ProcessTextPropertiesByElement (context, eeh, &eeh, &originalColor);

    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        {
        if (TEXT_ELM == childElm.GetLegacyType())
            {
            // Evilness:
            //  Because text elements potentially need to turn into text nodes due to some property changes
            //  (e.g. to retain fractions from RSC to TT), we cannot simply call text elements' EditProperties
            //  if they are already a child of a text node. Note that we are intentionally saving/restoring
            //  the context's element handle just like the base class implementation would.
            
            ElementHandleCP saveEh = context.GetCurrentElemHandleP ();
            context.SetCurrentElemHandleP (&eeh);
            
            Handler&            childElmHandler         = childElm.GetHandler ();
            TextElemHandler*    typedChildElmHandler    = dynamic_cast<TextElemHandler*>(&childElmHandler);
            
            BeAssert (NULL != typedChildElmHandler);
            if (NULL != typedChildElmHandler)
                typedChildElmHandler->ProcessPropertiesByElement (childElm, context);
            
            context.SetCurrentElemHandleP (saveEh);
            }
        else
            {
            BeDataAssert (false);
            }
        }

    if (ProcessTextPropertiesStatus_All != processStatus)
        TextHandlerBase::ProcessLayoutPropertiesByTextBlock (eeh, context);
    
    // Only do this if the element has already been changed.
    if (context.GetElementChanged ())
        {
        TextHandlerBase::SetOverridesFromStyle (context, eeh);
        
        for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
            {
            if (TEXT_ELM == childElm.GetLegacyType())
                {
                ElementHandleCP saveEh = context.GetCurrentElemHandleP ();
                context.SetCurrentElemHandleP (&eeh);
                
                TextHandlerBase::SetOverridesFromStyle (context, childElm);
                
                context.SetCurrentElemHandleP (saveEh);
                }
            else
                {
                BeAssert (false);
                }
            }
        
        this->ValidateElementRange(eeh);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  07/05
//---------------------------------------------------------------------------------------
void TextNodeHandler::_GetTypeName (WStringR typeName, UInt32 desiredLength)
    {
    typeName.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_TEXT_NODE_ELM), 0, desiredLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  07/06
//---------------------------------------------------------------------------------------
bool TextNodeHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    // Don't allow empty text nodes to be grouped...
    if (SupportOperation::CellGroup == stype && eh && (0 == eh->GetElementCP ()->GetComplexComponentCount ()))
        return false;

    return T_Super::_IsSupportedOperation (eh, stype);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  02/04
//---------------------------------------------------------------------------------------
bool TextNodeHandler::_IsTransformGraphics (ElementHandleCR eh, TransformInfoCR transform)
    {
    return (canUseTransformGraphics (eh, transform, this) || TextNodeHandler::HasAlongTextData (eh));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/08
//---------------------------------------------------------------------------------------
void TextNodeHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    TransformInfo tInfo (flattenTrans);
    
    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    DVec3d zeroVec;
    zeroVec.zero (); // So that child elms know transform already applied...

    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
        childElm.GetHandler().ConvertTo2d (childElm, flattenTrans, zeroVec);

    DgnV8ElementBlank   elm;
    DgnElementCP org = eeh.GetElementCP ();

    org->CopyTo (elm);
    Text_node_2d& textNode2d = elm.ToText_node_2dR();

    textNode2d.SetSizeWordsNoAttributes(sizeof(Text_node_2d) / 2);

    // convert rotation, height and length
    TextHandlerBase::ConvertTextParamsTo2D (textNode2d.lngthmult, textNode2d.hghtmult, textNode2d.rotationAngle, org->ToText_node_3d().lngthmult, org->ToText_node_3d().hghtmult, *org->ToText_node_3d().quat);
    
    textNode2d.componentCount = org->ToText_node_3d().componentCount;
    textNode2d.nodenumber     = org->ToText_node_3d().nodenumber;
    textNode2d.font           = org->ToText_node_3d().font;
    textNode2d.maxlngth       = org->ToText_node_3d().maxlngth;
    textNode2d.just           = org->ToText_node_3d().just;
    textNode2d.linespc        = org->ToText_node_3d().linespc;

    DataConvert::Points3dTo2d (&textNode2d.origin, &org->ToText_node_3d().origin, 1);

    ElementUtil::CopyAttributes (&elm, org);

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/08
//---------------------------------------------------------------------------------------
void TextNodeHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
        childElm.GetHandler().ConvertTo3d (childElm, elevation);

    DgnV8ElementBlank   elm;
    DgnElementCP org = eeh.GetElementCP ();

    org->CopyTo (elm);
    Text_node_3d& textNode3d = elm.ToText_node_3dR();
    textNode3d.SetSizeWordsNoAttributes(sizeof(Text_node_3d) / 2);

    textNode3d.componentCount = org->ToText_node_2d().componentCount;
    textNode3d.nodenumber     = org->ToText_node_2d().nodenumber;
    textNode3d.font           = org->ToText_node_2d().font;
    textNode3d.maxlngth       = org->ToText_node_2d().maxlngth;
    textNode3d.just           = org->ToText_node_2d().just;
    textNode3d.linespc        = org->ToText_node_2d().linespc;
    textNode3d.lngthmult      = org->ToText_node_2d().lngthmult;
    textNode3d.hghtmult       = org->ToText_node_2d().hghtmult;

    DataConvert::RotationToQuaternion (textNode3d.quat, org->ToText_node_2d().rotationAngle);
    DataConvert::Points2dTo3d (&textNode3d.origin, &org->ToText_node_2d().origin, 1, elevation);

    ElementUtil::CopyAttributes (&elm, org);

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Keith.Bentley   07/86
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::TransformTextNode (EditElementHandleR eeh, TransformCR transform, UInt32 options)
    {
    DPoint3d        origin;         if (SUCCESS != TextNodeHandler::GetUserOrigin   (eeh, origin))      return ERROR;
    RotMatrix       orientation;    if (SUCCESS != TextNodeHandler::GetOrientation  (eeh, orientation)) return ERROR;
    DPoint2d        textSize;       if (SUCCESS != TextNodeHandler::GetFontSize     (eeh, textSize))    return ERROR;
    TextParamWide   textParams;     if (SUCCESS != TextNodeHandler::GetTextParams   (eeh, textParams))  return ERROR;
    
    bool shouldScaleTextSize = (!textParams.exFlags.annotationScale || !(options & TRANSFORM_OPTIONS_AnnotationSizeMatchSource));
    
    transform.multiply (&origin);
    
    DPoint2d scaleFactor;
    
    TextString::TransformOrientationAndGetScale (scaleFactor, orientation, NULL, transform, true);
    
    if (shouldScaleTextSize)
        {
        textSize.x *= scaleFactor.x;
        textSize.y *= scaleFactor.y;

        textParams.ApplyScaleFactor (scaleFactor, true, true);
        }

    // Technically it's over-cautious to set the linespacing override for ATLEAST linespacing.
    //  A check for ATLEAST linespacing could be added here.  PC 2/04
    // JM 2013-Jul: Now that we show line spacing as a factor, there's even more value in re-visiting this...
    if (ISNOTEQUAL (eeh.GetElementCP ()->ToText_node_2d().linespc, 0.0) && ISNOTEQUAL(scaleFactor.y, 1.0))
        {
        textParams.overridesFromStyle.linespacing   = true;
        textParams.exFlags.styleOverrides           = true;
        }
    
    if (SUCCESS != TextNodeHandler::SetUserOrigin   (eeh, origin, false))       return ERROR;
    if (SUCCESS != TextNodeHandler::SetOrientation  (eeh, orientation, false))  return ERROR;
    if (SUCCESS != TextNodeHandler::SetFontSize     (eeh, textSize, false))     return ERROR;
    if (SUCCESS != TextNodeHandler::SetTextParams   (eeh, textParams, false))   return ERROR;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
static void adjustTransformForDisableRotateCharacters (TransformInfoR transformInfo, ElementHandleCR eh)
    {
    if (TRANSFORM_OPTIONS_DisableRotateCharacters != (TRANSFORM_OPTIONS_DisableRotateCharacters & transformInfo.GetOptions ()))
        return;
    
    // This essentially boils down to translating the origin, and not applying any orientation changes to the text; so convert to a translation+scale.
    // This workflow allows us to add this to the top of _OnTransform, and retain all existing logic without adding more conditionals and special cases inside _OnTransform.
    
    DPoint3d originalUserOrigin;
    
    switch (eh.GetLegacyType())
        {
        case TEXT_ELM:
            TextElemHandler::ComputeUserOrigin (eh, originalUserOrigin);
            break;
        
        case TEXT_NODE_ELM:
            TextNodeHandler::GetUserOrigin (eh, originalUserOrigin);
            break;
        
        default:
            {
            BeAssert (false && L"Unknown element type inside a text element handler.");
            return;
            }
        }
    
    RotMatrix   normalizedRMatrix;
    DVec3d      scaleFactors;
    
    transformInfo.GetTransform ()->GetMatrix (normalizedRMatrix);
    normalizedRMatrix.NormalizeColumnsOf (normalizedRMatrix, scaleFactors);
    
    Transform noScaleTransform = *transformInfo.GetTransform ();
    noScaleTransform.SetMatrix (normalizedRMatrix);
    
    DPoint3d transformedUserOrigin = originalUserOrigin;
    noScaleTransform.Multiply (transformedUserOrigin);
    
    DPoint3d userOriginDelta = transformedUserOrigin;
    userOriginDelta.Subtract (originalUserOrigin);
    
    RotMatrix scaleMatrix;
    scaleMatrix.InitFromScaleFactors (scaleFactors.x, scaleFactors.y, scaleFactors.z);
    
    transformInfo.SetOptions (transformInfo.GetOptions () ^ TRANSFORM_OPTIONS_DisableRotateCharacters);
    transformInfo.GetTransformR ().InitFrom (scaleMatrix, userOriginDelta);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/03
//---------------------------------------------------------------------------------------
StatusInt TextNodeHandler::_OnTransform (EditElementHandleR eeh, TransformInfoCR transform)
    {
    StatusInt status = T_Super::_OnTransform (eeh, transform);
    if (SUCCESS != status)
        return status;

    // Propagate annotation scale
    TransformInfo localTransInfo = transform;

    adjustTransformForDisableRotateCharacters (localTransInfo, eeh);

    if (localTransInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale)
       TextHandlerBase::UpdateAnnotationScale (eeh, localTransInfo.GetAnnotationScaleAction (), localTransInfo.GetAnnotationScale (), false);

    // NEEDS_WORK: Need to handle the no size change option

    bool shouldApplyMirrorTransform = hasMirrorTransform (eeh, localTransInfo);

    // Handle special mirroring case
    if (shouldApplyMirrorTransform && !TextNodeHandler::HasAlongTextData (eeh))
        {
        TextBlock textBlock (eeh);
        textBlock.ApplyTransform (*localTransInfo.GetTransform ());
        textBlock.Reprocess ();
        
        EditElementHandle potentialReplacement;
        if (TEXTBLOCK_TO_ELEMENT_RESULT_Success != TextHandlerBase::CreateElement (potentialReplacement, &eeh, textBlock))
            return ERROR;
        
        eeh.ReplaceElementDescr (potentialReplacement.ExtractElementDescr().get());
        
        return SUCCESS;
        }

    if (!shouldApplyMirrorTransform)
        {
        // Modify the transform to handle the special transform of annotation text that should not be scaled
        if (!canScaleAnnotationText (eeh, localTransInfo, this))
            {
            DPoint3d origin;
            TextNodeHandler::GetUserOrigin (eeh, origin);

            Transform unscaledTransform;
            LegacyMath::TMatrix::Unscale (&unscaledTransform, localTransInfo.GetTransform (), &origin);
            localTransInfo.GetTransformR () = unscaledTransform;
            localTransInfo.SetOptions (localTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_AnnotationSizeMatchSource);
            }

        // Transform the text node header itself
        if (SUCCESS != TextNodeHandler::TransformTextNode (eeh, *localTransInfo.GetTransform (), localTransInfo.GetOptions ()))
            return ERROR;
        }
    else
        {
        // This case means we're supposed to mirror, but we are along-element.
        //  In the case our dependency object is /not/ moving with us, we need to approximate anyway.
        //  If it /is/ moving with us, we don't really care what we do since the dependency callback will re-generate.
        //  In the first case, we will effectively loose the dependency, and upon editing the text, it will
        //  revert to a normal not-along-element form. We want to ensure the origin is close, but not do
        //  a full transform (e.g. transform_textNodeElmWithOptions) since that will apply the mirror as if
        //  we were making the text upside-down/backwards, which will screw up the second case above.

        DPoint3d origin;
        TextNodeHandler::GetUserOrigin (eeh, origin);
        
        localTransInfo.GetTransform()->Multiply (&origin, 1);
        
        TextNodeHandler::SetUserOrigin (eeh, origin, true);
        }

    // Transform the text element(s) in the text node
    for (ChildEditElemIter childElm (eeh, ExposeChildrenReason::Count); childElm.IsValid(); childElm = childElm.ToNext())
        status = childElm.GetHandler().ApplyTransform (childElm, localTransInfo);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/07
//---------------------------------------------------------------------------------------
void TextNodeHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    TextHandlerBase::ProcessTextPropertiesByElement (context, eh, NULL, NULL);

    for (ChildElemIter childElm (eh, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
        childElm.GetHandler ().QueryProperties (childElm, context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  05/05
//---------------------------------------------------------------------------------------
void TextNodeHandler::_Draw (ElementHandleCR eh, ViewContextR context)
    {
    if (0 == (DISPLAY_INFO_Edge & context.GetDisplayInfo (IsRenderable (eh))))
        return;

    DrawNodeNumber (eh, context);

    // NOTE: Since qvElem now cached on text node we need to check if text should be displayed...
    if (context.GetViewFlags () && context.GetViewFlags ()->fast_text)
        return;

    ViewContext::ContextMark mark (&context);
    IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (*this);

    if (extension)
        extension->_PushDisplayEffects (eh, context);
    
    // NOTE: Choose qvIndexBase of 1 to not conflict with index 0 which is used for text node number...
    StrokeTextNodeElm stroker;
    StrokeAnnotationElm annotationStroker (this, eh, context, &stroker, 1);
    
    annotationStroker.DrawUsingContext ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  05/05
//---------------------------------------------------------------------------------------
void TextNodeHandler::_DrawFiltered (ElementHandleCR eh, ViewContextR context, DPoint3dCP pts, double size)
    {
    if (DrawPurpose::UpdateDynamic == context.GetDrawPurpose ())
        return;

    // When a text node is filtered, we draw the first child (filtered). That way if it's empty we won't make it appear.
    ViewContext::ContextMark mark (&context);
    pushTextNodeTransform (eh, context);

    ChildElemIter childIter (eh, ExposeChildrenReason::Count);
    if (!childIter.IsValid ())
        return;

    DisplayHandlerP handler = childIter.GetDisplayHandler();
    if (!handler)
        return;

    handler->DrawFiltered(childIter, context, pts, size);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2011
//---------------------------------------------------------------------------------------
bool TextNodeHandler::_IsPlanar (ElementHandleCR eh, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal)
    {
    static double   COMPARE_EPSILON = 1.0e-8;
    bool            isFirstChild    = true;
    DVec3d          ourNormal;
    ourNormal.Init (0.0, 0.0, 0.0);

    for (ChildElemIter childEh (eh, ExposeChildrenReason::Count); childEh.IsValid (); childEh = childEh.ToNext ())
        {
        if (TEXT_ELM != childEh.GetLegacyType())
            { BeDataAssert (false); continue; }
        
        DisplayHandlerP childDisplayHandler = childEh.GetDisplayHandler ();
        if (NULL == childDisplayHandler)
            { BeAssert (false); continue; }
        
        DVec3d childNormal;
        
        if (!childDisplayHandler->IsPlanar (childEh, &childNormal, NULL, inputDefaultNormal))
            return false;
        
        if (isFirstChild)
            {
            isFirstChild    = false;
            ourNormal       = childNormal;

            continue;
            }
        
        if (childNormal.AngleTo (ourNormal) > COMPARE_EPSILON)
            return false;
        }
    
    if (isFirstChild && (NULL != normal))
        {
        RotMatrix rMatrix;
        _GetOrientation (eh, rMatrix);
        
        rMatrix.GetColumn (ourNormal, 2);
        }
    
    if (NULL != normal)
        *normal = ourNormal;

    if (NULL != point)
        _GetTransformOrigin (eh, *point);
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   01/07
//---------------------------------------------------------------------------------------
void TextNodeHandler::_GetOrientation (ElementHandleCR eh, RotMatrixR rotation)
    {
    DgnElementCP text = eh.GetElementCP ();
    
    if (DisplayHandler::Is3dElem (text))
        rotation.InitTransposedFromQuaternionWXYZ ( text->ToText_node_3d().quat);
    else
        rotation.InitFromAxisAndRotationAngle(2,  text->ToText_node_2d().rotationAngle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   01/07
//---------------------------------------------------------------------------------------
void TextNodeHandler::_GetTransformOrigin (ElementHandleCR eh, DPoint3dR origin)
    {
    TextNodeHandler::GetUserOrigin (eh, origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  10/04
//---------------------------------------------------------------------------------------
SnapStatus TextNodeHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    SnapPathP       snap = context->GetSnapPath ();
    SnapMode    snapMode = context->GetSnapMode ();

    switch (snapMode)
        {
        case SnapMode::NearestKeypoint:
            {
            if (snap->GetCount () > snapPathIndex+1)
                return context->DoSnapUsingNextInPath (snapPathIndex);

            // FALL THROUGH...
            }

        case SnapMode::Origin:
            {
            ElementHandle   eh (snap->GetPathElem (snapPathIndex));
            DPoint3d        hitPoint;

            _GetSnapOrigin (eh, hitPoint);
            context->ElmLocalToWorld (hitPoint);
            context->SetSnapInfo (snapPathIndex, snapMode, context->GetSnapSprite (snapMode), hitPoint, true, false);

            return SnapStatus::Success;
            }

        default:
            return context->DoTextSnap (snapPathIndex);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
void TextNodeHandler::_GetTextPartIds (ElementHandleCR eh, ITextQueryOptionsCR options, T_ITextPartIdPtrVectorR textPartIds) const
    {
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return; }
    
    // componentCount is at the same offset for 2D and 3D.
    // Text elements with empty strings should be considered invalid, so only considering component count for empty (e.g. empty text node placeholders).
    if (!options.ShouldIncludeEmptyParts () && (0 == eh.GetElementCP ()->ToText_node_2d().componentCount))
        return;
    
    // Text elements can only ever have a single text part, thus we don't need to squirrel any special data away.
    textPartIds.push_back (ITextPartId::Create ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   02/06
//---------------------------------------------------------------------------------------
Handler::PreActionStatus TextNodeHandler::_OnModify (EditElementHandleR eeh, ElementHandleCR eh)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        return PRE_ACTION_Ok;

    ChildElemIter       oldElm (eh, ExposeChildrenReason::Count);
    ChildEditElemIter   newElm (eeh, ExposeChildrenReason::Count);

    for (; oldElm.IsValid () && newElm.IsValid (); oldElm = oldElm.ToNext (), newElm = newElm.ToNext ())
        {
        if (TEXT_ELM != oldElm.GetLegacyType() || TEXT_ELM != newElm.GetLegacyType())
            return PRE_ACTION_Ok;

        if (0 != newElm.GetElementCP()->GetElementId().GetValue())
            continue;

        // if the old element had any xattributes, then we can't reuse its ID, the xattributes will be in the changeset
        // (and may have moved to another element). TR#209034
        XAttributeHandle anyXa = XAttributeCollection(oldElm.GetElementRef()).begin();
        if (anyXa.IsValid ())
            continue;

        newElm.GetElementP ()->SetElementId(oldElm.GetElementCP()->GetElementId());
        }

    return PRE_ACTION_Ok;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/2008
//---------------------------------------------------------------------------------------
StatusInt TextNodeHandler::_ComputeAnnotationScaledRange (ElementHandleCR eh, DRange3dR elemRangeOut, double scaleFactor)
    {
    if (!GetAnnotationScale (NULL, eh))
        return ERROR;
    
    DPoint3d origin;
    _GetTransformOrigin (eh, origin);
    
    return ComputeAnnotationScaledRange (eh, elemRangeOut, scaleFactor, &origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Deepak.Malkan                   09/2007
//---------------------------------------------------------------------------------------
bool TextNodeHandler::_GetAnnotationScale (double* annotationScale, ElementHandleCR eh) const
    {
    TextParamWide textParams;
    if (SUCCESS != TextNodeHandler::GetTextParams (eh, textParams))
        return false;
    
    if (NULL != annotationScale)
        *annotationScale = textParams.annotationScale;

    return textParams.exFlags.annotationScale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   KeithBentley    04/01
//---------------------------------------------------------------------------------------
void TextNodeHandler::VisitNodeChildren (ElementHandleCR eh, ViewContextR context)
    {
    ViewContext::ContextMark mark (&context);

    // Display by drawing children... don't check range when creating cache...
    UseChildren (eh, context, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   01/07
//---------------------------------------------------------------------------------------
ReprojectStatus TextNodeHandler::_OnGeoCoordinateReprojection (EditElementHandleR eeh, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    IGeoCoordinateReprojectionSettingsP settings     = reprojectionHelper.GetSettings ();
    bool                                doIndividual = reprojectionHelper.ShouldStroke (eeh, settings->DoMultilineTextElementsIndividually ());

    // If the "DoMultilineTextElementsIndividually" setting indicates, we apply the geographic reprojection to each child text element.
    if (doIndividual)
        {
        ReprojectStatus status = REPROJECT_Success;
        for (ChildEditElemIter childIter (eeh, ExposeChildrenReason::Count); childIter.IsValid (); )
            {
            ChildEditElemIter nextChild = childIter.ToNext();

            DisplayHandlerP dispHandler;
            if (NULL != (dispHandler = childIter.GetDisplayHandler ()))
                {
                ReprojectStatus childStatus;
                if (REPROJECT_Success != (childStatus = dispHandler->GeoCoordinateReprojection (childIter, reprojectionHelper, inChain)))
                    status = childStatus;

                dispHandler->ValidateElementRange(childIter);
                }
            }
        
        return status;
        }
    else
        {
        DPoint3d origin;
        _GetTransformOrigin (eeh, origin);

        TransformInfo   transform;
        ReprojectStatus status = reprojectionHelper.GetLocalTransform (&transform.GetTransformR(), origin, NULL, settings->RotateText(), settings->ScaleText());
        if ( (REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != status) )
            return status;

        // if can't transform, don't change what we have.
        if (SUCCESS != _ApplyTransform (eeh, transform))
            return REPROJECT_NoChange;

        return status;
        }
    }



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextElemHandler ------------------------------------------------------------------------------------------------------------- TextElemHandler --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IAnnotationHandlerP TextElemHandler::_GetIAnnotationHandler         (ElementHandleCR)                               { return this; }
void                TextElemHandler::_GetSnapOrigin                 (ElementHandleCR el, DPoint3dR origin)          { _GetTransformOrigin (el, origin); }
BentleyStatus       TextElemHandler::InitTextString                 (ElementHandleCR eh, TextStringR textString)    { return _InitTextString (eh, textString); }

//---------------------------------------------------------------------------------------
//! Given a text element, will edit all properties that CANNOT affect layout. This includes calling super's _EditProperties.
//! This should be called by both the text element's edit properties mechanism, as well as text node's edit properties mechanism when processing children (as text elements that are children of a text node cannot process layout-dependent properties themselves as that can turn them into text nodes).
// @bsimethod                                                   Jeff.Marker     02/08
//---------------------------------------------------------------------------------------
ProcessTextPropertiesStatus TextElemHandler::ProcessPropertiesByElement (EditElementHandleR eeh, PropertyContextR context)
    {
    UInt32 originalColor = eeh.GetElementCP ()->GetSymbology().color;

    T_Super::_EditProperties (eeh, context);

    ProcessTextPropertiesStatus status = TextHandlerBase::ProcessTextPropertiesByElement (context, eeh, &eeh, &originalColor);

    DisplayHandler::EditThicknessProperty (eeh, context);
    
    return status;
    }

//---------------------------------------------------------------------------------------
//! Edits all properties of a text element, including layout-dependent properties. This should NOT be called by text node as this can turn text elements into text nodes (and text nodes cannot have child text nodes).
// @bsimethod                                                   Brien.Bastings  03/07
//---------------------------------------------------------------------------------------
void TextElemHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    if (ProcessTextPropertiesStatus_All != ProcessPropertiesByElement (eeh, context))
        TextHandlerBase::ProcessLayoutPropertiesByTextBlock (eeh, context);
    
    // Only do this if the element has already been changed.
    if (context.GetElementChanged ())
        TextHandlerBase::SetOverridesFromStyle (context, eeh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   02/05
//---------------------------------------------------------------------------------------
void TextElemHandler::_GetDescription (ElementHandleCR eh, WStringR descriptionString, UInt32 desiredLength)
    {
    T_Super::_GetDescription (eh, descriptionString, desiredLength);

    if (descriptionString.size () > desiredLength)
        {
        descriptionString.resize (desiredLength);
        return;
        }

    UInt32 ohead = static_cast<UInt32>(descriptionString.size () + 5);
    if (ohead > desiredLength)
        return;

    WString textString;
    if (SUCCESS != TextElemHandler::GetString (eh, textString))
        return;
    
    if ((descriptionString.size () + textString.size ()) > desiredLength)
        {
        textString.resize (desiredLength - ohead);
        textString.append (L"...");
        }
    
    descriptionString.append (L": ").append (textString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  07/05
//---------------------------------------------------------------------------------------
void TextElemHandler::_GetTypeName (WStringR typeName, UInt32 desiredLength)
    {
    typeName.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_TEXT_ELM), 0, desiredLength);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  02/04
//---------------------------------------------------------------------------------------
bool TextElemHandler::_IsTransformGraphics (ElementHandleCR eh, TransformInfoCR transform)
    {
    return canUseTransformGraphics (eh, transform, this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/08
//---------------------------------------------------------------------------------------
void TextElemHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Check for flatten transform already applied by text node...
    if (0.0 != flattenDir.magnitude ())
        {
        TransformInfo tInfo (flattenTrans);

        eeh.GetHandler().ApplyTransform (eeh, tInfo);
        }

    DgnV8ElementBlank   elm;
    DgnElementCP org = eeh.GetElementCP ();

    org->CopyTo (elm);

    // convert rotation, height and length
    Text_2d& text2d = elm.ToText_2dR();
    TextHandlerBase::ConvertTextParamsTo2D (text2d.lngthmult, text2d.hghtmult, text2d.rotationAngle, org->ToText_3d().lngthmult, org->ToText_3d().hghtmult, *org->ToText_3d().quat);
    
    text2d.length      = org->ToText_3d().length;
    text2d.height      = org->ToText_3d().height;
    text2d.font        = org->ToText_3d().font;
    text2d.just        = org->ToText_3d().just;
    text2d.numchars    = org->ToText_3d().numchars;
    text2d.edflds      = org->ToText_3d().edflds;

    DataConvert::Points3dTo2d (&text2d.origin, &org->ToText_3d().origin, 1);

    int numBytes = elm.ToText_3d().numchars;

    // copy string
    memcpy (text2d.string, org->ToText_3d().string, numBytes);

    // copy Ed Fields
    char*       newtextEnd = text2d.string + numBytes;
    char const* oldtextEnd = org->ToText_3d().string + numBytes;

    memcpy (newtextEnd, oldtextEnd, 3 * text2d.edflds);

    int size = offsetof (Text_2d, string) + numBytes + 3 * text2d.edflds;

    text2d.SetSizeWordsNoAttributes((size + 1) / 2);
    
    ElementUtil::CopyAttributes (&elm, org);

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }
 

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/08
//---------------------------------------------------------------------------------------
void TextElemHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;
    DgnElementCP org = eeh.GetElementCP ();

    org->CopyTo (elm);

    Text_3d& text3d = elm.ToText_3dR();
    text3d.SetSizeWordsNoAttributes(offsetof (Text_3d, string) / 2);

    Text_2d const& text2d = org->ToText_2d();
    text3d.lngthmult   = text2d.lngthmult;
    text3d.hghtmult    = text2d.hghtmult;
    text3d.length      = text2d.length;
    text3d.height      = text2d.height;
    text3d.font        = text2d.font;
    text3d.just        = text2d.just;
    text3d.numchars    = text2d.numchars;
    text3d.edflds      = text2d.edflds;

    DataConvert::RotationToQuaternion (text3d.quat, text2d.rotationAngle);
    DataConvert::Points2dTo3d (&text3d.origin, &text2d.origin, 1, elevation);

    // memcpy the text and edfields
    int wordsToCopy = org->GetAttributeOffset() - offsetof (Text_2d, string) / 2;

    memcpy (text3d.string, text2d.string, 2 * wordsToCopy);

    text3d.IncrementSizeWords(wordsToCopy);

    ElementUtil::CopyAttributes (&elm, org);

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

//StatusInt TextElemHandler::_OnPostprocessCopyRemapRestore (EditElementHandleR eeh, DgnModelP modelRef) removed in graphite
//StatusInt TextElemHandler::_OnPreprocessCopy (EditElementHandleR eeh, CopyContextP ccP) removed in graphite

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     07/86
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::TransformTextElement (EditElementHandleR eeh, TransformCR transform, UInt32 options)
    {
    DPoint3d        origin;         if (SUCCESS != TextElemHandler::GetElementOrigin    (eeh, origin))      return ERROR;
    RotMatrix       orientation;    if (SUCCESS != TextElemHandler::GetOrientation      (eeh, orientation)) return ERROR;
    DPoint2d        textSize;       if (SUCCESS != TextElemHandler::GetFontSize         (eeh, textSize))    return ERROR;
    TextParamWide   textParams;     if (SUCCESS != TextElemHandler::GetTextParams       (eeh, textParams))  return ERROR;
    
    bool shouldScaleTextSize = (!textParams.exFlags.annotationScale || !(options & TRANSFORM_OPTIONS_AnnotationSizeMatchSource));
    
    transform.multiply (&origin);
    
    DPoint2d scaleFactor;
    
    TextString::TransformOrientationAndGetScale (scaleFactor, orientation, NULL, transform, true);
    
    if (shouldScaleTextSize)
        {
        textSize.x*= scaleFactor.x;
        textSize.y*= scaleFactor.y;
        eeh.GetElementP()->ToText_2dR().length  *= scaleFactor.x;
        eeh.GetElementP()->ToText_2dR().height  *= scaleFactor.y;
        
        textParams.ApplyScaleFactor (scaleFactor, false, true);
        }
    
    if (SUCCESS != TextElemHandler::SetElementOrigin    (eeh, origin, false))       return ERROR;
    if (SUCCESS != TextElemHandler::SetOrientation      (eeh, orientation, false))  return ERROR;
    if (SUCCESS != TextElemHandler::SetFontSize         (eeh, textSize, false))     return ERROR;
    if (SUCCESS != TextElemHandler::SetTextParams       (eeh, textParams, false))   return ERROR;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/03
//---------------------------------------------------------------------------------------
StatusInt TextElemHandler::_OnTransform (EditElementHandleR eeh, TransformInfoCR transform)
    {
    // Propagate annotation scale
    TransformInfo localTransInfo = transform;

    adjustTransformForDisableRotateCharacters (localTransInfo, eeh);

    if (localTransInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale)
        TextHandlerBase::UpdateAnnotationScale (eeh, localTransInfo.GetAnnotationScaleAction (), localTransInfo.GetAnnotationScale (), false);

    // NEEDS_WORK: Need to handle the no size change option

    // Handle special mirroring case
    if (hasMirrorTransform (eeh, localTransInfo))
        {
        TextBlock textBlock (eeh);
        textBlock.ApplyTransform (*localTransInfo.GetTransform ());
        textBlock.Reprocess ();
        
        EditElementHandle potentialReplacement;
        if (TEXTBLOCK_TO_ELEMENT_RESULT_Success != TextHandlerBase::CreateElement (potentialReplacement, &eeh, textBlock))
            return ERROR;
        
        eeh.ReplaceElementDescr (potentialReplacement.ExtractElementDescr().get());
        
        return SUCCESS;
        }
        
    // Modify the transform to handle the special transform of annotation text that should not be scaled
    if (!canScaleAnnotationText (eeh, localTransInfo, this))
        {
        // NEEDS_THOUGHT: I am going to use the mdlText_directCreateWide function to extract the user origin
        // for the text element and do this unscaling about the user origin.  However, I really dont
        // like the way this is done.  I think this should move into the above block that applies the
        // annotation scale.  I need to really reorganize this for 8.9.4
        TextBlock   textBlock           (eeh);
        DPoint3d    userOrigin          = textBlock.GetUserOrigin ();
        Transform   unscaledTransform;
        
        LegacyMath::TMatrix::Unscale (&unscaledTransform, localTransInfo.GetTransform (), &userOrigin);
        localTransInfo.GetTransformR() = unscaledTransform;        
        }

    // Don't change rotation of view independent text from a clone operation - leave it in its original rotation. TR#173478
    bool    honorViewIndependence = (eeh.GetElementCP()->IsViewIndependent() && (localTransInfo.GetOptions () & TRANSFORM_OPTIONS_FromClone));
    double  oldQuat[4];
    double  oldRotation = 0.0;

    if (honorViewIndependence)
        {
        if (DisplayHandler::Is3dElem (eeh.GetElementCP ()))
            memcpy (oldQuat, eeh.GetElementCP()->ToText_3d().quat, sizeof (oldQuat));
        else
            oldRotation = eeh.GetElementCP()->ToText_2d().rotationAngle;
        }

    // Standard text transformation. The special transform of annotation text that should not be scaled is handled within.
    if (SUCCESS != TextElemHandler::TransformTextElement (eeh, *localTransInfo.GetTransform (), localTransInfo.GetOptions ()))
        return ERROR;
    
    if (honorViewIndependence)
        {
        if (DisplayHandler::Is3dElem (eeh.GetElementCP ()))
            memcpy (eeh.GetElementP ()->ToText_3dR().quat, oldQuat, sizeof (oldQuat));
        else
            eeh.GetElementP ()->ToText_2dR().rotationAngle = oldRotation;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   01/07
//---------------------------------------------------------------------------------------
ReprojectStatus TextElemHandler::_OnGeoCoordinateReprojection (EditElementHandleR eeh, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    IGeoCoordinateReprojectionSettingsP settings    = reprojectionHelper.GetSettings ();
    DPoint3d                            origin;
    
    _GetTransformOrigin (eeh, origin);

    TransformInfo transform;
    ReprojectStatus status = reprojectionHelper.GetLocalTransform (&transform.GetTransformR(), origin, NULL, settings->RotateText(), settings->ScaleText());
    if ( (REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != status) )
        return status;

    // if can't transform, don't change what we have.
    if (SUCCESS != _ApplyTransform (eeh, transform))
        return REPROJECT_NoChange;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  03/07
//---------------------------------------------------------------------------------------
void TextElemHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    T_Super::_QueryProperties (eh, context);

    TextHandlerBase::ProcessTextPropertiesByElement (context, eh, NULL, NULL);

    DisplayHandler::QueryThicknessProperty (eh, context);
    }

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  11/09
//=======================================================================================
struct          TextThicknessStroker : IStrokeForCache, IDgnGlyphLayoutListener
{
ViewContextP    m_context;
DVec3d          m_textAxes[2];
bool            m_isCapped;
DVec3d          m_thicknessVector;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  11/09
//---------------------------------------------------------------------------------------
virtual UInt32 _OnFontAnnounced (TextStringCR text) override {return 0;}
virtual bool _DidCacheGlyphs () override {return true;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  11/09
//---------------------------------------------------------------------------------------
virtual void _OnGlyphAnnounced (DgnFontCR font, DgnGlyphCR glyph, DPoint3dCR glyphOffset) override
    {
    GPArraySmartP   gpa;

    if (SUCCESS != glyph.FillGpa (gpa))
        return;

    int         count = gpa->GetCount ();

    if (0 == count)
        return;

    Transform   offsetTrans;

    offsetTrans.initFrom (&glyphOffset);

    DVec3d      zVec;
    DPoint3d    origin;
    Transform   scaledTrans, compoundTrans;

    zVec.init (0.0, 0.0, 1.0);
    origin.init (0.0, 0.0, 0.0);
    scaledTrans.initFromOriginAndVectors (&origin, &m_textAxes[0], &m_textAxes[1], &zVec);
    compoundTrans.productOf (&offsetTrans, &scaledTrans);
    gpa->Transform (&compoundTrans);

    IDrawGeomR      output = m_context->GetIDrawGeom ();
    bool            isFilled = (0 != gpa->GetArrayMask (HPOINT_ARRAYMASK_FILL));
    BentleyStatus   status = SUCCESS;

    if (isFilled)
        {
        CurveVectorPtr      curves = gpa->ToCurveVector ();
        DgnExtrusionDetail  detail (curves, m_thicknessVector, true && curves->IsAnyRegionType ());
        ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

        output.DrawSolidPrimitive (*primitive);
        return;
        }

    for (int i=0; i<count && SUCCESS == status; )
        {
        switch (gpa->GetCurveType (i))
            {
            case GPCurveType::Ellipse:
                {
                DEllipse3d  ellipse;

                if (SUCCESS != (status = gpa->GetEllipse (&i, &ellipse)))
                    break;

                CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

                curves->push_back (ICurvePrimitive::CreateArc (ellipse));

                DgnExtrusionDetail  detail (curves, m_thicknessVector, false);
                ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

                output.DrawSolidPrimitive (*primitive);
                break;
                }

            case GPCurveType::Bezier:
            case GPCurveType::BSpline:
                {
                MSBsplineCurve  bcurve;

                if (SUCCESS != (status = gpa->GetBCurve (&i, &bcurve)))
                    break;

                CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

                curves->push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));

                DgnExtrusionDetail  detail (curves, m_thicknessVector, false);
                ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

                output.DrawSolidPrimitive (*primitive);

                bcurve.ReleaseMem ();
                break;
                }

            case GPCurveType::LineString:
                {
                size_t  i0  = (size_t)i;
                size_t  i1;

                gpa->IsLineString (i0, i1);

                int                     nPoints = (int)(i1 - i0) + 1;
                ScopedArray<DPoint3d>   points ((size_t) nPoints);

                if (SUCCESS != (status = gpa->GetLineString (&i, points.GetData (), &nPoints, nPoints)))
                    break;

                CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

                curves->push_back (ICurvePrimitive::CreateLineString (points.GetData (), nPoints));

                DgnExtrusionDetail  detail (curves, m_thicknessVector, false);
                ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnExtrusion (detail);

                output.DrawSolidPrimitive (*primitive);
                break;
                }

            default:
                {
                BeAssert (false);
                i++;
                break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  11/09
//---------------------------------------------------------------------------------------
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR   eh = *dh.GetElementHandleCP ();
    TextElemHandler*  textHandler;
    
    if (NULL == (textHandler = dynamic_cast <TextElemHandler*> (&eh.GetHandler ())))
        return;

    TextString  text;

    if (SUCCESS != textHandler->InitTextString (eh, text) || text.IsBlankString ())
        return;

    Transform   drawTrans;

    text.GetDrawTransform (drawTrans, false);
    context.PushTransform (drawTrans);

    DVec3dCP    thicknessVector = context.GetCurrentDisplayParams ()->GetThickness (m_isCapped);
    
    // Need thickness vector in glyph coordinates...
    if (thicknessVector)
        drawTrans.MultiplyTransposeMatrixOnly (m_thicknessVector, *thicknessVector);
    else
        m_thicknessVector = DVec3d::From (0.0, 0.0, 1.0);

    m_context = &context;

    // NOTE: Need text axes to compute gpa transform in OnGlyphAnnounced...
    text.GetProperties().GetAxes (m_textAxes[0], m_textAxes[1]);
    text.LoadGlyphs (this);

    context.PopTransformClip ();
    }

}; // TextThicknessStroker

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  11/09
//---------------------------------------------------------------------------------------
void TextElemHandler::DrawTextWithThickness (ElementHandleCR eh, ViewContextR context, UInt32 qvIndex)
    {
    TextThicknessStroker stroker;

    context.DrawWithThickness (eh, stroker, qvIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   11/04
//---------------------------------------------------------------------------------------
void TextElemHandler::_Draw (ElementHandleCR eh, ViewContextR context)
    {
    if (context.GetViewFlags () && context.GetViewFlags ()->fast_text)
        return;

    UInt32 info = context.GetDisplayInfo (IsRenderable (eh));
    
    // This block of code needs to be reviewed for annotation scale.
    if (0 != (DISPLAY_INFO_Thickness & info))
        {
        DrawTextWithThickness (eh, context, 1);
        return;
        }

    if (0 == (DISPLAY_INFO_Edge & info))
        return;

    // Always set mark in case of viewindependent text: restored in destructor!
    ViewContext::ContextMark mark(&context);
    pushVITextTransform (eh, context);

    StrokeTextElm textStroker;
    StrokeAnnotationElm annotationStroker (this, eh, context, &textStroker);
    annotationStroker.DrawUsingContext ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   11/04
//---------------------------------------------------------------------------------------
void TextElemHandler::_DrawFiltered (ElementHandleCR eh, ViewContextR context, DPoint3dCP pts, double size)
    {
    if (context.GetViewFlags () && context.GetViewFlags ()->fast_text)
        return;

    if (FILTER_LOD_ShowNothing == context.GetFilterLODFlag ())
        return;

    TextString textString;
    if (SUCCESS != InitTextString (eh, textString) || textString.IsBlankString ())
        return;
    
    T_Super::_DrawFiltered (eh, context, pts, size);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  06/09
//---------------------------------------------------------------------------------------
bool TextElemHandler::_IsPlanar (ElementHandleCR eh, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal)
    {
    if (normal)
        {
        RotMatrix rMatrix;
        _GetOrientation (eh, rMatrix);
        
        rMatrix.getColumn (normal, 2);
        }

    if (point)
        _GetTransformOrigin (eh, *point);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   04/06
//---------------------------------------------------------------------------------------
void TextElemHandler::_GetElemDisplayParams (ElementHandleCR eh, ElemDisplayParams& params, bool wantMaterials)
    {
    // This method is only needed for setting the override value for weight when hiliting and flashing with antialias text off.
    T_Super::_GetElemDisplayParams (eh, params, wantMaterials);

    DgnElementCP  textEl = eh.GetElementCP();
    UInt32       fontNum = DisplayHandler::Is3dElem (textEl) ? textEl->ToText_3d().font : textEl->ToText_2d().font;

    DgnFontCP font = DgnFontManager::ResolveFont (fontNum, eh.GetDgnModelP()->GetDgnProject(), DGNFONTVARIANT_DontCare);

    // The "true type" test is bogus. We really mean "is filled" but we can't tell that from the font number.
    if (DgnFontType::TrueType == font->GetType())
        params.SetWeight (0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   01/07
//---------------------------------------------------------------------------------------
void TextElemHandler::_GetOrientation (ElementHandleCR eh, RotMatrixR rotation)
    {
    DgnElementCP text = eh.GetElementCP ();
    
    if (DisplayHandler::Is3dElem (text))
        rotation.InitTransposedFromQuaternionWXYZ ( text->ToText_3d().quat);
    else
        rotation.InitFromAxisAndRotationAngle(2,  text->ToText_2d().rotationAngle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Keith.Bentley                   01/07
//---------------------------------------------------------------------------------------
void TextElemHandler::_GetTransformOrigin (ElementHandleCR eh, DPoint3dR origin)
    {
    origin.Zero ();

    // Old cold trusted what was cached on the element... could use a TextString to re-measure, but err on the side of performance for now.
    
    DPoint3d        userOriginOffset;
    DPoint2d        cachedSize          = { eh.GetElementCP ()->ToText_2d().length, eh.GetElementCP ()->ToText_2d().height };
    TextParamWide   textParams;

    if (SUCCESS != GetTextParams (eh, textParams))
        { BeAssert (false); return; }
    
    DgnFontCP font = DgnFontManager::ResolveFont (textParams.font, eh.GetDgnModelP ()->GetDgnProject (), DGNFONTVARIANT_DontCare);

    TextString::ComputeUserOriginOffset (userOriginOffset, cachedSize, 0.0, static_cast<TextElementJustification>(textParams.just), TO_BOOL (textParams.flags.vertical), eh.GetLegacyType(), font);

    if (SUCCESS != GetElementOrigin (eh, origin))
        { BeAssert (false); return; }
    
    RotMatrix   rMatrix;

    _GetOrientation (eh, rMatrix);
    rMatrix.Multiply (userOriginOffset);
    origin.Add (userOriginOffset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  10/04
//---------------------------------------------------------------------------------------
SnapStatus TextElemHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    return context->DoTextSnap (snapPathIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2009
//---------------------------------------------------------------------------------------
void TextElemHandler::_GetTextPartIds (ElementHandleCR, ITextQueryOptionsCR options, T_ITextPartIdPtrVectorR textPartIds) const
    {
    // Text elements should never have a blank string (for purposes of options.ShouldIncludeEmptyParts ()).
    //  Additionally, for this purpose, I don't (currently) care if we happen to encounter one (vs. the expense of digging out the string).
    
    // Text elements can only ever have a single text part, thus we don't need to squirrel any special data away.
    textPartIds.push_back (ITextPartId::Create ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      04/2008
//---------------------------------------------------------------------------------------
StatusInt TextElemHandler::_ComputeAnnotationScaledRange (ElementHandleCR eh, DRange3dR elemRangeOut, double scaleFactor)
    {
    if (!GetAnnotationScale (NULL, eh))
        return ERROR;
    
    DPoint3d origin;
    _GetTransformOrigin (eh, origin);
    
    return ComputeAnnotationScaledRange (eh, elemRangeOut, scaleFactor, &origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Deepak.Malkan                   09/2007
//---------------------------------------------------------------------------------------
bool TextElemHandler::_GetAnnotationScale (double* annotationScale, ElementHandleCR eh) const
    {
    TextParamWide textParams;
    if (SUCCESS != TextElemHandler::GetTextParams (eh, textParams))
        return false;

    if (annotationScale)
        *annotationScale = textParams.annotationScale;

    return textParams.exFlags.annotationScale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
void TextHandlerBase::ConvertTextParamsTo2D (double& lngthMult, double& hghtMult, double& rotation, double oldLngthMult, double oldHghtMult, double const & quaternion)
    {
    RotMatrix rMatrix;
    rMatrix.InitTransposedFromQuaternionWXYZ ( &quaternion);

    rotation = Angle::Atan2 (rMatrix.form3d[1][0], rMatrix.form3d[0][0]);
    
    double  sign    = ((rMatrix.form3d[2][2] < 0.0) ? -1.0 : 1.0);
    double  mult;
    
    mult = sqrt((rMatrix.form3d[0][0] * rMatrix.form3d[0][0]) + (rMatrix.form3d[1][0] * rMatrix.form3d[1][0]));
    lngthMult = (mult * oldLngthMult);

    mult = sqrt((rMatrix.form3d[0][1] * rMatrix.form3d[0][1]) + (rMatrix.form3d[1][1] * rMatrix.form3d[1][1]));
    hghtMult = (mult * oldHghtMult * sign);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextHandlerBase::UpdateAnnotationScale (EditElementHandleR eeh, AnnotationScaleAction action, double newAnnotationScale, bool forceTextNode)
    {
    if (!eeh.IsValid ())
        return ERROR;

    TextBlock textBlock (eeh);
    textBlock.SetTextPrimaryOriginType (ORIGIN_TYPE_User);

    TextBlockPropertiesCR tbProps = textBlock.GetProperties ();

    switch (action)
        {
        case AnnotationScaleAction::Add:
            {
            if (tbProps.HasAnnotationScale () && (newAnnotationScale == tbProps.GetAnnotationScale ()))
                return ERROR;
            
            TextBlockPropertiesPtr updatedTBProps = tbProps.Clone ();
            updatedTBProps->SetAnnotationScale (newAnnotationScale);
            textBlock.SetProperties (*updatedTBProps);
            
            break;
            }
        
        case AnnotationScaleAction::Update:
            {
            if (newAnnotationScale == tbProps.GetAnnotationScale ())
                return ERROR;
            
            TextBlockPropertiesPtr updatedTBProps = tbProps.Clone ();
            updatedTBProps->SetAnnotationScale (newAnnotationScale);
            textBlock.SetProperties (*updatedTBProps);
            
            break;
            }
       
        case AnnotationScaleAction::Remove:
            {
            if (!tbProps.HasAnnotationScale () && (1.0 == tbProps.GetAnnotationScale ()))
                return ERROR;
            
            TextBlockPropertiesPtr updatedTBProps = tbProps.Clone ();
            updatedTBProps->ClearAnnotationScale ();
            textBlock.SetProperties (*updatedTBProps);
            
            break;
            }
        }

    textBlock.SetForceTextNodeFlag (forceTextNode);
    textBlock.Reprocess ();

    EditElementHandle newEeh;
    textBlock.ToElement (newEeh, eeh.GetDgnModelP (), &eeh);
    eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
bool TextHandlerBase::AreEqual (ElementHandleCR lhsEh, ElementHandleCR rhsEh)
    {
    if (lhsEh.GetLegacyType() != rhsEh.GetLegacyType())
        return false;

    if (TEXT_ELM == lhsEh.GetLegacyType())
        return TextElemHandler::AreEqual (lhsEh, rhsEh);
    
    if (TEXT_NODE_ELM != rhsEh.GetLegacyType())
        { BeAssert (false); return false; }
    
    if (lhsEh.GetElementCP ()->ToText_node_2d().componentCount != rhsEh.GetElementCP ()->ToText_node_2d().componentCount)
        return false;
    
    if (!TextNodeHandler::AreEqual (lhsEh, rhsEh))
        return false;
    
    for (ChildElemIter lhsIter (lhsEh, ExposeChildrenReason::Count), rhsIter (rhsEh, ExposeChildrenReason::Count);
        lhsIter.IsValid () && rhsIter.IsValid ();
        lhsIter = lhsIter.ToNext (), rhsIter = rhsIter.ToNext ())
        {
        if (!TextElemHandler::AreEqual (lhsIter, rhsIter))
            return false;
        }
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
bool TextNodeHandler::AreEqual (ElementHandleCR lhsEh, ElementHandleCR rhsEh)
    {
    if (TEXT_NODE_ELM != lhsEh.GetLegacyType() || TEXT_NODE_ELM != rhsEh.GetLegacyType())
        { BeAssert (false); return false; }
    
    DPoint3d lhsEhOrigin; if (SUCCESS != TextNodeHandler::GetUserOrigin (lhsEh, lhsEhOrigin)) { return false; }
    DPoint3d rhsEhOrigin; if (SUCCESS != TextNodeHandler::GetUserOrigin (rhsEh, rhsEhOrigin)) { return false; }
    
    if (!lhsEhOrigin.isEqual (&rhsEhOrigin))
        return false;
    
    RotMatrix lhsEhOrientation; if (SUCCESS != TextNodeHandler::GetOrientation (lhsEh, lhsEhOrientation)) { return false; }
    RotMatrix rhsEhOrientation; if (SUCCESS != TextNodeHandler::GetOrientation (rhsEh, lhsEhOrientation)) { return false; }
    
    if (!lhsEhOrientation.isEqual (&rhsEhOrientation))
        return false;
    
    DPoint2d lhsEhTextSize; if (SUCCESS != TextNodeHandler::GetFontSize (lhsEh, lhsEhTextSize)) { return false; }
    DPoint2d rhsEhTextSize; if (SUCCESS != TextNodeHandler::GetFontSize (rhsEh, lhsEhTextSize)) { return false; }
    
    if (!lhsEhTextSize.isEqual (&rhsEhTextSize))
        return false;
    
    TextParamWide lhsEhTextParams; if (SUCCESS != TextNodeHandler::GetTextParams (lhsEh, lhsEhTextParams)) { return false; }
    TextParamWide rhsEhTextParams; if (SUCCESS != TextNodeHandler::GetTextParams (rhsEh, rhsEhTextParams)) { return false; }
    
    if (0 != memcmp (&lhsEhTextParams, &rhsEhTextParams, sizeof (lhsEhTextParams)))
        return false;
    
    UInt16 lhsEhMaxLineLength; if (SUCCESS != TextNodeHandler::GetMaxCharsPerLine (lhsEh, lhsEhMaxLineLength)) { return false; }
    UInt16 rhsEhMaxLineLength; if (SUCCESS != TextNodeHandler::GetMaxCharsPerLine (rhsEh, rhsEhMaxLineLength)) { return false; }
    
    if (lhsEhMaxLineLength != rhsEhMaxLineLength)
        return false;
    
    AlongTextLinkageData lhsEhATData;
    AlongTextLinkageData rhsEhATData;
    
    if (TextNodeHandler::GetAlongTextData (lhsEh, lhsEhATData) != TextNodeHandler::GetAlongTextData (rhsEh, rhsEhATData))
        return false;
    
    if (0 != memcmp (&lhsEhATData, &rhsEhATData, sizeof (lhsEhATData)))
        return SUCCESS;
    
    WhiteSpaceBitmaskValueVector lhsEhWSValues;
    WhiteSpaceBitmaskValueVector rhsEhWSValues;
    
    if (TextNodeHandler::GetWhiteSpaceBitmaskValues (lhsEh, lhsEhWSValues) != TextNodeHandler::GetWhiteSpaceBitmaskValues (rhsEh, rhsEhWSValues))
        return false;
    
    WhiteSpaceBitmaskValueVector::size_type lhsEhWhiteSpaceCount = lhsEhWSValues.size ();
    
    if (lhsEhWhiteSpaceCount != rhsEhWSValues.size ())
        return false;
    
    if (lhsEhWhiteSpaceCount > 0)
        {
        for (WhiteSpaceBitmaskValueVector::size_type i = 0; i < lhsEhWhiteSpaceCount; ++i)
            {
            if (lhsEhWSValues[i] != rhsEhWSValues[i])
                return SUCCESS;
            }
        }
    
    IndentationData lhsEhIndentation;
    IndentationData rhsEhIndentation;
    
    if (TextNodeHandler::GetIndentationData (lhsEh, lhsEhIndentation) != TextNodeHandler::GetIndentationData (rhsEh, rhsEhIndentation))
        return false;
    
    if (!lhsEhIndentation.Equals (rhsEhIndentation))
        return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
bool TextElemHandler::AreEqual (ElementHandleCR lhsEh, ElementHandleCR rhsEh)
    {
    if (TEXT_ELM != lhsEh.GetLegacyType() || TEXT_ELM != rhsEh.GetLegacyType())
        { BeAssert (false); return false; }
    
    if (DisplayHandler::Is3dElem (lhsEh.GetElementCP ()) != DisplayHandler::Is3dElem (rhsEh.GetElementCP ()))
        return false;
    
    DPoint3d lhsEhOrigin; if (SUCCESS != TextElemHandler::GetElementOrigin (lhsEh, lhsEhOrigin)) { return false; }
    DPoint3d rhsEhOrigin; if (SUCCESS != TextElemHandler::GetElementOrigin (rhsEh, rhsEhOrigin)) { return false; }
    
    if (!lhsEhOrigin.isEqual (&rhsEhOrigin))
        return false;
    
    RotMatrix lhsEhOrientation; if (SUCCESS != TextElemHandler::GetOrientation (lhsEh, lhsEhOrientation)) { return false; }
    RotMatrix rhsEhOrientation; if (SUCCESS != TextElemHandler::GetOrientation (rhsEh, lhsEhOrientation)) { return false; }
    
    if (!lhsEhOrientation.isEqual (&rhsEhOrientation))
        return false;
    
    DPoint2d lhsEhTextSize; if (SUCCESS != TextElemHandler::GetFontSize (lhsEh, lhsEhTextSize)) { return false; }
    DPoint2d rhsEhTextSize; if (SUCCESS != TextElemHandler::GetFontSize (rhsEh, lhsEhTextSize)) { return false; }
    
    if (!lhsEhTextSize.isEqual (&rhsEhTextSize))
        return false;
    
    TextParamWide lhsEhTextParams; if (SUCCESS != TextElemHandler::GetTextParams (lhsEh, lhsEhTextParams)) { return false; }
    TextParamWide rhsEhTextParams; if (SUCCESS != TextElemHandler::GetTextParams (rhsEh, rhsEhTextParams)) { return false; }
    
    if (0 != memcmp (&lhsEhTextParams, &rhsEhTextParams, sizeof (lhsEhTextParams)))
        return false;
    
    if (lhsEh.GetElementCP ()->ToText_2d().numchars != rhsEh.GetElementCP ()->ToText_2d().numchars)
        return false;
    
    CharCP  lhsEhString = NULL;
    CharCP  rhsEhString = NULL;
    
    if (DisplayHandler::Is3dElem (lhsEh.GetElementCP ()))
        {
        lhsEhString = lhsEh.GetElementCP ()->ToText_3d().string;
        rhsEhString = rhsEh.GetElementCP ()->ToText_3d().string;
        }
    else
        {
        lhsEhString = lhsEh.GetElementCP ()->ToText_2d().string;
        rhsEhString = rhsEh.GetElementCP ()->ToText_2d().string;
        }
    
    if (0 != memcmp (lhsEhString, rhsEhString, lhsEh.GetElementCP ()->ToText_2d().numchars))
        return false;
    
    EDFieldVector lhsEhEDFields; if (SUCCESS != TextElemHandler::GetEDFields (lhsEh, lhsEhEDFields)) { return false; }
    EDFieldVector rhsEhEDFields; if (SUCCESS != TextElemHandler::GetEDFields (rhsEh, rhsEhEDFields)) { return false; }
    
    EDFieldVector::size_type lhsEhNumEDFields = lhsEhEDFields.size ();
    
    if (lhsEhNumEDFields != rhsEhEDFields.size ())
        return false;
    
    if (lhsEhNumEDFields > 0)
        {
        for (EDFieldVector::size_type i = 0; i < lhsEhNumEDFields; ++i)
            if (0 != memcmp (&lhsEhEDFields[i], &rhsEhEDFields[i], sizeof (EDFieldVector::value_type)))
                return false;
        }
    
    WhiteSpaceBitmaskValueVector lhsEhWSValues;
    WhiteSpaceBitmaskValueVector rhsEhWSValues;
    
    if (TextNodeHandler::GetWhiteSpaceBitmaskValues (lhsEh, lhsEhWSValues) != TextNodeHandler::GetWhiteSpaceBitmaskValues (rhsEh, rhsEhWSValues))
        return false;
    
    WhiteSpaceBitmaskValueVector::size_type lhsEhWhiteSpaceCount = lhsEhWSValues.size ();
    
    if (lhsEhWhiteSpaceCount != rhsEhWSValues.size ())
        return false;
    
    if (lhsEhWhiteSpaceCount > 0)
        {
        for (WhiteSpaceBitmaskValueVector::size_type i = 0; i < lhsEhWhiteSpaceCount; ++i)
            {
            if (lhsEhWSValues[i] != rhsEhWSValues[i])
                return false;
            }
        }
    
    IndentationData lhsEhIndentation;
    IndentationData rhsEhIndentation;
    
    if (TextNodeHandler::GetIndentationData (lhsEh, lhsEhIndentation) != TextNodeHandler::GetIndentationData (rhsEh, rhsEhIndentation))
        return false;
    
    if (!lhsEhIndentation.Equals (rhsEhIndentation))
        return false;
    
    return true;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Generic Get/Set ------------------------------------------------------------------------------------------------------------- Generic Get/Set --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextHandlerBase::GetWhiteSpaceBitmaskValues (ElementHandleCR eh, WhiteSpaceBitmaskValueVectorR whiteSpaceValues)
    {
    whiteSpaceValues.clear ();
    
    BitMaskP whiteSpaceBitMask;
    if (SUCCESS != (BitMaskLinkage::ExtractBitMask (&whiteSpaceBitMask, eh.GetElementCP (), BITMASK_LINKAGE_KEY_TextWhiteSpace, 0)))
        return ERROR;
    
    // NOTE: Old code (pre-Beijing) had two paths for single- and double-bit bitmasks.
    //  This code did not expect a single-bit bitmask, would BeAssert if it found it, and had a comment saying it was a candidate for removal.
    //  Since I (or Chuck's longevity test) have not run into this BeAssert, I am actually removing it.
    //  Also prevents the need for requiring a TextParamWide as input to this method.
    
    int bitCount = whiteSpaceBitMask->GetCapacity ();
    
    for (int i = 0; i < bitCount; i+= 2)
        {
        bool    val1;
        bool    val2;

        val1 = whiteSpaceBitMask->Test( i);
        val2 = whiteSpaceBitMask->Test( i+1);
        
        int val = val1 * 2 + val2;
        
        whiteSpaceValues.push_back ((WhiteSpaceBitmaskValueVector::value_type)val);
        }

    whiteSpaceBitMask->Free ();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextHandlerBase::SetWhiteSpaceBitmaskValues (EditElementHandleR eeh, WhiteSpaceBitmaskValueVectorCP whiteSpaceValues)
    {
    // Regardless, attempt to delete the old linkage; no harm if it doesn't exist.
    BitMaskLinkage::DeleteBitMask (eeh.GetElementP (), BITMASK_LINKAGE_KEY_TextWhiteSpace, 0);
    
    if (NULL == whiteSpaceValues || 0 == whiteSpaceValues->size ())
        return SUCCESS;
    
    BitMaskP    whiteSpaceBitMask   = BitMask::Create (false);
    UInt32      bitIndex            = 0;
    
    for (WhiteSpaceBitmaskValue wsValue : *whiteSpaceValues)
        {
        switch (wsValue)
            {
            case WhiteSpaceBitmaskValue_Tab:
                whiteSpaceBitMask->SetBit (bitIndex++, 0);
                whiteSpaceBitMask->SetBit (bitIndex++, 0);
                break;
            
            case WhiteSpaceBitmaskValue_ParagraphBreak:
                whiteSpaceBitMask->SetBit (bitIndex++, 0);
                whiteSpaceBitMask->SetBit (bitIndex++, 1);
                break;
            
            case WhiteSpaceBitmaskValue_LineBreak:
                whiteSpaceBitMask->SetBit (bitIndex++, 1);
                whiteSpaceBitMask->SetBit (bitIndex++, 0);
                break;
            
            default:
                BeAssert (false);
                continue;
            }
        }
    
    DgnV8ElementBlank updatedElement;
    eeh.GetElementCP ()->CopyTo (updatedElement);
    
    if (whiteSpaceBitMask->GetCapacity () > 0)
        BitMaskLinkage::AppendBitMask (&updatedElement, BITMASK_LINKAGE_KEY_TextWhiteSpace, whiteSpaceBitMask);

    eeh.ReplaceElement (&updatedElement);

    whiteSpaceBitMask->Free ();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextHandlerBase::GetIndentationData (ElementHandleCR eh, IndentationDataR indentationData)
    {
    indentationData.Clear ();
    
    if (!TextIndentationLinkage::DoesElementHaveThisLinkage (eh))
        return ERROR;
    
    TextIndentationLinkage::FillIndentationDataFromLinkage (eh, indentationData);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextHandlerBase::SetIndentationData (EditElementHandleR eeh, IndentationDataCP indentationData)
    {
    TextIndentationLinkage::RemoveLinkages (eeh);
    
    if (NULL == indentationData)
        return SUCCESS;
    
    TextIndentationLinkage::AppendLinkageFromIndentationData (eeh, *indentationData);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Brien.Bastings  09/10
//---------------------------------------------------------------------------------------
StatusInt TextHandlerBase::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Text & geometry.GetOptions ()))
        {
        // Continue to support dropping text nodes to text provided only complex option is selected...
        if (0 == (DropGeometry::OPTION_Complex & geometry.GetOptions ()) || TEXT_NODE_ELM != eh.GetLegacyType())
            return ERROR;

        return ComplexHeaderDisplayHandler::DropComplex (eh, dropGeom);
        }

    // Text drop option shouldn't make a distinction between text and text nodes...
    return DropToElementDrawGeom::DoDrop (eh, dropGeom, geometry, DropGraphics ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
BentleyStatus TextHandlerBase::DropText (ElementHandleCR eh, ElementAgendaR agenda, RotMatrixCR orientation)
    {
    DisplayHandlerP dHandler = eh.GetDisplayHandler ();
    if (NULL == dHandler)
        return ERROR;

    if (SUCCESS != dHandler->Drop (eh, agenda, DropGeometry (DropGeometry::OPTION_Text)))
        return ERROR;

    // Only transform if was view-independent.
    if (!eh.GetElementCP()->IsViewIndependent())
        return SUCCESS;
    
    DPoint3d transformOrigin;
    dHandler->GetTransformOrigin (eh, transformOrigin);
                
    Transform transformData;
    transformData.InitFrom (orientation, transformOrigin);
                
    TransformInfo transform (transformData);
                
    for (EditElementHandleR droppedEeh: agenda)
        {
        DisplayHandlerP droppedDHandler = droppedEeh.GetDisplayHandler ();
        if (NULL == droppedDHandler)
            { BeAssert (false); continue; }

        if (SUCCESS != droppedDHandler->ApplyTransform (droppedEeh, transform))
            BeAssert (false);
        }
                
    return SUCCESS;        
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Text Node Get/Set --------------------------------------------------------------------------------------------------------- Text Node Get/Set --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetUserOrigin (ElementHandleCR eh, DPoint3dR userOrigin)
    {
    userOrigin.zero ();
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    if (DisplayHandler::Is3dElem (eh.GetElementCP ()))
        userOrigin = element.ToText_node_3d().origin;
    else
        userOrigin.init (&element.ToText_node_2d().origin);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetUserOrigin (EditElementHandleR eeh, DPoint3dCR userOrigin, bool reValidateRange)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    if (DisplayHandler::Is3dElem (&element))
        element.ToText_node_3dR().origin = userOrigin;
    else
        element.ToText_node_2dR().origin.init (&userOrigin);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange(eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetOrientation (ElementHandleCR eh, RotMatrixR orientation)
    {
    orientation.initIdentity ();
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    if (DisplayHandler::Is3dElem (&element))
        orientation.InitTransposedFromQuaternionWXYZ ( element.ToText_node_3d().quat);
    else
        orientation.InitFromAxisAndRotationAngle(2,  element.ToText_node_2d().rotationAngle);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetOrientation (EditElementHandleR eeh, RotMatrixCR orientation, bool reValidateRange)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    if (DisplayHandler::Is3dElem (&element))
        orientation.GetQuaternion(element.ToText_node_3dR().quat, true);
    else
        element.ToText_node_2dR().rotationAngle = orientation.ColumnXAngleXY ();
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetFontSize (ElementHandleCR eh, DPoint2dR textSize)
    {
    textSize.zero ();
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    textSize.setComponents (TextHandlerBase::ConvertMultToScale (element.ToText_node_2d().lngthmult), TextHandlerBase::ConvertMultToScale (element.ToText_node_2d().hghtmult));
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetFontSize (EditElementHandleR eeh, DPoint2dCR textSize, bool reValidateRange)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    element.ToText_node_2dR().lngthmult  = TextHandlerBase::ConvertScaleToMult (textSize.x);
    element.ToText_node_2dR().hghtmult   = TextHandlerBase::ConvertScaleToMult (textSize.y);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetTextParams (ElementHandleCR eh, TextParamWideR textParams)
    {
    textParams.Initialize ();
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    textParams.font             = element.ToText_node_2d().font;
    textParams.just             = element.ToText_node_2d().just;
    textParams.color            = element.ToText_node_2d().GetSymbology().color;
    textParams.viewIndependent  = element.ToText_node_2d().IsViewIndependent();
    textParams.lineSpacing      = element.ToText_node_2d().linespc;
    textParams.nodeNumber       = element.ToText_node_2d().nodenumber;
    
    TextHandlerBase::FillTextParamsFromLinkages (eh, textParams);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetTextParams (EditElementHandleR eeh, TextParamWide textParams, bool reValidateRange)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    // Enforce some constraints on the text parameters due to text node.
    textParams.exFlags.acadFittedText = false;
    
    // Although I hate this, it's been around a while, so continuing.
    if (!textParams.flags.codePage_deprecated || 0 == textParams.codePage_deprecated)
        {
        textParams.flags.codePage_deprecated    = true;
        LangCodePage cp;
        BeStringUtilities::GetCurrentCodePage (cp);
        textParams.codePage_deprecated          = (int)cp;
        }
    
    DgnElementR element = *eeh.GetElementP ();
    
    // Transfer parts from TextParamWide to element structure.
    element.SetViewIndependent(TO_BOOL(textParams.viewIndependent));
    
    if (textParams.exFlags.color)
        element.GetSymbologyR().color = textParams.color;
    
    element.ToText_node_2dR().nodenumber = textParams.nodeNumber;
    element.ToText_node_2dR().font       = static_cast<UInt16>(textParams.font);
    element.ToText_node_2dR().just       = static_cast<UInt16>(textParams.just);
    element.ToText_node_2dR().linespc    = textParams.lineSpacing;
    
    // Transfer parts from TextParamWide to linkages.
    //  Instead of wastefully sequentially removing/adding linkages (can cause re-allocs), do it on a single DgnElement.
    //  Additionally, even if we don't change size, re-allocing can still be bad if callers don't expect us to change size.
    DgnV8ElementBlank el;
    eeh.GetElementCP ()->CopyTo (el);
    
    TextHandlerBase::RemoveTextParamLinkages (el);
    TextHandlerBase::AppendLinkagesFromTextParams (el, textParams);
    
    eeh.ReplaceElement (&el);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetMaxCharsPerLine (ElementHandleCR eh, UInt16& maxCharsPerLine)
    {
    maxCharsPerLine = 0;
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    maxCharsPerLine = element.ToText_node_2d().maxlngth;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetMaxCharsPerLine (EditElementHandleR eeh, UInt16 maxCharsPerLine)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    element.ToText_node_2dR().maxlngth = maxCharsPerLine;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetNodeNumber (ElementHandleCR eh, UInt32& nodeNumber)
    {
    nodeNumber = 0;
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    nodeNumber = element.ToText_node_2d().nodenumber;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetNodeNumber (EditElementHandleR eeh, UInt32 nodeNumber)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    element.ToText_node_2dR().nodenumber = nodeNumber;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
bool TextNodeHandler::HasAlongTextData (ElementHandleCR eh)
    {
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        return false;
    
    DgnElementCR element = *eh.GetElementCP ();
    
    return (SUCCESS == DependencyManagerLinkage::GetLinkageFromMSElement (NULL, &element, ALONG_TEXT_DEPENDENCY_APP_ID, ALONG_TEXT_DEPENDENCY_APP_VALUE));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetAlongTextData (ElementHandleCR eh, AlongTextLinkageDataR alongTextData)
    {
    memset (&alongTextData, 0, sizeof (alongTextData));
    
    alongTextData.m___unusedref   = INVALID_ELEMENTID;
    alongTextData.m_rootId  = INVALID_ELEMENTID;
    
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR         element = *eh.GetElementCP ();
    DependencyLinkageAccessor linkage;
    
    if (SUCCESS != DependencyManagerLinkage::GetLinkageFromMSElement (&linkage, &element, ALONG_TEXT_DEPENDENCY_APP_ID, ALONG_TEXT_DEPENDENCY_APP_VALUE))
        return ERROR;
    
    alongTextData.m_rootId                  = linkage->root.far_e_v[0].s.elemid;
    alongTextData.m___unusedref             = 0;
    alongTextData.m_customDependencyData    = *((AlongTextLinkageData::CustomDependencyData*)&linkage->root.far_e_v[linkage->nRoots]);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetAlongTextData (EditElementHandleR eeh, AlongTextLinkageDataCP alongTextData)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    // Regardless, attempt to delete the old linkage; no harm if it doesn't exist.
    DependencyManagerLinkage::DeleteLinkage (eeh, ALONG_TEXT_DEPENDENCY_APP_ID, ALONG_TEXT_DEPENDENCY_APP_VALUE);
    
    if (NULL == alongTextData)
        return SUCCESS;
    
    DependencyLinkageP linkage = (DependencyLinkageP)_alloca (ALONG_TEXT_LINKAGE_SIZE);
    
    memset (linkage, 0, ALONG_TEXT_LINKAGE_SIZE);

    if (SUCCESS != DependencyManagerLinkage::InitLinkage (*linkage, ALONG_TEXT_DEPENDENCY_APP_ID, DEPENDENCY_DATA_TYPE_FAR_ELEM_ID_V, DEPENDENCY_ON_COPY_RemapRootsWithinSelection))
        return ERROR;

    linkage->appValue                  = ALONG_TEXT_DEPENDENCY_APP_VALUE;
    linkage->nRoots                    = ALONG_TEXT_MAX_NUMBER_OF_ROOTS;
    linkage->root.far_e_v[0].s.elemid  = alongTextData->m_rootId;

    AlongTextLinkageData::CustomDependencyData* extraData = (AlongTextLinkageData::CustomDependencyData*)&linkage->root.far_e_v[linkage->nRoots];
    
    *extraData = alongTextData->m_customDependencyData;
    extraData->m_parameters.m_areParametersUsed = 1;
    
    if (SUCCESS != DependencyManagerLinkage::AppendLinkage (eeh, *linkage, sizeof (*extraData)))
        return ERROR;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetWhiteSpaceBitmaskValues (ElementHandleCR eh, WhiteSpaceBitmaskValueVectorR whiteSpaceValues)
    {
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::GetWhiteSpaceBitmaskValues (eh, whiteSpaceValues);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetWhiteSpaceBitmaskValues (EditElementHandleR eeh, WhiteSpaceBitmaskValueVectorCP whiteSpaceValues)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::SetWhiteSpaceBitmaskValues (eeh, whiteSpaceValues);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::GetIndentationData (ElementHandleCR eh, IndentationDataR indentationData)
    {
    if (TEXT_NODE_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::GetIndentationData (eh, indentationData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::SetIndentationData (EditElementHandleR eeh, IndentationDataCP indentationData)
    {
    if (TEXT_NODE_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::SetIndentationData (eeh, indentationData);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Text Elem Get/Set --------------------------------------------------------------------------------------------------------- Text Elem Get/Set --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetElementOrigin (ElementHandleCR eh, DPoint3dR elementOrigin)
    {
    elementOrigin.zero ();
    
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    if (DisplayHandler::Is3dElem (&element))
        elementOrigin = element.ToText_3d().origin;
    else
        elementOrigin.init (&element.ToText_2d().origin);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetElementOrigin (EditElementHandleR eeh, DPoint3dCR elementOrigin, bool reValidateRange)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    if (DisplayHandler::Is3dElem (&element))
        element.ToText_3dR().origin = elementOrigin;
    else
        element.ToText_2dR().origin.init (&elementOrigin);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetOrientation (ElementHandleCR eh, RotMatrixR orientation)
    {
    orientation.initIdentity ();
    
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    if (DisplayHandler::Is3dElem (&element))
        orientation.InitTransposedFromQuaternionWXYZ ( element.ToText_3d().quat);
    else
        orientation.InitFromAxisAndRotationAngle(2,  element.ToText_2d().rotationAngle);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetOrientation (EditElementHandleR eeh, RotMatrixCR orientation, bool reValidateRange)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    if (DisplayHandler::Is3dElem (&element))
        orientation.GetQuaternion(element.ToText_3dR().quat, true);
    else
        element.ToText_2dR().rotationAngle = orientation.ColumnXAngleXY ();
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetFontSize (ElementHandleCR eh, DPoint2dR textSize)
    {
    textSize.zero ();
    
    if (TEXT_ELM != eh.GetLegacyType())
       { BeAssert (false); return ERROR; }
    
    DgnElementCR element = *eh.GetElementCP ();
    
    textSize.setComponents (TextHandlerBase::ConvertMultToScale (element.ToText_2d().lngthmult), TextHandlerBase::ConvertMultToScale (element.ToText_2d().hghtmult));
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetFontSize (EditElementHandleR eeh, DPoint2dCR textSize, bool reValidateRange)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    element.ToText_2dR().lngthmult   = TextHandlerBase::ConvertScaleToMult (textSize.x);
    element.ToText_2dR().hghtmult    = TextHandlerBase::ConvertScaleToMult (textSize.y);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetTextParams (ElementHandleCR eh, TextParamWideR textParams)
    {
    textParams.Initialize ();
    
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }

    DgnElementCR element = *eh.GetElementCP ();

    textParams.font             = element.ToText_2d().font;
    textParams.just             = element.ToText_2d().just;
    textParams.viewIndependent  = element.ToText_2d().IsViewIndependent();
    textParams.color            = element.ToText_2d().GetSymbology().color;

    TextHandlerBase::FillTextParamsFromLinkages (eh, textParams);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetTextParams (EditElementHandleR eeh, TextParamWide textParams, bool reValidateRange)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementR element = *eeh.GetElementP ();
    
    element.ToText_2dR().SetViewIndependent(TO_BOOL(textParams.viewIndependent));
    
    // This code page value is not strictly the code page of the element's string content; rather it is the codepage of the system when the element was created.
    //  The encoding of the element's string should always be that of the font, regardless of ACP. This value is horrible, but old versions will want it.
    if (!textParams.flags.codePage_deprecated || 0 == textParams.codePage_deprecated)
        {
        textParams.flags.codePage_deprecated    = true;
        LangCodePage cp;
        BeStringUtilities::GetCurrentCodePage (cp);    
        textParams.codePage_deprecated          = static_cast<int>(cp);
        }
    
    element.ToText_2dR().GetSymbologyR().color = textParams.color;
    element.ToText_2dR().font        = static_cast<UInt16>(textParams.font);
    element.ToText_2dR().just        = static_cast<UInt16>(textParams.just);
    
    // Instead of wastefully sequentially removing/adding linkages (can cause re-allocs), do it on a single DgnElement.
    //  Additionally, even if we don't change size, re-allocing can still be bad if callers don't expect us to change size.
    DgnV8ElementBlank el;
    eeh.GetElementCP ()->CopyTo (el);
    
    TextHandlerBase::RemoveTextParamLinkages (el);
    TextHandlerBase::AppendLinkagesFromTextParams (el, textParams);
    
    eeh.ReplaceElement (&el);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/12
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetFontChars (ElementHandleCR eh, T_FontChars& fontChars)
    {
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element     = *eh.GetElementCP ();
    CharCP      variString  = DisplayHandler::Is3dElem (&element) ? element.ToText_3d().string : element.ToText_2d().string;
    
    return VariCharConverter::VariCharToFontChar (fontChars, static_cast<VariCharCP>(variString), element.ToText_2d().numchars);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetString (ElementHandleCR eh, WStringR unicodeString)
    {
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnProjectP  project = eh.GetDgnProject();
    if (NULL == project)
        { BeAssert (false); return ERROR; }
    
    TextParamWide params;
    TextElemHandler::GetTextParams (eh, params);
    
    DgnFontCR fontForCodePage = DgnFontManager::GetFontForCodePage (params.font, params.shxBigFont, *project);
    
    return TextElemHandler::GetString (eh, unicodeString, fontForCodePage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetString (ElementHandleCR eh, WStringR unicodeString, DgnFontCR fontForCodePage)
    {
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element     = *eh.GetElementCP ();
    CharCP      variString  = DisplayHandler::Is3dElem (&element) ? element.ToText_3d().string : element.ToText_2d().string;
    
    bvector<FontChar> fontChars;
    if (SUCCESS != VariCharConverter::VariCharToFontChar (fontChars, static_cast<VariCharCP>(variString), element.ToText_2d().numchars))
        { BeAssert (false); return ERROR; }

    unicodeString = fontForCodePage.FontCharsToUnicodeString (&fontChars[0]);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetString (EditElementHandleR eeh, WCharCP unicodeString, TextParamWideCR textParams, DPoint2dCR textSize, bool reValidateRange)
    {
    if (TEXT_ELM != eeh.GetLegacyType() || NULL == eeh.GetDgnModelP () || NULL == unicodeString)
        { BeAssert (false); return ERROR; }
    
    size_t numChars = wcslen (unicodeString);
    
    if (0 == numChars)
        {
        BeAssert (false);
        return ERROR;
        }
    
    DgnProjectR  dgnFile = eeh.GetDgnModelP ()->GetDgnProject();
    DgnFontCR           fontForCodePage = DgnFontManager::GetFontForCodePage (textParams.font, textParams.shxBigFont, dgnFile);
    bvector<FontChar>   fontChars = fontForCodePage.UnicodeStringToFontChars (unicodeString);
    
    bvector<VariChar> variString;
    if (SUCCESS != VariCharConverter::FontCharToVariChar (variString, &fontChars[0], fontChars.size (), (LangCodePage::Unicode == fontForCodePage.GetCodePage ()), false))
        { BeAssert (false); return ERROR; }
    
    DgnV8ElementBlank updatedElement;
    eeh.GetElementCP ()->CopyTo (updatedElement);
    
    CharP elementString = DisplayHandler::Is3dElem (&updatedElement) ? updatedElement.ToText_3dR().string : updatedElement.ToText_2dR().string;
    
    EDFieldVector   edFields;
    size_t          elementEDFieldNumBytes  = 0;
    bool            didStringSizeChange     = (updatedElement.ToText_2d().numchars != variString.size ());
    
    if (didStringSizeChange)
        {
        if (SUCCESS != TextElemHandler::GetEDFields (eeh, edFields))
            return ERROR;
        
        elementEDFieldNumBytes = sizeof (EDFieldVector::value_type) * edFields.size ();
        
        updatedElement.ToText_2dR().numchars = static_cast<UInt16>(variString.size ());
        
        if (updatedElement.GetAttributeOffset() != updatedElement.GetSizeWords())
            {
            size_t      updatedCoreElementNumBytes  = (elementString - (CharP)&updatedElement) + variString.size () + elementEDFieldNumBytes;
            UInt32      existingLinkagesNumBytes    = (updatedElement.GetSizeWords() - updatedElement.GetAttributeOffset()) * 2;
            
            if (0 != (updatedCoreElementNumBytes % 2))
                ++updatedCoreElementNumBytes;
            
            CharCP  oldLinkagesLocation = (CharCP)&updatedElement + (updatedElement.GetAttributeOffset() * 2);
            CharP   newLinkagesLocation = (CharP)&updatedElement + updatedCoreElementNumBytes;
            
            memmove (newLinkagesLocation, oldLinkagesLocation, existingLinkagesNumBytes);
            
            updatedElement.SetAttributeOffset(static_cast<UInt32>(updatedCoreElementNumBytes / 2));
            updatedElement.SetSizeWords(updatedElement.GetAttributeOffset() + (existingLinkagesNumBytes / 2));
            }
        }
    
    memcpy (elementString, &variString[0], variString.size ());
    
    if (didStringSizeChange && !edFields.empty ())
        memcpy (elementString + variString.size (), &edFields[0], elementEDFieldNumBytes);
    
    // Compute the pre-computed extents.
    TextString txtString (unicodeString, NULL, NULL, TextStringProperties (textParams, textSize, DisplayHandler::Is3dElem (&updatedElement), dgnFile));
    
    // When the scale is negative (i.e. backwards/RTL for x, and upside-down/vertical for y), size should be negative.
    //  Otherwise, this function is meant to return the distance from the origin (0, 0) to the upper right corner, so extents.low is ignored.
    DRange2d    extents     = txtString.GetExtents ();
    double      totalWidth  = (textSize.x < 0.0) ? (extents.low.x - extents.high.x) : extents.high.x;
    double      totalHeight = (textSize.y < 0.0 || textParams.flags.vertical) ? (extents.low.y - extents.high.y) : extents.high.y;
    
    updatedElement.ToText_2dR().length   = totalWidth;
    updatedElement.ToText_2dR().height   = totalHeight;
    
    eeh.ReplaceElement (&updatedElement);
    
    if (reValidateRange)
        {
        if (NULL == eeh.GetDgnModelP ())
            { BeAssert (false); return ERROR; }
        
        return ((SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh)) ? SUCCESS : ERROR);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetEDFields (ElementHandleCR eh, EDFieldVectorR edFields)
    {
    edFields.clear ();
    
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DgnElementCR element     = *eh.GetElementCP ();
    bool        is3d        = DisplayHandler::Is3dElem (&element);
    UInt16      numEDFields = std::min<UInt16>(is3d ? element.ToText_3d().edflds : element.ToText_2d().edflds, MAX_EDFIELDS);
    
    if (numEDFields != 0)
        {
        char const * variString = (is3d ? element.ToText_3d().string : element.ToText_2d().string);
        
        TextEDField* edFieldBegin = (TextEDField*)(variString + element.ToText_2d().numchars);
        TextEDField* edFieldEnd   = edFieldBegin + numEDFields;
        edFields.assign (edFieldBegin, edFieldEnd);
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetEDFields (EditElementHandleR eeh, EDFieldVectorCP edFields)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    EDFieldVector::size_type numEDFields = 0;
    
    if (NULL != edFields)
        {
        BeAssert (edFields->size () <= MAX_EDFIELDS);
        numEDFields = std::min<EDFieldVector::size_type> (edFields->size (), MAX_EDFIELDS);
        }
    
    DgnV8ElementBlank updatedElement;
    eeh.GetElementCP ()->CopyTo (updatedElement);
    
    UInt16* elementEDFieldCount     = DisplayHandler::Is3dElem (&updatedElement) ? &updatedElement.ToText_3dR().edflds : &updatedElement.ToText_2dR().edflds;
    CharP elementString           = DisplayHandler::Is3dElem (&updatedElement) ? updatedElement.ToText_3dR().string : updatedElement.ToText_2dR().string;
    size_t  elementEDFieldNumBytes  = sizeof (EDFieldVector::value_type) * numEDFields;
    
    if (numEDFields != *elementEDFieldCount)
        {
        *elementEDFieldCount = static_cast<UInt16>(numEDFields);
        
        if (updatedElement.GetAttributeOffset() != updatedElement.GetSizeWords())
            {
            size_t      updatedCoreElementNumBytes  = (elementString - (CharP)&updatedElement) + updatedElement.ToText_2d().numchars + elementEDFieldNumBytes;
            UInt32      existingLinkagesNumBytes    = (updatedElement.GetSizeWords() - updatedElement.GetAttributeOffset()) * 2;
            
            if (0 != (updatedCoreElementNumBytes % 2))
                ++updatedCoreElementNumBytes;
            
            CharCP  oldLinkagesLocation = (CharCP)&updatedElement + (updatedElement.GetAttributeOffset() * 2);
            CharP   newLinkagesLocation = (CharP)&updatedElement + updatedCoreElementNumBytes;
            
            memmove (newLinkagesLocation, oldLinkagesLocation, existingLinkagesNumBytes);
            
            updatedElement.SetAttributeOffset(static_cast<UInt32>(updatedCoreElementNumBytes / 2));
            updatedElement.SetSizeWords(updatedElement.GetAttributeOffset() + (existingLinkagesNumBytes / 2));
            }
        }
    
    if (numEDFields > 0)
        memcpy (elementString + updatedElement.ToText_2d().numchars, &edFields->at (0), elementEDFieldNumBytes);
    
    eeh.ReplaceElement (&updatedElement);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetWhiteSpaceBitmaskValues (ElementHandleCR eh, WhiteSpaceBitmaskValueVectorR whiteSpaceValues)
    {
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::GetWhiteSpaceBitmaskValues (eh, whiteSpaceValues);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetWhiteSpaceBitmaskValues (EditElementHandleR eeh, WhiteSpaceBitmaskValueVectorCP whiteSpaceValues)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::SetWhiteSpaceBitmaskValues (eeh, whiteSpaceValues);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::GetIndentationData (ElementHandleCR eh, IndentationDataR indentationData)
    {
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::GetIndentationData (eh, indentationData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::SetIndentationData (EditElementHandleR eeh, IndentationDataCP indentationData)
    {
    if (TEXT_ELM != eeh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    return TextHandlerBase::SetIndentationData (eeh, indentationData);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::_InitTextString (ElementHandleCR eh, TextStringR txtString)
    {
    txtString.Clear ();
    
    if (TEXT_ELM != eh.GetLegacyType())
        { BeAssert (false); return ERROR; }
    
    DPoint3d        elementOrigin;  if (SUCCESS != TextElemHandler::GetElementOrigin    (eh, elementOrigin) )   return ERROR;
    RotMatrix       orientation;    if (SUCCESS != TextElemHandler::GetOrientation      (eh, orientation)   )   return ERROR;
    DPoint2d        textSize;       if (SUCCESS != TextElemHandler::GetFontSize         (eh, textSize)      )   return ERROR;
    TextParamWide   textParams;     if (SUCCESS != TextElemHandler::GetTextParams       (eh, textParams)    )   return ERROR;
    EDFieldVector   edFields;       if (SUCCESS != TextElemHandler::GetEDFields         (eh, edFields)      )   return ERROR;
    T_FontChars     fontChars;      if (SUCCESS != TextElemHandler::GetFontChars        (eh, fontChars)     )   return ERROR;
    
    DgnElementCR     element         = *eh.GetElementCP ();
    int             numEDFields     = static_cast<int>(edFields.size ());
    TextEDFieldCP   edFieldsArray   = ((numEDFields > 0) ? &edFields[0] : NULL);
    int             symbLineWeight  = element.GetSymbology().weight;
    
    txtString.Init (&fontChars[0], fontChars.size () - 1, elementOrigin, orientation, TextStringProperties (textParams, textSize, eh.GetDgnModelP ()->Is3d (), eh.GetDgnModelP ()->GetDgnProject ()), numEDFields, edFieldsArray, symbLineWeight);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextNodeHandler::CreateElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
DPoint3dCR          userOrigin,
RotMatrixCR         orientation,
DPoint2dCR          textSize,
TextParamWide       textParams,
UInt16              maxLineLength,
bool                is3d,
DgnModelR        modelRef
)
    {
    DgnElementCP templateElement = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   newElement;

    if ((NULL != templateElement) && (TEXT_NODE_ELM == templateElement->GetLegacyType()) && (DisplayHandler::Is3dElem (templateElement) == is3d))
        {
        // Linkages will be copied/pruned later.
        memcpy (&newElement, templateElement, templateElement->GetAttributeOffset() * 2);
        ElementUtil::SetRequiredFields (newElement, TEXT_NODE_ELM, templateElement->GetLevel(), false, (ElementUtil::ElemDim)is3d);

        // Enforce some constraints on the element header.
        newElement.SetDynamicRange(false);
        newElement.InvalidateElementId();
        }
    else
        {
        // ToText_node_3d() is larger than ToText_node_2d().
        memset (&newElement, 0, sizeof (Text_node_3d));
        ElementUtil::SetRequiredFields (newElement, TEXT_NODE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim)is3d);
        
        // This means that the template was of the wrong type, only take some select data from it.
        if (NULL != templateElement)
            {
            newElement.SetLevel(templateElement->GetLevelValue());

            if (templateElement->IsGraphic())
                memcpy (&newElement, templateElement, sizeof(DgnElement));
            }
        }
    
    // Regardless of a template, we only have/want the core element data at this point, so can push the size and offset.
    UInt32 elementSize = (is3d ? sizeof(Text_node_3d) : sizeof(Text_node_2d));
    elementSize /= 2;
        
    newElement.SetSizeWordsNoAttributes(elementSize);
    
    eeh.SetElementDescr(new MSElementDescr(newElement,modelRef), false);
    
    if (SUCCESS != TextNodeHandler::SetUserOrigin       (eeh, userOrigin, false))       { BeAssert (false); return ERROR; }
    if (SUCCESS != TextNodeHandler::SetOrientation      (eeh, orientation, false))      { BeAssert (false); return ERROR; }
    if (SUCCESS != TextNodeHandler::SetFontSize         (eeh, textSize, false))         { BeAssert (false); return ERROR; }
    if (SUCCESS != TextNodeHandler::SetTextParams       (eeh, textParams, false))       { BeAssert (false); return ERROR; }
    if (SUCCESS != TextNodeHandler::SetMaxCharsPerLine  (eeh, maxLineLength))           { BeAssert (false); return ERROR; }
    
    // Copy over any additional linkages from a template element.
    if (NULL != templateElement)
        copyNonTextLinkages (*templateEh, eeh);

    return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh) ? SUCCESS : ERROR);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/09
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::CreateElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
DPoint3dCR          elementOrigin,
RotMatrixCR         orientation,
DPoint2dCR          textSize,
TextParamWideCR     textParams,
WCharCP             unicodeString,
bool                is3d,
DgnModelR        modelRef
)
    {
    if (WString::IsNullOrEmpty (unicodeString))
        { BeAssert (false); return ERROR; }
    
    DgnElementCP templateElement = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   newElement;

    if ((NULL != templateElement) && (TEXT_ELM == templateElement->GetLegacyType()) && (DisplayHandler::Is3dElem (templateElement) == is3d))
        {
        // Linkages will be copied/pruned later.
        memcpy (&newElement, templateElement, templateElement->GetAttributeOffset() * 2);
        ElementUtil::SetRequiredFields (newElement, TEXT_ELM, templateElement->GetLevel(), false, (ElementUtil::ElemDim)is3d);

        // Enforce some constraints on the element header.
        newElement.SetDynamicRange(false);
        newElement.InvalidateElementId();
        }
    else
        {
        // Although element size is variable, we want to zero out the important parts... ToText_3d() is larger than ToText_2d().
        memset (&newElement, 0, sizeof (Text_3d));
        ElementUtil::SetRequiredFields (newElement, TEXT_ELM, LEVEL_DEFAULT_LEVEL_ID, false, (ElementUtil::ElemDim)is3d);

        // This means that the template was of the wrong type, only take some select data from it.
        if (NULL != templateElement)
            {
            newElement.SetLevel(templateElement->GetLevelValue());

            if (templateElement->IsGraphic())
                memcpy (&newElement, templateElement, sizeof(DgnElement));
            }
        }
    
    // Regardless of a template, we only have/want the core element data at this point, so can push the size and offset.
    UInt32 elementSize = (is3d ? sizeof (Text_3d) : sizeof (Text_2d));
    elementSize /= 2;
    
    newElement.SetSizeWordsNoAttributes(elementSize);

    // TextElemHandler::SetString (called below) has an optimization when the string size doesn't change (e.g. it doesn't move linkages around, it just memcpy's).
    //  Since numchars effectively describes elementSize and attrOffset, and we're resetting them, must also reset numChars, otherwise we could overwrite past the new end of the element.
    newElement.ToText_2dR().numchars = 0;
    
    eeh.SetElementDescr(new MSElementDescr(newElement,modelRef), false);
    eeh.ChangeElementHandler(GetInstance());
    
    if (SUCCESS != TextElemHandler::SetElementOrigin    (eeh, elementOrigin, false))                        { BeAssert (false); return ERROR; }
    if (SUCCESS != TextElemHandler::SetOrientation      (eeh, orientation, false))                          { BeAssert (false); return ERROR; }
    if (SUCCESS != TextElemHandler::SetFontSize         (eeh, textSize, false))                             { BeAssert (false); return ERROR; }
    if (SUCCESS != TextElemHandler::SetTextParams       (eeh, textParams, false))                           { BeAssert (false); return ERROR; }
    if (SUCCESS != TextElemHandler::SetString           (eeh, unicodeString, textParams, textSize, false))  { BeAssert (false); return ERROR; }
    if (SUCCESS != TextElemHandler::SetEDFields         (eeh, NULL))                                        { BeAssert (false); return ERROR; }
    
    // Copy over any additional linkages from a template element.
    if (NULL != templateElement)
        copyNonTextLinkages (*templateEh, eeh);
    
    return (SUCCESS == eeh.GetDisplayHandler ()->ValidateElementRange (eeh) ? SUCCESS : ERROR);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
BentleyStatus TextElemHandler::ComputeUserOrigin (ElementHandleCR eh, DPoint3dR userOrigin)
    {
    TextElemHandlerP handler = dynamic_cast<TextElemHandlerP>(&eh.GetHandler ());
    if (NULL == handler)
        { BeAssert (false); return ERROR; }
    
    // We could use the element's cached range to make this faster; choosing to sacrifice for accuracy.
    TextString ts;
    handler->InitTextString (eh, ts);

    return ts.ComputeUserOrigin (userOrigin);
    }

