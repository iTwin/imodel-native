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

    for (auto const& category : source.m_availableCategories)
        {
        auto iter = std::find_if(m_availableCategories.begin(), m_availableCategories.end(), [&category](PropertyCategorySpecificationP const& spec) { return spec->GetId().Equals(category->GetId()); });
        if (m_availableCategories.end() == iter)
            m_availableCategories.push_back(category);
        else if ((*iter)->GetPriority() < category->GetPriority())
            *iter = category;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TValue>
Nullable<ClassPropertyOverridesInfo::PrioritizedValue<TValue>> const& ClassPropertyOverridesInfo::GetOverrides(ECClassCR propertyClass, Utf8StringCR propertyName, Nullable<PrioritizedValue<TValue>> const& (*valuePicker)(Overrides const&)) const
    {
    // first, look for overrides specified specifically for some property
    auto iter = m_propertyOverrides.find(propertyName);
    if (m_propertyOverrides.end() != iter)
        return valuePicker(iter->second);

    // then, attempt to find a default class override with the highest priority
    Nullable<PrioritizedValue<TValue>> const* matchingOverride = nullptr;
    for (auto const& entry : m_defaultClassPropertyOverrides)
        {
        if (nullptr != entry.first && !entry.first->Is(&propertyClass))
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
    return GetOverrides<BoolOrString>(prop.GetClass(), prop.GetName(), [](Overrides const& ovr) -> NullablePrioritizedValue<BoolOrString> const& {return ovr.GetDisplayOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> const& ClassPropertyOverridesInfo::GetContentFieldRendererOverride(ECClassCR propertyClass, Utf8StringCR propertyName) const
    {
    return GetOverrides<std::shared_ptr<ContentFieldRenderer const>>(propertyClass, propertyName, [](Overrides const& ovr) -> auto const& {return ovr.GetRendererOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> const& ClassPropertyOverridesInfo::GetContentFieldRendererOverride(ECPropertyCR prop) const
    {
    return GetContentFieldRendererOverride(prop.GetClass(), prop.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& ClassPropertyOverridesInfo::GetContentFieldEditorOverride(ECClassCR propertyClass, Utf8StringCR propertyName) const
    {
    return GetOverrides<std::shared_ptr<ContentFieldEditor const>>(propertyClass, propertyName, [](Overrides const& ovr) -> auto const& {return ovr.GetEditorOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& ClassPropertyOverridesInfo::GetContentFieldEditorOverride(ECPropertyCR prop) const
    {
    return GetContentFieldEditorOverride(prop.GetClass(), prop.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<Utf8String> const& ClassPropertyOverridesInfo::GetLabelOverride(ECPropertyCR prop) const
    {
    return GetOverrides<Utf8String>(prop.GetClass(), prop.GetName(), [](Overrides const& ovr) -> NullablePrioritizedValue<Utf8String> const& {return ovr.GetLabelOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<PropertyCategoryIdentifier const*> const& ClassPropertyOverridesInfo::GetCategoryOverride(ECPropertyCR prop) const
    {
    return GetOverrides<PropertyCategoryIdentifier const*>(prop.GetClass(), prop.GetName(), [](Overrides const& ovr) -> NullablePrioritizedValue<PropertyCategoryIdentifier const*> const& {return ovr.GetCategoryOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<bool> const& ClassPropertyOverridesInfo::GetDoNotHideOtherPropertiesOnDisplayOverride(ECPropertyCR prop) const
    {
    return GetOverrides<bool>(prop.GetClass(), prop.GetName(), [](Overrides const& ovr) -> NullablePrioritizedValue<bool> const& {return ovr.GetDoNotHideOtherPropertiesOnDisplayOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<bool> const& ClassPropertyOverridesInfo::GetReadOnlyOverride(ECPropertyCR prop) const
    {
    return GetOverrides<bool>(prop.GetClass(), prop.GetName(), [](Overrides const& ovr) -> NullablePrioritizedValue<bool> const& {return ovr.GetReadOnlyOverride();});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassPropertyOverridesInfo::NullablePrioritizedValue<int> const& ClassPropertyOverridesInfo::GetPriorityOverride(ECPropertyCR prop) const
    {
    return GetOverrides<int>(prop.GetClass(), prop.GetName(), [](Overrides const& ovr) -> NullablePrioritizedValue<int> const& {return ovr.GetPriorityOverride();});
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

    if (identifier.GetType() == PropertyCategoryIdentifierType::SchemaCategory)
        {
        stack.push_back(std::make_unique<SchemaPropertyCategoryRef>(identifier.AsSchemaCategoryIdentifier()->GetCategoryName()));
        return SUCCESS;
        }

    auto iter = std::find_if(specs.begin(), specs.end(), [&identifier](PropertyCategorySpecificationCP spec) {return spec->GetId().Equals(identifier.AsIdIdentifier()->GetCategoryId());});
    if (iter == specs.end())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Failed to find category '%s' in current scope. Available categories: [%s]",
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
static ClassPropertyOverridesInfo::Overrides CreateOverrides(PropertySpecificationCR spec)
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
        ovr.SetCategoryOverride(spec.GetCategoryId());

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
void PropertyInfoStore::CollectPropertyOverrides(ECClassCP ecClass, PropertySpecificationCR spec)
    {
    ClassPropertyOverridesInfo info;
    if (spec.GetPropertyName().Equals("*"))
        info.SetClassOverrides(ecClass, CreateOverrides(spec));
    else
        info.SetPropertyOverrides(spec.GetPropertyName(), CreateOverrides(spec));

    auto iter = m_perClassPropertyOverrides.find(ecClass);
    if (m_perClassPropertyOverrides.end() == iter)
        m_perClassPropertyOverrides[ecClass] = info;
    else
        iter->second.Merge(info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyInfoStore::CollectCategories(ECClassCP ecClass, PropertyCategorySpecificationsList const& categorySpecifications)
    {
    ClassPropertyOverridesInfo info;
    info.SetAsAvailableCategories(categorySpecifications);

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
        CollectCategories(nullptr, specification->GetPropertyCategories());
        for (PropertySpecificationCP propertySpec : specification->GetPropertyOverrides())
            CollectPropertyOverrides(nullptr, *propertySpec);
        }

    for (ContentModifierCP modifier : contentModifiers)
        {
        DiagnosticsHelpers::ReportRule(*modifier);
        ECClassCP ecClass = m_schemaHelper.GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());

        CollectCategories(ecClass, modifier->GetPropertyCategories());
        for (PropertySpecificationCP propertySpec : modifier->GetPropertyOverrides())
            CollectPropertyOverrides(ecClass, *propertySpec);
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
PropertyInfoStore::PropertyInfoStore(ECSchemaHelper const& helper, bvector<ContentModifierCP> const& contentModifiers, ContentSpecificationCP spec, IPropertyCategorySupplierCP categorySupplier)
    : m_schemaHelper(helper), m_categorySupplier(categorySupplier)
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
std::shared_ptr<ContentFieldRenderer const> PropertyInfoStore::GetPropertyRenderer(ECClassCR ecClass, Utf8StringCR propertyName, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetContentFieldRendererOverride(ecClass, propertyName);
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
std::shared_ptr<ContentFieldEditor const> PropertyInfoStore::GetPropertyEditor(ECClassCR ecClass, Utf8StringCR propertyName, PropertySpecificationCP customOverride) const
    {
    auto const& ovr = GetOverrides(ecClass).GetContentFieldEditorOverride(ecClass, propertyName);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CategoryOverrideInfo const* PropertyInfoStore::GetCategoryOverride(PropertyCategoryIdentifier const& id, ECClassCP ecClass, PropertyCategorySpecificationsList const& categorySpecs) const
    {
    // check if we need to use class-based category
    bool shouldCreateWithClassCategory = ecClass != nullptr && id.AsIdIdentifier() != nullptr && id.AsIdIdentifier()->ShouldCreateClassCategory();
    Utf8CP classNameForCategoriesCache = shouldCreateWithClassCategory ? ecClass->GetFullName() : "";

    auto override = m_categoryOverridesCache.find(CategoryOverridesCacheKey(id, classNameForCategoriesCache));
    if (override != m_categoryOverridesCache.end())
        return &override->second;

    // create new category override and insert to cache
    bvector<std::shared_ptr<PropertyCategoryRef>> categories;
    if (SUCCESS == CreatePropertyCategoryFromSpec(categories, id, categorySpecs))
        {
        if (shouldCreateWithClassCategory && m_categorySupplier != nullptr)
            categories.push_back(std::make_unique<ConcretePropertyCategoryRef>(m_categorySupplier->CreateECClassCategory(*ecClass)));

        auto insertedElement = m_categoryOverridesCache.insert({ CategoryOverridesCacheKey(id, classNameForCategoriesCache), categories });
        return &insertedElement.first->second;
        }
    return nullptr;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
CategoryOverrideInfo const* PropertyInfoStore::GetCategoryOverride(ECPropertyCR prop, ECClassCR ecClass, PropertySpecificationCP customOverride, PropertyCategorySpecificationsList const* scopeCategorySpecs) const
    {
    auto const& classOverrides = GetOverrides(ecClass);
    auto const& categoryOverride = classOverrides.GetCategoryOverride(prop);
    bool hasPropertySpecCategoryOverride = customOverride && scopeCategorySpecs && nullptr != customOverride->GetCategoryId();

    if (hasPropertySpecCategoryOverride)
        {
        if (categoryOverride.IsValid() && categoryOverride.Value().priority > customOverride->GetOverridesPriority())
            return GetCategoryOverride(*categoryOverride.Value().value, &ecClass, classOverrides.GetAvailableCategories());

        // try to get categories from local scope
        auto overrideFromLocalScope = GetCategoryOverride(*customOverride->GetCategoryId(), &ecClass, *scopeCategorySpecs);
        if (nullptr != overrideFromLocalScope)
            return overrideFromLocalScope;

        // if local scope is empty, get from global scope
        return GetCategoryOverride(*customOverride->GetCategoryId(), &ecClass, classOverrides.GetAvailableCategories());
        }

    if (categoryOverride.IsValid())
        return GetCategoryOverride(*categoryOverride.Value().value, &ecClass, classOverrides.GetAvailableCategories());

    return nullptr;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+-----------+------*/
CategoryOverrideInfo const* PropertyInfoStore::GetCategoryOverride(ECClassCP ecClass, CalculatedPropertiesSpecificationCR spec) const
    {
    if (spec.GetCategoryId() == nullptr)
        return nullptr;

    if (ecClass != nullptr)
        {
        auto const& classOverrides = GetOverrides(*ecClass);
        return GetCategoryOverride(*spec.GetCategoryId(), ecClass, classOverrides.GetAvailableCategories());
        }

    auto const& classOverrides = m_perClassPropertyOverrides.find(ecClass);
    if (classOverrides != m_perClassPropertyOverrides.end())
        return GetCategoryOverride(*spec.GetCategoryId(), ecClass, classOverrides->second.GetAvailableCategories());

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
