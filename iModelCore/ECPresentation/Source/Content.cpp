/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Content.h>
#include <Units/Units.h>
#include <ECObjects/ECQuantityFormatting.h>
#include "ValueHelpers.h"
#include "RulesDriven/RulesEngine/LoggingHelper.h"
#include "RulesDriven/RulesEngine/QueryBuilderHelpers.h"
#include "../Localization/Xliffs/ECPresentation.xliff.h"

const int ContentDescriptor::Property::DEFAULT_PRIORITY = 0;

const Utf8CP ContentDisplayType::Undefined = "Undefined";
const Utf8CP ContentDisplayType::Grid = "Grid";
const Utf8CP ContentDisplayType::PropertyPane = "PropertyPane";
const Utf8CP ContentDisplayType::List = "List";
const Utf8CP ContentDisplayType::Graphics = "Graphics";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::Field::TypeDescription::Create(ECPropertyCR prop)
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
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::TypeDescription::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::PrimitiveTypeDescription::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::ArrayTypeDescription::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::StructTypeDescription::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::NestedContentTypeDescription::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentDescriptor::Field::ArrayTypeDescription::CreateTypeName(TypeDescription const& memberType)
    {
    Utf8String typeName = memberType.GetTypeName();
    typeName.append("[]");
    return typeName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor(IConnectionCR connection, JsonValueCR options, INavNodeKeysContainerCR input, Utf8String preferredDisplayType)
    : m_preferredDisplayType(preferredDisplayType), m_contentFlags(0), m_sortingFieldIndex(-1), m_sortDirection(SortDirection::Ascending), m_connectionId(connection.GetId()),
    m_inputKeys(&input), m_options(options)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor(ContentDescriptorCR other)
    : m_preferredDisplayType(other.m_preferredDisplayType), m_classes(other.m_classes), m_filterExpression(other.m_filterExpression), m_contentFlags(other.m_contentFlags),
    m_sortingFieldIndex(other.m_sortingFieldIndex), m_sortDirection(other.m_sortDirection), m_connectionId(other.m_connectionId), m_inputKeys(other.m_inputKeys),
    m_options(other.m_options), m_selectionInfo(other.m_selectionInfo)
    {
    bmap<Field const*, Field const*> fieldsRemapInfo;
    for (Field const* field : other.m_fields)
        {
        Field* copy = field->Clone();
        AddField(copy);
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
        || m_fields.size() != other.m_fields.size()
        || !m_connectionId.Equals(other.m_connectionId)
        || !m_inputKeys->GetHash().Equals(other.m_inputKeys->GetHash())
        || m_options != other.m_options)
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

    if ((m_selectionInfo.IsValid() && other.m_selectionInfo.IsNull()) || (m_selectionInfo.IsNull() && other.m_selectionInfo.IsValid()))
        return false;

    if (m_selectionInfo.IsValid() && other.m_selectionInfo.IsValid() && !(*m_selectionInfo == *other.m_selectionInfo))
        return false;

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
    std::copy_if(m_fields.begin(), m_fields.end(), std::back_inserter(fields), [this](Field const* f)
        {
        bool isHidden = (f->IsSystemField() || (f->IsDisplayLabelField() && !ShowLabels()));
        return !isHidden;
        });
    return fields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field const* ContentDescriptor::GetDistinctField() const
    {
    if (OnlyDistinctValues() && GetVisibleFields().size() == 1)
        {
        Field const* field = GetVisibleFields()[0];
        if (field->IsPropertiesField() || field->IsCalculatedPropertyField() || (field->IsDisplayLabelField() && ShowLabels()))
            return field;
        }
    return nullptr;
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

    if (field.IsSystemField() && field.AsSystemField()->IsECInstanceKeyField())
        {
        auto keyField = std::find_if(m_keyFields.begin(), m_keyFields.end(), [&field](Field const* f) {return f == &field;});
        if (m_keyFields.end() != keyField)
            m_keyFields.erase(keyField);
        }

    if (field.IsPropertiesField())
        OnECPropertiesFieldRemoved(*field.AsPropertiesField());

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
    BeAssert(m_connectionId.Equals(other.m_connectionId) && "Can't merge descriptors with different connections");
    BeAssert(m_options == other.m_options && "Can't merge descriptors with different options");
    BeAssert((m_selectionInfo.IsNull() && other.m_selectionInfo.IsNull())
        || (m_selectionInfo.IsValid() && other.m_selectionInfo.IsValid() && *m_selectionInfo == *other.m_selectionInfo)
        && "Can't merge descriptors with different selection info");

    for (SelectClassInfo const& sourceClassInfo : other.m_classes)
        {
        auto iter = std::find(m_classes.begin(), m_classes.end(), sourceClassInfo);
        if (m_classes.end() == iter)
            m_classes.push_back(sourceClassInfo);
        }

    NavNodeKeyList newKeys;
    for (NavNodeKeyCPtr inputKey : *other.m_inputKeys)
        {
        if (m_inputKeys->end() != m_inputKeys->find(inputKey))
            newKeys.push_back(inputKey);
        }
    if (0 != newKeys.size())
        {
        for (NavNodeKeyCPtr inputKey : *m_inputKeys)
            newKeys.push_back(inputKey);

        m_inputKeys = NavNodeKeyListContainer::Create(newKeys);
        }

    bmap<Field const*, Field const*> fieldsRemapInfo;
    for (Field const* sourceField : other.m_fields)
        {
        bool found = false;
        if (sourceField->IsPropertiesField())
            {
            ECPropertiesField* targetField = FindECPropertiesField(sourceField->AsPropertiesField()->GetProperties().front(), sourceField->GetLabel(), sourceField->GetCategory(), sourceField->GetEditor());
            if (nullptr != targetField)
                {
                found = true;
                for (ContentDescriptor::Property const& sourceProperty : sourceField->AsPropertiesField()->GetProperties())
                    targetField->AddProperty(sourceProperty);

                fieldsRemapInfo[sourceField] = targetField;
                }
            }
        if (found)
            continue;

        for (Field* targetField : m_fields)
            {
            if (targetField->IsPropertiesField())
                continue;

            if (*targetField == *sourceField)
                {
                found = true;
                break;
                }
            }

        if (!found)
            {
            Field* copy = sourceField->Clone();
            AddField(copy);
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
void ContentDescriptor::OnFlagAdded(ContentFlags flag) {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnFlagRemoved(ContentFlags flag) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::AddField(Field* field)
    {
    m_fields.push_back(field);
    if (field->IsSystemField() && field->AsSystemField()->IsECInstanceKeyField())
        m_keyFields.push_back(field->AsSystemField()->AsECInstanceKeyField());
    if (nullptr != field->AsPropertiesField())
        OnECPropertiesFieldAdded(*field->AsPropertiesField());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ECPropertiesField* ContentDescriptor::FindECPropertiesField(ECPropertyCR prop, ECClassCR propClass,
    RelatedClassPathCR pathFromSelectToPropertyClass, Utf8StringCR fieldLabel, ContentDescriptor::Category const& category, ContentFieldEditor const* editor)
    {
    auto iter = m_fieldsMap.find(ECPropertiesFieldKey(prop, propClass, pathFromSelectToPropertyClass, fieldLabel, category.GetName(), editor));
    if (m_fieldsMap.end() == iter)
        return nullptr;

    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnECPropertiesFieldRemoved(ECPropertiesField const& field)
    {
    Property const& prop = field.GetProperties().front();
    m_fieldsMap.erase(ECPropertiesFieldKey(prop, field.GetLabel(), field.GetCategory().GetName(), field.GetEditor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnECPropertiesFieldAdded(ECPropertiesField& field)
    {
    Property const& prop = field.GetProperties().front();
    m_fieldsMap[ECPropertiesFieldKey(prop, field.GetLabel(), field.GetCategory().GetName(), field.GetEditor())] = &field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Category::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category ContentDescriptor::Category::GetDefaultCategory()
    {
    Utf8String label = PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_Category_General());
    return ContentDescriptor::Category("General", label, "", 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category ContentDescriptor::Category::GetFavoriteCategory()
    {
    Utf8String label = PRESENTATION_LOCALIZEDSTRING(ECPresentationL10N::GetNameSpace(), ECPresentationL10N::LABEL_Category_Favorite());
    return ContentDescriptor::Category("Favorite", label, "", 500000, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category ContentDescriptor::Category::FromSpec(PropertyCategorySpecificationCR spec)
    {
    return ContentDescriptor::Category(spec.GetId(), spec.GetLabel(), spec.GetDescription(), spec.GetPriority(), spec.ShouldAutoExpand());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category ContentDescriptor::Category::FromSpec(Utf8StringCR id, PropertyCategorySpecificationsList const& specs)
    {
    auto iter = std::find_if(specs.begin(), specs.end(), [&id](PropertyCategorySpecificationCP spec) {return spec->GetId().Equals(id);});
    if (iter == specs.end())
        {
        bvector<Utf8String> availableCategoryIds;
        std::transform(specs.begin(), specs.end(), std::back_inserter(availableCategoryIds),
            [](PropertyCategorySpecificationCP spec) {return Utf8String("'").append(spec->GetId()).append("'");});
        LoggingHelper::LogMessage(Log::Content, Utf8PrintfString("Failed to find category '%s' in current scope. Available categories: [%s]",
            id.c_str(), BeStringUtilities::Join(availableCategoryIds, ",").c_str()).c_str(), LOG_WARNING);
        return Category();
        }
    return FromSpec(**iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::Property::operator==(Property const& other) const
    {
    return m_propertyClass == other.m_propertyClass
        && m_property == other.m_property
        && m_prefix.Equals(other.m_prefix)
        && m_pathFromSelectToPropertyClass == other.m_pathFromSelectToPropertyClass;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Property::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
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
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor* ContentFieldEditor::FromSpec(PropertyEditorSpecificationCR spec)
    {
    auto editor = new ContentFieldEditor(spec.GetEditorName());
    EditorParamsBuilder paramsBuilder(*editor);
    for (PropertyEditorParametersSpecificationCP paramsSpec : spec.GetParameters())
        paramsSpec->Accept(paramsBuilder);
    return editor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor::~ContentFieldEditor()
    {
    for (Params const* params : m_params)
        delete params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentFieldEditor::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentFieldEditor::Equals(ContentFieldEditor const& other) const
    {
    if (!m_name.Equals(other.m_name))
        return false;

    if (m_params.size() != other.m_params.size())
        return false;
    for (size_t i = 0; i < m_params.size(); ++i)
        {
        if (!m_params[i]->Equals(*other.m_params[i]))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor& ContentFieldEditor::operator=(ContentFieldEditor const& other)
    {
    m_name = other.m_name;
    for (Params const* otherParams : other.m_params)
        m_params.push_back(otherParams->Clone());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor& ContentFieldEditor::operator=(ContentFieldEditor&& other)
    {
    m_name = std::move(other.m_name);
    m_params.swap(other.m_params);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentFieldEditor::operator<(ContentFieldEditor const& other) const
    {
    if (m_name < other.m_name)
        return true;
    if (m_name > other.m_name)
        return false;
    return m_params < other.m_params;
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

const Utf8CP ContentDescriptor::DisplayLabelField::NAME = "/DisplayLabel/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::DisplayLabelField::~DisplayLabelField()
    {
    for (auto const& entry : m_labelOverrideValueSpecs)
        {
        for (InstanceLabelOverrideValueSpecification const* classSpec : entry.second)
            delete classSpec;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::DisplayLabelField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("string");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::DisplayLabelField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> ContentDescriptor::DisplayLabelField::CloneLabelOverrideValueSpecs(bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> const& specs)
    {
    bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> result;
    for (auto const& entry : specs)
        {
        bvector<InstanceLabelOverrideValueSpecification const*> classSpecs;
        for (InstanceLabelOverrideValueSpecification const* classSpec : entry.second)
            classSpecs.push_back(classSpec->Clone());
        result.Insert(entry.first, classSpecs);
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::CalculatedPropertyField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("string");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::CalculatedPropertyField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ECPropertiesField::ECPropertiesField(Category category, Property const& prop)
    {
    SetCategory(category);
    SetName(Utf8String("pc_").append(QueryBuilderHelpers::CreateClassNameForDescriptor(prop.GetPropertyClass())).append("_").append(prop.GetProperty().GetName()));
    SetLabel(prop.GetProperty().GetDisplayLabel());
    m_properties.push_back(prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::ECPropertiesField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::ECPropertiesField::_CreateTypeDescription() const
    {
    return TypeDescription::Create(m_properties.front().GetProperty());
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
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::_Equals(Field const& other) const
    {
    if (!Field::_Equals(other) || !other.IsPropertiesField())
        return false;

    if (m_properties.empty() || other.AsPropertiesField()->GetProperties().empty())
        return true;

    // if both lists have elements, just compare the first one
    ECPropertiesFieldKey thisKey(m_properties[0], GetLabel(), GetCategory().GetName(), GetEditor());
    ECPropertiesFieldKey otherKey(other.AsPropertiesField()->GetProperties()[0], other.GetLabel(), other.GetCategory().GetName(), other.GetEditor());
    return !(thisKey < otherKey) && !(otherKey < thisKey);
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

    if (GetProperties().front().GetProperty().GetIsNavigation() && nullptr == GetEditor())
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentDescriptor::Property const*> const& ContentDescriptor::ECPropertiesField::FindMatchingProperties(ECClassCP targetClass) const
    {
    static bvector<ContentDescriptor::Property const*> const s_empty;
    if (m_properties.empty())
        return s_empty;

    auto iter = m_matchingPropertiesCache.find(targetClass);
    if (m_matchingPropertiesCache.end() == iter)
        {
        bvector<ContentDescriptor::Property const*> matchingProperties;
        if (nullptr == targetClass)
            {
            for (Property const& p : m_properties)
                matchingProperties.push_back(&p);
            }
        else
            {
            for (Property const& prop : m_properties)
                {
                if (targetClass->Is(&prop.GetPropertyClass()) || prop.IsRelated() && targetClass->Is(prop.GetPathFromSelectToPropertyClass().front().GetSourceClass()))
                    matchingProperties.push_back(&prop);
                }
            }
        iter = m_matchingPropertiesCache.insert(std::make_pair(targetClass, matchingProperties)).first;
        }

    return iter->second;
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
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateFieldName(ContentDescriptor::ECPropertiesField const& field)
    {
    Utf8String name;
    bvector<Utf8String> relatedClassNames;
    bvector<Utf8String> propertyClassNames;

    bset<ECClassCP> usedRelatedClasses;
    bset<ECClassCP> usedPropertyClasses;

    for (ContentDescriptor::Property const& prop : field.GetProperties())
        {
        if (prop.IsRelated())
            {
            for (RelatedClass const& related : prop.GetPathFromSelectToPropertyClass())
                {
                if (usedRelatedClasses.end() != usedRelatedClasses.find(related.GetSourceClass()))
                    continue;

                relatedClassNames.push_back(QueryBuilderHelpers::CreateClassNameForDescriptor(*related.GetSourceClass()));
                usedRelatedClasses.insert(related.GetSourceClass());
                }
            }

        if (usedPropertyClasses.end() == usedPropertyClasses.find(&prop.GetProperty().GetClass()))
            {
            propertyClassNames.push_back(QueryBuilderHelpers::CreateClassNameForDescriptor(prop.GetProperty().GetClass()));
            usedPropertyClasses.insert(&prop.GetProperty().GetClass());
            }
        }

    if (!relatedClassNames.empty())
        name.append("rc_").append(BeStringUtilities::Join(relatedClassNames, "_")).append("_");

    if (!propertyClassNames.empty())
        name.append("pc_").append(BeStringUtilities::Join(propertyClassNames, "_")).append("_");

    name.append(field.GetProperties().front().GetProperty().GetName());
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentDescriptor::ECPropertiesField::_GetName() const
    {
    if (!m_isValidName)
        const_cast<ECPropertiesField*>(this)->SetName(CreateFieldName(*this));

    return Field::_GetName();
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
    if (m_priority != otherNestedContentField->m_priority)
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
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::NestedContentField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::CompositeContentField::_Equals(Field const& other) const
    {
    if (!NestedContentField::_Equals(other) || nullptr == other.AsNestedContentField()->AsCompositeContentField())
        return false;

    CompositeContentField const* otherCompositeContentField = other.AsNestedContentField()->AsCompositeContentField();
    return m_contentClass.GetId() == otherCompositeContentField->m_contentClass.GetId()
        && m_contentClassAlias.Equals(otherCompositeContentField->m_contentClassAlias);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::CompositeContentField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::RelatedContentField::_Equals(Field const& other) const
    {
    if (!NestedContentField::_Equals(other) || nullptr == other.AsNestedContentField()->AsRelatedContentField())
        return false;

    RelatedContentField const* otherRelatedContentField = other.AsNestedContentField()->AsRelatedContentField();
    return m_pathFromSelectClassToContentClass == otherRelatedContentField->m_pathFromSelectClassToContentClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::RelatedContentField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::ECInstanceKeyField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::ECNavigationInstanceIdField::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
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
    Utf8String name("/key/");
    bset<Utf8String> uniqueAliases;
    for (ECPropertiesField const* field : m_keyFields)
        {
        for (Property const& prop : field->GetProperties())
            {
            if (uniqueAliases.end() != uniqueAliases.find(prop.GetPrefix()))
                continue;

            if (prop.GetProperty().GetIsNavigation())
                continue;

            if (!uniqueAliases.empty())
                name.append("_");
            name.append(prop.GetPrefix());
            uniqueAliases.insert(prop.GetPrefix());
            }
        }
    SetName(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentDescriptor::ECInstanceKeyField::_GetName() const
    {
    if (!m_isValidName)
        const_cast<ECInstanceKeyField*>(this)->RecalculateName();
    return Field::_GetName();
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
* @bsimethod                                    Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentDescriptor::ECNavigationInstanceIdField::_GetName() const
    {
    if (SystemField::_GetName().empty())
        {
        Utf8String name("/id/");
        name.append(m_propertyField->GetName());
        const_cast<ECNavigationInstanceIdField*>(this)->SetName(name);
        }
    return SystemField::_GetName();
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
* @bsimethod                                    Saulius.Skliutas                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesFieldKey::operator<(ECPropertiesFieldKey const& rhs) const
    {
    if (GetEditor() != rhs.GetEditor())
        {
        if (nullptr == GetEditor() && nullptr != rhs.GetEditor())
            return true;
        if (nullptr != GetEditor() && nullptr == rhs.GetEditor())
            return false;
        return *GetEditor() < *rhs.GetEditor();
        }

    bool areSimilar = (rhs.IsRelated() == IsRelated()) && (GetClass() == rhs.GetClass() || !IsRelated());
    if (!areSimilar)
        return m_pathFromSelectToPropertyClass < rhs.m_pathFromSelectToPropertyClass;

    if (GetValueKind() < rhs.GetValueKind())
        return true;
    if (GetValueKind() > rhs.GetValueKind())
        return false;

    int typeCmp = strcmp(GetType(), rhs.GetType());
    if (typeCmp < 0)
        return true;
    if (typeCmp > 0)
        return false;

    int nameCmp = strcmp(GetName(), rhs.GetName());
    if (nameCmp < 0)
        return true;
    if (nameCmp > 0)
        return false;

    int labelCmp = strcmp(GetLabel(), rhs.GetLabel());
    if (labelCmp < 0)
        return true;
    if (labelCmp > 0)
        return false;

    int categoryNameCmp = strcmp(GetCategoryName().c_str(), rhs.GetCategoryName().c_str());
    if (categoryNameCmp < 0)
        return true;
    if (categoryNameCmp > 0)
        return false;

    return GetKoq() < rhs.GetKoq();
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
bvector<ECClassInstanceKey> const& ContentSetItem::GetPropertyValueKeys(FieldProperty const& fp) const
    {
    auto iter = m_fieldPropertyInstanceKeys.find(fp);
    if (m_fieldPropertyInstanceKeys.end() == iter)
        {
        static const bvector<ECClassInstanceKey> s_empty;
        return s_empty;
        }
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSetItem::AddUsersExtendedData(Utf8CP key, ECValueCR value)
    {
    m_extendedData.AddMember(rapidjson::Value(key, m_extendedData.GetAllocator()), ValueHelpers::GetJsonFromECValue(value, &m_extendedData.GetAllocator()), m_extendedData.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentSetItem::AsJson(int flags, rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, flags, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document Content::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
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
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult ECInstanceChangeResult::Ignore(Utf8String reason)
    {
    ECInstanceChangeResult result(SUCCESS);
    result.m_errorMessage = reason;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceChangeResult::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectionInfo::operator==(SelectionInfo const& other) const
    {
    return m_isSubSelection == other.m_isSubSelection
        && m_timestamp == other.m_timestamp
        && m_selectionProviderName == other.m_selectionProviderName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectionInfo::operator<(SelectionInfo const& other) const
    {
    if (!m_isSubSelection && other.m_isSubSelection)
        return true;
    if (m_isSubSelection && !other.m_isSubSelection)
        return false;

    if (m_timestamp < other.m_timestamp)
        return true;
    if (m_timestamp > other.m_timestamp)
        return false;

    int selectionProviderNameCmp = m_selectionProviderName.CompareTo(other.m_selectionProviderName);
    if (selectionProviderNameCmp < 0)
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyEnumFormatting(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
    {
    return ValueHelpers::GetEnumPropertyDisplayValue(formattedValue, ecProperty, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyKoqFormatting(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
    {
    KindOfQuantityCP koq = ecProperty.GetKindOfQuantity();
    if (nullptr == koq)
        return ERROR;

    // currently only doubles are supported
    if (!ecValue.IsDouble())
        return ERROR;

    // determine the presentation unit
    NamedFormatCP format = koq->GetDefaultPresentationFormat();
    if (nullptr == format || nullptr == format->GetCompositeMajorUnit())
        {
        BeAssert(false);
        return ERROR;
        }

    // apply formatting
    ECQuantityFormattingStatus status;
    formattedValue = ECQuantityFormatting::FormatPersistedValue(ecValue.GetDouble(), koq, *format->GetCompositeMajorUnit(), *format, &status);
    if (ECQuantityFormattingStatus::Success != status)
        {
        formattedValue.clear();
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyDoubleFormatting(Utf8StringR formattedValue, double value) const
    {
    formattedValue.Sprintf("%.2f", value);
    if (formattedValue.Equals("-0.00"))
        formattedValue = "0.00";
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyPoint3dFormatting(Utf8StringR formattedValue, DPoint3dCR point) const
    {
    Utf8String xValue, yValue, zValue;
    if (SUCCESS != _ApplyDoubleFormatting(xValue, point.x) || SUCCESS != _ApplyDoubleFormatting(yValue, point.y) || SUCCESS != _ApplyDoubleFormatting(zValue, point.z))
        return ERROR;

    formattedValue.clear();
    formattedValue.append("X: ").append(xValue).append(" ");
    formattedValue.append("Y: ").append(yValue).append(" ");
    formattedValue.append("Z: ").append(zValue);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyPoint2dFormatting(Utf8StringR formattedValue, DPoint2dCR point) const
    {
    Utf8String xValue, yValue;
    if (SUCCESS != _ApplyDoubleFormatting(xValue, point.x) || SUCCESS != _ApplyDoubleFormatting(yValue, point.y))
        return ERROR;

    formattedValue.clear();
    formattedValue.append("X: ").append(xValue).append(" ");
    formattedValue.append("Y: ").append(yValue);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyDateTimeFormatting(Utf8StringR formattedValue, DateTimeCR dt) const
    {
    formattedValue = dt.ToString();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
    {
    if (ecValue.IsNull())
        {
        formattedValue.clear();
        return SUCCESS;
        }

    if (SUCCESS == _ApplyEnumFormatting(formattedValue, ecProperty, ecValue))
        return SUCCESS;

    if (SUCCESS == _ApplyKoqFormatting(formattedValue, ecProperty, ecValue))
        return SUCCESS;

    if (ecValue.IsDouble() && SUCCESS == _ApplyDoubleFormatting(formattedValue, ecValue.GetDouble()))
        return SUCCESS;

    if (ecValue.IsPoint3d() && SUCCESS == _ApplyPoint3dFormatting(formattedValue, ecValue.GetPoint3d()))
        return SUCCESS;

    if (ecValue.IsPoint2d() && SUCCESS == _ApplyPoint2dFormatting(formattedValue, ecValue.GetPoint2d()))
        return SUCCESS;

    if (ecValue.IsDateTime() && SUCCESS == _ApplyDateTimeFormatting(formattedValue, ecValue.GetDateTime()))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR ecClass, RelatedClassPath const& relationshipPath, RelationshipMeaning relationshipMeaning) const
    {
    formattedLabel = ecProperty.GetDisplayLabel();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category DefaultCategorySupplier::_GetECClassCategory(ECClassCR ecClass) const
    {
    return ContentDescriptor::Category(ecClass.GetName(), ecClass.GetDisplayLabel(), ecClass.GetDescription(), 1000);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category DefaultCategorySupplier::_GetRelatedECClassCategory(ECClassCR ecClass, RelationshipMeaning relationshipMeaning) const
    {
    // if relationship meaning is RelatedInstance, return category of the ECClass so all it's properties get into a separate group
    if (RelationshipMeaning::RelatedInstance == relationshipMeaning)
        return GetECClassCategory(ecClass);

    // otherwise return invalid category so parent field's category is used
    return ContentDescriptor::Category();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category DefaultCategorySupplier::_GetPropertyCategory(ECPropertyCR ecProperty) const
    {
    PropertyCategoryCP propertyCategory = ecProperty.GetCategory();
    if (nullptr != propertyCategory)
        {
        return ContentDescriptor::Category(propertyCategory->GetName(), propertyCategory->GetDisplayLabel(),
            propertyCategory->GetDescription(), propertyCategory->GetPriority());
        }
    return ContentDescriptor::Category();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Category DefaultCategorySupplier::_CreateCategory(ECClassCR actualClass, ECPropertyCR ecProperty, RelationshipMeaning relationshipMeaning) const
    {
    // if property has a category - that's always what we want
    ContentDescriptor::Category category = GetPropertyCategory(ecProperty);
    if (category.IsValid())
        return category;

    // if relationship meaning is SameInstance and property doesn't have a category,
    // just return the default category
    if (RelationshipMeaning::SameInstance == relationshipMeaning)
        return ContentDescriptor::Category::GetDefaultCategory();

    // if relationship meaning is RelatedInstance and property doesn't have a category,
    // return category of the ECClass the property belongs to
    if (RelationshipMeaning::RelatedInstance == relationshipMeaning)
        return GetECClassCategory(actualClass);

    // should never get here - return an invalid category
    return ContentDescriptor::Category();
    }
