/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ECDbInternalTypes.h"
#include "MapStrategy.h"
#include "DbSchema.h"
#include "IssueReporter.h"
#include <set>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct DbMap;
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
struct ClassMappingInfoFactory final
    {
private:
    ClassMappingInfoFactory ();
    ~ClassMappingInfoFactory ();

public:
    static std::unique_ptr<ClassMappingInfo> Create(ClassMappingStatus&, SchemaImportContext&, ECDb const&, ECN::ECClassCR);
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
protected:
    ECDb const& m_ecdb;
    ECN::ECClassCR m_ecClass;
    MapStrategyExtendedInfo m_mapStrategyExtInfo;

    ClassMap const* m_tphBaseClassMap = nullptr;

private:
    Utf8String m_tableName;
    Utf8String m_ecInstanceIdColumnName;
    std::vector<IndexMappingInfoPtr> m_dbIndexes;
    ECN::PrimitiveECPropertyCP m_classHasCurrentTimeStampProperty;

    ClassMappingStatus EvaluateMapStrategy(SchemaImportContext&);
    virtual ClassMappingStatus _EvaluateMapStrategy(SchemaImportContext&);

    ClassMappingStatus TryGetBaseClassMap(ClassMap const*& baseClassMap) const;
    BentleyStatus InitializeClassHasCurrentTimeStampProperty();

protected:
    virtual BentleyStatus _InitializeFromSchema(SchemaImportContext&);

    BentleyStatus EvaluateTablePerHierarchyMapStrategy(SchemaImportContext&, ClassMap const& baseClassMap, ClassMappingCACache const&);
    bool ValidateTablePerHierarchyChildStrategy(MapStrategyExtendedInfo const& baseStrategy, ClassMappingCACache const&) const;
    BentleyStatus AssignMapStrategy(ClassMappingCACache const&);

    IssueReporter const& Issues() const;
    static void LogClassNotMapped (NativeLogging::SEVERITY, ECN::ECClassCR, Utf8CP explanation);

public:
    ClassMappingInfo(ECDb const&, ECN::ECClassCR);
    virtual ~ClassMappingInfo() {}

    ClassMappingStatus Initialize(SchemaImportContext&);

    MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
    ClassMap const* GetTphBaseClassMap() const { BeAssert(m_mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy); return m_tphBaseClassMap; }
    DbMap const& GetDbMap() const {return m_ecdb.Schemas().GetDbMap();}
    ECN::ECClassCR GetClass() const {return m_ecClass;}
    std::vector<IndexMappingInfoPtr> const& GetIndexInfos() const { return m_dbIndexes;}
    Utf8StringCR GetTableName() const {return m_tableName;}
    Utf8StringCR GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName;}
    ECN::PrimitiveECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    };

