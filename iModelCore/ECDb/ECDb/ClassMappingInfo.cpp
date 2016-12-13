/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
std::unique_ptr<ClassMappingInfo> ClassMappingInfoFactory::Create(ClassMappingStatus& mapStatus, ECDb const& ecdb, ECN::ECClassCR ecClass)
    {
    std::unique_ptr<ClassMappingInfo> info = nullptr;
    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    if (ecRelationshipClass != nullptr)
        info = std::unique_ptr<ClassMappingInfo>(new RelationshipMappingInfo(ecdb, *ecRelationshipClass));
    else
        info = std::unique_ptr<ClassMappingInfo>(new ClassMappingInfo(ecdb, ecClass));

    if (info == nullptr || (mapStatus = info->Initialize()) != ClassMappingStatus::Success)
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
    : m_ecdb(ecdb), m_ecClass(ecClass), m_mapsToVirtualTable(ecClass.GetClassModifier() == ECClassModifier::Abstract), m_classHasCurrentTimeStampProperty(nullptr), m_tphBaseClassMap(nullptr)
    {}

//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::Initialize()
    {
    if (SUCCESS != _InitializeFromSchema())
        return ClassMappingStatus::Error;

    return EvaluateMapStrategy();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                    05/2016
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::EvaluateMapStrategy()
    {
    //Default values for table name and primary key column name
    if (m_tableName.empty())
        {
        // if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        if (SUCCESS != ClassMap::DetermineTableName(m_tableName, m_ecClass))
            return ClassMappingStatus::Error;
        }

    ClassMappingStatus stat = _EvaluateMapStrategy();
    if (stat != ClassMappingStatus::Success)
        return stat;

    //! We override m_mapsToVirtualTable if TablePerHierarchy was used.
    if (m_mapsToVirtualTable && m_mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy)
        m_mapsToVirtualTable = false;

    return ClassMappingStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus ClassMappingInfo::_EvaluateMapStrategy()
    {
    if (m_ecClass.IsCustomAttributeClass() || m_ecClass.IsStructClass())
        {
        LogClassNotMapped(NativeLogging::LOG_DEBUG, m_ecClass, "ECClass is a custom attribute or ECStruct which is never mapped to a table in ECDb.");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    if (ClassMap::IsAnyClass(m_ecClass) || (m_ecClass.GetSchema().IsStandardSchema() && m_ecClass.GetName().CompareTo("InstanceCount") == 0))
        {
        LogClassNotMapped(NativeLogging::LOG_INFO, m_ecClass, "ECClass is a standard class not supported by ECDb.");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    ClassMap const* baseClassMap = nullptr;
    ClassMappingStatus stat = TryGetBaseClassMap(baseClassMap);
    if (stat != ClassMappingStatus::Success)
        return stat;

    BeAssert(GetDbMap().GetSchemaImportContext() != nullptr);
    ClassMappingCACache const* caCacheCP = GetDbMap().GetSchemaImportContext()->GetClassMappingCACache(m_ecClass);
    if (caCacheCP == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    ClassMappingCACache const& caCache = *caCacheCP;

    if (baseClassMap == nullptr)
        {
        if (m_mapsToVirtualTable) // abstract class
            {
            if (caCache.HasMapStrategy() && (caCache.GetStrategy() == MapStrategy::ExistingTable ||
                caCache.GetStrategy() == MapStrategy::OwnTable ||
                caCache.GetStrategy() == MapStrategy::SharedTable))
                {
                Issues().Report(ECDbIssueSeverity::Error, "Invalid MapStrategy '%s' on abstract ECClass '%s'. Only MapStrategies 'TablePerHierarchy' or 'NotMapped' are allowed on abstract classes.", MapStrategyExtendedInfo::ToString(caCache.GetStrategy()), m_ecClass.GetFullName());
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
            case MapStrategy::SharedTable:
                //Those parent strategies are not inherited to subclasses.
                return AssignMapStrategy(caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

            case MapStrategy::NotMapped:
            {
            if (caCache.HasMapStrategy())
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy()));
                return ClassMappingStatus::Error;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return ClassMappingStatus::Success;
            }

            case MapStrategy::TablePerHierarchy:
             return EvaluateTablePerHierarchyMapStrategy(*baseClassMap, caCache) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

            default:
                BeAssert(false && "should not be called");
                return ClassMappingStatus::Error;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateTablePerHierarchyMapStrategy(ClassMap const& baseClassMap, ClassMappingCACache const& caCache)
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

    DbTable const& baseClassJoinedTable = baseClassMap.GetJoinedTable();
    m_tableName = baseClassJoinedTable.GetName();
    m_ecInstanceIdColumnName.assign(baseClassJoinedTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId)->GetName());

    ClassMappingCACache const* baseClassCACache = GetDbMap().GetSchemaImportContext()->GetClassMappingCACache(baseClassMap.GetClass());
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
            if (SUCCESS != ClassMap::DetermineTableName(m_tableName, m_ecClass))
                return ERROR;

            //For classes in the joined table the id column name is determined like this:
            //"<Rootclass name><Rootclass ECInstanceId column name>"
            ECClassId rootClassId = baseClassMap.GetTphHelper()->DetermineTphRootClassId();
            BeAssert(rootClassId.IsValid());
            ECClassCP rootClass = m_ecdb.Schemas().GetECClass(rootClassId);
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
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class MapStrategy 'TablePerHierarchy'. "
                        "For subclasses of a class with MapStrategy 'TablePerHierarchy': MapStrategy must be 'NotMapped' or unset.",
                        m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy()));
        return false;
        }

    if (!m_ecInstanceIdColumnName.empty())
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. For subclasses of an ECClass with MapStrategy TablePerHierarchy, ECInstanceIdColumn may not be defined in the ClassMap custom attribute.",
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
        GetDbMap().Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has the 'JoinedTablePerDirectSubclass' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                     "the 'JoinedTablePerDirectSubclass' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
        return ERROR;
        }

    if (caCache.GetShareColumnsCA().IsValid())
        {
        GetDbMap().Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has the 'ShareColumns' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                     "the 'ShareColumns' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMappingInfo::_InitializeFromSchema()
    {
    ClassMappingCACache const* caCacheCP = GetDbMap().GetSchemaImportContext()->GetClassMappingCACache(m_ecClass);
    if (caCacheCP == nullptr)
        return ERROR;

    ClassMappingCACache const& caCache = *caCacheCP;

    ECDbClassMap const& classMap = caCache.GetClassMap();

    if (classMap.IsValid())
        {
        if (ECObjectsStatus::Success != classMap.TryGetTableName(m_tableName))
            return ERROR;

        MapStrategy strategy = caCache.GetStrategy();
        if (strategy == MapStrategy::ExistingTable || strategy == MapStrategy::SharedTable)
            {
            if (m_tableName.empty())
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. TableName must not be empty in ClassMap custom attribute if MapStrategy is 'SharedTable' or 'ExistingTable'.",
                                m_ecClass.GetFullName());
                return ERROR;
                }
            }
        else
            {
            if (!m_tableName.empty())
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. TableName must only be set in ClassMap custom attribute if MapStrategy is 'SharedTable' or 'ExistingTable'.",
                                m_ecClass.GetFullName());
                return ERROR;
                }
            }

        if (ECObjectsStatus::Success != classMap.TryGetECInstanceIdColumn(m_ecInstanceIdColumnName))
            return ERROR;

        }

    if (SUCCESS != IndexMappingInfo::CreateFromECClass(m_dbIndexes, m_ecdb, m_ecClass, caCache.GetDbIndexListCA()))
        return ERROR;

    return InitializeClassHasCurrentTimeStampProperty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 muhammad.zaighum                01/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::InitializeClassHasCurrentTimeStampProperty()
    {
    IECInstancePtr ca = m_ecClass.GetCustomAttributeLocal("ClassHasCurrentTimeStampProperty");
    if (ca == nullptr)
        return SUCCESS;

    ECValue v;
    if (ca->GetValue(v, "PropertyName") == ECObjectsStatus::Success && !v.IsNull())
        {
        ECPropertyCP prop = m_ecClass.GetPropertyP(v.GetUtf8CP());
        if (nullptr == prop)
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. The property '%s' specified in the 'ClassHasCurrentTimeStampProperty' custom attribute "
                            "does not exist in the ECClass.", m_ecClass.GetFullName(), v.GetUtf8CP());
            return ERROR;
            }
        PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
        if (primProp == nullptr || primProp->GetType() != PrimitiveType::PRIMITIVETYPE_DateTime)
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. The property '%s' specified in the 'ClassHasCurrentTimeStampProperty' custom attribute "
                            "is not a primitive property of type 'DateTime'.", m_ecClass.GetFullName(), prop->GetName().c_str());
            return ERROR;

            }

        m_classHasCurrentTimeStampProperty = prop;
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
                    &baseClassMap->GetJoinedTable() != &tphBaseClassMap->GetJoinedTable())
                    {
                    Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has two base ECClasses with MapStrategy 'TablePerHierarchy' which don't map to the same tables. "
                                    "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'. "
                                    "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'.",
                                    m_ecClass.GetFullName(), tphBaseClassMap->GetClass().GetFullName(),
                                    tphBaseClassMap->GetPrimaryTable().GetName().c_str(), tphBaseClassMap->GetJoinedTable().GetName().c_str(),
                                    baseClassMap->GetClass().GetFullName(), baseClassMap->GetPrimaryTable().GetName().c_str(), baseClassMap->GetJoinedTable().GetName().c_str());
                    return ClassMappingStatus::Error;
                    }

                break;
                }

                case MapStrategy::OwnTable:
                    //we ignore abstract classes with own table as they match with the other strategies
                    if (baseClassMap->GetClass().GetClassModifier() != ECClassModifier::Abstract &&  ownTableBaseClassMap == nullptr)
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
        if (Issues().IsSeverityEnabled(ECDbIssueSeverity::Error))
            {
            Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has two base ECClasses with incompatible MapStrategies. "
                            "Base ECClass '%s' has MapStrategy '%s'."
                            "Base ECClass '%s' has MapStrategy '%s'.",
                            m_ecClass.GetFullName(),
                            tphBaseClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(tphBaseClassMap->GetMapStrategy().GetStrategy()),
                            violatingClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(violatingClassMap->GetMapStrategy().GetStrategy()));
            }

        return ClassMappingStatus::Error;
        }

    //As NotMapped applies to subclasses, it always overrides OwnTable.
    if (notMappedBaseClassMap != nullptr)
        {
        foundBaseClassMap = notMappedBaseClassMap;
        return ClassMappingStatus::Success;
        }

    BeAssert(ownTableBaseClassMap != nullptr);
    foundBaseClassMap = ownTableBaseClassMap;
    return ClassMappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2016
//+---------------+---------------+---------------+---------------+---------------+------
IssueReporter const& ClassMappingInfo::Issues() const { return m_ecdb.GetECDbImplR().GetIssueReporter(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ClassMappingInfo::LogClassNotMapped(NativeLogging::SEVERITY severity, ECClassCR ecClass, Utf8CP explanation)
    {
    Utf8CP classTypeStr = ecClass.GetRelationshipClassCP() != nullptr ? "ECRelationshipClass" : "ECClass";
    LOG.messagev(severity, "Skipped %s '%s' during mapping: %s", classTypeStr, ecClass.GetFullName(), explanation);
    }

//****************************************************************************************************
//RelationshipClassMapInfo
//****************************************************************************************************

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipMappingInfo::_InitializeFromSchema()
    {
    if (SUCCESS != ClassMappingInfo::_InitializeFromSchema())
        return ERROR;

    ECRelationshipClass const* relClass = m_ecClass.GetRelationshipClassCP();
    BeAssert(relClass != nullptr);
    BeAssert(relClass->GetBaseClasses().size() <= 1 && "Should actually have been enforced by ECSchemaValidator");

    ECDbForeignKeyRelationshipMap foreignKeyRelMap;
    const bool hasForeignKeyRelMap = ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap(foreignKeyRelMap, *relClass);

    const bool usePkAsFk = ECDbMapCustomAttributeHelper::HasUsePrimaryKeyAsForeignKey(*relClass);

    ECDbLinkTableRelationshipMap linkTableRelationMap;
    const bool hasLinkTableRelMap = ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkTableRelationMap, *relClass);

    if (relClass->HasBaseClasses() && (hasForeignKeyRelMap || hasLinkTableRelMap))
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has a base class and therefore must neither have the ForeignKeyRelationshipMap nor the LinkTableRelationshipMap custom attribute. Only the root relationship class of a hierarchy can have these custom attributes.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (hasForeignKeyRelMap && hasLinkTableRelMap)
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has the violating custom attributes 'ForeignKeyRelationshipMap' and 'LinkTableRelationshipMap'.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (hasLinkTableRelMap && usePkAsFk)
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has the violating custom attributes 'UsePrimaryKeyAsForeignKey' and 'LinkTableRelationshipMap'.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if ((hasForeignKeyRelMap || usePkAsFk) && RequiresLinkTable())
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has the 'ForeignKeyRelationshipMap' or 'UsePrimaryKeyAsForeignKey' custom attribute, but implies a link table mapping because of its cardinality or because it defines ECProperties.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (hasForeignKeyRelMap)
        {
        ECRelationshipEnd foreignKeyEnd = relClass->GetStrengthDirection() == ECRelatedInstanceDirection::Forward ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;

        ECRelationshipConstraintCP foreignKeyConstraint = nullptr;
        if (foreignKeyEnd == ECRelationshipEnd_Target)
            {
            foreignKeyConstraint = &relClass->GetTarget();
            m_customMapType = RelationshipMappingInfo::CustomMapType::ForeignKeyOnTarget;
            }
        else
            {
            foreignKeyConstraint = &relClass->GetSource();
            m_customMapType = RelationshipMappingInfo::CustomMapType::ForeignKeyOnSource;
            }

        Utf8String onDeleteActionStr;
        if (ECObjectsStatus::Success != foreignKeyRelMap.TryGetOnDeleteAction(onDeleteActionStr))
            return ERROR;

        Utf8String onUpdateActionStr;
        if (ECObjectsStatus::Success != foreignKeyRelMap.TryGetOnUpdateAction(onUpdateActionStr))
            return ERROR;

        const ForeignKeyDbConstraint::ActionType onDeleteAction = ForeignKeyDbConstraint::ToActionType(onDeleteActionStr.c_str());
        if (onDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade && relClass->GetStrength() != StrengthType::Embedding)
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. The ForeignKeyRelationshipMap custom attribute can only define 'Cascade' as OnDeleteAction if the relationship strength is 'Embedding'.",
                            m_ecClass.GetFullName());
            return ERROR;
            }


        m_fkMappingInfo = std::make_unique<FkMappingInfo>(onDeleteAction, ForeignKeyDbConstraint::ToActionType(onUpdateActionStr.c_str()), usePkAsFk);
        return SUCCESS;
        }

    if (hasLinkTableRelMap)
        {
        Utf8String sourceIdColName;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetSourceECInstanceIdColumn(sourceIdColName))
            return ERROR;

        Utf8String targetIdColName;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetTargetECInstanceIdColumn(targetIdColName))
            return ERROR;

        bool allowDuplicateRelationships = false;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetAllowDuplicateRelationships(allowDuplicateRelationships))
            return ERROR;

        m_linkTableMappingInfo = std::make_unique<LinkTableMappingInfo>(sourceIdColName, targetIdColName, allowDuplicateRelationships);
        m_customMapType = RelationshipMappingInfo::CustomMapType::LinkTable;
        return SUCCESS;
        }

    if (!relClass->HasBaseClasses())
        {
        if (RequiresLinkTable())
            m_linkTableMappingInfo = std::make_unique<LinkTableMappingInfo>();
        else
            m_fkMappingInfo = std::make_unique<FkMappingInfo>(usePkAsFk);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingStatus RelationshipMappingInfo::_EvaluateMapStrategy()
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    std::vector<ECClass const*> sourceClasses = GetDbMap().GetFlattenListOfClassesFromRelationshipEnd(relClass->GetSource());
    std::vector<ECClass const*> targetClasses = GetDbMap().GetFlattenListOfClassesFromRelationshipEnd(relClass->GetTarget());
    if (ContainsClassWithNotMappedStrategy(sourceClasses) || ContainsClassWithNotMappedStrategy(targetClasses))
        {
        LogClassNotMapped(NativeLogging::LOG_WARNING, m_ecClass, "The source or target constraint contains at least one ECClass which is not mapped. Therefore the ECRelationshipClass is not mapped either.");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    BeAssert(GetDbMap().GetSchemaImportContext() != nullptr);
    ClassMappingCACache const* caCache = GetDbMap().GetSchemaImportContext()->GetClassMappingCACache(m_ecClass);
    if (caCache == nullptr)
        {
        BeAssert(false);
        return ClassMappingStatus::Error;
        }

    const bool hasBaseClasses = m_ecClass.HasBaseClasses();
    ClassMap const* firstBaseClassMap = nullptr;
    if (hasBaseClasses)
        {
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
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache->GetStrategy()));
                return ClassMappingStatus::Error;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return ClassMappingStatus::Success;
            }

        if (baseStrategy == MapStrategy::OwnTable || baseStrategy == MapStrategy::ExistingTable || baseStrategy == MapStrategy::SharedTable)
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its base class %s has the MapStrategy 'OwnTable', 'ExistingTable' or 'SharedTable' which is not supported in an ECRelationshipClass hierarchy.",
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
            if (SUCCESS != EvaluateForeignKeyStrategy(*caCache, firstBaseClassMap))
                return ClassMappingStatus::Error;
            }
        else
            {
            BeAssert(firstBaseClassMap->GetType() == ClassMap::Type::RelationshipLinkTable);
            if (SUCCESS != EvaluateLinkTableStrategy(*caCache, firstBaseClassMap))
                return ClassMappingStatus::Error;
            }

        if (baseStrategy != m_mapStrategyExtInfo.GetStrategy())
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its mapping type (%s) differs from the mapping type of its base relationship class %s (%s). The mapping type must not change within an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_mapStrategyExtInfo.GetStrategy()), 
                            firstBaseClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(baseStrategy));
            return ClassMappingStatus::Error;
            }

        return ClassMappingStatus::Success;
        }

    //no base class
    if (caCache->HasMapStrategy() && caCache->GetStrategy() == MapStrategy::NotMapped)
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return ClassMappingStatus::Success;
        }

    if (m_linkTableMappingInfo != nullptr)
        return EvaluateLinkTableStrategy(*caCache, firstBaseClassMap) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;

    BeAssert(m_fkMappingInfo != nullptr);
    return EvaluateForeignKeyStrategy(*caCache, firstBaseClassMap) == SUCCESS ? ClassMappingStatus::Success : ClassMappingStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateLinkTableStrategy(ClassMappingCACache const& caCache, ClassMap const* baseClassMap)
    {
    BeAssert(baseClassMap != nullptr || m_linkTableMappingInfo != nullptr);

    if (m_customMapType == CustomMapType::ForeignKeyOnSource || m_customMapType == CustomMapType::ForeignKeyOnTarget)
        {
        BeAssert(baseClassMap != nullptr && "Should not call this method if baseClassMap == nullptr");
        if (baseClassMap != nullptr)
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has the ForeignKeyRelationshipMap custom attribute but its base class %s is mapped to a link table. The mapping type must not change in an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), baseClassMap->GetClass().GetFullName());

        return ERROR;
        }

    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    if (relClass->GetStrength() == StrengthType::Embedding)
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. The mapping rules imply a link table relationship, and link table relationships with strength 'Embedding' is not supported. See API docs for details on the mapping rules.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (m_customMapType == CustomMapType::ForeignKeyOnSource || m_customMapType == CustomMapType::ForeignKeyOnTarget)
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. The mapping rules imply a link table relationship. Therefore it must not have a ForeignKeyRelationshipMap custom attribute. See API docs for details on the mapping rules.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (HasKeyProperties(relClass->GetSource()) || HasKeyProperties(relClass->GetTarget()))
        {
        Issues().Report(ECDbIssueSeverity::Error, "The ECRelationshipClass '%s' is mapped to a link table. One of its constraints has Key properties which is only supported for foreign key type relationships.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (baseClassMap != nullptr)
        {
        BeAssert(baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy);
        m_linkTableMappingInfo = std::make_unique<LinkTableMappingInfo>(DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClassMap->GetClass().GetRelationshipClassCP()));
        return EvaluateTablePerHierarchyMapStrategy(*baseClassMap, caCache);
        }


    //*** root rel class
    //Table retrieval is only needed for the root rel class. Subclasses will use the tables of its base class
    //TODO: How should we handle this properly?
    m_sourceTables = GetTablesFromRelationshipEnd(relClass->GetSource(), true);
    m_targetTables = GetTablesFromRelationshipEnd(relClass->GetTarget(), true);

    if (m_sourceTables.empty() || m_targetTables.empty())
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. Source or target constraint classes are abstract without subclasses. Consider applying the MapStrategy 'SharedTable' to the abstract constraint class.",
                        m_ecClass.GetFullName());
        return ERROR;
        //Keep that code in case we need to relax our rule again:
        //LogClassNotMapped(NativeLogging::LOG_WARNING, m_ecClass, "Source or target constraint classes are abstract without subclasses.");
        //m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, true);
        //return SUCCESS;
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

        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. It is mapped to a link table, but the %s mapped to more than one table, which is not supported for link tables.",
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
BentleyStatus RelationshipMappingInfo::EvaluateForeignKeyStrategy(ClassMappingCACache const& caCache, ClassMap const* baseClassMap)
    {
    BeAssert(baseClassMap!= nullptr || m_fkMappingInfo != nullptr);

    if (m_customMapType == CustomMapType::LinkTable)
        {
        BeAssert(baseClassMap != nullptr && "Should not call this method if baseClassMap == nullptr");
        if (baseClassMap != nullptr)
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has the LinkTableRelationshipMap custom attribute but its base class %s has the ForeignKey type mapping. The mapping type must not change in an ECRelationshipClass hierarchy.", 
                        m_ecClass.GetFullName(), baseClassMap->GetClass().GetFullName());

        return ERROR;
        }

    if (caCache.GetClassMap().IsValid())
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It implies the ForeignKey type mapping, but also has the ClassMap custom attribute. ForeignKey type mappings can only have the ClassMap custom attribute when the MapStrategy is set to 'NotMapped'.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    const StrengthType strength = relClass->GetStrength();
    const ECRelatedInstanceDirection strengthDirection = relClass->GetStrengthDirection();

    MapStrategy resolvedStrategy = MapStrategy::NotMapped;
    switch (m_cardinality)
        {
            case Cardinality::OneToOne:
            {
            if (m_customMapType == CustomMapType::ForeignKeyOnSource)
                {
                BeAssert(strengthDirection == ECRelatedInstanceDirection::Backward);
                resolvedStrategy = MapStrategy::ForeignKeyRelationshipInSourceTable;
                }
            else if (m_customMapType == CustomMapType::ForeignKeyOnTarget)
                {
                BeAssert(strengthDirection == ECRelatedInstanceDirection::Forward);
                resolvedStrategy = MapStrategy::ForeignKeyRelationshipInTargetTable;
                }
            else
                {
                BeAssert(m_customMapType == CustomMapType::None);
                if (strengthDirection == ECRelatedInstanceDirection::Backward)
                    resolvedStrategy = MapStrategy::ForeignKeyRelationshipInSourceTable;
                else
                    resolvedStrategy = MapStrategy::ForeignKeyRelationshipInTargetTable;
                }

            break;
            }

            case Cardinality::OneToMany:
            {
            if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Backward)
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Forward'.",
                                m_ecClass.GetFullName(), relClass->GetSource().GetMultiplicity().ToString().c_str(), relClass->GetTarget().GetMultiplicity().ToString().c_str());
                return ERROR;
                }

            resolvedStrategy = MapStrategy::ForeignKeyRelationshipInTargetTable;
            break;
            }

            case Cardinality::ManyToOne:
            {
            if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Forward)
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Backward'.",
                                m_ecClass.GetFullName(), relClass->GetSource().GetMultiplicity().ToString().c_str(), relClass->GetTarget().GetMultiplicity().ToString().c_str());
                return ERROR;
                }

            resolvedStrategy = MapStrategy::ForeignKeyRelationshipInSourceTable;
            break;
            }

            default:
                BeAssert(false && "ManyToMany case should have been handled already.");
                return ERROR;
        }

    BeAssert(resolvedStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable || resolvedStrategy == MapStrategy::ForeignKeyRelationshipInTargetTable);

    //evaluate end tables
    const bool foreignKeyEndIsSource = resolvedStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable;

    //check that the referenced end tables (including joined tables) are 1 at most
    std::set<DbTable const*> referencedTables = GetTablesFromRelationshipEnd(foreignKeyEndIsSource ? relClass->GetTarget() : relClass->GetSource(), false);
    if (referencedTables.size() > 1)
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its foreign key end (%s) references more than one table (%s). See API docs for details on the mapping rules.",
                        m_ecClass.GetFullName(), foreignKeyEndIsSource ? "Source" : "Target", foreignKeyEndIsSource ? "Target" : "Source");
        return ERROR;
        }

    //For the foreign key end we want to include joined tables as we have to create FKs into them.
    //For the referenced end we are just interested in the primary table and ignore joined tables.
    const bool ignoreJoinedTableOnSource = !foreignKeyEndIsSource;
    const bool ignoreJoinedTableOnTarget = foreignKeyEndIsSource;
    m_sourceTables = GetTablesFromRelationshipEnd(relClass->GetSource(), ignoreJoinedTableOnSource);
    m_targetTables = GetTablesFromRelationshipEnd(relClass->GetTarget(), ignoreJoinedTableOnTarget);
    
    if (m_sourceTables.empty() || m_targetTables.empty())
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass '%s'. Source or target constraint classes are abstract without subclasses. Consider applying the MapStrategy 'SharedTable' to the abstract constraint class.",
                        m_ecClass.GetFullName());
        return ERROR;
        //Keep that code in case we need to relax our rule again:
        //LogClassNotMapped(NativeLogging::LOG_WARNING, m_ecClass, "Source or target constraint classes are abstract without subclasses.");
        //m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, true);
        //return SUCCESS;
        }

    m_mapStrategyExtInfo = MapStrategyExtendedInfo(resolvedStrategy);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool RelationshipMappingInfo::ContainsClassWithNotMappedStrategy(std::vector<ECN::ECClassCP> const& classes) const
    {
    for (ECClassCP ecClass : classes)
        {
        ClassMap const* classMap = GetDbMap().GetClassMap(*ecClass);
        BeAssert(classMap != nullptr);
        if (classMap == nullptr || classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
            return true;
        }

    return false;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                06/2014
//+---------------+---------------+---------------+---------------+---------------+------
void RelationshipMappingInfo::DetermineCardinality()
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    const bool sourceIsM = relClass->GetSource().GetMultiplicity().GetUpperLimit() > 1;
    const bool targetIsM = relClass->GetTarget().GetMultiplicity().GetUpperLimit() > 1;
    if (sourceIsM && targetIsM)
        m_cardinality = Cardinality::ManyToMany;
    else if (!sourceIsM && targetIsM)
        m_cardinality = Cardinality::OneToMany;
    else if (sourceIsM && !targetIsM)
        m_cardinality = Cardinality::ManyToOne;
    else
        m_cardinality = Cardinality::OneToOne;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                10/2015
//+---------------+---------------+---------------+---------------+---------------+-
//static
bool RelationshipMappingInfo::HasKeyProperties(ECN::ECRelationshipConstraint const& constraint)
    {
    for (ECRelationshipConstraintClassCP constraintClass : constraint.GetConstraintClasses())
        {
        if (!constraintClass->GetKeys().empty())
            return true;
        }

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                10/2015
//+---------------+---------------+---------------+---------------+---------------+-
//static
bool RelationshipMappingInfo::DetermineAllowDuplicateRelationshipsFlagFromRoot(ECRelationshipClassCR baseRelClass)
    {
    ECDbLinkTableRelationshipMap linkRelMap;
    if (ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkRelMap, baseRelClass))
        {
        bool allowDuplicateRels = false;
        linkRelMap.TryGetAllowDuplicateRelationships(allowDuplicateRels);
        if (allowDuplicateRels)
            return true;
        }

    if (!baseRelClass.HasBaseClasses())
        return false;

    BeAssert(baseRelClass.GetBaseClasses()[0]->GetRelationshipClassCP() != nullptr);
    return DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseRelClass.GetBaseClasses()[0]->GetRelationshipClassCP());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                      12/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::set<DbTable const*> RelationshipMappingInfo::GetTablesFromRelationshipEnd(ECRelationshipConstraintCR relationshipEnd, bool ignoreJoinedTables) const
    {
    bool hasAnyClass = false;
    std::set<ClassMap const*> classMaps = GetDbMap().GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

    if (hasAnyClass)
        return std::set<DbTable const*>();

    std::map<DbTable const*, std::set<DbTable const*>> joinedTables;
    std::set<DbTable const*> tables;
    for (ClassMap const* classMap : classMaps)
        {
        std::vector<DbTable*> const& classPersistInTables = classMap->GetTables();
        if (classPersistInTables.size() == 1)
            {
            tables.insert(classPersistInTables.front());
            continue;
            }

        for (DbTable const* table : classPersistInTables)
            {
            if (DbTable const* primaryTable = table->GetParentOfJoinedTable())
                {
                joinedTables[primaryTable].insert(table);
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
            for (auto childTable : primaryTable->GetJoinedTables())
                tables.erase(childTable);

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

    std::map<PersistenceType, std::set<DbTable const*>> finalListOfTables;
    for (DbTable const* table : tables)
        {
        finalListOfTables[table->GetPersistenceType()].insert(table);
        }

    return finalListOfTables[PersistenceType::Persisted];
    }


//***************** IndexMappingInfo ****************************************

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus IndexMappingInfo::CreateFromECClass(std::vector<IndexMappingInfoPtr>& indexInfos, ECDbCR ecdb, ECClassCR ecClass, DbIndexList const& dbIndexListCA)
    {
    if (dbIndexListCA.IsValid())
        {
        bvector<DbIndexList::DbIndex> indices;
        if (ECObjectsStatus::Success != dbIndexListCA.GetIndexes(indices))
            return ERROR;

        for (DbIndexList::DbIndex const& index : indices)
            {
            bool addPropsAreNotNullWhereExp = false;

            Utf8CP whereClause = index.GetWhereClause();
            if (!Utf8String::IsNullOrEmpty(whereClause))
                {
                if (BeStringUtilities::StricmpAscii(whereClause, "IndexedColumnsAreNotNull") == 0 ||
                    BeStringUtilities::StricmpAscii(whereClause, "ECDB_NOTNULL") == 0) //legacy support
                    addPropsAreNotNullWhereExp = true;
                else
                    {
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Invalid where clause in DbIndexList::DbIndex: %s. Only 'IndexedColumnsAreNotNull' is supported by ECDb.", ecClass.GetFullName(), index.GetWhereClause());
                    return ERROR;
                    }
                }

            indexInfos.push_back(new IndexMappingInfo(index.GetName(), index.IsUnique(), index.GetProperties(), addPropsAreNotNullWhereExp));
            }
        }

    return CreateFromIdSpecificationCAs(indexInfos, ecdb, ecClass);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus IndexMappingInfo::CreateFromIdSpecificationCAs(std::vector<IndexMappingInfoPtr>& indexInfos, ECDbCR ecdb, ECN::ECClassCR ecClass)
    {
    std::vector<std::pair<Utf8CP, Utf8CP>> idSpecCAs;
    idSpecCAs.push_back(std::make_pair("BusinessKeySpecification", "PropertyName"));
    idSpecCAs.push_back(std::make_pair("GlobalIdSpecification", "PropertyName"));
    idSpecCAs.push_back(std::make_pair("SyncIDSpecification", "Property"));

    bmap<Utf8String, bvector<Utf8CP>, CompareIUtf8Ascii> distinctPropNames;

    for (std::pair<Utf8CP, Utf8CP> const& idSpecCA : idSpecCAs)
        {
        Utf8CP caName = idSpecCA.first;
        Utf8CP caPropName = idSpecCA.second;
        IECInstancePtr ca = ecClass.GetCustomAttribute(caName);
        if (ca == nullptr)
            continue;

        ECValue v;
        if (ECObjectsStatus::Success != ca->GetValue(v, caPropName) || v.IsNull() || Utf8String::IsNullOrEmpty(v.GetUtf8CP()))
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                          "Failed to map ECClass %s. Its custom attribute %s is invalid. Could not retrieve value of property '%s' from the custom attribute.",
                                                          ecClass.GetFullName(), caName, caPropName);
            return ERROR;
            }

        Utf8CP idPropName = v.GetUtf8CP();
        if (ecClass.GetPropertyP(idPropName) == nullptr)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                          "Failed to map ECClass %s. Its custom attribute %s is invalid. The property '%s' specified in the custom attribute does not exist in the ECClass.",
                                                          ecClass.GetFullName(), caName, idPropName);
            return ERROR;
            }

        distinctPropNames[idPropName].push_back(caName);
        }

    for (bpair<Utf8String, bvector<Utf8CP>> const& kvPair : distinctPropNames)
        {
        Utf8String indexName("ix_");
        indexName.append(ecClass.GetSchema().GetAlias()).append("_").append(ecClass.GetName()).append("_");

        for (Utf8CP caName : kvPair.second)
            {
            indexName.append(caName).append("_");
            }

        Utf8StringCR idPropName = kvPair.first;
        indexName.append(idPropName);

        std::vector<Utf8String> indexPropNameVector;
        indexPropNameVector.push_back(idPropName);
        indexInfos.push_back(new IndexMappingInfo(indexName.c_str(), false, indexPropNameVector, false));
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus IndexMappingInfoCache::TryGetIndexInfos(std::vector<IndexMappingInfoPtr> const*& indexInfos, ClassMap const& classMap) const
    {
    //first look in class map info cache
    auto classMapInfoCacheIt = m_schemaImportContext.GetClassMappingInfoCache().find(&classMap);
    if (classMapInfoCacheIt != m_schemaImportContext.GetClassMappingInfoCache().end())
        {
        indexInfos = &classMapInfoCacheIt->second->GetIndexInfos();
        return SUCCESS;
        }

    //now look in internal cache
    auto indexInfoCacheIt = m_indexInfoCache.find(&classMap);
    if (indexInfoCacheIt != m_indexInfoCache.end())
        {
        indexInfos = &indexInfoCacheIt->second;
        return SUCCESS;
        }

    //not in internal cache, so read index info from ECClass (and cache it)
    std::vector<IndexMappingInfoPtr>& newIndexInfos = m_indexInfoCache[&classMap];
    ECClassCR ecClass = classMap.GetClass();
    DbIndexList dbIndexListCA;
    ECDbMapCustomAttributeHelper::TryGetDbIndexList(dbIndexListCA, ecClass);
    if (SUCCESS != IndexMappingInfo::CreateFromECClass(newIndexInfos, m_ecdb, ecClass, dbIndexListCA))
        return ERROR;

    indexInfos = &newIndexInfos;
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
