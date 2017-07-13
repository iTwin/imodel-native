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
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingInfo::ClassMappingInfo(ECDb const& ecdb, ECClassCR ecClass)
    : m_ecdb(ecdb), m_ecClass(ecClass), m_classHasCurrentTimeStampProperty(nullptr), m_tphBaseClassMap(nullptr)
    {}

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

    SchemaPolicy const* noAdditionalRootEntityClassPolicy = nullptr;
    if (ctx.GetSchemaPolicies().IsOptedIn(noAdditionalRootEntityClassPolicy, SchemaPolicy::Type::NoAdditionalRootEntityClasses))
        {
        if (SUCCESS != noAdditionalRootEntityClassPolicy->GetAs<NoAdditionalRootEntityClassesPolicy>().Evaluate(m_ecdb, m_ecClass))
            return ClassMappingStatus::Error;
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
        {
        if (m_ecClass.GetClassModifier() == ECClassModifier::Abstract)
            {
            if (caCache.HasMapStrategy() && (caCache.GetStrategy() == MapStrategy::ExistingTable ||
                caCache.GetStrategy() == MapStrategy::OwnTable))
                {
                Issues().Report("Invalid MapStrategy '%s' on abstract ECClass '%s'. Only MapStrategies 'TablePerHierarchy' or 'NotMapped' are allowed on abstract classes.", MapStrategyExtendedInfo::ToString(caCache.GetStrategy()), m_ecClass.GetFullName());
                return ClassMappingStatus::Error;
                }
            }

        return AssignMapStrategy(caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;
        }

    BeAssert(baseClassMap != nullptr);
    MapStrategy baseStrategy = baseClassMap->GetMapStrategy().GetStrategy();

    switch (baseStrategy)
        {
            case MapStrategy::OwnTable:
            case MapStrategy::ExistingTable:
                //Those parent strategies are not inherited to subclasses.
                return AssignMapStrategy(caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

            case MapStrategy::NotMapped:
            {
            if (caCache.HasMapStrategy())
                {
                Issues().Report("Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy()));
                return ClassMappingStatus::Error;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return ClassMappingStatus::Success;
            }

            case MapStrategy::TablePerHierarchy:
             return EvaluateTablePerHierarchyMapStrategy(ctx, *baseClassMap, caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

            default:
                BeAssert(false && "should not be called");
                return ClassMappingStatus::Error;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateTablePerHierarchyMapStrategy(SchemaImportContext& ctx, ClassMap const& baseClassMap, ClassMappingCACache const& caCache)
    {
    MapStrategyExtendedInfo const& baseStrategy = baseClassMap.GetMapStrategy();
    if (baseStrategy.GetStrategy() != MapStrategy::TablePerHierarchy && !baseStrategy.GetTphInfo().IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    if (caCache.HasMapStrategy() && caCache.GetStrategy() == MapStrategy::NotMapped)
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return SUCCESS;
        }

    if (!ValidateTablePerHierarchyChildStrategy(baseStrategy, caCache))
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
bool ClassMappingInfo::ValidateTablePerHierarchyChildStrategy(MapStrategyExtendedInfo const& baseStrategy, ClassMappingCACache const& caCache) const
    {
    BeAssert(baseStrategy.GetStrategy() == MapStrategy::TablePerHierarchy && baseStrategy.GetTphInfo().IsValid());

    if (caCache.HasMapStrategy())
        {
        Issues().Report("Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class MapStrategy 'TablePerHierarchy'. "
                        "For subclasses of a class with MapStrategy 'TablePerHierarchy': MapStrategy must be 'NotMapped' or unset.",
                        m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy()));
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
    if (!caCache.HasMapStrategy())
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategyExtendedInfo::DEFAULT);
    else
        {
        MapStrategy strat = caCache.GetStrategy();
        if (strat != MapStrategy::TablePerHierarchy)
            m_mapStrategyExtInfo = MapStrategyExtendedInfo(strat);
        else
            {
            TablePerHierarchyInfo tphInfo;
            if (SUCCESS != tphInfo.Initialize(caCache.GetShareColumnsCA(), nullptr, nullptr, caCache.HasJoinedTablePerDirectSubclassOption(),
                                              m_ecClass, Issues()))
                return ERROR;

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(tphInfo);
            return SUCCESS;
            }
        }

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

        const MapStrategy strategy = caCache.GetStrategy();

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
//ForeignKeyMappingType
//****************************************************************************************************
std::unique_ptr<ForeignKeyMappingType> ForeignKeyMappingType::Create(ECN::ECRelationshipClassCR relClass, ECN::ECRelationshipEnd fkEnd, ForeignKeyConstraintCustomAttribute const& fkConstraintCA, IssueReporter const& issues)
    {
    if (!fkConstraintCA.IsValid())
        return std::make_unique<LogicalForeignKeyMappingType>(fkEnd);

    Nullable<Utf8String> onDeleteActionStr;
    if (SUCCESS != fkConstraintCA.TryGetOnDeleteAction(onDeleteActionStr))
        return nullptr;

    ForeignKeyDbConstraint::ActionType onDeleteAction;
    if (SUCCESS != ForeignKeyDbConstraint::TryParseActionType(onDeleteAction, onDeleteActionStr))
        {
        issues.Report("Failed to map ECRelationshipClass %s. The ForeignKeyConstraint custom attribute defines an invalid value for OnDeleteAction. See API documentation for valid values.",
                      relClass.GetFullName());
        return nullptr;
        }

    Nullable<Utf8String> onUpdateActionStr;
    if (SUCCESS != fkConstraintCA.TryGetOnUpdateAction(onUpdateActionStr))
        return nullptr;

    ForeignKeyDbConstraint::ActionType onUpdateAction;
    if (SUCCESS != ForeignKeyDbConstraint::TryParseActionType(onUpdateAction, onUpdateActionStr))
        {
        issues.Report("Failed to map ECRelationshipClass %s. The ForeignKeyConstraint custom attribute defines an invalid value for OnUpdateAction. See API documentation for valid values.",
                        relClass.GetFullName());
        return nullptr;
        }

    if (onDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade && relClass.GetStrength() != StrengthType::Embedding)
        {
        issues.Report("Failed to map ECRelationshipClass %s. The ForeignKeyConstraint custom attribute can only define 'Cascade' as OnDeleteAction if the relationship strength is 'Embedding'.",
                        relClass.GetFullName());
        return nullptr;
        }

    return std::make_unique<PhysicalForeignKeyMappingType>(fkEnd, onDeleteAction, onUpdateAction);
    }


//****************************************************************************************************
//LinkTableMappingType
//****************************************************************************************************
std::unique_ptr<LinkTableMappingType> LinkTableMappingType::Create(LinkTableRelationshipMapCustomAttribute const& linkTableRelationMapCA)
    {
    if (!linkTableRelationMapCA.IsValid())
        return std::unique_ptr<LinkTableMappingType>(new LinkTableMappingType());

    Nullable<Utf8String> sourceIdColName;
    if (SUCCESS != linkTableRelationMapCA.TryGetSourceECInstanceIdColumn(sourceIdColName))
        return nullptr;

    Nullable<Utf8String> targetIdColName;
    if (SUCCESS != linkTableRelationMapCA.TryGetTargetECInstanceIdColumn(targetIdColName))
        return nullptr;

    Nullable<bool> createForeignKeyConstraints;
    if (SUCCESS != linkTableRelationMapCA.TryGetCreateForeignKeyConstraints(createForeignKeyConstraints))
        return nullptr;

    Nullable<bool> allowDuplicateRelationships;
    if (SUCCESS != linkTableRelationMapCA.TryGetAllowDuplicateRelationships(allowDuplicateRelationships))
        return nullptr;

    return std::unique_ptr<LinkTableMappingType>(new LinkTableMappingType(sourceIdColName, targetIdColName,
                                                                          createForeignKeyConstraints.IsNull() ? true : createForeignKeyConstraints.Value(),
                                                                          allowDuplicateRelationships.IsNull() ? false : allowDuplicateRelationships.Value()));
    }


//****************************************************************************************************
//RelationshipClassMapInfo
//****************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus RelationshipMappingInfo::TryDetermineMappingType(std::unique_ptr<RelationshipMappingType>& mappingType, ECDbCR ecdb, SchemaImportContext const& ctx, ECRelationshipClassCR relClass)
    {
    LinkTableRelationshipMapCustomAttribute linkTableRelationshipMapCA;
    ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkTableRelationshipMapCA, relClass);

    ForeignKeyConstraintCustomAttribute const& foreignKeyConstraintCA = ctx.GetFkConstraintCAFromCache(relClass.GetId());

    const bool isRootClass = !relClass.HasBaseClasses();
    if (!isRootClass)
        {
        BeAssert(!foreignKeyConstraintCA.IsValid() && "Should have been caught before because a nav prop cannot point to a rel subclass");
        if (linkTableRelationshipMapCA.IsValid())
            {
            ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. It has a base class and therefore must not have the 'LinkTableRelationshipMap' custom attribute. Only the root relationship class of a hierarchy can have this custom attribute.",
                                                          relClass.GetFullName());
            return ERROR;
            }
        }

    if (linkTableRelationshipMapCA.IsValid() || (relClass.GetSource().GetMultiplicity().GetUpperLimit() > 1 && relClass.GetTarget().GetMultiplicity().GetUpperLimit() > 1) ||
        relClass.GetPropertyCount() > 0)
        {
        if (foreignKeyConstraintCA.IsValid())
            {
            ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. Its navigation property has the 'ForeignKeyConstraint' custom attribute. The relationship implies a link table mapping though (see API docs for rules when link table mapping is implied).",
                                                          relClass.GetFullName());
            return ERROR;
            }

        mappingType = LinkTableMappingType::Create(linkTableRelationshipMapCA);
        return mappingType != nullptr ? SUCCESS : ERROR;
        }

    //Now it would be a FK relationship, but this would require a nav prop. So check for nav prop now
    ECRelationshipEnd fkEnd;
    if (SUCCESS != TryDetermineFkEnd(fkEnd, relClass, ecdb.GetImpl().Issues()))
        return ERROR;

    const ECRelatedInstanceDirection navPropDir = fkEnd == ECRelationshipEnd::ECRelationshipEnd_Target ? ECRelatedInstanceDirection::Backward : ECRelatedInstanceDirection::Forward;

    //finally check whether the relationship requires a nav prop. If it does but doesn't have one, we also fall back to a link table
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT ClassId, Name FROM ec_Property WHERE NavigationRelationshipClassId=? AND NavigationDirection=? ORDER BY ClassId");
    if (stmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, relClass.GetId()) ||
        BE_SQLITE_OK != stmt->BindInt(2, Enum::ToInt(navPropDir)))
        {
        BeAssert(false);
        return ERROR;
        }

    std::vector<ECClassId> actualConstraintClassIds;
    Utf8String expectedNavPropName;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        if (!isRootClass)
            {
            ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. A navigation property is defined on its %s constraint although it has a base class. Navigation properties may only be defined for root relationship classes.",
                                                          relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
            return ERROR;
            }

        ECClassId constraintClassId = stmt->GetValueId<ECClassId>(0);
        if (!actualConstraintClassIds.empty() && actualConstraintClassIds.back() == constraintClassId)
            {
            ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. More than one navigation property for the same relationship is defined on %s constraint class %s.",
                                                            relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target",
                                                            ecdb.Schemas().GetClass(constraintClassId)->GetFullName());
            return ERROR;
            }

        actualConstraintClassIds.push_back(constraintClassId);

        Utf8CP actualNavPropName = stmt->GetValueText(1);
        if (expectedNavPropName.empty())
            expectedNavPropName.assign(actualNavPropName);
        else if (!expectedNavPropName.EqualsIAscii(actualNavPropName))
            {
            ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. The navigation properties must have the same name in all %s constraint classes. "
                                                            "Violating names: '%s' versus '%s'",
                                                            relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target",
                                                            expectedNavPropName.c_str(), actualNavPropName);
            return ERROR;
            }
        }

    if (isRootClass)
        {
        if (actualConstraintClassIds.empty())
            {
            BeAssert(!foreignKeyConstraintCA.IsValid() && "Should have been handled before");

            LOG.debugv("ECRelationshipClass '%s' is mapped to a link table because none of the constraint classes on the %s end define a navigation property for this relationship class.",
                       relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");


            mappingType = LinkTableMappingType::Create(linkTableRelationshipMapCA);
            return mappingType != nullptr ? SUCCESS : ERROR;
            }

        ECRelationshipConstraintCR fkConstraint = fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? relClass.GetSource() : relClass.GetTarget();
        std::vector<ECClassCP> expectedConstraintClasses(fkConstraint.GetConstraintClasses().begin(), fkConstraint.GetConstraintClasses().end());
        std::sort(expectedConstraintClasses.begin(), expectedConstraintClasses.end(), [] (ECClassCP lhs, ECClassCP rhs) { return lhs->GetId() < rhs->GetId(); });
        const size_t expectedConstraintClassCount = expectedConstraintClasses.size();
        if (actualConstraintClassIds.size() != expectedConstraintClassCount)
            {
            if (actualConstraintClassIds.size() < expectedConstraintClassCount)
                {
                ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. Not all %s constraint classes define a navigation property for this relationship class.",
                                                              relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
                }
            else
                {
                ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. More navigation properties found for this relationship class than expected: every %s constraint classes must define exactly one navigation property for this relationship class.",
                                                              relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
                }

            return ERROR;
            }

        for (size_t i = 0; i < expectedConstraintClassCount; i++)
            {
            ECClassCP expectedConstraintClass = expectedConstraintClasses[i];
            if (expectedConstraintClass->GetId() != actualConstraintClassIds[i])
                {
                if (expectedConstraintClass->GetId() > actualConstraintClassIds[i])
                    {
                    ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. No navigation property found for %s constraint class %s.",
                                                                  relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target",
                                                                  expectedConstraintClass->GetFullName());
                    }
                else
                    {
                    ecdb.GetImpl().Issues().Report("Failed to map ECRelationshipClass %s. Every %s constraint class must define exactly one navigation property for the relationship class.",
                                                                  relClass.GetFullName(), fkEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? "source" : "target");
                    }

                return ERROR;
                }
            }
        }

    mappingType = ForeignKeyMappingType::Create(relClass, fkEnd, foreignKeyConstraintCA, ecdb.GetImpl().Issues());
    return mappingType != nullptr ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus RelationshipMappingInfo::TryDetermineFkEnd(ECN::ECRelationshipEnd& fkEnd, ECN::ECRelationshipClassCR relClass, IssueReporter const& issues)
    {
    const StrengthType strength = relClass.GetStrength();
    const ECRelatedInstanceDirection strengthDirection = relClass.GetStrengthDirection();

    const bool sourceIsM = relClass.GetSource().GetMultiplicity().GetUpperLimit() > 1;
    const bool targetIsM = relClass.GetTarget().GetMultiplicity().GetUpperLimit() > 1;
    if (sourceIsM && targetIsM)
        {
        BeAssert(false && "Must not be called for M:N cardinalities");
        return ERROR;
        }

    if (!sourceIsM && targetIsM)
        {
        if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Backward)
            {
            issues.Report("Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Forward'.",
                          relClass.GetFullName(), relClass.GetSource().GetMultiplicity().ToString().c_str(), relClass.GetTarget().GetMultiplicity().ToString().c_str());
            return ERROR;
            }

        fkEnd = ECRelationshipEnd_Target;
        return SUCCESS;
        }

    if (sourceIsM && !targetIsM)
        {
        if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Forward)
            {
            issues.Report("Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Backward'.",
                            relClass.GetFullName(), relClass.GetSource().GetMultiplicity().ToString().c_str(), relClass.GetTarget().GetMultiplicity().ToString().c_str());
            return ERROR;
            }

        fkEnd = ECRelationshipEnd_Source;
        return SUCCESS;
        }

    BeAssert(!sourceIsM && !targetIsM);
    if (strengthDirection == ECRelatedInstanceDirection::Forward)
        fkEnd = ECRelationshipEnd_Target;
    else
        fkEnd = ECRelationshipEnd_Source;

    return SUCCESS;
    }


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
    return TryDetermineMappingType(m_mappingType, m_ecdb, ctx, *m_ecClass.GetRelationshipClassCP());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipMappingInfo::_EvaluateMapStrategy(SchemaImportContext& ctx)
    {
    ClassMappingCACache const* caCache = ctx.GetClassMappingCACache(m_ecClass);
    if (caCache == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    const bool hasBaseClasses = m_ecClass.HasBaseClasses();
    ClassMap const* firstBaseClassMap = nullptr;
    if (hasBaseClasses)
        {
        BeAssert(m_ecClass.GetBaseClasses().size() == 1 && "Only 1 base class allowed for rels. This should have been caught before");

        for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* baseClassMap = GetDbMap().GetClassMap(*baseClass);
            if (baseClassMap == nullptr)
                return ClassMappingStatus::BaseClassesNotMapped;

            if (firstBaseClassMap == nullptr)
                firstBaseClassMap = baseClassMap;
            }

        const MapStrategy baseStrategy = firstBaseClassMap->GetMapStrategy().GetStrategy();
        if (baseStrategy == MapStrategy::NotMapped)
            {
            if (caCache->HasMapStrategy())
                {
                Issues().Report("Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache->GetStrategy()));
                return ClassMappingStatus::Error;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return ClassMappingStatus::Success;
            }

        if (baseStrategy == MapStrategy::OwnTable || baseStrategy == MapStrategy::ExistingTable)
            {
            Issues().Report("Failed to map ECRelationshipClass %s. Its base class %s has the MapStrategy 'OwnTable' or 'ExistingTable' which is not supported in an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), firstBaseClassMap->GetClass().GetFullName());
            return ClassMappingStatus::Error;
            }

        if (caCache->HasMapStrategy() && caCache->GetStrategy() == MapStrategy::NotMapped)
            {
            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return ClassMappingStatus::Success;
            }

        if (firstBaseClassMap->GetType() == ClassMap::Type::RelationshipEndTable)
            {
            if (m_mappingType->GetType() == RelationshipMappingType::Type::LinkTable)
                {
                Issues().Report("Failed to map ECRelationshipClass %s. It would be mapped as link table, but its base class %s is mapped as foreign key relationship. The mapping type must not change within an ECRelationshipClass hierarchy.",
                                m_ecClass.GetFullName(), firstBaseClassMap->GetClass().GetFullName());

                return ClassMappingStatus::Error;
                }

            if (SUCCESS != EvaluateForeignKeyStrategy(ctx, *caCache))
                return ClassMappingStatus::Error;
            }
        else
            {
            BeAssert(firstBaseClassMap->GetType() == ClassMap::Type::RelationshipLinkTable);
            if (SUCCESS != EvaluateLinkTableStrategy(ctx, *caCache, firstBaseClassMap))
                return ClassMappingStatus::Error;
            }

        if (baseStrategy != m_mapStrategyExtInfo.GetStrategy())
            {
            Issues().Report("Failed to map ECRelationshipClass %s. Its mapping type (%s) differs from the mapping type of its base relationship class %s (%s). The mapping type must not change within an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_mapStrategyExtInfo.GetStrategy()),
                            firstBaseClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(baseStrategy));
            return ClassMappingStatus::Error;
            }

        return ClassMappingStatus::Success;
        }

    //no base class
    BeAssert(m_mappingType != nullptr);

    if (m_mappingType->IsLinkTable())
        {
        SchemaPolicy const* noAdditionalLinkTablesPolicy = nullptr;
        if (ctx.GetSchemaPolicies().IsOptedIn(noAdditionalLinkTablesPolicy, SchemaPolicy::Type::NoAdditionalLinkTables))
            {
            if (SUCCESS != noAdditionalLinkTablesPolicy->GetAs<NoAdditionalLinkTablesPolicy>().Evaluate(m_ecdb, *m_ecClass.GetRelationshipClassCP()))
                return ClassMappingStatus::Error;
            }
        }

    if (caCache->HasMapStrategy() && caCache->GetStrategy() == MapStrategy::NotMapped)
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    if (m_mappingType->IsLinkTable())
        {
        if (SUCCESS != FailIfConstraintClassIsNotMapped())
            return ClassMappingStatus::Error;

        return EvaluateLinkTableStrategy(ctx, *caCache, firstBaseClassMap) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

        }

    return EvaluateForeignKeyStrategy(ctx, *caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateLinkTableStrategy(SchemaImportContext& ctx, ClassMappingCACache const& caCache, ClassMap const* baseClassMap)
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();

    if (baseClassMap != nullptr)
        {
        BeAssert(baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
        return EvaluateTablePerHierarchyMapStrategy(ctx, *baseClassMap, caCache);
        }

    BeAssert(m_mappingType != nullptr);

    //*** root rel class
    //Table retrieval is only needed for the root rel class. Subclasses will use the tables of its base class
    //TODO: How should we handle this properly?
    m_sourceTables = GetTablesFromRelationshipEnd(GetDbMap(), ctx, relClass->GetSource(), true);
    m_targetTables = GetTablesFromRelationshipEnd(GetDbMap(), ctx, relClass->GetTarget(), true);

    if (m_sourceTables.empty() || m_targetTables.empty())
        {
        Issues().Report("Failed to map ECRelationshipClass '%s'. Source or target constraint classes are abstract without subclasses. Consider applying the MapStrategy 'TablePerHierarchy' to the abstract constraint class.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    const size_t sourceTableCount = m_sourceTables.size();
    const size_t targetTableCount = m_targetTables.size();
    if (sourceTableCount > 1 || targetTableCount > 1)
        {
        Utf8CP constraintStr = nullptr;
        if (sourceTableCount > 1 && targetTableCount > 1)
            constraintStr = "source and target constraints are";
        else if (sourceTableCount > 1)
            constraintStr = "source constraint is";
        else
            constraintStr = "target constraint is";

        Issues().Report("Failed to map ECRelationshipClass '%s'. It is mapped to a link table, but the %s mapped to more than one table, which is not supported for link tables.",
                        m_ecClass.GetFullName(), constraintStr);

        return ERROR;
        }


    if (caCache.GetClassMap().IsValid())
        return AssignMapStrategy(caCache);

    //sealed rel classes without base class get own table
    const MapStrategy strat = m_ecClass.GetClassModifier() != ECClassModifier::Sealed ? MapStrategy::TablePerHierarchy : MapStrategy::OwnTable;
    m_mapStrategyExtInfo = MapStrategyExtendedInfo(strat);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateForeignKeyStrategy(SchemaImportContext& ctx, ClassMappingCACache const& caCache)
    {
    if (caCache.GetClassMap().IsValid())
        {
        Issues().Report("Failed to map ECRelationshipClass %s. It implies the ForeignKey type mapping, but also has the ClassMap custom attribute. ForeignKey type mappings can only have the ClassMap custom attribute when the MapStrategy is set to 'NotMapped'.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    const MapStrategy strategy = m_mappingType->GetAs<ForeignKeyMappingType>().GetFkEnd() == ECRelationshipEnd_Source ? MapStrategy::ForeignKeyRelationshipInSourceTable : MapStrategy::ForeignKeyRelationshipInTargetTable;
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
        if (constraintClassMap == nullptr || constraintClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
            {
            Issues().Report("Failed to map ECRelationshipclass '%s'. The source or target constraint contains at least one ECClass which is not mapped. Mark the ECRelationshipClass with the 'NotMapped' strategy as well.",
                            m_ecClass.GetFullName());
            return ERROR;
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<DbTable const*> RelationshipMappingInfo::GetTablesFromRelationshipEnd(DbMap const&  dbMap,  SchemaImportContext& ctx, ECRelationshipConstraintCR relationshipEnd, bool ignoreJoinedTables) 
    {
    std::set<ClassMap const*> classMaps = dbMap.GetClassMapsFromRelationshipEnd(ctx, relationshipEnd);
    std::map<DbTable const*, std::set<DbTable const*>> joinedTables;
    std::set<DbTable const*> tables;
    for (ClassMap const* classMap : classMaps)
        {
        std::vector<DbTable const*> classPersistInTables;
        for (DbTable const* table : classMap->GetTables())
            if (table->GetType() != DbTable::Type::Overflow)
                classPersistInTables.push_back(table);

        if (classPersistInTables.size() == 1)
            {
            tables.insert(classPersistInTables.front());
            continue;
            }

        for (DbTable const* table : classPersistInTables)
            {
            if (DbTable::LinkNode const* previousTableNode = table->GetLinkNode().GetParent())
                {
                joinedTables[&previousTableNode->GetTable()].insert(table);
                tables.insert(table);
                }
            }
        }

    for (auto const& pair : joinedTables)
        {
        DbTable const* primaryTable = pair.first;
        std::set<DbTable const*> const& joinedTables = pair.second;

        bool isPrimaryTableSelected = tables.find(primaryTable) != tables.end();
        if (ignoreJoinedTables)
            {
            for (DbTable::LinkNode const* nextTableNode : primaryTable->GetLinkNode().GetChildren())
                tables.erase(&nextTableNode->GetTable());

            tables.insert(primaryTable);
            continue;
            }

        if (isPrimaryTableSelected)
            {
            for (DbTable const* joinedTable : joinedTables)
                tables.erase(joinedTable);
            }
        }

    if (!ignoreJoinedTables)
        return tables;

    std::set<DbTable const*> finalSetOfTables;
    for (DbTable const* table : tables)
        {
        if (table->GetType() != DbTable::Type::Virtual)
            finalSetOfTables.insert(table);
        }

    return finalSetOfTables;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
