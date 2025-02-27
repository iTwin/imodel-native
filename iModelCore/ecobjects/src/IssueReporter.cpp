/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// IssueCategory
IssueCategory IssueCategory::BusinessProperties = IssueCategory("BusinessProperties"); // should be used when an issue is related to ECFramework;

IssueCategory IssueCategory::SchemaSync = IssueCategory("SchemaSync");

// IssueType
IssueType IssueType::ECClass = IssueType("ECClass");
IssueType IssueType::ECCustomAttribute = IssueType("ECCustomAttribute");
IssueType IssueType::ECInstance = IssueType("ECInstance");
IssueType IssueType::ECProperty = IssueType("ECProperty");
IssueType IssueType::ECRelationshipClass = IssueType("ECRelationshipClass");
IssueType IssueType::ECSchema = IssueType("ECSchema");
IssueType IssueType::ECSQL = IssueType("ECSQL");
IssueType IssueType::ImportFailure = IssueType("ImportFailure");
IssueType IssueType::Units = IssueType("Units");
IssueType IssueType::Error = IssueType("Error");
IssueType IssueType::InvalidInputData = IssueType("InvalidInputData");

// ECIssueId
IssueId ECIssueId::EC_0001 = IssueId("EC_0001");
IssueId ECIssueId::EC_0002 = IssueId("EC_0002");
IssueId ECIssueId::EC_0003 = IssueId("EC_0003");
IssueId ECIssueId::EC_0004 = IssueId("EC_0004");
IssueId ECIssueId::EC_0005 = IssueId("EC_0005");
IssueId ECIssueId::EC_0006 = IssueId("EC_0006");
IssueId ECIssueId::EC_0007 = IssueId("EC_0007");
IssueId ECIssueId::EC_0008 = IssueId("EC_0008");
IssueId ECIssueId::EC_0009 = IssueId("EC_0009");
IssueId ECIssueId::EC_0010 = IssueId("EC_0010");
IssueId ECIssueId::EC_0011 = IssueId("EC_0011");
IssueId ECIssueId::EC_0012 = IssueId("EC_0012");
IssueId ECIssueId::EC_0013 = IssueId("EC_0013");
IssueId ECIssueId::EC_0014 = IssueId("EC_0014");
IssueId ECIssueId::EC_0015 = IssueId("EC_0015");
IssueId ECIssueId::EC_0016 = IssueId("EC_0016");
IssueId ECIssueId::EC_0017 = IssueId("EC_0017");
IssueId ECIssueId::EC_0018 = IssueId("EC_0018");
IssueId ECIssueId::EC_0019 = IssueId("EC_0019");
IssueId ECIssueId::EC_0020 = IssueId("EC_0020");
IssueId ECIssueId::EC_0021 = IssueId("EC_0021");
IssueId ECIssueId::EC_0022 = IssueId("EC_0022");
IssueId ECIssueId::EC_0023 = IssueId("EC_0023");
IssueId ECIssueId::EC_0024 = IssueId("EC_0024");
IssueId ECIssueId::EC_0025 = IssueId("EC_0025");
IssueId ECIssueId::EC_0026 = IssueId("EC_0026");
IssueId ECIssueId::EC_0027 = IssueId("EC_0027");
IssueId ECIssueId::EC_0028 = IssueId("EC_0028");
IssueId ECIssueId::EC_0029 = IssueId("EC_0029");
IssueId ECIssueId::EC_0030 = IssueId("EC_0030");
IssueId ECIssueId::EC_0031 = IssueId("EC_0031");
IssueId ECIssueId::EC_0032 = IssueId("EC_0032");
IssueId ECIssueId::EC_0033 = IssueId("EC_0033");
IssueId ECIssueId::EC_0034 = IssueId("EC_0034");
IssueId ECIssueId::EC_0035 = IssueId("EC_0035");
IssueId ECIssueId::EC_0036 = IssueId("EC_0036");
IssueId ECIssueId::EC_0037 = IssueId("EC_0037");
IssueId ECIssueId::EC_0038 = IssueId("EC_0038");
IssueId ECIssueId::EC_0039 = IssueId("EC_0039");
IssueId ECIssueId::EC_0040 = IssueId("EC_0040");
IssueId ECIssueId::EC_0041 = IssueId("EC_0041");
IssueId ECIssueId::EC_0042 = IssueId("EC_0042");
IssueId ECIssueId::EC_0043 = IssueId("EC_0043");
IssueId ECIssueId::EC_0044 = IssueId("EC_0044");
IssueId ECIssueId::EC_0045 = IssueId("EC_0045");
IssueId ECIssueId::EC_0046 = IssueId("EC_0046");
IssueId ECIssueId::EC_0047 = IssueId("EC_0047");
IssueId ECIssueId::EC_0048 = IssueId("EC_0048");
IssueId ECIssueId::EC_0049 = IssueId("EC_0049");
IssueId ECIssueId::EC_0050 = IssueId("EC_0050");
IssueId ECIssueId::EC_0051 = IssueId("EC_0051");
IssueId ECIssueId::EC_0052 = IssueId("EC_0052");
IssueId ECIssueId::EC_0053 = IssueId("EC_0053");
IssueId ECIssueId::EC_0054 = IssueId("EC_0054");
IssueId ECIssueId::EC_0055 = IssueId("EC_0055");
IssueId ECIssueId::EC_0056 = IssueId("EC_0056");
IssueId ECIssueId::EC_0057 = IssueId("EC_0057");
IssueId ECIssueId::EC_0058 = IssueId("EC_0058");
IssueId ECIssueId::EC_0059 = IssueId("EC_0059");
IssueId ECIssueId::EC_0060 = IssueId("EC_0060");
IssueId ECIssueId::EC_0061 = IssueId("EC_0061");

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
ECOBJECTS_EXPORT void IssueReporter::_Report(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message) const
    {
    BeMutexHolder lock(m_mutex);
    if (m_issueListener != nullptr)
        m_issueListener->ReportIssue(severity, category, type, id, message);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
