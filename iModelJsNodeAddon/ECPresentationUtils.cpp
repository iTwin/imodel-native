/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECPresentationUtils.h"
#include <Bentley/BeDirectoryIterator.h>

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationSerializer : IECPresentationSerializer
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ECClassCP GetClassFromFullName(IConnectionCR connection, Utf8CP fullClassName)
        {
        Utf8String schemaName, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, fullClassName))
            {
            BeAssert(false);
            return nullptr;
            }
        return connection.GetECDb().Schemas().GetClass(schemaName, className);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ConnectionEvent const&, rapidjson::Document::AllocatorType* allocator) const override
        {
        BeAssert(false && "Not expected to be used");
        return rapidjson::Document(allocator);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ContentFieldEditor const& editor, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("name", rapidjson::Value(editor.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
        rapidjson::Value paramsJson(rapidjson::kObjectType);
        for (ContentFieldEditor::Params const* params : editor.GetParams())
            {
            if (paramsJson.HasMember(params->GetName()))
                {
                BeAssert(false);
                continue;
                }
            paramsJson.AddMember(rapidjson::Value(params->GetName(), json.GetAllocator()), params->AsJson(&json.GetAllocator()), json.GetAllocator());
            }
        json.AddMember("params", paramsJson, json.GetAllocator());
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ParamsAsJson(ContentFieldEditor::Params const&, RapidJsonDocumentR) const override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("name", rapidjson::Value(category.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("label", rapidjson::Value(category.GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("description", rapidjson::Value(category.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("expand", category.ShouldExpand(), json.GetAllocator());
        json.AddMember("priority", category.GetPriority(), json.GetAllocator());
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();

        rapidjson::Value propertyJson(rapidjson::kObjectType);
        propertyJson.AddMember("classInfo", _AsJson(property.GetProperty().GetClass(), &json.GetAllocator()), json.GetAllocator());
        propertyJson.AddMember("name", rapidjson::Value(property.GetProperty().GetName().c_str(), json.GetAllocator()), json.GetAllocator());

        if (property.GetProperty().GetIsPrimitive() && nullptr != property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration())
            {
            rapidjson::Value enumJson(rapidjson::kObjectType);
            ECEnumerationCP propEnum = property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
            enumJson.AddMember("isStrict", rapidjson::Value(propEnum->GetIsStrict()), json.GetAllocator());
            enumJson.AddMember("choices", _AsJson(*propEnum, json.GetAllocator()), json.GetAllocator());
            propertyJson.AddMember("type", "enum", json.GetAllocator());
            propertyJson.AddMember("enumerationInfo", enumJson, json.GetAllocator());
            }
        else
            {
            propertyJson.AddMember("type", rapidjson::Value(property.GetProperty().GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
            }

        if (nullptr != property.GetProperty().GetKindOfQuantity())
            {
            KindOfQuantityCP koq = property.GetProperty().GetKindOfQuantity();
            propertyJson.AddMember("kindOfQuantity", _AsJson(*koq, json.GetAllocator()), json.GetAllocator());
            }

        json.AddMember("property", propertyJson, json.GetAllocator());
        json.AddMember("relatedClassPath", _AsJson(property.GetRelatedClassPath(), json.GetAllocator()), json.GetAllocator());

        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _FieldAsJson(ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const override
        {
        fieldBaseJson.SetObject();
        fieldBaseJson.AddMember("category", field.GetCategory().AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("name", rapidjson::Value(field.GetName().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("label", rapidjson::Value(field.GetLabel().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("type", field.GetTypeDescription().AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("isReadonly", field.IsReadOnly(), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("priority", field.GetPriority(), fieldBaseJson.GetAllocator());

        if (nullptr != field.GetEditor())
            fieldBaseJson.AddMember("editor", field.GetEditor()->AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const override
        {
        rapidjson::Value propertiesJson(rapidjson::kArrayType);
        for (ContentDescriptor::Property const& prop : ecPropertiesField.GetProperties())
            propertiesJson.PushBack(prop.AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("properties", propertiesJson, fieldBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const override
        {
        fieldBaseJson.AddMember("contentClassInfo", _AsJson(nestedContentField.GetContentClass(), &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("pathToPrimaryClass", _AsJson(nestedContentField.GetRelationshipPath(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        if (nestedContentField.ShouldAutoExpand())
            fieldBaseJson.AddMember("autoExpand", nestedContentField.ShouldAutoExpand(), fieldBaseJson.GetAllocator());

        rapidjson::Value nestedFieldsJson(rapidjson::kArrayType);
        for (ContentDescriptor::Field const* nestedField : nestedContentField.GetFields())
            nestedFieldsJson.PushBack(nestedField->AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        fieldBaseJson.AddMember("nestedFields", nestedFieldsJson, fieldBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::DisplayLabelField const&, RapidJsonDocumentR) const override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::CalculatedPropertyField const&, RapidJsonDocumentR) const override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::SystemField const&, RapidJsonDocumentR) const override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::ECInstanceKeyField const&, RapidJsonDocumentR) const override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::ECNavigationInstanceIdField const&, RapidJsonDocumentR) const override {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(UpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
#ifdef wip
        json.SetObject();

        switch (updateRecord.GetChangeType())
            {
            case ChangeType::Delete:
                {
                json.AddMember("Type", "Delete", json.GetAllocator());
                json.AddMember("Node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
                break;
                }
            case ChangeType::Insert:
                {
                json.AddMember("Type", "Insert", json.GetAllocator());
                json.AddMember("Node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
                json.AddMember("Position", (uint64_t)updateRecord.GetPosition(), json.GetAllocator());
                break;
                }
            case ChangeType::Update:
                {
                json.AddMember("Type", "Update", json.GetAllocator());
                json.AddMember("Node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
                json.AddMember("Changes", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
                RapidJsonValueR changesJson = json["Changes"];
                for (JsonChange const& change : updateRecord.GetChanges())
                    {
                    rapidjson::Value changeJson(rapidjson::kObjectType);
                    changeJson.AddMember("Name", rapidjson::Value(change.GetName(), json.GetAllocator()), json.GetAllocator());
                    changeJson.AddMember("OldValue", rapidjson::Value(change.GetOldValue(), json.GetAllocator()), json.GetAllocator());
                    changeJson.AddMember("NewValue", rapidjson::Value(change.GetNewValue(), json.GetAllocator()), json.GetAllocator());
                    changesJson.PushBack(changeJson, json.GetAllocator());
                    }
                break;
                }
            }

        RapidJsonValueCR extendedData = updateRecord.GetNode()->GetExtendedData();
        if (extendedData.HasMember("RulesetId"))
            json.AddMember("RulesetId", rapidjson::Value(extendedData["RulesetId"].GetString(), json.GetAllocator()), json.GetAllocator());
#endif
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const override
        {
#ifdef wip
        paramsBaseJson.CopyFrom(jsonParams.GetJson(), paramsBaseJson.GetAllocator());
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const override
        {
#ifdef wip
        paramsBaseJson.SetObject();
        paramsBaseJson.AddMember("HeightInRows", multilineParams.GetParameters().GetHeightInRows(), paramsBaseJson.GetAllocator());
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const override
        {
#ifdef wip
        paramsBaseJson.SetObject();

        if (nullptr != rangeParams.GetParameters().GetMinimumValue())
            paramsBaseJson.AddMember("Minimum", *rangeParams.GetParameters().GetMinimumValue(), paramsBaseJson.GetAllocator());
        else
            paramsBaseJson.AddMember("Minimum", rapidjson::Value(), paramsBaseJson.GetAllocator());

        if (nullptr != rangeParams.GetParameters().GetMaximumValue())
            paramsBaseJson.AddMember("Maximum", *rangeParams.GetParameters().GetMaximumValue(), paramsBaseJson.GetAllocator());
        else
            paramsBaseJson.AddMember("Maximum", rapidjson::Value(), paramsBaseJson.GetAllocator());
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const override
        {
#ifdef wip
        paramsBaseJson.SetObject();
        paramsBaseJson.AddMember("Minimum", sliderParams.GetParameters().GetMinimumValue(), paramsBaseJson.GetAllocator());
        paramsBaseJson.AddMember("Maximum", sliderParams.GetParameters().GetMaximumValue(), paramsBaseJson.GetAllocator());
        paramsBaseJson.AddMember("IntervalsCount", sliderParams.GetParameters().GetIntervalsCount(), paramsBaseJson.GetAllocator());
        paramsBaseJson.AddMember("ValueFactor", sliderParams.GetParameters().GetValueFactor(), paramsBaseJson.GetAllocator());
        paramsBaseJson.AddMember("IsVertical", sliderParams.GetParameters().IsVertical(), paramsBaseJson.GetAllocator());
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _TypeDescriptionAsJson(ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override
        {
        typeDescriptionBaseJson.SetObject();
        typeDescriptionBaseJson.AddMember("typeName", rapidjson::Value(typeDescription.GetTypeName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR typeDescriptionBaseJson) const override
        {
        typeDescriptionBaseJson.AddMember("valueFormat", "Primitive", typeDescriptionBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override
        {
        typeDescriptionBaseJson.AddMember("valueFormat", "Array", typeDescriptionBaseJson.GetAllocator());
        typeDescriptionBaseJson.AddMember("memberType", arrayTypeDescription.GetMemberType()->AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override
        {
        typeDescriptionBaseJson.AddMember("valueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
        rapidjson::Value members(rapidjson::kArrayType);
        for (ECPropertyCP prop : structTypeDescription.GetStruct().GetProperties())
            {
            rapidjson::Value member(rapidjson::kObjectType);
            member.AddMember("name", rapidjson::StringRef(prop->GetName().c_str()), typeDescriptionBaseJson.GetAllocator());
            member.AddMember("label", rapidjson::StringRef(prop->GetDisplayLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
            member.AddMember("type", ContentDescriptor::Field::TypeDescription::Create(*prop)->AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
            members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
            }
        typeDescriptionBaseJson.AddMember("members", members, typeDescriptionBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const override
        {
        typeDescriptionBaseJson.AddMember("valueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
        rapidjson::Value members(rapidjson::kArrayType);
        for (ContentDescriptor::Field const* nestedField : nestedContentTypeDescription.GetNestedContentField().GetFields())
            {
            rapidjson::Value member(rapidjson::kObjectType);
            member.AddMember("name", rapidjson::Value(nestedField->GetName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
            member.AddMember("label", rapidjson::Value(nestedField->GetLabel().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
            member.AddMember("type", nestedField->GetTypeDescription().AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
            members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
            }
        typeDescriptionBaseJson.AddMember("members", members, typeDescriptionBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ECClassCR ecClass, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("id", rapidjson::Value(ecClass.GetId().ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("name", rapidjson::StringRef(ecClass.GetFullName()), json.GetAllocator());
        json.AddMember("label", rapidjson::StringRef(ecClass.GetDisplayLabel().c_str()), json.GetAllocator());
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("displayType", rapidjson::Value(contentDescriptor.GetPreferredDisplayType().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("selectClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (SelectClassInfo const& selectClass : contentDescriptor.GetSelectClasses())
            {
            rapidjson::Value selectClassJson(rapidjson::kObjectType);
            selectClassJson.AddMember("selectClassInfo", _AsJson(selectClass.GetSelectClass(), &json.GetAllocator()), json.GetAllocator());
            selectClassJson.AddMember("isSelectPolymorphic", selectClass.IsSelectPolymorphic(), json.GetAllocator());
            selectClassJson.AddMember("pathToPrimaryClass", _AsJson(selectClass.GetPathToPrimaryClass(), json.GetAllocator()), json.GetAllocator());
            selectClassJson.AddMember("relatedPropertyPaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            for (RelatedClassPathCR propertyPath : selectClass.GetRelatedPropertyPaths())
                selectClassJson["relatedPropertyPaths"].PushBack(_AsJson(propertyPath, json.GetAllocator()), json.GetAllocator());
            selectClassJson.AddMember("navigationPropertyClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            for (RelatedClassCR navPropertyClass : selectClass.GetNavigationPropertyClasses())
                selectClassJson["navigationPropertyClasses"].PushBack(_AsJson(navPropertyClass, json.GetAllocator()), json.GetAllocator());
            selectClassJson.AddMember("relatedInstanceClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            for (RelatedClassCR relatedInstanceClass : selectClass.GetRelatedInstanceClasses())
                selectClassJson["relatedInstanceClasses"].PushBack(_AsJson(relatedInstanceClass, json.GetAllocator()), json.GetAllocator());
            json["selectClasses"].PushBack(selectClassJson, json.GetAllocator());
            }

        bvector<ContentDescriptor::Field*> visibleFields = contentDescriptor.GetVisibleFields();
        json.AddMember("fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (ContentDescriptor::Field const* field : visibleFields)
            json["fields"].PushBack(field->AsJson(&json.GetAllocator()), json.GetAllocator());
        if (-1 != contentDescriptor.GetSortingFieldIndex())
            {
            json.AddMember("sortingFieldName", rapidjson::Value(contentDescriptor.GetSortingField()->GetName().c_str(), json.GetAllocator()), json.GetAllocator());
            json.AddMember("sortDirection", (int)contentDescriptor.GetSortDirection(), json.GetAllocator());
            }

        json.AddMember("contentFlags", contentDescriptor.GetContentFlags(), json.GetAllocator());
        json.AddMember("connectionId", rapidjson::Value(contentDescriptor.GetConnectionId().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("filterExpression", rapidjson::Value(contentDescriptor.GetFilterExpression().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("inputKeysHash", rapidjson::Value(contentDescriptor.GetInputNodeKeys().GetHash().c_str(), json.GetAllocator()), json.GetAllocator());

        rapidjson::Document options(&json.GetAllocator());
        options.Parse(contentDescriptor.GetOptions().ToString().c_str());
        json.AddMember("contentOptions", options, json.GetAllocator());

        if (nullptr != contentDescriptor.GetSelectionInfo())
            json.AddMember("selectionInfo", _AsJson(*contentDescriptor.GetSelectionInfo(), json.GetAllocator()), json.GetAllocator());

        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayLabel & flags))
            json.AddMember("label", rapidjson::Value(contentSetItem.GetDisplayLabel().c_str(), json.GetAllocator()), json.GetAllocator());

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_ImageId & flags))
            json.AddMember("imageId", rapidjson::Value(contentSetItem.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_Values & flags))
            json.AddMember("values", rapidjson::Value(contentSetItem.GetValues(), json.GetAllocator()), json.GetAllocator());

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayValues & flags))
            json.AddMember("displayValues", rapidjson::Value(contentSetItem.GetDisplayValues(), json.GetAllocator()), json.GetAllocator());

        if (contentSetItem.GetClass() != nullptr && 0 != (ContentSetItem::SerializationFlags::SERIALIZE_ClassInfo & flags))
            json.AddMember("classInfo", _AsJson(*contentSetItem.GetClass(), &json.GetAllocator()), json.GetAllocator());

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_PrimaryKeys & flags))
            {
            rapidjson::Value primaryKeys(rapidjson::kArrayType);
            for (ECClassInstanceKeyCR key : contentSetItem.GetKeys())
                primaryKeys.PushBack(_AsJson(key, &json.GetAllocator()), json.GetAllocator());
            json.AddMember("primaryKeys", primaryKeys, json.GetAllocator());
            }

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_MergedFieldNames & flags))
            {
            rapidjson::Value fieldNamesJson(rapidjson::kArrayType);
            for (Utf8StringCR fieldName : contentSetItem.GetMergedFieldNames())
                fieldNamesJson.PushBack(rapidjson::Value(fieldName.c_str(), json.GetAllocator()), json.GetAllocator());
            json.AddMember("mergedFieldNames", fieldNamesJson, json.GetAllocator());
            }

        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_UsersExtendedData & flags) && contentSetItem.GetUsersExtendedData().GetJson().MemberCount() > 0)
            {
            json.AddMember("extendedData", rapidjson::Value(contentSetItem.GetUsersExtendedData().GetJson(), json.GetAllocator()), json.GetAllocator());
            }

#ifdef wip
        if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_FieldPropertyInstanceKeys & flags))
            {
            rapidjson::Value fieldValueKeys(rapidjson::kObjectType);
            for (auto pair : contentSetItem.GetFieldInstanceKeys())
                {
                Utf8CP fieldName = pair.first.GetField().GetName().c_str();
                if (!fieldValueKeys.HasMember(fieldName))
                    fieldValueKeys.AddMember(rapidjson::StringRef(fieldName), rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
                rapidjson::Value& fieldProperties = fieldValueKeys[fieldName];

                rapidjson::Value propertyKeys(rapidjson::kArrayType);
                for (ECClassInstanceKeyCR key : pair.second)
                    propertyKeys.PushBack(_AsJson(key, &json.GetAllocator()), json.GetAllocator());

                rapidjson::Value fieldProperty(rapidjson::kObjectType);
                fieldProperty.AddMember("PropertyIndex", (uint64_t)pair.first.GetPropertyIndex(), json.GetAllocator());
                fieldProperty.AddMember("Keys", propertyKeys, json.GetAllocator());
                fieldProperties.PushBack(fieldProperty, json.GetAllocator());
                }
            json.AddMember("FieldValueKeys", fieldValueKeys, json.GetAllocator());
            }
#endif

        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("className", rapidjson::StringRef(key.GetClass()->GetFullName()), json.GetAllocator());
        json.AddMember("id", rapidjson::Value(key.GetId().ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator()), json.GetAllocator());
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(Content const& content, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("descriptor", content.GetDescriptor().AsJson(&json.GetAllocator()), json.GetAllocator());

        rapidjson::Value set(rapidjson::kArrayType);
        DataContainer<ContentSetItemCPtr> container = content.GetContentSet();
        for (ContentSetItemCPtr item : container)
            {
            if (item.IsValid())
                set.PushBack(item->AsJson(ContentSetItem::SERIALIZE_All, &json.GetAllocator()), json.GetAllocator());
            }
        json.AddMember("contentSet", set, json.GetAllocator());

        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(ECInstanceChangeResult const&, rapidjson::Document::AllocatorType* allocator) const override
        {
        BeAssert(false && "Not expected to be used");
        return rapidjson::Document(allocator);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _NavNodeKeyAsJson(NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.SetObject();
        navNodeKeyBaseJson.AddMember("type", rapidjson::Value(navNodeKey.GetType().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

        rapidjson::Value pathJson(rapidjson::kArrayType);
        for (Utf8StringCR partialHash : navNodeKey.GetHashPath())
            pathJson.PushBack(rapidjson::Value(partialHash.c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

        navNodeKeyBaseJson.AddMember("pathFromRoot", pathJson, navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bvector<Utf8String> ParseNodeKeyHashPath(JsonValueCR pathJson)
        {
        bvector<Utf8String> path;
        for (JsonValueCR pathElement : pathJson)
            path.push_back(pathElement.asString());
        return path;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bvector<Utf8String> ParseNodeKeyHashPath(RapidJsonValueCR pathJson)
        {
        bvector<Utf8String> path;
        for (RapidJsonValueCR pathElement : pathJson.GetArray())
            path.push_back(pathElement.GetString());
        return path;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        if (!json.isObject() || json.isNull())
            {
            BeAssert(false);
            return nullptr;
            }
        Utf8CP type = json["type"].asCString();
        if (nullptr == type)
            {
            BeAssert(false);
            return nullptr;
            }
        if (0 == strcmp("ECInstanceNode", type)) // @deprecated
            return _GetECInstanceNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_ECInstancesNode, type))
            return _GetECInstanceNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
            return _GetECClassGroupingNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
            return _GetECPropertyGroupingNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_DisplayLabelGroupingNode, type))
            return _GetLabelGroupingNodeKeyFromJson(json);
        return _GetBaseNavNodeKeyFromJson(json);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeKeyPtr _GetNavNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        if (!json.IsObject() || json.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }
        Utf8CP type = json["type"].GetString();
        if (nullptr == type)
            {
            BeAssert(false);
            return nullptr;
            }
        if (0 == strcmp("ECInstanceNode", type)) // @deprecated
            return _GetECInstanceNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_ECInstancesNode, type))
            return _GetECInstanceNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
            return _GetECClassGroupingNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
            return _GetECPropertyGroupingNodeKeyFromJson(connection, json);
        if (0 == strcmp(NAVNODE_TYPE_DisplayLabelGroupingNode, type))
            return _GetLabelGroupingNodeKeyFromJson(json);
        return _GetBaseNavNodeKeyFromJson(json);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(JsonValueCR json) const override
        {
        Utf8CP type = json["type"].asCString();
        return NavNodeKey::Create(type, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeKeyPtr _GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const override
        {
        Utf8CP type = json["type"].GetString();
        return NavNodeKey::Create(type, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ECInstancesNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        rapidjson::Value instanceKeysJson(rapidjson::kArrayType);
        for (ECClassInstanceKeyCR instanceKey : ecInstanceNodeKey.GetInstanceKeys())
            instanceKeysJson.PushBack(_AsJson(instanceKey, &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        navNodeKeyBaseJson.AddMember("instanceKeys", instanceKeysJson, navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        bvector<ECClassInstanceKey> instanceKeys;
        if (json.isMember("instanceKeys") && json["instanceKeys"].isArray())
            {
            JsonValueCR instanceKeysJson = json["instanceKeys"];
            for (Json::ArrayIndex i = 0; i < instanceKeysJson.size(); ++i)
                {
                Utf8CP className = instanceKeysJson[i]["className"].asCString();
                ECClassCP ecClass = GetClassFromFullName(connection, className);
                ECInstanceId instanceId(ECInstanceId::FromString(instanceKeysJson[i]["id"].asCString()));
                instanceKeys.push_back(ECClassInstanceKey(ecClass, instanceId));
                }
            }
        return ECInstancesNodeKey::Create(instanceKeys, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECInstancesNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        bvector<ECClassInstanceKey> instanceKeys;
        if (json.HasMember("instanceKeys") && json["instanceKeys"].IsArray())
            {
            RapidJsonValueCR instanceKeysJson = json["instanceKeys"];
            for (rapidjson::SizeType i = 0; i < instanceKeysJson.Size(); ++i)
                {
                Utf8CP className = instanceKeysJson[i]["className"].GetString();
                ECClassCP ecClass = GetClassFromFullName(connection, className);
                ECInstanceId instanceId(ECInstanceId::FromString(instanceKeysJson[i]["id"].GetString()));
                instanceKeys.push_back(ECClassInstanceKey(ecClass, instanceId));
                }
            }
        return ECInstancesNodeKey::Create(instanceKeys, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.AddMember("groupedInstancesCount", groupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
        navNodeKeyBaseJson.AddMember("className", rapidjson::StringRef(groupingNodeKey.GetECClass().GetFullName()), navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        uint64_t groupedInstancesCount = json["groupedInstancesCount"].asUInt64();
        Utf8CP className = json["className"].asCString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUint64();
        Utf8CP className = json["className"].GetString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.AddMember("groupedInstancesCount", propertyGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
        navNodeKeyBaseJson.AddMember("className", rapidjson::StringRef(propertyGroupingNodeKey.GetECClass().GetFullName()), navNodeKeyBaseJson.GetAllocator());
        navNodeKeyBaseJson.AddMember("propertyName", rapidjson::Value(propertyGroupingNodeKey.GetPropertyName().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        if (nullptr != propertyGroupingNodeKey.GetGroupingValue())
            navNodeKeyBaseJson.AddMember("groupingValue", rapidjson::Value(*propertyGroupingNodeKey.GetGroupingValue(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        uint64_t groupedInstancesCount = json["groupedInstancesCount"].asUInt64();
        Utf8CP className = json["className"].asCString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        Utf8CP propertyName = json["propertyName"].asCString();
        if (!json.isMember("groupingValue"))
            return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, nullptr, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);

        rapidjson::Document groupingValue;
        groupingValue.Parse(Json::FastWriter().write(json["groupingValue"]).c_str());
        return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, &groupingValue, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUint64();
        Utf8CP className = json["className"].GetString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        Utf8CP propertyName = json["propertyName"].GetString();
        rapidjson::Value const* groupingValue = nullptr;
        if (json.HasMember("groupingValue"))
            groupingValue = &json["groupingValue"];
        return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, groupingValue, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.AddMember("groupedInstancesCount", labelGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
        navNodeKeyBaseJson.AddMember("label", rapidjson::Value(labelGroupingNodeKey.GetLabel().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const override
        {
        uint64_t groupedInstancesCount = json["groupedInstancesCount"].asUInt64();
        Utf8CP label = json["label"].asCString();
        return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const override
        {
        uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUint64();
        Utf8CP label = json["label"].GetString();
        return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        if (navNode.GetKey().IsNull())
            {
            BeAssert(false);
            json.AddMember("key", NavNodeKey::Create("", bvector<Utf8String>())->AsJson(&json.GetAllocator()), json.GetAllocator());
            }
        else
            json.AddMember("key", navNode.GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
        json.AddMember("label", rapidjson::Value(navNode.GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
        if (navNode.HasChildren())
            json.AddMember("hasChildren", navNode.HasChildren(), json.GetAllocator());
        if (!navNode.GetDescription().empty())
            json.AddMember("description", rapidjson::Value(navNode.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
        if (!navNode.GetExpandedImageId().empty())
            json.AddMember("imageId", rapidjson::Value(navNode.GetExpandedImageId().c_str(), json.GetAllocator()), json.GetAllocator());
        if (!navNode.GetForeColor().empty())
            json.AddMember("foreColor", rapidjson::Value(navNode.GetForeColor().c_str(), json.GetAllocator()), json.GetAllocator());
        if (!navNode.GetBackColor().empty())
            json.AddMember("backColor", rapidjson::Value(navNode.GetBackColor().c_str(), json.GetAllocator()), json.GetAllocator());
        if (!navNode.GetFontStyle().empty() && !navNode.GetFontStyle().EqualsI("regular"))
            json.AddMember("fontStyle", rapidjson::Value(navNode.GetFontStyle().c_str(), json.GetAllocator()), json.GetAllocator());
        if (!navNode.IsSelectable())
            json.AddMember("isSelectionDisabled", !navNode.IsSelectable(), json.GetAllocator());
        if (navNode.IsEditable())
            json.AddMember("isEditable", navNode.IsEditable(), json.GetAllocator());
        if (navNode.IsCheckboxVisible())
            json.AddMember("isCheckboxVisible", navNode.IsCheckboxVisible(), json.GetAllocator());
        if (navNode.IsChecked())
            json.AddMember("isChecked", navNode.IsChecked(), json.GetAllocator());
        if (navNode.IsCheckboxEnabled())
            json.AddMember("isCheckboxEnabled", navNode.IsCheckboxEnabled(), json.GetAllocator());
        if (navNode.IsExpanded())
            json.AddMember("isExpanded", navNode.IsExpanded(), json.GetAllocator());
        if (navNode.GetUsersExtendedData().GetJson().MemberCount() > 0)
            json.AddMember("extendedData", rapidjson::Value(navNode.GetUsersExtendedData().GetJson(), json.GetAllocator()), json.GetAllocator());
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        if (navNodesPathElement.GetNode().IsNull())
            return json;

        json.SetObject();
        json.AddMember("node", navNodesPathElement.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
        json.AddMember("index", (uint64_t)navNodesPathElement.GetIndex(), json.GetAllocator());
        json.AddMember("isMarked", navNodesPathElement.IsMarked(), json.GetAllocator());

        rapidjson::Value childrenJson;
        childrenJson.SetArray();
        for (NodesPathElement const& child : navNodesPathElement.GetChildren())
            childrenJson.PushBack(child.AsJson(&json.GetAllocator()), json.GetAllocator());

        json.AddMember("children", childrenJson, json.GetAllocator());

        rapidjson::Value filteringData;
        filteringData.SetObject();

        filteringData.AddMember("occurances", navNodesPathElement.GetFilteringData().GetOccurances(), json.GetAllocator());
        filteringData.AddMember("childrenOccurances", navNodesPathElement.GetFilteringData().GetChildrenOccurances(), json.GetAllocator());

        json.AddMember("filteringData", filteringData, json.GetAllocator());

        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document _AsJson(KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();

        rapidjson::Value instances(rapidjson::kObjectType);
        for (auto& pair : keySet.GetInstanceKeys())
            {
            ECClassCP ecClass = pair.first;
            rapidjson::Value instanceIds(rapidjson::kArrayType);
            for (ECInstanceId const& instanceId : pair.second)
                instanceIds.PushBack(instanceId.GetValueUnchecked(), json.GetAllocator());
            instances.AddMember(rapidjson::Value(ecClass->GetName().c_str(), json.GetAllocator()), instanceIds, json.GetAllocator());
            }

        rapidjson::Value nodeKeys(rapidjson::kArrayType);
        for (NavNodeKeyCPtr const& key : keySet.GetNavNodeKeys())
            nodeKeys.PushBack(key->AsJson(&json.GetAllocator()), json.GetAllocator());

        json.AddMember("instanceKeys", instances, json.GetAllocator());
        json.AddMember("nodeKeys", nodeKeys, json.GetAllocator());

        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    KeySetPtr _GetKeySetFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        InstanceKeyMap instanceKeys;
        JsonValueCR instanceKeysJson = json["instanceKeys"];
        if (instanceKeysJson.isArray())
            {
            for (Json::ArrayIndex i = 0; i < instanceKeysJson.size(); ++i)
                {
                JsonValueCR instanceKeysEntry = instanceKeysJson[i];
                BeAssert(instanceKeysEntry.isArray() && 2 == instanceKeysEntry.size());
                ECClassCP ecClass = GetClassFromFullName(connection, instanceKeysEntry[0].asCString());
                if (nullptr == ecClass)
                    {
                    BeAssert(false);
                    continue;
                    }

                JsonValueCR instanceIdsJson = instanceKeysEntry[1];
                BeAssert(instanceIdsJson.isArray());
                bset<ECInstanceId> ids;
                for (Json::ArrayIndex j = 0; j < instanceIdsJson.size(); ++j)
                    ids.insert((ECInstanceId)BeInt64Id::FromString(instanceIdsJson[j].asCString()));
                instanceKeys[ecClass] = ids;
                }
            }

        NavNodeKeySet nodeKeys;
        for (JsonValueCR nodeKeyJson : json["nodeKeys"])
            nodeKeys.insert(_GetNavNodeKeyFromJson(connection, nodeKeyJson));

        return KeySet::Create(instanceKeys, nodeKeys);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Value _AsJson(ECEnumerationCR enumeration, rapidjson::Document::AllocatorType& allocator) const override
        {
        rapidjson::Value json(rapidjson::kArrayType);
        for (ECEnumeratorCP enumerator : enumeration.GetEnumerators())
            {
            rapidjson::Value choice(rapidjson::kObjectType);
            choice.AddMember("label", rapidjson::Value(enumerator->GetDisplayLabel().c_str(), allocator), allocator);
            if (enumerator->IsInteger())
                choice.AddMember("value", rapidjson::Value(enumerator->GetInteger()), allocator);
            else
                choice.AddMember("value", rapidjson::Value(enumerator->GetString().c_str(), allocator), allocator);
            json.PushBack(choice, allocator);
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Value _AsJson(KindOfQuantityCR koq, rapidjson::Document::AllocatorType& allocator) const override
        {
        rapidjson::Value json(rapidjson::kObjectType);
        json.AddMember("name", rapidjson::StringRef(koq.GetFullName().c_str()), allocator);
        json.AddMember("label", rapidjson::StringRef(koq.GetDisplayLabel().c_str()), allocator);
        json.AddMember("persistenceUnit", rapidjson::Value(koq.GetPersistenceUnit()->GetName().c_str(), allocator), allocator);
        json.AddMember("currentFormatId", rapidjson::Value(koq.GetDefaultPresentationFormat()->GetName().c_str(), allocator), allocator);
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Value _AsJson(RelatedClassCR relatedClass, rapidjson::Document::AllocatorType& allocator) const override
        {
        rapidjson::Value json(rapidjson::kObjectType);
        json.AddMember("sourceClassInfo", _AsJson(*relatedClass.GetSourceClass(), &allocator), allocator);
        json.AddMember("targetClassInfo", _AsJson(*relatedClass.GetTargetClass(), &allocator), allocator);
        json.AddMember("relationshipInfo", _AsJson(*relatedClass.GetRelationship(), &allocator), allocator);
        json.AddMember("isForwardRelationship", relatedClass.IsForwardRelationship(), allocator);
        json.AddMember("isPolymorphicRelationship", relatedClass.IsPolymorphic(), allocator);
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Value _AsJson(RelatedClassPathCR path, rapidjson::Document::AllocatorType& allocator) const override
        {
        rapidjson::Value json(rapidjson::kArrayType);
        for (RelatedClass const& relatedClass : path)
            {
            if (!relatedClass.IsValid())
                {
                BeAssert(false);
                continue;
                }
            json.PushBack(_AsJson(relatedClass, allocator), allocator);
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Value _AsJson(SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType& allocator) const override
        {
        rapidjson::Value info(rapidjson::kObjectType);
        info.AddMember("providerName", rapidjson::Value(selectionInfo.GetSelectionProviderName().c_str(), allocator), allocator);
        info.AddMember("level", selectionInfo.IsSubSelection() ? 1 : 0, allocator);
        return info;
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2018
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationLocalizationProvider : ILocalizationProvider
{
private:
    bvector<BeFileName> m_localeDirectories;
    mutable bmap<Utf8String, bmap<Utf8String, rapidjson::Document*>> m_cache;
private:
    static bvector<Utf8String> GetLocaleVariants(Utf8StringCR activeLocale)
        {
        bvector<Utf8String> variants;
        variants.push_back(activeLocale);
        Utf8String cultureNeutral;
        if (Utf8String::npos != activeLocale.GetNextToken(cultureNeutral, "-", 0) && cultureNeutral.length() != activeLocale.length())
            variants.push_back(cultureNeutral);
        if (!activeLocale.EqualsI("en") && !cultureNeutral.EqualsI("en"))
            variants.push_back("en");
        return variants;
        }
    bvector<BeFileName> GetLocalizationDirectoryPaths(Utf8StringCR locale) const
        {
        bvector<BeFileName> result;
        bvector<Utf8String> localeVariants = GetLocaleVariants(locale);
        for (Utf8StringCR locale : localeVariants)
            {
            for (BeFileName dir : m_localeDirectories)
                {
                BeFileName localeDir(dir);
                localeDir.AppendUtf8(locale.c_str());
                if (localeDir.DoesPathExist())
                    result.push_back(localeDir);
                }
            }
        return result;
        }
    bvector<BeFileName> GetLocalizationFilePaths(Utf8StringCR locale, Utf8StringCR ns) const
        {
        BeFileName nsFileName(ns);
        nsFileName.AppendExtension(L"json");
        bvector<BeFileName> result;
        bvector<BeFileName> dirs = GetLocalizationDirectoryPaths(locale);
        for (BeFileNameCR dir : dirs)
            {
            bvector<BeFileName> matches;
            BeDirectoryIterator::WalkDirsAndMatch(matches, dir, nsFileName.c_str(), true);
            result.insert(result.end(), matches.begin(), matches.end());
            }
        return result;
        }
    static rapidjson::Document ReadLocalizationFile(BeFileNameCR path)
        {
        rapidjson::Document json;
        BeFile file;
        if (BeFileStatus::Success != file.Open(path.c_str(), BeFileAccess::Read))
            {
            BeAssert(false);
            return json;
            }
        bvector<Byte> data;
        if (BeFileStatus::Success != file.ReadEntireFile(data))
            {
            BeAssert(false);
            return json;
            }
        data.push_back(0);
        json.Parse((Utf8CP)data.begin());
        return json;
        }
    static void MergeLocalizationData(RapidJsonValueR target, rapidjson::Document::AllocatorType& targetAllocator, RapidJsonValueCR source)
        {
        if (source.IsObject())
            {
            BeAssert(target.IsNull() || target.IsObject());
            if (target.IsNull())
                target.SetObject();
            for (auto sourceIter = source.MemberBegin(); sourceIter != source.MemberEnd(); ++sourceIter)
                {
                Utf8CP key = sourceIter->name.GetString();
                if (!target.HasMember(key))
                    target.AddMember(rapidjson::Value(key, targetAllocator), rapidjson::Value(), targetAllocator);
                MergeLocalizationData(target[key], targetAllocator, sourceIter->value);
                }
            }
        else if (source.IsString())
            {
            BeAssert(target.IsNull() || target.IsString());
            // note: do not overwrite if the string is already set - we parse
            // more specific locales first and finish with less specific ones
            if (target.IsNull())
                target.SetString(source.GetString(), targetAllocator);
            }
        else
            {
            BeAssert(false);
            }
        }
    rapidjson::Document* CreateNamespace(Utf8StringCR locale, Utf8StringCR ns) const
        {
        if (m_localeDirectories.empty() || locale.empty())
            return nullptr;

        rapidjson::Document* json = new rapidjson::Document();
        json->SetObject();
        bvector<BeFileName> filePaths = GetLocalizationFilePaths(locale, ns);
        for (BeFileNameCR filePath : filePaths)
            MergeLocalizationData(*json, json->GetAllocator(), ReadLocalizationFile(filePath));
        return json;
        }
    void ClearCache()
        {
        for (auto localeEntry : m_cache)
            {
            for (auto namespaceEntry : localeEntry.second)
                DELETE_AND_CLEAR(namespaceEntry.second);
            }
        m_cache.clear();
        }
protected:
    bool _GetString(Utf8StringCR locale, Utf8StringCR key, Utf8StringR localizedValue) const override
        {
        Utf8String ns;
        size_t pos;
        if (Utf8String::npos == (pos = key.GetNextToken(ns, ":", 0)))
            return false;

        auto localeIter = m_cache.find(locale);
        if (m_cache.end() == localeIter)
            localeIter = m_cache.Insert(locale, bmap<Utf8String, rapidjson::Document*>()).first;

        auto namespaceIter = localeIter->second.find(ns);
        if (localeIter->second.end() == namespaceIter)
            namespaceIter = localeIter->second.Insert(ns, CreateNamespace(locale, ns)).first;

        rapidjson::Value const* curr = namespaceIter->second;
        if (nullptr == curr)
            return false;

        Utf8String id(key.begin() + pos, key.end());
        bvector<Utf8String> idPath;
        BeStringUtilities::Split(id.c_str(), ".", idPath);
        for (Utf8StringCR idPart : idPath)
            {
            auto iter = curr->FindMember(idPart.c_str());
            if (curr->MemberEnd() == iter)
                return false;
            curr = &iter->value;
            }
        if (!curr || !curr->IsString())
            return false;
        localizedValue.AssignOrClear(curr->GetString());
        return true;
        }
public:
    IModelJsECPresentationLocalizationProvider(bvector<BeFileName> directories = bvector<BeFileName>()): m_localeDirectories(directories) {}
    ~IModelJsECPresentationLocalizationProvider() {ClearCache();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct IModelJsECPresentationStaticSetupHelper
{
private:
    IModelJsECPresentationSerializer* m_serializer;
public:
    IModelJsECPresentationStaticSetupHelper()
        : m_serializer(new IModelJsECPresentationSerializer())
        {
        IECPresentationManager::SetSerializer(m_serializer);
        }
    ~IModelJsECPresentationStaticSetupHelper()
        {
        IECPresentationManager::SetSerializer(nullptr);
        }
    IModelJsECPresentationSerializer& GetSerializer() {return *m_serializer;}
    };
static IModelJsECPresentationStaticSetupHelper s_staticSetup;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager* ECPresentationUtils::CreatePresentationManager(Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin& locations, 
    IJsonLocalState& localState, Utf8StringCR id, bvector<Utf8String> const& localeDirectories, bmap<int, unsigned> taskAllocationSlots, Utf8StringCR mode)
    {
    BeFileName assetsDir = locations.GetDgnPlatformAssetsDirectory();
    BeFileName tempDir = locations.GetLocalTempDirectoryBaseName();
    tempDir.AppendToPath(L"ecpresentation");
    if (!tempDir.DoesPathExist())
        BeFileName::CreateNewDirectory(tempDir.c_str());
    if (!id.empty())
        {
        tempDir.AppendToPath(WString(id.c_str(), true).c_str());
        if (!tempDir.DoesPathExist())
            BeFileName::CreateNewDirectory(tempDir.c_str());
        }
    RulesDrivenECPresentationManager::Paths pathParams(assetsDir, tempDir);

    RulesDrivenECPresentationManager::Params::MultiThreadingParams threadingParams(taskAllocationSlots);

    bvector<BeFileName> localeDirectoryPaths;
    for (Utf8StringCR dir : localeDirectories)
        localeDirectoryPaths.push_back(BeFileName(dir).AppendSeparator());
    
    RulesDrivenECPresentationManager::Params params(pathParams);
    params.SetMultiThreadingParams(threadingParams);
    params.SetLocalizationProvider(new IModelJsECPresentationLocalizationProvider(localeDirectoryPaths));
    params.SetLocalState(&localState);
    params.SetMode(mode.Equals("ro") ? RulesDrivenECPresentationManager::Mode::ReadOnly : RulesDrivenECPresentationManager::Mode::ReadWrite);
    return new RulesDrivenECPresentationManager(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetupRulesetDirectories(RulesDrivenECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str()));
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetupSupplementalRulesetDirectories(RulesDrivenECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str())));
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::GetRulesets(SimpleRuleSetLocater& locater, Utf8StringCR rulesetId)
    {
    bvector<PresentationRuleSetPtr> rulesets = locater.LocateRuleSets(rulesetId.c_str());
    Json::Value json(Json::arrayValue);
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        {
        Json::Value hashedRulesetJson;
        hashedRulesetJson["ruleset"] = ruleset->WriteToJsonValue();
        hashedRulesetJson["hash"] = ruleset->GetHash();
        json.append(hashedRulesetJson);
        }
    return ECPresentationResult(std::move(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::AddRuleset(SimpleRuleSetLocater& locater, Utf8StringCR rulesetJsonString)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::ReadFromJsonString(rulesetJsonString);
    if (ruleset.IsNull())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "Failed to create rule set from serialized JSON");
    locater.AddRuleSet(*ruleset);
    rapidjson::Document result;
    result.SetString(ruleset->GetHash().c_str(), result.GetAllocator());
    return ECPresentationResult(std::move(result));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::RemoveRuleset(SimpleRuleSetLocater& locater, Utf8StringCR rulesetId, Utf8StringCR hash)
    {
    bvector<PresentationRuleSetPtr> rulesets = locater.LocateRuleSets(rulesetId.c_str());
    for (PresentationRuleSetPtr const& ruleset : rulesets)
        {
        if (ruleset->GetHash().Equals(hash))
            {
            locater.RemoveRuleSet(rulesetId);
            return ECPresentationResult(rapidjson::Value(true));
            }
        }
    return ECPresentationResult(rapidjson::Value(false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::ClearRulesets(SimpleRuleSetLocater& locater)
    {
    locater.Clear();
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value GetCommonOptions(JsonValueCR params)
    {
    if (!params.isMember("rulesetId") || !params["rulesetId"].isString())
        {
        static const Json::Value s_empty;
        return s_empty;
        }
    RulesDrivenECPresentationManager::CommonOptions options(params["rulesetId"].asCString());
    options.SetLocale((params.isMember("locale") && params["locale"].isString()) ? params["locale"].asCString() : "");
    options.SetPriority((params.isMember("priority") && params["priority"].isInt()) ? params["priority"].asInt() : DEFAULT_REQUEST_PRIORITY);
    return options.GetJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static RulesDrivenECPresentationManager::NavigationOptions GetNavigationOptions(JsonValueCR params)
    {
    RulesDrivenECPresentationManager::NavigationOptions options(GetCommonOptions(params));
    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static RulesDrivenECPresentationManager::ContentOptions GetContentOptions(JsonValueCR params)
    {
    RulesDrivenECPresentationManager::ContentOptions options(GetCommonOptions(params));
    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PageOptions GetPageOptions(JsonValueCR params)
    {
    PageOptions pageOptions;
    if (params.isMember("paging"))
        {
        if (params["paging"].isMember("start"))
            pageOptions.SetPageStart((size_t)params["paging"]["start"].asUInt64());
        if (params["paging"].isMember("size"))
            pageOptions.SetPageSize((size_t)params["paging"]["size"].asUInt64());
        }
    return pageOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::GetRulesetVariableValue(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType)
    {
    rapidjson::Document response;
    IUserSettings& settings = manager.GetUserSettings().GetSettings(rulesetId);

    if (variableType.Equals("bool"))
        response.SetBool(settings.GetSettingBoolValue(variableId.c_str()));
    else if(variableType.Equals("string"))
        response.SetString(settings.GetSettingValue(variableId.c_str()).c_str(), response.GetAllocator());
    else if (variableType.Equals("int"))
        response.SetInt64(settings.GetSettingIntValue(variableId.c_str()));
    else if (variableType.Equals("int[]"))
        {
        response.SetArray();
        bvector<int64_t> intValues = settings.GetSettingIntValues(variableId.c_str());
        for (int64_t value : intValues)
            response.PushBack(value, response.GetAllocator());
        }
    else if (variableType.Equals("id64"))
        response.SetString(BeInt64Id(settings.GetSettingIntValue(variableId.c_str())).ToHexStr().c_str(), response.GetAllocator());
    else if (variableType.Equals("id64[]"))
        {
        response.SetArray();
        bvector<int64_t> intValues = settings.GetSettingIntValues(variableId.c_str());
        for (int64_t value : intValues)
            response.PushBack(rapidjson::Value(BeInt64Id(value).ToHexStr().c_str(), response.GetAllocator()), response.GetAllocator());
        }
    else
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "type");
    return ECPresentationResult(std::move(response));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationResult ECPresentationUtils::SetRulesetVariableValue(RulesDrivenECPresentationManager& manager, Utf8StringCR rulesetId, Utf8StringCR variableId, Utf8StringCR variableType, JsonValueCR value)
    {
    IUserSettings& settings = manager.GetUserSettings().GetSettings(rulesetId);
    if (variableType.Equals("bool"))
        settings.SetSettingBoolValue(variableId.c_str(), value.asBool());
    else if (variableType.Equals("string"))
        settings.SetSettingValue(variableId.c_str(), value.asCString());
    else if (variableType.Equals("id64"))
        settings.SetSettingIntValue(variableId.c_str(), BeInt64Id::FromString(value.asCString()).GetValue());
    else if (variableType.Equals("id64[]"))
        {
        bvector<int64_t> values;
        for (Json::ArrayIndex i = 0; i < value.size(); i++)
            values.push_back(BeInt64Id::FromString(value[i].asCString()).GetValue());
        settings.SetSettingIntValues(variableId.c_str(), values);
        }
    else if(variableType.Equals("int"))
        settings.SetSettingIntValue(variableId.c_str(), value.asInt64());
    else if (variableType.Equals("int[]"))
        {
        bvector<int64_t> values;
        for (Json::ArrayIndex i = 0; i < value.size(); i++)
            values.push_back(value[i].asInt64());
        settings.SetSettingIntValues(variableId.c_str(), values);
        }
    else
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "type");
    return ECPresentationResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NavNodeCPtr> GetNode(IECPresentationManager& mgr, IConnectionCR connection, JsonValueCR params, PresentationRequestContextCR context)
    {
    NavNodeKeyCPtr key = NavNodeKey::FromJson(connection, params["nodeKey"]);
    if (key.IsNull())
        return NavNodeCPtr();
    return mgr.GetNode(connection.GetECDb(), *key, GetNavigationOptions(params).GetJson(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetRootNodesCount(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    return manager.GetRootNodesCount(db, GetNavigationOptions(params).GetJson(), context)
        .then([](size_t count)
            {
            return ECPresentationResult(rapidjson::Value((int64_t) count));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetRootNodes(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    return manager.GetRootNodes(db, GetPageOptions(params), GetNavigationOptions(params).GetJson(), context)
        .then([context](DataContainer<NavNodeCPtr> nodes)
            {
            context.OnTaskStart();
            rapidjson::Document json;
            json.SetArray();
            for (NavNodeCPtr const& node : nodes)
                json.PushBack(node->AsJson(&json.GetAllocator()), json.GetAllocator());
            return ECPresentationResult(std::move(json));
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetChildrenCount(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetNavigationOptions(params).GetJson();
    return GetNode(manager, *connection, params, context)
        .then([&manager, &db, options, context](NavNodeCPtr parentNode)
            {
            context.OnTaskStart();

            if (parentNode.IsNull())
                {
                BeAssert(false);
                return folly::makeFutureWith([]() {return ECPresentationResult(ECPresentationStatus::InvalidArgument, "parent node");});
                }

            return manager.GetChildrenCount(db, *parentNode, options, context)
                .then([](size_t count)
                    {
                    return ECPresentationResult(rapidjson::Value((int64_t) count));
                    });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetChildren(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetNavigationOptions(params).GetJson();
    PageOptions pageOptions = GetPageOptions(params);
    return GetNode(manager, *connection, params, context)
        .then([&manager, &db, options, pageOptions, context](NavNodeCPtr parentNode)
            {
            context.OnTaskStart();

            if (parentNode.IsNull())
                {
                BeAssert(false);
                return folly::makeFutureWith([]() {return ECPresentationResult(ECPresentationStatus::InvalidArgument, "parent node");});
                }

            return manager.GetChildren(db, *parentNode, pageOptions, options, context)
                .then([context](DataContainer<NavNodeCPtr> nodes)
                    {
                    context.OnTaskStart();
                    rapidjson::Document json;
                    json.SetArray();
                    for (NavNodeCPtr const& node : nodes)
                        json.PushBack(node->AsJson(&json.GetAllocator()), json.GetAllocator());
                    return ECPresentationResult(std::move(json));
                    });
            });
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 06/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult>  ECPresentationUtils::GetNodesPaths(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    if (!params.isMember("markedIndex"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "markedIndex");

    bvector<bvector<ECInstanceKey>> keys;
    JsonValueCR keyArraysJson = params["paths"];
    if (!keyArraysJson.isArray())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "paths");

    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetNavigationOptions(params).GetJson();
    for (Json::ArrayIndex x = 0; x < keyArraysJson.size(); x++)
        {
        JsonValueCR keysJson = keyArraysJson[x];

        if (!keysJson.isArray())
            return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("paths[%" PRIu64 "]", (uint64_t)x));

        keys.push_back(bvector<ECInstanceKey>());
        for (Json::ArrayIndex i = 0; i < keysJson.size(); i++)
            {
            ECInstanceId id;
            ECInstanceId::FromString(id, keysJson[i]["id"].asCString());
            if (!id.IsValid())
                return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("paths[%" PRIu64 "][%" PRIu64 "].id", (uint64_t)x, (uint64_t)i));

            ECClassCP ecClass = IModelJsECPresentationSerializer::GetClassFromFullName(*connection, keysJson[i]["className"].asCString());
            if (ecClass == nullptr)
                return ECPresentationResult(ECPresentationStatus::InvalidArgument, Utf8PrintfString("paths[%" PRIu64 "][%" PRIu64 "].className", (uint64_t)x, (uint64_t)i));

            keys[x].push_back(ECInstanceKey(ecClass->GetId(), id));
            }
        }

    return manager.GetNodePaths(db, keys, params["markedIndex"].asInt64(), options, context)
        .then([context](bvector<NodesPathElement> result)
            {
            context.OnTaskStart();
            rapidjson::Document response;
            response.SetArray();
            for (size_t i = 0; i < result.size(); i++)
                response.PushBack(result[i].AsJson(&response.GetAllocator()), response.GetAllocator());
            return ECPresentationResult(std::move(response));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                06/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult>  ECPresentationUtils::GetFilteredNodesPaths(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    if (!params.isMember("filterText"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "filterText");

    Json::Value options = GetNavigationOptions(params).GetJson();
    return manager.GetFilteredNodePaths(db, params["filterText"].asCString(), options, context)
        .then([context](bvector<NodesPathElement> result)
            {
            context.OnTaskStart();
            rapidjson::Document response;
            response.SetArray();
            for (size_t i = 0; i < result.size(); i++)
                response.PushBack(result[i].AsJson(&response.GetAllocator()), response.GetAllocator());
            return ECPresentationResult(std::move(response));
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NavNodesContainer> GetNodes(RulesDrivenECPresentationManager& manager, ECDbCR db, NavNodeCP parentNode,
    JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    if (parentNode)
        return manager.GetChildren(db, *parentNode, PageOptions(), jsonOptions, context);
    return manager.GetRootNodes(db, PageOptions(), jsonOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<folly::Unit> LoadHierarchyIncrementally(RulesDrivenECPresentationManager& manager, ECDbCR db, NavNodeCP parentNode,
    JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    return GetNodes(manager, db, parentNode, jsonOptions, context).then([&, jsonOptions, context](NavNodesContainer container)
        {
        std::vector<folly::Future<folly::Unit>> childrenFutures;
        for (NavNodeCPtr node : container)
            {
            if (node->HasChildren())
                childrenFutures.push_back(LoadHierarchyIncrementally(manager, db, node.get(), jsonOptions, context));
            }
        return folly::collect(childrenFutures).then();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::LoadHierarchy(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    Json::Value options = GetCommonOptions(params);
    return LoadHierarchyIncrementally(manager, db, nullptr, options, context).then([]()
        {
        return ECPresentationResult();
        });
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct DescriptorOverrideHelper
{
private:
    JsonValueCR m_overridesJson;
private:
    Utf8CP GetSortingFieldName() const { return m_overridesJson["sortingFieldName"].asCString(); }
    SortDirection GetSortDirection() const { return (SortDirection)m_overridesJson["sortDirection"].asInt(); }
    int GetContentFlags() const { return m_overridesJson["contentFlags"].asInt(); }
    Utf8CP GetFilterExpression() const { return m_overridesJson["filterExpression"].isNull() ? "" : m_overridesJson["filterExpression"].asCString(); }
    bvector<Utf8String> GetHiddenFieldNames() const
        {
        bvector<Utf8String> names;
        JsonValueCR arr = m_overridesJson["hiddenFieldNames"];
        for (Json::ArrayIndex i = 0; i < arr.size(); ++i)
            names.push_back(arr[i].asCString());
        return names;
        }
public:
    DescriptorOverrideHelper(JsonValueCR json) : m_overridesJson(json) {}
    Utf8CP GetDisplayType() const { return m_overridesJson["displayType"].asCString(); }
    ContentDescriptorPtr GetOverridenDescriptor(ContentDescriptorCR defaultDescriptor) const
        {
        if (!defaultDescriptor.GetPreferredDisplayType().Equals(GetDisplayType()))
            {
            BeAssert(false);
            return ContentDescriptor::Create(defaultDescriptor);
            }
        ContentDescriptorPtr descriptorCopy = ContentDescriptor::Create(defaultDescriptor);
        descriptorCopy->SetSortingField(GetSortingFieldName());
        descriptorCopy->SetSortDirection(GetSortDirection());
        descriptorCopy->SetContentFlags(GetContentFlags());
        descriptorCopy->SetFilterExpression(GetFilterExpression());
        bvector<Utf8String> hiddenFieldNames = GetHiddenFieldNames();
        for (Utf8StringCR name : hiddenFieldNames)
            descriptorCopy->RemoveField(name.c_str());
        return descriptorCopy;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetDisplayType(JsonValueCR params)
    {
    return params.isMember("displayType") ? params["displayType"].asCString() : ContentDisplayType::Undefined;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static int GetContentFlags(JsonValueCR params) { return params.isMember("contentFlags") ? params["contentFlags"].asInt() : 0; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static KeySetPtr GetKeys(IConnectionCR connection, JsonValueCR params)
    {
    if (!params.isMember("keys"))
        return KeySet::Create();
    return KeySet::FromJson(connection, params["keys"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentDescriptor(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    Json::Value options = GetContentOptions(params).GetJson();
    return manager.GetContentDescriptor(db, GetDisplayType(params), GetContentFlags(params), *GetKeys(*connection, params), nullptr, options, context)
        .then([context](ContentDescriptorCPtr descriptor)
            {
            context.OnTaskStart();
            if (descriptor.IsValid())
                return ECPresentationResult(descriptor->AsJson());
            return ECPresentationResult();
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContent(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    JsonValueCR descriptorOverridesJson = params["descriptorOverrides"];
    PageOptions pageOptions = GetPageOptions(params);
    Json::Value options = GetContentOptions(params).GetJson();
    return manager.GetContentDescriptor(db, GetDisplayType(descriptorOverridesJson), GetContentFlags(descriptorOverridesJson), *GetKeys(*connection, params), nullptr, options, context)
        .then([&manager, descriptorOverridesJson, pageOptions, context](ContentDescriptorCPtr descriptor)
            {
            context.OnTaskStart();
            if (descriptor.IsNull())
                return folly::makeFutureWith([]() { return ECPresentationResult(); });
            ContentDescriptorCPtr overridenDescriptor = DescriptorOverrideHelper(descriptorOverridesJson).GetOverridenDescriptor(*descriptor);
            return manager.GetContent(*overridenDescriptor, pageOptions, context).then([context](ContentCPtr content)
                {
                context.OnTaskStart();
                return ECPresentationResult(content->AsJson());
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetContentSetSize(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    JsonValueCR descriptorOverridesJson = params["descriptorOverrides"];
    Json::Value options = GetContentOptions(params).GetJson();
    return manager.GetContentDescriptor(db, GetDisplayType(descriptorOverridesJson), GetContentFlags(descriptorOverridesJson), *GetKeys(*connection, params), nullptr, options, context)
        .then([&manager, descriptorOverridesJson, context](ContentDescriptorCPtr descriptor) {
            context.OnTaskStart();
            if (descriptor.IsNull())
                return folly::makeFutureWith([]() { return ECPresentationResult(rapidjson::Value(0)); });
            ContentDescriptorCPtr overridenDescriptor = DescriptorOverrideHelper(descriptorOverridesJson).GetOverridenDescriptor(*descriptor);
            return manager.GetContentSetSize(*overridenDescriptor, context).then([context](size_t size)
                {
                context.OnTaskStart();
                return ECPresentationResult(rapidjson::Value((int64_t)size));
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 06/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetDistinctValues(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    if (!params.isMember("fieldName"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "fieldName");
    if (!params.isMember("maximumValueCount"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "maximumValueCount");

    IConnectionCP connection = manager.GetConnections().GetConnection(db);
    JsonValueCR descriptorOverridesJson = params["descriptorOverrides"];
    Json::Value options = GetContentOptions(params).GetJson();
    Utf8String fieldName = params["fieldName"].asString();
    int64_t maximumValueCount = params["maximumValueCount"].asInt64();
    return manager.GetContentDescriptor(db, GetDisplayType(descriptorOverridesJson), GetContentFlags(descriptorOverridesJson), *GetKeys(*connection, params), nullptr, options, context)
        .then([&manager, descriptorOverridesJson, fieldName, maximumValueCount, context] (ContentDescriptorCPtr descriptor)
            {
            if (descriptor.IsNull())
                return folly::makeFutureWith([] () { return ECPresentationResult(ECPresentationStatus::InvalidArgument, "descriptor"); });

            ContentDescriptorPtr overridenDescriptor = DescriptorOverrideHelper(descriptorOverridesJson).GetOverridenDescriptor(*descriptor);
            bvector<ContentDescriptor::Field*> fieldsCopy = descriptor->GetAllFields();
            for (ContentDescriptor::Field const* field : fieldsCopy)
                {
                if (!field->GetName().Equals(fieldName))
                    overridenDescriptor->RemoveField(field->GetName().c_str());
                }
            overridenDescriptor->AddContentFlag(ContentFlags::DistinctValues);

            return manager.GetContent(*overridenDescriptor, PageOptions(), context)
                .then([fieldName, maximumValueCount, context](ContentCPtr content)
                {
                context.OnTaskStart();
                DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();

                rapidjson::Document response;
                response.SetArray();
                for (size_t i = 0; i < contentSet.GetSize(); i++)
                    {
                    if (maximumValueCount != 0 && response.Size() >= maximumValueCount)
                        break;

                    RapidJsonValueCR value = contentSet.Get(i)->GetDisplayValues()[fieldName.c_str()];
                    if (value.IsNull() || (value.IsString() && 0 == value.GetStringLength()))
                        continue;

                    response.PushBack(rapidjson::Value(value.GetString(), response.GetAllocator()), response.GetAllocator());
                    }
                return ECPresentationResult(std::move(response));
                });
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/18
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ECPresentationResult> ECPresentationUtils::GetDisplayLabel(RulesDrivenECPresentationManager& manager, ECDbR db, JsonValueCR params, PresentationRequestContextCR context)
    {
    IConnectionCPtr connection = manager.GetConnections().GetConnection(db);
    if (!params.isMember("key"))
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "key");

    ECClassCP ecClass = IModelJsECPresentationSerializer::GetClassFromFullName(*connection, params["key"]["className"].asCString());
    if (ecClass == nullptr)
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "key.className");

    ECInstanceId id;
    ECInstanceId::FromString(id, params["key"]["id"].asCString());
    if (!id.IsValid())
        return ECPresentationResult(ECPresentationStatus::InvalidArgument, "key.id");

    return manager.GetDisplayLabel(db, ECInstanceKey(ecClass->GetId(), id), Json::Value(), context)
        .then([context](Utf8String label)
        {
        context.OnTaskStart();
        rapidjson::Document responseJson;
        responseJson.SetString(label.c_str(), responseJson.GetAllocator());
        return ECPresentationResult(std::move(responseJson));
        });
    }
