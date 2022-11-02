/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Content.h>
#include <Units/Units.h>
#include <ECObjects/ECQuantityFormatting.h>
#include "ContentHelpers.h"
#include "../Shared/ValueHelpers.h"
#include "../Shared/ECSchemaHelper.h"

const int ContentDescriptor::Property::DEFAULT_PRIORITY = 0;

const Utf8CP ContentDisplayType::Undefined = "Undefined";
const Utf8CP ContentDisplayType::Grid = "Grid";
const Utf8CP ContentDisplayType::PropertyPane = "PropertyPane";
const Utf8CP ContentDisplayType::List = "List";
const Utf8CP ContentDisplayType::Graphics = "Graphics";

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateClassNameForDescriptor(ECClassCR ecClass)
    {
    return Utf8String(ecClass.GetSchema().GetAlias()).append("_").append(ecClass.GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::Field::TypeDescription::Create(ECPropertyCR prop)
    {
    if (prop.GetIsPrimitive() && nullptr != prop.GetAsPrimitiveProperty()->GetEnumeration())
        return new PrimitiveTypeDescription("enum");

    if (prop.GetIsNavigation())
        return new PrimitiveTypeDescription("navigation");

    if (prop.GetIsPrimitiveArray())
        return new ArrayTypeDescription(*new PrimitiveTypeDescription(ECSchemaHelper::GetTypeName(prop)));

    if (prop.GetIsStructArray())
        return new ArrayTypeDescription(*new StructTypeDescription(prop.GetAsStructArrayProperty()->GetStructElementType()));

    if (prop.GetIsStruct())
        return new StructTypeDescription(prop.GetAsStructProperty()->GetType());

    return new PrimitiveTypeDescription(ECSchemaHelper::GetTypeName(prop));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::TypeDescription::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::PrimitiveTypeDescription::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::ArrayTypeDescription::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::StructTypeDescription::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::NestedContentTypeDescription::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentDescriptor::Field::ArrayTypeDescription::CreateTypeName(TypeDescription const& memberType)
    {
    Utf8String typeName = memberType.GetTypeName();
    typeName.append("[]");
    return typeName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor(IConnectionCR connection, PresentationRuleSetCR ruleset, RulesetVariables rulesetVariables, INavNodeKeysContainerCR inputKeys)
    : m_connectionId(connection.GetId()), m_ruleset(&ruleset), m_rulesetVariables(rulesetVariables), m_inputKeys(&inputKeys),
    m_preferredDisplayType(ContentDisplayType::Undefined), m_contentFlags(0), m_unitSystem(UnitSystem::Undefined),
    m_sortingFieldIndex(-1), m_sortDirection(SortDirection::Ascending)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ContentDescriptor(ContentDescriptorCR other)
    : m_preferredDisplayType(other.m_preferredDisplayType), m_classes(other.m_classes), m_specificationClasses(other.m_specificationClasses), m_filterExpression(other.m_filterExpression), m_contentFlags(other.m_contentFlags),
    m_sortingFieldIndex(other.m_sortingFieldIndex), m_sortDirection(other.m_sortDirection), m_connectionId(other.m_connectionId), m_inputKeys(other.m_inputKeys),
    m_selectionInfo(other.m_selectionInfo), m_categories(other.m_categories), m_totalFieldsCount(other.m_totalFieldsCount), m_unitSystem(other.m_unitSystem),
    m_ruleset(other.m_ruleset), m_rulesetVariables(other.m_rulesetVariables)
    {
    for (Field const* field : other.m_fields)
        AddRootField(*field->Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::~ContentDescriptor()
    {
    for (Field* field : m_fields)
        delete field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::Equals(ContentDescriptorCR other) const
    {
    return ContentHelpers::AreDescriptorsEqual(*this, other, RulesetCompareOption::ByPointer);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentDescriptor::GetFieldIndex(Utf8CP name) const
    {
    for (size_t i = 0; i < m_fields.size(); ++i)
        {
        if (m_fields[i]->GetUniqueName().Equals(name))
            return (int)i;
        }
    return -1;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentDescriptor::Field*> ContentDescriptor::GetVisibleFields() const
    {
    bvector<Field*> fields;
    std::copy_if(m_fields.begin(), m_fields.end(), std::back_inserter(fields), [](Field const* f)
        {
        bool isHidden = f->IsDisplayLabelField();
        return !isHidden;
        });
    return fields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t CountFields(bvector<ContentDescriptor::Field*> const& fields)
    {
    size_t count = 0;
    for (auto const& field : fields)
        {
        count += 1;
        if (field->IsNestedContentField())
            count += CountFields(field->AsNestedContentField()->GetFields());
        }
    return count;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentDescriptor::GetTotalFieldsCount() const
    {
    if (m_totalFieldsCount.IsNull())
        m_totalFieldsCount = CountFields(m_fields);
    return m_totalFieldsCount.Value();
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field const* ContentDescriptor::GetDistinctField() const
    {
    if (OnlyDistinctValues() && GetVisibleFields().size() == 1)
        {
        Field const* field = GetVisibleFields()[0];
        if (field->IsPropertiesField() || field->IsCalculatedPropertyField())
            return field;
        }

    if (OnlyDistinctValues() && GetVisibleFields().size() == 0 && ShowLabels())
        return GetDisplayLabelField();

    return nullptr;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ContentDescriptor::Field const* FindField(bvector<ContentDescriptor::Field*> const& fields, IContentFieldMatcherCR matcher)
    {
    for (auto field : fields)
        {
        if (matcher.Matches(*field))
            return field;

        ContentDescriptor::Field const* nestedField = nullptr;
        if (field->IsNestedContentField() && nullptr != (nestedField = FindField(field->AsNestedContentField()->GetFields(), matcher)))
            return nestedField;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field const* ContentDescriptor::FindField(IContentFieldMatcherCR matcher) const
    {
    return ::FindField(m_fields, matcher);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertiesContentFieldMatcher::SelectToPropertyPathsMatch(RelatedClassPathCR path, ContentDescriptor::Field const& field)
    {
    size_t relationshipPathLength = 0;
    bvector<ContentDescriptor::RelatedContentField const*> ancestors;
    ContentDescriptor::NestedContentField const* curr = field.GetParent();
    while (curr)
        {
        if (curr->AsRelatedContentField())
            {
            ancestors.push_back(curr->AsRelatedContentField());
            relationshipPathLength += curr->AsRelatedContentField()->GetPathFromSelectToContentClass().size();
            }
        curr = curr->GetParent();
        }

    if (relationshipPathLength != path.size())
        return false;

    auto pathIter = path.begin();
    for (auto ancestorIter = ancestors.rbegin(); ancestorIter != ancestors.rend(); ++ancestorIter)
        {
        for (RelatedClassCR rel : (*ancestorIter)->GetPathFromSelectToContentClass())
            {
            bool classesMatch =
                rel.GetSourceClass()->Is((*pathIter).GetSourceClass())
                && &rel.GetTargetClass().GetClass() == &(*pathIter).GetTargetClass().GetClass()
                && &rel.GetRelationship().GetClass() == &(*pathIter).GetRelationship().GetClass();
            if (!classesMatch)
                return false;
            ++pathIter;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::RemoveRootField(bvector<Field*>::const_iterator const& iter)
    {
    if (m_fields.end() == iter)
        return false;

    if (*iter)
        {
        Field const& field = **iter;

        auto categoryUseCount = field.GetCategory().use_count();
        if (categoryUseCount && categoryUseCount <= 3)
            {
            // remove category if the removed field was the only user of the category - in that case `categoryUseCount = 3`:
            // 1 ref from the field, 1 ref from the descriptor categories list and 1 ref from `field.GetCategory()` used to call `use_count()`
            m_categories.erase(std::remove(m_categories.begin(), m_categories.end(), field.GetCategory()), m_categories.end());
            }

        if (field.IsPropertiesField())
            OnECPropertiesFieldRemoved(*field.AsPropertiesField());

        delete &field;
        }

    m_fields.erase(iter);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::RemoveRootField(std::function<bool(Field const&)> const& pred)
    {
    auto iter = std::find_if(m_fields.begin(), m_fields.end(), [&pred](Field const* f) {return pred(*f);});
    return RemoveRootField(iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::RemoveRootField(Field const& field) { return RemoveRootField([&field](Field const& f) {return &f == &field;});}


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ExclusivelyIncludeFields(bvector<ContentDescriptor::Field const*> const& includeFields)
    {
    bvector<ContentDescriptor::Field*> rootFields;
    bmap<ContentDescriptor::Field const*, ContentDescriptor::NestedContentField*> clones;
    for (auto const& includeField : includeFields)
        {
        ContentDescriptor::Field* clone = nullptr;
        ContentDescriptor::Field const* curr = includeField;
        while (curr)
            {
            auto prev = clone;
            auto cloneIter = clones.find(curr);
            if (clones.end() != cloneIter)
                {
                // `curr` is already cloned, which means the fields hierarchy from root the `curr` is already
                // built - we only need to add `prev` and that's it
                if (prev)
                    {
                    prev->SetParent(cloneIter->second->AsNestedContentField());
                    cloneIter->second->GetFields().push_back(prev);
                    clone = nullptr;
                    }
                break;
                }

            // `curr` is not cloned yet
            clone = curr->Clone();
            if (clone->IsNestedContentField())
                {
                clones.Insert(curr, clone->AsNestedContentField());
                clone->AsNestedContentField()->ClearFields();
                if (prev)
                    {
                    prev->SetParent(clone->AsNestedContentField());
                    clone->AsNestedContentField()->GetFields().push_back(prev);
                    }
                }
            curr = curr->GetParent();
            }
        if (clone != nullptr)
            rootFields.push_back(clone);
        }

    ClearFields();
    for (auto const& field : rootFields)
        AddRootField(*field);

    UpdateSelectClasses();
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ExcludeFields(bvector<ContentDescriptor::Field const*> const& excludeFields)
    {
    bmap<ContentDescriptor::Field const*, ContentDescriptor::Field*> rootReplacements;
    bmap<ContentDescriptor::NestedContentField const*, ContentDescriptor::NestedContentField*> clones;
    for (auto const& excludeField : excludeFields)
        {
        // source field => cloned field or nullptr if source field is removed
        bpair<ContentDescriptor::Field const*, ContentDescriptor::Field*> curr(excludeField, nullptr);
        ContentDescriptor::NestedContentField const* parent = excludeField->GetParent();
        while (parent)
            {
            auto cloneIter = clones.find(parent);
            if (clones.end() == cloneIter)
                {
                // if not done yet, clone the parent
                auto parentClone = parent->Clone()->AsNestedContentField();
                cloneIter = clones.Insert(parent, parentClone).first;
                }
            auto& parentClone = *cloneIter->second;

            // either remove or replace curr in its parent field
            auto pred = [&curr](auto const& f){ return f.GetUniqueName().Equals(curr.first->GetUniqueName()); };
            if (curr.second && !curr.second->AsNestedContentField()->GetFields().empty())
                parentClone.ReplaceField(pred, *curr.second);
            else
                parentClone.RemoveField(pred);

            curr = make_bpair(parent, &parentClone);
            parent = parent->GetParent();
            }
        rootReplacements[curr.first] = curr.second;
        }

    if (rootReplacements.empty())
        return false;

    for (auto const& entry : rootReplacements)
        {
        auto iter = std::find(m_fields.begin(), m_fields.end(), entry.first);
        if (m_fields.end() == iter)
            continue;

        if (entry.second == nullptr)
            {
            // we don't have a replacement, so just remove the field from descriptor
            RemoveRootField(*entry.first);
            }
        else if (entry.second->IsNestedContentField() && entry.second->AsNestedContentField()->GetFields().empty())
            {
            // we have a replacement, but it's empty - remove the field from descriptor and delete the clone
            delete entry.second;
            RemoveRootField(*entry.first);
            }
        else
            {
            // we have a valid replacement - delete the field that's stored in the descriptor and replace it
            // with the clone
            delete entry.first;
            *iter = entry.second;
            }
        }
    UpdateSelectClasses();
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::ClearFields()
    {
    for (Field* field : m_fields)
        delete field;
    m_fields.clear();
    m_fieldsMap.clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddSelectClass(bvector<SelectClassInfo>& targetClasses, bmap<Utf8String, bvector<size_t>>& targetSpecificationClassIndexes,
    SelectClassInfo selectClass, bvector<Utf8String> const& contentSpecificationHashes)
    {
    targetClasses.push_back(selectClass);

    for (auto const& contentSpecificationHash : contentSpecificationHashes)
        {
        auto specIter = targetSpecificationClassIndexes.find(contentSpecificationHash);
        if (targetSpecificationClassIndexes.end() == specIter)
            specIter = targetSpecificationClassIndexes.Insert(contentSpecificationHash, bvector<size_t>()).first;
        specIter->second.push_back(targetClasses.size() - 1);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::AddSelectClass(SelectClassInfo selectClass, Utf8StringCR contentSpecificationHash)
    {
    ::AddSelectClass(m_classes, m_specificationClasses, selectClass, { contentSpecificationHash });
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassInfo const*> ContentDescriptor::GetSelectClasses(Utf8StringCR specificationHash) const
    {
    bvector<SelectClassInfo const*> result;
    auto iter = m_specificationClasses.find(specificationHash);
    if (m_specificationClasses.end() == iter)
        return result;

    for (auto index : iter->second)
        {
        if (index >= m_classes.size())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Index out of bounds in specification classes map. Looking for: %" PRIu64 ", total classes: %" PRIu64, index, m_classes.size()));
        result.push_back(&m_classes[index]);
        }
    return result;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::UpdateSelectClasses()
    {
    bvector<SelectClassInfo> classes;
    bmap<Utf8String, bvector<size_t>> specificationClassIndexes;
    for (size_t i = 0; i < m_classes.size(); ++i)
        {
        auto const& selectClass = m_classes[i];
        bool hasRootField = false;
        for (auto const& field : m_fields)
            {
            bool fieldMatchesClass = true;
            if (field->IsPropertiesField())
                {
                fieldMatchesClass = ContainerHelpers::Contains(field->AsPropertiesField()->GetProperties(),
                    [&selectClass](auto const& prop){return prop.GetPropertyClass().Is(&selectClass.GetSelectClass().GetClass());});
                }
            else if (field->IsNestedContentField() && field->AsNestedContentField()->AsCompositeContentField())
                {
                fieldMatchesClass = field->AsNestedContentField()->AsCompositeContentField()->GetContentClass().Is(&selectClass.GetSelectClass().GetClass());
                }
            else if (field->IsNestedContentField() && field->AsNestedContentField()->AsRelatedContentField())
                {
                fieldMatchesClass = field->AsNestedContentField()->AsRelatedContentField()->GetPathFromSelectToContentClass().front().GetSourceClass()->Is(&selectClass.GetSelectClass().GetClass());
                }
            if (fieldMatchesClass)
                {
                hasRootField = true;
                break;
                }
            }
        if (hasRootField)
            {
            bvector<Utf8String> contentSpecificationHashes;
            for (auto const& entry : m_specificationClasses)
                {
                if (ContainerHelpers::Contains(entry.second, i))
                    contentSpecificationHashes.push_back(entry.first);
                }
            ::AddSelectClass(classes, specificationClassIndexes, selectClass, contentSpecificationHashes);
            }
        }
    m_classes = classes;
    m_specificationClasses = specificationClassIndexes;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::MergeWith(ContentDescriptorCR other)
    {
    if (!m_preferredDisplayType.Equals(other.m_preferredDisplayType))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different display types: '%s' vs '%s'", m_preferredDisplayType.c_str(), other.m_preferredDisplayType.c_str()));

    if (m_contentFlags != other.m_contentFlags)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different content flags: %d vs %d", m_contentFlags, other.m_contentFlags));

    if (m_sortDirection != other.m_sortDirection)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different sort directions: %d vs %d", m_sortDirection, other.m_sortDirection));

    if (!m_filterExpression.Equals(other.m_filterExpression))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different filter expressions: '%s' vs '%s'", m_filterExpression.c_str(), other.m_filterExpression.c_str()));

    if (!m_connectionId.Equals(other.m_connectionId))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different connection IDs: '%s' vs '%s'", m_connectionId.c_str(), other.m_connectionId.c_str()));

    if (m_unitSystem != other.m_unitSystem)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different unit systems: %d vs %d", (int)m_unitSystem, (int)other.m_unitSystem));

    if (m_ruleset != other.m_ruleset)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different rulesets: %s vs %s", m_ruleset->GetRuleSetId().c_str(), other.m_ruleset->GetRuleSetId().c_str()));

    if (m_rulesetVariables != other.m_rulesetVariables)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge descriptors with different ruleset variables: %s vs %s", m_rulesetVariables.GetSerializedInternalJsonObjectString().c_str(), other.m_rulesetVariables.GetSerializedInternalJsonObjectString().c_str()));

    if (m_selectionInfo.IsNull() && other.m_selectionInfo.IsValid() || m_selectionInfo.IsValid() && other.m_selectionInfo.IsNull() || m_selectionInfo.IsValid() && other.m_selectionInfo.IsValid() && *m_selectionInfo != *other.m_selectionInfo)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Attempting to merge descriptors with different selection infos");

    std::unordered_map<size_t, size_t> selectClassesRemap; // index in other.m_classes => index in this->m_classes
    for (size_t i = 0; i < other.m_classes.size(); ++i)
        {
        SelectClassInfo const& sourceClassInfo = other.m_classes[i];
        auto iter = std::find(m_classes.begin(), m_classes.end(), sourceClassInfo);
        if (m_classes.end() != iter)
            {
            selectClassesRemap[i] = iter - m_classes.begin();
            }
        else
            {
            m_classes.push_back(sourceClassInfo);
            selectClassesRemap[i] = m_classes.size() - 1;
            }
        }
    for (auto const& otherEntry : other.m_specificationClasses)
        {
        Utf8StringCR specHash = otherEntry.first;
        auto specIter = m_specificationClasses.find(specHash);
        if (m_specificationClasses.end() == specIter)
            specIter = m_specificationClasses.Insert(specHash, bvector<size_t>()).first;
        for (size_t otherIndex : otherEntry.second)
            {
            auto thisIndexIter = selectClassesRemap.find(otherIndex);
            if (selectClassesRemap.end() == thisIndexIter)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Class index not found for specification %s", specHash.c_str()));

            specIter->second.push_back(thisIndexIter->second);
            }
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

    for (std::shared_ptr<ContentDescriptor::Category const> const& sourceCategory : other.m_categories)
        {
        if (!ContainerHelpers::Contains(m_categories, [&](auto const& c){return c->GetName().Equals(sourceCategory->GetName());}))
            m_categories.push_back(sourceCategory);
        }

    for (Field const* sourceField : other.m_fields)
        {
        bool found = false;
        if (sourceField->IsPropertiesField())
            {
            ECPropertiesField* targetField = FindECPropertiesField(sourceField->AsPropertiesField()->GetProperties().front(), sourceField->GetLabel(),
                sourceField->GetCategory().get(), sourceField->GetRenderer(), sourceField->GetEditor());
            if (nullptr != targetField)
                {
                found = true;
                for (ContentDescriptor::Property const& sourceProperty : sourceField->AsPropertiesField()->GetProperties())
                    targetField->AddProperty(sourceProperty);
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
            AddRootField(*sourceField->Clone());
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::HasContentFlag(ContentFlags flag) const
    {
    return 0 != ((int)flag & m_contentFlags);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::AddContentFlag(ContentFlags flag)
    {
    if (0 != ((int)flag & m_contentFlags))
        return;

    m_contentFlags |= (int)flag;
    OnFlagAdded(flag);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::RemoveContentFlag(ContentFlags flag)
    {
    if (0 == ((int)flag & m_contentFlags))
        return;

    m_contentFlags &= ~(int)flag;
    OnFlagRemoved(flag);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnFlagAdded(ContentFlags flag) {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnFlagRemoved(ContentFlags flag) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field& ContentDescriptor::AddRootField(Field& field)
    {
    field.SetParent(nullptr);
    m_fields.push_back(&field);
    if (nullptr != field.AsPropertiesField())
        OnECPropertiesFieldAdded(*field.AsPropertiesField());
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ECPropertiesField* ContentDescriptor::FindECPropertiesField(ECPropertyCR prop, ECClassCR propClass,
    Utf8StringCR fieldLabel, ContentDescriptor::Category const* category, ContentFieldRenderer const* renderer, ContentFieldEditor const* editor)
    {
    auto iter = m_fieldsMap.find(ECPropertiesFieldKey(prop, propClass, fieldLabel, category ? category->GetName() : "", renderer, editor));
    if (m_fieldsMap.end() == iter)
        return nullptr;

    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnECPropertiesFieldRemoved(ECPropertiesField const& field)
    {
    Property const& prop = field.GetProperties().front();
    Utf8String categoryName = field.GetCategory() ? field.GetCategory()->GetName() : "";
    m_fieldsMap.erase(ECPropertiesFieldKey(prop, field.GetLabel(), categoryName, field.GetRenderer(), field.GetEditor()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentDescriptor::OnECPropertiesFieldAdded(ECPropertiesField& field)
    {
    Property const& prop = field.GetProperties().front();
    Utf8String categoryName = field.GetCategory() ? field.GetCategory()->GetName() : "";
    m_fieldsMap[ECPropertiesFieldKey(prop, field.GetLabel(), categoryName, field.GetRenderer(), field.GetEditor())] = &field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx(GetUnitSystem(), nullptr);
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::Category::Equals(ContentDescriptor::Category const& other, bool compareParents, bool compareChildren) const
    {
    return m_name == other.m_name
        && m_label == other.m_label
        && m_priority == other.m_priority
        && m_description == other.m_description
        && m_shouldExpand == other.m_shouldExpand
        && (!compareParents || m_parentCategory.expired() == other.m_parentCategory.expired() && (m_parentCategory.expired() || m_parentCategory.lock()->Equals(*other.m_parentCategory.lock())))
        && (!compareChildren || std::equal(m_childCategories.begin(), m_childCategories.end(), other.m_childCategories.begin(), other.m_childCategories.end(),
            [](auto const& lhs, auto const& rhs){return lhs->Equals(*rhs, false);}));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Category::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Category::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::Property::operator==(Property const& other) const
    {
    return m_propertyClass == other.m_propertyClass
        && m_property == other.m_property
        && m_prefix.Equals(other.m_prefix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Property::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Property::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid property access token '%s' for property '%s.%s'",
                accessString.c_str(), structProperty.GetClass().GetFullName(), structProperty.GetName().c_str()));
            break;
            }
        ECStructClassCR structClass = ecProperty->GetAsStructProperty()->GetType();
        ecProperty = structClass.GetPropertyP(accessorSplits[i].c_str());
        }
    if (nullptr == ecProperty || !ecProperty->GetIsPrimitive())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Access token '%s' for property '%s.%s' did not result in a valid primitive property",
            accessString.c_str(), structProperty.GetClass().GetFullName(), structProperty.GetName().c_str()));
        return nullptr;
        }
    return ecProperty->GetAsPrimitiveProperty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldRenderer* ContentFieldRenderer::FromSpec(CustomRendererSpecificationCR spec)
    {
    return new ContentFieldRenderer(spec.GetRendererName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentFieldRenderer::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentFieldRenderer::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentFieldRenderer::operator<(ContentFieldRenderer const& other) const
    {
    return m_name < other.m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentFieldEditor::Params::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor::~ContentFieldEditor()
    {
    for (Params const* params : m_params)
        delete params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentFieldEditor::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentFieldEditor::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor& ContentFieldEditor::operator=(ContentFieldEditor const& other)
    {
    m_name = other.m_name;
    for (Params const* otherParams : other.m_params)
        m_params.push_back(otherParams->Clone());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentFieldEditor& ContentFieldEditor::operator=(ContentFieldEditor&& other)
    {
    m_name = std::move(other.m_name);
    m_params.swap(other.m_params);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::TypeDescription::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::Field::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescription const& ContentDescriptor::Field::GetTypeDescription() const
    {
    if (m_type.IsNull())
        m_type = _CreateTypeDescription();
    return *m_type;
    }

const Utf8CP ContentDescriptor::DisplayLabelField::NAME = "/DisplayLabel/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::DisplayLabelField::~DisplayLabelField()
    {
    for (auto const& entry : m_labelOverrideSpecs)
        {
        for (auto classSpec : entry.second)
            delete classSpec;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::DisplayLabelField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("string");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::DisplayLabelField::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<ECClassCP, bvector<InstanceLabelOverride const*>> ContentDescriptor::DisplayLabelField::CloneLabelOverrideValueSpecs(bmap<ECClassCP, bvector<InstanceLabelOverride const*>> const& specs)
    {
    bmap<ECClassCP, bvector<InstanceLabelOverride const*>> result;
    for (auto const& entry : specs)
        {
        bvector<InstanceLabelOverride const*> classSpecs;
        for (auto classSpec : entry.second)
            classSpecs.push_back(new InstanceLabelOverride(*classSpec));
        result.Insert(entry.first, classSpecs);
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::CalculatedPropertyField::_CreateTypeDescription() const
    {
    return new PrimitiveTypeDescription("string");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::CalculatedPropertyField::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::ECPropertiesField::ECPropertiesField(std::shared_ptr<Category const> category, Property const& prop)
    {
    SetCategory(category);
    SetLabel(prop.GetProperty().GetDisplayLabel());
    m_properties.push_back(prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::ECPropertiesField::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::ECPropertiesField::_CreateTypeDescription() const
    {
    return TypeDescription::Create(m_properties.front().GetProperty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::IsCompositePropertiesField() const
    {
    if (m_properties.empty())
        return false;

    ECPropertyCR prop = m_properties.front().GetProperty();
    return prop.GetIsStruct() || prop.GetIsArray();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::ContainsProperty(ECPropertyCR prop) const
    {
    return m_properties.end() != std::find_if(m_properties.begin(), m_properties.end(), [&](ContentDescriptor::Property const& propertyInfo)
        {
        return &propertyInfo.GetProperty() == &prop;
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::_Equals(Field const& other) const
    {
    if (!Field::_Equals(other) || !other.IsPropertiesField())
        return false;

    if (m_properties.empty() || other.AsPropertiesField()->GetProperties().empty())
        return true;

    // if both lists have elements, just compare the first one
    ECPropertiesFieldKey thisKey(m_properties[0], GetLabel(), GetCategory() ? GetCategory()->GetName() : "", GetRenderer(), GetEditor());
    ECPropertiesFieldKey otherKey(other.AsPropertiesField()->GetProperties()[0], other.GetLabel(), other.GetCategory() ? other.GetCategory()->GetName() : "", other.GetRenderer(), other.GetEditor());
    return !(thisKey < otherKey) && !(otherKey < thisKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesField::_IsReadOnly() const
    {
    if (m_isReadOnly.IsValid())
        return m_isReadOnly.Value();

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentDescriptor::Property const*> const& ContentDescriptor::ECPropertiesField::FindMatchingProperties(ECClassCP targetClass) const
    {
    static bvector<ContentDescriptor::Property const*> const s_empty;
    if (m_properties.empty())
        return s_empty;

    BeMutexHolder lock(m_matchingPropertiesCacheMutex);
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
                if (targetClass->Is(&prop.GetPropertyClass()))
                    matchingProperties.push_back(&prop);
                }
            }
        iter = m_matchingPropertiesCache.insert(std::make_pair(targetClass, matchingProperties)).first;
        }

    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentDescriptor::ECPropertiesField::_GetPriority() const
    {
    if (m_priority.IsValid())
        return m_priority.Value();

    int maxPriority = 0;
    for (Property prop : m_properties)
        {
        if (prop.GetPriority() > maxPriority)
            maxPriority = prop.GetPriority();
        }

    return maxPriority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentDescriptor::ECPropertiesField::_CreateName() const
    {
    Utf8String name;
    bvector<Utf8String> propertyClassNames;

    bset<ECClassCP> usedRelatedClasses;
    bset<ECClassCP> usedPropertyClasses;

    for (ContentDescriptor::Property const& prop : GetProperties())
        {
        if (usedPropertyClasses.end() == usedPropertyClasses.find(&prop.GetProperty().GetClass()))
            {
            propertyClassNames.push_back(CreateClassNameForDescriptor(prop.GetProperty().GetClass()));
            usedPropertyClasses.insert(&prop.GetProperty().GetClass());
            }
        }

    if (!propertyClassNames.empty())
        name.append("pc_").append(BeStringUtilities::Join(propertyClassNames, "_")).append("_");

    name.append(GetProperties().front().GetProperty().GetName());
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Field::TypeDescriptionPtr ContentDescriptor::NestedContentField::_CreateTypeDescription() const
    {
    return new NestedContentTypeDescription(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::NestedContentField::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentDescriptor::NestedContentField::_CreateName() const
    {
    Utf8String name("ncc_");
    name.append(CreateClassNameForDescriptor(GetContentClass()));
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::CompositeContentField::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::RelatedContentField::_Equals(Field const& other) const
    {
    if (!NestedContentField::_Equals(other) || nullptr == other.AsNestedContentField()->AsRelatedContentField())
        return false;

    RelatedContentField const* otherRelatedContentField = other.AsNestedContentField()->AsRelatedContentField();
    return m_pathFromSelectClassToContentClass == otherRelatedContentField->m_pathFromSelectClassToContentClass &&
        m_isRelationshipField == otherRelatedContentField->m_isRelationshipField;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentDescriptor::RelatedContentField::_AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentDescriptor::RelatedContentField::CreateRelationshipName() const
    {
    Utf8String name("ncc_");
    name.append(CreateClassNameForDescriptor(GetRelationshipClass()));
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ContentDescriptor::RelatedContentField::_CreateName() const
    {
    Utf8String name;
    if (!m_pathFromSelectClassToContentClass.empty())
        name.append("rc_");
    for (RelatedClassCR rel : m_pathFromSelectClassToContentClass)
        {
        name.append(CreateClassNameForDescriptor(*rel.GetSourceClass()));
        name.append("_");
        }
    name.append(m_isRelationshipField ? CreateRelationshipName() : NestedContentField::_CreateName());
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsMultiplicityMoreThanOne(RelationshipMultiplicityCR multiplicity)
    {
    return multiplicity.IsUpperLimitUnbounded() || multiplicity.GetUpperLimit() > 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::RelatedContentField::IsXToMany() const
    {
    for (auto const& rel : m_pathFromSelectClassToContentClass)
        {
        if (!rel.GetRelationship().IsValid())
            continue;

        if (rel.IsForwardRelationship() && IsMultiplicityMoreThanOne(rel.GetRelationship().GetClass().GetTarget().GetMultiplicity()))
            return true;
        if (!rel.IsForwardRelationship() && IsMultiplicityMoreThanOne(rel.GetRelationship().GetClass().GetSource().GetMultiplicity()))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentDescriptor::ECPropertiesFieldKey::operator<(ECPropertiesFieldKey const& rhs) const
    {
    if (GetRenderer() != rhs.GetRenderer())
        PTR_VALUE_LESS_COMPARE(GetRenderer(), rhs.GetRenderer());

    if (GetEditor() != rhs.GetEditor())
        PTR_VALUE_LESS_COMPARE(GetEditor(), rhs.GetEditor());

    NUMERIC_LESS_COMPARE(GetValueKind(), rhs.GetValueKind());
    STR_LESS_COMPARE(GetType(), rhs.GetType());
    STR_LESS_COMPARE(GetName(), rhs.GetName());
    STR_LESS_COMPARE(GetLabel(), rhs.GetLabel());
    STR_LESS_COMPARE(GetCategoryName().c_str(), rhs.GetCategoryName().c_str());
    NUMERIC_LESS_COMPARE(GetKoq(), rhs.GetKoq());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSetItem::IsMerged(Utf8StringCR fieldName) const
    {
    return (m_mergedFieldNames.end() != std::find(m_mergedFieldNames.begin(), m_mergedFieldNames.end(), fieldName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassInstanceKey> const& ContentSetItem::GetPropertyValueKeys(FieldPropertyIdentifier const& fp) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSetItem::AddUsersExtendedData(Utf8CP key, ECValueCR value)
    {
    m_extendedData.AddMember(rapidjson::Value(key, m_extendedData.GetAllocator()), ValueHelpers::GetJsonFromECValue(value, &m_extendedData.GetAllocator()), m_extendedData.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentSetItem::AsJson(ECPresentationSerializerContextR ctx, int flags, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, flags, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentSetItem::AsJson(int flags, rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, flags, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document Content::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document Content::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DisplayValueGroup::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document DisplayValueGroup::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult ECInstanceChangeResult::Success(ECValue changedValue)
    {
    ECInstanceChangeResult result(SUCCESS);
    result.m_changedValue = changedValue;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult ECInstanceChangeResult::Error(Utf8String message)
    {
    ECInstanceChangeResult result(ERROR);
    result.m_errorMessage = message;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult ECInstanceChangeResult::Ignore(Utf8String reason)
    {
    ECInstanceChangeResult result(SUCCESS);
    result.m_errorMessage = reason;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceChangeResult::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ECInstanceChangeResult::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SelectionInfo::operator==(SelectionInfo const& other) const
    {
    return m_isSubSelection == other.m_isSubSelection
        && m_timestamp == other.m_timestamp
        && m_selectionProviderName == other.m_selectionProviderName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Formatting::Format const* DefaultPropertyFormatter::_GetActiveFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystem) const
    {
    return ValueHelpers::GetPresentationFormat(koq, unitSystem, m_defaultFormats);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyEnumFormatting(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
    {
    return ValueHelpers::GetEnumPropertyDisplayValue(formattedValue, ecProperty, ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyKoqFormatting(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue, ECPresentation::UnitSystem unitSystemGroup) const
    {
    KindOfQuantityCP koq = ecProperty.GetKindOfQuantity();
    if (nullptr == koq)
        return ERROR;

    // currently only doubles are supported
    if (!ecValue.IsDouble())
        return ERROR;

    // determine the presentation unit
    Formatting::Format const* format = GetActiveFormat(*koq, unitSystemGroup);
    if (nullptr == format || nullptr == format->GetCompositeMajorUnit())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Content, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Failed to format property '%s.%s' value as active format does not have a composite major unit",
            ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str()));
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyDoubleFormatting(Utf8StringR formattedValue, double value) const
    {
    formattedValue.Sprintf("%.2f", value);
    if (formattedValue.Equals("-0.00"))
        formattedValue = "0.00";
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_ApplyDateTimeFormatting(Utf8StringR formattedValue, DateTimeCR dt) const
    {
    formattedValue = dt.ToString();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue, ECPresentation::UnitSystem unitSystem) const
    {
    if (ecValue.IsNull())
        {
        formattedValue.clear();
        return SUCCESS;
        }

    if (SUCCESS == _ApplyEnumFormatting(formattedValue, ecProperty, ecValue))
        return SUCCESS;

    if (SUCCESS == _ApplyKoqFormatting(formattedValue, ecProperty, ecValue, unitSystem))
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DefaultPropertyFormatter::_GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR ecClass, RelatedClassPath const& relationshipPath, RelationshipMeaning relationshipMeaning) const
    {
    formattedLabel = ecProperty.GetDisplayLabel();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ContentDescriptor::Category> DefaultCategorySupplier::_CreateDefaultCategory() const
    {
    return std::make_unique<ContentDescriptor::Category>("/selected-item/",
        CommonStrings::ECPRESENTATION_LABEL_SELECTEDITEMS,
        CommonStrings::ECPRESENTATION_DESCRIPTION_SELECTEDITEMS,
        1000);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ContentDescriptor::Category> DefaultCategorySupplier::_CreateECClassCategory(ECClassCR ecClass) const
    {
    return std::make_unique<ContentDescriptor::Category>(ecClass.GetName(), ecClass.GetDisplayLabel(), ecClass.GetDescription(), 1000);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ContentDescriptor::Category> DefaultCategorySupplier::_CreatePropertyCategory(ECPropertyCR ecProperty) const
    {
    PropertyCategoryCP propertyCategory = ecProperty.GetCategory();
    if (nullptr != propertyCategory)
        {
        return std::make_unique<ContentDescriptor::Category>(propertyCategory->GetName(), propertyCategory->GetDisplayLabel(),
            propertyCategory->GetDescription(), propertyCategory->GetPriority());
        }
    return nullptr;
    }
