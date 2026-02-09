/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace connectivity;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ECSqlToJson(BeJsValue out, ECDbCR ecdb, Utf8CP ecsql) {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return ERROR;

    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql, ecdb.GetImpl().Issues());
    if (exp == nullptr)
        return ERROR;

    out.SetEmptyObject();
    Exp::JsonFormat fmt;
    exp->ToJson(out, fmt);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::NormalizeECSql(Utf8StringR out, ECDbCR ecdb, Utf8CP ecsql) {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
        return ERROR;

    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql, ecdb.GetImpl().Issues());
    if (exp == nullptr)
        return ERROR;

    out = exp->ToECSql();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(Utf8StringR parseNodeTreeString, ECDbCR ecdb, Utf8CP ecsql) {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    OSQLParser parser;
    Utf8String error;
    auto parseNode = parser.parseTree(error, ecsql);
    if (parseNode == nullptr) {
        if (!error.empty())
            ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0497, error.c_str());

        return ERROR;
    }

    parseNode->showParseTree(parseNodeTreeString);
    return SUCCESS;
}

void GenerateExpTree(Json::Value& expTree, Exp const& exp);

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree(Json::Value& expTree, Utf8StringR ecsqlFromExpTree, ECDbCR ecdb, Utf8CP ecsql) {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql, ecdb.GetImpl().Issues());
    if (exp == nullptr)
        return ERROR;

    ecsqlFromExpTree = exp->ToECSql();
    GenerateExpTree(expTree, *exp);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void GenerateExpTree(Json::Value& expTree, Exp const& exp) {
    expTree = Json::Value(Json::objectValue);
    expTree["Exp"] = Json::Value(exp.ToString());

    if (exp.GetChildrenCount() == 0)
        return;

    Json::Value& children = expTree["Children"] = Json::Value(Json::arrayValue);
    for (Exp const* childExp : exp.GetChildren()) {
        BeAssert(childExp != nullptr);
        Json::Value child;
        GenerateExpTree(child, *childExp);
        children.append(child);
    }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
