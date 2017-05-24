/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "PropertyMap.h"
#include "IssueReporter.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ClassMapper final
    {
    private:
        //======================================================================================
        // @bsiclass                                              Krischan.Eberle        12/2016
        //======================================================================================
        struct TableMapper final : NonCopyableClass
            {
            private:
                TableMapper();
                ~TableMapper();

                static DbTable* CreateTableForExistingTableStrategy(ClassMap const&, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId);
                static DbTable* CreateTableForOtherStrategies(ClassMap const&, Utf8StringCR tableName, DbTable::Type, bool isVirtual, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable);

                static BentleyStatus CreateClassIdColumn(DbSchema&, DbTable&, PersistenceType);

            public:
                static DbTable* FindOrCreateTable(ClassMap const&, Utf8StringCR tableName, DbTable::Type, MapStrategyExtendedInfo const&, bool isVirtual, Utf8StringCR primaryKeyColumnName, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable);
            };

        ClassMap& m_classMap;
        DbClassMapLoadContext const* m_loadContext;

        explicit ClassMapper(ClassMap& classMap) : m_classMap(classMap), m_loadContext(nullptr) {}
        ClassMapper(ClassMap& classMap, DbClassMapLoadContext const& loadContext) : m_classMap(classMap), m_loadContext(&loadContext) {}

        PropertyMap* ProcessProperty(ECN::ECPropertyCR);

        RefCountedPtr<DataPropertyMap> MapPrimitiveProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* compoundPropMap);
        RefCountedPtr<Point2dPropertyMap> MapPoint2dProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
        RefCountedPtr<Point3dPropertyMap> MapPoint3dProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
        RefCountedPtr<PrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(ECN::PrimitiveArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<StructPropertyMap> MapStructProperty(ECN::StructECPropertyCR, StructPropertyMap const* parentPropMap);
        RefCountedPtr<StructArrayPropertyMap> MapStructArrayProperty(ECN::StructArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<NavigationPropertyMap> MapNavigationProperty(ECN::NavigationECPropertyCR);
        Utf8String ComputeAccessString(ECN::ECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        static ECN::ECRelationshipEnd GetConstraintEnd(ECN::NavigationECPropertyCR, NavigationPropertyMap::NavigationEnd);
        static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR, RelationshipClassMapCR, NavigationPropertyMap::NavigationEnd);

        static BentleyStatus DetermineColumnInfoForPrimitiveProperty(DbColumn::CreateParams&, ClassMap const&, ECN::PrimitiveECPropertyCR, Utf8StringCR accessString);

    public:
        static DbTable* FindOrCreateTable(ClassMap const& classMap, Utf8StringCR tableName, DbTable::Type tableType, MapStrategyExtendedInfo const& mapStrategy, bool isVirtual, Utf8StringCR primaryKeyColumnName, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable) { return TableMapper::FindOrCreateTable(classMap, tableName, tableType, mapStrategy, isVirtual, primaryKeyColumnName, exclusiveRootClassId, primaryTable); }

        static PropertyMap* MapProperty(ClassMap& classMap, ECN::ECPropertyCR ecProperty);
        static PropertyMap* LoadPropertyMap(ClassMap& classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext);
        static BentleyStatus CreateECInstanceIdPropertyMap(ClassMap& classMap);
        static BentleyStatus CreateECClassIdPropertyMap(ClassMap& classMap);
        static BentleyStatus SetupNavigationPropertyMap(NavigationPropertyMap& propertyMap);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE