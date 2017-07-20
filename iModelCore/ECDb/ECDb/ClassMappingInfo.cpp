/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <stack>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**********************************************************************************************
// ClassMappingInfoFactory
//**********************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ClassMappingInfo> ClassMappingInfoFactory::Create(ClassMappingStatus& mapStatus, SchemaImportContext& ctx, ECDb const& ecdb, ECN::ECClassCR ecClass)
    {
    std::unique_ptr<ClassMappingInfo> info = nullptr;
    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    if (ecRelationshipClass != nullptr)
        info = std::unique_ptr<ClassMappingInfo>(new RelationshipMappingInfo(ecdb, *ecRelationshipClass));
    else
        info = std::unique_ptr<ClassMappingInfo>(new ClassMappingInfo(ecdb, ecClass));

    if (info == nullptr || (mapStatus = info->Initialize(ctx)) != ClassMappingStatus::Success)
        return nullptr;

    return info;
    }


//**********************************************************************************************
// ClassMappingInfo
//**********************************************************************************************
//---------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                    07/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
MapStrategy ClassMappingInfo::GetDefaultStrategy(ECN::ECClassCR ecClass)
    {
    if (ecClass.IsRelationshipClass() && ecClass.GetClassModifier() != ECClassModifier::Sealed)
        return MapStrategy::TablePerHierarchy;

    return MapStrategy::OwnTable;
    }


//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::Initialize(SchemaImportContext& ctx)
    {
    if (SUCCESS != _InitializeFromSchema(ctx))
        return ClassMappingStatus::Error;

    return EvaluateMapStrategy(ctx);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                    05/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::EvaluateMapStrategy(SchemaImportContext& ctx)
    {
    //Default values for table name and primary key column name
    if (m_tableName.empty())
        {
        // if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        if (SUCCESS != DbMappingManager::Tables::DetermineTableName(m_tableName, m_ecClass))
            return ClassMappingStatus::Error;
        }

    ClassMappingStatus stat = _EvaluateMapStrategy(ctx);
    if (stat != ClassMappingStatus::Success)
        return stat;

    if (m_ecInstanceIdColumnName.empty())
        m_ecInstanceIdColumnName.assign(COL_DEFAULTNAME_Id);

    return ClassMappingStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::_EvaluateMapStrategy(SchemaImportContext& ctx)
    {
    if (m_ecClass.IsCustomAttributeClass() || m_ecClass.IsStructClass())
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    ClassMap const* baseClassMap = nullptr;
    ClassMappingStatus stat = TryGetBaseClassMap(baseClassMap);
    if (stat != ClassMappingStatus::Success)
        return stat;

    ClassMappingCACache const* caCacheCP = ctx.GetClassMappingCACache(m_ecClass);
    if (caCacheCP == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    ClassMappingCACache const& caCache = *caCacheCP;

    if (baseClassMap == nullptr)
        return EvaluateRootClassMapStrategy(ctx, caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

    return EvaluateNonRootClassMapStrategy(ctx, *baseClassMap, caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateRootClassMapStrategy(SchemaImportContext& ctx, ClassMappingCACache const& caCache)
    {
    if (!caCache.GetStrategy().IsNull())
        {
        if (m_ecClass.GetClassModifier() != ECClassModifier::Sealed && caCache.GetStrategy() == MapStrategy::ExistingTable)
            {
            Issues().Report("Invalid MapStrategy 'ExistingTable' on ECClass '%s'. Only sealed classes can be assigned that map strategy.", m_ecClass.GetFullName());
            return ERROR;
            }

        if (m_ecClass.IsRelationshipClass() && caCache.GetStrategy() == MapStrategy::OwnTable)
            {
            Issues().Report("Invalid MapStrategy 'OwnTable' on ECRelationshipClass '%s'. Relationship classes cannot be assigned that map strategy.", m_ecClass.GetFullName());
            return ERROR;
            }
        }

    SchemaPolicy const* noAdditionalRootEntityClassPolicy = nullptr;
    if (ctx.GetSchemaPolicies().IsOptedIn(noAdditionalRootEntityClassPolicy, SchemaPolicy::Type::NoAdditionalRootEntityClasses))
        {
        if (SUCCESS != noAdditionalRootEntityClassPolicy->GetAs<NoAdditionalRootEntityClassesPolicy>().Evaluate(m_ecdb, m_ecClass))
            return ERROR;
        }

    return AssignMapStrategy(caCache);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateNonRootClassMapStrategy(SchemaImportContext& ctx, ClassMap const& baseClassMap, ClassMappingCACache const& caCache)
    {
    const MapStrategy baseStrategy = baseClassMap.GetMapStrategy().GetStrategy();

    switch (baseStrategy)
        {
            case MapStrategy::OwnTable:
                //not inherited to subclasses.
                BeAssert(!baseClassMap.GetClass().IsRelationshipClass() && "OwnTable not allowed for rels");
                return AssignMapStrategy(caCache);
            
            case MapStrategy::ExistingTable:
                BeAssert(false && "ExistingTable only allowed for sealed classes. This should have been caught before");
                return ERROR;

            case MapStrategy::NotMapped:
            {
            if (!caCache.GetStrategy().IsNull())
                {
                Issues().Report("Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy().Value()));
                return ERROR;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return SUCCESS;
            }

            case MapStrategy::TablePerHierarchy:
                return EvaluateNonRootClassTablePerHierarchyMapStrategy(ctx, baseClassMap, caCache);

            case MapStrategy::ForeignKeyRelationshipInSourceTable:
            case MapStrategy::ForeignKeyRelationshipInTargetTable:
                m_mapStrategyExtInfo = MapStrategyExtendedInfo(baseStrategy);
                return SUCCESS; //Nothing to do for fk relationship subclasses

            default:
                BeAssert(false && "should not be called");
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateNonRootClassTablePerHierarchyMapStrategy(SchemaImportContext& ctx, ClassMap const& baseClassMap, ClassMappingCACache const& caCache)
    {
    MapStrategyExtendedInfo const& baseStrategy = baseClassMap.GetMapStrategy();
    if (baseStrategy.GetStrategy() != MapStrategy::TablePerHierarchy && !baseStrategy.GetTphInfo().IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    if (!caCache.GetStrategy().IsNull() && caCache.GetStrategy() == MapStrategy::NotMapped)
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return SUCCESS;
        }

    if (!ValidateNonRootClassTablePerHierarchyStrategy(baseStrategy, caCache))
        return ERROR;

    m_tphBaseClassMap = &baseClassMap; //only need to hold the base class map for TPH case

    DbTable const& baseClassJoinedTable = baseClassMap.GetJoinedOrPrimaryTable();
    m_tableName = baseClassJoinedTable.GetName();
    m_ecInstanceIdColumnName.assign(baseClassJoinedTable.FindFirst(DbColumn::Kind::ECInstanceId)->GetName());

    ClassMappingCACache const* baseClassCACache = ctx.GetClassMappingCACache(baseClassMap.GetClass());
    if (baseClassCACache == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    TablePerHierarchyInfo tphInfo;
    if (SUCCESS != tphInfo.Initialize(caCache.GetShareColumnsCA(), &baseClassMap.GetMapStrategy(), &baseClassCACache->GetShareColumnsCA(),
                                                  caCache.HasJoinedTablePerDirectSubclassOption(), m_ecClass, Issues()))
        return ERROR;

    if (tphInfo.GetJoinedTableInfo() == JoinedTableInfo::JoinedTable)
        {
        if (baseClassMap.GetMapStrategy().GetTphInfo().GetJoinedTableInfo() == JoinedTableInfo::ParentOfJoinedTable)
            {
            //Joined tables are named after the class which becomes the root class of classes in the joined table
            if (SUCCESS != DbMappingManager::Tables::DetermineTableName(m_tableName, m_ecClass))
                return ERROR;

            //For classes in the joined table the id column name is determined like this:
            //"<Rootclass name><Rootclass ECInstanceId column name>"
            ECClassId rootClassId = baseClassMap.GetTphHelper()->DetermineTphRootClassId();
            BeAssert(rootClassId.IsValid());
            ECClassCP rootClass = m_ecdb.Schemas().GetClass(rootClassId);
            if (rootClass == nullptr)
                {
                BeAssert(false && "There should always be a root class map which defines the TablePerHierarchy strategy");
                return ERROR;
                }

            m_ecInstanceIdColumnName.Sprintf("%s%s", rootClass->GetName().c_str(), m_ecInstanceIdColumnName.c_str());
            }
        }

    m_mapStrategyExtInfo = MapStrategyExtendedInfo(tphInfo);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassMappingInfo::ValidateNonRootClassTablePerHierarchyStrategy(MapStrategyExtendedInfo const& baseStrategy, ClassMappingCACache const& caCache) const
    {
    BeAssert(baseStrategy.GetStrategy() == MapStrategy::TablePerHierarchy && baseStrategy.GetTphInfo().IsValid());

    if (!caCache.GetStrategy().IsNull())
        {
        Issues().Report("Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class MapStrategy 'TablePerHierarchy'. "
                        "For subclasses of a class with MapStrategy 'TablePerHierarchy': MapStrategy must be 'NotMapped' or unset.",
                        m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy().Value()));
        return false;
        }

    if (!m_ecInstanceIdColumnName.empty())
        {
        Issues().Report("Failed to map ECClass %s. For subclasses of an ECClass with MapStrategy TablePerHierarchy, ECInstanceIdColumn may not be defined in the ClassMap custom attribute.",
                        m_ecClass.GetFullName());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::AssignMapStrategy(ClassMappingCACache const& caCache)
    {
    const MapStrategy strat = !caCache.GetStrategy().IsNull() ? caCache.GetStrategy().Value() : GetDefaultStrategy(m_ecClass);
    if (strat == MapStrategy::TablePerHierarchy)
        {
        TablePerHierarchyInfo tphInfo;
        if (SUCCESS != tphInfo.Initialize(caCache.GetShareColumnsCA(), nullptr, nullptr, caCache.HasJoinedTablePerDirectSubclassOption(),
                                          m_ecClass, Issues()))
            return ERROR;

        m_mapStrategyExtInfo = MapStrategyExtendedInfo(tphInfo);
        return SUCCESS;
        }

    m_mapStrategyExtInfo = MapStrategyExtendedInfo(strat);
    BeAssert(m_mapStrategyExtInfo.GetStrategy() != MapStrategy::TablePerHierarchy);
    if (caCache.HasJoinedTablePerDirectSubclassOption())
        {
        GetDbMap().Issues().Report("ECClass '%s' has the 'JoinedTablePerDirectSubclass' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                     "the 'JoinedTablePerDirectSubclass' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
        return ERROR;
        }

    if (caCache.GetShareColumnsCA().IsValid())
        {
        GetDbMap().Issues().Report("ECClass '%s' has the 'ShareColumns' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                     "the 'ShareColumns' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMappingInfo::_InitializeFromSchema(SchemaImportContext& ctx)
    {
    ClassMappingCACache const* caCacheCP = ctx.GetClassMappingCACache(m_ecClass);
    if (caCacheCP == nullptr)
        return ERROR;

    ClassMappingCACache const& caCache = *caCacheCP;

    ClassMapCustomAttribute const& classMapCA = caCache.GetClassMap();

    if (classMapCA.IsValid())
        {
        if (m_ecClass.IsEntityClass() && m_ecClass.GetEntityClassCP()->IsMixin())
            {
            Issues().Report("Failed to map Mixin ECClass %s. Mixins may not have the ClassMap custom attribute.",
                            m_ecClass.GetFullName());
            return ERROR;
            }

        Nullable<MapStrategy> const& strategy = caCache.GetStrategy();

        if (strategy == MapStrategy::TablePerHierarchy && m_ecClass.GetClassModifier() == ECClassModifier::Sealed)
            {
            Issues().Report("Failed to map ECClass %s. MapStrategy 'TablePerHierarchy' cannot be applied to ECClasses with modifier 'Sealed'.",
                            m_ecClass.GetFullName());
            return ERROR;
            }

        Nullable<Utf8String> tableName;
        if (SUCCESS != classMapCA.TryGetTableName(tableName))
            return ERROR;

        if (strategy == MapStrategy::ExistingTable)
            {
            if (tableName.IsNull())
                {
                Issues().Report("Failed to map ECClass %s. TableName must not be empty in ClassMap custom attribute if MapStrategy is 'ExistingTable'.",
                                m_ecClass.GetFullName());
                return ERROR;
                }
            }
        else
            {
            if (!tableName.IsNull())
                {
                Issues().Report("Failed to map ECClass %s. TableName must only be set in ClassMap custom attribute if MapStrategy is 'ExistingTable'.",
                                m_ecClass.GetFullName());
                return ERROR;
                }
            }

        if (!tableName.IsNull())
            m_tableName.assign(tableName.Value());

        Nullable<Utf8String> idColName;
        if (SUCCESS != classMapCA.TryGetECInstanceIdColumn(idColName))
            return ERROR;

        if (!idColName.IsNull())
            m_ecInstanceIdColumnName.assign(idColName.Value());
        }

    return InitializeClassHasCurrentTimeStampProperty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 muhammad.zaighum                01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::InitializeClassHasCurrentTimeStampProperty()
    {
    IECInstancePtr ca = m_ecClass.GetCustomAttributeLocal("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
    if (ca == nullptr)
        return SUCCESS;

    if (m_ecClass.IsEntityClass() && m_ecClass.GetEntityClassCP()->IsMixin())
        {
        Issues().Report("Failed to map Mixin ECClass %s. Mixins may not have the ClassHasCurrentTimeStampProperty custom attribute.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    ECValue v;
    if (ca->GetValue(v, "PropertyName") == ECObjectsStatus::Success && !v.IsNull())
        {
        ECPropertyCP prop = m_ecClass.GetPropertyP(v.GetUtf8CP());
        if (nullptr == prop)
            {
            Issues().Report("Failed to map ECClass %s. The property '%s' specified in the 'ClassHasCurrentTimeStampProperty' custom attribute "
                            "does not exist in the ECClass.", m_ecClass.GetFullName(), v.GetUtf8CP());
            return ERROR;
            }
        PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
        if (primProp == nullptr || primProp->GetType() != PrimitiveType::PRIMITIVETYPE_DateTime)
            {
            Issues().Report("Failed to map ECClass %s. The property '%s' specified in the 'ClassHasCurrentTimeStampProperty' custom attribute "
                            "is not a primitive property of type 'DateTime'.", m_ecClass.GetFullName(), prop->GetName().c_str());
            return ERROR;

            }

        m_classHasCurrentTimeStampProperty = primProp;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::TryGetBaseClassMap(ClassMap const*& foundBaseClassMap) const
    {
    if (!m_ecClass.HasBaseClasses())
        {
        foundBaseClassMap = nullptr;
        return ClassMappingStatus::Success;
        }

    ClassMap const* tphBaseClassMap = nullptr;
    ClassMap const* ownTableBaseClassMap = nullptr;
    ClassMap const* notMappedBaseClassMap = nullptr;

    const bool isMultiInheritance = m_ecClass.GetBaseClasses().size() > 1;
    for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
        {
        ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            return ClassMappingStatus::BaseClassesNotMapped;

        if (!isMultiInheritance)
            {
            foundBaseClassMap = baseClassMap;
            return ClassMappingStatus::Success;
            }

        MapStrategy baseMapStrategy = baseClassMap->GetMapStrategy().GetStrategy();
        switch (baseMapStrategy)
            {
                case MapStrategy::TablePerHierarchy:
                {
                if (tphBaseClassMap == nullptr)
                    {
                    tphBaseClassMap = baseClassMap;
                    break;
                    }

                if (&baseClassMap->GetPrimaryTable() != &tphBaseClassMap->GetPrimaryTable() ||
                    &baseClassMap->GetJoinedOrPrimaryTable() != &tphBaseClassMap->GetJoinedOrPrimaryTable())
                    {
                    Issues().Report("ECClass '%s' has two base ECClasses with MapStrategy 'TablePerHierarchy' which don't map to the same tables. "
                                    "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'. "
                                    "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'.",
                                    m_ecClass.GetFullName(), tphBaseClassMap->GetClass().GetFullName(),
                                    tphBaseClassMap->GetPrimaryTable().GetName().c_str(), tphBaseClassMap->GetJoinedOrPrimaryTable().GetName().c_str(),
                                    baseClassMap->GetClass().GetFullName(), baseClassMap->GetPrimaryTable().GetName().c_str(), baseClassMap->GetJoinedOrPrimaryTable().GetName().c_str());
                    return ClassMappingStatus::Error;
                    }

                break;
                }

                case MapStrategy::OwnTable:
                    //we ignore abstract classes with own table as they match with the other strategies
                    if (baseClassMap->GetClass().GetClassModifier() != ECClassModifier::Abstract && ownTableBaseClassMap == nullptr)
                        ownTableBaseClassMap = baseClassMap;

                    break;

                case MapStrategy::NotMapped:
                    if (notMappedBaseClassMap == nullptr)
                        notMappedBaseClassMap = baseClassMap;

                    break;

                default:
                    BeAssert(false && "Unhandled MapStrategy for regular ECClass");
                    break;
            }
        }

    BeAssert(isMultiInheritance);
    if (tphBaseClassMap != nullptr)
        {
        //TPH must not be combined with other strategies
        if (notMappedBaseClassMap == nullptr && ownTableBaseClassMap == nullptr)
            {
            foundBaseClassMap = tphBaseClassMap;
            return ClassMappingStatus::Success;
            }

        ClassMap const* violatingClassMap = notMappedBaseClassMap != nullptr ? notMappedBaseClassMap : ownTableBaseClassMap;
        BeAssert(violatingClassMap != nullptr);
        Issues().Report("ECClass '%s' has two base ECClasses with incompatible MapStrategies. "
                        "Base ECClass '%s' has MapStrategy '%s'."
                        "Base ECClass '%s' has MapStrategy '%s'.",
                        m_ecClass.GetFullName(),
                        tphBaseClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(tphBaseClassMap->GetMapStrategy().GetStrategy()),
                        violatingClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(violatingClassMap->GetMapStrategy().GetStrategy()));

        return ClassMappingStatus::Error;
        }

    //As NotMapped applies to subclasses, it always overrides OwnTable.
    if (notMappedBaseClassMap != nullptr)
        {
        foundBaseClassMap = notMappedBaseClassMap;
        return ClassMappingStatus::Success;
        }

    foundBaseClassMap = ownTableBaseClassMap;
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2016
//+---------------+---------------+---------------+---------------+---------------+------
IssueReporter const& ClassMappingInfo::Issues() const { return m_ecdb.GetImpl().Issues(); }


//****************************************************************************************************
//RelationshipClassMapInfo
//****************************************************************************************************

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipMappingInfo::_InitializeFromSchema(SchemaImportContext& ctx)
    {
    if (SUCCESS != ClassMappingInfo::_InitializeFromSchema(ctx))
        return ERROR;

    BeAssert(m_ecClass.GetRelationshipClassCP() != nullptr);
    BeAssert(m_ecClass.GetRelationshipClassCP()->GetBaseClasses().size() <= 1 && "Should actually have been enforced by ECSchemaValidator");

    //determine whether a link table is required or not
    return DbMappingManager::Classes::TryDetermineRelationshipMappingType(m_mappingType, ctx, *m_ecClass.GetRelationshipClassCP());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipMappingInfo::_EvaluateMapStrategy(SchemaImportContext& ctx)
    {
    ClassMappingStatus stat = ClassMappingInfo::_EvaluateMapStrategy(ctx);
    if (ClassMappingStatus::Success != stat)
        return stat;

    ClassMappingCACache const* caCache = ctx.GetClassMappingCACache(m_ecClass);
    if (caCache == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    if (m_ecClass.HasBaseClasses())
        {
        BeAssert(m_ecClass.GetBaseClasses().size() == 1 && "Only 1 base class allowed for rels. This should have been caught before");

        ClassMap const* baseClassMap = GetDbMap().GetClassMap(*m_ecClass.GetBaseClasses()[0]);
        if (baseClassMap == nullptr)
            return ClassMappingStatus::BaseClassesNotMapped;

        const MapStrategy baseStrategy = baseClassMap->GetMapStrategy().GetStrategy();

        if (baseClassMap->GetType() == ClassMap::Type::RelationshipEndTable)
            {
            if (m_mappingType == RelationshipMappingType::LinkTable)
                {
                Issues().Report("Failed to map ECRelationshipClass %s. It would be mapped as link table, but its base class %s is mapped as foreign key relationship. The mapping type must not change within an ECRelationshipClass hierarchy.",
                                m_ecClass.GetFullName(), baseClassMap->GetClass().GetFullName());

                return ClassMappingStatus::Error;
                }

            if (SUCCESS != EvaluateForeignKeyStrategy(ctx, *caCache))
                return ClassMappingStatus::Error;
            }
        else
            {
            BeAssert(baseClassMap->GetType() == ClassMap::Type::RelationshipLinkTable);
            BeAssert(baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
            }

        if (baseStrategy != m_mapStrategyExtInfo.GetStrategy())
            {
            Issues().Report("Failed to map ECRelationshipClass %s. Its mapping type (%s) differs from the mapping type of its base relationship class %s (%s). The mapping type must not change within an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_mapStrategyExtInfo.GetStrategy()),
                            baseClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(baseStrategy));
            return ClassMappingStatus::Error;
            }

        return ClassMappingStatus::Success;
        }

    //no base class
    if (m_mappingType != RelationshipMappingType::LinkTable)
        return EvaluateForeignKeyStrategy(ctx, *caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;


    SchemaPolicy const* noAdditionalLinkTablesPolicy = nullptr;
    if (ctx.GetSchemaPolicies().IsOptedIn(noAdditionalLinkTablesPolicy, SchemaPolicy::Type::NoAdditionalLinkTables))
        {
        if (SUCCESS != noAdditionalLinkTablesPolicy->GetAs<NoAdditionalLinkTablesPolicy>().Evaluate(m_ecdb, *m_ecClass.GetRelationshipClassCP()))
            return ClassMappingStatus::Error;
        }

    //WIP_CLEANUP this should be checked for any kind of relationship, not just root link tables.
    //However, there is an issue with this call as it expects that the constraints class have been loaded already
    //which is not the case for end table rels
    if (SUCCESS != FailIfConstraintClassIsNotMapped())
        return ClassMappingStatus::Error;

    return EvaluateRootClassLinkTableStrategy(ctx, *caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateRootClassLinkTableStrategy(SchemaImportContext& ctx, ClassMappingCACache const& caCache)
    {
    BeAssert(m_mappingType == RelationshipMappingType::LinkTable);
    return AssignMapStrategy(caCache);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateForeignKeyStrategy(SchemaImportContext& ctx, ClassMappingCACache const& caCache)
    {
    BeAssert(m_mappingType != RelationshipMappingType::LinkTable);
    if (caCache.GetClassMap().IsValid())
        {
        Issues().Report("Failed to map ECRelationshipClass %s. It implies the ForeignKey type mapping, but also has the ClassMap custom attribute. ForeignKey type mappings cannot have the ClassMap custom attribute.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    const MapStrategy strategy = m_mappingType == RelationshipMappingType::ForeignKeyOnSource ? MapStrategy::ForeignKeyRelationshipInSourceTable : MapStrategy::ForeignKeyRelationshipInTargetTable;
    m_mapStrategyExtInfo = MapStrategyExtendedInfo(strategy);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                    05/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::FailIfConstraintClassIsNotMapped() const
    {
    for (bpair<ECClassId, LightweightCache::RelationshipEnd> const& kvPair : GetDbMap().GetLightweightCache().GetConstraintClassesForRelationshipClass(m_ecClass.GetId()))
        {
        ECClassCP constraintClass = m_ecdb.Schemas().GetClass(kvPair.first);
        if (constraintClass == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        ClassMap const* constraintClassMap = GetDbMap().GetClassMap(*constraintClass);
        //WIP_CLEANUP constraintClassMap can be null if it wasn't mapped yet. This check has to go elsewhere
        if (constraintClassMap == nullptr || constraintClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
            {
            Issues().Report("Failed to map ECRelationshipclass '%s'. The source or target constraint contains at least one ECClass which is not mapped. Mark the ECRelationshipClass with the 'NotMapped' strategy as well.",
                            m_ecClass.GetFullName());
            return ERROR;
            }
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
