/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <stack>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**********************************************************************************************
// ClassMappingInfoFactory
//**********************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ClassMappingInfo> ClassMappingInfoFactory::Create(MappingStatus& mapStatus, ECN::ECClassCR ecClass, ECDbMap const& ecDbMap)
    {
    std::unique_ptr<ClassMappingInfo> info = nullptr;
    ECRelationshipClassCP ecRelationshipClass = ecClass.GetRelationshipClassCP();
    if (ecRelationshipClass != nullptr)
        info = std::unique_ptr<ClassMappingInfo>(new RelationshipMappingInfo(*ecRelationshipClass, ecDbMap));
    else
        info = std::unique_ptr<ClassMappingInfo>(new ClassMappingInfo(ecClass, ecDbMap));

    if (info == nullptr || (mapStatus = info->Initialize()) != MappingStatus::Success)
        return nullptr;

    return info;
    }


//**********************************************************************************************
// ClassMappingInfo
//**********************************************************************************************
//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
ClassMappingInfo::ClassMappingInfo(ECClassCR ecClass, ECDbMap const& ecDbMap)
    : m_ecdbMap(ecDbMap), m_ecClass(ecClass), m_mapsToVirtualTable(ecClass.GetClassModifier() == ECClassModifier::Abstract), m_classHasCurrentTimeStampProperty(nullptr), m_baseClassMap(nullptr)
    {}

