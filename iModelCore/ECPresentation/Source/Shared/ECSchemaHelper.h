/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "../Rules/CommonToolsInternal.h"
#include "ValueHelpers.h"
#include "ExtendedData.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename ClassType> struct SupportedClassInfo
{
private:
    ClassType const* m_class;
    int m_flags;
public:
    SupportedClassInfo() : m_class(nullptr), m_flags(0) {}
    SupportedClassInfo(ClassType const& ecClass) : m_class(&ecClass), m_flags(0) {}
    SupportedClassInfo(ClassType const& ecClass, int flags) : m_class(&ecClass), m_flags(flags) {}
    bool operator<(SupportedClassInfo<ClassType> const& other) const
        {
        if (m_class == other.m_class)
            return false;
        if (nullptr == other.m_class)
            return false;
        if (nullptr == m_class)
            return true;
        return m_class->GetId() < other.m_class->GetId();
        }
    ClassType const& GetClass() const {return *m_class;}
    int GetFlags() const {return m_flags;}
    void SetFlags(int flags) {m_flags = flags;}
    bool IsInclude() const {return 0 != ((int)CLASS_SELECTION_FLAG_Include & m_flags);}
    bool IsExclude() const {return 0 != ((int)CLASS_SELECTION_FLAG_Exclude & m_flags);}
    bool IsPolymorphic() const {return 0 != ((int)CLASS_SELECTION_FLAG_Polymorphic & m_flags);}
};
typedef SupportedClassInfo<ECEntityClass> SupportedEntityClassInfo;
typedef SupportedClassInfo<ECRelationshipClass> SupportedRelationshipClassInfo;

//! A set of supported classes.
typedef bset<SupportedClassInfo<ECClass>> SupportedClassInfos;

//! A set of supported entity classes.
typedef bset<SupportedEntityClassInfo> SupportedEntityClassInfos;

//! A set of supported relationship classes.
typedef bset<SupportedRelationshipClassInfo> SupportedRelationshipClassInfos;

//! A set of schemas.
typedef bset<ECSchemaCP, ECSchemaNameComparer> ECSchemaSet;

//! A map of class and polymorphism flags.
typedef bmap<ECEntityClassCP, bool, ECClassNameComparer> ECClassSet;

//! A set of ECClass & ECRelationshipClass pairs.
typedef bset<RelatedClass> RelatedClassSet;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassUseCounter
{
private:
    bmap<ECClassCP, int> m_counters;
public:
    int Inc(ECClassCP ecClass) {return m_counters[ecClass]++;}
    int GetCount(ECClassCP ecClass) const
        {
        auto iter = m_counters.find(ecClass);
        return (iter != m_counters.end()) ? iter->second : 0;
        }
    void Merge(ECClassUseCounter const& other)
        {
        for (auto const& entry : other.m_counters)
            m_counters[entry.first] += entry.second;
        }
};

struct ECExpressionsCache;
struct RelatedPathsCache;
struct InstanceFilteringParams;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECSchemaHelper : NonCopyableClass
{
    struct SupportedClassesResolver;

    struct RelationshipClassPathOptionsDeprecated
        {
        ECClassCR m_sourceClass;
        int m_relationshipDirection;
        int m_depth;
        Utf8CP m_supportedSchemas;
        Utf8CP m_supportedRelationships;
        Utf8CP m_supportedClasses;
        bool m_handleRelatedClassesPolymorphically;
        ECClassCP m_targetClass;
        ECClassUseCounter& m_relationshipsUseCounter;

        RelationshipClassPathOptionsDeprecated(ECClassCR sourceClass, int relationshipDirection, int depth, Utf8CP supportedSchemas,
            Utf8CP supportedRelationships, Utf8CP supportedClasses, bool handleRelatedClassesPolymorphically,
            ECClassUseCounter& relationshipsUseCounter, ECClassCP targetClass = nullptr)
            : m_relationshipsUseCounter(relationshipsUseCounter), m_sourceClass(sourceClass), m_supportedSchemas(supportedSchemas),
            m_supportedRelationships(supportedRelationships), m_supportedClasses(supportedClasses), m_targetClass(targetClass),
            m_relationshipDirection(relationshipDirection), m_depth(depth), m_handleRelatedClassesPolymorphically(handleRelatedClassesPolymorphically)
            {}
        };

