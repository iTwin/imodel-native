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
        RefCountedPtr<WipPoint2dPropertyMap> MapPoint2dProperty(ECN::PrimitiveECPropertyCR property, WipVerticalPropertyMap const* parent);
        RefCountedPtr<WipPoint3dPropertyMap> MapPoint3dProperty(ECN::PrimitiveECPropertyCR property, WipVerticalPropertyMap const* parent);
        RefCountedPtr<WipVerticalPropertyMap> MapPrimitiveProperty(ECN::PrimitiveECPropertyCR property, WipVerticalPropertyMap const* parent);
        RefCountedPtr<WipPrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(ECN::ArrayECPropertyCR property, WipVerticalPropertyMap const* parent);
        RefCountedPtr<WipStructPropertyMap> MapStructProperty(ECN::StructECPropertyCR property, WipVerticalPropertyMap const* parent);
        RefCountedPtr<WipStructArrayPropertyMap> MapStructArrayProperty(ECN::StructArrayECPropertyCR property, WipVerticalPropertyMap const* parent);
        RefCountedPtr<WipNavigationPropertyMap> MapNavigationProperty(ECN::NavigationECPropertyCR property);
        Utf8String ComputeAccessString(ECN::ECPropertyCR ecProperty, WipVerticalPropertyMap const* parent);
        DbColumn* DoFindOrCreateColumnsInTable(ECN::ECPropertyCR ecProperty, Utf8CP accessString, DbColumn::Type colType);
        WipPropertyMap* ProcessProperty(ECN::ECPropertyCR ecProperty);
        static ECN::ECRelationshipEnd GetConstraintEnd(ECN::NavigationECPropertyCR prop, WipNavigationPropertyMap::NavigationEnd end);
        static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR navProp, RelationshipClassMapCR relClassMap, WipNavigationPropertyMap::NavigationEnd end);

    public:
        //Navigation property map is not finished. It require a second pass and Nav->Setup() method must be called on it.
        //This method does not create system property maps
        static ECN::ECRelationshipConstraintCR GetConstraint(ECN::NavigationECPropertyCR navProp, WipNavigationPropertyMap::NavigationEnd end);
        static BentleyStatus DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraints::Collation& collation, ECDbCR ecdb, ECN::ECPropertyCR ecProp, Utf8CP propAccessString);
        static WipPropertyMap* MapProperty(ClassMapR classMap, ECN::ECPropertyCR ecProperty);
        static WipPropertyMap* LoadPropertyMap(ClassMapR classMap, ECN::ECPropertyCR ecProperty, DbClassMapLoadContext const& loadContext);
        static BentleyStatus CreateECInstanceIdPropertyMap(ClassMap& classMap);
        static BentleyStatus CreateECClassIdPropertyMap(ClassMap& classMap);
        static BentleyStatus SetupNavigationPropertyMap(WipNavigationPropertyMap& propertyMap);
        static bool IsNavigationPropertySupportedInECSql(WipNavigationPropertyMap const& navProperty, bool logIfNotSupported) 
            {
            ECDbCR ecdb = navProperty.GetClassMap().GetDbMap().GetECDb();
            ECN::NavigationECPropertyCR navigationProperty = *navProperty.GetProperty().GetAsNavigationProperty();
            if (navigationProperty.IsMultiple())
                {
                if (logIfNotSupported)
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                  "NavigationECProperty '%s.%s' cannot be used in ECSQL because its multiplicity is %s. Only the multiplicities %s or %s are supported.",
                                                                  navigationProperty.GetClass().GetFullName(), navigationProperty.GetName().c_str(),
                                                                  ClassMapper::GetConstraint(navigationProperty, WipNavigationPropertyMap::NavigationEnd::To).GetMultiplicity().ToString().c_str(),
                                                                  ECN::RelationshipMultiplicity::ZeroOne().ToString().c_str(),
                                                                  ECN::RelationshipMultiplicity::OneOne().ToString().c_str());
                return false;
                }


            ClassMapCP relClassMap = ecdb.Schemas().GetDbMap().GetClassMap(*navigationProperty.GetRelationshipClass());
            if (relClassMap == nullptr)
                {
                if (logIfNotSupported)
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a virtual table.",
                                                                   navigationProperty.GetClass().GetFullName(), navigationProperty.GetName().c_str());

                return false;
                }

            if (relClassMap->GetType() != ClassMap::Type::RelationshipEndTable)
                {
                if (logIfNotSupported)
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a link table.",
                                                                   navigationProperty.GetClass().GetFullName(), navigationProperty.GetName().c_str());
                return false;
                }

            return true;
            }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE