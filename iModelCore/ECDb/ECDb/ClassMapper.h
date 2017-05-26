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
#include "ClassMappingInfo.h"
#include "IssueReporter.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ClassMapper final
    {
    public:
        //======================================================================================
        // @bsiclass                                              Krischan.Eberle        12/2016
        //======================================================================================
        struct TableMapper final : NonCopyableClass
            {
            private:
                TableMapper();
                ~TableMapper();

                static DbTable* FindOrCreateTable(ClassMap const&, ClassMappingInfo const&, DbTable::Type, DbTable const* primaryTable);
                static DbTable* CreateTableForExistingTableStrategy(ClassMap const&, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId);
                static DbTable* CreateTableForOtherStrategies(ClassMap const&, Utf8StringCR tableName, DbTable::Type, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable);

                static BentleyStatus CreateClassIdColumn(DbSchema&, DbTable&, PersistenceType);
                
                static bool IsExclusiveRootClassOfTable(ClassMappingInfo const&);
                static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);

            public:
                static BentleyStatus MapToTable(ClassMap&, ClassMappingInfo const&);
                static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
            };

        enum class PropertyMapInheritanceMode
            {
            NotInherited, //!< indicates that base property map is not inherited, but created from scratch
            Clone //! inherited property maps areGet cloned from the base class property map
            };


    private:
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
        static PropertyMap* MapProperty(ClassMap&, ECN::ECPropertyCR);
        static PropertyMap* LoadPropertyMap(ClassMap&, ECN::ECPropertyCR, DbClassMapLoadContext const&);
        static BentleyStatus SetupNavigationPropertyMap(NavigationPropertyMap&);

        //! Rules:
        //! If MapStrategy != TPH: NotInherited
        //! Else: Clone
        static PropertyMapInheritanceMode GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE