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
    ECN::PrimitiveECPropertyCP m_classHasCurrentTimeStampProperty = nullptr;

    ClassMappingStatus EvaluateMapStrategy(SchemaImportContext&);

    ClassMappingStatus TryGetBaseClassMap(ClassMap const*& baseClassMap) const;
    BentleyStatus InitializeClassHasCurrentTimeStampProperty();

protected:

    virtual BentleyStatus _InitializeFromSchema(SchemaImportContext&);
    virtual ClassMappingStatus _EvaluateMapStrategy(SchemaImportContext&);

    BentleyStatus EvaluateRootClassMapStrategy(SchemaImportContext&, ClassMappingCACache const&);
    BentleyStatus EvaluateNonRootClassMapStrategy(SchemaImportContext&, ClassMap const& baseClassMap, ClassMappingCACache const&);

    BentleyStatus EvaluateNonRootClassTablePerHierarchyMapStrategy(SchemaImportContext&, ClassMap const& baseClassMap, ClassMappingCACache const&);
    bool ValidateNonRootClassTablePerHierarchyStrategy(MapStrategyExtendedInfo const& baseStrategy, ClassMappingCACache const&) const;
    
    BentleyStatus AssignMapStrategy(ClassMappingCACache const&);

    IssueReporter const& Issues() const;

    static MapStrategy GetDefaultStrategy(ECN::ECClassCR);

public:
    ClassMappingInfo(ECDb const& ecdb, ECN::ECClassCR ecClass) : m_ecdb(ecdb), m_ecClass(ecClass) {}
    virtual ~ClassMappingInfo() {}

    ClassMappingStatus Initialize(SchemaImportContext&);

    MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
    ClassMap const* GetTphBaseClassMap() const { BeAssert(m_mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy); return m_tphBaseClassMap; }
    DbMap const& GetDbMap() const {return m_ecdb.Schemas().GetDbMap();}
    ECN::ECClassCR GetClass() const {return m_ecClass;}
    Utf8StringCR GetTableName() const {return m_tableName;}
    Utf8StringCR GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName;}
    ECN::PrimitiveECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }

    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle     06/2017
