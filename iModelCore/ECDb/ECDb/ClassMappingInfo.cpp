/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <stack>
#include "SqlNames.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**********************************************************************************************
// ClassMappingInfo
//**********************************************************************************************

//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::Initialize()
    {
    if (SUCCESS != InitializeFromSchema())
        return ClassMappingStatus::Error;

    return EvaluateMapStrategy();
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::InitializeFromSchema()
    {
    ECEntityClassCP entityClass = m_ecClass.GetEntityClassCP();
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    if (relClass != nullptr)
        {
        BeAssert(relClass->GetBaseClasses().size() <= 1 && "Should actually have been enforced by ECSchemaValidator");

        //determine whether a link table is required or not
        RelationshipMappingType mappingType;
        if (SUCCESS != DbMappingManager::Classes::TryDetermineRelationshipMappingType(mappingType, m_ctx, *m_ecClass.GetRelationshipClassCP()))
            return ERROR;

        m_relMappingType = mappingType;
        }

    ClassMapCustomAttribute classMapCA;
    ECDbMapCustomAttributeHelper::TryGetClassMap(classMapCA, m_ecClass);
    ECDbMapCustomAttributeHelper::TryGetShareColumns(m_shareColumnsCA, m_ecClass);
    m_hasJoinedTablePerDirectSubclassCA = entityClass == nullptr ? false : ECDbMapCustomAttributeHelper::HasJoinedTablePerDirectSubclass(*entityClass);

    if (classMapCA.IsValid())
        {
        if (entityClass != nullptr && entityClass->IsMixin())
            {
            m_ctx.Issues().ReportV("Failed to map Mixin ECClass %s. Mixins may not have the ClassMap custom attribute.",
                            m_ecClass.GetFullName());
            return ERROR;
            }

        Nullable<Utf8String> mapStrategyStr;
        if (SUCCESS != classMapCA.TryGetMapStrategy(mapStrategyStr))
            return ERROR;

        if (!mapStrategyStr.IsNull())
            {
            MapStrategy mapStrat;
            if (SUCCESS != MapStrategyExtendedInfo::ParseMapStrategy(mapStrat, mapStrategyStr.Value()))
                {
                m_ctx.Issues().ReportV("ECClass '%s' has a ClassMap custom attribute with an invalid value for MapStrategy: %s.", m_ecClass.GetFullName(), mapStrategyStr.Value().c_str());
                return ERROR;
                }

            m_userDefinedStrategy = mapStrat;
            }

        Nullable<Utf8String> tableName;
        if (SUCCESS != classMapCA.TryGetTableName(tableName))
            return ERROR;

        if (!tableName.IsNull())
            m_tableName.assign(tableName.Value());

        Nullable<Utf8String> idColName;
        if (SUCCESS != classMapCA.TryGetECInstanceIdColumn(idColName))
            return ERROR;

        if (!idColName.IsNull())
            m_ecInstanceIdColumnName.assign(idColName.Value());
        }

    //do some validation on the input from the schema
    if (m_relMappingType == RelationshipMappingType::ForeignKeyOnSource || m_relMappingType == RelationshipMappingType::ForeignKeyOnTarget)
        {
        if (!m_userDefinedStrategy.IsNull() && m_userDefinedStrategy.Value() != MapStrategy::NotMapped)
            {
            m_ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. ECRelationshipClasses mapped as foreign key cannot be assigned a MapStrategy other than '%s'.",
                            m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(MapStrategy::NotMapped));
            return ERROR;
            }
        }

    if (m_userDefinedStrategy.IsNull())
        {
        if (!m_tableName.empty())
            {
            m_ctx.Issues().ReportV("Failed to map ECClass %s. TableName must only be set in ClassMap custom attribute if MapStrategy is '%s'.",
                            m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(MapStrategy::ExistingTable));
            return ERROR;
            }
        }
    else
        {
        switch (m_userDefinedStrategy.Value())
            {
                case MapStrategy::ExistingTable:
                {
                if (m_ecClass.GetClassModifier() != ECClassModifier::Sealed)
                    {
                    m_ctx.Issues().ReportV("Invalid MapStrategy '%s' on ECClass '%s'. Only sealed classes can be assigned that map strategy.", MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()), m_ecClass.GetFullName());
                    return ERROR;
                    }

                if (m_tableName.empty())
                    {
                    m_ctx.Issues().ReportV("Failed to map ECClass %s. TableName must not be empty in ClassMap custom attribute if MapStrategy is '%s'.",
                                    m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
                    return ERROR;
                    }

                break;
                }

                case MapStrategy::OwnTable:
                {
                if (m_ecClass.IsRelationshipClass())
                    {
                    m_ctx.Issues().ReportV("Invalid MapStrategy '%s' on ECRelationshipClass '%s'. Relationship classes cannot be assigned that map strategy.", MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()), m_ecClass.GetFullName());
                    return ERROR;
                    }

                if (!m_tableName.empty())
                    {
                    m_ctx.Issues().ReportV("Failed to map ECClass %s. TableName must not be set in ClassMap custom attribute if MapStrategy is '%s'.",
                                    m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
                    return ERROR;
                    }

                break;
                }

                case MapStrategy::TablePerHierarchy:
                {
                if (m_ecClass.GetClassModifier() == ECClassModifier::Sealed)
                    {
                    m_ctx.Issues().ReportV("Failed to map ECClass %s. MapStrategy '%s' cannot be applied to ECClasses with modifier 'Sealed'.",
                                    m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
                    return ERROR;
                    }

                if (!m_tableName.empty())
                    {
                    m_ctx.Issues().ReportV("Failed to map ECClass %s. TableName must not be set in ClassMap custom attribute if MapStrategy is '%s'.",
                                    m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
                    return ERROR;
                    }

                break;
                }

                default:
                {
                if (!m_tableName.empty())
                    {
                    m_ctx.Issues().ReportV("Failed to map ECClass %s. TableName must not be set in ClassMap custom attribute if MapStrategy is '%s'.",
                                    m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
                    return ERROR;
                    }

                break;
                }
            }
        }

    if (m_tableName.empty())
        {
        // if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        if (SUCCESS != DbMappingManager::Tables::DetermineTableName(m_tableName, m_ecClass))
            return ERROR;
        }

    return InitializeClassHasCurrentTimeStampProperty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 muhammad.zaighum                01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::InitializeClassHasCurrentTimeStampProperty()
    {
    PrimitiveECPropertyCP currentTimeStampProp = nullptr;
    if (SUCCESS != CoreCustomAttributeHelper::GetCurrentTimeStampProperty(currentTimeStampProp, m_ecClass))
        return ERROR;

    if (currentTimeStampProp == nullptr)
        return SUCCESS;

    if (!currentTimeStampProp->GetIsReadOnly())
        {
        m_ctx.Issues().ReportV("Failed to map ECClass %s. The property '%s' specified in the ClassHasCurrentTimeStampProperty custom attribute must be set to read-only.",
                        m_ecClass.GetFullName(), currentTimeStampProp->GetName().c_str());
        return ERROR;
        }

    if (m_ecClass.IsEntityClass() && m_ecClass.GetEntityClassCP()->IsMixin())
        {
        m_ctx.Issues().ReportV("Failed to map Mixin ECClass %s. Mixins may not have the ClassHasCurrentTimeStampProperty custom attribute.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    m_classHasCurrentTimeStampProperty = currentTimeStampProp;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::EvaluateMapStrategy()
    {
    if (m_ecClass.IsCustomAttributeClass() || m_ecClass.IsStructClass())
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    ClassMap const* baseClassMap = nullptr;
    ClassMappingStatus stat = TryGetBaseClassMap(baseClassMap, m_ctx.GetECDb(), m_ecClass);
    if (stat != ClassMappingStatus::Success)
        return stat;

    MapStrategy strategy = MapStrategy::OwnTable;
    if (!m_userDefinedStrategy.IsNull())
        strategy = m_userDefinedStrategy.Value();
    else
        {
        if (m_relMappingType == RelationshipMappingType::LinkTable)
            {
            if (m_ecClass.GetClassModifier() != ECClassModifier::Sealed)
                strategy = MapStrategy::TablePerHierarchy;
            }
        else if (m_relMappingType == RelationshipMappingType::ForeignKeyOnSource)
            strategy = MapStrategy::ForeignKeyRelationshipInSourceTable;
        else if (m_relMappingType == RelationshipMappingType::ForeignKeyOnTarget)
            strategy = MapStrategy::ForeignKeyRelationshipInTargetTable;
        }

    if (strategy == MapStrategy::TablePerHierarchy)
        {
        TablePerHierarchyInfo tphInfo;
        if (SUCCESS != tphInfo.Initialize(m_shareColumnsCA, nullptr, m_hasJoinedTablePerDirectSubclassCA, m_ecClass, m_ctx.Issues()))
            return ClassMappingStatus::Error;

        m_mapStrategyExtInfo = MapStrategyExtendedInfo(strategy, tphInfo);
        }
    else
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(strategy);


    if (baseClassMap == nullptr)
        {
        if (SUCCESS != EvaluateRootClassMapStrategy())
            return ClassMappingStatus::Error;
        }
    else
        {
        if (SUCCESS != EvaluateNonRootClassMapStrategy(*baseClassMap))
            return ClassMappingStatus::Error;
        }

    if (!m_mapStrategyExtInfo.IsTablePerHierarchy())
        {
        if (m_hasJoinedTablePerDirectSubclassCA)
            {
            m_ctx.Issues().ReportV("ECClass '%s' has the 'JoinedTablePerDirectSubclass' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                       "The 'JoinedTablePerDirectSubclass' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
            return ClassMappingStatus::Error;
            }

        if (m_shareColumnsCA.IsValid())
            {
            m_ctx.Issues().ReportV("ECClass '%s' has the 'ShareColumns' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                       "The 'ShareColumns' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
            return ClassMappingStatus::Error;
            }
        }
    if (m_ecInstanceIdColumnName.empty())
        m_ecInstanceIdColumnName.assign(COL_DEFAULTNAME_Id);

    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateRootClassMapStrategy()
    {
    ECEntityClassCP entityClass = m_ecClass.GetEntityClassCP();
    if (entityClass != nullptr)
        {
        BeAssert(m_relMappingType.IsNull());
        SchemaPolicy const* noAdditionalRootEntityClassPolicy = nullptr;
        if (m_ctx.GetSchemaPolicies().IsOptedIn(noAdditionalRootEntityClassPolicy, SchemaPolicy::Type::NoAdditionalRootEntityClasses))
            {
            if (m_ctx.GetBuiltinSchemaNames().find(entityClass->GetSchema().GetName().c_str()) == m_ctx.GetBuiltinSchemaNames().end() &&
                SUCCESS != noAdditionalRootEntityClassPolicy->GetAs<NoAdditionalRootEntityClassesPolicy>().Evaluate(m_ctx.GetECDb(), *entityClass))
                return ERROR;
            }

        return SUCCESS;
        }

    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    BeAssert(relClass != nullptr && "Other class types should have been caught before");
    BeAssert(!m_relMappingType.IsNull());
    if (m_relMappingType != RelationshipMappingType::LinkTable)
        return SUCCESS;

    SchemaPolicy const* noAdditionalLinkTablesPolicy = nullptr;
    if (m_ctx.GetSchemaPolicies().IsOptedIn(noAdditionalLinkTablesPolicy, SchemaPolicy::Type::NoAdditionalLinkTables))
        {
        if (SUCCESS != noAdditionalLinkTablesPolicy->GetAs<NoAdditionalLinkTablesPolicy>().Evaluate(m_ctx.GetECDb(), *relClass))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateNonRootClassMapStrategy(ClassMap const& baseClassMap)
    {
    const MapStrategy baseStrategy = baseClassMap.GetMapStrategy().GetStrategy();
    switch (baseStrategy)
        {
            case MapStrategy::OwnTable:
                //not inherited to subclasses, so no need to consider base class
                BeAssert(!baseClassMap.GetClass().IsRelationshipClass() && "OwnTable not allowed for rels");
                return SUCCESS;
            
            case MapStrategy::ExistingTable:
                BeAssert(false && "ExistingTable only allowed for sealed classes. This should have been caught before");
                return ERROR;

            case MapStrategy::NotMapped:
            {
            if (!m_userDefinedStrategy.IsNull())
                {
                m_ctx.Issues().ReportV("Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
                return ERROR;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return SUCCESS;
            }

            case MapStrategy::TablePerHierarchy:
                return EvaluateNonRootClassTablePerHierarchyMapStrategy(baseClassMap);

            case MapStrategy::ForeignKeyRelationshipInSourceTable:
            {                
            if (m_mapStrategyExtInfo.GetStrategy() != MapStrategy::ForeignKeyRelationshipInSourceTable)
                {
                m_ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. It would be mapped with strategy %s, but its base class %s is mapped as foreign key relationship. The mapping type must not change within an ECRelationshipClass hierarchy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_mapStrategyExtInfo.GetStrategy()), baseClassMap.GetClass().GetFullName());

                return ERROR;
                }

            return SUCCESS;
            }
            case MapStrategy::ForeignKeyRelationshipInTargetTable:
            {
            if (m_mapStrategyExtInfo.GetStrategy() != MapStrategy::ForeignKeyRelationshipInTargetTable)
                {
                m_ctx.Issues().ReportV("Failed to map ECRelationshipClass %s. It would be mapped with strategy %s, but its base class %s is mapped as foreign key relationship. The mapping type must not change within an ECRelationshipClass hierarchy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_mapStrategyExtInfo.GetStrategy()), baseClassMap.GetClass().GetFullName());

                return ERROR;
                }

            return SUCCESS;
            }

            default:
                BeAssert(false && "should not be called");
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateNonRootClassTablePerHierarchyMapStrategy(ClassMap const& baseClassMap)
    {
    MapStrategyExtendedInfo const& baseStrategy = baseClassMap.GetMapStrategy();
    if (baseStrategy.GetStrategy() != m_mapStrategyExtInfo.GetStrategy() && (!baseStrategy.IsTablePerHierarchy() || !baseStrategy.GetTphInfo().IsValid()))
        {
        BeAssert(false);
        return ERROR;
        }

    if (!m_userDefinedStrategy.IsNull())
        {
        if (m_userDefinedStrategy == MapStrategy::NotMapped)
            {
            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return SUCCESS;
            }

        m_ctx.Issues().ReportV("Failed to map ECClass %s. For subclasses of a class with MapStrategy 'TablePerHierarchy' MapStrategy must be 'NotMapped' or unset.",
                        m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_userDefinedStrategy.Value()));
        return ERROR;
        }

    if (!m_ecInstanceIdColumnName.empty())
        {
        m_ctx.Issues().ReportV("Failed to map ECClass %s. For subclasses of a class with MapStrategy 'TablePerHierarchy', ECInstanceIdColumn may not be defined in the ClassMap custom attribute.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    m_tphBaseClassMap = &baseClassMap; //only need to hold the base class map for TPH case

    DbTable const& baseClassJoinedTable = baseClassMap.GetJoinedOrPrimaryTable();
    m_tableName = baseClassJoinedTable.GetName();
    m_ecInstanceIdColumnName.assign(baseClassJoinedTable.FindFirst(DbColumn::Kind::ECInstanceId)->GetName());

    TablePerHierarchyInfo tphInfo;
    if (SUCCESS != tphInfo.Initialize(m_shareColumnsCA, &baseClassMap.GetMapStrategy(), m_hasJoinedTablePerDirectSubclassCA, m_ecClass, m_ctx.Issues()))
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
            ECClassCP rootClass = m_ctx.GetECDb().Schemas().GetClass(rootClassId);
            if (rootClass == nullptr)
                {
                BeAssert(false && "There should always be a root class map which defines the TablePerHierarchy strategy");
                return ERROR;
                }

            m_ecInstanceIdColumnName.Sprintf("%s%s", rootClass->GetName().c_str(), m_ecInstanceIdColumnName.c_str());
            }
        }

    m_mapStrategyExtInfo = MapStrategyExtendedInfo(baseClassMap.GetMapStrategy().GetStrategy(), tphInfo);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
ClassMappingStatus ClassMappingInfo::TryGetBaseClassMap(ClassMap const*& foundBaseClassMap, ECDbCR ecdb, ECClassCR ecClass)
    {
    if (!ecClass.HasBaseClasses())
        {
        foundBaseClassMap = nullptr;
        return ClassMappingStatus::Success;
        }

    ClassMap const* tphBaseClassMap = nullptr;
    ClassMap const* ownTableBaseClassMap = nullptr;
    ClassMap const* notMappedBaseClassMap = nullptr;

    const bool isMultiInheritance = ecClass.GetBaseClasses().size() > 1;
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        ClassMap const* baseClassMap = ecdb.Schemas().Main().GetClassMap(*baseClass);
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
                    ecdb.GetImpl().Issues().ReportV("ECClass '%s' has two base ECClasses with MapStrategy 'TablePerHierarchy' which don't map to the same tables. "
                                    "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'. "
                                    "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'.",
                                    ecClass.GetFullName(), tphBaseClassMap->GetClass().GetFullName(),
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
        ecdb.GetImpl().Issues().ReportV("ECClass '%s' has two base ECClasses with incompatible MapStrategies. "
                        "Base ECClass '%s' has MapStrategy '%s'."
                        "Base ECClass '%s' has MapStrategy '%s'.",
                        ecClass.GetFullName(),
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

END_BENTLEY_SQLITE_EC_NAMESPACE
