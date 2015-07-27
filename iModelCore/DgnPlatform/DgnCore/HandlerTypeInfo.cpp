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
static bool     isBlankString(Utf8CP testStr)
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
static BentleyStatus getCategoryDisplayName(Utf8StringR displayNameStr, DgnCategories::Category const& category, Utf8CP displayFormat)
    {
    BeAssert(category.IsValid());

    for (size_t formatIndex = 0; formatIndex < strlen(displayFormat); formatIndex++)
        {
        Utf8Char formatChar = displayFormat[formatIndex];

        switch (formatChar)
            {
            case LEVEL_NAME_DISPLAY_FORMAT:
                {
                displayNameStr.append(category.GetLabel()? category.GetLabel(): ""); // string conversion
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
        displayNameStr.assign(category.GetLabel()? category.GetLabel(): ""); // string conversion

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void getCategoryString(Utf8StringR categoryStr, DgnElementCR element)
    {
    DgnCategories::Category const& category = element.GetDgnDb().Categories().Query(element.GetCategoryId());

    if (!category.IsValid())
        return;

    Utf8String displayFormat;

#ifdef WIP_CFGVAR // MS_LEVEL_DISPLAY_FORMAT
    if (SUCCESS != ConfigurationManager::GetVariable(displayFormat, L"MS_LEVEL_DISPLAY_FORMAT") || displayFormat.size() == 0)
#endif
        displayFormat = LEVEL_NAME_DISPLAY_FORMAT_STRING;

    Utf8String     displayName;

    if (SUCCESS != getCategoryDisplayName(displayName, category, displayFormat.c_str()))
        return;

    categoryStr.assign("");//DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Category).c_str());
    categoryStr.append(displayName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/10
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnPlatformLib::Host::GraphicsAdmin::_GetInfoString(HitDetailCP hit, Utf8StringR pathDescr, Utf8CP delimiter) const
    {
    if (nullptr == hit)
        return;

    GeometricElementCPtr element = hit->GetElement();

    if (!element.IsValid())
        {
        IElemTopologyCP elemTopo = hit->GetElemTopology();
        ITransientGeometryHandlerP transientHandler = (nullptr != elemTopo ? elemTopo->_GetTransientGeometryHandler() : nullptr);

        if (nullptr != transientHandler)
            transientHandler->_GetTransientInfoString(*hit, pathDescr, delimiter);
        return;
        }

    pathDescr = element->GetCode();

    Utf8String categoryStr, modelStr;

    modelStr.assign(DgnCoreL10N::GetString(DgnCoreL10N::DISPLAY_INFO_MessageID_Model()).c_str()).append(element->GetModel()->GetModelName());
    getCategoryString(categoryStr, *element);

    pathDescr.append(delimiter).append(modelStr.c_str());
    pathDescr.append(delimiter).append(categoryStr.c_str());

#if defined (NEEDSWORK_ELEMENT_HANDLER)
    dHandler->GetPathDescription(eh, pathDescr, path, categoryStr.c_str(), modelStr.c_str(), delimiter);
#endif
    }
