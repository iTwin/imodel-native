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
struct ClassMappingInfo final : NonCopyableClass
{
private:
    SchemaImportContext& m_ctx;
    ECN::ECClassCR m_ecClass;
    Nullable<MapStrategy> m_userDefinedStrategy;
    MapStrategyExtendedInfo m_mapStrategyExtInfo;
    ShareColumnsCustomAttribute m_shareColumnsCA;
    bool m_hasJoinedTablePerDirectSubclassCA = false;
    Nullable<RelationshipMappingType> m_relMappingType;

    ClassMap const* m_tphBaseClassMap = nullptr;

    Utf8String m_tableName;
    Utf8String m_ecInstanceIdColumnName;
    ECN::PrimitiveECPropertyCP m_classHasCurrentTimeStampProperty = nullptr;

    BentleyStatus InitializeFromSchema();
    BentleyStatus InitializeClassHasCurrentTimeStampProperty();
    ClassMappingStatus EvaluateMapStrategy();
    BentleyStatus EvaluateRootClassMapStrategy();
    BentleyStatus EvaluateNonRootClassMapStrategy(ClassMap const& baseClassMap);
    BentleyStatus EvaluateNonRootClassTablePerHierarchyMapStrategy(ClassMap const& baseClassMap);

    DbMap const& GetDbMap() const;
    IssueReporter const& Issues() const;

    static ClassMappingStatus TryGetBaseClassMap(ClassMap const*& baseClassMap, ECDbCR, ECN::ECClassCR);

public:
    ClassMappingInfo(SchemaImportContext& ctx, ECN::ECClassCR ecClass) : m_ctx(ctx), m_ecClass(ecClass) {}
    ~ClassMappingInfo() {}

    ClassMappingStatus Initialize();

    ECN::ECClassCR GetClass() const { return m_ecClass; }
    MapStrategyExtendedInfo const& GetMapStrategy() const { return m_mapStrategyExtInfo; }
    ClassMap const* GetTphBaseClassMap() const { BeAssert(m_mapStrategyExtInfo.IsTablePerHierarchy()); return m_tphBaseClassMap; }
    Utf8StringCR GetTableName() const {return m_tableName;}
    Utf8StringCR GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName;}
    ECN::PrimitiveECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
