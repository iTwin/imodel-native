/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PropertyInfoStore.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PropertyInfoStore.h"
#include "ContentFieldEditors.h"
#include "LoggingHelper.h"

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::InsertPropertiesDisplayInfo(ECClassCP ecClass, bset<PropertiesDisplayInfo> const& source, bset<PropertiesDisplayInfo>& target)
    {
    for (PropertiesDisplayInfo const& info : source)
        {
        auto iter = target.find(info);
        if (target.end() == iter)
            {
            target.insert(info);
            continue;
            }
        if (iter->GetPriority() < info.GetPriority())
            target.insert(target.erase(iter), info);
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::CollectPropertiesDisplayRules(ECClassCP ecClass, PropertiesDisplaySpecificationCR spec)
    {
    bset<PropertiesDisplayInfo> propertiesInfos;
    bvector<Utf8String> propertyNamesVec;
    BeStringUtilities::Split(spec.GetPropertyNames().c_str(), ",", propertyNamesVec);
    for (Utf8StringR propertyName : propertyNamesVec)
        {
        propertyName.Trim();
        propertiesInfos.insert(PropertiesDisplayInfo(propertyName, spec.GetPriority(), spec.IsDisplayed()));
        }

    auto iter = m_perClassPropertyDisplayInfos.find(ecClass);
    if (m_perClassPropertyDisplayInfos.end() == iter)
        m_perClassPropertyDisplayInfos[ecClass] = propertiesInfos;
    else
        InsertPropertiesDisplayInfo(ecClass, propertiesInfos, iter->second);
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::InitPropertiesDisplayInfo(ContentSpecificationCP specification, ContentModifierList const& contentModifiers)
    {
    if (nullptr != specification)
        {
        for (PropertiesDisplaySpecificationCP displaySpec : specification->GetPropertiesDisplaySpecifications())
            CollectPropertiesDisplayRules(nullptr, *displaySpec);
        }

    for (ContentModifierCP modifier : contentModifiers)
        {
        for (PropertiesDisplaySpecificationCP displaySpec : modifier->GetPropertiesDisplaySpecifications())
            {
            ECClassCP ecClass = m_schemaHelper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                BeAssert(false);
                continue;
                }
            CollectPropertiesDisplayRules(ecClass, *displaySpec);
            }
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
bset<PropertiesDisplayInfo> const& PropertyInfoStore::GetPropertiesDisplayInfo(ECClassCR ecClass) const
    {
    auto iter = m_aggregatedPropertyDisplayInfos.find(&ecClass);
    if (m_aggregatedPropertyDisplayInfos.end() == iter)
        {
        bset<PropertiesDisplayInfo> properties;

        auto anyClassIter = m_perClassPropertyDisplayInfos.find(nullptr);
        if (m_perClassPropertyDisplayInfos.end() != anyClassIter)
            InsertPropertiesDisplayInfo(&ecClass, anyClassIter->second, properties);

        auto perClassIter = m_perClassPropertyDisplayInfos.find(&ecClass);
        if (m_perClassPropertyDisplayInfos.end() != perClassIter)
            InsertPropertiesDisplayInfo(&ecClass, perClassIter->second, properties);

        bool hasDisplayedPropertySpec = false;
        for (PropertiesDisplayInfo const& info : properties)
            {
            if (info.IsDisplayed())
                {
                hasDisplayedPropertySpec = true;
                break;
                }
            }
        if (hasDisplayedPropertySpec)
            {
            for (ECPropertyCP prop : ecClass.GetProperties())
                {
                PropertiesDisplayInfo info(prop->GetName(), 0, false);
                if (properties.end() == properties.find(info))
                    properties.insert(info);
                }
            }

        for (ECClassCP base : ecClass.GetBaseClasses())
            InsertPropertiesDisplayInfo(base, GetPropertiesDisplayInfo(*base), properties);


        iter = m_aggregatedPropertyDisplayInfos.Insert(&ecClass, properties).first;
        }
    return iter->second;
    }
    
/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2017
+---------------+---------------+---------------+---------------+-----------+------*/
ContentFieldEditor const* PropertyInfoStore::CreateEditor(PropertyEditorsSpecificationCR spec)
    {
    ContentFieldEditor* editor = new ContentFieldEditor(spec.GetEditorName());
    EditorParamsBuilder paramsBuilder(*editor);
    for (PropertyEditorParametersSpecificationCP paramsSpec : spec.GetParameters())
        paramsSpec->Accept(paramsBuilder);
    return editor;
    }
    
/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::InitPropertyEditors(ContentSpecificationCP specification, ContentModifierList const& contentModifiers)
    {
    if (nullptr != specification)
        {
        for (PropertyEditorsSpecificationCP editorSpec : specification->GetPropertyEditors())
            {
            bmap<Utf8String, ContentFieldEditor const*>& editors = m_propertyEditors[nullptr];
            editors[editorSpec->GetPropertyName()] = CreateEditor(*editorSpec);
            }
        }

    for (ContentModifierCP modifier : contentModifiers)
        {
        for (PropertyEditorsSpecificationCP editorSpec : modifier->GetPropertyEditors())
            {
            ECClassCP ecClass = m_schemaHelper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                BeAssert(false);
                continue;
                }
            bmap<Utf8String, ContentFieldEditor const*>& editors = m_propertyEditors[ecClass];
            editors[editorSpec->GetPropertyName()] = CreateEditor(*editorSpec);
            }
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2017
+---------------+---------------+---------------+---------------+-----------+------*/
PropertyInfoStore::PropertyInfoStore(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset, ContentSpecificationCP spec)
    : m_schemaHelper(helper)
    {
    InitPropertiesDisplayInfo(spec, ruleset.GetContentModifierRules());
    InitPropertyEditors(spec, ruleset.GetContentModifierRules());
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
bool PropertyInfoStore::ShouldDisplay(ECPropertyCR prop, ECClassCR ecClass) const
    {
    // schema custom attribute overrides everything
    IECInstancePtr hideCustomAttribute = prop.GetCustomAttribute("CoreCustomAttributes", "HiddenProperty");
    if (hideCustomAttribute.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == hideCustomAttribute->GetValue(value, "Show") && (value.IsNull() || (value.IsBoolean() && false == value.GetBoolean())))
            return false;
        }

    bset<PropertiesDisplayInfo> const& properties = GetPropertiesDisplayInfo(ecClass);
    if (properties.empty())
        return true;

    auto iter = properties.find(PropertiesDisplayInfo(prop.GetName()));
    if (properties.end() != iter)
        return iter->IsDisplayed();

    return true;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
ContentFieldEditor const* PropertyInfoStore::GetPropertyEditor(ECPropertyCR ecProperty, ECClassCR ecClass, bool searchForAnyClassEditor) const
    {
    Utf8StringCR propertyName = ecProperty.GetName();

    // is there an editor that matches the provided class?
    auto specificClassIter = m_propertyEditors.find(&ecClass);
    if (m_propertyEditors.end() != specificClassIter)
        {
        auto propertyIter = specificClassIter->second.find(propertyName);
        if (specificClassIter->second.end() != propertyIter)
            return propertyIter->second;
        }

    if (searchForAnyClassEditor)
        {
        // is there an editor that matches any class?
        auto anyClassIter = m_propertyEditors.find(nullptr);
        if (m_propertyEditors.end() != anyClassIter)
            {
            auto propertyIter = anyClassIter->second.find(propertyName);
            if (anyClassIter->second.end() != propertyIter)
                return propertyIter->second;
            }
        }

    // it's possible that one of the base classes of the provided class has an editor - check that
    for (ECClassCP base : ecClass.GetBaseClasses())
        {
        ContentFieldEditor const* baseEditor = GetPropertyEditor(ecProperty, *base, false);
        if (nullptr != baseEditor)
            return baseEditor;
        }

    return nullptr;
    }