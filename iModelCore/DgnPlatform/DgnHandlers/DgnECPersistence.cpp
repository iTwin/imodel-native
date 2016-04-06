/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnECPersistence.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnECPersistence.h>
#include    <DgnPlatform/DgnECTypes.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE_EC 
USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::Initialize (ECDbCR ecDb) 
    {
    m_categoryClass = ecDb.Schemas().GetECClass("EditorCustomAttributes", "Category");
    BeAssert (m_categoryClass != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECStandardCategoryHelper::GetCategory (StandardCategory standard)
    {
    CategoriesByStandard::iterator it = m_categoriesByStandard.find (standard);
    if (it != m_categoriesByStandard.end())
        return it->second;
    POSTCONDITION (m_categoryClass != NULL, NULL);

    IECInstancePtr instance = m_categoryClass->GetDefaultStandaloneEnabler()->CreateInstance();
    SetStringValue (*instance, "Name", GetName (standard));
    SetStringValue (*instance, "DisplayLabel", GetDisplayLabel (standard).c_str());
    SetIntegerValue (*instance, "Priority", GetPriority (standard));
    SetBooleanValue (*instance, "Expand", GetDefaultExpand (standard));

    m_categoriesByStandard.Insert ((int) standard, instance);
    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECStandardCategoryHelper::GetDisplayLabel (StandardCategory standard)
    {
    L10N::StringId displayLabelMsgId;
    switch (standard)
        {
        case General:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_General();
        case Extended:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_Extended();
        case RawData:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_RawData();
        case Geometry:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_Geometry();
        case Groups:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_Groups();
        case Material:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_Material();
        case Relationships:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_Relationships();
        case Miscellaneous:
        default:
            displayLabelMsgId = DgnCoreL10N::ECPROPERTYCATEGORY_Miscellaneous();
        }

    return DgnCoreL10N::GetString(displayLabelMsgId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
int ECStandardCategoryHelper::GetPriority (StandardCategory standard)
    {
    switch (standard)
        {
        case General:
            return CategorySortPriorityVeryHigh;
        case Extended:
            return CategorySortPriorityMedium;
        case RawData:
            return CategorySortPriorityVeryLow;
        case Geometry:
            return CategorySortPriorityHigh;
        case Groups:
            return CategorySortPriorityMedium + 2000;
        case Material:
            return CategorySortPriorityMedium + 1000;
        case Relationships:
            return CategorySortPriorityMedium - 1000;
        default:
            return CategorySortPriorityVeryLow + 1000;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECStandardCategoryHelper::GetDefaultExpand (StandardCategory standard)
    {
    switch (standard)
        {
        // only General is opened by default.
        case General:
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECStandardCategoryHelper::GetName (StandardCategory standard)
    {
    switch (standard)
        {
        case General:
            return "General";
        case Extended:
            return "Extended";
        case RawData:
            return "RawData";
        case Geometry:
            return "Geometry";
        case Groups:
            return "Groups";
        case Material:
            return "Material";
        case Relationships:
            return "Relationships";
        }
    return "Miscellaneous";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetValue (IECInstanceR instance, Utf8CP name, ECValueCR ecValue)
    {
    ECObjectsStatus status = instance.SetValue (name, ecValue);
    if (ECObjectsStatus::Success != status)
        { BeAssert (false); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetStringValue (IECInstanceR instance, Utf8CP name, Utf8CP val)
    {
    ECValue ecValue;
    ecValue.SetUtf8CP (val);
    SetValue (instance, name, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetBooleanValue (IECInstanceR instance, Utf8CP name, bool val)
    {
    ECValue ecValue;
    ecValue.SetBoolean(val);
    SetValue (instance, name, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECStandardCategoryHelper::SetIntegerValue (IECInstanceR instance, Utf8CP name, int val)
    {
    ECValue ecValue;
    ecValue.SetInteger (val);
    SetValue (instance, name, ecValue);
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPropertyFormatterPtr DgnECPropertyFormatter::Create (DgnModelP dgnModel)
    {
    return new DgnECPropertyFormatter (dgnModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DgnECPropertyFormatter::DgnECPropertyFormatter (DgnModelP dgnModel) : m_dgnModel (dgnModel)
    {
    if (m_dgnModel != NULL)
        m_standardCategoryHelper.Initialize (dgnModel->GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnECPropertyFormatter::_FormattedStringFromECValue 
(
Utf8StringR strVal, 
ECValueCR ecValue, 
ECPropertyCR ecProperty, 
bool isArrayMember
) const
    {
    BeAssert (!isArrayMember || (isArrayMember && ecProperty.GetIsArray()));
    IDgnECTypeAdapterR ecTypeAdapter = isArrayMember ? IDgnECTypeAdapter::GetForArrayMember (*ecProperty.GetAsArrayProperty()) : IDgnECTypeAdapter::GetForProperty (ecProperty);

    IDgnECTypeAdapterContextPtr context = StandaloneTypeAdapterContext::Create (ecProperty, IDgnECTypeAdapterContext::COMPONENT_INDEX_None, m_dgnModel);

    bool status = ecTypeAdapter.ConvertToString(strVal, ecValue, *context);
    POSTCONDITION (status && "Could not format property to a string", status);

    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr DgnECPropertyFormatter::_GetPropertyCategory (ECN::ECPropertyCR ecProperty)
    {
    IECInstancePtr categoryAttr = ecProperty.GetCustomAttribute ("Category");

    // If no category is specified, use the standard "Miscellaneous" category
    if (categoryAttr.IsNull())
        return m_standardCategoryHelper.GetCategory (ECStandardCategoryHelper::Miscellaneous);

    // If a category "Name" is specified, use the user defined category
    // Note: We match the 8.11.9 logic here - even if the property has a "Standard" defined, we need
    // to use the "Name" if that's defined - see TFS#67022
    ECValue ecValue;
    if (ECObjectsStatus::Success == categoryAttr->GetValue (ecValue, "Name") && !ecValue.IsNull())
        return categoryAttr;

    // If no standard category is defined, use the user defined category. 
    if (ECObjectsStatus::Success != categoryAttr->GetValue(ecValue, "Standard") || ecValue.IsNull())
        return categoryAttr;

    // Use one of the standard categories
    ECStandardCategoryHelper::StandardCategory standard = (ECStandardCategoryHelper::StandardCategory) ecValue.GetInteger();
    return m_standardCategoryHelper.GetCategory (standard);
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// static
BentleyStatus DgnECPersistence::GetElementInfo(JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, DgnElementId elementId, DgnDbCR dgndb)
    {
    DgnElementCPtr element = dgndb.Elements().GetElement(elementId);
    if (!element.IsValid())
        {
        BeAssert(false && "Element not found");
        return ERROR;
        }

    DgnECPropertyFormatterPtr propertyFormatter = DgnECPropertyFormatter::Create(element->GetModel().get());
    JsonECSqlSelectAdapter::FormatOptions formatOptions (ECValueFormat::FormattedStrings, propertyFormatter.get());

    JsonReader jsonReader(dgndb, element->GetElementClassId());
    return jsonReader.Read (jsonInstances, jsonDisplayInfo, (ECInstanceId) elementId.GetValue(), formatOptions);
    }

