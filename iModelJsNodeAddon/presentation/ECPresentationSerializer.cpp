/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECPresentationSerializer.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define PUSH_JSON_WITH_CONTEXT_IF_VALID(jsonArray, jsonAllocator, objPtr, ctx) \
    if (objPtr != nullptr) \
        jsonArray.PushBack(objPtr->AsJson(ctx, &jsonAllocator), jsonAllocator); \
    else \
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, NativeLogging::LOG_ERROR, "Attempted to serialize NULL object");

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP IModelJsECPresentationSerializer::GetClassFromFullName(ECDbCR db, BeJsConst fullClassNameJson)
    {
    ECClassCP ecClass = ECJsonUtilities::GetClassFromClassNameJson(fullClassNameJson, db.GetClassLocater());
    if (nullptr == ecClass)
        DIAGNOSTICS_LOG(DiagnosticsCategory::Default, NativeLogging::LOG_DEBUG, NativeLogging::LOG_ERROR, Utf8PrintfString("Failed to find a requested ECClass: '%s'", fullClassNameJson.asCString()));
    return ecClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavNodeKeyCPtr> IModelJsECPresentationSerializer::GetNavNodeKeysFromSerializedJson(IConnectionCR connection, Utf8CP serializedJson)
    {
    bvector<NavNodeKeyCPtr> nodeKeys;
    rapidjson::Document jsonNodeKeys;
    jsonNodeKeys.Parse(serializedJson);
    if (jsonNodeKeys.IsArray())
        {
        for (rapidjson::SizeType i = 0; i < jsonNodeKeys.Size(); ++i)
            {
            NavNodeKeyCPtr key = NavNodeKey::FromJson(connection, BeJsConst(jsonNodeKeys[i], jsonNodeKeys.GetAllocator()));
            if (key.IsValid())
                nodeKeys.push_back(key);
            }
        }
    return nodeKeys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, ConnectionEvent const&, rapidjson::Document::AllocatorType* allocator) const
    {
    return rapidjson::Document(allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, ContentFieldRenderer const& renderer, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("name", rapidjson::Value(renderer.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentFieldEditor const& editor, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("name", rapidjson::Value(editor.GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Value paramsJson(rapidjson::kObjectType);
    for (ContentFieldEditor::Params const* params : editor.GetParams())
        paramsJson.AddMember(rapidjson::Value(params->GetName(), json.GetAllocator()), params->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("params", paramsJson, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateFullCategoryName(ContentDescriptor::Category const& category)
    {
    Utf8String name;
    if (category.GetParentCategory())
        name.append(CreateFullCategoryName(*category.GetParentCategory())).append("-");
    name.append(category.GetName());
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP ParseRelationshipMeaning(RelationshipMeaning relationshipMeaning)
    {
    if (relationshipMeaning == RelationshipMeaning::RelatedInstance)
        return "RelatedInstance";
    if (relationshipMeaning == RelationshipMeaning::SameInstance)
        return "SameInstance";
    return "undefined";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, ContentDescriptor::Category const& category, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("name", rapidjson::Value(CreateFullCategoryName(category).c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("label", rapidjson::Value(category.GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("description", rapidjson::Value(category.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("expand", category.ShouldExpand(), json.GetAllocator());
    json.AddMember("priority", category.GetPriority(), json.GetAllocator());
    if (category.GetParentCategory())
        json.AddMember("parent", rapidjson::Value(CreateFullCategoryName(*category.GetParentCategory()).c_str(), json.GetAllocator()), json.GetAllocator());
    if (!category.GetRendererName().empty())
        {
        rapidjson::Value renderer(rapidjson::kObjectType);
        renderer.AddMember("name", rapidjson::Value(category.GetRendererName().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("renderer", renderer, json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Property const& property, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    rapidjson::Value propertyJson(rapidjson::kObjectType);
    propertyJson.AddMember("classInfo", IModelJsECPresentationSerializer::_AsJson(ctx, property.GetProperty().GetClass(), &json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("name", rapidjson::Value(property.GetProperty().GetName().c_str(), json.GetAllocator()), json.GetAllocator());

    if (property.GetProperty().GetIsPrimitive() && nullptr != property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration())
        {
        rapidjson::Value enumJson(rapidjson::kObjectType);
        ECEnumerationCP propEnum = property.GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
        enumJson.AddMember("isStrict", rapidjson::Value(propEnum->GetIsStrict()), json.GetAllocator());
        enumJson.AddMember("choices", IModelJsECPresentationSerializer::_AsJson(ctx, *propEnum, json.GetAllocator()), json.GetAllocator());
        propertyJson.AddMember("type", "enum", json.GetAllocator());
        propertyJson.AddMember("enumerationInfo", enumJson, json.GetAllocator());
        }
    else
        {
        propertyJson.AddMember("type", rapidjson::Value(property.GetProperty().GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
        if (property.GetProperty().GetIsPrimitive())
            {
            Utf8StringCR extendedTypeName = property.GetProperty().GetAsPrimitiveProperty()->GetExtendedTypeName();
            if (extendedTypeName.length() > 0)
                propertyJson.AddMember("extendedType", rapidjson::Value(extendedTypeName.c_str(), json.GetAllocator()), json.GetAllocator());
            }
        }

    if (nullptr != property.GetProperty().GetKindOfQuantity())
        {
        KindOfQuantityCP koq = property.GetProperty().GetKindOfQuantity();
        propertyJson.AddMember("kindOfQuantity", IModelJsECPresentationSerializer::_AsJson(ctx, *koq, json.GetAllocator()), json.GetAllocator());
        }

    if (property.GetProperty().GetIsNavigation())
        {
        ECRelationshipClassCP relationshipClass = property.GetProperty().GetAsNavigationProperty()->GetRelationshipClass();
        bool isForwardRelationship = property.GetProperty().GetAsNavigationProperty()->GetDirection() == ECRelatedInstanceDirection::Forward;
        ECRelationshipConstraintCR targetConstraint = isForwardRelationship ? relationshipClass->GetTarget() : relationshipClass->GetSource();

        rapidjson::Value navigationJson(rapidjson::kObjectType);
        navigationJson.AddMember("classInfo", IModelJsECPresentationSerializer::_AsJson(ctx, *relationshipClass, &json.GetAllocator()), json.GetAllocator());
        navigationJson.AddMember("isForwardRelationship", isForwardRelationship, json.GetAllocator());
        navigationJson.AddMember("targetClassInfo", IModelJsECPresentationSerializer::_AsJson(ctx, *targetConstraint.GetAbstractConstraint(), &json.GetAllocator()), json.GetAllocator());
        navigationJson.AddMember("isTargetPolymorphic", targetConstraint.GetIsPolymorphic(), json.GetAllocator());
        propertyJson.AddMember("navigationPropertyInfo", navigationJson, json.GetAllocator());
        }

    json.AddMember("property", propertyJson, json.GetAllocator());
    json.AddMember("relatedClassPath", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator()); // wip: added only for backwards compatibility

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_FieldAsJson(ContextR ctx, ContentDescriptor::Field const& field, RapidJsonDocumentR fieldBaseJson) const
    {
    fieldBaseJson.SetObject();
    if (field.GetCategory())
        fieldBaseJson.AddMember("category", rapidjson::Value(CreateFullCategoryName(*field.GetCategory()).c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("name", rapidjson::Value(field.GetUniqueName().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("label", rapidjson::Value(field.GetLabel().c_str(), fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("type", field.GetTypeDescription().AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("isReadonly", field.IsReadOnly(), fieldBaseJson.GetAllocator());
    fieldBaseJson.AddMember("priority", field.GetPriority(), fieldBaseJson.GetAllocator());

    if (nullptr != field.GetRenderer())
        fieldBaseJson.AddMember("renderer", field.GetRenderer()->AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());

    if (nullptr != field.GetEditor())
        fieldBaseJson.AddMember("editor", field.GetEditor()->AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::ECPropertiesField const& ecPropertiesField, RapidJsonDocumentR fieldBaseJson) const
    {
    // Note: ContentDescriptor::Property contains ECProperty and some additional data like source class alias
    // and actual class. There may be multiple entries, having the same ECProperty and different other attributes.
    // However, we're only serializing stuff from ECProperty ant not the other stuff, so need this to avoid
    // serializing the same property multiple times.
    bset<ECPropertyCP> serializedProperties;

    rapidjson::Value propertiesJson(rapidjson::kArrayType);
    for (ContentDescriptor::Property const& prop : ecPropertiesField.GetProperties())
        {
        if (serializedProperties.end() != serializedProperties.find(&prop.GetProperty()))
            continue;

        propertiesJson.PushBack(prop.AsJson(ctx, &fieldBaseJson.GetAllocator()), fieldBaseJson.GetAllocator());
        serializedProperties.insert(&prop.GetProperty());
        }
    fieldBaseJson.AddMember("properties", propertiesJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_NestedContentFieldAsJson(ContextR ctx, ContentDescriptor::NestedContentField const& nestedContentField, RapidJsonDocumentR fieldBaseJson) const
    {
    if (nestedContentField.ShouldAutoExpand())
        fieldBaseJson.AddMember("autoExpand", nestedContentField.ShouldAutoExpand(), fieldBaseJson.GetAllocator());

    rapidjson::Value nestedFieldsJson(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentField.GetFields())
        PUSH_JSON_WITH_CONTEXT_IF_VALID(nestedFieldsJson, fieldBaseJson.GetAllocator(), nestedField, ctx);
    fieldBaseJson.AddMember("nestedFields", nestedFieldsJson, fieldBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::CompositeContentField const& compositeContentField, RapidJsonDocumentR nestedContentBaseJson) const
    {
    nestedContentBaseJson.AddMember("contentClassInfo", IModelJsECPresentationSerializer::_AsJson(ctx, compositeContentField.GetContentClass(), &nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator());
    nestedContentBaseJson.AddMember("pathToPrimaryClass", rapidjson::Document(rapidjson::kArrayType), nestedContentBaseJson.GetAllocator()); // just for backwards compatibility
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::RelatedContentField const& relatedContentField, RapidJsonDocumentR nestedContentBaseJson) const
    {
    RelatedClassPath pathFromRelatedContentToSelectClass(relatedContentField.GetPathFromSelectToContentClass());
    pathFromRelatedContentToSelectClass.Reverse("", false);

    nestedContentBaseJson.AddMember("contentClassInfo", IModelJsECPresentationSerializer::_AsJson(ctx, relatedContentField.GetContentClass(), &nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator()); // just for backwards compatibility
    nestedContentBaseJson.AddMember("pathToPrimaryClass", IModelJsECPresentationSerializer::_AsJson(ctx, pathFromRelatedContentToSelectClass, nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator());
    nestedContentBaseJson.AddMember("relationshipMeaning", rapidjson::StringRef(ParseRelationshipMeaning(relatedContentField.GetRelationshipMeaning())), nestedContentBaseJson.GetAllocator());

    rapidjson::Value actualSourceClassIdsJson(rapidjson::kArrayType);
    for (auto const& actualSourceClass : relatedContentField.GetActualSourceClasses())
        actualSourceClassIdsJson.PushBack(IModelJsECPresentationSerializer::_AsJson(ctx, actualSourceClass->GetId(), &nestedContentBaseJson.GetAllocator()), nestedContentBaseJson.GetAllocator());
    nestedContentBaseJson.AddMember("actualPrimaryClassIds", actualSourceClassIdsJson, nestedContentBaseJson.GetAllocator());
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct HierarchyUpdateSerializer : HierarchyChangeRecordDiffHandler
{
private:
    NavNodeCPtr m_updatedNode;
    rapidjson::Document::AllocatorType& m_allocator;
    rapidjson::Value& m_changes;

private:
    //! Make sure `member` string lifetime is static
    void Add(rapidjson::Document::StringRefType member, rapidjson::Value& value)
        {
        m_changes.AddMember(member, value, m_allocator);
        }

protected:
    void _HandleKey(NavNodeKeyCR newValue) override
        {
        Add("key", newValue.AsJson(&m_allocator).Move());
        if (!m_changes.HasMember("labelDefinition"))
            Add("labelDefinition", m_updatedNode->GetLabelDefinition().AsJson(&m_allocator).Move());
        }
    void _HandleHasChildren(bool newValue) override {Add("hasChildren", rapidjson::Value(newValue).Move());}
    void _HandleIsChecked(bool newValue) override {Add("isChecked", rapidjson::Value(newValue).Move());}
    void _HandleIsCheckboxVisible(bool newValue) override {Add("isCheckboxVisible", rapidjson::Value(newValue).Move());}
    void _HandleIsCheckboxEnabled(bool newValue) override {Add("isCheckboxEnabled", rapidjson::Value(newValue).Move());}
    void _HandleShouldAutoExpand(bool newValue) override {Add("isExpanded", rapidjson::Value(newValue).Move());}
    void _HandleDescription(Utf8StringCR newValue) override {Add("description", rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleImageId(Utf8StringCR newValue) override {Add("imageId", rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleForeColor(Utf8StringCR newValue) override {Add("foreColor", rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleBackColor(Utf8StringCR newValue) override {Add("backColor", rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleFontStyle(Utf8StringCR newValue) override {Add("fontStyle", rapidjson::Value(newValue.c_str(), m_allocator).Move());}
    void _HandleType(Utf8StringCR newValue) override {}
    void _HandleLabelDefinition(LabelDefinitionCR newValue) override
        {
        Add("labelDefinition", newValue.AsJson(&m_allocator).Move());
        if (!m_changes.HasMember("key"))
            Add("key", m_updatedNode->GetKey()->AsJson(&m_allocator).Move());
        }

public:
    HierarchyUpdateSerializer(NavNodeCPtr updatedNode, rapidjson::Value& changes, rapidjson::Document::AllocatorType& allocator)
        : m_updatedNode(updatedNode), m_changes(changes), m_allocator(allocator) {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassSerializationContext
{
private:
	IECPresentationSerializer::ContextR m_serializationContext;
    IECClassSerializer* m_previous;
public:
    ECClassSerializationContext(IECPresentationSerializer::ContextR serializationContext, IECClassSerializer& classSerializer)
        : m_serializationContext(serializationContext)
        {
        m_previous = m_serializationContext.GetClassSerializer();
        m_serializationContext.SetClassSerializer(&classSerializer);
        }
    ~ECClassSerializationContext() {m_serializationContext.SetClassSerializer(m_previous);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document CompressedClassSerializer::CreateAccumulatedClassesMap(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    for (ECClassCP ecClass : m_ecClasses)
        {
        rapidjson::Document classJson(allocator);
        classJson.SetObject();
        classJson.AddMember("name", rapidjson::StringRef(ecClass->GetFullName()), json.GetAllocator());
        classJson.AddMember("label", rapidjson::StringRef(ecClass->GetDisplayLabel().c_str()), json.GetAllocator());
        json.AddMember(rapidjson::Value(ecClass->GetId().ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator()), classJson, json.GetAllocator());
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document CompressedClassSerializer::_SerializeECClass(ECClassCR ecClass, rapidjson::Document::AllocatorType& allocator)
    {
    m_ecClasses.insert(&ecClass);

    rapidjson::Document json(&allocator);
    json.SetString(ecClass.GetId().ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator());

    return json;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DefaultClassSerializer::_SerializeECClass(ECClassCR ecClass, rapidjson::Document::AllocatorType& allocator)
    {
    rapidjson::Document json(&allocator);
    json.SetObject();
    json.AddMember("id", rapidjson::Value(ecClass.GetId().ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("name", rapidjson::StringRef(ecClass.GetFullName()), json.GetAllocator());
    json.AddMember("label", rapidjson::StringRef(ecClass.GetDisplayLabel().c_str()), json.GetAllocator());
    return json;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, HierarchyChangeRecord const& changeRecord, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    switch (changeRecord.GetChangeType())
        {
        case ChangeType::Delete:
            {
            json.AddMember("type", "Delete", json.GetAllocator());
            json.AddMember("target", changeRecord.GetNode()->GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
            if (changeRecord.GetParentNode().IsValid())
                json.AddMember("parent", changeRecord.GetParentNode()->GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("position", (uint64_t)changeRecord.GetPosition(), json.GetAllocator());
            break;
            }
        case ChangeType::Insert:
            {
            json.AddMember("type", "Insert", json.GetAllocator());
            if (changeRecord.GetParentNode().IsValid())
                json.AddMember("parent", changeRecord.GetParentNode()->GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
            json.AddMember("node", changeRecord.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
            json.AddMember("position", (uint64_t)changeRecord.GetPosition(), json.GetAllocator());
            break;
            }
        case ChangeType::Update:
            {
            NodeChanges const& changes = changeRecord.GetNodeChanges();
            json.AddMember("type", "Update", json.GetAllocator());
            json.AddMember("target", changes.GetPreviousNode()->GetKey()->AsJson(&json.GetAllocator()), json.GetAllocator());
            rapidjson::Value changesJson(rapidjson::kObjectType);
            HierarchyUpdateSerializer serializer(changes.GetUpdatedNode(), changesJson, json.GetAllocator());
            changes.FindChanges(serializer);
            json.AddMember("changes", changesJson, json.GetAllocator());
            break;
            }
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, HierarchyUpdateRecord const& updateRecord, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (updateRecord.GetParentNode().IsValid())
        json.AddMember("parent", updateRecord.GetParentNode()->GetKey()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("nodesCount", (uint64_t)updateRecord.GetNodesCount(), json.GetAllocator());

    rapidjson::Value expandedNodes(rapidjson::kArrayType);
    for (HierarchyUpdateRecord::ExpandedNode const& expandedNode : updateRecord.GetExpandedNodes())
        expandedNodes.PushBack(expandedNode.AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    if (!expandedNodes.Empty())
        json.AddMember("expandedNodes", expandedNodes.Move(), json.GetAllocator());

    if (updateRecord.GetInstanceFilter())
        json.AddMember("instanceFilter", AsJson(ctx, *updateRecord.GetInstanceFilter(), &json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, HierarchyUpdateRecord::ExpandedNode const& expandedNode, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("node", expandedNode.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("position", (uint64_t)expandedNode.GetPosition(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, FieldEditorJsonParams const& jsonParams, RapidJsonDocumentR paramsBaseJson) const
    {
#ifdef wip
    paramsBaseJson.CopyFrom(jsonParams.GetJson(), paramsBaseJson.GetAllocator());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, FieldEditorMultilineParams const& multilineParams, RapidJsonDocumentR paramsBaseJson) const
    {
#ifdef wip
    paramsBaseJson.SetObject();
    paramsBaseJson.AddMember("HeightInRows", multilineParams.GetParameters().GetHeightInRows(), paramsBaseJson.GetAllocator());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, FieldEditorRangeParams const& rangeParams, RapidJsonDocumentR paramsBaseJson) const
    {
#ifdef wip
    paramsBaseJson.SetObject();

    if (rangeParams.GetParameters().GetMinimumValue().IsValid())
        paramsBaseJson.AddMember("Minimum", rangeParams.GetParameters().GetMinimumValue().Value(), paramsBaseJson.GetAllocator());
    else
        paramsBaseJson.AddMember("Minimum", rapidjson::Value(), paramsBaseJson.GetAllocator());

    if (rangeParams.GetParameters().GetMaximumValue().IsValid())
        paramsBaseJson.AddMember("Maximum", rangeParams.GetParameters().GetMaximumValue().Value(), paramsBaseJson.GetAllocator());
    else
        paramsBaseJson.AddMember("Maximum", rapidjson::Value(), paramsBaseJson.GetAllocator());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, FieldEditorSliderParams const& sliderParams, RapidJsonDocumentR paramsBaseJson) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_TypeDescriptionAsJson(ContextR, ContentDescriptor::Field::TypeDescription const& typeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.SetObject();
    typeDescriptionBaseJson.AddMember("typeName", rapidjson::Value(typeDescription.GetTypeName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, ContentDescriptor::Field::PrimitiveTypeDescription const&, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Primitive", typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::ArrayTypeDescription const& arrayTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Array", typeDescriptionBaseJson.GetAllocator());
    typeDescriptionBaseJson.AddMember("memberType", arrayTypeDescription.GetMemberType()->AsJson(ctx, &typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::StructTypeDescription const& structTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ECPropertyCP prop : structTypeDescription.GetStruct().GetProperties())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("name", rapidjson::StringRef(prop->GetName().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("label", rapidjson::StringRef(prop->GetDisplayLabel().c_str()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("type", ContentDescriptor::Field::TypeDescription::Create(*prop)->AsJson(ctx, &typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor::Field::NestedContentTypeDescription const& nestedContentTypeDescription, RapidJsonDocumentR typeDescriptionBaseJson) const
    {
    typeDescriptionBaseJson.AddMember("valueFormat", "Struct", typeDescriptionBaseJson.GetAllocator());
    rapidjson::Value members(rapidjson::kArrayType);
    for (ContentDescriptor::Field const* nestedField : nestedContentTypeDescription.GetNestedContentField().GetFields())
        {
        rapidjson::Value member(rapidjson::kObjectType);
        member.AddMember("name", rapidjson::Value(nestedField->GetUniqueName().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("label", rapidjson::Value(nestedField->GetLabel().c_str(), typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        member.AddMember("type", nestedField->GetTypeDescription().AsJson(ctx, &typeDescriptionBaseJson.GetAllocator()), typeDescriptionBaseJson.GetAllocator());
        members.PushBack(member, typeDescriptionBaseJson.GetAllocator());
        }
    typeDescriptionBaseJson.AddMember("members", members, typeDescriptionBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ECClassCR ecClass, rapidjson::Document::AllocatorType* allocator) const
    {
	IECClassSerializer* serializer = ctx.GetClassSerializer();
	if (nullptr == serializer)
	    {
		// TODO: log an error
		return rapidjson::Document(allocator);
	    }

	return serializer->SerializeECClass(ecClass, *allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, SelectClassInfo const& selectClass, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    json.AddMember("selectClassInfo", IModelJsECPresentationSerializer::_AsJson(ctx, selectClass.GetSelectClass().GetClass(), &json.GetAllocator()), json.GetAllocator());
    json.AddMember("isSelectPolymorphic", selectClass.GetSelectClass().IsSelectPolymorphic(), json.GetAllocator());

    RelatedClassPath pathFromSelectToInputClass(selectClass.GetPathFromInputToSelectClass());
    pathFromSelectToInputClass.Reverse("", false);
    json.AddMember("pathToPrimaryClass", IModelJsECPresentationSerializer::_AsJson(ctx, pathFromSelectToInputClass, json.GetAllocator()), json.GetAllocator()); // deprecated
    if (!selectClass.GetPathFromInputToSelectClass().empty())
        json.AddMember("pathFromInputToSelectClass", IModelJsECPresentationSerializer::_AsJson(ctx, selectClass.GetPathFromInputToSelectClass(), json.GetAllocator()), json.GetAllocator());

    json.AddMember("relatedPropertyPaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (RelatedClassPathCR propertyPath : selectClass.GetRelatedPropertyPaths())
        json["relatedPropertyPaths"].PushBack(_AsJson(ctx, propertyPath, json.GetAllocator()), json.GetAllocator());

    json.AddMember("navigationPropertyClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (RelatedClassCR navPropertyClass : selectClass.GetNavigationPropertyClasses())
        json["navigationPropertyClasses"].PushBack(_AsJson(ctx, navPropertyClass, json.GetAllocator()), json.GetAllocator());

    rapidjson::Value relatedInstancePaths(rapidjson::kArrayType);
    json.AddMember("relatedInstanceClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator()); // deprecated
    for (RelatedClassPathCR relatedInstancePath : selectClass.GetRelatedInstancePaths())
        {
        json["relatedInstanceClasses"].PushBack(_AsJson(ctx, relatedInstancePath.back(), json.GetAllocator()), json.GetAllocator());
        relatedInstancePaths.PushBack(_AsJson(ctx, relatedInstancePath, json.GetAllocator()), json.GetAllocator());
        }
    if (!relatedInstancePaths.Empty())
        json.AddMember("relatedInstancePaths", relatedInstancePaths, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentDescriptor const& contentDescriptor, rapidjson::Document::AllocatorType* allocator) const
    {
	CompressedClassSerializer classSerializer;
	ECClassSerializationContext classSerializationContext(ctx, classSerializer);

    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("displayType", rapidjson::Value(contentDescriptor.GetPreferredDisplayType().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("selectClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (SelectClassInfo const& selectClass : contentDescriptor.GetSelectClasses())
        json["selectClasses"].PushBack(AsJson(ctx, selectClass, &json.GetAllocator()), json.GetAllocator());

    json.AddMember("categories", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (auto const& category : contentDescriptor.GetCategories())
        PUSH_JSON_WITH_CONTEXT_IF_VALID(json["categories"], json.GetAllocator(), category, ctx);

    bvector<ContentDescriptor::Field*> visibleFields = contentDescriptor.GetVisibleFields();
    json.AddMember("fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (ContentDescriptor::Field const* field : visibleFields)
        PUSH_JSON_WITH_CONTEXT_IF_VALID(json["fields"], json.GetAllocator(), field, ctx);
    if (-1 != contentDescriptor.GetSortingFieldIndex() && contentDescriptor.GetSortingField())
        {
        json.AddMember("sortingFieldName", rapidjson::Value(contentDescriptor.GetSortingField()->GetUniqueName().c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("sortDirection", (int)contentDescriptor.GetSortDirection(), json.GetAllocator());
        }

    json.AddMember("contentFlags", contentDescriptor.GetContentFlags(), json.GetAllocator());
    json.AddMember("connectionId", rapidjson::Value(contentDescriptor.GetConnectionId().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("fieldsFilterExpression", rapidjson::Value(contentDescriptor.GetFieldsFilterExpression().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("inputKeysHash", rapidjson::Value(contentDescriptor.GetInputNodeKeys().GetHash().c_str(), json.GetAllocator()), json.GetAllocator());

    // add the options just to keep backwards compatibility
    rapidjson::Value contentOptionsJson(rapidjson::kObjectType);
    contentOptionsJson.AddMember("RulesetId", rapidjson::Value(contentDescriptor.GetRuleset().GetRuleSetId().c_str(), json.GetAllocator()), json.GetAllocator());
    if (contentDescriptor.GetUnitSystem() != ECPresentation::UnitSystem::Undefined)
        contentOptionsJson.AddMember("UnitSystem", (int)contentDescriptor.GetUnitSystem(), json.GetAllocator());
    json.AddMember("contentOptions", contentOptionsJson, json.GetAllocator());

    if (nullptr != contentDescriptor.GetSelectionInfo())
        json.AddMember("selectionInfo", IModelJsECPresentationSerializer::_AsJson(ctx, *contentDescriptor.GetSelectionInfo(), json.GetAllocator()), json.GetAllocator());

    // add this last to make sure all necessary classes are captured by classSerializer
    json.AddMember("classesMap", classSerializer.CreateAccumulatedClassesMap(&json.GetAllocator()), json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ContentSetItem const& contentSetItem, int flags, rapidjson::Document::AllocatorType* allocator) const
    {
	DefaultClassSerializer classSerializer;
	ECClassSerializationContext classSerializationContext(ctx, classSerializer);

    rapidjson::Document json(allocator);
    json.SetObject();

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayLabel & flags))
        json.AddMember("labelDefinition", IModelJsECPresentationSerializer::_AsJson(ctx, contentSetItem.GetDisplayLabelDefinition(), &json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_ImageId & flags))
        json.AddMember("imageId", rapidjson::Value(contentSetItem.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_Values & flags))
        {
        json.AddMember("values", rapidjson::Value(contentSetItem.GetValues(), json.GetAllocator()), json.GetAllocator());
        for (auto const& nestedContentEntry : contentSetItem.GetNestedContent())
            {
            rapidjson::Value nestedValuesJson(rapidjson::kArrayType);
            for (auto const& nestedItem : nestedContentEntry.second)
                {
                static const int valueSerializationFlags = ContentSetItem::SERIALIZE_PrimaryKeys | ContentSetItem::SERIALIZE_Values | ContentSetItem::SERIALIZE_DisplayValues | ContentSetItem::SERIALIZE_MergedFieldNames;
                nestedValuesJson.PushBack(nestedItem->AsJson(ctx, valueSerializationFlags, &json.GetAllocator()), json.GetAllocator());
                }
            json["values"].AddMember(rapidjson::Value(nestedContentEntry.first.c_str(), json.GetAllocator()), nestedValuesJson, json.GetAllocator());
            }
        }

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_DisplayValues & flags))
        {
        json.AddMember("displayValues", rapidjson::Value(contentSetItem.GetDisplayValues(), json.GetAllocator()), json.GetAllocator());
        for (auto const& nestedContentEntry : contentSetItem.GetNestedContent())
            {
            rapidjson::Value nestedValuesJson(rapidjson::kArrayType);
            for (auto const& nestedItem : nestedContentEntry.second)
                nestedValuesJson.PushBack(nestedItem->AsJson(ctx, (int)ContentSetItem::SERIALIZE_DisplayValues, &json.GetAllocator()), json.GetAllocator());
            json["displayValues"].AddMember(rapidjson::Value(nestedContentEntry.first.c_str(), json.GetAllocator()), nestedValuesJson, json.GetAllocator());
            }
        }

    if (contentSetItem.GetClass() != nullptr && 0 != (ContentSetItem::SerializationFlags::SERIALIZE_ClassInfo & flags))
        json.AddMember("classInfo", IModelJsECPresentationSerializer::_AsJson(ctx, *contentSetItem.GetClass(), &json.GetAllocator()), json.GetAllocator());

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_InputKeys & flags))
        {
        rapidjson::Value inputKeys(rapidjson::kArrayType);
        for (ECClassInstanceKeyCR key : contentSetItem.GetInputKeys())
            inputKeys.PushBack(_AsJson(ctx, key, &json.GetAllocator()), json.GetAllocator());
        json.AddMember("inputKeys", inputKeys, json.GetAllocator());
        }

    if (0 != (ContentSetItem::SerializationFlags::SERIALIZE_PrimaryKeys & flags))
        {
        rapidjson::Value primaryKeys(rapidjson::kArrayType);
        for (ECClassInstanceKeyCR key : contentSetItem.GetKeys())
            primaryKeys.PushBack(_AsJson(ctx, key, &json.GetAllocator()), json.GetAllocator());
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, DisplayValueGroupCR value, rapidjson::Document::AllocatorType* allocator) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, BeInt64Id const& id, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetString(id.ToString(BeInt64Id::UseHex::Yes).c_str(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ECClassInstanceKeyCR key, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("className", rapidjson::StringRef(key.GetClass()->GetFullName()), json.GetAllocator());
    json.AddMember("id", AsJson(ctx, key.GetId(), &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, Content const& content, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("descriptor", content.GetDescriptor().AsJson(ctx, &json.GetAllocator()), json.GetAllocator());

    rapidjson::Value set(rapidjson::kArrayType);
    DataContainer<ContentSetItemCPtr> container = content.GetContentSet();
    for (ContentSetItemCPtr item : container)
        {
        if (item.IsValid())
            set.PushBack(item->AsJson(ctx, ContentSetItem::SERIALIZE_All, &json.GetAllocator()), json.GetAllocator());
        else
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, NativeLogging::LOG_ERROR, "Attempted to serialize NULL ContentSetItem object");
        }
    json.AddMember("contentSet", set, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, ECInstanceChangeResult const&, rapidjson::Document::AllocatorType* allocator) const
    {
    return rapidjson::Document(allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_NavNodeKeyAsJson(ContextR, NavNodeKey const& navNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.SetObject();
    navNodeKeyBaseJson.AddMember("version", 2, navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("type", rapidjson::Value(navNodeKey.GetType().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

    rapidjson::Value pathJson(rapidjson::kArrayType);
    for (Utf8StringCR partialHash : navNodeKey.GetHashPath())
        pathJson.PushBack(rapidjson::Value(partialHash.c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());

    navNodeKeyBaseJson.AddMember("pathFromRoot", pathJson, navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<Utf8String> ParseNodeKeyHashPath(BeJsConst pathJson)
    {
    bvector<Utf8String> path;
    pathJson.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst pathElement)
        {
        path.push_back(pathElement.asString());
        return false;
        });
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr IModelJsECPresentationSerializer::_GetNavNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    if (!json.isObject() || json.isNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Attempting to parse node key from JSON that is not an object");

    Utf8CP type = json["type"].asCString();
    if (nullptr == type)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Attempting to parse node key from JSON without 'type'");

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodeKeyPtr IModelJsECPresentationSerializer::_GetBaseNavNodeKeyFromJson(BeJsConst json) const
    {
    Utf8CP type = json["type"].asCString();
    return NavNodeKey::Create(type, "", ParseNodeKeyHashPath(json["pathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR ctx, ECInstancesNodeKey const& ecInstanceNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    rapidjson::Value instanceKeysJson(rapidjson::kArrayType);
    for (ECClassInstanceKeyCR instanceKey : ecInstanceNodeKey.GetInstanceKeys())
        instanceKeysJson.PushBack(_AsJson(ctx, instanceKey, &navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("instanceKeys", instanceKeysJson, navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstancesNodeKeyPtr IModelJsECPresentationSerializer::_GetECInstanceNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    bvector<ECClassInstanceKey> instanceKeys;
    if (json.isMember("instanceKeys") && json["instanceKeys"].isArray())
        {
        json["instanceKeys"].ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst instanceKeyJson)
            {
            ECClassCP ecClass = GetClassFromFullName(connection, instanceKeyJson["className"]);
            ECInstanceId instanceId(instanceKeyJson["id"].GetUInt64());
            instanceKeys.push_back(ECClassInstanceKey(ecClass, instanceId));
            return false;
            });
        }
    return ECInstancesNodeKey::Create(instanceKeys, "", ParseNodeKeyHashPath(json["pathFromRoot"]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, ECClassGroupingNodeKey const& groupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("groupedInstancesCount", groupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("className", rapidjson::StringRef(groupingNodeKey.GetECClass().GetFullName()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetECClassGroupingNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUInt64();
    ECClassCP ecClass = GetClassFromFullName(connection, json["className"]);
    return ECClassGroupingNodeKey::Create(*ecClass, false, "", ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, ECPropertyGroupingNodeKey const& propertyGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("groupedInstancesCount", propertyGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("className", rapidjson::StringRef(propertyGroupingNodeKey.GetECClass().GetFullName()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("propertyName", rapidjson::Value(propertyGroupingNodeKey.GetPropertyName().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("groupingValues", rapidjson::Value(propertyGroupingNodeKey.GetGroupingValuesArray(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    // deprecated:
    rapidjson::Value firstGroupingValue;
    if (!propertyGroupingNodeKey.GetGroupingValuesArray().Empty())
        firstGroupingValue = rapidjson::Value(propertyGroupingNodeKey.GetGroupingValuesArray()[0], navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("groupingValue", firstGroupingValue, navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetECPropertyGroupingNodeKeyFromJson(IConnectionCR connection, BeJsConst json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUInt64();
    ECClassCP ecClass = GetClassFromFullName(connection, json["className"]);
    Utf8CP propertyName = json["propertyName"].asCString();
    rapidjson::Document groupingValues;
    if (json.isMember("groupingValues"))
        groupingValues.Parse(json["groupingValues"].ToJsonString().c_str());
    else
        groupingValues.SetArray();
    return ECPropertyGroupingNodeKey::Create(*ecClass, propertyName, groupingValues, "", ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsECPresentationSerializer::_AsJson(ContextR, LabelGroupingNodeKey const& labelGroupingNodeKey, RapidJsonDocumentR navNodeKeyBaseJson) const
    {
    navNodeKeyBaseJson.AddMember("groupedInstancesCount", labelGroupingNodeKey.GetGroupedInstancesCount(), navNodeKeyBaseJson.GetAllocator());
    navNodeKeyBaseJson.AddMember("label", rapidjson::Value(labelGroupingNodeKey.GetLabel().c_str(), navNodeKeyBaseJson.GetAllocator()), navNodeKeyBaseJson.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelGroupingNodeKeyPtr IModelJsECPresentationSerializer::_GetLabelGroupingNodeKeyFromJson(BeJsConst json) const
    {
    uint64_t groupedInstancesCount = json["groupedInstancesCount"].GetUInt64();
    Utf8CP label = json["label"].asCString();
    return LabelGroupingNodeKey::Create(label, "", ParseNodeKeyHashPath(json["pathFromRoot"]), groupedInstancesCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, NavNode const& navNode, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("key", navNode.GetKey()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("labelDefinition", IModelJsECPresentationSerializer::_AsJson(ctx, navNode.GetLabelDefinition(), &json.GetAllocator()), json.GetAllocator());
    if (navNode.HasChildren())
        json.AddMember("hasChildren", navNode.HasChildren(), json.GetAllocator());
    if (!navNode.GetDescription().empty())
        json.AddMember("description", rapidjson::Value(navNode.GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    if (!navNode.GetImageId().empty())
        json.AddMember("imageId", rapidjson::Value(navNode.GetImageId().c_str(), json.GetAllocator()), json.GetAllocator());
    if (!navNode.GetForeColor().empty())
        json.AddMember("foreColor", rapidjson::Value(navNode.GetForeColor().c_str(), json.GetAllocator()), json.GetAllocator());
    if (!navNode.GetBackColor().empty())
        json.AddMember("backColor", rapidjson::Value(navNode.GetBackColor().c_str(), json.GetAllocator()), json.GetAllocator());
    if (!navNode.GetFontStyle().empty() && !navNode.GetFontStyle().EqualsI("regular"))
        json.AddMember("fontStyle", rapidjson::Value(navNode.GetFontStyle().c_str(), json.GetAllocator()), json.GetAllocator());
    if (navNode.IsCheckboxVisible())
        json.AddMember("isCheckboxVisible", navNode.IsCheckboxVisible(), json.GetAllocator());
    if (navNode.IsChecked())
        json.AddMember("isChecked", navNode.IsChecked(), json.GetAllocator());
    if (navNode.IsCheckboxEnabled())
        json.AddMember("isCheckboxEnabled", navNode.IsCheckboxEnabled(), json.GetAllocator());
    if (navNode.ShouldAutoExpand())
        json.AddMember("isExpanded", navNode.ShouldAutoExpand(), json.GetAllocator());
    if (navNode.GetUsersExtendedData().GetJson().MemberCount() > 0)
        json.AddMember("extendedData", rapidjson::Value(navNode.GetUsersExtendedData().GetJson(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, LabelDefinition const& labelDefinition, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (!labelDefinition.IsDefinitionValid())
        return json;

    json.AddMember("displayValue", rapidjson::Value(labelDefinition.GetDisplayValue().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("typeName", rapidjson::Value(labelDefinition.GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
    if (nullptr != labelDefinition.GetRawValue())
        json.AddMember("rawValue", labelDefinition.GetRawValue()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR, LabelDefinition::SimpleRawValue const& value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.CopyFrom(value.GetValue(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, LabelDefinition::CompositeRawValue const& value, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("separator", rapidjson::Value(value.GetSeparator().c_str(), json.GetAllocator()), json.GetAllocator());
    rapidjson::Value values(rapidjson::kArrayType);
    for (LabelDefinitionCPtr labelValue : value.GetValues())
        PUSH_JSON_WITH_CONTEXT_IF_VALID(values, json.GetAllocator(), labelValue, ctx);

    json.AddMember("values", values, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, NodesPathElement const& navNodesPathElement, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    if (navNodesPathElement.GetNode().IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Attempted to serialize NodesPathElement with NULL node");

    json.SetObject();
    json.AddMember("node", navNodesPathElement.GetNode()->AsJson(ctx, &json.GetAllocator()), json.GetAllocator());
    json.AddMember("index", (uint64_t)navNodesPathElement.GetIndex(), json.GetAllocator());
    json.AddMember("isMarked", navNodesPathElement.IsMarked(), json.GetAllocator());

    rapidjson::Value childrenJson;
    childrenJson.SetArray();
    for (NodesPathElement const& child : navNodesPathElement.GetChildren())
        childrenJson.PushBack(child.AsJson(ctx, &json.GetAllocator()), json.GetAllocator());

    json.AddMember("children", childrenJson, json.GetAllocator());

    rapidjson::Value filteringData;
    filteringData.SetObject();

    filteringData.AddMember("occurances", navNodesPathElement.GetFilteringData().GetOccurances(), json.GetAllocator());
    filteringData.AddMember("childrenOccurances", navNodesPathElement.GetFilteringData().GetChildrenOccurances(), json.GetAllocator());

    json.AddMember("filteringData", filteringData, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, KeySet const& keySet, rapidjson::Document::AllocatorType* allocator) const
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
        PUSH_JSON_WITH_CONTEXT_IF_VALID(nodeKeys, json.GetAllocator(), key, ctx);

    json.AddMember("instanceKeys", instances, json.GetAllocator());
    json.AddMember("nodeKeys", nodeKeys, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr IModelJsECPresentationSerializer::_GetKeySetFromJson(IConnectionCR connection, BeJsConst json) const
    {
    return IModelJsECPresentationSerializer::GetKeySetFromJson(connection, json);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
KeySetPtr IModelJsECPresentationSerializer::GetKeySetFromJson(IConnectionCR connection, BeJsConst json)
    {
    InstanceKeyMap instanceKeys;
    if (json.isMember("instanceKeys") && json["instanceKeys"].isArray())
        {
        json["instanceKeys"].ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst instanceKeysEntry)
            {
            if (!instanceKeysEntry.isArray() || 2 != instanceKeysEntry.size())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Invalid instance key entry in KeySet. Expected an array of 2 items: [class_name, instance_ids_list]");

            ECClassCP ecClass = GetClassFromFullName(connection, instanceKeysEntry[0]);
            if (nullptr == ecClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, NativeLogging::LOG_DEBUG, NativeLogging::LOG_ERROR, Utf8PrintfString("Found invalid ECClass in given KeySet: '%s'", instanceKeysEntry[0].asCString()));
                return false;
                }

            BeJsConst instanceIdsJson = instanceKeysEntry[1];
            if (!instanceIdsJson.isArray())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Invalid instance keys list in KeySet. Expected an array of IDs.");

            bset<ECInstanceId> ids;
            instanceIdsJson.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst instanceIdJson)
                {
                ids.insert(ECInstanceId(instanceIdJson.GetUInt64()));
                return false;
                });
            instanceKeys[ecClass] = ids;
            return false;
            });
        }

    NavNodeKeySet nodeKeys;
    if (json.isMember("nodeKeys") && json["nodeKeys"].isArray())
        {
        json["nodeKeys"].ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst nodeKeyJson)
            {
            nodeKeys.insert(NavNodeKey::FromJson(connection, nodeKeyJson));
            return false;
            });
        }

    return KeySet::Create(instanceKeys, nodeKeys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(ContextR, ECEnumerationCR enumeration, rapidjson::Document::AllocatorType& allocator) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(ContextR ctx, KindOfQuantityCR koq, rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("name", rapidjson::StringRef(koq.GetFullName().c_str()), allocator);
    json.AddMember("label", rapidjson::StringRef(koq.GetDisplayLabel().c_str()), allocator);
    json.AddMember("persistenceUnit", rapidjson::Value(koq.GetPersistenceUnit()->GetName().c_str(), allocator), allocator);

    IECPropertyFormatter const* formatter = ctx.GetPropertyFormatter();
    if (formatter == nullptr)
        return json;

    Formatting::Format const* format = formatter->GetActiveFormat(koq, ctx.GetUnitSystem());
    rapidjson::Document formatJson(rapidjson::kObjectType, &allocator);
    if (nullptr != format && format->ToJson(formatJson, false))
        json.AddMember("activeFormat", formatJson, allocator);

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(ContextR ctx, RelatedClassCR relatedClass, rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("sourceClassInfo", IModelJsECPresentationSerializer::_AsJson(ctx, *relatedClass.GetSourceClass(), &allocator), allocator);
    json.AddMember("targetClassInfo", IModelJsECPresentationSerializer::_AsJson(ctx, relatedClass.GetTargetClass().GetClass(), &allocator), allocator);
    json.AddMember("isPolymorphicTargetClass", relatedClass.GetTargetClass().IsSelectPolymorphic(), allocator);
    if (relatedClass.GetRelationship().IsValid())
        {
        json.AddMember("relationshipInfo", IModelJsECPresentationSerializer::_AsJson(ctx, relatedClass.GetRelationship().GetClass(), &allocator), allocator);
        json.AddMember("isPolymorphicRelationship", relatedClass.GetRelationship().IsSelectPolymorphic(), allocator);
        }
    json.AddMember("isForwardRelationship", relatedClass.IsForwardRelationship(), allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(ContextR ctx, RelatedClassPathCR path, rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (RelatedClass const& relatedClass : path)
        json.PushBack(_AsJson(ctx, relatedClass, allocator), allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Value IModelJsECPresentationSerializer::_AsJson(ContextR, SelectionInfo const& selectionInfo, rapidjson::Document::AllocatorType& allocator) const
    {
    rapidjson::Value info(rapidjson::kObjectType);
    info.AddMember("providerName", rapidjson::Value(selectionInfo.GetSelectionProviderName().c_str(), allocator), allocator);
    info.AddMember("level", selectionInfo.IsSubSelection() ? 1 : 0, allocator);
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables IModelJsECPresentationSerializer::GetRulesetVariablesFromJson(BeJsConst json)
    {
    RulesetVariables variables;
    if (!json.isArray())
        return variables;

    json.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst item)
        {
        if (!item.isMember("id") || !item.isMember("type") || !item.isMember("value"))
            return false;

        Utf8CP variableId = item["id"].asCString();
        Utf8CP variableType = item["type"].asCString();
        BeJsConst variableValue = item["value"];
        if (0 == strcmp("bool", variableType))
            variables.SetBoolValue(variableId, variableValue.asBool(false));
        else if (0 == strcmp("string", variableType))
            variables.SetStringValue(variableId, variableValue.asCString(""));
        else if (0 == strcmp("id64", variableType) || 0 == strcmp("int", variableType))
            variables.SetIntValue(variableId, variableValue.GetUInt64());
        else if ((0 == strcmp("id64[]", variableType) || 0 == strcmp("int[]", variableType)) && variableValue.isArray())
            {
            bvector<int64_t> values;
            variableValue.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst varJson)
                {
                values.push_back(varJson.GetInt64());
                return false;
                });
            variables.SetIntValues(variableId, values);
            }
        return false;
        });

    return variables;
    }

#define PRESENTATION_JSON_ATTRIBUTE_RelatedClass_SourceClassName            "sourceClassName"
#define PRESENTATION_JSON_ATTRIBUTE_RelatedClass_TargetClassName            "targetClassName"
#define PRESENTATION_JSON_ATTRIBUTE_RelatedClass_RelationshipName           "relationshipName"
#define PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsPolymorphicRelationship  "isPolymorphicRelationship"
#define PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsForwardRelationship      "isForwardRelationship"
#define PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsPolymorphicTargetClass   "isPolymorphicTargetClass"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static RelatedClass GetRelatedClassFromJson(ECDbCR db, BeJsConst json, bool defaultIsPolymorphicValue = false)
    {
    RelatedClass invalid;
    if (!json.isObject())
        return invalid;

    ECClassCP sourceClass = IModelJsECPresentationSerializer::GetClassFromFullName(db, json[PRESENTATION_JSON_ATTRIBUTE_RelatedClass_SourceClassName]);
    ECClassCP targetClass = IModelJsECPresentationSerializer::GetClassFromFullName(db, json[PRESENTATION_JSON_ATTRIBUTE_RelatedClass_TargetClassName]);
    ECClassCP relationship = IModelJsECPresentationSerializer::GetClassFromFullName(db, json[PRESENTATION_JSON_ATTRIBUTE_RelatedClass_RelationshipName]);
    if (!sourceClass || !targetClass || !relationship || !relationship->IsRelationshipClass())
        return invalid;

    bool isForwardRelationship = json.hasMember(PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsForwardRelationship) ? json[PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsForwardRelationship].asBool() : false;
    bool isPolymorphicRelationship = json.hasMember(PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsPolymorphicRelationship) ? json[PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsPolymorphicRelationship].asBool() : defaultIsPolymorphicValue;
    bool isPolymorphicTargetClass = json.hasMember(PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsPolymorphicTargetClass) ? json[PRESENTATION_JSON_ATTRIBUTE_RelatedClass_IsPolymorphicTargetClass].asBool() : defaultIsPolymorphicValue;

    return RelatedClass(*sourceClass, SelectClass<ECRelationshipClass>(*relationship->GetRelationshipClassCP(), "", isPolymorphicRelationship),
        isForwardRelationship, SelectClass<ECClass>(*targetClass, "", isPolymorphicTargetClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClassPath IModelJsECPresentationSerializer::GetRelatedClassPathFromJson(ECDbCR db, BeJsConst json, bool defaultIsPolymorphicValue)
    {
    RelatedClassPath path;
    if (!json.isArray())
        return path;

    json.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst stepJson)
        {
        RelatedClass rc = GetRelatedClassFromJson(db, stepJson, defaultIsPolymorphicValue);
        if (rc.IsValid())
            path.push_back(rc);
        return false;
        });

    return path;
    }

#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_FilterExpression                   "expression"
#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_SelectClassName                    "selectClassName"
#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances                   "relatedInstances"
#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_PathToProperty    "pathFromSelectToPropertyClass"
#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_IsRequired        "isRequired"
#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_Alias             "alias"
#define PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_RelationshipAlias "relationshipAlias"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> ParseInstanceFilterRelatedInstances(IConnectionCR connection, BeJsConst json)
    {
    if (!json.isArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, "Expected instance filter JSON member `" PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances "` to be an array");

    bvector<RelatedClassPath> instances;
    json.ForEachArrayMember([&](BeJsConst::ArrayIndex, BeJsConst instanceDef)
        {
        if (!instanceDef.isObject() || !instanceDef.hasMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_PathToProperty))
            return false;

        RelatedClassPath pathToProperty = IModelJsECPresentationSerializer::GetRelatedClassPathFromJson(connection.GetECDb(), instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_PathToProperty], true);
        if (pathToProperty.empty())
            return false;

        bool hasAlias = false;
        if (instanceDef.hasMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_Alias) && instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_Alias].isString())
            {
            Utf8String alias = instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_Alias].ToUtf8CP();
            pathToProperty.back().GetTargetClass().SetAlias(alias);
            hasAlias = true;
            }

        if (instanceDef.hasMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_RelationshipAlias) && instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_RelationshipAlias].isString())
            {
            Utf8String alias = instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_RelationshipAlias].ToUtf8CP();
            pathToProperty.back().GetRelationship().SetAlias(alias);
            }

        if (!hasAlias)
            return false;

        if (instanceDef.hasMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_IsRequired))
            {
            bool isRequired = instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_IsRequired].isBool() ? instanceDef[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_IsRequired].asBool() : false;
            pathToProperty.back().SetIsTargetOptional(!isRequired);
            }

        instances.push_back(pathToProperty);
        return false;
        });

    return instances;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<InstanceFilterDefinition> IModelJsECPresentationSerializer::_GetInstanceFilterFromJson(IConnectionCR connection, BeJsConst json) const
    {
    if (!json.isObject())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, "Expected instance filter JSON to be an object");
    if (!json.hasMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_FilterExpression) || !json[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_FilterExpression].isString())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Serialization, "Expected instance filter JSON member `" PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_FilterExpression "` to be a string");
    Utf8String expression = json[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_FilterExpression].ToUtf8CP();

    ECClassCP selectClass = GetClassFromFullName(connection, json[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_SelectClassName]);

    bvector<RelatedClassPath> relatedInstances;
    if (json.hasMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances))
        relatedInstances = ParseInstanceFilterRelatedInstances(connection, json[PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances]);

    if (selectClass)
        return std::make_unique<InstanceFilterDefinition>(expression, *selectClass, relatedInstances);

    return std::make_unique<InstanceFilterDefinition>(expression);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document IModelJsECPresentationSerializer::_AsJson(ContextR ctx, InstanceFilterDefinitionCR filter, rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    if (filter.GetSelectClass())
        json.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_SelectClassName, rapidjson::Value(filter.GetSelectClass()->GetFullName(), json.GetAllocator()), json.GetAllocator());
    json.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_FilterExpression, rapidjson::Value(filter.GetExpression().c_str(), json.GetAllocator()), json.GetAllocator());
    if (!filter.GetRelatedInstances().empty())
        {
        rapidjson::Value relatedInstancesJson(rapidjson::kArrayType);
        for (auto const& relatedInstanceDef : filter.GetRelatedInstances())
            {
            rapidjson::Value relatedInstanceJson(rapidjson::kObjectType);
            relatedInstanceJson.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_PathToProperty, _AsJson(ctx, relatedInstanceDef, json.GetAllocator()), json.GetAllocator());
            if (!relatedInstanceDef.back().GetTargetClass().GetAlias().empty())
                relatedInstanceJson.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_Alias, rapidjson::Value(relatedInstanceDef.back().GetTargetClass().GetAlias().c_str(), json.GetAllocator()), json.GetAllocator());
            if (!relatedInstanceDef.back().GetRelationship().GetAlias().empty())
                relatedInstanceJson.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_RelationshipAlias, rapidjson::Value(relatedInstanceDef.back().GetRelationship().GetAlias().c_str(), json.GetAllocator()), json.GetAllocator());
            if (!relatedInstanceDef.back().IsTargetOptional())
                relatedInstanceJson.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances_IsRequired, true, json.GetAllocator());
            relatedInstancesJson.PushBack(relatedInstanceJson, json.GetAllocator());
            }
        json.AddMember(PRESENTATION_JSON_ATTRIBUTE_InstanceFilterDefinition_RelatedInstances, relatedInstancesJson, json.GetAllocator());
        }
    return json;
    }
