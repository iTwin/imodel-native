/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ContentFieldEditors.h>
#include "PropertyInfoStore.h"
#include "LoggingHelper.h"

#define SET_PRIORITZED_NULLABLE_MEMBER(member_name) \
    if (source.member_name.IsValid()) \
        { \
        if (member_name.IsNull() || member_name.Value().priority < source.member_name.Value().priority) \
            member_name = source.member_name; \
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassPropertyOverridesInfo::Overrides::Merge(Overrides const& source)
    {
    SET_PRIORITZED_NULLABLE_MEMBER(m_display);
    SET_PRIORITZED_NULLABLE_MEMBER(m_labelOverride);
    SET_PRIORITZED_NULLABLE_MEMBER(m_category);
    SET_PRIORITZED_NULLABLE_MEMBER(m_editor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassPropertyOverridesInfo::Merge(ClassPropertyOverridesInfo const& source)
    {
    for (auto const& entry : source.m_propertyOverrides)
        {
        auto iter = m_propertyOverrides.find(entry.first);
        if (m_propertyOverrides.end() == iter)
            m_propertyOverrides[entry.first] = entry.second;
        else
            iter->second.Merge(entry.second);
        }

    for (auto const& entry : source.m_defaultClassPropertyOverrides)
        {
        auto iter = m_defaultClassPropertyOverrides.find(entry.first);
        if (m_defaultClassPropertyOverrides.end() == iter)
            m_defaultClassPropertyOverrides[entry.first] = entry.second;
        else
            iter->second.Merge(entry.second);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TValue>
Nullable<ClassPropertyOverridesInfo::PrioritizedValue<TValue>> const& ClassPropertyOverridesInfo::GetOverrides(ECPropertyCR prop, Nullable<PrioritizedValue<TValue>> const& (*valuePicker)(Overrides const&)) const
    {
    // first, look for overrides specified specifically for some property
    auto iter = m_propertyOverrides.find(prop.GetName());
    if (m_propertyOverrides.end() != iter)
        return valuePicker(iter->second);

    // then, attempt to find a default class override with the highest priority
    Nullable<PrioritizedValue<TValue>> const* matchingOverride = nullptr;
    for (auto const& entry : m_defaultClassPropertyOverrides)
        {
        if (nullptr != entry.first && !entry.first->Is(&prop.GetClass()))
            continue;

        Nullable<PrioritizedValue<TValue>> const& prioritizedValue = valuePicker(entry.second);
        if (prioritizedValue.IsNull())
            continue;

        if (nullptr == matchingOverride || prioritizedValue.Value().priority > matchingOverride->Value().priority)
            matchingOverride = &prioritizedValue;
        }
    if (nullptr != matchingOverride)
        return *matchingOverride;

    static const Nullable<ClassPropertyOverridesInfo::PrioritizedValue<TValue>> s_null;
    return s_null;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<bool> const& ClassPropertyOverridesInfo::GetDisplayOverride(ECPropertyCR prop) const
    {
    return GetOverrides<bool>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<bool> const& {return ovr.GetDisplayOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& ClassPropertyOverridesInfo::GetContentFieldEditorOverride(ECPropertyCR prop) const
    {
    return GetOverrides<std::shared_ptr<ContentFieldEditor const>>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& {return ovr.GetEditorOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<Utf8String> const& ClassPropertyOverridesInfo::GetLabelOverride(ECPropertyCR prop) const
    {
    return GetOverrides<Utf8String>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<Utf8String> const& {return ovr.GetLabelOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<ContentDescriptor::Category> const& ClassPropertyOverridesInfo::GetCategoryOverride(ECPropertyCR prop) const
    {
    return GetOverrides<ContentDescriptor::Category>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<ContentDescriptor::Category> const& {return ovr.GetCategoryOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static std::shared_ptr<ContentFieldEditor> CreateEditorOverride(PropertyEditorSpecificationCR spec)
    {
    return std::shared_ptr<ContentFieldEditor>(ContentFieldEditor::FromSpec(spec));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Category CreateCategoryOverride(Utf8StringCR id, PropertyCategorySpecificationsList const& categorySpecs)
    {
    return ContentDescriptor::Category::FromSpec(id, categorySpecs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static ClassPropertyOverridesInfo::Overrides CreateOverrides(PropertySpecificationCR spec, PropertyCategorySpecificationsList const& categorySpecifications)
    {
    ClassPropertyOverridesInfo::Overrides ovr(spec.GetOverridesPriority());
    if (nullptr != spec.GetEditorOverride())
        ovr.SetEditorOverride(CreateEditorOverride(*spec.GetEditorOverride()));
    if (!spec.GetLabelOverride().empty())
        ovr.SetLabelOverride(spec.GetLabelOverride());
    if (!spec.GetCategoryId().empty())
        ovr.SetCategoryOverride(CreateCategoryOverride(spec.GetCategoryId(), categorySpecifications));
    if (spec.IsDisplayed().IsValid())
        ovr.SetDisplayOverride(spec.IsDisplayed().Value());
    return ovr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static ClassPropertyOverridesInfo::Overrides CreateDefaultHiddenPropertiesOverride()
    {
    ClassPropertyOverridesInfo::Overrides ovr(INT_MIN);
    ovr.SetDisplayOverride(false);
    return ovr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyInfoStore::CollectPropertyOverrides(ECClassCP ecClass, PropertySpecificationCR spec, PropertyCategorySpecificationsList const& categorySpecifications)
    {
    ClassPropertyOverridesInfo info;
    if (spec.GetPropertyName().Equals("*"))
        info.SetClassOverrides(ecClass, CreateOverrides(spec, categorySpecifications));
    else
        info.SetPropertyOverrides(spec.GetPropertyName(), CreateOverrides(spec, categorySpecifications));
    auto iter = m_perClassPropertyOverrides.find(ecClass);
    if (m_perClassPropertyOverrides.end() == iter)
        m_perClassPropertyOverrides[ecClass] = info;
    else
        iter->second.Merge(info);
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::InitPropertyOverrides(ContentSpecificationCP specification, ContentModifierList const& contentModifiers)
    {
    if (nullptr != specification)
        {
        for (PropertySpecificationCP propertySpec : specification->GetPropertyOverrides())
            CollectPropertyOverrides(nullptr, *propertySpec, specification->GetPropertyCategories());
        }

    for (ContentModifierCP modifier : contentModifiers)
        {
        for (PropertySpecificationCP propertySpec : modifier->GetPropertyOverrides())
            {
            ECClassCP ecClass = m_schemaHelper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            if (nullptr == ecClass)
                {
                BeAssert(false);
                continue;
                }
            CollectPropertyOverrides(ecClass, *propertySpec, modifier->GetPropertyCategories());
            }
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas            07/2017
+---------------+---------------+---------------+---------------+-----------+------*/
ClassPropertyOverridesInfo const& PropertyInfoStore::GetOverrides(ECClassCR ecClass) const
    {
    auto iter = m_aggregatedOverrides.find(&ecClass);
    if (m_aggregatedOverrides.end() == iter)
        {
        ClassPropertyOverridesInfo info;

        // merge in overrides that apply for any class (defined at specification level)
        auto anyClassIter = m_perClassPropertyOverrides.find(nullptr);
        if (m_perClassPropertyOverrides.end() != anyClassIter)
            info.Merge(anyClassIter->second);

        // merge in overrides that apply for supplied class (defined as content modifier)
        auto perClassIter = m_perClassPropertyOverrides.find(&ecClass);
        if (m_perClassPropertyOverrides.end() != perClassIter)
            info.Merge(perClassIter->second);

        // find if there's at least one override that requires property to be displayed
        bool hasDisplayOverride = false;
        for (auto const& entry : info.GetPropertyOverrides())
            {
            if (entry.second.GetDisplayOverride().IsValid() && true == entry.second.GetDisplayOverride().Value().value)
                {
                hasDisplayOverride = true;
                break;
                }
            }
        if (hasDisplayOverride)
            {
            // if there's at least one spec requiring property display, it means we should hide all 
            // others - insert hiding display infos with low priority so they don't override any
            // explicitly specified infos
            info.SetClassOverrides(&ecClass, CreateDefaultHiddenPropertiesOverride());
            for (ECClassCP baseClass : ecClass.GetBaseClasses())
                info.SetClassOverrides(baseClass, CreateDefaultHiddenPropertiesOverride());
            }

        // merge in overrides of base class properties
        for (ECClassCP base : ecClass.GetBaseClasses())
            info.Merge(GetOverrides(*base));

        iter = m_aggregatedOverrides.Insert(&ecClass, info).first;
        }
    return iter->second;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2017
+---------------+---------------+---------------+---------------+-----------+------*/
PropertyInfoStore::PropertyInfoStore(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset, ContentSpecificationCP spec)
    : m_schemaHelper(helper)
    {
    InitPropertyOverrides(spec, ruleset.GetContentModifierRules());
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2019
+---------------+---------------+---------------+---------------+-----------+------*/
bool PropertyInfoStore::ShouldDisplay(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetDisplayOverride(prop);
    if (customOverride && customOverride->IsDisplayed().IsValid() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : customOverride->IsDisplayed().Value();
    if (customOverride && customOverride->IsDisplayed().IsValid())
        return customOverride->IsDisplayed().Value();
    if (ovr.IsValid())
        return ovr.Value().value;

    IECInstancePtr hideCustomAttribute = prop.GetCustomAttribute("CoreCustomAttributes", "HiddenProperty");
    if (hideCustomAttribute.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == hideCustomAttribute->GetValue(value, "Show") && (value.IsNull() || (value.IsBoolean() && false == value.GetBoolean())))
            return false;
        }
    return true;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2019
+---------------+---------------+---------------+---------------+-----------+------*/
std::shared_ptr<ContentFieldEditor const> PropertyInfoStore::GetPropertyEditor(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetContentFieldEditorOverride(prop);
    if (customOverride && customOverride->GetEditorOverride() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : CreateEditorOverride(*customOverride->GetEditorOverride());
    if (customOverride && customOverride->GetEditorOverride())
        return CreateEditorOverride(*customOverride->GetEditorOverride());
    if (ovr.IsValid())
        return ovr.Value().value;
    return nullptr;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2019
+---------------+---------------+---------------+---------------+-----------+------*/
Utf8String PropertyInfoStore::GetLabelOverride(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetLabelOverride(prop);
    if (customOverride && !customOverride->GetLabelOverride().empty() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : customOverride->GetLabelOverride();
    if (customOverride && !customOverride->GetLabelOverride().empty())
        return customOverride->GetLabelOverride();
    if (ovr.IsValid())
        return ovr.Value().value;
    return "";
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis            10/2019
+---------------+---------------+---------------+---------------+-----------+------*/
ContentDescriptor::Category PropertyInfoStore::GetCategoryOverride(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride, PropertyCategorySpecificationsList const* scopeCategorySpecs) const
    {
    auto const& ovr = GetOverrides(ecClass).GetCategoryOverride(prop);
    if (customOverride && scopeCategorySpecs && !customOverride->GetCategoryId().empty() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : CreateCategoryOverride(customOverride->GetCategoryId(), *scopeCategorySpecs);
    if (customOverride && scopeCategorySpecs && !customOverride->GetCategoryId().empty())
        return CreateCategoryOverride(customOverride->GetCategoryId(), *scopeCategorySpecs);
    if (ovr.IsValid())
        return ovr.Value().value;
    return ContentDescriptor::Category();
    }