/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------
// @bsimethod
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
            case MapStrategy::TablePerHierarchy:
                return "TablePerHierarchy";
            default:
                BeAssert(false && "Unhandled value for ECDbMapStrategy in ToString");
                return nullptr;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::Initialize(ShareColumnsCustomAttribute const& shareColumnsCA, MapStrategyExtendedInfo const* baseMapStrategy, bool hasJoinedTablePerDirectSubclassOption, ECClassCR ecClass, IssueDataSource const& issues)
    {
    if (SUCCESS != DetermineSharedColumnsInfo(shareColumnsCA, baseMapStrategy, ecClass, issues))
        return ERROR;

    if (SUCCESS != DetermineJoinedTableInfo(hasJoinedTablePerDirectSubclassOption, baseMapStrategy, ecClass, issues))
        return ERROR;

    if (m_joinedTableInfo == JoinedTableInfo::ParentOfJoinedTable && m_shareColumnsMode == ShareColumnsMode::Yes)
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0238,
            "Failed to map ECClass %s. It defines the JoinedTablePerDirectSubclass custom attribute, although it or its base class already enabled column sharing.", ecClass.GetFullName());
        return ERROR;
        }

    m_isValid = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::DetermineSharedColumnsInfo(ShareColumnsCustomAttribute const& shareColumnsCA, MapStrategyExtendedInfo const* baseMapStrategy, ECClassCR ecClass, IssueDataSource const& issues)
    {
    m_shareColumnsMode = ShareColumnsMode::No; //default
    //first check whether column sharing is inherited from base class.
    if (baseMapStrategy != nullptr && baseMapStrategy->GetTphInfo().GetShareColumnsMode() != ShareColumnsMode::No)
        {
        if (shareColumnsCA.IsValid())
            {
            issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0239,
                "Failed to map ECClass %s. It defines the ShareColumns custom attribute, although one of its base classes has defined it already.", ecClass.GetFullName());

            return ERROR;
            }

        m_shareColumnsMode = ShareColumnsMode::Yes;
        m_maxSharedColumnsBeforeOverflow = baseMapStrategy->GetTphInfo().GetMaxSharedColumnsBeforeOverflow();
        return SUCCESS;
        }


    //now see whether it is enabled for this class
    if (shareColumnsCA.IsValid()) //CA exists on this class
        {
        Nullable<bool> applyToSubclassesOnly;
        if (SUCCESS != shareColumnsCA.TryGetApplyToSubclassesOnly(applyToSubclassesOnly))
            return ERROR;

        //default (if not set in CA) is false
        if (!applyToSubclassesOnly.IsNull() && applyToSubclassesOnly.Value())
            m_shareColumnsMode = ShareColumnsMode::ApplyToSubclassesOnly;
        else
            m_shareColumnsMode = ShareColumnsMode::Yes;

        if (SUCCESS != shareColumnsCA.TryGetMaxSharedColumnsBeforeOverflow(m_maxSharedColumnsBeforeOverflow))
            {
            issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0240,
                "Failed to map ECClass %s. It has the ShareColumns custom attribute with an invalid value for 'MaxSharedColumnsBeforeOverflow'. Either provide a non-negative value or omit the property.", ecClass.GetFullName());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TablePerHierarchyInfo::DetermineJoinedTableInfo(bool hasJoinedTablePerDirectSubclassOption, MapStrategyExtendedInfo const* baseMapStrategy, ECClassCR ecClass, IssueDataSource const& issues)
    {
    if (baseMapStrategy != nullptr && baseMapStrategy->GetTphInfo().GetJoinedTableInfo() != JoinedTableInfo::None)
        {
        if (hasJoinedTablePerDirectSubclassOption)
            {
            issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0241,
                "Failed to map ECClass %s. It defines the JoinedTablePerDirectSubclass custom attribute, although one of its base classes has defined it already.", ecClass.GetFullName());

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

END_BENTLEY_SQLITE_EC_NAMESPACE


