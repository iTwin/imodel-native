/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECPresentationSerializer.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP IModelJsECPresentationSerializer::GetClassFromFullName(IConnectionCR connection, Utf8CP fullClassName)
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ConnectionEvent const&, rapidjson::Document::AllocatorType* allocator) const
    {
    BeAssert(false && "Not expected to be used");
    return rapidjson::Document(allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContentFieldEditor const& editor, rapidjson::Document::AllocatorType* allocator) const
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator) const
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    rapidjson::Value propertyJson(rapidjson::kObjectType);
    propertyJson.AddMember("classInfo", IModelJsECPresentationSerializer::_AsJson(property.GetProperty().GetClass(), &json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("name", rapidjson::Value(property.GetProperty().GetName().c_str(), json.GetAllocator()), json.GetAllocator());

    if (property.GetProperty().GetIsPrimitive() && nullptr != property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration())
        {
        rapidjson::Value enumJson(rapidjson::kObjectType);
        ECEnumerationCP propEnum = property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
        enumJson.AddMember("isStrict", rapidjson::Value(propEnum->GetIsStrict()), json.GetAllocator());
        enumJson.AddMember("choices", IModelJsECPresentationSerializer::_AsJson(*propEnum, json.GetAllocator()), json.GetAllocator());
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
        propertyJson.AddMember("kindOfQuantity", IModelJsECPresentationSerializer::_AsJson(*koq, json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("property", propertyJson, json.GetAllocator());
    json.AddMember("relatedClassPath", IModelJsECPresentationSerializer::_AsJson(property.GetPathFromSelectToPropertyClass(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_FieldAsJson(ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.SetObject();
    fieldBaseJson.AddMember("category", field.GetCategory().AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("name", rapidjson::Value(field.GetUniqueName().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
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
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const
    {
    rapidjson::Value propertiesJson(rapidjson::kArrayType);
    for (ContentDescriptor::Property const& prop : ecPropertiesField.GetProperties())
        propertiesJson.PushBack(prop.AsJson(&fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("properties", propertiesJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_NestedContentFieldAsJson(ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const
    {
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
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::CompositeContentField const& compositeContentField, RapidJsonDocumentR nestedContentBaseJson) const
    {
    nestedContentBaseJson.AddMember("contentClassInfo", IModelJsECPresentationSerializer::_AsJson(compositeContentField.GetContentClass(), &nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator());
    nestedContentBaseJson.AddMember("pathToPrimaryClass", rapidjson::Document(rapidjson::kArrayType), nestedContentBaseJson.GetAllocator()); // just for backwards compatibility
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::RelatedContentField const& relatedContentField, RapidJsonDocumentR nestedContentBaseJson) const
    {
    nestedContentBaseJson.AddMember("contentClassInfo", IModelJsECPresentationSerializer::_AsJson(relatedContentField.GetContentClass(), &nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator()); // just for backwards compatibility
    nestedContentBaseJson.AddMember("pathToPrimaryClass", IModelJsECPresentationSerializer::_AsJson(RelatedClassPath(relatedContentField.GetPathFromSelectToContentClass()).Reverse("", false), nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(HierarchyUpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    switch (updateRecord.GetChangeType())
        {
        case ChangeType::Delete:
            {
            json.AddMember("type", "Delete", json.GetAllocator());
            json.AddMember("node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
            break;
            }
        case ChangeType::Insert:
            {
            json.AddMember("type", "Insert", json.GetAllocator());
            json.AddMember("node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("position", (uint64_t)updateRecord.GetPosition(), json.GetAllocator());
            break;
            }
        case ChangeType::Update:
            {
            json.AddMember("type", "Update", json.GetAllocator());
            json.AddMember("node", updateRecord.GetNode()->AsJson(&json.GetAllocator()), json.GetAllocator());
            rapidjson::Value changesJson(rapidjson::kArrayType);
            for (JsonChange const& change : updateRecord.GetChanges())
                {
                rapidjson::Value changeJson(rapidjson::kObjectType);
                changeJson.AddMember("name", rapidjson::Value(change.GetName(), json.GetAllocator()), json.GetAllocator());
                changeJson.AddMember("old", rapidjson::Value(change.GetOldValue(), json.GetAllocator()), json.GetAllocator());
                changeJson.AddMember("new", rapidjson::Value(change.GetNewValue(), json.GetAllocator()), json.GetAllocator());
                changesJson.PushBack(changeJson, json.GetAllocator());
                }
            json.AddMember("changes", changesJson, json.GetAllocator());
            break;
            }
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const
    {
#ifdef wip
    paramsBaseJson.CopyFrom(jsonParams.GetJson(), paramsBaseJson.GetAllocator());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const
    {
#ifdef wip
    paramsBaseJson.SetObject();
    paramsBaseJson.AddMember("HeightInRows", multilineParams.GetParameters().GetHeightInRows(), paramsBaseJson.GetAllocator());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const
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
void IModelJsECPresentationSerializer::_AsJson(FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const
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
void IModelJsECPresentationSerializer::_TypeDescriptionAsJson(ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.SetObject();
    typeDescriptionBaseJson.AddMember("typeName", rapidjson::Value(typeDescription.GetTypeName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Primitive", typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Array", typeDescriptionBaseJson.GetAllocator());
    typeDescriptionBaseJson.AddMember("memberType", arrayTypeDescription.GetMemberType()->AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
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
void IModelJsECPresentationSerializer::_AsJson(ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentTypeDescription.GetNestedContentField().GetFields())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("name", rapidjson::Value(nestedField->GetUniqueName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("label", rapidjson::Value(nestedField->GetLabel().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("type", nestedField->GetTypeDescription().AsJson(&typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ECClassCR ecClass, rapidjson::Document::AllocatorType* allocator) const
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("displayType", rapidjson::Value(contentDescriptor.GetPreferredDisplayType().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("selectClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (SelectClassInfo const& selectClass : contentDescriptor.GetSelectClasses())
        {
        rapidjson::Value selectClassJson(rapidjson::kObjectType);
        selectClassJson.AddMember("selectClassInfo", IModelJsECPresentationSerializer::_AsJson(selectClass.GetSelectClass().GetClass(), &json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("isSelectPolymorphic", selectClass.GetSelectClass().IsSelectPolymorphic(), json.GetAllocator());
        selectClassJson.AddMember("pathToPrimaryClass", IModelJsECPresentationSerializer::_AsJson(RelatedClassPath(selectClass.GetPathFromInputToSelectClass()).Reverse("", false), json.GetAllocator()), json.GetAllocator()); // deprecated
        selectClassJson.AddMember("pathFromInputToSelectClass", IModelJsECPresentationSerializer::_AsJson(selectClass.GetPathFromInputToSelectClass(), json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("relatedPropertyPaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (RelatedClassPathCR propertyPath : selectClass.GetRelatedPropertyPaths())
            selectClassJson["relatedPropertyPaths"].PushBack(_AsJson(propertyPath, json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("navigationPropertyClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (RelatedClassCR navPropertyClass : selectClass.GetNavigationPropertyClasses())
            selectClassJson["navigationPropertyClasses"].PushBack(_AsJson(navPropertyClass, json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("relatedInstanceClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator()); // deprecated
        selectClassJson.AddMember("relatedInstancePaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (RelatedClassPathCR relatedInstancePath : selectClass.GetRelatedInstancePaths())
            {
            selectClassJson["relatedInstanceClasses"].PushBack(_AsJson(relatedInstancePath.back(), json.GetAllocator()), json.GetAllocator());
            selectClassJson["relatedInstancePaths"].PushBack(_AsJson(relatedInstancePath, json.GetAllocator()), json.GetAllocator());
            }
        json["selectClasses"].PushBack(selectClassJson, json.GetAllocator());
        }

    bvector<ContentDescriptor::Field*> visibleFields = contentDescriptor.GetVisibleFields();
    json.AddMember("fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (ContentDescriptor::Field const* field : visibleFields)
        json["fields"].PushBack(field->AsJson(&json.GetAllocator()), json.GetAllocator());
    if (-1 != contentDescriptor.GetSortingFieldIndex())
        {
        json.AddMember("sortingFieldName", rapidjson::Value(contentDescriptor.GetSortingField()->GetUniqueName().c_str(), json.GetAllocator()), json.GetAllocator());
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
        json.AddMember("selectionInfo", IModelJsECPresentationSerializer::_AsJson(*contentDescriptor.GetSelectionInfo(), json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayLabel & flags))
        json.AddMember("labelDefinition", IModelJsECPresentationSerializer::_AsJson(contentSetItem.GetDisplayLabelDefinition(), &json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_ImageId & flags))
        json.AddMember("imageId", rapidjson::Value(contentSetItem.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_Values & flags))
        json.AddMember("values", rapidjson::Value(contentSetItem.GetValues(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayValues & flags))
        json.AddMember("displayValues", rapidjson::Value(contentSetItem.GetDisplayValues(), json.GetAllocator()), json.GetAllocator());

    if (contentSetItem.GetClass() != nullptr && 0 != (ContentSetItem::SerializationFlags::SERIALIZE_ClassInfo & flags))
        json.AddMember("classInfo", IModelJsECPresentationSerializer::_AsJson(*contentSetItem.GetClass(), &json.GetAllocator()), json.GetAllocator());

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
* @bsimethod                                    Grigas.Petraitis                 05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(DisplayValueGroupCR value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("displayValue", rapidjson::Value(value.GetDisplayValue().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Value rawValuesJson(rapidjson::kArrayType);
    for (RapidJsonValueCR rawValue : value.GetRawValues())
        rawValuesJson.PushBack(rapidjson::Value(rawValue, json.GetAllocator()), json.GetAllocator());
    json.AddMember("groupedRawValues", rawValuesJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType* allocator) const
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(Content const& content, rapidjson::Document::AllocatorType* allocator) const
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ECInstanceChangeResult const&, rapidjson::Document::AllocatorType* allocator) const
    {
    BeAssert(false && "Not expected to be used");
    return rapidjson::Document(allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_NavNodeKeyAsJson(NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
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
NavNodeKeyPtr IModelJsECPresentationSerializer::_GetNavNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
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
        return IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECInstancesNode, type))
        return IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
        return IModelJsECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
        return IModelJsECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_DisplayLabelGroupingNode, type))
        return IModelJsECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(json);
    return IModelJsECPresentationSerializer::_GetBaseNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr IModelJsECPresentationSerializer::_GetNavNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
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
        return IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECInstancesNode, type))
        return IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECClassGroupingNode, type))
        return IModelJsECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_ECPropertyGroupingNode, type))
        return IModelJsECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(connection, json);
    if (0 == strcmp(NAVNODE_TYPE_DisplayLabelGroupingNode, type))
        return IModelJsECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(json);
    return IModelJsECPresentationSerializer::_GetBaseNavNodeKeyFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr IModelJsECPresentationSerializer::_GetBaseNavNodeKeyFromJson(JsonValueCR json) const
    {
    Utf8CP type = json["type"].asCString();
    return NavNodeKey::Create(type, ParseNodeKeyHashPath(json["pathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr IModelJsECPresentationSerializer::_GetBaseNavNodeKeyFromJson(RapidJsonValueCR json) const
    {
    Utf8CP type = json["type"].GetString();
    return NavNodeKey::Create(type, ParseNodeKeyHashPath(json["pathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ECInstancesNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    rapidjson::Value instanceKeysJson(rapidjson::kArrayType);
    for (ECClassInstanceKeyCR instanceKey : ecInstanceNodeKey.GetInstanceKeys())
        instanceKeysJson.PushBack(_AsJson(instanceKey, &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("instanceKeys", instanceKeysJson, navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstancesNodeKeyPtr IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
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
ECInstancesNodeKeyPtr IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
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
void IModelJsECPresentationSerializer::_AsJson(ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("groupedInstancesCount", groupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("className", rapidjson::StringRef(groupingNodeKey.GetECClass().GetFullName()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].asUInt64();
    Utf8CP className = json["className"].asCString();
    ECClassCP ecClass = GetClassFromFullName(connection, className);
    return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUint64();
    Utf8CP className = json["className"].GetString();
    ECClassCP ecClass = GetClassFromFullName(connection, className);
    return ECClassGroupingNodeKey::Create(*ecClass, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
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
ECPropertyGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, JsonValueCR json) const
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
ECPropertyGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, RapidJsonValueCR json) const
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
void IModelJsECPresentationSerializer::_AsJson(LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("groupedInstancesCount", labelGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("label", rapidjson::Value(labelGroupingNodeKey.GetLabel().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LabelGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(JsonValueCR json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].asUInt64();
    Utf8CP label = json["label"].asCString();
    return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LabelGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(RapidJsonValueCR json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUint64();
    Utf8CP label = json["label"].GetString();
    return LabelGroupingNodeKey::Create(label, ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const
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
    json.AddMember("labelDefinition", IModelJsECPresentationSerializer::_AsJson(navNode.GetLabelDefinition(), &json.GetAllocator()), json.GetAllocator());
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
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (!labelDefinition.IsDefinitionValid())
        return json;

    json.AddMember("displayValue", rapidjson::Value(labelDefinition.GetDisplayValue().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("typeName", rapidjson::Value(labelDefinition.GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
    if (nullptr != labelDefinition.GetRawValue())
        json.AddMember("rawValue", labelDefinition.GetRawValue()->AsJson(&json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.CopyFrom(value.GetValue(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("separator", rapidjson::Value(value.GetSeparator().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Value values(rapidjson::kArrayType);
    for (LabelDefinitionCPtr labelValue : value.GetValues())
        values.PushBack(labelValue->AsJson(&json.GetAllocator()), json.GetAllocator());

    json.AddMember("values", values, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator) const
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
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const
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
KeySetPtr IModelJsECPresentationSerializer::_GetKeySetFromJson(IConnectionCR connection, JsonValueCR json) const
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
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(ECEnumerationCR enumeration, rapidjson::Document::AllocatorType& allocator) const
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
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(KindOfQuantityCR koq, rapidjson::Document::AllocatorType& allocator) const
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
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(RelatedClassCR relatedClass, rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("sourceClassInfo", IModelJsECPresentationSerializer::_AsJson(*relatedClass.GetSourceClass(), &allocator), allocator);
    json.AddMember("targetClassInfo", IModelJsECPresentationSerializer::_AsJson(relatedClass.GetTargetClass().GetClass(), &allocator), allocator);
    json.AddMember("isPolymorphicRelationship", relatedClass.GetTargetClass().IsSelectPolymorphic(), allocator);
    if (relatedClass.GetRelationship())
        json.AddMember("relationshipInfo", IModelJsECPresentationSerializer::_AsJson(*relatedClass.GetRelationship(), &allocator), allocator);
    json.AddMember("isForwardRelationship", relatedClass.IsForwardRelationship(), allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(RelatedClassPathCR path, rapidjson::Document::AllocatorType& allocator) const
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
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value info(rapidjson::kObjectType);
    info.AddMember("providerName", rapidjson::Value(selectionInfo.GetSelectionProviderName().c_str(), allocator), allocator);
    info.AddMember("level", selectionInfo.IsSubSelection() ? 1 : 0, allocator);
    return info;
    }