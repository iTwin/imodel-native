/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParser.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlParser.h"
#include "Parser/SqlNode.h"
#include "Parser/SqlParse.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace std;
using namespace connectivity;

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlParser::Parse (ECSqlParseTreePtr& ecsqlParseTree, ECSqlStatusContext& statusContext, ECDbCR ecdb, Utf8CP ecsql, IClassMap::View classView)
    {
    BeAssert (!Utf8String::IsNullOrEmpty (ecsql));

    ECSqlParseContext parseContext (ecdb, classView, statusContext);
    auto expr = Parse (ecsql, parseContext);
    if (expr.get())
        ecsqlParseTree.reset(expr.release());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<Exp> ECSqlParser::Parse (Utf8CP ecsql, ECSqlParseContext& parseContext)
    {
    RefCountedPtr<com::sun::star::lang::XMultiServiceFactory> serviceFactory = 
        com::sun::star::lang::XMultiServiceFactory::CreateInstance();
    //Parse statement
    Utf8String error;
    OSQLParser ecsqlParser (serviceFactory);
    auto ecsqlParseTree = ecsqlParser.parseTree (error, ecsql);
    if (ecsqlParseTree == nullptr || !error.empty())
        {
        parseContext.SetError (ECSqlStatus::InvalidECSql, error.c_str ());
        return nullptr;
        }

    if (!ecsqlParseTree->isRule())
        {
        BeAssert (false && "ECSQL grammar has changed, but parser wasn't adopted.");
        parseContext.SetError (ECSqlStatus::ProgrammerError, "ECSQL grammar has changed, but parser wasn't adopted.");
        return nullptr;
        }

    unique_ptr<Exp> rootExp = nullptr;
    switch (ecsqlParseTree->getKnownRuleID())
        {
        case OSQLParseNode::insert_statement:
            rootExp = parse_insert_statement(parseContext, ecsqlParseTree);
            break;

        case OSQLParseNode::update_statement_searched:
            rootExp = parse_update_statement_searched(parseContext, ecsqlParseTree);
            break;

        case OSQLParseNode::delete_statement_searched:
            rootExp = parse_delete_statement_searched(parseContext, ecsqlParseTree);
            break;

        case OSQLParseNode::select_statement:
            rootExp = parse_select_statement(parseContext, ecsqlParseTree);
            break;

        case OSQLParseNode::manipulative_statement:
            parseContext.SetError (ECSqlStatus::InvalidECSql, "Manipulative statements are not supported."); 
            return nullptr;

        default:
            BeAssert (false && "Not a valid statement");
            parseContext.SetError (ECSqlStatus::ProgrammerError, "Not a valid statement");
            return nullptr;
        };

    if (rootExp == nullptr)
        return nullptr;

    //resolve types and references now that first pass parsing is done and all nodes are available
    const ECSqlStatus stat = parseContext.FinalizeParsing(*rootExp);
    if (stat == ECSqlStatus::Success)
        return rootExp;

    return nullptr;
    }

//****************** Parsing SELECT statement ***********************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<SingleSelectStatementExp> ECSqlParser::parse_single_select_statement (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!ctx.IsSuccess())
        return nullptr;
    
    //WIP_ECSQL: change all following to unique_ptr this will take care of memory leak in case failure
    //data source must be resolved before anything else. We do not want to make two passes over tree


    auto opt_all_distinct         =  parse_opt_all_distinct    (ctx, parseNode->getChild(1));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto selection                =  parse_selection           (ctx, parseNode->getChild(2));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto table_exp                =  parseNode->getChild(3);
    if (!ctx.IsSuccess ())
        return nullptr;
    
    auto from_clause              = parse_from_clause          (ctx, table_exp->getChild(0));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto opt_where_clause         = parse_opt_where_clause     (ctx, table_exp->getChild(1));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto opt_group_by_clause      = parse_group_by_clause      (ctx, table_exp->getChild(2));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto opt_having_clause        = parse_having_clause        (ctx, table_exp->getChild(3));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto opt_order_by_clause      = parse_order_by_clause      (ctx, table_exp->getChild(5));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto limit_offset_clause      = parse_limit_offset_clause  (ctx, table_exp->getChild(6));
    if (!ctx.IsSuccess ())
        return nullptr;

    if (!ctx.IsSuccess())
        return nullptr;

    if (selection != nullptr && from_clause != nullptr)
        {
        return unique_ptr<SingleSelectStatementExp>(
                        new SingleSelectStatementExp (
                            opt_all_distinct, 
                            move (selection), 
                            move (from_clause),
                            move (opt_where_clause),
                            move (opt_order_by_clause), 
                            move (opt_group_by_clause), 
                            move (opt_having_clause),
                            move (limit_offset_clause)));
        }

    BeAssert (false && "Wrong Grammar. selection and from_clause must be provided");
    ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong Grammar. selection and from_clause must be provided");
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<SelectClauseExp> ECSqlParser::parse_selection (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!ctx.IsSuccess())
        return nullptr;

    if (SQL_ISRULE(parseNode, selection))
        {       
        auto n = parseNode->getChild(0);
        if (Exp::IsAsteriskToken (n->getTokenValue().c_str ()))
            {
            auto propertyNameExp = unique_ptr<PropertyNameExp> (new PropertyNameExp(Exp::ASTERISK_TOKEN));
            auto selection = new SelectClauseExp();
            selection->AddProperty(std::unique_ptr<DerivedPropertyExp> (new DerivedPropertyExp(move (propertyNameExp), nullptr)));
            return unique_ptr<SelectClauseExp>(selection);
            }
        }

    if (!SQL_ISRULE(parseNode, scalar_exp_commalist))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto selection = unique_ptr<SelectClauseExp> (new SelectClauseExp());

    for (size_t n = 0; n < parseNode->count(); n++)
        {
        auto derivedProperty = parse_derived_column (ctx, parseNode->getChild (n));
        if (derivedProperty.get() != nullptr)
            selection->AddProperty (move (derivedProperty));
        else
            break; //Error
        }

    if (ctx.IsSuccess())
        return selection;        
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<DerivedPropertyExp> ECSqlParser::parse_derived_column (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, derived_column ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    auto first  = parseNode->getChild(0);
    auto opt_as_clause = parseNode->getChild(1);   
    auto value = parse_value_exp (ctx, first);
    if (value == nullptr)
        return nullptr;

    Utf8String columnAlias;
    if (opt_as_clause->count() > 0 )
        columnAlias = opt_as_clause->getChild(1)->getTokenValue();
    else
        columnAlias = opt_as_clause->getTokenValue();
    if (ctx.IsSuccess())
        return unique_ptr<DerivedPropertyExp>(new DerivedPropertyExp (move (value), columnAlias.c_str ())); 

    return nullptr;
    }

//****************** Parsing INSERT statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<InsertStatementExp> ECSqlParser::parse_insert_statement (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!ctx.IsSuccess())
        return nullptr;

    //insert does not support polymorphic classes. Passing false therefore.
    auto tableNodeExp = parse_table_node (ctx, parseNode->getChild(2), false);
    if (!ctx.IsSuccess ())
        return nullptr;

    auto insertPropertyNameListExp = parse_opt_column_ref_commalist (ctx, parseNode->getChild(3));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto valuesOrQuerySpecExp = parse_values_or_query_spec (ctx, parseNode->getChild(4));
    if (!ctx.IsSuccess ())
        return nullptr;

    return unique_ptr<InsertStatementExp>(new InsertStatementExp (move (tableNodeExp), move (insertPropertyNameListExp), move (valuesOrQuerySpecExp)));
    }

//****************** Parsing UPDATE statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<UpdateStatementExp> ECSqlParser::parse_update_statement_searched (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!ctx.IsSuccess ())
        return nullptr;

    //rule: update_statement_searched: SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause
    auto classRefExp = parse_table_ref (ctx, parseNode->getChild (1));
    if (!ctx.IsSuccess ())
        return nullptr;

    if (classRefExp->GetType () != Exp::Type::ClassName)
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "ECSQL UPDATE statements only support ECClass references as target. Subqueries or join clauses are not supported.");
        return nullptr;
        }

    auto assignmentListExp = parse_assignment_commalist (ctx, parseNode->getChild (3));
    if (!ctx.IsSuccess ())
        return nullptr;

    auto opt_where_clause = parse_opt_where_clause (ctx, parseNode->getChild (4));
    if (!ctx.IsSuccess ())
        return nullptr;

    return unique_ptr<UpdateStatementExp> (new UpdateStatementExp (move (classRefExp), move (assignmentListExp), move (opt_where_clause)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<AssignmentListExp> ECSqlParser::parse_assignment_commalist (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    auto listExp = unique_ptr<AssignmentListExp> (new AssignmentListExp ());
    const size_t assignmentCount = parseNode->count ();
    for (size_t i = 0; i < assignmentCount; i++)
        {
        auto assignmentNode = parseNode->getChild (i);
        BeAssert (SQL_ISRULE (assignmentNode, assignment) && assignmentNode->count () == 3 && "Wrong ECSQL grammar. Expected rule assignment.");

        auto lhsExp = parse_column_ref (ctx, assignmentNode->getChild (0));
        if (!ctx.IsSuccess ())
            return nullptr;

        auto rhsExp = parse_value_exp (ctx, assignmentNode->getChild (2));
        if (!ctx.IsSuccess ())
            return nullptr;

        listExp->AddAssignmentExp (unique_ptr<AssignmentExp> (new AssignmentExp (move (lhsExp), move (rhsExp))));
        }

    return move (listExp);
    }

//****************** Parsing UPDATE statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<DeleteStatementExp> ECSqlParser::parse_delete_statement_searched (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!ctx.IsSuccess ())
        return nullptr;

    //rule: delete_statement_searched: SQL_TOKEN_DELETE SQL_TOKEN_FROM table_ref opt_where_clause
    auto classRefExp = parse_table_ref (ctx, parseNode->getChild (2));
    if (!ctx.IsSuccess ())
        return nullptr;

    if (classRefExp->GetType () != Exp::Type::ClassName)
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "ECSQL DELETE statements only support ECClass references as target. Subqueries or join clauses are not supported.");
        return nullptr;
        }

    auto opt_where_clause = parse_opt_where_clause (ctx, parseNode->getChild (3));
    if (!ctx.IsSuccess ())
        return nullptr;

    return unique_ptr<DeleteStatementExp> (new DeleteStatementExp (move (classRefExp), move (opt_where_clause)));
    }

