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
            m_mapStrategy = ECDbMapStrategyHelper::DEFAULT;
        else
            {
            if (SUCCESS != TryParse(m_mapStrategy, mapStrategyStr.c_str(), ecClass))
                return ERROR;
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
BentleyStatus ClassMappingCACache::TryParse(ECDbMapStrategy& mapStrategy, Utf8CP mapStrategyStr, ECClassCR ecClass)
    {
    if (BeStringUtilities::StricmpAscii(mapStrategyStr, "OwnTable") == 0)
        mapStrategy = ECDbMapStrategy::OwnTable;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "NotMapped") == 0)
        mapStrategy = ECDbMapStrategy::NotMapped;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "TablePerHierarchy") == 0)
        mapStrategy = ECDbMapStrategy::TablePerHierarchy;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "SharedTable") == 0)
        mapStrategy = ECDbMapStrategy::SharedTable;
    else if (BeStringUtilities::StricmpAscii(mapStrategyStr, "ExistingTable") == 0)
        mapStrategy = ECDbMapStrategy::ExistingTable;
    else
        {
        LOG.errorv("ECClass '%s' has a ClassMap custom attribute with an invalid value for MapStrategy: %s.", ecClass.GetFullName(), mapStrategyStr);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::Initialize(ShareColumns const& shareColumnsCA, ShareColumns const* baseClassShareColumnsCA, bool hasJoinedTablePerDirectSubclassOption, JoinedTableInfo const* baseClassJoinedTableInfo, ECClassCR ecClass, IssueReporter const& issues)
    {
    if (SUCCESS != DetermineSharedColumnsInfo(shareColumnsCA, baseClassShareColumnsCA, ecClass, issues))
        return ERROR;

    return DetermineJoinedTableInfo(hasJoinedTablePerDirectSubclassOption, baseClassJoinedTableInfo, ecClass, issues);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::DetermineSharedColumnsInfo(ShareColumns const& shareColumnsCA, ShareColumns const* baseClassShareColumnsCA, ECClassCR ecClass, IssueReporter const& issues)
    {
    //first check whether column sharing is inherited from base class.
    m_isSharedColumns = baseClassShareColumnsCA != nullptr && baseClassShareColumnsCA->IsValid();
    if (m_isSharedColumns)
        {
        if (shareColumnsCA.IsValid())
            {
            issues.Report(ECDbIssueSeverity::Error, "Failed to map ECClass %s. It defines the ShareColumns custom attribute, although one of its base classes has defined it already.",
                            ecClass.GetFullName());

            return ERROR;
            }

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

        m_isSharedColumns = !applyToSubclassesOnly;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle              08/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, JoinedTableInfo const* baseClassJoinedTableInfo, ECClassCR ecClass, IssueReporter const& issues)
    {
    if (baseClassJoinedTableInfo != nullptr && *baseClassJoinedTableInfo != JoinedTableInfo::None)
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
// @bsimethod                                 Krischan.Eberle              06/2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECDbMapStrategyHelper::ToString(ECDbMapStrategy strategy)
    {
    switch (strategy)
        {
            case ECDbMapStrategy::ExistingTable:
                return "ExistingTable";
            case ECDbMapStrategy::ForeignKeyRelationshipInSourceTable:
                return "ForeignKeyRelationshipInSourceTable";
            case ECDbMapStrategy::ForeignKeyRelationshipInTargetTable:
                return "ForeignKeyRelationshipInTargetTable";
            case ECDbMapStrategy::NotMapped:
                return "NotMapped";
            case ECDbMapStrategy::OwnTable:
                return "OwnTable";
            case ECDbMapStrategy::SharedTable:
                return "SharedTable";
            case ECDbMapStrategy::TablePerHierarchy:
                return "TablePerHierarchy";
            default:
                BeAssert(false && "Unhandled value for ECDbMapStrategy in ToString");
                return nullptr;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE


