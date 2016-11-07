/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbPch.h"
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

    private:
        ClassMapper(ClassMapR classMap) : m_classMap(classMap), m_loadContext(nullptr) {}
        ClassMapper(ClassMapR classMap, DbClassMapLoadContext const& loadContext) : m_classMap(classMap), m_loadContext(&loadContext) {}

        PropertyMap* ProcessProperty(ECN::ECPropertyCR);

        RefCountedPtr<Point2dPropertyMap> MapPoint2dProperty(ECN::PrimitiveECPropertyCR, StructPropertyMapBuilder* parentBuilder);
        RefCountedPtr<Point3dPropertyMap> MapPoint3dProperty(ECN::PrimitiveECPropertyCR, StructPropertyMapBuilder* parentBuilder);
        RefCountedPtr<DataPropertyMap> MapPrimitiveProperty(ECN::PrimitiveECPropertyCR, StructPropertyMapBuilder* parentBuilder);
        RefCountedPtr<PrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(ECN::PrimitiveArrayECPropertyCR, StructPropertyMapBuilder* parentBuilder);
        RefCountedPtr<StructPropertyMap> MapStructProperty(ECN::StructECPropertyCR, StructPropertyMapBuilder* parentBuilder);
        RefCountedPtr<StructArrayPropertyMap> MapStructArrayProperty(ECN::StructArrayECPropertyCR, StructPropertyMapBuilder* parentBuilder);
        RefCountedPtr<NavigationPropertyMap> MapNavigationProperty(ECN::NavigationECPropertyCR);
        Utf8String ComputeAccessString(ECN::ECPropertyCR, DataPropertyMap const* parent);
        DbColumn* DoFindOrCreateColumnsInTable(ECN::ECPropertyCR, Utf8CP accessString, DbColumn::Type);
        static ECN::ECRelationshipEnd GetConstraintEnd(ECN::NavigationECPropertyCR, NavigationPropertyMap::NavigationEnd);
        static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR, RelationshipClassMapCR, NavigationPropertyMap::NavigationEnd);

    public:
        //Navigation property map is not finished. It require a second pass and Nav->Setup() method must be called on it.
        //This method does not create system property maps
        static BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation& collation, ECDbCR ecdb, ECN::ECPropertyCR ecProp, Utf8CP propAccessString);
        static PropertyMap* MapProperty(ClassMapR classMap, ECN::ECPropertyCR ecProperty);
        static PropertyMap* LoadPropertyMap(ClassMapR classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext);
        static BentleyStatus CreateECInstanceIdPropertyMap(ClassMap& classMap);
        static BentleyStatus CreateECClassIdPropertyMap(ClassMap& classMap);
        static BentleyStatus SetupNavigationPropertyMap(NavigationPropertyMap& propertyMap);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE