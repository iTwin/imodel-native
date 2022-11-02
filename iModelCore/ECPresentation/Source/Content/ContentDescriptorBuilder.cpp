/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "ContentQueryBuilder.h"
#include "PropertyInfoStore.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldCreateFields(ContentDescriptorCR descriptor)
    {
    return !descriptor.HasContentFlag(ContentFlags::NoFields)
        && !descriptor.HasContentFlag(ContentFlags::KeysOnly);
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CategoriesSupplierContext
{
private:
    ContentDescriptorBuilder::Context const& m_descriptorBuilderContext;
    PropertyInfoStore const& m_propertyInfos;
    bmap<std::pair<Utf8String, ContentDescriptor::Category const*>, std::shared_ptr<ContentDescriptor::Category>> m_categories; // the key is category name and parent category pointer
    std::shared_ptr<ContentDescriptor::Category> m_defaultCategory;
public:
    CategoriesSupplierContext(ContentDescriptorBuilder::Context const& descriptorBuilderContext, PropertyInfoStore const& propertyInfos)
        : m_descriptorBuilderContext(descriptorBuilderContext), m_propertyInfos(propertyInfos)
        {}
    ContentDescriptorBuilder::Context const& GetDescriptorBuilderContext() const {return m_descriptorBuilderContext;}
    IPropertyCategorySupplierCR GetPropertyCategories() const {return m_descriptorBuilderContext.GetCategorySupplier();}
    PropertyInfoStore const& GetPropertyInfos() const {return m_propertyInfos;}
    bmap<std::pair<Utf8String, ContentDescriptor::Category const*>, std::shared_ptr<ContentDescriptor::Category>> const& GetAllCategories() const {return m_categories;}
    bmap<std::pair<Utf8String, ContentDescriptor::Category const*>, std::shared_ptr<ContentDescriptor::Category>>& GetAllCategories() {return m_categories;}
    std::shared_ptr<ContentDescriptor::Category> GetDefaultCategory() const {return m_defaultCategory;}
    void SetDefaultCategory(std::shared_ptr<ContentDescriptor::Category> category) {m_defaultCategory = category;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct CategoriesSupplier
{
    enum class ParentOption
        {
        NoParent,
        ParentOnlyFromStack,
        ParentFromStackOrDefault,
        };

private:
    CategoriesSupplierContext& m_context;
    bvector<std::shared_ptr<ContentDescriptor::Category>> m_categoriesStack;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetSharedCategory(Utf8StringCR name, ContentDescriptor::Category const* parentCategory) const
        {
        auto it = m_context.GetAllCategories().find(std::make_pair(name, parentCategory));
        if (m_context.GetAllCategories().end() != it)
            return it->second;

        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetDefaultCategory()
        {
        if (!m_context.GetDefaultCategory())
            {
            std::shared_ptr<ContentDescriptor::Category> defaultCategory;
            DefaultPropertyCategoryOverrideCP ovr = m_context.GetDescriptorBuilderContext().GetRulesPreprocessor().GetDefaultPropertyCategoryOverride();
            if (ovr)
                {
                DiagnosticsHelpers::ReportRule(*ovr);
                defaultCategory = std::make_shared<ContentDescriptor::Category>(ovr->GetSpecification().GetId(), ovr->GetSpecification().GetLabel(),
                    ovr->GetSpecification().GetDescription(), ovr->GetSpecification().GetPriority(), ovr->GetSpecification().ShouldAutoExpand());
                }
            else
                {
                defaultCategory = m_context.GetPropertyCategories().CreateDefaultCategory();
                }
            m_context.GetAllCategories().Insert(std::make_pair(defaultCategory->GetName(), defaultCategory->GetParentCategory()), defaultCategory);
            m_context.SetDefaultCategory(defaultCategory);
            }
        return m_context.GetDefaultCategory();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetRootCategory()
        {
        for (auto iter = m_categoriesStack.rbegin(); iter != m_categoriesStack.rend(); ++iter)
            {
            auto category = *iter;
            if (nullptr == category->GetParentCategory())
                return category;
            }
        return GetDefaultCategory();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> PrepareCategoryForReturn(std::shared_ptr<ContentDescriptor::Category> category, ParentOption parentOption)
        {
        auto parentCategory = (parentOption != ParentOption::NoParent) ? GetParentCategory(parentOption == ParentOption::ParentFromStackOrDefault) : nullptr;
        if (auto shared = GetSharedCategory(category->GetName(), parentCategory.get()))
            return shared;

        category->SetParentCategory(parentCategory);
        m_context.GetAllCategories().Insert(std::make_pair(category->GetName(), category->GetParentCategory()), category);
        return category;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> PrepareCategoryForReturn(CategoryOverrideInfo const& overrideInfo)
        {
        std::shared_ptr<ContentDescriptor::Category> parentCategory = nullptr;
        for (auto const& categoryRef : overrideInfo.GetCategoriesStack())
            {
            if (categoryRef->IsRootCategoryRef())
                {
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, !parentCategory, Utf8PrintfString("Detected root category reference in categories "
                    "stack when parent category is already set. Existing parent category name: `%s`", parentCategory->GetName().c_str()));
                parentCategory = GetRootCategory();
                continue;
                }
            if (categoryRef->IsDefaultParentCategoryRef())
                {
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, !parentCategory, Utf8PrintfString("Detected default parent category reference in categories "
                    "stack when parent category is already set. Existing parent category name: `%s`", parentCategory->GetName().c_str()));
                parentCategory = GetParentCategory(true);
                continue;
                }

            auto const& category = categoryRef->AsConcreteCategoryRef()->GetCategory();
            if (auto shared = GetSharedCategory(category.GetName().c_str(), parentCategory.get()))
                {
                parentCategory = shared;
                continue;
                }

            auto newCategory = std::make_shared<ContentDescriptor::Category>(category);
            newCategory->SetParentCategory(parentCategory);
            m_context.GetAllCategories().Insert(std::make_pair(newCategory->GetName(), newCategory->GetParentCategory()), newCategory);
            parentCategory = newCategory;
            }
        return parentCategory;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    CategoriesSupplier(CategoriesSupplierContext& context) : m_context(context) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    CategoriesSupplierContext& GetContext() {return m_context;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetParentCategory(bool useDefaulIfStackEmpty)
        {
        if (!m_categoriesStack.empty())
            return m_categoriesStack.back();
        return useDefaulIfStackEmpty ? GetDefaultCategory() : nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void PushParentCategory(std::shared_ptr<ContentDescriptor::Category> category) { m_categoriesStack.push_back(category); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetPropertiesFieldCategory(ECPropertyCR ecProperty, ECClassCR actualClass, RelatedClassPathCR pathFromSelectToPropertyClass,
        RelationshipMeaning meaning, bool isInNestedContent, PropertySpecificationCP overrides, PropertyCategorySpecificationsList const* scopePropertyCategories)
        {
        // category overrides in presentation rules take highest precedence
        std::unique_ptr<CategoryOverrideInfo const> categoryOverride = m_context.GetPropertyInfos().GetCategoryOverride(ecProperty, actualClass, overrides, scopePropertyCategories);
        if (categoryOverride)
            return PrepareCategoryForReturn(*categoryOverride);

        // then check for property categories in schema
        std::shared_ptr<ContentDescriptor::Category const> category = m_context.GetPropertyCategories().CreatePropertyCategory(ecProperty);
        if (category)
            return PrepareCategoryForReturn(std::make_unique<ContentDescriptor::Category>(*category), ParentOption::ParentFromStackOrDefault);

        // if this is a related property with 'same instance' meaning, use the related class as the category
        if (!pathFromSelectToPropertyClass.empty() && meaning == RelationshipMeaning::SameInstance)
            return PrepareCategoryForReturn(m_context.GetPropertyCategories().CreateECClassCategory(actualClass), ParentOption::ParentFromStackOrDefault);

        // otherwise, just use the parent category (no need to prepare the category because it is was already prepared when building category stack)
        return GetParentCategory(true);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetRelatedContentFieldCategory(ECClassCR contentClass, RelationshipMeaning meaning)
        {
        auto parentCategoryOption = (meaning == RelationshipMeaning::RelatedInstance) ? ParentOption::ParentOnlyFromStack : ParentOption::ParentFromStackOrDefault;
        return PrepareCategoryForReturn(m_context.GetPropertyCategories().CreateECClassCategory(contentClass), parentCategoryOption);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> GetCalculatedFieldCategory()
        {
        // just use the default category
        // TODO: support category overrides for calculated fields
        return PrepareCategoryForReturn(GetDefaultCategory(), ParentOption::NoParent);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool HasSharedCategory(ECClassCR contentClass, RelationshipMeaning meaning)
        {
        auto parentCategory = GetParentCategory(meaning == RelationshipMeaning::SameInstance);
        return GetSharedCategory(m_context.GetPropertyCategories().CreateECClassCategory(contentClass)->GetName(), parentCategory.get()) != nullptr;
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ContentPropertiesAppender : ContentSpecificationsHandler::PropertyAppender
{
    //===================================================================================
    // @bsiclass
    //===================================================================================
    enum class FieldCreateAction
        {
        Skip,
        Merge,
        Create,
        };

    //===================================================================================
    // @bsiclass
    //===================================================================================
    struct PropertySpecificationOverrides
    {
    private:
        PropertySpecificationCP m_labelOverride;
        PropertySpecificationCP m_categoryOverride;
        PropertySpecificationCP m_rendererOverride;
        PropertySpecificationCP m_editorOverride;
        PropertySpecificationCP m_readOnlyOverride;
        PropertySpecificationCP m_priorityOverride;
    public:
        PropertySpecificationOverrides(PropertySpecificationCP labelOverride, PropertySpecificationCP categoryOverride,
            PropertySpecificationCP rendererOverride, PropertySpecificationCP editorOverride, PropertySpecificationCP readOnlyOverride,
            PropertySpecificationCP priorityOverride)
            : m_labelOverride(labelOverride), m_categoryOverride(categoryOverride), m_rendererOverride(rendererOverride), m_editorOverride(editorOverride),
            m_readOnlyOverride(readOnlyOverride), m_priorityOverride(priorityOverride)
            {}

        PropertySpecificationCP GetLabelOverrideSpecification() const { return m_labelOverride; }
        PropertySpecificationCP GetCategoryOverrideSpecification() const { return m_categoryOverride; }
        PropertySpecificationCP GetRendererOverrideSpecification() const { return m_rendererOverride; }
        PropertySpecificationCP GetEditorOverrideSpecification() const { return m_editorOverride; }
        PropertySpecificationCP GetReadOnlyOverrideSpecification() const { return m_readOnlyOverride; }
        PropertySpecificationCP GetPriorityOverrideSpecification() const { return m_priorityOverride; }
    };

    //===================================================================================
    // @bsiclass
    //===================================================================================
    struct FieldAttributes
    {
    private:
        Utf8String m_label;
        std::shared_ptr<ContentDescriptor::Category> m_category;
        std::shared_ptr<ContentFieldRenderer const> m_renderer;
        std::shared_ptr<ContentFieldEditor const> m_editor;
        Nullable<bool> m_isReadOnly;
        Nullable<int> m_priority;
        bool m_shouldAutoExpand;
    public:
        FieldAttributes(Utf8String label, std::shared_ptr<ContentDescriptor::Category> category, std::shared_ptr<ContentFieldRenderer const> renderer, std::shared_ptr<ContentFieldEditor const> editor,
            Nullable<bool> isReadOnly, Nullable<int> priority, bool shouldAutoExpand)
            : m_label(label), m_category(category), m_renderer(renderer), m_editor(editor), m_isReadOnly(isReadOnly), m_priority(priority), m_shouldAutoExpand(shouldAutoExpand)
            {}
        Utf8StringCR GetLabel() const {return m_label;}
        std::shared_ptr<ContentDescriptor::Category> GetCategory() const {return m_category;}
        std::shared_ptr<ContentFieldRenderer const> GetRenderer() const {return m_renderer;}
        std::shared_ptr<ContentFieldEditor const> GetEditor() const {return m_editor;}
        Nullable<bool> GetIsReadOnly() const {return m_isReadOnly;}
        Nullable<int32_t> GetPriority() const {return m_priority;}
        bool ShouldAutoExpand() const {return m_shouldAutoExpand;}
    };

protected:
    ContentDescriptorBuilder::Context& m_context;
    PropertyInfoStore const& m_propertyInfos;
    ContentDescriptorR m_descriptor;
    ECClassCR m_actualPropertyClass;
    PropertyCategorySpecificationsList const* m_scopePropertyCategories;
    CategoriesSupplier m_categoriesSupplier;

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CreateFieldDisplayLabel(ECPropertyCR ecProperty, RelatedClassPathCR pathFromSelectToPropertyClass,
        RelationshipMeaning relationshipMeaning, PropertySpecificationCP overrides) const
        {
        Utf8String labelOverride = m_propertyInfos.GetLabelOverride(ecProperty, m_actualPropertyClass, overrides);
        if (!labelOverride.empty())
            return labelOverride;

        Utf8String displayLabel;
        if (nullptr != m_context.GetPropertyFormatter() && SUCCESS == m_context.GetPropertyFormatter()->GetFormattedPropertyLabel(displayLabel, ecProperty, m_actualPropertyClass, pathFromSelectToPropertyClass, relationshipMeaning))
            return displayLabel;

        return ecProperty.GetDisplayLabel();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentDescriptor::Category> CreatePropertiesFieldCategory(ECPropertyCR ecProperty, RelatedClassPathCR pathFromSelectToPropertyClass,
        RelationshipMeaning relationshipMeaning, bool isInNestedContent, PropertySpecificationCP overrides)
        {
        return m_categoriesSupplier.GetPropertiesFieldCategory(ecProperty, m_actualPropertyClass, pathFromSelectToPropertyClass,
            relationshipMeaning, isInNestedContent, overrides, m_scopePropertyCategories);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentFieldRenderer const> CreateFieldRenderer(ECPropertyCR ecProperty, PropertySpecificationCP overrides) const
        {
        return m_propertyInfos.GetPropertyRenderer(ecProperty, m_actualPropertyClass, overrides);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentFieldEditor const> CreateFieldEditor(ECPropertyCR ecProperty, PropertySpecificationCP overrides) const
        {
        return m_propertyInfos.GetPropertyEditor(ecProperty, m_actualPropertyClass, overrides);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Nullable<bool> CreateFieldReadOnlyFlag(ECPropertyCR ecProperty, PropertySpecificationCP overrides) const
        {
        return m_propertyInfos.GetReadOnlyOverride(ecProperty, m_actualPropertyClass, overrides);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Nullable<int> CreateFieldPriority(ECPropertyCR ecProperty, PropertySpecificationCP overrides) const
        {
        return m_propertyInfos.GetPriorityOverride(ecProperty, m_actualPropertyClass, overrides);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _Supports(ECPropertyCR ecProperty, PropertySpecificationsList const& overrides) override
        {
        // don't support any properties if we're not creating fields
        if (!ShouldCreateFields(m_descriptor))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Descriptor doesn't want fields created - skip.");
            return false;
            }

        // don't support hidden properties
        if (!m_propertyInfos.ShouldDisplay(ecProperty, m_actualPropertyClass, [this]() { return CreateExpressionContext(m_context); }, overrides))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Property is hidden - skip.");
            return false;
            }

        // don't support binary and igeometry properties
        // note: we don't want to these fields even if the above says we should..
        if (ecProperty.GetIsPrimitive() && (PRIMITIVETYPE_Binary == ecProperty.GetAsPrimitiveProperty()->GetType() || PRIMITIVETYPE_IGeometry == ecProperty.GetAsPrimitiveProperty()->GetType()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Binary and IGeometry fields are not supported - skip.");
            return false;
            }

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentSpecificationsHandler::PropertyAppendResult _DoAppendProperty(ECPropertyCR, Utf8CP propertyClassAlias, PropertySpecificationsList const&) = 0;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentSpecificationsHandler::PropertyAppendResult _Append(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationsList const& overrides) override
        {
        ContentSpecificationsHandler::PropertyAppendResult result(false);
        RelatedClass pathFromPropertyClassToNavigationPropertyTargetClass;
        if (ecProperty.GetIsNavigation())
            {
            // note: pathFromPropertyClassToNavigationPropertyTargetClass need to be declared outside this scope, because 
            // we're using its target class alias
            pathFromPropertyClassToNavigationPropertyTargetClass = m_context.GetSchemaHelper().GetForeignKeyClass(ecProperty);
            pathFromPropertyClassToNavigationPropertyTargetClass.GetTargetClass().SetAlias(m_context.CreateNavigationClassAlias(pathFromPropertyClassToNavigationPropertyTargetClass.GetTargetClass().GetClass()));
            pathFromPropertyClassToNavigationPropertyTargetClass.GetRelationship().SetAlias(m_context.CreateNavigationClassAlias(pathFromPropertyClassToNavigationPropertyTargetClass.GetRelationship().GetClass()));
            result.GetAppendedNavigationPropertyPaths().push_back(pathFromPropertyClassToNavigationPropertyTargetClass);
            propertyClassAlias = pathFromPropertyClassToNavigationPropertyTargetClass.GetTargetClass().GetAlias().c_str();
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Switched to alias `%s` as it's a navigation property.", propertyClassAlias));
            }
        result.MergeWith(_DoAppendProperty(ecProperty, propertyClassAlias, overrides));
        return result;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentPropertiesAppender(ContentDescriptorBuilder::Context& context, PropertyInfoStore const& propertyInfos, ContentDescriptorR descriptor,
        CategoriesSupplier categoriesSupplier, ECClassCR actualPropertyClass, PropertyCategorySpecificationsList const* scopePropertyCategories)
        : m_context(context), m_descriptor(descriptor), m_propertyInfos(propertyInfos), m_actualPropertyClass(actualPropertyClass),
        m_categoriesSupplier(categoriesSupplier), m_scopePropertyCategories(scopePropertyCategories)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertySpecificationOverrides FindPropertySpecificationOverrides(PropertySpecificationsList const& overrides) const
        {
        auto displayLabelOverrideSpec = ContainerHelpers::FindFirst<PropertySpecificationP>(overrides, [&](auto const spec) {
            return !spec->GetLabelOverride().empty(); }, nullptr);

        auto propertiesFieldCategoryOverrideSpec = ContainerHelpers::FindFirst<PropertySpecificationP>(overrides, [&](auto const spec) {
            return spec->GetCategoryId() != nullptr; }, nullptr);

        auto fieldRendererOverrideSpec = ContainerHelpers::FindFirst<PropertySpecificationP>(overrides, [&](auto const spec) {
            return spec->GetRendererOverride() != nullptr; }, nullptr);

        auto fieldEditorOverrideSpec = ContainerHelpers::FindFirst<PropertySpecificationP>(overrides, [&](auto const spec) {
            return spec->GetEditorOverride() != nullptr; }, nullptr);

        auto fieldReadOnlyOverrideSpec = ContainerHelpers::FindFirst<PropertySpecificationP>(overrides, [&](auto const spec) {
            return spec->IsReadOnly() != nullptr; }, nullptr);

        auto fieldPriorityOverrideSpec = ContainerHelpers::FindFirst<PropertySpecificationP>(overrides, [&](auto const spec) {
            return spec->GetPriority() != nullptr; }, nullptr);

        return PropertySpecificationOverrides(displayLabelOverrideSpec, propertiesFieldCategoryOverrideSpec, fieldRendererOverrideSpec,
            fieldEditorOverrideSpec, fieldReadOnlyOverrideSpec, fieldPriorityOverrideSpec);
        }
};

//===================================================================================
// @bsiclass
//===================================================================================
struct DirectPropertiesAppender : ContentPropertiesAppender
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldAttributes CreateFieldAttributes(ECPropertyCR ecProperty, PropertySpecificationsList const& overrides, bool isInNestedContent)
        {
        auto propertyOverrides = FindPropertySpecificationOverrides(overrides);

        return FieldAttributes(
            CreateFieldDisplayLabel(ecProperty, RelatedClassPath(), RelationshipMeaning::SameInstance, propertyOverrides.GetLabelOverrideSpecification()),
            CreatePropertiesFieldCategory(ecProperty, RelatedClassPath(), RelationshipMeaning::SameInstance, isInNestedContent, propertyOverrides.GetCategoryOverrideSpecification()),
            CreateFieldRenderer(ecProperty, propertyOverrides.GetRendererOverrideSpecification()),
            CreateFieldEditor(ecProperty, propertyOverrides.GetEditorOverrideSpecification()),
            CreateFieldReadOnlyFlag(ecProperty, propertyOverrides.GetReadOnlyOverrideSpecification()),
            CreateFieldPriority(ecProperty, propertyOverrides.GetPriorityOverrideSpecification()),
            false);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldCreateAction GetActionForPropertyField(ContentDescriptor::ECPropertiesField*& mergeField, bvector<ContentDescriptor::Field*> const& fields,
        ECPropertyCR ecProperty, Utf8CP propertyClassAlias, FieldAttributes const& fieldAttributes)
        {
        ContentDescriptor::ECPropertiesField* field = m_descriptor.FindECPropertiesField(ecProperty, m_actualPropertyClass,
            fieldAttributes.GetLabel(), fieldAttributes.GetCategory().get(), fieldAttributes.GetRenderer().get(), fieldAttributes.GetEditor().get());
        if (nullptr != field)
            {
            for (ContentDescriptor::Property const& prop : field->AsPropertiesField()->GetProperties())
                {
                // skip if already included in descriptor
                if (&ecProperty == &prop.GetProperty()
                    && &m_actualPropertyClass == &prop.GetPropertyClass()
                    && 0 == strcmp(propertyClassAlias, prop.GetPrefix()))
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Descriptor already contains a properties field with exact same property.");
                    return FieldCreateAction::Skip;
                    }
                }

            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Descriptor contains a properties field with similar properties.");
            mergeField = field->AsPropertiesField();
            return FieldCreateAction::Merge;
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Similar properties field was not found in descriptor.");
        return FieldCreateAction::Create;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECPropertiesField* GetPropertiesField(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationsList const& overrides)
        {
        FieldAttributes fieldAttributes = CreateFieldAttributes(ecProperty, overrides, false);

        ContentDescriptor::ECPropertiesField* field = nullptr;
        if (FieldCreateAction::Skip == GetActionForPropertyField(field, m_descriptor.GetAllFields(), ecProperty, propertyClassAlias, fieldAttributes))
            return nullptr;

        if (nullptr == field)
            {
            // did not find a field with similar properties - create a new one
            field = new ContentDescriptor::ECPropertiesField(fieldAttributes.GetCategory(), fieldAttributes.GetLabel(), fieldAttributes.GetRenderer().get(), fieldAttributes.GetEditor().get(),
                fieldAttributes.GetIsReadOnly(), fieldAttributes.GetPriority());
            }

        return field;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentSpecificationsHandler::PropertyAppendResult _DoAppendProperty(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationsList const& overrides) override
        {
        // get the field to append the property to (a new field will be created if necessary)
        ContentDescriptor::ECPropertiesField* field = GetPropertiesField(ecProperty, propertyClassAlias, overrides);
        if (nullptr == field)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Didn't find any properties field to append the property to - skip.");
            return false;
            }

        // append the property definition
        field->AddProperty(ContentDescriptor::Property(propertyClassAlias, m_actualPropertyClass, ecProperty));

        // if new field was created add it to descriptor
        if (1 == field->GetProperties().size())
            {
            m_descriptor.AddRootField(*field);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Appended the property to a new properties field and added it to descriptor.");
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Appended the property to an existing properties field.");
            }

        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    DirectPropertiesAppender(ContentDescriptorBuilder::Context& context, PropertyInfoStore const& propertyInfos, ContentDescriptorR descriptor,
        CategoriesSupplier categoriesSupplier, ECClassCR actualPropertyClass, PropertyCategorySpecificationsList const* scopePropertyCategories)
        : ContentPropertiesAppender(context, propertyInfos, descriptor, categoriesSupplier, actualPropertyClass, scopePropertyCategories)
        {}
};

//===================================================================================
// @bsiclass
//===================================================================================
struct RelatedContentPropertiesAppender : ContentPropertiesAppender
{
private:
    std::unique_ptr<ContentSpecificationsHandler::PropertyAppendResult::ReplacedRelationshipPath> m_pathReplaceInfo;
    ContentDescriptor::RelatedContentField& m_relatedContentField;
    std::function<void(ContentDescriptor::RelatedContentField&)> m_onPropertiesAppended;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldAttributes CreateFieldAttributes(ECPropertyCR ecProperty, PropertySpecificationsList const& overrides, bool isInNestedContent)
        {
        auto propertyOverrides = FindPropertySpecificationOverrides(overrides);

        return FieldAttributes(
            CreateFieldDisplayLabel(ecProperty, m_relatedContentField.GetPathFromSelectToContentClass(), m_relatedContentField.GetRelationshipMeaning(), propertyOverrides.GetLabelOverrideSpecification()),
            CreatePropertiesFieldCategory(ecProperty, m_relatedContentField.GetPathFromSelectToContentClass(), m_relatedContentField.GetRelationshipMeaning(), isInNestedContent, propertyOverrides.GetCategoryOverrideSpecification()),
            CreateFieldRenderer(ecProperty, propertyOverrides.GetRendererOverrideSpecification()),
            CreateFieldEditor(ecProperty, propertyOverrides.GetEditorOverrideSpecification()),
            CreateFieldReadOnlyFlag(ecProperty, propertyOverrides.GetReadOnlyOverrideSpecification()),
            CreateFieldPriority(ecProperty, propertyOverrides.GetPriorityOverrideSpecification()),
            false);
        }

    /*---------------------------------------------------------------------------------**//**
    * TODO: move to RelatedClassPath?
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static RelatedClassPath GetPathDifference(RelatedClassPathCR lhs, RelatedClassPathCR rhs)
        {
        RelatedClassPath diff;
        for (size_t i = rhs.size(); i < lhs.size(); ++i)
            diff.push_back(lhs[i]);
        return diff;
        }

    /*---------------------------------------------------------------------------------**//**
    * TODO: move to RelatedClassPath?
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool EndsWithSameRelatedClass(RelatedClassPathCR path, RelatedClassPathCR fieldPath)
        {
        if (path.size() != fieldPath.size())
            return false;

        for (size_t i = 0; i < path.size(); ++i)
            {
            if (&path[i].GetRelationship().GetClass() != &fieldPath[i].GetRelationship().GetClass())
                return false;
            }

        if (fieldPath.back().IsForwardRelationship() != path.back().IsForwardRelationship())
            return false;

        if (fieldPath.back().IsForwardRelationship()
            && &fieldPath.back().GetTargetClass().GetClass() == &path.back().GetTargetClass().GetClass()
            && fieldPath.back().GetTargetClass().IsSelectPolymorphic() == path.back().GetTargetClass().IsSelectPolymorphic())
            return true;

        if (!fieldPath.back().IsForwardRelationship() && fieldPath.back().GetSourceClass() == path.back().GetSourceClass())
            return true;

        return false;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Nullable<uint64_t> GetMaxTargetsCount(Nullable<uint64_t> lhs, Nullable<uint64_t> rhs)
        {
        bool lhsHasValue = lhs.IsValid();
        bool rhsHasValue = rhs.IsValid();
        if (!lhsHasValue && !rhsHasValue)
            return Nullable<uint64_t>();
        if (lhsHasValue && !rhsHasValue)
            return lhs.Value();
        if (!lhsHasValue && rhsHasValue)
            return rhs.Value();
        return std::max(lhs.Value(), rhs.Value());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static RelatedClassPath GetSimilarStepsBetweenPaths(RelatedClassPath& lhsPath, RelatedClassPath& rhsPath)
        {
        RelatedClassPath similarPath;

        for (size_t i = 0; i < lhsPath.size() && i < rhsPath.size(); ++i)
            {
            if (&lhsPath[i].GetTargetClass().GetClass() != &rhsPath[i].GetTargetClass().GetClass())
                break;
            if (lhsPath[i].Is(rhsPath[i]))
                {
                similarPath.push_back(rhsPath[i]);
                continue;
                }
            if (rhsPath[i].Is(lhsPath[i]))
                {
                similarPath.push_back(lhsPath[i]);
                continue;
                }
            break;
            }

        return similarPath;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct MergeWithExistingRelatedContentFieldResult
        {
        ContentDescriptor::RelatedContentField* field;
        std::unique_ptr<bpair<RelatedClassPath, RelatedClassPath>> pathReplacement;
        ContentDescriptor::RelatedContentField* nestingField;
        MergeWithExistingRelatedContentFieldResult() : field(nullptr), nestingField(nullptr) {}
        };
    static MergeWithExistingRelatedContentFieldResult MergeWithExistingRelatedContentField(RelatedClassPathR path, bvector<ContentDescriptor::Field*> const& fields,
        std::unordered_set<ECClassCP> const& sourceClasses, bool isRelationshipField)
        {
        MergeWithExistingRelatedContentFieldResult result;
        for (ContentDescriptor::Field* existingField : fields)
            {
            if (!existingField->IsNestedContentField())
                continue;

            ContentDescriptor::RelatedContentField* existingRelatedContentField = existingField->AsNestedContentField()->AsRelatedContentField();
            if (nullptr == existingRelatedContentField)
                continue;

            bool startsWith = false;
            std::shared_ptr<RelatedClassPath> replacePath;
            if (path.StartsWith(existingRelatedContentField->GetPathFromSelectToContentClass()))
                {
                startsWith = true;
                }
            else if (existingRelatedContentField->GetPathFromSelectToContentClass().size() <= path.size())
                {
                replacePath = std::make_shared<RelatedClassPath>(GetSimilarStepsBetweenPaths(existingRelatedContentField->GetPathFromSelectToContentClass(), path));
                if (replacePath->size() == existingRelatedContentField->GetPathFromSelectToContentClass().size())
                    startsWith = true;
                }
                
            if (startsWith)
                {
                // create separate relationship and related content fields even if paths are the same
                if (path.size() == existingRelatedContentField->GetPathFromSelectToContentClass().size() && existingRelatedContentField->IsRelationshipField() != isRelationshipField)
                    continue;

                // do not add any nested fields inside a relationship field
                if (path.size() > existingRelatedContentField->GetPathFromSelectToContentClass().size() && existingRelatedContentField->IsRelationshipField())
                    continue;
            
                if (replacePath != nullptr)
                    existingRelatedContentField->SetPathFromSelectToContentClass(*replacePath);

                // if m_pathFromSelectToPropertyClass starts with existing nested content field path, this field should be nested in existing field
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Found a related content field with similar start - merging");
                ContainerHelpers::Push(existingRelatedContentField->GetActualSourceClasses(), sourceClasses);
                RelatedClassPath prevPath = path;
                path = GetPathDifference(path, existingRelatedContentField->GetPathFromSelectToContentClass());
                if (path.empty())
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Paths are completely equal - reuse the related content field.");
                    result.field = existingRelatedContentField;
                    result.pathReplacement = std::make_unique<bpair<RelatedClassPath, RelatedClassPath>>(prevPath, existingRelatedContentField->GetPathFromSelectToContentClass());
                    result.pathReplacement->second.SetTargetsCount(GetMaxTargetsCount(existingRelatedContentField->GetPathFromSelectToContentClass().GetTargetsCount(), prevPath.GetTargetsCount()));
                    }
                else
                    {
                    path.SetTargetsCount(prevPath.GetTargetsCount());
                    auto nestedMerge = MergeWithExistingRelatedContentField(path, existingRelatedContentField->GetFields(), sourceClasses, isRelationshipField);
                    if (nestedMerge.field)
                        {
                        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Paths are similar - reuse the related content field.");
                        result.field = nestedMerge.field;
                        result.pathReplacement = std::make_unique<bpair<RelatedClassPath, RelatedClassPath>>(prevPath, RelatedClassPath::Combine(existingRelatedContentField->GetPathFromSelectToContentClass(), nestedMerge.pathReplacement->second));
                        }
                    else if (nestedMerge.nestingField)
                        {
                        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Use child field of existing related content field as parent field for the one we're going to create");
                        result.nestingField = nestedMerge.nestingField;
                        result.pathReplacement = std::make_unique<bpair<RelatedClassPath, RelatedClassPath>>(prevPath, RelatedClassPath::Combine(existingRelatedContentField->GetPathFromSelectToContentClass(), nestedMerge.pathReplacement->second));
                        }
                    else
                        {
                        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Use existing related content field as parent field for the one we're going to create");
                        result.nestingField = existingRelatedContentField;
                        result.pathReplacement = std::make_unique<bpair<RelatedClassPath, RelatedClassPath>>(prevPath, RelatedClassPath::Combine(existingRelatedContentField->GetPathFromSelectToContentClass(), path));
                        }
                    }
                break;
                }
            else if (EndsWithSameRelatedClass(path, existingRelatedContentField->GetPathFromSelectToContentClass()) && existingRelatedContentField->IsRelationshipField() == isRelationshipField)
                {
                // if m_pathFromSelectToPropertyClass and existing nested content field paths both target the same class, unify the paths and update the
                // existing field - no need to create a new one
                RelatedClassPath unifiedPath;
                if (SUCCESS == RelatedClassPath::Unify(unifiedPath, existingRelatedContentField->GetPathFromSelectToContentClass(), path))
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Found a related content field with the same target class - reuse the related content field after unifying the paths.");
                    RelatedClassPath prevFieldPath = existingRelatedContentField->GetPathFromSelectToContentClass();
                    unifiedPath.SetTargetsCount(GetMaxTargetsCount(existingRelatedContentField->GetPathFromSelectToContentClass().GetTargetsCount(), path.GetTargetsCount()));
                    existingRelatedContentField->SetPathFromSelectToContentClass(unifiedPath);
                    ContainerHelpers::Push(existingRelatedContentField->GetActualSourceClasses(), sourceClasses);
                    result.field = existingRelatedContentField;
                    result.pathReplacement = std::make_unique<bpair<RelatedClassPath, RelatedClassPath>>(prevFieldPath, unifiedPath);
                    break;
                    }
                }
            }
        return result;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static FieldAttributes CreateRelatedContentFieldAttributes(ECClassCR relatedContentClass, CategoriesSupplier& categoriesSupplier, bvector<RelatedPropertiesSpecification const*> const& relatedPropertySpecsStack)
        {
        return FieldAttributes(
            relatedContentClass.GetDisplayLabel(),
            categoriesSupplier.GetParentCategory(true),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            !relatedPropertySpecsStack.empty() ? relatedPropertySpecsStack.back()->ShouldAutoExpand() : false);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void BuildCategoriesStack(CategoriesSupplier& categoriesSupplier, bvector<RelatedPropertiesSpecification const*> const& relatedPropertySpecsStack, RelatedClassPathCR pathFromSelectToPropertyClass, bool isRelationshipClass)
        {
        size_t stepsCount = 0;
        for (size_t i = 0; i < relatedPropertySpecsStack.size(); ++i)
            {
            RelatedPropertiesSpecificationCP spec = relatedPropertySpecsStack[i];
            stepsCount += spec->GetPropertiesSource() ? spec->GetPropertiesSource()->GetSteps().size() : 1;
            if (stepsCount == 0 || stepsCount > pathFromSelectToPropertyClass.size())
                {
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Relationship path step count too large compared to relationship path length. "
                    "Steps: % " PRIu64 ". Path: % " PRIu64 ".", (uint64_t)stepsCount, (uint64_t)pathFromSelectToPropertyClass.size()));
                }

            bool isLastStep = (i == relatedPropertySpecsStack.size() - 1);

            ECClassCR relationshipClass = pathFromSelectToPropertyClass[stepsCount - 1].GetRelationship().GetClass();
            bool relationshipCategoryAlreadyExists = categoriesSupplier.HasSharedCategory(relationshipClass, spec->GetRelationshipMeaning());
            bool shouldCreateIntermediateRelationshipCategory = (!isLastStep && relationshipCategoryAlreadyExists) || spec->ShouldForceCreateRelationshipCategory();
            bool shouldCreateRelationshipCategoryForPropertyClass = RelationshipMeaning::RelatedInstance == spec->GetRelationshipMeaning() && isLastStep && isRelationshipClass;
            if (relationshipCategoryAlreadyExists || shouldCreateIntermediateRelationshipCategory || shouldCreateRelationshipCategoryForPropertyClass)
                categoriesSupplier.PushParentCategory(categoriesSupplier.GetRelatedContentFieldCategory(relationshipClass, spec->GetRelationshipMeaning()));

            bool shouldCreateTargetCategoryForPropertyClass = RelationshipMeaning::RelatedInstance == spec->GetRelationshipMeaning() && !isRelationshipClass;
            if (shouldCreateTargetCategoryForPropertyClass || !isLastStep)
                {
                ECClassCR relatedContentClass = pathFromSelectToPropertyClass[stepsCount - 1].GetTargetClass().GetClass();
                categoriesSupplier.PushParentCategory(categoriesSupplier.GetRelatedContentFieldCategory(relatedContentClass, spec->GetRelationshipMeaning()));
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void RemoveRelationshipFieldsCategoryIfUnused(ContentDescriptor::RelatedContentField& field, CategoriesSupplierContext& categoriesSupplierContext, bvector<RelatedPropertiesSpecification const*> const& relatedPropertySpecsStack)
        {
        bool fieldHasRelationshipCategory = field.IsRelationshipField() && relatedPropertySpecsStack.back()->GetRelationshipMeaning() == RelationshipMeaning::RelatedInstance;
        if (fieldHasRelationshipCategory && field.GetFields().empty() && !relatedPropertySpecsStack.back()->ShouldForceCreateRelationshipCategory())
            {
            auto it = categoriesSupplierContext.GetAllCategories().find(std::make_pair(field.GetCategory()->GetName(), field.GetCategory()->GetParentCategory()));
            if (categoriesSupplierContext.GetAllCategories().end() != it)
                {
                it->second->SetParentCategory(nullptr);
                categoriesSupplierContext.GetAllCategories().erase(it);
                field.SetCategory(nullptr);
                }
            }
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentSpecificationsHandler::PropertyAppendResult _DoAppendProperty(ECPropertyCR ecProperty, Utf8CP alias, PropertySpecificationsList const& overrides) override
        {
        Utf8String propertySourceAlias(alias);

        std::unique_ptr<ContentSpecificationsHandler::PropertyAppendResult::ReplacedRelationshipPath> replacedPathFromSelectToPropertyClass;
        if (m_pathReplaceInfo)
            {
            replacedPathFromSelectToPropertyClass = std::make_unique<ContentSpecificationsHandler::PropertyAppendResult::ReplacedRelationshipPath>(*m_pathReplaceInfo);
            if (!ecProperty.GetIsNavigation())
                propertySourceAlias = m_relatedContentField.IsRelationshipField() ? m_relatedContentField.GetRelationshipClassAlias() : m_relatedContentField.GetContentClassAlias();
            }

        FieldAttributes fieldAttributes = CreateFieldAttributes(ecProperty, overrides, true);
        ContentDescriptor::ECPropertiesField propertyField(fieldAttributes.GetCategory(), fieldAttributes.GetLabel(), fieldAttributes.GetRenderer().get(), fieldAttributes.GetEditor().get());
        propertyField.AddProperty(ContentDescriptor::Property(propertySourceAlias, m_actualPropertyClass, ecProperty));

        for (ContentDescriptor::Field* nestedField : m_relatedContentField.GetFields())
            {
            if (*nestedField == propertyField)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Related content field already contains a similar property field - skip.");
                return ContentSpecificationsHandler::PropertyAppendResult(false, std::move(replacedPathFromSelectToPropertyClass));
                }
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Appended property field under the related content field.");
        m_relatedContentField.GetFields().push_back(new ContentDescriptor::ECPropertiesField(propertyField));
        m_relatedContentField.GetFields().back()->SetParent(&m_relatedContentField);
        return ContentSpecificationsHandler::PropertyAppendResult(true, std::move(replacedPathFromSelectToPropertyClass));
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    RelatedContentPropertiesAppender(ContentDescriptorBuilder::Context& context, PropertyInfoStore const& propertyInfos, ContentDescriptorR descriptor,
        CategoriesSupplier categoriesSupplier, ContentDescriptor::RelatedContentField& relatedContentField, ECClassCR actualPropertyClass,
        PropertyCategorySpecificationsList const* scopePropertyCategories, std::unique_ptr<ContentSpecificationsHandler::PropertyAppendResult::ReplacedRelationshipPath> pathReplaceInfo,
        std::function<void(ContentDescriptor::RelatedContentField&)> onPropertiesAppended)
        : ContentPropertiesAppender(context, propertyInfos, descriptor, categoriesSupplier, actualPropertyClass, scopePropertyCategories),
        m_relatedContentField(relatedContentField), m_pathReplaceInfo(std::move(pathReplaceInfo)), m_onPropertiesAppended(onPropertiesAppended)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~RelatedContentPropertiesAppender()
        {
        if (m_onPropertiesAppended)
            m_onPropertiesAppended(m_relatedContentField);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct GetOrCreateRelatedContentFieldResult
        {
        ContentDescriptor::RelatedContentField* field;
        std::unique_ptr<bpair<RelatedClassPath, RelatedClassPath>> pathReplacement;
        std::function<void(ContentDescriptor::RelatedContentField&)> onPropertiesAppended;
        };
    static GetOrCreateRelatedContentFieldResult GetOrCreateRelatedContentField(ContentDescriptorR descriptor,
        CategoriesSupplier& categoriesSupplier, bvector<RelatedPropertiesSpecification const*> const& relatedPropertySpecsStack,
        RelatedClassPathCR pathFromSelectToPropertyClass, ECClassCR actualPropertyClass, std::unordered_set<ECClassCP> const& actualSourceClasses)
        {
        auto scope = Diagnostics::Scope::Create("Get or create related content field");

        // attempt to find a content field that the new nested field should be nested in
        RelatedClassPath pathFromSelectToContentClass = pathFromSelectToPropertyClass;
        auto mergeResult = MergeWithExistingRelatedContentField(pathFromSelectToContentClass, descriptor.GetAllFields(), actualSourceClasses, actualPropertyClass.IsRelationshipClass());

        BuildCategoriesStack(categoriesSupplier, relatedPropertySpecsStack, mergeResult.pathReplacement ? mergeResult.pathReplacement->second : pathFromSelectToPropertyClass, actualPropertyClass.IsRelationshipClass());

        // use the field we found
        if (mergeResult.field)
            {
            if (relatedPropertySpecsStack.back()->ShouldSkipIfDuplicate())
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Found a similar related content field and we're skipping duplicate fields - skip creating the related content field.");
                return { nullptr, nullptr, nullptr };
                }

            if (mergeResult.field->GetCategory() == nullptr)
                mergeResult.field->SetCategory(categoriesSupplier.GetParentCategory(true));

            if (mergeResult.pathReplacement)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Merged the field with existing one. Replaced path from `%s` to `%s`.",
                    DiagnosticsHelpers::CreateRelatedClassPathStr(mergeResult.pathReplacement->first).c_str(), DiagnosticsHelpers::CreateRelatedClassPathStr(mergeResult.pathReplacement->second).c_str()));
                }
            else
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Merged the field with existing one at path `%s`.",
                    DiagnosticsHelpers::CreateRelatedClassPathStr(pathFromSelectToContentClass).c_str()));
                }

            auto onPropertiesAppended = [&categoriesSupplierContext = categoriesSupplier.GetContext(), &relatedPropertySpecsStack](ContentDescriptor::RelatedContentField& field)
                {
                RemoveRelationshipFieldsCategoryIfUnused(field, categoriesSupplierContext, relatedPropertySpecsStack);
                };
            return { mergeResult.field, std::move(mergeResult.pathReplacement), onPropertiesAppended };
            }

        // create the field
        FieldAttributes fieldAttributes = CreateRelatedContentFieldAttributes(actualPropertyClass, categoriesSupplier, relatedPropertySpecsStack);

        ContentDescriptor::RelatedContentField* relatedContentField = new ContentDescriptor::RelatedContentField(fieldAttributes.GetCategory(),
            fieldAttributes.GetLabel(), pathFromSelectToContentClass, bvector<ContentDescriptor::Field*>(), fieldAttributes.ShouldAutoExpand(),
            ContentDescriptor::Property::DEFAULT_PRIORITY, actualPropertyClass.IsRelationshipClass());

        relatedContentField->SetActualSourceClasses(actualSourceClasses);
        relatedContentField->SetRelationshipMeaning(relatedPropertySpecsStack.back()->GetRelationshipMeaning());

        if (fieldAttributes.GetRenderer())
            relatedContentField->SetRenderer(new ContentFieldRenderer(*fieldAttributes.GetRenderer()));

        if (fieldAttributes.GetEditor())
            relatedContentField->SetEditor(new ContentFieldEditor(*fieldAttributes.GetEditor()));

        auto onPropertiesAppended = [&descriptor, nestingField = mergeResult.nestingField, &categoriesSupplierContext = categoriesSupplier.GetContext(), &relatedPropertySpecsStack]
            (ContentDescriptor::RelatedContentField& field)
            {
            RemoveRelationshipFieldsCategoryIfUnused(field, categoriesSupplierContext, relatedPropertySpecsStack);

            if (nestingField)
                {
                // if the field is nested, set the parent and push the field as a child
                field.SetParent(nestingField);
                nestingField->GetFields().push_back(&field);
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Created a new field and merged it under `%s`.", nestingField->GetUniqueName().c_str()));
                }
            else
                {
                // if the field is not nested, just append it to the descriptor
                descriptor.AddRootField(field);
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Created a new field and added it as a root field.");
                }
            };
        return { relatedContentField, std::move(mergeResult.pathReplacement), onPropertiesAppended };
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignFieldNames(bmap<Utf8String, uint64_t>& requestedNameCounts, bvector<ContentDescriptor::Field*> const& fields)
    {
    for (ContentDescriptor::Field* field : fields)
        {
        if (field->GetUniqueName().empty())
            {
            Utf8String requestedName = field->CreateName();
            auto iter = requestedNameCounts.find(requestedName);
            if (requestedNameCounts.end() == iter)
                {
                requestedNameCounts.Insert(requestedName, 1);
                field->SetUniqueName(requestedName);
                }
            else
                {
                ++iter->second;
                field->SetUniqueName(Utf8String(requestedName).append("_").append(std::to_string(iter->second)));
                }
            }
        if (field->IsNestedContentField())
            AssignFieldNames(requestedNameCounts, field->AsNestedContentField()->GetFields());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
struct CategoriesPtrComparer
    {
    bool operator()(T const& lhs, T const& rhs) const
        {
        return lhs->GetName().CompareTo(rhs->GetName()) < 0;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilderImpl : ContentSpecificationsHandler
{
private:
    ContentDescriptorPtr m_descriptor;
    ContentSpecificationCP m_specification;
    std::unique_ptr<PropertyInfoStore> m_propertyInfos;
    std::unique_ptr<CategoriesSupplierContext> m_categoriesSupplierContext;
    bool m_isRecursiveSpecification;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorBuilder::Context& GetContext() {return static_cast<ContentDescriptorBuilder::Context&>(ContentSpecificationsHandler::GetContext());}
    ContentDescriptorBuilder::Context const& GetContext() const { return static_cast<ContentDescriptorBuilder::Context const&>(ContentSpecificationsHandler::GetContext()); }

    /*---------------------------------------------------------------------------------**//**
     * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t AddCalculatedFields(CalculatedPropertiesSpecificationList const& calculatedProperties, ECClassCP ecClass)
        {
        size_t count = 0;
        for (size_t i = 0; i < calculatedProperties.size(); i++)
            {
            Utf8String label = calculatedProperties[i]->GetLabel();

            Utf8String propertyName = Utf8String("CalculatedProperty_").append(std::to_string(i).c_str());
            for (ContentDescriptor::Field const* field : m_descriptor->GetAllFields())
                {
                if (!field->IsCalculatedPropertyField())
                    continue;

                ContentDescriptor::CalculatedPropertyField const* existingCalculatedField = field->AsCalculatedPropertyField();
                if (existingCalculatedField->GetRequestedName() == propertyName && nullptr != ecClass && ecClass->Is(existingCalculatedField->GetClass()))
                    return count;
                }

            auto category = CategoriesSupplier(*m_categoriesSupplierContext).GetCalculatedFieldCategory();
            m_descriptor->AddRootField(*new ContentDescriptor::CalculatedPropertyField(category, label, propertyName,
                calculatedProperties[i]->GetValue(), ecClass, calculatedProperties[i]->GetPriority()));
            ++count;
            }
        return count;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t AddCalculatedFieldsFromContentModifiers(ECClassCR ecClass)
        {
        size_t count = 0;
        for (ContentModifierCP modifier : GetContext().GetRulesPreprocessor().GetContentModifiers())
            {
            ECClassCP modifierClass = GetContext().GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            if (ecClass.Is(modifierClass))
                {
                DiagnosticsHelpers::ReportRule(*modifier);
                count += AddCalculatedFields(modifier->GetCalculatedProperties(), modifierClass);
                }
            }
        return count;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void RemoveEmptyChildNestedFields(ContentDescriptor::NestedContentField& parentField)
        {
        for (auto it = parentField.GetFields().rbegin(); it != parentField.GetFields().rend(); ++it)
            {
            ContentDescriptor::Field* field = *it;
            if (!field->IsNestedContentField())
                continue;
            RemoveEmptyChildNestedFields(*(field->AsNestedContentField()));
            if (field->AsNestedContentField()->GetFields().empty())
                parentField.RemoveField((it + 1).base());
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void RemoveUnusedChildCategories(std::shared_ptr<ContentDescriptor::Category>& parentCategory)
        {
        for (auto it = parentCategory->GetChildCategories().rbegin(); it != parentCategory->GetChildCategories().rend(); ++it)
            RemoveCategoryFromParentIfNotUsed(*it);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool RemoveCategoryFromParentIfNotUsed(std::shared_ptr<ContentDescriptor::Category>& category)
        {
        RemoveUnusedChildCategories(category);
        auto categoryUseCount = category.use_count();
        if (category->GetChildCategories().empty() && (categoryUseCount <= 1 || (categoryUseCount <= 2 && category->GetParentCategory())))
            {
            // Remove category if there are no fields using this category - in that case `categoryUseCount = 1` or `categoryUseCount = 2`:
            // 1 ref from `m_categoriesSupplierContext->GetAllCategories()`, 1 ref from parent category if it exists.
            category->SetParentCategory(nullptr);
            return true;
            }
        return false;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<std::unique_ptr<RelatedPropertySpecificationPaths>> _GetRelatedPropertyPaths(RelatedPropertyPathsParams const& params) const override
        {
        if (ShouldCreateFields(*m_descriptor))
            return ContentSpecificationsHandler::_GetRelatedPropertyPaths(params);
        return bvector<std::unique_ptr<RelatedPropertySpecificationPaths>>();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppenderPtr _CreatePropertyAppender(std::unordered_set<ECClassCP> const& actualSourceClasses, RelatedClassPathCR pathFromSelectToPropertyClass, ECClassCR propertyClass,
        bvector<RelatedPropertiesSpecification const*> const& relatedPropertyStack, PropertyCategorySpecificationsList const* categorySpecifications) override
        {
        CategoriesSupplier categoriesSupplier(*m_categoriesSupplierContext);

        if (pathFromSelectToPropertyClass.empty())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Path from select to property class is empty - returning direct properties appender.");
            return new DirectPropertiesAppender(GetContext(), *m_propertyInfos, *m_descriptor, categoriesSupplier, propertyClass, categorySpecifications);
            }

        auto relatedContentField = RelatedContentPropertiesAppender::GetOrCreateRelatedContentField(*m_descriptor, categoriesSupplier, relatedPropertyStack,
            pathFromSelectToPropertyClass, propertyClass, actualSourceClasses);
        if (!relatedContentField.field)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Failed to get or create a related content field - returning NULL property appender.");
            return nullptr;
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, "Returning a related content properties appender");

        std::unique_ptr<ContentSpecificationsHandler::PropertyAppendResult::ReplacedRelationshipPath> replacedPathFromSelectToPropertyClass;
        if (relatedContentField.pathReplacement)
            replacedPathFromSelectToPropertyClass = std::make_unique<ContentSpecificationsHandler::PropertyAppendResult::ReplacedRelationshipPath>(relatedContentField.pathReplacement->first, relatedContentField.pathReplacement->second);

        return new RelatedContentPropertiesAppender(GetContext(), *m_propertyInfos, *m_descriptor, categoriesSupplier, *relatedContentField.field,
            propertyClass, categorySpecifications, std::move(replacedPathFromSelectToPropertyClass), relatedContentField.onPropertiesAppended);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _OnContentAppended() override
        {
        for (auto it = m_descriptor->GetAllFields().rbegin(); it != m_descriptor->GetAllFields().rend(); ++it)
            {
            ContentDescriptor::Field* field = *it;
            if (!field->IsNestedContentField())
                continue;
            RemoveEmptyChildNestedFields(*(field->AsNestedContentField()));
            if (field->AsNestedContentField()->GetFields().empty())
                m_descriptor->RemoveRootField((it + 1).base());
            }

        for (auto it = m_categoriesSupplierContext->GetAllCategories().begin(); it != m_categoriesSupplierContext->GetAllCategories().end();)
            {
            bool shouldRemoveCategoryFromContext = RemoveCategoryFromParentIfNotUsed(it->second);
            if (shouldRemoveCategoryFromContext)
                it = m_categoriesSupplierContext->GetAllCategories().erase(it);
            else
                ++it;
            }

        if (nullptr != m_specification && ShouldCreateFields(*m_descriptor))
            {
            int count = AddCalculatedFields(m_specification->GetCalculatedProperties(), nullptr);
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Added %" PRIu64 " fields from specification.", (uint64_t)count));
            }

        for (auto const& category : m_categoriesSupplierContext->GetAllCategories())
            m_descriptor->GetCategories().push_back(category.second);

        bmap<Utf8String, uint64_t> requestedNameCounts;
        AssignFieldNames(requestedNameCounts, m_descriptor->GetAllFields());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ContentSource> _BuildContentSource(bvector<RelatedClassPath> const& paths, ContentSpecificationCR spec) override
        {
        if (m_isRecursiveSpecification)
            {
            // ContentSpecificationsHandler::_BuildContentSource splits paths into
            // derived paths based on content modifiers. Don't do that for recursive selects.
            bvector<ContentSource> sources;
            for (RelatedClassPathCR path : paths)
                ContainerHelpers::Push(sources, CreateContentSources(path, spec));
            return sources;
            }

        return ContentSpecificationsHandler::_BuildContentSource(paths, spec);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AppendClass(SelectClassInfo const& classInfo) override
        {
        m_descriptor->AddSelectClass(classInfo, m_specification ? m_specification->GetHash() : "");

        if (ShouldCreateFields(*m_descriptor))
            {
            size_t count = AddCalculatedFieldsFromContentModifiers(classInfo.GetSelectClass().GetClass());
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Added %" PRIu64 " fields from content modifiers.", (uint64_t)count));
            }
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorBuilderImpl(ContentDescriptorBuilder::Context& context, ContentSpecificationCP specification)
        : ContentSpecificationsHandler(context), m_specification(specification), m_isRecursiveSpecification(false)
        {
        m_propertyInfos = std::make_unique<PropertyInfoStore>(GetContext().GetSchemaHelper(), GetContext().GetRulesPreprocessor().GetContentModifiers(), specification);
        m_categoriesSupplierContext = std::make_unique<CategoriesSupplierContext>(context, *m_propertyInfos);

        m_descriptor = ContentDescriptor::Create(GetContext().GetConnection(), context.GetRuleset(), context.GetRulesetVariables(), GetContext().GetInputKeys());
        m_descriptor->SetUnitSystem(context.GetUnitSystem());
        m_descriptor->SetPreferredDisplayType(GetContext().GetPreferredDisplayType());
        m_descriptor->SetContentFlags(specification ? _GetContentFlags(*specification) : GetContext().GetContentFlagsCalculator() ? context.GetContentFlagsCalculator()(0) : 0);
        if (nullptr != GetContext().GetSelectionInfo())
            m_descriptor->SetSelectionInfo(*GetContext().GetSelectionInfo());

        if (nullptr == m_descriptor->GetDisplayLabelField())
            {
            auto fieldCategory = CategoriesSupplier(*m_categoriesSupplierContext).GetCalculatedFieldCategory();
            ContentDescriptor::DisplayLabelField* field = new ContentDescriptor::DisplayLabelField(fieldCategory, CommonStrings::ECPRESENTATION_DISPLAYLABEL);
            field->SetLabelOverrideSpecs(QueryBuilderHelpers::GetLabelOverrideValuesMap(GetContext().GetSchemaHelper(), GetContext().GetRulesPreprocessor().GetInstanceLabelOverrides()));
            m_descriptor->AddRootField(*field);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Added display label field.");
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(SelectedNodeInstancesSpecificationCR spec, IParsedInput const& specificationInput)
        {
        ContentSpecificationsHandler::HandleSpecification(spec, specificationInput);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(ContentRelatedInstancesSpecificationCR spec, IParsedInput const& specificationInput)
        {
        m_isRecursiveSpecification = spec.IsRecursive();
        ContentSpecificationsHandler::HandleSpecification(spec, specificationInput);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR spec)
        {
        ContentSpecificationsHandler::HandleSpecification(spec);
        }

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct NestedContentFieldDataContext : ContentDescriptorBuilder::Context
    {
    private:
        bset<std::shared_ptr<ContentDescriptor::Category const>, CategoriesPtrComparer<std::shared_ptr<ContentDescriptor::Category const>>> m_categories;
        bvector<RelatedClass> m_navigationPropertyClasses;
        bvector<RelatedClassPath> m_relatedPropertyPaths;
    public:
        NestedContentFieldDataContext(NestedContentFieldDataContext const& other) : ContentDescriptorBuilder::Context(other) {/* note: we don't want to copy private members */}
        NestedContentFieldDataContext(ContentDescriptorBuilder::Context const& other) : ContentDescriptorBuilder::Context(other) {}
        bset<std::shared_ptr<ContentDescriptor::Category const>, CategoriesPtrComparer<std::shared_ptr<ContentDescriptor::Category const>>>& GetCategories() {return m_categories;}
        bvector<RelatedClass>& GetNavigationPropertyClasses() {return m_navigationPropertyClasses;}
        bvector<RelatedClassPath>& GetRelatedPropertyPaths() {return m_relatedPropertyPaths;}
    };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void GatherNestedContentFieldData(NestedContentFieldDataContext& context, ContentDescriptor::NestedContentField const& parent)
        {
        for (ContentDescriptor::Field const* field : parent.GetFields())
            {
            auto category = field->GetCategory() ? field->GetCategory() : parent.GetCategory();
            if (category)
                context.GetCategories().insert(category);

            if (field->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const* propertiesField = field->AsPropertiesField();
                ContentDescriptor::Property const& prop = propertiesField->GetProperties().back();
                if (prop.GetProperty().GetIsNavigation())
                    {
                    RelatedClass navigationPropRelatedClass = context.GetSchemaHelper().GetForeignKeyClass(prop.GetProperty());
                    navigationPropRelatedClass.GetRelationship().SetAlias(context.CreateNavigationClassAlias(navigationPropRelatedClass.GetRelationship().GetClass()));
                    navigationPropRelatedClass.GetTargetClass().SetAlias(prop.GetPrefix());
                    context.GetNavigationPropertyClasses().push_back(navigationPropRelatedClass);
                    }
                }
            else if (field->IsNestedContentField() && field->AsNestedContentField()->AsRelatedContentField())
                {
                ContentDescriptor::RelatedContentField const* relatedContentField = field->AsNestedContentField()->AsRelatedContentField();
                context.GetRelatedPropertyPaths().push_back(relatedContentField->GetPathFromSelectToContentClass());

                NestedContentFieldDataContext relatedContentData(context);
                GatherNestedContentFieldData(relatedContentData, *relatedContentField);
                ContainerHelpers::Push(context.GetCategories(), relatedContentData.GetCategories());
                ContainerHelpers::Push(context.GetRelatedPropertyPaths(), ContainerHelpers::TransformContainer<bvector<RelatedClassPath>>(relatedContentData.GetNavigationPropertyClasses(),
                    [&](RelatedClassCR navigationPropertyClass){return RelatedClassPath::Combine(relatedContentField->GetPathFromSelectToContentClass(), navigationPropertyClass);}));
                ContainerHelpers::Push(context.GetRelatedPropertyPaths(), ContainerHelpers::TransformContainer<bvector<RelatedClassPath>>(relatedContentData.GetRelatedPropertyPaths(),
                    [&](RelatedClassPathCR relatedPropertyPath){return RelatedClassPath::Combine(relatedContentField->GetPathFromSelectToContentClass(), relatedPropertyPath);}));
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleContentField(ContentDescriptor::NestedContentField const& contentField)
        {
        for (ContentDescriptor::Field const* field : contentField.GetFields())
            {
            m_descriptor->AddRootField(*field->Clone());
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Cloned nested field `%s` into descriptor.", field->GetUniqueName().c_str()));
            }
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Total fields: %" PRIu64, (uint64_t)m_descriptor->GetAllFields().size()));

        NestedContentFieldDataContext nestedContentFieldData(GetContext());
        GatherNestedContentFieldData(nestedContentFieldData, contentField);

        ContainerHelpers::Push(m_descriptor->GetCategories(), nestedContentFieldData.GetCategories());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Total categories: %" PRIu64, (uint64_t)m_descriptor->GetCategories().size()));

        SelectClassInfo selectInfo(SelectClassWithExcludes<ECClass>(contentField.GetContentClass(), contentField.GetContentClassAlias()));
        ContainerHelpers::Push(selectInfo.GetNavigationPropertyClasses(), nestedContentFieldData.GetNavigationPropertyClasses());
        ContainerHelpers::Push(selectInfo.GetRelatedPropertyPaths(), nestedContentFieldData.GetRelatedPropertyPaths());
        m_descriptor->AddSelectClass(selectInfo, m_specification ? m_specification->GetHash() : "");
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_DEBUG, Utf8PrintfString("Select class: `%s`.", selectInfo.GetSelectClass().GetClass().GetFullName()));

        bmap<Utf8String, uint64_t> requestedNameCounts;
        AssignFieldNames(requestedNameCounts, m_descriptor->GetAllFields());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorPtr GetDescriptor()
        {
        if (m_descriptor->GetSelectClasses().empty())
            return nullptr;

        return m_descriptor;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(SelectedNodeInstancesSpecificationCR specification, IParsedInput const& specificationInput) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create content descriptor for %s: ", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification, specificationInput);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentRelatedInstancesSpecificationCR specification, IParsedInput const& specificationInput) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create content descriptor for %s: ", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification, specificationInput);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentInstancesOfSpecificClassesSpecificationCR specification) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create content descriptor for %s: ", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentDescriptor::NestedContentField const& contentField) const
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create content descriptor for nested content field: ", contentField.GetUniqueName().c_str()));
    ContentDescriptorBuilderImpl builder(m_context, nullptr);
    builder.HandleContentField(contentField);
    return builder.GetDescriptor();
    }
