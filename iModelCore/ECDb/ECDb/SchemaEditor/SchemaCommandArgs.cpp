/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/SchemaEditor.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

SchemaCommandResult SchemaCommandResult::Failure(Status status, Utf8CP code, Utf8CP arg, Utf8StringCR message) {
    SchemaCommandResult result;
    result.m_status = status == Status::OK ? Status::ERROR : status;
    result.AddIssue({code == nullptr ? "" : code, arg == nullptr ? "" : arg, message});
    return result;
}

void SchemaCommandResult::AddIssue(SchemaCommandIssue issue) {
    if (IsOk())
        m_status = Status::ERROR;
    m_issues.push_back(std::move(issue));
}

void SchemaCommandResult::SetFailedCommand(Utf8StringCR command, size_t index) {
    m_failedCommand = command;
    m_failedCommandIndex = index;
}

void SchemaCommandResult::SetBlockingInstances(ECInstanceKeyMultiMap instances) {
    m_blockingInstances = std::move(instances);
}

void SchemaCommandResult::ToJson(BeJsValue out) const {
    Utf8CP status = "ERROR";
    switch (m_status) {
    case Status::OK:
        status = "OK";
        break;
    case Status::ERROR:
        status = "ERROR";
        break;
    case Status::ERROR_INVALID_ARGS:
        status = "ERROR_INVALID_ARGS";
        break;
    case Status::ERROR_VALIDATION:
        status = "ERROR_VALIDATION";
        break;
    case Status::ERROR_NOT_FOUND:
        status = "ERROR_NOT_FOUND";
        break;
    case Status::ERROR_DATA_TRANSFORM_REQUIRED:
        status = "ERROR_DATA_TRANSFORM_REQUIRED";
        break;
    case Status::ERROR_DATA_DELETION_REQUIRED:
        status = "ERROR_DATA_DELETION_REQUIRED";
        break;
    case Status::ERROR_READONLY:
        status = "ERROR_READONLY";
        break;
    case Status::ERROR_SESSION:
        status = "ERROR_SESSION";
        break;
    }

    out.SetEmptyObject();
    out["status"] = status;
    auto issues = out["issues"];
    issues.SetEmptyArray();
    for (auto const& issue : m_issues) {
        auto json = issues.appendObject();
        json["code"] = issue.code;
        json["arg"] = issue.arg;
        json["message"] = issue.message;
    }

    auto instances = out["blockingInstances"];
    instances.SetEmptyArray();
    for (auto const& instance : m_blockingInstances) {
        auto json = instances.appendObject();
        json["classId"] = instance.first.ToHexStr();
        json["instanceId"] = instance.second.ToHexStr();
    }

    if (!m_failedCommand.empty() || m_failedCommandIndex != SIZE_MAX)
        out["failedCommand"] = m_failedCommand;
    if (m_failedCommandIndex != SIZE_MAX)
        out["failedCommandIndex"] = static_cast<uint64_t>(m_failedCommandIndex);
}

END_BENTLEY_SQLITE_EC_NAMESPACE
