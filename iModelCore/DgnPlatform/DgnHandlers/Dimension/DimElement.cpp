/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/DimElement.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

// macro to allocate an element on the stack and get it from another element
#define DUP_ELEMENT_ON_STACK(elDst,elSrc) {size_t sizElm = elSrc->Size (); elDst = (DgnElement*)alloca(sizElm); memcpy(elDst, elSrc, sizElm); }

typedef struct
    {
    Int64       tagValue;
    DPoint3d    point;
    } DimBasePoint;

//static byte changeDimOpt;                                    unused var removed in graphite
//static int  changeDimJust;                                   unused var removed in graphite

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//static void startChangeDimView (void);                       unused fdecl removed in graphite
//static void acceptChangeDimView (void);                      unused fdecl removed in graphite
//static void acceptChangeToAltSymbology (void);               unused fdecl removed in graphite
//static void acceptChangeToStdSymbology (void);               unused fdecl removed in graphite

Public int mdlDim_styleSetToActive (DgnElement *, int);      // TMP

/*---------------------------------------------------------------------------------**//**

    Notes on propagation of dimstyle properties to dimensions.
    ----------------------------------------------------------

    When a style is saved, the properties from the style are propagated to the
    dependant dimension elements through the same code that is used to create a
    new dimension, mdlDim_updateFromDimStyle.  The exceptions to propagation are
    controlled by shield flags.

    For most dimension element properties, there is a corresponding shield flag which
    protects that property from being changed when the style is saved.  Note that it
    is an 'inconsistant state' (bug) for a dimension to disagree with its style if it
    does not also have a shield flag set.

    There are two ways to change a property on a dimension.  The first is called
    'update', such as when a style is saved.  Updates must respect any existing
    shields.  The second way is called 'set'.  Sets ignore the existing shields,
    but they *must* also set the shield immediately to avoid an inconsistant state.

    The current implementation is not ideal, since all the non-shielded properties
    on the dimension elements are actually redundant with the dimstyle.  If only
    shielded properties were stored, there would be no need for explicit shields,
    and there would be no possibility of inconsistant states as described above.
    See the 'once and for all' note below.


    Notes on storage of text style properties for dimStyles and dimensions.
    -----------------------------------------------------------------------
    The persistant data structures for both dimStyles and dimensions each store
    the LegacyTextStyle data structure as a 'packed' linkage.  Packed linkages store
    only those values that differ from the default values.

    Note that several text style properties were already present on dimStyles and
    dimensions prior to the integration of text styles.  These properties are NOT
    redundantly stored in the linkage, see mdlDim_setTextStyle and
    mdlDimStyle_setTextStyleProp.  Those properties are called 'explicit' while
    the bulk of the textstyle properties are 'implicit'.

    The complete list of explicit properties is: font, height, width, color,
    use underline and use stackedFractions.  All other textstyle properties are
    implicit.

    When a text style is saved, the properties from the style are propagated both
    to the dimStyles and the dimension elements.  The current implementation handles
    the two propagation steps in series, ie. first the properties are propagated to
    any dependant dimStyles and then to any dependant dimensions (See discussion
    below on a suggested improvement called 'staged propagation').


    Notes on propagation of text style properties to dimStyles.
    ------------------------------------------------------------
    Propagation of properties from text styles to dimStyles is controlled by the
    overrideFlags section of the textstyle structure on the dimStyle.  These bits
    within the textstyle data structure are called the dimStyle's TSOs.  A dimStyle's
    TSOs act as shields to prevent the text style from changing the dimstyle's
    properties during a normal update.  The dimStyle's TSOs are not themselves
    propagated from the text style, but are instead directly controlled by the
    dimStyle.

    Note that the dimStyle TSOs are not actually stored within the textstyle on the
    dimStyle.  They are derived by the API (mdlDimStyle_getTextStyleProp) from
    first-class dimStyle properties.  Importantly, only 'explicit' properties
    overrides are derived in this way.  DimStyles do not have any mechanism to
    control the propagation of the 'implicit' properties.  This is by design,
    since it simplifies the implementation.  The underlying assumption is that
    users will not have reason to override each and every textstyle property,
    preferring instead to simply create a new textstyle.


    Notes on propagation of text style properties to dimensions.
    ------------------------------------------------------------
    When either a text style or a dimStyle is saved, its properties are propagated
    to dependant dimensions.  This section converns the propagation of the 'explicit'
    and 'implicit' text style properties.  For general dimStyle properties see
    above.

    When a dimStyle is saved, its textstyle properties are propagated to the
    dependant dimensions.  This is done via a two stage process.  First, the
    implicit properties are propagated by the function adim_updateTextStyleInfo.
    Dimensions have a set of TSOs that can be used to block this propagation.
    Note that although dimensions also have TSOs for the explicit properties,
    these TSOs are manually switched ON in this step to block their propagation.
    The second step is to propagate the explicit properties.  This is done in
    the main section of mdlDim_updateFromDimStyle, respecting any existing
    shield flags on the dimension.

    Another important step is to propagate the dimStyle's TSOs to the dimension
    element.  This is done in adim_updateTextStyleOverrides.  Note that *only*
    explicit property TSO flags are propagated in this step.  This is important.
    The dimStyle has nothing to say about the implicit properties, it just
    forwards them over from the text style.

    When a text style is saved, its properties are also propagated to the
    dimension elements.  The current implementation does this directly, although
    we should consider moving to a staged propagation scheme at some point.
    The direct propagation is done in mdlDim_updateFromTextStyle.  In this step
    both the implicit and explicit properties must be propagated and both the
    dimensions TSOs and shields must be respected.


    Notes on 'staged propagation' plan to improve the existing implementation.
    --------------------------------------------------------------------------
    We currently implement propagation from text styles to dimensions directly
    via mdlDim_updateFromTextStyle.  We should consider a plan where this step
    is eliminated.  When a text style is saved, its properties would be propagated
    first to the dependant dimStyles.  Then, when the dimStyles are saved, their
    properties would be in turn propagated to the dimensions.  This new scheme
    would eliminate the possibility of an inconsistant state where the the dimStyle
    and the text style are pushing the dimension to different values.  Some
    bugs have created this situation in the past where a property ends up being
    controlled by whichever style is saved last.  Not good.


    Notes on resolving propagation problems once and for all.
    ---------------------------------------------------------
    The fact is that all this complicated propagation logic is not really necessary
    in the first place.  There is no reason for the non-shielded dimension properties
    to be present on the dimension in the first place.  If the dimension were able
    to refer to its style at stroke time, all the redundant data would not be needed.

    The current situation is the result of design decisions made early in the V8
    project.  Those decisions were based mainly on unfamiliarity with the stroke
    code and uncertainty about the time constraints for that project.

    If/when we are able to make massive file format changes again, we should
    *strongly* consider a dimension element data structure in which every property
    is optional, and we only store exlicit values for those properties which are
    locally overidden.


    Some quick definitions (described above in detail).
    ---------------------------------------------------
    Explicit TS Prop: Can be specifically controlled by DimStyle (ex. Height, width)
    Implicit TS Prop: Not directly controlled by DimStyle (ex. underline offset)

    Shields      - stored on dims to protect from dimstyle AND textstyle
    Explicit TSO - stored on dims to protect from textstyle
                   stored on dimstyles to protect from textstyle
    Implicit TSO - stored on dims to protect from textstyle
                   not stored on dimstyles, they cannot control implicit props


    Notes on minimum leaders and elbow length
    -----------------------------------------
    By design, all size properties in style have units in terms of text height except
    for center mark and stack offset. However, the minimum leader uses text width units!
    This may be a legacy bug that has existed since day 1 and we have to live with it
    perhaps forever.  Now we have split minleader into inside and outside minimum leaders.
    We have little choice but to keep this width units for both properties.  On element
    however, most size values are in UOR's except for note, which is handled in
    different code (note.cpp).  In summary:

    Inside minimum leader (geom.margin) => text width units in style, UOR's in element
    Outside minimum leader              => text width units in style, UOR's in element
    Ball & chain elbow length           => text height units in style, UOR's in element
    Note elbow length                   => text height units in style and in element

+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
WStringP DimStrings::GetString (size_t iString)
    {
    if (6 <= iString)
        { BeAssert(false); return NULL; }

    return &m_strings[iString];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
WStringP DimStrings::GetString (DimensionTextPartType partType, DimensionTextPartSubType subPartType)
    {
    UInt        iString = DIMTEXTPART_Secondary == partType ? 3 : 0;

    switch (subPartType)
        {
        default:                                // Fallthru
        case DIMTEXTSUBPART_Main:               iString += 0;   break;
        case DIMTEXTSUBPART_Limit_Upper:        iString += 0;   break;
        case DIMTEXTSUBPART_Limit_Lower:        iString += 1;   break;
        case DIMTEXTSUBPART_Tolerance_Plus:     iString += 1;   break;
        case DIMTEXTSUBPART_Tolerance_Minus:    iString += 2;   break;
        }

    return &m_strings[iString];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDim_setStyleUniqueId
(
EditElementHandleR         dimElement,
UInt64               styleUniqueIdIn
)
    {
    if (DIMENSION_ELM != dimElement.GetLegacyType())
        return  ERROR;

    dimElement.GetElementP()->ToDimensionElmR().dimStyleId = styleUniqueIdIn;
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    07/93
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_insertViewRotBlock
(
EditElementHandleR dimElement,
RotMatrixCR     rMatrix
)
    {
    DimViewBlock    viewBlock;

    adim_deleteOptionBlock (dimElement, ADBLK_VIEWROT);

    memset (&viewBlock, 0, sizeof(viewBlock));
    viewBlock.nWords = sizeof (viewBlock) / 2;
    viewBlock.type   = ADBLK_VIEWROT;

    if (dimElement.GetElementP()->ToDimensionElm().Is3d())
        rMatrix.GetQuaternion(viewBlock.viewRot, true);
    else
        rMatrix.GetRowValuesXY(viewBlock.viewRot);

    mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*)&viewBlock, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/08
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    adim_usesAnyAltSymb (DimensionElm const &dim)
    {
    for (int iPoint = 0; iPoint < dim.nPoints; iPoint++)
        {
        DimText const *dimText = dim.GetDimTextCP(iPoint);

        if ( ! dimText->flags.b.noWitness && dimText->flags.b.altSymb)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionType   BentleyApi::adim_getTemplateNumber
(
ElementHandleCR dimElement
)
    {
    DimensionType   templateNo;

    byte intTemplateNo = dimElement.GetElementCP()->ToDimensionElm().dimcmd;
    switch (static_cast<DimensionType>(intTemplateNo))
        {
        case DimensionType::LabelLine:            // no dimension template.
        case DimensionType::Note:
            templateNo = DimensionType::None;
            break;

        case DimensionType::AngleAxisX:          // These dimensions share the DimensionType::AngleAxis template.
        case DimensionType::AngleAxisY:
            templateNo = DimensionType::AngleAxis;
            break;

        default:
            {
            if ( (0 == intTemplateNo) || (intTemplateNo > static_cast<byte>(DimensionType::MaxThatHasTemplate)))
                templateNo = DimensionType::None;
            else
                templateNo = static_cast<DimensionType>(intTemplateNo);
            break;
            }
        }

    return templateNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isPropertyShielded
(
DimStylePropMaskP     shieldFlags,
DimStyleProp          property
)
    {
    if (NULL == shieldFlags)
        return false;

    return shieldFlags->GetPropertyBit (property);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    clearShieldFlag
(
DimStylePropMaskP     shieldFlags,
DimStyleProp          property
)
    {
    if (NULL == shieldFlags)
        return;

    shieldFlags->SetPropertyBit (property, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    setUnitLabel
(
EditElementHandleR dimElement,
WChar         *pUnitLabelIn,
int              labelFlagIn
)
    {
    if (dimElement.IsValid())
        {
        if (NULL == pUnitLabelIn || '\0' == pUnitLabelIn[0])
            mdlDim_removeUnitLabel (dimElement, labelFlagIn);
        else
            mdlDim_setUnitLabel (dimElement, pUnitLabelIn, labelFlagIn);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_isLinearDimension
(
ElementHandleCR dimElement
)
    {
    return  DIM_LINEAR (static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_isAngularDimension
(
ElementHandleCR dimElement
)
    {
    return  DIM_ANGULAR (static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_isRadialDimension
(
ElementHandleCR dimElement
)
    {
    return  DIM_RADIAL(static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_isNoteDimension
(
ElementHandleCR dimElement
)
    {
    return  DIM_NOTE(static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_isOrdinateDimension
(
ElementHandleCR dimElement
)
    {
    return  DIM_ORDINATE(static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_isLabelLineDimension
(
ElementHandleCR dimElement
)
    {
    return  DIM_LABELLINE(static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_setDimTextWitnessLineFromTemplate
(
ElementHandleCR     dimElement,
DimText *           dimText,
int                 iPoint,
bool                noWitness   /* => true: no witness, false: use template */
)
    {
    DimensionElm const* pDimElm = &dimElement.GetElementCP()->ToDimensionElm();
    int     iFirstWitness = DIM_ANGULAR (static_cast<DimensionType>(pDimElm->dimcmd)) ? 1 : 0;

    /*-------------------------------------------------------------------
    Display of witness is controlled per point.  Each point can override
    locally or get from the style.
    -------------------------------------------------------------------*/
    if (!dimText->flags.b.witCtrlLocal)
        {
        bool    showWitness = false;

        if (!noWitness)
            showWitness = (iPoint <= iFirstWitness) ? pDimElm->tmpl.left_witness : pDimElm->tmpl.right_witness;

        if (mdlDim_isOrdinateDimension (dimElement))
            showWitness = true;

        dimText->flags.b.noWitness = !showWitness;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_setWitnessLinesFromTemplate
(
EditElementHandleR    dimElement,
bool            noWitness   /* => true: no witness, false: use template */
)
    {
    for (int iPoint = 0; iPoint < dimElement.GetElementCP()->ToDimensionElm().nPoints; iPoint++)
        adim_setDimTextWitnessLineFromTemplate (dimElement, dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(iPoint), iPoint, noWitness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    wStringHasUserText (WStringCP pString)
    {
    if (NULL == pString)
        return false;

    if (pString->empty())
        return false;

    if (L'*' == (*pString)[0] && L'\0' == (*pString)[1])
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    hasSecondaryUserText (ElementHandleCR dimElement)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == hdlr)
        return false;

    int             iPoint, iSegment, numSegments = hdlr->GetNumSegments (dimElement);
    bool            hasSecondaryUserText = false;
    DimStrings      dimStrings;
    DimStringConfig dimStrConfig;

    /*----------------------------------------------------------------------------------
      This function should probably be enhanced to search for secondary overridden
      multiline text as well.
    ----------------------------------------------------------------------------------*/
    for (iSegment = 0; iSegment < numSegments; iSegment++)
        {
        BentleyApi::mdlDim_getTextPointNo (&iPoint, dimElement, iSegment);

        if (SUCCESS != hdlr->GetStrings (dimElement, dimStrings, iPoint, &dimStrConfig))
            continue;

        if ( ! dimStrConfig.dual)
            continue;

        if ( ! dimStrConfig.tolerance)
            {
            hasSecondaryUserText = wStringHasUserText (dimStrings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Main));
            }
        else
            {
            if (dimStrConfig.limit)
                {
                hasSecondaryUserText = wStringHasUserText (dimStrings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Limit_Upper)) ||
                                        wStringHasUserText (dimStrings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Limit_Lower));
                }
            else
                {
                hasSecondaryUserText = wStringHasUserText (dimStrings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Main)) ||
                                        wStringHasUserText (dimStrings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Tolerance_Plus)) ||
                                        wStringHasUserText (dimStrings.GetString (DIMTEXTPART_Secondary, DIMTEXTSUBPART_Tolerance_Minus));
                }
            }

        if (hasSecondaryUserText)
            break;
        }

    return hasSecondaryUserText;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_checkUseOfAngularSettings
(
bool&               primary,
bool&               secondary,
ElementHandleCR     dimElement
)
    {
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    if (pDim->frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
        {
        primary   = true;
        secondary = true;
        }
    else
    if (mdlDim_isLabelLineDimension (dimElement))
        {
        // Even if labelLineInvertLabels ends up true, we still use primary settings
        // for distance and secondary for angle
        primary   = false;
        secondary = true;
        }
    else
        {
        primary   = false;
        secondary = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_clearTextStyleOverrides (EditElementHandleR dimElement)
    {
    LegacyTextStyle   textStyle;
    DimensionHandler&   hdlr = DimensionHandler::GetInstance();

    if (SUCCESS == hdlr.GetTextStyle (dimElement, &textStyle))
        {
        memset (&textStyle.overrideFlags, 0, sizeof(textStyle).overrideFlags);

        hdlr.SetTextStyle (dimElement, &textStyle, false, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_updateTextStyleOverrides
(
DgnTextStylePropertyMaskR  elementTextStyle,  /* <=> */
DimStylePropMaskP       shieldFlags,        /*  => */
DgnTextStylePropertyMaskCR pStyleTextStyle     /*  => */
)
    {
    /*--------------------------------------------------------------------------------
       Propagate the dimstyle's override flags (which protect it from the text style)
       onto the dimension, so that it will also be protected from the text style as
       well.  Note that the override flags themselves can be shielded.

       In other words, update the overrideFlags in the dimension to match the ones
       in the dimStyle.

       Only propagate the 'explicit' TSOs.  The dimstyle has nothing to save about
       the implicit properties at all.
    --------------------------------------------------------------------------------*/
    if ( ! isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_Font_BOOLINT))
        elementTextStyle.SetProperty(DgnTextStyleProperty::Font, pStyleTextStyle.IsPropertySet(DgnTextStyleProperty::Font));

    if ( ! isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT))
        elementTextStyle.SetProperty(DgnTextStyleProperty::Height, pStyleTextStyle.IsPropertySet(DgnTextStyleProperty::Height));

    if ( ! isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT))
        elementTextStyle.SetProperty(DgnTextStyleProperty::Width, pStyleTextStyle.IsPropertySet(DgnTextStyleProperty::Width));

    if ( ! isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT))
        elementTextStyle.SetProperty(DgnTextStyleProperty::IsUnderlined, pStyleTextStyle.IsPropertySet(DgnTextStyleProperty::IsUnderlined));

    if ( ! isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_OverrideColor_BOOLINT))
        elementTextStyle.SetProperty(DgnTextStyleProperty::HasColor, pStyleTextStyle.IsPropertySet(DgnTextStyleProperty::HasColor));

    if ( ! isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_Color_COLOR))
        elementTextStyle.SetProperty(DgnTextStyleProperty::Color, pStyleTextStyle.IsPropertySet(DgnTextStyleProperty::Color));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_VANCOUVER_MERGE // annotation
static void                applyAnnotationScaleToDimTextStyle
(
LegacyTextStyle *                 dimElmTextStyle,
int                         option,
double                      dAnnotScale
)
    {
    LegacyTextStyleOverrideFlags  applyFlags;

    // Apply annotation scale to element text style in the following cases.
    // a. During create_dimension and change_dimension, the unscaled textsize in the tcb is used.
    //    Therefore apply the scale here.
    // b. In the update workflows, we need to check if the dimension element overrides the textsize.
    //    If yes, then the dimension's existing textsize is already scaled, so don't reapply. If no,
    //    then we are inheriting from a given textstyle (which holds unscaled textsize).
    //    Therefore apply the scale here.

    if (ADIM_PARAMS_CREATE == option || ADIM_PARAMS_CHANGE == option || ADIM_PARAMS_CREATE_FROMDWG == option)
        {
        // Apply to all values
        memset (&applyFlags, 0xff, sizeof (applyFlags));
        }
    else
        {
        // The textstyleoverrides mean that we don't want to propagate scale
        // to the corresponding properties. The apply function needs overrides
        // that mean the opposite.

        int     numShorts = sizeof (LegacyTextStyleOverrideFlags) / sizeof (UShort);
        UShort * pShort1 = (UShort *) &dimElmTextStyle->overrideFlags;
        UShort * pShort2 = (UShort *) &applyFlags;

        for (int iShort = 0; iShort < numShorts; iShort++, pShort1++, pShort2++)
            *pShort2 = ~(*pShort1);
        }

    dimElmTextStyle->Scale(dAnnotScale, applyFlags);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnTextStylePropertyMaskPtr     adim_getTextstylePropertyMask(DgnTextStyleCR style)
    {
    LegacyTextStyle textStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(style);
    return DgnTextStylePersistence::Legacy::FromLegacyMask(textStyle.overrideFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void  adim_updateTextStyleInfo
(
LegacyTextStyle&          dimElmTextStyle,      /* <=> */
bool               *pUsedTextStyleColor,  /* <=  */
double             *pWidthFactor,         /* <=  */
int                 option,               /* => replace or update tcb information        */
DgnTextStyleCR      textStyle,           /* =>  */
DimStylePropMaskP   shieldFlags,
bool                useTextStyle,
double              dAnnotScale
)
    {
    // A text style's TSOs refer to its parent style, so they have no meaning
    // for dimension elements.
    DgnTextStylePropertyMaskPtr dimShields = DgnTextStylePersistence::Legacy::FromLegacyMask(dimElmTextStyle.overrideFlags);
    if (ADIM_PARAMS_UPDATE_FROMTEXTSTYLE != option)
        adim_updateTextStyleOverrides (*dimShields, shieldFlags, *adim_getTextstylePropertyMask(textStyle));

    /*--------------------------------------------------------------------------------
        Make a local copy of the overrideFlags.  We need to make some local changes
        before the call to textstyle_updateFromResolvedStyle, but we don't want
        those changes to be stored back to the dimension element.
    --------------------------------------------------------------------------------*/
    if (useTextStyle)
        {
        /* If the text style has invalid values, we don't want to use them.
           This is common in DWG files */
        double height;
        textStyle.GetPropertyValue(DgnTextStyleProperty::Height, height);
        if (LegacyMath::DEqual(height, 0.0))
            dimShields->SetProperty(DgnTextStyleProperty::Height, true);

        double width;
        textStyle.GetPropertyValue(DgnTextStyleProperty::Width, width);
        if (LegacyMath::DEqual(width, 0.0))
            dimShields->SetProperty(DgnTextStyleProperty::Width, true);

        /* If the property is shielded, then we don't want to use it. */
        if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_General_Font_FONT))
            dimShields->SetProperty(DgnTextStyleProperty::Font, true);

        if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_Height_DOUBLE))
            dimShields->SetProperty(DgnTextStyleProperty::Height, true);

        if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_Width_DOUBLE))
            dimShields->SetProperty(DgnTextStyleProperty::Width, true);

        if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_Underline_BOOLINT))
            dimShields->SetProperty(DgnTextStyleProperty::IsUnderlined, true);

        if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_OverrideColor_BOOLINT))
            dimShields->SetProperty(DgnTextStyleProperty::HasColor, true);

        if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Text_Color_COLOR))
            dimShields->SetProperty(DgnTextStyleProperty::Color, true);
        }
    else
        {
        /* If we are not using a text style, these properties never come
           from the dimstyle's text style.  They may come directly from the
           dimstyle later */

        dimShields->SetProperty (DgnTextStyleProperty::Font, true);
        dimShields->SetProperty (DgnTextStyleProperty::Height, true);
        dimShields->SetProperty (DgnTextStyleProperty::Width, true);
        dimShields->SetProperty (DgnTextStyleProperty::IsUnderlined, true);
        dimShields->SetProperty (DgnTextStyleProperty::HasColor, true);
        dimShields->SetProperty (DgnTextStyleProperty::Color, true);
        }

    /* If the text style doesn't hold a color, then it should NOT be changing
       the color on the dimension.  I consider this a bug in
       textstyle_updateFromResolvedStyle     JS 4/22/03 */
    if (dimShields->IsPropertySet (DgnTextStyleProperty::HasColor))
        dimShields->SetProperty(DgnTextStyleProperty::Color, true);

    DgnTextStylePtr dimTextStylePtr = DgnTextStyle::Create (textStyle.GetProject());
    DgnTextStylePersistence::Legacy::FromLegacyStyle(*dimTextStylePtr, dimElmTextStyle);
    dimTextStylePtr->CopyPropertyValuesFrom(textStyle, *dimShields);
    dimElmTextStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*dimTextStylePtr);

    //applyAnnotationScaleToDimTextStyle (&dimElmTextStyle, option, dAnnotScale);

    if (pUsedTextStyleColor)
        *pUsedTextStyleColor = (! dimElmTextStyle.overrideFlags.colorFlag) && dimShields->IsPropertySet (DgnTextStyleProperty::HasColor);

    if (pWidthFactor)
        *pWidthFactor = dimElmTextStyle.widthFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     adim_updateFromTextStyle
(
LegacyTextStyle&          dimElmTextStyle,    /* <=> dimension's text style to be updated */
EditElementHandleR  dimElement          /*  => dimension */
)
    {
    bool            anyChanges = false;

    DgnProjectP project = dimElement.GetDgnProject();
    
    if (0 == dimElement.GetElementCP()->ToDimensionElm().textStyleId)
        { BeAssert(0); return false; }

    DgnTextStylePtr textStyleObj = project->Styles().TextStyles().QueryById(DgnStyleId(dimElement.GetElementCP()->ToDimensionElm().textStyleId));
    if (textStyleObj.IsNull())
        { BeAssert(0); return false; }

    double              dAnnotScale     = 1.0;
    DimensionHandler&   hdlr            = DimensionHandler::GetInstance();
    DimStylePropMaskPtr shieldFlags     = hdlr.GetOverrideFlags (dimElement);

    mdlDim_getEffectiveAnnotationScale (&dAnnotScale, dimElement);

    DgnTextStylePtr oldDimTextStyle = DgnTextStyle::Create(*project);
    DgnTextStylePersistence::Legacy::FromLegacyStyle(*oldDimTextStyle, dimElmTextStyle);

    adim_updateTextStyleInfo (dimElmTextStyle, NULL, NULL, ADIM_PARAMS_UPDATE_FROMTEXTSTYLE, *textStyleObj, shieldFlags.get(), true, dAnnotScale);
    DgnTextStylePtr dimElemStylePtr = DgnTextStyle::Create(*project);
    DgnTextStylePersistence::Legacy::FromLegacyStyle(*dimElemStylePtr, dimElmTextStyle);

    DgnTextStylePropertyMaskPtr   compareMask = oldDimTextStyle->Compare (*dimElemStylePtr);
    if (compareMask->AreAnyPropertiesSet())
        anyChanges = true;

    /*--------------------------------------------------------------------------------
        Since we are updating directly from a textstyle, we need to manually handle
        all the dim properties that are normally dependant on the text height.
    --------------------------------------------------------------------------------*/
    double      heightScale, widthScale, width;

    textStyleObj->GetPropertyValue(DgnTextStyleProperty::Width, width);
    if (LegacyMath::DEqual(width, 0.0))
        dimElmTextStyle.width = dimElmTextStyle.height * dimElmTextStyle.widthFactor;

    heightScale = dimElmTextStyle.height / dimElement.GetElementCP()->ToDimensionElm().text.height;
    widthScale  = dimElmTextStyle.width / dimElement.GetElementCP()->ToDimensionElm().text.width;

    if (1.0 != heightScale || 1.0 != widthScale)
        {
        if (1.0 != heightScale)
            {
            DimensionElm& dimEl = dimElement.GetElementP()->ToDimensionElmR();
            dimEl.geom.witOffset     *= heightScale;
            dimEl.geom.witExtend     *= heightScale;
            dimEl.geom.textMargin    *= heightScale;
            dimEl.geom.textLift      *= heightScale;
            dimEl.geom.margin        *= heightScale;
            dimEl.geom.termWidth     *= heightScale;
            dimEl.geom.termHeight    *= heightScale;

            anyChanges = true;
            }

        DimTolrBlock   *pTolBlock = NULL;

        if (NULL != (pTolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL)))
            {
            if (1.0 != heightScale)
                {
                pTolBlock->tolHeight    *= heightScale;
                pTolBlock->tolHorizSep  *= heightScale;
                pTolBlock->tolVertSep   *= heightScale;

                anyChanges = true;
                }

            if (1.0 != widthScale)
                {
                pTolBlock->tolWidth     *= widthScale;

                anyChanges = true;
                }
            }
        }
    return anyChanges;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DimPropUpdater
{
DimensionStyleCR    m_dimStyle;
DimStylePropMaskR   m_shieldFlags;
IDimCreateDataCP    m_createData;

double              m_dimTextHeight;
double              m_dimTextWidth;
double              m_dimTextHeightChangeRatio;
double              m_dimTextWidthChangeRatio;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimPropUpdater (DimensionStyleCR dimStyle, DimStylePropMaskR shields, IDimCreateDataCP createData)
    :
    m_createData (createData),
    m_dimStyle (dimStyle),
    m_shieldFlags (shields)
    {
    }

bool    IsPropertyShielded (DimStyleProp prop) const        { return TO_BOOL (isPropertyShielded (&m_shieldFlags, prop)); }
void    ClearShieldFlag (DimStyleProp prop)                 { clearShieldFlag (&m_shieldFlags, prop); }
bool    IsControlledByDimStyle (DimStyleProp prop) const    { return m_dimStyle.IsPropertyLocallyControlled (prop); }

bool    IsUpdateNeeded (DimStyleProp prop) const;

bool    GetStyleLineStyle (Int32& styleVal, bool& styleOverride, DimStyleProp prop) const;
bool    GetStyleColor (UInt32& styleVal, bool& styleOverride, DimStyleProp prop) const;
bool    GetStyleWeight (UInt32& styleVal, bool& styleOverride, DimStyleProp prop) const;

byte    GetResolvedAccuracy     (byte    dimVal, DimStyleProp prop) const;
bool    GetResolvedBoolean      (bool    dimVal, DimStyleProp prop) const;
UInt16  GetResolvedInteger      (UInt16  dimVal, DimStyleProp prop) const;
Int32   GetResolvedLineStyle    (Int32   dimVal, DimStyleProp prop) const;
UInt32  GetResolvedColor        (UInt32  dimVal, DimStyleProp prop) const;
UInt32  GetResolvedWeight       (UInt32  dimVal, DimStyleProp prop) const;
LevelId GetResolvedLevel        (LevelId dimVal, DimStyleProp prop) const;
UInt16  GetResolvedChar         (UInt16& dimVal, DimStyleProp prop) const;
UInt32  GetResolvedFont         (UInt32& dimVal, DimStyleProp prop) const;
double  GetResolvedDouble       (double  dimVal, DimStyleProp prop) const;
UInt16  GetResolvedTemplateFlag (UInt16  dimVal, DimStyleProp prop, DimensionType iTemplate) const;

bool    UpdateInteger   (UInt16&  dimVal, DimStyleProp prop) const;
bool    UpdateLineStyle (Int32&   dimVal, DimStyleProp prop) const;
bool    UpdateColor     (UInt32&  dimVal, DimStyleProp prop) const;
bool    UpdateWeight    (UInt32&  dimVal, DimStyleProp prop) const;
bool    UpdateLevel     (LevelId& dimVal, DimStyleProp prop) const;
bool    UpdateChar      (UInt16&  dimVal, DimStyleProp prop) const;
bool    UpdateFont      (UInt32&  dimVal, DimStyleProp prop) const;
bool    UpdateDouble    (double&  dimVal, DimStyleProp prop) const;
void    UpdateString    (WCharP dimVal, int numChars, DimStyleProp prop) const;

void    UpdateRelativeDistance (double& dimVal, DimStyleProp prop, bool relativeToHeight) const;
void    UpdateHeightRelativeDistance (double& dimVal, DimStyleProp prop) const;
void    UpdateWidthRelativeDistance (double& dimVal, DimStyleProp prop) const;

void    GetDimSymbol (DimSymbol *pDimSym, int type, DimStyleProp charProp, DimStyleProp fontProp, DimStyleProp cellProp, DgnProjectR dgnFile, UInt64& cellId);

void    UpdateFitOption             (EditElementHandleR dimElement);
void    UpdateUnitInfo              (EditElementHandleR dimElement, bool    updatePrimary);
void    UpdateAltFormatBlock        (EditElementHandleR dimElement, bool    updatingPrimary);
void    UpdateToleranceInfo         (EditElementHandleR dimElement);
void    UpdateTerminatorInfo        (EditElementHandleR dimElement);
void    UpdatePrefixSuffixInfo      (EditElementHandleR dimElement);
void    UpdateCustomDiameterInfo    (EditElementHandleR dimElement);
void    UpdatePrefixSuffixSymbolInfo (EditElementHandleR dimElement, bool    updatingPrefix);
void    UpdateTermSymbologyInfo     (EditElementHandleR dimElement);
void    UpdateBallAndChainInfo      (EditElementHandleR dimElement);
void    UpdateTemplateInfo          (EditElementHandleR dimElement, DimensionType templateNum);
void    UpdateStyleExtensions       (EditElementHandleR dimElement, IDimCreateDataCP createData, int option, bool    bPropagateStyleAnnotationScaleFlag);

void    UpdateDimensionProperties (EditElementHandleR dimElement, int option, IDimCreateDataCP createData);
UInt64  GetSharedCellID (WCharCP name);
}; //DimPropUpdater

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64       DimPropUpdater::GetSharedCellID (WCharCP name)
    {
    if (NULL == m_createData)
        {
        BeAssert(false);
        return INVALID_ELEMENTID;
        }

    DgnProjectR        destFile = *m_dimStyle.GetDgnProject();
    PersistentElementRefPtr sharedCellDef = ISharedCellQuery::FindDefinitionByName (name, destFile);

    return sharedCellDef.IsValid()? sharedCellDef->GetElementId().GetValue() : INVALID_ELEMENTID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::IsUpdateNeeded (DimStyleProp prop) const
    {
    if (IsPropertyShielded (prop))
        return false;

    if ( ! IsControlledByDimStyle (prop))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte    DimPropUpdater::GetResolvedAccuracy (byte dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    byte styleVal;
    if (SUCCESS != m_dimStyle.GetAccuracyProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::GetResolvedBoolean (bool dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    bool styleVal;
    if (SUCCESS != m_dimStyle.GetBooleanProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16  DimPropUpdater::GetResolvedInteger (UInt16 dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    int styleVal;
    if (SUCCESS != m_dimStyle.GetIntegerProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return static_cast<UInt16>(styleVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateInteger (UInt16& dimVal, DimStyleProp prop) const
    {
    Int16 styleVal = GetResolvedInteger (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::GetStyleLineStyle (Int32& styleVal, bool& styleOverride, DimStyleProp prop) const
    {
    styleOverride = IsControlledByDimStyle (prop);

    if (SUCCESS != m_dimStyle.GetLineStyleProp (styleVal, prop))
        { BeAssert (0); }

    return ! IsPropertyShielded (prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
Int32   DimPropUpdater::GetResolvedLineStyle (Int32 dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    Int32 styleVal;
    if (SUCCESS != m_dimStyle.GetLineStyleProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateLineStyle (Int32& dimVal, DimStyleProp prop) const
    {
    Int32 styleVal = GetResolvedLineStyle (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::GetStyleColor (UInt32& styleVal, bool& styleOverride, DimStyleProp prop) const
    {
    styleOverride = IsControlledByDimStyle (prop);

    if (SUCCESS != m_dimStyle.GetColorProp (styleVal, prop))
        { BeAssert (0); }

    return ! IsPropertyShielded (prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32   DimPropUpdater::GetResolvedColor (UInt32 dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    UInt32 styleVal;
    if (SUCCESS != m_dimStyle.GetColorProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateColor (UInt32& dimVal, DimStyleProp prop) const
    {
    Int32 styleVal = GetResolvedColor (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::GetStyleWeight (UInt32& styleVal, bool& styleOverride, DimStyleProp prop) const
    {
    styleOverride = IsControlledByDimStyle (prop);

    if (SUCCESS != m_dimStyle.GetWeightProp (styleVal, prop))
        { BeAssert (0); }

    return ! IsPropertyShielded (prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32   DimPropUpdater::GetResolvedWeight (UInt32 dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    UInt32 styleVal;
    if (SUCCESS != m_dimStyle.GetWeightProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateWeight (UInt32& dimVal, DimStyleProp prop) const
    {
    Int32 styleVal = GetResolvedWeight (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId DimPropUpdater::GetResolvedLevel (LevelId dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    LevelId styleVal;
    if (SUCCESS != m_dimStyle.GetLevelProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateLevel (LevelId& dimVal, DimStyleProp prop) const
    {
    LevelId styleVal = GetResolvedLevel (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16  DimPropUpdater::GetResolvedChar (UInt16& dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    UInt16 styleVal;
    if (SUCCESS != m_dimStyle.GetCharProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateChar (UInt16& dimVal, DimStyleProp prop) const
    {
    UInt16 styleVal = GetResolvedChar (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  DimPropUpdater::GetResolvedFont (UInt32& dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    UInt32 styleVal;
    if (SUCCESS != m_dimStyle.GetFontProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateFont (UInt32& dimVal, DimStyleProp prop) const
    {
    UInt32 styleVal = GetResolvedFont (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
double  DimPropUpdater::GetResolvedDouble (double dimVal, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return dimVal;

    double styleVal;
    if (SUCCESS != m_dimStyle.GetDoubleProp (styleVal, prop))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DimPropUpdater::UpdateDouble (double& dimVal, DimStyleProp prop) const
    {
    double styleVal = GetResolvedDouble (dimVal, prop);

    if (dimVal == styleVal)
        return false;

    dimVal = styleVal;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateRelativeDistance (double& dimVal, DimStyleProp prop, bool relativeToHeight) const
    {
    double dimBaseVal;
    double baseChangeRatio;

    if (relativeToHeight)
        {
        dimBaseVal      = m_dimTextHeight;
        baseChangeRatio = m_dimTextHeightChangeRatio;
        }
    else
        {
        dimBaseVal      = m_dimTextWidth;
        baseChangeRatio = m_dimTextWidthChangeRatio;
        }

    /* If shielded, the dimension is locally controlling the ratio (val / dimTextHeight), since the
       dimTextHeight might have changed, we multiply the change in to preserve the ratio. */
    dimVal *= baseChangeRatio;

    if ( ! IsUpdateNeeded (prop))
        return;

    double styleVal;
    if (SUCCESS != m_dimStyle.GetDoubleProp (styleVal, prop))
        { BeAssert (0); return; }

    dimVal = styleVal * dimBaseVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateHeightRelativeDistance (double& dimVal, DimStyleProp prop) const
    {
    UpdateRelativeDistance (dimVal, prop, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateWidthRelativeDistance (double& dimVal, DimStyleProp prop) const
    {
    UpdateRelativeDistance (dimVal, prop, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateString (WCharP dimVal, int numChars, DimStyleProp prop) const
    {
    if ( ! IsUpdateNeeded (prop))
        return;

    WString styleVal;
    if (SUCCESS != m_dimStyle.GetStringProp (styleVal, prop))
        { BeAssert (0); return; }

    if (0 == wcsncmp (dimVal, styleVal.c_str(), numChars))
        return;

    wcsncpy (dimVal, styleVal.c_str(), numChars);
    (&dimVal)[numChars - 1] = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16          DimPropUpdater::GetResolvedTemplateFlag (UInt16 dimVal, DimStyleProp propertyNum, DimensionType dimensionType) const
    {
    if (m_shieldFlags.GetTemplateBit (propertyNum, dimensionType))
        return dimVal;

    UInt16 styleVal;
    if (SUCCESS != m_dimStyle.GetTemplateFlagProp (styleVal, dimensionType, propertyNum))
        { BeAssert (0); return dimVal; }

    return styleVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void                DimPropUpdater::UpdateUnitInfo
(
EditElementHandleR     dimElement,
bool                updatePrimary
)
    {
    byte            blockType;
    bool            specifyUnits = false;
    bool            addUnitBlock = false;
    WChar         subLabel[MAX_UNIT_LABEL_LENGTH];
    WChar         masterLabel[MAX_UNIT_LABEL_LENGTH];

    int             masterLabelFlag, subLabelFlag;
    DimStyleProp    useUnitProp, unitProp, masterLabelProp, subLabelProp;
    if (updatePrimary)
        {
        blockType           = ADBLK_PRIMARY;
        useUnitProp         = DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT;
        unitProp            = DIMSTYLE_PROP_Value_Unit_UNITS;
        masterLabelProp     = DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR;
        subLabelProp        = DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR;
        masterLabelFlag     = DIMLABEL_MASTUNIT;
        subLabelFlag        = DIMLABEL_SUBUNIT;
        }
    else
        {
        blockType           = ADBLK_SECONDARY;
        useUnitProp         = DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT;
        unitProp            = DIMSTYLE_PROP_Value_UnitSec_UNITS;
        masterLabelProp     = DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR;
        subLabelProp        = DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR;
        masterLabelFlag     = DIMLABEL_SECONDARY_MASTUNIT;
        subLabelFlag        = DIMLABEL_SECONDARY_SUBUNIT;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
        {
        adim_deleteOptionBlock (dimElement, blockType);

        mdlDim_removeUnitLabel (dimElement, masterLabelFlag);
        mdlDim_removeUnitLabel (dimElement, subLabelFlag);
        return;
        }

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    DimUnitBlock const*pUnitBlock = NULL;
    DimUnitBlock    unitBlock;
    if (NULL != (pUnitBlock = (DimUnitBlock const*) mdlDim_getOptionBlock (dimElement, blockType, NULL)))
        {
        unitBlock = *pUnitBlock;

        // Existance of the block implies units were overridden.
        specifyUnits = true;
        }
    else
        {
        memset (&unitBlock, 0, sizeof (unitBlock));

        unitBlock.nWords   = sizeof (unitBlock) / 2;
        unitBlock.type     = blockType;
        }

    mdlDim_getUnitLabel (subLabel, dimElement, subLabelFlag);
    mdlDim_getUnitLabel (masterLabel, dimElement, masterLabelFlag);

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    if (updatePrimary)
        specifyUnits = ! GetResolvedBoolean ( ! specifyUnits, useUnitProp); // API returns the flag reversed for primary
    else
        specifyUnits =   GetResolvedBoolean (   specifyUnits, useUnitProp);

    if (specifyUnits)
        {
        addUnitBlock = true;

        if (! IsPropertyShielded (unitProp))
            {
            UnitDefinition  styleMasterUnit;
            UnitDefinition  styleSubUnit;

            m_dimStyle.GetUnitsProp (&styleMasterUnit, &styleSubUnit, unitProp);

            UnitDefinition  blockMasterUnit, blockSubUnit;

            BentleyApi::adim_extractUnitBlock (NULL, &blockMasterUnit, &blockSubUnit, &unitBlock);

            if (0 != styleMasterUnit.CompareByScale (blockMasterUnit) ||
                0 != styleSubUnit.CompareByScale (blockSubUnit))
                {
                /* If we are getting updated from the style's units, it doesn't make any sense to
                   maintain the shield flags on the unit labels */
                ClearShieldFlag (masterLabelProp);
                ClearShieldFlag (subLabelProp);

                BentleyApi::adim_createUnitBlock (&unitBlock, updatePrimary, NULL, &styleMasterUnit, &styleSubUnit);
                }
            }

        UpdateString (masterLabel,  MAX_UNIT_LABEL_LENGTH,  masterLabelProp);
        UpdateString (subLabel,     MAX_UNIT_LABEL_LENGTH,  subLabelProp);
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addUnitBlock)
        {
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader *) &unitBlock, NULL);

        setUnitLabel (dimElement, masterLabel, masterLabelFlag);
        setUnitLabel (dimElement, subLabel, subLabelFlag);
        }
    else
        {
        adim_deleteOptionBlock (dimElement, blockType);

        mdlDim_removeUnitLabel (dimElement, masterLabelFlag);
        mdlDim_removeUnitLabel (dimElement, subLabelFlag);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimPropUpdater::UpdateAltFormatBlock
(
EditElementHandleR     dimElement,              /* <=> */
bool                updatingPrimary    /*  => */
)
    {
    byte            blockType;
    bool            useAltFormat;
    bool            addAltFmtBlock = false;
    DimStyleProp    thresholdProp, thresholdEqualProp, thresholdLessProp, accuracyProp, zeroMasterProp, zeroSubProp;
    DimStyleProp    showLabelProp, showSubUnitProp, showDelimProp, showMasterProp;

    DimAltFmtBlock  altFmtBlock, *pAltFmtBlock = NULL;

    if (updatingPrimary)
        {
        dimElement.GetElementP()->ToDimensionElmR().text.b.hasAltFormat = GetResolvedBoolean (dimElement.GetElementP()->ToDimensionElm().text.b.hasAltFormat, DIMSTYLE_PROP_Value_AltIsActive_BOOLINT);

        blockType           = ADBLK_ALTFORMAT;
        useAltFormat        = dimElement.GetElementP()->ToDimensionElm().text.b.hasAltFormat;

        thresholdProp       = DIMSTYLE_PROP_Value_AltThreshold_DOUBLE;
        thresholdEqualProp  = DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT;
        thresholdLessProp   = DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT;
        accuracyProp        = DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY;
        showLabelProp       = DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT;
        showSubUnitProp     = DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT;
        showDelimProp       = DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT;
        showMasterProp      = DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT;
        zeroMasterProp      = DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT;
        zeroSubProp         = DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT;

        if (dimElement.GetElementP()->ToDimensionElm().frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
            useAltFormat = dimElement.GetElementP()->ToDimensionElmR().text.b.hasAltFormat = false;
        }
    else
        {
        dimElement.GetElementP()->ToDimensionElmR().text.b.hasSecAltFormat = GetResolvedBoolean (dimElement.GetElementP()->ToDimensionElm().text.b.hasSecAltFormat, DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT);

        blockType           = ADBLK_SECALTFORMAT;
        useAltFormat        = dimElement.GetElementP()->ToDimensionElm().text.b.hasSecAltFormat;

        thresholdProp       = DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE;
        thresholdEqualProp  = DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT;
        thresholdLessProp   = DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT;
        accuracyProp        = DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY;
        showLabelProp       = DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT;
        showSubUnitProp     = DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT;
        showDelimProp       = DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT;
        showMasterProp      = DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT;
        zeroMasterProp      = DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT;
        zeroSubProp         = DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT;

        if (dimElement.GetElementP()->ToDimensionElm().frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
            useAltFormat = dimElement.GetElementP()->ToDimensionElmR().text.b.hasSecAltFormat = false;
        }

    if (useAltFormat)
        {
        /*----------------------------------------------------------------------
           Extract existing block or create a new one
        ----------------------------------------------------------------------*/
        if (NULL != (pAltFmtBlock = (DimAltFmtBlock*) mdlDim_getOptionBlock (dimElement, blockType, NULL)))
            {
            altFmtBlock = *pAltFmtBlock;
            }
        else
            {
            memset (&altFmtBlock, 0, sizeof (altFmtBlock));

            altFmtBlock.nWords   = sizeof (altFmtBlock) / 2;
            altFmtBlock.type     = blockType;
            }

        /*----------------------------------------------------------------------
           Update block to match the style properties
        ----------------------------------------------------------------------*/
        UpdateDouble (altFmtBlock.thresholdValue, thresholdProp);

        altFmtBlock.flags.equalToThreshold     =   GetResolvedBoolean (   altFmtBlock.flags.equalToThreshold,     thresholdEqualProp);
        altFmtBlock.flags.greaterThanThreshold = ! GetResolvedBoolean ( ! altFmtBlock.flags.greaterThanThreshold, thresholdLessProp);

        altFmtBlock.flags.accuracy = GetResolvedAccuracy (altFmtBlock.flags.accuracy, accuracyProp);

        altFmtBlock.flags.adp_subunits    =   GetResolvedBoolean (   altFmtBlock.flags.adp_subunits,    showSubUnitProp);
        altFmtBlock.flags.adp_label       =   GetResolvedBoolean (   altFmtBlock.flags.adp_label,       showLabelProp);
        altFmtBlock.flags.adp_delimiter   =   GetResolvedBoolean (   altFmtBlock.flags.adp_delimiter,   showDelimProp);
        altFmtBlock.flags.adp_nomastunits = ! GetResolvedBoolean ( ! altFmtBlock.flags.adp_nomastunits, showMasterProp);

        altFmtBlock.flags.adp_allowZeroMast = GetResolvedBoolean (   altFmtBlock.flags.adp_allowZeroMast, zeroMasterProp);
        altFmtBlock.flags.adp_hideZeroSub   = GetResolvedBoolean (   altFmtBlock.flags.adp_hideZeroSub,   zeroSubProp);

        /*----------------------------------------------------------------------
           Criteria to determine if block should be added
        ----------------------------------------------------------------------*/
        addAltFmtBlock = true;
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addAltFmtBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader *) &altFmtBlock, NULL);
    else
        adim_deleteOptionBlock (dimElement, blockType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateToleranceInfo (EditElementHandleR dimElement)
    {
    bool            addTolBlock = false;
    DimTolrBlock    tolBlock, *pTolBlock = NULL;

    if (mdlDim_isNoteDimension (dimElement) || mdlDim_isLabelLineDimension (dimElement))
        {
        dimElement.GetElementP()->ToDimensionElmR().flag.tolmode   = 0;
        dimElement.GetElementP()->ToDimensionElmR().flag.tolerance = false;
        }
    else
        {
        dimElement.GetElementP()->ToDimensionElmR().flag.tolmode   = GetResolvedBoolean (dimElement.GetElementP()->ToDimensionElm().flag.tolmode, DIMSTYLE_PROP_Tolerance_Mode_BOOLINT);
        dimElement.GetElementP()->ToDimensionElmR().flag.tolerance = GetResolvedBoolean (dimElement.GetElementP()->ToDimensionElm().flag.tolerance, DIMSTYLE_PROP_Tolerance_Show_BOOLINT);
        }

    if (dimElement.GetElementP()->ToDimensionElm().flag.tolerance)
        {
        /*----------------------------------------------------------------------
           Extract existing block or create a new one
        ----------------------------------------------------------------------*/
        if (NULL != (pTolBlock = (DimTolrBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL)))
            {
            tolBlock = *pTolBlock;
            }
        else
            {
            memset (&tolBlock, 0, sizeof (tolBlock));

            tolBlock.nWords   = sizeof (tolBlock) / 2;
            tolBlock.type     = ADBLK_TOLERANCE;
            }

        /*----------------------------------------------------------------------
           Update block to match the style properties
        ----------------------------------------------------------------------*/
        tolBlock.flags.signForZero  = GetResolvedBoolean (tolBlock.flags.signForZero, DIMSTYLE_PROP_Tolerance_ShowSignForZero_BOOLINT);
        tolBlock.flags.stackEqual   = GetResolvedBoolean (tolBlock.flags.stackEqual, DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT);

        UpdateDouble (tolBlock.lowerTol, DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE);
        UpdateDouble (tolBlock.upperTol, DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE);

        UpdateWidthRelativeDistance  (tolBlock.tolWidth,    DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE);
        UpdateHeightRelativeDistance (tolBlock.tolHeight,   DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE);

        UpdateHeightRelativeDistance (tolBlock.tolHorizSep, DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE);
        UpdateHeightRelativeDistance (tolBlock.tolVertSep,  DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE);

        if (! IsPropertyShielded (DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR))
            {
            int pmType;
            m_dimStyle.GetIntegerProp (pmType, DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER);

            if (DIMSTYLE_VALUE_Symbol_CustomType_Character == pmType)
                m_dimStyle.GetCharProp (tolBlock.pmChar, DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR);
            else
                tolBlock.pmChar = 0;
            }

        tolBlock.pmFont      = 0;

        UpdateChar (tolBlock.prefix, DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR);
        UpdateChar (tolBlock.suffix, DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR);

        /*----------------------------------------------------------------------
           Criteria to determine if block should be added
        ----------------------------------------------------------------------*/
        if (tolBlock.lowerTol || tolBlock.upperTol)
            addTolBlock = true;
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addTolBlock)
        {
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader *) &tolBlock, NULL);
        }
    else
        {
        dimElement.GetElementP()->ToDimensionElmR().flag.tolerance = false;
        adim_deleteOptionBlock (dimElement, ADBLK_TOLERANCE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimPropUpdater::GetDimSymbol
(
DimSymbol          *pDimSym,        /* <= */
int                 type,           /* => */
DimStyleProp        charProp,       /* => */
DimStyleProp        fontProp,       /* => */
DimStyleProp        cellProp,       /* => */
DgnProjectR project,         /* => */
UInt64&          cellId
)
    {
    switch (type)
        {
        case 0:
            {
            memset (pDimSym, 0, sizeof(*pDimSym));
            break;
            }
        case 1:
            {
            UpdateChar (pDimSym->symbol.symb, charProp);
            UpdateFont (pDimSym->symbol.font, fontProp);

            break;
            }
        case 2:
        case 3:
            {
            if (! IsPropertyShielded (cellProp))
                {
                WString cellName;

                m_dimStyle.GetStringProp (cellName, cellProp);
                cellId = GetSharedCellID (cellName.c_str());
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimPropUpdater::UpdateTerminatorInfo (EditElementHandleR dimElement)
    {
    bool            addTermBlock = false;
    DimTermBlock    termBlock, *pTermBlock = NULL;

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    if (NULL != (pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, NULL)))
        {
        termBlock = *pTermBlock;
        }
    else
        {
        memset (&termBlock, 0, sizeof (termBlock));

        termBlock.nWords = sizeof(termBlock) / 2;
        termBlock.type   = ADBLK_TERMINATOR;

        termBlock.scScale = 1.0;
        }

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    termBlock.flags.arrow  = GetResolvedInteger (termBlock.flags.arrow,  DIMSTYLE_PROP_Terminator_ArrowType_INTEGER);
    termBlock.flags.stroke = GetResolvedInteger (termBlock.flags.stroke, DIMSTYLE_PROP_Terminator_StrokeType_INTEGER);
    termBlock.flags.origin = GetResolvedInteger (termBlock.flags.origin, DIMSTYLE_PROP_Terminator_OriginType_INTEGER);
    termBlock.flags.dot    = GetResolvedInteger (termBlock.flags.dot,    DIMSTYLE_PROP_Terminator_DotType_INTEGER);

    bool uniformScale;
    m_dimStyle.GetBooleanProp (uniformScale, DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT);

    if (uniformScale)
        {
        /*-----------------------------------------------------------------------
        Terminator height is disabled.  Terminator width acts as a scale factor
        for shared cell terminators.
        -----------------------------------------------------------------------*/
        termBlock.scScale = dimElement.GetElementCP()->ToDimensionElm().geom.termWidth;

        if (2 == termBlock.flags.arrow)
            termBlock.flags.arrow = 3;
        if (2 == termBlock.flags.stroke)
            termBlock.flags.stroke = 3;
        if (2 == termBlock.flags.origin)
            termBlock.flags.origin = 3;
        if (2 == termBlock.flags.dot)
            termBlock.flags.dot = 3;
        }

    UInt64 cellId[4] = {0};
    if (termBlock.flags.arrow || termBlock.flags.stroke || termBlock.flags.origin || termBlock.flags.dot)
        {
        DgnProjectP project = dimElement.GetDgnProject();

        addTermBlock = true;

        GetDimSymbol (&termBlock.arrow,  termBlock.flags.arrow,
                      DIMSTYLE_PROP_Terminator_ArrowChar_CHAR,
                      DIMSTYLE_PROP_Terminator_ArrowFont_FONT,
                      DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR,
                      *project, cellId[DEPENDENCYAPPVALUE_ArrowHeadTerminator-1]);

        GetDimSymbol (&termBlock.stroke,  termBlock.flags.stroke,
                      DIMSTYLE_PROP_Terminator_StrokeChar_CHAR,
                      DIMSTYLE_PROP_Terminator_StrokeFont_FONT,
                      DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR,
                      *project, cellId[DEPENDENCYAPPVALUE_StrokeTerminator-1]);

        GetDimSymbol (&termBlock.origin,  termBlock.flags.origin,
                      DIMSTYLE_PROP_Terminator_OriginChar_CHAR,
                      DIMSTYLE_PROP_Terminator_OriginFont_FONT,
                      DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR,
                      *project, cellId[DEPENDENCYAPPVALUE_OriginTerminator-1]);

        GetDimSymbol (&termBlock.dot,  termBlock.flags.dot,
                      DIMSTYLE_PROP_Terminator_DotChar_CHAR,
                      DIMSTYLE_PROP_Terminator_DotFont_FONT,
                      DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR,
                      *project, cellId[DEPENDENCYAPPVALUE_DotTerminator-1]);
        }

    // Need to store scScale if the note terminator type is uniform cell scale
    if (false == addTermBlock && uniformScale)
        {
        int iNoteTerminatorType = 0;
        m_dimStyle.GetIntegerProp (iNoteTerminatorType, DIMSTYLE_PROP_Terminator_NoteType_INTEGER);
        if (iNoteTerminatorType > 1)
            addTermBlock = true;
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addTermBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*)&termBlock, cellId);
    else
        adim_deleteOptionBlock (dimElement, ADBLK_TERMINATOR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdatePrefixSuffixInfo (EditElementHandleR dimElement)
    {
    bool            addCharBlock = false;
    DimPreSufBlock  charBlock, *pCharBlock = NULL;

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    if (NULL != (pCharBlock = (DimPreSufBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_PRESUF, NULL)))
        {
        charBlock = *pCharBlock;
        }
    else
        {
        memset (&charBlock, 0, sizeof (charBlock));

        charBlock.nWords   = sizeof (charBlock) / 2;
        charBlock.type     = ADBLK_PRESUF;
        }

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    if (dimElement.GetElementCP()->ToDimensionElm().flag.dual)
        {
        UpdateChar (charBlock.upperPrefix, DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR);
        UpdateChar (charBlock.upperSuffix, DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR);
        UpdateChar (charBlock.lowerPrefix, DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR);
        UpdateChar (charBlock.lowerSuffix, DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR);
        }
    else
        {
        UpdateChar (charBlock.mainPrefix, DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR);
        UpdateChar (charBlock.mainSuffix, DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR);
        }

    if (charBlock.mainPrefix  || charBlock.mainSuffix  ||
        charBlock.upperPrefix || charBlock.upperSuffix ||
        charBlock.lowerPrefix || charBlock.lowerSuffix)
        {
        addCharBlock = true;
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addCharBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*)&charBlock, NULL);
    else
        adim_deleteOptionBlock (dimElement, ADBLK_PRESUF);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateCustomDiameterInfo (EditElementHandleR dimElement)
    {
    UInt16          symbolType = DIMSTYLE_VALUE_Symbol_CustomType_Default;
    bool            addSymBlock = false;
    DimSymBlock     symBlock, *pSymBlock = NULL;

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    if (NULL != (pSymBlock = (DimSymBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_DIAMSYM, NULL)))
        {
        symBlock = *pSymBlock;

        // The existence of the block implies type 1
        symbolType = DIMSTYLE_VALUE_Symbol_CustomType_Character;
        }
    else
        {
        memset (&symBlock, 0, sizeof (symBlock));

        symBlock.nWords   = sizeof (symBlock) / 2;
        symBlock.type     = ADBLK_DIAMSYM;
        }

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    UpdateInteger (symbolType, DIMSTYLE_PROP_Symbol_DiameterType_INTEGER);

    if (DIMSTYLE_VALUE_Symbol_CustomType_Character == symbolType)
        {
        UpdateChar (symBlock.symChar, DIMSTYLE_PROP_Symbol_DiameterChar_CHAR);
        UpdateFont (symBlock.symFont, DIMSTYLE_PROP_Symbol_DiameterFont_FONT);

        addSymBlock = 0 != symBlock.symChar;
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addSymBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*)&symBlock, NULL);
    else
        adim_deleteOptionBlock (dimElement, ADBLK_DIAMSYM);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdatePrefixSuffixSymbolInfo
(
EditElementHandleR dimElement,
bool                updatingPrefix
)
    {
    UInt16          symbolType = DIMSTYLE_VALUE_Symbol_PreSufType_None;
    byte            blockType;
    UShort          depAppValue;
    bool            addSymBlock = false;
    UInt64       prevCellID = 0, cellID = 0;
    DimSymBlock     symBlock, *pSymBlock = NULL;
    DimStyleProp    symTypeProp, symCharProp, symFontProp, symCellProp;

    if (updatingPrefix)
        {
        blockType       = ADBLK_PRESYMBOL;
        depAppValue     = DEPENDENCYAPPVALUE_PrefixCell;
        symTypeProp     = DIMSTYLE_PROP_Symbol_PrefixType_INTEGER;
        symCharProp     = DIMSTYLE_PROP_Symbol_PrefixChar_CHAR;
        symFontProp     = DIMSTYLE_PROP_Symbol_PrefixFont_FONT;
        symCellProp     = DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR;
        }
    else // updatingSuffix
        {
        blockType       = ADBLK_SUFSYMBOL;
        depAppValue     = DEPENDENCYAPPVALUE_SuffixCell;
        symTypeProp     = DIMSTYLE_PROP_Symbol_SuffixType_INTEGER;
        symCharProp     = DIMSTYLE_PROP_Symbol_SuffixChar_CHAR;
        symFontProp     = DIMSTYLE_PROP_Symbol_SuffixFont_FONT;
        symCellProp     = DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR;
        }

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    if (NULL != (pSymBlock = (DimSymBlock*) mdlDim_getOptionBlock (dimElement, blockType, NULL)))
        {
        symBlock = *pSymBlock;

        // The existence of the block implies type 1
        symbolType = DIMSTYLE_VALUE_Symbol_PreSufType_Character;
        }
    else
        {
        memset (&symBlock, 0, sizeof (symBlock));

        symBlock.nWords   = sizeof (symBlock) / 2;
        symBlock.type     = blockType;
        }

    if (SUCCESS == adim_getCellId (&prevCellID, dimElement, depAppValue))
        {
        // The existence of the dependency linkage implies type 2
        symbolType = DIMSTYLE_VALUE_Symbol_PreSufType_Cell;
        }

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    UpdateInteger (symbolType, symTypeProp);

    if (DIMSTYLE_VALUE_Symbol_PreSufType_Character == symbolType)
        {
        UpdateChar (symBlock.symChar, symCharProp);
        UpdateFont (symBlock.symFont, symFontProp);

        addSymBlock = 0 != symBlock.symChar;
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addSymBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*)&symBlock, NULL);
    else
        adim_deleteOptionBlock (dimElement, blockType);

    if (DIMSTYLE_VALUE_Symbol_PreSufType_Cell == symbolType)
        {
        if (! IsPropertyShielded (symCellProp))
            {
            WString cellName;

            m_dimStyle.GetStringProp (cellName, symCellProp);
            cellID = GetSharedCellID (cellName.c_str());
            }
        }

    if (cellID != prevCellID)
        {
        DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElement.GetElementP(), DEPENDENCYAPPID_DimensionCell, depAppValue);

        if (0 != cellID)
            {
            DgnV8ElementBlank tmpElement;
            dimElement.GetElementCP()->CopyTo (tmpElement);
            DependencyManagerLinkage::AppendSimpleLinkageToMSElement (&tmpElement, DEPENDENCYAPPID_DimensionCell, depAppValue, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, cellID);
            dimElement.ReplaceElement (&tmpElement);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateTermSymbologyInfo (EditElementHandleR dimElement)
    {
    bool                addSymbBlock = false;
    DimTermSymbBlock    symbBlock, *pSymbBlock = NULL;

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    if (NULL != (pSymbBlock = (DimTermSymbBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMSYMB, NULL)))
        {
        symbBlock = *pSymbBlock;
        }
    else
        {
        memset (&symbBlock, 0, sizeof (symbBlock));

        symbBlock.nWords   = sizeof (symbBlock) / 2;
        symbBlock.type     = ADBLK_TERMSYMB;
        }

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    bool    useSymb;

    UInt32  styleColor;
    if (GetStyleColor (styleColor, useSymb, DIMSTYLE_PROP_Terminator_Color_COLOR))
        {
        symbBlock.termSymb.useColor = useSymb;
        symbBlock.termSymb.color    = styleColor;
        }

    UInt32  styleWeight;
    if (GetStyleWeight (styleWeight, useSymb, DIMSTYLE_PROP_Terminator_Weight_WEIGHT))
        {
        symbBlock.termSymb.useWeight = useSymb;
        symbBlock.termSymb.weight    = styleWeight;
        }

    Int32  styleLineStyle;
    if (GetStyleLineStyle (styleLineStyle, useSymb, DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE))
        {
        symbBlock.termSymb.useStyle = useSymb;
        symbBlock.termSymb.style    = styleLineStyle;
        }

    /*----------------------------------------------------------------------
       Criteria to determine if block should be added
    ----------------------------------------------------------------------*/
    if (symbBlock.termSymb.useStyle || symbBlock.termSymb.useWeight || symbBlock.termSymb.useColor)
        addSymbBlock = true;

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addSymbBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*) &symbBlock, NULL);
    else
        adim_deleteOptionBlock (dimElement, ADBLK_TERMSYMB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateBallAndChainInfo (EditElementHandleR dimElement)
    {
    bool                bncActive = false;
    bool                addBNCBlock = false;
    DimOffsetBlock      bncBlock, *pBNCBlock = NULL;

    /*----------------------------------------------------------------------
       Extract existing block or create a new one
    ----------------------------------------------------------------------*/
    if (NULL != (pBNCBlock = (DimOffsetBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_EXTOFFSET, NULL)))
        {
        bncBlock = *pBNCBlock;

        // The existence of the block implies that ball and chain is active
        bncActive = true;
        }
    else
        {
        memset (&bncBlock, 0, sizeof (bncBlock));

        bncBlock.nWords   = sizeof (bncBlock) / 2;
        bncBlock.type     = ADBLK_EXTOFFSET;
        }

    /*----------------------------------------------------------------------
       Update block to match the style properties
    ----------------------------------------------------------------------*/
    if (! IsPropertyShielded (DIMSTYLE_PROP_BallAndChain_Mode_INTEGER))
        {
        DimStyleProp_BallAndChain_Mode  activeMode = DIMSTYLE_VALUE_BallAndChain_Mode_None;
        m_dimStyle.GetIntegerProp ((int&)activeMode, DIMSTYLE_PROP_BallAndChain_Mode_INTEGER);
        // just add the block for enable or auto mode, the auto mode flag will be set in the extensions later on.
        bncActive = activeMode != DIMSTYLE_VALUE_BallAndChain_Mode_None;
        }

    if (bncActive)
        {
        addBNCBlock = true;

        bncBlock.flags.chainType       = GetResolvedInteger (bncBlock.flags.chainType,       DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER);
        bncBlock.flags.terminator      = GetResolvedInteger (bncBlock.flags.terminator,      DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER);
        bncBlock.flags.elbow           = GetResolvedBoolean (bncBlock.flags.elbow,           DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT);
        bncBlock.flags.alignment       = GetResolvedInteger (bncBlock.flags.alignment,       DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER);
        bncBlock.flags.noDockOnDimLine = GetResolvedBoolean (bncBlock.flags.noDockOnDimLine, DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT);
        }

    /*----------------------------------------------------------------------
       Add or remove block
    ----------------------------------------------------------------------*/
    if (addBNCBlock)
        mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader*) &bncBlock, NULL);
    else
        adim_deleteOptionBlock (dimElement, ADBLK_EXTOFFSET);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimPropUpdater::UpdateTemplateInfo
(
EditElementHandleR  dimElement,
DimensionType       templateNum
)
    {
    DimensionElmR dimEl = dimElement.GetElementP()->ToDimensionElmR();
    dimEl.tmpl.first_term     = GetResolvedTemplateFlag (dimEl.tmpl.first_term,     DIMSTYLE_PROP_Terminator_First_TEMPLATEFLAG,     templateNum);
    dimEl.tmpl.left_term      = GetResolvedTemplateFlag (dimEl.tmpl.left_term,      DIMSTYLE_PROP_Terminator_Left_TEMPLATEFLAG,      templateNum);
    dimEl.tmpl.right_term     = GetResolvedTemplateFlag (dimEl.tmpl.right_term,     DIMSTYLE_PROP_Terminator_Right_TEMPLATEFLAG,     templateNum);
    dimEl.tmpl.bowtie_symbol  = GetResolvedTemplateFlag (dimEl.tmpl.bowtie_symbol,  DIMSTYLE_PROP_Terminator_Joint_TEMPLATEFLAG,     templateNum);
    dimEl.tmpl.pre_symbol     = GetResolvedTemplateFlag (dimEl.tmpl.pre_symbol,     DIMSTYLE_PROP_Symbol_Prefix_TEMPLATEFLAG,        templateNum);
    dimEl.tmpl.stacked        = GetResolvedTemplateFlag (dimEl.tmpl.stacked,        DIMSTYLE_PROP_General_Stacked_TEMPLATEFLAG,      templateNum);
    dimEl.tmpl.post_symbol    = GetResolvedTemplateFlag (dimEl.tmpl.post_symbol,    DIMSTYLE_PROP_Symbol_Suffix_TEMPLATEFLAG,        templateNum);
    dimEl.tmpl.above_symbol   = GetResolvedTemplateFlag (dimEl.tmpl.above_symbol,   DIMSTYLE_PROP_Text_ArcLengthSymbol_TEMPLATEFLAG, templateNum);
    dimEl.tmpl.left_witness   = GetResolvedTemplateFlag (dimEl.tmpl.left_witness,   DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG,   templateNum);
    dimEl.tmpl.right_witness  = GetResolvedTemplateFlag (dimEl.tmpl.right_witness,  DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG,  templateNum);
    dimEl.tmpl.vertical_text  = GetResolvedTemplateFlag (dimEl.tmpl.vertical_text,  DIMSTYLE_PROP_Text_Vertical_TEMPLATEFLAG,        templateNum);
    dimEl.tmpl.nofit_vertical = GetResolvedTemplateFlag (dimEl.tmpl.nofit_vertical, DIMSTYLE_PROP_Text_NoFitVertical_TEMPLATEFLAG,   templateNum);
    dimEl.tmpl.altExt         = GetResolvedTemplateFlag (dimEl.tmpl.altExt,       DIMSTYLE_PROP_ExtensionLine_AngleChordAlign_TEMPLATEFLAG, templateNum);

    if (m_shieldFlags.GetTemplateBit (DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG, templateNum))
        return;
    
    UShort val;
    if (SUCCESS != m_dimStyle.GetTemplateFlagProp (val, templateNum, DIMSTYLE_PROP_General_ShowCenterMark_TEMPLATEFLAG))
        BeAssert (0);
    else
        dimEl.tmpl.centermark = val;

    if (SUCCESS != m_dimStyle.GetTemplateFlagProp (val, templateNum, DIMSTYLE_PROP_General_ShowCenterMarkLeft_TEMPLATEFLAG))
        BeAssert (0);
    else
        dimEl.tmpl.centerLeft = val;

    if (SUCCESS != m_dimStyle.GetTemplateFlagProp (val, templateNum, DIMSTYLE_PROP_General_ShowCenterMarkRight_TEMPLATEFLAG))
        BeAssert (0);
    else
        dimEl.tmpl.centerRight = val;

    if (SUCCESS != m_dimStyle.GetTemplateFlagProp (val, templateNum, DIMSTYLE_PROP_General_ShowCenterMarkTop_TEMPLATEFLAG))
        BeAssert (0);
    else
        dimEl.tmpl.centerTop = val;

    if (SUCCESS != m_dimStyle.GetTemplateFlagProp (val, templateNum, DIMSTYLE_PROP_General_ShowCenterMarkBottom_TEMPLATEFLAG))
        BeAssert (0);
    else
        dimEl.tmpl.centerBottom = val;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimPropUpdater::UpdateStyleExtensions
(
EditElementHandleR     dimElement,
IDimCreateDataCP    createData,
int                 option,
bool                bPropagateStyleAnnotationScaleFlag
)
    {
    bool                      linkageChanged = false;
    DimStyleExtensions const* pStyleExtensions = &m_dimStyle.GetStyleExtensionsCP();
    DimStyleExtensions        elmExtensions;

    /*----------------------------------------------------------------------
       Extract existing data from the dimension (if any)
    ----------------------------------------------------------------------*/
    memset (&elmExtensions, 0, sizeof (elmExtensions));
    mdlDim_getStyleExtension (&elmExtensions, dimElement);

    /*----------------------------------------------------------------------
       Update element data to match the style properties
    ----------------------------------------------------------------------*/
    if (! IsPropertyShielded (DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY))
        {
        bool    styleMod = pStyleExtensions->modifiers & STYLE_Extension_PrimaryToleranceAccuracy;
        bool    dimMod   = elmExtensions.modifiers & STYLE_Extension_PrimaryToleranceAccuracy;

        if (styleMod != dimMod || pStyleExtensions->primaryTolAcc != elmExtensions.primaryTolAcc)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_PrimaryToleranceAccuracy)
                {
                elmExtensions.modifiers |= STYLE_Extension_PrimaryToleranceAccuracy;
                elmExtensions.primaryTolAcc = pStyleExtensions->primaryTolAcc;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_PrimaryToleranceAccuracy;
                elmExtensions.primaryTolAcc = 0;
                }

            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY))
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_SecondaryToleranceAccuracy);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_SecondaryToleranceAccuracy);

        if (styleMod != dimMod || pStyleExtensions->secondaryTolAcc != elmExtensions.secondaryTolAcc)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_SecondaryToleranceAccuracy)
                {
                elmExtensions.modifiers |= STYLE_Extension_SecondaryToleranceAccuracy;
                elmExtensions.secondaryTolAcc = pStyleExtensions->secondaryTolAcc;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_SecondaryToleranceAccuracy;
                elmExtensions.secondaryTolAcc = 0;
                }

            linkageChanged = true;
            }
        }

    if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_RoundOff_DOUBLE))
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_RoundOff);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_RoundOff);

        if (styleMod != dimMod || pStyleExtensions->dRoundOff != elmExtensions.dRoundOff)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_RoundOff)
                {
                elmExtensions.modifiers |= STYLE_Extension_RoundOff;
                elmExtensions.dRoundOff = pStyleExtensions->dRoundOff;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_RoundOff;
                elmExtensions.dRoundOff = 0;
                }

            linkageChanged = true;
            }
        }

    if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_SecRoundOff_DOUBLE))
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_SecondaryRoundOff);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_SecondaryRoundOff);

        if (styleMod != dimMod || pStyleExtensions->dSecondaryRoundOff != elmExtensions.dSecondaryRoundOff)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_SecondaryRoundOff)
                {
                elmExtensions.modifiers |= STYLE_Extension_SecondaryRoundOff;
                elmExtensions.dSecondaryRoundOff = pStyleExtensions->dSecondaryRoundOff;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_SecondaryRoundOff;
                elmExtensions.dSecondaryRoundOff = 0;
                }

            linkageChanged = true;
            }
        }

    if ( ! IsPropertyShielded (DIMSTYLE_PROP_Text_StackedFractionScale_DOUBLE))
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_StackedFractionScale);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_StackedFractionScale);

        if (styleMod != dimMod || pStyleExtensions->stackedFractionScale != elmExtensions.stackedFractionScale)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_StackedFractionScale &&
                0.0 <  pStyleExtensions->stackedFractionScale &&
                1.0 != pStyleExtensions->stackedFractionScale)
                {
                elmExtensions.modifiers |= STYLE_Extension_StackedFractionScale;
                elmExtensions.stackedFractionScale = pStyleExtensions->stackedFractionScale;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_StackedFractionScale;
                elmExtensions.stackedFractionScale = 0.0;
                }

            linkageChanged = true;
            }
        }

    if ( ! IsPropertyShielded (DIMSTYLE_PROP_Text_InlineTextLift_DOUBLE))
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_InlineTextLift);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_InlineTextLift);

        if (styleMod != dimMod || pStyleExtensions->dInlineTextLift != elmExtensions.dInlineTextLift)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_InlineTextLift &&
                !LegacyMath::DEqual(pStyleExtensions->dInlineTextLift, 0.0))
                {
                elmExtensions.modifiers |= STYLE_Extension_InlineTextLift;
                elmExtensions.dInlineTextLift = pStyleExtensions->dInlineTextLift;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_InlineTextLift;
                elmExtensions.dInlineTextLift = 0.0;
                }

            linkageChanged = true;
            }
        }

    if (true) /* no shield for dwg specifics */
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_Dwg_Flags);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_Dwg_Flags);

        if (styleMod != dimMod || 0 != memcmp(&pStyleExtensions->dwgSpecifics.flags, &elmExtensions.dwgSpecifics.flags, sizeof(elmExtensions).dwgSpecifics.flags))
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_Dwg_Flags)
                {
                elmExtensions.modifiers |= STYLE_Extension_Dwg_Flags;
                elmExtensions.dwgSpecifics.flags = pStyleExtensions->dwgSpecifics.flags;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_Dwg_Flags;
                memset (&elmExtensions.dwgSpecifics.flags, 0, sizeof(elmExtensions).dwgSpecifics.flags);
                }

            linkageChanged = true;
            }
        }

    if ( ! IsPropertyShielded (DIMSTYLE_PROP_BallAndChain_ElbowLength_DOUBLE))
        {
        bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_BncElbowLength);
        bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_BncElbowLength);

        if (styleMod != dimMod || pStyleExtensions->dBncElbowLength != elmExtensions.dBncElbowLength)
            {
            if (pStyleExtensions->modifiers & STYLE_Extension_BncElbowLength)
                {
                elmExtensions.modifiers |= STYLE_Extension_BncElbowLength;
                elmExtensions.dBncElbowLength = pStyleExtensions->dBncElbowLength * dimElement.GetElementCP()->ToDimensionElm().text.height;
                }
            else
                {
                elmExtensions.modifiers &= ~STYLE_Extension_BncElbowLength;
                elmExtensions.dBncElbowLength = 0.0;
                }

            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_BallAndChain_UseElbowLength_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags3.uUseBncElbowLength != pStyleExtensions->flags3.uUseBncElbowLength)
            {
            elmExtensions.flags3.uUseBncElbowLength = pStyleExtensions->flags3.uUseBncElbowLength;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_General_SuppressUnfitTerminators_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags3.uNoTermsOutside != pStyleExtensions->flags3.uNoTermsOutside)
            {
            elmExtensions.flags3.uNoTermsOutside = pStyleExtensions->flags3.uNoTermsOutside;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_General_TightFitTextAbove_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags3.uTightFitTextAbove != pStyleExtensions->flags3.uTightFitTextAbove)
            {
            elmExtensions.flags3.uTightFitTextAbove = pStyleExtensions->flags3.uTightFitTextAbove;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_BallAndChain_Mode_INTEGER))
        {
        // only set ball&chain auto mode here, the enable mode has been set in adim_updateBallAndChainInfo.
        if (elmExtensions.flags3.uAutoBallNChain != pStyleExtensions->flags3.uAutoBallNChain)
            {
            elmExtensions.flags3.uAutoBallNChain = pStyleExtensions->flags3.uAutoBallNChain;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_General_FitInclinedTextBox_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags3.uFitInclinedTextBox != pStyleExtensions->flags3.uFitInclinedTextBox)
            {
            elmExtensions.flags3.uFitInclinedTextBox = pStyleExtensions->flags3.uFitInclinedTextBox;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT))
        {
        DimensionType           dimensionType = static_cast<DimensionType>(dimElement.GetElementCP()->ToDimensionElm().dimcmd);
        if (DIM_RADIAL(dimensionType) || DIM_ORDINATE(dimensionType))
            {
            /*---------------------------------------------------------------------------
            Shield radial & ordinate dimensions from extending underline for now as this
            new setting will be used to support the same behavior for radial & ordinate
            dimensions in the future.
            ---------------------------------------------------------------------------*/
            m_shieldFlags.SetPropertyBit (DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT, true);
            }
        else
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags3.uExtendDimLineUnderText != pStyleExtensions->flags3.uExtendDimLineUnderText)
                {
                elmExtensions.flags3.uExtendDimLineUnderText = pStyleExtensions->flags3.uExtendDimLineUnderText;
                linkageChanged = true;
                }
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_General_UseMinLeader_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags3.uIgnoreMinLeader != pStyleExtensions->flags3.uIgnoreMinLeader)
            {
            elmExtensions.flags3.uIgnoreMinLeader = pStyleExtensions->flags3.uIgnoreMinLeader;
            linkageChanged = true;
            }
        }

    if (mdlDim_isNoteDimension (dimElement))
        {
        if ( ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE))
            {
            bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_NoteElbowLength);
            bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_NoteElbowLength);

            // Legacy dims had a hard-coded value of 2.0. Therefore, set it to 2.0 if style does not provide a value.
            if (styleMod != dimMod || !styleMod || pStyleExtensions->dNoteElbowLength != elmExtensions.dNoteElbowLength)
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_NoteElbowLength)
                    elmExtensions.dNoteElbowLength = pStyleExtensions->dNoteElbowLength;
                else
                    elmExtensions.dNoteElbowLength = 2.0;

                elmExtensions.modifiers |= STYLE_Extension_NoteElbowLength;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE))
            {
            bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_NoteLeftMargin);
            bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_NoteLeftMargin);

            // Legacy dims had a hard-coded value of 0.5. Therefore, set it to 0.5 if style does not provide a value.
            if (styleMod != dimMod || !styleMod || pStyleExtensions->dNoteLeftMargin != elmExtensions.dNoteLeftMargin)
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_NoteLeftMargin)
                    elmExtensions.dNoteLeftMargin = pStyleExtensions->dNoteLeftMargin;
                else
                    elmExtensions.dNoteLeftMargin = 0.5;

                elmExtensions.modifiers |= STYLE_Extension_NoteLeftMargin;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE))
            {
            bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_NoteLowerMargin);
            bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_NoteLowerMargin);

            // Legacy dims had a hard-coded value of 0.5. Therefore, set it to 0.5 if style does not provide a value.
            if (styleMod != dimMod || !styleMod || pStyleExtensions->dNoteLowerMargin != elmExtensions.dNoteLowerMargin)
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_NoteLowerMargin)
                    elmExtensions.dNoteLowerMargin = pStyleExtensions->dNoteLowerMargin;
                else
                    elmExtensions.dNoteLowerMargin = 0.5;

                elmExtensions.modifiers |= STYLE_Extension_NoteLowerMargin;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_FrameScale_DOUBLE))
            {
            bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_NoteFrameScale);
            bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_NoteFrameScale);

            if (styleMod != dimMod || pStyleExtensions->dNoteFrameScale != elmExtensions.dNoteFrameScale)
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_NoteFrameScale)
                    {
                    elmExtensions.modifiers |= STYLE_Extension_NoteFrameScale;
                    elmExtensions.dNoteFrameScale = pStyleExtensions->dNoteFrameScale;
                    }
                else
                    {
                    elmExtensions.modifiers &= ~STYLE_Extension_NoteFrameScale;
                    elmExtensions.dNoteFrameScale = 1.0;
                    }

                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE))
            {
            bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_AnnotationScale);
            bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_AnnotationScale);

            if (styleMod != dimMod || pStyleExtensions->dAnnotationScale != elmExtensions.dAnnotationScale)
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_AnnotationScale)
                    {
                    elmExtensions.modifiers |= STYLE_Extension_AnnotationScale;
                    elmExtensions.dAnnotationScale = pStyleExtensions->dAnnotationScale;
                    }
                else
                    {
                    elmExtensions.modifiers &= ~STYLE_Extension_AnnotationScale;
                    elmExtensions.dAnnotationScale = 1.0;
                    }

                linkageChanged = true;
                }
            }

        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER, option) &&
            ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.uMultiJustVertical != pStyleExtensions->flags.uMultiJustVertical)
                {
                elmExtensions.flags.uMultiJustVertical = pStyleExtensions->flags.uMultiJustVertical;
                linkageChanged = true;
                }
            }

        if (! IsPropertyShielded (DIMSTYLE_PROP_MLNote_LeaderType_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteLeaderType != pStyleExtensions->flags2.uNoteLeaderType)
                {
                elmExtensions.flags2.uNoteLeaderType = pStyleExtensions->flags2.uNoteLeaderType;
                linkageChanged = true;
                }
            }

        if (! IsPropertyShielded (DIMSTYLE_PROP_MLNote_ScaleFrame_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteScaleFrame != pStyleExtensions->flags2.uNoteScaleFrame)
                {
                elmExtensions.flags2.uNoteScaleFrame = pStyleExtensions->flags2.uNoteScaleFrame;
                linkageChanged = true;
                }
            }

        if (! IsPropertyShielded (DIMSTYLE_PROP_Terminator_Note_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteTerminator != pStyleExtensions->flags2.uNoteTerminator)
                {
                adim_extensionsSetNoteTerminator (elmExtensions, adim_extensionsGetNoteTerminator (*pStyleExtensions));
                linkageChanged = true;
                }
            }

        if (! IsPropertyShielded (DIMSTYLE_PROP_Terminator_NoteType_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteTerminatorType != pStyleExtensions->flags2.uNoteTerminatorType)
                {
                int iPrevDimTermType = elmExtensions.flags2.uNoteTerminatorType;
                elmExtensions.flags2.uNoteTerminatorType = pStyleExtensions->flags2.uNoteTerminatorType;

                bool uniformScale;
                m_dimStyle.GetBooleanProp (uniformScale, DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT);

                if (uniformScale && elmExtensions.flags2.uNoteTerminatorType == 2)
                    elmExtensions.flags2.uNoteTerminatorType = 3; // note the flag taking on additional meaning

                if (iPrevDimTermType != elmExtensions.flags2.uNoteTerminatorType)
                    linkageChanged = true;
                }
            }

        // Need to delete current cell dependency irrespective whether it exists or not
        DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElement.GetElementP(), DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_NoteTerminator);
        if (elmExtensions.flags2.uNoteTerminatorType)
            {
            if (elmExtensions.flags2.uNoteTerminatorType > 1 && ! IsPropertyShielded (DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR))
                {
                WString     cellName;
                UInt64 scDefId = 0;

                m_dimStyle.GetStringProp (cellName, DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR);
                scDefId = GetSharedCellID (cellName.c_str());
                DgnV8ElementBlank tmpElement;
                dimElement.GetElementCP()->CopyTo(tmpElement);
                DependencyManagerLinkage::AppendSimpleLinkageToMSElement (&tmpElement, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_NoteTerminator, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, scDefId);
                dimElement.ReplaceElement (&tmpElement);
                }
            else if (elmExtensions.flags2.uNoteTerminatorType == 1 && ! IsPropertyShielded (DIMSTYLE_PROP_Terminator_NoteChar_CHAR))
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_NoteTerminatorFont)
                    {
                    elmExtensions.modifiers |= STYLE_Extension_NoteTerminatorFont;
                    elmExtensions.iNoteTerminatorFont = pStyleExtensions->iNoteTerminatorFont;
                    }
                else
                    {
                    elmExtensions.modifiers &= ~STYLE_Extension_NoteTerminatorFont;
                    elmExtensions.iNoteTerminatorFont = 0;
                    }

                if (pStyleExtensions->modifiers & STYLE_Extension_NoteTerminatorChar)
                    {
                    elmExtensions.modifiers |= STYLE_Extension_NoteTerminatorChar;
                    elmExtensions.iNoteTerminatorChar = pStyleExtensions->iNoteTerminatorChar;
                    }
                else
                    {
                    elmExtensions.modifiers &= ~STYLE_Extension_NoteTerminatorChar;
                    elmExtensions.iNoteTerminatorChar = 0;
                    }
                }
            }

        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_TextRotation_INTEGER, option) &&
            ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_TextRotation_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteTextRotation != pStyleExtensions->flags2.uNoteTextRotation)
                {
                elmExtensions.flags2.uNoteTextRotation = pStyleExtensions->flags2.uNoteTextRotation;
                linkageChanged = true;
                }
            }

        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER, option) &&
            ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteHorAttachment != pStyleExtensions->flags2.uNoteHorAttachment)
                {
                elmExtensions.flags2.uNoteHorAttachment = pStyleExtensions->flags2.uNoteHorAttachment;
                linkageChanged = true;
                }
            }

        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER, option) &&
            ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteVerLeftAttachment != pStyleExtensions->flags2.uNoteVerLeftAttachment)
                {
                elmExtensions.flags2.uNoteVerLeftAttachment = pStyleExtensions->flags2.uNoteVerLeftAttachment;
                linkageChanged = true;
                }
            }

        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER, option) &&
            ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uNoteVerRightAttachment != pStyleExtensions->flags2.uNoteVerRightAttachment)
                {
                elmExtensions.flags2.uNoteVerRightAttachment = pStyleExtensions->flags2.uNoteVerRightAttachment;
                linkageChanged = true;
                }
            }

        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        // Note : This flag was deprecated in 8.5.0.2 and replaced by VerLeft and VerRight flags.
        // However, we need to continue propagating it from style to dimension because a value of
        // 0 for VerLeft and VerRight means the decision is defered to ElbowJointLoc.
        // My understanding is that we don't need to check for any shields before propagating it
        // because a shield on VerLeft or VerRight means their values are non-0, therefore not
        // dependent on ElbowJointLoc. Also, the attachment point is practically unaffected by
        // save-style because the cell-portion of a note does not react to style updates.
        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER, option) &&
            elmExtensions.flags.uElbowJointLocation != pStyleExtensions->flags.uElbowJointLocation)
            {
            elmExtensions.flags.uElbowJointLocation = pStyleExtensions->flags.uElbowJointLocation;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags.uNoReduceFraction != pStyleExtensions->flags.uNoReduceFraction)
            {
            elmExtensions.flags.uNoReduceFraction = pStyleExtensions->flags.uNoReduceFraction;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_NoReduceAltFraction_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags.uNoReduceAltFraction != pStyleExtensions->flags.uNoReduceAltFraction)
            {
            elmExtensions.flags.uNoReduceAltFraction = pStyleExtensions->flags.uNoReduceAltFraction;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_NoReduceTolFraction_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags.uNoReduceTolFraction != pStyleExtensions->flags.uNoReduceTolFraction)
            {
            elmExtensions.flags.uNoReduceTolFraction = pStyleExtensions->flags.uNoReduceTolFraction;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_NoReduceSecFraction_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags.uNoReduceSecFraction != pStyleExtensions->flags.uNoReduceSecFraction)
            {
            elmExtensions.flags.uNoReduceSecFraction = pStyleExtensions->flags.uNoReduceSecFraction;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_NoReduceAltSecFraction_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags.uNoReduceAltSecFraction != pStyleExtensions->flags.uNoReduceAltSecFraction)
            {
            elmExtensions.flags.uNoReduceAltSecFraction = pStyleExtensions->flags.uNoReduceAltSecFraction;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_NoReduceTolSecFraction_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags.uNoReduceTolSecFraction != pStyleExtensions->flags.uNoReduceTolSecFraction)
            {
            elmExtensions.flags.uNoReduceTolSecFraction = pStyleExtensions->flags.uNoReduceTolSecFraction;
            linkageChanged = true;
            }
        }

    // Propagate the style's annotation scale override flag to dimension only if it was obeyed while creating or
    // updating the dimension
    if (bPropagateStyleAnnotationScaleFlag &&
        ! IsPropertyShielded (DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags2.uNotUseModelAnnotationScale != pStyleExtensions->flags2.uNotUseModelAnnotationScale)
            {
            elmExtensions.flags2.uNotUseModelAnnotationScale = pStyleExtensions->flags2.uNotUseModelAnnotationScale;
            linkageChanged = true;
            }
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Text_Embed_BOOLINT))
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        if (elmExtensions.flags2.uTextLocation != pStyleExtensions->flags2.uTextLocation)
            {
            elmExtensions.flags2.uTextLocation = pStyleExtensions->flags2.uTextLocation;
            linkageChanged = true;
            }
        }

    /*----------------------------------------------------------------------
       Extensions that are stored on Ordinate Dimensions only
    ----------------------------------------------------------------------*/
    if (mdlDim_isOrdinateDimension (dimElement))
        {
        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE))
            {
            bool    styleMod = TO_BOOL (pStyleExtensions->modifiers & STYLE_Extension_OrdinateDatumValue);
            bool    dimMod   = TO_BOOL (elmExtensions.modifiers & STYLE_Extension_OrdinateDatumValue);

            if (styleMod != dimMod || pStyleExtensions->dOrdinateDatumValue != elmExtensions.dOrdinateDatumValue)
                {
                if (pStyleExtensions->modifiers & STYLE_Extension_OrdinateDatumValue)
                    {
                    elmExtensions.modifiers |= STYLE_Extension_OrdinateDatumValue;
                    elmExtensions.dOrdinateDatumValue = pStyleExtensions->dOrdinateDatumValue;
                    }
                else
                    {
                    elmExtensions.modifiers &= ~STYLE_Extension_OrdinateDatumValue;
                    elmExtensions.dOrdinateDatumValue = 0.0;
                    }

                linkageChanged = true;
                }

            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.uOrdUseDatumValue != pStyleExtensions->flags.uOrdUseDatumValue)
                {
                elmExtensions.flags.uOrdUseDatumValue = pStyleExtensions->flags.uOrdUseDatumValue;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_OrdDecrementReverse_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.uOrdDecrementReverse != pStyleExtensions->flags.uOrdDecrementReverse)
                {
                elmExtensions.flags.uOrdDecrementReverse = pStyleExtensions->flags.uOrdDecrementReverse;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_OrdFreeLocation_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags2.uOrdFreeLocation != pStyleExtensions->flags2.uOrdFreeLocation)
                {
                elmExtensions.flags2.uOrdFreeLocation = pStyleExtensions->flags2.uOrdFreeLocation;
                linkageChanged = true;
                }
            }
        }

    /*----------------------------------------------------------------------
       Extensions that are stored on Label Lines only
    ----------------------------------------------------------------------*/
    if (mdlDim_isLabelLineDimension (dimElement))
        {
        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.labelLineSupressAngle != pStyleExtensions->flags.labelLineSupressAngle)
                {
                elmExtensions.flags.labelLineSupressAngle = pStyleExtensions->flags.labelLineSupressAngle;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.labelLineSupressLength != pStyleExtensions->flags.labelLineSupressLength)
                {
                elmExtensions.flags.labelLineSupressLength = pStyleExtensions->flags.labelLineSupressLength;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.labelLineInvertLabels != pStyleExtensions->flags.labelLineInvertLabels)
                {
                elmExtensions.flags.labelLineInvertLabels = pStyleExtensions->flags.labelLineInvertLabels;
                linkageChanged = true;
                }
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT))
            {
            // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
            if (elmExtensions.flags.uLabelLineAdjacentLabels != pStyleExtensions->flags.uLabelLineAdjacentLabels)
                {
                elmExtensions.flags.uLabelLineAdjacentLabels = pStyleExtensions->flags.uLabelLineAdjacentLabels;
                linkageChanged = true;
                }
            }

        /* Direction Format info is not stored in the dimstyle.  Here we update
           from the tcb only while creating a new dimension. */
        if (ADIM_PARAMS_CREATE == option || ADIM_PARAMS_CHANGE == option)
            {
            if (adim_updateExtensionsFromDirectionFormat (elmExtensions, createData->_GetDirFormat()))
                linkageChanged = true;
            }
        }

    /*----------------------------------------------------------------------
       Replace or delete the linkage from the dimension
    ----------------------------------------------------------------------*/
    if (linkageChanged)
        mdlDim_setStyleExtension (dimElement, &elmExtensions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateFitOption (EditElementHandleR dimElement)
    {
    DimStyleProp_FitOptions fitOption = DIMSTYLE_VALUE_FitOption_MoveTermsFirst;

    m_dimStyle.GetIntegerProp ((int&) fitOption, DIMSTYLE_PROP_General_FitOption_INTEGER);

    mdlDim_setFitOption(dimElement, fitOption);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 01/04
+---------------+---------------+---------------+---------------+---------------+------*/
static double      resolveAnnotationScaleForCreate
(
EditElementHandleR     dimElement,
bool                bUseStyleScaleIn,
double              dModelScaleIn,
double              dStyleScaleIn
)
    {
    double          dResolvedScale = 1.0;

    // When creating or changing dim, force apply the appropriate annot scale
    if (true == bUseStyleScaleIn)
        {
        // Apply the style's annot scale
        dResolvedScale = dStyleScaleIn;
        mdlDim_setAnnotationScale (dimElement, &dStyleScaleIn);
        }
#ifdef DGNPROJECT_MODELS_CHANGES_WIP
    else if (dgnModel_getModelFlag (dimElement.GetDgnModelP(), MODELFLAG_USE_ANNOTATION_SCALE))
        {
        // Apply model annot scale only if Use Annot Scale lock is on
        dResolvedScale = dModelScaleIn;
        mdlDim_overallSetModelAnnotationScale (dimElement, &dModelScaleIn);
        }
#endif
    else
        {
        // When changing to a non-annotationscale system, remove any existing values
        mdlDim_overallSetModelAnnotationScale (dimElement, NULL);
        mdlDim_setAnnotationScale (dimElement, NULL);
        }

    return dResolvedScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 01/04
+---------------+---------------+---------------+---------------+---------------+------*/
static double      resolveAnnotationScaleForUpdate
(
EditElementHandleR     dimElement,
bool                *bObeyedStyleOverrideFlag,
DimStylePropMaskP   shieldFlags,
bool                bUseStyleScaleIn,
double              dStyleScaleIn
)
    {
    /*--------------------------------------------------------------------------

        Notes for future improvements after the 8.11.9 release.

        This code is flawed in that it will never switch a dimension's annotation
        scale source from Style to Model.  The output variable bObeyedStyleOverrideFlag
        is only set to true if the style requests 'Style' source but never 'Model'
        source.  The calling code uses that variable to decide about updating the
        style extension that stores the source.

        Beware, the correct fix goes beyond bObeyedStyleOverrideFlag.  To be done
        right switching to Model source requires calling IAnnotationHandler::AddAnnotationScale.

        I have prototyped that code but I am not committing it for the 8.11.9 cycle
        since I consider this code to be too delicate to predict potentially bad
        side effects.  (Note to self: Josh, check your email archives subject line:
        'Switching annotation scale source').

        One complication is that Style scale does not normally effect Dimension Height
        like Model scale does.  Style scale is propagated from the style directly into
        the dim's properties.  On the other hand, Model scale is propagated by the
        Handler's _OnTransform method. After speaking with Don, we concluded that we can
        and should change Style scale to at least behaviorally  match Model scale.
        It would be nice if we could consolidate the code paths as well.

        -JS 11/28/2011

    --------------------------------------------------------------------------*/

    double          dResolvedScale               = 1.0;
    double          dPrevScale                   = 1.0;
    bool            bUseStyleScaleOnPrevDim      = true;
    bool            bPrevDimUsedModelScale       = false;
    bool            bPrevDimUsedStyleScale       = false;

    *bObeyedStyleOverrideFlag = false;

    /*--------------------------------------------------------------------------
      Get the previously used annotation scale source and value
    --------------------------------------------------------------------------*/
    if (false == mdlDim_getNotUseModelAnnotationScaleFlag (&bUseStyleScaleOnPrevDim, dimElement))
        bUseStyleScaleOnPrevDim = false;

    if (false != bUseStyleScaleOnPrevDim)
        bPrevDimUsedStyleScale = mdlDim_getAnnotationScale (&dPrevScale, dimElement);
    else
        bPrevDimUsedModelScale = mdlDim_overallGetModelAnnotationScale (&dPrevScale, dimElement);

    // bPrevDimUsedStyleScale = true  and bPrevDimUsedModelScale = true  => NOT POSSIBLE
    // bPrevDimUsedStyleScale = true  and bPrevDimUsedModelScale = false => Currently using Style Scale
    // bPrevDimUsedStyleScale = false and bPrevDimUsedModelScale = true  => Currently using Model Scale
    // bPrevDimUsedStyleScale = false and bPrevDimUsedModelScale = false => Currently supposed to use model scale but scale lock was off

    /*--------------------------------------------------------------------------
      Derive the appropriate annotation scale to use
    --------------------------------------------------------------------------*/

    // General explanation of this case:
    //    If the style currently supplies its own scale, propagate it to the dimension
    //    only if neither the scale source nor the scale value is overridden.
    //
    // The other cases are enumerated for clarity only.

    // When updating from dimstyle or textstyle, watch for the shields.
    if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT) &&
        bUseStyleScaleIn != bUseStyleScaleOnPrevDim)
        {
        // If the source is shielded, we cannot get the new annot scale from a different source
        dResolvedScale = dPrevScale;
        }
    else
        {
        // If the source is shielded and the source hasn't changed, Or the source is not shielded,
        // get the new annot scale
        if (false == bUseStyleScaleIn)
            {
            // The following code is for clarity. All it does is set the resolved scale back to the current value.
            // Justification : Since the model annot scale is propagated elsewhere, saving from style should not change it here.
            if (false == bPrevDimUsedModelScale && false == bPrevDimUsedStyleScale)
                {
                // If the dim was supposed to use model annot scale but the lock was off,
                // leave it clean (also legacy case)
                dResolvedScale = dPrevScale;
                }
            else if (bPrevDimUsedModelScale)
                {
                // Apply the same previous model annot scale if it was already using one.
                // New model annot scale cannot be propagated during save style.
                dResolvedScale = dPrevScale;
                }
            else
                {
                // Otherwise, reinstate the previous annot scale
                dResolvedScale = dPrevScale;
                }
            }
        else
            {
            // Get the annot scale from the style.
            if (isPropertyShielded (shieldFlags, DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE))
                {
                // If the style's annot scale value is shielded, cannot change it
                dResolvedScale = dPrevScale;
                }
            else
                {
                // Otherwise, use the new annot scale from style
                dResolvedScale = dStyleScaleIn;
                mdlDim_setAnnotationScale (dimElement, &dStyleScaleIn);
                *bObeyedStyleOverrideFlag = true;
                }
            }
        }

    return dResolvedScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimPropUpdater::UpdateDimensionProperties (EditElementHandleR dimElement, int option, IDimCreateDataCP createData)
    {

    bool                gotOriginalHeight= false;
    bool                usedTextStyleColor = false;
    bool                bNotUseModelAnnotScale = false;
    bool                bObeyedStyleAnnotationScaleFlag = true;
    double              originalTextHeight = 0.0;
    double              originalTextWidth = 0.0;
    double              textStyleWidthFactor = 1.0;
    double              modelAnnScale = 1.0, styleAnnScale = 1.0, resolvedAnnScale = 1.0;
    DimensionHandler&   hdlr = DimensionHandler::GetInstance();

    /*--------------------------------------------------------------------------
      tcb's distances have the model annotation scale incorporated in it. dssp's
      textsize has the model or style annotation scale incorporated in it.
      (note: model annotation scale is incorporated only if the use annot scale
      lock is on). In order to have a better handle on the propogation of textsize
      and annotation scales to the dimension, temporarily reverse the effect of
      annotation scales on the tcb's distances and dsp's textsize. Note that dimcen and
      stack_offset also have the respective annotation scale incorporated in it,
      but we are leaving them as-is.
    --------------------------------------------------------------------------*/
#ifdef WIP_V10_MODELINFO
    modelAnnScale = dgnModel_getEffectiveAnnotationScale (dimElement.GetDgnModelP());
#else
    modelAnnScale = 1.0;
#endif

    m_dimStyle.GetBooleanProp (bNotUseModelAnnotScale, DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT);
    if (false == bNotUseModelAnnotScale)
        styleAnnScale = modelAnnScale;
    else
        m_dimStyle.GetDoubleProp (styleAnnScale, DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE);

    if (fabs (styleAnnScale - 0.0) < mgds_fc_epsilon)
        styleAnnScale = 1.0;

    /*--------------------------------------------------------------------------
      When updating from a style, do not use any active settings.
    --------------------------------------------------------------------------*/
    if (ADIM_PARAMS_CREATE == option || ADIM_PARAMS_CHANGE == option)
        {
        Symbology       symbology = createData->_GetSymbology();
        DimensionElmR dimEl = dimElement.GetElementP()->ToDimensionElmR();
        dimEl.GetSymbologyR().style      = symbology.style;
        dimEl.GetSymbologyR().color      = symbology.color;
        dimEl.GetSymbologyR().weight     = symbology.weight;
        dimEl.SetLevel(createData->_GetLevelID().GetValue());

        DgnTextStyleCR  textStyle = createData->_GetTextStyle();
        double          uorScale  = 1.0;

        textStyle.GetPropertyValue (DgnTextStyleProperty::Width, dimEl.text.width);
        dimEl.text.width*= uorScale;

        textStyle.GetPropertyValue(DgnTextStyleProperty::Height, dimEl.text.height);
        dimEl.text.height*= uorScale;

        bool underline, fraction = false;
        textStyle.GetPropertyValue(DgnTextStyleProperty::IsUnderlined, underline);
        dimEl.flag.underlineText = (UShort) underline;
        dimEl.flag.stackFract    = !fraction;

        DgnFontCP  font;
        textStyle.GetPropertyValue(DgnTextStyleProperty::Font, font);

        UInt32  fontNum;
        dimElement.GetDgnProject()->Fonts().AcquireFontNumber(fontNum, *font);

        if (ADIM_PARAMS_CREATE != option)
            hdlr.ChangeFont (dimElement, fontNum, -1);
        else
            dimEl.text.font = fontNum;

        dimEl.frmt.angleMode_deprecated = createData->_GetDirFormat().GetLegacyAngleMode();

//TODO TEST Dim annotation scale
        resolvedAnnScale = resolveAnnotationScaleForCreate (dimElement, bNotUseModelAnnotScale, modelAnnScale, styleAnnScale);
        }
    else
        {
        resolvedAnnScale = resolveAnnotationScaleForUpdate (dimElement, &bObeyedStyleAnnotationScaleFlag, &m_shieldFlags, bNotUseModelAnnotScale, styleAnnScale);
        }

    /*-----------------------------------------------------------------------------------
        Associate to dimension style
    -----------------------------------------------------------------------------------*/
    mdlDim_setStyleUniqueId (dimElement, m_dimStyle.GetID());

    /*-----------------------------------------------------------------------------------
        Get the text style, and update the dimension based on it.
    -----------------------------------------------------------------------------------*/
    if (! IsPropertyShielded (DIMSTYLE_PROP_Text_TextStyleID_INTEGER))
        {
        DgnTextStylePtr     textStylePtr;

        m_dimStyle.GetTextStyleProp (textStylePtr, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE);

        dimElement.GetElementP()->ToDimensionElmR().textStyleId = textStylePtr->GetId().GetValue();

        LegacyTextStyle           dimElmTextStyle;

        if (SUCCESS != hdlr.GetTextStyle (dimElement, &dimElmTextStyle))
            {
            BeAssert (0);
            }
        else
            {
            adim_updateTextStyleInfo (dimElmTextStyle, &usedTextStyleColor, &textStyleWidthFactor, option, *textStylePtr, &m_shieldFlags, 0 != dimElement.GetElementP()->ToDimensionElm().textStyleId, resolvedAnnScale);

            /*--------------------------------------------------------------------------------
                Store the updated textstyle onto the dimension
            --------------------------------------------------------------------------------*/
            hdlr.SetTextStyle (dimElement, &dimElmTextStyle, false, true);
            }
        }

    /*-------------------------------------------------------------------
      Update display header structures
    -------------------------------------------------------------------*/
    Symbology& dimSymb = dimElement.GetElementP()->ToDimensionElmR().GetSymbologyR();
    UpdateLineStyle (dimSymb.style,  DIMSTYLE_PROP_General_LineStyle_LINESTYLE);
    UpdateColor     (dimSymb.color,  DIMSTYLE_PROP_General_Color_COLOR);
    UpdateWeight    (dimSymb.weight, DIMSTYLE_PROP_General_Weight_WEIGHT);
    UpdateLevel     ((LevelId&)dimElement.GetElementP()->ToDimensionElmR().GetLevelR(),       DIMSTYLE_PROP_Placement_Level_LEVEL);

    if (! IsPropertyShielded (DIMSTYLE_PROP_General_DimensionScale_DOUBLE))
        {
        double  refScale;
        mdlDim_overallGetRefScale (&refScale, dimElement);

        double  styleScale;
        m_dimStyle.GetDoubleProp (styleScale, DIMSTYLE_PROP_General_DimensionScale_DOUBLE);

        dimElement.GetElementP()->ToDimensionElmR().SetScale (styleScale / refScale);
        }

    /*--------------------------------------------------------------------
      Update dimension text info structure
    --------------------------------------------------------------------*/
    if (0.0 < dimElement.GetElementP()->ToDimensionElm().text.height && 0.0 < dimElement.GetElementP()->ToDimensionElm().text.width)
        {
        originalTextHeight = dimElement.GetElementP()->ToDimensionElm().text.height;
        originalTextWidth  = dimElement.GetElementP()->ToDimensionElm().text.width;

        gotOriginalHeight  = true;
        }

    if (IsUpdateNeeded (DIMSTYLE_PROP_Text_Height_DOUBLE))
        {
        double styleVal;
        m_dimStyle.GetDoubleProp (styleVal, DIMSTYLE_PROP_Text_Height_DOUBLE);

        if (styleVal)
            dimElement.GetElementP()->ToDimensionElmR().text.height  = styleVal * resolvedAnnScale;
        }

    if ( ! IsPropertyShielded (DIMSTYLE_PROP_Text_Width_DOUBLE))
        {
        double styleVal;
        m_dimStyle.GetDoubleProp (styleVal, DIMSTYLE_PROP_Text_Width_DOUBLE);

        if (IsControlledByDimStyle (DIMSTYLE_PROP_Text_Width_DOUBLE) && styleVal)
            dimElement.GetElementP()->ToDimensionElmR().text.width   = styleVal * resolvedAnnScale;
       else if (LegacyMath::DEqual(styleVal, 0.0) && !LegacyMath::DEqual(textStyleWidthFactor, 0.0))
            dimElement.GetElementP()->ToDimensionElmR().text.width   = dimElement.GetElementP()->ToDimensionElm().text.height * textStyleWidthFactor;
        }

    m_dimTextHeight            = dimElement.GetElementP()->ToDimensionElm().text.height;
    m_dimTextWidth             = dimElement.GetElementP()->ToDimensionElm().text.height;

    if (gotOriginalHeight)
        {
        m_dimTextHeightChangeRatio = dimElement.GetElementP()->ToDimensionElm().text.height / originalTextHeight;
        m_dimTextWidthChangeRatio  = dimElement.GetElementP()->ToDimensionElm().text.width  / originalTextWidth;
        }
    else
        {
        m_dimTextHeightChangeRatio = 1.0;
        m_dimTextWidthChangeRatio  = 1.0;
        }

    UInt32  dimFont = dimElement.GetElementP()->ToDimensionElm().text.font;
    if (UpdateFont (dimFont, DIMSTYLE_PROP_General_Font_FONT))
        hdlr.ChangeFont (dimElement, dimFont, 0);

    dimElement.GetElementP()->ToDimensionElmR().text.b.useColor = usedTextStyleColor || GetResolvedBoolean (dimElement.GetElementP()->ToDimensionElm().text.b.useColor, DIMSTYLE_PROP_Text_OverrideColor_BOOLINT);

    // Can't use UpdateColorProp here.  In the case where the 'useColor' flag is being
    // defined by the LegacyTextStyle, that method might prevent updating the color.
    // Another option is to change 'IsUpdateNeeded' to account for this.
    if (! IsPropertyShielded (DIMSTYLE_PROP_Text_Color_COLOR))
        {
        if (dimElement.GetElementP()->ToDimensionElm().text.b.useColor)
            m_dimStyle.GetColorProp (dimElement.GetElementP()->ToDimensionElmR().text.color, DIMSTYLE_PROP_Text_Color_COLOR);
        }

    if (! IsPropertyShielded (DIMSTYLE_PROP_Text_Weight_WEIGHT))
        {
        dimElement.GetElementP()->ToDimensionElmR().text.b.useWeight = GetResolvedBoolean (dimElement.GetElementP()->ToDimensionElm().text.b.useWeight, DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT);

        if (dimElement.GetElementP()->ToDimensionElm().text.b.useWeight)
            m_dimStyle.GetWeightProp (dimElement.GetElementP()->ToDimensionElmR().text.weight, DIMSTYLE_PROP_Text_Weight_WEIGHT);
        else
            dimElement.GetElementP()->ToDimensionElmR().text.weight = dimElement.GetElementP()->ToDimensionElm().GetSymbology().weight;
        }

    bool    useSymb;

    Int32   styleLineStyle;
    if (GetStyleLineStyle (styleLineStyle, useSymb, DIMSTYLE_PROP_ExtensionLine_LineStyle_LINESTYLE))
        dimElement.GetElementP()->ToDimensionElmR().altSymb.style = useSymb ? styleLineStyle : dimElement.GetElementP()->ToDimensionElm().GetSymbology().style;

    UInt32   styleWeight;
    if (GetStyleWeight (styleWeight, useSymb, DIMSTYLE_PROP_ExtensionLine_Weight_WEIGHT))
        dimElement.GetElementP()->ToDimensionElmR().altSymb.weight = useSymb ? styleWeight : dimElement.GetElementP()->ToDimensionElm().GetSymbology().weight;

    UInt32  styleColor;
    if (GetStyleColor (styleColor, useSymb, DIMSTYLE_PROP_ExtensionLine_Color_COLOR))
        dimElement.GetElementP()->ToDimensionElmR().altSymb.color = useSymb ? styleColor : dimElement.GetElementP()->ToDimensionElm().GetSymbology().color;

    /*--------------------------------------------------------------------
      Update dimension text format structure
    --------------------------------------------------------------------*/
    DimensionElmR dimEl = dimElement.GetElementP()->ToDimensionElmR();
    dimEl.frmt.angleMeasure = GetResolvedBoolean (dimEl.frmt.angleMeasure, DIMSTYLE_PROP_Value_AngleMeasure_BOOLINT);

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_AngleFormat_INTEGER))
        {
        int angleFormat;

        m_dimStyle.GetIntegerProp (angleFormat, DIMSTYLE_PROP_Value_AngleFormat_INTEGER);

        dimEl.frmt.angleFormat = dimEl.frmt.radians = dimEl.flag.centesimal = 0;

        switch (static_cast<AngleFormatVals>(angleFormat))
            {
            case AngleFormatVals::DegMinSec:
            case AngleFormatVals::DegMin:
                dimEl.frmt.angleFormat = 1;
                break;
            case AngleFormatVals::Radians:
                dimEl.frmt.radians     = 1;
                break;
            case AngleFormatVals::Centesimal:
                dimEl.flag.centesimal  = 1;
                break;
            }
        }

    dimEl.frmt.dmsAccuracyMode   =   GetResolvedInteger (dimEl.frmt.dmsAccuracyMode,      DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER);
    dimEl.frmt.noNonStackedSpace = ! GetResolvedBoolean (!dimEl.frmt.noNonStackedSpace, DIMSTYLE_PROP_Value_SpaceAfterNonStackedFraction_BOOLINT);

    if (! IsPropertyShielded (DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT))
        {
        bool styleVal;
        m_dimStyle.GetBooleanProp (styleVal, DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT);
        dimEl.frmt.metricSpc = styleVal;

        if (dimEl.frmt.metricSpc != 0)
            {
            m_dimStyle.GetBooleanProp (styleVal, DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT);
            dimEl.flag.thousandSep = styleVal;
            }
        }

    bool    isDegreeValue = false;

    if (dimEl.frmt.angleMeasure && mdlDim_isAngularDimension (dimElement))
        {
        dimEl.frmt.primaryAccuracy   = GetResolvedInteger (dimEl.frmt.primaryAccuracy, DIMSTYLE_PROP_Value_AnglePrecision_INTEGER);
        dimEl.frmt.secondaryAccuracy = 0;
        isDegreeValue = true;
        }
    else if (dimEl.dimcmd == static_cast<byte>(DimensionType::LabelLine))
        {
        dimEl.frmt.primaryAccuracy   = GetResolvedAccuracy (dimEl.frmt.primaryAccuracy,  DIMSTYLE_PROP_Value_Accuracy_ACCURACY);
        dimEl.frmt.secondaryAccuracy = GetResolvedInteger (dimEl.frmt.secondaryAccuracy, DIMSTYLE_PROP_Value_AnglePrecision_INTEGER);
        }
    else
        {
        dimEl.frmt.primaryAccuracy   = GetResolvedAccuracy (dimEl.frmt.primaryAccuracy,   DIMSTYLE_PROP_Value_Accuracy_ACCURACY);
        dimEl.frmt.secondaryAccuracy = GetResolvedAccuracy (dimEl.frmt.secondaryAccuracy, DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY);
        }

    dimEl.frmt.decimalComma   = GetResolvedBoolean (dimEl.frmt.decimalComma,   DIMSTYLE_PROP_Text_DecimalComma_BOOLINT);
    dimEl.frmt.superscriptLSD = GetResolvedBoolean (dimEl.frmt.superscriptLSD, DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT);
    dimEl.frmt.roundLSD       = GetResolvedBoolean (dimEl.frmt.roundLSD,       DIMSTYLE_PROP_Value_RoundLSD_BOOLINT);
    dimEl.frmt.omitLeadDelim  = GetResolvedBoolean (dimEl.frmt.omitLeadDelim,  DIMSTYLE_PROP_Text_OmitLeadingDelimiter_BOOLINT);

#if defined (BEIJING_DGNPLATFORM_WIP_JS)
    // This flag is unused, set it for V7 compatibility
    dimEl.frmt.localFileUnits = dssP->data.ad4.ext_dimflg.masterfileUnits ? false : true;
#endif

    dimEl.frmt.adp_label         =   GetResolvedBoolean (dimEl.frmt.adp_label,            DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT);
    dimEl.frmt.adp_subunits      =   GetResolvedBoolean (dimEl.frmt.adp_subunits,         DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT);
    dimEl.frmt.adp_delimiter     =   GetResolvedBoolean (dimEl.frmt.adp_delimiter,        DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT);
    dimEl.text.b.adp_nomastunits = ! GetResolvedBoolean ( ! dimEl.text.b.adp_nomastunits, DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT);
    dimEl.frmt.adp_allowZeroMast =   GetResolvedBoolean (dimEl.frmt.adp_allowZeroMast,    DIMSTYLE_PROP_Value_ShowZeroMasterUnit_BOOLINT);
    dimEl.frmt.adp_hideZeroSub   = ! GetResolvedBoolean ( ! dimEl.frmt.adp_hideZeroSub,   DIMSTYLE_PROP_Value_ShowZeroSubUnit_BOOLINT);

    dimEl.frmt.adp_label2        =   GetResolvedBoolean (dimEl.frmt.adp_label2,           DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT);
    dimEl.frmt.adp_subunits2     =   GetResolvedBoolean (dimEl.frmt.adp_subunits2,        DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT);
    dimEl.frmt.adp_delimiter2    =   GetResolvedBoolean (dimEl.frmt.adp_delimiter2,       DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT);
    dimEl.text.b.adp_nomastunits2= ! GetResolvedBoolean ( ! dimEl.text.b.adp_nomastunits2,DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT);
    dimEl.frmt.adp_allowZeroMast2=   GetResolvedBoolean (dimEl.frmt.adp_allowZeroMast2,   DIMSTYLE_PROP_Value_SecShowZeroMasterUnit_BOOLINT);
    dimEl.frmt.adp_hideZeroSub2  = ! GetResolvedBoolean ( ! dimEl.frmt.adp_hideZeroSub2,  DIMSTYLE_PROP_Value_SecShowZeroSubUnit_BOOLINT);

    /*--------------------------------------------------------------------
      Update dimension geometry structure
    --------------------------------------------------------------------*/
    UpdateHeightRelativeDistance (dimEl.geom.witOffset,  DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE);
    UpdateHeightRelativeDistance (dimEl.geom.witExtend,  DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE);
    UpdateHeightRelativeDistance (dimEl.geom.textMargin, DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE);
    UpdateHeightRelativeDistance (dimEl.geom.textLift,   DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE);

    UpdateDouble (dimEl.geom.stackOffset, DIMSTYLE_PROP_General_StackOffset_DOUBLE);

    UpdateWidthRelativeDistance  (dimEl.geom.margin,     DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE); // Relative to width
    UpdateHeightRelativeDistance (dimEl.geom.termWidth,  DIMSTYLE_PROP_Terminator_Width_DOUBLE);
    UpdateHeightRelativeDistance (dimEl.geom.termHeight, DIMSTYLE_PROP_Terminator_Height_DOUBLE);

    /*--------------------------------------------------------------------
      Update dimension flags structure
    --------------------------------------------------------------------*/
    dimEl.flag.joiner      = GetResolvedBoolean (dimEl.flag.joiner,      DIMSTYLE_PROP_ExtensionLine_Join_BOOLINT);
    dimEl.flag.boxText     = GetResolvedBoolean (dimEl.flag.boxText,     DIMSTYLE_PROP_Text_Box_BOOLINT);
    dimEl.flag.capsuleText = GetResolvedBoolean (dimEl.flag.capsuleText, DIMSTYLE_PROP_Text_Capsule_BOOLINT);

    // Must be checked AFTER dimEl.frmt.angleMeasure is set
    bool    primaryUsesAngular, secondaryUsesAngular;

    adim_checkUseOfAngularSettings (primaryUsesAngular, secondaryUsesAngular, dimElement);

    if (primaryUsesAngular)
        {
        dimEl.flag.leadingZero   = GetResolvedBoolean (dimEl.flag.leadingZero,   DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT);
        dimEl.flag.trailingZeros = GetResolvedBoolean (dimEl.flag.trailingZeros, DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT);
        }
    else
        {
        dimEl.flag.leadingZero   = GetResolvedBoolean (dimEl.flag.leadingZero,   DIMSTYLE_PROP_Text_LeadingZero_BOOLINT);
        dimEl.flag.trailingZeros = GetResolvedBoolean (dimEl.flag.trailingZeros, DIMSTYLE_PROP_Value_ShowTrailingZeros_BOOLINT);
        }

    if (secondaryUsesAngular)
        {
        dimEl.flag.leadingZero2   = GetResolvedBoolean (dimEl.flag.leadingZero2,   DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT);
        dimEl.flag.trailingZeros2 = GetResolvedBoolean (dimEl.flag.trailingZeros2, DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT);
        }
    else
        {
        dimEl.flag.leadingZero2   = GetResolvedBoolean (dimEl.flag.leadingZero2,   DIMSTYLE_PROP_Text_SecLeadingZero_BOOLINT);
        dimEl.flag.trailingZeros2 = GetResolvedBoolean (dimEl.flag.trailingZeros2, DIMSTYLE_PROP_Value_SecShowTrailingZeros_BOOLINT);
        }

    dimEl.flag.embed          = GetResolvedBoolean (dimEl.flag.embed,      DIMSTYLE_PROP_Text_Embed_BOOLINT);
    dimEl.flag.horizontal     = GetResolvedBoolean (dimEl.flag.horizontal, DIMSTYLE_PROP_Text_Horizontal_BOOLINT);
    dimEl.flag.arrowhead      = GetResolvedInteger (dimEl.flag.arrowhead,  DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER);
    dimEl.flag.noAutoTextLift = ! GetResolvedBoolean ( ! dimEl.flag.noAutoTextLift,  DIMSTYLE_PROP_Text_AutoLift_BOOLINT);
    dimEl.flag.relDimLine     = GetResolvedBoolean (dimEl.flag.relDimLine, DIMSTYLE_PROP_General_RelativeDimLine_BOOLINT);
    dimEl.flag.underlineText  = GetResolvedBoolean (dimEl.flag.underlineText, DIMSTYLE_PROP_Text_Underline_BOOLINT);
    dimEl.flag.stackFract     = GetResolvedBoolean (dimEl.flag.stackFract, DIMSTYLE_PROP_Text_StackedFractions_BOOLINT);

    dimEl.text.b.stackedFractionType  = GetResolvedInteger (dimEl.text.b.stackedFractionType,  DIMSTYLE_PROP_Text_StackedFractionType_INTEGER);
    dimEl.text.b.stackedFractionAlign = GetResolvedInteger (dimEl.text.b.stackedFractionAlign, DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER);
    dimEl.text.b.superscriptMode      = GetResolvedInteger (dimEl.text.b.superscriptMode,      DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER);

#if defined (BEIJING_DGNPLATFORM_WIP_JS) //TODO userPrefsP->compatibility.major
// need to consult the host?
    if (userPrefsP->compatibility.major >= 5)
#endif
        {
        int termMode;
        m_dimStyle.GetIntegerProp (termMode, DIMSTYLE_PROP_Terminator_Mode_INTEGER);

        if (termMode < 2)
            UpdateFitOption (dimElement);

        dimEl.flag.noLevelSymb = GetResolvedBoolean (dimEl.flag.noLevelSymb, DIMSTYLE_PROP_General_IgnoreLevelSymbology_BOOLINT);
        }

    dimEl.extFlag.noLineThruArrowTerm  = GetResolvedBoolean (dimEl.extFlag.noLineThruArrowTerm,  DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT);
    dimEl.extFlag.noLineThruStrokeTerm = GetResolvedBoolean (dimEl.extFlag.noLineThruStrokeTerm, DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT);
    dimEl.extFlag.noLineThruOriginTerm = GetResolvedBoolean (dimEl.extFlag.noLineThruOriginTerm, DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT);
    dimEl.extFlag.noLineThruDotTerm    = GetResolvedBoolean (dimEl.extFlag.noLineThruDotTerm,    DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT);

    if (mdlDim_isNoteDimension (dimElement))
        {
        if ( ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT))
            {
            bool styleVal;
            m_dimStyle.GetBooleanProp (styleVal, DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT);

            mdlDim_setNoteLeaderDisplay (dimElement, styleVal);
            }

        if ( ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_FrameType_INTEGER))
            {
            int frameType = DIMSTYLE_VALUE_MLNote_FrameType_None;
            m_dimStyle.GetIntegerProp (frameType, DIMSTYLE_PROP_MLNote_FrameType_INTEGER);
            mdlDim_setNoteFrameType (dimElement, (DimStyleProp_MLNote_FrameType) frameType);
            }

        if (mdlNote_isPropertyUpdateAllowed (DIMSTYLE_PROP_MLNote_Justification_INTEGER, option) &&
            ! IsPropertyShielded (DIMSTYLE_PROP_MLNote_Justification_INTEGER))
            {
            int multiJust;
            m_dimStyle.GetIntegerProp (multiJust, DIMSTYLE_PROP_MLNote_Justification_INTEGER);

            mdlDim_setNoteHorizontalJustification (dimElement, (DimStyleProp_MLNote_Justification) multiJust);
            }
        }

    if (dimEl.dimcmd != static_cast<byte>(DimensionType::Note)  &&      /* not a note           */
        dimEl.dimcmd != static_cast<byte>(DimensionType::LabelLine) )   /* not a label line     */
        {
        if (! IsPropertyShielded (DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT))
            {
            bool  dualValFromStyle;

            m_dimStyle.GetBooleanProp (dualValFromStyle, DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT);

            /*------------------------------------------------------------------
              1. If the user has gone to the trouble of overriding the secondary
                 text, we display it even if the style claims that its not dual.
              2. If measuring degree value, do not show regular secondary units.
            ------------------------------------------------------------------*/
            if (hasSecondaryUserText (dimElement))
                {
                if (!dimEl.flag.dual || dualValFromStyle)
                    dimEl.flag.dual = dualValFromStyle;
                }
            else
                {
                if (isDegreeValue)
                    dimEl.flag.dual = false;
                else
                    dimEl.flag.dual = dualValFromStyle;
                }
            }
        }

    /*--------------------------------------------------------------------
    Update altFormat blocks
    --------------------------------------------------------------------*/
    UpdateAltFormatBlock (dimElement, true);
    UpdateAltFormatBlock (dimElement, false);

    /*--------------------------------------------------------------------
    Update tolerance block
    --------------------------------------------------------------------*/
    UpdateToleranceInfo (dimElement);

    /*-------------------------------------------------------------------
    Update terminator block
    -------------------------------------------------------------------*/
    UpdateTerminatorInfo (dimElement);

    /*-------------------------------------------------------------------
    Update prefix/suffix block
    -------------------------------------------------------------------*/
    UpdatePrefixSuffixInfo (dimElement);

    /*-------------------------------------------------------------------
    Update replacement diameter symbol block
    -------------------------------------------------------------------*/
    UpdateCustomDiameterInfo (dimElement);

    /*-------------------------------------------------------------------
    Update custom prefix/suffix symbol/cell blocks
    -------------------------------------------------------------------*/
    UpdatePrefixSuffixSymbolInfo (dimElement, true);
    UpdatePrefixSuffixSymbolInfo (dimElement, false);

    /*-------------------------------------------------------------------
    Update primary/secondary unit blocks
    -------------------------------------------------------------------*/
    UpdateUnitInfo (dimElement, true);
    UpdateUnitInfo (dimElement, false);

    /*-------------------------------------------------------------------
    Update terminator symbology block
    -------------------------------------------------------------------*/
    UpdateTermSymbologyInfo (dimElement);

    /*--------------------------------------------------------------------
    Update ball and chain block
    --------------------------------------------------------------------*/
    UpdateBallAndChainInfo (dimElement);

    bool    showAnyWitness;
    m_dimStyle.GetBooleanProp (showAnyWitness, DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT);

    /*--------------------------------------------------------------------
    Update dimension template structure
    --------------------------------------------------------------------*/
    DimensionType   templateNumber;

    if (DimensionType::None != (templateNumber = adim_getTemplateNumber (dimElement)))
        UpdateTemplateInfo (dimElement, templateNumber);

    // Must be after the template has been updated
    if (! IsPropertyShielded (DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE))
        {
        double centerMarkSize;
        m_dimStyle.GetDoubleProp (centerMarkSize, DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE);

        if (!centerMarkSize)
            {
            if (!dimEl.tmpl.centerLeft && !dimEl.tmpl.centerRight &&
                !dimEl.tmpl.centerTop  && !dimEl.tmpl.centerBottom)
                dimEl.geom.centerSize = dimEl.geom.termHeight;
            }
        else
            {
            dimEl.geom.centerSize     = centerMarkSize;
            }

        if ( ! showAnyWitness)
            {
            dimEl.tmpl.left_witness  = dimEl.tmpl.right_witness =
            dimEl.tmpl.centerLeft    = dimEl.tmpl.centerRight   =
            dimEl.tmpl.centerTop     = dimEl.tmpl.centerBottom  = 0;
            }
        else if (dimEl.tmpl.centermark && centerMarkSize < 0)
            {
            dimEl.tmpl.centerLeft    = dimEl.tmpl.centerRight   =
            dimEl.tmpl.centerTop     = dimEl.tmpl.centerBottom  = 1;
            dimEl.geom.centerSize    = - dimEl.geom.centerSize;
            }
        }

    /*-------------------------------------------------------------------
    Turn on/off display of witness lines based on dimEl.template
    -------------------------------------------------------------------*/
    adim_setWitnessLinesFromTemplate (dimElement, !showAnyWitness);

    /*-------------------------------------------------------------------
    Dont allow automatic text adjustment (vert) for stacked dimensions
    -------------------------------------------------------------------*/
    if (dimEl.tmpl.stacked)
        dimEl.flag.noAutoTextLift =  1;

    /*-------------------------------------------------------------------
    TR-78246 : Apply the text justification
    -------------------------------------------------------------------*/
    int dimJust;
    m_dimStyle.GetIntegerProp (dimJust, DIMSTYLE_PROP_Text_Justification_INTEGER);

    if (! IsPropertyShielded (DIMSTYLE_PROP_Text_Justification_INTEGER) && DIMTEXT_OFFSET != dimJust)
        {
        /*-------------------------------------------------------------------------------
        shield pushTextRight for angular dimension for now - angular dimension currently
        always pushes text to one side by ignoring start vs end point.  So to support
        pushTextRight we will need one more flag to reserve this legacy behavior. Since
        current behavior happens to be the same as DWG, we can leave this unchanged until
        there will be a request for it, if any, in the future.
        -------------------------------------------------------------------------------*/
        if (DIM_ANGULAR(static_cast<DimensionType>(dimEl.dimcmd)) && DIMSTYLE_VALUE_Text_Justification_CenterRight == dimJust)
            {
            m_shieldFlags.SetPropertyBit (DIMSTYLE_PROP_Text_Justification_INTEGER, true);
            }
        else
            {
            int pointIndex;
            for (pointIndex = 1; pointIndex < dimEl.nPoints; pointIndex++)
                {
                if (DIMTEXT_OFFSET == dimEl.GetDimTextCP(pointIndex)->flags.b.just)
                    continue;

                if (DIMSTYLE_VALUE_Text_Justification_CenterRight != dimJust)
                    {
                    dimEl.GetDimTextP(pointIndex)->flags.b.just = dimJust;
                    dimEl.GetDimTextP(pointIndex)->flags.b.pushTextRight = false;
                    }
                else
                    {
                    dimEl.GetDimTextP(pointIndex)->flags.b.just = DIMSTYLE_VALUE_Text_Justification_CenterLeft;
                    dimEl.GetDimTextP(pointIndex)->flags.b.pushTextRight = true;
                    }
                }
            }
        }

    /*-------------------------------------------------------------------
    Notes always have a right terminator
    -------------------------------------------------------------------*/
    if (DimensionType::Note == static_cast<DimensionType>(dimEl.dimcmd))
        dimEl.tmpl.right_term = 1;

    /*--------------------------------------------------------------------------
    Update contents of the style extension linkage
    --------------------------------------------------------------------------*/
    UpdateStyleExtensions (dimElement, createData, option, bObeyedStyleAnnotationScaleFlag);

    if (mdlDim_isNoteDimension (dimElement) && (ADIM_PARAMS_CREATE == option || ADIM_PARAMS_CHANGE == option))
        mdlDim_setNoteAllowAutoMode (dimElement, true);

   /*------------------------------------------------------------------
      Items past this point are only set at create time
    --------------------------------------------------------------------*/
    if (option != ADIM_PARAMS_CREATE)
        return;

    /*--------------------------------------------------------------------
      Initialize parameters not changed on dimension update
    --------------------------------------------------------------------*/
    dimEl.flag.alignment = GetResolvedInteger (dimEl.flag.alignment, DIMSTYLE_PROP_General_Alignment_INTEGER);

    /* Make sure arbitrary alignment is used only on linear dimension   */
    if (dimEl.flag.alignment == 3 && dimEl.dimcmd > 4)
        dimEl.flag.alignment = 2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::adim_updateExtensionsFromDirectionFormat
(
DimStyleExtensions&     elmExtensions,
DirectionFormatterCR    dirFormat
)
    {
    /*--------------------------------------------------------------------------
        This function is called when creating a new dimension, and also when
        upgrading a dimension V7->V8, or from pre-Athens->Athens.
    --------------------------------------------------------------------------*/
    bool        changed = false;

    if (elmExtensions.flags3.directionClockwise != dirFormat.GetClockwise())
        {
        // No effort is made to keep the flags modifier bit accurate, it is derived from the flags.
        elmExtensions.flags3.directionClockwise = dirFormat.GetClockwise();
        changed = true;
        }

    if (elmExtensions.directionMode != static_cast<UInt16>(dirFormat.GetDirectionMode()))
        {
        DirectionMode directionMode = dirFormat.GetDirectionMode();

        if (DirectionMode::Azimuth != directionMode)
            {
            elmExtensions.modifiers |= STYLE_Extension_DirectionMode;
            elmExtensions.directionMode = static_cast<UInt16>(directionMode);
            }
        else
            {
            elmExtensions.modifiers &= ~STYLE_Extension_DirectionMode;
            elmExtensions.directionMode = static_cast<UInt16>(DirectionMode::Azimuth);
            }

        changed = true;
        }

    if (elmExtensions.directionBaseDir != dirFormat.GetBaseDirection())
        {
        double  baseDir = dirFormat.GetBaseDirection();

        if (0.0 != baseDir)
            {
            elmExtensions.modifiers |= STYLE_Extension_DirectionBaseDir;
            elmExtensions.directionBaseDir = baseDir;
            }
        else
            {
            elmExtensions.modifiers &= ~STYLE_Extension_DirectionBaseDir;
            elmExtensions.directionBaseDir = 0.0;
            }

        changed = true;
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void* BentleyApi::mdlDim_getEditOptionBlock (EditElementHandleR dim, int reqType, UInt64* element)
    {
    return const_cast<void*> (mdlDim_getOptionBlock (dim, reqType, element));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    05/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public void    BentleyApi::shiftElementData
(
DimensionElm    *dim,
char            *pStart,
int             nMove
)
    {
    /*-----------------------------------------------------------------
    Shift the data between pStart and the end of the element up or down
    by nMove. Fix wtf, attindex and adjust attributes for word boundary
    -----------------------------------------------------------------*/
    char        *pDim, *pAttr, *pEnd;

    size_t dimSize = ((DgnElementP)dim)->Size ();

    pDim    = (char*)dim;
    pAttr   = (char*)((UShort*)dim + dim->GetAttributeOffset());
    pEnd    = pDim + dimSize;

    if (pStart > pEnd)
        {
        // I'm not sure why this happens, but I saw it crash once and it corrupts memory (RBB 9/00)
#if defined (DEBUG)
        printf ("Error ShiftElementData  - pStart > pEnd");
#endif
        BeAssert(false);
        return;
        }

    memmove(pStart+nMove, pStart, pEnd-pStart);

    /*-----------------------------------------------------------------
    Attribute data needs to start on a word boundary. If the shift was
    odd then move the attribute data. If the text heap already has a
    1 byte pad, remove it and shift down, otherrwise shift up.
    -----------------------------------------------------------------*/
    if ((nMove % 2) != 0)
        {
        pAttr   += nMove;
        pEnd    += nMove;

        if (dim->flag.textHeapPad)
            {
            if (pAttr != pEnd)
                memmove(pAttr-1, pAttr, pEnd-pAttr);

            dim->flag.textHeapPad = false;
            nMove -= 1;
            }
        else
            {
            if (pAttr != pEnd)
                memmove(pAttr+1, pAttr, pEnd-pAttr);

            dim->flag.textHeapPad = true;
            nMove += 1;
            }
        }

    dim->IncrementSizeWords(nMove/2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::ChangeFont
(
EditElementHandleR  dimElement,
UInt32              newFont,
UInt32              newBigFont
)
    {
    LegacyTextStyle   dimTextStyle;
    GetTextStyle (dimElement, &dimTextStyle);

    BentleyApi::adim_changeTextHeapEncoding (dimElement, newFont, newBigFont, dimTextStyle.fontNo, dimTextStyle.shxBigFont);

    dimTextStyle.fontNo     = newFont;
    dimTextStyle.shxBigFont = newBigFont;

    adim_setTextStyle (dimElement, &dimTextStyle, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     DimensionHandler::UpdateFromDimStyle
(
EditElementHandleR  eeh,
DimensionStyleCR    dimStyle,
IDimCreateDataCP    createData,
int                 option
)
    {
    if (ADIM_PARAMS_CREATE == option || ADIM_PARAMS_CHANGE == option)
        {
        if (NULL == createData)
            { BeAssert(false); return; }
        }

    /*--------------------------------------------------------------------------
      The Shield Flags are used to "protect" individual properties from being
      clobbered during a style update, in other words: "instance specific overrides".
    --------------------------------------------------------------------------*/
    IDimensionQuery*    dimQuery = dynamic_cast <IDimensionQuery*> (&eeh.GetHandler ());

    if (NULL == dimQuery)
        { BeAssert(false); return; }

    DimStylePropMaskPtr     shieldFlags = dimQuery->GetOverrideFlags(eeh);

    /*--------------------------------------------------------------------------
      Propagate all the properties from the DimStyle to the Dimension
    --------------------------------------------------------------------------*/
    DimPropUpdater updater (dimStyle, *shieldFlags, createData);
    updater.UpdateDimensionProperties (eeh, option, createData);

   /*---------------------------------------------------------------------------
      Rewrite the shield flags which may have changed.
    --------------------------------------------------------------------------*/
    SaveShieldsDirect (eeh, *shieldFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionUpdateData: IDimCreateData
    {
    private:
        DimensionStyleCR    m_dimStyle;
        DgnTextStylePtr     m_textStyle;
        RotMatrix           m_matrix;
        RotMatrix           m_viewMatrix;
        int                 m_viewNumber;
        LevelId             m_levelId;
        Symbology           m_symbology;
    public:
    DimensionUpdateData (DimensionStyleCR style, EditElementHandleR dimElement)
        :m_dimStyle (style)
        {
        m_dimStyle.GetTextStyleProp (m_textStyle, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE);

        DimensionHandler* dimHandler = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
        dimHandler->GetRotationMatrix (dimElement, m_matrix);

        dimHandler->GetViewRotation (dimElement, m_viewMatrix);
        m_viewNumber = dimElement.GetElementP()->ToDimensionElm().view;
        m_levelId = dimElement.GetElementP()->ToDimensionElm().GetLevel();

        m_symbology.style = dimElement.GetElementP()->ToDimensionElm().GetSymbology().style;
        m_symbology.color = dimElement.GetElementP()->ToDimensionElm().GetSymbology().color;
        m_symbology.weight = dimElement.GetElementP()->ToDimensionElm().GetSymbology().weight;
        }

    virtual DimensionStyleCR                _GetDimStyle()      const {return m_dimStyle;}
    virtual DgnTextStyleCR                  _GetTextStyle()     const {return *m_textStyle;}
    virtual Symbology                       _GetSymbology()     const {return m_symbology;}
    virtual LevelId                         _GetLevelID()       const {return m_levelId;}
    virtual int                             _GetViewNumber()    const {return m_viewNumber;}
    virtual RotMatrixCR                     _GetDimRMatrix()    const {return m_matrix;}
    virtual RotMatrixCR                     _GetViewRMatrix()   const {return m_viewMatrix;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionHandler::_ApplyDimensionStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle, bool retainOverrides)
    {
    int                 option = ADIM_PARAMS_CHANGE;
    DimStylePropMaskPtr oldShields = NULL;

    if (DIMENSION_ELM != eeh.GetElementCP()->GetLegacyType())
        return;

//TODO Test mdlDimStyle_isActive (dimStyle
//    if (mdlDimStyle_isActive (dimStyle))
//        option = ADIM_PARAMS_CHANGE;

    if (retainOverrides)
        {
        oldShields = DimStylePropMask::ExtractFromLinkages (eeh);
        }
    else
        {
        DimStylePropMask::DeleteLinkages (*eeh.GetElementP());
        adim_clearTextStyleOverrides (eeh);
        }

    DimensionUpdateData updateData (dimStyle, eeh);

    UpdateFromDimStyle (eeh, dimStyle, &updateData, option);
    SetShieldsFromStyle (eeh, dimStyle);

    if (retainOverrides && oldShields.IsValid())
        {
        DimStylePropMaskPtr newShields = DimStylePropMask::ExtractFromLinkages (eeh);

        DimStylePropMask::LogicalOperation (*newShields, *oldShields, BitMaskOperation::Or);
        SaveShieldsDirect (eeh, *newShields);
        }

    if (ADIM_PARAMS_CHANGE != option)
        return;

    bool useAltColor, useAltWeight, useAltStyle;

    dimStyle.GetBooleanProp (useAltColor,  DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT);
    dimStyle.GetBooleanProp (useAltWeight, DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT);
    dimStyle.GetBooleanProp (useAltStyle,  DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT);

    if (useAltColor || useAltWeight || useAltStyle)
        {
        int iPoint;

        for (iPoint = 0; iPoint < eeh.GetElementCP()->ToDimensionElm().nPoints; iPoint++)
            eeh.GetElementP()->ToDimensionElmR().GetDimTextP(iPoint)->flags.b.altSymb = true;
        }

    ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimStylePropMask_stripDisabledOverrides
(
DimStylePropMaskR               propMask,
DimensionStyleCR                dimStyle,
bool                            hasTextStyle
)
    {
    int                 numProps = 0;
    PropToOverrideMap  *propToOverridesMap = dgnDimStyle_getPropToOverridesMap (&numProps);

    /*--------------------------------------------------------------------------
       Some override props are not stored in the dimension elements, and are
       marked in the PropToOverrideMap as "notInDimElm".  When we do a match,
       those props will always be turned ON in the style.

       Because they are always ON when matched, it is wrong to determine
       shields for these props by comparing the matched style with the saved
       one.  We would end up with them generally shielded for no good reason.

       As a compromise, we will NOT shield these props when they are OFF in
       the saved style.  When they are off, this means that the style isn't
       going to push a new value to them anyway.

       There is a special case here for certain Explicit LegacyTextStyle Props.
       These props are effectively stored in the dimension as TSOs, but only
       if there is an embedded LegacyTextStyle in the dimension.
    --------------------------------------------------------------------------*/
    for (int iProp = 0; iProp < numProps; iProp++)
        {
        if ( ! propToOverridesMap[iProp].notInDimElm)
            continue;

        if (hasTextStyle)
            {
            bool    dontStrip = false;

            switch (propToOverridesMap[iProp].m_override)
                {
                case DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT:
                case DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT:
                case DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT:
                case DIMSTYLE_PROP_Text_Font_BOOLINT:
                    dontStrip = true;
                }

            if (dontStrip)
                continue;
            }

        bool isOverridden = false;

        dimStyle.GetBooleanProp (isOverridden, propToOverridesMap[iProp].m_override);

        if ( ! isOverridden)
            {
            propMask.SetPropertyBit (propToOverridesMap[iProp].m_override, false);
            propMask.SetPropertyBit (propToOverridesMap[iProp].m_property, false);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    adim_updateShieldsByCompareWithStyle (EditElementHandleR eeh, DgnProjectR destinationFile)
    {
    // elmP *MUST* be a full size element buffer
    UInt64       dimStyleID = mdlDim_getDimStyleID (eeh);

    if (0 == dimStyleID)
        return false;

    DimensionHandler&   hdlr = DimensionHandler::GetInstance();

    // Get the saved style
    // NOTE_KN: eeh, has DgnProjectP() of the source file that it is being copied from, must use the destination file.
    DimensionStylePtr   savedDimStyle = DimensionStyle::GetByID (dimStyleID, destinationFile);

    if ( ! savedDimStyle.IsValid())
        { /*BeAssert (0);*/ return false; } // graphite removed support for dimension style lookup

    // Get the style representing the element
    DimensionStylePtr   elementDimStyle = savedDimStyle->Copy();
    hdlr.ExtractDimStyle (*elementDimStyle, eeh);

    // Find the differences between the styles
    DimStylePropMaskPtr newShields = elementDimStyle->Compare (*savedDimStyle, DIMSTYLE_COMPAREOPTS_IgnoreUnusedDiffs);
    dimStylePropMask_stripDisabledOverrides (*newShields, *savedDimStyle, 0 != eeh.GetElementCP()->ToDimensionElm().textStyleId);

    // Get the existing shields
    DimStylePropMaskPtr shields = hdlr.GetOverrideFlags (eeh);

    // OR the existing shields with the new differences and set back on the element
    DimStylePropMask::LogicalOperation (*newShields, *shields, BitMaskOperation::Or);
    hdlr.SaveShieldsDirect (eeh, *newShields);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct MLTextProcessPropertiesArg
    {
    bool                m_canChange;
    bool                m_changed;
    PropertyContextR    m_proc;

    MLTextProcessPropertiesArg (bool canChange, PropertyContextR proc)
        :
        m_proc (proc)
        {
        m_canChange = canChange;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       mlTextProcessPropertiesCallback
(
DimFormattedText   **ppFmt,
void                *pData,
DgnModelP        modelRef,
DimMLText           *pText,
int                 currentRow
)
    {
    MLTextProcessPropertiesArg*   pArg = (MLTextProcessPropertiesArg *) pData;

    DPoint2d        size = (*ppFmt)->GetTileSize();
    TextParamWide   sWide= (*ppFmt)->GetTextParamWide();

    // Let the TextHandler do the job
    if (TextHandlerBase::ProcessNonLayoutPropertiesDirect (sWide, 0, size, pArg->m_proc, pArg->m_canChange)
            || TextHandlerBase::ProcessLayoutPropertiesDirect (sWide, size, false, 0, pArg->m_proc, pArg->m_canChange))
        {
        TextHandlerBase::SetOverridesFromStyle (pArg->m_proc, sWide, size, 0, (*ppFmt)->IsNodeComponent());
        (*ppFmt)->SetTextParamWide(sWide);
        pArg->m_changed = true;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_ProcessPropertiesHelper
(
EditElementHandleP     elmP,
ElementHandleCR        element,
PropertyContextR       proc
)
    {
    EditElementHandle duplicateElm;

    // This method expects that inElement remains unchanged.
    if (elmP && elmP == &element)
        duplicateElm.Duplicate (element);

    ElementHandleCR inElement = duplicateElm.IsValid() ? duplicateElm : element;

    IEditProperties*    editor = proc.GetIEditPropertiesP();
    bool                sizeChangeAllowed = true;
    bool                wantUpdateOverrides = NULL != editor && EditPropertyPurpose::Change == editor->_GetEditPropertiesPurpose ();
    bool                changed = false;

    /*--------------------------------------------------------------------------
        Extension Lines - Color, Weight, LineStyle
    --------------------------------------------------------------------------*/
    DimensionElm const* dim = &inElement.GetElementCP()->ToDimensionElm();
    bool anyAltSymb = adim_usesAnyAltSymb (*dim);

    changed |= proc.DoColorCallback     (elmP ? &elmP->GetElementP()->ToDimensionElmR().altSymb.color  : NULL, EachColorArg (dim->altSymb.color,  SETPROPCBEIFLAG(anyAltSymb), proc));
    changed |= proc.DoWeightCallback    (elmP ? &elmP->GetElementP()->ToDimensionElmR().altSymb.weight : NULL, EachWeightArg (dim->altSymb.weight, SETPROPCBEIFLAG(anyAltSymb), proc));
    changed |= proc.DoLineStyleCallback (elmP ? &elmP->GetElementP()->ToDimensionElmR().altSymb.style  : NULL, EachLineStyleArg (dim->altSymb.style,  NULL, SETPROPCBEIFLAG(anyAltSymb), proc));

    /*--------------------------------------------------------------------------
        Tolerance Block - Font
    --------------------------------------------------------------------------*/
    DimTolrBlock const* tolBlockCP = static_cast <DimTolrBlock const*> (mdlDim_getOptionBlock (inElement, ADBLK_TOLERANCE, NULL));

    if (NULL != tolBlockCP)
        {
        DimTolrBlock *pTolBlock = elmP ? static_cast <DimTolrBlock*> (mdlDim_getEditOptionBlock (*elmP, ADBLK_TOLERANCE, NULL)) : NULL;

        changed |= proc.DoFontCallback (pTolBlock ? &pTolBlock->pmFont : NULL, EachFontArg (tolBlockCP->pmFont, PROPSCALLBACK_FLAGS_NoFlagsSet, proc));
        }

    /*--------------------------------------------------------------------------
        Symbol Blocks - Font
    --------------------------------------------------------------------------*/
    DimSymBlock const*symbolBlockCP;

    if (NULL != (symbolBlockCP = static_cast <DimSymBlock const*> (mdlDim_getOptionBlock (inElement, ADBLK_DIAMSYM, NULL))))
        {
        DimSymBlock* symbolBlockP = elmP ? static_cast <DimSymBlock*> (mdlDim_getEditOptionBlock (*elmP, ADBLK_DIAMSYM, NULL)) : NULL;

        changed |= proc.DoFontCallback (symbolBlockP ? &symbolBlockP->symFont : NULL, EachFontArg (symbolBlockCP->symFont, PROPSCALLBACK_FLAGS_NoFlagsSet, proc));
        }

    if (NULL != (symbolBlockCP = static_cast <DimSymBlock const *> (mdlDim_getOptionBlock (inElement, ADBLK_PRESYMBOL, NULL))))
        {
        DimSymBlock* symbolBlockP = elmP ? static_cast <DimSymBlock*> (mdlDim_getEditOptionBlock (*elmP, ADBLK_PRESYMBOL, NULL)) : NULL;

        changed |= proc.DoFontCallback (symbolBlockP ? &symbolBlockP->symFont : NULL, EachFontArg (symbolBlockCP->symFont, PROPSCALLBACK_FLAGS_NoFlagsSet, proc));
        }

    if (NULL != (symbolBlockCP = static_cast <DimSymBlock const *> (mdlDim_getOptionBlock (inElement, ADBLK_SUFSYMBOL, NULL))))
        {
        DimSymBlock*symbolBlockP = elmP ? static_cast <DimSymBlock*> (mdlDim_getEditOptionBlock (*elmP, ADBLK_SUFSYMBOL, NULL)) : NULL;

        changed |= proc.DoFontCallback (symbolBlockP ? &symbolBlockP->symFont : NULL, EachFontArg (symbolBlockCP->symFont, PROPSCALLBACK_FLAGS_NoFlagsSet, proc));
        }

    /*--------------------------------------------------------------------------
        Terminator Block - Font
    --------------------------------------------------------------------------*/
    DimTermBlock const   *termBlockCP = static_cast <DimTermBlock const *> (mdlDim_getOptionBlock (inElement, ADBLK_TERMINATOR, NULL));

    if (NULL != termBlockCP)
        {
        DimTermBlock* termBlockP = elmP ? static_cast <DimTermBlock*> (mdlDim_getEditOptionBlock (*elmP, ADBLK_TERMINATOR, NULL)) : NULL;

        changed |= proc.DoFontCallback (termBlockP ? &termBlockP->arrow.symbol.font :  NULL, EachFontArg (termBlockCP->arrow.symbol.font,  SETPROPCBEIFLAG(1 == termBlockCP->flags.arrow), proc));
        changed |= proc.DoFontCallback (termBlockP ? &termBlockP->stroke.symbol.font : NULL, EachFontArg (termBlockCP->stroke.symbol.font, SETPROPCBEIFLAG(1 == termBlockCP->flags.stroke), proc));
        changed |= proc.DoFontCallback (termBlockP ? &termBlockP->origin.symbol.font : NULL, EachFontArg (termBlockCP->origin.symbol.font, SETPROPCBEIFLAG(1 == termBlockCP->flags.origin), proc));
        changed |= proc.DoFontCallback (termBlockP ? &termBlockP->dot.symbol.font :    NULL, EachFontArg (termBlockCP->dot.symbol.font,    SETPROPCBEIFLAG(1 == termBlockCP->flags.origin), proc));
        }

    /*--------------------------------------------------------------------------
        Terminator Symbology Block - Color, Weight, LineStyle
    --------------------------------------------------------------------------*/
    DimTermSymbBlock const    *termSymbBlockCP = static_cast <DimTermSymbBlock const *> (mdlDim_getOptionBlock (inElement, ADBLK_TERMSYMB, NULL));

    if (NULL != termSymbBlockCP)
        {
        DimTermSymbBlock * termSymbBlockP = elmP ? static_cast <DimTermSymbBlock *> (mdlDim_getEditOptionBlock (*elmP, ADBLK_TERMSYMB, NULL)) : NULL;

        changed |= proc.DoColorCallback     (termSymbBlockP ? &termSymbBlockP->termSymb.color  : NULL, EachColorArg (termSymbBlockCP->termSymb.color,           SETPROPCBEIFLAG(1 == termSymbBlockCP->termSymb.useColor),  proc));
        changed |= proc.DoWeightCallback    (termSymbBlockP ? &termSymbBlockP->termSymb.weight : NULL, EachWeightArg (termSymbBlockCP->termSymb.weight,         SETPROPCBEIFLAG(1 == termSymbBlockCP->termSymb.useWeight), proc));
        changed |= proc.DoLineStyleCallback (termSymbBlockP ? &termSymbBlockP->termSymb.style  : NULL, EachLineStyleArg (termSymbBlockCP->termSymb.style, NULL, SETPROPCBEIFLAG(1 == termSymbBlockCP->termSymb.useStyle),  proc));
        }

    /*--------------------------------------------------------------------------
        Point Overrides - Color, Weight, LineStyle
    --------------------------------------------------------------------------*/
    for (int iPoint = 0; iPoint < dim->nPoints; iPoint++)
        {
        UInt32  color;

        if (mdlDim_pointGetWitnessColor (&color, inElement, iPoint))
            {
            if (proc.DoColorCallback (elmP ? &color : NULL, EachColorArg (color, PROPSCALLBACK_FLAGS_NoFlagsSet, proc)))
                {
                mdlDim_pointSetWitnessColor (*elmP, iPoint, &color);
                changed = true;
                }
            }

        UInt32  weight;

        if (mdlDim_pointGetWitnessWeight (&weight, inElement, iPoint))
            {
            if (proc.DoWeightCallback (elmP ? &weight : NULL, EachWeightArg (weight, PROPSCALLBACK_FLAGS_NoFlagsSet, proc)))
                {
                mdlDim_pointSetWitnessWeight (*elmP, iPoint, &weight);
                changed = true;
                }
            }

        Int32   lineStyle;

        if (mdlDim_pointGetWitnessStyle (&lineStyle, inElement, iPoint))
            {
            if (proc.DoLineStyleCallback (elmP ? &lineStyle : NULL, EachLineStyleArg (lineStyle, NULL, PROPSCALLBACK_FLAGS_NoFlagsSet, proc)))
                {
                mdlDim_pointSetWitnessStyle (*elmP, iPoint, &lineStyle);
                changed = true;
                }
            }
        }

    /*--------------------------------------------------------------------------
        LegacyTextStyle ID
    --------------------------------------------------------------------------*/
    EachTextStyleArg    textStyleArg (dim->textStyleId, PROPSCALLBACK_FLAGS_NoFlagsSet, proc);

    changed |= proc.DoTextStyleCallback (elmP ? &elmP->GetElementP()->ToDimensionElmR().textStyleId : NULL, textStyleArg);

    StyleParamsRemapping    textStyleParamsRemapping = textStyleArg.GetRemappingAction ();

    /*--------------------------------------------------------------------------
        Embedded LegacyTextStyle - Color, Font, LineStyle
    --------------------------------------------------------------------------*/
    LegacyTextStyle   textStyle;

    if (SUCCESS == GetTextStyle (inElement, &textStyle))
        {
        DgnModelP    destDgnModel = proc.GetDestinationDgnModel();
        
        DgnTextStylePtr dimElementStyle;
        bool textStyleChanged;
        if (&proc.GetSourceDgnModel()->GetDgnProject() != &proc.GetDestinationDgnModel()->GetDgnProject())
            {
            // We cannot use a normal DgnTextStyle because fonts in the source project may be missing and we cannot go to/from ID to font object.
            // You may think we could just make a normal DgnTextStyle from the source project, but process properties must use IDs. Even if we were to remap to a last resort font, this font will not have an entry in the font map, and we'll fail to grant it an ID because the source project is read-only.
            // Thus, we can use the specialized NoFontObjectDgnTextStyle to do remapping, and then reconstruct a normal DgnTextStyle.
            NoFontObjectDgnTextStyle textStyleForClone(proc.GetSourceDgnModel()->GetDgnProject());
            DgnTextStylePersistence::Legacy::FromLegacyStyle(textStyleForClone, textStyle);
            textStyleChanged = textStyleForClone.ProcessPropertiesForClone(proc, false);
            
            dimElementStyle = textStyleForClone.Clone();
            }
        else
            {
            dimElementStyle = DgnTextStyle::Create(destDgnModel->GetDgnProject());
            DgnTextStylePersistence::Legacy::FromLegacyStyle(*dimElementStyle, textStyle);
            textStyleChanged = dimElementStyle->ProcessProperties(proc, elmP ? true : false);
            }
         
        if (NULL != elmP && 0 != elmP->GetElementCP()->ToDimensionElm().textStyleId)
            {
            switch (textStyleParamsRemapping)
                {
                case StyleParamsRemapping::Override:
                    {
                    DgnTextStylePtr oldStyle = destDgnModel->GetDgnProject().Styles().TextStyles().QueryById(DgnStyleId(elmP->GetElementCP()->ToDimensionElm().textStyleId));
                    if (oldStyle.IsValid())
                        {
                        DgnTextStylePropertyMaskPtr diff = dimElementStyle->Compare(*oldStyle);
                        diff->LogicalOr(*DgnTextStylePersistence::Legacy::FromLegacyMask(textStyle.overrideFlags));
                        textStyle.overrideFlags = DgnTextStylePersistence::Legacy::ToLegacyMask(*diff);
                        textStyleChanged = true;
                        }
                    break;
                    }
                case StyleParamsRemapping::ApplyStyle:
                    {
                    //  Apply the text style's parameters to the dimension's textstyle.
                    if (adim_updateFromTextStyle (textStyle, *elmP))
                        textStyleChanged = true;

                    break;
                    }
                }
            }

        if (textStyleChanged)
            {
            LegacyTextStyle v8style = DgnTextStylePersistence::Legacy::ToLegacyStyle(*dimElementStyle);
            SetTextStyle (*elmP, &v8style, false, sizeChangeAllowed);
            changed = true;
            }
        }

    /*--------------------------------------------------------------------------
        The embedded LegacyTextStyle does not include Weight!
    --------------------------------------------------------------------------*/
    DgnFontCR   fontObj = *DgnFontManager::ResolveFont (dim->text.font, proc.GetDestinationDgnModel ()->GetDgnProject (), DGNFONTVARIANT_DontCare);
    bool    useTextWeight = (DgnFontType::TrueType != fontObj.GetType());

    changed |= proc.DoWeightCallback (elmP ? &elmP->GetElementP()->ToDimensionElmR().text.weight : NULL, EachWeightArg (dim->text.weight, SETPROPCBEIFLAG(useTextWeight), proc));

    /*--------------------------------------------------------------------------
        Multiline Text Linkages - Color, Weight, LineStyle, Font
    --------------------------------------------------------------------------*/
    DimMLText*                  pText = NULL;
    int                         nSegments = dim->nPoints - 1;
    MLTextProcessPropertiesArg  arg (NULL != elmP ? true : false, proc);

    for (int iSeg = 0; iSeg < nSegments; iSeg++)
        {
        if (SUCCESS == mdlDimText_create (&pText) &&
            SUCCESS == mdlDim_getText (pText, inElement, iSeg))
            {
            arg.m_changed = false;

            mdlDimText_traverseFormatters (pText, mlTextProcessPropertiesCallback, &arg, NULL);

            if (arg.m_changed)
                {
                mdlDim_setText (*elmP, pText, iSeg);
                changed = true;
                }
            }

        if (NULL != pText)
            mdlDimText_free (&pText);
        }

    /*--------------------------------------------------------------------------
        DimStyle
    --------------------------------------------------------------------------*/
    EachDimStyleArg dimStyleArg (dim->dimStyleId, PROPSCALLBACK_FLAGS_NoFlagsSet, proc);

    changed |= proc.DoDimStyleCallback (elmP ? &elmP->GetElementP()->ToDimensionElmR().dimStyleId : NULL, dimStyleArg);

    if (NULL == elmP || 0 == elmP->GetElementCP()->ToDimensionElm().dimStyleId)
        return changed;

    StyleParamsRemapping    paramsRemapping = dimStyleArg.GetRemappingAction ();

    /*--------------------------------------------------------------------------
      The element potentially been assigned to a new style.  This change may
      have an impact on the parameters of elmP that are derived from the style.

         *** This step should be done after all ids are remapped ***
    --------------------------------------------------------------------------*/
    DgnModelP    modelRef = proc.GetDestinationDgnModel();

    // Save off a copy so we can memcmp to test if there was any change
    DgnElement* saveDimElm = NULL;
    DUP_ELEMENT_ON_STACK (saveDimElm, elmP->GetElementCP())

    switch (paramsRemapping)
        {
        case StyleParamsRemapping::Override:
            {
            wantUpdateOverrides = true;
            break;
            }

        case StyleParamsRemapping::ApplyStyle:
            {
            DimensionStylePtr  dimStyle = DimensionStyle::GetByID (elmP->GetElementCP()->ToDimensionElm().dimStyleId, modelRef->GetDgnProject());

            //  Apply the style's parameters to the element.
            DimensionHandler::UpdateFromDimStyle (*elmP, *dimStyle, NULL, ADIM_PARAMS_UPDATE_FROMDIMSTYLE);
            break;
            }
        }

    /*--------------------------------------------------------------------------
      We want to update the overrides if either:
        a) Requested by the DimStyleCallback
        b) The EditPurpose is Change
      But, we can't do it for the legacy case where we are not allowed to
      change the element's size.
    --------------------------------------------------------------------------*/
    if (wantUpdateOverrides && sizeChangeAllowed)
        {
        // If any of the element's properties don't match the style, set up new
        // overrides for them, while preserving all existing overrides
        adim_updateShieldsByCompareWithStyle (*elmP, modelRef->GetDgnProject());
        }

    if (saveDimElm->GetSizeWords() != elmP->GetElementCP()->GetSizeWords() ||
        0 != memcmp (saveDimElm, elmP->GetElementCP(), elmP->GetElementCP()->Size ()))
        {
        changed = true;
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_ProcessProperties (PropertyContextR context, ElementHandleCR eh, EditElementHandleP eehP)
    {
    // Optimization when only processing level during add...
    if (ELEMENT_PROPERTY_Level == context.GetElementPropertiesMask ())
        return;

    if (NULL == eehP)
        {
        _ProcessPropertiesHelper (NULL, eh, context);

        return;
        }

    if (!_ProcessPropertiesHelper (eehP, eh, context))
        return;

    context.SetElementChanged ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_QueryProperties (ElementHandleCR eh, PropertyContextR context)
    {
    HitPathCP   path = context.GetQueryPath ();

    if (path && QueryPropertyPurpose::Match == context.GetIQueryPropertiesP ()->_GetQueryPropertiesPurpose ()) // Report properties for hit detail...
        {
        UInt32  dimPartName = 0;

        path->GetDimensionParameters (&dimPartName, NULL, NULL, NULL, NULL);
        if (dimPartName)
            {
            Symbology symb;
            UInt32 dimPartName=0;
            if (SUCCESS == mdlDim_getPartSymbology (&symb, eh, dimPartName))
                {
                //TODO test transparency level and font
                if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                    context.DoColorCallback (NULL, EachColorArg (symb.color, PROPSCALLBACK_FLAGS_IsBaseID, context));

                if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                    context.DoLineStyleCallback (NULL, EachLineStyleArg (symb.style, NULL, PROPSCALLBACK_FLAGS_IsBaseID, context));

                if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                    context.DoWeightCallback (NULL, EachWeightArg (symb.weight, PROPSCALLBACK_FLAGS_IsBaseID, context));

#ifdef WIP_VANCOUVER_MERGE // material
                // Report properties common to all components (that aren't propagated to component edP)...
                if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
                    {
                    IMaterialPropertiesExtension    *mExtension = IMaterialPropertiesExtension::Cast (eh.GetHandler ());
                    DgnMaterialId                    materialId;
                    
                    if (mExtension && SUCCESS ==  mExtension->StoresAttachmentInfo (eh, materialId))
                        context.DoMaterialCallback (NULL, EachMaterialArg (materialId, PROPSCALLBACK_FLAGS_IsBaseID, context));
                    }
#endif

#ifdef WIP_VANCOUVER_MERGE // template
                if (0 != (ELEMENT_PROPERTY_ElementTemplate & context.GetElementPropertiesMask ()))
                    context.DoElementTemplateCallback (NULL, EachElementTemplateArg (TemplateRefAttributes::GetReferencedTemplateIDFromHandle (eh), PROPSCALLBACK_FLAGS_IsBaseID, context));
#endif
                return;
                }
            }
        }

    T_Super::_QueryProperties (eh, context);

    _ProcessProperties (context, eh, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    _ProcessProperties (context, eeh, &eeh);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
int  BentleyApi::mdlDim_deleteTextCluster
(
EditElementHandleR    dimElement,
int           pointNo
)
    {
    DgnV8ElementBlank tmpElement;
    dimElement.GetElementCP()->CopyTo (tmpElement);
    DimensionElm  *dim = &tmpElement.ToDimensionElmR();

    if (pointNo == LAST_POINT)
        pointNo = dim->nPoints - 1;
    else if (pointNo >= (int)dim->nPoints || pointNo < 0)
        return (ERROR);
    //Since it is a delete this function is okay
    char     *textHeap = (char*)mdlDim_getEditOptionBlockFromElement (tmpElement, dimElement, ADBLK_TEXT, NULL);
    DimText  *dt = dim->GetDimTextP(pointNo);

    if (dt->address < 0)
        return (ERROR);
    
    char     *start, *end;
                                       /* Find start of text cluster        */
    start = end = textHeap + dt->address;

    for (int i=0; i<6; i++)                /* Find end of text cluster          */
        {
        if (dt->flags.s & (1 << (i + 8)))
            end += VariCharConverter::ComputeNumBytes (end);
        }
                                       /* Shift remainder of text heap down */
    ptrdiff_t size = end - start;
    BentleyApi::shiftElementData(dim, end, (int)-size);
    
    for (int i=0; i < (int)dimElement.GetElementP()->ToDimensionElm().nPoints; i++)
        if (dimElement.GetElementP()->ToDimensionElm().GetDimTextCP(static_cast<UInt32>(i))->address > dt->address)
            dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(static_cast<UInt32>(i))->address -= static_cast<Int16>(size);

    dt->address  = -1;                 /* Remove text info from dim point   */
    dt->flags.b.uprTxt     = dt->flags.b.uprTxtUTol =
    dt->flags.b.uprTxtLTol = dt->flags.b.lowTxt     =
    dt->flags.b.lowTxtUTol = dt->flags.b.lowTxtLTol = 0;

    dimElement.ReplaceElement (&tmpElement);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus  addSharedCellDefDependency
(
DgnElement               *dimElm,          /* => Dimension element to work on   */
DimOptionBlockHeader    *optionBlock,     /* => Option block to be updated     */
UInt64*              cellId
)
    {
    /* When inserting a terminator block...need to add cell ids as dependency linkage */
    if (optionBlock->type != ADBLK_TERMINATOR)
        return SUCCESS;
    
    if (NULL == cellId)
        return ERROR;

    DimTermBlock    *termBlockP = (DimTermBlock *) optionBlock;

    DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_ArrowHeadTerminator);
    if (termBlockP->flags.arrow > 1)
        DependencyManagerLinkage::AppendSimpleLinkageToMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_ArrowHeadTerminator, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, cellId[DEPENDENCYAPPVALUE_ArrowHeadTerminator -1]);

    DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_StrokeTerminator);
    if (termBlockP->flags.stroke > 1)
        DependencyManagerLinkage::AppendSimpleLinkageToMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_StrokeTerminator, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, cellId[DEPENDENCYAPPVALUE_StrokeTerminator-1]);

    DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_OriginTerminator);
    if (termBlockP->flags.origin > 1)
        DependencyManagerLinkage::AppendSimpleLinkageToMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_OriginTerminator, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, cellId[DEPENDENCYAPPVALUE_OriginTerminator-1]);

    DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_DotTerminator);
    if (termBlockP->flags.dot > 1)
        DependencyManagerLinkage::AppendSimpleLinkageToMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_DotTerminator, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, cellId[DEPENDENCYAPPVALUE_DotTerminator-1]);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/08
+---------------+---------------+---------------+---------------+---------------+------*/
static void    removeSharedCellDefDependency
(
DgnElement       *dimElm,          /* => Dimension element to work on   */
int             blockType         /* => */
)
    {
    /* When removing a terminator block...need to remove cell ids dependency linkages */
    if (ADBLK_TERMINATOR == blockType)
        {
        DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_ArrowHeadTerminator);
        DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_StrokeTerminator);
        DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_OriginTerminator);
        DependencyManagerLinkage::DeleteLinkageFromMSElement (dimElm, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_DotTerminator);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::mdlDim_insertOptionBlock
(
EditElementHandleR   dimElement,             /* => Dimension element to work on   */
DimOptionBlockHeader *newBlock,            /* => Option block to insert         */
UInt64*           cellId
)
    {
    char        *src, *dst, *end;
    DimOptionBlockHeader *tmpBlock;
    DgnV8ElementBlank tmpelement;
    dimElement.GetElementCP()->CopyTo(tmpelement);
    if (tmpBlock = (DimOptionBlockHeader*) mdlDim_getEditOptionBlockFromElement (tmpelement,dimElement, newBlock->type, NULL))
        {
        BeAssert (tmpBlock->nWords == newBlock->nWords);
        memcpy (tmpBlock, newBlock, newBlock->nWords * 2);

        if (SUCCESS != addSharedCellDefDependency (&tmpelement, tmpBlock, cellId))
            return ERROR;

        dimElement.ReplaceElement (&tmpelement);
        return (SUCCESS);
        }
    
    size_t dimSize = tmpelement.Size ();

    if (dimSize + (int) newBlock->nWords * 2 > MAX_V8_ELEMENT_SIZE)
        return (ERROR);
    
    DimensionElm* dim = &tmpelement.ToDimensionElmR();
    tmpBlock = reinterpret_cast<DimOptionBlockHeader *>(dim->GetDimViewBlockP());

    src = (char *) tmpBlock;
    dst = src + (newBlock->nWords * 2);
    end = (char *) dim + static_cast<ptrdiff_t>(dimSize);
    memmove (dst, src, end - src);

    memcpy (tmpBlock, newBlock, newBlock->nWords * 2);

    dim->nOptions++;
    dim->IncrementSizeWords(newBlock->nWords);

    if (SUCCESS != addSharedCellDefDependency ((DgnElement *)dim, tmpBlock, cellId))
        return ERROR;

    dimElement.ReplaceElement (&tmpelement);

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             02/91
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::adim_deleteOptionBlock
(
EditElementHandleR dimElement,               /* => Dimension element block is in  */
int           reqType             /* => Requested block type           */
)
    {
    DimOptionBlockHeader *tmpBlock;
    char        *src, *dst, *end;
    
    DimensionElm* dim = &dimElement.GetElementP()->ToDimensionElmR();
    if (!(tmpBlock = (DimOptionBlockHeader*) mdlDim_getEditOptionBlock (dimElement, reqType, NULL)))
        {
        removeSharedCellDefDependency ((DgnElement *)dim, reqType);
        return (ERROR);
        }

    int nWords = tmpBlock->nWords;
    dst    = (char*)tmpBlock;
    src    = dst + static_cast<ptrdiff_t>(nWords * 2);
    end    = (char*)dim + static_cast<ptrdiff_t>(((DgnElementP)dim)->Size ());

    memmove (dst, src, end - src);

    dim->nOptions--;
    dim->IncrementSizeWords( - nWords);

    // this actually remove the dependency linkages
    removeSharedCellDefDependency ((DgnElement *)dim, reqType);

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
int  BentleyApi::adim_getCellId
(
UInt64*  uniqueIdP,         /* <= uniqueId */
ElementHandleCR element,              /*  => element to test */
UShort      type                /*  => app value to look for **/
)
    {
    if (uniqueIdP)
        *uniqueIdP = 0L;

    DependencyLinkageAccessor depLinkage;

    if (SUCCESS != DependencyManagerLinkage::GetLinkageFromMSElement (&depLinkage, element.GetElementCP(), DEPENDENCYAPPID_DimensionCell, type))
        return ERROR;

    // The linkage is invalid during the remap process. Return SUCCESS so we do not alter the termBlock.
    if (depLinkage->u.f.invalid)
        return SUCCESS;

    if (uniqueIdP)
        *uniqueIdP = depLinkage->root.elemid[0];

    return (0 != depLinkage->root.elemid[0] ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/01
+---------------+---------------+---------------+---------------+---------------+------*/
UShort  DimensionElm::ComputeCheckSum () const
    {
    UShort          checkSum = 0;
    DimOptionBlockHeader const *blockPtr = (DimOptionBlockHeader const*) (point + (nPoints ? nPoints : 1));

    // First part of dimension
    UShort const* pShort = NULL;
    UShort const* pEnd = NULL;
    for (pShort = (UShort const*) &dimStyleId, pEnd =  (UShort const*) &point[0]; pShort < pEnd;)
        checkSum ^= *pShort++;

    // Points.
    for (pShort = (UShort const*) &point[0], pEnd =  (UShort const*) &point[nPoints]; pShort < pEnd;)
        checkSum ^= *pShort++;


    // Options Blocks
    for (int i=0; i< (int) nOptions; i++)
        {
        if (blockPtr->type != ADBLK_PROXYCELL)
            {
            for (pShort = (UShort const *) blockPtr, pEnd = pShort + blockPtr->nWords; pShort < pEnd;)
                checkSum ^= *pShort++;
            }

        blockPtr += blockPtr->nWords;
        }

    // Linkages.
    for (pShort = (UShort const*) this + GetAttributeOffset(),  pEnd = (UShort const*) this + GetSizeWords(); pShort < pEnd;)
        checkSum ^= *pShort++;

    return checkSum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/01
+---------------+---------------+---------------+---------------+---------------+------*/
UShort  DimensionElm::ComputeAdditiveCheckSum (int checkSumType) const
    {
    UShort          checkSum = 0;
    DimOptionBlockHeader const *blockPtr = (DimOptionBlockHeader const *) (point + (nPoints ? nPoints : 1));
    UShort const *pLinkageStart = (UShort const*) this + GetAttributeOffset();

#if defined (DEBUG_CHECKSUM)
    int iStep = 0;
    printf ("Dumping checksum for elem: %lld\n", ehdr.uniqueId);
#endif
    UShort const* pShort = NULL;
    UShort const* pEnd = NULL;
    // First part of dimension
    for (pShort = (UShort const*) &dimStyleId, pEnd =  (UShort const*) &point[0]; pShort < pEnd; pShort++)
        {
        // changed this from "checkSum += *pShort++" to avoid overflow runtime check in VC2008.
        checkSum = ((checkSum + *pShort) & 0xffff);
#if defined (DEBUG_CHECKSUM)
        printf ("  [1][%d] checksum = %d\n", iStep++, checkSum);
#endif
        }

    // Points.
    for (pShort = (UShort const*) &point[0], pEnd =  (UShort const*) &point[nPoints]; pShort < pEnd; pShort++)
        {
        checkSum = ((checkSum + *pShort) & 0xffff);
#if defined (DEBUG_CHECKSUM)
        printf ("  [2][%d] checksum = %d\n", iStep++, checkSum);
#endif
        }


    // Options Blocks
    for (int i=0; i< (int) nOptions; i++)
        {
        if (blockPtr->type != ADBLK_PROXYCELL)
            {
            for (pShort = (UShort const*) blockPtr, pEnd = pShort + blockPtr->nWords; pShort < pEnd; pShort++)
                {
                checkSum = ((checkSum + *pShort) & 0xffff);
#if defined (DEBUG_CHECKSUM)
                printf ("  [3][%d] checksum = %d\n", iStep++, checkSum);
#endif
                }
            }

        blockPtr += blockPtr->nWords;
        }

    // Text Heap
    if (checkSumType > 1)
        {
        for (pShort = (UShort const *) blockPtr; pShort < pLinkageStart; pShort++)
            {
            checkSum = ((checkSum + *pShort) & 0xffff);
#if defined (DEBUG_CHECKSUM)
        printf ("  [4][%d] checksum = %d\n", iStep++, checkSum);
#endif
            }
        }

    // Linkages.
    DgnV8ElementBlank   element;

    ((DgnElementCP) this)->CopyTo (element);

    // Excluding proxy dependency linkage as root ids change from a copy operation.
    if (checkSumType > 2)
        DependencyManagerLinkage::DeleteLinkageFromMSElement (&element, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_ProxyCell);

    pLinkageStart = (UShort const*) &element + element.GetAttributeOffset();

    for (pShort = pLinkageStart,  pEnd = (UShort const *)&element + element.GetSizeWords(); pShort < pEnd; pShort++)
        {
        checkSum = ((checkSum + *pShort) & 0xffff);
#if defined (DEBUG_CHECKSUM)
        printf ("  [5][%d] checksum = %d\n", iStep++, checkSum);
#endif
        }

    return checkSum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetProxyCell
(
ElementHandleCR     dimElement,
ElementId&          cellId,
DPoint3d            *pOrigin,
RotMatrix           *pRMatrix
) const
    {
    DimProxyCellBlock       *pCellBlock;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (NULL != (pCellBlock = (DimProxyCellBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_PROXYCELL, NULL)) &&
        (pCellBlock->checkSum == (0 == pCellBlock->checkSumType ? dim->ComputeCheckSum() : dim->ComputeAdditiveCheckSum(pCellBlock->checkSumType))))
        {
        if (pOrigin)
            *pOrigin = pCellBlock->origin;

        if (pRMatrix)
            *pRMatrix = pCellBlock->rotScale;

        return (BentleyStatus) adim_getCellId ((UInt64*)&cellId, dimElement, DEPENDENCYAPPVALUE_ProxyCell);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHandlersStatus   DimensionHandler::_SetProxyCell
(
EditElementHandleR  dimensionElement,
ElementId const&    cellId,
DPoint3dCR          origin,
RotMatrixCR         rMatrix
)
    {
    if (!dimensionElement.IsValid())
        {
        return DGNHANDLERS_STATUS_BadArg;
        }
    else
        {
        DimProxyCellBlock       proxyCellBlock, *pCellBlock;

        memset (&proxyCellBlock, 0, sizeof(proxyCellBlock));

        proxyCellBlock.nWords   = sizeof (proxyCellBlock) / 2;
        proxyCellBlock.type     = ADBLK_PROXYCELL;
        proxyCellBlock.origin  = origin;
        proxyCellBlock.rotScale = rMatrix;

        mdlDim_insertOptionBlock (dimensionElement, (DimOptionBlockHeader*) &proxyCellBlock, NULL);
        
        {
        DgnV8ElementBlank tmpElement;
        dimensionElement.GetElementCP()->CopyTo(tmpElement);
        DependencyManagerLinkage::DeleteLinkageFromMSElement (&tmpElement, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_ProxyCell);
        DependencyManagerLinkage::AppendSimpleLinkageToMSElement (&tmpElement, DEPENDENCYAPPID_DimensionCell, DEPENDENCYAPPVALUE_ProxyCell, DEPENDENCY_ON_COPY_DeepCopyRootsAlways, cellId.GetValue());
        dimensionElement.ReplaceElement (&tmpElement);
        }
        if (NULL != (pCellBlock = (DimProxyCellBlock*) mdlDim_getOptionBlock (dimensionElement, ADBLK_PROXYCELL, NULL)))
            {
            pCellBlock->checkSumType = CURRENT_DIM_CHECKSUM_TYPE; // Designate additive checksum (inc. textheap)
            pCellBlock->checkSum = dimensionElement.GetElementCP()->ToDimensionElm().ComputeAdditiveCheckSum (pCellBlock->checkSumType);
            }

        return DGNHANDLERS_STATUS_Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_synchProxyCheckSum (EditElementHandleR dimElement)
    {
    if (!dimElement.IsValid())
        return false;

    if  (DIMENSION_ELM == dimElement.GetLegacyType())
        {
        DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();

        DimProxyCellBlock   *pCellBlock = (DimProxyCellBlock*) mdlDim_getEditOptionBlock (dimElement, ADBLK_PROXYCELL, NULL);

        if  (NULL != pCellBlock)
            {
            pCellBlock->checkSumType = CURRENT_DIM_CHECKSUM_TYPE;
            pCellBlock->checkSum = dim->ComputeAdditiveCheckSum (pCellBlock->checkSumType);
            return  true;
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    enforceMasterSubShieldFlags
(
DimStylePropMaskR   shieldFlags,
DimStyleProp        masterProp,
DimStyleProp       *pSubProps,
int                 numSubProps
)
    {
    int     iSub;
    bool    anyOn = false;

    if (isPropertyShielded (&shieldFlags, masterProp))
        {
        anyOn = true;
        }
    else
        {
        for (iSub = 0; iSub < numSubProps; iSub++)
            {
            if (isPropertyShielded (&shieldFlags, pSubProps[iSub]))
                {
                anyOn = true;
                break;
                }
            }
        }

    // Set all master and subs to be the same
    shieldFlags.SetPropertyBit (masterProp, anyOn);

    for (iSub = 0; iSub < numSubProps; iSub++)
        shieldFlags.SetPropertyBit (pSubProps[iSub], anyOn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    consolidateRedundantShieldFlags
(
DimStylePropMaskR   shieldFlags     /* <=> */
)
    {
    int             numSubProps;
    DimStyleProp    masterProp;
    DimStyleProp    subProps[5];

    /*----------------------------------------------------------------------
      Some properties are related so closely that the user thinks of them as
      a single property... so storing independant shield flags for each one
      doesn't make sense.

      1) Arbitrarily designate one of the group to be the "master" and the
         others are "subordinate".
      2) If any of the group is ON, then turn on the master.
      3) Always turn OFF all the subordinates.
      3) Later, only the master should be checked.
    ----------------------------------------------------------------------*/

    // Text Color and text color override flag
    masterProp  = DIMSTYLE_PROP_Text_Color_COLOR;
    subProps[0] = DIMSTYLE_PROP_Text_OverrideColor_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Text Weight and text weight override flag
    masterProp  = DIMSTYLE_PROP_Text_Weight_WEIGHT;
    subProps[0] = DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Term Color and term color override flag
    masterProp  = DIMSTYLE_PROP_Terminator_Color_COLOR;
    subProps[0] = DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Term Weight and term weight override flag
    masterProp  = DIMSTYLE_PROP_Terminator_Weight_WEIGHT;
    subProps[0] = DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Term Style and term style override flag
    masterProp  = DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE;
    subProps[0] = DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Text Orientation flags (above, inline, outside, top-left)
    masterProp  = DIMSTYLE_PROP_Text_Embed_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Text_Location_INTEGER;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Arrow terminator Character and font
    masterProp  = DIMSTYLE_PROP_Terminator_ArrowChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Terminator_ArrowFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Stroke terminator Character and font
    masterProp  = DIMSTYLE_PROP_Terminator_StrokeChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Terminator_StrokeFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Origin terminator Character and font
    masterProp  = DIMSTYLE_PROP_Terminator_OriginChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Terminator_OriginFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Dot terminator Character and font
    masterProp  = DIMSTYLE_PROP_Terminator_DotChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Terminator_DotFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Note terminator Character and font
    masterProp  = DIMSTYLE_PROP_Terminator_NoteChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Terminator_NoteFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Diameter Character and font
    masterProp  = DIMSTYLE_PROP_Symbol_DiameterChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Symbol_DiameterFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Prefix Character and font
    masterProp  = DIMSTYLE_PROP_Symbol_PrefixChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Symbol_PrefixFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Suffix Character and font
    masterProp  = DIMSTYLE_PROP_Symbol_SuffixChar_CHAR;
    subProps[0] = DIMSTYLE_PROP_Symbol_SuffixFont_FONT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Primary, standard label format
    masterProp  = DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT;
    subProps[1] = DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT;
    subProps[2] = DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT;
    numSubProps = 3;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Secondary, standard label format
    masterProp  = DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT;
    subProps[1] = DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT;
    subProps[2] = DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT;
    numSubProps = 3;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Primary, alternate label format
    masterProp  = DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT;
    subProps[1] = DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT;
    subProps[2] = DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT;
    numSubProps = 3;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Secondary, alternate label format
    masterProp  = DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT;
    subProps[1] = DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT;
    subProps[2] = DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT;
    numSubProps = 3;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Primary, alternate threshold criteria
    masterProp  = DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Secondary, alternate threshold criteria
    masterProp  = DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Ordinate Datum Value and use flag
    masterProp  = DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE;
    subProps[0] = DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);

    // Metric Space and thousands separator
    masterProp  = DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT;
    subProps[0] = DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT;
    numSubProps = 1;
    enforceMasterSubShieldFlags (shieldFlags, masterProp, subProps, numSubProps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    stripUnusedShieldFlags
(
DimStylePropMaskP   shieldFlags,    // <=>
ElementHandleCR        dimElement      //  =>
)
    {
    DgnElementCP pDimElm = dimElement.GetElementCP();

    /*----------------------------------------------------------------------
      Some dimstyle properties are optionally stored in the dimension
      elements.  We do not maintain shield flags for those properties if
      they are not being stored. Ex. Option Blocks, Linkages.

      Storing shield flags for unstored properties would be a bad idea,
      since we don't have the value that was to be "shielded".
    ----------------------------------------------------------------------*/
    DimTermBlock    *pTermBlock = NULL;

    /*----------------------------------------------------------------------
      Option Blocks
    ----------------------------------------------------------------------*/
    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_TOLERANCE, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR);
        }
    UInt64 cellID [4] = {INVALID_ELEMENTID, INVALID_ELEMENTID, INVALID_ELEMENTID, INVALID_ELEMENTID};
    if (NULL == (pTermBlock = (DimTermBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMINATOR, cellID)))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_ArrowType_INTEGER);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_StrokeType_INTEGER);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OriginType_INTEGER);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_DotType_INTEGER);

        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_ArrowChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_StrokeChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OriginChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_DotChar_CHAR);
        }
    else
        {
        if (INVALID_ELEMENTID == cellID [DEPENDENCYAPPVALUE_ArrowHeadTerminator -1] || 1 != pTermBlock->flags.arrow)
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_ArrowChar_CHAR);

        if (INVALID_ELEMENTID == cellID[DEPENDENCYAPPVALUE_StrokeTerminator -1] || 1 != pTermBlock->flags.stroke)
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_StrokeChar_CHAR);

        if (INVALID_ELEMENTID == cellID[DEPENDENCYAPPVALUE_OriginTerminator-1] || 1 != pTermBlock->flags.origin)
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OriginChar_CHAR);

        if (INVALID_ELEMENTID == cellID[DEPENDENCYAPPVALUE_DotTerminator-1] || 1 != pTermBlock->flags.dot)
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_DotChar_CHAR);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_PRESYMBOL, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_PrefixChar_CHAR);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_SUFSYMBOL, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_SuffixChar_CHAR);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_DIAMSYM, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_DiameterChar_CHAR);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_PRESUF, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR);

        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR);
        }
    else
        {
        if (pDimElm->ToDimensionElm().flag.dual)
            {
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR);
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR);
            }
        else
            {
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR);
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR);
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR);
            clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR);
            }
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_PRIMARY, NULL))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_Unit_UNITS);

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_SECONDARY, NULL))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_UnitSec_UNITS);

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_TERMSYMB, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_Color_COLOR);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_Weight_WEIGHT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_EXTOFFSET, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_ALTFORMAT, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltThreshold_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT);
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_SECALTFORMAT, NULL))
        {
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT);
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT);
        }

    /*----------------------------------------------------------------------
      Linkages
    ----------------------------------------------------------------------*/
    /*--------------------------------------------------------------------------
      Note on the extension linkage

      It is a packed linkage meaning it doesn't store a value if that value is
      equal to the default value of that property.  We do NOT want to clear the
      shield flags for unstored extension values.  If we did, it would not be
      possible to shield the value in the case where it is equal to the default.
    --------------------------------------------------------------------------*/

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_PrefixCell))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR);

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_SuffixCell))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR);

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_ArrowHeadTerminator))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR);

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_StrokeTerminator))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR);

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_OriginTerminator))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR);

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_DotTerminator))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR);

    if (SUCCESS != adim_getCellId (NULL, dimElement, DEPENDENCYAPPVALUE_NoteTerminator))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR);

    if (SUCCESS != mdlDim_getUnitLabel (NULL, dimElement, DIMLABEL_MASTUNIT))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR);

    if (SUCCESS != mdlDim_getUnitLabel (NULL, dimElement, DIMLABEL_SUBUNIT))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR);

    if (SUCCESS != mdlDim_getUnitLabel (NULL, dimElement, DIMLABEL_SECONDARY_MASTUNIT))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR);

    if (SUCCESS != mdlDim_getUnitLabel (NULL, dimElement, DIMLABEL_SECONDARY_SUBUNIT))
        clearShieldFlag (shieldFlags, DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR);

    /*----------------------------------------------------------------------
      Template
    ----------------------------------------------------------------------*/
    if (true)
        {
        DimensionType   thisType = adim_getTemplateNumber (dimElement);
        int             thisIndex = static_cast<int>(thisType) - 1;

        // Clear all but one template
        for (int iTemplate = 0; iTemplate < 24; iTemplate++)
            {
            if (iTemplate != thisIndex)
                shieldFlags->ClearTemplateBits (iTemplate);
            }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionHandler::SaveShieldsDirect (EditElementHandleR element, DimStylePropMaskR propMask)
    {
    if (0 == mdlDim_getDimStyleID (element) || false == propMask.AnyBitSet ())
        {
        DimStylePropMask::DeleteLinkages (*element.GetElementP());
        }
    else
        {
        consolidateRedundantShieldFlags (propMask);
        stripUnusedShieldFlags (&propMask, element);
        DgnV8ElementBlank tmpElement;
        element.GetElementCP()->CopyTo(tmpElement);
        /*----------------------------------------------------------------------
          Add the shield flags to the dimension
        ----------------------------------------------------------------------*/
        propMask.AppendAsLinkages (tmpElement);
        element.ReplaceElement (&tmpElement);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlDim_setOverridesDirect
(
EditElementHandleR  dimElement,         /* <=> */
DimStylePropMaskP   shieldFlags,        /* <=> */
bool                applyStyle          /*  => */
)
    {
    if (DIMENSION_ELM != dimElement.GetLegacyType())
        return DGNHANDLERS_STATUS_BadArg;

    DimensionHandler& hdlr = DimensionHandler::GetInstance();

    if (NULL == shieldFlags)
        DimStylePropMask::DeleteLinkages (*dimElement.GetElementP());
    else
        hdlr.SaveShieldsDirect (dimElement, *shieldFlags);

    if (false != applyStyle)
        {
        DimensionStylePtr   dimStyle = DimensionStyle::GetByID (dimElement.GetElementCP()->ToDimensionElm().dimStyleId, *dimElement.GetDgnProject());

        if (dimStyle.IsValid())
            hdlr.UpdateFromDimStyle (dimElement, *dimStyle, NULL, ADIM_PARAMS_UPDATE_FROMDIMSTYLE);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionStyle::IsWitnessVisible (ElementHandleCR dimElem, int iPoint) const
    {
    /* If witness lines are globally off in the style, we are done */
    if ( ! m_data.ad1.mode.witness)
        return false;

    int     firstWitness;

    switch (static_cast<DimensionType>(dimElem.GetElementCP()->ToDimensionElm().dimcmd))
        {
        case DimensionType::AngleSize:
        case DimensionType::AngleLocation:
        case DimensionType::AngleLines:
        case DimensionType::AngleAxisX:
        case DimensionType::AngleAxisY:
        case DimensionType::ArcSize:
        case DimensionType::ArcLocation:
            firstWitness = 1;
            break;
        default:
            firstWitness = 0;
            break;
        }

    int     iTemplate = dimElem.GetElementCP()->ToDimensionElm().dimcmd - 1;
    bool    witnessOnInTemplate = false;

    iPoint = (iPoint >= 0) ? iPoint : dimElem.GetElementCP()->ToDimensionElm().nPoints - 1;

    if (iPoint > firstWitness)
        {
        witnessOnInTemplate = m_data.ad4.dim_template[iTemplate].right_witness;
        }
    else
        {
        witnessOnInTemplate = m_data.ad4.dim_template[iTemplate].left_witness;
        }

    return witnessOnInTemplate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DimensionHandler::SetShieldsFromStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle)
    {
    // strip all existing shield flags
    DimStylePropMask::DeleteLinkages (*eeh.GetElementP());

    // Compare to the style in the file, and store the differences
    DimensionStylePtr styleFromFile = DimensionStyle::GetByName (dimStyle.GetName().c_str(), *dimStyle.GetDgnProject());

    if ( ! styleFromFile.IsValid())
        return;

    DimStylePropMaskPtr differences = dimStyle.Compare (*styleFromFile);

    if ( ! differences.IsValid())
        { BeAssert (0); return; }

    DimensionHandler::SaveShieldsDirect (eeh, *differences);

    int     iPoint, numPoints = eeh.GetElementCP()->ToDimensionElm().nPoints;
    bool    witOnInDim;
    bool    witOnInStyle;

    for (iPoint = 0; iPoint < numPoints; iPoint++)
        {
        witOnInDim   = !eeh.GetElementCP()->ToDimensionElm().GetDimTextCP(iPoint)->flags.b.noWitness;
        witOnInStyle = styleFromFile->IsWitnessVisible (eeh, iPoint);

        if (witOnInDim != witOnInStyle)
            eeh.GetElementP()->ToDimensionElmR().GetDimTextP(iPoint)->flags.b.witCtrlLocal = true;
        }

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DimStylePropMaskPtr     DimensionHandler::_GetOverrideFlags (ElementHandleCR eh) const
    {
    return DimStylePropMask::ExtractFromLinkages (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt adim_getCellNameFromId
(
WChar        *pCellName,
int             numChars,
UInt64       elementId,
DgnProjectP project
)
    {
    ElementRefP  elemRef = project->Models().GetElementById(ElementId(elementId)).get();

    if (NULL == elemRef)
        return ERROR;

    return CellUtil::GetCellName (pCellName, numChars, *elemRef->GetUnstableMSElementCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void dgnDimStyle_setTermSymbol
(
DimensionStyleR dimStyle,
int             symbolType,
DimSymbol      *pSymbol,
int             termType, 
UInt64       cellId
)
    {
    DimStyleProp iType, iCell, iFont, iChar;

    switch (termType)
        {
        case DIMSTYLE_VALUE_Terminator_Type_Arrow:
            {
            iType   = DIMSTYLE_PROP_Terminator_ArrowType_INTEGER;
            iCell   = DIMSTYLE_PROP_Terminator_ArrowCellName_MSWCHAR;
            iFont   = DIMSTYLE_PROP_Terminator_ArrowFont_FONT;
            iChar   = DIMSTYLE_PROP_Terminator_ArrowChar_CHAR;
            break;
            }
        case DIMSTYLE_VALUE_Terminator_Type_Stroke:
            {
            iType   = DIMSTYLE_PROP_Terminator_StrokeType_INTEGER;
            iCell   = DIMSTYLE_PROP_Terminator_StrokeCellName_MSWCHAR;
            iFont   = DIMSTYLE_PROP_Terminator_StrokeFont_FONT;
            iChar   = DIMSTYLE_PROP_Terminator_StrokeChar_CHAR;
            break;
            }
        case DIMSTYLE_VALUE_Terminator_Type_Origin:
            {
            iType   = DIMSTYLE_PROP_Terminator_OriginType_INTEGER;
            iCell   = DIMSTYLE_PROP_Terminator_OriginCellName_MSWCHAR;
            iFont   = DIMSTYLE_PROP_Terminator_OriginFont_FONT;
            iChar   = DIMSTYLE_PROP_Terminator_OriginChar_CHAR;
            break;
            }
        case DIMSTYLE_VALUE_Terminator_Type_Dot:
            {
            iType   = DIMSTYLE_PROP_Terminator_DotType_INTEGER;
            iCell   = DIMSTYLE_PROP_Terminator_DotCellName_MSWCHAR;
            iFont   = DIMSTYLE_PROP_Terminator_DotFont_FONT;
            iChar   = DIMSTYLE_PROP_Terminator_DotChar_CHAR;
            break;
            }
        default:
            BeAssert (false);
            // Can't imagine anyting below this will work right?
            return;
            break;    
        }

    /* Because symbolType=3 implies cell + uniform scale, so we need to decouple it here: */
    if  (3 == symbolType)
        {
        symbolType = DIMSTYLE_VALUE_Symbol_TermType_Cell;
        dimStyle.SetBooleanProp (true, DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT);
        }

    dimStyle.SetIntegerProp (symbolType, iType);

    switch (symbolType)
        {
        case 0:     // none
            {
            break;
            }
        case 1:     // char
            {
            dimStyle.SetFontProp (pSymbol->symbol.font, iFont);
            dimStyle.SetCharProp (pSymbol->symbol.symb, iChar);

            break;
            }
        case 2:     // cell
        case 3:     // cell
            {
            WChar         nameBuf[MAX_CELLNAME_LENGTH];

            adim_getCellNameFromId (nameBuf, MAX_CELLNAME_LENGTH, cellId, dimStyle.GetDgnProject());
            dimStyle.SetStringProp (nameBuf, iCell);

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dgnDimStyle_setWitnesslineSymbology
(
DimensionStyleR     dimStyle,
DimensionElm const* pDim
)
    {
    /*-----------------------------------------------------------------------------------
    When a dimension's witness lines have local symbology overridden, chances are that some
    are overridden while others are not.  The rule we use here is that if ANY visible
    witnees line has a local symbology overridden, that one rules!
    -----------------------------------------------------------------------------------*/
    if ( ! adim_usesAnyAltSymb (*pDim))
        return;

    dimStyle.SetBooleanProp (true,                    DIMSTYLE_PROP_ExtensionLine_OverrideColor_BOOLINT);
    dimStyle.SetColorProp   (pDim->altSymb.color,     DIMSTYLE_PROP_ExtensionLine_Color_COLOR);

    dimStyle.SetBooleanProp   (true,                  DIMSTYLE_PROP_ExtensionLine_OverrideLineStyle_BOOLINT);
    dimStyle.SetLineStyleProp (pDim->altSymb.style,   DIMSTYLE_PROP_ExtensionLine_LineStyle_LINESTYLE);

    dimStyle.SetBooleanProp (true,                    DIMSTYLE_PROP_ExtensionLine_OverrideWeight_BOOLINT);
    dimStyle.SetWeightProp  (pDim->altSymb.weight,    DIMSTYLE_PROP_ExtensionLine_Weight_WEIGHT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   adim_getUnitDefinitions
(
UnitDefinitionR     masterUnit,
UnitDefinitionR     subUnit,
ElementHandleCR        dimElement,
bool                isSecondary
)
    {
    DimUnitBlock*   pUnitBlock = NULL;
    int             blockKey       = isSecondary ? ADBLK_SECONDARY             : ADBLK_PRIMARY;
    int             masterLabelKey = isSecondary ? DIMLABEL_SECONDARY_MASTUNIT : DIMLABEL_MASTUNIT;
    int             subLabelKey    = isSecondary ? DIMLABEL_SECONDARY_SUBUNIT  : DIMLABEL_SUBUNIT;

    if (NULL == (pUnitBlock = (DimUnitBlock*) mdlDim_getOptionBlock (dimElement, blockKey, NULL)))
        return ERROR;

    BentleyApi::adim_extractUnitBlock (NULL, &masterUnit, &subUnit, pUnitBlock);

    WChar label[MAX_UNIT_LABEL_LENGTH];

    label[0] = 0L;
    mdlDim_getUnitLabel (label, dimElement, masterLabelKey);
    masterUnit.SetLabel (label);

    label[0] = 0L;
    mdlDim_getUnitLabel (label, dimElement, subLabelKey);
    subUnit.SetLabel (label);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void        dgnDimStyle_setUnitDefinitions
(
DimensionStyleR     dimStyle,
UnitDefinitionCR    masterUnit,
UnitDefinitionCR    subUnit,
bool                isSecondary
)
    {
    DimStyleProp    unitProp        = isSecondary ? DIMSTYLE_PROP_Value_Unit_UNITS                 : DIMSTYLE_PROP_Value_Unit_UNITS;
    DimStyleProp    masterLabelProp = isSecondary ? DIMSTYLE_PROP_Value_UnitLabelSecMaster_MSWCHAR : DIMSTYLE_PROP_Value_UnitLabelMaster_MSWCHAR;
    DimStyleProp    subLabelProp    = isSecondary ? DIMSTYLE_PROP_Value_UnitLabelSecSub_MSWCHAR    : DIMSTYLE_PROP_Value_UnitLabelSub_MSWCHAR;

    dimStyle.SetUnitsProp (masterUnit, subUnit, unitProp);
    dimStyle.SetStringProp (masterUnit.GetLabel().c_str(), masterLabelProp);
    dimStyle.SetStringProp (subUnit.GetLabel().c_str(), subLabelProp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void  DimensionHandler::ExtractDimStyle (DimensionStyleR style, ElementHandleCR eh) const
    {
    DgnElementCP         elemCP  = eh.GetElementCP();

    if (elemCP->GetLegacyType() != DIMENSION_ELM)
        return;

    DimensionElm const* pDim    = &elemCP->ToDimensionElm();

    // The element data is cooked with the model's units and potentially annotation
    // scale.  The style data should be in terms of the non-model units, without
    // annotation scale.

    double annotationScale = 1.0;
    mdlDim_getEffectiveAnnotationScale(&annotationScale, eh);
    if (annotationScale < 1.0e-14)
        {
        BeDataAssert (false && "bad annotationScale");
        annotationScale = 1.0;
        }

    /*--------------------------------------------------------------------------
       Here we are using the TSOs from the embedded LegacyTextStyle to determine the
       user's intentions regarding the 'Explicit TS Props' (see notes at the
       top of this file).

       For example, if the text height TSO is ON, this implies that the user
       intends that the font should be controlled by the DimStyle, which
       implies that the OverrideFont property should be turned on.

       Note that this argument only works for dims that are based on named
       textstyles which have an embedded LegacyTextStyle to examine.  Without the
       embedded LegacyTextStyle to guide us, there is no way to know if the dim's
       text height originated from the DimStyle or the active tcb->textStyle.
       So we take the 'safe' route, which is to always turn ON the Override
       flags, so that a newly placed dimension will match the matched one.
    --------------------------------------------------------------------------*/
    LegacyTextStyleOverrideFlags  elemTSOs;
    memset (&elemTSOs, 0xffff, sizeof(elemTSOs));

    if (0 != pDim->textStyleId)
        {
        LegacyTextStyle   elemTextStyle;

        GetTextStyle (eh, &elemTextStyle);

        elemTSOs = elemTextStyle.overrideFlags;
        }

    style.SetIntegerProp (pDim->textStyleId,            DIMSTYLE_PROP_Text_TextStyleID_INTEGER);

    style.SetBooleanProp (elemTSOs.height,              DIMSTYLE_PROP_Text_OverrideHeight_BOOLINT);
    style.SetDoubleProp  (pDim->text.height/annotationScale,            DIMSTYLE_PROP_Text_Height_DOUBLE);

    style.SetBooleanProp (elemTSOs.width,               DIMSTYLE_PROP_Text_OverrideWidth_BOOLINT);
    style.SetDoubleProp  (pDim->text.width/annotationScale,             DIMSTYLE_PROP_Text_Width_DOUBLE);

    style.SetBooleanProp (elemTSOs.underline,           DIMSTYLE_PROP_Text_OverrideUnderline_BOOLINT);
    style.SetBooleanProp (pDim->flag.underlineText,     DIMSTYLE_PROP_Text_Underline_BOOLINT);

    style.SetBooleanProp (elemTSOs.fontNo,              DIMSTYLE_PROP_Text_Font_BOOLINT);
    style.SetFontProp    (pDim->text.font,              DIMSTYLE_PROP_General_Font_FONT);

    bool colorFlag = false;

    if ( ! pDim->text.b.useColor)
        colorFlag = false;
    else
        colorFlag = elemTSOs.colorFlag;

    style.SetBooleanProp (colorFlag,                    DIMSTYLE_PROP_Text_OverrideColor_BOOLINT);
    style.SetColorProp   (pDim->text.color,             DIMSTYLE_PROP_Text_Color_COLOR);

    style.SetBooleanProp (pDim->text.b.useWeight,       DIMSTYLE_PROP_Text_OverrideWeight_BOOLINT);
    style.SetWeightProp  (pDim->text.weight,            DIMSTYLE_PROP_Text_Weight_WEIGHT);

    style.SetDoubleProp  (pDim->GetScale(),                  DIMSTYLE_PROP_General_DimensionScale_DOUBLE);

    style.SetBooleanProp (true,                         DIMSTYLE_PROP_Placement_OverrideLevel_BOOLINT);
    style.SetLevelProp   (LevelId(pDim->GetLevel()),             DIMSTYLE_PROP_Placement_Level_LEVEL);

    style.SetBooleanProp (true,                         DIMSTYLE_PROP_General_OverrideColor_BOOLINT);
    style.SetColorProp   (pDim->GetSymbology().color,        DIMSTYLE_PROP_General_Color_COLOR);

    style.SetBooleanProp (true,                         DIMSTYLE_PROP_General_OverrideLineStyle_BOOLINT);
    style.SetLineStyleProp (pDim->GetSymbology().style,      DIMSTYLE_PROP_General_LineStyle_LINESTYLE);

    style.SetBooleanProp (true,                         DIMSTYLE_PROP_General_OverrideWeight_BOOLINT);
    style.SetWeightProp  (pDim->GetSymbology().weight,       DIMSTYLE_PROP_General_Weight_WEIGHT);

    dgnDimStyle_setWitnesslineSymbology (style, pDim);

    style.SetBooleanProp (pDim->frmt.angleMeasure,     DIMSTYLE_PROP_Value_AngleMeasure_BOOLINT);
    style.SetIntegerProp (pDim->flag.alignment,        DIMSTYLE_PROP_General_Alignment_INTEGER);
    style.SetIntegerProp (static_cast<int>(adim_getAngleFormat (eh)),    DIMSTYLE_PROP_Value_AngleFormat_INTEGER);
    style.SetIntegerProp (pDim->frmt.dmsAccuracyMode,  DIMSTYLE_PROP_Value_DMSPrecisionMode_INTEGER);

    style.SetBooleanProp (! pDim->frmt.noNonStackedSpace,    DIMSTYLE_PROP_Value_SpaceAfterNonStackedFraction_BOOLINT);
    style.SetBooleanProp (! pDim->flag.stackFract,           DIMSTYLE_PROP_Text_StackedFractions_BOOLINT);
    style.SetIntegerProp (pDim->text.b.stackedFractionAlign, DIMSTYLE_PROP_Text_StackedFractionAlignment_INTEGER);
    style.SetIntegerProp (pDim->text.b.stackedFractionType,  DIMSTYLE_PROP_Text_StackedFractionType_INTEGER);
    style.SetIntegerProp (pDim->text.b.superscriptMode,      DIMSTYLE_PROP_Text_SuperscriptMode_INTEGER);

    if (pDim->frmt.angleMeasure && mdlDim_isAngularDimension (eh))
        {
        style.SetIntegerProp (pDim->frmt.primaryAccuracy,    DIMSTYLE_PROP_Value_AnglePrecision_INTEGER);
        }
    else if (DimensionType::LabelLine == static_cast<DimensionType>(pDim->dimcmd))
        {
        style.SetAccuracyProp ((byte) pDim->frmt.primaryAccuracy,   DIMSTYLE_PROP_Value_Accuracy_ACCURACY);
        style.SetIntegerProp  (pDim->frmt.secondaryAccuracy, DIMSTYLE_PROP_Value_AnglePrecision_INTEGER);
        }
    else
        {
        style.SetAccuracyProp ((byte) pDim->frmt.primaryAccuracy,   DIMSTYLE_PROP_Value_Accuracy_ACCURACY);
        style.SetAccuracyProp ((byte) pDim->frmt.secondaryAccuracy, DIMSTYLE_PROP_Value_SecAccuracy_ACCURACY);
        }

    style.SetBooleanProp (pDim->frmt.decimalComma,         DIMSTYLE_PROP_Text_DecimalComma_BOOLINT);
    style.SetBooleanProp (pDim->frmt.superscriptLSD,       DIMSTYLE_PROP_Value_SuperscriptLSD_BOOLINT);
    style.SetBooleanProp (pDim->frmt.roundLSD,             DIMSTYLE_PROP_Value_RoundLSD_BOOLINT);
    style.SetBooleanProp (pDim->frmt.omitLeadDelim,        DIMSTYLE_PROP_Text_OmitLeadingDelimiter_BOOLINT);
    style.SetBooleanProp (pDim->frmt.metricSpc,            DIMSTYLE_PROP_Value_ThousandsSpace_BOOLINT);

    style.SetBooleanProp (pDim->frmt.adp_subunits,         DIMSTYLE_PROP_Value_ShowSubUnit_BOOLINT);
    style.SetBooleanProp (pDim->frmt.adp_delimiter,        DIMSTYLE_PROP_Value_ShowDelimiter_BOOLINT);
    style.SetBooleanProp (pDim->frmt.adp_label,            DIMSTYLE_PROP_Value_ShowUnitLabel_BOOLINT);
    style.SetBooleanProp (! pDim->text.b.adp_nomastunits,  DIMSTYLE_PROP_Value_ShowMasterUnit_BOOLINT);
    style.SetBooleanProp (pDim->frmt.adp_allowZeroMast,    DIMSTYLE_PROP_Value_ShowZeroMasterUnit_BOOLINT);
    style.SetBooleanProp (! pDim->frmt.adp_hideZeroSub,    DIMSTYLE_PROP_Value_ShowZeroSubUnit_BOOLINT);

    style.SetBooleanProp (pDim->frmt.adp_subunits2,        DIMSTYLE_PROP_Value_SecShowSubUnit_BOOLINT);
    style.SetBooleanProp (pDim->frmt.adp_delimiter2,       DIMSTYLE_PROP_Value_SecShowDelimiter_BOOLINT);
    style.SetBooleanProp (pDim->frmt.adp_label2,           DIMSTYLE_PROP_Value_SecShowUnitLabel_BOOLINT);
    style.SetBooleanProp (! pDim->text.b.adp_nomastunits2, DIMSTYLE_PROP_Value_SecShowMasterUnit_BOOLINT);
    style.SetBooleanProp (pDim->frmt.adp_allowZeroMast2,   DIMSTYLE_PROP_Value_SecShowZeroMasterUnit_BOOLINT);
    style.SetBooleanProp (! pDim->frmt.adp_hideZeroSub2,   DIMSTYLE_PROP_Value_SecShowZeroSubUnit_BOOLINT);

    style.SetDoubleProp  (pDim->geom.witOffset  / pDim->text.height, DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE);
    style.SetDoubleProp  (pDim->geom.witExtend  / pDim->text.height, DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE);
    style.SetDoubleProp  (pDim->geom.textMargin / pDim->text.height, DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE);
    style.SetDoubleProp  (pDim->geom.textLift   / pDim->text.height, DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE);
    style.SetDoubleProp  (pDim->geom.margin     / pDim->text.width,  DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE);
    style.SetDoubleProp  (pDim->geom.termWidth  / pDim->text.height, DIMSTYLE_PROP_Terminator_Width_DOUBLE);
    style.SetDoubleProp  (pDim->geom.termHeight / pDim->text.height, DIMSTYLE_PROP_Terminator_Height_DOUBLE);
    style.SetDoubleProp  (pDim->geom.stackOffset/annotationScale,    DIMSTYLE_PROP_General_StackOffset_DOUBLE);

    style.SetBooleanProp (pDim->flag.thousandSep,  DIMSTYLE_PROP_Value_ThousandsSeparator_BOOLINT);
    style.SetBooleanProp (pDim->flag.joiner,       DIMSTYLE_PROP_ExtensionLine_Join_BOOLINT);
    style.SetBooleanProp (pDim->flag.boxText,      DIMSTYLE_PROP_Text_Box_BOOLINT);
    style.SetBooleanProp (pDim->flag.capsuleText,  DIMSTYLE_PROP_Text_Capsule_BOOLINT);

    bool    primaryUsesAngular, secondaryUsesAngular;

    adim_checkUseOfAngularSettings (primaryUsesAngular, secondaryUsesAngular, eh);

    if (! primaryUsesAngular)
        {
        style.SetBooleanProp (pDim->flag.leadingZero,      DIMSTYLE_PROP_Text_LeadingZero_BOOLINT);
        style.SetBooleanProp (pDim->flag.trailingZeros,    DIMSTYLE_PROP_Value_ShowTrailingZeros_BOOLINT);
        }
    else
        {
        style.SetBooleanProp (pDim->flag.leadingZero,      DIMSTYLE_PROP_Value_AngleLeadingZero_BOOLINT);
        style.SetBooleanProp (pDim->flag.trailingZeros,    DIMSTYLE_PROP_Value_AngleTrailingZeros_BOOLINT);
        }

    if (! secondaryUsesAngular)
        {
        style.SetBooleanProp (pDim->flag.leadingZero2,     DIMSTYLE_PROP_Text_SecLeadingZero_BOOLINT);
        style.SetBooleanProp (pDim->flag.trailingZeros2,   DIMSTYLE_PROP_Value_SecShowTrailingZeros_BOOLINT);
        }
    else
        {
        // Do nothing.  Style has no place to put the secondary angular leading / trailing zero flags.
        }

    style.SetBooleanProp (pDim->flag.embed,            DIMSTYLE_PROP_Text_Embed_BOOLINT);
    style.SetBooleanProp (pDim->flag.horizontal,       DIMSTYLE_PROP_Text_Horizontal_BOOLINT);
    style.SetBooleanProp (!pDim->flag.noAutoTextLift,  DIMSTYLE_PROP_Text_AutoLift_BOOLINT);

    style.SetIntegerProp (pDim->flag.arrowhead,        DIMSTYLE_PROP_Terminator_Arrowhead_INTEGER);

    DimStyleProp_FitOptions     fitOption   = DIMSTYLE_VALUE_FitOption_MoveTermsFirst;

    if (SUCCESS == mdlDim_getFitOption(&fitOption, eh))
        style.SetIntegerProp (fitOption,               DIMSTYLE_PROP_General_FitOption_INTEGER);

    style.SetBooleanProp (pDim->flag.relDimLine,       DIMSTYLE_PROP_General_RelativeDimLine_BOOLINT);

    style.SetBooleanProp (pDim->extFlag.noLineThruArrowTerm,   DIMSTYLE_PROP_Terminator_NoLineThruArrow_BOOLINT);
    style.SetBooleanProp (pDim->extFlag.noLineThruStrokeTerm,  DIMSTYLE_PROP_Terminator_NoLineThruStroke_BOOLINT);
    style.SetBooleanProp (pDim->extFlag.noLineThruOriginTerm,  DIMSTYLE_PROP_Terminator_NoLineThruOrigin_BOOLINT);
    style.SetBooleanProp (pDim->extFlag.noLineThruDotTerm,     DIMSTYLE_PROP_Terminator_NoLineThruDot_BOOLINT);

    if (mdlDim_isNoteDimension (eh))
        {
        int         frameType = 0;
        int         just = 0;
        bool        bLeaderDisplay = false;

        if (SUCCESS == mdlDim_getNoteLeaderDisplay (&bLeaderDisplay, eh))
            style.SetBooleanProp(TO_BOOL(bLeaderDisplay), DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT);

        if (SUCCESS == mdlDim_getNoteFrameType ((DimStyleProp_MLNote_FrameType *) &frameType, eh))
            style.SetIntegerProp (frameType, DIMSTYLE_PROP_MLNote_FrameType_INTEGER);

        if (SUCCESS == mdlDim_getNoteHorizontalJustification ((DimStyleProp_MLNote_Justification *) &just, eh))
            style.SetIntegerProp (just, DIMSTYLE_PROP_MLNote_Justification_INTEGER);
        }

    if ( ! (mdlDim_isNoteDimension (eh) || mdlDim_isLabelLineDimension (eh)) )
        {
        style.SetBooleanProp (pDim->flag.dual,                 DIMSTYLE_PROP_Text_ShowSecondary_BOOLINT);
        style.SetBooleanProp (pDim->flag.tolerance,            DIMSTYLE_PROP_Tolerance_Show_BOOLINT);
        style.SetBooleanProp (pDim->flag.tolmode,              DIMSTYLE_PROP_Tolerance_Mode_BOOLINT);
        }

    if (true)
        {
        // ADBLK_PRIMARY
        UnitDefinition      masterUnit, subUnit;

        if (SUCCESS == adim_getUnitDefinitions (masterUnit, subUnit, eh, false))
            {
            style.SetBooleanProp (false, DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT);
            }
        else
            {
            style.SetBooleanProp (true, DIMSTYLE_PROP_Value_UseWorkingUnits_BOOLINT);

            masterUnit = eh.GetDgnModelP()->GetModelInfo().GetMasterUnit();
            subUnit    = eh.GetDgnModelP()->GetModelInfo().GetSubUnit();
            }

        dgnDimStyle_setUnitDefinitions (style, masterUnit, subUnit, false);
        }

    if (true)
        {
        // ADBLK_SECONDARY
        UnitDefinition      masterUnit, subUnit;

        if (SUCCESS == adim_getUnitDefinitions (masterUnit, subUnit, eh, true))
            dgnDimStyle_setUnitDefinitions (style, masterUnit, subUnit, true);
        }

    if (true)
        {
        DimTolrBlock*       pTolBlock  = NULL;

        if (NULL != (pTolBlock = (DimTolrBlock *) mdlDim_getOptionBlock (eh, ADBLK_TOLERANCE, NULL)))
            {
            int pmCharType;

            style.SetDoubleProp (pTolBlock->lowerTol/annotationScale, DIMSTYLE_PROP_Tolerance_LowerValue_DOUBLE);
            style.SetDoubleProp (pTolBlock->upperTol/annotationScale, DIMSTYLE_PROP_Tolerance_UpperValue_DOUBLE);

            style.SetDoubleProp (pTolBlock->tolHeight   / pDim->text.height, DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE);
            style.SetDoubleProp (pTolBlock->tolHorizSep / pDim->text.height, DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE);
            style.SetDoubleProp (pTolBlock->tolVertSep  / pDim->text.height, DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE);

            style.SetCharProp (pTolBlock->prefix, DIMSTYLE_PROP_Symbol_TolPrefixChar_CHAR);
            style.SetCharProp (pTolBlock->suffix, DIMSTYLE_PROP_Symbol_TolSuffixChar_CHAR);

            if (0 == pTolBlock->pmChar)
                pmCharType = DIMSTYLE_VALUE_Symbol_CustomType_Default;
            else
                pmCharType = DIMSTYLE_VALUE_Symbol_CustomType_Character;

            style.SetCharProp (pTolBlock->pmChar,  DIMSTYLE_PROP_Symbol_PlusMinusChar_CHAR);
            style.SetIntegerProp (pmCharType,      DIMSTYLE_PROP_Symbol_PlusMinusType_INTEGER);

            style.SetBooleanProp (pTolBlock->flags.stackEqual,  DIMSTYLE_PROP_Tolerance_StackEqual_BOOLINT);
            style.SetBooleanProp (pTolBlock->flags.signForZero, DIMSTYLE_PROP_Tolerance_ShowSignForZero_BOOLINT);
            }
        else
            {
            style.SetBooleanProp (false,           DIMSTYLE_PROP_Tolerance_Show_BOOLINT);
            }
        }

    if (true)
        {
        DimTermBlock*       pTermBlock = NULL;
        UInt64 cellID  [4] = {INVALID_ELEMENTID, INVALID_ELEMENTID, INVALID_ELEMENTID, INVALID_ELEMENTID};
        if (NULL != (pTermBlock = (DimTermBlock *) mdlDim_getOptionBlock (eh, ADBLK_TERMINATOR, cellID)))
            {
            /*---------------------------------------------------------------------------
            The terminator symbol type in the option block can have value=3 which implies
            2 variables: cell + uniform scale.  The value is to be decoupled in the
            setTermSymbol function.
            ---------------------------------------------------------------------------*/
            style.SetBooleanProp (false,  DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT);
            dgnDimStyle_setTermSymbol (style, cellID[DEPENDENCYAPPVALUE_ArrowHeadTerminator-1] == INVALID_ELEMENTID ? 0 :pTermBlock->flags.arrow,  &pTermBlock->arrow,  DIMSTYLE_VALUE_Terminator_Type_Arrow, cellID[DEPENDENCYAPPVALUE_ArrowHeadTerminator-1]);
            dgnDimStyle_setTermSymbol (style, cellID[DEPENDENCYAPPVALUE_StrokeTerminator-1]    == INVALID_ELEMENTID ? 0 :pTermBlock->flags.stroke, &pTermBlock->stroke, DIMSTYLE_VALUE_Terminator_Type_Stroke, cellID[DEPENDENCYAPPVALUE_StrokeTerminator-1]);
            dgnDimStyle_setTermSymbol (style, cellID[DEPENDENCYAPPVALUE_OriginTerminator-1]    == INVALID_ELEMENTID ? 0 :pTermBlock->flags.origin, &pTermBlock->origin, DIMSTYLE_VALUE_Terminator_Type_Origin, cellID[DEPENDENCYAPPVALUE_OriginTerminator-1]);
            dgnDimStyle_setTermSymbol (style, cellID[DEPENDENCYAPPVALUE_DotTerminator-1]       == INVALID_ELEMENTID ? 0 :pTermBlock->flags.dot,    &pTermBlock->dot,    DIMSTYLE_VALUE_Terminator_Type_Dot, cellID[DEPENDENCYAPPVALUE_DotTerminator-1]);
            }
        else
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_TermType_Default, DIMSTYLE_PROP_Terminator_ArrowType_INTEGER);
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_TermType_Default, DIMSTYLE_PROP_Terminator_StrokeType_INTEGER);
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_TermType_Default, DIMSTYLE_PROP_Terminator_OriginType_INTEGER);
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_TermType_Default, DIMSTYLE_PROP_Terminator_DotType_INTEGER);
            }
        }

    if (true)
        {
        DimPreSufBlock*     pCharBlock = NULL;

        if (NULL != (pCharBlock = (DimPreSufBlock *) mdlDim_getOptionBlock (eh, ADBLK_PRESUF, NULL)))
            {
            if (pDim->flag.dual)
                {
                style.SetCharProp (pCharBlock->upperPrefix, DIMSTYLE_PROP_Symbol_UpperPrefixChar_CHAR);
                style.SetCharProp (pCharBlock->upperSuffix, DIMSTYLE_PROP_Symbol_UpperSuffixChar_CHAR);
                style.SetCharProp (pCharBlock->lowerPrefix, DIMSTYLE_PROP_Symbol_LowerPrefixChar_CHAR);
                style.SetCharProp (pCharBlock->lowerSuffix, DIMSTYLE_PROP_Symbol_LowerSuffixChar_CHAR);
                }
            else
                {
                style.SetCharProp (pCharBlock->mainPrefix, DIMSTYLE_PROP_Symbol_MainPrefixChar_CHAR);
                style.SetCharProp (pCharBlock->mainSuffix, DIMSTYLE_PROP_Symbol_MainSuffixChar_CHAR);
                }
            }
        }

    if (true)
        {
        DimSymBlock    *pSymBlock  = NULL;

        if (NULL != (pSymBlock = (DimSymBlock *) mdlDim_getOptionBlock (eh, ADBLK_DIAMSYM, NULL)))
            {
            style.SetCharProp (pSymBlock->symChar, DIMSTYLE_PROP_Symbol_DiameterChar_CHAR);
            style.SetFontProp (pSymBlock->symFont, DIMSTYLE_PROP_Symbol_DiameterFont_FONT);
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_CustomType_Character, DIMSTYLE_PROP_Symbol_DiameterType_INTEGER);
            }
        else
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_CustomType_Default, DIMSTYLE_PROP_Symbol_DiameterType_INTEGER);
            }
        }

    if (true)
        {
        UInt64       symbolId;
        WChar         cellNameBuf[MAX_CELLNAME_LENGTH];
        DimSymBlock    *pSymBlock  = NULL;

        cellNameBuf[0] = '\0';
        if (NULL != (pSymBlock = (DimSymBlock *) mdlDim_getOptionBlock (eh, ADBLK_PRESYMBOL, NULL)))
            {
            style.SetCharProp (pSymBlock->symChar, DIMSTYLE_PROP_Symbol_PrefixChar_CHAR);
            style.SetFontProp (pSymBlock->symFont, DIMSTYLE_PROP_Symbol_PrefixFont_FONT);
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_PreSufType_Character, DIMSTYLE_PROP_Symbol_PrefixType_INTEGER);
            }
        else
        if (SUCCESS == adim_getCellId (&symbolId, eh, DEPENDENCYAPPVALUE_PrefixCell) &&
            SUCCESS == adim_getCellNameFromId (cellNameBuf, MAX_CELLNAME_LENGTH, symbolId, eh.GetDgnProject()))
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_PreSufType_Cell, DIMSTYLE_PROP_Symbol_PrefixType_INTEGER);
            style.SetStringProp  (cellNameBuf, DIMSTYLE_PROP_Symbol_PrefixCellName_MSWCHAR);
            }
        else
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_PreSufType_None, DIMSTYLE_PROP_Symbol_PrefixType_INTEGER);
            }
        }

    if (true)
        {
        UInt64       symbolId;
        WChar         cellNameBuf[MAX_CELLNAME_LENGTH];
        DimSymBlock    *pSymBlock  = NULL;

        cellNameBuf[0] = '\0';
        if (NULL != (pSymBlock = (DimSymBlock *) mdlDim_getOptionBlock (eh, ADBLK_SUFSYMBOL, NULL)))
            {
            style.SetCharProp (pSymBlock->symChar, DIMSTYLE_PROP_Symbol_SuffixChar_CHAR);
            style.SetFontProp (pSymBlock->symFont, DIMSTYLE_PROP_Symbol_SuffixFont_FONT);
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_PreSufType_Character, DIMSTYLE_PROP_Symbol_SuffixType_INTEGER);
            }
        else
        if (SUCCESS == adim_getCellId (&symbolId, eh, DEPENDENCYAPPVALUE_SuffixCell) &&
            SUCCESS == adim_getCellNameFromId (cellNameBuf, MAX_CELLNAME_LENGTH, symbolId, eh.GetDgnProject()))
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_PreSufType_Cell, DIMSTYLE_PROP_Symbol_SuffixType_INTEGER);
            style.SetStringProp  (cellNameBuf, DIMSTYLE_PROP_Symbol_SuffixCellName_MSWCHAR);
            }
        else
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_PreSufType_None, DIMSTYLE_PROP_Symbol_SuffixType_INTEGER);
            }
        }

    if (true)
        {
        DimTermSymbBlock*   pSymbBlock = NULL;

        if (NULL != (pSymbBlock = (DimTermSymbBlock *) mdlDim_getOptionBlock (eh, ADBLK_TERMSYMB, NULL)))
            {
            style.SetBooleanProp (pSymbBlock->termSymb.useStyle,  DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT);
            style.SetLineStyleProp (pSymbBlock->termSymb.style,   DIMSTYLE_PROP_Terminator_LineStyle_LINESTYLE);

            style.SetBooleanProp (pSymbBlock->termSymb.useWeight, DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT);
            style.SetWeightProp (pSymbBlock->termSymb.weight,     DIMSTYLE_PROP_Terminator_Weight_WEIGHT);

            style.SetBooleanProp (pSymbBlock->termSymb.useColor,  DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT);
            style.SetColorProp (pSymbBlock->termSymb.color,       DIMSTYLE_PROP_Terminator_Color_COLOR);
            }
        else
            {
            style.SetBooleanProp (false,   DIMSTYLE_PROP_Terminator_OverrideLineStyle_BOOLINT);
            style.SetBooleanProp (false,   DIMSTYLE_PROP_Terminator_OverrideWeight_BOOLINT);
            style.SetBooleanProp (false,   DIMSTYLE_PROP_Terminator_OverrideColor_BOOLINT);
            }
        }

    if (true)
        {
        DimAltFmtBlock*     pAltFmtBlock = NULL;

        if (pDim->text.b.hasAltFormat &&
            NULL != (pAltFmtBlock = (DimAltFmtBlock *) mdlDim_getOptionBlock (eh, ADBLK_ALTFORMAT, NULL)))
            {
            style.SetBooleanProp (true, DIMSTYLE_PROP_Value_AltIsActive_BOOLINT);

            style.SetDoubleProp  (pAltFmtBlock->thresholdValue/annotationScale,         DIMSTYLE_PROP_Value_AltThreshold_DOUBLE);
            style.SetBooleanProp (! pAltFmtBlock->flags.greaterThanThreshold, DIMSTYLE_PROP_Value_AltShowWhenThresholdLess_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.equalToThreshold, DIMSTYLE_PROP_Value_AltShowWhenThresholdEqual_BOOLINT);

            style.SetAccuracyProp((byte)pAltFmtBlock->flags.accuracy,  DIMSTYLE_PROP_Value_AltAccuracy_ACCURACY);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_delimiter,   DIMSTYLE_PROP_Value_AltShowDelimiter_BOOLINT);
            style.SetBooleanProp (!pAltFmtBlock->flags.adp_nomastunits,DIMSTYLE_PROP_Value_AltShowMasterUnit_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_subunits,    DIMSTYLE_PROP_Value_AltShowSubUnit_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_label,       DIMSTYLE_PROP_Value_AltShowUnitLabel_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_allowZeroMast, DIMSTYLE_PROP_Value_AltShowZeroMasterUnit_BOOLINT);
            style.SetBooleanProp (! pAltFmtBlock->flags.adp_hideZeroSub, DIMSTYLE_PROP_Value_AltShowZeroSubUnit_BOOLINT);
            }
        else
            {
            style.SetBooleanProp (false, DIMSTYLE_PROP_Value_AltIsActive_BOOLINT);
            }
        }

    if (true)
        {
        DimAltFmtBlock*     pAltFmtBlock = NULL;

        if (pDim->text.b.hasSecAltFormat &&
            NULL != (pAltFmtBlock = (DimAltFmtBlock *) mdlDim_getOptionBlock (eh, ADBLK_SECALTFORMAT, NULL)))
            {
            style.SetBooleanProp (true, DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT);

            style.SetDoubleProp  (pAltFmtBlock->thresholdValue/annotationScale,        DIMSTYLE_PROP_Value_AltSecThreshold_DOUBLE);
            style.SetBooleanProp (! pAltFmtBlock->flags.greaterThanThreshold, DIMSTYLE_PROP_Value_AltSecShowWhenThresholdLess_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.equalToThreshold, DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT);

            style.SetAccuracyProp((byte)pAltFmtBlock->flags.accuracy,  DIMSTYLE_PROP_Value_AltSecAccuracy_ACCURACY);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_delimiter,   DIMSTYLE_PROP_Value_AltSecShowDelimiter_BOOLINT);
            style.SetBooleanProp (!pAltFmtBlock->flags.adp_nomastunits,DIMSTYLE_PROP_Value_AltSecShowMasterUnit_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_subunits,    DIMSTYLE_PROP_Value_AltSecShowSubUnit_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_label,       DIMSTYLE_PROP_Value_AltSecShowUnitLabel_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.equalToThreshold, DIMSTYLE_PROP_Value_AltSecShowWhenThresholdEqual_BOOLINT);
            style.SetBooleanProp (pAltFmtBlock->flags.adp_allowZeroMast, DIMSTYLE_PROP_Value_AltSecShowZeroMasterUnit_BOOLINT);
            style.SetBooleanProp (! pAltFmtBlock->flags.adp_hideZeroSub, DIMSTYLE_PROP_Value_AltSecShowZeroSubUnit_BOOLINT);
            }
        else
            {
            style.SetBooleanProp (false, DIMSTYLE_PROP_Value_AltSecIsActive_BOOLINT);
            }
        }

    if (true)
        {
        DimOffsetBlock*     pOffsetBlock = NULL;

        if (NULL != (pOffsetBlock = (DimOffsetBlock *) mdlDim_getOptionBlock (eh, ADBLK_EXTOFFSET, NULL)))
            {
            // just set enable mode now, auto mode will be set in the extensions section later on:
            style.SetIntegerProp (DIMSTYLE_VALUE_BallAndChain_Mode_On, DIMSTYLE_PROP_BallAndChain_Mode_INTEGER);
            style.SetIntegerProp (pOffsetBlock->flags.terminator,      DIMSTYLE_PROP_BallAndChain_ChainTerminator_INTEGER);
            style.SetIntegerProp (pOffsetBlock->flags.chainType,       DIMSTYLE_PROP_BallAndChain_ChainType_INTEGER);
            style.SetBooleanProp (pOffsetBlock->flags.elbow,           DIMSTYLE_PROP_BallAndChain_ShowTextLeader_BOOLINT);
            style.SetIntegerProp (pOffsetBlock->flags.alignment,       DIMSTYLE_PROP_BallAndChain_Alignment_INTEGER);
            style.SetBooleanProp (pOffsetBlock->flags.noDockOnDimLine, DIMSTYLE_PROP_BallAndChain_NoDockOnDimLine_BOOLINT);
            }
        else
            {
            style.SetIntegerProp (DIMSTYLE_VALUE_BallAndChain_Mode_None, DIMSTYLE_PROP_BallAndChain_Mode_INTEGER);
            }
        }

    /*-----------------------------------------------------------------------------------
        template
    -----------------------------------------------------------------------------------*/
    DimensionType iDimType = adim_getTemplateNumber (eh);

    if (pDim->nPoints > 1 && pDim->GetDimTextCP(1)->flags.b.just)
        {
        DimStyleProp_Text_Justification     textJust = (DimStyleProp_Text_Justification)pDim->GetDimTextCP(1)->flags.b.just;
        if (DIMSTYLE_VALUE_Text_Justification_CenterLeft == textJust && pDim->GetDimTextCP(1)->flags.b.pushTextRight)
            textJust = DIMSTYLE_VALUE_Text_Justification_CenterRight;

        style.SetIntegerProp (textJust, DIMSTYLE_PROP_Text_Justification_INTEGER);
        }

    if (DimensionType::None != iDimType)
        style.SetTemplateData (pDim->tmpl, iDimType);

    /*-----------------------------------------------------------------------------------
        This tries to account for strange logic in updateFromDimStyle
    -----------------------------------------------------------------------------------*/
    double centerSize;

    style.GetDoubleProp (centerSize, DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE);

    if (!centerSize && (!pDim->tmpl.centerLeft && !pDim->tmpl.centerRight &&
                        !pDim->tmpl.centerTop  && !pDim->tmpl.centerBottom) )
        {
        if (pDim->geom.centerSize == pDim->geom.termHeight)
            centerSize = 0.0;
        }
    else
        {
        centerSize = pDim->geom.centerSize;
        }

    style.SetDoubleProp (centerSize/annotationScale, DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE);

    /*-----------------------------------------------------------------------------------
    Display of witness lines is controlled per point on the dimension.

    If all the witness lines are OFF in the dim, turn them OFF style, otherwise leave them on.
    Also use the points to determine the correct setting for left / right witness.
    -----------------------------------------------------------------------------------*/
    if (true)
        {
        int     iPoint;
        bool    hasAtLeastOneWitness = false;
        UShort  hasLeftWitness       = false;
        UShort  hasRightWitness      = false;

        int     iFirstWitness = DIM_ANGULAR (static_cast<DimensionType>(pDim->dimcmd)) ? 1 : 0;

        for (iPoint = iFirstWitness; iPoint < pDim->nPoints; iPoint++)
            {
            if (!pDim->GetDimTextCP(iPoint)->flags.b.noWitness)
                {
                if (iPoint == iFirstWitness)
                    hasLeftWitness   = true;
                else
                    hasRightWitness  = true;

                hasAtLeastOneWitness = true;
                }
            }

        style.SetBooleanProp (hasAtLeastOneWitness, DIMSTYLE_PROP_ExtensionLine_ShowAny_BOOLINT);

        style.SetTemplateFlagProp (hasLeftWitness,  iDimType, DIMSTYLE_PROP_ExtensionLine_Left_TEMPLATEFLAG);
        style.SetTemplateFlagProp (hasRightWitness, iDimType, DIMSTYLE_PROP_ExtensionLine_Right_TEMPLATEFLAG);
        }

    /*-----------------------------------------------------------------------------------
        Extensions
    -----------------------------------------------------------------------------------*/
    if (true)
        {
        DimStyleExtensions  elmExtensions;

        memset (&elmExtensions, 0, sizeof (DimStyleExtensions));

        if (SUCCESS != mdlDim_getStyleExtension (&elmExtensions, eh))
            {
            /*-------------------------------------------------------------------------------------------------------------
            When the extensions linkage does not exist on the dimension, we initialize the extensions on the style to none,
            to make the match logic work the same as the display logic.  The stroke code updates version and memsets style 
            extensions to 0, except for the angle formats.  But since angle formats are used from tcb anyway, instead from 
            style, there is no benefit to update angle formats for the style here.  TR 329846.
            -------------------------------------------------------------------------------------------------------------*/
            style.SetStyleExtensions (elmExtensions);
            }
        else
            {
            if (elmExtensions.modifiers & STYLE_Extension_PrimaryToleranceAccuracy)
                style.SetAccuracyProp (*((byte*)&elmExtensions.primaryTolAcc), DIMSTYLE_PROP_Tolerance_Accuracy_ACCURACY);

            if (elmExtensions.modifiers & STYLE_Extension_SecondaryToleranceAccuracy)
                style.SetAccuracyProp (*((byte*)&elmExtensions.secondaryTolAcc), DIMSTYLE_PROP_Tolerance_SecAccuracy_ACCURACY);

            if (elmExtensions.modifiers & STYLE_Extension_RoundOff)
                style.SetDoubleProp (elmExtensions.dRoundOff, DIMSTYLE_PROP_Value_RoundOff_DOUBLE);

            if (elmExtensions.modifiers & STYLE_Extension_SecondaryRoundOff)
                style.SetDoubleProp (elmExtensions.dSecondaryRoundOff, DIMSTYLE_PROP_Value_SecRoundOff_DOUBLE);

            if (elmExtensions.modifiers & STYLE_Extension_StackedFractionScale)
                style.SetDoubleProp (elmExtensions.stackedFractionScale, DIMSTYLE_PROP_Text_StackedFractionScale_DOUBLE);

            if (elmExtensions.modifiers & STYLE_Extension_AnnotationScale)
                style.SetDoubleProp (elmExtensions.dAnnotationScale, DIMSTYLE_PROP_Placement_AnnotationScale_DOUBLE);

            if (elmExtensions.modifiers & STYLE_Extension_InlineTextLift)
                style.SetDoubleProp (elmExtensions.dInlineTextLift, DIMSTYLE_PROP_Text_InlineTextLift_DOUBLE);

            if (elmExtensions.modifiers & STYLE_Extension_Dwg_Flags)
                style.SetDwgSpecificFlags (*((UShort*) &elmExtensions.dwgSpecifics.flags));

            /* If it is an ordinate dimension that the style is being matched from, then
             * adopt the ordinate dim specific settings */
            if (mdlDim_isOrdinateDimension (eh))
                {
                if (elmExtensions.modifiers & STYLE_Extension_OrdinateDatumValue)
                    style.SetDoubleProp (elmExtensions.dOrdinateDatumValue, DIMSTYLE_PROP_Value_OrdDatumValue_DOUBLE);

                style.SetBooleanProp (elmExtensions.flags.uOrdUseDatumValue,    DIMSTYLE_PROP_Value_OrdUseDatumValue_BOOLINT);
                style.SetBooleanProp (elmExtensions.flags.uOrdDecrementReverse, DIMSTYLE_PROP_Value_OrdDecrementReverse_BOOLINT);
                style.SetBooleanProp (elmExtensions.flags2.uOrdFreeLocation,    DIMSTYLE_PROP_Value_OrdFreeLocation_BOOLINT);
                }

            /* If it is an label line that the style is being matched from, then
             * adopt the label line dim specific settings */
            if (mdlDim_isLabelLineDimension (eh))
                {
                style.SetBooleanProp (elmExtensions.flags.labelLineSupressAngle,    DIMSTYLE_PROP_Value_LabelLineSuppressAngle_BOOLINT);
                style.SetBooleanProp (elmExtensions.flags.labelLineSupressLength,   DIMSTYLE_PROP_Value_LabelLineSuppressLength_BOOLINT);
                style.SetBooleanProp (elmExtensions.flags.labelLineInvertLabels,    DIMSTYLE_PROP_Value_LabelLineInvertLabels_BOOLINT);
                style.SetBooleanProp (elmExtensions.flags.uLabelLineAdjacentLabels, DIMSTYLE_PROP_Value_LabelLineAdjacentLabels_BOOLINT);
                }

            /* If it is a note that the style is being matched from, then adopt the note dim specific settings */
            if (mdlDim_isNoteDimension (eh))
                {
                // left vertical attachment - must be paired with get/set API's
                DimStyleProp_MLNote_VerAttachment   verticalAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_Top;
                adim_getMLNoteVerLeftAttachment (&elmExtensions, (UInt16*)&verticalAttachment);
                style.SetIntegerProp (verticalAttachment,                           DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER);

                // right vertical attachment - must be paired with get/set API's
                adim_getMLNoteVerRightAttachment (&elmExtensions, (UInt16*)&verticalAttachment);
                style.SetIntegerProp (verticalAttachment,                           DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER);

                style.SetIntegerProp (elmExtensions.flags.uMultiJustVertical,           DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER);
                style.SetIntegerProp (adim_extensionsGetNoteTerminator(elmExtensions),  DIMSTYLE_PROP_Terminator_Note_INTEGER);
                style.SetIntegerProp (elmExtensions.flags2.uNoteTextRotation,           DIMSTYLE_PROP_MLNote_TextRotation_INTEGER);
                style.SetIntegerProp (elmExtensions.flags2.uNoteHorAttachment,          DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER);
                style.SetBooleanProp (elmExtensions.flags2.uNoteLeaderType,             DIMSTYLE_PROP_MLNote_LeaderType_BOOLINT);
                style.SetBooleanProp (elmExtensions.flags2.uNoteScaleFrame,             DIMSTYLE_PROP_MLNote_ScaleFrame_BOOLINT);

                // Store only the relevant data based on the note terminator type
                switch (elmExtensions.flags2.uNoteTerminatorType)
                    {
                    case 0:     // none
                        {
                        style.SetIntegerProp (elmExtensions.flags2.uNoteTerminatorType, DIMSTYLE_PROP_Terminator_NoteType_INTEGER);
                        break;
                        }
                    case 1:     // char
                        {
                        style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_TermType_Character, DIMSTYLE_PROP_Terminator_NoteType_INTEGER);

                        if (elmExtensions.modifiers & STYLE_Extension_NoteTerminatorFont)
                            style.SetFontProp (elmExtensions.iNoteTerminatorFont, DIMSTYLE_PROP_Terminator_NoteFont_FONT);

                        if (elmExtensions.modifiers & STYLE_Extension_NoteTerminatorChar)
                            style.SetCharProp (elmExtensions.iNoteTerminatorChar, DIMSTYLE_PROP_Terminator_NoteChar_CHAR);

                        break;
                        }
                    case 2:     // cell
                    case 3:     // cell
                        {
                        /* decouple type 3 into cell + uniform scale, i.e. revert the process of updateFromDimStyle */
                        style.SetIntegerProp (DIMSTYLE_VALUE_Symbol_TermType_Cell, DIMSTYLE_PROP_Terminator_NoteType_INTEGER);
                        if  (3 == elmExtensions.flags2.uNoteTerminatorType)
                            style.SetBooleanProp (true, DIMSTYLE_PROP_Terminator_UniformCellScale_BOOLINT);

                        WChar     noteCellName[MAX_CELLNAME_LENGTH];
                        UInt64   scDefId = 0;

                        adim_getCellId (&scDefId, eh, DEPENDENCYAPPVALUE_NoteTerminator);
                        adim_getCellNameFromId (noteCellName, MAX_CELLNAME_LENGTH, scDefId, eh.GetDgnProject());
                        style.SetStringProp (noteCellName, DIMSTYLE_PROP_Terminator_NoteCellName_MSWCHAR);
                        break;
                        }
                    }

                if (elmExtensions.modifiers & STYLE_Extension_NoteLeftMargin)
                    style.SetDoubleProp (elmExtensions.dNoteLeftMargin, DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE);

                if (elmExtensions.modifiers & STYLE_Extension_NoteLowerMargin)
                    style.SetDoubleProp (elmExtensions.dNoteLowerMargin, DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE);

                if (elmExtensions.modifiers & STYLE_Extension_NoteFrameScale)
                    style.SetDoubleProp (elmExtensions.dNoteFrameScale, DIMSTYLE_PROP_MLNote_FrameScale_DOUBLE);
                }

            /* The elbow length is needed for multiple dim types, so do not check for any specific dim type. */
            if (elmExtensions.modifiers & STYLE_Extension_NoteElbowLength)
                style.SetDoubleProp (elmExtensions.dNoteElbowLength, DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE);

            style.SetBooleanProp (elmExtensions.flags.uNoReduceFraction,            DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags.uNoReduceAltFraction,         DIMSTYLE_PROP_Value_NoReduceAltFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags.uNoReduceTolFraction,         DIMSTYLE_PROP_Value_NoReduceTolFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags.uNoReduceSecFraction,         DIMSTYLE_PROP_Value_NoReduceSecFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags.uNoReduceAltSecFraction,      DIMSTYLE_PROP_Value_NoReduceAltSecFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags.uNoReduceTolSecFraction,      DIMSTYLE_PROP_Value_NoReduceTolSecFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags2.uNotUseModelAnnotationScale, DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT);

            if (elmExtensions.modifiers & STYLE_Extension_BncElbowLength)
                style.SetDoubleProp (elmExtensions.dBncElbowLength / pDim->text.height, DIMSTYLE_PROP_BallAndChain_ElbowLength_DOUBLE);

            /* No effort is made to keep the flags modifier bit accurate, it is derived from the flags. */
            style.SetBooleanProp (elmExtensions.flags3.uNoTermsOutside,         DIMSTYLE_PROP_General_SuppressUnfitTerminators_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags3.uUseBncElbowLength,      DIMSTYLE_PROP_BallAndChain_UseElbowLength_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags3.uTightFitTextAbove,      DIMSTYLE_PROP_General_TightFitTextAbove_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags3.uAutoBallNChain,         DIMSTYLE_PROP_Value_NoReduceFraction_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags3.uFitInclinedTextBox,     DIMSTYLE_PROP_General_FitInclinedTextBox_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags3.uIgnoreMinLeader,        DIMSTYLE_PROP_General_UseMinLeader_BOOLINT);
            style.SetBooleanProp (elmExtensions.flags3.uExtendDimLineUnderText, DIMSTYLE_PROP_General_ExtendDimLineUnderText_BOOLINT);

#if defined (BEIJING_DGNPLATFORM_WIP_JS) //TODO Dim dependant properties
// NEEDSWORK:
// These properties cannot be copied directly using the api without disrupting other properties,
// need to consider each one along with its non-extension conterparts

            style.m_extensions.flags2.uTextLocation               = elmExtensions.flags2.uTextLocation;
            style.m_extensions.flags3.uPushTextRight              = elmExtensions.flags3.uPushTextRight;
            style.m_extensions.flags3.uAutoBallNChain             = elmExtensions.flags3.uAutoBallNChain;
#endif
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/03
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr       DimensionHandler::_GetDimensionStyle (ElementHandleCR dimElm) const
    {
    UInt64           styleID = mdlDim_getDimStyleID (dimElm);
    DimensionStylePtr   dimStyle;

    if (0 != styleID)
        {
        // If the dimension refers to a named style, intialize to that style.
        dimStyle = DimensionStyle::GetByID (styleID, *dimElm.GetDgnProject());
        }

    if (!dimStyle.IsValid())
        {
        // initialize to the settings element from the dimension's file
        dimStyle = DimensionStyle::GetSettings (*( dimElm.GetDgnProject()));
        }

    if (!dimStyle.IsValid())//A file could have no settings element as well. Use default
        dimStyle = DimensionStyle::Create (L"", *dimElm.GetDgnProject());
    // Overlay the dimension's settings
    ExtractDimStyle (*dimStyle, dimElm);

    return dimStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
// static DimensionStylePtr    adim_getDimStyle
// (
// ElementHandleCR    dimElem
// )
//     {
//     IDimensionQuery*    dimQuery = dynamic_cast <IDimensionQuery*> (&dimElem.GetHandler ());
// 
//     if (NULL == dimQuery)
//         { BeAssert(0); return NULL; }
// 
//     return dimQuery->GetDimensionStyle (dimElem);
//     }
// 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 08/06
+---------------+---------------+---------------+---------------+---------------+------*/
// static bool    areDimSettingsDifferent
// (
// ElementHandleCR    dim1,
// ElementHandleCR    dim2
// )
//     {
//     DimensionStylePtr dimStyle1 = adim_getDimStyle (dim1);
//     DimensionStylePtr dimStyle2 = adim_getDimStyle (dim2);
// 
//     if (dimStyle1.IsNull() || dimStyle2.IsNull())
//         { BeAssert(0); return false; }
// 
//     DimStylePropMaskPtr diffs = dimStyle1->Compare (*dimStyle2);
// 
//     return diffs->AnyBitSet();
//     }
// 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 08/06
+---------------+---------------+---------------+---------------+---------------+------*/
// ITransactionHandler::PreActionStatus DimensionHandler::_OnReplace (EditElementHandleR eeh, ElementHandleCR eh)
//     {
//     if (!mdlDim_isNoteDimension (eh))
//         return PRE_ACTION_Ok;
// 
//     DgnModelP dgnModel = eh.GetDgnModel()->GetDgnModelP();
// 
//     // Dimensions are modified during upgrade processing, but those changes should not require
//     // note processing.  This test is being added in response to a request from the ProjectWise
//     // ScanRefs program.  ScanRefs processes dgn files out-of-context, meaning they cannot guarantee
//     // that all external resources are available.  Specifically, if a necessary rsc font is not
//     // available, and the following code tries to roundtrip the Note text through a TextBlock, then
//     // the text elements will be changed to the fallback rsc font.  That is a bad side effect for
//     // ScanRefs.
//     if (dgnModel->IsInUpgradeProcessing ())
//         return PRE_ACTION_Ok;
// 
//     // this kludge is in here to prevent this code from executing while doing geographic reprojection. 
//     //  userData2 is set to this value in CacheReproject::DoReproject for where it is set. Fixes TR#311767.
//     MSElementDescrCP    edP;
//     if ( (NULL != (edP = eeh.PeekElementDescrCP())) && (0x04151956 == edP->h.appData2) )
//         return PRE_ACTION_Ok;
// 
//     /*-------------------------------------------------------------------
//       Step 1
//       Compute any missing dimsettings on the note dimension and store them.
//       The purpose of doing it just before replacing the dimension in file
//       is that we cover all modify codepaths.
//     -------------------------------------------------------------------*/
//     bool        changed = false;
// 
//     mdlNote_rectifyDimSettings (&changed, eeh, eeh, eeh.GetDgnModel ());
// 
//     /*-------------------------------------------------------------------
//       Step 2
//       One or more dimension settings may have changed (because of saving
//       dimstyle). We cannot let the dependency callback handle it because
//       it doesn't have the old dim before the change occured. Therefore,
//       this is the only place to update the note.
//     -------------------------------------------------------------------*/
//     if (!changed || !areDimSettingsDifferent (eh, eeh))
//         return PRE_ACTION_Ok;
// 
//     // Do not regenerate if we are in the middle of ddgn refresh local copy,
//     // because if the note cell had a conflict and we rewrote it here, its
//     // conflict mark would be removed. As a result it wouldn't appear in the
//     // conflicts list.
// #if defined (BEIJING_DGNPLATFORM_WIP_JS) //TODO Dim SharedFile IsUpdateWorkfileInProgress
//     DgnProjectP    dgnFile = eh.GetDgnProject ());
// 
//     if ((NULL != dgnFile) && (Bentley::DgnPlatform::DgnHistory::SharedFile::IsUpdateWorkfileInProgress (dgnFile)))
//         return PRE_ACTION_Ok;
// #endif
// 
//     //getOwnedCell
//     EditElementHandle cellElm;
//     UInt64 cellID  = INVALID_ELEMENTID;
//     if (SUCCESS != mdlNote_getRootNoteCellId (&cellID, eh) || SUCCESS != cellElm.FindByID (cellID, eh.GetDgnModel ()))
//         return PRE_ACTION_Ok;
// 
//     UInt64 ownerDimElementID = INVALID_ELEMENTID;
//     NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&cellElm.GetHandler());
//     if (NULL == noteHandler)
//         return PRE_ACTION_Ok;
// 
//     if (SUCCESS != noteHandler->GetLeaderDimension (ownerDimElementID, NULL, cellElm) || ownerDimElementID != eh.GetElementCP ()->ehdr.uniqueId)
//         return PRE_ACTION_Ok;
// 
//         ElementHandle textElemHandle;
//     if (SUCCESS != mdlNote_findTextNodeElement (textElemHandle, cellElm))
//         return PRE_ACTION_Ok;
//     
//     // Prepare the new dim and set its elementRef to indicate that it represents the modified state of a persistent dim
//     if (eeh.PeekElementDescrCP ())
//         eeh.GetElementDescrP ()->GetElementRef() = eh.GetElementRef ();
// 
//     EditElementHandle  newDimElmHandle;
//     EditElementHandle  newCellElemHandle;
//     if (SUCCESS == mdlNote_updateSettings (newCellElemHandle, &textElemHandle, NULL, eeh, &eh, NULL, eeh.GetDgnModel (), true, NULL, NULL))
//         newCellElemHandle.ReplaceInModel (cellElm.GetElementRef());
// 
//     return PRE_ACTION_Ok;
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        DimensionElm::GetPoint (int index) const
    {
    BeAssert (index < nPoints);

    if (-1 == index)
        index = nPoints -1;

    DimPoint const  *dimPoint = point + index;

    DPoint3d requiredPoint = dimPoint->point;

    if (0 == index)
        {
        if (!Is3d())
            requiredPoint.z = 0.0;
        return requiredPoint;
        }

    if (!flag.relStat && dimPoint->text.flags.b.relative)
        requiredPoint.Add(point[0].point);

    if (!Is3d())
        requiredPoint.z = 0.0;

    return requiredPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DimText*        DimensionElm::GetDimTextP (UInt32 index)
    {
    return const_cast<DimText*> (GetDimTextCP(index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DimText const* DimensionElm::GetDimTextCP (UInt32 index) const
    {
    BeAssert (index < nPoints);
    if (index >= nPoints)
        return NULL;

    return  &((point + index)->text);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DimViewBlock const*   DimensionElm::GetDimViewBlockCP () const
    {
    return reinterpret_cast<DimViewBlock const*> (point + (nPoints ? nPoints : 1));
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
 DimViewBlock*   DimensionElm::GetDimViewBlockP ()
    {
    return const_cast<DimViewBlock*> (GetDimViewBlockCP());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    abeesh.basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionElm::SetPoint (DPoint3dCR pointIn, int index)
    {
    DimPoint *dimPoint = point + index;
    dimPoint->point = pointIn;
    
    if ((0 == index) || (!(!flag.relStat && dimPoint->text.flags.b.relative)))
        return;

    DPoint3d origin = point[0].point;
    origin.Negate();
    ((point + index)->point).Add (origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double          DimensionElm::GetScale () const
    {
    // some corrupt files have dim->scale == NAN. This will find that.
    if (!BeNumerical::BeFinite(scale) || BeNumerical::BeIsnan (scale))
        return 1.0;

    return scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionElm::SetScale (double val)
    {
    // some corrupt files have dim->scale == NAN. This will find that.
    if (!BeNumerical::BeFinite(val) || BeNumerical::BeIsnan (val))
        return;

    scale = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionElm::DeletePoint (int index)
    {
    char* dst = (char*)(point + index);
    char* src = dst + sizeof(DimPoint);
    char *end = (char*)this + ((DgnElementCP) this)->Size ();

    memmove (dst, src, end - src);

    nPoints--;

    DecrementSizeWords(sizeof(DimPoint) / 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionElm::InsertPoint (int index)
    {
    //Increment the point count
    if (index == 0 && nPoints == 0)
        {
        nPoints++;
        return;
        }

    size_t dimSize = ((DgnElementCP) this)->Size ();
    char* srcP = (char *) (point + index);
    char* dstP = srcP + static_cast<ptrdiff_t>(sizeof (DimPoint));
    char* endP = (char *) (this) + static_cast<ptrdiff_t>(dimSize);

    memmove (dstP, srcP, endP - srcP);
    memset ((point + index), 0, sizeof (DimPoint));

    IncrementSizeWords(sizeof(DimPoint) / 2);
    nPoints++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
byte            DimensionElm::GetNumberOfPoints () const
    {
    return nPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionElm::IsComputedDataDifferent (DimensionElm const* oldElm) const
    {
    // NOTE: Check if other parts of dimension computed from the assoc points changed for dependency callback implementation...
    if (GetSizeWords() != oldElm->GetSizeWords())
        return true;

    int     pointStart = offsetof (DimensionElm, point);
    int     pointEnd   = pointStart + (this->nPoints * sizeof (DimPoint));

    // Yuck...see if something about the dimension really changed
    if (0 != memcmp (oldElm, this, pointStart))
        return true;

    size_t  dimElmSize = (this->GetSizeWords() > MAX_V8_ELEMENT_SIZE ? MAX_V8_ELEMENT_SIZE : (this->GetSizeWords() * 2));

    if (0 != memcmp ((byte*) (oldElm) + pointEnd, (byte*) (this) + pointEnd, dimElmSize - pointEnd))
        return true;

    for (int index = 0; index < this->nPoints; index++)
        {
        if (!LegacyMath::RpntEqual (&this->point[index].point, &oldElm->point[index].point))
            return true;
        }

    return false;
    }
