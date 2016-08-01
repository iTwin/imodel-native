/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "MapStrategy.h"
#include "DbSchema.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECDbMap;
struct ClassMap;
struct ClassMappingInfo;
struct SchemaImportContext;

enum class MappingStatus
    {
    Success = 0,
    BaseClassesNotMapped = 1,    // We have temporarily stopped mapping a given branch of the class hierarchy because
                                 // we haven't mapped one or more of its base classes. This can happen in the case 
                                 // of multiple inheritance, where we attempt to map a child class for which 
                                 // not all parent classes have been mapped
    Error = 666
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct ClassMappingInfoFactory
    {
private:
    ClassMappingInfoFactory ();
    ~ClassMappingInfoFactory ();

public:
    static std::unique_ptr<ClassMappingInfo> Create(MappingStatus&, ECN::ECClassCR, ECDbMap const&);
    };

struct IndexMappingInfo;
typedef RefCountedPtr<IndexMappingInfo> IndexMappingInfoPtr;

//======================================================================================
//! Info class used during schema import in order to create the mapping information for classes
//! and relationships
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMappingInfo : NonCopyableClass
{
private:
    Utf8String m_tableName;
    Utf8String m_ecInstanceIdColumnName;
    std::vector<IndexMappingInfoPtr> m_dbIndexes;
    bool m_isMapToVirtualTable;
    ECN::ECPropertyCP m_classHasCurrentTimeStampProperty;

protected:
    ECDbMap const& m_ecdbMap;
    ECN::ECClassCR m_ecClass;
    ECDbMapStrategy m_resolvedStrategy;
    ClassMap const* m_baseClassMap;

private:
    BentleyStatus DoEvaluateMapStrategy(UserECDbMapStrategy&, ClassMap const* baseClassMap);

    MappingStatus TryGetBaseClassMap(ClassMap const*& baseClassMap) const;
    BentleyStatus InitializeClassHasCurrentTimeStampProperty();

    MappingStatus EvaluateMapStrategy();
    virtual MappingStatus _EvaluateMapStrategy();

protected:
    BentleyStatus EvaluateTablePerHierarchyMapStrategy(ClassMap const& baseClassMap, UserECDbMapStrategy const&);
    bool ValidateTablePerHierarchyChildStrategy(ECDbMapStrategy const& baseStrategy, UserECDbMapStrategy const&) const;
    virtual BentleyStatus _InitializeFromSchema();

    IssueReporter const& Issues() const;
    static void LogClassNotMapped (NativeLogging::SEVERITY, ECN::ECClassCR, Utf8CP explanation);

public:
    ClassMappingInfo(ECN::ECClassCR, ECDbMap const&);
    virtual ~ClassMappingInfo() {}

    MappingStatus Initialize();

    ECDbMapStrategy const& GetMapStrategy () const{ return m_resolvedStrategy; }
    ECDbMap const& GetECDbMap() const {return m_ecdbMap;}
    ECN::ECClassCR GetECClass() const {return m_ecClass;}
    std::vector<IndexMappingInfoPtr> const& GetIndexInfos() const { return m_dbIndexes;}
    Utf8CP GetTableName() const {return m_tableName.c_str();}
    Utf8CP GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName.c_str();}
    ECN::ECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    ClassMap const* GetBaseClassMap () const { return m_baseClassMap; }
    //! Virtual tables are not persisted   
    bool IsMapToVirtualTable () const { return m_isMapToVirtualTable; }
    };


//======================================================================================
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipEndColumns
    {
private:
    Utf8String m_ecInstanceIdColumnName;
    Utf8String m_ecClassIdColumnName;

public:
    RelationshipEndColumns() {}
    RelationshipEndColumns(Utf8CP ecInstanceIdColumnName, Utf8CP ecClassIdColumnName = nullptr) : m_ecInstanceIdColumnName(ecInstanceIdColumnName), m_ecClassIdColumnName(ecClassIdColumnName) {}
    Utf8CP GetECInstanceIdColumnName() const { return m_ecInstanceIdColumnName.c_str(); }
    Utf8CP GetECClassIdColumnName() const { return m_ecClassIdColumnName.c_str(); }
    };

