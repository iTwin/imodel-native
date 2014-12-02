/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HandlerTypeInfo.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

enum
    {
    MAX_INFO_STRING_LEN = 4096,
    };

/*=================================================================================**//**
  @bsiclass                                                     Brien.Bastings  03/10
+===============+===============+===============+===============+===============+======*/
struct PathPropertyQuery : public IQueryProperties
{
LevelId         m_level;

PathPropertyQuery () {m_level.Invalidate();}

LevelId         GetEffectiveLevelId () {return m_level;}

virtual ElementProperties _GetQueryPropertiesMask () override {return ELEMENT_PROPERTY_Level;}

virtual void    _EachLevelCallback (EachLevelArg& arg) override
    {
    // Get effective base level id for this path...
    if (0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return;

    if (0 != ((PROPSCALLBACK_FLAGS_ElementIgnoresID | PROPSCALLBACK_FLAGS_UndisplayedID) & arg.GetPropertyFlags ()))
        return;

    m_level = arg.GetEffectiveValue ();
    }

}; // PathPropertyQuery

/**
 * Level Display Name Format Defines
 */
#define LEVEL_NAME_DISPLAY_FORMAT_STRING            L"N"
#define LEVEL_CODE_DISPLAY_FORMAT_STRING            L"C"
#define LEVEL_DESCRIPTION_DISPLAY_FORMAT_STRING     L"D"

/**
 * Level Display Name Format Values
 */
#define LEVEL_NAME_DISPLAY_FORMAT                   L'N'
#define LEVEL_CODE_DISPLAY_FORMAT                   L'C'
#define LEVEL_DESCRIPTION_DISPLAY_FORMAT            L'D'
#define LEVEL_ID_DISPLAY_FORMAT                     L'I'

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Trefz   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isBlankString (Utf8CP testStr)
    {
    if (!testStr || strlen (testStr) < 1)
        return true;

    Utf8CP p = testStr;

    while (*p)
        {
        if (!isspace (*p))
            return true;
        p++;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getLevelDisplayName (WStringR displayNameStr, DgnLevels::Level const& level, WCharCP displayFormat)
    {
    BeAssert (level.IsValid ());

    for (size_t formatIndex = 0; formatIndex < wcslen (displayFormat); formatIndex++)
        {
        WChar formatChar = displayFormat[formatIndex];

        switch (formatChar)
            {
            case LEVEL_NAME_DISPLAY_FORMAT:
                {
                displayNameStr.AppendUtf8 (level.GetName()? level.GetName(): ""); // string conversion
                break;
                }

            case LEVEL_DESCRIPTION_DISPLAY_FORMAT:
                {
                if (isBlankString (level.GetDescription ()))
                    displayNameStr.AppendUtf8 (DgnCoreL10N::GetString(DgnCoreL10N::IDS_LevelDescriptionNone).c_str());
                else
                    displayNameStr.AppendUtf8 (level.GetDescription ()); // string conversion
                break;
                }

            case LEVEL_ID_DISPLAY_FORMAT:
                {
                WChar     tmpStr[64];

                BeStringUtilities::Snwprintf (tmpStr, L"%d", level.GetLevelId ());
                displayNameStr.append (tmpStr);
                break;
                }

            default:
                {
                // NOTE: Assumed that non-code characters are delimiters...
                displayNameStr.append (formatChar, 1);
                break;
                }
            }
        }

    if (displayNameStr.empty ())
        displayNameStr.AssignUtf8 (level.GetName()? level.GetName(): ""); // string conversion

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void getLevelString (WStringR levelStr, DisplayPathCP path)
    {
    PathPropertyQuery   queryObj;

    // NOTE: Should really always have a hit path from locate...
    if (path->GetPathType () >= DisplayPathType::Hit)
        PropertyContext::QueryPathProperties (toHitPath (path), &queryObj);
    else
        PropertyContext::QueryElementProperties (ElementHandle (path->GetTailElem ()), &queryObj);

    DgnLevels::Level const& level = DGN_TABLE_LEVEL_FOR_MODEL(path->GetRoot()).QueryLevelById (queryObj.GetEffectiveLevelId ());

    if (!level.IsValid ())
        return;

    WString displayFormat;

#ifdef WIP_CFGVAR // MS_LEVEL_DISPLAY_FORMAT
    if (SUCCESS != ConfigurationManager::GetVariable (displayFormat, L"MS_LEVEL_DISPLAY_FORMAT") || displayFormat.size () == 0)
#endif
        displayFormat = LEVEL_NAME_DISPLAY_FORMAT_STRING;

    WString     displayName;

    if (SUCCESS != getLevelDisplayName (displayName, level, displayFormat.c_str ()))
        return;

    levelStr.AssignUtf8 (DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Level).c_str());
    levelStr.append (displayName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void getGroupString (WStringR groupStr, ElementHandleCR eh, bool doNamedGroups)
    {
#ifdef DGN_GGROUP_REORG_WIP
    int         ggroup = eh.GetElementCP ()->hdr.dhdr.grphgrp;
    WString     tmpStr;

    // NOTE: Used to check tcb->fbfdcn.gglk and only report GG id when enabled...
    if (ggroup)
        {
        WChar ggVal[100];

        tmpStr.AssignUtf8 (dgnCoreGetMessage(DgnCoreL10N::DISPLAY_INFO_MessageID_GG).c_str());
        BeStringUtilities::Snwprintf (ggVal, _countof(ggVal), L"%d", ggroup);
        tmpStr.append (L"{").append (ggVal).append (L"}");
        }

    if (doNamedGroups)
        getNamedGroupString (tmpStr, eh);

    if (tmpStr.empty ())
        return;

    groupStr.AssignUtf8 (dgnCoreGetMessage(DgnCoreL10N::DISPLAY_INFO_MessageID_Groups).c_str()).append (tmpStr);
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::GraphicsAdmin::_GetInfoString (DisplayPathCP path, WStringR pathDescr, WCharCP delimiter) const
    {
    ElementHandle   eh (path->GetHeadElem ());
    DisplayHandlerP dHandler;

    if (NULL == (dHandler = eh.GetDisplayHandler ()))
        {
        eh.GetHandler().GetDescription (eh, pathDescr, MAX_INFO_STRING_LEN);
        return;
        }

    WString     levelStr, modelStr, groupStr;

#if defined (WIP_V10_TRANSIENTS)
    // If description linkage has been added to transient, use it...
    if (path->InTransientModel () && mdlElement_attributePresent (eh.GetElementCP (), LINKAGEID_String, NULL))
        {
        WCharP    scratch = (WCharP) _alloca (MAX_LINKAGE_STRING_BYTES);

        if (SUCCESS == mdlLinkage_extractNamedStringLinkageByIndex (scratch, MAX_LINKAGE_STRING_LENGTH, eh.GetElementCP (), STRING_LINKAGE_KEY_Description, 0))
            {
            pathDescr.assign (scratch);
            return;
            }

        modelStr.assign (ConfigurationManager::GetString (DGNCORE_STRINGS, DgnCoreL10N::DISPLAY_INFO_MessageID_Transient));
        }
    else
#endif
        {
        DgnModelP dgnModel = eh.GetDgnModelP();
        if (dgnModel)
            modelStr.AssignUtf8 (DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Model).c_str()).append (WString(dgnModel->GetModelName(), true).c_str());

        getGroupString (groupStr, eh, true);
        }

    getLevelString (levelStr, path);

    dHandler->GetPathDescription (eh, pathDescr, path, levelStr.c_str(), modelStr.c_str(), groupStr.c_str(), delimiter);
    }
