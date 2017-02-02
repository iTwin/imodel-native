/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HandlerTypeInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Trefz   01/04
 +---------------+---------------+---------------+---------------+---------------+------*/
static bool isBlankString(Utf8CP testStr)
    {
    if (!testStr || strlen(testStr) < 1)
        return true;

    Utf8CP p = testStr;

    while (*p)
        {
        if (!isspace(*p))
            return true;
        p++;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getCategoryDisplayName(Utf8StringR displayNameStr, DgnCategory const& category, Utf8CP displayFormat)
    {
    for (size_t formatIndex = 0; formatIndex < strlen(displayFormat); formatIndex++)
        {
        Utf8Char formatChar = displayFormat[formatIndex];

        switch (formatChar)
            {
            case LEVEL_NAME_DISPLAY_FORMAT:
                {
                displayNameStr.append(category.GetCode().GetValueCP()? category.GetCode().GetValueCP(): ""); // string conversion
                break;
                }

            case LEVEL_DESCRIPTION_DISPLAY_FORMAT:
                {
                if (isBlankString(category.GetDescription()))
                    displayNameStr.append("");//DgnCoreL10N::GetString(DgnCoreL10N::IDS_CategoryDescriptionNone).c_str());
                else
                    displayNameStr.append(category.GetDescription()); // string conversion
                break;
                }

            case LEVEL_ID_DISPLAY_FORMAT:
                {
                Utf8Char     tmpStr[64];

                BeStringUtilities::Snprintf(tmpStr, "%lld", category.GetCategoryId().GetValue());
                displayNameStr.append(tmpStr);
                break;
                }

            default:
                {
                // NOTE: Assumed that non-code characters are delimiters...
                displayNameStr.append(formatChar, 1);
                break;
                }
            }
        }

    if (displayNameStr.empty())
        displayNameStr.assign(category.GetCode().GetValueCP()? category.GetCode().GetValueCP(): ""); // string conversion

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getCategoryString(DgnCategoryId categoryId, DgnDbR db)
    {
    DgnCategoryCPtr category = DgnCategory::Get(db, categoryId);
    if (!category.IsValid())
        return nullptr;

    Utf8String displayName;
    if (SUCCESS != getCategoryDisplayName(displayName, *category, LEVEL_NAME_DISPLAY_FORMAT_STRING))
        return nullptr;

    return  DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Category()) + displayName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HitDetail::_GetInfoString(Utf8CP delimiter) const
    {
    if (m_hitDescription.IsValid())
        return m_hitDescription->GetDescription();

    auto el = GetElement();
    if (!el.IsValid())
        return nullptr;

    auto geom = el->ToGeometrySource();
    if (nullptr == geom)
        return nullptr;

    return  el->GetDisplayLabel() + delimiter + 
            getCategoryString(geom->GetCategoryId(), el->GetDgnDb()) + delimiter +
            DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Model()) + el->GetModel()->GetName();
    }
