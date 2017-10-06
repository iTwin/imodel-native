/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECSchemaHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "ECSchemaHelper.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::ECSchemaHelper(ECDbCR db, RelatedPathsCache& relatedPathsCache, ECSqlStatementCache const* statementCache)
    : m_db(db), m_relatedPathsCache(relatedPathsCache), m_statementCache(statementCache), m_ownsStatementCache(nullptr == statementCache)
    {
    if (nullptr == m_statementCache)
        m_statementCache = new ECSqlStatementCache(10, "ECSchemaHelper");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaHelper::~ECSchemaHelper()
    {
    if (m_ownsStatementCache)
        delete m_statementCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchemaHelper::GetSchema(Utf8CP schemaName) const {return m_db.Schemas().GetSchema(schemaName);}

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
ECClassCP ECSchemaHelper::GetECClass(ECClassId id) const {return m_db.Schemas().GetClass(id);}

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

        ECClassContainerCR classContainer = schema->GetClasses();
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
        if (nullptr == m_db.Schemas().GetSchema(name.c_str()))
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
    bvector<ECSchemaCP> allSchemas = m_db.Schemas().GetSchemas();    

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
    IdSet<ECClassId> DeterminePolymorphicallySupportedClassesIds(ECDbCR db, BeSQLite::EC::ECSqlStatementCache const& statementsCache, bool include) const
        {
        IdSet<ECClassId> const& ids = include ? GetIncludedEntityClassIds() : GetExcludedEntityClassIds();

        Utf8String whereClause;
        IdSetHelper::BindSetAction action = IdSetHelper::CreateInVirtualSetClause(whereClause, ids, "[TargetECInstanceId]");
        Utf8String query =
            "SELECT SourceECInstanceId"
            " FROM [meta].[ClassHasAllBaseClasses] ";
        query.append("WHERE ").append(whereClause);

        CachedECSqlStatementPtr stmt = statementsCache.GetPreparedStatement(db, query.c_str());
        if (IdSetHelper::BIND_VirtualSet == action)
            {
            stmt->BindVirtualSet(1, ids);
            }
        else
            {
            int i = 1;
            for (ECClassId id : ids)
                stmt->BindId(i++, id);
            }

        IdSet<ECClassId> classesIds;
        while (DbResult::BE_SQLITE_ROW == stmt->Step())
            classesIds.insert(stmt->GetValueId<ECClassId>(0));
        return classesIds;
        }
    
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    SupportedClassesResolver(ECSchemaSet const& supportedSchemas, SupportedEntityClassInfos const* classInfos, SupportedRelationshipClassInfos const* relationshipInfos)
        : m_flags(0)
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
    IdSet<ECClassId> const& GetPolymorphicallyIncludedClassIds(ECDbCR db, BeSQLite::EC::ECSqlStatementCache const& statementsCache) const
        {
        if (0 == (m_flags & DeterminedPolymorphicallyIncludedClasses))
            {
            m_polymorphicallyIncludedClassIds = DeterminePolymorphicallySupportedClassesIds(db, statementsCache, true);
            m_flags |= DeterminedPolymorphicallyIncludedClasses;
            }
        return m_polymorphicallyIncludedClassIds;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Saulius.Skliutas                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    IdSet<ECClassId> const& GetPolymorphicallyExcludedClassIds(ECDbCR db, BeSQLite::EC::ECSqlStatementCache const& statementsCache) const
        {
        if (0 == (m_flags & DeterminedPolymorphicallyExcludedClasses))
            {
            m_polymorphicallyExcludedClassIds = DeterminePolymorphicallySupportedClassesIds(db, statementsCache, false);
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
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRelationshipPathsQuery(ECSchemaHelper::SupportedClassesResolver const& resolver, 
    int relationshipDirection, int depth, bool include)
    {
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
            query.append("InVirtualSet(:supportedRelationshipIds, [relationship].[ECInstanceId])");
            hasCondition = true;
            }

        if (!ShouldAcceptAllClasses(resolver, include))
            {
            if (hasCondition)
                query.append(" AND ");
            query.append("InVirtualSet(:supportedClassIds, [end].[ECinstanceId])");
            hasCondition = true;
            }
        }

    if (hasCondition)
        query.append(" AND ");
    query.append("InVirtualSet(:sourceClassIds, [start].[ECInstanceId])");

    if (((int)ECRelatedInstanceDirection::Backward | (int)ECRelatedInstanceDirection::Forward) != relationshipDirection)
        query.append(" AND [startConstraint].[RelationshipEnd] = :relationshipEnd");
    
    if (resolver.GetAcceptAllRelationships())
        query.append(" AND InVirtualSet(:supportedSchemaIds, [relationship].[Schema].[Id])");
    if (include && resolver.GetAcceptAllIncludeClasses())
        query.append(" AND InVirtualSet(:supportedSchemaIds, [end].[Schema].[Id])");

    query.append(" ORDER BY [start].[ECInstanceId], [end].[ECInstanceId], [relationship].[ECInstanceId]");

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus BindVariables(ECSqlStatement& stmt, VirtualSet const& sourceClassIds, ECSchemaHelper::SupportedClassesResolver const& resolver, 
    int relationshipDirection, int depth, bool include, BeSQLite::EC::ECSqlStatementCache const& statementsCache)
    {
    if (!stmt.BindVirtualSet(stmt.GetParameterIndex("sourceClassIds"), sourceClassIds).IsSuccess())
        {
        BeAssert(false);
        return ERROR;
        }
    
    if (((int)ECRelatedInstanceDirection::Backward | (int)ECRelatedInstanceDirection::Forward) != relationshipDirection)
        {
        ECRelationshipEnd relationshipEnd = (ECRelatedInstanceDirection::Backward == (ECRelatedInstanceDirection)relationshipDirection) 
            ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;
        if (!stmt.BindInt(stmt.GetParameterIndex("relationshipEnd"), relationshipEnd).IsSuccess())
            {
            BeAssert(false);
            return ERROR;
            }
        }

    if (depth <= 0 && !resolver.GetAcceptAllRelationships())
        {
        IdSet<ECClassId> const& ids = resolver.GetRelationshipClassIds();
        if (!stmt.BindVirtualSet(stmt.GetParameterIndex("supportedRelationshipIds"), ids).IsSuccess())
            {
            BeAssert(false);
            return ERROR;
            }
        }
    
    if (depth <= 0 && !ShouldAcceptAllClasses(resolver, include))
        {
        IdSet<ECClassId> const& ids = include ? resolver.GetPolymorphicallyIncludedClassIds(*stmt.GetECDb(), statementsCache) : resolver.GetPolymorphicallyExcludedClassIds(*stmt.GetECDb(), statementsCache);
        if(!stmt.BindVirtualSet(stmt.GetParameterIndex("supportedClassIds"), ids).IsSuccess())
            {
            BeAssert(false);
            return ERROR;
            }
        }
    
    if (resolver.GetAcceptAllRelationships() || include && resolver.GetAcceptAllIncludeClasses())
        {
        if (!stmt.BindVirtualSet(stmt.GetParameterIndex("supportedSchemaIds"), resolver.GetSchemaIds()).IsSuccess())
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaHelper::GetPaths(bvector<bpair<RelatedClassPath, bool>>& paths, bmap<ECRelationshipClassCP, int>& relationshipsUseCounter, 
    bset<RelatedClass>& usedRelationships, SupportedClassesResolver const& resolver, VirtualSet const& sourceClassIds, int relationshipDirection, 
    int depth, ECEntityClassCP targetClass, bool include) const
    {
    Utf8String query = CreateRelationshipPathsQuery(resolver, relationshipDirection, depth, include);
    CachedECSqlStatementPtr stmt = m_statementCache->GetPreparedStatement(m_db, query.c_str());
    if (stmt.IsValid())
        GetPaths(paths, relationshipsUseCounter, usedRelationships, *stmt, resolver, sourceClassIds, relationshipDirection, depth, targetClass, include);
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
    bset<RelatedClass>& usedRelationships, ECSqlStatement& stmt, SupportedClassesResolver const& resolver, 
    VirtualSet const& sourceClassIds, int relationshipDirection, int depth, ECEntityClassCP targetClass, bool include) const
    {
    // bind to get includes
    if (SUCCESS != BindVariables(stmt, sourceClassIds, resolver, relationshipDirection, depth, include, *m_statementCache))
        {
        BeAssert(false);
        return;
        }
    
    bvector<RelatedClass> specs;
    ECClassVirtualSet relatedClasses;

    // get the results
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ECEntityClassCP source = GetECClass(stmt.GetValueId<ECClassId>(0))->GetEntityClassCP();
        ECRelationshipClassCP relationship = GetECClass(stmt.GetValueId<ECClassId>(1))->GetRelationshipClassCP();
        ECEntityClassCP actualRelated = GetECClass(stmt.GetValueId<ECClassId>(3))->GetEntityClassCP();
        if (nullptr == source || nullptr == relationship || nullptr == actualRelated)
            {
            BeAssert(false);
            continue;
            }
        bool isForward = !(ECRelationshipEnd_Source == (ECRelationshipEnd)stmt.GetValueInt(4)); // note: reverse the direction
        
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
            relatedClasses.Add(*actualRelated);
            }
        }

    if (0 != depth && !specs.empty())
        {
        bvector<bpair<RelatedClassPath, bool>> subPaths;
        GetPaths(subPaths, relationshipsUseCounter, usedRelationships, resolver, relatedClasses, 
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
    Utf8StringCR supportedSchemas, Utf8StringCR supportedRelationships, Utf8StringCR supportedClasses,
    bmap<ECRelationshipClassCP, int>& relationshipsUseCounter, ECEntityClassCP targetClass)
    : m_relationshipsUseCounter(relationshipsUseCounter), m_sourceClass(sourceClass), m_supportedSchemas(supportedSchemas),
    m_supportedRelationships(supportedRelationships), m_supportedClasses(supportedClasses)
    {
    m_specificationId = -1;
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
    if (-1 != options.GetSpecificationId())
        {
        RelatedPathsCache::Key key(options.m_sourceClass, options.m_targetClass, options.GetSpecificationId());
        RelatedPathsCache::Result const* result = m_relatedPathsCache.Get(key);
        if (nullptr != result)
            {
            for (auto const& pair : result->m_relationshipCounter)
                options.m_relationshipsUseCounter[pair.first] += pair.second;
            return result->m_paths;
            }
        }

    bvector<bpair<RelatedClassPath, bool>> paths;
    SupportedEntityClassInfos classInfos = GetECClassesFromClassList(options.m_supportedClasses, true);
    SupportedRelationshipClassInfos relationshipInfos = GetECRelationshipClasses(options.m_supportedRelationships);
    SupportedClassesResolver resolver(GetECSchemas(options.m_supportedSchemas), 
        (classInfos.empty() && options.m_supportedClasses.empty()) ? nullptr : &classInfos, 
        (relationshipInfos.empty() && options.m_supportedRelationships.empty()) ? nullptr : &relationshipInfos);
    
    bset<RelatedClass> usedRelationships;
    ECClassVirtualSet sourceSet(options.m_sourceClass);

    // get includes
    GetPaths(paths, options.m_relationshipsUseCounter, usedRelationships, resolver, sourceSet,
        options.m_relationshipDirection, options.m_depth, options.m_targetClass, true);
    
    if (resolver.HasExcludes())
        {
        // get excludes
        usedRelationships.clear();
        GetPaths(paths, options.m_relationshipsUseCounter, usedRelationships, resolver, sourceSet,
            options.m_relationshipDirection, options.m_depth, options.m_targetClass, false);
        }
    
    // cache if necessary
    if (-1 != options.GetSpecificationId())
        {
        RelatedPathsCache::Key key(options.m_sourceClass, options.m_targetClass, options.GetSpecificationId());
        m_relatedPathsCache.Put(key, RelatedPathsCache::Result(paths, options.m_relationshipsUseCounter));
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
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TSet>
static IdSetHelper::BindSetAction CreateInVirtualSetClauseInternal(Utf8StringR clause, TSet const& set, Utf8StringCR idFieldName)
    {
    if (set.empty())
        {
        clause.assign("0");
        return IdSetHelper::BIND_Ids;
        }

    if (set.size() > 100)
        {
        clause.assign("InVirtualSet(?, ");
        clause.append(idFieldName).append(")");
        return IdSetHelper::BIND_VirtualSet;
        }
    
    Utf8String idsArg(set.size() * 2 - 1, '?');
    for (size_t i = 1; i < set.size() * 2; i += 2)
        idsArg[i] = ',';
    clause.assign(idFieldName).append(" IN (");
    clause.append(idsArg).append(")");
    return IdSetHelper::BIND_Ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IdSetHelper::BindSetAction IdSetHelper::CreateInVirtualSetClause(Utf8StringR clause, bvector<ECInstanceKey> const& keys, Utf8StringCR idFieldName)
    {
    return CreateInVirtualSetClauseInternal(clause, keys, idFieldName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IdSetHelper::BindSetAction IdSetHelper::CreateInVirtualSetClause(Utf8StringR clause, IdSet<ECClassId> const& ids, Utf8StringCR idFieldName)
    {
    return CreateInVirtualSetClauseInternal(clause, ids, idFieldName);
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2015
+===============+===============+===============+===============+===============+======*/
struct ECInstanceLoadStatementCache : BeSQLite::Db::AppData
{
private:
    bmap<Utf8String, ECSqlStatement*> m_statements;
public:
    static RefCountedPtr<ECInstanceLoadStatementCache> Create() {return new ECInstanceLoadStatementCache();}
    ~ECInstanceLoadStatementCache()
        {
        for (auto statement : m_statements)
            delete statement.second;
        }
    ECSqlStatement* GetStatement(ECDbCR db, Utf8CP query)
        {
        auto iter = m_statements.find(query);
        if (m_statements.end() == iter)
            iter = m_statements.Insert(query, new ECSqlStatement()).first;
        if (!iter->second->IsPrepared())
            {
            if (ECSqlStatus::Success != iter->second->Prepare(db, query))
                return nullptr;
            }
        else
            {
            iter->second->Reset();
            }
        return iter->second;
        }
};
static ECInstanceLoadStatementCache::Key s_cacheKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSqlStatement* GetPreparedStatement(ECDbCR db, Utf8CP ecsql)
    {
    RefCountedPtr<BeSQLite::Db::AppData> cache = db.FindAppData(s_cacheKey);
    if (cache.IsNull())
        {
        cache = ECInstanceLoadStatementCache::Create();
        db.AddAppData(s_cacheKey, cache.get(), true);
        }
    return static_cast<ECInstanceLoadStatementCache*>(cache.get())->GetStatement(db, ecsql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstancesHelper::LoadInstance(ECDbCR db, ECInstanceKeyCR key)
    {
    ECClassCP selectClass = db.Schemas().GetClass(key.GetClassId());
    if (nullptr == selectClass || !selectClass->IsEntityClass())
        {
        BeAssert(false);
        return nullptr;
        }

    Utf8String ecsql("SELECT * FROM ONLY ");
    ecsql.append(selectClass->GetECSqlName()).append(" WHERE ECInstanceId=?");
    ECSqlStatement* stmt = GetPreparedStatement(db, ecsql.c_str());
    if (nullptr == stmt)
        {
        BeAssert(false);
        return nullptr;
        }

    if (ECSqlStatus::Success != stmt->BindId(1, key.GetInstanceId()))
        {
        BeAssert(false);
        return nullptr;
        }

    ECInstanceECSqlSelectAdapter adapter(*stmt);
    if (BeSQLite::DbResult::BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        return nullptr;
        }

    IECInstancePtr instance = adapter.GetInstance();
    stmt->Reset();
    return instance;
    }