    struct MultiRelationshipPathOptions
        {
        ECClassCR m_sourceClass;
        RelationshipPathSpecification const& m_path;
        bool m_mergePolymorphicPaths;
        ECClassUseCounter& m_relationshipsUseCounter;
        MultiRelationshipPathOptions(ECClassCR sourceClass, RelationshipPathSpecification const& path, bool mergePolymorphicPaths, ECClassUseCounter& relationshipsUseCounter)
            : m_sourceClass(sourceClass), m_path(path), m_relationshipsUseCounter(relationshipsUseCounter), m_mergePolymorphicPaths(mergePolymorphicPaths)
            {}
        };
    struct RepeatableMultiRelationshipPathOptions
        {
        ECClassCR m_sourceClass;
        RepeatableRelationshipPathSpecification const& m_path;
        bool m_mergePolymorphicPaths;
        ECClassUseCounter& m_relationshipsUseCounter;
        RepeatableMultiRelationshipPathOptions(ECClassCR sourceClass, RepeatableRelationshipPathSpecification const& path, bool mergePolymorphicPaths, ECClassUseCounter& relationshipsUseCounter)
            : m_sourceClass(sourceClass), m_path(path), m_relationshipsUseCounter(relationshipsUseCounter), m_mergePolymorphicPaths(mergePolymorphicPaths)
            {}
        };

    struct RelationshipPathsRequestParams
        {
        struct PathSpecification
            {
            int m_targetIndex;
            RelationshipPathSpecification const* m_specification;
            bool m_isTargetPolymorphic;
            bool m_applyTargetInstancesCheck;
            bvector<Utf8String> m_stepInstanceFilters;
            PathSpecification(int targetIndex, RelationshipPathSpecification const& spec, bool isTargetPolymorphic)
                : m_targetIndex(targetIndex), m_specification(&spec), m_isTargetPolymorphic(isTargetPolymorphic), m_applyTargetInstancesCheck(false)
                {}
            PathSpecification(int targetIndex, RelationshipPathSpecification const& spec, bool isTargetPolymorphic, bvector<Utf8String> stepInstanceFilters)
                : m_targetIndex(targetIndex), m_specification(&spec), m_isTargetPolymorphic(isTargetPolymorphic), m_applyTargetInstancesCheck(true), m_stepInstanceFilters(stepInstanceFilters)
                {}
            };

        SelectClassWithExcludes<ECClass> m_source;
        ConstRef<bvector<PathSpecification>> m_paths;
        InstanceFilteringParams const* m_sourceInstanceFilter;
        ConstRef<bvector<RelatedClassPath>> m_relatedInstancePaths;
        ECClassUseCounter& m_relationshipsUseCounter;
        bool m_countTargets;