//======================================================================================
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipMappingInfo final : public ClassMappingInfo
    {
public:
    struct FkMappingInfo final : NonCopyableClass
        {
        private:
            ECN::ECRelationshipEnd m_fkEnd;
            bool m_useECInstanceIdAsFk;
            bool m_isPhysicalFk = false;
            ForeignKeyDbConstraint::ActionType m_onDeleteAction = ForeignKeyDbConstraint::ActionType::NotSpecified;
            ForeignKeyDbConstraint::ActionType m_onUpdateAction = ForeignKeyDbConstraint::ActionType::NotSpecified;

        public:
            FkMappingInfo(ECN::ECRelationshipEnd fkEnd, bool usePkAsFk) : m_fkEnd(fkEnd), m_useECInstanceIdAsFk(usePkAsFk) {}

            FkMappingInfo(ECN::ECRelationshipEnd fkEnd, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction, bool usePkAsFk)
                : m_fkEnd(fkEnd), m_useECInstanceIdAsFk(usePkAsFk), m_isPhysicalFk(true), m_onDeleteAction(onDeleteAction), m_onUpdateAction(onUpdateAction)
                {}

            ECN::ECRelationshipEnd GetFkEnd() const { return m_fkEnd; }
            bool UseECInstanceIdAsFk() const { return m_useECInstanceIdAsFk; }
            bool IsPhysicalFk() const { return m_isPhysicalFk; }
            ForeignKeyDbConstraint::ActionType GetOnDeleteAction() const { BeAssert(IsPhysicalFk()); return m_onDeleteAction; }
            ForeignKeyDbConstraint::ActionType GetOnUpdateAction() const { BeAssert(IsPhysicalFk()); return m_onUpdateAction; }
        };

    struct LinkTableMappingInfo final : NonCopyableClass
        {
    private:
        Nullable<Utf8String> m_sourceIdColumnName;
        Nullable<Utf8String> m_targetIdColumnName;
        bool m_createForeignKeyConstraints = true;
        bool m_allowDuplicateRelationships = false;
    
    public:
        LinkTableMappingInfo() {}
        LinkTableMappingInfo(Nullable<Utf8String> const& sourceIdColname, Nullable<Utf8String> const& targetIdColName, Nullable<bool> createForeignKeyConstraints, Nullable<bool> allowDuplicateRelationships)
            : m_sourceIdColumnName(sourceIdColname), m_targetIdColumnName(targetIdColName)
            {
            if (!createForeignKeyConstraints.IsNull())
                m_createForeignKeyConstraints = createForeignKeyConstraints.Value();

            if (!allowDuplicateRelationships.IsNull())
                m_allowDuplicateRelationships = allowDuplicateRelationships.Value();
            }

        Nullable<Utf8String> const& GetSourceIdColumnName() const { return m_sourceIdColumnName; }
        Nullable<Utf8String> const& GetTargetIdColumnName() const { return m_targetIdColumnName; }
        bool GetCreateForeignKeyConstraintsFlag() const { return m_createForeignKeyConstraints; }
        bool AllowDuplicateRelationships() const { return m_allowDuplicateRelationships; }
        };
private:
    bool m_isRootClass = false;
    std::unique_ptr<FkMappingInfo> m_fkMappingInfo;
    std::unique_ptr<LinkTableMappingInfo> m_linkTableMappingInfo;
    std::set<DbTable const*> m_sourceTables;
    std::set<DbTable const*> m_targetTables;

    BentleyStatus _InitializeFromSchema(SchemaImportContext&) override;
    ClassMappingStatus _EvaluateMapStrategy(SchemaImportContext&) override;

    BentleyStatus EvaluateLinkTableStrategy(SchemaImportContext&, ClassMappingCACache const&, ClassMap const* baseClassMap);
    BentleyStatus EvaluateForeignKeyStrategy(SchemaImportContext&, ClassMappingCACache const&, ClassMap const* baseClassMap);

    BentleyStatus FailIfConstraintClassIsNotMapped() const;

    //! Determines whether the specified ECRelationship requires to be mapped to a link table.
    BentleyStatus DetermineFkOrLinkTableMapping(bool& isFkMapping, ECN::ECRelationshipEnd& fkEnd, bool useECInstanceIdAsFk, LinkTableRelationshipMapCustomAttribute const&) const;

    static BentleyStatus TryDetermineFkEnd(ECN::ECRelationshipEnd&, ECN::ECRelationshipClassCR, IssueReporter const&);

public:
    RelationshipMappingInfo(ECDb const& ecdb, ECN::ECRelationshipClassCR relationshipClass) 
        : ClassMappingInfo(ecdb, relationshipClass), m_isRootClass(!relationshipClass.HasBaseClasses()) {}

    ~RelationshipMappingInfo() {}
    static std::set<DbTable const*> GetTablesFromRelationshipEnd(DbMap const&  dbMap, SchemaImportContext&, ECN::ECRelationshipConstraintCR, bool ignoreJoinedTables) ;

    //only available for root classes. Subclasses just inherit from their base class
    FkMappingInfo const* GetFkMappingInfo() const { BeAssert(m_isRootClass && m_fkMappingInfo != nullptr); return m_fkMappingInfo.get(); }
    //only available for root classes. Subclasses just inherit from their base class
    LinkTableMappingInfo const* GetLinkTableMappingInfo() const { BeAssert(m_isRootClass && m_linkTableMappingInfo != nullptr); return m_linkTableMappingInfo.get(); }
    std::set<DbTable const*> const& GetSourceTables() const { BeAssert(m_isRootClass); return m_sourceTables; }
    std::set<DbTable const*> const& GetTargetTables() const { BeAssert(m_isRootClass); return m_targetTables;}
    };


//======================================================================================
// @bsiclass                                                Affan.Khan  02/2012
//+===============+===============+===============+===============+===============+======
struct IndexMappingInfo final : RefCountedBase
    {
    private:
        Nullable<Utf8String> m_name;
        bool m_isUnique = false;
        std::vector<Utf8String> m_properties;
        bool m_addPropsAreNotNullWhereExp;

        IndexMappingInfo(Nullable<Utf8String> const& name, Nullable<bool> isUnique, std::vector<Utf8String> const& properties, bool addPropsAreNotNullWhereExp)
            : m_name(name), m_isUnique(isUnique.IsNull() ? false : isUnique.Value()), m_properties(properties), m_addPropsAreNotNullWhereExp(addPropsAreNotNullWhereExp)
            {}

        IndexMappingInfo(Nullable<Utf8String> const& name, Nullable<bool> isUnique, bvector<Utf8String> const& properties, bool addPropsAreNotNullWhereExp)
            : m_name(name), m_isUnique(isUnique.IsNull() ? false : isUnique.Value()), m_addPropsAreNotNullWhereExp(addPropsAreNotNullWhereExp)
            {
            m_properties.insert(m_properties.begin(), properties.begin(), properties.end());
            }

        IndexMappingInfo(Nullable<Utf8String> const& name, IndexMappingInfo const& rhs) : m_name(name), m_isUnique(rhs.m_isUnique), m_properties(rhs.m_properties), m_addPropsAreNotNullWhereExp(rhs.m_addPropsAreNotNullWhereExp) {}

    public:
        static IndexMappingInfoPtr Clone(Nullable<Utf8String> const& name, IndexMappingInfo const& rhs) { return new IndexMappingInfo(name, rhs); }
        static BentleyStatus CreateFromECClass(std::vector<IndexMappingInfoPtr>&, ECDbCR, ECN::ECClassCR, DbIndexListCustomAttribute const&);

        Nullable<Utf8String> const& GetName() const { return m_name; }
        bool GetIsUnique() const { return m_isUnique; }
        std::vector<Utf8String> const& GetProperties() const { return m_properties; }
        bool IsAddPropsAreNotNullWhereExp() const { return m_addPropsAreNotNullWhereExp; }
    };

//======================================================================================
// @bsiclass                                                Krischan.Eberle  02/2016
//+===============+===============+===============+===============+===============+======
struct IndexMappingInfoCache final : NonCopyableClass
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
