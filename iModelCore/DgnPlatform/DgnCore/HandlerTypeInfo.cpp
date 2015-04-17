/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HandlerTypeInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define DGN_TABLE_LEVEL_FOR_MODEL(m)  (m)->GetDgnDb().Categories()
enum
    {
    MAX_INFO_STRING_LEN = 4096,
    };

/*=================================================================================**//**
  @bsiclass                                                     Brien.Bastings  03/10
+===============+===============+===============+===============+===============+======*/
struct PathPropertyQuery : public IQueryProperties
{
DgnCategoryId m_category;

PathPropertyQuery () {m_category.Invalidate();}

DgnCategoryId GetEffectiveCategoryId () {return m_category;}

virtual ElementProperties _GetQueryPropertiesMask () override {return ELEMENT_PROPERTY_Category;}

virtual void    _EachCategoryCallback (EachCategoryArg& arg) override
    {
    // Get effective base category id for this path...
    if (0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return;

    if (0 != ((PROPSCALLBACK_FLAGS_ElementIgnoresID | PROPSCALLBACK_FLAGS_UndisplayedID) & arg.GetPropertyFlags ()))
        return;

    m_category = arg.GetEffectiveValue ();
    }

}; // PathPropertyQuery

/**
 * Category Display Name Format Defines
 */
#define LEVEL_NAME_DISPLAY_FORMAT_STRING            "N"
#define LEVEL_CODE_DISPLAY_FORMAT_STRING            "C"
#define LEVEL_DESCRIPTION_DISPLAY_FORMAT_STRING     "D"

/**
 * Category Display Name Format Values
 */
#define LEVEL_NAME_DISPLAY_FORMAT                   'N'
#define LEVEL_CODE_DISPLAY_FORMAT                   'C'
#define LEVEL_DESCRIPTION_DISPLAY_FORMAT            'D'
#define LEVEL_ID_DISPLAY_FORMAT                     'I'

#if defined (NEEDSWORK_DGNITEM)
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
static BentleyStatus getCategoryDisplayName (Utf8StringR displayNameStr, DgnCategories::Category const& category, Utf8CP displayFormat)
    {
    BeAssert (category.IsValid ());

    for (size_t formatIndex = 0; formatIndex < strlen (displayFormat); formatIndex++)
        {
        Utf8Char formatChar = displayFormat[formatIndex];

        switch (formatChar)
            {
            case LEVEL_NAME_DISPLAY_FORMAT:
                {
                displayNameStr.append(category.GetName()? category.GetName(): ""); // string conversion
                break;
                }

            case LEVEL_DESCRIPTION_DISPLAY_FORMAT:
                {
                if (isBlankString (category.GetDescription ()))
                    displayNameStr.append(DgnCoreL10N::GetString(DgnCoreL10N::IDS_CategoryDescriptionNone).c_str());
                else
                    displayNameStr.append (category.GetDescription ()); // string conversion
                break;
                }

            case LEVEL_ID_DISPLAY_FORMAT:
                {
                Utf8Char     tmpStr[64];

                BeStringUtilities::Snprintf (tmpStr, "%d", category.GetCategoryId());
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
        displayNameStr.assign(category.GetName()? category.GetName(): ""); // string conversion

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void getCategoryString (Utf8StringR categoryStr, DisplayPathCP path)
    {
    PathPropertyQuery   queryObj;

    // NOTE: Should really always have a hit path from locate...
    if (path->GetPathType () >= DisplayPathType::Hit)
        PropertyContext::QueryPathProperties (toHitPath (path), &queryObj);
    else
        PropertyContext::QueryElementProperties (ElementHandle (path->GetTailElem ()), &queryObj);

    DgnCategories::Category const& category = DGN_TABLE_LEVEL_FOR_MODEL(path->GetRoot()).QueryCategoryById (queryObj.GetEffectiveCategoryId ());

    if (!category.IsValid ())
        return;

    Utf8String displayFormat;

#ifdef WIP_CFGVAR // MS_LEVEL_DISPLAY_FORMAT
    if (SUCCESS != ConfigurationManager::GetVariable (displayFormat, L"MS_LEVEL_DISPLAY_FORMAT") || displayFormat.size () == 0)
#endif
        displayFormat = LEVEL_NAME_DISPLAY_FORMAT_STRING;

    Utf8String     displayName;

    if (SUCCESS != getCategoryDisplayName (displayName, category, displayFormat.c_str ()))
        return;

    categoryStr.assign(DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Category).c_str());
    categoryStr.append (displayName);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::GraphicsAdmin::_GetInfoString (DisplayPathCP path, Utf8StringR pathDescr, Utf8CP delimiter) const
    {
#if defined (NEEDSWORK_DGNITEM)
    ElementHandle   eh (path->GetHeadElem ());
    DisplayHandlerP dHandler;

    if (NULL == (dHandler = eh.GetDisplayHandler ()))
        {
        eh.GetHandler().GetDescription (eh, pathDescr, MAX_INFO_STRING_LEN);
        return;
        }

    Utf8String     categoryStr, modelStr;

#if defined (WIP_V10_TRANSIENTS)
    // If description linkage has been added to transient, use it...
    if (path->InTransientModel () && mdlElement_attributePresent (eh.GetGraphicsCP (), LINKAGEID_String, NULL))
        {
        WCharP    scratch = (WCharP) _alloca (MAX_LINKAGE_STRING_BYTES);

        if (SUCCESS == mdlLinkage_extractNamedStringLinkageByIndex (scratch, MAX_LINKAGE_STRING_LENGTH, eh.GetGraphicsCP (), STRING_LINKAGE_KEY_Description, 0))
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
            modelStr.assign(DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Model).c_str()).append (dgnModel->GetModelName());

        }

    getCategoryString (categoryStr, path);

    dHandler->GetPathDescription (eh, pathDescr, path, categoryStr.c_str(), modelStr.c_str(), delimiter);
#endif
    }
