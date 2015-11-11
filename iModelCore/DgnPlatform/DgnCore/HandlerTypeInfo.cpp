/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HandlerTypeInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
static void getCategoryString(Utf8StringR categoryStr, DgnCategoryId categoryId, DgnDbR dgnDb)
    {
    DgnCategoryCPtr category = DgnCategory::QueryCategory(categoryId, dgnDb);

    if (!category.IsValid())
        return;

    Utf8String displayFormat;

#ifdef WIP_CFGVAR // MS_LEVEL_DISPLAY_FORMAT
    if (SUCCESS != ConfigurationManager::GetVariable(displayFormat, L"MS_LEVEL_DISPLAY_FORMAT") || displayFormat.size() == 0)
#endif
        displayFormat = LEVEL_NAME_DISPLAY_FORMAT_STRING;

    Utf8String     displayName;

    if (SUCCESS != getCategoryDisplayName(displayName, *category, displayFormat.c_str()))
        return;

    categoryStr.assign(DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Category()).c_str());
    categoryStr.append(displayName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometrySource::_GetInfoString(HitDetailCR hit, Utf8StringR descr, Utf8CP delimiter) const
    {
    DgnElementCP el = ToElement();
    Utf8String   categoryStr, modelStr;

    getCategoryString(categoryStr, GetCategoryId(), GetSourceDgnDb());

    if (nullptr == el)
        {
        descr = categoryStr.c_str();
        return;
        }

    modelStr.assign(DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Model()).c_str()).append(el->GetModel()->GetCode().GetValue());

    descr = el->GetCode().GetValue();
    descr.append(delimiter).append(modelStr.c_str());
    descr.append(delimiter).append(categoryStr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void HitDetail::_GetInfoString(Utf8StringR descr, Utf8CP delimiter) const
    {
    // NOT_NOW_GEOMETRY_SOURCE - ITransientGeometryHandler should provide a GeometrySource...
    DgnElementCPtr   element = GetElement();
    GeometrySourceCP geom = (element.IsValid() ? element->ToGeometrySource() : nullptr);

    if (nullptr == geom)
        {
        IElemTopologyCP elemTopo = GetElemTopology();
        ITransientGeometryHandlerP transientHandler = (nullptr != elemTopo ? elemTopo->_GetTransientGeometryHandler() : nullptr);

        if (nullptr != transientHandler)
            transientHandler->_GetTransientInfoString(*this, descr, delimiter);
        return;
        }

    geom->GetInfoString(*this, descr, delimiter);
    }
