/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "ECSchemaHelper.h"
#include "ECExpressions/ECExpressionContextsProvider.h"
#include "Queries/PresentationQuery.h"
#include "Queries/QueryBuilderHelpers.h"
#include "Queries/CustomFunctions.h"
#include "Queries/QueryExecutor.h"
#include <set>
#include <regex>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCacheDeprecated::Key::Key(ECSchemaHelper::RelationshipClassPathOptionsDeprecated const& options)
    {
    m_sourceClass = &options.m_sourceClass;
    m_relationshipDirection = options.m_relationshipDirection;
    m_depth = options.m_depth;
    m_supportedSchemas = options.m_supportedSchemas;
    m_supportedRelationships = options.m_supportedRelationships;
    m_supportedClasses = options.m_supportedClasses;
    m_targetClass = options.m_targetClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPathsCacheDeprecated::Key::operator<(Key const& other) const
    {
    NUMERIC_LESS_COMPARE(m_sourceClass, other.m_sourceClass);
    NUMERIC_LESS_COMPARE(m_targetClass, other.m_targetClass);
    NUMERIC_LESS_COMPARE(m_relationshipDirection, other.m_relationshipDirection);
    NUMERIC_LESS_COMPARE(m_depth, other.m_depth);
    STR_LESS_COMPARE(m_supportedClasses.c_str(), other.m_supportedClasses.c_str());
    STR_LESS_COMPARE(m_supportedRelationships.c_str(), other.m_supportedRelationships.c_str());
    STR_LESS_COMPARE(m_supportedSchemas.c_str(), other.m_supportedSchemas.c_str());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCache::Key::Key(Key const& other)
    : m_sourceClass(other.m_sourceClass), m_pathSpecification(other.m_pathSpecification), m_mergePolymorphicPaths(other.m_mergePolymorphicPaths)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCache::Key::Key(ECSchemaHelper::MultiRelationshipPathOptions const& options)
    : m_sourceClass(&options.m_sourceClass), m_mergePolymorphicPaths(options.m_mergePolymorphicPaths)
    {
    for (RelationshipStepSpecification const* step : options.m_path.GetSteps())
        {
        m_pathSpecification.AddStep(*new RepeatableRelationshipStepSpecification(step->GetRelationshipClassName(),
            step->GetRelationDirection(), step->GetTargetClassName()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCache::Key::Key(ECSchemaHelper::RepeatableMultiRelationshipPathOptions const& options)
    : m_sourceClass(&options.m_sourceClass), m_pathSpecification(options.m_path), m_mergePolymorphicPaths(options.m_mergePolymorphicPaths)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPathsCache::Key::operator<(Key const& other) const
    {
    NUMERIC_LESS_COMPARE(m_mergePolymorphicPaths, other.m_mergePolymorphicPaths);
    NUMERIC_LESS_COMPARE(m_sourceClass, other.m_sourceClass);
    STR_LESS_COMPARE(m_pathSpecification.GetHash().c_str(), other.m_pathSpecification.GetHash().c_str());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::ECSchemaHelper(IConnectionCR connection, RelatedPathsCache* relatedPathsCache, ECExpressionsCache* ecexpressionsCache)
    : m_connection(connection),
    m_relatedPathsCache(relatedPathsCache), m_ownsRelatedPathsCache(nullptr == relatedPathsCache),
    m_ecexpressionsCache(ecexpressionsCache), m_ownsECExpressionsCache(nullptr == ecexpressionsCache)
    {
    if (nullptr == m_relatedPathsCache)
        m_relatedPathsCache = new RelatedPathsCache();
    if (nullptr == m_ecexpressionsCache)
        m_ecexpressionsCache = new ECExpressionsCache();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::~ECSchemaHelper()
    {
    if (m_ownsRelatedPathsCache)
        DELETE_AND_CLEAR(m_relatedPathsCache);
    if (m_ownsECExpressionsCache)
        DELETE_AND_CLEAR(m_ecexpressionsCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchemaHelper::GetSchema(Utf8CP schemaName, bool fullyLoad) const
    {
    return m_connection.GetECDb().Schemas().GetSchema(schemaName, fullyLoad);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaHelper::GetECClass(Utf8CP schemaName, Utf8CP className, bool isFullSchemaName) const
    {
    Utf8String schemaNameStr = schemaName;
    if (isFullSchemaName)
        {
        uint32_t versionMajor, versionMinor;
        ECSchema::ParseSchemaFullName(schemaNameStr, versionMajor, versionMinor, schemaName);
        }
    return m_connection.GetECDb().Schemas().GetClass(schemaNameStr, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaHelper::GetECClass(Utf8CP fullClassName) const
    {
    Utf8String schemaName, className;
    ECClass::ParseClassName(schemaName, className, fullClassName);
    return GetECClass(schemaName.c_str(), className.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaHelper::GetECClass(ECClassId id) const
    {
    return m_connection.GetECDb().Schemas().GetClass(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> ECSchemaHelper::GetECClassesByName(Utf8CP name) const
    {
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetECClassesByName");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    static Utf8CP statementStr = "SELECT ECInstanceId FROM [meta].[ECClassDef] WHERE Name = ?";
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), statementStr);
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", statementStr));

    stmt->BindText(1, name, IECSqlBinder::MakeCopy::No);

    bvector<ECClassId> ids;
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        ids.push_back(stmt->GetValueId<ECClassId>(0));

    bvector<ECClassCP> classes;
    for (ECClassId id : ids)
        classes.push_back(m_connection.GetECDb().Schemas().GetClass(id));
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsSchemaHidden(ECSchemaCR ecSchema)
    {
    IECInstancePtr options = ecSchema.GetCustomAttribute("CoreCustomAttributes", "HiddenSchema");
    if (options.IsNull())
        return false;

    ECValue value;
    if (ECObjectsStatus::Success == options->GetValue(value, "ShowClasses") && !value.IsNull() && value.IsBoolean() && true == value.GetBoolean())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsClassHidden(ECClassCR ecClass, bool checkBaseClasses = true)
    {
    IECInstancePtr options = checkBaseClasses ? ecClass.GetCustomAttribute("CoreCustomAttributes", "HiddenClass") : ecClass.GetCustomAttributeLocal("CoreCustomAttributes", "HiddenClass");
    if (options.IsNull())
        return false;

    ECValue value;
    if (ECObjectsStatus::Success == options->GetValue(value, "Show") && !value.IsNull() && value.IsBoolean() && true == value.GetBoolean())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* The goal of this method is to check is any of the classes are hidden in a hierarchy
* between the checked class and any of the requested classes
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TECClass>
static bool IsClassHidden(TECClass const& checkedClass, std::function<bool(TECClass const&)> const& requestedBaseClassMatcher)
    {
    // don't care about custom attributes if this class was specifically requested
    if (requestedBaseClassMatcher(checkedClass))
        return false;

    // check custom attribute
    if (IsClassHidden(checkedClass, false))
        return true;

    // note: GetBaseClasses returns only direct base classes (not the whole base class hierarchy)
    for (ECClassCP baseClass : checkedClass.GetBaseClasses())
        {
        if (IsClassHidden(*static_cast<TECClass const*>(baseClass), requestedBaseClassMatcher))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsAllowed(ECSchemaCR schema)
    {
    // WIP: temporary workaround until TFS#782547 is implemented
    static const std::set<Utf8String> s_ignoredSchemas = {
        "ECDbMap",
        "ECDbSchemaPolicies",
        "ECDbSystem",
        };
    if (s_ignoredSchemas.end() != s_ignoredSchemas.find(schema.GetName()))
        return false;

    if (schema.IsStandardSchema())
        return false;

    if (schema.IsSystemSchema())
        return false;

    if (schema.GetSupplementalInfo().IsValid())
        return false;

    if (IsSchemaHidden(schema))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::ParseECSchemas(ECSchemaSet& schemas, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const
    {
    schemas.clear();
    bvector<Utf8String> schemaNames;
    ParseECSchemaNames(schemaNames, exclude, commaSeparatedSchemaList);
    for (Utf8StringCR name : schemaNames)
        {
        ECSchemaCP schema = GetSchema(name.c_str(), false);
        if (nullptr == schema)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Requested ECSchema does not exist: '%s'", name.c_str()));
            continue;
            }
        schemas.insert(schema);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::ParseECSchemaNames(bvector<Utf8String>& names, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const
    {
    names.clear();
    exclude = false;

    if (commaSeparatedSchemaList.empty())
        return;

    auto begin = commaSeparatedSchemaList.begin(),
         end = commaSeparatedSchemaList.begin();

    if (commaSeparatedSchemaList.length() > 2 && commaSeparatedSchemaList[0] == 'E' && commaSeparatedSchemaList[1] == ':')
        {
        exclude = true;
        begin += 2;
        end += 2;
        }

    while (commaSeparatedSchemaList.end() != end)
        {
        if (*end == ',')
            {
            names.push_back(Utf8String(begin, end));

            auto next = end + 1;
            if (commaSeparatedSchemaList.end() != next && ' ' == *next)
                begin = end = next + 1;
            else
                begin = end = next;
            }
        end++;
        }
    if (end != begin)
        names.push_back(Utf8String(begin, end));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedClassInfos ECSchemaHelper::GetECClassesFromClassList(Utf8StringCR classListStr, bool supportExclusion) const
    {
    SupportedClassesParser parser(*this, classListStr, supportExclusion);
    return parser.GetClassInfos();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedClassInfos ECSchemaHelper::GetECClassesFromClassList(bvector<MultiSchemaClass*> const& classList, bool areExcluded) const
    {
    auto formFlags = [](bool areExcluded, bool isPolymorphic)
        {
        int flag = 0;

        if (isPolymorphic)
            flag |= CLASS_SELECTION_FLAG_Polymorphic;

        if (areExcluded)
            flag |= CLASS_SELECTION_FLAG_Exclude;
        else
            flag |= CLASS_SELECTION_FLAG_Include;

        return flag;
        };

    SupportedClassInfos classes;
    for (MultiSchemaClass const* schemaClass : classList)
        {
        for (Utf8String const& className : schemaClass->GetClassNames())
            {
            auto ecClass = GetECClass(schemaClass->GetSchemaName().c_str(), className.c_str(), false);
            if (ecClass == nullptr)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Given ECClass `%s.%s` does not exist. Ignoring.",
                    schemaClass->GetSchemaName().c_str(), className.c_str()));
                continue;
                }
            classes.insert(SupportedClassInfo<ECClass>(*ecClass, formFlags(areExcluded, schemaClass->GetArePolymorphic())));
            }
        }
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedRelationshipClassInfos ECSchemaHelper::GetECRelationshipClasses(Utf8StringCR relationshipListStr) const
    {
    SupportedClassesParser parser(*this, relationshipListStr, false);
    SupportedClassInfos classInfos = parser.GetClassInfos();
    SupportedRelationshipClassInfos relationshipClasses;
    for (SupportedClassInfo<ECClass> const& info : classInfos)
        {
        if (info.GetClass().IsRelationshipClass())
            relationshipClasses.insert(SupportedRelationshipClassInfo(*info.GetClass().GetRelationshipClassCP(), info.GetFlags()));
        }
    return relationshipClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TClass>
static Utf8String CreateClassListString(bset<SupportedClassInfo<TClass>> const& classes)
    {
    bmap<ECSchemaCP, bvector<ECClass const*>> classesBySchema;
    for (auto const& c : classes)
        classesBySchema[&c.GetClass().GetSchema()].push_back(&c.GetClass());

    Utf8String spec;
    for (auto const& entry : classesBySchema)
        {
        spec.append(entry.first->GetName()).append(":");
        for (size_t i = 0; i < entry.second.size(); ++i)
            {
            if (i > 0)
                spec.append(",");
            spec.append(entry.second[i]->GetName());
            }
        spec.append(";");
        }
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSchemaHelper::CreateClassListString(SupportedClassInfos const& classes) const
    {
    return ::CreateClassListString(classes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSchemaHelper::CreateClassListString(SupportedRelationshipClassInfos const& classes) const
    {
    return ::CreateClassListString(classes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassSet ECSchemaHelper::GetECClasses(ECSchemaSet const& schemas) const
    {
    // get all supported classes from the schemas set
    bset<ECEntityClassCP> allSupportedClasses;
    for (ECSchemaCP schema : schemas)
        {
        if (nullptr == schema)
            continue;

        ECSchemaCP loadedSchema = GetConnection().GetECDb().Schemas().GetSchema(schema->GetName(), true);
        ECClassContainerCR classContainer = loadedSchema->GetClasses();
        for (auto iter = classContainer.begin(); iter != classContainer.end(); ++iter)
            {
            if (!(*iter)->IsEntityClass())
                continue;

            if (IsClassHidden(**iter))
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Skipping hidden ECClass: '%s'", (*iter)->GetFullName()));
                continue;
                }

            allSupportedClasses.insert((*iter)->GetEntityClassCP());
            }
        }

    // smallen the list of classes by excluding all subclasses
    ECClassSet classes;
    bset<ECEntityClassCP> parentClasses;
    for (ECEntityClassCP ecClass : allSupportedClasses)
        {
        bool isAlreadyIncludedPolymorphically = false;
        for (ECEntityClassCP parentClass : parentClasses)
            {
            if (ecClass->Is(parentClass))
                {
                isAlreadyIncludedPolymorphically = true;
                break;
                }
            }
        if (isAlreadyIncludedPolymorphically)
            continue;

        bool containsAllDerivedClasses = true;
        ECDerivedClassesList const& derivedClasses = ecClass->GetDerivedClasses();
        for (ECClassCP derivedClass : derivedClasses)
            {
            if (!derivedClass->IsEntityClass())
                continue;

            if (IsClassHidden(*derivedClass))
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Skipping hidden ECClass: '%s'", derivedClass->GetFullName()));
                continue;
                }

            if (allSupportedClasses.end() == allSupportedClasses.find(derivedClass->GetEntityClassCP()))
                {
                containsAllDerivedClasses = false;
                break;
                }
            }
        if (containsAllDerivedClasses)
            {
            auto iter = parentClasses.begin();
            while (parentClasses.end() != iter)
                {
                ECEntityClassCP parentClass = *iter;
                if (parentClass->Is(ecClass))
                    {
                    iter = parentClasses.erase(iter);
                    classes.erase(parentClass);
                    }
                else
                    iter++;
                }

            parentClasses.insert(ecClass);
            }
        classes[ecClass] = containsAllDerivedClasses;
        }

    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassSet ECSchemaHelper::GetECClassesFromSchemaList(Utf8StringCR schemaListStr) const
    {
    return GetECClasses(GetECSchemas(schemaListStr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void CollectDerivedECClassesRecursively(bvector<ECClassCP>& result, ECClassCP baseClass, SchemaManagerCR schemas)
    {
    for (ECClassCP ecClass : schemas.GetDerivedClasses(*baseClass))
        {
        result.push_back(ecClass);
        CollectDerivedECClassesRecursively(result, ecClass, schemas);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> ECSchemaHelper::GetDerivedECClassesRecursively(ECClassCP baseClass) const
    {
    bvector<ECClassCP> result;
    CollectDerivedECClassesRecursively(result, baseClass, m_connection.GetECDb().Schemas());
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaHelper::AreSchemasSupported(Utf8StringCR schemaListStr) const
    {
    bvector<Utf8String> schemaNames;
    bool exclude;
    ParseECSchemaNames(schemaNames, exclude, schemaListStr);

    if (exclude)
        return true;

    for (Utf8StringCR name : schemaNames)
        {
        if (nullptr == m_connection.GetECDb().Schemas().GetSchema(name, false))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaSet ECSchemaHelper::GetECSchemas(Utf8StringCR supportedSchemasStr) const
    {
    ECSchemaSet schemas;
    bool exclude = false;
    ParseECSchemas(schemas, exclude, supportedSchemasStr);

    // if there are schemas specified and they're not excluded, return immediately
    if (!exclude && !schemas.empty())
        return schemas;

    // no schemas means all schemas (except some specific ones)
    bvector<ECSchemaCP> allSchemas = m_connection.GetECDb().Schemas().GetSchemas(false);

    ECSchemaSet allSupportedSchemas;
    for (ECSchemaCP schema : allSchemas)
        {
        if (IsAllowed(*schema))
            allSupportedSchemas.insert(schema);
        }

    // if the schemas are not excluded, return them
    if (!exclude)
        return allSupportedSchemas;

    // handle exclusion
    ECSchemaSet supportedSchemasNoExcluded;
    for (ECSchemaCP schema : allSupportedSchemas)
        {
        if (schemas.end() == schemas.find(schema))
            supportedSchemasNoExcluded.insert(schema);
        }
    return supportedSchemasNoExcluded;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECSchemaHelper::SupportedClassesResolver
{
    enum Flags
        {
        DeterminedSchemaIds = 1 << 0,
        DeterminedIncludedEntityClasses = 1 << 1,
        DeterminedExcludedEntityClasses = 1 << 2,
        DeterminedRelationshipClasses = 1 << 3,
        DeterminedPolymorphicallyIncludedClasses = 1 << 4,
        DeterminedPolymorphicallyExcludedClasses = 1 << 5,
        };

private:
    mutable int m_flags;
    mutable ECSchemaSet m_schemaList;
    mutable IdSet<ECSchemaId> m_schemaIds;
    mutable IdSet<ECClassId> m_includedEntityClassIds;
    mutable IdSet<ECClassId> m_polymorphicallyIncludedClassIds;
    mutable bvector<ECClassCP> m_excludedClasses;
    mutable bvector<ECClassCP> m_polymorphicallyExcludedClasses;
    mutable IdSet<ECClassId> m_relationshipClassIds;
    SupportedClassInfos const* m_classInfos;
    SupportedRelationshipClassInfos const* m_relationshipInfos;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void AddClassId(IdSet<ECClassId>& list, ECClassCR ecClass) {list.insert(ecClass.GetId());}

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    SupportedClassesResolver(ECSchemaSet const& supportedSchemas,
        SupportedClassInfos const* classInfos, SupportedRelationshipClassInfos const* relationshipInfos)
        : m_flags(0)
        {
        m_schemaList = supportedSchemas;
        m_classInfos = classInfos;
        m_relationshipInfos = relationshipInfos;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool GetAcceptAllIncludeClasses() const {return GetIncludedEntityClassIds().empty() && nullptr == m_classInfos;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool GetAcceptAllRelationships() const {return GetRelationshipClassIds().empty() && nullptr == m_relationshipInfos;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    SupportedClassInfo<ECClass> const* GetInfo(ECClassCR ecClass) const
        {
        if (nullptr == m_classInfos)
            return nullptr;
        auto iter = m_classInfos->find(SupportedClassInfo<ECClass>(ecClass));
        if (m_classInfos->end() != iter)
            return &*iter;
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECSchemaId> const& GetSchemaIds() const
        {
        if (0 == (m_flags & DeterminedSchemaIds))
            {
            for (ECSchemaCP schema : m_schemaList)
                m_schemaIds.insert(schema->GetId());
            m_flags |= DeterminedSchemaIds;
            }
        return m_schemaIds;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> const& GetIncludedEntityClassIds() const
        {
        if (0 == (m_flags & DeterminedIncludedEntityClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (auto const& info : *m_classInfos)
                    {
                    if (info.IsInclude())
                        m_includedEntityClassIds.insert(info.GetClass().GetId());
                    }
                }
            m_flags |= DeterminedIncludedEntityClasses;
            }
        return m_includedEntityClassIds;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> const& GetRelationshipClassIds() const
        {
        if (0 == (m_flags & DeterminedRelationshipClasses))
            {
            if (nullptr != m_relationshipInfos)
                {
                for (SupportedRelationshipClassInfo const& info : *m_relationshipInfos)
                    {
                    if (info.IsInclude())
                        m_relationshipClassIds.insert(info.GetClass().GetId());
                    }
                }
            m_flags |= DeterminedRelationshipClasses;
            }
        return m_relationshipClassIds;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ECClassCP> const& GetExcludedClasses() const
        {
        if (0 == (m_flags & DeterminedExcludedEntityClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (auto const& info : *m_classInfos)
                    {
                    if (info.IsExclude())
                        {
                        // note: we're not going to have a class that's both included, excluded and
                        // polymorphic. if that's the case - we want to include it polymorphically
                        // and exclude non-polymorphically
                        if (!info.IsPolymorphic() || info.IsInclude())
                            m_excludedClasses.push_back(&info.GetClass());
                        }
                    }
                }
            m_flags |= DeterminedExcludedEntityClasses;
            }
        return m_excludedClasses;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ECClassCP> const& GetPolymorphicallyExcludedClasses() const
        {
        if (0 == (m_flags & DeterminedPolymorphicallyExcludedClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (auto const& info : *m_classInfos)
                    {
                    // see the comment in `GetExcludedClasses` for more details on this check
                    if (!info.IsInclude() && info.IsExclude() && info.IsPolymorphic())
                        m_polymorphicallyExcludedClasses.push_back(&info.GetClass());
                    }
                }
            m_flags |= DeterminedPolymorphicallyExcludedClasses;
            }
        return m_polymorphicallyExcludedClasses;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassVirtualSet : VirtualSet
{
private:
    bmap<ECClassId, ECClassCP> m_map;
public:
    ECClassVirtualSet() {}
    explicit ECClassVirtualSet(ECClassCR ecClass) {Add(ecClass);}
    bool _IsInSet(int nVals, DbValue const* vals) const override
        {
        DbValue const& val = vals[0];
        return m_map.end() != m_map.find(val.GetValueId<ECClassId>());
        }
    void Add(ECClassCR ecClass) {m_map[ecClass.GetId()] = &ecClass;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeBindings(BoundQueryValuesList& target, BoundQueryValuesList&& source)
    {
    std::move(source.begin(), source.end(), std::back_inserter(target));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static QueryClauseAndBindings CreateRelationshipPathsQuery(ECSchemaHelper::SupportedClassesResolver const& resolver,
    bset<ECClassId> const& sourceClassIds, int relationshipDirection, bool applyTargetFilters, bool wantActualTargets)
    {
    BoundQueryValuesList bindings;
    Utf8String query = ""
        "SELECT [source].[ECInstanceId] sourceClassId, "
        "       [relationship].[ECInstanceId] relationshipClassId, "
        "       [target].[ECInstanceId] actualRelatedClassId, "
        "       [sourceConstraint].[RelationshipEnd] relationshipEnd "

        "  FROM [meta].[ECClassDef] [source], [meta].[ECClassDef] [target] "

        "  JOIN [meta].[ClassHasAllBaseClasses] sourceBaseRel ON [sourceBaseRel].[SourceECInstanceId] = [source].[ECInstanceId] "
        "  JOIN [meta].[ECClassDef] sourceBase ON [sourceBase].[ECInstanceId] = [sourceBaseRel].[TargetECInstanceId] "

        "  JOIN [meta].[ClassHasAllBaseClasses] targetBaseRel ON [targetBaseRel].[SourceECInstanceId] = [target].[ECInstanceId] "
        "  JOIN [meta].[ECClassDef] targetBase ON [targetBase].[ECInstanceId] = [targetBaseRel].[TargetECInstanceId] "

        "  JOIN [meta].[RelationshipConstraintHasClasses] sourceConstraintRel "
        "       ON     [sourceConstraint].[IsPolymorphic] AND [sourceConstraintRel].[TargetECInstanceId] = [sourceBase].[ECInstanceId] "
        "       OR NOT [sourceConstraint].[IsPolymorphic] AND [sourceConstraintRel].[TargetECInstanceId] = [source].[ECInstanceId] AND [source].[ECInstanceId] = [sourceBase].[ECInstanceId] "
        "  JOIN [meta].[ECRelationshipConstraintDef] sourceConstraint ON [sourceConstraint].[ECInstanceId] = [sourceConstraintRel].[SourceECInstanceId] "

        "  JOIN [meta].[RelationshipConstraintHasClasses] targetConstraintRel "
        "       ON     [targetConstraint].[IsPolymorphic] AND [targetConstraintRel].[TargetECInstanceId] = [targetBase].[ECInstanceId] "
        "       OR NOT [targetConstraint].[IsPolymorphic] AND [targetConstraintRel].[TargetECInstanceId] = [targetBase].[ECInstanceId] AND [target].[ECInstanceId] = [targetBase].[ECInstanceId] "
        "  JOIN [meta].[ECRelationshipConstraintDef] targetConstraint ON [targetConstraint].[ECInstanceId] = [targetConstraintRel].[SourceECInstanceId] AND [targetConstraint].[RelationshipEnd] <> [sourceConstraint].[RelationshipEnd] "

        "  JOIN [meta].[ECClassDef] relationship ON [relationship].[ECInstanceId] = [sourceConstraint].[RelationshipClass].[Id] "
        "       AND [relationship].[ECInstanceId] = [targetConstraint].[RelationshipClass].[Id] "

        " WHERE ";

    bool hasCondition = false;

    if (applyTargetFilters)
        {
        if (!resolver.GetAcceptAllRelationships())
            {
            if (hasCondition)
                query.append(" AND ");
            ValuesFilteringHelper helper(resolver.GetRelationshipClassIds());
            query.append(helper.CreateWhereClause("[relationship].[ECInstanceId]"));
            MergeBindings(bindings, helper.CreateBoundValues());
            hasCondition = true;
            }

        if (!resolver.GetAcceptAllIncludeClasses())
            {
            if (hasCondition)
                query.append(" AND ");

            ValuesFilteringHelper helper(resolver.GetIncludedEntityClassIds());
            if (wantActualTargets)
                {
                // take all classes that are equal to or derive from given target class
                query.append("[target].[ECInstanceId] IN (SELECT SourceECInstanceId FROM meta.ClassHasAllBaseClasses WHERE ")
                    .append(helper.CreateWhereClause("TargetECInstanceId"))
                    .append(")");
                }
            else
                {
                query.append(helper.CreateWhereClause("[target].[ECInstanceId]"));
                }
            MergeBindings(bindings, helper.CreateBoundValues());
            hasCondition = true;
            }
        else if (!wantActualTargets)
            {
            if (hasCondition)
                query.append(" AND ");
            query.append("[target].[ECInstanceId] = [targetBase].[ECInstanceId]");
            hasCondition = true;
            }
        }

    if (hasCondition)
        query.append(" AND ");

    ValuesFilteringHelper sourceIdsHelper(sourceClassIds);
    query.append(sourceIdsHelper.CreateWhereClause("[source].[ECInstanceId]"));
    MergeBindings(bindings, sourceIdsHelper.CreateBoundValues());

    if (((int)ECRelatedInstanceDirection::Backward | (int)ECRelatedInstanceDirection::Forward) != relationshipDirection)
        {
        ECRelationshipEnd relationshipEnd = (ECRelatedInstanceDirection::Backward == (ECRelatedInstanceDirection)relationshipDirection)
            ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
        query.append(" AND [sourceConstraint].[RelationshipEnd] = ?");
        bindings.push_back(std::make_unique<BoundQueryECValue>(ECValue((int)relationshipEnd)));
        }

    ValuesFilteringHelper schemaIdsHelper(resolver.GetSchemaIds());
    if (resolver.GetAcceptAllRelationships() && !resolver.GetSchemaIds().empty())
        {
        query.append(" AND ");
        query.append(schemaIdsHelper.CreateWhereClause("[relationship].[Schema].[Id]"));
        MergeBindings(bindings, schemaIdsHelper.CreateBoundValues());
        }
    if (resolver.GetAcceptAllIncludeClasses() && !resolver.GetSchemaIds().empty())
        {
        query.append(" AND ");
        query.append(schemaIdsHelper.CreateWhereClause("[target].[Schema].[Id]"));
        MergeBindings(bindings, schemaIdsHelper.CreateBoundValues());
        }

    query.append(" ORDER BY [source].[ECInstanceId], [target].[ECInstanceId], [relationship].[ECInstanceId]");

    return QueryClauseAndBindings(query, bindings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoesPathContainAnotherPath(RelatedClassPath const& containingPath, RelatedClassPath const& candidatePath, bool handleTargetClassPolymorphically)
    {
    for (size_t i = 0; i < containingPath.size(); ++i)
        {
        if (!candidatePath[i].GetRelationship().GetClass().Is(&containingPath[i].GetRelationship().GetClass())
            || containingPath[i].IsForwardRelationship() != candidatePath[i].IsForwardRelationship())
            {
            return false;
            }

        if (!candidatePath[i].GetSourceClass()->Is(containingPath[i].GetSourceClass()))
            return false;

        if (handleTargetClassPolymorphically)
            {
            if (!containingPath[i].GetTargetClass().IsSelectPolymorphic() && &candidatePath[i].GetTargetClass().GetClass() != &containingPath[i].GetTargetClass().GetClass()
                || containingPath[i].GetTargetClass().IsSelectPolymorphic() && !candidatePath[i].GetTargetClass().GetClass().Is(&containingPath[i].GetTargetClass().GetClass()))
                {
                return false;
                }
            }
        else
            {
            if (&candidatePath[i].GetTargetClass().GetClass() != &containingPath[i].GetTargetClass().GetClass())
                return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPath(bvector<RelatedClassPath>& paths, RelatedClassPath const& path, bool handleTargetClassPolymorphically)
    {
    if (paths.size() > 0)
        {
        size_t frontIndex = 0;
        while (frontIndex < paths.size())
            {
            size_t backIndex = paths.size() - frontIndex - 1;
            RelatedClassPath const& includedPath = paths[backIndex];

            // if path lengths are different, they can't be similar
            if (includedPath.size() != path.size())
                {
                ++frontIndex;
                continue;
                }

            // the new path is already included polymorphically by another path
            if (DoesPathContainAnotherPath(includedPath, path, handleTargetClassPolymorphically))
                return;

            // the new path includes another (already included) path polymorphically -
            // the included path must be removed
            if (DoesPathContainAnotherPath(path, includedPath, handleTargetClassPolymorphically))
                {
                paths.erase(paths.begin() + backIndex);
                continue;
                }

            ++frontIndex;
            }
        }

    // did not find any similar paths - simply append the new path
    paths.push_back(path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MatchesPathRequest(RelatedClass const& related, ECSchemaHelper::SupportedClassesResolver const& resolver)
    {
    IdSet<ECClassId> const& classIds = resolver.GetIncludedEntityClassIds();
    if (!resolver.GetAcceptAllIncludeClasses() && !classIds.Contains(related.GetTargetClass().GetClass().GetId()))
        {
        // if specific classes are requested, make sure the candidate is in that list
        return false;
        }
    if (resolver.GetAcceptAllIncludeClasses() && !resolver.GetSchemaIds().Contains(related.GetTargetClass().GetClass().GetSchema().GetId()))
        {
        // if any class is okay, make sure the candidate class is from supported schemas list
        return false;
        }

    IdSet<ECClassId> const& relationshipIds = resolver.GetRelationshipClassIds();
    if (!resolver.GetAcceptAllRelationships() && !relationshipIds.Contains(related.GetRelationship().GetClass().GetId()))
        {
        // is specific relationships are requested, make sure the candidate is in that list
        return false;
        }
    if (resolver.GetAcceptAllRelationships() && !resolver.GetSchemaIds().Contains(related.GetRelationship().GetClass().GetSchema().GetId()))
        {
        // if any relationship is okay, make sure the candidate relationship is from supported schemas list
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRelationshipAlias(ECClassCR relationship, ECClassUseCounter& counter)
    {
    return RULES_ENGINE_RELATED_CLASS_ALIAS(relationship, counter.Inc(&relationship));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateTargetClassAlias(ECClassCR targetClass, ECClassUseCounter& counter)
    {
    return RULES_ENGINE_RELATED_CLASS_ALIAS(targetClass, counter.Inc(&targetClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AcceptECClass(ECClassCP& ecClass, ECClassCP targetClass, ECSchemaHelper::SupportedClassesResolver const& resolver)
    {
    if (targetClass && targetClass->Is(ecClass))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Changing found ECClass '%s' to more specific requested target class '%s'", ecClass->GetFullName(), targetClass->GetFullName()));
        ecClass = targetClass;
        return true;
        }
    if (targetClass && !ecClass->Is(targetClass))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Skipping ECClass '%s' as it's not a requested target class '%s'", ecClass->GetFullName(), targetClass->GetFullName()));
        return false;
        }
    if (IsClassHidden<ECClass>(*ecClass, [&](ECClassCR c){return resolver.GetInfo(c) != nullptr;}))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Skipping hidden ECClass: '%s'", ecClass->GetFullName()));
        return false;
        }
    DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Using ECClass: '%s'", ecClass->GetFullName()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @deprecated In favor of the version with RelationshipPathSpecification. Left only to support deprecated cases.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::GetPathsDeprecated(bvector<RelatedClassPath>& paths, ECClassUseCounter& relationshipsUseCounter,
    bset<RelatedClass, RelatedClass::ClassPointersComparer>& usedRelationships, SupportedClassesResolver const& resolver, bset<ECClassId> const& sourceClassIds,
    bool handleRelatedClassesPolymorphically, int relationshipDirection, int depth, ECClassCP targetClass) const
    {
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetPaths");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    QueryClauseAndBindings clause = CreateRelationshipPathsQuery(resolver, sourceClassIds, relationshipDirection, depth <= 0, !handleRelatedClassesPolymorphically);
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), clause.GetClause().c_str());
    if (!stmt.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", clause.GetClause().c_str()));

    if (SUCCESS != clause.GetBindings().Bind(*stmt))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to bind statement parameters");

    bvector<RelatedClass> specs;
    bset<ECClassId> relatedClassIds;

    // get the results
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        {
        ECEntityClassCP source = GetECClass(stmt->GetValueId<ECClassId>(0))->GetEntityClassCP();
        ECRelationshipClassCP relationship = GetECClass(stmt->GetValueId<ECClassId>(1))->GetRelationshipClassCP();
        ECClassCP actualRelated = GetECClass(stmt->GetValueId<ECClassId>(2));
        if (nullptr == source || nullptr == relationship || nullptr == actualRelated)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to find either source, relationship or target class");

        bool isForward = (ECRelationshipEnd_Source == (ECRelationshipEnd)stmt->GetValueInt(3));

        // filter by target class (if specified)
        if (!AcceptECClass(actualRelated, targetClass, resolver))
            continue;

        auto const* info = resolver.GetInfo(*actualRelated);
        bool isTargetPolymorphic = (nullptr == info || info->IsPolymorphic()) && handleRelatedClassesPolymorphically;
        RelatedClass relatedClassSpec(*source,
            SelectClass<ECRelationshipClass>(*relationship, CreateRelationshipAlias(*relationship, relationshipsUseCounter)), isForward,
            SelectClass<ECClass>(*actualRelated, CreateTargetClassAlias(*actualRelated, relationshipsUseCounter), isTargetPolymorphic));

        // avoid recursive relationship loops
        if (usedRelationships.end() != usedRelationships.find(relatedClassSpec))
            continue;

        bool matchesRecursiveRequest = (depth < 0 && MatchesPathRequest(relatedClassSpec, resolver));

        if (0 == depth || matchesRecursiveRequest)
            AppendPath(paths, {relatedClassSpec}, handleRelatedClassesPolymorphically);

        if (depth > 0 || matchesRecursiveRequest)
            {
            specs.push_back(relatedClassSpec);
            usedRelationships.insert(relatedClassSpec);
            relatedClassIds.insert(actualRelated->GetId());
            }
        }

    txn.Cancel();

    if (0 != depth && !specs.empty())
        {
        bvector<RelatedClassPath> subPaths;
        GetPathsDeprecated(subPaths, relationshipsUseCounter, usedRelationships, resolver, relatedClassIds,
            handleRelatedClassesPolymorphically, relationshipDirection, depth - 1, targetClass);
        for (RelatedClassPath const& subPath : subPaths)
            {
            for (RelatedClassCR sourceClassSpec : specs)
                {
                if (&sourceClassSpec.GetTargetClass().GetClass() != subPath[0].GetSourceClass())
                    continue;

                AppendPath(paths, RelatedClassPath::Combine({sourceClassSpec}, subPath), handleRelatedClassesPolymorphically);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyExcludedClasses(bvector<RelatedClassPath>& paths, bvector<ECClassCP> const& excludedClasses, bool excludePolymorphically, SchemaManagerCR schemas)
    {
    for (RelatedClassPathR path : paths)
        {
        RelatedClassR targetClass = path.back();
        for (ECClassCP excludedClass : excludedClasses)
            {
            if (excludedClass->Is(&targetClass.GetTargetClass().GetClass()))
                targetClass.GetTargetClass().GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*excludedClass, "", excludePolymorphically));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @deprecated In favor of the version with RelationshipPathSpecification. Left only to support deprecated cases.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetRelationshipClassPathsDeprecated(RelationshipClassPathOptionsDeprecated const& options) const
    {
    // look for cached results first
    BeMutexHolder lock(m_relatedPathsCache->GetMutex());
    RelatedPathsCacheDeprecated::Key key(options);
    RelatedPathsCacheDeprecated::Result const* cacheResult = m_relatedPathsCache->GetDeprecated().Get(key);
    if (nullptr != cacheResult)
        {
        options.m_relationshipsUseCounter.Merge(cacheResult->m_relationshipCounter);
        return cacheResult->m_paths;
        }
    lock.unlock();

    bvector<RelatedClassPath> paths;
    SupportedClassInfos classInfos = GetECClassesFromClassList(options.m_supportedClasses, true);
    SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(options.m_supportedRelationships);
    SupportedClassesResolver resolver(GetECSchemas(options.m_supportedSchemas),
        (classInfos.empty() && 0 == *options.m_supportedClasses) ? nullptr : &classInfos,
        (relationshipInfos.empty() && 0 == *options.m_supportedRelationships) ? nullptr : &relationshipInfos);

    bset<RelatedClass, RelatedClass::ClassPointersComparer> usedRelationships;
    bset<ECClassId> sourceClassIds;
    sourceClassIds.insert(options.m_sourceClass.GetId());

    GetPathsDeprecated(paths, options.m_relationshipsUseCounter, usedRelationships, resolver, sourceClassIds,
        options.m_handleRelatedClassesPolymorphically, options.m_relationshipDirection, options.m_depth, options.m_targetClass);

    ApplyExcludedClasses(paths, resolver.GetExcludedClasses(), false, m_connection.GetECDb().Schemas());
    ApplyExcludedClasses(paths, resolver.GetPolymorphicallyExcludedClasses(), true, m_connection.GetECDb().Schemas());

    // cache
    m_relatedPathsCache->GetDeprecated().Put(key, std::make_unique<RelatedPathsCacheDeprecated::Result>(paths, options.m_relationshipsUseCounter));
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetPaths(ECClassId sourceClassId, bvector<RepeatableRelationshipStepSpecification> pathSpecification,
    bset<RelatedClass>& usedRelationships, ECClassUseCounter& relationshipsUseCounter, bool handleRelatedClassesPolymorphically) const
    {
    bvector<RelatedClassPath> paths;
    if (pathSpecification.empty())
        return paths;

    RepeatableRelationshipStepSpecification stepSpecification = pathSpecification.front();
    pathSpecification.erase(pathSpecification.begin());

    SupportedClassInfos classInfos = GetECClassesFromClassList(stepSpecification.GetTargetClassName(), false);
    SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(stepSpecification.GetRelationshipClassName());
    SupportedClassesResolver resolver(ECSchemaSet(),
        (classInfos.empty() && stepSpecification.GetTargetClassName().empty()) ? nullptr : &classInfos,
        (relationshipInfos.empty() && stepSpecification.GetRelationshipClassName().empty()) ? nullptr : &relationshipInfos);
    ECClassCP targetClass = stepSpecification.GetTargetClassName().empty() ? nullptr : GetECClass(stepSpecification.GetTargetClassName().c_str());

    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetPaths");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    QueryClauseAndBindings clause = CreateRelationshipPathsQuery(resolver, ContainerHelpers::Create<bset<ECClassId>>(sourceClassId), stepSpecification.GetRelationDirection(),
        true, paths.empty() && !handleRelatedClassesPolymorphically);
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), clause.GetClause().c_str());
    if (!stmt.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", clause.GetClause().c_str()));

    clause.Bind(*stmt);

    // get the results
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        {
        ECClassCP source = GetECClass(stmt->GetValueId<ECClassId>(0));
        ECRelationshipClassCP relationship = GetECClass(stmt->GetValueId<ECClassId>(1))->GetRelationshipClassCP();
        ECClassCP actualRelated = GetECClass(stmt->GetValueId<ECClassId>(2));
        if (nullptr == source || nullptr == relationship || nullptr == actualRelated)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to find either source, relationship or target class");

        bool isForward = (ECRelationshipEnd_Source == (ECRelationshipEnd)stmt->GetValueInt(3));

        // filter by target class (if specified)
        if (!AcceptECClass(actualRelated, targetClass, resolver))
            continue;

        auto const* info = resolver.GetInfo(*actualRelated);
        bool isTargetPolymorphic = (nullptr == info || info->IsPolymorphic()) && handleRelatedClassesPolymorphically;
        RelatedClass relatedClassSpec(*source,
            SelectClass<ECRelationshipClass>(*relationship, CreateRelationshipAlias(*relationship, relationshipsUseCounter)), isForward,
            SelectClass<ECClass>(*actualRelated, CreateTargetClassAlias(*actualRelated, relationshipsUseCounter), isTargetPolymorphic));
        RelatedClassPath path{relatedClassSpec};

        if (stepSpecification.GetCount() > 1)
            {
            if (!actualRelated->Is(isForward ? relationship->GetSource().GetAbstractConstraint() : relationship->GetTarget().GetAbstractConstraint()))
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, Utf8PrintfString("Found related class '%s' which is neither target not source of relationship '%s'",
                    actualRelated->GetFullName(), relationship->GetFullName()));
                }
            else
                {
                for (int i = 1; i < stepSpecification.GetCount(); ++i)
                    {
                    RelatedClass repeatedStep(relatedClassSpec);
                    repeatedStep.SetSourceClass(*actualRelated);
                    repeatedStep.GetRelationship().SetAlias(CreateRelationshipAlias(*relationship, relationshipsUseCounter));
                    repeatedStep.GetTargetClass().SetAlias(CreateTargetClassAlias(*actualRelated, relationshipsUseCounter));
                    path.push_back(repeatedStep);
                    }
                }
            }

        if (pathSpecification.empty())
            {
            AppendPath(paths, path, handleRelatedClassesPolymorphically);
            }
        else
            {
            bvector<RelatedClassPath> subPaths = GetPaths(actualRelated->GetId(), pathSpecification, usedRelationships,
                relationshipsUseCounter, handleRelatedClassesPolymorphically);
            for (RelatedClassPath subPath : subPaths)
                AppendPath(paths, RelatedClassPath::Combine(path, subPath), handleRelatedClassesPolymorphically);
            }
        }

    txn.Cancel();
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TOptions>
bvector<RelatedClassPath> ECSchemaHelper::GetCachedRelationshipPaths(TOptions const& options, std::function<bvector<RelatedClassPath>()> const& handler) const
    {
    // look for cached results first
    BeMutexHolder lock(m_relatedPathsCache->GetMutex());
    RelatedPathsCache::Key key(options);
    RelatedPathsCache::Result const* cacheResult = m_relatedPathsCache->Get(key);
    if (nullptr != cacheResult)
        {
        options.m_relationshipsUseCounter.Merge(cacheResult->m_relationshipCounter);
        return cacheResult->m_paths;
        }
    lock.unlock();

    // calculate
    bvector<RelatedClassPath> paths = handler();

    // cache
    m_relatedPathsCache->Put(key, std::make_unique<RelatedPathsCache::Result>(paths, options.m_relationshipsUseCounter));
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetRelationshipClassPaths(MultiRelationshipPathOptions const& options) const
    {
    return GetCachedRelationshipPaths(options, [&]()
        {
        bset<RelatedClass> usedRelationships;
        auto steps = ContainerHelpers::TransformContainer<bvector<RepeatableRelationshipStepSpecification>>(options.m_path.GetSteps(),
            [](RelationshipStepSpecification const* spec) {return RepeatableRelationshipStepSpecification(spec->GetRelationshipClassName(), spec->GetRelationDirection(), spec->GetTargetClassName()); });
        return GetPaths(options.m_sourceClass.GetId(), steps, usedRelationships, options.m_relationshipsUseCounter, options.m_mergePolymorphicPaths);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetRelationshipClassPaths(RepeatableMultiRelationshipPathOptions const& options) const
    {
    return GetCachedRelationshipPaths(options, [&]()
        {
        bset<RelatedClass> usedRelationships;
        auto steps = ContainerHelpers::TransformContainer<bvector<RepeatableRelationshipStepSpecification>>(options.m_path.GetSteps(),
            [](RepeatableRelationshipStepSpecification const* spec) {return RepeatableRelationshipStepSpecification(*spec); });
        return GetPaths(options.m_sourceClass.GetId(), steps, usedRelationships, options.m_relationshipsUseCounter, options.m_mergePolymorphicPaths);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
enum class RelationshipsJoinIdType
    {
    ECClassId,
    ECInstanceId,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void JoinRelationships(ComplexGenericQueryR query, RelatedClassPath const& path, RelationshipsJoinIdType idType, Utf8StringP lastJoinAlias, bool* wasLastJoinForward)
    {
    static Utf8CP s_sourceECClassId = "SourceECClassId";
    static Utf8CP s_sourceECInstanceId = "SourceECInstanceId";
    static Utf8CP s_targetECClassId = "TargetECClassId";
    static Utf8CP s_targetECInstanceId = "TargetECInstanceId";

    Utf8CP sourceId = (idType == RelationshipsJoinIdType::ECClassId) ? s_sourceECClassId : s_sourceECInstanceId;
    Utf8CP targetId = (idType == RelationshipsJoinIdType::ECClassId) ? s_targetECClassId : s_targetECInstanceId;

    Utf8String prevJoinAlias;
    bool prevJoinWasForward = false;
    for (size_t i = 0; i < path.size(); ++i)
        {
        RelatedClassCR r = path[i];
        Utf8String joinAlias = Utf8String("r").append(std::to_string(i).c_str());
        if (i == 0)
            {
            query.From(SelectClass<ECClass>(r.GetRelationship().GetClass(), joinAlias, true));
            }
        else
            {
            Utf8PrintfString joinClause("[%s].[%s] = [%s].[%s]",
                joinAlias.c_str(), r.IsForwardRelationship() ? sourceId : targetId,
                prevJoinAlias.c_str(), prevJoinWasForward ? targetId : sourceId);
            query.Join(SelectClass<ECClass>(r.GetRelationship().GetClass(), joinAlias, true), QueryClauseAndBindings(joinClause), false);
            }
        prevJoinAlias = joinAlias;
        prevJoinWasForward = r.IsForwardRelationship();
        }
    if (nullptr != lastJoinAlias)
        lastJoinAlias->AssignOrClear(prevJoinAlias.c_str());
    if (nullptr != wasLastJoinForward)
        *wasLastJoinForward = prevJoinWasForward;
    }

#define TARGET_IDS_QUERY_FIELD_NAME_SourceId    "SourceId"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ComplexGenericQueryPtr CreateTargetIdsPartialQuery(bvector<RelatedClassPath> const& paths)
    {
    GenericQueryPtr nestedQuery;
    for (RelatedClassPath const& path : paths)
        {
        if (path.empty())
            continue;

        ComplexGenericQueryPtr query = ComplexGenericQuery::Create();

        Utf8String lastJoinAlias;
        JoinRelationships(*query, path, RelationshipsJoinIdType::ECClassId, &lastJoinAlias, nullptr);

        RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create();
        RefCountedPtr<PresentationQueryContractSimpleField> targetIdField = PresentationQueryContractSimpleField::Create("TargetId", path.back().IsForwardRelationship() ? "TargetECInstanceId" : "SourceECInstanceId");
        targetIdField->SetPrefixOverride(lastJoinAlias);
        contract->AddField(*targetIdField);
        contract->AddField(*PresentationQueryContractSimpleField::Create(TARGET_IDS_QUERY_FIELD_NAME_SourceId, path.front().IsForwardRelationship() ? "SourceECInstanceId" : "TargetECInstanceId"));
        query->SelectContract(*contract, "r0");
        QueryBuilderHelpers::SetOrUnion<GenericQuery>(nestedQuery, *query);
        }

    if (nestedQuery.IsNull())
        return nullptr;

    ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
    query->SelectAll();
    query->From(*nestedQuery);
    return query;
    }

typedef std::unordered_map<ECInstanceId, bvector<ECInstanceId>> T_ECInstanceIdsMap;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static T_ECInstanceIdsMap GetTargetIds(IConnectionCR connection, bvector<RelatedClassPath> const& paths, bvector<ECInstanceId> const& sourceIds)
    {
    T_ECInstanceIdsMap result;

    auto query = CreateTargetIdsPartialQuery(paths);
    if (query.IsNull())
        return result;

    query->Where(ValuesFilteringHelper(sourceIds).Create(TARGET_IDS_QUERY_FIELD_NAME_SourceId));

    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(),
        connection.GetDb(), query->ToString().c_str());
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", query->ToString().c_str()));

    query->BindValues(*stmt);

    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        {
        auto targetId = stmt->GetValueId<ECInstanceId>(0);
        auto sourceId = stmt->GetValueId<ECInstanceId>(1);
        auto iter = result.find(sourceId);
        if (result.end() == iter)
            iter = result.insert(std::make_pair(sourceId, bvector<ECInstanceId>())).first;
        iter->second.push_back(targetId);
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static T_ECInstanceIdsMap GetRecursiveTargetIds(IConnectionCR connection, bvector<RelatedClassPath> const& paths, bvector<ECInstanceId> const& sourceIds, bool wantGroupingByInputId)
    {
    RecursiveQueryInfo recursiveQueryInfo(paths);
    RecursiveQueriesHelper helper(connection, recursiveQueryInfo);
    if (wantGroupingByInputId)
        return helper.GetRecursiveTargetIdsGroupedBySourceId(sourceIds);

    return { std::make_pair(ECInstanceId(), helper.GetRecursiveTargetIds(sourceIds)) };
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InputKeyAndTargetClass
    {
    ECInstanceKey m_inputKey;
    ECClassCP m_targetClass;
    InputKeyAndTargetClass(ECInstanceKey inputKey, ECClassCR targetClass)
        : m_inputKey(inputKey), m_targetClass(&targetClass)
        {}
    bool operator==(InputKeyAndTargetClass const& other) const
        {
        return m_targetClass == other.m_targetClass && m_inputKey == other.m_inputKey;
        }

    struct Hasher
        {
        size_t operator()(InputKeyAndTargetClass const& value) const
            {
            return std::hash<ECInstanceKey>{}(value.m_inputKey) * 31 + std::hash<ECClassCP>{}(value.m_targetClass);
            }
        };
    };

typedef std::unordered_map<InputKeyAndTargetClass, bvector<ECInstanceId>, InputKeyAndTargetClass::Hasher> T_ECClassInstanceIds;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelationshipStepSource
{
private:
    std::pair<ECClassCP, bvector<ECInstanceId>> m_firstStepInput;
    T_ECClassInstanceIds m_aggregatedOutputs;
public:
    RelationshipStepSource(ECClassCR sourceClass, bvector<ECInstanceId> sourceIds) : m_firstStepInput(&sourceClass, sourceIds) {}
    RelationshipStepSource(T_ECClassInstanceIds source) : m_aggregatedOutputs(source) {}
    T_ECClassInstanceIds GetInputs() const
        {
        // if all previous steps where recursive it means that next step might be reached from begining of relationship path.
        // In this case first step input should be included in the input for next step
        auto copy = m_aggregatedOutputs;
        if (m_firstStepInput.first && !m_firstStepInput.second.empty())
            ContainerHelpers::Push(copy[InputKeyAndTargetClass(ECInstanceKey(), *m_firstStepInput.first)], m_firstStepInput.second);
        return copy;
        }
    T_ECClassInstanceIds const& GetOutputs() const {return m_aggregatedOutputs;}
    void AppendOutput(T_ECClassInstanceIds const& source)
        {
        for (auto const& entry : source)
            ContainerHelpers::Push(m_aggregatedOutputs[entry.first], entry.second);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static T_ECClassInstanceIds GetRelationshipStepTargets(RelationshipStepSource const& source, bool groupByInputKey, std::function<std::pair<ECClassCP, T_ECInstanceIdsMap>(ECClassCR, bvector<ECInstanceId> const&, bool)> getTargets)
    {
    T_ECClassInstanceIds targets;
    for (auto const& inputEntry : source.GetInputs())
        {
        bool wantInputKeyGrouping = groupByInputKey && !inputEntry.first.m_inputKey.IsValid();
        std::pair<ECClassCP, T_ECInstanceIdsMap> stepTarget = getTargets(*inputEntry.first.m_targetClass, inputEntry.second, wantInputKeyGrouping);
        if (nullptr == stepTarget.first)
            continue;

        if (wantInputKeyGrouping)
            {
            for (auto const& targetEntry : stepTarget.second)
                {
                InputKeyAndTargetClass key(ECInstanceKey(inputEntry.first.m_targetClass->GetId(), targetEntry.first), *stepTarget.first);
                ContainerHelpers::MovePush(targets[key], targetEntry.second);
                }
            }
        else
            {
            InputKeyAndTargetClass key(inputEntry.first.m_inputKey, *stepTarget.first);
            for (auto const& targetEntry : stepTarget.second)
                ContainerHelpers::MovePush(targets[key], targetEntry.second);
            }
        }
    return targets;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetRecursiveRelationshipClassPaths(ECSchemaHelper const& schemaHelper, ECClassCR inputClass, bvector<ECInstanceId> const& inputIds,
    bvector<RepeatableRelationshipStepSpecification*> const& pathSteps, ECClassUseCounter& relationshipsUseCount, bool mergePolymorphicPaths, bool groupByInputKey)
    {
    RelationshipStepSource source(inputClass, inputIds);
    bool hadRecursiveRelationships = false;
    RepeatableRelationshipPathSpecification intermediatePathSpec;
    for (RepeatableRelationshipStepSpecification const* stepSpec : pathSteps)
        {
        if (stepSpec->GetCount() == 0)
            {
            if (!intermediatePathSpec.GetSteps().empty())
                {
                // get targets and their ids for the intermediate path
                T_ECClassInstanceIds stepTargets = GetRelationshipStepTargets(source, groupByInputKey, [&](ECClassCR sourceClass, bvector<ECInstanceId> const& sourceIds, bool wantGroupingByInputId) {
                    ECSchemaHelper::RepeatableMultiRelationshipPathOptions intermediatePathOptions(sourceClass, intermediatePathSpec, true, relationshipsUseCount);
                    bvector<RelatedClassPath> intermediateSpecPaths = schemaHelper.GetRelationshipClassPaths(intermediatePathOptions);
                    if (intermediateSpecPaths.empty())
                        return std::pair<ECClassCP, T_ECInstanceIdsMap>();
                    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, (1 == intermediateSpecPaths.size()), Utf8PrintfString("Expected to find 0 or 1 intermediate specification path, actually found: %" PRIu64, (uint64_t)intermediateSpecPaths.size()));
                    ECClassCP targetClass = &intermediateSpecPaths.back().back().GetTargetClass().GetClass();
                    return std::make_pair(targetClass, GetTargetIds(schemaHelper.GetConnection(), intermediateSpecPaths, sourceIds));
                    });

                // no need to collect any results form steps before. Create new source consisting only of intermediate path target.
                source = RelationshipStepSource(stepTargets);
                }
            // recursively find target ids using the recursive path specification
            RelationshipPathSpecification recursiveStepSpec(*new RelationshipStepSpecification(*stepSpec));
            T_ECClassInstanceIds stepTargets = GetRelationshipStepTargets(source, groupByInputKey, [&](ECClassCR sourceClass, bvector<ECInstanceId> const& sourceIds, bool wantGroupingByInputId) {
                ECSchemaHelper::MultiRelationshipPathOptions recursiveStepOptions(sourceClass, recursiveStepSpec, true, relationshipsUseCount);
                bvector<RelatedClassPath> recursiveStepPaths = schemaHelper.GetRelationshipClassPaths(recursiveStepOptions);
                if (recursiveStepPaths.empty())
                    return std::pair<ECClassCP, T_ECInstanceIdsMap>();
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, (1 == recursiveStepPaths.size()), Utf8PrintfString("Expected to find 0 or 1 recursive step path, actually found: %" PRIu64, (uint64_t)recursiveStepPaths.size()));
                ECClassCP targetClass = &recursiveStepPaths.back().back().GetTargetClass().GetClass();
                return std::make_pair(targetClass, GetRecursiveTargetIds(schemaHelper.GetConnection(), recursiveStepPaths, sourceIds, wantGroupingByInputId));
                });

            // need to append this step's target ids in order to not lose results from previous steps
            source.AppendOutput(stepTargets);
            // clear intermediate path
            intermediatePathSpec.ClearSteps();
            hadRecursiveRelationships = true;
            }
        else
            {
            intermediatePathSpec.AddStep(*new RepeatableRelationshipStepSpecification(*stepSpec));
            }
        }

    // the last step was not recursive - create paths from all aggregated source classes to this step
    if (!intermediatePathSpec.GetSteps().empty())
        {
        bvector<RelatedClassPath> paths;
        for (auto const& inputEntry : source.GetInputs())
            {
            ECSchemaHelper::RepeatableMultiRelationshipPathOptions endingPathOptions(*inputEntry.first.m_targetClass, intermediatePathSpec, mergePolymorphicPaths, relationshipsUseCount);
            bvector<RelatedClassPath> pathsFromInput = schemaHelper.GetRelationshipClassPaths(endingPathOptions);
            if (pathsFromInput.empty())
                continue;

            if (!hadRecursiveRelationships)
                {
                ContainerHelpers::MovePush(paths, pathsFromInput);
                continue;
                }

            RelatedClass pathsPrefix(inputClass, SelectClass<ECClass>(*inputEntry.first.m_targetClass, "related"), inputEntry.second);
            for (RelatedClassPathR path : pathsFromInput)
                {
                path.insert(path.begin(), pathsPrefix);
                path.SetInputKey(inputEntry.first.m_inputKey);
                }
            ContainerHelpers::MovePush(paths, pathsFromInput);
            }
        return paths;
        }

    // the last step was recursive - need to collect all target classes from recursive steps before it
    bvector<RelatedClassPath> paths;
    for (auto const& outputEntry : source.GetOutputs())
        {
        if (outputEntry.second.empty())
            continue;

        RelatedClassPath path({ RelatedClass(inputClass, SelectClass<ECClass>(*outputEntry.first.m_targetClass, "related"), outputEntry.second) });
        path.SetInputKey(outputEntry.first.m_inputKey);
        paths.push_back(path);
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetRecursiveRelationshipClassPaths(ECClassCR sourceClass, bvector<ECInstanceId> const& sourceIds,
    bvector<RepeatableRelationshipPathSpecification*> const& relationshipPathSpecs, ECClassUseCounter& relationshipsUseCount,
    bool mergePolymorphicPaths, bool groupByInputKey) const
    {
    bvector<RelatedClassPath> paths;
    for (RepeatableRelationshipPathSpecification const* pathSpec : relationshipPathSpecs)
        {
        ContainerHelpers::Push(paths, ::GetRecursiveRelationshipClassPaths(*this, sourceClass, sourceIds,
            pathSpec->GetSteps(), relationshipsUseCount, mergePolymorphicPaths, groupByInputKey));
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList ECSchemaHelper::GetRelationshipConstraintClasses(ECRelationshipClassCR relationship, ECRelatedInstanceDirection direction, Utf8StringCR supportedSchemasStr) const
    {
    ECRelationshipConstraintClassList const* classes = nullptr;
    switch (direction)
        {
        case ECRelatedInstanceDirection::Backward:
            classes = &relationship.GetSource().GetConstraintClasses();
            break;
        case ECRelatedInstanceDirection::Forward:
            classes = &relationship.GetTarget().GetConstraintClasses();
            break;
        }
    if (nullptr == classes)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to determine relationship constraint for '%s' with direction %d", relationship.GetFullName(), (int)direction));

    ECSchemaSet supportedSchemas = GetECSchemas(supportedSchemasStr);
    ECRelationshipConstraintClassList supportedClasses;
    for (ECClassCP ecClass : *classes)
        {
        if (!ecClass->IsEntityClass() && !ecClass->IsRelationshipClass())
            continue;

        if (IsClassHidden(*ecClass))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Skipping hidden ECClass: '%s'", ecClass->GetFullName()));
            continue;
            }

        if (supportedSchemas.end() == supportedSchemas.find(&ecClass->GetSchema()))
            continue;

        supportedClasses.push_back(ecClass);
        }
    return supportedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCP GetClassFromRelationshipConstraint(ECRelationshipConstraint const& constraint)
    {
    // no idea which one to return when there're multiple constraint classes
    return constraint.GetConstraintClasses().front();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClass ECSchemaHelper::GetForeignKeyClass(ECPropertyCR prop) const
    {
    if (!prop.GetIsNavigation())
        return RelatedClass();

    NavigationECPropertyCR navigationProp = *prop.GetAsNavigationProperty();
    ECRelationshipClassCP relationship = navigationProp.GetRelationshipClass();
    bool isRelationshipForward = (navigationProp.GetDirection() == ECRelatedInstanceDirection::Forward);
    ECRelationshipConstraintCR sourceConstraint = isRelationshipForward ? relationship->GetSource() : relationship->GetTarget();
    ECRelationshipConstraintCR targetConstraint = isRelationshipForward ? relationship->GetTarget() : relationship->GetSource();
    bool isTargetClassOptional = (targetConstraint.GetMultiplicity().GetLowerLimit() == 0);
    ECClassCP sourceClass = GetClassFromRelationshipConstraint(sourceConstraint);
    ECClassCP targetClass = GetClassFromRelationshipConstraint(targetConstraint);
    if (nullptr != sourceClass && nullptr != targetClass)
        return RelatedClass(*sourceClass, SelectClass<ECRelationshipClass>(*relationship, ""), isRelationshipForward, SelectClass<ECClass>(*targetClass, ""), isTargetClassOptional);

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, Utf8PrintfString("Failed to determine source and target for navigation property '%s.%s'", prop.GetClass().GetFullName(), prop.GetName().c_str()));
    return RelatedClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<class TClass>
static void PushClassIfNotExists(bvector<TClass const*>& list, TClass const& cls)
    {
    for (size_t i = 0; i < list.size(); ++i)
        {
        TClass const* existing = list[i];
        if (cls.Is(existing))
            return;
        if (existing->Is(&cls))
            {
            list[i] = &cls;
            return;
            }
        }
    list.push_back(&cls);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool SupportsAny(ECRelationshipConstraintCR constraint, SupportedClassInfos const& container)
    {
    for (auto const& info : container)
        {
        if (constraint.SupportsClass(info.GetClass()))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedRelationshipClassInfos ECSchemaHelper::GetPossibleRelationships(ECClassCR source, RequiredRelationDirection direction, Utf8StringCR targetClassNames) const
    {
    auto targetClasses = GetECClassesFromClassList(targetClassNames, false);
    bvector<ECRelationshipClassCP> relationships;
    for (ECSchemaCP schema : GetECSchemas(""))
        {
        for (ECClassCP ecClass : schema->GetClasses())
            {
            if (!ecClass->IsRelationshipClass())
                continue;

            ECRelationshipClassCP rel = ecClass->GetRelationshipClassCP();
            if (0 != (RequiredRelationDirection_Forward & direction) && rel->GetSource().SupportsClass(source) && SupportsAny(rel->GetTarget(), targetClasses))
                PushClassIfNotExists<ECRelationshipClass>(relationships, *rel);
            if (0 != (RequiredRelationDirection_Backward & direction) && rel->GetTarget().SupportsClass(source) && SupportsAny(rel->GetSource(), targetClasses))
                PushClassIfNotExists<ECRelationshipClass>(relationships, *rel);
            }
        }
    return ContainerHelpers::TransformContainer<SupportedRelationshipClassInfos>(relationships, [](auto const& rel){return SupportedRelationshipClassInfo(*rel, CLASS_SELECTION_FLAG_Include | CLASS_SELECTION_FLAG_Polymorphic);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void RenameInstanceFilterAlias(Utf8StringR filter, Utf8CP from, Utf8CP to)
    {
    std::regex reg(Utf8PrintfString("(^|[\\s\\(\\)])(\\[?%s\\]?)([\\s\\.\\(\\)])", from));
    filter = std::regex_replace(filter, reg, Utf8PrintfString("$1[%s]$3", to));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void FixRelationshipStepTargetInstanceFilter(RelatedClass& step, Utf8CP fromAlias = "this")
    {
    if (step.GetTargetInstanceFilter().empty())
        return;

    Utf8String targetInstanceFilterWithFixedAlias(step.GetTargetInstanceFilter());
    RenameInstanceFilterAlias(targetInstanceFilterWithFixedAlias, fromAlias, step.GetTargetClass().GetAlias().c_str());
    step.SetTargetInstanceFilter(targetInstanceFilterWithFixedAlias);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void PushRelationshipStep(bvector<RelatedClass>& steps, ECClassCR source, ECRelationshipClassCR rel, bool isForwardRelationship, ECClassCR target, bool isTargetOptional,
    Utf8StringCR targetInstanceFilter, ECClassUseCounter& classCounter)
    {
    RelatedClass rc(source, SelectClass<ECRelationshipClass>(rel, CreateRelationshipAlias(rel, classCounter)), isForwardRelationship,
        SelectClass<ECClass>(target, CreateTargetClassAlias(target, classCounter)), isTargetOptional);
    if (!targetInstanceFilter.empty())
        {
        rc.SetTargetInstanceFilter(targetInstanceFilter);
        FixRelationshipStepTargetInstanceFilter(rc);
        }
    steps.push_back(rc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void PushRelationshipSteps(bvector<RelatedClass>& steps, ECClassCR source, ECRelationshipClassCR rel, bool isForwardRelationship, SupportedClassInfos const& targetClassInfos,
    Utf8StringCR targetInstanceFilter, ECClassUseCounter& classCounter, bool looseSourceMatching)
    {
    ECRelationshipConstraintCR sourceConstraint = isForwardRelationship ? rel.GetSource() : rel.GetTarget();
    ECRelationshipConstraintCR targetConstraint = isForwardRelationship ? rel.GetTarget() : rel.GetSource();

    // when loose matching, we allow the given class to be a base class of the given relationship, otherwise
    // we require it to be completely supported
    if (!sourceConstraint.SupportsClass(source))
        {
        if (!looseSourceMatching)
            return;

        auto constraintClass = sourceConstraint.GetAbstractConstraint();
        if (!constraintClass || !constraintClass->Is(&source))
            return;
        }

    bool isTargetClassOptional = (targetConstraint.GetMultiplicity().GetLowerLimit() == 0);
    if (targetClassInfos.empty())
        {
        ECClassCR targetClass = *targetConstraint.GetAbstractConstraint();
        PushRelationshipStep(steps, source, rel, isForwardRelationship, targetClass, isTargetClassOptional, targetInstanceFilter, classCounter);
        }
    else
        {
        for (auto const& targetClassInfo : targetClassInfos)
            {
            ECClassCR targetClass = targetClassInfo.GetClass();
            PushRelationshipStep(steps, source, rel, isForwardRelationship, targetClass, isTargetClassOptional, targetInstanceFilter, classCounter);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetECSqlTargetInstanceFilter(ECSchemaHelper const& helper, Utf8CP instanceFilter)
    {
    auto ecsqlTargetInstanceFilter = ECExpressionsHelper(helper.GetECExpressionsCache()).ConvertToECSql(instanceFilter, nullptr, nullptr);
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, ecsqlTargetInstanceFilter.GetBindings().empty(), Utf8PrintfString("Expected no bindings, but got %" PRIu64, (uint64_t)ecsqlTargetInstanceFilter.GetBindings().size()));
    return ecsqlTargetInstanceFilter.GetClause();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClass> BuildRelationshipStepsFromStepSpecification(ECSchemaHelper const& helper, ECClassCR source, RelationshipStepSpecification const& stepSpecification,
    Utf8CP stepInstanceFilter, ECClassUseCounter& classCounter, bool looseSourceMatching)
    {
    bvector<RelatedClass> steps;

    SupportedClassInfos targetClassInfos = helper.GetECClassesFromClassList(stepSpecification.GetTargetClassName(), false);
    Utf8String targetInstanceFilter = GetECSqlTargetInstanceFilter(helper, stepInstanceFilter);
    SupportedRelationshipClassInfos relationshipInfos = helper.GetECRelationshipClasses(stepSpecification.GetRelationshipClassName());
    bool tryForwardRelationship = (stepSpecification.GetRelationDirection() == RequiredRelationDirection_Forward || stepSpecification.GetRelationDirection() == RequiredRelationDirection_Both);
    bool tryBackwardRelationship = (stepSpecification.GetRelationDirection() == RequiredRelationDirection_Backward || stepSpecification.GetRelationDirection() == RequiredRelationDirection_Both);

    for (auto const& relationshipInfo : relationshipInfos)
        {
        if (tryForwardRelationship)
            PushRelationshipSteps(steps, source, relationshipInfo.GetClass(), true, targetClassInfos, targetInstanceFilter, classCounter, looseSourceMatching);
        if (tryBackwardRelationship)
            PushRelationshipSteps(steps, source, relationshipInfo.GetClass(), false, targetClassInfos, targetInstanceFilter, classCounter, looseSourceMatching);
        }
    return steps;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> BuildRelationshipPathsFromStepSpecifications(ECSchemaHelper const& helper, ECClassCR source, size_t stepIndex, bvector<RelationshipStepSpecification*> const& stepSpecs,
    bvector<Utf8String> const& stepInstanceFilters, ECClassUseCounter& classCounter, bool looseSourceMatching)
    {
    bvector<RelatedClassPath> paths;
    if (stepIndex >= stepSpecs.size())
        return paths;

    RelationshipStepSpecification const& currSpec = *stepSpecs[stepIndex];
    Utf8CP currStepInstanceFilter = stepInstanceFilters.size() > stepIndex ? stepInstanceFilters[stepIndex].c_str() : nullptr;
    bvector<RelatedClass> steps = BuildRelationshipStepsFromStepSpecification(helper, source, currSpec, currStepInstanceFilter, classCounter, looseSourceMatching);
    for (auto& step : steps)
        {
        bvector<RelatedClassPath> next = BuildRelationshipPathsFromStepSpecifications(helper, step.GetTargetClass().GetClass(), stepIndex + 1, stepSpecs, stepInstanceFilters, classCounter, looseSourceMatching);
        if (next.empty())
            {
            paths.push_back({step});
            }
        else
            {
            ContainerHelpers::MoveTransformContainer(paths, next, [&step](auto&& path)
                {
                path.insert(path.begin(), step);
                return std::move(path);
                });
            }
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::BuildRelationshipPathsFromStepSpecifications(ECClassCR source, bvector<RelationshipStepSpecification*> const& stepSpecifications,
    bvector<Utf8String> const& stepInstanceFilters, bool looseSourceMatching) const
    {
    ECClassUseCounter classCounter;
    return ::BuildRelationshipPathsFromStepSpecifications(*this, source, 0, stepSpecifications, stepInstanceFilters, classCounter, looseSourceMatching);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TClass>
static Utf8String CreateECClassIdSelectClause(SelectClass<TClass> const& selectClass, bool actual)
    {
    return actual
        ? Utf8PrintfString("[%s].[ECClassId]", selectClass.GetAlias().c_str())
        : Utf8PrintfString("%" PRIu64, selectClass.GetClass().GetId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String BuildPathInfoSelectClause(RelatedClassPathCR path, bool selectClassIdsPolymorphically)
    {
    Utf8String clause = "'[";
    for (size_t i = 0; i < path.size(); ++i)
        {
        auto const& step = path[i];
        if (clause.length() > 2)
            clause.append(",");

        Utf8String targetInstanceFilterSelector;
        if (!step.GetTargetInstanceFilter().empty())
            {
            targetInstanceFilterSelector = step.GetTargetInstanceFilter();
            // rename the alias to "this"
            RenameInstanceFilterAlias(targetInstanceFilterSelector, step.GetTargetClass().GetAlias().c_str(), "this");
            // escape all single quotes to avoid ending or starting a sqlite string
            targetInstanceFilterSelector.ReplaceAll("'", "''");
            // escape all double quotes and backslashes
            targetInstanceFilterSelector.ReplaceAll("\\", "\\\\");
            targetInstanceFilterSelector.ReplaceAll("\"", "\\\"");
            // wrap in double quotes to make it a JSON string
            targetInstanceFilterSelector.AddQuotes();
            }
        else
            {
            targetInstanceFilterSelector = "null";
            }

        bool selectRelationshipIdPolymorphically = selectClassIdsPolymorphically && nullptr == step.GetNavigationProperty();
        bool selectTargetClassIdPolymorphically = selectClassIdsPolymorphically || (i < path.size() - 1);

        clause.append("{");
        clause.append("\"RelId\":' || CAST(").append(CreateECClassIdSelectClause(step.GetRelationship(), selectRelationshipIdPolymorphically)).append(" AS TEXT) || ',");
        clause.append("\"IsRelFwd\":").append(step.IsForwardRelationship() ? "true" : "false").append(",");
        clause.append("\"IsTgtOptional\":true").append(",");
        clause.append("\"TgtClassId\":' || CAST(").append(CreateECClassIdSelectClause(step.GetTargetClass(), selectTargetClassIdPolymorphically)).append(" AS TEXT) || ',");
        clause.append("\"Filter\":").append(targetInstanceFilterSelector);
        clause.append("}");
        }
    clause.append("]'");
    return clause;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document BuildPathInfoJsonFromRelationshipPath(RelatedClassPathCR path)
    {
    rapidjson::Document pathInfoJson;
    pathInfoJson.SetArray();
    for (auto const& step : path)
        {
        rapidjson::Value stepInfoJson(rapidjson::kObjectType);
        stepInfoJson.AddMember("RelId", step.GetRelationship().GetClass().GetId().GetValue(), pathInfoJson.GetAllocator());
        stepInfoJson.AddMember("IsRelFwd", step.IsForwardRelationship(), pathInfoJson.GetAllocator());
        stepInfoJson.AddMember("IsTgtOptional", step.IsTargetOptional(), pathInfoJson.GetAllocator());
        stepInfoJson.AddMember("TgtClassId", step.GetTargetClass().GetClass().GetId().GetValue(), pathInfoJson.GetAllocator());
        pathInfoJson.PushBack(stepInfoJson, pathInfoJson.GetAllocator());
        }
    return pathInfoJson;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static RelatedClassPath BuildPathFromPathInfoJsonAndTargetClassId(ECClassCR source, RapidJsonValueCR json, SchemaManagerCR schemas)
    {
    RelatedClassPath path;
    ECClassCP currSource = &source;
    for (rapidjson::SizeType i = 0; i < json.Size(); ++i)
        {
        ECRelationshipClassCP relationship = schemas.GetClass(ECClassId(json[i]["RelId"].GetUint64()))->GetRelationshipClassCP();
        bool isRelationshipForward = json[i]["IsRelFwd"].GetBool();
        bool isTargetOptional = json[i]["IsTgtOptional"].GetBool();
        ECClassCP targetClass = schemas.GetClass(ECClassId(json[i]["TgtClassId"].GetUint64()));
        Utf8CP targetInstanceFilter = (json[i].HasMember("Filter") && json[i]["Filter"].IsString()) ? json[i]["Filter"].GetString() : nullptr;
        RelatedClass rc(*currSource, SelectClass<ECRelationshipClass>(*relationship, ""), isRelationshipForward,
            SelectClass<ECClass>(*targetClass, ""), isTargetOptional);
        if (targetInstanceFilter)
            rc.SetTargetInstanceFilter(targetInstanceFilter);
        path.push_back(rc);
        currSource = targetClass;
        }
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TClass>
static PresentationQueryContractFieldPtr CreateTargetClassIdGroupingField(SelectClass<TClass> const& selectClass)
    {
    Utf8String classIdSelectClause = CreateECClassIdSelectClause(selectClass, true);
    Utf8String groupingClause = Utf8String("+").append(classIdSelectClause);
    auto field = PresentationQueryContractSimpleField::Create(Utf8String(selectClass.GetAlias()).append("_ECClassId").c_str(), classIdSelectClause.c_str());
    field->SetGroupingClause(groupingClause);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static UnionGenericQueryPtr CreateRelationshipPathsWithTargetInstancesQuery(ECSchemaHelper const& helper, SelectClass<ECClass> const& sourceClass,
    RelationshipPathSpecification const& pathSpecification, InstanceFilteringParams const* sourceInstanceFilter, bvector<RelatedClassPath> const& relatedInstancePaths,
    bool isTargetPolymorphic, bool countTargets, bvector<Utf8String> const& stepInstanceFilters)
    {
    ECClassCP requestedTargetClass = (!pathSpecification.GetSteps().empty() && !pathSpecification.GetSteps().back()->GetTargetClassName().empty())
        ? helper.GetECClass(pathSpecification.GetSteps().back()->GetTargetClassName().c_str()) : nullptr;

    UnionGenericQueryPtr unionQuery = UnionGenericQuery::Create({});

    auto paths = helper.BuildRelationshipPathsFromStepSpecifications(sourceClass.GetClass(), pathSpecification.GetSteps(), stepInstanceFilters, false);

    SelectClassWithExcludes<ECClass> selectClass(sourceClass);
    if (selectClass.GetAlias().empty())
        selectClass.SetAlias("this");

    for (auto& path : paths)
        {
        Utf8String pathInfoSelectClause = BuildPathInfoSelectClause(path, isTargetPolymorphic);
        for (RelatedClassR rc : path)
            rc.SetIsTargetOptional(false);

        SelectClassWithExcludes<ECClass> const& targetClass = !path.empty() ? path.back().GetTargetClass() : selectClass;
        auto sourceClassIdField = PresentationQueryContractSimpleField::Create("SourceECClassId", CreateECClassIdSelectClause(selectClass, true).c_str(), false);
        auto sourceECClassIdsField = PresentationQueryContractFunctionField::Create("SourceECClassIds", FUNCTION_NAME_JsonConcat, { sourceClassIdField }, false, true);
        auto targetClassIdSelectField = PresentationQueryContractSimpleField::Create("TargetECClassId", CreateECClassIdSelectClause(targetClass, isTargetPolymorphic).c_str());
        auto pathInfoField = PresentationQueryContractSimpleField::Create("PathInfo", pathInfoSelectClause.c_str(), false);
        auto requestedTargetECClassIdField = PresentationQueryContractSimpleField::Create("RequestedTargetECClassId", requestedTargetClass ? Utf8PrintfString("%" PRIu64, requestedTargetClass->GetId().GetValue()).c_str() : "0", false);

        ComplexGenericQueryPtr pathQuery = ComplexGenericQuery::Create();
        pathQuery->SelectContract(*SimpleQueryContract::Create(
            {
            sourceClassIdField,
            targetClassIdSelectField,
            pathInfoField,
            requestedTargetECClassIdField,
            }));
        pathQuery->From(selectClass);
        pathQuery->Join(path);

        for (auto const& relatedInstancePath : relatedInstancePaths)
            pathQuery->Join(relatedInstancePath);

        if (!targetClass.IsSelectPolymorphic())
            pathQuery->Where(Utf8PrintfString("[%s].[%s] = ?", targetClass.GetAlias().c_str(), targetClassIdSelectField->GetName()).c_str(), {std::make_shared<BoundQueryId>(targetClass.GetClass().GetId())});

        if (sourceInstanceFilter)
            QueryBuilderHelpers::ApplyInstanceFilter(*pathQuery, *sourceInstanceFilter);

        if (!pathQuery.IsValid())
            continue;

        auto groupByContract = SimpleQueryContract::Create(
            {
            sourceECClassIdsField,
            });
        for (auto const& step : path)
            {
            if (isTargetPolymorphic && step.GetNavigationProperty() == nullptr)
                groupByContract->AddField(*CreateTargetClassIdGroupingField(step.GetRelationship()));
            groupByContract->AddField(*CreateTargetClassIdGroupingField(step.GetTargetClass()));
            }
        pathQuery->GroupByContract(*groupByContract);

        unionQuery->AddQuery(*pathQuery);
        }

    if (unionQuery->GetQueries().empty())
        return nullptr;

    return unionQuery;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static UnionGenericQueryPtr CreateRelationshipPathQuery(ECSchemaHelper const& helper, SelectClass<ECClass> sourceClass,
    RelationshipPathSpecification const& pathSpecification, bool isTargetPolymorphic)
    {
    ECClassCP requestedTargetClass = (!pathSpecification.GetSteps().empty() && !pathSpecification.GetSteps().back()->GetTargetClassName().empty())
        ? helper.GetECClass(pathSpecification.GetSteps().back()->GetTargetClassName().c_str()) : nullptr;

    UnionGenericQueryPtr unionQuery = UnionGenericQuery::Create({});
    bvector<RelatedClassPath> paths = helper.BuildRelationshipPathsFromStepSpecifications(sourceClass.GetClass(), pathSpecification.GetSteps(), {}, false);
    for (auto& path : paths)
        {
        rapidjson::Document pathInfoJson = BuildPathInfoJsonFromRelationshipPath(path);
        Utf8CP targetClassIdsQuery = isTargetPolymorphic
            ? "SELECT SourceECInstanceId AS ECClassId FROM meta.ClassHasAllBaseClasses WHERE TargetECInstanceId = ?"
            : "SELECT ECInstanceId AS ECClassId FROM meta.ECClassDef WHERE ECInstanceId = ?";
        ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
        query->SelectContract(*SimpleQueryContract::Create(
            {
            PresentationQueryContractSimpleField::Create("SourceECClassIds", Utf8PrintfString("'[%" PRIu64 "]'", sourceClass.GetClass().GetId().GetValue()).c_str(), false),
            PresentationQueryContractSimpleField::Create("PathInfo", Utf8PrintfString("'%s'", BeRapidJsonUtilities::ToString(pathInfoJson).c_str()).c_str(), false),
            PresentationQueryContractSimpleField::Create("RequestedTargetECClassId", Utf8PrintfString("%" PRIu64, requestedTargetClass ? requestedTargetClass->GetId().GetValue() : 0).c_str(), false),
            }));
        query->From(*StringGenericQuery::Create(targetClassIdsQuery, {std::make_shared<BoundQueryId>(path.back().GetTargetClass().GetClass().GetId())}));
        unionQuery->AddQuery(*query);
        }
    return !unionQuery->GetQueries().empty() ? unionQuery : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Nullable<uint64_t> ECSchemaHelper::QueryTargetsCount(RelatedClassPathCR path, InstanceFilteringParams const* sourceInstanceFilter, bvector<RelatedClassPath> const& relatedInstancePaths) const
    {
    if (path.empty())
        return nullptr;

    SelectClass<ECClass> selectClass(*path[0].GetSourceClass(), "this");
    SelectClass<ECClass> const& targetClass = path.back().GetTargetClass();

    ComplexGenericQueryPtr targetCountsQuery = ComplexGenericQuery::Create();

    auto sourceInstanceIdSelectField = PresentationQueryContractSimpleField::Create("/SourceECInstanceId/", "ECInstanceId");
    sourceInstanceIdSelectField->SetGroupingClause(Utf8PrintfString("[%s].[ECInstanceId]", selectClass.GetAlias().c_str()));

    auto targetClassIdSelectField = PresentationQueryContractSimpleField::Create("/TargetECClassId/", "ECClassId");
    targetClassIdSelectField->SetGroupingClause(Utf8PrintfString("[%s].[ECClassId]", targetClass.GetAlias().c_str()));

    auto targetCountsContract = SimpleQueryContract::Create(
        {
        sourceInstanceIdSelectField,
        targetClassIdSelectField,
        PresentationQueryContractFunctionField::Create("TargetsCount", "COUNT",
            {
            PresentationQueryContractSimpleField::Create("", "1", false),
            }, false, true),
        });
    targetCountsQuery->SelectContract(*targetCountsContract, selectClass.GetAlias().c_str());
    targetCountsQuery->From(selectClass);
    targetCountsQuery->Join(path);

    for (auto const& relatedInstancePath : relatedInstancePaths)
        targetCountsQuery->Join(relatedInstancePath);

    if (!targetClass.IsSelectPolymorphic())
        targetCountsQuery->Where(Utf8PrintfString("[%s].[%s] = ?", targetClass.GetAlias().c_str(), targetClassIdSelectField->GetName()).c_str(), { std::make_shared<BoundQueryId>(targetClass.GetClass().GetId()) });

    if (sourceInstanceFilter)
        QueryBuilderHelpers::ApplyInstanceFilter(*targetCountsQuery, *sourceInstanceFilter);

    targetCountsQuery->GroupByContract(*targetCountsContract);

    auto possibleTargetsCountField = PresentationQueryContractFunctionField::Create("PossibleTargetsCount", "MAX",
        {
        PresentationQueryContractSimpleField::Create("", "[T].[TargetsCount]", false),
        }, false);
    auto possibleTargetsCountQuery = ComplexGenericQuery::Create();
    possibleTargetsCountQuery->SelectContract(*SimpleQueryContract::Create(
        {
        possibleTargetsCountField,
        }));
    possibleTargetsCountQuery->From(*targetCountsQuery, "T");

    // prepare & bind
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::QueryTargetsCount");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), possibleTargetsCountQuery->ToString().c_str());
    if (!stmt.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", possibleTargetsCountQuery->ToString().c_str()));

    possibleTargetsCountQuery->BindValues(*stmt);

    // query the count
    if (BE_SQLITE_ROW != QueryExecutorHelper::Step(*stmt))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Query returned 0 rows");

    return stmt->GetValueUInt64(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignClassAliases(RelatedClassPath::iterator begin, RelatedClassPath::iterator end, ECClassUseCounter& counter, bool skipFirstRelationship)
    {
    for (auto iter = begin; iter != end; ++iter)
        {
        RelatedClassR rel = *iter;
        if (!skipFirstRelationship)
            rel.GetRelationship().SetAlias(CreateRelationshipAlias(rel.GetRelationship().GetClass(), counter));
        rel.GetTargetClass().SetAlias(CreateTargetClassAlias(rel.GetTargetClass().GetClass(), counter));
        skipFirstRelationship = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* This method uses a very sub-optimal algorithm for assigning similar aliases for
* similar parts of relationship paths. Relationships and classes with similar aliases
* are reused when generating the query, allowing us to reduce the total amount of JOINs.
* An optimal solution would use a tree structure to store relationship paths.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignClassAliases(bvector<RelatedClassPath*> const& paths)
    {
    if (paths.empty())
        return;

    ECClassUseCounter counter;

    // assign aliases for the first path
    AssignClassAliases(paths.front()->begin(), paths.front()->end(), counter, false);

    // for every other path we want to:
    // 1. find a previous path with already assigned aliases that has similar steps in front
    // 2. copy class aliases from those steps to matching steps of current path
    // 3. generate aliases for the rest of steps in current path
    for (size_t i = 1; i < paths.size(); ++i)
        {
        int maxMatchingStepsCount = -1;
        SelectClass<ECRelationshipClass> const* lastMatchingRelationship = nullptr;
        RelatedClassPathR currPath = *paths[i];
        for (size_t j = 0; j < i; ++j)
            {
            int matchingStepsCount = 0;
            RelatedClassPathCR testedPath = *paths[j];
            while (matchingStepsCount < (int)testedPath.size() && matchingStepsCount < (int)currPath.size())
                {
                RelatedClassR currStep = currPath[matchingStepsCount];
                RelatedClassCR testedStep = testedPath[matchingStepsCount];
                if (!currStep.SourceAndRelationshipEqual(testedStep))
                    {
                    // the source or relationship are different in this step - paths start to deviate which means
                    // we don't care about testedPath anymore
                    break;
                    }
                else if (&currStep.GetTargetClass().GetClass() != &testedStep.GetTargetClass().GetClass())
                    {
                    // source and relationship are same, but target differs - we can reuse relationship if both steps are non-optional
                    if (!currStep.IsTargetOptional() && !testedStep.IsTargetOptional() && matchingStepsCount > maxMatchingStepsCount)
                        lastMatchingRelationship = &testedStep.GetRelationship();
                    break;
                    }

                ++matchingStepsCount;
                if (matchingStepsCount > maxMatchingStepsCount)
                    {
                    // we matched more steps than was matched while testing previous paths - copy aliases
                    currStep.GetRelationship().SetAlias(testedStep.GetRelationship().GetAlias());
                    currStep.GetTargetClass().SetAlias(testedStep.GetTargetClass().GetAlias());
                    // reset because the whole step was matched
                    lastMatchingRelationship = nullptr;
                    }
                }
            if (matchingStepsCount > maxMatchingStepsCount)
                {
                // store the number of max steps matched
                maxMatchingStepsCount = matchingStepsCount;
                if (maxMatchingStepsCount >= (int)currPath.size())
                    {
                    // if we matched all steps of current path, there's no point in iterating anymore
                    break;
                    }
                }
            }
        // generate class aliases for steps that we didn't match
        if (maxMatchingStepsCount < 0)
            maxMatchingStepsCount = 0;

        bool skipFirstRelationship = false;
        if (nullptr != lastMatchingRelationship && currPath.size() != maxMatchingStepsCount)
            {
            currPath[maxMatchingStepsCount].GetRelationship().SetAlias(lastMatchingRelationship->GetAlias());
            skipFirstRelationship = true;
            }

        AssignClassAliases(currPath.begin() + maxMatchingStepsCount, currPath.end(), counter, skipFirstRelationship);
        }

    // fix target instance filters to use the new aliases
    for (auto& path : paths)
        {
        for (auto& step : *path)
            FixRelationshipStepTargetInstanceFilter(step);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::RelationshipPathsResponse ECSchemaHelper::GetRelationshipPaths(RelationshipPathsRequestParams const& params) const
    {
    auto scope = Diagnostics::Scope::Create("Get relationship paths");

    // build a query that selects ECClassIds of all matching classes that have instances
    int maxTargetIndex = -1;
    UnionGenericQueryPtr unionQuery = UnionGenericQuery::Create({});
    for (auto const& pathSpecEntry : *params.m_paths)
        {
        UnionGenericQueryPtr specQuery = pathSpecEntry.m_applyTargetInstancesCheck
            ? CreateRelationshipPathsWithTargetInstancesQuery(*this, params.m_source, *pathSpecEntry.m_specification, params.m_sourceInstanceFilter, params.m_relatedInstancePaths, pathSpecEntry.m_isTargetPolymorphic, params.m_countTargets, pathSpecEntry.m_stepInstanceFilters)
            : CreateRelationshipPathQuery(*this, params.m_source, *pathSpecEntry.m_specification, pathSpecEntry.m_isTargetPolymorphic);
        if (specQuery.IsValid())
            {
            auto indexedQuery = ComplexGenericQuery::Create();
            auto selectContract = SimpleQueryContract::Create(
                {
                PresentationQueryContractSimpleField::Create("Index", std::to_string(pathSpecEntry.m_targetIndex).c_str()), // 0
                PresentationQueryContractSimpleField::Create("PathInfo", "PathInfo"),                                       // 1
                PresentationQueryContractSimpleField::Create("SourceECClassIds", "SourceECClassIds"),                       // 2
                PresentationQueryContractSimpleField::Create("RequestedTargetECClassId", "RequestedTargetECClassId"),       // 3
                });
            indexedQuery->SelectContract(*selectContract);
            indexedQuery->From(*specQuery);
            unionQuery->AddQuery(*indexedQuery);
            }
        if (pathSpecEntry.m_targetIndex > maxTargetIndex)
            maxTargetIndex = pathSpecEntry.m_targetIndex;
        }

    RelationshipPathsResponse paths(maxTargetIndex + 1);
    if (unionQuery->GetQueries().empty())
        return paths;

    // prepare & bind
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetRelationshipPaths");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), unionQuery->ToString().c_str());
    if (!stmt.IsValid())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", unionQuery->ToString().c_str()));

    unionQuery->BindValues(*stmt);

    // create the paths list
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        {
        int index = stmt->GetValueInt(0);

        if (stmt->IsValueNull(1))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Got NULL path info");

        rapidjson::Document::AllocatorType alloc(1024);
        rapidjson::Document pathInfoJson(&alloc);
        pathInfoJson.Parse(stmt->GetValueText(1));
        auto path = BuildPathFromPathInfoJsonAndTargetClassId(params.m_source.GetClass(), pathInfoJson, m_connection.GetECDb().Schemas());
        if (path.empty())
            continue;

        // filter-out hidden classes
        ECClassCR actualTargetClass = path.back().GetTargetClass().GetClass();
        ECClassId requestedTargetClassId = stmt->GetValueId<ECClassId>(3);
        if (IsClassHidden<ECClass>(actualTargetClass, [&](ECClassCR c){return c.GetId() == requestedTargetClassId;}))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_TRACE, LOG_INFO, Utf8PrintfString("Skipping hidden ECClass: '%s'", actualTargetClass.GetFullName()));
            continue;
            }

        rapidjson::Document sourceClassIdsJson;
        sourceClassIdsJson.Parse(stmt->GetValueText(2));
        if (sourceClassIdsJson.Empty())
            continue;

        std::unordered_set<ECClassCP> sourceClasses;
        for (auto iter = sourceClassIdsJson.Begin(); iter != sourceClassIdsJson.End(); ++iter)
            sourceClasses.insert(m_connection.GetECDb().Schemas().GetClass(ECClassId(iter->GetUint64())));

        paths.GetPaths(index).push_back(RelationshipPathsResponse::RelationshipPathResult{ path, sourceClasses });
        }

    bvector<RelatedClassPath*> allPaths;
    for (size_t i = 0; i <= maxTargetIndex; ++i)
        {
        for (auto& pathResult : paths.GetPaths(i))
            allPaths.push_back(&pathResult.m_path);
        }
    AssignClassAliases(allPaths);

    if (params.m_countTargets)
        {
        for (auto path : allPaths)
            path->SetTargetsCount(QueryTargetsCount(*path, params.m_sourceInstanceFilter, params.m_relatedInstancePaths));
        }

    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String, bvector<RelatedClassPath>> ECSchemaHelper::GetRelatedInstancePaths(ECClassCR selectClass,
    RelatedInstanceSpecificationList const& relatedInstanceSpecs, ECClassUseCounter& relationshipsUseCount) const
    {
    size_t specIndex = 0;
    auto pathSpecs = ContainerHelpers::TransformContainer<bvector<RelationshipPathsRequestParams::PathSpecification>>(relatedInstanceSpecs, [&specIndex](auto const& spec)
        {
        return RelationshipPathsRequestParams::PathSpecification(specIndex++, spec->GetRelationshipPath(), false);
        });
    bvector<RelatedClassPath> noRelatedInstances;
    ECSchemaHelper::RelationshipPathsRequestParams params(SelectClass<ECClass>(selectClass, ""), pathSpecs, nullptr, noRelatedInstances, relationshipsUseCount, false);
    auto indexedPaths = GetRelationshipPaths(params);

    bmap<Utf8String, bvector<RelatedClassPath>> paths;
    for (size_t i = 0; i < relatedInstanceSpecs.size(); ++i)
        {
        RelatedInstanceSpecificationCR spec = *relatedInstanceSpecs[i];
        if (paths.end() != paths.find(spec.GetAlias()))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Related instance alias must be unique per parent specification. Found multiple alias: '%s'", spec.GetAlias().c_str()));
            continue;
            }
        for (auto& pathResult : indexedPaths.GetPaths(i))
            {
            auto& path = pathResult.m_path;
            for (RelatedClassR step : path)
                step.SetIsTargetOptional(!spec.IsRequired());
            if (!path.empty())
                path.back().GetTargetClass().SetAlias(spec.GetAlias());
            }
        if (!indexedPaths.GetPaths(i).empty())
            paths.Insert(spec.GetAlias(), ContainerHelpers::TransformContainer<bvector<RelatedClassPath>>(indexedPaths.GetPaths(i), [](auto const& pathResult){return pathResult.m_path;}));
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSchemaHelper::GetTypeName(ECPropertyCR property)
    {
    if (property.GetIsPrimitive())
        {
        Utf8StringCR extendedTypeName = property.GetAsPrimitiveProperty()->GetExtendedTypeName();
        if (extendedTypeName.length() > 0)
            return extendedTypeName;
        }
    return property.GetTypeName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SupportedClassesParser::Parse(ECSchemaHelper const& helper, Utf8StringCR str, bool supportExclusion)
    {
    ClassNamesParser names(str.c_str(), supportExclusion);
    for (auto entry : names)
        {
        ECClassCP ecClass = helper.GetECClass(entry.GetSchemaName(), entry.GetClassName());
        if (nullptr == ecClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Requested ECClass not found: '%s.%s'", entry.GetSchemaName(), entry.GetClassName()));
            continue;
            }
        SupportedClassInfo<ECClass> info(*ecClass, entry.GetFlags());
        auto iter = m_classInfos.find(info);
        if (m_classInfos.end() != iter)
            {
            if (entry.IsPolymorphic() && entry.IsExclude())
                m_classInfos.erase(iter);
            else
                iter->SetFlags(iter->GetFlags() | entry.GetFlags());
            }
        else
            {
            m_classInfos.insert(info);
            }
        }
    }

static BeMutex s_getIntanceMutex;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECInstancesHelper::LoadInstance(IECInstancePtr& instance, IConnectionCR connection, ECInstanceKeyCR key)
    {
    ECClassCP selectClass = connection.GetECDb().Schemas().GetClass(key.GetClassId());
    if (nullptr == selectClass || !selectClass->IsEntityClass())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("ECClass not found: %" PRIu64, key.GetClassId().GetValue()));

    Savepoint txn(connection.GetDb(), "ECInstancesHelper::LoadInstance");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    Utf8String ecsql("SELECT * FROM ONLY ");
    ecsql.append(selectClass->GetECSqlName()).append(" WHERE ECInstanceId=?");
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), ecsql.c_str());
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", ecsql.c_str()));

    if (ECSqlStatus::Success != stmt->BindId(1, key.GetInstanceId()))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to bind ECInstance ID: %" PRIu64, key.GetInstanceId().GetValue()));

    ECInstanceECSqlSelectAdapter adapter(*stmt);
    DbResult result = QueryExecutorHelper::Step(*stmt);
    if (BE_SQLITE_ROW == result)
        {
        BeMutexHolder lock(s_getIntanceMutex);
        instance = adapter.GetInstance();
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ECInstancesHelper::GetValue(IConnectionCR connection, ECInstanceKeyCR key, Utf8CP propertyName)
    {
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(key.GetClassId());
    if (nullptr == ecClass || !ecClass->IsEntityClass())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("ECClass not found: %" PRIu64, key.GetClassId().GetValue()));

    ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
    if (nullptr == ecProperty || !ecProperty->GetIsPrimitive())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("ECProperty not found or is not primitive: '%s.%s'", ecClass->GetFullName(), propertyName));

    return GetValue(connection, *ecClass, key.GetInstanceId(), *ecProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ECInstancesHelper::GetValue(IConnectionCR connection, ECClassCR ecClass, ECInstanceId instanceId, ECPropertyCR ecProperty)
    {
    Savepoint txn(connection.GetDb(), "ECInstancesHelper::GetValue");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    Utf8String ecsql("SELECT ");
    ecsql.append("[").append(ecProperty.GetName()).append("] ");
    ecsql.append("FROM ONLY ");
    ecsql.append(ecClass.GetECSqlName()).append(" WHERE ECInstanceId = ?");
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), ecsql.c_str());
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", ecsql.c_str()));

    if (ECSqlStatus::Success != stmt->BindId(1, instanceId))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to bind ECInstance ID: %" PRIu64, instanceId.GetValue()));

    DbResult stepStatus = QueryExecutorHelper::Step(*stmt);
    if (BE_SQLITE_ROW != stepStatus)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unexpected statement step result: %d", (int)stepStatus));

    return ValueHelpers::GetECValueFromSqlValue(ecProperty.GetAsPrimitiveProperty()->GetType(), stmt->GetValue(0));
    }
