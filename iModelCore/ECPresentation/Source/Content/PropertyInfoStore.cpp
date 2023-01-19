/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ContentFieldEditors.h>
#include "PropertyInfoStore.h"

#define SET_PRIORITZED_NULLABLE_MEMBER(member_name) \
    if (source.member_name.IsValid()) \
        { \
        if (member_name.IsNull() || member_name.Value().priority < source.member_name.Value().priority) \
            member_name = source.member_name; \
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassPropertyOverridesInfo::Overrides::Merge(Overrides const& source)
    {
    SET_PRIORITZED_NULLABLE_MEMBER(m_display);
    SET_PRIORITZED_NULLABLE_MEMBER(m_labelOverride);
    SET_PRIORITZED_NULLABLE_MEMBER(m_category);
    SET_PRIORITZED_NULLABLE_MEMBER(m_renderer);
    SET_PRIORITZED_NULLABLE_MEMBER(m_editor);
    SET_PRIORITZED_NULLABLE_MEMBER(m_doNotHideOtherPropertiesOnDisplayOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<BoolOrString> const& ClassPropertyOverridesInfo::GetDisplayOverride(ECPropertyCR prop) const
    {
    return GetOverrides<BoolOrString>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<BoolOrString> const& {return ovr.GetDisplayOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> const& ClassPropertyOverridesInfo::GetContentFieldRendererOverride(ECPropertyCR prop) const
    {
    return GetOverrides<std::shared_ptr<ContentFieldRenderer const>>(prop, [](Overrides const& ovr) -> auto const& {return ovr.GetRendererOverride(); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& ClassPropertyOverridesInfo::GetContentFieldEditorOverride(ECPropertyCR prop) const
    {
    return GetOverrides<std::shared_ptr<ContentFieldEditor const>>(prop, [](Overrides const& ovr) -> auto const& {return ovr.GetEditorOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<Utf8String> const& ClassPropertyOverridesInfo::GetLabelOverride(ECPropertyCR prop) const
    {
    return GetOverrides<Utf8String>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<Utf8String> const& {return ovr.GetLabelOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<CategoryOverrideInfo const>> const& ClassPropertyOverridesInfo::GetCategoryOverride(ECPropertyCR prop) const
    {
    return GetOverrides<std::shared_ptr<CategoryOverrideInfo const>>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<std::shared_ptr<CategoryOverrideInfo const>> const& {return ovr.GetCategoryOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<bool> const& ClassPropertyOverridesInfo::GetDoNotHideOtherPropertiesOnDisplayOverride(ECPropertyCR prop) const
    {
    return GetOverrides<bool>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<bool> const& {return ovr.GetDoNotHideOtherPropertiesOnDisplayOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<bool> const& ClassPropertyOverridesInfo::GetReadOnlyOverride(ECPropertyCR prop) const
    {
    return GetOverrides<bool>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<bool> const& {return ovr.GetReadOnlyOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<int> const& ClassPropertyOverridesInfo::GetPriorityOverride(ECPropertyCR prop) const
    {
    return GetOverrides<int>(prop, [](Overrides const& ovr) -> NullablePrioritizedValue<int> const& {return ovr.GetPriorityOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::shared_ptr<ContentFieldRenderer> CreateRendererOverride(CustomRendererSpecificationCR spec)
    {
    return std::shared_ptr<ContentFieldRenderer>(ContentFieldRenderer::FromSpec(spec));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::shared_ptr<ContentFieldEditor> CreateEditorOverride(PropertyEditorSpecificationCR spec)
    {
    return std::shared_ptr<ContentFieldEditor>(ContentFieldEditor::FromSpec(spec));
    }

/*---------------------------------------------------------------------------------**//**
* Creates a category from specification. Doesn't create/set the parent even if it's set in specification.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<ContentDescriptor::Category> CreatePropertyCategoryFromSpec(PropertyCategorySpecificationCR spec)
    {
    Utf8String rendererName = spec.GetRendererOverride() ? spec.GetRendererOverride()->GetRendererName() : "";
    return std::make_unique<ContentDescriptor::Category>(spec.GetId(), spec.GetLabel(), spec.GetDescription(), spec.GetPriority(), spec.ShouldAutoExpand(), rendererName);
    }

/*---------------------------------------------------------------------------------**//**
* Creates a categories hierarchy from a category ID and a list of category specifications. The ID must point
* to an existing category in the specifications list. If the category specification specifies a parent id, then it
* has to exist in the list as well.
*
* The result is a list of categories where the first category in the list is the topmost (parentless) category and
* the last category is the category with `categoryId` ID. If the function returns SUCCESS, the stack should always
* contain at least one category.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus CreatePropertyCategoryFromSpec(bvector<std::shared_ptr<PropertyCategoryRef>>& stack, PropertyCategoryIdentifier const& identifier, PropertyCategorySpecificationsList const& specs)
    {
    auto scope = Diagnostics::Scope::Create("Create category");

    if (identifier.GetType() == PropertyCategoryIdentifierType::Root)
        {
        stack.push_back(VirtualPropertyCategoryRef::CreateRootCategoryRef());
        return SUCCESS;
        }

    if (identifier.GetType() == PropertyCategoryIdentifierType::DefaultParent)
        {
        stack.push_back(VirtualPropertyCategoryRef::CreateDefaultParentCategoryRef());
        return SUCCESS;
        }

    auto iter = std::find_if(specs.begin(), specs.end(), [&identifier](PropertyCategorySpecificationCP spec) {return spec->GetId().Equals(identifier.AsIdIdentifier()->GetCategoryId());});
    if (iter == specs.end())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to find category '%s' in current scope. Available categories: [%s]",
            identifier.AsIdIdentifier()->GetCategoryId().c_str(),
            BeStringUtilities::Join(ContainerHelpers::TransformContainer<T_Utf8StringVector>(specs, [](auto const& spec) {return Utf8String("'").append(spec->GetId()).append("'"); })).c_str()));
        return ERROR;
        }
    PropertyCategorySpecificationCR spec = **iter;
    if (nullptr != spec.GetParentId())
        {
        if (SUCCESS != CreatePropertyCategoryFromSpec(stack, *spec.GetParentId(), specs))
            return ERROR;
        }
    stack.push_back(std::make_unique<ConcretePropertyCategoryRef>(CreatePropertyCategoryFromSpec(spec)));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static std::unique_ptr<CategoryOverrideInfo> CreateCategoryOverride(PropertyCategoryIdentifier const& id, PropertyCategorySpecificationsList const& categorySpecs)
    {
    bvector<std::shared_ptr<PropertyCategoryRef>> categories;
    if (SUCCESS == CreatePropertyCategoryFromSpec(categories, id, categorySpecs))
        return std::make_unique<CategoryOverrideInfo>(categories);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ClassPropertyOverridesInfo::Overrides CreateOverrides(PropertySpecificationCR spec, PropertyCategorySpecificationsList const& categorySpecifications)
    {
    ClassPropertyOverridesInfo::Overrides ovr(spec.GetOverridesPriority());
    if (nullptr != spec.GetRendererOverride())
        {
        auto rendererOverride = CreateRendererOverride(*spec.GetRendererOverride());
        if (rendererOverride != nullptr)
            ovr.SetRendererOverride(std::move(rendererOverride));
        }
    if (nullptr != spec.GetEditorOverride())
        {
        auto editorOverride = CreateEditorOverride(*spec.GetEditorOverride());
        if (editorOverride != nullptr)
            ovr.SetEditorOverride(std::move(editorOverride));
        }
    if (!spec.GetLabelOverride().empty())
        ovr.SetLabelOverride(spec.GetLabelOverride());
    if (nullptr != spec.GetCategoryId())
        {
        auto categoryOverride = CreateCategoryOverride(*spec.GetCategoryId(), categorySpecifications);
        if (categoryOverride != nullptr)
            ovr.SetCategoryOverride(std::move(categoryOverride));
        }
    if (spec.IsDisplayed().IsValid())
        ovr.SetDisplayOverride(spec.IsDisplayed());

    ovr.SetDoNotHideOtherPropertiesOnDisplayOverride(spec.DoNotHideOtherPropertiesOnDisplayOverride());

    if (spec.IsReadOnly().IsValid())
        ovr.SetReadOnlyOverride(spec.IsReadOnly().Value());
    if (spec.GetPriority().IsValid())
        ovr.SetPriorityOverride(spec.GetPriority().Value());

    return ovr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ClassPropertyOverridesInfo::Overrides CreateDefaultHiddenPropertiesOverride()
    {
    ClassPropertyOverridesInfo::Overrides ovr(INT_MIN);
    ovr.SetDisplayOverride(false);
    return ovr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
void PropertyInfoStore::InitPropertyOverrides(ContentSpecificationCP specification, bvector<ContentModifierCP> const& contentModifiers)
    {
    if (nullptr != specification)
        {
        for (PropertySpecificationCP propertySpec : specification->GetPropertyOverrides())
            CollectPropertyOverrides(nullptr, *propertySpec, specification->GetPropertyCategories());
        }

    for (ContentModifierCP modifier : contentModifiers)
        {
        DiagnosticsHelpers::ReportRule(*modifier);
        for (PropertySpecificationCP propertySpec : modifier->GetPropertyOverrides())
            {
            ECClassCP ecClass = m_schemaHelper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            CollectPropertyOverrides(ecClass, *propertySpec, modifier->GetPropertyCategories());
            }
        }
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
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

        // find if there's at least one override that requires property to be displayed alone (hiding other properties)
        bool shouldHideOtherProperties = false;
        for (auto const& entry : info.GetPropertyOverrides())
            {
            const auto displayOverride = entry.second.GetDisplayOverride();
            const auto doNotHideOtherPropertiesOverride = entry.second.GetDoNotHideOtherPropertiesOnDisplayOverride();
            if ((displayOverride.IsValid() && (displayOverride.Value().value.IsString() || displayOverride.Value().value == true)) &&
                (doNotHideOtherPropertiesOverride.IsNull() || false == doNotHideOtherPropertiesOverride.Value().value))
                {
                shouldHideOtherProperties = true;
                break;
                }
            }
        if (shouldHideOtherProperties)
            {
            // if there's at least one spec requiring property display and requesting to hide other properties,
            // it means we should insert hiding display infos with low priority so they don't override any
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
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
PropertyInfoStore::PropertyInfoStore(ECSchemaHelper const& helper, bvector<ContentModifierCP> const& contentModifiers, ContentSpecificationCP spec)
    : m_schemaHelper(helper)
    {
    InitPropertyOverrides(spec, contentModifiers);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Nullable<bool> EvaluateDisplayOverride(BoolOrString const& displayOverride, std::function<ExpressionContextPtr()> const& expressionContextFactory)
    {
    if (!displayOverride.IsValid())
        return nullptr;

    if (displayOverride.IsBoolean())
        return displayOverride.GetBoolean();

    EvaluationResult result;
    ExpressionStatus status = ECEvaluator::EvaluateExpression(result, displayOverride.GetString().c_str(), *expressionContextFactory());

    if (status != ExpressionStatus::Success || !result.IsECValue() || !result.GetECValue()->IsBoolean())
        return nullptr;

    return result.GetECValue()->GetBoolean();
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
bool PropertyInfoStore::ShouldDisplay(ECPropertyCR prop, ECClassCR ecClass, std::function<ExpressionContextPtr()> const& expressionContextFactory, PropertySpecificationsList const& customOverrides) const
    {
    auto const displayOverride = ContainerHelpers::FindFirst<PropertySpecificationP>(customOverrides, [&](auto const& spec)
        {
        return spec->IsDisplayed().IsValid();
        }, nullptr);
    Nullable<bool> displayOverrideValue = nullptr;
    if (displayOverride && displayOverride->IsDisplayed().IsValid())
        displayOverrideValue = EvaluateDisplayOverride(displayOverride->IsDisplayed(), expressionContextFactory);

    auto const& ovr = GetOverrides(ecClass).GetDisplayOverride(prop);
    Nullable<bool> ovrValue = nullptr;
    if (ovr.IsValid())
        ovrValue = EvaluateDisplayOverride(ovr.Value().value, expressionContextFactory);

    if (displayOverride && displayOverrideValue.IsValid() && ovrValue.IsValid())
        return (ovr.Value().priority > displayOverride->GetOverridesPriority()) ? ovrValue.Value() : displayOverrideValue.Value();
    if (displayOverrideValue.IsValid())
        return displayOverrideValue.Value();
    if (ovrValue.IsValid())
        return ovrValue.Value();

    IECInstancePtr hideCustomAttribute = prop.GetCustomAttribute("CoreCustomAttributes", "HiddenProperty");
    if (hideCustomAttribute.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == hideCustomAttribute->GetValue(value, "Show") && (value.IsNull() || (value.IsBoolean() && false == value.GetBoolean())))
            return false;
        }
    return true;
    }

/*-----------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
std::shared_ptr<ContentFieldRenderer const> PropertyInfoStore::GetPropertyRenderer(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetContentFieldRendererOverride(prop);
    if (customOverride && customOverride->GetRendererOverride() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : CreateRendererOverride(*customOverride->GetRendererOverride());
    if (customOverride && customOverride->GetRendererOverride())
        return CreateRendererOverride(*customOverride->GetRendererOverride());
    if (ovr.IsValid())
        return ovr.Value().value;
    return nullptr;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
std::unique_ptr<CategoryOverrideInfo const> PropertyInfoStore::GetCategoryOverride(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride, PropertyCategorySpecificationsList const* scopeCategorySpecs) const
    {
    auto const& ovr = GetOverrides(ecClass).GetCategoryOverride(prop);
    bool hasPropertySpecCategoryOverride = customOverride && scopeCategorySpecs && nullptr != customOverride->GetCategoryId();

    if (hasPropertySpecCategoryOverride)
        {
        if (ovr.IsValid() && ovr.Value().priority > customOverride->GetOverridesPriority())
            return std::make_unique<CategoryOverrideInfo>(*ovr.Value().value);

        return CreateCategoryOverride(*customOverride->GetCategoryId(), *scopeCategorySpecs);
        }

    if (ovr.IsValid())
        return std::make_unique<CategoryOverrideInfo>(*ovr.Value().value);

    return nullptr;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
Nullable<bool> PropertyInfoStore::GetReadOnlyOverride(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetReadOnlyOverride(prop);
    if (customOverride && customOverride->IsReadOnly().IsValid() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : customOverride->IsReadOnly().Value();
    if (customOverride && customOverride->IsReadOnly().IsValid())
        return customOverride->IsReadOnly().Value();
    if (ovr.IsValid())
        return ovr.Value().value;
    return nullptr;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
Nullable<int> PropertyInfoStore::GetPriorityOverride(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetPriorityOverride(prop);
    if (customOverride && customOverride->GetPriority().IsValid() && ovr.IsValid())
        return (ovr.Value().priority > customOverride->GetOverridesPriority()) ? ovr.Value().value : customOverride->GetPriority().Value();
    if (customOverride && customOverride->GetPriority().IsValid())
        return customOverride->GetPriority().Value();
    if (ovr.IsValid())
        return ovr.Value().value;
    return nullptr;
    }
