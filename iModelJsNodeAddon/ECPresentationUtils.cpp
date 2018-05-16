/*--------------------------------------------------------------------------------------+
|
|     $Source: ECPresentationUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationUtils.h"

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
        propertyJson.AddMember("classInfo", _AsJson(property.GetPropertyClass(), &json.GetAllocator()), json.GetAllocator());
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
        fieldBaseJson.AddMember("pathToPrimary", _AsJson(nestedContentField.GetRelationshipPath(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());

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
            member.AddMember("name", rapidjson::StringRef(nestedField->GetName().c_str()), typeDescriptionBaseJson.GetAllocator());
            member.AddMember("label", rapidjson::StringRef(nestedField->GetLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
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
        json.AddMember("displayType", rapidjson::StringRef(contentDescriptor.GetPreferredDisplayType().c_str()), json.GetAllocator());
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
            json["selectClasses"].PushBack(selectClassJson, json.GetAllocator());
            }

        bvector<ContentDescriptor::Field*> visibleFields = contentDescriptor.GetVisibleFields();
        json.AddMember("fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (ContentDescriptor::Field const* field : visibleFields)
            json["fields"].PushBack(field->AsJson(&json.GetAllocator()), json.GetAllocator());
        if (-1 != contentDescriptor.GetSortingFieldIndex())
            {
            json.AddMember("sortingFieldName", rapidjson::StringRef(contentDescriptor.GetSortingField()->GetName().c_str()), json.GetAllocator());
            json.AddMember("sortDirection", (int)contentDescriptor.GetSortDirection(), json.GetAllocator());
            }

        json.AddMember("contentFlags", contentDescriptor.GetContentFlags(), json.GetAllocator());
        json.AddMember("connectionId", rapidjson::StringRef(contentDescriptor.GetConnection().GetId().c_str()), json.GetAllocator());
        json.AddMember("filterExpression", rapidjson::StringRef(contentDescriptor.GetFilterExpression().c_str()), json.GetAllocator());
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
        for (Utf8StringCR pathElement : navNodeKey.GetPathFromRoot())
            pathJson.PushBack(rapidjson::Value(pathElement.c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

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
        if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
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
        if (0 == strcmp(NAVNODE_TYPE_ECInstanceNode, type))
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
    void _AsJson(ECInstanceNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.AddMember("instanceKey", _AsJson(ecInstanceNodeKey.GetClassInstanceKey(), &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECInstanceNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        Utf8CP className = json["instanceKey"]["className"].asCString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        ECInstanceId instanceId(ECInstanceId::FromString(json["instanceKey"]["id"].asCString()));
        return ECInstanceNodeKey::Create(ECClassInstanceKey(ecClass, instanceId), ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECInstanceNodeKeyPtr _GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        Utf8CP className = json["instanceKey"]["className"].GetString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        ECInstanceId instanceId(ECInstanceId::FromString(json["instanceKey"]["id"].GetString()));
        return ECInstanceNodeKey::Create(ECClassInstanceKey(ecClass, instanceId), ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.AddMember("className", rapidjson::StringRef(groupingNodeKey.GetECClass().GetFullName()), navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const override
        {
        Utf8CP className = json["className"].asCString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassGroupingNodeKeyPtr _GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        Utf8CP className = json["className"].GetString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
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
        Utf8CP className = json["className"].asCString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        Utf8CP propertyName = json["propertyName"].asCString();
        if (!json.isMember("groupingValue"))
            return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, nullptr, ParseNodeKeyHashPath(json["pathFromRoot"]));

        rapidjson::Document groupingValue;
        groupingValue.Parse(Json::FastWriter().write(json["groupingValue"]).c_str());
        return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, &groupingValue, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECPropertyGroupingNodeKeyPtr _GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const override
        {
        Utf8CP className = json["className"].GetString();
        ECClassCP ecClass = GetClassFromFullName(connection, className);
        Utf8CP propertyName = json["propertyName"].GetString();
        rapidjson::Value const* groupingValue = nullptr;
        if (json.HasMember("groupingValue"))
            groupingValue = &json["groupingValue"];
        return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, groupingValue, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const override
        {
        navNodeKeyBaseJson.AddMember("label", rapidjson::Value(labelGroupingNodeKey.GetLabel().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const override
        {
        Utf8CP label = json["label"].asCString();
        return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["pathFromRoot"]));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    LabelGroupingNodeKeyPtr _GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const override
        {
        Utf8CP label = json["label"].GetString();
        return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["pathFromRoot"]));
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
        json.AddMember("description", rapidjson::Value(navNode.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("imageId", rapidjson::Value(navNode.GetExpandedImageId().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("foreColor", rapidjson::Value(navNode.GetForeColor().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("backColor", rapidjson::Value(navNode.GetBackColor().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("fontStyle", rapidjson::Value(navNode.GetFontStyle().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("type", rapidjson::Value(navNode.GetType().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("hasChildren", navNode.HasChildren(), json.GetAllocator());
        json.AddMember("isSelectable", navNode.IsSelectable(), json.GetAllocator());
        json.AddMember("isEditable", navNode.IsEditable(), json.GetAllocator());
        json.AddMember("isChecked", navNode.IsChecked(), json.GetAllocator());
        json.AddMember("isCheckboxVisible", navNode.IsCheckboxVisible(), json.GetAllocator());
        json.AddMember("isCheckboxEnabled", navNode.IsCheckboxEnabled(), json.GetAllocator());
        json.AddMember("isExpanded", navNode.IsExpanded(), json.GetAllocator());
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
    rapidjson::Document _AsJson(SelectionChangedEvent const&, rapidjson::Document::AllocatorType* allocator) const override
        {
        BeAssert(false && "Not expected to be used");
        return rapidjson::Document(allocator);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    SelectionChangedEventPtr _GetSelectionChangedEventFromJson(IConnectionCacheCR, JsonValueCR json) const override
        {
        BeAssert(false && "Not expected to be used");
        return nullptr;
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
        json.AddMember("persistenceUnit", rapidjson::Value(koq.GetPersistenceUnit().ToText(true).c_str(), allocator), allocator);
        json.AddMember("currentFusId", rapidjson::Value(koq.GetDefaultPresentationUnit().ToText(true).c_str(), allocator), allocator);
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
            rapidjson::Value relatedClassJson(rapidjson::kObjectType);
            relatedClassJson.AddMember("sourceClassInfo", _AsJson(*relatedClass.GetSourceClass(), &allocator), allocator);
            relatedClassJson.AddMember("targetClassInfo", _AsJson(*relatedClass.GetTargetClass(), &allocator), allocator);
            relatedClassJson.AddMember("relationshipInfo", _AsJson(*relatedClass.GetRelationship(), &allocator), allocator);
            relatedClassJson.AddMember("isForwardRelationship", relatedClass.IsForwardRelationship(), allocator);
            relatedClassJson.AddMember("isPolymorphicRelationship", relatedClass.IsPolymorphic(), allocator);
            json.PushBack(relatedClassJson, allocator);
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Value _AsJson(SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType& allocator) const override
        {
        rapidjson::Value info(rapidjson::kObjectType);
        info.AddMember("providerName", rapidjson::StringRef(selectionInfo.GetSelectionProviderName().c_str()), allocator);
        info.AddMember("level", selectionInfo.IsSubSelection() ? 1 : 0, allocator);
        return info;
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2018
+===============+===============+===============+===============+===============+======*/
struct SerializerSetupHelper
    {
    SerializerSetupHelper() {IECPresentationManager::SetSerializer(new IModelJsECPresentationSerializer());}
    ~SerializerSetupHelper() {IECPresentationManager::SetSerializer(nullptr);}
    };
