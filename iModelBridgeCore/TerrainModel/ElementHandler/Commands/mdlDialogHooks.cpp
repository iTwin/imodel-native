/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/mdlDialogHooks.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "StdAfx.h"

#pragma unmanaged 
//#include    <msdimstyle.h>
//#include    <midimstyle.fdf>
//#include    <dimapps.ids>
//#include    <WordlibControl.h>

//TBD - port 'padlock' hook item entirely here
#define DIMSTYLE                L"DIMSTYLE"

// mdlDimStyle_getDoubleDirect (activeDimStyle, &value, &valueInherited, styleProperty, GET_EFFECTIVE_VALUE)
// If GET_EFFECTIVE_VALUE is TRUE mdlDimStyle_getDoubleDirect tests property checkbox value
// and returns value accordingly.
// Pass FALSE to ignore checkbox setting
#define GET_EFFECTIVE_DIMSTYLE_VALUE    (TRUE)

//#define DIALOGID_DimStyle                       2
#define TEXTID_GenericDistanceProp              4
#define GENERICID_GenericLock                     2

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
inline TextStyleProperty GetPairedTextProperty (DimStyleProp dimProp)
    {
    switch (dimProp)
        {
        case DIMSTYLE_PROP_Text_Height_DISTANCE:
            { return TextStyle_Height; }
        case DIMSTYLE_PROP_Text_Width_DISTANCE:
            { return TextStyle_Width; }
        }
    BeAssert (0);
    return TextStyle_InvalidProperty;
    }

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
template<typename ITEM_TYPE> class DialogItemHookOverride
{
    MdlDesc   *m_itemHookMD;	/* mdl process descriptor for hook */
    void    *m_itemHookFunc;	/* mdl hook offset */

public:
    DialogItemHookOverride (void) : m_itemHookMD (NULL), m_itemHookFunc (NULL)
        {}

private:
    void Init (MdlDesc *itemHookMD, void *itemHookFunc)
        {
        BeAssert (NULL == m_itemHookMD && NULL == m_itemHookFunc && "-");
        m_itemHookMD = itemHookMD;
        m_itemHookFunc = itemHookFunc;
        }

public:
    //ToDo void Init (RawItemHdr *rawItemP, MdlFunctionP hook)
    //    {
    //    BeAssert (NULL != rawItemP && "-");
    //    Init (rawItemP->itemHookMD, rawItemP->itemHookFunc);
    //    rawItemP->itemHookMD = mdlSystem_getCurrMdlDesc ();
    //    rawItemP->itemHookFunc = hook;
    //    }

    void Clean (void)
        {
        m_itemHookMD = NULL;
        m_itemHookFunc = NULL;
        }

    void Call (DialogItemMessage *dimP)
        {
        if (IsEngaged ())
            mdlDialog_callFunction (m_itemHookMD, m_itemHookFunc, dimP);
        }

    bool IsEngaged (void) const
        {
        return NULL != m_itemHookMD && NULL != m_itemHookFunc;
        }

}; // End DialogItemHookOverride class 

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
class HeightWidthTextHookOverride
{

public:

    static DimStyleProp GetProperty (DialogItemP diP)
        {
        return DimStyleProp (diP->itemArg);
        }

    static DialogItem* GetToolsettingsDialogItem (DimStyleProp styleProperty)
        {
        MSDialogP   toolsettingsP;
        DialogItem* txtRsc;

        if (NULL == (toolsettingsP = mdlDialog_getToolSettings ()))
            { return NULL; }

        for (int i=0; NULL != (txtRsc = mdlDialog_itemGetByTypeAndId (toolsettingsP, RTYPE_Text, TEXTID_GenericDistanceProp, i)); i = txtRsc->itemIndex + 1)
            {
            if (styleProperty == GetProperty(txtRsc))
                { return txtRsc; }
            }
        return NULL;
        }

}; // End HeightWidthTextHookOverride
#ifdef ToDo
/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
class ToolsettingsDimStyleHooks : public DialogItemHookOverride<DItem_GenericRsc>
{
static void     Hook (DialogItemMessage *dimP);

static ToolsettingsDimStyleHooks s_ovr;

public:

    static ToolsettingsDimStyleHooks& Get (void)
        { return s_ovr; }

    void Init (RawItemHdr *rih)
        { __super::Init (rih, &Hook); }

}; // End ToolsettingsDimStyleHooks class

ToolsettingsDimStyleHooks ToolsettingsDimStyleHooks::s_ovr;

///// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
inline bool GetLockValue (void)
    {
    DialogBox*  toolsettingsP;
    DialogItem* lockP;

    toolsettingsP = mdlDialog_getToolSettings ();
    BeAssert (toolsettingsP);
    if (!toolsettingsP)
        { return false; }

    lockP = mdlDialog_itemGetByTypeAndId (toolsettingsP, RTYPE_Generic, GENERICID_GenericLock, 0);
    BeAssert (lockP);
    if (!lockP)
        { return false; }

    // ???
    if (DIMSTYLE_PROP_Text_Height_DOUBLE != lockP->itemArg /* || \
        0 != strcmp (DIMSTYLE, mdlSystem_getMdlTaskID(lockP->rawItemP->itemHookMD))*/)
        { return false; }
    return 0 != lockP->rawItemP->userDataP;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod 							JoshSchifter	08/04
+---------------+---------------+---------------+---------------+---------------+------*/
static BoolInt  dimStyle_unApplyAnnotationScale
(
DgnDimStyleP    dimStyle    /* <=> */
)
    {
    if ( ! dimStyle->bUsingAnnotationScale)
        return FALSE;

    mdlDimStyle_unapplyAnnotationScaleToScaledTextSize (dimStyle);

    return TRUE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod 							JoshSchifter	08/04
+---------------+---------------+---------------+---------------+---------------+------*/
static BoolInt  dimStyle_applyAnnotationScale
(
DgnDimStyleP    dimStyle    /* <=> */
)
    {
    if (dimStyle->bUsingAnnotationScale)
        return FALSE;

    mdlDimStyle_applyAnnotationScaleToUnscaledTextSize (dimStyle);

    return TRUE;
    }

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
void     ToolsettingsDimStyleHooks::Hook (DialogItemMessage *dimP)
    {
    switch (dimP->messageType)
        {
        case DITEM_MESSAGE_BUTTON:
            {
            int*            val;
            DialogItem*     txtWidth;
            DgnDimStyleP    activeDimStyle;
            double          value;
            BoolInt         isValueInherited;
            StatusInt       result;

            dimP->msgUnderstood = TRUE;
            if (dimP->u.button.buttonTrans != BUTTONTRANS_UP)
                { return; }

            val = (int*)&dimP->dialogItemP->rawItemP->userDataP;
            (*val) = !(*val);
            if (!(*val))
                {
                mdlDialog_rItemDraw (dimP->dialogItemP->rawItemP);
                return;
                }

            if (NULL == (activeDimStyle = mdlDimStyle_getActive()))
                { return; }
            result = mdlDimStyle_getDoubleProp2 (activeDimStyle, &value, &isValueInherited,DIMSTYLE_PROP_Text_Height_DOUBLE, GET_EFFECTIVE_DIMSTYLE_VALUE);
            BeAssert (SUCCESS == result && "Dimension Style property getting error");
            if (0 != activeDimStyle->textStyleId || !isValueInherited)
                {
                result = mdlDimStyle_setDoubleDirect (activeDimStyle, value,DIMSTYLE_PROP_Text_Width_DOUBLE);
                BeAssert (SUCCESS == result && "Dimension Style property setting error");
                }
            else
                {
                BoolInt     REDRAW = TRUE;

                TextStyleManager::GetActiveStyle()->SetProperty (TextStyle_Width, value);
                txtWidth = HeightWidthTextHookOverride::GetToolsettingsDialogItem (DIMSTYLE_PROP_Text_Width_DOUBLE);
                BeAssert (txtWidth && "-");
                if (txtWidth)
                    {
                    mdlDialog_rItemReloadData (txtWidth->rawItemP, REDRAW);
                    mdlDialog_itemsSynch (dimP->db);
                    }
                }
            return;
            }
        }
    Get().Call (dimP);

    if (DITEM_MESSAGE_DESTROY == dimP->messageType)
        Get().Clean ();
    }

///// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
static void DimStyleComboMonitor (DialogItemMessage const* dimP)
    {
    TRACE ("Combo: %d", dimP->messageType);
    switch (dimP->messageType)
        {
        case DITEM_MESSAGE_SETSTATE:
            {
            DialogItem* txtHeight;
            DialogItem* txtWidth;
            
            txtHeight = HeightWidthTextHookOverride::GetToolsettingsDialogItem (DIMSTYLE_PROP_Text_Height_DOUBLE);
            txtWidth = HeightWidthTextHookOverride::GetToolsettingsDialogItem (DIMSTYLE_PROP_Text_Width_DOUBLE);
            BeAssert (txtHeight && txtWidth && "-");
            if (NULL != txtHeight)
                { mdlDialog_rItemSynch (txtHeight->rawItemP); }
            if (NULL != txtWidth)
                { mdlDialog_rItemSynch (txtWidth->rawItemP); }
            break;
            }
        }
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
static int messageClasses[] = {MESSAGE_CLASS_ITEM};
static void    DialogItemFilter (DialogFilterMessage *dfmP)
    {
    if (dfmP->beforeHook)
        {
        if (DITEM_MESSAGE_CREATE == dfmP->current.dimP->messageType && \
            NULL != dfmP->current.dimP->dialogItemP->rawItemP && \
            NULL != dfmP->current.dimP->dialogItemP->rawItemP->itemHookMD && \
            NULL != dfmP->current.dimP->dialogItemP->rawItemP->itemHookFunc && \
            DIALOGID_ToolSettings == dfmP->current.dimP->dialogId )
            {
            switch (dfmP->current.dimP->dialogItemP->type)
                {
                case RTYPE_Text:
                    {
                    if (TEXTID_GenericDistanceProp == dfmP->current.dimP->dialogItemP->id && \
                        0 == strcmp (DIMSTYLE, mdlSystem_getMdlTaskID(dfmP->current.dimP->dialogItemP->rawItemP->itemHookMD)))
                        {
                        DimStyleProp styleProperty;

                        styleProperty = HeightWidthTextHookOverride::GetProperty (dfmP->current.dimP->dialogItemP);
                        switch (styleProperty)
                            {
                            case DIMSTYLE_PROP_Text_Height_DOUBLE:
                            case DIMSTYLE_PROP_Text_Width_DOUBLE:
                                //HeightWidthTextHookOverride::Get (styleProperty).Init (dfmP->current.dimP->dialogItemP->rawItemP);
                                break;
                            }
                        }
                    break;
                    }
                case RTYPE_Generic:
                    if (DIMSTYLE_PROP_Text_Height_DOUBLE == dfmP->current.dimP->dialogItemP->itemArg && \
                        0 == strcmp (DIMSTYLE, mdlSystem_getMdlTaskID(dfmP->current.dimP->dialogItemP->rawItemP->itemHookMD)))
                        { ToolsettingsDimStyleHooks::Get().Init (dfmP->current.dimP->dialogItemP->rawItemP); }
                    break;
                }
            }
        }
    else if (DIALOGID_ToolSettings == dfmP->current.dimP->dialogId /*&& \
        DITEM_MESSAGE_CREATE == dfmP->current.dimP->messageType */ && \
        RTYPE_ComboBox == dfmP->current.dimP->dialogItemP->type && \
        COMBOBOXID_DimStyle == dfmP->current.dimP->dialogItemP->id)
        {
        DimStyleComboMonitor (dfmP->current.dimP);
        //DtmStyleComboBoxHook::Get().Init (dfmP->current.dimP->dialogItemP->rawItemP);
        }
    }
#endif

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void EngageDialogFilter (void)
    {
//ToDo    mdlDialog_filterPublish (DialogItemFilter, _countof (messageClasses), messageClasses, "TerrainModelListener");
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void DisengageDialogFilter (void)
    {
// ToDo    mdlDialog_filterRemove (mdlSystem_getCurrMdlDesc());
// ToDo    ToolsettingsDimStyleHooks::Get().Clean ();
    }

#ifdef ToDo
namespace SpotLabelHelpers {

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
struct TextStyleEventHandler : Bentley::Ustn::Text::ITextStyleEventHandler
{
    virtual void OnTextStyleEvent (DgnTextStyleCP before, DgnTextStyleCP after, TextStyleEventType type) override
        { wordlib_textstyleevent (before, after, type); }

}; // End TextStyleEventHandler struct

} // end SpotLabelHelpers namespace

static Bentley::Ustn::WordlibControl    *s_dummyWordlibControl = NULL;
static SpotLabelHelpers::TextStyleEventHandler s_dummyTextStyleEventHandler;
#endif
/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void EngageWordlib (void)
    {
    mdlSystem_loadMdlProgram (L"TextEditor.ma", nullptr, nullptr);
    // ToDo
    //int messageClasses[] = {MESSAGE_CLASS_HANDLER};

    //BeAssert (NULL == s_dummyWordlibControl);
    //s_dummyWordlibControl = new Bentley::Ustn::WordlibControl ();
    //mdlWordlib_initialize (*s_dummyWordlibControl);
    //TextStyleManager::AddEventHandler (s_dummyTextStyleEventHandler);
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void DisengageWordlib (void)
    {
    // ToDo
    //BeAssert (NULL != s_dummyWordlibControl);
    //mdlWordlib_terminate (*s_dummyWordlibControl);
    //delete s_dummyWordlibControl;
    //s_dummyWordlibControl = NULL;
    //TextStyleManager::RemoveEventHandler (s_dummyTextStyleEventHandler);
    }

StatusInt    dimStyle_scaleDistanceToModel
(
double             *pValue,         /* <=> */
DgnModelP           sourceCache,    /*  => */
DgnModelP           destCache       /*  => */
)
    {
    double          uorScale;
    StatusInt       status;

    if (SUCCESS == (status = dgnModel_getUorScaleBetweenModels (&uorScale, sourceCache, destCache)))
        {
        *pValue *= uorScale;
        }

    return status;
    }

static StatusInt    dimStyle_doubleStringFromValue
(
WStringR            string,     /* <= */
double              value,      /* => */
bool                isDistance, /* => */
DimensionStyleCR    dimStyle    /* => */
)
    {
    if ( ! isDistance)
        {
        WString::Sprintf (string, L"%f", value);
        }
    else
        {
        DgnFileP    dgnFile = dimStyle.GetFile();
        DgnModelP   defaultModel = dgnFile->LoadRootModelById (NULL, dgnFile->GetDefaultModelId(), false, false);

        dimStyle_scaleDistanceToModel (&value, defaultModel, ISessionMgr::GetActiveDgnModelP());

        mdlString_fromUors (string, value);
        }

    return SUCCESS;
    }


void hook_labelContoursTextHeightWidth (DialogItemMessage* dimP)
    {
    static bool creationComplete = false;

    switch (dimP->messageType)
        {
        case DITEM_MESSAGE_CREATE:
            {
            long    bFalse = FALSE;
            wchar_t    minValStr[64] = L"0.0";

            dimP->msgUnderstood = TRUE;
            //dimStyle_getMinValueStringForProperty
//ToDo            sprintf (minValStr, L"%f", "0.0");
            mdlDialog_textSetInfo (NULL, NULL, NULL, NULL, NULL, minValStr, NULL, NULL, NULL,
                                   &bFalse, FALSE, dimP->dialogItemP->rawItemP);
            return;
            }

        case DITEM_MESSAGE_ALLCREATED:
            creationComplete = true;
            break;

        case DITEM_MESSAGE_DESTROY:
            {
            creationComplete = false;
            break;
            }

        case DITEM_MESSAGE_GETSTATE:
            {
            DimensionStylePtr   dimStyle = DimensionStyle::GetActive();

            if (dimStyle.IsNull ())
                { return; }

            double       value;
            DimStyleProp iProperty = HeightWidthTextHookOverride::GetProperty (dimP->dialogItemP);

            dimStyle->GetEffectiveDoubleProp (value, NULL, iProperty);

            WString valueString;
            dimStyle_doubleStringFromValue (valueString, value, true, *dimStyle);

            dimP->u.value.msValueDescrP->SetWChar(valueString.c_str());
            dimP->u.value.hookHandled = true;
            //DimStyleProp    dimStyleProperty;
            //double          value;
            //StatusInt       result;
            //bool*        NOT_INTERESTED_IN_VALUE_INHERITED = NULL;
            //bool         needToReapplyAnnScale;

            //needToReapplyAnnScale = dimStyle_unApplyAnnotationScale (dimStyle);
            //dimP->msgUnderstood = TRUE;
            //dimStyleProperty = 
            //result = mdlDimStyle_getDoubleProp2 (dimStyle, &value, NOT_INTERESTED_IN_VALUE_INHERITED, dimStyleProperty, GET_EFFECTIVE_DIMSTYLE_VALUE);
            //BeAssert (SUCCESS == result && "Dimension Style property getting error");
            //mdlString_fromUors (dimP->u.value.stringValueP, value);
            //if (needToReapplyAnnScale)
            //    dimStyle_applyAnnotationScale (dimStyle);
            //dimP->u.value.hookHandled = TRUE;
            return;
            }

        case DITEM_MESSAGE_SETSTATE:
            {
            //DgnDimStyleP    activeDimStyle;
            //double          currValue, newValue;
            //StatusInt       result;
            //DimStyleProp    dimStyleProperty;
            //bool         needToReapplyAnnScale, isValueInherited;

            //if (NULL == (activeDimStyle = mdlDimStyle_getActive()))
            //    { return; }
            //mdlString_toUors (&newValue, dimP->u.value.value.charPFormat);
            //if (newValue < fc_epsilon)   
            //    { return; }
            //needToReapplyAnnScale = dimStyle_unApplyAnnotationScale (activeDimStyle);
            //dimP->msgUnderstood = TRUE;
            //dimP->u.value.hookHandled  = TRUE;
            //dimStyleProperty = HeightWidthTextHookOverride::GetProperty (dimP->dialogItemP);

            //result = mdlDimStyle_getDoubleProp2 (activeDimStyle, &currValue, &isValueInherited, dimStyleProperty, GET_EFFECTIVE_DIMSTYLE_VALUE);
            //BeAssert (SUCCESS == result && "Dimension Style property getting error");
            //if (0 != activeDimStyle->textStyleId || !isValueInherited)
            //    {
            //    if (fabs (currValue - newValue) > fc_epsilon)
            //        {
            //        result = mdlDimStyle_setDoubleDirect (activeDimStyle, newValue, dimStyleProperty);
            //        BeAssert (SUCCESS == result && "Dimension Style property setting error");
            //        dimP->u.value.valueChanged = TRUE;
            //        if (creationComplete && GetLockValue())
            //            {
            //            result = mdlDimStyle_setDoubleDirect (activeDimStyle, newValue,
            //                dimStyleProperty == DIMSTYLE_PROP_Text_Height_DOUBLE ? DIMSTYLE_PROP_Text_Width_DOUBLE : DIMSTYLE_PROP_Text_Height_DOUBLE);
            //            BeAssert (SUCCESS == result && "Dimension Style property setting error");
            //            }
            //        }
            //    }
            //else
            //    {
            //    TextStyleProperty   textStyleProperty;

            //    textStyleProperty = GetPairedTextProperty (dimStyleProperty);
            //    currValue = TextStyleManager::GetActiveSetting<double> (textStyleProperty);
            //    if (fabs (currValue - newValue) > fc_epsilon)
            //        {
            //        TextStyleManager::GetActiveStyle()->SetProperty (textStyleProperty, newValue);
            //        dimP->u.value.valueChanged = TRUE;
            //        if (creationComplete && GetLockValue())
            //            {
            //            TextStyleManager::GetActiveStyle()->SetProperty (textStyleProperty == TextStyle_Height ? TextStyle_Width : TextStyle_Height, newValue);
            //            }
            //        }
            //    }
            //if (needToReapplyAnnScale)
            //    dimStyle_applyAnnotationScale (activeDimStyle);
            //return;
            }

        case DITEM_MESSAGE_SYNCHRONIZE:
            {
            // This is to prevent disabling text items
            //dimP->msgUnderstood = TRUE;
            return;
            }
        }
    }
