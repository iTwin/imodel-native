/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentDescriptorBuilder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "QueryBuilder.h"
#include "LoggingHelper.h"
#include "PropertyInfoStore.h"

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

private:
    ContentDescriptorBuilder::Context& m_context;
    PropertyInfoStore const& m_propertyInfos;
    ContentDescriptorR m_descriptor;
    ECClassCR m_actualClass;
    RelatedClassPath const& m_relatedClassPath;
    RelationshipMeaning m_relationshipMeaning;
    ContentDescriptor::ECInstanceKeyField* m_keyField;
    ContentDescriptor::NestedContentField* m_nestedContentField;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CreateFieldDisplayLabel(ECPropertyCR ecProperty)
        {
        Utf8String displayLabel;
        if (nullptr == m_context.GetPropertyFormatter()
            || SUCCESS != m_context.GetPropertyFormatter()->GetFormattedPropertyLabel(displayLabel, ecProperty, m_actualClass, m_relatedClassPath, m_relationshipMeaning))
            {
            if (!m_relatedClassPath.empty() && RelationshipMeaning::SameInstance != m_relationshipMeaning)
                displayLabel.append(m_actualClass.GetDisplayLabel()).append(" ").append(ecProperty.GetDisplayLabel());
            else
                displayLabel = ecProperty.GetDisplayLabel();
            }
        return displayLabel;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String CreateNestedContentFieldName(RelatedClassPath const& relationshipPath) const
        {
        Utf8String name;
        for (RelatedClassCR rel : relationshipPath)
            {
            name.append(rel.GetTargetClass()->GetName());
            name.append("_");
            }
        name.append(m_actualClass.GetName());
        return name;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECInstanceKeyField* FindKeyField(ContentDescriptor::ECPropertiesField const& propertiesField) const
        {
        for (ContentDescriptor::Field* descriptorField : m_descriptor.GetAllFields())
            {
            if (!descriptorField->IsSystemField() || !descriptorField->AsSystemField()->IsECInstanceKeyField())
                continue;

            ContentDescriptor::ECInstanceKeyField* keyField = descriptorField->AsSystemField()->AsECInstanceKeyField();
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
        ECPropertyCR ecProperty, Utf8CP propertyClassAlias)
        {
        ContentFieldEditor const* newPropertyEditor = m_propertyInfos.GetPropertyEditor(ecProperty, m_actualClass);

        for (ContentDescriptor::Field* field : fields)
            {
            if (!field->IsPropertiesField())
                continue;

            bool areEditorsEqual = (nullptr == newPropertyEditor && nullptr == field->GetEditor()
                || nullptr != newPropertyEditor && nullptr != field->GetEditor() && field->GetEditor()->Equals(*newPropertyEditor));
            if (!areEditorsEqual)
                continue;

            for (ContentDescriptor::Property const& prop : field->AsPropertiesField()->GetProperties())
                {
                // skip if already included in descriptor
                if (&ecProperty == &prop.GetProperty() 
                    && &m_actualClass == &prop.GetPropertyClass()
                    && 0 == strcmp(propertyClassAlias, prop.GetPrefix()))
                    {
                    return FieldCreateAction::Skip;
                    }

                // if properties in this field are similar, the new property should be included in this field
                bool isNewPropertyRelated = !m_relatedClassPath.empty();
                bool areSimilar = (prop.IsRelated() == isNewPropertyRelated) && (&m_actualClass == &prop.GetPropertyClass() || !isNewPropertyRelated);
                if (areSimilar)
                    areSimilar = ArePropertiesSimilar(ecProperty, prop.GetProperty());
                if (areSimilar)
                    {
                    mergeField = field->AsPropertiesField();
                    m_keyField = FindKeyField(*mergeField);
                    return FieldCreateAction::Merge;
                    }
                }
            }

        return FieldCreateAction::Create;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::ECPropertiesField* GetPropertiesField(ECPropertyCR ecProperty, Utf8CP propertyClassAlias)
        {
        ContentDescriptor::ECPropertiesField* field = nullptr;
        if (FieldCreateAction::Skip == GetActionForPropertyField(field, m_descriptor.GetAllFields(), ecProperty, propertyClassAlias))
            return nullptr;
                
        // did not find a field with similar properties - create a new one
        if (nullptr == field)
            {
            if (nullptr == m_keyField && !m_relatedClassPath.empty())
                {
                m_keyField = new ContentDescriptor::ECInstanceKeyField();
                m_descriptor.GetAllFields().push_back(m_keyField);
                }

            ECClassCR primaryClass = m_relatedClassPath.empty() ? m_actualClass : *m_relatedClassPath.front().GetSourceClass();
            ContentDescriptor::Category fieldCategory = m_context.GetCategorySupplier().GetCategory(primaryClass, m_relatedClassPath, ecProperty);
            ContentFieldEditor const* editor = m_propertyInfos.GetPropertyEditor(ecProperty, m_actualClass);
            field = new ContentDescriptor::ECPropertiesField(fieldCategory, "", CreateFieldDisplayLabel(ecProperty), editor ? new ContentFieldEditor(*editor) : nullptr);
            m_descriptor.GetAllFields().push_back(field);

            if (!m_relatedClassPath.empty())
                m_keyField->AddKeyField(*field);

            if (ecProperty.GetIsNavigation())
                m_descriptor.GetAllFields().push_back(new ContentDescriptor::ECNavigationInstanceIdField(*field));
            }

        return field;
        }
    
    /*---------------------------------------------------------------------------------**//**
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
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptor::NestedContentField* GetXToManyNestedContentField(Utf8CP classAlias)
        {
        if (nullptr == m_nestedContentField)
            {
            // find a content field that the new nested field should be nested in
            ContentDescriptor::NestedContentField* nestingField = nullptr;
            RelatedClassPath relationshipPath = m_relatedClassPath;
            for (ContentDescriptor::Field* descriptorField : m_descriptor.GetAllFields())
                {
                if (!descriptorField->IsNestedContentField())
                    continue;

                if (PathStartsWith(m_relatedClassPath, descriptorField->AsNestedContentField()->GetRelationshipPath()))
                    {
                    // this field should be nested in descriptorField
                    nestingField = descriptorField->AsNestedContentField();
                    relationshipPath = GetPathDifference(m_relatedClassPath, descriptorField->AsNestedContentField()->GetRelationshipPath());
                    break;
                    }
                }
        
            // create the field
            ECClassCR primaryClass = *m_relatedClassPath.front().GetTargetClass();
            ContentDescriptor::Category fieldCategory = m_context.GetCategorySupplier().GetCategory(primaryClass, relationshipPath, m_actualClass);
            m_nestedContentField = new ContentDescriptor::NestedContentField(fieldCategory, CreateNestedContentFieldName(relationshipPath), 
                m_actualClass.GetDisplayLabel(), m_actualClass, classAlias, relationshipPath);
            if (nullptr != nestingField)
                {
                // if the field is nested, add it to the nesting field
                nestingField->GetFields().push_back(m_nestedContentField);
                }
            else
                {
                // if the field is not nested, just append it to the descriptor
                m_descriptor.GetAllFields().push_back(m_nestedContentField);
                }
            }
        return m_nestedContentField;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool ArePropertiesSimilar(ECPropertyCR lhs, ECPropertyCR rhs)
        {
        if (!lhs.GetName().Equals(rhs.GetName()))
            return false;

        if (!lhs.GetTypeName().Equals(rhs.GetTypeName()))
            return false;

        if (lhs.GetKindOfQuantity() != rhs.GetKindOfQuantity())
            return false;

        return true;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool IsXToManyRelated() const
        {
        for (RelatedClassCR rel : m_relatedClassPath)
            {
            if (rel.IsForwardRelationship() && rel.GetRelationship()->GetSource().GetMultiplicity().GetUpperLimit() > 1)
                return true;
            if (!rel.IsForwardRelationship() && rel.GetRelationship()->GetTarget().GetMultiplicity().GetUpperLimit() > 1)
                return true;
            }
        return false;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AppendSimpleProperty(ECPropertyCR p, Utf8CP propertyClassAlias)
        {
        // get the field to append the property to (a new field will be created if necessary)
        ContentDescriptor::ECPropertiesField* field = GetPropertiesField(p, propertyClassAlias);
        if (nullptr == field)
            return false;

        // append the property definition
        field->GetProperties().push_back(ContentDescriptor::Property(propertyClassAlias, m_actualClass, p));

        // set the "related" flag, if necessary
        if (!m_relatedClassPath.empty())
            field->GetProperties().back().SetIsRelated(m_relatedClassPath);
        
        // update field name (which depends on the properties in the field)
        field->SetName(QueryBuilderHelpers::CreateFieldName(*field));
        if (nullptr != m_keyField)
            m_keyField->RecalculateName();

        return true;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool AppendXToManyRelatedProperty(ECPropertyCR p, Utf8CP propertyClassAlias)
        {
        ContentDescriptor::NestedContentField* field = GetXToManyNestedContentField(propertyClassAlias);
        if (nullptr == field)
            return false;

        ContentDescriptor::ECPropertiesField* propertyField = new ContentDescriptor::ECPropertiesField(m_actualClass, 
            ContentDescriptor::Property(propertyClassAlias, m_actualClass, p));
        field->GetFields().push_back(propertyField);
        return false;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _Supports(ECPropertyCR ecProperty) override
        {
        // don't support any properties if descriptor has NoFields flag
        if (m_descriptor.HasContentFlag(ContentFlags::NoFields))
            return false;

        // don't support hidden properties
        if (!m_propertyInfos.ShouldDisplay(ecProperty, m_actualClass))
            return false;

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
    bool _Append(ECPropertyCR ecProperty, Utf8CP propertyClassAlias) override
        {        
        if (IsXToManyRelated())
            return AppendXToManyRelatedProperty(ecProperty, propertyClassAlias);
        return AppendSimpleProperty(ecProperty, propertyClassAlias);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentPropertiesAppender(ContentDescriptorBuilder::Context& context, PropertyInfoStore const& propertyInfos, ContentDescriptorR descriptor,        
        ECClassCR actualClass, RelatedClassPath const& relatedClassPath, RelationshipMeaning relationshipMeaning)
        : m_context(context), m_descriptor(descriptor), m_propertyInfos(propertyInfos), m_relatedClassPath(relatedClassPath), m_actualClass(actualClass), 
        m_relationshipMeaning(relationshipMeaning), m_keyField(nullptr), m_nestedContentField(nullptr)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentDescriptorBuilderImpl : ContentSpecificationsHandler
{
private:
    ContentDescriptorPtr m_descriptor;
    ContentSpecificationCP m_specification;
    PropertyInfoStore m_propertyInfos;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorBuilder::Context& GetContext() {return static_cast<ContentDescriptorBuilder::Context&>(ContentSpecificationsHandler::GetContext());}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsRecursiveSelect(SelectClassInfo const& classInfo)
        {
        if (classInfo.GetPathToPrimaryClass().empty())
            return false;

        RelatedClassCR relationshipInfo = classInfo.GetPathToPrimaryClass().front();
        return relationshipInfo.GetSourceClass()->Is(relationshipInfo.GetTargetClass())
            || relationshipInfo.GetTargetClass()->Is(relationshipInfo.GetSourceClass());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool CanMergeSelectClasses(SelectClassInfo const& lhs, SelectClassInfo const& rhs)
        {
        if (&lhs.GetSelectClass() != &rhs.GetSelectClass())
            return false;

        if (!lhs.GetPathToPrimaryClass().empty() && !rhs.GetPathToPrimaryClass().empty())
            {
            RelatedClassCR lhsRelated = lhs.GetPathToPrimaryClass().front();
            RelatedClassCR rhsRelated = rhs.GetPathToPrimaryClass().front();
            if (lhsRelated.IsForwardRelationship() != rhsRelated.IsForwardRelationship())
                return false;
            }
        
        return true;
        }

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
                    GetContext().GetLocalizationProvider(), GetContext().GetRuleset(), modifierClass);
                }
            }
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ShouldIncludeRelatedProperties() const override {return !m_descriptor->HasContentFlag(ContentFlags::NoFields);}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyAppenderPtr _CreatePropertyAppender(ECClassCR propertyClass, RelatedClassPath const& pathToSelectClass, RelationshipMeaning relationshipMeaning) override
        {
        return new ContentPropertiesAppender(GetContext(), m_propertyInfos, *m_descriptor, propertyClass, pathToSelectClass, relationshipMeaning);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AppendClass(SelectClassInfo const& classInfo) override
        {
        if (IsRecursiveSelect(classInfo))
            {
            for (SelectClassInfo& selectClassInfoIter : m_descriptor->GetSelectClasses())
                {
                if (CanMergeSelectClasses(selectClassInfoIter, classInfo))
                    return;
                }
            }

        m_descriptor->GetSelectClasses().push_back(classInfo);

        if (!m_descriptor->HasContentFlag(ContentFlags::NoFields))
            AddCalculatedFieldsFromContentModifiers(classInfo.GetSelectClass());
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorBuilderImpl(ContentDescriptorBuilder::Context& context, ContentSpecificationCP specification)
        : ContentSpecificationsHandler(context), m_specification(specification),
        m_propertyInfos(GetContext().GetSchemaHelper(), GetContext().GetRuleset(), specification)
        {
        m_descriptor = ContentDescriptor::Create(GetContext().GetPreferredDisplayType());
        if (nullptr != m_specification)
            QueryBuilderHelpers::ApplyDefaultContentFlags(*m_descriptor, GetContext().GetPreferredDisplayType(), *m_specification);
        }
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(SelectedNodeInstancesSpecificationCR spec, IParsedSelectionInfo const& selection) {ContentSpecificationsHandler::HandleSpecification(spec, selection);}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(ContentRelatedInstancesSpecificationCR spec, IParsedSelectionInfo const& selection) {ContentSpecificationsHandler::HandleSpecification(spec, selection);}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleSpecification(ContentInstancesOfSpecificClassesSpecificationCR spec) {ContentSpecificationsHandler::HandleSpecification(spec);}
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void HandleContentField(ContentDescriptor::NestedContentField const& contentField)
        {
        m_descriptor->GetSelectClasses().push_back(SelectClassInfo(contentField.GetContentClass(), true));
        for (ContentDescriptor::Field const* field : contentField.GetFields())
            {
            if (field->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const* propertiesField = field->AsPropertiesField();
                BeAssert(1 == propertiesField->GetProperties().size());
                bvector<RelatedClassPath> navigationPropertiesPaths;
                PropertyAppenderPtr appender = _CreatePropertyAppender(contentField.GetContentClass(), RelatedClassPath(), RelationshipMeaning::SameInstance);
                appender->Append(propertiesField->GetProperties().front().GetProperty(), contentField.GetContentClassAlias().c_str());
                m_descriptor->GetAllFields().back()->SetName(field->GetName());
                }
            else if (field->IsNestedContentField())
                {
                ContentDescriptor::NestedContentField const* nestedField = field->AsNestedContentField();
                m_descriptor->GetAllFields().push_back(nestedField->Clone());
                }
            else
                {
                BeAssert(false && "Unexpected type of field");
                }
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                10/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentDescriptorPtr GetDescriptor()
        {
        if (m_descriptor->GetSelectClasses().empty())
            return nullptr;

        if (nullptr != m_specification && !m_descriptor->HasContentFlag(ContentFlags::NoFields))
            {
            QueryBuilderHelpers::AddCalculatedFields(*m_descriptor, m_specification->GetCalculatedProperties(),
                GetContext().GetLocalizationProvider(), GetContext().GetRuleset(), nullptr);
            }

        return m_descriptor;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(SelectedNodeInstancesSpecificationCR specification, IParsedSelectionInfo const& selection) const
    {
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification, selection);
    return builder.GetDescriptor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptorPtr ContentDescriptorBuilder::CreateDescriptor(ContentRelatedInstancesSpecificationCR specification, IParsedSelectionInfo const& selection) const
    {
    ContentDescriptorBuilderImpl builder(m_context, &specification);
    builder.HandleSpecification(specification, selection);
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