/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ECDbInternalTypes.h"
#include "MapStrategy.h"
#include "DbSchema.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECDbMap;
struct ClassMap;
struct ClassMappingInfo;
struct SchemaImportContext;

enum class ClassMappingStatus
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
    static std::unique_ptr<ClassMappingInfo> Create(ClassMappingStatus&, ECDb const&, ECN::ECClassCR);
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
    bool m_mapsToVirtualTable;
    ECN::ECPropertyCP m_classHasCurrentTimeStampProperty;

protected:
    ECDb const& m_ecdb;
    ECN::ECClassCR m_ecClass;
    MapStrategyExtendedInfo m_mapStrategyExtInfo;

    ClassMap const* m_tphBaseClassMap;

private:
    ClassMappingStatus EvaluateMapStrategy();
    virtual ClassMappingStatus _EvaluateMapStrategy();

    ClassMappingStatus TryGetBaseClassMap(ClassMap const*& baseClassMap) const;
    BentleyStatus InitializeClassHasCurrentTimeStampProperty();

protected:
    virtual BentleyStatus _InitializeFromSchema();

    BentleyStatus EvaluateTablePerHierarchyMapStrategy(ClassMap const& baseClassMap, ClassMappingCACache const&);
    bool ValidateTablePerHierarchyChildStrategy(MapStrategyExtendedInfo const& baseStrategy, ClassMappingCACache const&) const;
    BentleyStatus AssignMapStrategy(ClassMappingCACache const&);

    IssueReporter const& Issues() const;
    static void LogClassNotMapped (NativeLogging::SEVERITY, ECN::ECClassCR, Utf8CP explanation);

public:
    ClassMappingInfo(ECDb const&, ECN::ECClassCR);
    virtual ~ClassMappingInfo() {}

    ClassMappingStatus Initialize();

    MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
    ClassMap const* GetTphBaseClassMap() const { BeAssert(m_mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy); return m_tphBaseClassMap; }
    ECDbMap const& GetDbMap() const {return m_ecdb.Schemas().GetDbMap();}
    ECN::ECClassCR GetECClass() const {return m_ecClass;}
    std::vector<IndexMappingInfoPtr> const& GetIndexInfos() const { return m_dbIndexes;}
    Utf8StringCR GetTableName() const {return m_tableName;}
    Utf8StringCR GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName;}
    ECN::ECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    //! Virtual tables are not persisted   
    bool MapsToVirtualTable () const { return m_mapsToVirtualTable; }
    };

//======================================================================================
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

    struct FkMappingInfo : NonCopyableClass
        {
        private:
            ForeignKeyDbConstraint::ActionType m_onDeleteAction;
            ForeignKeyDbConstraint::ActionType m_onUpdateAction;
            bool m_useECInstanceIdAsFk;

        public:
            explicit FkMappingInfo(bool usePkAsFk) : FkMappingInfo(ForeignKeyDbConstraint::ActionType::NotSpecified, ForeignKeyDbConstraint::ActionType::NotSpecified, usePkAsFk) {}
            FkMappingInfo(ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction, bool usePkAsFk)
                : m_onDeleteAction(onDeleteAction), m_onUpdateAction(onUpdateAction), m_useECInstanceIdAsFk(usePkAsFk)
                {}

            ForeignKeyDbConstraint::ActionType GetOnDeleteAction() const { return m_onDeleteAction; }
            ForeignKeyDbConstraint::ActionType GetOnUpdateAction() const { return m_onUpdateAction; }
            bool UseECInstanceIdAsFk() const { return m_useECInstanceIdAsFk; }
        };

    struct LinkTableMappingInfo : NonCopyableClass
        {
    private:
        Utf8String m_sourceIdColumnName;
        Utf8String m_targetIdColumnName;
        bool m_allowDuplicateRelationships;
    
    public:
        LinkTableMappingInfo() : LinkTableMappingInfo(false) {}
        LinkTableMappingInfo(bool allowDuplicateRelationships) : m_allowDuplicateRelationships(allowDuplicateRelationships) {}
        LinkTableMappingInfo(Utf8StringCR sourceIdColname, Utf8StringCR targetIdColName, bool allowDuplicateRelationships)
            : m_sourceIdColumnName(sourceIdColname), m_targetIdColumnName(targetIdColName), m_allowDuplicateRelationships(allowDuplicateRelationships)
            {}

        Utf8StringCR GetSourceIdColumnName() const { return m_sourceIdColumnName; }
        Utf8StringCR GetTargetIdColumnName() const { return m_targetIdColumnName; }
        bool AllowDuplicateRelationships() const { return m_allowDuplicateRelationships; }
        };
private:
    Cardinality m_cardinality;
    CustomMapType m_customMapType;
    std::unique_ptr<FkMappingInfo> m_fkMappingInfo;
    std::unique_ptr<LinkTableMappingInfo> m_linkTableMappingInfo;
    std::set<DbTable const*> m_sourceTables;
    std::set<DbTable const*> m_targetTables;

    virtual BentleyStatus _InitializeFromSchema() override;
    virtual ClassMappingStatus _EvaluateMapStrategy();

    BentleyStatus EvaluateLinkTableStrategy(ClassMappingCACache const&, ClassMap const* baseClassMap);
    BentleyStatus EvaluateForeignKeyStrategy(ClassMappingCACache const&, ClassMap const* baseClassMap);

    void DetermineCardinality();

    std::set<DbTable const*> GetTablesFromRelationshipEnd(ECN::ECRelationshipConstraintCR, bool ignoreJoinedTables) const;
    bool ContainsClassWithNotMappedStrategy(std::vector<ECN::ECClassCP> const& classes) const;
    static bool HasKeyProperties(ECN::ECRelationshipConstraint const&);

    static bool DetermineAllowDuplicateRelationshipsFlagFromRoot(ECN::ECRelationshipClassCR baseRelClass);

    bool RequiresLinkTable() const { return m_cardinality == Cardinality::ManyToMany || m_ecClass.GetPropertyCount() > 0; }

public:
    RelationshipMappingInfo(ECDb const& ecdb, ECN::ECRelationshipClassCR relationshipClass) : ClassMappingInfo(ecdb, relationshipClass),
        m_customMapType(CustomMapType::None), m_fkMappingInfo(nullptr), m_linkTableMappingInfo(nullptr) 
        {
        DetermineCardinality();
        }

    virtual ~RelationshipMappingInfo() {}

    Cardinality GetCardinality() const { return m_cardinality; }
    CustomMapType GetCustomMapType() const { return m_customMapType; }
    FkMappingInfo const* GetFkMappingInfo() const { BeAssert(m_fkMappingInfo != nullptr); return m_fkMappingInfo.get(); }
    LinkTableMappingInfo const* GetLinkTableMappingInfo() const { BeAssert(m_linkTableMappingInfo != nullptr); return m_linkTableMappingInfo.get(); }
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
        //pass nullptr to DbIndexList if ecClass doesn't have the DbIndexList CA. 
        static BentleyStatus CreateFromECClass(std::vector<IndexMappingInfoPtr>& , ECDbCR, ECN::ECClassCR, ECN::DbIndexList const&);

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
