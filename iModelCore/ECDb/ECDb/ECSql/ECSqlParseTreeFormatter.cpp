/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParseTreeFormatter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree (Utf8StringR parseNodeTree, ECDbCR ecdb, Utf8CP ecsql)
    {
    if (Utf8String::IsNullOrEmpty (ecsql))
        return ERROR;

    RefCountedPtr<com::sun::star::lang::XMultiServiceFactory> serviceFactory = com::sun::star::lang::XMultiServiceFactory::CreateInstance();
    connectivity::OSQLParser aParser(serviceFactory);
    Utf8String error;
    connectivity::OSQLParseNode* n = aParser.parseTree(error, ecsql);
    if (n == nullptr)
        {
        if (!error.empty())
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, error.c_str());

        return ERROR;
        }

    n->showParseTree(parseNodeTree);
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

    std::unique_ptr<Exp> exp = nullptr;
    ECSqlParser parser;
    if (SUCCESS != parser.Parse(exp, ecdb, ecsql, IClassMap::View::DomainClass))
        return ERROR;

    if (exp != nullptr)
        {
        expTreeToECSql = exp->ToECSql();
        GenerateExpTree(expTree, *exp, 0);
        return SUCCESS;
        }

    return ERROR;
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
