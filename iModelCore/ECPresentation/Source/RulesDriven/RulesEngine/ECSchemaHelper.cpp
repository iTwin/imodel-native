/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ECSchemaHelper.h"
#include "NavigationQuery.h"
#include "LoggingHelper.h"
#include "ECExpressionContextsProvider.h"
#include "QueryBuilderHelpers.h"
#include <set>

#define NUMERIC_LESS_COMPARE(lhs, rhs) \
    if (lhs < rhs) \
        return true; \
    if (lhs > rhs) \
        return false; \

#define STR_LESS_COMPARE(name, lhs, rhs) \
    int cmp_##name = lhs.CompareTo(rhs); \
    if (cmp_##name < 0) \
        return true; \
    if (cmp_##name > 0) \
        return false; \

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCacheDeprecated::Key::Key(ECSchemaHelper::RelationshipClassPathOptions const& options)
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
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPathsCacheDeprecated::Key::operator<(Key const& other) const
    {
    NUMERIC_LESS_COMPARE(m_sourceClass, other.m_sourceClass);
    NUMERIC_LESS_COMPARE(m_targetClass, other.m_targetClass);
    NUMERIC_LESS_COMPARE(m_relationshipDirection, other.m_relationshipDirection);
    NUMERIC_LESS_COMPARE(m_depth, other.m_depth);
    STR_LESS_COMPARE(classes, m_supportedClasses, other.m_supportedClasses);
    STR_LESS_COMPARE(relationships, m_supportedRelationships, other.m_supportedRelationships);
    STR_LESS_COMPARE(schemas, m_supportedSchemas, other.m_supportedSchemas);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCache::Key::Key(Key const& other)
    : m_sourceClass(other.m_sourceClass), m_pathSpecification(other.m_pathSpecification), m_mergePolymorphicPaths(other.m_mergePolymorphicPaths)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPathsCache::Key::Key(ECSchemaHelper::RepeatableMultiRelationshipPathOptions const& options)
    : m_sourceClass(&options.m_sourceClass), m_pathSpecification(options.m_path), m_mergePolymorphicPaths(options.m_mergePolymorphicPaths)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPathsCache::Key::operator<(Key const& other) const
    {
    NUMERIC_LESS_COMPARE(m_mergePolymorphicPaths, other.m_mergePolymorphicPaths);
    NUMERIC_LESS_COMPARE(m_sourceClass, other.m_sourceClass);
    STR_LESS_COMPARE(path, m_pathSpecification.GetHash(), other.m_pathSpecification.GetHash());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolymorphicallyRelatedClassesCache::Key::operator<(PolymorphicallyRelatedClassesCache::Key const& other) const
    {
    NUMERIC_LESS_COMPARE(m_source, other.m_source);
    NUMERIC_LESS_COMPARE(m_direction, other.m_direction);
    COMPARE_VEC(m_rels, other.m_rels);
    COMPARE_VEC(m_baseClasses, other.m_baseClasses);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClass> const* PolymorphicallyRelatedClassesCache::Get(Key const& key) const
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_map.find(key);
    return m_map.end() != iter ? &iter->second : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClass> const& PolymorphicallyRelatedClassesCache::Add(Key key, bvector<RelatedClass> relatedClasses)
    {
    BeMutexHolder lock(m_mutex);
    auto iter = m_map.Insert(key, relatedClasses).first;
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PolymorphicallyRelatedClassesCache::Clear()
    {
    BeMutexHolder lock(m_mutex);
    m_map.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::ECSchemaHelper(IConnectionCR connection, RelatedPathsCache* relatedPathsCache,
    PolymorphicallyRelatedClassesCache* polymorphicallyRelatedClassesCache, ECExpressionsCache* ecexpressionsCache)
    : m_connection(connection),
    m_relatedPathsCache(relatedPathsCache), m_ownsRelatedPathsCache(nullptr == relatedPathsCache),
#ifdef wip_check_if_necessary
    m_polymorphicallyRelatedClassesCache(polymorphicallyRelatedClassesCache), m_ownsPolymorphicallyRelatedClassesCache(nullptr == polymorphicallyRelatedClassesCache),
#endif
    m_ecexpressionsCache(ecexpressionsCache), m_ownsECExpressionsCache(nullptr == ecexpressionsCache)
    {
    m_relatedPathsCacheDeprecated = new RelatedPathsCacheDeprecated();
    if (nullptr == m_relatedPathsCache)
        m_relatedPathsCache = new RelatedPathsCache();
#ifdef wip_check_if_necessary
    if (nullptr == m_polymorphicallyRelatedClassesCache)
        m_polymorphicallyRelatedClassesCache = new PolymorphicallyRelatedClassesCache();
#endif
    if (nullptr == m_ecexpressionsCache)
        m_ecexpressionsCache = new ECExpressionsCache();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::~ECSchemaHelper()
    {
    if (m_ownsRelatedPathsCache)
        DELETE_AND_CLEAR(m_relatedPathsCache);
#ifdef wip_check_if_necessary
    if (m_ownsPolymorphicallyRelatedClassesCache)
        DELETE_AND_CLEAR(m_polymorphicallyRelatedClassesCache);
#endif
    if (m_ownsECExpressionsCache)
        DELETE_AND_CLEAR(m_ecexpressionsCache);
    DELETE_AND_CLEAR(m_relatedPathsCacheDeprecated);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchemaHelper::GetSchema(Utf8CP schemaName) const {return m_connection.GetECDb().Schemas().GetSchema(schemaName);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaHelper::GetECClass(Utf8CP schemaName, Utf8CP className, bool isFullSchemaName) const
    {
    Utf8String schemaNameStr = schemaName;
    if (isFullSchemaName)
        {
        uint32_t versionMajor, versionMinor;
        ECSchema::ParseSchemaFullName(schemaNameStr, versionMajor, versionMinor, schemaName);
        }

    ECSchemaCP schema = GetSchema(schemaNameStr.c_str());
    if (nullptr == schema)
        return nullptr;

    return schema->GetClassCP(className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaHelper::GetECClass(Utf8CP fullClassName) const
    {
    Utf8String schemaName, className;
    ECClass::ParseClassName(schemaName, className, fullClassName);
    return GetECClass(schemaName.c_str(), className.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchemaHelper::GetECClass(ECClassId id) const {return m_connection.GetECDb().Schemas().GetClass(id);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECClassCP> ECSchemaHelper::GetECClassesByName(Utf8CP name) const
    {
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetECClassesByName");
    BeAssert(txn.IsActive());

    static Utf8CP statementStr = "SELECT ECInstanceId FROM [meta].[ECClassDef] WHERE Name = ?";
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), statementStr);
    if (stmt.IsNull())
        {
        BeAssert(false);
        return bvector<ECClassCP>();
        }
    stmt->BindText(1, name, IECSqlBinder::MakeCopy::No);

    bvector<ECClassId> ids;
    while (DbResult::BE_SQLITE_ROW == stmt->Step())
        ids.push_back(stmt->GetValueId<ECClassId>(0));

    bvector<ECClassCP> classes;
    for (ECClassId id : ids)
        classes.push_back(m_connection.GetECDb().Schemas().GetClass(id));
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsSchemaHidden(ECSchemaCR ecSchema)
    {
    IECInstancePtr options = ecSchema.GetCustomAttribute("HiddenSchema");
    if (options.IsNull())
        return false;

    ECValue value;
    if (ECObjectsStatus::Success == options->GetValue(value, "ShowClasses") && !value.IsNull() && value.IsBoolean() && true == value.GetBoolean())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsClassHidden(ECClassCR ecClass)
    {
    IECInstancePtr options = ecClass.GetCustomAttribute("HiddenClass");
    if (options.IsNull())
        return false;

    ECValue value;
    if (ECObjectsStatus::Success == options->GetValue(value, "Show") && !value.IsNull() && value.IsBoolean() && true == value.GetBoolean())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsAllowed(ECClassCR ecClass)
    {
    if (!ecClass.IsEntityClass() && !ecClass.IsRelationshipClass())
        return false;

    if (IsClassHidden(ecClass))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
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
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::ParseECSchemas(ECSchemaSet& schemas, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const
    {
    schemas.clear();
    bvector<Utf8String> schemaNames;
    ParseECSchemaNames(schemaNames, exclude, commaSeparatedSchemaList);
    for (Utf8StringCR name : schemaNames)
        {
        ECSchemaCP schema = GetSchema(name.c_str());
        if (nullptr == schema)
            {
            BeAssert(false);
            return;
            }
        schemas.insert(schema);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
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
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SupportedEntityClassInfos ECSchemaHelper::GetECClassesFromClassList(Utf8StringCR classListStr, bool supportExclusion) const
    {
    SupportedClassesParser parser(*this, classListStr, supportExclusion);
    SupportedClassInfos classInfos = parser.GetClassInfos();
    SupportedEntityClassInfos entityClasses;
    for (SupportedClassInfo<ECClass> const& info : classInfos)
        {
        if (info.GetClass().IsEntityClass())
            entityClasses.insert(SupportedEntityClassInfo(*info.GetClass().GetEntityClassCP(), info.GetFlags()));
        }
    return entityClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
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
* @bsimethod                                    Grigas.Petraitis                04/2015
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
            if (nullptr != (*iter)->GetRelationshipClassCP())
                continue;

            if (!IsAllowed(**iter))
                continue;

            BeAssert((*iter)->GetEntityClassCP() != nullptr);
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
            if (!IsAllowed(*derivedClass))
                continue;

            BeAssert(derivedClass->GetEntityClassCP() != nullptr);
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
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassSet ECSchemaHelper::GetECClassesFromSchemaList(Utf8StringCR schemaListStr) const
    {
    return GetECClasses(GetECSchemas(schemaListStr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
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
        if (nullptr == m_connection.GetECDb().Schemas().GetSchema(name.c_str()))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
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
* @bsiclass                                     Grigas.Petraitis                06/2015
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
    IConnectionCR m_connection;
    mutable int m_flags;
    mutable ECSchemaSet m_schemaList;
    mutable IdSet<ECSchemaId> m_schemaIds;
    mutable IdSet<ECClassId> m_includedEntityClassIds;
    mutable IdSet<ECClassId> m_polymorphicallyIncludedClassIds;
    mutable bvector<ECClassCP> m_excludedClasses;
    mutable bvector<ECClassCP> m_polymorphicallyExcludedClasses;
    mutable IdSet<ECClassId> m_relationshipClassIds;
    SupportedEntityClassInfos const* m_classInfos;
    SupportedRelationshipClassInfos const* m_relationshipInfos;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void AddClassId(IdSet<ECClassId>& list, ECClassCR ecClass) {list.insert(ecClass.GetId());}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> DeterminePolymorphicallyIncludedClassesIds() const
        {
        Savepoint txn(m_connection.GetDb(), "SupportedClassesResolver::DeterminePolymorphicallySupportedClassesIds");
        BeAssert(txn.IsActive());

        IdSet<ECClassId> const& ids = GetIncludedEntityClassIds();
        IdsFilteringHelper<IdSet<ECClassId>> helper(ids);

        Utf8String query = "SELECT SourceECInstanceId FROM [meta].[ClassHasAllBaseClasses] ";
        query.append("WHERE ").append(helper.CreateWhereClause("[TargetECInstanceId]"));

        CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), query.c_str());

        BoundQueryValuesList bindings = helper.CreateBoundValues();
        uint32_t i = 1;
        for (BoundQueryValue const* binding : bindings)
            binding->Bind(*stmt, i++);

        IdSet<ECClassId> classesIds;
        while (DbResult::BE_SQLITE_ROW == stmt->Step())
            classesIds.insert(stmt->GetValueId<ECClassId>(0));

        for (BoundQueryValue const* binding : bindings)
            delete binding;

        return classesIds;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    SupportedClassesResolver(IConnectionCR connection, ECSchemaSet const& supportedSchemas,
        SupportedEntityClassInfos const* classInfos, SupportedRelationshipClassInfos const* relationshipInfos)
        : m_flags(0), m_connection(connection)
        {
        m_schemaList = supportedSchemas;
        m_classInfos = classInfos;
        m_relationshipInfos = relationshipInfos;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool GetAcceptAllIncludeClasses() const {return GetIncludedEntityClassIds().empty() && nullptr == m_classInfos;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool GetAcceptAllRelationships() const {return GetRelationshipClassIds().empty() && nullptr == m_relationshipInfos;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    SupportedEntityClassInfo const* GetInfo(ECEntityClassCR ecClass) const
        {
        if (nullptr == m_classInfos)
            return nullptr;
        auto iter = m_classInfos->find(SupportedEntityClassInfo(ecClass));
        if (m_classInfos->end() != iter)
            return &*iter;
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    SupportedRelationshipClassInfo const* GetInfo(ECRelationshipClassCR ecClass) const
        {
        if (nullptr == m_relationshipInfos)
            return nullptr;
        auto iter = m_relationshipInfos->find(SupportedRelationshipClassInfo(ecClass));
        if (m_relationshipInfos->end() != iter)
            return &*iter;
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
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
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> const& GetIncludedEntityClassIds() const
        {
        if (0 == (m_flags & DeterminedIncludedEntityClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (SupportedEntityClassInfo const& info : *m_classInfos)
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
    * @bsimethod                                    Grigas.Petraitis                07/2016
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
    * @bsimethod                                    Saulius.Skliutas                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> const& GetPolymorphicallyIncludedClassIds() const
        {
        if (0 == (m_flags & DeterminedPolymorphicallyIncludedClasses))
            {
            m_polymorphicallyIncludedClassIds = DeterminePolymorphicallyIncludedClassesIds();
            m_flags |= DeterminedPolymorphicallyIncludedClasses;
            }
        return m_polymorphicallyIncludedClassIds;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                12/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ECClassCP> const& GetExcludedClasses() const
        {
        if (0 == (m_flags & DeterminedExcludedEntityClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (SupportedEntityClassInfo const& info : *m_classInfos)
                    {
                    if (info.IsExclude() && !info.IsPolymorphic())
                        m_excludedClasses.push_back(&info.GetClass());
                    }
                }
            m_flags |= DeterminedExcludedEntityClasses;
            }
        return m_excludedClasses;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                12/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<ECClassCP> const& GetPolymorphicallyExcludedClasses() const
        {
        if (0 == (m_flags & DeterminedPolymorphicallyExcludedClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (SupportedEntityClassInfo const& info : *m_classInfos)
                    {
                    if (info.IsExclude() && info.IsPolymorphic())
                        m_polymorphicallyExcludedClasses.push_back(&info.GetClass());
                    }
                }
            m_flags |= DeterminedPolymorphicallyExcludedClasses;
            }
        return m_polymorphicallyExcludedClasses;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
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
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeBindings(BoundQueryValuesList& target, BoundQueryValuesList&& source)
    {
    std::move(source.begin(), source.end(), std::back_inserter(target));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static QueryClauseAndBindings CreateRelationshipPathsQuery(ECSchemaHelper::SupportedClassesResolver const& resolver,
    bset<ECClassId> const& sourceClassIds, int relationshipDirection, int depth)
    {
    BoundQueryValuesList bindings;
    Utf8String query = ""
        "SELECT [start].[ECInstanceId] startClassId, "
        "       [relationship].[ECInstanceId] relationshipClassId, "
        "       [endBase].[ECInstanceId] baseRelatedClassId, "
        "       [end].[ECInstanceId] actualRelatedClassId, "
        "       [startConstraint].[RelationshipEnd] relationshipEnd "

        "  FROM [meta].[ECClassDef] start "
        "  JOIN [meta].[ClassHasAllBaseClasses] startBaseRel ON [startBaseRel].[SourceECInstanceId] = [start].[ECInstanceId] "
        "  JOIN [meta].[ECClassDef] startBase ON [startBase].[ECInstanceId] = [startBaseRel].[TargetECInstanceId] "

        "  JOIN [meta].[RelationshipConstraintHasClasses] startConstraintRel ON [startConstraint].[IsPolymorphic] AND [startConstraintRel].[TargetECInstanceId] = [startBase].[ECInstanceId] "
        "       OR NOT [startConstraint].[IsPolymorphic] AND [startConstraintRel].[TargetECInstanceId] = [start].[ECInstanceId] AND [start].[ECInstanceId] = [startBase].[ECInstanceId] "
        "  JOIN [meta].[ECRelationshipConstraintDef] startConstraint ON [startConstraint].[ECInstanceId] = [startConstraintRel].[SourceECInstanceId] "

        "  JOIN [meta].[ECClassDef] [end] ON 1 = 1 "
        "  JOIN [meta].[ClassHasAllBaseClasses] endBaseRel ON [endBaseRel].[SourceECInstanceId] = [end].[ECInstanceId] "
        "  JOIN [meta].[ECClassDef] endBase ON [endBase].[ECInstanceId] = [endBaseRel].[TargetECInstanceId] "

        "  JOIN [meta].[RelationshipConstraintHasClasses] endConstraintRel ON [endConstraint].[IsPolymorphic] AND [endConstraintRel].[TargetECInstanceId] = [endBase].[ECInstanceId] "
        "       OR NOT [endConstraint].[IsPolymorphic] AND [endConstraintRel].[TargetECInstanceId] = [endBase].[ECInstanceId] AND [end].[ECInstanceId] = [endBase].[ECInstanceId] "
        "  JOIN [meta].[ECRelationshipConstraintDef] endConstraint ON [endConstraint].[ECInstanceId] = [endConstraintRel].[SourceECInstanceId] AND [endConstraint].[RelationshipEnd] <> [startConstraint].[RelationshipEnd] "

        "  JOIN [meta].[ECClassDef] relationship ON [relationship].[ECInstanceId] = [startConstraint].[RelationshipClass].[Id] "
        "       AND [relationship].[ECInstanceId] = [endConstraint].[RelationshipClass].[Id] "

        " WHERE ";

    bool hasCondition = false;

    if (depth <= 0)
        {
        if (!resolver.GetAcceptAllRelationships())
            {
            if (hasCondition)
                query.append(" AND ");
            IdsFilteringHelper<IdSet<ECClassId>> helper(resolver.GetRelationshipClassIds());
            query.append(helper.CreateWhereClause("[relationship].[ECInstanceId]"));
            MergeBindings(bindings, helper.CreateBoundValues());
            hasCondition = true;
            }

        if (!resolver.GetAcceptAllIncludeClasses())
            {
            if (hasCondition)
                query.append(" AND ");
            IdSet<ECClassId> const& ids = resolver.GetPolymorphicallyIncludedClassIds();
            IdsFilteringHelper<IdSet<ECClassId>> helper(ids);
            query.append(helper.CreateWhereClause("[end].[ECInstanceId]"));
            MergeBindings(bindings, helper.CreateBoundValues());
            hasCondition = true;
            }
        }

    if (hasCondition)
        query.append(" AND ");

    IdsFilteringHelper<bset<ECClassId>> sourceIdsHelper(sourceClassIds);
    query.append(sourceIdsHelper.CreateWhereClause("[start].[ECInstanceId]"));
    MergeBindings(bindings, sourceIdsHelper.CreateBoundValues());

    if (((int)ECRelatedInstanceDirection::Backward | (int)ECRelatedInstanceDirection::Forward) != relationshipDirection)
        {
        ECRelationshipEnd relationshipEnd = (ECRelatedInstanceDirection::Backward == (ECRelatedInstanceDirection)relationshipDirection)
            ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
        query.append(" AND [startConstraint].[RelationshipEnd] = ?");
        bindings.push_back(new BoundQueryECValue(ECValue((int)relationshipEnd)));
        }

    IdsFilteringHelper<IdSet<ECSchemaId>> schemaIdsHelper(resolver.GetSchemaIds());
    if (resolver.GetAcceptAllRelationships() && !resolver.GetSchemaIds().empty())
        {
        query.append(" AND ");
        query.append(schemaIdsHelper.CreateWhereClause("[relationship].[Schema].[Id]"));
        MergeBindings(bindings, schemaIdsHelper.CreateBoundValues());
        }
    if (resolver.GetAcceptAllIncludeClasses() && !resolver.GetSchemaIds().empty())
        {
        query.append(" AND ");
        query.append(schemaIdsHelper.CreateWhereClause("[end].[Schema].[Id]"));
        MergeBindings(bindings, schemaIdsHelper.CreateBoundValues());
        }

    query.append(" ORDER BY [start].[ECInstanceId], [end].[ECInstanceId], [relationship].[ECInstanceId]");

    return QueryClauseAndBindings(query, bindings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoesPathContainAnotherPath(RelatedClassPath const& containingPath, RelatedClassPath const& candidatePath, bool handleTargetClassPolymorphically)
    {
    for (size_t i = 0; i < containingPath.size(); ++i)
        {
        if (!candidatePath[i].GetRelationship()->Is(containingPath[i].GetRelationship())
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
            if (candidatePath[i].GetTargetClass() != containingPath[i].GetTargetClass())
                return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
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
* @bsimethod                                    Grigas.Petraitis                04/2017
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
    if (!resolver.GetAcceptAllRelationships() && !relationshipIds.Contains(related.GetRelationship()->GetId()))
        {
        // is specific relationships are requested, make sure the candidate is in that list
        return false;
        }
    if (resolver.GetAcceptAllRelationships() && !resolver.GetSchemaIds().Contains(related.GetRelationship()->GetSchema().GetId()))
        {
        // if any relationship is okay, make sure the candidate relationship is from supported schemas list
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRelationshipAlias(ECClassCR relationship, ECClassUseCounter& counter)
    {
    Utf8String alias("rel_");
    alias.append(relationship.GetSchema().GetAlias()).append("_");
    alias.append(relationship.GetName()).append("_");
    alias.append(std::to_string(counter.Inc(&relationship)).c_str());
    return alias;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateTargetClassAlias(ECClassCR relationship, ECClassUseCounter& counter)
    {
    Utf8String alias("target_");
    alias.append(relationship.GetSchema().GetAlias()).append("_");
    alias.append(relationship.GetName()).append("_");
    alias.append(std::to_string(counter.Inc(&relationship)).c_str());
    return alias;
    }

/*---------------------------------------------------------------------------------**//**
* @deprecated In favor of the version with RelationshipPathSpecification. Left only to support deprecated cases.
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::GetPaths(bvector<RelatedClassPath>& paths, ECClassUseCounter& relationshipsUseCounter,
    bset<RelatedClass>& usedRelationships, SupportedClassesResolver const& resolver, bset<ECClassId> const& sourceClassIds,
    bool handleRelatedClassesPolymorphically, int relationshipDirection, int depth, ECEntityClassCP targetClass) const
    {
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetPaths");
    BeAssert(txn.IsActive());

    QueryClauseAndBindings clause = CreateRelationshipPathsQuery(resolver, sourceClassIds, relationshipDirection, depth);
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), clause.GetClause().c_str());
    if (!stmt.IsValid())
        {
        BeAssert(false);
        return;
        }
    uint32_t index = 1;
    for (BoundQueryValue const* binding : clause.GetBindings())
        binding->Bind(*stmt, index++);

    bvector<RelatedClass> specs;
    bset<ECClassId> relatedClassIds;

    // get the results
    while (DbResult::BE_SQLITE_ROW == stmt->Step())
        {
        ECEntityClassCP source = GetECClass(stmt->GetValueId<ECClassId>(0))->GetEntityClassCP();
        ECRelationshipClassCP relationship = GetECClass(stmt->GetValueId<ECClassId>(1))->GetRelationshipClassCP();
        ECEntityClassCP actualRelated = GetECClass(stmt->GetValueId<ECClassId>(3))->GetEntityClassCP();
        if (nullptr == source || nullptr == relationship || nullptr == actualRelated)
            {
            BeAssert(false);
            continue;
            }
        bool isForward = (ECRelationshipEnd_Source == (ECRelationshipEnd)stmt->GetValueInt(4));

        // filter by target class (if specified)
        if (targetClass && targetClass->GetEntityClassCP() && targetClass->Is(actualRelated))
            actualRelated = targetClass->GetEntityClassCP();
        else if (targetClass && !actualRelated->Is(targetClass) || IsClassHidden(*actualRelated) && !resolver.GetInfo(*actualRelated))
            continue;

        SupportedEntityClassInfo const* info = resolver.GetInfo(*actualRelated);
        bool isTargetPolymorphic = (nullptr == info || info->IsPolymorphic()) && handleRelatedClassesPolymorphically;
        RelatedClass relatedClassSpec(*source, SelectClass(*actualRelated, isTargetPolymorphic), *relationship, isForward);
        relatedClassSpec.SetRelationshipAlias(CreateRelationshipAlias(*relationship, relationshipsUseCounter));
        relatedClassSpec.SetTargetClassAlias(CreateTargetClassAlias(*actualRelated, relationshipsUseCounter));

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
        GetPaths(subPaths, relationshipsUseCounter, usedRelationships, resolver, relatedClassIds,
            handleRelatedClassesPolymorphically, relationshipDirection, depth - 1, targetClass);
        for (RelatedClassPath const& subPath : subPaths)
            {
            for (RelatedClassCR sourceClassSpec : specs)
                {
                if (&sourceClassSpec.GetTargetClass().GetClass() != subPath[0].GetSourceClass())
                    continue;

                RelatedClassPath aggregatedPath = subPath;
                aggregatedPath.insert(aggregatedPath.begin(), sourceClassSpec);
                AppendPath(paths, aggregatedPath, handleRelatedClassesPolymorphically);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyExcludedClasses(bvector<RelatedClassPath>& paths, bvector<ECClassCP> const& excludedClasses, bool excludePolymorphically, SchemaManagerCR schemas)
    {
    for (RelatedClassPathR path : paths)
        {
        RelatedClassR targetClass = path.back();
        for (ECClassCP excludedClass : excludedClasses)
            {
            if (excludedClass->Is(&targetClass.GetTargetClass().GetClass()))
                targetClass.GetTargetClass().GetDerivedExcludedClasses().push_back(SelectClass(*excludedClass, excludePolymorphically));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @deprecated In favor of the version with RelationshipPathSpecification. Left only to support deprecated cases.
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetRelationshipClassPaths(RelationshipClassPathOptions const& options) const
    {
    // look for cached results first
    BeMutexHolder lock(m_relatedPathsCache->GetMutex());
    RelatedPathsCacheDeprecated::Key key(options);
    RelatedPathsCacheDeprecated::Result const* cacheResult = m_relatedPathsCacheDeprecated->Get(key);
    if (nullptr != cacheResult)
        {
        options.m_relationshipsUseCounter.Merge(cacheResult->m_relationshipCounter);
        return cacheResult->m_paths;
        }
    lock.unlock();

    bvector<RelatedClassPath> paths;
    SupportedEntityClassInfos classInfos = GetECClassesFromClassList(options.m_supportedClasses, true);
    SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(options.m_supportedRelationships);
    SupportedClassesResolver resolver(m_connection, GetECSchemas(options.m_supportedSchemas),
        (classInfos.empty() && 0 == *options.m_supportedClasses) ? nullptr : &classInfos,
        (relationshipInfos.empty() && 0 == *options.m_supportedRelationships) ? nullptr : &relationshipInfos);

    bset<RelatedClass> usedRelationships;
    bset<ECClassId> sourceClassIds;
    sourceClassIds.insert(options.m_sourceClass.GetId());

    GetPaths(paths, options.m_relationshipsUseCounter, usedRelationships, resolver, sourceClassIds,
        options.m_handleRelatedClassesPolymorphically, options.m_relationshipDirection, options.m_depth, options.m_targetClass);

    ApplyExcludedClasses(paths, resolver.GetExcludedClasses(), false, m_connection.GetECDb().Schemas());
    ApplyExcludedClasses(paths, resolver.GetPolymorphicallyExcludedClasses(), true, m_connection.GetECDb().Schemas());

    // cache
    m_relatedPathsCacheDeprecated->Put(key, RelatedPathsCacheDeprecated::Result(paths, options.m_relationshipsUseCounter));
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetPaths(ECClassId sourceClassId, bvector<RepeatableRelationshipStepSpecification> pathSpecification,
    bset<RelatedClass>& usedRelationships, ECClassUseCounter& relationshipsUseCounter, bool handleRelatedClassesPolymorphically) const
    {
    bvector<RelatedClassPath> paths;
    if (pathSpecification.empty())
        return paths;

    RepeatableRelationshipStepSpecification stepSpecification = pathSpecification.front();
    pathSpecification.erase(pathSpecification.begin());

    SupportedEntityClassInfos classInfos = GetECClassesFromClassList(stepSpecification.GetTargetClassName(), false);
    SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(stepSpecification.GetRelationshipClassName());
    SupportedClassesResolver resolver(m_connection, ECSchemaSet(),
        (classInfos.empty() && stepSpecification.GetTargetClassName().empty()) ? nullptr : &classInfos,
        (relationshipInfos.empty() && stepSpecification.GetRelationshipClassName().empty()) ? nullptr : &relationshipInfos);
    ECClassCP targetClass = stepSpecification.GetTargetClassName().empty() ? nullptr : GetECClass(stepSpecification.GetTargetClassName().c_str());

    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetPaths");
    BeAssert(txn.IsActive());

    QueryClauseAndBindings clause = CreateRelationshipPathsQuery(resolver, ContainerHelpers::Create<bset<ECClassId>>(sourceClassId), stepSpecification.GetRelationDirection(), 0);
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), clause.GetClause().c_str());
    if (!stmt.IsValid())
        {
        BeAssert(false);
        return paths;
        }
    clause.Bind(*stmt);

    // get the results
    while (DbResult::BE_SQLITE_ROW == stmt->Step())
        {
        ECEntityClassCP source = GetECClass(stmt->GetValueId<ECClassId>(0))->GetEntityClassCP();
        ECRelationshipClassCP relationship = GetECClass(stmt->GetValueId<ECClassId>(1))->GetRelationshipClassCP();
        ECEntityClassCP actualRelated = GetECClass(stmt->GetValueId<ECClassId>(3))->GetEntityClassCP();
        if (nullptr == source || nullptr == relationship || nullptr == actualRelated)
            {
            BeAssert(false);
            continue;
            }
        bool isForward = (ECRelationshipEnd_Source == (ECRelationshipEnd)stmt->GetValueInt(4));

        // filter by target class (if specified)
        if (targetClass && targetClass->GetEntityClassCP() && targetClass->Is(actualRelated))
            actualRelated = targetClass->GetEntityClassCP();
        else if (targetClass && !actualRelated->Is(targetClass) || IsClassHidden(*actualRelated) && !resolver.GetInfo(*actualRelated))
            continue;

        SupportedEntityClassInfo const* info = resolver.GetInfo(*actualRelated);
        bool isTargetPolymorphic = (nullptr == info || info->IsPolymorphic()) && handleRelatedClassesPolymorphically;
        RelatedClass relatedClassSpec(*source, SelectClass(*actualRelated, isTargetPolymorphic), *relationship, isForward);
        relatedClassSpec.SetRelationshipAlias(CreateRelationshipAlias(*relationship, relationshipsUseCounter));
        relatedClassSpec.SetTargetClassAlias(CreateTargetClassAlias(*actualRelated, relationshipsUseCounter));
        RelatedClassPath path{relatedClassSpec};

        if (stepSpecification.GetCount() > 1)
            {
            if (!actualRelated->Is(isForward ? relationship->GetSource().GetAbstractConstraint() : relationship->GetTarget().GetAbstractConstraint()))
                {
                BeAssert(false);
                }
            else
                {
                for (int i = 1; i < stepSpecification.GetCount(); ++i)
                    {
                    RelatedClass repeatedStep(relatedClassSpec);
                    repeatedStep.SetSourceClass(*actualRelated);
                    repeatedStep.SetRelationshipAlias(CreateRelationshipAlias(*relationship, relationshipsUseCounter));
                    repeatedStep.SetTargetClassAlias(CreateRelationshipAlias(*actualRelated, relationshipsUseCounter));
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
                {
                RelatedClassPath aggregatedPath = path;
                ContainerHelpers::Push(aggregatedPath, subPath);
                AppendPath(paths, aggregatedPath, handleRelatedClassesPolymorphically);
                }
            }
        }

    txn.Cancel();
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
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
    m_relatedPathsCache->Put(key, RelatedPathsCache::Result(paths, options.m_relationshipsUseCounter));
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetRecursiveRelationshipClassPaths(ECSchemaHelper const& schemaHelper, ECClassCR inputClass, bvector<ECInstanceId> const& inputIds,
    bvector<RepeatableRelationshipStepSpecification*> const& pathSteps, ECClassUseCounter& relationshipsUseCount, bool mergePolymorphicPaths)
    {
    ECClassCP sourceClass = &inputClass;
    bset<ECInstanceId> sourceIds = ContainerHelpers::TransformContainer<bset<ECInstanceId>>(inputIds);
    bool hadRecursiveRelationships = false;
    RepeatableRelationshipPathSpecification intermediatePathSpec;
    bvector<RelatedClassPath> intermediatePaths;
    for (RepeatableRelationshipStepSpecification const* stepSpec : pathSteps)
        {
        if (stepSpec->GetCount() == 0)
            {
            if (!intermediatePathSpec.GetSteps().empty())
                {
                // find paths for the intermediate path specification
                ECSchemaHelper::RepeatableMultiRelationshipPathOptions intermediatePathOptions(*sourceClass, intermediatePathSpec, true, relationshipsUseCount);
                bvector<RelatedClassPath> intermediateSpecPaths = schemaHelper.GetRelationshipClassPaths(intermediatePathOptions);
                if (intermediateSpecPaths.empty())
                    {
                    BeAssert(false && "Relationship path specification didn't result to a valid path. Is the specification correct?");
                    break;
                    }
                BeAssert(1 == intermediateSpecPaths.size());
                RelatedClassCR intermediateTargetClass = intermediateSpecPaths.back().back();
                // get target ids for the intermediate path
                sourceIds = schemaHelper.GetTargetIds(intermediateSpecPaths, sourceIds);
                sourceClass = intermediateTargetClass.IsForwardRelationship() ? &intermediateTargetClass.GetTargetClass().GetClass() : intermediateTargetClass.GetSourceClass();
                }
            // recursively find target ids using the recursive path specification
            RelationshipPathSpecification recursiveStepSpec(*new RelationshipStepSpecification(*stepSpec));
            ECSchemaHelper::MultiRelationshipPathOptions recursiveStepOptions(*sourceClass, recursiveStepSpec, true, relationshipsUseCount);
            bvector<RelatedClassPath> recursiveStepPaths = schemaHelper.GetRelationshipClassPaths(recursiveStepOptions);
            BeAssert(1 == recursiveStepPaths.size());
            RelatedClassCR recursiveStepTargetClass = recursiveStepPaths.back().back();
            InstanceFilteringParams::RecursiveQueryInfo recursiveQueryInfo(recursiveStepPaths);
            ContainerHelpers::Push(sourceIds, RecursiveQueriesHelper(schemaHelper.GetConnection(), recursiveQueryInfo).GetRecursiveChildrenIds(sourceIds));
            sourceClass = recursiveStepTargetClass.IsForwardRelationship() ? &recursiveStepTargetClass.GetTargetClass().GetClass() : recursiveStepTargetClass.GetSourceClass();
            // clear intermediate path
            intermediatePathSpec.ClearSteps();
            hadRecursiveRelationships = true;
            }
        else
            {
            intermediatePathSpec.AddStep(*new RepeatableRelationshipStepSpecification(*stepSpec));
            }
        }

    bvector<RelatedClassPath> paths;
    if (!intermediatePathSpec.GetSteps().empty())
        {
        ECSchemaHelper::RepeatableMultiRelationshipPathOptions endingPathOptions(*sourceClass, intermediatePathSpec, mergePolymorphicPaths, relationshipsUseCount);
        paths = schemaHelper.GetRelationshipClassPaths(endingPathOptions);
        }

    if (!hadRecursiveRelationships)
        return paths;

    if (sourceIds.empty())
        return bvector<RelatedClassPath>();

    RelatedClass pathsPrefix(inputClass, *sourceClass, sourceIds, "related");
    if (paths.empty())
        return {RelatedClassPath{pathsPrefix}};

    for (RelatedClassPathR path : paths)
        path.insert(path.begin(), pathsPrefix);
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetRecursiveRelationshipClassPaths(ECClassCR sourceClass, bvector<ECInstanceId> const& sourceIds,
    bvector<RepeatableRelationshipPathSpecification*> const& relationshipPathSpecs, ECClassUseCounter& relationshipsUseCount, bool mergePolymorphicPaths) const
    {
    bvector<RelatedClassPath> paths;
    for (RepeatableRelationshipPathSpecification const* pathSpec : relationshipPathSpecs)
        {
        ContainerHelpers::Push(paths, ::GetRecursiveRelationshipClassPaths(*this, sourceClass, sourceIds,
            pathSpec->GetSteps(), relationshipsUseCount, mergePolymorphicPaths));
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
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
        {
        BeAssert(false);
        return bvector<ECClassCP>();
        }

    ECSchemaSet supportedSchemas = GetECSchemas(supportedSchemasStr);
    ECRelationshipConstraintClassList supportedClasses;
    for (ECClassCP ecClass : *classes)
        {
        if (!IsAllowed(*ecClass))
            continue;

        if (supportedSchemas.end() == supportedSchemas.find(&ecClass->GetSchema()))
            continue;

        supportedClasses.push_back(ecClass);
        }
    return supportedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ECClassCP GetClassFromRelationshipConstraint(ECRelationshipConstraint const& constraint)
    {
    // no idea which one to return when there're multiple constraint classes
    return constraint.GetConstraintClasses().front();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedClass ECSchemaHelper::GetForeignKeyClass(ECPropertyCR prop) const
    {
    if (!prop.GetIsNavigation())
        return RelatedClass();

    NavigationECPropertyCR navigationProp = *prop.GetAsNavigationProperty();
    ECRelationshipClassCP relationship = navigationProp.GetRelationshipClass();
    bool isRelationshipForward = (navigationProp.GetDirection() == ECRelatedInstanceDirection::Forward);
    ECClassCP sourceClass = GetClassFromRelationshipConstraint(isRelationshipForward ? relationship->GetSource() : relationship->GetTarget());
    ECClassCP targetClass = GetClassFromRelationshipConstraint(isRelationshipForward ? relationship->GetTarget() : relationship->GetSource());
    if (nullptr != sourceClass && nullptr != targetClass)
        return RelatedClass(*sourceClass, *targetClass, *relationship, isRelationshipForward);

    BeAssert(false);
    return RelatedClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<SelectClass> GetAllDerivedClasses(ECClassCR baseClass)
    {
    bvector<SelectClass> derivedClasses;
    for (ECClassCP derivedClass : baseClass.GetDerivedClasses())
        {
        bvector<SelectClass> nestedDerivedClasses = GetAllDerivedClasses(*derivedClass);
        derivedClasses.push_back(SelectClass(*derivedClass, false));
        ContainerHelpers::Push(derivedClasses, nestedDerivedClasses);
        }
    return derivedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClass> ECSchemaHelper::GetPolymorphicallyRelatedClasses(RelatedClassCR relatedClass, ECClassUseCounter& relationshipsUseCounter) const
    {
#ifdef wip_check_if_necessary
    PolymorphicallyRelatedClassesCache::Key key = {&sourceClass, direction, relationships, targetBaseClasses};
    BeMutexHolder lock(m_polymorphicallyRelatedClassesCache->GetMutex());
    bvector<RelatedClass> const* polymorphicallyRelatedClasses = m_polymorphicallyRelatedClassesCache->Get(key);
    if (!polymorphicallyRelatedClasses)
        {
#endif
        bvector<RelatedClass> vec;

        RelatedClass baseRelatedClass(relatedClass);
        baseRelatedClass.GetTargetClass().SetIsSelectPolymorphic(false);
        vec.push_back(baseRelatedClass);

        bvector<SelectClass> derivedClasses = GetAllDerivedClasses(relatedClass.GetTargetClass().GetClass());
        for (SelectClass derivedClass : derivedClasses)
            {
            if (IsClassHidden(derivedClass.GetClass()))
                continue;

            RelatedClass derivedRelatedClass(relatedClass);
            derivedRelatedClass.SetTargetClass(derivedClass);
            derivedRelatedClass.SetTargetClassAlias(CreateTargetClassAlias(derivedClass.GetClass(), relationshipsUseCounter));
            derivedRelatedClass.SetRelationshipAlias(CreateRelationshipAlias(*relatedClass.GetRelationship(), relationshipsUseCounter));
            vec.push_back(derivedRelatedClass);
            }
        return vec;

#ifdef wip_check_if_necessary
        polymorphicallyRelatedClasses = &m_polymorphicallyRelatedClassesCache->Add(key, std::move(vec));
        }
    return *polymorphicallyRelatedClasses;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
enum class RelationshipsJoinIdType
    {
    ECClassId,
    ECInstanceId,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
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
            query.From(*r.GetRelationship(), true, joinAlias.c_str());
            }
        else
            {
            Utf8PrintfString joinClause("[%s].[%s] = [%s].[%s]",
                joinAlias.c_str(), r.IsForwardRelationship() ? sourceId : targetId,
                prevJoinAlias.c_str(), prevJoinWasForward ? targetId : sourceId);
            query.Join(SelectClass(*r.GetRelationship(), true), joinAlias.c_str(), QueryClauseAndBindings(joinClause), false);
            }
        prevJoinAlias = joinAlias;
        prevJoinWasForward = r.IsForwardRelationship();
        }
    if (nullptr != lastJoinAlias)
        lastJoinAlias->AssignOrClear(prevJoinAlias.c_str());
    if (nullptr != wasLastJoinForward)
        *wasLastJoinForward = prevJoinWasForward;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchemaHelper::DoesRelatedPropertyPathHaveInstances(RelatedClassPathCR pathToSelectClass, RelatedClassPathCR pathFromSelectToPropertyClass,
    InstanceFilteringParams const* filteringParams) const
    {
    if (pathFromSelectToPropertyClass.empty())
        return false;

    ComplexGenericQueryPtr filteringQuery;
    if (nullptr != filteringParams)
        {
        Utf8String filteringSelectAlias = (pathToSelectClass.empty()) ? "this" : "nestedRel";
        filteringQuery = ComplexGenericQuery::Create();
        filteringQuery->SelectContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("ECInstanceId", "ECInstanceId")), filteringSelectAlias.c_str());
        filteringQuery->From(*pathFromSelectToPropertyClass.front().GetSourceClass(), true, filteringSelectAlias.c_str());
        InstanceFilteringResult applyFilterResult = QueryBuilderHelpers::ApplyInstanceFilter(*filteringQuery, *filteringParams, RelatedClassPath(pathToSelectClass).Reverse("", false));
        switch (applyFilterResult)
            {
            case InstanceFilteringResult::Success:
                for (RelatedClassPathCR relatedInstancePath : filteringParams->GetSelectInfo().GetRelatedInstancePaths())
                    filteringQuery->Join(relatedInstancePath);
                break;
            case InstanceFilteringResult::NoFilter:
                filteringQuery = nullptr;
                break;
            case InstanceFilteringResult::NoResults:
                return false;
            }
        }

    RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create(
        {
        PresentationQueryContractSimpleField::Create("", "1", false),
        });
    ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
    query->SelectContract(*contract, "r0");

    Utf8String lastJoinAlias;
    bool wasLastJoinForward;
    JoinRelationships(*query, pathFromSelectToPropertyClass, RelationshipsJoinIdType::ECInstanceId, &lastJoinAlias, &wasLastJoinForward);

    query->Where(Utf8PrintfString("[%s].[%s] = ?", lastJoinAlias.c_str(), wasLastJoinForward ? "TargetECClassId" : "SourceECClassId").c_str(),
        {new BoundQueryId(pathFromSelectToPropertyClass.back().GetTargetClass().GetClass().GetId())});
    if (filteringQuery.IsValid())
        {
        query->Where(Utf8PrintfString("[%s].[%s] IN (%s)", "r0", pathFromSelectToPropertyClass.front().IsForwardRelationship() ? "SourceECInstanceId" : "TargetECInstanceId", filteringQuery->ToString().c_str()).c_str(),
            filteringQuery->GetBoundValues().Clone());
        }

    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(),
        m_connection.GetDb(), query->ToString().c_str());
    if (stmt.IsNull())
        {
        BeAssert(false);
        return false;
        }
    query->BindValues(*stmt);
    return (BE_SQLITE_ROW == stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECInstanceId> ECSchemaHelper::GetTargetIds(bvector<RelatedClassPath> const& paths, bset<ECInstanceId> const& sourceIds) const
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
        contract->AddField(*PresentationQueryContractSimpleField::Create("SourceId", path.front().IsForwardRelationship() ? "SourceECInstanceId" : "TargetECInstanceId"));
        query->SelectContract(*contract, "r0");
        QueryBuilderHelpers::SetOrUnion<GenericQuery>(nestedQuery, *query);
        }

    ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
    query->SelectAll();
    query->From(*nestedQuery);
    query->Where(IdsFilteringHelper<bset<ECInstanceId>>(sourceIds).Create("[SourceId]"));

    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(),
        m_connection.GetDb(), query->ToString().c_str());
    if (stmt.IsNull())
        {
        BeAssert(false);
        return bset<ECInstanceId>();
        }

    query->BindValues(*stmt);

    bset<ECInstanceId> targetIds;
    while (BE_SQLITE_ROW == stmt->Step())
        targetIds.insert(stmt->GetValueId<ECInstanceId>(0));
    return targetIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String, bvector<RelatedClassPath>> ECSchemaHelper::GetRelatedInstancePaths(ECClassCR selectClass,
    RelatedInstanceSpecificationList const& relatedInstanceSpecs, ECClassUseCounter& relationshipsUseCount) const
    {
    bmap<Utf8String, bvector<RelatedClassPath>> paths;
    for (RelatedInstanceSpecificationCP spec : relatedInstanceSpecs)
        {
        if (paths.end() != paths.find(spec->GetAlias()))
            {
            BeAssert(false && "related instance alias must be unique per parent specification");
            continue;
            }

        ECSchemaHelper::MultiRelationshipPathOptions options(selectClass, spec->GetRelationshipPath(), true, relationshipsUseCount);
        bvector<RelatedClassPath> specPaths = GetRelationshipClassPaths(options);
        for (RelatedClassPathR specPath : specPaths)
            {
            for (RelatedClassR specPathClass : specPath)
                specPathClass.SetIsTargetOptional(!spec->IsRequired());
            if (!specPath.empty())
                specPath.back().SetTargetClassAlias(spec->GetAlias().c_str());
            }
        if (!specPaths.empty())
            paths.Insert(spec->GetAlias(), specPaths);
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SupportedClassesParser::Parse(ECSchemaHelper const& helper, Utf8StringCR str, bool supportExclusion)
    {
    ClassNamesParser names(str.c_str(), supportExclusion);
    for (auto entry : names)
        {
        ECSchemaCP ecSchema = helper.GetSchema(entry.GetSchemaName());
        if (nullptr == ecSchema)
            {
            BeAssert(false);
            continue;
            }
        ECClassCP ecClass = ecSchema->GetClassCP(entry.GetClassName());
        if (nullptr == ecClass)
            {
            // TODO: VSTS#210508
            // BeAssert(false);
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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECInstancesHelper::LoadInstance(IECInstancePtr& instance, IConnectionCR connection, ECInstanceKeyCR key)
    {
    ECClassCP selectClass = connection.GetECDb().Schemas().GetClass(key.GetClassId());
    if (nullptr == selectClass || !selectClass->IsEntityClass())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    Savepoint txn(connection.GetDb(), "ECInstancesHelper::LoadInstance");
    BeAssert(txn.IsActive());

    Utf8String ecsql("SELECT * FROM ONLY ");
    ecsql.append(selectClass->GetECSqlName()).append(" WHERE ECInstanceId=?");
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), ecsql.c_str());
    if (stmt.IsNull())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    if (ECSqlStatus::Success != stmt->BindId(1, key.GetInstanceId()))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    ECInstanceECSqlSelectAdapter adapter(*stmt);
    DbResult result = stmt->Step();
    if (DbResult::BE_SQLITE_ROW == result)
        {
        BeMutexHolder lock(s_getIntanceMutex);
        instance = adapter.GetInstance();
        }

    BeAssert(BE_SQLITE_ROW == result || BE_SQLITE_DONE == result || BE_SQLITE_INTERRUPT == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ECInstancesHelper::GetValue(IConnectionCR connection, ECInstanceKeyCR key, Utf8CP propertyName)
    {
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(key.GetClassId());
    if (nullptr == ecClass || !ecClass->IsEntityClass())
        {
        BeAssert(false);
        return ECValue();
        }
    ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
    if (nullptr == ecProperty || !ecProperty->GetIsPrimitive())
        {
        BeAssert(false);
        return ECValue();
        }
    return GetValue(connection, *ecClass, key.GetInstanceId(), *ecProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ECInstancesHelper::GetValue(IConnectionCR connection, ECClassCR ecClass, ECInstanceId instanceId, ECPropertyCR ecProperty)
    {
    Savepoint txn(connection.GetDb(), "ECInstancesHelper::GetValue");
    BeAssert(txn.IsActive());

    Utf8String ecsql("SELECT ");
    ecsql.append("[").append(ecProperty.GetName()).append("] ");
    ecsql.append("FROM ONLY ");
    ecsql.append(ecClass.GetECSqlName()).append(" WHERE ECInstanceId = ?");
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), ecsql.c_str());
    if (stmt.IsNull())
        {
        BeAssert(false);
        return ECValue();
        }

    if (ECSqlStatus::Success != stmt->BindId(1, instanceId))
        {
        BeAssert(false);
        return ECValue();
        }

    if (DbResult::BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        return ECValue();
        }

    return ValueHelpers::GetECValueFromSqlValue(ecProperty.GetAsPrimitiveProperty()->GetType(), stmt->GetValue(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstancesHelper::SetValue(IConnectionCR connection, ECInstanceKeyCR key, Utf8CP propertyName, ECValueCR value)
    {
    ECClassCP ecClass = connection.GetECDb().Schemas().GetClass(key.GetClassId());
    if (nullptr == ecClass || !ecClass->IsEntityClass())
        {
        BeAssert(false);
        return;
        }
    ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
    if (nullptr == ecProperty || !ecProperty->GetIsPrimitive())
        {
        BeAssert(false);
        return;
        }
    SetValue(connection, *ecClass, key.GetInstanceId(), *ecProperty, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstancesHelper::SetValue(IConnectionCR connection, ECClassCR ecClass, ECInstanceId instanceId, ECPropertyCR ecProperty, ECValueCR value)
    {
    Utf8String ecsql("UPDATE ");
    ecsql.append(ecClass.GetECSqlName()).append(" ");
    ecsql.append("SET ").append("[").append(ecProperty.GetName()).append("] = ?");
    ecsql.append(" WHERE ECInstanceId = ?");

    ECSqlStatus status;
    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), ecsql.c_str());
    if (stmt.IsNull())
        {
        BeAssert(false);
        return;
        }

    BoundQueryECValue bv(value);
    status = bv.Bind(*stmt, 1);
    BeAssert(status.IsSuccess());

    status = stmt->BindId(2, instanceId);
    BeAssert(status.IsSuccess());

    DbResult result = stmt->Step();
    BeAssert(DbResult::BE_SQLITE_DONE == result);
    UNUSED_VARIABLE(result);
    }
