/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParseTreeFormatter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace connectivity;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree(Utf8StringR parseNodeTreeString, ECDbCR ecdb, Utf8CP ecsql)
    {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    OSQLParser parser;
    Utf8String error;
    std::unique_ptr<OSQLParseNode> parseNode(parser.parseTree(error, ecsql));
    if (parseNode == nullptr)
        {
        if (!error.empty())
            ecdb.GetImpl().Issues().Report(error.c_str());

        return ERROR;
        }

    parseNode->showParseTree(parseNodeTreeString);
    return SUCCESS;
    }

void GenerateExpTree(Json::Value& expTree, Exp const& exp);

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree(Json::Value& expTree, Utf8StringR ecsqlFromExpTree, ECDbCR ecdb, Utf8CP ecsql)
    {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql, ScopedIssueReporter(ecdb, true));
    if (exp == nullptr)
        return ERROR;

    ecsqlFromExpTree = exp->ToECSql();
    GenerateExpTree(expTree, *exp);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void GenerateExpTree(Json::Value& expTree, Exp const& exp)
    {
    expTree = Json::Value(Json::objectValue);
    expTree["Exp"] = Json::Value(exp.ToString());

    if (exp.GetChildrenCount() == 0)
        return;

    Json::Value& children = expTree["Children"] = Json::Value(Json::arrayValue);
    for (Exp const* childExp : exp.GetChildren())
        {
        BeAssert(childExp != nullptr);
        Json::Value child;
        GenerateExpTree(child, *childExp);
        children.append(child);
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
