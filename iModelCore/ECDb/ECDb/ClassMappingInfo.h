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

//======================================================================================
// @bsienum                                                  Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
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
// @bsienum                                                 Krischan.Eberle  07/2017
//+===============+===============+===============+===============+===============+======
enum class RelationshipMappingType
    {
    ForeignKeyOnSource,
    ForeignKeyOnTarget,
    LinkTable
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
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipMappingInfo final : public ClassMappingInfo
    {
private:
    RelationshipMappingType m_mappingType;

    BentleyStatus _InitializeFromSchema(SchemaImportContext&) override;
    ClassMappingStatus _EvaluateMapStrategy(SchemaImportContext&) override;

    BentleyStatus EvaluateRootClassLinkTableStrategy(SchemaImportContext&, ClassMappingCACache const&);
    BentleyStatus EvaluateForeignKeyStrategy(SchemaImportContext&, ClassMappingCACache const&);

    BentleyStatus FailIfConstraintClassIsNotMapped() const;
public:
    RelationshipMappingInfo(ECDb const& ecdb, ECN::ECRelationshipClassCR relationshipClass)  : ClassMappingInfo(ecdb, relationshipClass) {}
    ~RelationshipMappingInfo() {}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
