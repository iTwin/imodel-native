/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapStrategy.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP MapStrategyExtendedInfo::ToString(MapStrategy strategy)
    {
    switch (strategy)
        {
            case MapStrategy::ExistingTable:
                return "ExistingTable";
            case MapStrategy::ForeignKeyRelationshipInSourceTable:
                return "ForeignKeyRelationshipInSourceTable";
            case MapStrategy::ForeignKeyRelationshipInTargetTable:
                return "ForeignKeyRelationshipInTargetTable";
            case MapStrategy::NotMapped:
                return "NotMapped";
            case MapStrategy::OwnTable:
                return "OwnTable";
            case MapStrategy::SharedTable:
                return "SharedTable";
            case MapStrategy::TablePerHierarchy:
                return "TablePerHierarchy";
            default:
                BeAssert(false && "Unhandled value for ECDbMapStrategy in ToString");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::Initialize(ShareColumns const& shareColumnsCA, MapStrategyExtendedInfo const* baseMapStrategy, ShareColumns const* baseClassShareColumnsCA, bool hasJoinedTablePerDirectSubclassOption, ECClassCR ecClass, IssueReporter const& issues)
    {
    if (SUCCESS != DetermineSharedColumnsInfo(shareColumnsCA, baseMapStrategy, baseClassShareColumnsCA, ecClass, issues))
        return ERROR;

    if (SUCCESS != DetermineJoinedTableInfo(hasJoinedTablePerDirectSubclassOption, baseMapStrategy, ecClass, issues))
        return ERROR;

    m_isValid = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::DetermineSharedColumnsInfo(ShareColumns const& shareColumnsCA, MapStrategyExtendedInfo const* baseMapStrategy, ShareColumns const* baseClassShareColumnsCA, ECClassCR ecClass, IssueReporter const& issues)
    {
    //first check whether column sharing is inherited from base class.
    if (baseMapStrategy != nullptr && baseMapStrategy->GetTphInfo().UseSharedColumns())
        {
        if (shareColumnsCA.IsValid())
            {
            issues.Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. It defines the ShareColumns custom attribute, although one of its base classes has defined it already.",
                          ecClass.GetFullName());

            return ERROR;
            }

        m_useSharedColumns = true;
        m_sharedColumnCount = baseMapStrategy->GetTphInfo().GetSharedColumnCount();
        m_excessColumnName.assign(baseMapStrategy->GetTphInfo().GetExcessColumnName());
        return SUCCESS;
        }

    //then check whether the base class CA indicates column sharing (if AppliesToSubclassesOnly the base class map strategy doesn't have column sharing yet)
    if (baseClassShareColumnsCA != nullptr && baseClassShareColumnsCA->IsValid())
        {
        if (shareColumnsCA.IsValid())
            {
            issues.Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. It defines the ShareColumns custom attribute, although one of its base classes has defined it already.",
                            ecClass.GetFullName());

            return ERROR;
            }

        m_useSharedColumns = true;
        if (ECObjectsStatus::Success != baseClassShareColumnsCA->TryGetSharedColumnCount(m_sharedColumnCount))
            return ERROR;

        if (ECObjectsStatus::Success != baseClassShareColumnsCA->TryGetExcessColumnName(m_excessColumnName))
            return ERROR;

        return SUCCESS;
        }

    //now see whether it is enabled for this class
    if (shareColumnsCA.IsValid()) //CA exists on this class
        {
        //if it says that it should only apply to subclasses set column sharing to false for this class
        bool applyToSubclassesOnly = false;
        if (ECObjectsStatus::Success != shareColumnsCA.TryGetApplyToSubclassesOnly(applyToSubclassesOnly))
            return ERROR;

        m_useSharedColumns = !applyToSubclassesOnly;

        if (m_useSharedColumns)
            {
            if (ECObjectsStatus::Success != shareColumnsCA.TryGetSharedColumnCount(m_sharedColumnCount))
                return ERROR;

            if (ECObjectsStatus::Success != shareColumnsCA.TryGetExcessColumnName(m_excessColumnName))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, MapStrategyExtendedInfo const* baseMapStrategy, ECClassCR ecClass, IssueReporter const& issues)
    {
    if (baseMapStrategy != nullptr && baseMapStrategy->GetTphInfo().GetJoinedTableInfo() != JoinedTableInfo::None)
        {
        if (hasJoinedTablePerDirectSubclassOption)
            {
            issues.Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. It defines the JoinedTablePerDirectSubclass custom attribute, although one of its base classes has defined it already.",
                          ecClass.GetFullName());

            return ERROR;
            }

        m_joinedTableInfo = JoinedTableInfo::JoinedTable;
        return SUCCESS;
        }

    if (hasJoinedTablePerDirectSubclassOption)
        m_joinedTableInfo = JoinedTableInfo::ParentOfJoinedTable;
    else
        m_joinedTableInfo = JoinedTableInfo::None;

    return SUCCESS;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMappingCACache::Initialize(ECN::ECClassCR ecClass)
    {
    if (ECDbMapCustomAttributeHelper::TryGetClassMap(m_classMapCA, ecClass))
        {
        Utf8String mapStrategyStr;
        if (ECObjectsStatus::Success != m_classMapCA.TryGetMapStrategy(mapStrategyStr))
            return ERROR;

        if (mapStrategyStr.empty())
            {
            m_hasMapStrategy = false;
            //only set this to default in case using code calls GetStrategy without
            //checking for m_hasMapStrategy
            m_mapStrategy = MapStrategyExtendedInfo::DEFAULT; 
            }
        else
            {
            if (SUCCESS != TryParse(m_mapStrategy, mapStrategyStr.c_str(), ecClass))
                return ERROR;

            m_hasMapStrategy = true;
            }
        }

    ECDbMapCustomAttributeHelper::TryGetShareColumns(m_shareColumnsCA, ecClass);

    m_hasJoinedTablePerDirectSubclassOption = ECDbMapCustomAttributeHelper::HasJoinedTablePerDirectSubclass(ecClass);
    return SUCCESS;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ClassMappingCACache::TryParse(MapStrategy& mapStrategy, Utf8CP mapStrategyStr, ECClassCR ecClass)
    {
    if (BeStringUtilities::StricmpAscii(mapStrategyStr, "OwnTable") == 0)
        mapStrategy = MapStrategy::OwnTable;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "NotMapped") == 0)
        mapStrategy = MapStrategy::NotMapped;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "TablePerHierarchy") == 0)
        mapStrategy = MapStrategy::TablePerHierarchy;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "SharedTable") == 0)
        mapStrategy = MapStrategy::SharedTable;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "ExistingTable") == 0)
        mapStrategy = MapStrategy::ExistingTable;
    else
        {
        LOG.errorv("ECClass '%s' has a ClassMap custom attribute with an invalid value for MapStrategy: %s.", ecClass.GetFullName(), mapStrategyStr);
        return ERROR;
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


