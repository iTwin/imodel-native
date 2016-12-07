/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        ClassMapR m_classMap;
        DbClassMapLoadContext const* m_loadContext;

        explicit ClassMapper(ClassMapR classMap) : m_classMap(classMap), m_loadContext(nullptr) {}
        ClassMapper(ClassMapR classMap, DbClassMapLoadContext const& loadContext) : m_classMap(classMap), m_loadContext(&loadContext) {}

        PropertyMap* ProcessProperty(ECN::ECPropertyCR);

        RefCountedPtr<Point2dPropertyMap> MapPoint2dProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<Point3dPropertyMap> MapPoint3dProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<DataPropertyMap> MapPrimitiveProperty(ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* compoundPropMap);
        RefCountedPtr<PrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(ECN::PrimitiveArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<StructPropertyMap> MapStructProperty(ECN::StructECPropertyCR, StructPropertyMap const* parentPropMap);
        RefCountedPtr<StructArrayPropertyMap> MapStructArrayProperty(ECN::StructArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        RefCountedPtr<NavigationPropertyMap> MapNavigationProperty(ECN::NavigationECPropertyCR);
        Utf8String ComputeAccessString(ECN::ECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
        DbColumn* DoFindOrCreateColumnsInTable(ECN::ECPropertyCR, Utf8StringCR accessString, DbColumn::Type);
        static ECN::ECRelationshipEnd GetConstraintEnd(ECN::NavigationECPropertyCR, NavigationPropertyMap::NavigationEnd);
        static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR, RelationshipClassMapCR, NavigationPropertyMap::NavigationEnd);

    public:
        static BentleyStatus DetermineColumnInfo(DbColumn::CreateParams&, ECDbCR ecdb, ECN::ECPropertyCR ecProp, Utf8StringCR propAccessString);
        static PropertyMap* MapProperty(ClassMapR classMap, ECN::ECPropertyCR ecProperty);
        static PropertyMap* LoadPropertyMap(ClassMapR classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext);
        static BentleyStatus CreateECInstanceIdPropertyMap(ClassMap& classMap);
        static BentleyStatus CreateECClassIdPropertyMap(ClassMap& classMap);
        static BentleyStatus SetupNavigationPropertyMap(NavigationPropertyMap& propertyMap);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE