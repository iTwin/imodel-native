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
RelatedPathsCache::Key::Key(ECSchemaHelper::RelationshipClassPathOptions const& options)
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
bool RelatedPathsCache::Key::operator<(Key const& other) const
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
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolymorphicallyRelatedClassesCache::Key::operator<(PolymorphicallyRelatedClassesCache::Key const& other) const
    {
    if (m_source < other.m_source)
        return true;
    if (m_source > other.m_source)
        return false;

    if (m_direction < other.m_direction)
        return true;
    if (m_direction > other.m_direction)
        return false;

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
    m_polymorphicallyRelatedClassesCache(polymorphicallyRelatedClassesCache), m_ownsPolymorphicallyRelatedClassesCache(nullptr == polymorphicallyRelatedClassesCache),
    m_ecexpressionsCache(ecexpressionsCache), m_ownsECExpressionsCache(nullptr == ecexpressionsCache)
    {
    if (nullptr == m_relatedPathsCache)
        m_relatedPathsCache = new RelatedPathsCache();
    if (nullptr == m_polymorphicallyRelatedClassesCache)
        m_polymorphicallyRelatedClassesCache = new PolymorphicallyRelatedClassesCache();
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
    if (m_ownsPolymorphicallyRelatedClassesCache)
        DELETE_AND_CLEAR(m_polymorphicallyRelatedClassesCache);
    if (m_ownsECExpressionsCache)
        DELETE_AND_CLEAR(m_ecexpressionsCache);
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
    SupportedClassNamesParser parser(*this, classListStr, supportExclusion);
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
    SupportedClassNamesParser parser(*this, relationshipListStr, false);
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
    mutable IdSet<ECClassId> m_excludedEntityClassIds;
    mutable IdSet<ECClassId> m_relationshipClassIds;
    mutable IdSet<ECClassId> m_polymorphicallyIncludedClassIds;
    mutable IdSet<ECClassId> m_polymorphicallyExcludedClassIds;
    SupportedEntityClassInfos const* m_classInfos;
    SupportedRelationshipClassInfos const* m_relationshipInfos;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                08/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddOrRemoveFlag(int flag, bool add)
        {
        if (add)
            m_flags |= flag;
        else
            m_flags &= ~flag;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void AddClassId(IdSet<ECClassId>& list, ECClassCR ecClass)
        {
        list.insert(ecClass.GetId());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> DeterminePolymorphicallySupportedClassesIds(bool include) const
        {
        Savepoint txn(m_connection.GetDb(), "SupportedClassesResolver::DeterminePolymorphicallySupportedClassesIds");
        BeAssert(txn.IsActive());

        IdSet<ECClassId> const& ids = include ? GetIncludedEntityClassIds() : GetExcludedEntityClassIds();
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
    bool GetAcceptAllExcludeClasses() const {return GetExcludedEntityClassIds().empty() && nullptr == m_classInfos;}

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
    IdSet<ECClassId> const& GetExcludedEntityClassIds() const
        {
        if (0 == (m_flags & DeterminedExcludedEntityClasses))
            {
            if (nullptr != m_classInfos)
                {
                for (SupportedEntityClassInfo const& info : *m_classInfos)
                    {
                    if (info.IsExclude())
                        m_excludedEntityClassIds.insert(info.GetClass().GetId());
                    }
                }
            m_flags |= DeterminedExcludedEntityClasses;
            }
        return m_excludedEntityClassIds;
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
            m_polymorphicallyIncludedClassIds = DeterminePolymorphicallySupportedClassesIds(true);
            m_flags |= DeterminedPolymorphicallyIncludedClasses;
            }
        return m_polymorphicallyIncludedClassIds;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> const& GetPolymorphicallyExcludedClassIds() const
        {
        if (0 == (m_flags & DeterminedPolymorphicallyExcludedClasses))
            {
            m_polymorphicallyExcludedClassIds = DeterminePolymorphicallySupportedClassesIds(false);
            m_flags |= DeterminedPolymorphicallyExcludedClasses;
            }
        return m_polymorphicallyExcludedClassIds;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool HasExcludes() const {return !GetExcludedEntityClassIds().empty();}
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
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldAcceptAllClasses(ECSchemaHelper::SupportedClassesResolver const& resolver, bool include)
    {
    return include && resolver.GetAcceptAllIncludeClasses() || !include && resolver.GetAcceptAllExcludeClasses();
    }

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
static BoundQueryValuesList CreateRelationshipPathsQuery(Utf8StringR query, ECSchemaHelper::SupportedClassesResolver const& resolver,
    bset<ECClassId> const& sourceClassIds, int relationshipDirection, int depth, bool include)
    {
    BoundQueryValuesList bindings;
    query = ""
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

        if (!ShouldAcceptAllClasses(resolver, include))
            {
            if (hasCondition)
                query.append(" AND ");
            IdSet<ECClassId> const& ids = include ? resolver.GetPolymorphicallyIncludedClassIds() : resolver.GetPolymorphicallyExcludedClassIds();
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
    if (resolver.GetAcceptAllRelationships())
        {
        query.append(" AND ");
        query.append(schemaIdsHelper.CreateWhereClause("[relationship].[Schema].[Id]"));
        MergeBindings(bindings, schemaIdsHelper.CreateBoundValues());
        }
    if (include && resolver.GetAcceptAllIncludeClasses())
        {
        query.append(" AND ");
        query.append(schemaIdsHelper.CreateWhereClause("[end].[Schema].[Id]"));
        MergeBindings(bindings, schemaIdsHelper.CreateBoundValues());
        }

    query.append(" ORDER BY [start].[ECInstanceId], [end].[ECInstanceId], [relationship].[ECInstanceId]");

    return bindings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoesPathContainAnotherPath(RelatedClassPath const& containingPath, RelatedClassPath const& candidatePath)
    {
    for (size_t i = 0; i < containingPath.size(); ++i)
        {
        if (!candidatePath[i].GetRelationship()->Is(containingPath[i].GetRelationship())
            || containingPath[i].IsForwardRelationship() != candidatePath[i].IsForwardRelationship())
            {
            return false;
            }

        if (!containingPath[i].IsPolymorphic() && candidatePath[i].GetTargetClass() != containingPath[i].GetTargetClass()
            || containingPath[i].IsPolymorphic() && !candidatePath[i].GetTargetClass()->Is(containingPath[i].GetTargetClass()))
            {
            return false;
            }

        if (!containingPath[i].IsPolymorphic() && candidatePath[i].GetSourceClass() != containingPath[i].GetSourceClass()
            || containingPath[i].IsPolymorphic() && !candidatePath[i].GetSourceClass()->Is(containingPath[i].GetSourceClass()))
            {
            return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPath(bvector<RelatedClassPath>& paths, RelatedClassPath const& path)
    {
    if (paths.size() > 0)
        {
        size_t index = paths.size();
        while (index > 0)
            {
            RelatedClassPath const& includedPath = paths[index - 1];

            // if path lengths are different, they can't be similar
            if (includedPath.size() != path.size())
                {
                --index;
                continue;
                }

            // the new path is already included polymorphically by another path
            if (DoesPathContainAnotherPath(includedPath, path))
                return;

            // the new path includes another (already included) path polymorphically -
            // the included path must be removed
            if (DoesPathContainAnotherPath(path, includedPath))
                {
                paths.erase(paths.begin() + index - 1);
                continue;
                }

            --index;
            }
        }

    // did not find any similar paths - simply append the new path
    paths.push_back(path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPath(bvector<bpair<RelatedClassPath, bool>>& paths, RelatedClassPath const& path, bool forInclude)
    {
    if (paths.size() > 0)
        {
        size_t index = paths.size();
        while (index > 0)
            {
            RelatedClassPath const& includedPath = paths[index - 1].first;
            bool isIncludedPathForInclude = paths[index - 1].second;

            // if path lengths are different, they can't be similar
            if (includedPath.size() != path.size())
                {
                --index;
                continue;
                }

            // if one path is included and another - excluded, they can't be similar
            if (forInclude != isIncludedPathForInclude)
                {
                --index;
                continue;
                }

            // the new path is already included polymorphically by another path
            if (DoesPathContainAnotherPath(includedPath, path))
                return;

            // the new path includes another (already included) path polymorphically -
            // the included path must be removed
            if (DoesPathContainAnotherPath(path, includedPath))
                {
                paths.erase(paths.begin() + index - 1);
                continue;
                }

            --index;
            }
        }

    // did not find any similar paths - simply append the new path
    paths.push_back(bpair<RelatedClassPath, bool>(path, forInclude));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MatchesPathRequest(RelatedClass const& related, ECSchemaHelper::SupportedClassesResolver const& resolver, bool include)
    {
    IdSet<ECClassId> const& classIds = include ? resolver.GetIncludedEntityClassIds() : resolver.GetExcludedEntityClassIds();
    if (!ShouldAcceptAllClasses(resolver, include) && !classIds.Contains(related.GetTargetClass()->GetId()))
        {
        // if specific classes are requested, make sure the candidate is in that list
        return false;
        }
    if (ShouldAcceptAllClasses(resolver, include) && !resolver.GetSchemaIds().Contains(related.GetTargetClass()->GetSchema().GetId()))
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
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::GetPaths(bvector<bpair<RelatedClassPath, bool>>& paths, bmap<ECRelationshipClassCP, int>& relationshipsUseCounter,
    bset<RelatedClass>& usedRelationships, SupportedClassesResolver const& resolver, bset<ECClassId> const& sourceClassIds, int relationshipDirection,
    int depth, ECEntityClassCP targetClass, bool include) const
    {
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetPaths");
    BeAssert(txn.IsActive());

    Utf8String query;
    BoundQueryValuesList bindings = CreateRelationshipPathsQuery(query, resolver, sourceClassIds, relationshipDirection, depth, include);
    CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(), m_connection.GetDb(), query.c_str());
    if (!stmt.IsValid())
        {
        BeAssert(false);
        return;
        }
    uint32_t index = 1;
    for (BoundQueryValue const* binding : bindings)
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
        bool isForward = !(ECRelationshipEnd_Source == (ECRelationshipEnd)stmt->GetValueInt(4)); // note: reverse the direction

        // filter by target class (if specified)
        if (nullptr != targetClass)
            {
            if (targetClass->Is(actualRelated))
                actualRelated = targetClass;
            else
                continue;
            }

        Utf8String relationshipAlias("rel_");
        relationshipAlias.append(relationship->GetSchema().GetAlias()).append("_");
        relationshipAlias.append(relationship->GetName()).append("_");
        relationshipAlias.append(std::to_string(relationshipsUseCounter[relationship]++).c_str());

        SupportedEntityClassInfo const* info = resolver.GetInfo(*actualRelated);
        RelatedClass relatedClassSpec(*source, *actualRelated, *relationship, isForward);
        relatedClassSpec.SetRelationshipAlias(relationshipAlias);
        relatedClassSpec.SetIsPolymorphic(nullptr == info || info->IsPolymorphic() && !(info->IsInclude() && info->IsExclude() && !include));

        // avoid recursive relationship loops
        if (usedRelationships.end() != usedRelationships.find(relatedClassSpec))
            continue;

        bool matchesRecursiveRequest = (depth < 0 && MatchesPathRequest(relatedClassSpec, resolver, include));

        if (0 == depth || matchesRecursiveRequest)
            AppendPath(paths, {relatedClassSpec}, include);

        if (depth > 0 || matchesRecursiveRequest)
            {
            specs.push_back(relatedClassSpec);
            usedRelationships.insert(relatedClassSpec);
            relatedClassIds.insert(actualRelated->GetId());
            }
        }

    txn.Cancel();

    for (BoundQueryValue const* binding : bindings)
        delete binding;

    if (0 != depth && !specs.empty())
        {
        bvector<bpair<RelatedClassPath, bool>> subPaths;
        GetPaths(subPaths, relationshipsUseCounter, usedRelationships, resolver, relatedClassIds,
            relationshipDirection, depth - 1, targetClass, include);
        for (auto& pair : subPaths)
            {
            RelatedClassPath const& subPath = pair.first;
            for (RelatedClassCR sourceClassSpec : specs)
                {
                if (sourceClassSpec.GetTargetClass() != subPath[0].GetSourceClass())
                    continue;

                RelatedClassPath aggregatedPath = subPath;
                aggregatedPath.insert(aggregatedPath.begin(), sourceClassSpec);
                AppendPath(paths, aggregatedPath, include);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::RelationshipClassPathOptions::RelationshipClassPathOptions(ECClassCR sourceClass, int relationshipDirection, int depth,
    Utf8CP supportedSchemas, Utf8CP supportedRelationships, Utf8CP supportedClasses,
    bmap<ECRelationshipClassCP, int>& relationshipsUseCounter, ECEntityClassCP targetClass)
    : m_relationshipsUseCounter(relationshipsUseCounter), m_sourceClass(sourceClass), m_supportedSchemas(supportedSchemas),
    m_supportedRelationships(supportedRelationships), m_supportedClasses(supportedClasses)
    {
    m_relationshipDirection = relationshipDirection;
    m_depth = depth;
    m_targetClass = targetClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<bpair<RelatedClassPath, bool>> ECSchemaHelper::GetRelationshipClassPaths(RelationshipClassPathOptions const& options) const
    {
    // look for cached results first
    BeMutexHolder lock(m_relatedPathsCache->GetMutex());
    RelatedPathsCache::Key key(options);
    RelatedPathsCache::Result const* cacheResult = m_relatedPathsCache->Get(key);
    if (nullptr != cacheResult)
        {
        for (auto const& pair : cacheResult->m_relationshipCounter)
            options.m_relationshipsUseCounter[pair.first] += pair.second;
        return cacheResult->m_paths;
        }
    lock.unlock();

    bvector<bpair<RelatedClassPath, bool>> paths;
    SupportedEntityClassInfos classInfos = GetECClassesFromClassList(options.m_supportedClasses, true);
    SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(options.m_supportedRelationships);
    SupportedClassesResolver resolver(m_connection, GetECSchemas(options.m_supportedSchemas),
        (classInfos.empty() && 0 == *options.m_supportedClasses) ? nullptr : &classInfos,
        (relationshipInfos.empty() && 0 == *options.m_supportedRelationships) ? nullptr : &relationshipInfos);

    bset<RelatedClass> usedRelationships;
    bset<ECClassId> sourceClassIds;
    sourceClassIds.insert(options.m_sourceClass.GetId());

    // get includes
    GetPaths(paths, options.m_relationshipsUseCounter, usedRelationships, resolver, sourceClassIds,
        options.m_relationshipDirection, options.m_depth, options.m_targetClass, true);

    if (resolver.HasExcludes())
        {
        // get excludes
        usedRelationships.clear();
        GetPaths(paths, options.m_relationshipsUseCounter, usedRelationships, resolver, sourceClassIds,
            options.m_relationshipDirection, options.m_depth, options.m_targetClass, false);
        }

    // cache
    m_relatedPathsCache->Put(key, RelatedPathsCache::Result(paths, options.m_relationshipsUseCounter));
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
    ECClassCP sourceClass = GetClassFromRelationshipConstraint((navigationProp.GetDirection() == ECRelatedInstanceDirection::Backward)
        ? relationship->GetTarget() : relationship->GetSource());
    ECClassCP targetClass = GetClassFromRelationshipConstraint((navigationProp.GetDirection() == ECRelatedInstanceDirection::Backward)
        ? relationship->GetSource() : relationship->GetTarget());
    if (nullptr != sourceClass && nullptr != targetClass)
        return RelatedClass(*sourceClass, *targetClass, *relationship, navigationProp.GetDirection() == ECRelatedInstanceDirection::Backward);

    BeAssert(false);
    return RelatedClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> ECSchemaHelper::GetPolymorphicallyRelatedClassesWithInstances(ECClassCR sourceClass, Utf8StringCR relationshipNames,
    ECRelatedInstanceDirection direction, Utf8StringCR baseClassNames, RelatedClassPathCR relatedClassPath, InstanceFilteringParams const* filteringParams) const
    {
    bvector<RelatedClassPath> paths;
    Savepoint txn(m_connection.GetDb(), "ECSchemaHelper::GetPolymorphicallyRelatedClassesWithInstances");
    BeAssert(txn.IsActive());

    bool isNested = !relatedClassPath.empty();
    ComplexGenericQueryPtr filteringQuery = ComplexGenericQuery::Create();
    filteringQuery->SelectContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("ECInstanceId", "ECInstanceId")), isNested ? "nestedRel" : "this");
    filteringQuery->From(sourceClass, true, isNested ? "nestedRel" : "this");
    if (nullptr != filteringParams)
        {
        if (InstanceFilteringResult::NoResults == QueryBuilderHelpers::ApplyInstanceFilter(*filteringQuery, *filteringParams, relatedClassPath))
            return paths;
        }
    BoundQueryValuesList filterBindings = filteringQuery->GetBoundValues();
    if (nullptr != filteringParams)
        {
        for (RelatedClass const& relatedInstanceClass : filteringParams->GetSelectInfo().GetRelatedInstanceClasses())
            filteringQuery->Join(relatedInstanceClass);
        }

    SupportedEntityClassInfos baseClassInfos = GetECClassesFromClassList(baseClassNames, false);
    bvector<ECEntityClassCP> baseClasses;
    std::transform(baseClassInfos.begin(), baseClassInfos.end(), std::back_inserter(baseClasses), [](SupportedEntityClassInfo const& i){return &i.GetClass();});
    bvector<ECRelationshipClassCP> relationships;
    if (relationshipNames.empty())
        {
        bmap<ECRelationshipClassCP, int> relationshipCounter;
        RelationshipClassPathOptions params(sourceClass, (int)direction, 0, "", "", baseClassNames.c_str(), relationshipCounter);
        bvector<bpair<RelatedClassPath, bool>> relationshipPaths = GetRelationshipClassPaths(params);
        for (bpair<RelatedClassPath, bool> const& entry : relationshipPaths)
            {
            if (!entry.second)
                continue;

            if (entry.first.size() != 1)
                {
                BeAssert(false);
                continue;
                }

            ECRelationshipClassCP relationship = entry.first[0].GetRelationship();
            if (relationships.end() == std::find(relationships.begin(), relationships.end(), relationship))
                relationships.push_back(relationship);
            }
        }
    else
        {
        SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(relationshipNames);
        std::transform(relationshipInfos.begin(), relationshipInfos.end(), std::back_inserter(relationships),
            [](SupportedRelationshipClassInfo const& info) { return &info.GetClass(); });
        }

    PolymorphicallyRelatedClassesCache::Key key = {&sourceClass, direction, relationships, baseClasses};
    BeMutexHolder lock(m_polymorphicallyRelatedClassesCache->GetMutex());
    bvector<RelatedClass> const* polymorphicallyRelatedClasses = m_polymorphicallyRelatedClassesCache->Get(key);
    if (!polymorphicallyRelatedClasses)
        {
        bool first = true;
        Utf8String q("SELECT RelationshipId, RelatedClassId FROM (");
        for (ECRelationshipClassCP rel : relationships)
            {
            if (!first)
                q.append(" UNION ALL ");
            q.append("SELECT ");
            q.append("r.ECClassId AS RelationshipId,");
            q.append("r.").append(ECRelatedInstanceDirection::Forward == direction ? "TargetECClassId" : "SourceECClassId").append(" AS RelatedClassId,");
            q.append("b.TargetECInstanceId AS BaseClassId ");
            q.append("FROM ").append(rel->GetECSqlName()).append(" r ");
            q.append("JOIN [meta].[ClassHasAllBaseClasses] b ON b.SourceECInstanceId = r.");
            q.append(ECRelatedInstanceDirection::Forward == direction ? "SourceECClassId" : "TargetECClassId");
            first = false;
            }
        q.append(") WHERE BaseClassId = ? ");
        q.append("GROUP BY RelationshipId, RelatedClassId");

        CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(),
            m_connection.GetDb(), q.c_str());
        if (stmt.IsNull())
            {
            BeAssert(false);
            return bvector<RelatedClassPath>();
            }
        stmt->BindId(1, sourceClass.GetId());

        bvector<RelatedClass> vec;
        while (BE_SQLITE_ROW == stmt->Step())
            {
            ECClassId relationshipId = stmt->GetValueId<ECClassId>(0);
            ECRelationshipClassCP relationship = GetECClass(relationshipId)->GetRelationshipClassCP();
            ECClassId targetClassId = stmt->GetValueId<ECClassId>(1);
            ECEntityClassCP targetClass = GetECClass(targetClassId)->GetEntityClassCP();

            if (IsClassHidden(*targetClass) && baseClassInfos.end() == baseClassInfos.find(*targetClass))
                continue;

            vec.push_back(RelatedClass(sourceClass, *targetClass, *relationship,
                ECRelatedInstanceDirection::Forward == direction,
                Utf8String("target_").append(std::to_string(vec.size()).c_str()).c_str(),
                Utf8String("rel_").append(std::to_string(vec.size()).c_str()).c_str(),
                true,
                false));
            }
        polymorphicallyRelatedClasses = &m_polymorphicallyRelatedClassesCache->Add(key, std::move(vec));
        }

    bvector<RelatedClass> polymorphicallyRelatedClassesCopy = *polymorphicallyRelatedClasses;
    lock.unlock();

    for (RelatedClass const& relatedClass : polymorphicallyRelatedClassesCopy)
        {
        BoundQueryValuesList filterBindingsCopy;
        for (BoundQueryValue const* binding : filterBindings)
            filterBindingsCopy.push_back(binding->Clone());

        RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create();
        contract->AddField(*PresentationQueryContractSimpleField::Create("RelatedClassId", relatedClass.IsForwardRelationship() ? "TargetECClassId" : "SourceECClassId"));
        contract->AddField(*PresentationQueryContractSimpleField::Create("RelatedInstanceId", relatedClass.IsForwardRelationship() ?  "SourceECInstanceId" : "TargetECInstanceId"));
        ComplexGenericQueryPtr nestedQuery = ComplexGenericQuery::Create();
        nestedQuery->SelectContract(*contract, "rel");
        nestedQuery->From(*relatedClass.GetRelationship(), true, "rel");
        nestedQuery->Where(relatedClass.IsForwardRelationship() ? "[rel].[TargetECClassId] = ?" : "[rel].[SourceECClassId] = ?", {new BoundQueryId(relatedClass.GetTargetClass()->GetId())});

        ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
        query->SelectAll();
        query->From(*nestedQuery);
        query->Where(Utf8String("[RelatedInstanceId] IN (").append(filteringQuery->ToString()).append(")").c_str(), filterBindingsCopy);
        query->GroupByContract(*SimpleQueryContract::Create(*PresentationQueryContractSimpleField::Create("RelatedClassId", "RelatedClassId")));

        CachedECSqlStatementPtr stmt = m_connection.GetStatementCache().GetPreparedStatement(m_connection.GetECDb().Schemas(),
            m_connection.GetDb(), query->ToString().c_str());
        if (stmt.IsNull())
            {
            BeAssert(false);
            continue;
            }

        query->BindValues(*stmt);

        while (BE_SQLITE_ROW == stmt->Step())
            {
            ECClassId derivedClassId = stmt->GetValueId<ECClassId>(0);
            ECClassCP derivedClass = GetECClass(derivedClassId);

            bool derivesFromBase = false;
            for (ECEntityClassCP baseClass : baseClasses)
                {
                if (derivedClass->Is(baseClass))
                    {
                    derivesFromBase = true;
                    break;
                    }
                }
            if (!derivesFromBase && !baseClasses.empty())
                continue;

            RelatedClass derivedPath(sourceClass, *derivedClass, *relationships[0], true);
            AppendPath(paths, {derivedPath});
            }
        }
    return paths;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SupportedClassNamesParser::UpdateFlags(Utf8String::const_iterator& begin, Utf8String::const_iterator const& end)
    {
    if (end - begin >= 3 && *begin == 'P' && *(begin + 1) == 'E' && *(begin + 2) == ':')
        {
        m_currentFlags = CLASS_FLAG_Polymorphic | CLASS_FLAG_Exclude;
        begin = begin + 3;
        return true;
        }
    if (end - begin >= 2 && *begin == 'E' && *(begin + 1) == ':')
        {
        m_currentFlags = CLASS_FLAG_Exclude;
        begin = begin + 2;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SupportedClassNamesParser::Advance(Utf8String::const_iterator& begin, Utf8String::const_iterator& end) const
    {
    auto next = end + 1;
    if (m_str.end() != next && ' ' == *next)
        begin = end = next + 1;
    else
        begin = end = next;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SupportedClassNamesParser::ParseSchema(Utf8String::const_iterator const& begin, Utf8String::const_iterator const& end)
    {
    m_currentSchema = m_helper.GetSchema(Utf8String(begin, end).c_str());
    if (nullptr == m_currentSchema)
        BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SupportedClassNamesParser::ParseClass(Utf8String::const_iterator const& begin, Utf8String::const_iterator const& end)
    {
    if (nullptr == m_currentSchema)
        {
        BeAssert(false);
        return;
        }
    ECClassCP ecClass = m_currentSchema->GetClassCP(Utf8String(begin, end).c_str());
    if (nullptr == ecClass)
        {
        BeAssert(false);
        return;
        }

    SupportedClassInfo<ECClass> info(*ecClass, m_currentFlags);
    auto iter = m_classInfos.find(info);
    if (m_classInfos.end() != iter)
        {
        if ((CLASS_FLAG_Exclude | CLASS_FLAG_Polymorphic) == ((CLASS_FLAG_Exclude | CLASS_FLAG_Polymorphic) & m_currentFlags))
            m_classInfos.erase(iter);
        else
            iter->SetFlags(iter->GetFlags() | m_currentFlags);
        }
    else
        {
        m_classInfos.insert(info);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SupportedClassNamesParser::Parse()
    {
    auto begin = m_str.begin();
    auto end = m_str.begin();

    while (m_str.end() != end)
        {
        if (*end == ':')
            {
            if (!m_supportExclusion || !UpdateFlags(begin, end + 1))
                ParseSchema(begin, end);
            Advance(begin, end);
            }
        else if (*end == ',' || *end == ';')
            {
            ParseClass(begin, end);
            Advance(begin, end);
            }
        end++;
        }
    if (end != begin)
        ParseClass(begin, end);
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
