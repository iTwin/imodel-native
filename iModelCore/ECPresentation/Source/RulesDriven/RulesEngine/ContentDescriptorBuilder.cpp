/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "QueryBuilder.h"
#include "LoggingHelper.h"
#include "PropertyInfoStore.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyFieldLocalization(ContentDescriptor::Field& field, ContentDescriptorBuilder::Context const& context)
    {
    if (!context.GetLocalizationProvider())
        return;

    LocalizationHelper helper(*context.GetLocalizationProvider(), context.GetLocale(), &context.GetRuleset());

    Utf8String fieldLabel = field.GetLabel();
    if (helper.LocalizeString(fieldLabel))
        field.SetLabel(fieldLabel);

    Utf8String categoryLabel = field.GetCategory().GetLabel();
    if (helper.LocalizeString(categoryLabel))
        {
        ContentDescriptor::Category localizedCategory(field.GetCategory());
        localizedCategory.SetLabel(categoryLabel);
        field.SetCategory(localizedCategory);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldCreateFields(ContentDescriptorCR descriptor)
    {
    return !descriptor.HasContentFlag(ContentFlags::NoFields)
        && !descriptor.HasContentFlag(ContentFlags::KeysOnly);
    }

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct ContentPropertiesAppender : ContentSpecificationsHandler::PropertyAppender
{
    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    enum class FieldCreateAction
        {
        Skip,
        Merge,
        Create,
        };

    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            10/2019
    //===================================================================================
    struct FieldAttributes
    {
    private:
        Utf8String m_label;
        ContentDescriptor::Category m_category;
        std::shared_ptr<ContentFieldEditor const> m_editor;
    public:
        FieldAttributes(Utf8String label, ContentDescriptor::Category category, std::shared_ptr<ContentFieldEditor const> editor)
            : m_label(label), m_category(category), m_editor(editor)
            {}
        Utf8StringCR GetLabel() const {return m_label;}
        ContentDescriptor::Category const& GetCategory() const {return m_category;}
        std::shared_ptr<ContentFieldEditor const> GetEditor() const {return m_editor;}
    };

private:
    ContentDescriptorBuilder::Context& m_context;
    PropertyInfoStore const& m_propertyInfos;
    ContentDescriptorR m_descriptor;
    ECClassCR m_actualClass;
    RelatedClassPath const& m_pathFromSelectToPropertyClass;
    RelationshipMeaning m_relationshipMeaning;
    ContentDescriptor::ECInstanceKeyField* m_keyField;
    ContentDescriptor::RelatedContentField* m_nestedContentField;
    bool m_expandNestedFields;
    PropertyCategorySpecificationsList const* m_scopePropertyCategories;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CreateFieldDisplayLabel(ECPropertyCR ecProperty, PropertySpecificationCP overrides) const
        {
        Utf8String labelOverride = m_propertyInfos.GetLabelOverride(ecProperty, m_actualClass, overrides);
        if (!labelOverride.empty())
            return labelOverride;

        Utf8String displayLabel;
        if (nullptr != m_context.GetPropertyFormatter() && SUCCESS == m_context.GetPropertyFormatter()->GetFormattedPropertyLabel(displayLabel, ecProperty, m_actualClass, m_pathFromSelectToPropertyClass, m_relationshipMeaning))
            return displayLabel;

        if (!m_pathFromSelectToPropertyClass.empty() && RelationshipMeaning::SameInstance != m_relationshipMeaning && nullptr == m_nestedContentField)
            return Utf8String(m_actualClass.GetDisplayLabel()).append(" ").append(ecProperty.GetDisplayLabel());

        return ecProperty.GetDisplayLabel();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::Category CreateFieldCategory(ECPropertyCR ecProperty, PropertySpecificationCP overrides, std::function<ContentDescriptor::Category()> const& defaultCategoryFactory) const
        {
        ContentDescriptor::Category categoryOverride = m_propertyInfos.GetCategoryOverride(ecProperty, m_actualClass, overrides, m_scopePropertyCategories);
        if (categoryOverride.IsValid())
            return categoryOverride;
        return defaultCategoryFactory();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::shared_ptr<ContentFieldEditor const> CreateFieldEditor(ECPropertyCR ecProperty, PropertySpecificationCP overrides) const
        {
        return m_propertyInfos.GetPropertyEditor(ecProperty, m_actualClass, overrides);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldAttributes CreateFieldAttributes(ECPropertyCR ecProperty, PropertySpecificationCP overrides, std::function<ContentDescriptor::Category()> const& defaultCategoryFactory) const
        {
        return FieldAttributes(
            CreateFieldDisplayLabel(ecProperty, overrides),
            CreateFieldCategory(ecProperty, overrides, defaultCategoryFactory),
            CreateFieldEditor(ecProperty, overrides)
            );
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECInstanceKeyField* FindKeyField(ContentDescriptor::ECPropertiesField const& propertiesField) const
        {
        for (ContentDescriptor::ECInstanceKeyField* keyField : m_descriptor.GetECInstanceKeyFields())
            {
            for (ContentDescriptor::ECPropertiesField const* keyedField : keyField->GetKeyFields())
                {
                if (keyedField == &propertiesField)
                    return keyField;
                }
            }
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldCreateAction GetActionForPropertyField(ContentDescriptor::ECPropertiesField*& mergeField, bvector<ContentDescriptor::Field*> const& fields,
        ECPropertyCR ecProperty, Utf8CP propertyClassAlias, FieldAttributes const& fieldAttributes)
        {
        ContentDescriptor::ECPropertiesField* field = m_descriptor.FindECPropertiesField(ecProperty, m_actualClass, m_pathFromSelectToPropertyClass,
            fieldAttributes.GetLabel(), fieldAttributes.GetCategory(), fieldAttributes.GetEditor().get());
        if (nullptr != field)
            {
            for (ContentDescriptor::Property const& prop : field->AsPropertiesField()->GetProperties())
                {
                // skip if already included in descriptor
                if (&ecProperty == &prop.GetProperty()
                    && &m_actualClass == &prop.GetPropertyClass()
                    && 0 == strcmp(propertyClassAlias, prop.GetPrefix()))
                    {
                    return FieldCreateAction::Skip;
                    }
                }

            mergeField = field->AsPropertiesField();
            m_keyField = FindKeyField(*mergeField);
            return FieldCreateAction::Merge;
            }

        return FieldCreateAction::Create;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECPropertiesField* GetPropertiesField(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationCP overrides)
        {
        FieldAttributes fieldAttributes = CreateFieldAttributes(ecProperty, overrides, [&]() {return m_context.GetCategorySupplier().CreateCategory(m_actualClass, ecProperty, m_relationshipMeaning);});

        ContentDescriptor::ECPropertiesField* field = nullptr;
        if (FieldCreateAction::Skip == GetActionForPropertyField(field, m_descriptor.GetAllFields(), ecProperty, propertyClassAlias, fieldAttributes))
            return nullptr;

        // did not find a field with similar properties - create a new one
        if (nullptr == field)
            {
            if (!m_keyField && !m_pathFromSelectToPropertyClass.empty() && !ecProperty.GetIsNavigation())
                {
                // only create related instance key field for non-navigation properties
                m_keyField = new ContentDescriptor::ECInstanceKeyField();
                m_descriptor.AddField(m_keyField);
                }
            field = new ContentDescriptor::ECPropertiesField(fieldAttributes.GetCategory(), fieldAttributes.GetLabel(), fieldAttributes.GetEditor().get());
            ApplyFieldLocalization(*field, m_context);
            }

        return field;
        }

    /*---------------------------------------------------------------------------------**//**
    * TODO: move to RelatedClassPath?
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool PathStartsWith(RelatedClassPathCR path, RelatedClassPathCR sub)
        {
        if (sub.size() > path.size())
            return false;

        for (size_t i = 0; i < sub.size(); ++i)
            {
            if (sub[i] != path[i])
                return false;
            }
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * TODO: move to RelatedClassPath?
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static RelatedClassPath GetPathDifference(RelatedClassPathCR lhs, RelatedClassPathCR rhs)
        {
        BeAssert(PathStartsWith(lhs, rhs));
        RelatedClassPath diff;
        for (size_t i = rhs.size(); i < lhs.size(); ++i)
            diff.push_back(lhs[i]);
        return diff;
        }

    /*---------------------------------------------------------------------------------**//**
    * TODO: move to RelatedClassPath?
    * @bsimethod                                    Mantas.Kontrimas                02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool EndsWithSameRelatedClass(RelatedClassPathCR path, RelatedClassPathCR fieldPath)
        {
        if (path.size() != fieldPath.size())
            return false;

        for (size_t i = 0; i < path.size(); ++i)
            {
            if (path[i].GetRelationship() != fieldPath[i].GetRelationship())
                return false;
            }

        if ((fieldPath.back().IsForwardRelationship() && fieldPath.back().GetTargetClass() == path.back().GetTargetClass()) ||
            (!fieldPath.back().IsForwardRelationship() && fieldPath.back().GetSourceClass() == path.back().GetSourceClass()))
            return true;

        return false;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::RelatedContentField* GetXToManyNestedContentField()
        {
        if (nullptr == m_nestedContentField)
            {
            // attempt to find a content field that the new nested field should be nested in
            ContentDescriptor::RelatedContentField* nestingField = nullptr;
            RelatedClassPath pathFromSelectToContentClass = m_pathFromSelectToPropertyClass;
            for (ContentDescriptor::Field* descriptorField : m_descriptor.GetAllFields())
                {
                if (!descriptorField->IsNestedContentField())
                    continue;

                ContentDescriptor::RelatedContentField* existingField = descriptorField->AsNestedContentField()->AsRelatedContentField();
                if (nullptr == existingField)
                    continue;

                RelatedClassPathCR existingFieldPathFromSelectToContentClass = existingField->GetPathFromSelectToContentClass();
                if (PathStartsWith(m_pathFromSelectToPropertyClass, existingFieldPathFromSelectToContentClass))
                    {
                    // if m_pathFromSelectToPropertyClass starts with existing nested content field path, this field should be nested in existing field
                    nestingField = existingField;
                    pathFromSelectToContentClass = GetPathDifference(m_pathFromSelectToPropertyClass, existingFieldPathFromSelectToContentClass);
                    break;
                    }
                else if (EndsWithSameRelatedClass(m_pathFromSelectToPropertyClass, existingFieldPathFromSelectToContentClass))
                    {
                    // if m_pathFromSelectToPropertyClass and existing nested content field paths both target the same class, unify the paths and update the
                    // existing field - no need to create a new one
                    RelatedClassPath unifiedPath = RelatedClassPath::Unify(existingFieldPathFromSelectToContentClass, m_pathFromSelectToPropertyClass);
                    if (!unifiedPath.empty())
                        {
                        m_nestedContentField = existingField;
                        m_nestedContentField->SetPathFromSelectToContentClass(unifiedPath);
                        return m_nestedContentField;
                        }
                    }
                }

            // create the field
            FieldAttributes fieldAttributes(m_actualClass.GetDisplayLabel(), m_context.GetCategorySupplier().GetRelatedECClassCategory(m_actualClass, m_relationshipMeaning), nullptr);
            m_nestedContentField = new ContentDescriptor::RelatedContentField(fieldAttributes.GetCategory(),
                fieldAttributes.GetLabel(), pathFromSelectToContentClass, bvector<ContentDescriptor::Field*>(), m_expandNestedFields);
            if (fieldAttributes.GetEditor())
                m_nestedContentField->SetEditor(new ContentFieldEditor(*fieldAttributes.GetEditor()));
            ApplyFieldLocalization(*m_nestedContentField, m_context);

            if (nullptr != nestingField)
                {
                // if the field is nested, add it to the nesting field
                nestingField->GetFields().push_back(m_nestedContentField);
                }
            else
                {
                // if the field is not nested, just append it to the descriptor
                m_descriptor.AddField(m_nestedContentField);
                }
            }
        return m_nestedContentField;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool IsXToManyRelated() const
        {
        for (RelatedClassCR rel : m_pathFromSelectToPropertyClass)
            {
            if (rel.IsForwardRelationship() && rel.GetRelationship()->GetTarget().GetMultiplicity().GetUpperLimit() > 1)
                return true;
            if (!rel.IsForwardRelationship() && rel.GetRelationship()->GetSource().GetMultiplicity().GetUpperLimit() > 1)
                return true;
            }
        return false;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AppendSimpleProperty(ECPropertyCR p, Utf8CP propertyClassAlias, PropertySpecificationCP overrides)
        {
        // get the field to append the property to (a new field will be created if necessary)
        ContentDescriptor::ECPropertiesField* field = GetPropertiesField(p, propertyClassAlias, overrides);
        if (nullptr == field)
            return false;

        ContentDescriptor::Property prop(propertyClassAlias, m_actualClass, p);
        // set the "related" flag, if necessary
        if (!m_pathFromSelectToPropertyClass.empty())
            prop.SetIsRelated(m_pathFromSelectToPropertyClass, m_relationshipMeaning);

        // append the property definition
        field->AddProperty(prop);

        // if new field was created add it to descriptor
        if (1 == field->GetProperties().size())
            {
            m_descriptor.AddField(field);
            if (!m_pathFromSelectToPropertyClass.empty() && m_keyField)
                m_keyField->AddKeyField(*field);
            }

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AppendXToManyRelatedProperty(ECPropertyCR p, PropertySpecificationCP overrides)
        {
        ContentDescriptor::RelatedContentField* field = GetXToManyNestedContentField();
        if (nullptr == field)
            return false;

        FieldAttributes fieldAttributes = CreateFieldAttributes(p, overrides, [&](){return m_context.GetCategorySupplier().GetPropertyCategory(p);});
        ContentDescriptor::ECPropertiesField propertyField(fieldAttributes.GetCategory(), fieldAttributes.GetLabel(), fieldAttributes.GetEditor().get());
        propertyField.AddProperty(ContentDescriptor::Property(field->GetPathFromSelectToContentClass().back().GetTargetClassAlias(), m_actualClass, p));
        ApplyFieldLocalization(propertyField, m_context);

        for (ContentDescriptor::Field* nestedField : field->GetFields())
            {
            if (*nestedField == propertyField)
                return false;
            }

        field->GetFields().push_back(new ContentDescriptor::ECPropertiesField(propertyField));
        return false;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _Supports(ECPropertyCR ecProperty, PropertySpecificationCP overrides) override
        {
        // don't support any properties if we're not creating fields
        if (!ShouldCreateFields(m_descriptor))
            return false;

        // don't support hidden properties
        if (!m_propertyInfos.ShouldDisplay(ecProperty, m_actualClass, overrides))
            return false;

        // note: we don't want to return binary, igeometry, etc. fields even if the above says we should..

        // don't support binary and igeometry properties
        if (ecProperty.GetIsPrimitive() && (PRIMITIVETYPE_Binary == ecProperty.GetAsPrimitiveProperty()->GetType() || PRIMITIVETYPE_IGeometry == ecProperty.GetAsPrimitiveProperty()->GetType()))
            return false;

        // don't support nested navigation properties
        if (IsXToManyRelated() && ecProperty.GetIsNavigation())
            return false;

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _Append(ECPropertyCR ecProperty, Utf8CP propertyClassAlias, PropertySpecificationCP overrides) override
        {
        if (IsXToManyRelated())
            return AppendXToManyRelatedProperty(ecProperty, overrides);
        return AppendSimpleProperty(ecProperty, propertyClassAlias, overrides);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentPropertiesAppender(ContentDescriptorBuilder::Context& context, PropertyInfoStore const& propertyInfos, ContentDescriptorR descriptor,
        ECClassCR actualClass, RelatedClassPath const& pathFromSelectToPropertyClass, RelationshipMeaning relationshipMeaning, bool expandNestedFields,
        PropertyCategorySpecificationsList const* scopePropertyCategories)
        : m_context(context), m_descriptor(descriptor), m_propertyInfos(propertyInfos), m_pathFromSelectToPropertyClass(pathFromSelectToPropertyClass), m_actualClass(actualClass),
        m_relationshipMeaning(relationshipMeaning), m_keyField(nullptr), m_nestedContentField(nullptr), m_expandNestedFields(expandNestedFields),
        m_scopePropertyCategories(scopePropertyCategories)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignFieldNames(bvector<ContentDescriptor::Field*> const& fields)
    {
    bmap<Utf8String, uint64_t> requestedNameCounts;
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
            AssignFieldNames(field->AsNestedContentField()->GetFields());
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilderImpl : ContentSpecificationsHandler
{
private:
    ContentDescriptorPtr m_descriptor;
    ContentSpecificationCP m_specification;
    PropertyInfoStore m_propertyInfos;
    bool m_isRecursiveSpecification;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorBuilder::Context& GetContext() {return static_cast<ContentDescriptorBuilder::Context&>(ContentSpecificationsHandler::GetContext());}
    ContentDescriptorBuilder::Context const& GetContext() const { return static_cast<ContentDescriptorBuilder::Context const&>(ContentSpecificationsHandler::GetContext()); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Aidas.Vaiksnoras                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddCalculatedFieldsFromContentModifiers(ECClassCR ecClass)
        {
        for (ContentModifierCP modifier : GetContext().GetRuleset().GetContentModifierRules())
            {
            ECClassCP modifierClass = GetContext().GetSchemaHelper().GetECClass(modifier->GetSchemaName().c_str(), modifier->GetClassName().c_str());
            if (ecClass.Is(modifierClass))
                {
                QueryBuilderHelpers::AddCalculatedFields(*m_descriptor, modifier->GetCalculatedProperties(),
                    GetContext().GetLocalizationProvider(), GetContext().GetLocale(), GetContext().GetRuleset(), modifierClass);
                }
            }
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ShouldIncludeRelatedProperties() const override {return ShouldCreateFields(*m_descriptor);}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppenderPtr _CreatePropertyAppender(ECClassCR propertyClass, RelatedClassPath const& pathToSelectClass, RelationshipMeaning relationshipMeaning,
        bool expandNestedFields, PropertyCategorySpecificationsList const* categorySpecifications) override
        {
        return new ContentPropertiesAppender(GetContext(), m_propertyInfos, *m_descriptor,
            propertyClass, pathToSelectClass, relationshipMeaning, expandNestedFields, categorySpecifications);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                02/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _OnContentAppended() override
        {
        if (nullptr != m_specification && ShouldCreateFields(*m_descriptor))
            {
            QueryBuilderHelpers::AddCalculatedFields(*m_descriptor, m_specification->GetCalculatedProperties(),
                GetContext().GetLocalizationProvider(), GetContext().GetLocale(), GetContext().GetRuleset(), nullptr);
            }

        AssignFieldNames(m_descriptor->GetAllFields());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
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
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AppendClass(SelectClassInfo const& classInfo) override
        {
        m_descriptor->GetSelectClasses().push_back(classInfo);

        if (ShouldCreateFields(*m_descriptor))
            AddCalculatedFieldsFromContentModifiers(classInfo.GetSelectClass().GetClass());
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorBuilderImpl(ContentDescriptorBuilder::Context& context, ContentSpecificationCP specification)
        : ContentSpecificationsHandler(context), m_specification(specification), m_isRecursiveSpecification(false),
        m_propertyInfos(GetContext().GetSchemaHelper(), GetContext().GetRuleset(), specification)
        {
        RulesDrivenECPresentationManager::ContentOptions options(GetContext().GetRuleset().GetRuleSetId(), GetContext().GetLocale(), GetContext().GetUnitSystem());
        m_descriptor = ContentDescriptor::Create(GetContext().GetConnection(), options.GetJson(), GetContext().GetInputKeys(), GetContext().GetPreferredDisplayType());
        m_descriptor->SetContentFlags(specification ? _GetContentFlags(*specification) : GetContext().GetContentFlagsCalculator() ? context.GetContentFlagsCalculator()(0) : 0);
        if (nullptr != GetContext().GetSelectionInfo())
            m_descriptor->SetSelectionInfo(*GetContext().GetSelectionInfo());

        if (nullptr == m_descriptor->GetDisplayLabelField())
            {
            ContentDescriptor::DisplayLabelField* field = new ContentDescriptor::DisplayLabelField(PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel()));
            ApplyFieldLocalization(*field, GetContext());
            m_descriptor->AddField(field);
            m_descriptor->GetDisplayLabelField()->SetOverrideValueSpecs(QueryBuilderHelpers::GetLabelOverrideValuesMap(GetContext().GetSchemaHelper(), GetContext().GetRuleset().GetInstanceLabelOverrides()));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(SelectedNodeInstancesSpecificationCR spec, IParsedInput const& input)
        {
        ContentSpecificationsHandler::HandleSpecification(spec, input);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(ContentRelatedInstancesSpecificationCR spec, IParsedInput const& input)
        {
        m_isRecursiveSpecification = spec.IsRecursive();
        ContentSpecificationsHandler::HandleSpecification(spec, input);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR spec)
        {
        ContentSpecificationsHandler::HandleSpecification(spec);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleContentField(ContentDescriptor::NestedContentField const& contentField)
        {
        m_descriptor->GetSelectClasses().push_back(SelectClassInfo(SelectClassWithExcludes(contentField.GetContentClass())));
        for (ContentDescriptor::Field const* field : contentField.GetFields())
            {
            if (field->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const* propertiesField = field->AsPropertiesField();
                BeAssert(1 == propertiesField->GetProperties().size());
                ContentDescriptor::ECPropertiesField* clone = propertiesField->Clone()->AsPropertiesField();
                clone->GetProperties().back().SetPrefix(contentField.GetContentClassAlias());
                m_descriptor->AddField(clone);
                }
            else if (field->IsNestedContentField())
                {
                ContentDescriptor::NestedContentField const* nestedField = field->AsNestedContentField();
                m_descriptor->AddField(nestedField->Clone());
                }
            else
                {
                BeAssert(false && "Unexpected type of field");
                }
            }
        AssignFieldNames(m_descriptor->GetAllFields());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorPtr GetDescriptor()
        {
        if (m_descriptor->GetSelectClasses().empty())
            return nullptr;

        return m_descriptor;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(SelectedNodeInstancesSpecificationCR specification, IParsedInput const& input) const
    {
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification, input);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentRelatedInstancesSpecificationCR specification, IParsedInput const& input) const
    {
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification, input);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentInstancesOfSpecificClassesSpecificationCR specification) const
    {
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentDescriptor::NestedContentField const& contentField) const
    {
    ContentDescriptorBuilderImpl builder(m_context, nullptr);
    builder.HandleContentField(contentField);
    return builder.GetDescriptor();
    }
