/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ContentFieldEditors.h>
#include "PropertyInfoStore.h"
#include "LoggingHelper.h"

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            02/2018
+---------------+---------------+---------------+---------------+-----------+------*/
bool DisplayInfo::ShouldDisplay(ECClassCR, ECPropertyCR prop) const
    {
    // first, look for display info specified for some property
    auto iter = m_propertyDisplayInfos.find(PropertyDisplayInfo(prop.GetName()));
    if (m_propertyDisplayInfos.end() != iter)
        return iter->IsDisplayed();

    // then, find a default class display info with the highest priority
    DefaultDisplayInfo const* matchingDisplayInfo = nullptr;
    for (auto pair : m_defaultDisplayInfos)
        {
        if (nullptr != pair.first && !pair.first->Is(&prop.GetClass()))
            continue;

        if (nullptr == matchingDisplayInfo || pair.second.GetPriority() > matchingDisplayInfo->GetPriority())
            matchingDisplayInfo = &pair.second;
        }
    // if found, use its display flag
    if (nullptr != matchingDisplayInfo)
        return matchingDisplayInfo->ShouldDisplay();
    
    // by default - display
    return true;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void DisplayInfo::Merge(DisplayInfo const& source)
    {
    for (PropertyDisplayInfo const& info : source.m_propertyDisplayInfos)
        {
        auto iter = m_propertyDisplayInfos.find(info);
        if (m_propertyDisplayInfos.end() == iter)
            m_propertyDisplayInfos.insert(info);
        else if (iter->GetPriority() < info.GetPriority())
            m_propertyDisplayInfos.insert(m_propertyDisplayInfos.erase(iter), info);
        }

    for (auto pair : source.m_defaultDisplayInfos)
        {
        ECClassCP key = pair.first;
        DefaultDisplayInfo const& info = pair.second;

        auto iter = m_defaultDisplayInfos.find(key);
        if (m_defaultDisplayInfos.end() == iter)
            m_defaultDisplayInfos.Insert(key, info);
        else if (iter->second.GetPriority() < info.GetPriority())
            {
            m_defaultDisplayInfos.erase(iter);
            m_defaultDisplayInfos.Insert(key, info);
            }
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::CollectPropertiesDisplayRules(ECClassCP ecClass, PropertiesDisplaySpecificationCR spec)
    {
    DisplayInfo info;
    if (spec.GetPropertyNames().Equals("*"))
        {
        info.AddDefaultDisplayInfo(ecClass, DefaultDisplayInfo(spec.IsDisplayed(), spec.GetPriority()));
        }
    else
        {
        bvector<Utf8String> propertyNamesVec;
        BeStringUtilities::Split(spec.GetPropertyNames().c_str(), ",", propertyNamesVec);
        for (Utf8StringR propertyName : propertyNamesVec)
            {
            propertyName.Trim();
            info.GetPropertyDisplayInfos().insert(PropertyDisplayInfo(propertyName, spec.GetPriority(), spec.IsDisplayed()));
            }
        }

    auto iter = m_perClassPropertyDisplayInfos.find(ecClass);
    if (m_perClassPropertyDisplayInfos.end() == iter)
        m_perClassPropertyDisplayInfos[ecClass] = info;
    else
        iter->second.Merge(info);
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::InitPropertyDisplayInfos(ContentSpecificationCP specification, ContentModifierList const& contentModifiers)
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
                //BeAssert(false);
                continue;
                }
            CollectPropertiesDisplayRules(ecClass, *displaySpec);
            }
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
DisplayInfo const& PropertyInfoStore::GetDisplayInfo(ECClassCR ecClass) const
    {
    auto iter = m_aggregatedPropertyDisplayInfos.find(&ecClass);
    if (m_aggregatedPropertyDisplayInfos.end() == iter)
        {
        DisplayInfo info;

        // merge in display infos that apply for any class (defined at specification level)
        auto anyClassIter = m_perClassPropertyDisplayInfos.find(nullptr);
        if (m_perClassPropertyDisplayInfos.end() != anyClassIter)
            info.Merge(anyClassIter->second);

        // merge in display infos that apply for supplied class (defined as content modifier)
        auto perClassIter = m_perClassPropertyDisplayInfos.find(&ecClass);
        if (m_perClassPropertyDisplayInfos.end() != perClassIter)
            info.Merge(perClassIter->second);

        // find if there's at least one spec that requires property to be displayed
        bool hasDisplayedPropertySpec = false;
        for (PropertyDisplayInfo const& propertyDisplayInfo : info.GetPropertyDisplayInfos())
            {
            if (propertyDisplayInfo.IsDisplayed())
                {
                hasDisplayedPropertySpec = true;
                break;
                }
            }
        if (hasDisplayedPropertySpec)
            {
            // if there's at least one spec requiring property display, it means we should hide all 
            // others - insert hiding display infos with low priority so they don't override any
            // explicitly specified infos
            info.AddDefaultDisplayInfo(&ecClass, DefaultDisplayInfo(false, 0));
            for (ECClassCP baseClass : ecClass.GetBaseClasses())
                info.AddDefaultDisplayInfo(baseClass, DefaultDisplayInfo(false, 0));

            }

        // merge in display infos of base class properties
        for (ECClassCP base : ecClass.GetBaseClasses())
            info.Merge(GetDisplayInfo(*base));


        iter = m_aggregatedPropertyDisplayInfos.Insert(&ecClass, info).first;
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
                //BeAssert(false);
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
    InitPropertyDisplayInfos(spec, ruleset.GetContentModifierRules());
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

    DisplayInfo const& displayInfo = GetDisplayInfo(ecClass);
    return displayInfo.ShouldDisplay(ecClass, prop);
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