/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParseTreeFormatter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
bool ECSqlParseTreeFormatter::ParseAndFormatECSqlParseNodeTree (Utf8StringR parseNodeTree, Utf8StringR error, Utf8CP ecsql)
    {
    if (Utf8String::IsNullOrEmpty (ecsql))
        {
        error = "ECSQL string was nullptr or empty.";
        return false;
        }

    RefCountedPtr<com::sun::star::lang::XMultiServiceFactory> serviceFactory = com::sun::star::lang::XMultiServiceFactory::CreateInstance();
    connectivity::OSQLParser aParser(serviceFactory);
    connectivity::OSQLParseNode* n = aParser.parseTree(error, ecsql);
    if (n != nullptr)
        n->showParseTree(parseNodeTree);

    return !(n == nullptr ||  !error.empty());
    }

void GenerateExpTree (Utf8StringR expTree, Exp const* exp, int indentLevel);

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
bool ECSqlParseTreeFormatter::ParseAndFormatECSqlExpTree (Utf8StringR expTree, Utf8StringR expTreeToECSql, Utf8StringR error, Utf8CP ecsql, ECDbR db)
    {
    if (Utf8String::IsNullOrEmpty (ecsql))
        {
        error = "ECSQL string was nullptr or empty.";
        return false;
        }

    std::shared_ptr<Exp> exp;
    std::vector<Utf8String> warnings;
    ECSqlStatusContext status;
    ECSqlParser::Parse (exp, status, db, ecsql, IClassMap::View::DomainClass);
    if (!status.IsSuccess ())
        {
        error = status.ToString ();
        return false;
        }

    if (exp != nullptr)
        {
        expTreeToECSql = exp->ToECSql();
        GenerateExpTree (expTree, exp.get (), 0);
        }

    return !(exp == nullptr ||  !error.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void GenerateExpTree (Utf8StringR expTree, Exp const* exp, int indentLevel)
    {
    for (int i = 0; i < indentLevel; i++)
        expTree.append ("   ");
    
    expTree.append (exp->ToString ().c_str ()).append ("\r\n");

    indentLevel++;
    for (auto child : exp->GetChildren ())
        {
        GenerateExpTree (expTree, child, indentLevel);
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