//****************** Parsing common expression ***********************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<PropertyNameExp> ECSqlParser::parse_column (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (parseNode->getNodeType() != SQL_NODE_NAME)
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::InvalidECSql, "Wrong grammar");
        return nullptr;
        }

    PropertyPath propPath;
    propPath.Push (parseNode->getTokenValue().c_str());

    return unique_ptr<PropertyNameExp> (new PropertyNameExp(move (propPath)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
std::unique_ptr<PropertyNameListExp> ECSqlParser::parse_opt_column_ref_commalist (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, opt_column_ref_commalist))
        {
        BeAssert (false && "Invalid grammar. Expecting opt_column_ref_commalist");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting opt_column_ref_commalist");
        return nullptr;
        }

    const size_t childCount = parseNode->count ();
    if (childCount == 0) 
        return nullptr; //User never provided a insert column name list clause 

    BeAssert (childCount == 3);
    //first and third nodes are ( and ). Second node is the the list node
    return parse_column_ref_commalist(ctx, parseNode->getChild(1));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
std::unique_ptr<PropertyNameListExp> ECSqlParser::parse_column_ref_commalist(ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, column_ref_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting column_ref_commalist");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting column_ref_commalist");
        return nullptr;
        }

    const size_t columnCount = parseNode->count();
    auto listExp = unique_ptr<PropertyNameListExp>(new PropertyNameListExp());
    for (size_t i = 0; i < columnCount; i++)
        {
        auto propertyNameExp = parse_column_ref(ctx, parseNode->getChild(i));
        if (!ctx.IsSuccess())
            {
            return nullptr;
            }

        listExp->AddPropertyNameExp(propertyNameExp);
        }

    return listExp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<FoldFunctionCallExp> ECSqlParser::parse_fold (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE (parseNode, fold))
        {
        BeAssert (false && "Wrong grammar. Expecting fold");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting fold.");
        return nullptr;
        }

    //first node is LOWER | UPPER, second is (, third is arg, fourth is )
    const size_t childCount = parseNode->count ();
    if (childCount != 4)
        {
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. fold is expected to have four child nodes.");
        return nullptr;
        }

    auto functionNameNode = parseNode->getChild (0);
    const auto functionNameTokenId = functionNameNode->getTokenID ();
    FoldFunctionCallExp::FoldFunction foldFunction;
    switch (functionNameTokenId)
        {
            case SQL_TOKEN_LOWER: foldFunction = FoldFunctionCallExp::FoldFunction::Lower; break;
            case SQL_TOKEN_UPPER: foldFunction = FoldFunctionCallExp::FoldFunction::Upper; break;
            default:
                {
                BeAssert (false && "Wrong grammar. Only LOWER or UPPER are valid function names for fold rule.");
                ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Only LOWER or UPPER are valid function names for fold rule.");
                return nullptr;
                }
        }

    auto argNode = parseNode->getChild (2);
    auto valueExp = parse_value_exp (ctx, argNode);
    if (!ctx.IsSuccess ())
        return nullptr;

    auto foldExp = std::unique_ptr<FoldFunctionCallExp>(new FoldFunctionCallExp(foldFunction));
    foldExp->AddArgument (move (valueExp));
    return std::move (foldExp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2014
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<PropertyNameExp> ECSqlParser::parse_property_path (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE (parseNode, property_path))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    PropertyPath propertyPath;
    for (size_t i = 0; i < parseNode->count (); i++)
        {
        auto property_path_entry = parseNode->getChild (i);
        auto first = property_path_entry->getFirst ();
        Utf8StringCR tokenValue = first->getTokenValue();
        if (first->getNodeType () == SQL_NODE_NAME)
            {
            auto opt_column_array_idx = property_path_entry->getChild (1);
            int arrayIndex = -1;
            if (opt_column_array_idx->getFirst () != nullptr)
                {
                arrayIndex = atoi (opt_column_array_idx->getFirst ()->getTokenValue ().c_str ());
                }

            if (arrayIndex < 0)
                propertyPath.Push(tokenValue.c_str());
            else
                propertyPath.Push(tokenValue.c_str(), (size_t) arrayIndex);

            }
        else if (first->getNodeType () == SQL_NODE_PUNCTUATION)
            {
            propertyPath.Push(tokenValue.c_str());
            }
        else
            {
            BeAssert (false && "Wrong grammar");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
            return nullptr;
            }
        }

    return std::unique_ptr<PropertyNameExp> (new PropertyNameExp (move (propertyPath)));
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<PropertyNameExp> ECSqlParser::parse_column_ref (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, column_ref ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    return move (parse_property_path (ctx, parseNode->getFirst ()));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ParameterExp> ECSqlParser::parse_parameter (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE (parseNode, parameter) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto const& paramTokenValue =parseNode->getChild(0)->getTokenValue ();
    
    Utf8CP paramName = nullptr;
    if (paramTokenValue.Equals(":"))
        {
        auto param_nameNode = parseNode->getChild(1);
        paramName = param_nameNode->getTokenValue ().c_str ();
        }
    else
        {
        if (!paramTokenValue.Equals ("?"))
            {
            BeAssert (paramTokenValue.Equals ("?") && "Invalid grammar. Only : or ? allowed as parameter tokens");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Only : or ? allowed as parameter tokens");
            return nullptr;
            }
        }

    return unique_ptr<ParameterExp> (new ParameterExp (paramName));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ValueExp> ECSqlParser::parse_term (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, term ) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }        
    auto operand_left  = parseNode->getChild(0);
    auto opNode = parseNode->getChild(1);
    auto operand_right = parseNode->getChild(2);

    auto operand_left_expr = parse_value_exp(ctx, operand_left);
    auto operand_right_expr = parse_value_exp(ctx, operand_right);

    if (!ctx.IsSuccess())
        return nullptr;

    BinarySqlOperator op;
    if (opNode->getTokenValue() == "*")
        op = BinarySqlOperator::Multiply;
    else if(opNode->getTokenValue() == "/")
        op = BinarySqlOperator::Divide;
    else
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    return unique_ptr<ValueExp>(new BinaryValueExp (move (operand_left_expr), op, move (operand_right_expr)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
// WIP_ECSQL: Implement Case operation also correct datatype list in sqlbison.y as
// per ECSQLTypes. We only support premitive type casting
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<CastExp> ECSqlParser::parse_cast_spec (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, cast_spec ) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    auto cast_operand = parseNode->getChild(2);
    auto cast_target = parseNode->getChild(4);

    auto cast_operand_expr = parse_value_exp(ctx, cast_operand);
    if (!ctx.IsSuccess())
        return nullptr;

    Utf8CP sqlType = nullptr;
    switch(cast_target->getTokenID())
        {
        case SQL_TOKEN_BINARY:
            sqlType = "BINARY"; break;
        case SQL_TOKEN_BOOLEAN:
            sqlType = "BOOLEAN"; break;
        case SQL_TOKEN_DOUBLE:
            sqlType = "DOUBLE"; break;
        case SQL_TOKEN_INT:
            sqlType = "INT"; break;
        case SQL_TOKEN_INTEGER:
            sqlType = "INTEGER"; break;
        case SQL_TOKEN_INT32:
            sqlType = "INT32"; break;
        case SQL_TOKEN_LONG:
            sqlType = "LONG"; break;
        case SQL_TOKEN_INT64:
            sqlType = "INT64"; break;
        case SQL_TOKEN_STRING:
            sqlType = "STRING"; break;
        case SQL_TOKEN_DATE:
            sqlType = "DATE"; break;
        case SQL_TOKEN_TIMESTAMP:
            sqlType = "TIMESTAMP"; break;
        case SQL_TOKEN_DATETIME:
            sqlType = "DATETIME"; break;
        case SQL_TOKEN_POINT2D:
            sqlType = "POINT2D"; break;
        case SQL_TOKEN_POINT3D:
            sqlType = "POINT3D"; break;
        default:
            BeAssert (false && "Unknown cast target type");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Unknown cast target type");
        }

    return unique_ptr<CastExp>(new CastExp(move (cast_operand_expr), sqlType));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<FunctionCallExp> ECSqlParser::parse_fct_spec (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, fct_spec ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    OSQLParseNode* functionNameNode = parseNode->getChild (0);
    Utf8CP knownFunctionName = functionNameNode->getTokenValue ().c_str ();
    if (Utf8String::IsNullOrEmpty (knownFunctionName))
        {
        const auto tokenId = functionNameNode->getTokenID ();
        ctx.SetError(ECSqlStatus::InvalidECSql, "Function with token ID %d not yet supported.", tokenId);
        return nullptr;
        }

    auto functionCallExp = std::unique_ptr<FunctionCallExp> (new FunctionCallExp (knownFunctionName));
    //parse function args. (if child parse node count is < 4, function doesn't have args)
    if (parseNode->count() == 4)
        {
        OSQLParseNode* argumentsNode = parseNode->getChild(2);
        if (SQL_ISRULE(argumentsNode, function_args_commalist))
            {
            for (size_t i = 0; i < argumentsNode->count(); i++)
                {
                if (SUCCESS != parse_and_add_functionarg(ctx, *functionCallExp, argumentsNode->getChild(i)))
                    return nullptr;
                }
            }
        else
            {
            if (SUCCESS != parse_and_add_functionarg(ctx, *functionCallExp, argumentsNode))
                return nullptr;
            }
        }

    if (ctx.IsSuccess())
        return functionCallExp;

    BeAssert(false);
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_and_add_functionarg(ECSqlParseContext& ctx, FunctionCallExp& functionCallExp, connectivity::OSQLParseNode const* argNode)
    {
    auto argument_expr = parse_result(ctx, argNode);
    if (argument_expr == nullptr)
        return ERROR; // error reporting already done in child call

    functionCallExp.AddArgument(move(argument_expr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<SetFunctionCallExp> ECSqlParser::parse_general_set_fct(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, general_set_fct) && parseNode->count() == 3)
        {
        BeAssert(false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    OSQLParseNode* functionNameNode = parseNode->getChild(0);
    SetFunctionCallExp::Function function;
    switch (functionNameNode->getTokenID())
        {
            case SQL_TOKEN_ANY: function = SetFunctionCallExp::Function::Any; break;
            case SQL_TOKEN_AVG: function = SetFunctionCallExp::Function::Avg; break;
            case SQL_TOKEN_COUNT: function = SetFunctionCallExp::Function::Count; break;
            case SQL_TOKEN_EVERY: function = SetFunctionCallExp::Function::Every; break;
            case SQL_TOKEN_MIN: function = SetFunctionCallExp::Function::Min; break;
            case SQL_TOKEN_MAX: function = SetFunctionCallExp::Function::Max; break;
            case SQL_TOKEN_SUM: function = SetFunctionCallExp::Function::Sum; break;
            case SQL_TOKEN_SOME: function = SetFunctionCallExp::Function::Some; break;
            default:
                {
                ctx.SetError(ECSqlStatus::InvalidECSql, "Unsupported standard SQL function with token ID %d", functionNameNode->getTokenID());
                return nullptr;
                }
        }

    //Following cover COUNT(ALL|DISTINCT funtion_arg) and AVG,MAX...(ALL|DISTINCT funtion_arg)
    OSQLParseNode* opt_all_distinctNode = parseNode->getChild(2);
    SqlSetQuantifier setQuantifier = parse_opt_all_distinct(ctx, opt_all_distinctNode);

    auto functionCallExp = unique_ptr<SetFunctionCallExp>(new SetFunctionCallExp(function, setQuantifier));

    if (function == SetFunctionCallExp::Function::Count &&
        Exp::IsAsteriskToken(parseNode->getChild(2)->getTokenValue().c_str()))
        {
        auto argExp = ConstantValueExp::Create(ctx, Exp::ASTERISK_TOKEN, ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies));
        functionCallExp->AddArgument(move(argExp));
        }
    else
        {
        if (SUCCESS != parse_and_add_functionarg(ctx, *functionCallExp, parseNode->getChild(3/*function_arg*/)))
            return nullptr;
        }

    if (ctx.IsSuccess())
        return functionCallExp;

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<ECClassIdFunctionExp> ECSqlParser::parse_ecclassid_fct_spec (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    const auto childNodeCount = parseNode->count ();
    if (!SQL_ISRULE (parseNode, ecclassid_fct_spec) || (childNodeCount != 3 && childNodeCount != 5))
        {
        BeAssert (false && "Wrong grammar. Expecting ecclassid_fct_spec.");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting ecclassid_fct_spec.");
        return nullptr;
        }

    auto firstChildNode = parseNode->getChild (0);
    if (childNodeCount != 3)
        {
        auto prefixPath = parse_property_path (ctx, firstChildNode);
        if (prefixPath.get() != nullptr && prefixPath->GetPropertyPath ().Size () == 1)
            {
            Utf8CP classAlias = prefixPath->GetPropertyPath ()[0].GetPropertyName();
            return std::unique_ptr<ECClassIdFunctionExp> (new ECClassIdFunctionExp (classAlias));
            }
        else
            {
            BeAssert (false && "Wrong grammar. Expecting <class-alias>.GetECClassId().");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting <class-alias>.GetECClassId().");
            return nullptr;
            }
        }

    return unique_ptr<ECClassIdFunctionExp> (new ECClassIdFunctionExp (nullptr));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_result (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {    
    return parse_value_exp(ctx, parseNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_value_exp_primary (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, value_exp_primary ) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    unique_ptr<ValueExp> exp = parse_value_exp(ctx,  parseNode->getChild(1));
    //This rule is expected to always have parentheses
    BeAssert(parseNode->getChild(0)->getTokenValue() == "(" &&
             parseNode->getChild(2)->getTokenValue() == ")");

    if (parseNode->getChild(0)->getTokenValue().Equals("("))
        exp->SetHasParentheses();

    return exp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_num_value_exp (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, num_value_exp ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    auto operand_left  = parseNode->getChild(0);
    auto opNode        = parseNode->getChild(1);
    auto operand_right = parseNode->getChild(2);

    auto operand_left_expr = parse_value_exp(ctx, operand_left);
    auto operand_right_expr = parse_value_exp(ctx, operand_right);

    BinarySqlOperator op;
    if (opNode->getTokenValue() == "+")
        op = BinarySqlOperator::Plus;
    else if (opNode->getTokenValue() == "-")
        op = BinarySqlOperator::Minus;
    else
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    return unique_ptr<ValueExp>(new BinaryValueExp(move (operand_left_expr), op, move (operand_right_expr)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<UnaryValueExp> ECSqlParser::parse_factor (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, factor ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto opNode = parseNode->getChild(0);
    auto operandNode = parseNode->getChild(1);

    auto operand_expr = parse_value_exp(ctx, operandNode);
    if (!ctx.IsSuccess())
        return nullptr;

    UnarySqlOperator op = UnarySqlOperator::Plus;
    if (opNode->getTokenValue ().Equals ("+"))
        op = UnarySqlOperator::Plus;
    else if (opNode->getTokenValue().Equals ("-"))
        op = UnarySqlOperator::Minus;
    else
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    return unique_ptr<UnaryValueExp> (new UnaryValueExp (operand_expr.release (), op));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_concatenation (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, concatenation))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    auto operand_left = parseNode->getChild(0);
    auto operand_right = parseNode->getChild(2);

    auto operand_left_expr = parse_value_exp(ctx, operand_left); 
    auto operand_right_expr = parse_value_exp(ctx, operand_right);

    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<ValueExp>(new BinaryValueExp(move (operand_left_expr), BinarySqlOperator::Concat, move (operand_right_expr)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_datetime_value_exp(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, datetime_value_exp ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto datetime_term = parseNode->getByRule(OSQLParseNode::datetime_term);
    if (datetime_term == nullptr)
        return nullptr;
    return parse_datetime_term (ctx, datetime_term);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_datetime_term (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, datetime_term ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto datetime_factor = parseNode->getByRule(OSQLParseNode::datetime_factor);
    if (datetime_factor == nullptr)
        return nullptr;
    return parse_datetime_factor (ctx, datetime_factor);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_datetime_factor (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, datetime_factor) && (parseNode->count() == 1 || parseNode->count() == 2))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto datetime_primary = parseNode->getByRule(OSQLParseNode::datetime_primary);
    if (datetime_primary == nullptr)
        return nullptr;

    auto constant_expr = parse_datetime_primary (ctx, datetime_primary);
    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<ValueExp>(constant_expr.release());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ConstantValueExp> ECSqlParser::parse_datetime_primary  (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, datetime_primary ))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto datetime_value_fct = parseNode->getByRule(OSQLParseNode::datetime_value_fct);
    if (datetime_value_fct == nullptr)
        return nullptr;
    return parse_datetime_value_fct (ctx, datetime_value_fct);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ConstantValueExp> ECSqlParser::parse_datetime_value_fct(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, datetime_value_fct ) && (parseNode->count() == 1 || parseNode->count() == 2))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    auto type = parseNode->getChild(0);
    if (parseNode->count() == 1 ) //Keyword
        {
        if (type->getTokenID() == SQL_TOKEN_CURRENT_DATE)
            return ConstantValueExp::Create (ctx, "CURRENT_DATE", ECSqlTypeInfo (ECN::PRIMITIVETYPE_DateTime));
        if (type->getTokenID() == SQL_TOKEN_CURRENT_TIMESTAMP)
            return ConstantValueExp::Create (ctx, "CURRENT_TIMESTAMP", ECSqlTypeInfo (ECN::PRIMITIVETYPE_DateTime));

        ctx.SetError(ECSqlStatus::InvalidECSql, "Unrecognized keyword '%s'.", parseNode->getTokenValue().c_str());
        return nullptr;
        }

    auto unparsedDateOrTimestampValue = parseNode->getChild(1)->getTokenValue().c_str ();
    //WIP_ECSQL: Parse date value into a structure
    if (type->getTokenID () == SQL_TOKEN_DATE || type->getTokenID () == SQL_TOKEN_TIMESTAMP)
        return ConstantValueExp::Create (ctx, unparsedDateOrTimestampValue, ECSqlTypeInfo (ECN::PRIMITIVETYPE_DateTime));

    BeAssert (false && "Wrong grammar");
    ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
    return nullptr;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<FromExp> ECSqlParser::parse_from_clause (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!ctx.IsSuccess())
        return nullptr;        

    auto from_clause = unique_ptr<FromExp>(new FromExp());
    OSQLParseNode* table_ref_commalist = parseNode->getChild(1);
    for (size_t n = 0; n < table_ref_commalist->count(); n++)
        {
        auto classRef = parse_table_ref (ctx, table_ref_commalist->getChild (n));
        if (classRef.get() != nullptr)
            {
            ECSqlStatus status = from_clause->TryAddClassRef(ctx, classRef.get());
            if (status != ECSqlStatus::Success)
                {
                return nullptr;
                }
            else
                classRef.release();
            }
        else
            break; //Error
        }

    if (ctx.IsSuccess())
        return from_clause;     
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ClassRefExp> ECSqlParser::parse_table_ref (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISRULE (parseNode, qualified_join) || SQL_ISRULE (parseNode, relationship_join))
        return unique_ptr<ClassRefExp> (parse_joined_table (ctx, parseNode).release ());

    if (!SQL_ISRULE (parseNode, table_ref))
        {
        BeAssert (false && "Wrong grammar. Expecting table_name.");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting table_name.");
        return nullptr;
        }

    OSQLParseNode* opt_only = parseNode->getChild (0);
    OSQLParseNode* second = parseNode->getChild (1);

    bool isPolymorphic = !(opt_only->getTokenID () == SQL_TOKEN_ONLY);
    if (SQL_ISRULE (second, table_node))
        {
        auto classNameExp = parse_table_node (ctx, second, isPolymorphic);
        auto table_primary_as_range_column = parseNode->getChild (2);
        if (!ctx.IsSuccess ())
            {
            BeAssert (classNameExp == nullptr);
            return nullptr;
            }

        if (table_primary_as_range_column->count () > 0)
            {
            OSQLParseNode* table_alias = table_primary_as_range_column->getChild (1);
            OSQLParseNode* opt_column_commalist = table_primary_as_range_column->getChild (2);
            if (opt_column_commalist->count () > 0)
                {
                BeAssert (false && "Range column not supported");
                //WIP_ECSQL:Right error code?
                ctx.SetError (ECSqlStatus::ProgrammerError, "Range column not supported");
                }

            if (!table_alias->getTokenValue ().empty ())
                classNameExp->SetAlias (table_alias->getTokenValue ());
            }

        return std::move (classNameExp);
        }

    if (SQL_ISRULE(second, subquery))
        {
        auto subquery = parse_subquery(ctx, second);
        auto range_variable = parseNode->getChild(2/*range_variable*/);
        auto alias = Utf8String();
        if (range_variable->count() > 0)
            alias = range_variable->getChild(1/*SQL_TOKEN_NAME*/)->getTokenValue();
   
         if (!ctx.IsSuccess())
                return nullptr;
  
        auto subQueryRef = unique_ptr<SubqueryRefExp>( new SubqueryRefExp(subquery.release(), alias, isPolymorphic));
        return std::move(subQueryRef);
        }

    //Commented out in Grammer.
    //if (second->getTokenValue().Equals("("))
    //    {
    //    return unique_ptr<ClassRefExp>(parse_joined_table(ctx, parseNode->getChild(2/*joined_table*/)).release());
    //    }

    BeAssert (false && "Case not supported");
    ctx.SetError (ECSqlStatus::ProgrammerError, "Case not supported");
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<JoinExp> ECSqlParser::parse_joined_table (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    switch(parseNode->getKnownRuleID())
        {
        case OSQLParseNode::qualified_join:
            return parse_qualified_join (ctx, parseNode);
        case OSQLParseNode::cross_union:
            return unique_ptr<JoinExp>(parse_cross_union (ctx, parseNode).release());
        case OSQLParseNode::relationship_join:
            return unique_ptr<JoinExp>(parse_relationship_join (ctx, parseNode).release());
        }       

    BeAssert (false && "Wrong grammar");
    ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<RelationshipJoinExp> ECSqlParser::parse_relationship_join (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, relationship_join))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto from_table_ref = parse_table_ref(ctx, parseNode->getChild(0/*table_ref*/));
    auto join_type = parse_join_type (ctx, parseNode->getChild (1/*join_type*/));
    auto to_table_ref = parse_table_ref(ctx, parseNode->getChild(3/*table_ref*/));
    //TODO: need to decide whether we support ONLY in USING clause.
    auto table_node = parse_table_node(ctx, parseNode->getChild(5/*table_node*/), true);
    auto op_relationship_direction = parseNode->getChild(6/*op_relationship_direction*/);

    if (!ctx.IsSuccess())
        return nullptr;

    if (!(join_type == ECSqlJoinType::InnerJoin || join_type == ECSqlJoinType::None))
        {
        ctx.SetError (ECSqlStatus::ProgrammerError, "Supported join type is INNER JOIN");
        return nullptr;
        }

    JoinDirection direction = JoinDirection::Implied;
    if (op_relationship_direction->getTokenID() == SQL_TOKEN_FORWARD)
        direction = JoinDirection::Forward;
    else if (op_relationship_direction->getTokenID() == SQL_TOKEN_REVERSE)
        direction = JoinDirection::Reverse;

    return unique_ptr<RelationshipJoinExp>(new RelationshipJoinExp(move (from_table_ref), move (to_table_ref),  move (table_node), direction));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<JoinExp> ECSqlParser::parse_qualified_join (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, qualified_join))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto from_table_ref = parse_table_ref(ctx, parseNode->getChild(0/*table_ref*/));
    if (from_table_ref == nullptr)
        {
        BeAssert (!ctx.IsSuccess ());
        return nullptr;
        }

    auto key = parseNode->getChild(1);
    if (key->getTokenID() == SQL_TOKEN_NATURAL)
        {
        auto join_type = parse_join_type(ctx, parseNode->getChild(2/*join_type*/));
        auto to_table_ref = parse_table_ref(ctx, parseNode->getChild(4/*table_ref*/));
        if (ctx.IsSuccess())
            return unique_ptr<JoinExp>(new NaturalJoinExp(move (from_table_ref), move (to_table_ref), join_type));
        }
    else
        {
        auto join_type = parse_join_type(ctx, parseNode->getChild (1/*join_type*/));
        auto to_table_ref = parse_table_ref(ctx, parseNode->getChild (3/*table_ref*/));
        auto join_spec = parse_join_spec(ctx, parseNode->getChild (4/*join_spec*/));
        if (ctx.IsSuccess())
            return unique_ptr<JoinExp>(new QualifiedJoinExp(move (from_table_ref), move (to_table_ref), join_type, move (join_spec)));
        }

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<CrossJoinExp> ECSqlParser::parse_cross_union (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, cross_union))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }
    auto from_table_ref = parse_table_ref(ctx, parseNode->getChild(0/*table_ref*/));
    auto to_table_ref = parse_table_ref(ctx, parseNode->getChild(3/*table_ref*/));
    if (ctx.IsSuccess())
        return unique_ptr<CrossJoinExp>(new CrossJoinExp(move (from_table_ref), move (to_table_ref)));
    
    BeAssert (false && "Wrong grammar");
    ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar");
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlJoinType ECSqlParser::parse_join_type (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISRULE(parseNode, join_type))
        {
        if (parseNode->count() == 0) //default value
            return ECSqlJoinType::InnerJoin;
        auto first = parseNode->getChild(0);
        if (first->getTokenID() == SQL_TOKEN_INNER)
            return ECSqlJoinType::InnerJoin;
        }
    if (SQL_ISRULE(parseNode, outer_join_type))
        return parse_outer_join_type(ctx, parseNode);

    BeAssert (false && "Invalid grammar. Expected JoinType");
    ctx.SetError(ECSqlStatus::ProgrammerError,"Invalid grammar. Expected JoinType");
    return ECSqlJoinType::None;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlJoinType ECSqlParser::parse_outer_join_type (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, outer_join_type))
        {
        BeAssert (false && "Invalid grammar. Expected OuterJoinType");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expected OuterJoinType");
        return ECSqlJoinType::None;
        }
    auto n = parseNode->getChild(0);
    switch(n->getTokenID())
        {
        case SQL_TOKEN_LEFT: return ECSqlJoinType::LeftOuterJoin;
        case SQL_TOKEN_RIGHT: return ECSqlJoinType::RightOuterJoin;
        case SQL_TOKEN_FULL: return ECSqlJoinType::FullOuterJoin;
        }

    BeAssert (false && "Invalid grammar. Expected LEFT, RIGHT or FULL");
    ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expected LEFT, RIGHT or FULL");
    return ECSqlJoinType::None;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<JoinSpecExp> ECSqlParser::parse_join_spec (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    switch(parseNode->getKnownRuleID())
        {
        case OSQLParseNode::join_condition:
            return unique_ptr<JoinSpecExp>(parse_join_condition (ctx, parseNode).release());
        case OSQLParseNode::named_columns_join:
            return unique_ptr<JoinSpecExp>(parse_named_columns_join (ctx, parseNode).release());
        }       

    BeAssert (false && "Wrong grammar");
    ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<JoinConditionExp> ECSqlParser::parse_join_condition (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, join_condition))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }

    auto search_condition = parse_search_condition(ctx, parseNode->getChild(1/*search_condition*/));    
    if (ctx.IsSuccess())
        return unique_ptr<JoinConditionExp>(new JoinConditionExp(move (search_condition)));

    BeAssert (false && "Wrong grammar");
    ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
    return nullptr; 
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<NamedPropertiesJoinExp> ECSqlParser::parse_named_columns_join (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, named_columns_join))
        {
        BeAssert (false && "Wrong grammar");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar");
        return nullptr;
        }    
    
    auto expr = unique_ptr<NamedPropertiesJoinExp>(new NamedPropertiesJoinExp());
    auto column_commalist = parseNode->getChild(2/*column_commalist*/); 
    for(size_t i =0; i <column_commalist->count(); i++)
        expr->Append(column_commalist->getChild(i)->getTokenValue());
    return expr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ClassNameExp> ECSqlParser::parse_table_node(ECSqlParseContext& ctx, OSQLParseNode const* parseNode, bool isPolymorphic)
    {
    if (!SQL_ISRULE(parseNode, table_node))
        {
        BeAssert (false && "Wrong grammar. Expecting table_node");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting table_node");
        return nullptr;
        }  
    auto first = parseNode->getChild(0);
    Utf8CP className = nullptr;
    Utf8CP schemaName = nullptr;
    Utf8CP catalogName = nullptr;
    switch(first->getKnownRuleID())
        {
        case OSQLParseNode::table_name:
            {
            className = parse_table_name (ctx, first); 
            break;
            }
        case OSQLParseNode::schema_name:  
            {
            schemaName = parse_schema_name (className, ctx, first); 
            break;
            }
        case OSQLParseNode::catalog_name: 
            {
            catalogName = parse_catalog_name (schemaName, className, ctx, first); 
            break;
            }
        default:
            BeAssert (false && "Wrong Grammar. Expecting table_name, schema_name or catalog_name");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong Grammar. Expecting table_name, schema_name or catalog_name");
        };

    if (ctx.IsSuccess())
        {
        shared_ptr<ClassNameExp::Info> classNameExpInfo;
        auto resolveStatus = ctx.TryResolveClass(classNameExpInfo, schemaName, className);
        if (resolveStatus == ECSqlStatus::Success)
            return unique_ptr<ClassNameExp> (new ClassNameExp (className, schemaName, catalogName, classNameExpInfo, isPolymorphic));
        }

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlParser::parse_table_name (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, table_name))
        {
        BeAssert (false && "Wrong grammar. Expecting table_name");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting table_name");
        return nullptr;
        }

    return parseNode->getChild(0)->getTokenValue().c_str ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlParser::parse_schema_name(Utf8CP& className, ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, schema_name))
        {
        BeAssert (false && "Wrong grammar. Expecting schema_name");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting schema_name");
        return nullptr;
        }
    OSQLParseNode* schemaNameNode = parseNode->getChild(0);
    OSQLParseNode* tableNameNode  = parseNode->getChild(2);

    className = parse_table_name (ctx, tableNameNode);
    return schemaNameNode->getTokenValue().c_str ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlParser::parse_catalog_name(Utf8CP& schemaName, Utf8CP& className, ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, catalog_name))
        {
        BeAssert (false && "Wrong grammar. Expecting catalog_name");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting catalog_name");
        return nullptr;
        }

    OSQLParseNode* catalogNameNode = parseNode->getChild(0);
    OSQLParseNode* schemaNameNode  = parseNode->getChild(2);

    schemaName = parse_schema_name (className, ctx, schemaNameNode);

    return catalogNameNode->getTokenValue().c_str ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<WhereExp> ECSqlParser::parse_opt_where_clause (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISRULE(parseNode, opt_where_clause))
        return nullptr;

    if (!SQL_ISRULE(parseNode, where_clause))
        {
        BeAssert (false && "Wrong grammar. Expecting where_clause");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting where_clause");
        return nullptr;
        }

    unique_ptr<BooleanExp> search_condition = parse_search_condition(ctx, parseNode->getChild(1/*search_condition*/));
    if (ctx.IsSuccess())
        return unique_ptr<WhereExp>(new WhereExp(move (search_condition)));

    return nullptr;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<BooleanExp> ECSqlParser::parse_search_condition (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    const auto rule = parseNode->getKnownRuleID();
    switch(rule)
        {
        case OSQLParseNode::search_condition:
            {
            auto op1 = parse_search_condition(ctx, parseNode->getChild(0/*search_condition*/));
            //auto op = parseNode->getChild(1/*SQL_TOKEN_OR*/);
            auto op2 = parse_search_condition(ctx, parseNode->getChild(2/*boolean_tern*/));
            if (ctx.IsSuccess())
                return unique_ptr<BooleanExp>(new BinaryBooleanExp(move (op1), BooleanSqlOperator::Or, move (op2)));

            break;
            }
        case OSQLParseNode::boolean_term:
            {
            auto op1 = parse_search_condition(ctx, parseNode->getChild(0/*boolean_term*/));
            //auto op = parseNode->getChild(1/*SQL_TOKEN_AND*/);
            auto op2 = parse_search_condition(ctx, parseNode->getChild(2/*boolean_factor*/));

            if (ctx.IsSuccess())
                return unique_ptr<BooleanExp>(new BinaryBooleanExp(move (op1), BooleanSqlOperator::And, move (op2)));

            break;
            }
        case OSQLParseNode::boolean_factor:
            {
            auto operandValueExp = parse_search_condition(ctx, parseNode->getChild(1/*boolean_test*/));
            if (ctx.IsSuccess())
                return unique_ptr<BooleanExp>(new BooleanFactorExp(move(operandValueExp), true));
            
            break;
            }
        case OSQLParseNode::boolean_test:
            {
            auto op1 = parse_search_condition(ctx, parseNode->getChild(0/*boolean_primary*/));
            //auto is = parseNode->getChild(1/*SQL_TOKEN_IS*/);
            auto sql_not = parse_sql_not(ctx, parseNode->getChild(2/*sql_not*/));
            auto truth_value_expr = parse_trueth_value(ctx, parseNode->getChild(3/*truth_value*/));
            // X IS [NOT] NULL|TRUE|FALSE|UNKNOWN
            if (ctx.IsSuccess())
                {
                return unique_ptr<BooleanExp> (
                    new BinaryBooleanExp(
                        move (op1), 
                        sql_not ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is,
                        move (truth_value_expr)));
                }

            break;
            }
        case OSQLParseNode::boolean_primary:
            {
            BeAssert(parseNode->count() == 3);
            unique_ptr<BooleanExp> exp = parse_search_condition(ctx, parseNode->getChild(1/*search_condition*/));
            exp->SetHasParentheses();
            return exp;
            }
        case OSQLParseNode::unary_predicate:
            return parse_unary_predicate(ctx, parseNode);

        case OSQLParseNode::comparison_predicate:
            {
            if (parseNode->count() == 3 /*row_value_constructor comparison row_value_constructor*/)
                {
                auto op1 = parse_row_value_constructor (ctx, parseNode->getChild(0/*row_value_constructor*/));
                auto op  = parse_comparison (ctx, parseNode->getChild(1/*comparison*/));
                auto op2 = parse_row_value_constructor (ctx, parseNode->getChild(2/*row_value_constructor*/));
                if (ctx.IsSuccess())
                    return unique_ptr<BooleanExp>(new BinaryBooleanExp(move (op1), op, move (op2)));            
                }

            break;
            }
        case OSQLParseNode::between_predicate:
            {
            auto lhsOperand = parse_row_value_constructor (ctx, parseNode->getChild(0/*row_value_constructor*/));

            auto between_predicate_part_2 = parseNode->getChild(1/*between_predicate_part_2*/);
            auto sql_not = parse_sql_not(ctx, between_predicate_part_2->getChild(0/*sql_not*/));
            const auto op = sql_not ? BooleanSqlOperator::NotBetween : BooleanSqlOperator::Between;

            auto lowerBound = parse_row_value_constructor(ctx, between_predicate_part_2->getChild(2/*row_value_constructor*/));
            auto upperBound = parse_row_value_constructor(ctx, between_predicate_part_2->getChild(4/*row_value_constructor*/));
            auto betweenRangeValueExp = std::unique_ptr<ValueExp> (new BetweenRangeValueExp (move (lowerBound), move (upperBound)));

            if (!ctx.IsSuccess())
                break;

            return unique_ptr<BooleanExp>(new BinaryBooleanExp(move (lhsOperand), op, move (betweenRangeValueExp)));
            }

        case OSQLParseNode::all_or_any_predicate:
            {
            auto op1 = parse_row_value_constructor (ctx, parseNode->getChild(0/*comparison*/));
            auto quantified_comparison_predicate_part_2 = parseNode->getChild(1/*quantified_comparison_predicate_part_2*/);
            auto comparison = parse_comparison(ctx, quantified_comparison_predicate_part_2->getChild(0/*sql_not*/));
            auto any_all_some = parse_any_all_some(ctx, quantified_comparison_predicate_part_2->getChild(1/*any_all_some*/));
            auto subquery = parse_subquery(ctx, quantified_comparison_predicate_part_2->getChild(4/*row_value_constructor*/));

            if (ctx.IsSuccess())
                return unique_ptr<BooleanExp>(new AllOrAnyExp(move (op1), comparison, any_all_some, move (subquery)));

            break;
            }
        case OSQLParseNode::existence_test:
            {
            auto subquery = parse_subquery(ctx, parseNode->getChild(1/*subquery*/));
            if (ctx.IsSuccess())
                return unique_ptr<BooleanExp>(new SubqueryTestExp(SubqueryTestOperator::Exists, move (subquery)));
            
            break;
            }
        case OSQLParseNode::unique_test:
            {
            auto subquery = parse_subquery(ctx, parseNode->getChild(1/*subquery*/));
            if (ctx.IsSuccess())
                return unique_ptr<BooleanExp>(new SubqueryTestExp(SubqueryTestOperator::Unique, move(subquery)));
            
            break;
            }
        case OSQLParseNode::test_for_null:
            {
            auto row_value_constructor = parse_row_value_constructor(ctx, parseNode->getChild(0/*row_value_constructor*/));
            auto null_predicate_part_2 = parseNode->getChild(1/*row_value_constructor*/);
            auto sql_not = parse_sql_not(ctx, null_predicate_part_2->getChild(1/*sql_not*/));
            auto nullExp = parse_value_exp (ctx, null_predicate_part_2->getChild(2/*NULL*/));
            if (ctx.IsSuccess())
                {
                return unique_ptr<BooleanExp> (new BinaryBooleanExp(
                                                        move (row_value_constructor), 
                                                        sql_not? BooleanSqlOperator::IsNot:BooleanSqlOperator::Is,
                                                        move (nullExp)));

                }

            break;
            }
        case OSQLParseNode::in_predicate:
            return parse_in_predicate (ctx, parseNode);

        case OSQLParseNode::like_predicate:
            return parse_like_predicate (ctx, parseNode);

        case OSQLParseNode::rtreematch_predicate:
            return parse_rtreematch_predicate(ctx, parseNode);

        default:
            BeAssert (false && "Invalid grammar");
            ctx.SetError (ECSqlStatus::ProgrammerError,"Invalid grammar");
            break;
        }

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlParser::isPredicate(OSQLParseNode const* parseNode)
    {
    const auto rule = parseNode->getKnownRuleID();
    switch(rule)            
        {
        case OSQLParseNode::boolean_term:
        case OSQLParseNode::boolean_factor:
        case OSQLParseNode::boolean_test:
        case OSQLParseNode::boolean_primary:
        //Predicates
        case OSQLParseNode::comparison_predicate:
        case OSQLParseNode::between_predicate:
        case OSQLParseNode::all_or_any_predicate:
        case OSQLParseNode::existence_test:
        case OSQLParseNode::unique_test:
        case OSQLParseNode::test_for_null:
        case OSQLParseNode::in_predicate:
        case OSQLParseNode::like_predicate:
        case OSQLParseNode::unary_predicate:
            return true;
        }
    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<BooleanExp> ECSqlParser::parse_in_predicate (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if(!SQL_ISRULE (parseNode, in_predicate))
        {
        BeAssert (false && "Wrong grammar. Expecting in_predicate");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting in_predicate");
        return nullptr;
        }

    const auto firstChildNode = parseNode->getChild (0);
    if (SQL_ISRULE (firstChildNode, in_predicate_part_2))
        {
        BeAssert (false);
        ctx.SetError (ECSqlStatus::InvalidECSql, "IN predicate without left-hand side property not supported.");
        return nullptr;
        }

    //first item is row value ctor node
    auto lhsExp = parse_row_value_constructor (ctx, firstChildNode);
    if (!ctx.IsSuccess ())
        return nullptr;

    auto inOperator = BooleanSqlOperator::In;
    auto rhsExp = parse_in_predicate_part_2 (ctx, inOperator, parseNode->getChild (1));
    if (!ctx.IsSuccess ())
        return nullptr;

    return unique_ptr<BooleanExp> (new BinaryBooleanExp (move (lhsExp), inOperator, move(rhsExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<ComputedExp> ECSqlParser::parse_in_predicate_part_2 (ECSqlParseContext& ctx, BooleanSqlOperator& inOperator, OSQLParseNode const* parseNode)
    {
    //in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value
    if (!SQL_ISRULE (parseNode, in_predicate_part_2))
        {
        BeAssert (false && "Invalid grammar. Expecting in_predicate_part_2");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting in_predicate_part_2");
        return nullptr;
        }

    const auto sqlnotFlag = parse_sql_not (ctx, parseNode->getChild (0));
    inOperator = sqlnotFlag ? BooleanSqlOperator::NotIn : BooleanSqlOperator::In;

    //third item is the predicate value (second item is IN token)
    auto in_predicate_valueNode = parseNode->getChild (2);

    OSQLParseNode* in_predicate_valueFirstChildNode = nullptr;
    if (SQL_ISRULE ((in_predicate_valueFirstChildNode = in_predicate_valueNode->getChild (0)), subquery))
        return parse_value_exp (ctx, in_predicate_valueFirstChildNode);
    else
        //if no subquery it must be '(' value_exp_commalist ')'. Safety check is done in parse_value_exp_commalist method
        return parse_value_exp_commalist (ctx, in_predicate_valueNode->getChild (1));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<BooleanExp> ECSqlParser::parse_like_predicate (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if(!SQL_ISRULE (parseNode, like_predicate))
        {
        BeAssert (false && "Wrong grammar. Expecting like_predicate");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting like_predicate");
        return nullptr;
        }
    
    //first item is row value ctor node
    const auto firstChildNode = parseNode->getChild (0);
    auto lhsExp = parse_row_value_constructor (ctx, firstChildNode);
    if (!ctx.IsSuccess ())
        return nullptr;

    auto likeOperator = BooleanSqlOperator::Like;
    auto rhsExp = parse_like_predicate_part_2 (ctx, likeOperator, parseNode->getChild (1));
    if (!ctx.IsSuccess ())
        return nullptr;

    return unique_ptr<BooleanExp> (new BinaryBooleanExp (move (lhsExp), likeOperator, move(rhsExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<ComputedExp> ECSqlParser::parse_like_predicate_part_2 (ECSqlParseContext& ctx, BooleanSqlOperator& likeOperator, OSQLParseNode const* parseNode)
    {
    //character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape
    //other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape
    if (!SQL_ISRULE (parseNode, character_like_predicate_part_2) && !SQL_ISRULE (parseNode, other_like_predicate_part_2))
        {
        BeAssert (false && "Invalid grammar. Expecting character_like_predicate_part_2 or other_like_predicate_part_2");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting character_like_predicate_part_2 or other_like_predicate_part_2");
        return nullptr;
        }

    const auto sqlnotFlag = parse_sql_not (ctx, parseNode->getChild (0));
    likeOperator = sqlnotFlag ? BooleanSqlOperator::NotLike : BooleanSqlOperator::Like;

    //third item is value_exp_primary or string_value_exp value (second item is LIKE token)
    auto valueExpNode = parseNode->getChild (2);
    auto rhsExp = parse_value_exp (ctx, valueExpNode);

    //fourth item is escape clause. Escape clause node always exists. If no escape was specified, the node is empty
    unique_ptr<ValueExp> escapeExp = nullptr;
    auto escapeClauseNode = parseNode->getChild (3);
    const auto escapeChildNodeCount = escapeClauseNode->count ();
    //no nodes means no escape clause
    if (escapeChildNodeCount > 0)
        {
        if (escapeChildNodeCount != 2)
            {
            BeAssert (false && "Invalid grammar. Corrupt opt_escape expression");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Corrupt opt_escape expression");
            return nullptr;
            }

        //second child node has escape expression (first node is ESCAPE token)
        escapeExp = parse_value_exp (ctx, escapeClauseNode->getChild (1));
        }

    return unique_ptr<ComputedExp> (new LikeRhsValueExp (move (rhsExp), move (escapeExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    05/2015
//+---------------+---------------+---------------+---------------+---------------+--------
std::unique_ptr<BooleanExp> ECSqlParser::parse_rtreematch_predicate(ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, rtreematch_predicate))
        {
        BeAssert(false && "Wrong grammar. Expecting rtreematch_predicate");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting rtreematch_predicate");
        return nullptr;
        }

    OSQLParseNode const* lhsNode = parseNode->getChild(0);
    unique_ptr<ValueExp> lhsExp = parse_row_value_constructor(ctx, lhsNode);
    if (!ctx.IsSuccess())
        return nullptr;

    //rest of predicate is in match_predicate_part_2 rule node
    OSQLParseNode const* part2Node = parseNode->getChild(1);

    const bool isNot = parse_sql_not(ctx, part2Node->getChild(0));
    const BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotMatch : BooleanSqlOperator::Match;

    //second child node is SQL_TOKEN_MATCH, and third therefore the rhs function call
    unique_ptr<FunctionCallExp> rhsExp = parse_fct_spec(ctx, part2Node->getChild(2));
    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<BooleanExp>(new BinaryBooleanExp(move(lhsExp), op, move(rhsExp)));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<SubqueryExp> ECSqlParser::parse_subquery (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if(!SQL_ISRULE(parseNode, subquery))
        {
        BeAssert (false && "Wrong grammar. Expecting subquery");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Wrong grammar. Expecting subquery");
        return nullptr;
        }

    unique_ptr<SelectStatementExp> compound_select = parse_select_statement (ctx, parseNode->getChild(1/*query_exp*/));
    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<SubqueryExp> (new SubqueryExp (std::move(compound_select)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<ValueExp> ECSqlParser::parse_row_value_constructor(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    return parse_value_exp(ctx, parseNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<ValueExpListExp> ECSqlParser::parse_row_value_constructor_commalist(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, row_value_constructor_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting row_value_constructor_commalist");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting row_value_constructor_commalist");
        return nullptr;
        }


    auto valueListExp = unique_ptr<ValueExpListExp>(new ValueExpListExp());
    const size_t childCount = parseNode->count();
    for (size_t i = 0; i < childCount; i++)
        {
        auto valueExp = parse_row_value_constructor(ctx, parseNode->getChild(i));
        if (!ctx.IsSuccess())
            {
            return nullptr;
            }

        valueListExp->AddValueExp(valueExp);
        }

    return valueListExp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BooleanSqlOperator ECSqlParser::parse_comparison(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISRULE(parseNode, comparison))
        {
        if (parseNode->count() == 4 /*SQL_TOKEN_IS sql_not SQL_TOKEN_DISTINCT SQL_TOKEN_FROM*/)
            {
            ctx.SetError(ECSqlStatus::InvalidECSql,"'IS [NOT] DISTINCT FROM' operator not supported in ECSQL.");
            return BooleanSqlOperator::LessThanOrEqualTo;
            }
        if (parseNode->count() == 2 /*SQL_TOKEN_IS sql_not*/)
            {
            auto sql_not = parse_sql_not(ctx, parseNode->getChild(1/*sql_not*/));
            return sql_not ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is;
            }
        }
    switch(parseNode->getNodeType())
        {
        case SQL_NODE_LESS: return BooleanSqlOperator::LessThan;
        case SQL_NODE_NOTEQUAL: return BooleanSqlOperator::NotEqualTo;
        case SQL_NODE_EQUAL: return BooleanSqlOperator::EqualTo;
        case SQL_NODE_GREAT: return BooleanSqlOperator::GreaterThan;
        case SQL_NODE_LESSEQ: return BooleanSqlOperator::LessThanOrEqualTo;
        case SQL_NODE_GREATEQ: return BooleanSqlOperator::GreaterThanOrEqualTo;
        }

    BeAssert (false && "'comparison' rule not handled");
    ctx.SetError (ECSqlStatus::ProgrammerError,"'comparison' rule not handled");
    return BooleanSqlOperator::LessThanOrEqualTo;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
SqlCompareListType ECSqlParser::parse_any_all_some(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    switch(parseNode->getTokenID())
        {
        case SQL_TOKEN_ANY: return SqlCompareListType::Any;
        case SQL_TOKEN_SOME: return SqlCompareListType::Some;
        case SQL_TOKEN_ALL: return SqlCompareListType::All;
        default:
            BeAssert (false && "Invalid grammar");
            ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar");
        }

    return SqlCompareListType::All;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
unique_ptr<ValueExp> ECSqlParser::parse_trueth_value(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    return parse_value_exp (ctx, parseNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlParser::parse_sql_not(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISRULE(parseNode, sql_not))
        return false;
    if (parseNode->getNodeType() == SQL_NODE_KEYWORD)
        return parseNode->getTokenID() == SQL_TOKEN_NOT;

    BeAssert (false && "Invalid grammar");
    ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar");
    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool ECSqlParser::parse_all(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (parseNode->getNodeType() == SQL_NODE_KEYWORD)
        return parseNode->getTokenID() == SQL_TOKEN_ALL;
    else
        //if ALL wasn't specified, it is an rule node (and no keyword node)
        return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<GroupByExp> ECSqlParser::parse_group_by_clause (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, opt_group_by_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting opt_group_by_clause");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting opt_group_by_clause"); 
        return nullptr; 
        }   

    if (parseNode->count () == 0) 
        return nullptr; //User never provided a GROUP BY clause 
    
    unique_ptr<ValueExpListExp> listExp = parse_value_exp_commalist(ctx, parseNode->getChild(2));
    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<GroupByExp>(new GroupByExp(move(listExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<HavingExp> ECSqlParser::parse_having_clause (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, opt_having_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting opt_having_clause");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting opt_having_clause"); 
        return nullptr; 
        }   

    if (parseNode->count () == 0) 
        return nullptr; //User never provided a HAVING clause 

    unique_ptr<BooleanExp> searchConditionExp = parse_search_condition(ctx, parseNode->getChild(1));
    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<HavingExp>(new HavingExp(move(searchConditionExp)));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
OrderBySpecExp::SortDirection ECSqlParser::parse_opt_asc_desc(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISRULE(parseNode, opt_asc_desc))        
        return OrderBySpecExp::SortDirection::NotSpecified;

    switch(parseNode->getTokenID())
        {
        case SQL_TOKEN_ASC:
            return OrderBySpecExp::SortDirection::Ascending;
        case SQL_TOKEN_DESC:
            return OrderBySpecExp::SortDirection::Descending;
        }

    ctx.SetError(ECSqlStatus::ProgrammerError, "Case not handled");
        return OrderBySpecExp::SortDirection::NotSpecified;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<OrderByExp> ECSqlParser::parse_order_by_clause (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, opt_order_by_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting opt_order_by_clause");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting opt_order_by_clause"); 
        return nullptr; 
        }   

    if (parseNode->count () == 0) 
        return nullptr; //User never provided a ORDER BY clause 

    std::vector<unique_ptr<OrderBySpecExp>> orderBySpecs;
    auto ordering_spec_commalist = parseNode->getChild (2 /*ordering_spec_commalist*/);
    for( size_t nPos = 0; nPos < ordering_spec_commalist->count(); nPos++)
        {
        auto ordering_spec = ordering_spec_commalist->getChild(nPos);
        auto row_value_constructor_elem = ordering_spec->getChild (0/*row_value_constructor_elem*/);
        std::unique_ptr<ComputedExp> sortValue;
        if (isPredicate(row_value_constructor_elem))
            sortValue = parse_search_condition(ctx, row_value_constructor_elem);
        else
            sortValue = parse_row_value_constructor(ctx, row_value_constructor_elem);
        
        auto sortDirection = parse_opt_asc_desc(ctx, ordering_spec->getChild (1/*opt_asc_desc*/));
        if (!ctx.IsSuccess())
            return nullptr;

        orderBySpecs.push_back(unique_ptr<OrderBySpecExp>(new OrderBySpecExp(sortValue, sortDirection)));
        }
    
    return unique_ptr<OrderByExp>(new OrderByExp(orderBySpecs));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  07/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<LimitOffsetExp> ECSqlParser::parse_limit_offset_clause (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    //If no limit clause was specified in the ECSQL, the parse node has the rule opt_limit_offset_clause, i.e. it is empty.
    //In this case no further processing needed.
    if (SQL_ISRULE(parseNode, opt_limit_offset_clause))        
        return nullptr; 

    //If a limit clause was specified in the ECSQL, the parse node has the rule limit_offset_clause
    if (!SQL_ISRULE(parseNode, limit_offset_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting limit_offset_clause");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting limit_offset_clause"); 
        return nullptr; 
        }   

    auto limitExpr = parse_value_exp (ctx, parseNode->getChild (1));

    auto offsetNode = parseNode->getChild (2);
    if (offsetNode == nullptr)
        {
        BeAssert (false && "Invalid grammar. Offset parse node is never expected to be null in limit_offset_clause");
        ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Offset parse node is never expected to be null in limit_offset_clause"); 
        return nullptr;
        }

    if (offsetNode->count () == 0)
        return unique_ptr<LimitOffsetExp> (new LimitOffsetExp (move(limitExpr)));
    else
        {
        auto offsetExpr = parse_value_exp (ctx, offsetNode->getChild (1));
        return unique_ptr<LimitOffsetExp> (new LimitOffsetExp (move (limitExpr), move(offsetExpr)));
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
SqlSetQuantifier ECSqlParser::parse_opt_all_distinct (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (SQL_ISTOKEN(parseNode, ALL)) 
        return SqlSetQuantifier::All;
    else if (SQL_ISTOKEN(parseNode, DISTINCT)) 
        return  SqlSetQuantifier::Distinct;       
    return SqlSetQuantifier::NotSpecified;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
SelectStatementExp::Operator  ECSqlParser::parse_compound_select_op (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (SQL_ISTOKEN (parseNode, UNION))
        return SelectStatementExp::Operator::Union;
    else if (SQL_ISTOKEN (parseNode, INTERSECT))
        return SelectStatementExp::Operator::Intersect;
    else if (SQL_ISTOKEN (parseNode, EXCEPT))
        return SelectStatementExp::Operator::Except;

    return SelectStatementExp::Operator::None;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<UnaryPredicateExp> ECSqlParser::parse_unary_predicate(ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, unary_predicate) || parseNode->count() != 1)
        {
        BeAssert(false && "Invalid grammar. Expecting unary_predicate");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting unary_predicate");
        return nullptr;
        }

    unique_ptr<ValueExp> valueExp = parse_value_exp(ctx, parseNode->getChild(0));
    if (!ctx.IsSuccess())
        return nullptr;

    return unique_ptr<UnaryPredicateExp>(new UnaryPredicateExp(move(valueExp)));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<SelectStatementExp> ECSqlParser::parse_select_statement (ECSqlParseContext& ctx, connectivity::OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE (parseNode, select_statement))
        {
        BeAssert(false && "Invalid grammar. Expecting select_statement with four child nodes");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting select_statement with four child nodes.");
        return nullptr;
        }

    if (parseNode->count () == 1)
        {
        auto single_select = parse_single_select_statement (ctx, parseNode->getChild (0));
        if (single_select == nullptr)
            return nullptr;

        return std::unique_ptr<SelectStatementExp> (new SelectStatementExp (std::move (single_select)));
        }
    else if (parseNode->count () == 4)
        {
        auto single_select = parse_single_select_statement (ctx, parseNode->getChild (0));
        if (single_select == nullptr)
            return nullptr;

        if (!single_select->IsCoreSelect())
            {
            ctx.SetError(ECSqlStatus::UserError, "SELECT in UNION must not containt ORDER BY OR LIMIT caluse. If one is used it should be in the end  of UNION statement. -> %s", single_select->ToECSql().c_str());
            return nullptr;
            }

        SelectStatementExp::Operator op = parse_compound_select_op (ctx, parseNode->getChild (1));
        bool all = parse_all (ctx, parseNode->getChild (2));
        auto compound_select = parse_select_statement (ctx, parseNode->getChild (3));
        if (compound_select == nullptr)
            return nullptr;

        return std::unique_ptr<SelectStatementExp> (new SelectStatementExp (std::move (single_select), op, all, std::move (compound_select)));
        }

    BeAssert (false && "Invalid grammar. Expecting select_statement with four child nodes or exactly one child");
    ctx.SetError (ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting select_statement with four child nodes or exactly one child");
    return nullptr;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
unique_ptr<ValueExp> ECSqlParser::parse_value_exp(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    unique_ptr<ValueExp> valueExp = nullptr;;
    if (parseNode->isRule())
        {
        switch(parseNode->getKnownRuleID())
            {
            case OSQLParseNode::cast_spec:
                valueExp = parse_cast_spec (ctx, parseNode); break;
            case OSQLParseNode::column_ref:
                valueExp = parse_column_ref(ctx, parseNode); break;
            case OSQLParseNode::num_value_exp:
                valueExp = parse_num_value_exp(ctx, parseNode); break;
            case OSQLParseNode::concatenation:
                valueExp = parse_concatenation(ctx, parseNode); break;
            case OSQLParseNode::datetime_value_exp:
                valueExp = parse_datetime_value_exp(ctx, parseNode); break;
            case OSQLParseNode::ecclassid_fct_spec:
                valueExp = parse_ecclassid_fct_spec (ctx, parseNode); break;
            case OSQLParseNode::factor:
                valueExp = parse_factor (ctx, parseNode); break;
            case OSQLParseNode::fold:
                valueExp = parse_fold (ctx, parseNode); break;
            case OSQLParseNode::general_set_fct: 
                valueExp = parse_general_set_fct(ctx, parseNode); break;
            case OSQLParseNode::fct_spec:
                valueExp = parse_fct_spec (ctx, parseNode); break;
            case OSQLParseNode::term:
                valueExp = parse_term(ctx, parseNode); break;   
            case OSQLParseNode::parameter:
                valueExp = parse_parameter(ctx, parseNode); break;   
            case OSQLParseNode::subquery:
                {
                auto subQuery = parse_subquery(ctx, parseNode);   
                if (subQuery != nullptr)//Must return just one column in select list
                    //We can tell that until we resolve all the columns
                        valueExp = unique_ptr<SubqueryValueExp>(new SubqueryValueExp(move (subQuery)));
                break;
                }
            case OSQLParseNode::value_exp_primary:
                valueExp = parse_value_exp_primary (ctx, parseNode); break;
            default:
                BeAssert (false && "Grammar rule not handled.");
                ctx.SetError (ECSqlStatus::ProgrammerError, "Grammar rule not handled.");
                return nullptr;

            };
        }
    else
        {
        //constant value
        Utf8CP value = nullptr;
        ECSqlTypeInfo dataType;
        switch(parseNode->getNodeType())
            {
            case SQL_NODE_INTNUM:
                value = parseNode->getTokenValue ().c_str ();
                dataType = ECSqlTypeInfo (PRIMITIVETYPE_Long);
                break;
            case SQL_NODE_APPROXNUM:
                value = parseNode->getTokenValue ().c_str ();
                dataType = ECSqlTypeInfo (PRIMITIVETYPE_Double);
                break;
            case SQL_NODE_STRING:
                value = parseNode->getTokenValue ().c_str ();
                dataType = ECSqlTypeInfo (PRIMITIVETYPE_String);
                break;
            case SQL_NODE_KEYWORD:
                {
                if (parseNode->getTokenID() == SQL_TOKEN_NULL)
                    {
                    value = "NULL";
                    dataType = ECSqlTypeInfo (ECSqlTypeInfo::Kind::Null);
                    }
                else if (parseNode->getTokenID() == SQL_TOKEN_TRUE)
                    {
                    value = "TRUE";
                    dataType = ECSqlTypeInfo (PRIMITIVETYPE_Boolean);
                    }
                else if (parseNode->getTokenID() == SQL_TOKEN_FALSE)
                    {
                    value = "FALSE";
                    dataType = ECSqlTypeInfo (PRIMITIVETYPE_Boolean);
                    }
                break;
                }
            default:
                BeAssert (false && "Node type not handled.");
                ctx.SetError (ECSqlStatus::ProgrammerError, "Node type not handled.");
                return nullptr;
            };

        valueExp = ConstantValueExp::Create (ctx, value, dataType);
        }

    if (!ctx.IsSuccess())
        return nullptr;

    return valueExp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
unique_ptr<ValueExpListExp> ECSqlParser::parse_value_exp_commalist (ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, value_exp_commalist))
        {
        BeAssert (false && "Invalid grammar. Expecting value_exp_commalist");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting value_expr_commalist");
        return nullptr;
        }

    auto valueListExp = unique_ptr<ValueExpListExp> (new ValueExpListExp ());
    const size_t childCount = parseNode->count ();
    for (size_t i = 0; i < childCount; i++)
        {
        auto valueExp = parse_value_exp (ctx, parseNode->getChild (i));
        if (!ctx.IsSuccess ())
            {
            return nullptr;
            }

        valueListExp->AddValueExp (valueExp);
        }

    return valueListExp;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
unique_ptr<ValueExpListExp> ECSqlParser::parse_values_or_query_spec(ECSqlParseContext& ctx, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, values_or_query_spec))
        {
        BeAssert (false && "Invalid grammar. Expecting values_or_query_spec");
        ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid grammar. Expecting values_or_query_spec");
        return nullptr;
        }

    //1st: VALUES, 2nd:(, 3rd: row_value_constructor_commalist, 4th:)
    BeAssert (parseNode->count () == 4);
    OSQLParseNode const* listNode = parseNode->getChild(2);
    return parse_row_value_constructor_commalist(ctx, listNode);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
