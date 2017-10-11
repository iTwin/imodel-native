/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECSchemaHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "../../ECDbBasedCache.h"
#include "../../ValueHelpers.h"
#include "ExtendedData.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* ECClass comparer which guarantees that classes in a sorted set always are in the same
* order (as opposed to the default comparer which compares by pointers).
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECSchemaNameComparer
    {
    bool operator() (ECSchemaCP const& lhs, ECSchemaCP const& rhs) const {return lhs->GetSchemaKey() < rhs->GetSchemaKey();}
    };

/*=================================================================================**//**
* ECClass comparer which guarantees that classes in a sorted set always are in the same
* order (as opposed to the default comparer which compares by pointers).
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECClassNameComparer
    {
    bool operator() (ECClassCP const& lhs, ECClassCP const& rhs) const {return strcmp(lhs->GetFullName(), rhs->GetFullName()) < 0;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
enum SupportedClassFlags
    {
    CLASS_FLAG_Include     = 1 << 0,
    CLASS_FLAG_Exclude     = 1 << 1,
    CLASS_FLAG_Polymorphic = 1 << 2,
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
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
    bool IsInclude() const {return 0 != ((int)CLASS_FLAG_Include & m_flags);}
    bool IsExclude() const {return 0 != ((int)CLASS_FLAG_Exclude & m_flags);}
    bool IsPolymorphic() const {return 0 != ((int)CLASS_FLAG_Polymorphic & m_flags);}
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

struct RelatedPathsCache;
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct ECSchemaHelper : NonCopyableClass
{
    struct SupportedClassesResolver;

    struct RelationshipClassPathOptions
        {
        ECClassCR m_sourceClass;
        int m_relationshipDirection;
        int m_depth;
        Utf8CP m_supportedSchemas;
        Utf8CP m_supportedRelationships;
        Utf8CP m_supportedClasses;
        ECEntityClassCP m_targetClass;
        bmap<ECRelationshipClassCP, int>& m_relationshipsUseCounter;

        ECPRESENTATION_EXPORT RelationshipClassPathOptions(ECClassCR sourceClass, int relationshipDirection, int depth,
            Utf8CP supportedSchemas, Utf8CP supportedRelationships, Utf8CP supportedClasses,
            bmap<ECRelationshipClassCP, int>& relationshipsUseCounter, ECEntityClassCP targetClass = nullptr);
        };

private:
    BeSQLite::EC::ECDbCR m_db;
    RelatedPathsCache& m_relatedPathsCache;
    BeSQLite::EC::ECSqlStatementCache const* m_statementCache;
    bool m_ownsStatementCache;

private:
    void ParseECSchemaNames(bvector<Utf8String>& schemaNames, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const;
    void ParseECSchemas(ECSchemaSet& schemas, bool& exclude, Utf8StringCR commaSeparatedSchemaList) const;
    SupportedRelationshipClassInfos GetECRelationshipClasses(Utf8StringCR commaSeparatedClassList) const;
    ECClassSet GetECClasses(ECSchemaSet const& schemas) const;
    ECSchemaSet GetECSchemas(Utf8StringCR supportedSchemasStr) const;
    void GetPaths(bvector<bpair<RelatedClassPath, bool>>& paths, bmap<ECRelationshipClassCP, int>& relationshipsUseCounter, 
        bset<RelatedClass>&, SupportedClassesResolver const&, BeSQLite::VirtualSet const& sourceClassIds, int relationshipDirection, 
        int depth, ECEntityClassCP targetClass, bool include) const;
    void GetPaths(bvector<bpair<RelatedClassPath, bool>>& paths, bmap<ECRelationshipClassCP, int>& relationshipsUseCounter, 
        bset<RelatedClass>&, BeSQLite::EC::ECSqlStatement&, SupportedClassesResolver const&, BeSQLite::VirtualSet const& sourceClassIds, 
        int relationshipDirection, int depth, ECEntityClassCP targetClass, bool include) const;
                
public:
    ECPRESENTATION_EXPORT ECSchemaHelper(BeSQLite::EC::ECDbCR db, RelatedPathsCache&, BeSQLite::EC::ECSqlStatementCache const*);
    ECPRESENTATION_EXPORT ~ECSchemaHelper();
    BeSQLite::EC::ECDbCR GetDb() const {return m_db;}

    ECPRESENTATION_EXPORT ECSchemaCP GetSchema(Utf8CP schemaName) const;
    ECPRESENTATION_EXPORT ECClassCP GetECClass(Utf8CP schemaName, Utf8CP className, bool isFullSchemaName = false) const;
    ECPRESENTATION_EXPORT ECClassCP GetECClass(Utf8CP fullClassName) const;
    ECPRESENTATION_EXPORT ECClassCP GetECClass(ECClassId) const;
    ECPRESENTATION_EXPORT bvector<ECClassCP> GetECClassesByName(Utf8CP name) const;
    ECPRESENTATION_EXPORT bool AreSchemasSupported(Utf8StringCR schemaListStr) const;
    ECPRESENTATION_EXPORT ECClassSet GetECClassesFromSchemaList(Utf8StringCR schemaListStr) const;
    ECPRESENTATION_EXPORT SupportedEntityClassInfos GetECClassesFromClassList(Utf8StringCR classListStr, bool supportExclusion) const;
    ECPRESENTATION_EXPORT bvector<bpair<RelatedClassPath, bool>> GetRelationshipClassPaths(RelationshipClassPathOptions const&) const;
    ECPRESENTATION_EXPORT ECRelationshipConstraintClassList GetRelationshipConstraintClasses(ECRelationshipClassCR relationship, ECRelatedInstanceDirection direction, Utf8StringCR supportedSchemas) const;
    ECPRESENTATION_EXPORT RelatedClass GetForeignKeyClass(ECPropertyCR prop) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct RelatedPathsCache
{
    struct Key
        {
        ECClassCP m_sourceClass;
        int m_relationshipDirection;
        int m_depth;
        Utf8String m_supportedSchemas;
        Utf8String m_supportedRelationships;
        Utf8String m_supportedClasses;
        ECEntityClassCP m_targetClass;

        Key() : m_sourceClass(nullptr), m_targetClass(nullptr), m_relationshipDirection(0), m_depth(0) {}
        Key(ECSchemaHelper::RelationshipClassPathOptions const& options);
        bool operator<(Key const& other) const;
        };

    struct Result
        {
        bvector<bpair<RelatedClassPath, bool>> m_paths;
        bmap<ECRelationshipClassCP, int> m_relationshipCounter;
        Result() {}
        Result(bvector<bpair<RelatedClassPath, bool>> paths, bmap<ECRelationshipClassCP, int> relationshipCounter)
            : m_paths(paths), m_relationshipCounter(relationshipCounter)
            {}
        };

private:
    bmap<Key, Result> m_cache;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                02/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Result const* Get(Key const& key) const
        {
        auto iter = m_cache.find(key);
        if (m_cache.end() == iter)
            return nullptr;
        return &iter->second;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                02/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    Result const* Put(Key const& key, Result&& result)
        {
        return &m_cache.Insert(key, result).first->second;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct ECInstancesHelper : NonCopyableClass
{
private:
    ECInstancesHelper() {}
public:
    static IECInstancePtr LoadInstance(BeSQLite::EC::ECDbCR db, BeSQLite::EC::ECInstanceKeyCR key);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct IdSetHelper : NonCopyableClass
{
    enum BindSetAction
        {
        BIND_VirtualSet,
        BIND_Ids,
        };

private:
    IdSetHelper() {}
public:
    ECPRESENTATION_EXPORT static BindSetAction CreateInVirtualSetClause(Utf8StringR clause, bvector<ECInstanceKey> const& keys, Utf8StringCR idFieldName);
    ECPRESENTATION_EXPORT static BindSetAction CreateInVirtualSetClause(Utf8StringR clause, BeSQLite::IdSet<ECClassId> const& keys, Utf8StringCR idFieldName);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct SupportedClassNamesParser
{
private:
    ECSchemaHelper const& m_helper;
    Utf8StringCR m_str;
    bool m_supportExclusion;
    ECSchemaCP m_currentSchema;
    int m_currentFlags;
    SupportedClassInfos m_classInfos;

private:
    bool UpdateFlags(Utf8String::const_iterator& begin, Utf8String::const_iterator const& end);
    void Advance(Utf8String::const_iterator& begin, Utf8String::const_iterator& end) const;
    void ParseSchema(Utf8String::const_iterator const& begin, Utf8String::const_iterator const& end);
    void ParseClass(Utf8String::const_iterator const& begin, Utf8String::const_iterator const& end);
    ECPRESENTATION_EXPORT void Parse();

public:
    SupportedClassNamesParser(ECSchemaHelper const& helper, Utf8StringCR str, bool supportExclusion) 
        : m_helper(helper), m_str(str), m_currentFlags(CLASS_FLAG_Polymorphic | CLASS_FLAG_Include), m_currentSchema(nullptr), m_supportExclusion(supportExclusion)
        {
        Parse();
        }
    SupportedClassInfos const& GetClassInfos() const {return m_classInfos;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
