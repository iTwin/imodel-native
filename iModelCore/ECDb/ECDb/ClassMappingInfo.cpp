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
std::unique_ptr<ClassMappingInfo> ClassMappingInfoFactory::Create(MappingStatus& mapStatus, ECN::ECClassCR ecClass, ECDbMapCR ecDbMap)
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
ClassMappingInfo::ClassMappingInfo (ECClassCR ecClass, ECDbMapCR ecDbMap)
    : m_ecdbMap(ecDbMap), m_ecClass(ecClass), m_isMapToVirtualTable(ecClass.GetClassModifier() == ECClassModifier::Abstract), m_classHasCurrentTimeStampProperty(nullptr), m_parentClassMap(nullptr)
    {}

//---------------------------------------------------------------------------------
//@bsimethod                                 Affan.Khan                            07/2012
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus ClassMappingInfo::Initialize()
    {
    if (SUCCESS != _InitializeFromSchema())
        return MappingStatus::Error;

    //Default values for table name and primary key column name
    if (m_tableName.empty())
        {
        // if hint does not supply a table name, use {ECSchema prefix}_{ECClass name}
        if (SUCCESS != ClassMap::DetermineTableName(m_tableName, m_ecClass))
            return MappingStatus::Error;
        }

    return _EvaluateMapStrategy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MappingStatus ClassMappingInfo::_EvaluateMapStrategy()
    {
    if (m_ecClass.IsCustomAttributeClass() || m_ecClass.IsStructClass())
        {
        LogClassNotMapped(NativeLogging::LOG_DEBUG, m_ecClass, "ECClass is a custom attribute or ECStruct which is never mapped to a table in ECDb.");
        m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
        return MappingStatus::Success;
        }

    if (ClassMap::IsAnyClass(m_ecClass) || (m_ecClass.GetSchema().IsStandardSchema() && m_ecClass.GetName().CompareTo("InstanceCount") == 0))
        {
        LogClassNotMapped(NativeLogging::LOG_INFO, m_ecClass, "ECClass is a standard class not supported by ECDb.");
        m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
        return MappingStatus::Success;
        }

    BeAssert(m_ecdbMap.GetSchemaImportContext() != nullptr);
    UserECDbMapStrategy* userStrategy = m_ecdbMap.GetSchemaImportContext()->GetUserStrategyP(m_ecClass);
    if (userStrategy == nullptr)
        {
        BeAssert(false);
        return MappingStatus::Error;
        }

    bool baseClassesNotMappedYet;
    if (SUCCESS != DoEvaluateMapStrategy(baseClassesNotMappedYet, *userStrategy))
        return baseClassesNotMappedYet ? MappingStatus::BaseClassesNotMapped : MappingStatus::Error;

    BeAssert(m_resolvedStrategy.IsResolved());

    //DoEvaluateMapStrategy can set the column name. So only set it to a default, if it hasn't been set so far.
    if (m_ecInstanceIdColumnName.empty())
        m_ecInstanceIdColumnName = ECDB_COL_ECInstanceId;

    //! We override m_isMapToVirtualTable if SharedTable was used.
    if (m_isMapToVirtualTable)
        m_isMapToVirtualTable = m_resolvedStrategy.GetStrategy() != ECDbMapStrategy::Strategy::SharedTable;

    return MappingStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMappingInfo::DoEvaluateMapStrategy(bool& baseClassesNotMappedYet, UserECDbMapStrategy& userStrategy)
    {
    bvector<ClassMap const*> baseClassMaps;
    bvector<ClassMap const*> polymorphicSharedTableClassMaps; // SharedTable (AppliesToSubclasses) have the highest priority, but there can be only one
    bvector<ClassMap const*> polymorphicOwnTableClassMaps; // OwnTable (AppliesToSubclasses) have second priority
    bvector<ClassMap const*> polymorphicNotMappedClassMaps; // NotMapped (AppliesToSubclasses) has priority only over NotMapped

    baseClassesNotMappedYet = !GatherBaseClassMaps(baseClassMaps, polymorphicSharedTableClassMaps, polymorphicOwnTableClassMaps, polymorphicNotMappedClassMaps, m_ecClass);
    if (baseClassesNotMappedYet)
        return ERROR;

    if (baseClassMaps.empty())
        {
        BeAssert(polymorphicSharedTableClassMaps.empty() && polymorphicOwnTableClassMaps.empty() && polymorphicNotMappedClassMaps.empty());
        if (SUCCESS != m_resolvedStrategy.Assign(userStrategy))
            {
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid MapStrategy '%s' on ECClass '%s'.", 
                                                                         userStrategy.ToString().c_str(), m_ecClass.GetFullName());
            return ERROR;
            }

        return SUCCESS;
        }

    // ClassMappingRule: No more than one ancestor of a class can use SharedTable-Polymorphic strategy. Mapping fails if this is violated
    if (polymorphicSharedTableClassMaps.size() > 1)
        {
        IssueReporter const& issues = m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter();
        if (issues.IsSeverityEnabled(ECDbIssueSeverity::Error))
            {
            Utf8String baseClasses;
            for (ClassMap const* baseMap : polymorphicSharedTableClassMaps)
                {
                baseClasses.append(baseMap->GetClass().GetFullName());
                baseClasses.append(" ");
                }

            issues.Report(ECDbIssueSeverity::Error, "ECClass '%s' has two or more base ECClasses which use the MapStrategy 'SharedTable (AppliesToSubclasses)'. This is not supported. The base ECClasses are : %s",
                          m_ecClass.GetFullName(), baseClasses.c_str());
            }

        return ERROR;
        }

    ClassMap const* parentClassMap = nullptr;
    if (!polymorphicSharedTableClassMaps.empty())
        parentClassMap = polymorphicSharedTableClassMaps[0];
    else if (!polymorphicOwnTableClassMaps.empty())
        parentClassMap = polymorphicOwnTableClassMaps[0];
    else if (!polymorphicNotMappedClassMaps.empty())
        parentClassMap = polymorphicNotMappedClassMaps[0];
    else
        parentClassMap = baseClassMaps[0];

    ECDbMapStrategy const& parentStrategy = parentClassMap->GetMapStrategy();
    if (!ValidateChildStrategy(parentStrategy, userStrategy))
        return ERROR;

    // ClassMappingRule: If exactly 1 ancestor ECClass is using SharedTable (AppliesToSubclasses), use this
    if (polymorphicSharedTableClassMaps.size() == 1)
        return EvaluateSharedTableMapStrategy(*parentClassMap, parentStrategy, userStrategy);

    // ClassMappingRule: If one or more parent is using OwnClass-polymorphic, use OwnClass-polymorphic mapping
    if (polymorphicOwnTableClassMaps.size() > 0)
        return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::OwnTable, true);

    // ClassMappingRule: If one or more parent is using NotMapped-polymorphic, use NotMapped-polymorphic
    if (polymorphicNotMappedClassMaps.size() > 0)
        return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, true);

    return m_resolvedStrategy.Assign(userStrategy);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingInfo::EvaluateSharedTableMapStrategy(ClassMap const& parentClassMap, ECDbMapStrategy const& parentStrategy, UserECDbMapStrategy const& userStrategy)
    {
    m_parentClassMap = &parentClassMap;
    BeAssert(parentStrategy.GetStrategy() == ECDbMapStrategy::Strategy::SharedTable && parentStrategy.AppliesToSubclasses());

    if (!m_ecInstanceIdColumnName.empty())
        {
        m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                      "For subclasses of an ECClass with MapStrategy SharedTable(AppliesToSubclasses), ECInstanceIdColumn may not be defined in the ClassMap custom attribute. Violating ECClass: %s",
                                                                      m_ecClass.GetFullName());
        return ERROR;
        }


    DbTable const& parentJoinedTable = m_parentClassMap->GetJoinedTable();
    m_tableName = parentJoinedTable.GetName();
    m_ecInstanceIdColumnName.assign(parentJoinedTable.GetFilteredColumnFirst(DbColumn::Kind::ECInstanceId)->GetName());

    UserECDbMapStrategy const* parentUserStrategy = m_ecdbMap.GetSchemaImportContext()->GetUserStrategy(parentClassMap.GetClass());
    if (parentUserStrategy == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const UserECDbMapStrategy::Options userOptions = userStrategy.GetOptions();

    ECDbMapStrategy::Options options = ECDbMapStrategy::Options::None;
    int minimumSharedColumnCount = ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT;
    if (!Enum::Contains(userOptions, UserECDbMapStrategy::Options::DisableSharedColumns) &&
        (Enum::Contains(userOptions, UserECDbMapStrategy::Options::SharedColumns) ||
         Enum::Contains(parentStrategy.GetOptions(), ECDbMapStrategy::Options::SharedColumns) ||
         Enum::Contains(parentUserStrategy->GetOptions(), UserECDbMapStrategy::Options::SharedColumnsForSubclasses)))
        {
        options = ECDbMapStrategy::Options::SharedColumns;

        //we are only using the min shared col count for classes that enable shared columns
        if (Enum::Contains(userOptions, UserECDbMapStrategy::Options::SharedColumns))
            minimumSharedColumnCount = userStrategy.GetMinimumSharedColumnCount();

        if (minimumSharedColumnCount == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
            minimumSharedColumnCount = parentUserStrategy->GetMinimumSharedColumnCount();
        }

    if (Enum::Contains(userOptions, UserECDbMapStrategy::Options::JoinedTablePerDirectSubclass))
        options = options | ECDbMapStrategy::Options::ParentOfJoinedTable;
    else if (Enum::Intersects(parentStrategy.GetOptions(), ECDbMapStrategy::Options::JoinedTable | ECDbMapStrategy::Options::ParentOfJoinedTable))
        {
        for (ECClassCP anotherBaseClass : m_ecClass.GetBaseClasses())
            {
            ClassMap const* anotherBaseClassMap = GetECDbMap().GetClassMap(*anotherBaseClass);
            BeAssert(anotherBaseClassMap != nullptr);
            if (anotherBaseClassMap == &parentClassMap || anotherBaseClassMap->GetMapStrategy().IsNotMapped())
                continue;

            //! Skip interface classes implement by primary class
            if (anotherBaseClassMap->IsMappedToSingleTable() && anotherBaseClassMap->GetPrimaryTable().GetPersistenceType() == PersistenceType::Virtual)
                continue;

            if (&parentClassMap.GetPrimaryTable() != &anotherBaseClassMap->GetPrimaryTable() || &parentClassMap.GetJoinedTable() != &anotherBaseClassMap->GetJoinedTable())
                {
                m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                             "ECClass '%s' has two base ECClasses which don't map to the same tables. "
                                                                             "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'. "
                                                                             "Base ECClass '%s' is mapped to primary table '%s' and joined table '%s'.",
                                                                             m_ecClass.GetFullName(), parentClassMap.GetClass().GetFullName(),
                                                                             parentClassMap.GetPrimaryTable().GetName().c_str(), parentClassMap.GetJoinedTable().GetName().c_str(),
                                                                             anotherBaseClass->GetFullName(), anotherBaseClassMap->GetPrimaryTable().GetName().c_str(),
                                                                             anotherBaseClassMap->GetJoinedTable().GetName().c_str());
                return ERROR;
                }
            }

        options = options | ECDbMapStrategy::Options::JoinedTable;
        if (Enum::Contains(parentUserStrategy->GetOptions(), UserECDbMapStrategy::Options::JoinedTablePerDirectSubclass))
            {
            //Joined tables are named after the class which becomes the root class of classes in the joined table
            if (SUCCESS != ClassMap::DetermineTableName(m_tableName, m_ecClass))
                return ERROR;

            //For classes in the joined table the id column name is determined like this:
            //"<Rootclass name><Rootclass ECInstanceId column name>"
            ClassMap const* rootClassMap = m_parentClassMap->FindSharedTableRootClassMap();
            if (rootClassMap == nullptr)
            {
            BeAssert(false && "There should always be a root class map which defines the shared table strategy");
            return ERROR;
            }

            m_ecInstanceIdColumnName.Sprintf("%s%s", rootClassMap->GetClass().GetName().c_str(), m_ecInstanceIdColumnName.c_str());
            }
        }

    return m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::SharedTable, options, minimumSharedColumnCount, true);

    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassMappingInfo::ValidateChildStrategy(ECDbMapStrategy const& parentStrategy, UserECDbMapStrategy const& childStrategy) const
    {
    if (!parentStrategy.AppliesToSubclasses())
        {
        BeAssert(parentStrategy.AppliesToSubclasses() && "In ClassMapInfo::ValidateChildStrategy parentStrategy should always apply to subclasses");
        return false;
        }

    bool isValid = true;
    Utf8CP detailError = nullptr;
    switch (parentStrategy.GetStrategy())
        {
            case ECDbMapStrategy::Strategy::SharedTable:
                {
                const ECDbMapStrategy::Options parentOptions = parentStrategy.GetOptions();
                const UserECDbMapStrategy::Options childOptions = childStrategy.GetOptions();
                isValid = childStrategy.GetStrategy() == UserECDbMapStrategy::Strategy::None &&
                    !childStrategy.AppliesToSubclasses();

                //if shared columns has already been specified on parent, neither SharedColumnsForSubclasses nor
                //minimum shared col count cannot be specified on child
                if (isValid && Enum::Contains(parentOptions, ECDbMapStrategy::Options::SharedColumns))
                    isValid = !Enum::Contains(childOptions, UserECDbMapStrategy::Options::SharedColumnsForSubclasses) && 
                    childStrategy.GetMinimumSharedColumnCount() == ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT;

                if (isValid)
                    isValid = !Enum::Contains(childOptions, UserECDbMapStrategy::Options::JoinedTablePerDirectSubclass) ||
                             !Enum::Intersects(parentOptions, ECDbMapStrategy::Options::JoinedTable | ECDbMapStrategy::Options::ParentOfJoinedTable);

                if (!isValid)
                    detailError = "For subclasses of a class with MapStrategy SharedTable (AppliesToSubclasses): Strategy must be unset; "
                    "Options must not specify " USERMAPSTRATEGY_OPTIONS_SHAREDCOLUMNSFORSUBCLASSES " and MinimumSharedColumnCount must not be set "
                    "if 'shared columns' were already enabled on a base class; "
                    "Options must not specify " USERMAPSTRATEGY_OPTIONS_JOINEDTABLEPERDIRECTSUBCLASS " "
                    "if it was already specified on a base class.";

                break;
                }

            //in all other cases there must not be any MapStrategy defined in subclasses
            default:
                {
                isValid = childStrategy.GetStrategy() == UserECDbMapStrategy::Strategy::None &&
                    !childStrategy.AppliesToSubclasses() &&
                    childStrategy.GetOptions() == UserECDbMapStrategy::Options::None;

                if (!isValid)
                    detailError = "For subclasses of a class with MapStrategy SharedTable (AppliesToSubclasses), no MapStrategy may be defined.";

                break;
                }
        }

    if (!isValid)
        {
        m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                        "The '%s' ECClass' MapStrategy '%s' does not match the parent's MapStrategy. %s",
                        m_ecClass.GetFullName(), childStrategy.ToString().c_str(), detailError);
        }

    return isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMappingInfo::_InitializeFromSchema ()
    {
    ECDbClassMap customClassMap;
    const bool hasCustomClassMap = ECDbMapCustomAttributeHelper::TryGetClassMap(customClassMap, m_ecClass);
    if (hasCustomClassMap)
        {
        UserECDbMapStrategy const* userStrategy = m_ecdbMap.GetSchemaImportContext()->GetUserStrategy(m_ecClass, &customClassMap);
        if (userStrategy == nullptr || !userStrategy->IsValid())
            return ERROR;

        ECObjectsStatus ecstat = customClassMap.TryGetTableName(m_tableName);
        if (ECObjectsStatus::Success != ecstat)
            return ERROR;

        if ((userStrategy->GetStrategy() == UserECDbMapStrategy::Strategy::ExistingTable ||
             (userStrategy->GetStrategy() == UserECDbMapStrategy::Strategy::SharedTable && !userStrategy->AppliesToSubclasses())))
            {
            if (m_tableName.empty())
                {
                m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                              "TableName must not be empty in ClassMap custom attribute on ECClass %s if MapStrategy is 'SharedTable (AppliesToSubclasses)' or if MapStrategy is 'ExistingTable'.",
                                                                              m_ecClass.GetFullName());
                return ERROR;
                }
            }
        else
            {
            if (!m_tableName.empty())
                {
                m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                              "TableName must only be set in ClassMap custom attribute on ECClass %s if MapStrategy is 'SharedTable (AppliesToSubclasses)' or 'ExistingTable'.",
                                                                              m_ecClass.GetFullName());
                return ERROR;
                }
            }

        ecstat = customClassMap.TryGetECInstanceIdColumn(m_ecInstanceIdColumnName);
        if (ECObjectsStatus::Success != ecstat)
            return ERROR;
        }

    if (SUCCESS != IndexMappingInfo::CreateFromECClass(m_dbIndexes, m_ecdbMap.GetECDb(), m_ecClass, hasCustomClassMap? &customClassMap : nullptr))
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
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                    "Failed to map ECClass %s. The property '%s' specified in the 'ClassHasCurrentTimeStampProperty' custom attribute "
                    "does not exist in the ECClass.", m_ecClass.GetFullName(), v.GetUtf8CP());
            return ERROR;
            }
        PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
        if (primProp == nullptr || primProp->GetType() != PrimitiveType::PRIMITIVETYPE_DateTime)
            {
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                "Failed to map ECClass %s. The property '%s' specified in the 'ClassHasCurrentTimeStampProperty' custom attribute "
                "is not a primitive property of type 'DateTime'.", m_ecClass.GetFullName(), prop->GetName().c_str());
            return ERROR;

            }

        m_classHasCurrentTimeStampProperty = prop;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if all base classes have been mapped.
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassMappingInfo::GatherBaseClassMaps(bvector<ClassMap const*>& baseClassMaps,bvector<ClassMap const*>& tphMaps,
                                       bvector<ClassMap const*>& tpcMaps,bvector<ClassMap const*>& nmhMaps,ECClassCR ecClass) const
    {
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        ClassMap const* baseClassMap = m_ecdbMap.GetClassMap(*baseClass);
        if (baseClassMap == nullptr)
            return false;

        ECDbMapStrategy const& baseMapStrategy = baseClassMap->GetMapStrategy();
        if (!baseMapStrategy.AppliesToSubclasses())
            {
            // ClassMappingRule: non-polymorphic MapStrategies used in base classes have no effect on child classes
            return true;
            }

        DbTable const& baseTable = baseClassMap->GetPrimaryTable();
        switch (baseMapStrategy.GetStrategy())
            {
                case ECDbMapStrategy::Strategy::SharedTable:
                    {
                    bool add = true;
                    for (ClassMap const* classMap : tphMaps)
                        {
                        if (&classMap->GetPrimaryTable() == &baseTable)
                            {
                            add = false;
                            break;
                            }
                        }

                    if (add)
                        tphMaps.push_back(baseClassMap);
                    break;
                    }

                case ECDbMapStrategy::Strategy::OwnTable:
                    tpcMaps.push_back(baseClassMap);
                    break;

                case ECDbMapStrategy::Strategy::NotMapped:
                    nmhMaps.push_back(baseClassMap);
                    break;

                default:
                    BeAssert(false);
                    break;
            }

        baseClassMaps.push_back (baseClassMap);
        }

    return true;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ClassMappingInfo::LogClassNotMapped (NativeLogging::SEVERITY severity, ECClassCR ecClass, Utf8CP explanation)
    {
    Utf8CP classTypeStr = ecClass.GetRelationshipClassCP () != nullptr ? "ECRelationshipClass" : "ECClass";
    LOG.messagev (severity, "Skipped %s '%s' during mapping: %s", classTypeStr, ecClass.GetFullName (), explanation);
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

    auto relClass = GetECClass ().GetRelationshipClassCP ();
    BeAssert (relClass != nullptr);

    ECDbForeignKeyRelationshipMap foreignKeyRelMap;
    const bool hasForeignKeyRelMap = ECDbMapCustomAttributeHelper::TryGetForeignKeyRelationshipMap(foreignKeyRelMap, *relClass);
    ECDbLinkTableRelationshipMap linkTableRelationMap;
    const bool hasLinkTableRelMap = ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(linkTableRelationMap, *relClass);

    if (hasForeignKeyRelMap && hasLinkTableRelMap)
        {
        m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                    "ECRelationshipClass '%s' can only have either the ForeignKeyRelationshipMap or the LinkTableRelationshipMap custom attribute but not both.",
                   GetECClass().GetFullName());
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
                    m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                            "ForeignKeyRelationshipMap custom attribute on ECRelationshipClass '%s' must not have a value for ForeignKeyColumn as there are Key properties defined in the ECRelationshipConstraint on the foreign key end.",
                               GetECClass().GetFullName());
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
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                       "ForeignKeyRelationshipMap custom attribute on ECRelationshipClass '%s' can only define 'Cascade' as OnDeleteAction if the relationship strength is 'Embedding'.",
                       GetECClass().GetFullName());
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
// @bsimethod                                 Affan.Khan                01 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RelationshipMappingInfo::ResolveEndTables(EndTablesOptimizationOptions source, EndTablesOptimizationOptions target)
    {
    ECRelationshipClassCP relationshipClass = GetECClass().GetRelationshipClassCP();
    if (source != EndTablesOptimizationOptions::Skip)
        m_sourceTables = m_ecdbMap.GetTablesFromRelationshipEnd(relationshipClass->GetSource(), source);

    if (target != EndTablesOptimizationOptions::Skip)
        m_targetTables = m_ecdbMap.GetTablesFromRelationshipEnd(relationshipClass->GetTarget(), target);

    return m_sourceTables.empty() || m_targetTables.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
MappingStatus RelationshipMappingInfo::_EvaluateMapStrategy()
    {
    MappingStatus stat = ClassMappingInfo::_EvaluateMapStrategy();
    if (stat != MappingStatus::Success)
        return stat;

    if (m_resolvedStrategy.IsNotMapped())
        return MappingStatus::Success;

    ECRelationshipClassCP relationshipClass = GetECClass().GetRelationshipClassCP();
    ECRelationshipConstraintR source = relationshipClass->GetSource();
    ECRelationshipConstraintR target = relationshipClass->GetTarget();
    const StrengthType strength = relationshipClass->GetStrength();
    const ECRelatedInstanceDirection strengthDirection = relationshipClass->GetStrengthDirection();

    DetermineCardinality(source, target);
    const bool userStrategyIsForeignKeyMapping = m_customMapType == CustomMapType::ForeignKeyOnSource || m_customMapType == CustomMapType::ForeignKeyOnTarget;
    if (m_resolvedStrategy.GetStrategy() == ECDbMapStrategy::Strategy::SharedTable && m_resolvedStrategy.AppliesToSubclasses())
        {
        if (userStrategyIsForeignKeyMapping)
            {
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                "Failed to map ECRelationshipClass %s. Is has a ForeignKeyRelationshipClassMap CA and at the same time is part of a class hierarchy with the 'SharedTable (AppliesToSubclasses)' MapStrategy.",
                GetECClass().GetFullName());

            return MappingStatus::Error;
            }

        if (ResolveEndTables(EndTablesOptimizationOptions::ReferencedEnd, EndTablesOptimizationOptions::ReferencedEnd) == ERROR)
            {
            LogClassNotMapped(NativeLogging::LOG_WARNING, *relationshipClass, "Source or target constraints don't include any concrete classes or its classes are not mapped to tables.");
            m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
            return MappingStatus::Success;
            }

        return MappingStatus::Success;
        }


    if (m_customMapType == CustomMapType::LinkTable ||
        m_cardinality == Cardinality::ManyToMany ||
        GetECClass().GetPropertyCount() > 0)
        {
        if (strength == StrengthType::Embedding)
            {
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                         "Failed to map ECRelationshipClass %s. The mapping rules imply a link table relationship, and link table relationships with strength 'Embedding' is not supported. See API docs for details on the mapping rules.",
                                                                         GetECClass().GetFullName());
            return MappingStatus::Error;
            }

        if (userStrategyIsForeignKeyMapping)
            {
            m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                "Failed to map ECRelationshipClass %s. The mapping rules imply a link table relationship. Therefore it must not have a ForeignKeyRelationshipMap custom attribute. See API docs for details on the mapping rules.",
                GetECClass().GetFullName());
            return MappingStatus::Error;
            }

        if (SUCCESS != ResolveEndTables(EndTablesOptimizationOptions::ReferencedEnd, EndTablesOptimizationOptions::ReferencedEnd))
            {
            LogClassNotMapped(NativeLogging::LOG_WARNING, *relationshipClass, "Source or target constraints don't include any concrete classes or its classes are not mapped to tables.");
            m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
            return MappingStatus::Success;
            }

        BeAssert(m_resolvedStrategy.IsResolved());
        return MappingStatus::Success;
        }

    //FK type relationship mapping
    if (SUCCESS != ResolveEndTables(EndTablesOptimizationOptions::ForeignEnd, EndTablesOptimizationOptions::ForeignEnd))
        {
        LogClassNotMapped(NativeLogging::LOG_WARNING, *relationshipClass, "Source or target constraints don't include any concrete classes or its classes are not mapped to tables.");
        m_resolvedStrategy.Assign(ECDbMapStrategy::Strategy::NotMapped, false);
        return MappingStatus::Success;
        }

    BeAssert(!m_allowDuplicateRelationships && "This can only be true if CustomMapType is LinkTable. That condition was already handled before though.");
    const size_t sourceTableCount = m_sourceTables.size();
    const size_t targetTableCount = m_targetTables.size();
    ECDbMapStrategy::Strategy resolvedStrategy;
    switch (m_cardinality)
        {
            case Cardinality::OneToOne:
                {
                // Don't persist at an end that has more than one table
                if (sourceTableCount > 1 || targetTableCount > 1)
                    {
                    if (userStrategyIsForeignKeyMapping)
                        {
                        Utf8CP constraintStr = nullptr;
                        if (sourceTableCount > 1 && targetTableCount > 1)
                            constraintStr = "source and target constraints are";
                        else if (sourceTableCount > 1)
                            constraintStr = "source constraint is";
                        else
                            constraintStr = "target constraint is";

                        m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                   "Failed to map ECRelationshipClass %s. It implies a link table relationship as the %s mapped to more than one end table. Therefore it must not have a ForeignKeyRelationshipMap custom attribute. See API docs for details on the mapping rules.",
                                   GetECClass().GetFullName(), constraintStr);
                        return MappingStatus::Error;
                        }

                    resolvedStrategy = ECDbMapStrategy::Strategy::OwnTable;
                    break;
                    }

                if (m_customMapType == CustomMapType::ForeignKeyOnSource)
                    {
                    BeAssert(strengthDirection == ECRelatedInstanceDirection::Backward);
                    resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable;
                    }
                else if (m_customMapType == CustomMapType::ForeignKeyOnTarget)
                    {
                    BeAssert(strengthDirection == ECRelatedInstanceDirection::Forward);
                    resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable;
                    }
                else
                    {
                    BeAssert(m_customMapType == CustomMapType::None);
                    if (strengthDirection == ECRelatedInstanceDirection::Backward)
                        resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable;
                    else
                        resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable;
                    }

                break;
                }

            case Cardinality::OneToMany:
                {
                if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Backward)
                    {
                    m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                                 "Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Forward'.",
                                                                                 GetECClass().GetFullName(), relationshipClass->GetSource().GetCardinality().ToString().c_str(), relationshipClass->GetTarget().GetCardinality().ToString().c_str());
                    return MappingStatus::Error;
                    }

                if (sourceTableCount > 1)
                    {
                    m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                        "Failed to map ECRelationshipClass %s. Its foreign key end (Target) references more than one table (Source). This is not supported. Either define the MapStrategy 'SharedTable' on the classes of the referenced constraint or modify the ECRelationshipClass accordingly.",
                        GetECClass().GetFullName());
                    return MappingStatus::Error;
                    }

                resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable;
                break;
                }

            case Cardinality::ManyToOne:
                {
                if (strength == StrengthType::Embedding && strengthDirection == ECRelatedInstanceDirection::Forward)
                    {
                    m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                                 "Failed to map ECRelationshipClass %s. For strength 'Embedding', the cardinality '%s:%s' requires the strength direction to be 'Backward'.",
                                                                                 GetECClass().GetFullName(), relationshipClass->GetSource().GetCardinality().ToString().c_str(), relationshipClass->GetTarget().GetCardinality().ToString().c_str());
                    return MappingStatus::Error;
                    }

                if (targetTableCount > 1)
                    {
                    m_ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                               "Failed to map ECRelationshipClass %s. Its foreign key end (Source) references more than one table (Target). This is not supported. Either define the MapStrategy 'SharedTable' on the classes of the referenced constraint or modify the ECRelationshipClass accordingly.",
                               GetECClass().GetFullName());
                    return MappingStatus::Error;
                    }

                resolvedStrategy = ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable;
                break;
                }

            default:
                BeAssert(false && "ManyToMany case should have been handled already.");
                return MappingStatus::Error;
        }

    if (resolvedStrategy == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable)
        ResolveEndTables(EndTablesOptimizationOptions::Skip, EndTablesOptimizationOptions::ReferencedEnd);
    else if (resolvedStrategy == ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable)
        ResolveEndTables(EndTablesOptimizationOptions::ReferencedEnd, EndTablesOptimizationOptions::Skip);
    else
        ResolveEndTables(EndTablesOptimizationOptions::ReferencedEnd, EndTablesOptimizationOptions::ReferencedEnd);

    return m_resolvedStrategy.Assign(resolvedStrategy, false) == SUCCESS ? MappingStatus::Success : MappingStatus::Error;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipEndColumns const& RelationshipMappingInfo::GetColumnsMapping(ECRelationshipEnd end) const
    {
    if (end == ECRelationshipEnd_Source)
        {
        BeAssert(m_customMapType != CustomMapType::ForeignKeyOnTarget && m_resolvedStrategy.GetStrategy() != ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable);
        return m_sourceColumnsMapping;
        }

    BeAssert(m_customMapType != CustomMapType::ForeignKeyOnSource && m_resolvedStrategy.GetStrategy() != ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable);
    return m_targetColumnsMapping;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                06/2014
//+---------------+---------------+---------------+---------------+---------------+------
void RelationshipMappingInfo::DetermineCardinality(ECRelationshipConstraintCR source, ECRelationshipConstraintCR target)
    {
    const bool sourceIsM = source.GetCardinality().GetUpperLimit() > 1;
    const bool targetIsM = target.GetCardinality().GetUpperLimit() > 1;
    if (sourceIsM && targetIsM)
        m_cardinality = Cardinality::ManyToMany;
    else if (!sourceIsM && targetIsM)
        m_cardinality = Cardinality::OneToMany;
    else if (sourceIsM && !targetIsM)
        m_cardinality = Cardinality::ManyToOne;
    else
        m_cardinality = Cardinality::OneToOne;
    }
  
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::vector<std::pair<Utf8String, Utf8String>> IndexMappingInfo::s_idSpecCustomAttributeNames;

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
                    ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid where clause in ClassMap::DbIndex: %s. Only 'IndexedColumnsAreNotNull' is supported by ECDb.", index.GetWhereClause());
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
        indexName.Sprintf("ix_%s_%s_%s_%s", ecClass.GetSchema().GetNamespacePrefix().c_str(), ecClass.GetName().c_str(),
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
BentleyStatus IndexMappingInfoCache::TryGetIndexInfos(std::vector<IndexMappingInfoPtr> const*& indexInfos, ClassMapCR classMap) const
    {
    //first look in class map info cache
    auto classMapInfoCacheIt = m_schemaImportContext.GetClassMapInfoCache().find(&classMap);
    if (classMapInfoCacheIt != m_schemaImportContext.GetClassMapInfoCache().end())
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
