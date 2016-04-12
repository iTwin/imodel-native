/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParseTreeFormatter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace connectivity;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree (Utf8StringR parseNodeTreeString, ECDbCR ecdb, Utf8CP ecsql)
    {
    if (Utf8String::IsNullOrEmpty (ecsql))
        return ERROR;

    OSQLParser parser(com::sun::star::lang::XMultiServiceFactory::CreateInstance());
    Utf8String error;
    std::unique_ptr<OSQLParseNode> parseNode(parser.parseTree(error, ecsql));
    if (parseNode == nullptr)
        {
        if (!error.empty())
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, error.c_str());

        return ERROR;
        }

    parseNode->showParseTree(parseNodeTreeString);
    return SUCCESS;
    }

void GenerateExpTree (Utf8StringR expTree, Exp const& exp, int indentLevel);

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree(Utf8StringR expTree, Utf8StringR expTreeToECSql, ECDbCR ecdb, Utf8CP ecsql)
    {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return ERROR;

    ECSqlParser parser;
    std::unique_ptr<Exp> exp = parser.Parse(ecdb, ecsql);
    if (exp == nullptr)
        return ERROR;

    expTreeToECSql = exp->ToECSql();
    GenerateExpTree(expTree, *exp, 0);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void GenerateExpTree (Utf8StringR expTree, Exp const& exp, int indentLevel)
    {
    for (int i = 0; i < indentLevel; i++)
        expTree.append ("   ");
    
    expTree.append (exp.ToString ().c_str ()).append ("\r\n");

    indentLevel++;
    for (Exp const* child : exp.GetChildren ())
        {
        BeAssert(child != nullptr);
        GenerateExpTree (expTree, *child, indentLevel);
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