        RelationshipPathsRequestParams(SelectClassWithExcludes<ECClass> source, ConstRef<bvector<PathSpecification>> paths, InstanceFilteringParams const* sourceInstanceFilter, ConstRef<bvector<RelatedClassPath>> relatedInstancePaths, ECClassUseCounter& relationshipsUseCounter, bool countTargets)
            : m_source(source), m_paths(paths), m_sourceInstanceFilter(sourceInstanceFilter), m_relatedInstancePaths(relatedInstancePaths), m_relationshipsUseCounter(relationshipsUseCounter), m_countTargets(countTargets)
            {}
        };
    struct RelationshipPathsResponse
        {
        struct RelationshipPathResult
            {
            RelatedClassPath m_path;
            std::unordered_set<ECClassCP> m_actualSourceClasses;
            };
        private:
            bvector<bvector<RelationshipPathResult>> m_indexedPaths;
        public:
            RelationshipPathsResponse(size_t pathsCount) : m_indexedPaths(pathsCount) {}
            bvector<RelationshipPathResult>& GetPaths(size_t targetIndex) {return m_indexedPaths[targetIndex];}
            bvector<RelationshipPathResult> const& GetPaths(size_t targetIndex) const {return m_indexedPaths[targetIndex];}
        };

private:
    IConnectionCR m_connection;
    RelatedPathsCache* m_relatedPathsCache;
    bool m_ownsRelatedPathsCache;
    ECExpressionsCache* m_ecexpressionsCache;
    bool m_ownsECExpressionsCache;

private:
    void ParseECSchemaNames(bvector<Utf8String>& schemaNames, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const;
    void ParseECSchemas(ECSchemaSet& schemas, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const;
    ECClassSet GetECClasses(ECSchemaSet const& schemas) const;
    ECSchemaSet GetECSchemas(Utf8StringCR supportedSchemasStr) const;

    Nullable<uint64_t> QueryTargetsCount(RelatedClassPathCR, InstanceFilteringParams const*, bvector<RelatedClassPath> const&) const;

    void GetPathsDeprecated(bvector<RelatedClassPath>& paths, ECClassUseCounter& relationshipsUseCounter,
        bset<RelatedClass, RelatedClass::ClassPointersComparer>&, SupportedClassesResolver const&, bset<ECClassId> const& sourceClassIds,
        bool handleRelatedClassesPolymorphically, int relationshipDirection, int depth, ECClassCP targetClass) const;
    bvector<RelatedClassPath> GetPaths(ECClassId sourceClassId, bvector<RepeatableRelationshipStepSpecification> path,
        bset<RelatedClass>& usedRelationships, ECClassUseCounter& relationshipsUseCounter, bool handleRelatedClassesPolymorphically) const;
    template<typename TOptions> bvector<RelatedClassPath> GetCachedRelationshipPaths(TOptions const& options, std::function<bvector<RelatedClassPath>()> const& handler) const;

public: // internal
    // used by: GetRelatedInstancePaths
    ECPRESENTATION_EXPORT bvector<RelatedClassPath> GetRelationshipClassPaths(MultiRelationshipPathOptions const&) const;
    ECPRESENTATION_EXPORT bvector<RelatedClassPath> GetRelationshipClassPaths(RepeatableMultiRelationshipPathOptions const&) const;

public:
    ECPRESENTATION_EXPORT ECSchemaHelper(IConnectionCR, RelatedPathsCache*, ECExpressionsCache*);
    ECPRESENTATION_EXPORT ~ECSchemaHelper();
    IConnectionCR GetConnection() const {return m_connection;}
    ECExpressionsCache& GetECExpressionsCache() const {return *m_ecexpressionsCache;}

    ECPRESENTATION_EXPORT ECSchemaCP GetSchema(Utf8CP schemaName, bool fullyLoad) const;
    ECPRESENTATION_EXPORT ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className, bool isFullSchemaName = false) const;
    ECPRESENTATION_EXPORT ECClassCP GetECClass(Utf8CP fullClassName) const;
    ECPRESENTATION_EXPORT ECClassCP GetECClass(ECClassId) const;
    ECPRESENTATION_EXPORT bvector<ECClassCP> GetECClassesByName(Utf8CP name) const;
    ECPRESENTATION_EXPORT bool AreSchemasSupported(Utf8StringCR schemaListStr) const;
    ECPRESENTATION_EXPORT ECClassSet GetECClassesFromSchemaList(Utf8StringCR schemaListStr) const;
    ECPRESENTATION_EXPORT bvector<ECClassCP> GetDerivedECClassesRecursively(ECClassCP baseClass) const;
    ECPRESENTATION_EXPORT SupportedClassInfos GetECClassesFromClassList(Utf8StringCR classListStr, bool supportExclusion) const;
    ECPRESENTATION_EXPORT SupportedClassInfos GetECClassesFromClassList(bvector<MultiSchemaClass*> const& classList, bool areExcluded) const;
    ECPRESENTATION_EXPORT SupportedRelationshipClassInfos GetECRelationshipClasses(Utf8StringCR commaSeparatedClassList) const;
    ECPRESENTATION_EXPORT Utf8String CreateClassListString(SupportedClassInfos const&) const;
    ECPRESENTATION_EXPORT Utf8String CreateClassListString(SupportedRelationshipClassInfos const&) const;
    ECPRESENTATION_EXPORT SupportedRelationshipClassInfos GetPossibleRelationships(ECClassCR source, RequiredRelationDirection, Utf8StringCR targetClassNames) const;
    ECPRESENTATION_EXPORT ECRelationshipConstraintClassList GetRelationshipConstraintClasses(ECRelationshipClassCR relationship,
        ECRelatedInstanceDirection direction, Utf8StringCR supportedSchemas) const;
    ECPRESENTATION_EXPORT RelatedClass GetForeignKeyClass(ECPropertyCR prop) const;
    ECPRESENTATION_EXPORT bvector<RelatedClassPath> BuildRelationshipPathsFromStepSpecifications(ECClassCR source, bvector<RelationshipStepSpecification*> const&,
        bvector<Utf8String> const& stepInstanceFilters, bool looseSourceMatching) const;

    //! Used by:
    //! - ContentSpecificationsHandler to support ContentRelatedInstances deprecated use case
    //! - NavigationQueryBuilder to support RelatedInstanceNodesSpecification deprecated use case
    ECPRESENTATION_EXPORT bvector<RelatedClassPath> GetRelationshipClassPathsDeprecated(RelationshipClassPathOptionsDeprecated const&) const;
    //! Used by:
    //! - ContentSpecificationsHandler to support ContentRelatedInstances
    //! - NavigationQueryBuilder to support RelatedInstanceNodesSpecification
    ECPRESENTATION_EXPORT bvector<RelatedClassPath> GetRecursiveRelationshipClassPaths(ECClassCR sourceClass, bvector<ECInstanceId> const& sourceIds,
        bvector<RepeatableRelationshipPathSpecification*> const&, ECClassUseCounter&, bool mergePolymorphicPaths, bool groupByInputKey) const;

    //! Finds paths based on given relationship path specifications. Size of the resulting vector depends on the maximum target index
    //! specified in path specifications. One specification may result in multiple paths, which are put into resulting vector at target index.
    //! Optionally filters the paths based on whether they point to at least one target instance.
    //!
    //! Used by: ContentSpecificationsHandler to find related property paths
    ECPRESENTATION_EXPORT RelationshipPathsResponse GetRelationshipPaths(RelationshipPathsRequestParams const&) const;

    //! Finds related instance paths based on given RelatedInstanceSpecificationList and associates them with alias in the specification.
    //!
    //! Used by: ContentSpecificationsHandler, NavigationQueryBuilder to find related instance paths
    ECPRESENTATION_EXPORT bmap<Utf8String, bvector<RelatedClassPath>> GetRelatedInstancePaths(ECClassCR selectClass,
        RelatedInstanceSpecificationList const&, ECClassUseCounter&) const;

    //! Returns ExtendedTypeName if property has it, otherwise returns TypeName.
    ECPRESENTATION_EXPORT static Utf8String GetTypeName(ECPropertyCR property);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelatedPathsCacheDeprecated
{
    struct Key
        {
        ECClassCP m_sourceClass;
        int m_relationshipDirection;
        int m_depth;
        Utf8String m_supportedSchemas;
        Utf8String m_supportedRelationships;
        Utf8String m_supportedClasses;
        ECClassCP m_targetClass;

        Key() : m_sourceClass(nullptr), m_targetClass(nullptr), m_relationshipDirection(0), m_depth(0) {}
        Key(ECSchemaHelper::RelationshipClassPathOptionsDeprecated const& options);
        bool operator<(Key const& other) const;
        };

    struct Result
        {
        bvector<RelatedClassPath> m_paths;
        ECClassUseCounter m_relationshipCounter;
        Result() {}
        Result(bvector<RelatedClassPath> paths, ECClassUseCounter relationshipCounter)
            : m_paths(paths), m_relationshipCounter(relationshipCounter)
            {}
        };

private:
    std::map<Key, std::unique_ptr<Result const>> m_cache;
    mutable BeMutex m_mutex;

public:
    BeMutex& GetMutex() const { return m_mutex; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Result const* Get(Key const& key) const
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_cache.find(key);
        if (m_cache.end() == iter)
            return nullptr;
        return iter->second.get();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Result const* Put(Key const& key, std::unique_ptr<Result const> result)
        {
        BeMutexHolder lock(m_mutex);
        return m_cache.insert(std::make_pair(key, std::move(result))).first->second.get();
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelatedPathsCache
{
    struct Key
        {
        ECClassCP m_sourceClass;
        RepeatableRelationshipPathSpecification m_pathSpecification;
        bool m_mergePolymorphicPaths;
        Key() : m_sourceClass(nullptr) {}
        Key(Key const& other);
        Key(ECSchemaHelper::MultiRelationshipPathOptions const& options);
        Key(ECSchemaHelper::RepeatableMultiRelationshipPathOptions const& options);
        bool operator<(Key const& other) const;
        };
    struct Result
        {
        bvector<RelatedClassPath> m_paths;
        ECClassUseCounter m_relationshipCounter;
        Result() {}
        Result(bvector<RelatedClassPath> paths, ECClassUseCounter relationshipCounter)
            : m_paths(paths), m_relationshipCounter(relationshipCounter)
            {}
        };
private:
    mutable BeMutex m_mutex;
    RelatedPathsCacheDeprecated m_deprecated;
    std::map<Key, std::unique_ptr<Result const>> m_cache;
public:
    RelatedPathsCacheDeprecated& GetDeprecated() {return m_deprecated;}
    RelatedPathsCacheDeprecated const& GetDeprecated() const {return m_deprecated;}

    BeMutex& GetMutex() const { return m_mutex; }

    Result const* Get(Key const& key) const
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_cache.find(key);
        if (m_cache.end() == iter)
            return nullptr;
        return iter->second.get();
        }
    Result const* Put(Key const& key, std::unique_ptr<Result const> result)
        {
        BeMutexHolder lock(m_mutex);
        return m_cache.insert(std::make_pair(key, std::move(result))).first->second.get();
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstancesHelper : NonCopyableClass
{
private:
    ECInstancesHelper() {}
public:
    ECPRESENTATION_EXPORT static DbResult LoadInstance(IECInstancePtr&, IConnectionCR, ECInstanceKeyCR);
    static ECValue GetValue(IConnectionCR, ECInstanceKeyCR, Utf8CP propertyName);
    static ECValue GetValue(IConnectionCR, ECClassCR, ECInstanceId, ECPropertyCR);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SupportedClassesParser
{
private:
    SupportedClassInfos m_classInfos;
private:
    ECPRESENTATION_EXPORT void Parse(ECSchemaHelper const& helper, Utf8StringCR str, bool supportExclusion);
public:
    SupportedClassesParser(ECSchemaHelper const& helper, Utf8StringCR str, bool supportExclusion)
        {
        Parse(helper, str, supportExclusion);
        }
    SupportedClassInfos const& GetClassInfos() const {return m_classInfos;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