//+===============+===============+===============+===============+===============+======
struct RelationshipMappingType
    {
    enum class Type
        {
        PhysicalForeignKey,
        LogicalForeignKey,
        LinkTable
        };

    private:
        Type m_type;

    protected:
        explicit RelationshipMappingType(Type type) : m_type(type) {}

    public:
        virtual ~RelationshipMappingType() {}
        Type GetType() const { return m_type; }

        bool IsLinkTable() const { return m_type == Type::LinkTable; }

        template<typename T>
        T const& GetAs() const
            {
            BeAssert(dynamic_cast<T const*> (this) != nullptr);
            return static_cast<T const&> (*this);
            }
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle     06/2017
//+===============+===============+===============+===============+===============+======
struct ForeignKeyMappingType : RelationshipMappingType
    {
private:
    ECN::ECRelationshipEnd m_fkEnd;

protected:
    ForeignKeyMappingType(Type type, ECN::ECRelationshipEnd fkEnd) : RelationshipMappingType(type), m_fkEnd(fkEnd) {}

public:
    static std::unique_ptr<ForeignKeyMappingType> Create(ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd, ForeignKeyConstraintCustomAttribute const&, IssueReporter const&);

    virtual ~ForeignKeyMappingType() {}
    ECN::ECRelationshipEnd GetFkEnd() const { return m_fkEnd; }
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle     06/2017
//+===============+===============+===============+===============+===============+======
struct LogicalForeignKeyMappingType final : ForeignKeyMappingType
    {
    public:
        explicit LogicalForeignKeyMappingType(ECN::ECRelationshipEnd fkEnd) : ForeignKeyMappingType(Type::LogicalForeignKey, fkEnd) {}
        ~LogicalForeignKeyMappingType() {}
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle     06/2017
//+===============+===============+===============+===============+===============+======
struct PhysicalForeignKeyMappingType final : ForeignKeyMappingType
    {
    private:
        ForeignKeyDbConstraint::ActionType m_onDeleteAction = ForeignKeyDbConstraint::ActionType::NotSpecified;
        ForeignKeyDbConstraint::ActionType m_onUpdateAction = ForeignKeyDbConstraint::ActionType::NotSpecified;

    public:
        PhysicalForeignKeyMappingType(ECN::ECRelationshipEnd fkEnd, ForeignKeyDbConstraint::ActionType onDeleteAction, ForeignKeyDbConstraint::ActionType onUpdateAction)
            : ForeignKeyMappingType(Type::PhysicalForeignKey, fkEnd), m_onDeleteAction(onDeleteAction), m_onUpdateAction(onUpdateAction)
            {}

        ~PhysicalForeignKeyMappingType() {}
        
        ForeignKeyDbConstraint::ActionType GetOnDeleteAction() const { return m_onDeleteAction; }
        ForeignKeyDbConstraint::ActionType GetOnUpdateAction() const { return m_onUpdateAction; }
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle     06/2017
//+===============+===============+===============+===============+===============+======
struct LinkTableMappingType final : RelationshipMappingType
    {
    private:
        Nullable<Utf8String> m_sourceIdColumnName;
        Nullable<Utf8String> m_targetIdColumnName;
        bool m_createForeignKeyConstraints = true;
        bool m_allowDuplicateRelationships = false;

        LinkTableMappingType() : RelationshipMappingType(Type::LinkTable) {}
        LinkTableMappingType(Nullable<Utf8String> const& sourceIdColName, Nullable<Utf8String> const& targetIdColName, bool createFkConstraints, bool allowDuplicateRelationships) : RelationshipMappingType(Type::LinkTable), m_sourceIdColumnName(sourceIdColName), m_targetIdColumnName(targetIdColName), m_createForeignKeyConstraints(createFkConstraints), m_allowDuplicateRelationships(allowDuplicateRelationships) {}

    public:
        ~LinkTableMappingType() {}

        static std::unique_ptr<LinkTableMappingType> Create(LinkTableRelationshipMapCustomAttribute const&);

        Nullable<Utf8String> const& GetSourceIdColumnName() const { return m_sourceIdColumnName; }
        Nullable<Utf8String> const& GetTargetIdColumnName() const { return m_targetIdColumnName; }
        bool GetCreateForeignKeyConstraintsFlag() const { return m_createForeignKeyConstraints; }
        bool AllowDuplicateRelationships() const { return m_allowDuplicateRelationships; }
    };

//======================================================================================
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipMappingInfo final : public ClassMappingInfo
    {
private:
    bool m_isRootClass = false;
    std::unique_ptr<RelationshipMappingType> m_mappingType;
    std::set<DbTable const*> m_sourceTables;
    std::set<DbTable const*> m_targetTables;

    BentleyStatus _InitializeFromSchema(SchemaImportContext&) override;
    ClassMappingStatus _EvaluateMapStrategy(SchemaImportContext&) override;

    BentleyStatus EvaluateRootClassLinkTableStrategy(SchemaImportContext&, ClassMappingCACache const&);
    BentleyStatus EvaluateForeignKeyStrategy(SchemaImportContext&, ClassMappingCACache const&);

    BentleyStatus FailIfConstraintClassIsNotMapped() const;

    //! Determines whether the specified ECRelationship requires to be mapped to a link table.
    static BentleyStatus TryDetermineMappingType(std::unique_ptr<RelationshipMappingType>&, ECDbCR, SchemaImportContext const&, ECN::ECRelationshipClassCR);

    static BentleyStatus TryDetermineFkEnd(ECN::ECRelationshipEnd&, ECN::ECRelationshipClassCR, IssueReporter const&);

public:
    RelationshipMappingInfo(ECDb const& ecdb, ECN::ECRelationshipClassCR relationshipClass)  : ClassMappingInfo(ecdb, relationshipClass), m_isRootClass(!relationshipClass.HasBaseClasses()) {}

    ~RelationshipMappingInfo() {}

    static std::set<DbTable const*> GetTablesFromRelationshipEnd(DbMap const&  dbMap, SchemaImportContext&, ECN::ECRelationshipConstraintCR, bool ignoreJoinedTables) ;

    //only available for root classes. Subclasses just inherit from their base class
    RelationshipMappingType const& GetMappingType() const { BeAssert(m_isRootClass); return *m_mappingType; }
    std::set<DbTable const*> const& GetSourceTables() const { BeAssert(m_isRootClass); return m_sourceTables; }
    std::set<DbTable const*> const& GetTargetTables() const { BeAssert(m_isRootClass); return m_targetTables;}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