//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus ClassMappingInfo::Initialize()
    {
    if (SUCCESS != _InitializeFromSchema())
        return MappingStatus::Error;

    return EvaluateMapStrategy();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                 Krischan.Eberle                    05/2016
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus ClassMappingInfo::EvaluateMapStrategy()
    {
    //Default values for table name and primary key column name
    if (m_tableName.empty())
        {
        // if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        if (SUCCESS != ClassMap::DetermineTableName(m_tableName, m_ecClass))
            return MappingStatus::Error;
        }


    MappingStatus stat = _EvaluateMapStrategy();
    if (stat != MappingStatus::Success)
        return stat;

    //_EvaluateMapStrategy can set the column name. So only set it to a default, if it hasn't been set so far.
    if (m_ecInstanceIdColumnName.empty())
        m_ecInstanceIdColumnName = ECDB_COL_ECInstanceId;

    //! We override m_mapsToVirtualTable if TablePerHierarchy was used.
    if (m_mapsToVirtualTable && m_mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy)
        m_mapsToVirtualTable = false;

    return MappingStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus ClassMappingInfo::_EvaluateMapStrategy()
    {
    if (m_ecClass.IsCustomAttributeClass() || m_ecClass.IsStructClass())
        {
        LogClassNotMapped(NativeLogging::LOG_DEBUG, m_ecClass, "ECClass is a custom attribute or ECStruct which is never mapped to a table in ECDb.");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return MappingStatus::Success;
        }

    if (ClassMap::IsAnyClass(m_ecClass) || (m_ecClass.GetSchema().IsStandardSchema() && m_ecClass.GetName().CompareTo("InstanceCount") == 0))
        {
        LogClassNotMapped(NativeLogging::LOG_INFO, m_ecClass, "ECClass is a standard class not supported by ECDb.");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return MappingStatus::Success;
        }

    ClassMap const* baseClassMap = nullptr;
    MappingStatus stat = TryGetBaseClassMap(baseClassMap);
    if (stat != MappingStatus::Success)
        return stat;

    BeAssert(m_ecdbMap.GetSchemaImportContext() != nullptr);
    ClassMappingCACache const* caCacheCP = m_ecdbMap.GetSchemaImportContext()->GetClassMappingCACache(m_ecClass);
    if (caCacheCP == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
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
                return MappingStatus::Error;
                }
            }

        return AssignMapStrategy(caCache) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;
        }

    BeAssert(baseClassMap != nullptr);
    MapStrategy baseStrategy = baseClassMap->GetMapStrategy().GetStrategy();

    switch (baseStrategy)
        {
            case MapStrategy::OwnTable:
            case MapStrategy::ExistingTable:
            case MapStrategy::SharedTable:
                //Those parent strategies are not inherited to subclasses.
                return AssignMapStrategy(caCache) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;

            case MapStrategy::NotMapped:
            {
            if (caCache.HasMapStrategy())
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache.GetStrategy()));
                return MappingStatus::Error;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return MappingStatus::Success;
            }

            case MapStrategy::TablePerHierarchy:
             return EvaluateTablePerHierarchyMapStrategy(*baseClassMap, caCache) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;
            default:
                BeAssert(false && "should not be called");
                return MappingStatus::Error;
        }

    return MappingStatus::Success;
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

    m_baseClassMap = &baseClassMap; //only need to hold the base class map for TPH case

    DbTable const& baseClassJoinedTable = baseClassMap.GetJoinedTable();
    m_tableName = baseClassJoinedTable.GetName();
    m_ecInstanceIdColumnName.assign(baseClassJoinedTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId)->GetName());

    ClassMappingCACache const* baseClassCACache = m_ecdbMap.GetSchemaImportContext()->GetClassMappingCACache(baseClassMap.GetClass());
    if (baseClassCACache == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    JoinedTableInfo baseClassJoinedTableInfo = baseClassMap.GetMapStrategy().GetTphInfo().GetJoinedTableInfo();
    TablePerHierarchyInfo tphInfo;
    if (SUCCESS != tphInfo.Initialize(caCache.GetShareColumnsCA(), &baseClassMap.GetMapStrategy(), &baseClassCACache->GetShareColumnsCA(),
                                                  caCache.HasJoinedTablePerDirectSubclassOption(), m_ecClass, Issues()))
        return ERROR;

    if (tphInfo.GetJoinedTableInfo() == JoinedTableInfo::JoinedTable)
        {
        for (ECClassCP anotherBaseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* anotherBaseClassMap = GetECDbMap().GetClassMap(*anotherBaseClass);
            BeAssert(anotherBaseClassMap != nullptr);
            if (anotherBaseClassMap == &baseClassMap || anotherBaseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
                continue;

            //! Skip interface classes implement by primary class
            if (anotherBaseClassMap->IsMappedToSingleTable() && anotherBaseClassMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Virtual)
                continue;

            if (&baseClassMap.GetPrimaryTable() != &anotherBaseClassMap->GetPrimaryTable() || &baseClassMap.GetJoinedTable() != &anotherBaseClassMap->GetJoinedTable())
                {
                m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                             "ECClass '%s' has two base ECClasses which don't map to the same tables. "
                                                                             "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'. "
                                                                             "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'.",
                                                                             m_ecClass.GetFullName(), baseClassMap.GetClass().GetFullName(),
                                                                             baseClassMap.GetPrimaryTable().GetName().c_str(), baseClassMap.GetJoinedTable().GetName().c_str(),
                                                                             anotherBaseClass->GetFullName(), anotherBaseClassMap->GetPrimaryTable().GetName().c_str(),
                                                                             anotherBaseClassMap->GetJoinedTable().GetName().c_str());
                return ERROR;
                }
            }

        if (baseClassJoinedTableInfo == JoinedTableInfo::ParentOfJoinedTable)
            {
            //Joined tables are named after the class which becomes the root class of classes in the joined table
            if (SUCCESS != ClassMap::DetermineTableName(m_tableName, m_ecClass))
                return ERROR;

            //For classes in the joined table the id column name is determined like this:
            //"<Rootclass name><Rootclass ECInstanceId column name>"
            ClassMap const* rootClassMap = baseClassMap.FindTablePerHierarchyRootClassMap();
            if (rootClassMap == nullptr)
                {
                BeAssert(false && "There should always be a root class map which defines the TablePerHierarchy strategy");
                return ERROR;
                }

            m_ecInstanceIdColumnName.Sprintf("%s%s", rootClassMap->GetClass().GetName().c_str(), m_ecInstanceIdColumnName.c_str());
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
        GetECDbMap().Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has the 'JoinedTablePerDirectSubclass' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
                                     "the 'JoinedTablePerDirectSubclass' custom attribute can only be used with the MapStrategy 'TablePerHierarchy'.", m_ecClass.GetFullName());
        return ERROR;
        }

    if (caCache.GetShareColumnsCA().IsValid())
        {
        GetECDbMap().Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has the 'ShareColumns' custom attribute but not the MapStrategy 'TablePerHierarchy'. "
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
    ClassMappingCACache const* caCacheCP = m_ecdbMap.GetSchemaImportContext()->GetClassMappingCACache(m_ecClass);
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

    if (SUCCESS != IndexMappingInfo::CreateFromECClass(m_dbIndexes, m_ecdbMap.GetECDb(), m_ecClass, classMap.IsValid() ? &classMap : nullptr))
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
MappingStatus ClassMappingInfo::TryGetBaseClassMap(ClassMap const*& baseClassMap) const
    {
    std::vector<ClassMap const*> tphBaseClassMaps;
    ClassMap const* ownTableBaseClassMap = nullptr;
    ClassMap const* notMappedBaseClassMap = nullptr;
    for (ECClassCP baseClass : m_ecClass.GetBaseClasses())
        {
        ClassMap const* baseClassMap = m_ecdbMap.GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            return MappingStatus::BaseClassesNotMapped;

        MapStrategy baseMapStrategy = baseClassMap->GetMapStrategy().GetStrategy();
        switch (baseMapStrategy)
            {
                case MapStrategy::TablePerHierarchy:
                {
                DbTable const& baseTable = baseClassMap->GetPrimaryTable();
                bool add = true;
                for (ClassMap const* classMap : tphBaseClassMaps)
                    {
                    if (&classMap->GetPrimaryTable() == &baseTable)
                        {
                        add = false;
                        break;
                        }
                    }

                if (add)
                    tphBaseClassMaps.push_back(baseClassMap);
                break;
                }

                case MapStrategy::OwnTable:
                    ownTableBaseClassMap = baseClassMap;
                    break;

                case MapStrategy::NotMapped:
                    notMappedBaseClassMap = baseClassMap;
                    break;

                default:
                    BeAssert(false && "Unhandled MapStrategy for regular ECClass");
                    break;
            }
        }

    if (tphBaseClassMaps.size() > 1)
        {
        // ClassMappingRule: No more than one ancestor of a class can use TablePerHierarchy strategy. Mapping fails if this is violated
        if (Issues().IsSeverityEnabled(ECDbIssueSeverity::Error))
            {
            Utf8String baseClasses;
            for (ClassMap const* baseMap : tphBaseClassMaps)
                {
                baseClasses.append(baseMap->GetClass().GetFullName());
                baseClasses.append(" ");
                }

            Issues().Report(ECDbIssueSeverity::Error, "ECClass '%s' has more than one base ECClass with the MapStrategy 'TablePerHierarchy'. This is not supported. The violating base ECClasses are: %s",
                            m_ecClass.GetFullName(), baseClasses.c_str());
            }

        return MappingStatus::Error;
        }

    if (!tphBaseClassMaps.empty())
        baseClassMap = tphBaseClassMaps[0];
    else if (ownTableBaseClassMap != nullptr)
        baseClassMap = ownTableBaseClassMap;
    else
        baseClassMap = notMappedBaseClassMap;

    return MappingStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2016
//+---------------+---------------+---------------+---------------+---------------+------
IssueReporter const& ClassMappingInfo::Issues() const { return m_ecdbMap.Issues(); }

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
    ECDbLinkTableRelationshipMap linkTableRelationMap;
    const bool hasLinkTableRelMap = ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkTableRelationMap, *relClass);

    if (hasForeignKeyRelMap && hasLinkTableRelMap)
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has a base class and therefore must not define Key properties on its constraints.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (relClass->HasBaseClasses() && (hasForeignKeyRelMap || hasLinkTableRelMap))
        {
        Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. It has a base class and therefore must neither have the ForeignKeyRelationshipMap nor the LinkTableRelationshipMap custom attribute. Only the root relationship class of a hierarchy can have these custom attributes.",
                        m_ecClass.GetFullName());
        return ERROR;
        }

    if (hasForeignKeyRelMap)
        {
        ECRelationshipEnd foreignKeyEnd = relClass->GetStrengthDirection() == ECRelatedInstanceDirection::Forward ? ECRelationshipEnd_Target : ECRelationshipEnd_Source;

        RelationshipEndColumns* foreignKeyColumnsMapping = nullptr;
        ECRelationshipConstraintCP foreignKeyConstraint = nullptr;
        if (foreignKeyEnd == ECRelationshipEnd_Target)
            {
            foreignKeyConstraint = &relClass->GetTarget();
            foreignKeyColumnsMapping = &m_targetColumnsMapping;
            m_sourceColumnsMappingIsNull = true;
            m_targetColumnsMappingIsNull = false;
            m_customMapType = RelationshipMappingInfo::CustomMapType::ForeignKeyOnTarget;
            }
        else
            {
            foreignKeyConstraint = &relClass->GetSource();
            foreignKeyColumnsMapping = &m_sourceColumnsMapping;
            m_sourceColumnsMappingIsNull = false;
            m_targetColumnsMappingIsNull = true;
            m_customMapType = RelationshipMappingInfo::CustomMapType::ForeignKeyOnSource;
            }

        Utf8String foreignKeyColName;
        Utf8String foreignKeyClassIdColName;
        if (ECObjectsStatus::Success != foreignKeyRelMap.TryGetForeignKeyColumn(foreignKeyColName))
            return ERROR;

        if (!foreignKeyColName.empty())
            {
            for (ECRelationshipConstraintClassCP constraintClass : foreignKeyConstraint->GetConstraintClasses())
                {
                if (!constraintClass->GetKeys().empty())
                    {
                    Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. The ForeignKeyRelationshipMap custom attribute must not have a value for ForeignKeyColumn as there are Key properties defined in the ECRelationshipConstraint on the foreign key end.",
                                    m_ecClass.GetFullName());
                    return ERROR;
                    }
                }
            }

        *foreignKeyColumnsMapping = RelationshipEndColumns(foreignKeyColName.c_str());

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

        m_onDeleteAction = onDeleteAction;
        m_onUpdateAction = ForeignKeyDbConstraint::ToActionType(onUpdateActionStr.c_str());

        //default ForeignKeyRelationshipMap.CreateIndex is true in case CA or the CreateIndex prop is not set
        m_createIndexOnForeignKey = true;
        if (ECObjectsStatus::Success != foreignKeyRelMap.TryGetCreateIndex(m_createIndexOnForeignKey))
            return ERROR;

        return SUCCESS;
        }

    if (hasLinkTableRelMap)
        {
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetAllowDuplicateRelationships(m_allowDuplicateRelationships))
            return ERROR;

        Utf8String sourceIdColName;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetSourceECInstanceIdColumn(sourceIdColName))
            return ERROR;

        Utf8String sourceClassIdColName;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetSourceECClassIdColumn(sourceClassIdColName))
            return ERROR;

        Utf8String targetIdColName;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetTargetECInstanceIdColumn(targetIdColName))
            return ERROR;

        Utf8String targetClassIdColName;
        if (ECObjectsStatus::Success != linkTableRelationMap.TryGetTargetECClassIdColumn(targetClassIdColName))
            return ERROR;

        m_sourceColumnsMappingIsNull = false;
        m_sourceColumnsMapping = RelationshipEndColumns(sourceIdColName.c_str(), sourceClassIdColName.c_str());
        m_targetColumnsMappingIsNull = false;
        m_targetColumnsMapping = RelationshipEndColumns(targetIdColName.c_str(), targetClassIdColName.c_str());
        m_customMapType = RelationshipMappingInfo::CustomMapType::LinkTable;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus RelationshipMappingInfo::_EvaluateMapStrategy()
    {
    DetermineCardinality();

    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    std::vector<ECClass const*> sourceClasses = m_ecdbMap.GetFlattenListOfClassesFromRelationshipEnd(relClass->GetSource());
    std::vector<ECClass const*> targetClasses = m_ecdbMap.GetFlattenListOfClassesFromRelationshipEnd(relClass->GetTarget());
    if (ContainsClassWithNotMappedStrategy(sourceClasses) || ContainsClassWithNotMappedStrategy(targetClasses))
        {
        LogClassNotMapped(NativeLogging::LOG_WARNING, m_ecClass, "The source or target constraint contains at least one ECClass which is not mapped. Therefore the ECRelationshipClass is not mapped either.");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return MappingStatus::Success;
        }

    BeAssert(m_ecdbMap.GetSchemaImportContext() != nullptr);
    ClassMappingCACache const* caCache = m_ecdbMap.GetSchemaImportContext()->GetClassMappingCACache(m_ecClass);
    if (caCache == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    const bool hasBaseClasses = m_ecClass.HasBaseClasses();
    ClassMap const* baseClassMap = nullptr;
    if (hasBaseClasses)
        {
        ECRelationshipClassCP baseClass = m_ecClass.GetBaseClasses()[0]->GetRelationshipClassCP();
        BeAssert(baseClass != nullptr);
        baseClassMap = m_ecdbMap.GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            return MappingStatus::BaseClassesNotMapped;

        const MapStrategy baseStrategy = baseClassMap->GetMapStrategy().GetStrategy();

        if (baseStrategy == MapStrategy::NotMapped)
            {
            if (caCache->HasMapStrategy())
                {
                Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Its MapStrategy '%s' does not match the base class's MapStrategy 'NotMapped'. "
                                "Subclasses of an ECClass with MapStrategy 'NotMapped' must not define a MapStrategy.",
                                m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(caCache->GetStrategy()));
                return MappingStatus::Error;
                }

            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return MappingStatus::Success;
            }

        if (baseStrategy == MapStrategy::ExistingTable || baseStrategy == MapStrategy::SharedTable)
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its base class %s has the MapStrategy 'ExistingTable' or 'SharedTable' which is not supported in an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), baseClassMap->GetClass().GetFullName());
            return MappingStatus::Error;
            }

        if (caCache->HasMapStrategy() && caCache->GetStrategy() == MapStrategy::NotMapped)
            {
            m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
            return MappingStatus::Success;
            }

        m_baseClassMap = baseClassMap;
        if (baseClassMap->GetType() == ClassMap::Type::RelationshipEndTable)
            {
            if (SUCCESS != EvaluateForeignKeyStrategy(*caCache, baseClassMap))
                return MappingStatus::Error;
            }
        else
            {
            BeAssert(baseClassMap->GetType() == ClassMap::Type::RelationshipLinkTable);
            if (SUCCESS != EvaluateLinkTableStrategy(*caCache, baseClassMap))
                return MappingStatus::Error;
            }

        if (baseStrategy != m_mapStrategyExtInfo.GetStrategy())
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. Its mapping type (%s) differs from the mapping type of its base relationship class %s (%s). The mapping type must not change within an ECRelationshipClass hierarchy.",
                            m_ecClass.GetFullName(), MapStrategyExtendedInfo::ToString(m_mapStrategyExtInfo.GetStrategy()), 
                            baseClassMap->GetClass().GetFullName(), MapStrategyExtendedInfo::ToString(baseStrategy));
            return MappingStatus::Error;
            }

        return MappingStatus::Success;
        }

    //no base class
    if (caCache->HasMapStrategy() && caCache->GetStrategy() == MapStrategy::NotMapped)
        {
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::NotMapped);
        return MappingStatus::Success;
        }

    if (m_customMapType == CustomMapType::LinkTable || m_cardinality == Cardinality::ManyToMany || m_ecClass.GetPropertyCount() > 0)
        return EvaluateLinkTableStrategy(*caCache, baseClassMap) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;

    return EvaluateForeignKeyStrategy(*caCache, baseClassMap) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateLinkTableStrategy(ClassMappingCACache const& caCache, ClassMap const* baseClassMap)
    {
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
        BeAssert(!m_allowDuplicateRelationships && "m_allowDuplicateRelationships is expected to only be set in root class");
        m_allowDuplicateRelationships = DetermineAllowDuplicateRelationshipsFlagFromRoot(*baseClassMap->GetClass().GetRelationshipClassCP());

        if (baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::TablePerHierarchy)
            return EvaluateTablePerHierarchyMapStrategy(*baseClassMap, caCache);

        if (caCache.GetClassMap().IsValid())
            {
            Issues().Report(ECDbIssueSeverity::Error, "Failed to map ECRelationshipClass %s. ECRelationship subclasses may not define the ClassMap custom attribute.",
                            m_ecClass.GetFullName());
            return ERROR;
            }


        BeAssert(baseClassMap->GetMapStrategy().GetStrategy() == MapStrategy::OwnTable && "all other strategy cases shoudld have been caught before");
        m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::OwnTable);
        return SUCCESS;
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
    const MapStrategy strat = m_ecClass.GetClassModifier() == ECClassModifier::Abstract ? MapStrategy::TablePerHierarchy : MapStrategy::OwnTable;
    m_mapStrategyExtInfo = MapStrategyExtendedInfo(strat);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                05 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::EvaluateForeignKeyStrategy(ClassMappingCACache const& caCache, ClassMap const* baseClassMap)
    {
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
        ClassMap const* classMap = m_ecdbMap.GetClassMap(*ecClass);
        BeAssert(classMap != nullptr);
        if (classMap == nullptr || classMap->GetMapStrategy().GetStrategy() == MapStrategy::NotMapped)
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipEndColumns const& RelationshipMappingInfo::GetColumnsMapping(ECRelationshipEnd end) const
    {
    if (end == ECRelationshipEnd_Source)
        {
        BeAssert(m_customMapType != CustomMapType::ForeignKeyOnTarget && m_mapStrategyExtInfo.GetStrategy() != MapStrategy::ForeignKeyRelationshipInTargetTable);
        return m_sourceColumnsMapping;
        }

    BeAssert(m_customMapType != CustomMapType::ForeignKeyOnSource && m_mapStrategyExtInfo.GetStrategy() != MapStrategy::ForeignKeyRelationshipInSourceTable);
    return m_targetColumnsMapping;
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
    std::set<ClassMap const*> classMaps = m_ecdbMap.GetClassMapsFromRelationshipEnd(relationshipEnd, &hasAnyClass);

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
BentleyStatus IndexMappingInfo::CreateFromECClass(std::vector<IndexMappingInfoPtr>& indexInfos, ECDbCR ecdb, ECClassCR ecClass, ECDbClassMap const* customClassMap)
    {
    if (customClassMap != nullptr)
        {
        bvector<ECDbClassMap::DbIndex> indices;
        if (ECObjectsStatus::Success != customClassMap->TryGetIndexes(indices))
            return ERROR;

        for (ECDbClassMap::DbIndex const& index : indices)
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
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. Invalid where clause in ClassMap::DbIndex: %s. Only 'IndexedColumnsAreNotNull' is supported by ECDb.", ecClass.GetFullName(), index.GetWhereClause());
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

        Utf8String indexName;
        indexName.Sprintf("ix_%s_%s_%s_%s", ecClass.GetSchema().GetAlias().c_str(), ecClass.GetName().c_str(),
                          caName, idPropName);

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
    ECDbClassMap customClassMap;
    const bool hasCustomClassMap = ECDbMapCustomAttributeHelper::TryGetClassMap(customClassMap, ecClass);
    if (SUCCESS != IndexMappingInfo::CreateFromECClass(newIndexInfos, m_ecdb, ecClass, hasCustomClassMap ? &customClassMap : nullptr))
        return ERROR;

    indexInfos = &newIndexInfos;
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
