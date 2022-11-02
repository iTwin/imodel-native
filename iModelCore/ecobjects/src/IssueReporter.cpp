/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// IssueCategory
IssueCategory IssueCategory::BusinessProperties = "BusinessProperties"; // should be used when an issue is related to ECFramework;

// IssueType
IssueType IssueType::ECClass = "ECClass";
IssueType IssueType::ECCustomAttribute = "ECCustomAttribute";
IssueType IssueType::ECInstance = "ECInstance";
IssueType IssueType::ECProperty = "ECProperty";
IssueType IssueType::ECRelationshipClass = "ECRelationshipClass";
IssueType IssueType::ECSchema = "ECSchema";
IssueType IssueType::ECSQL = "ECSQL";
IssueType IssueType::ImportFailure = "ImportFailure";
IssueType IssueType::Units = "Units";
IssueType IssueType::Error = "Error";
IssueType IssueType::InvalidInputData = "InvalidInputData";

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECOBJECTS_EXPORT BentleyStatus IssueReporter::AddListener(IIssueListener const& issueListener)
    {
    BeMutexHolder lock(m_mutex);
    if (m_issueListener != nullptr)
        return ERROR;

    m_issueListener = &issueListener;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECOBJECTS_EXPORT void IssueReporter::RemoveListener()
    {
    BeMutexHolder lock(m_mutex);
    m_issueListener = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECOBJECTS_EXPORT void IssueReporter::_Report(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const
    {
    BeMutexHolder lock(m_mutex);
    if (m_issueListener != nullptr)
        m_issueListener->ReportIssue(severity, category, type, message);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
