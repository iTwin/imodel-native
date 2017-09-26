/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/Content.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Content.h>
#include <ECPresentation/SelectionManager.h>
#include "ValueHelpers.h"
#include "../Localization/Xliffs/ECPresentation.xliff.h"

const Utf8CP ContentDisplayType::Undefined = "Undefined";
const Utf8CP ContentDisplayType::Grid = "Grid";
const Utf8CP ContentDisplayType::PropertyPane = "PropertyPane";
const Utf8CP ContentDisplayType::Graphics = "Graphics";

static ContentDescriptor::Field::TypeDescriptionPtr CreateTypeDescription(ECPropertyCR);

//===================================================================================
// @bsiclass                                    Grigas.Petraitis            09/2017
//===================================================================================
rapidjson::Document ContentDescriptor::Field::TypeDescription::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("TypeName", rapidjson::Value(m_typeName.c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }
//===================================================================================
// @bsiclass                                    Grigas.Petraitis            09/2017
//===================================================================================
struct PrimitiveTypeDescription : ContentDescriptor::Field::TypeDescription
{
protected:
    rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const
        {
        rapidjson::Document json = TypeDescription::_AsJson(allocator);
        json.AddMember("ValueFormat", "Primitive", json.GetAllocator());
        return json;
        }
public:
    PrimitiveTypeDescription(Utf8String type) : TypeDescription(type) {}
};
//===================================================================================
// @bsiclass                                    Grigas.Petraitis            09/2017
//===================================================================================
struct ArrayTypeDescription : ContentDescriptor::Field::TypeDescription
{
private:
    ContentDescriptor::Field::TypeDescriptionPtr m_memberType;
private:
    static Utf8String CreateTypeName(TypeDescription const& memberType)
        {
        Utf8String typeName = memberType.GetTypeName();
        typeName.append("[]");
        return typeName;
        }
protected:
    rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const
        {
        rapidjson::Document json = TypeDescription::_AsJson(allocator);
        json.AddMember("ValueFormat", "Array", json.GetAllocator());
        json.AddMember("MemberType", m_memberType->AsJson(&json.GetAllocator()), json.GetAllocator());
        return json;
        }
public:
    ArrayTypeDescription(TypeDescription& memberType) : TypeDescription(CreateTypeName(memberType)), m_memberType(&memberType) {}
};
//===================================================================================
// @bsiclass                                    Grigas.Petraitis            09/2017
//===================================================================================
struct StructTypeDescription : ContentDescriptor::Field::TypeDescription
{
private:
    ECStructClassCR m_struct;
protected:
    rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const
        {
        rapidjson::Document json = TypeDescription::_AsJson(allocator);
        json.AddMember("ValueFormat", "Struct", json.GetAllocator());
        rapidjson::Value members(rapidjson::kArrayType);
        for (ECPropertyCP prop : m_struct.GetProperties())
            {
            rapidjson::Value member(rapidjson::kObjectType);
            member.AddMember("Name", rapidjson::StringRef(prop->GetName().c_str()), json.GetAllocator());
            member.AddMember("Label", rapidjson::StringRef(prop->GetDisplayLabel().c_str()), json.GetAllocator());
            member.AddMember("Type", CreateTypeDescription(*prop)->AsJson(&json.GetAllocator()), json.GetAllocator());
            members.PushBack(member, json.GetAllocator());
            }
        json.AddMember("Members", members, json.GetAllocator());
        return json;
        }
public:
    StructTypeDescription(ECStructClassCR structClass) : TypeDescription(structClass.GetName()), m_struct(structClass) {}
};
//===================================================================================
// @bsiclass                                    Grigas.Petraitis            09/2017
//===================================================================================
struct NestedContentTypeDescription : ContentDescriptor::Field::TypeDescription
{
private:
    ContentDescriptor::NestedContentField const& m_field;
protected:
    rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const
        {
        rapidjson::Document json = TypeDescription::_AsJson(allocator);
        json.AddMember("ValueFormat", "Struct", json.GetAllocator());
        rapidjson::Value members(rapidjson::kArrayType);
        for (ContentDescriptor::Field const* nestedField : m_field.GetFields())
            {
            rapidjson::Value member(rapidjson::kObjectType);
            member.AddMember("Name", rapidjson::StringRef(nestedField->GetName().c_str()), json.GetAllocator());
            member.AddMember("Label", rapidjson::StringRef(nestedField->GetLabel().c_str()), json.GetAllocator());
            member.AddMember("Type", nestedField->GetTypeDescription().AsJson(&json.GetAllocator()), json.GetAllocator());
            members.PushBack(member, json.GetAllocator());
            }
        json.AddMember("Members", members, json.GetAllocator());
        return json;
        }
public:
    NestedContentTypeDescription(ContentDescriptor::NestedContentField const& field) : TypeDescription(field.GetContentClass().GetDisplayLabel()), m_field(field) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor(ContentDescriptorCR other) 
    : m_preferredDisplayType(other.m_preferredDisplayType), m_classes(other.m_classes), m_filterExpression(other.m_filterExpression),
    m_contentFlags(other.m_contentFlags), m_sortingFieldIndex(other.m_sortingFieldIndex), m_sortDirection(other.m_sortDirection)
    {
    bmap<Field const*, Field const*> fieldsRemapInfo;
    for (Field const* field : other.m_fields)
        {
        Field* copy = field->Clone();
        m_fields.push_back(copy);
        fieldsRemapInfo[field] = copy;
        }
    for (Field* field : m_fields)
        field->NotifyFieldsCloned(fieldsRemapInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::~ContentDescriptor()
    {
    for (Field* field : m_fields)
        delete field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::Equals(ContentDescriptor const& other) const
    {
    if (!m_preferredDisplayType.Equals(other.m_preferredDisplayType)
        || m_contentFlags != other.m_contentFlags
        || m_sortingFieldIndex != other.m_sortingFieldIndex
        || m_sortDirection != other.m_sortDirection
        || m_filterExpression != other.m_filterExpression
        || m_classes.size() != other.m_classes.size()
        || m_fields.size() != other.m_fields.size())
        {
        return false;
        }

    for (size_t i = 0; i < m_classes.size(); ++i)
        {
        if (m_classes[i] != other.m_classes[i])
            return false;
        }

    for (size_t i = 0; i < m_fields.size(); ++i)
        {
        if (*m_fields[i] != *other.m_fields[i])
            return false;
        }

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentDescriptor::GetFieldIndex(Utf8CP name) const
    {
    for (size_t i = 0; i < m_fields.size(); ++i)
        {
        if (m_fields[i]->GetName().Equals(name))
            return (int)i;
        }
    return -1;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentDescriptor::Field*> ContentDescriptor::GetVisibleFields() const
    {
    bvector<Field*> fields;
    std::copy_if(m_fields.begin(), m_fields.end(), std::back_inserter(fields), [](Field const* f){return !f->IsSystemField();});
    return fields;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::RemoveField(Field const& field)
    {
    auto iter = std::find_if(m_fields.begin(), m_fields.end(), [&field](Field const* f){return *f == field;});
    if (m_fields.end() == iter)
        return;

    m_fields.erase(iter);

    bvector<Field*> fieldsToRemove;
    for (Field* siblingField : m_fields)
        {
        if (siblingField->NotifyFieldRemoved(field))
            fieldsToRemove.push_back(siblingField);
        }
    for (Field* siblingField : fieldsToRemove)
        RemoveField(*siblingField);

    delete &field;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::RemoveField(Utf8CP name)
    {
    int index = GetFieldIndex(name);
    if (index >= 0)
        RemoveField((size_t)index);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::MergeWith(ContentDescriptorCR other)
    {
    BeAssert(m_preferredDisplayType.Equals(other.m_preferredDisplayType) && "Can't merge descriptors with different display types");
    BeAssert(m_contentFlags == other.m_contentFlags && "Can't merge descriptors with different content flags");
    BeAssert(m_sortDirection == other.m_sortDirection && "Can't merge descriptors with different sort directions");
    BeAssert(m_filterExpression.Equals(other.m_filterExpression) && "Can't merge descriptors with different filter expressions");

    for (SelectClassInfo const& sourceClassInfo : other.m_classes)
        {
        auto iter = std::find(m_classes.begin(), m_classes.end(), sourceClassInfo);
        if (m_classes.end() == iter)
            m_classes.push_back(sourceClassInfo);
        }

    bmap<Field const*, Field const*> fieldsRemapInfo;
    for (Field const* sourceField : other.m_fields)
        {
        bool found = false;
        for (Field* targetField : m_fields)
            {
            if (*targetField == *sourceField)
                {
                found = true;
                break;
                }
            if (targetField->IsPropertiesField() && sourceField->IsPropertiesField())
                {
                for (ContentDescriptor::Property const& sourceProperty : sourceField->AsPropertiesField()->GetProperties())
                    {
                    for (ContentDescriptor::Property& targetProperty : targetField->AsPropertiesField()->GetProperties())
                        {
                        if (sourceProperty.GetProperty().GetName().Equals(targetProperty.GetProperty().GetName()) &&
                            sourceProperty.GetProperty().GetTypeName().Equals(targetProperty.GetProperty().GetTypeName()))
                            {
                            bvector<ContentDescriptor::Property>& propertyVector = targetField->AsPropertiesField()->GetProperties();
                            propertyVector.push_back(sourceProperty);
                            fieldsRemapInfo[sourceField] = targetField;
                            found = true;
                            break;
                            }
                        }
                    if (found)
                        break;
                    }
                }
            if (found)
                break;
            }
        if (!found)
            {
            Field* copy = sourceField->Clone();
            m_fields.push_back(copy);
            fieldsRemapInfo[sourceField] = copy;
            }
        }
    for (Field* field : m_fields)
        field->NotifyFieldsCloned(fieldsRemapInfo);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::SetContentFlags(int flags)
    {
    int diff = flags ^ m_contentFlags;
    int additions = flags & diff;
    int removals = m_contentFlags & diff;

    m_contentFlags = flags;

    int steps = 0;
    while (additions > 0)
        {
        int addition = (additions & 1) << steps;
        if (0 != addition)
            OnFlagAdded((ContentFlags)addition);
        additions >>= 1;
        ++steps;
        }

    steps = 0;
    while (removals > 0)
        {
        int removal = removals & 1;
        if (0 != removal)
            OnFlagRemoved((ContentFlags)removal);
        removals >>= 1;
        ++steps;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::HasContentFlag(ContentFlags flag) const
    {
    return 0 != ((int)flag & m_contentFlags);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::AddContentFlag(ContentFlags flag)
    {
    if (0 != ((int)flag & m_contentFlags))
        return;

    m_contentFlags |= (int)flag;
    OnFlagAdded(flag);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::RemoveContentFlag(ContentFlags flag)
    {
    if (0 == ((int)flag & m_contentFlags))
        return;

    m_contentFlags &= ~(int)flag;
    OnFlagRemoved(flag);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnFlagAdded(ContentFlags flag)
    {
    if (ContentFlags::ShowLabels == flag)
        {
        Utf8String displayLabel = L10N::GetString(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_General_DisplayLabel());
        if (displayLabel.empty())
            {
            BeAssert(false);
            displayLabel = "Display Label";
            }
        m_fields.insert(m_fields.begin(), new DisplayLabelField(displayLabel));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnFlagRemoved(ContentFlags flag)
    {
    if (ContentFlags::ShowLabels == flag)
        {
        auto iter = std::remove_if(m_fields.begin(), m_fields.end(), [](Field const* field){return field->IsDisplayLabelField();});
        if (m_fields.end() != iter)
            {
            delete *iter;
            m_fields.erase(iter);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetClassInfoJson(ECClassCR ecClass, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Document json(&allocator);
    json.SetObject();
    json.AddMember("Id", rapidjson::Value(ecClass.GetId().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Name", rapidjson::StringRef(ecClass.GetFullName()), json.GetAllocator());
    json.AddMember("Label", rapidjson::StringRef(ecClass.GetDisplayLabel().c_str()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Value CreateRelationshipPathJson(RelatedClassPathCR path, rapidjson::Document::AllocatorType& allocator)
    {
    rapidjson::Value pathJson(rapidjson::kArrayType);
    for (RelatedClass const& relatedClass : path)
        {
        BeAssert(relatedClass.IsValid());
        rapidjson::Value relatedClassJson(rapidjson::kObjectType);
        relatedClassJson.AddMember("SourceClassInfo", GetClassInfoJson(*relatedClass.GetSourceClass(), allocator), allocator);
        relatedClassJson.AddMember("TargetClassInfo", GetClassInfoJson(*relatedClass.GetTargetClass(), allocator), allocator);
        relatedClassJson.AddMember("RelationshipInfo", GetClassInfoJson(*relatedClass.GetRelationship(), allocator), allocator);
        relatedClassJson.AddMember("IsForwardRelationship", relatedClass.IsForwardRelationship(), allocator);
        relatedClassJson.AddMember("IsPolymorphicRelationship", relatedClass.IsPolymorphic(), allocator);
        pathJson.PushBack(relatedClassJson, allocator);
        }
    return pathJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    bvector<Field*> visibleFields = GetVisibleFields();
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("PreferredDisplayType", rapidjson::StringRef(m_preferredDisplayType.c_str()), json.GetAllocator());
    json.AddMember("SelectClasses", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (SelectClassInfo const& selectClass : m_classes)
        {
        rapidjson::Value selectClassJson(rapidjson::kObjectType);
        selectClassJson.AddMember("SelectClassInfo", GetClassInfoJson(selectClass.GetSelectClass(), json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("IsPolymorphic", selectClass.IsSelectPolymorphic(), json.GetAllocator());
        selectClassJson.AddMember("PathToPrimaryClass", CreateRelationshipPathJson(selectClass.GetPathToPrimaryClass(), json.GetAllocator()), json.GetAllocator());
        selectClassJson.AddMember("RelatedPropertyPaths", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (RelatedClassPathCR propertyPath : selectClass.GetRelatedPropertyPaths())
            selectClassJson["RelatedPropertyPaths"].PushBack(CreateRelationshipPathJson(propertyPath, json.GetAllocator()), json.GetAllocator());
        json["SelectClasses"].PushBack(selectClassJson, json.GetAllocator());
        }
    json.AddMember("Fields", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
    for (Field const* field : visibleFields)
        json["Fields"].PushBack(field->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("SortingFieldIndex", m_sortingFieldIndex, json.GetAllocator());
    json.AddMember("SortDirection", (int)m_sortDirection, json.GetAllocator());
    json.AddMember("ContentFlags", m_contentFlags, json.GetAllocator());
    json.AddMember("FilterExpression", rapidjson::StringRef(m_filterExpression.c_str()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Category::AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Name", rapidjson::Value(GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("DisplayLabel", rapidjson::Value(GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Description", rapidjson::Value(GetDescription().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Expand", ShouldExpand(), json.GetAllocator());
    json.AddMember("Priority", GetPriority(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category ContentDescriptor::Category::GetDefaultCategory()
    {
    Utf8String label = L10N::GetString(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_Category_Miscellaneous());
    return ContentDescriptor::Category("Miscellaneous", label, "", 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category ContentDescriptor::Category::GetFavoriteCategory()
    {
    Utf8String label = L10N::GetString(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_Category_Favorite());
    return ContentDescriptor::Category("Favorite", label, "", 500000, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::Property::operator==(Property const& other) const
    {
    return m_propertyClass == other.m_propertyClass
        && m_property == other.m_property 
        && m_prefix.Equals(other.m_prefix) 
        && m_relatedClassPath == other.m_relatedClassPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Value GetEnumerationChoicesJson(ECEnumerationCR enumeration, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (ECEnumeratorCP enumerator : enumeration.GetEnumerators())
        {
        rapidjson::Value choice(rapidjson::kObjectType);
        choice.AddMember("Label", rapidjson::Value(enumerator->GetDisplayLabel().c_str(), allocator), allocator);
        if (enumerator->IsInteger())
            choice.AddMember("Value", rapidjson::Value(enumerator->GetInteger()), allocator);
        else
            choice.AddMember("Value", rapidjson::Value(enumerator->GetString().c_str(), allocator), allocator);

        json.PushBack(choice, allocator);
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Value CreateKindOfQuantityJson(KindOfQuantityCR koq, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Value json(rapidjson::kObjectType);
    json.AddMember("Name", rapidjson::StringRef(koq.GetFullName().c_str()), allocator);
    json.AddMember("DisplayLabel", rapidjson::StringRef(koq.GetDisplayLabel().c_str()), allocator);
    json.AddMember("PersistenceUnit", rapidjson::Value(koq.GetPersistenceUnit().ToText(true).c_str(), allocator), allocator);
    json.AddMember("CurrentUnit", rapidjson::Value(koq.GetDefaultPresentationUnit().ToText(true).c_str(), allocator), allocator);

    rapidjson::Value units(rapidjson::kArrayType);
    for (auto unit : koq.GetPresentationUnitList())
        units.PushBack(rapidjson::Value(unit.ToText(true).c_str(), allocator), allocator);

    json.AddMember("PresentationUnits", units, allocator);

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Property::AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    
    rapidjson::Value propertyJson(rapidjson::kObjectType);
    propertyJson.AddMember("BaseClassInfo", GetClassInfoJson(m_property->GetClass(), json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("ActualClassInfo", GetClassInfoJson(*m_propertyClass, json.GetAllocator()), json.GetAllocator());
    propertyJson.AddMember("Name", rapidjson::StringRef(m_property->GetName().c_str()), json.GetAllocator());

    if (m_property->GetIsPrimitive() && nullptr != m_property->GetAsPrimitiveProperty()->GetEnumeration())
        {
        ECEnumerationCP propEnum = m_property->GetAsPrimitiveProperty()->GetEnumeration();
        propertyJson.AddMember("Type", rapidjson::Value("enum", json.GetAllocator()), json.GetAllocator());
        propertyJson.AddMember("IsStrict", rapidjson::Value(propEnum->GetIsStrict()), json.GetAllocator());
        propertyJson.AddMember("Choices", GetEnumerationChoicesJson(*propEnum, json.GetAllocator()), json.GetAllocator());
        }
    else
        {
        propertyJson.AddMember("Type", rapidjson::Value(m_property->GetTypeName().c_str(), json.GetAllocator()), json.GetAllocator());
        }

    if (nullptr != m_property->GetKindOfQuantity())
        {
        KindOfQuantityCP koq = m_property->GetKindOfQuantity();
        propertyJson.AddMember("KindOfQuantity", CreateKindOfQuantityJson(*koq, json.GetAllocator()), json.GetAllocator());
        }

    json.AddMember("Property", propertyJson, json.GetAllocator());
    json.AddMember("RelatedClassPath", CreateRelationshipPathJson(m_relatedClassPath, json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveECPropertyCP ContentDescriptor::Property::GetPrimitiveProperty(StructECPropertyCR structProperty, Utf8StringCR accessString)
    {
    ECPropertyCP ecProperty = &structProperty;
    bvector<Utf8String> accessorSplits;
    BeStringUtilities::Split(accessString.c_str(), ".", accessorSplits);
    for (size_t i = 1; i < accessorSplits.size(); ++i)
        {
        if (!ecProperty->GetIsStruct())
            {
            BeAssert(false);
            break;
            }
        ECStructClassCR structClass = ecProperty->GetAsStructProperty()->GetType();
        ecProperty = structClass.GetPropertyP(accessorSplits[i].c_str());
        }
    if (nullptr == ecProperty || !ecProperty->GetIsPrimitive())
        {
        BeAssert(false);
        return nullptr;
        }
    return ecProperty->GetAsPrimitiveProperty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescription const& ContentDescriptor::Field::GetTypeDescription() const
    {
    if (m_type.IsNull())
        m_type = _CreateTypeDescription();
    return *m_type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Category", GetCategory().AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("Name", rapidjson::Value(GetName().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("DisplayLabel", rapidjson::Value(GetLabel().c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("Type", GetTypeDescription().AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("IsReadOnly", _IsReadOnly(), json.GetAllocator());
    json.AddMember("Priority", _GetPriority(), json.GetAllocator());
    json.AddMember("Editor", rapidjson::Value(GetEditor().c_str(), json.GetAllocator()), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::DisplayLabelField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("string");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::CalculatedPropertyField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("string");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Field::TypeDescriptionPtr CreateTypeDescription(ECPropertyCR prop) 
    {
    if (prop.GetIsPrimitive() && nullptr != prop.GetAsPrimitiveProperty()->GetEnumeration())
        return new PrimitiveTypeDescription("enum");

    if (prop.GetIsNavigation())
        return new PrimitiveTypeDescription("navigation");
    
    if (prop.GetIsPrimitiveArray())
        return new ArrayTypeDescription(*new PrimitiveTypeDescription(prop.GetTypeName()));

    if (prop.GetIsStructArray())
        return new ArrayTypeDescription(*new StructTypeDescription(prop.GetAsStructArrayProperty()->GetStructElementType()));

    if (prop.GetIsStruct())
        return new StructTypeDescription(prop.GetAsStructProperty()->GetType());

    return new PrimitiveTypeDescription(prop.GetTypeName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::ECPropertiesField::_CreateTypeDescription() const
    {
    return CreateTypeDescription(m_properties.front().GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::IsCompositePropertiesField() const
    {
    if (m_properties.empty())
        return false;

    ECPropertyCR prop = m_properties.front().GetProperty();
    return prop.GetIsStruct() || prop.GetIsArray();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::ECPropertiesField::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = Field::_AsJson(allocator);

    rapidjson::Value propertiesJson(rapidjson::kArrayType);
    for (Property const& prop : m_properties)
        propertiesJson.PushBack(prop.AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("Properties", propertiesJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::_Equals(Field const& other) const
    {
    if (!Field::_Equals(other) || !other.IsPropertiesField())
        return false;

    if (m_properties.empty() || other.AsPropertiesField()->GetProperties().empty())
        return true;

    // if both lists have elements, just compare the first one
    return m_properties[0] == other.AsPropertiesField()->GetProperties()[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::_IsReadOnly() const
    {
    // just tell the field is read only if at least one of its properties is read only
    for (Property const& prop : m_properties)
        {
        if (prop.GetProperty().GetIsReadOnly())
            return true;
        }

    if (GetProperties().front().GetProperty().GetIsNavigation() && GetEditor().empty())
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::ECPropertiesField::InitFromProperty(ECClassCR primaryClass, ContentDescriptor::Property const& prop, 
    IPropertyCategorySupplierP categorySupplier)
    {
    if (nullptr != categorySupplier)
        SetCategory(categorySupplier->GetCategory(primaryClass, prop.GetRelatedClassPath(), prop.GetProperty()));
    SetName(Utf8String(prop.GetPropertyClass().GetName()).append("_").append(prop.GetProperty().GetName()));
    SetLabel(prop.GetProperty().GetDisplayLabel());
    m_properties.push_back(prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentDescriptor::Property const*> ContentDescriptor::ECPropertiesField::FindMatchingProperties(ECClassCP targetClass) const
    {
    bvector<ContentDescriptor::Property const*> matchingProperties;
    if (m_properties.empty())
        return matchingProperties;

    if (nullptr == targetClass)
        {
        for (Property const& p : m_properties)
            matchingProperties.push_back(&p);
        }
    else
        {
        for (Property const& prop : m_properties)
            {
            if (targetClass->Is(&prop.GetPropertyClass()) || prop.IsRelated() && targetClass->Is(prop.GetRelatedClassPath().front().GetTargetClass()))
                matchingProperties.push_back(&prop);
            }
        }
    return matchingProperties;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tautvydas.Zinys                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentDescriptor::ECPropertiesField::_GetPriority() const
    {
    int maxPriority = 0;
    for (Property prop : m_properties)
        {
        if (prop.GetPriority() > maxPriority)
            maxPriority = prop.GetPriority();
        }

    return maxPriority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::NestedContentField::_CreateTypeDescription() const
    {
    return new NestedContentTypeDescription(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::NestedContentField::_Equals(Field const& other) const
    {
    if (!Field::_Equals(other) || !other.IsNestedContentField())
        return false;

    NestedContentField const* otherNestedContentField = other.AsNestedContentField();
    if (&m_contentClass != &otherNestedContentField->m_contentClass
        || m_relationshipPath != otherNestedContentField->m_relationshipPath
        || m_priority != otherNestedContentField->m_priority)
        {
        return false;
        }

    if (m_fields.size() != otherNestedContentField->m_fields.size())
        return false;

    for (size_t i = 0; i < m_fields.size(); ++i)
        {
        if (*m_fields[i] != *otherNestedContentField->m_fields[i])
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::NestedContentField::_AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json = Field::_AsJson(allocator);
    json.AddMember("ContentClassInfo", GetClassInfoJson(m_contentClass, json.GetAllocator()), json.GetAllocator());
    json.AddMember("PathToPrimary", CreateRelationshipPathJson(m_relationshipPath, json.GetAllocator()), json.GetAllocator());
    
    rapidjson::Value nestedFieldsJson(rapidjson::kArrayType);
    for (Field const* nestedField : m_fields)
        nestedFieldsJson.PushBack(nestedField->AsJson(&json.GetAllocator()), json.GetAllocator());
    json.AddMember("NestedFields", nestedFieldsJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::ECInstanceKeyField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("ECInstanceKey");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::ECInstanceKeyField::RecalculateName()
    {
    Utf8String name = "key";
    bset<Utf8String> uniqueAliases;
    for (ECPropertiesField const* field : m_keyFields)
        {
        for (Property const& prop : field->GetProperties())
            {
            if (uniqueAliases.end() != uniqueAliases.find(prop.GetPrefix()))
                continue;

            if (prop.GetProperty().GetIsNavigation())
                continue;

            name.append("_").append(prop.GetPrefix());
            uniqueAliases.insert(prop.GetPrefix());
            }
        }
    SetName(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::ECInstanceKeyField::_OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo)
    {
    for (size_t i = 0; i < m_keyFields.size(); ++i)
        {
        auto iter = fieldsRemapInfo.find(m_keyFields[i]);
        if (fieldsRemapInfo.end() != iter)
            {
            BeAssert(iter->second->IsPropertiesField());
            m_keyFields[i] = iter->second->AsPropertiesField();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECInstanceKeyField::_OnFieldRemoved(ContentDescriptor::Field const& field)
    {
    for (size_t i = 0; i < m_keyFields.size(); ++i)
        {
        if (&field == m_keyFields[i])
            {
            m_keyFields.erase(m_keyFields.begin() + i);
            break;
            }
        }
    return m_keyFields.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas               09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECNavigationInstanceIdField::_Equals(Field const& other) const
    {
    if (!other.IsSystemField() || !other.AsSystemField()->IsECNavigationInstanceIdField())
        return false;

    if (*other.AsSystemField()->AsECNavigationInstanceIdField()->m_propertyField != *m_propertyField)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::ECNavigationInstanceIdField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("ECInstanceId");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas               08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::ECNavigationInstanceIdField::_OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo)
    {
    auto iter = fieldsRemapInfo.find(m_propertyField);
    if (fieldsRemapInfo.end() == iter)
        return;

    if (!iter->second->IsPropertiesField())
        {
        BeAssert(false);
        return;
        }

    m_propertyField = iter->second->AsPropertiesField();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECNavigationInstanceIdField::_OnFieldRemoved(ContentDescriptor::Field const& field)
    {
    return (&field == m_propertyField);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSetItem::IsMerged(Utf8StringCR fieldName) const
    {
    return (m_mergedFieldNames.end() != std::find(m_mergedFieldNames.begin(), m_mergedFieldNames.end(), fieldName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceKey> const& ContentSetItem::GetPropertyValueKeys(FieldProperty const& fp) const
    {
    auto iter = m_fieldPropertyInstanceKeys.find(fp);
    if (m_fieldPropertyInstanceKeys.end() == iter)
        {
        static bvector<ECInstanceKey> s_empty;
        return s_empty;
        }
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentSetItem::AsJson(int flags, rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();

    if (0 != (SERIALIZE_DisplayLabel & flags))
        json.AddMember("DisplayLabel", rapidjson::Value(m_displayLabel.c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (SERIALIZE_ImageId & flags))
        json.AddMember("ImageId", rapidjson::Value(m_imageId.c_str(), json.GetAllocator()), json.GetAllocator());

    if (0 != (SERIALIZE_Values & flags))
        json.AddMember("Values", rapidjson::Value(m_values, json.GetAllocator()), json.GetAllocator());

    if (0 != (SERIALIZE_DisplayValues & flags))
        json.AddMember("DisplayValues", rapidjson::Value(m_displayValues, json.GetAllocator()), json.GetAllocator());

    if (m_class != nullptr && 0 != (SERIALIZE_ClassInfo & flags))
        json.AddMember("ClassInfo", GetClassInfoJson(*m_class, json.GetAllocator()), json.GetAllocator());

    if (0 != (SERIALIZE_PrimaryKeys & flags))
        {
        rapidjson::Value primaryKeys(rapidjson::kArrayType);
        for (ECInstanceKeyCR key : m_keys)
            primaryKeys.PushBack(ValueHelpers::GetJson(key, &json.GetAllocator()), json.GetAllocator());
        json.AddMember("PrimaryKeys", primaryKeys, json.GetAllocator());
        }

    if (0 != (SERIALIZE_MergedFieldNames & flags))
        {
        rapidjson::Value fieldNamesJson(rapidjson::kArrayType);
        for (Utf8StringCR fieldName : m_mergedFieldNames)
            fieldNamesJson.PushBack(rapidjson::Value(fieldName.c_str(), json.GetAllocator()), json.GetAllocator());
        json.AddMember("MergedFieldNames", fieldNamesJson, json.GetAllocator());
        }

    if (0 != (SERIALIZE_FieldPropertyInstanceKeys & flags))
        {
        rapidjson::Value fieldValueKeys(rapidjson::kObjectType);
        for (auto pair : m_fieldPropertyInstanceKeys)
            {
            Utf8CP fieldName = pair.first.GetField().GetName().c_str();
            if (!fieldValueKeys.HasMember(fieldName))
                fieldValueKeys.AddMember(rapidjson::StringRef(fieldName), rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
            rapidjson::Value& fieldProperties = fieldValueKeys[fieldName];

        rapidjson::Value propertyKeys(rapidjson::kArrayType);
        for (ECInstanceKeyCR key : pair.second)
            propertyKeys.PushBack(ValueHelpers::GetJson(key, &json.GetAllocator()), json.GetAllocator());

            rapidjson::Value fieldProperty(rapidjson::kObjectType);
            fieldProperty.AddMember("PropertyIndex", rapidjson::Value((uint64_t)pair.first.GetPropertyIndex()), json.GetAllocator());
            fieldProperty.AddMember("Keys", propertyKeys, json.GetAllocator());
            fieldProperties.PushBack(fieldProperty, json.GetAllocator());
            }
        json.AddMember("FieldValueKeys", fieldValueKeys, json.GetAllocator());
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document Content::AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("Descriptor", m_descriptor->AsJson(&json.GetAllocator()), json.GetAllocator());

    rapidjson::Value set(rapidjson::kArrayType);
    DataContainer<ContentSetItemCPtr> container = GetContentSet();
    for (ContentSetItemCPtr item : container)
        {
        if (item.IsValid())
            set.PushBack(item->AsJson(ContentSetItem::SERIALIZE_All, &json.GetAllocator()), json.GetAllocator());
        }
    json.AddMember("ContentSet", set, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult ECInstanceChangeResult::Success(ECValue changedValue)
    {
    ECInstanceChangeResult result(SUCCESS);
    result.m_changedValue = changedValue;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult ECInstanceChangeResult::Error(Utf8String message)
    {
    ECInstanceChangeResult result(ERROR);
    result.m_errorMessage = message;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceChangeResult::AsJson(rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    if (SUCCESS == m_status)
        {
        json.AddMember("IsSuccess", true, json.GetAllocator());
        json.AddMember("Value", ValueHelpers::GetJsonFromECValue(m_changedValue, &json.GetAllocator()), json.GetAllocator());
        }
    else
        {
        json.AddMember("IsSuccess", false, json.GetAllocator());
        json.AddMember("ErrorMessage", rapidjson::Value(m_errorMessage.c_str(), json.GetAllocator()), json.GetAllocator());
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionInfo::SelectionInfo(ISelectionProvider const& selectionProvider, SelectionChangedEventCR evt)
    : m_isValid(true), m_selectionProviderName(evt.GetSourceName()), m_isSubSelection(evt.IsSubSelection()), 
    m_keys(evt.IsSubSelection() ? selectionProvider.GetSubSelection(evt.GetConnection()) : selectionProvider.GetSelection(evt.GetConnection()))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionInfo::SelectionInfo(bvector<ECClassCP> const& classes)
    : m_isValid(true), m_isSubSelection(false)
    {
    NavNodeKeyList keys;
    for (ECClassCP ecClass : classes)
        keys.push_back(ECInstanceNodeKey::Create(ecClass->GetId(), ECInstanceId()));
    m_keys = NavNodeKeyListContainer::Create(std::move(keys));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionInfo& SelectionInfo::operator=(SelectionInfo const& other)
    {
    m_isValid = other.m_isValid;
    m_selectionProviderName = other.m_selectionProviderName;
    m_isSubSelection = other.m_isSubSelection;
    m_keys = other.m_keys;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionInfo& SelectionInfo::operator=(SelectionInfo&& other)
    {
    m_isValid = other.m_isValid;
    m_isSubSelection = other.m_isSubSelection;
    m_selectionProviderName.swap(other.m_selectionProviderName);
    m_keys.swap(other.m_keys);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectionInfo::operator==(SelectionInfo const& other) const
    {
    if (m_isValid != other.m_isValid || !m_selectionProviderName.Equals(other.m_selectionProviderName) || m_isSubSelection != other.m_isSubSelection)
        return false;

    if (m_keys->size() != other.m_keys->size())
        return false;

    for (auto thisIter = m_keys->begin(); thisIter != m_keys->end(); ++thisIter)
        {
        for (auto otherIter = other.m_keys->begin(); otherIter != other.m_keys->end(); ++otherIter)
            {
            NavNodeKeyCP thisKey = (*thisIter).get();
            NavNodeKeyCP otherKey = (*otherIter).get();
            if (0 != thisKey->Compare(*otherKey))
                return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
    {
    if (SUCCESS == ValueHelpers::GetEnumPropertyDisplayValue(formattedValue, ecProperty, ecValue))
        return SUCCESS;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR ecClass, RelatedClassPath const& relationshipPath, RelationshipMeaning relationshipMeaning) const
    {
    formattedLabel.clear();
    if (!relationshipPath.empty() && RelationshipMeaning::RelatedInstance == relationshipMeaning)
        formattedLabel.append(ecClass.GetDisplayLabel()).append(" ").append(ecProperty.GetDisplayLabel());
    else
        formattedLabel = ecProperty.GetDisplayLabel();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectionInfo::operator<(SelectionInfo const& other) const
    {
    if (!m_isValid)
        return false;

    if (!m_isSubSelection && other.m_isSubSelection)
        return true;
    if (m_isSubSelection && !other.m_isSubSelection)
        return false;

    int providerCmp = m_selectionProviderName.CompareTo(other.m_selectionProviderName);
    if (providerCmp < 0)
        return true;
    if (providerCmp > 0)
        return false;

    if (m_keys->size() < other.m_keys->size())
        return true;
    if (m_keys->size() > other.m_keys->size())
        return false;

    for (auto thisIter = m_keys->begin(); thisIter != m_keys->end(); ++thisIter)
        {
        for (auto otherIter = other.m_keys->begin(); otherIter != other.m_keys->end(); ++otherIter)
            {
            NavNodeKeyCP thisKey = (*thisIter).get();
            NavNodeKeyCP otherKey = (*otherIter).get();
            int keyCmp = thisKey->Compare(*otherKey);
            if (keyCmp < 0)
                return true;
            if (keyCmp > 0)
                return false;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category DefaultCategorySupplier::_GetCategory(ECClassCR, RelatedClassPathCR, ECPropertyCR prop)
    {
    PropertyCategoryCP propertyCategory = prop.GetCategory();
    if (nullptr == propertyCategory)
        return ContentDescriptor::Category::GetDefaultCategory();

    return ContentDescriptor::Category(propertyCategory->GetName(), propertyCategory->GetDisplayLabel(), propertyCategory->GetDescription(), propertyCategory->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category DefaultCategorySupplier::_GetCategory(ECClassCR, RelatedClassPathCR, ECClassCR nestedContentClass)
    {
    return ContentDescriptor::Category(nestedContentClass.GetName(), nestedContentClass.GetDisplayLabel(), "", NESTED_CONTENT_CATEGORY_PRIORITY);
    }