static SerializerSetupHelper s_setupSerializer;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager* ECPresentationUtils::CreatePresentationManager(IConnectionManagerR connections, Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin& locations)
    {
    BeFileName assetsDir = locations.GetDgnPlatformAssetsDirectory();
    BeFileName tempDir = locations.GetLocalTempDirectoryBaseName();
    RulesDrivenECPresentationManager::Paths paths(assetsDir, tempDir);
    RulesDrivenECPresentationManager* manager = new RulesDrivenECPresentationManager(connections, paths);

    BeFileName supplementalsDirectory = BeFileName(assetsDir).AppendToPath(L"PresentationRules");
    manager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalsDirectory.GetNameUtf8().c_str())));

    return manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::SetupRulesetDirectories(RulesDrivenECPresentationManager& manager, bvector<Utf8String> const& directories)
    {
    Utf8String joinedDirectories = BeStringUtilities::Join(directories, ";");
    manager.GetLocaters().RegisterLocater(*DirectoryRuleSetLocater::Create(joinedDirectories.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static JsonValueCR GetManagerOptions(JsonValueCR params)
    {
    if (!params.isMember("options") || !params["options"].isObject())
        {
        static Json::Value s_default;
        return s_default;
        }
    return params["options"];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PageOptions GetPageOptions(JsonValueCR params)
    {
    PageOptions pageOptions;
    if (params.isMember("pageOptions"))
        {
        if (params["pageOptions"].isMember("pageStart"))
            pageOptions.SetPageStart((size_t)params["pageOptions"]["pageStart"].asUInt64());
        if (params["pageOptions"].isMember("pageSize"))
            pageOptions.SetPageSize((size_t)params["pageOptions"]["pageSize"].asUInt64());
        }
    return pageOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NavNodeCPtr> GetNode(IECPresentationManager& mgr, IConnectionCR connection, JsonValueCR params)
    {
    NavNodeKeyCPtr key = NavNodeKey::FromJson(connection, params["nodeKey"]);
    if (key.IsNull())
        {
        // wip: throw
        return NavNodeCPtr();
        }
    return mgr.GetNode(connection.GetECDb(), *key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetRootNodesCount(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    size_t count = manager.GetRootNodesCount(db, GetManagerOptions(params)).get();
    response.SetInt64((int64_t)count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetRootNodes(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    response.SetArray();
    DataContainer<NavNodeCPtr> nodes = manager.GetRootNodes(db, GetPageOptions(params), GetManagerOptions(params)).get();
    for (NavNodeCPtr const& node : nodes)
        response.PushBack(node->AsJson(&response.GetAllocator()), response.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetChildrenCount(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    IConnectionCPtr connection = manager.Connections().GetConnection(db);
    NavNodeCPtr parentNode = GetNode(manager, *connection, params).get(); // wip: stop using .get()
    size_t count = manager.GetChildrenCount(db, *parentNode, GetManagerOptions(params)).get();
    response.SetInt64((int64_t)count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetChildren(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    IConnectionCPtr connection = manager.Connections().GetConnection(db);
    NavNodeCPtr parentNode = GetNode(manager, *connection, params).get(); // wip: stop using .get()
    response.SetArray();
    DataContainer<NavNodeCPtr> nodes = manager.GetChildren(db, *parentNode, GetPageOptions(params), GetManagerOptions(params)).get();
    for (NavNodeCPtr const& node : nodes)
        response.PushBack(node->AsJson(&response.GetAllocator()), response.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetNodePaths(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetFilteredNodesPaths(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
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
    ContentDescriptorCPtr GetOverridenDescriptor(ContentDescriptorCR defaultDescriptor) const
        {
        if (!defaultDescriptor.GetPreferredDisplayType().Equals(GetDisplayType()))
            {
            BeAssert(false);
            return &defaultDescriptor;
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
void ECPresentationUtils::GetContentDescriptor(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    IConnectionCPtr connection = manager.Connections().GetConnection(db);
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, GetDisplayType(params), *GetKeys(*connection, params), nullptr, GetManagerOptions(params)).get();
    if (descriptor.IsValid())
        response = descriptor->AsJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContent(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    IConnectionCPtr connection = manager.Connections().GetConnection(db);
    DescriptorOverrideHelper helper(params["descriptorOverrides"]);
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, helper.GetDisplayType(), *GetKeys(*connection, params), nullptr, GetManagerOptions(params)).get();
    if (descriptor.IsNull())
        return;

    ContentDescriptorCPtr overridenDescriptor = helper.GetOverridenDescriptor(*descriptor);
    ContentCPtr content = manager.GetContent(*overridenDescriptor, GetPageOptions(params)).get();
    response = content->AsJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetContentSetSize(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    IConnectionCPtr connection = manager.Connections().GetConnection(db);
    DescriptorOverrideHelper helper(params["descriptorOverrides"]);
    ContentDescriptorCPtr descriptor = manager.GetContentDescriptor(db, helper.GetDisplayType(), *GetKeys(*connection, params), nullptr, GetManagerOptions(params)).get();
    if (descriptor.IsNull())
        return;

    ContentDescriptorCPtr overridenDescriptor = helper.GetOverridenDescriptor(*descriptor);
    size_t size = manager.GetContentSetSize(*overridenDescriptor).get();
    response.SetUint64(size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::GetDistinctValues(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::SaveValueChange(IECPresentationManagerR manager, ECDbR db, JsonValueCR params, rapidjson::Document& response)
    {
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::AddRuleSet(SimpleRuleSetLocater& locater, Utf8StringCR rulesetJsonString)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(rulesetJsonString);
    if (ruleSet != nullptr)
        locater.AddRuleSet(*ruleSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::RemoveRuleSet(SimpleRuleSetLocater& locater, Utf8StringCR ruleSetId)
    {
    locater.RemoveRuleSet(ruleSetId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kililnskas                 05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationUtils::ClearRuleSets(SimpleRuleSetLocater& locater)
    {
    locater.Clear();
    }