//======================================================================================
//!This class grabs information from ForeignKeyRelationshipMap or LinkTableRelationshipMap ECCustomAttribute and evaluates
//! it along with other standard metadata on the ECRelationshipClass
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipMappingInfo : public ClassMappingInfo
    {
public:
    enum class Cardinality
        {
        ManyToMany,
        OneToMany,
        ManyToOne,
        OneToOne
        };

    enum class CustomMapType
        {
        None,
        ForeignKeyOnTarget,
        ForeignKeyOnSource,
        LinkTable
        };

private:
    Cardinality m_cardinality;
    bool m_sourceColumnsMappingIsNull;
    RelationshipEndColumns m_sourceColumnsMapping;
    bool m_targetColumnsMappingIsNull;
    RelationshipEndColumns m_targetColumnsMapping;
    CustomMapType m_customMapType;
    bool m_allowDuplicateRelationships;
    ForeignKeyDbConstraint::ActionType m_onDeleteAction;
    ForeignKeyDbConstraint::ActionType m_onUpdateAction;
    bool m_createIndexOnForeignKey;
    std::set<DbTable const*>  m_sourceTables;
    std::set<DbTable const*>  m_targetTables;

    virtual BentleyStatus _InitializeFromSchema() override;
    virtual MappingStatus _EvaluateMapStrategy();

    BentleyStatus EvaluateLinkTableStrategy(UserECDbMapStrategy const&, ClassMap const* baseClassMap);
    BentleyStatus EvaluateForeignKeyStrategy(UserECDbMapStrategy const&, ClassMap const* baseClassMap);

    void DetermineCardinality();

    std::set<DbTable const*> GetTablesFromRelationshipEnd(ECN::ECRelationshipConstraintCR, bool ignoreJoinedTables) const;
    bool ContainsClassWithNotMappedStrategy(std::vector<ECN::ECClassCP> const& classes) const;
    static bool HasKeyProperties(ECN::ECRelationshipConstraint const&);

    static bool DetermineAllowDuplicateRelationshipsFlagFromRoot(ECN::ECRelationshipClassCR baseRelClass);

public:
    RelationshipMappingInfo(ECN::ECRelationshipClassCR relationshipClass, ECDbMap const& ecdbMap) : ClassMappingInfo(relationshipClass, ecdbMap), m_sourceColumnsMappingIsNull(true), m_targetColumnsMappingIsNull(true),
        m_customMapType(CustomMapType::None), m_allowDuplicateRelationships(false), 
        m_onDeleteAction(ForeignKeyDbConstraint::ActionType::NotSpecified), m_onUpdateAction(ForeignKeyDbConstraint::ActionType::NotSpecified), m_createIndexOnForeignKey(true)
        {}

    virtual ~RelationshipMappingInfo() {}

    Cardinality GetCardinality() const { return m_cardinality; }

    CustomMapType GetCustomMapType() const { return m_customMapType; }
    bool AllowDuplicateRelationships() const { BeAssert((m_customMapType == CustomMapType::LinkTable || m_customMapType == CustomMapType::None) && !m_resolvedStrategy.IsForeignKeyMapping()); return m_allowDuplicateRelationships; }
    ForeignKeyDbConstraint::ActionType GetOnDeleteAction() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping());  return m_onDeleteAction; }
    ForeignKeyDbConstraint::ActionType GetOnUpdateAction() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping()); return m_onUpdateAction; }
    bool CreateIndexOnForeignKey() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping()); return m_createIndexOnForeignKey; }

    RelationshipEndColumns const& GetColumnsMapping(ECN::ECRelationshipEnd end) const;
    std::set<DbTable const*> const& GetSourceTables() const {return m_sourceTables;}
    std::set<DbTable const*> const& GetTargetTables() const {return m_targetTables;}
    };


//======================================================================================
// @bsiclass                                                Affan.Khan  02/2012
//+===============+===============+===============+===============+===============+======
struct IndexMappingInfo : RefCountedBase
    {
    private:
        Utf8String m_name;
        bool m_isUnique;
        std::vector<Utf8String> m_properties;
        bool m_addPropsAreNotNullWhereExp;

        IndexMappingInfo(Utf8CP name, bool isUnique, std::vector<Utf8String> const& properties, bool addPropsAreNotNullWhereExp)
            : m_name(name), m_isUnique(isUnique), m_properties(properties), m_addPropsAreNotNullWhereExp(addPropsAreNotNullWhereExp)
            {}

        IndexMappingInfo(Utf8CP name, bool isUnique, bvector<Utf8String> const& properties, bool addPropsAreNotNullWhereExp)
            : m_name(name), m_isUnique(isUnique), m_addPropsAreNotNullWhereExp(addPropsAreNotNullWhereExp)
            {
            m_properties.insert(m_properties.begin(), properties.begin(), properties.end());
            }

        IndexMappingInfo(Utf8CP name, IndexMappingInfo const& rhs) : m_name(name), m_isUnique(rhs.m_isUnique), m_properties(rhs.m_properties), m_addPropsAreNotNullWhereExp(rhs.m_addPropsAreNotNullWhereExp) {}

        static BentleyStatus CreateFromIdSpecificationCAs(std::vector<IndexMappingInfoPtr>& indexInfos, ECDbCR, ECN::ECClassCR);

    public:
        static IndexMappingInfoPtr Clone(Utf8CP name, IndexMappingInfo const& rhs) { return new IndexMappingInfo(name, rhs); }
        //!@param customClassMap pass nullptr if @p ecClass doesn't have the ClassMap CA. 
        static BentleyStatus CreateFromECClass(std::vector<IndexMappingInfoPtr>& indexInfos, ECDbCR, ECN::ECClassCR ecClass, ECN::ECDbClassMap const* customClassMap);

        Utf8CP GetName() const { return m_name.c_str(); }
        bool GetIsUnique() const { return m_isUnique; }
        std::vector<Utf8String> const& GetProperties() const { return m_properties; }
        bool IsAddPropsAreNotNullWhereExp() const { return m_addPropsAreNotNullWhereExp; }
    };

//======================================================================================
// @bsiclass                                                Krischan.Eberle  02/2016
//+===============+===============+===============+===============+===============+======
struct IndexMappingInfoCache : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    SchemaImportContext const& m_schemaImportContext;
    mutable bmap<ClassMap const*, std::vector<IndexMappingInfoPtr>> m_indexInfoCache;

public:
    IndexMappingInfoCache(ECDbCR ecdb, SchemaImportContext const& ctx) : m_ecdb(ecdb), m_schemaImportContext(ctx) {}
    BentleyStatus TryGetIndexInfos(std::vector<IndexMappingInfoPtr> const*& indexInfos, ClassMap const&) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
