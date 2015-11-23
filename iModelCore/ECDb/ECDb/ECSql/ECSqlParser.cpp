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
BentleyStatus ECSqlParser::Parse (ECSqlParseTreePtr& ecsqlParseTree, ECDbCR ecdb, Utf8CP ecsql, IClassMap::View classView) const
    {
    ecsqlParseTree = nullptr;

    ScopedContext scopedContext (*this, ecdb, classView);
    //Parse statement
    Utf8String error;
    OSQLParser* ecsqlParser = GetSharedParser();
    OSQLParseNode* ecsqlParseTreeRaw = ecsqlParser->parseTree (error, ecsql);
    if (ecsqlParseTreeRaw == nullptr || !error.empty())
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, error.c_str ());
        return ERROR;
        }

    if (!ecsqlParseTreeRaw->isRule())
        {
        BeAssert (false && "ECSQL grammar has changed, but parser wasn't adopted.");
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL grammar has changed, but parser wasn't adopted.");
        return ERROR;
        }

    switch (ecsqlParseTreeRaw->getKnownRuleID())
        {
        case OSQLParseNode::insert_statement:
        {
        std::unique_ptr<InsertStatementExp> exp = nullptr;
        if (SUCCESS != parse_insert_statement(exp, ecsqlParseTreeRaw))
            return ERROR;

        ecsqlParseTree = move(exp);
        break;
        }

        case OSQLParseNode::update_statement_searched:
        {
        std::unique_ptr<UpdateStatementExp> exp = nullptr;
        if (SUCCESS != parse_update_statement_searched(exp, ecsqlParseTreeRaw))
            return ERROR;

        ecsqlParseTree = move(exp);
        break;
        }

        case OSQLParseNode::delete_statement_searched:
        {
        std::unique_ptr<DeleteStatementExp> exp = nullptr;
        if (SUCCESS != parse_delete_statement_searched(exp, ecsqlParseTreeRaw))
            return ERROR;

        ecsqlParseTree = move(exp);
        break;
        }

        case OSQLParseNode::select_statement:
        {
        std::unique_ptr<SelectStatementExp> exp = nullptr;
        if (SUCCESS != parse_select_statement(exp, ecsqlParseTreeRaw))
            return ERROR;

        ecsqlParseTree = move(exp);
        break;
        }

        case OSQLParseNode::manipulative_statement:
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "Manipulative statements are not supported.");
            return ERROR;

        default:
            BeAssert (false && "Not a valid statement");
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "Not a valid statement");
            return ERROR;
        };

    if (ecsqlParseTree == nullptr)
        {
        BeAssert(ecsqlParseTree != nullptr);
        return ERROR;
        }

    //resolve types and references now that first pass parsing is done and all nodes are available
    return m_context->FinalizeParsing(*ecsqlParseTree);
    }

//****************** Parsing SELECT statement ***********************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_single_select_statement(unique_ptr<SingleSelectStatementExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    SqlSetQuantifier opt_all_distinct;
    if (SUCCESS != parse_opt_all_distinct(opt_all_distinct, parseNode->getChild(1)))
        return ERROR;

    unique_ptr<SelectClauseExp> selectClauseExp = nullptr;
    if (SUCCESS != parse_selection(selectClauseExp, parseNode->getChild(2)))
        return ERROR;

    OSQLParseNode const* table_exp = parseNode->getChild(3);
    if (table_exp == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    unique_ptr<FromExp> fromExp = nullptr;
    if (SUCCESS != parse_from_clause(fromExp, table_exp->getChild(0)))
        return ERROR;

    unique_ptr<WhereExp> whereExp = nullptr;
    if (SUCCESS != parse_opt_where_clause(whereExp, table_exp->getChild(1)))
        return ERROR;

    unique_ptr<GroupByExp> groupByExp = nullptr;
    if (SUCCESS != parse_group_by_clause(groupByExp, table_exp->getChild(2)))
        return ERROR;

    unique_ptr<HavingExp> havingExp = nullptr;
    if (SUCCESS != parse_having_clause(havingExp, table_exp->getChild(3)))
        return ERROR;

    unique_ptr<OrderByExp> orderByExp = nullptr;
    if (SUCCESS != parse_order_by_clause(orderByExp, table_exp->getChild(5)))
        return ERROR;

    unique_ptr<LimitOffsetExp> limitOffsetExp = nullptr;
    if (SUCCESS != parse_limit_offset_clause(limitOffsetExp, table_exp->getChild(6)))
        return ERROR;

    unique_ptr<OptionsExp> optionsExp = nullptr;
    if (SUCCESS != parse_opt_ecsqloptions_clause(optionsExp, table_exp->getChild(9)))
        return ERROR;

    if (selectClauseExp == nullptr || fromExp == nullptr)
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL without select clause or from clause is invalid.");
        return ERROR;
        }

    exp = unique_ptr<SingleSelectStatementExp>(new SingleSelectStatementExp(
            opt_all_distinct,
            move(selectClauseExp),
            move(fromExp),
            move(whereExp),
            move(orderByExp),
            move(groupByExp),
            move(havingExp),
            move(limitOffsetExp),
            move(optionsExp)));

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_selection (unique_ptr<SelectClauseExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (SQL_ISRULE(parseNode, selection))
        {       
        auto n = parseNode->getChild(0);
        if (Exp::IsAsteriskToken (n->getTokenValue().c_str ()))
            {
            exp = unique_ptr<SelectClauseExp>(new SelectClauseExp());
            exp->AddProperty(std::unique_ptr<DerivedPropertyExp> (new DerivedPropertyExp(unique_ptr<PropertyNameExp>(new PropertyNameExp(Exp::ASTERISK_TOKEN)), nullptr)));
            return SUCCESS;
            }
        }

    if (!SQL_ISRULE(parseNode, scalar_exp_commalist))
        {
        BeAssert (false && "Wrong grammar");
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Wrong grammar");
        return ERROR;
        }

    unique_ptr<SelectClauseExp> selectClauseExp = unique_ptr<SelectClauseExp> (new SelectClauseExp());

    for (size_t n = 0; n < parseNode->count(); n++)
        {
        unique_ptr<DerivedPropertyExp> derivedPropExp = nullptr;
        BentleyStatus stat = parse_derived_column (derivedPropExp, parseNode->getChild (n));
        if (SUCCESS != stat)
            return stat;

        if (derivedPropExp != nullptr)
            selectClauseExp->AddProperty (move (derivedPropExp));
        }

    exp = move(selectClauseExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_derived_column (unique_ptr<DerivedPropertyExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, derived_column ))
        {
        BeAssert (false && "Wrong grammar");
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* first  = parseNode->getChild(0);
    OSQLParseNode const* opt_as_clause = parseNode->getChild(1);

    unique_ptr<ValueExp> valExp = nullptr;
    BentleyStatus stat = parse_value_exp (valExp, first);
    if (stat != SUCCESS)
        return stat;

    Utf8String columnAlias;
    if (opt_as_clause->count() > 0 )
        columnAlias = opt_as_clause->getChild(1)->getTokenValue();
    else
        columnAlias = opt_as_clause->getTokenValue();

    exp = unique_ptr<DerivedPropertyExp>(new DerivedPropertyExp (move (valExp), columnAlias.c_str ()));
    return SUCCESS;
    }

//****************** Parsing INSERT statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_insert_statement (unique_ptr<InsertStatementExp>& insertExp, connectivity::OSQLParseNode const* parseNode) const
    {
    insertExp = nullptr;
    //insert does not support polymorphic classes. Passing false therefore.
    unique_ptr<ClassNameExp> classNameExp = nullptr;
    BentleyStatus stat = parse_table_node(classNameExp, parseNode->getChild(2), false);
    if (SUCCESS != stat)
        return stat;

    unique_ptr<PropertyNameListExp> insertPropertyNameListExp = nullptr;
    stat = parse_opt_column_ref_commalist (insertPropertyNameListExp, parseNode->getChild(3));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ValueExpListExp> valuesOrQuerySpecExp = nullptr;
    stat = parse_values_or_query_spec (valuesOrQuerySpecExp, parseNode->getChild(4));
    if (SUCCESS != stat)
        return stat;

    insertExp = unique_ptr<InsertStatementExp> (new InsertStatementExp(classNameExp, insertPropertyNameListExp, valuesOrQuerySpecExp));
    return SUCCESS;
    }

//****************** Parsing UPDATE statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_update_statement_searched (unique_ptr<UpdateStatementExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    exp = nullptr;
    //rule: update_statement_searched: SQL_TOKEN_UPDATE table_ref SQL_TOKEN_SET assignment_commalist opt_where_clause
    unique_ptr<ClassRefExp> classRefExp = nullptr;
    BentleyStatus stat = parse_table_ref (classRefExp, parseNode->getChild (1));
    if (SUCCESS != stat)
        return stat;

    if (classRefExp->GetType () != Exp::Type::ClassName)
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL UPDATE statements only support ECClass references as target. Subqueries or join clauses are not supported.");
        return ERROR;
        }

    unique_ptr<AssignmentListExp> assignmentListExp = nullptr;
    stat = parse_assignment_commalist(assignmentListExp, parseNode->getChild(3));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<WhereExp> opt_where_clause = nullptr;
    stat = parse_opt_where_clause(opt_where_clause, parseNode->getChild(4));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<OptionsExp> opt_options_clause = nullptr;
    stat = parse_opt_ecsqloptions_clause(opt_options_clause, parseNode->getChild(5));
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<UpdateStatementExp> (new UpdateStatementExp (move (classRefExp), move (assignmentListExp), move (opt_where_clause), move(opt_options_clause)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_assignment_commalist (unique_ptr<AssignmentListExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    auto listExp = unique_ptr<AssignmentListExp> (new AssignmentListExp ());
    const size_t assignmentCount = parseNode->count ();
    for (size_t i = 0; i < assignmentCount; i++)
        {
        auto assignmentNode = parseNode->getChild (i);
        BeAssert (SQL_ISRULE (assignmentNode, assignment) && assignmentNode->count () == 3 && "Wrong ECSQL grammar. Expected rule assignment.");

        unique_ptr<PropertyNameExp> lhsExp = nullptr;
        BentleyStatus stat = parse_column_ref (lhsExp, assignmentNode->getChild (0));
        if (SUCCESS != stat)
            return stat;

        unique_ptr<ValueExp> rhsExp = nullptr;
        stat = parse_value_exp (rhsExp, assignmentNode->getChild (2));
        if (SUCCESS != stat)
            return stat;

        listExp->AddAssignmentExp (unique_ptr<AssignmentExp> (new AssignmentExp (move (lhsExp), move (rhsExp))));
        }

    exp = move (listExp);
    return SUCCESS;
    }

//****************** Parsing UPDATE statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_delete_statement_searched (unique_ptr<DeleteStatementExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    //rule: delete_statement_searched: SQL_TOKEN_DELETE SQL_TOKEN_FROM table_ref opt_where_clause
    unique_ptr<ClassRefExp> classRefExp = nullptr;
    BentleyStatus stat = parse_table_ref(classRefExp, parseNode->getChild(2));
    if (SUCCESS != stat)
        return stat;

    if (classRefExp->GetType () != Exp::Type::ClassName)
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL DELETE statements only support ECClass references as target. Subqueries or join clauses are not supported.");
        return ERROR;
        }

    unique_ptr<WhereExp> opt_where_clause = nullptr;
    stat = parse_opt_where_clause(opt_where_clause, parseNode->getChild(3));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<OptionsExp> opt_options_clause = nullptr;
    stat = parse_opt_ecsqloptions_clause(opt_options_clause, parseNode->getChild(4));
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<DeleteStatementExp> (new DeleteStatementExp (move (classRefExp), move (opt_where_clause), move(opt_options_clause)));
    return SUCCESS;
    }

//****************** Parsing common expression ***********************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_column (unique_ptr<PropertyNameExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (parseNode->getNodeType() != SQL_NODE_NAME)
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    PropertyPath propPath;
    propPath.Push (parseNode->getTokenValue().c_str());

    exp = unique_ptr<PropertyNameExp> (new PropertyNameExp(move (propPath)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_opt_column_ref_commalist (std::unique_ptr<PropertyNameListExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_column_ref_commalist))
        {
        BeAssert (false && "Invalid grammar. Expecting opt_column_ref_commalist");
        return ERROR;
        }

    const size_t childCount = parseNode->count ();
    if (childCount == 0) 
        return SUCCESS; //User never provided a insert column name list clause (no error)

    BeAssert (childCount == 3);
    //first and third nodes are ( and ). Second node is the the list node
    return parse_column_ref_commalist(exp, parseNode->getChild(1));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_opt_ecsqloptions_clause(std::unique_ptr<OptionsExp>& exp, connectivity::OSQLParseNode const* parseNode) const
     {
     exp = nullptr;
     if (SQL_ISRULE(parseNode, opt_ecsqloptions_clause))
         return SUCCESS; //this rule is hit if no options clause was given. So just return in that case

     if (!SQL_ISRULE(parseNode, ecsqloptions_clause))
         {
         BeAssert(false && "Wrong grammar. Expecting ecsqloptions_clause");
         return ERROR;
         }

     //first node is OPTIONS keyword, second node is option list
     OSQLParseNode const* optionListNode = parseNode->getChild(1);

     const size_t childCount = optionListNode->count();
     if (childCount == 0)
         return SUCCESS; //User never provided options

     auto optionsExp = unique_ptr<OptionsExp>(new OptionsExp());
     for (size_t i = 0; i < childCount; i++)
         {
         unique_ptr<OptionExp> optionExp = nullptr;
         BentleyStatus stat = parse_ecsqloption(optionExp, optionListNode->getChild(i));
         if (SUCCESS != stat)
             return stat;

         if (SUCCESS != optionsExp->AddOptionExp(move(optionExp)))
             return ERROR;
         }

     exp = std::move(optionsExp);
     return SUCCESS;
    }

 //-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_column_ref_commalist(std::unique_ptr<PropertyNameListExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, column_ref_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting column_ref_commalist");
        return ERROR;
        }

    const size_t columnCount = parseNode->count();
    auto listExp = unique_ptr<PropertyNameListExp>(new PropertyNameListExp());
    for (size_t i = 0; i < columnCount; i++)
        {
        unique_ptr<PropertyNameExp> propertyNameExp = nullptr;
        BentleyStatus stat = parse_column_ref(propertyNameExp, parseNode->getChild(i));
        if (SUCCESS != stat)
            return stat;

        listExp->AddPropertyNameExp(propertyNameExp);
        }

    exp = std::move(listExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    01/2014
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_fold (std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE (parseNode, fold))
        {
        BeAssert (false && "Wrong grammar. Expecting fold");
        return ERROR;
        }

    //first node is LOWER | UPPER, second is (, third is arg, fourth is )
    const size_t childCount = parseNode->count ();
    if (childCount != 4)
        {
        BeAssert(false && "fold is expected to have 4 children");
        return ERROR;
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
                GetIssueReporter().Report(ECDbIssueSeverity::Error, "Wrong grammar. Only LOWER or UPPER are valid function names for fold rule.");
                return ERROR;
                }
        }

    auto argNode = parseNode->getChild (2);
    unique_ptr<ValueExp> valueExp = nullptr;
    BentleyStatus stat = parse_value_exp(valueExp, argNode);
    if (SUCCESS != stat)
        return stat;

    unique_ptr<FoldFunctionCallExp> foldExp = unique_ptr<FoldFunctionCallExp>(new FoldFunctionCallExp(foldFunction));
    foldExp->AddArgument(move(valueExp));
    exp = move(foldExp);
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       01/2014
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_property_path (std::unique_ptr<PropertyNameExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE (parseNode, property_path))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
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
            return ERROR;
            }
        }

    exp = std::unique_ptr<PropertyNameExp> (new PropertyNameExp (move (propertyPath)));
    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_column_ref (std::unique_ptr<PropertyNameExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, column_ref ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    return parse_property_path (exp, parseNode->getFirst ());
    }

 
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_parameter (std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE (parseNode, parameter) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
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
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid grammar. Only : or ? allowed as parameter tokens");
            return ERROR;
            }
        }

    exp = unique_ptr<ValueExp> (new ParameterExp (paramName));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_term (std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, term ) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto operand_left  = parseNode->getChild(0);
    auto opNode = parseNode->getChild(1);
    auto operand_right = parseNode->getChild(2);

    unique_ptr<ValueExp> operand_left_expr = nullptr;
    BentleyStatus stat = parse_value_exp(operand_left_expr, operand_left);
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ValueExp> operand_right_expr = nullptr;
    stat = parse_value_exp(operand_right_expr, operand_right);
    if (SUCCESS != stat)
        return stat;

    Utf8StringCR opStr = opNode->getTokenValue();
    BinarySqlOperator op;
    if (opStr.Equals("*"))
        op = BinarySqlOperator::Multiply;
    else if(opStr.Equals("/"))
        op = BinarySqlOperator::Divide;
    else if (opStr.Equals("%"))
        op = BinarySqlOperator::Modulo;
    else
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    exp = unique_ptr<ValueExp>(new BinaryValueExp (move (operand_left_expr), op, move (operand_right_expr)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
// WIP_ECSQL: Implement Case operation also correct datatype list in sqlbison.y as
// per ECSQLTypes. We only support premitive type casting
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_cast_spec (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, cast_spec ) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto cast_operand = parseNode->getChild(2);
    auto cast_target = parseNode->getChild(4);

    unique_ptr<ValueExp> cast_operand_expr = nullptr;
    BentleyStatus stat = parse_value_exp(cast_operand_expr, cast_operand);
    if (SUCCESS != stat)
        return stat;

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
            return ERROR;
        }

    exp = unique_ptr<ValueExp>(new CastExp(move (cast_operand_expr), sqlType));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_fct_spec (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, fct_spec ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode* functionNameNode = parseNode->getChild (0);
    Utf8StringCR knownFunctionName = functionNameNode->getTokenValue ();
    if (knownFunctionName.empty())
        {
        const auto tokenId = functionNameNode->getTokenID ();
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function with token ID %d not yet supported.", tokenId);
        return ERROR;
        }

    if (GetPointCoordinateFunctionExp::IsPointCoordinateFunction(knownFunctionName))
        return parse_getpointcoordinate_fct_spec(exp, *parseNode, knownFunctionName);

    unique_ptr<FunctionCallExp> functionCallExp = unique_ptr<FunctionCallExp> (new FunctionCallExp (knownFunctionName.c_str()));
    //parse function args. (if child parse node count is < 4, function doesn't have args)
    if (parseNode->count() == 4)
        {
        OSQLParseNode* argumentsNode = parseNode->getChild(2);
        if (SQL_ISRULE(argumentsNode, function_args_commalist))
            {
            for (size_t i = 0; i < argumentsNode->count(); i++)
                {
                BentleyStatus stat = parse_and_add_functionarg(*functionCallExp, argumentsNode->getChild(i));
                if (SUCCESS != stat)
                    return stat;
                }
            }
        else
            {
            BentleyStatus stat = parse_and_add_functionarg(*functionCallExp, argumentsNode);
            if (SUCCESS != stat)
                return stat;
            }
        }

    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_getpointcoordinate_fct_spec(std::unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const& parseNode, Utf8StringCR functionName) const
    {
    if (parseNode.count() != 4)
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function %s requires exactly one argument.", functionName.c_str());
        return ERROR;
        }

    OSQLParseNode* argumentsNode = parseNode.getChild(2);
    if (SQL_ISRULE(argumentsNode, function_args_commalist))
        {
        if (argumentsNode->count() != 1)
            {
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "Function %s requires exactly one argument.", functionName.c_str());
            return ERROR;
            }

        argumentsNode = argumentsNode->getChild(0);
        }

    unique_ptr<ValueExp> argExp = nullptr;
    if (SUCCESS != parse_functionarg(argExp, *argumentsNode))
        return ERROR;

    exp = unique_ptr<ValueExp>(new GetPointCoordinateFunctionExp(functionName, std::move(argExp)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_and_add_functionarg(FunctionCallExp& functionCallExp, connectivity::OSQLParseNode const* argNode) const
    {
    unique_ptr<ValueExp> argument_expr = nullptr;
    BentleyStatus stat = parse_functionarg(argument_expr, *argNode);
    if (SUCCESS != stat)
        return stat;

    functionCallExp.AddArgument(move(argument_expr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2015
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_functionarg(unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const& argNode) const
    {
    return parse_result(exp, &argNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_general_set_fct(unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, general_set_fct) && parseNode->count() == 3)
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
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
                GetIssueReporter().Report(ECDbIssueSeverity::Error, "Unsupported standard SQL function with token ID %d", functionNameNode->getTokenID());
                return ERROR;
                }
        }

    //Following cover COUNT(ALL|DISTINCT funtion_arg) and AVG,MAX...(ALL|DISTINCT funtion_arg)
    OSQLParseNode* opt_all_distinctNode = parseNode->getChild(2);
    SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified;
    BentleyStatus stat = parse_opt_all_distinct(setQuantifier, opt_all_distinctNode);
    if (SUCCESS != stat)
        return stat;

    auto functionCallExp = unique_ptr<SetFunctionCallExp>(new SetFunctionCallExp(function, setQuantifier));

    if (function == SetFunctionCallExp::Function::Count &&
        Exp::IsAsteriskToken(parseNode->getChild(2)->getTokenValue().c_str()))
        {
        unique_ptr<ValueExp> argExp = nullptr;
        stat = LiteralValueExp::Create(argExp, *m_context, Exp::ASTERISK_TOKEN, ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies));
        if (SUCCESS != stat)
            return stat;

        functionCallExp->AddArgument(move(argExp));
        }
    else
        {
        stat = parse_and_add_functionarg(*functionCallExp, parseNode->getChild(3/*function_arg*/));
        if (SUCCESS != stat)
            return stat;
        }

    exp = move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_ecclassid_fct_spec (unique_ptr<ValueExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    const auto childNodeCount = parseNode->count ();
    if (!SQL_ISRULE (parseNode, ecclassid_fct_spec) || (childNodeCount != 3 && childNodeCount != 5))
        {
        BeAssert (false && "Wrong grammar. Expecting ecclassid_fct_spec.");
        return ERROR;
        }

    auto firstChildNode = parseNode->getChild (0);
    if (childNodeCount != 3)
        {
        std::unique_ptr<PropertyNameExp> prefixPath = nullptr;
        BentleyStatus stat = parse_property_path(prefixPath, firstChildNode);
        if (SUCCESS != stat)
            return stat;

        if (prefixPath->GetPropertyPath ().Size () == 1)
            {
            Utf8CP classAlias = prefixPath->GetPropertyPath ()[0].GetPropertyName();
            exp = std::unique_ptr<ECClassIdFunctionExp> (new ECClassIdFunctionExp (classAlias));
            return SUCCESS;
            }
        else
            {
            BeAssert (false && "Wrong grammar. Expecting <class-alias>.GetECClassId().");
            return ERROR;
            }
        }

    exp = unique_ptr<ValueExp> (new ECClassIdFunctionExp (nullptr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_ecsqloption(std::unique_ptr<OptionExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    const size_t childNodeCount = parseNode->count();
    BeAssert(childNodeCount == 1 || childNodeCount == 3);

    Utf8CP optionName = parseNode->getChild(0)->getTokenValue().c_str();
    Utf8String optionValue;
    if (childNodeCount == 3)
        {
        OSQLParseNode const* valNode = parseNode->getChild(2);
        BeAssert(valNode != nullptr);
        ECSqlTypeInfo dataType;
        if (SUCCESS != parse_literal(optionValue, dataType, *valNode))
            return ERROR;
        }

    exp = std::unique_ptr<OptionExp>(new OptionExp(optionName, optionValue.c_str()));
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_result (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {    
    return parse_value_exp(exp, parseNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_value_exp_primary (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, value_exp_primary ) && parseNode->count() == 3)
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    //This rule is expected to always have parentheses
    BeAssert(parseNode->getChild(0)->getTokenValue() == "(" &&
             parseNode->getChild(2)->getTokenValue() == ")");

    BentleyStatus stat = parse_value_exp(exp, parseNode->getChild(1));
    if (SUCCESS != stat)
        return stat;

    if (parseNode->getChild(0)->getTokenValue().Equals("("))
        exp->SetHasParentheses();

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_num_value_exp (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, num_value_exp ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto operand_left  = parseNode->getChild(0);
    auto opNode        = parseNode->getChild(1);
    auto operand_right = parseNode->getChild(2);

    unique_ptr<ValueExp> operand_left_expr = nullptr;
    BentleyStatus stat = parse_value_exp(operand_left_expr, operand_left);
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ValueExp> operand_right_expr = nullptr;
    stat = parse_value_exp(operand_right_expr, operand_right);
    if (SUCCESS != stat)
        return stat;

    BinarySqlOperator op;
    if (opNode->getTokenValue() == "+")
        op = BinarySqlOperator::Plus;
    else if (opNode->getTokenValue() == "-")
        op = BinarySqlOperator::Minus;
    else
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    exp = unique_ptr<ValueExp>(new BinaryValueExp(move (operand_left_expr), op, move (operand_right_expr)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_factor (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, factor ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto opNode = parseNode->getChild(0);
    auto operandNode = parseNode->getChild(1);

    unique_ptr<ValueExp> operand_expr = nullptr;
    BentleyStatus stat = parse_value_exp(operand_expr, operandNode);
    if (SUCCESS != stat)
        return stat;

    Utf8StringCR opStr = opNode->getTokenValue();
    UnarySqlOperator op = UnarySqlOperator::Plus;
    if (opStr.Equals ("+"))
        op = UnarySqlOperator::Plus;
    else if (opStr.Equals ("-"))
        op = UnarySqlOperator::Minus;
    else
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    exp = unique_ptr<UnaryValueExp> (new UnaryValueExp (operand_expr.release (), op));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_concatenation (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, concatenation))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto operand_left = parseNode->getChild(0);
    auto operand_right = parseNode->getChild(2);

    unique_ptr<ValueExp> operand_left_expr = nullptr;
    BentleyStatus stat = parse_value_exp(operand_left_expr, operand_left);
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ValueExp> operand_right_expr = nullptr;
    stat = parse_value_exp(operand_right_expr, operand_right);
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<ValueExp>(new BinaryValueExp(move (operand_left_expr), BinarySqlOperator::Concat, move (operand_right_expr)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_datetime_value_exp(unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, datetime_value_exp ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto datetime_term = parseNode->getByRule(OSQLParseNode::datetime_term);
    if (datetime_term == nullptr)
        return ERROR;

    return parse_datetime_term (exp, datetime_term);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_datetime_term (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, datetime_term ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto datetime_factor = parseNode->getByRule(OSQLParseNode::datetime_factor);
    if (datetime_factor == nullptr)
        return ERROR;

    return parse_datetime_factor (exp, datetime_factor);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_datetime_factor (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, datetime_factor) && (parseNode->count() == 1 || parseNode->count() == 2))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto datetime_primary = parseNode->getByRule(OSQLParseNode::datetime_primary);
    if (datetime_primary == nullptr)
        return ERROR;

    return parse_datetime_primary(exp, datetime_primary);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_datetime_primary (unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, datetime_primary ))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto datetime_value_fct = parseNode->getByRule(OSQLParseNode::datetime_value_fct);
    if (datetime_value_fct == nullptr)
        return ERROR;

    return parse_datetime_value_fct (exp, datetime_value_fct);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_datetime_value_fct(unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, datetime_value_fct ) && (parseNode->count() == 1 || parseNode->count() == 2))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    auto type = parseNode->getChild(0);
    if (parseNode->count() == 1 ) //Keyword
        {
        if (type->getTokenID() == SQL_TOKEN_CURRENT_DATE)
            return LiteralValueExp::Create (exp, *m_context, "CURRENT_DATE", ECSqlTypeInfo (ECN::PRIMITIVETYPE_DateTime));
        if (type->getTokenID() == SQL_TOKEN_CURRENT_TIMESTAMP)
            return LiteralValueExp::Create (exp, *m_context, "CURRENT_TIMESTAMP", ECSqlTypeInfo (ECN::PRIMITIVETYPE_DateTime));

        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Unrecognized keyword '%s'.", parseNode->getTokenValue().c_str());
        return ERROR;
        }

    auto unparsedDateOrTimestampValue = parseNode->getChild(1)->getTokenValue().c_str ();
    //WIP_ECSQL: Parse date value into a structure
    if (type->getTokenID () == SQL_TOKEN_DATE || type->getTokenID () == SQL_TOKEN_TIMESTAMP)
        return LiteralValueExp::Create (exp, *m_context, unparsedDateOrTimestampValue, ECSqlTypeInfo (ECN::PRIMITIVETYPE_DateTime));

    exp = nullptr;
    BeAssert (false && "Wrong grammar");
    return ERROR;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_from_clause (unique_ptr<FromExp>& exp, OSQLParseNode const* parseNode) const
    {
    unique_ptr<FromExp> from_clause = unique_ptr<FromExp>(new FromExp());
    OSQLParseNode* table_ref_commalist = parseNode->getChild(1);
    for (size_t n = 0; n < table_ref_commalist->count(); n++)
        {
        unique_ptr<ClassRefExp> classRefExp = nullptr;
        BentleyStatus stat = parse_table_ref(classRefExp, table_ref_commalist->getChild(n));
        if (stat != SUCCESS)
            return stat;

        stat = from_clause->TryAddClassRef(*m_context, move(classRefExp));
        if (stat != SUCCESS)
            return stat;
        }

    exp = move(from_clause);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_table_ref (unique_ptr<ClassRefExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (SQL_ISRULE(parseNode, qualified_join) || SQL_ISRULE(parseNode, relationship_join))
        {
        unique_ptr<JoinExp> joinExp = nullptr;
        BentleyStatus stat = parse_joined_table(joinExp, parseNode);
        if (SUCCESS == stat)
            exp = move(joinExp);

        return stat;
        }

    if (!SQL_ISRULE (parseNode, table_ref))
        {
        BeAssert (false && "Wrong grammar. Expecting table_name.");
        return ERROR;
        }

    OSQLParseNode* opt_only = parseNode->getChild (0);
    OSQLParseNode* second = parseNode->getChild (1);

    bool isPolymorphic = !(opt_only->getTokenID () == SQL_TOKEN_ONLY);
    if (SQL_ISRULE (second, table_node))
        {
        unique_ptr<ClassNameExp> classNameExp = nullptr;
        BentleyStatus stat = parse_table_node(classNameExp, second, isPolymorphic);
        if (SUCCESS != stat)
            return stat;

        auto table_primary_as_range_column = parseNode->getChild (2);

        if (table_primary_as_range_column->count () > 0)
            {
            OSQLParseNode* table_alias = table_primary_as_range_column->getChild (1);
            OSQLParseNode* opt_column_commalist = table_primary_as_range_column->getChild (2);
            if (opt_column_commalist->count () > 0)
                {
                BeAssert (false && "Range column not supported");
                return ERROR;
                }

            if (!table_alias->getTokenValue ().empty ())
                classNameExp->SetAlias (table_alias->getTokenValue ());
            }

        exp = std::move (classNameExp);
        return SUCCESS;
        }

    if (SQL_ISRULE(second, subquery))
        {
        unique_ptr<SubqueryExp> subqueryExp = nullptr;
        BentleyStatus stat = parse_subquery(subqueryExp, second);
        if (SUCCESS != stat)
            return stat;

        auto range_variable = parseNode->getChild(2/*range_variable*/);
        auto alias = Utf8String();
        if (range_variable->count() > 0)
            alias = range_variable->getChild(1/*SQL_TOKEN_NAME*/)->getTokenValue();
    
        exp = unique_ptr<ClassRefExp>(new SubqueryRefExp(move(subqueryExp), alias, isPolymorphic));
        return SUCCESS;
        }

    BeAssert (false && "Case not supported");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_joined_table (unique_ptr<JoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    switch(parseNode->getKnownRuleID())
        {
        case OSQLParseNode::qualified_join:
            return parse_qualified_join (exp, parseNode);
        case OSQLParseNode::cross_union:
        {
        unique_ptr<CrossJoinExp> joinExp = nullptr;
        BentleyStatus stat = parse_cross_union(joinExp, parseNode);
        if (SUCCESS == stat)
            exp = move(joinExp);

        return stat;
        }
        case OSQLParseNode::relationship_join:
        {
        unique_ptr<RelationshipJoinExp> joinExp = nullptr;
        BentleyStatus stat = parse_relationship_join(joinExp, parseNode);
        if (SUCCESS == stat)
            exp = move(joinExp);

        return stat;
        }

        default:
            BeAssert(false && "Wrong grammar");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_relationship_join (unique_ptr<RelationshipJoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, relationship_join))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    unique_ptr<ClassRefExp> from_table_ref = nullptr;
    BentleyStatus stat = parse_table_ref(from_table_ref, parseNode->getChild(0/*table_ref*/));
    if (SUCCESS != stat)
        return stat;

    ECSqlJoinType join_type = ECSqlJoinType::InnerJoin;
    stat = parse_join_type(join_type, parseNode->getChild(1/*join_type*/));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ClassRefExp> to_table_ref = nullptr;
    stat = parse_table_ref(to_table_ref, parseNode->getChild(3/*table_ref*/));
    if (SUCCESS != stat)
        return stat;

    //TODO: need to decide whether we support ONLY in USING clause.
    unique_ptr<ClassNameExp> table_node = nullptr;
    stat = parse_table_node(table_node, parseNode->getChild(5/*table_node*/), true);
    if (SUCCESS != stat)
        return stat;

    auto op_relationship_direction = parseNode->getChild(6/*op_relationship_direction*/);

    if (!(join_type == ECSqlJoinType::InnerJoin || join_type == ECSqlJoinType::None))
        {
        BeAssert(false && "Supported join type is INNER JOIN");
        return ERROR;
        }

    JoinDirection direction = JoinDirection::Implied;
    if (op_relationship_direction->getTokenID() == SQL_TOKEN_FORWARD)
        direction = JoinDirection::Forward;
    else if (op_relationship_direction->getTokenID() == SQL_TOKEN_REVERSE)
        direction = JoinDirection::Reverse;

    exp = unique_ptr<RelationshipJoinExp>(new RelationshipJoinExp(move (from_table_ref), move (to_table_ref),  move (table_node), direction));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_qualified_join(unique_ptr<JoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, qualified_join))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    unique_ptr<ClassRefExp> from_table_ref = nullptr;
    BentleyStatus stat = parse_table_ref(from_table_ref, parseNode->getChild(0/*table_ref*/));
    if (SUCCESS != stat)
        return stat;

    ECSqlJoinType joinType = ECSqlJoinType::InnerJoin;
    auto key = parseNode->getChild(1);
    if (key->getTokenID() == SQL_TOKEN_NATURAL)
        {
        stat = parse_join_type(joinType, parseNode->getChild(2/*join_type*/));
        if (SUCCESS != stat)
            return stat;

        unique_ptr<ClassRefExp> to_table_ref = nullptr;
        stat = parse_table_ref(to_table_ref, parseNode->getChild(4/*table_ref*/));
        if (SUCCESS != stat)
            return stat;

        exp = unique_ptr<JoinExp>(new NaturalJoinExp(move(from_table_ref), move(to_table_ref), joinType));
        return SUCCESS;
        }

    stat = parse_join_type(joinType, parseNode->getChild(1/*join_type*/));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ClassRefExp> to_table_ref = nullptr;
    stat = parse_table_ref(to_table_ref, parseNode->getChild(3/*table_ref*/));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<JoinSpecExp> join_spec = nullptr;
    stat = parse_join_spec(join_spec, parseNode->getChild(4/*join_spec*/));
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<JoinExp>(new QualifiedJoinExp(move(from_table_ref), move(to_table_ref), joinType, move(join_spec)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_cross_union (unique_ptr<CrossJoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, cross_union))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    unique_ptr<ClassRefExp> from_table_ref = nullptr;
    BentleyStatus stat = parse_table_ref(from_table_ref, parseNode->getChild(0/*table_ref*/));
    if (SUCCESS != stat)
        return stat;

    unique_ptr<ClassRefExp> to_table_ref = nullptr;
    stat = parse_table_ref(to_table_ref, parseNode->getChild(3/*table_ref*/));
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<CrossJoinExp>(new CrossJoinExp(move (from_table_ref), move (to_table_ref)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_join_type (ECSqlJoinType& joinType, OSQLParseNode const* parseNode) const
    {
    joinType = ECSqlJoinType::None;

    if (SQL_ISRULE(parseNode, join_type))
        {
        if (parseNode->count() == 0) //default value
            {
            joinType = ECSqlJoinType::InnerJoin;
            return SUCCESS;
            }

        auto first = parseNode->getChild(0);
        if (first->getTokenID() == SQL_TOKEN_INNER)
            {
            joinType = ECSqlJoinType::InnerJoin;
            return SUCCESS;
            }
        }

    if (SQL_ISRULE(parseNode, outer_join_type))
        return parse_outer_join_type(joinType, parseNode);

    BeAssert (false && "Invalid grammar. Expected JoinType");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_outer_join_type (ECSqlJoinType& joinType, OSQLParseNode const* parseNode) const
    {
    joinType = ECSqlJoinType::None;

    if (!SQL_ISRULE(parseNode, outer_join_type))
        {
        BeAssert (false && "Invalid grammar. Expected OuterJoinType");
        return ERROR;
        }

    auto n = parseNode->getChild(0);
    switch(n->getTokenID())
        {
            case SQL_TOKEN_LEFT: joinType = ECSqlJoinType::LeftOuterJoin; break;
            case SQL_TOKEN_RIGHT: joinType = ECSqlJoinType::RightOuterJoin; break;
            case SQL_TOKEN_FULL: joinType = ECSqlJoinType::FullOuterJoin; break;
            default:
                BeAssert(false && "Invalid grammar. Expected LEFT, RIGHT or FULL");
                return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_join_spec (unique_ptr<JoinSpecExp>& exp, OSQLParseNode const* parseNode) const
    {
    switch(parseNode->getKnownRuleID())
        {
        case OSQLParseNode::join_condition:
        {
        unique_ptr<JoinConditionExp> joinCondExp = nullptr;
        BentleyStatus stat = parse_join_condition(joinCondExp, parseNode);
        if (stat == SUCCESS)
            exp = move(joinCondExp);

        return stat;
        }
        case OSQLParseNode::named_columns_join:
        {
        unique_ptr<NamedPropertiesJoinExp> namedPropJoinExp = nullptr;
        BentleyStatus stat = parse_named_columns_join(namedPropJoinExp, parseNode);
        if (stat == SUCCESS)
            exp = move(namedPropJoinExp);

        return stat;

        }

        default:
            BeAssert(false && "Wrong grammar");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_join_condition (unique_ptr<JoinConditionExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, join_condition))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }

    unique_ptr<BooleanExp> search_condition = nullptr;
    BentleyStatus stat = parse_search_condition(search_condition, parseNode->getChild(1/*search_condition*/));
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<JoinConditionExp>(new JoinConditionExp(move (search_condition)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_named_columns_join (unique_ptr<NamedPropertiesJoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, named_columns_join))
        {
        BeAssert (false && "Wrong grammar");
        return ERROR;
        }
    
    exp = unique_ptr<NamedPropertiesJoinExp>(new NamedPropertiesJoinExp());
    auto column_commalist = parseNode->getChild(2/*column_commalist*/); 
    for(size_t i =0; i <column_commalist->count(); i++)
        exp->Append(column_commalist->getChild(i)->getTokenValue());

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_table_node(unique_ptr<ClassNameExp>& exp, OSQLParseNode const* parseNode, bool isPolymorphic) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, table_node))
        {
        BeAssert (false && "Wrong grammar. Expecting table_node");
        return ERROR;
        }

    BentleyStatus stat;
    OSQLParseNode const* first = parseNode->getChild(0);
    Utf8CP className = nullptr;
    Utf8CP schemaName = nullptr;
    Utf8CP catalogName = nullptr;
    switch(first->getKnownRuleID())
        {
        case OSQLParseNode::table_name:
            {
            stat = parse_table_name (className, first);
            break;
            }
        case OSQLParseNode::schema_name:  
            {
            stat = parse_schema_name (schemaName, className, first);
            break;
            }
        case OSQLParseNode::catalog_name: 
            {
            stat = parse_catalog_name (catalogName, schemaName, className, first);
            break;
            }
        default:
            BeAssert (false && "Wrong Grammar. Expecting table_name, schema_name or catalog_name");
            return ERROR;

        };

    if (SUCCESS != stat)
        return stat;

    shared_ptr<ClassNameExp::Info> classNameExpInfo = nullptr;
    stat = m_context->TryResolveClass(classNameExpInfo, schemaName, className);
    if (SUCCESS != stat)
        return stat;

    exp = unique_ptr<ClassNameExp> (new ClassNameExp(className, schemaName, catalogName, classNameExpInfo, isPolymorphic));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_table_name (Utf8CP& className, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, table_name))
        {
        BeAssert (false && "Wrong grammar. Expecting table_name");
        return ERROR;
        }

    className = parseNode->getChild(0)->getTokenValue().c_str ();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_schema_name(Utf8CP& schemaName, Utf8CP& className, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, schema_name))
        {
        BeAssert (false && "Wrong grammar. Expecting schema_name");
        return ERROR;
        }

    OSQLParseNode* schemaNameNode = parseNode->getChild(0);
    OSQLParseNode* tableNameNode  = parseNode->getChild(2);

    BentleyStatus stat = parse_table_name (className, tableNameNode);
    if (stat != SUCCESS)
        return stat;

    schemaName = schemaNameNode->getTokenValue().c_str ();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_catalog_name(Utf8CP& catalogName, Utf8CP& schemaName, Utf8CP& className, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, catalog_name))
        {
        BeAssert (false && "Wrong grammar. Expecting catalog_name");
        return ERROR;
        }

    OSQLParseNode* catalogNameNode = parseNode->getChild(0);
    OSQLParseNode* schemaNameNode  = parseNode->getChild(2);

    BentleyStatus stat = parse_schema_name (schemaName, className, schemaNameNode);
    if (SUCCESS != stat)
        return stat;

    catalogName = catalogNameNode->getTokenValue().c_str ();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_opt_where_clause (unique_ptr<WhereExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (SQL_ISRULE(parseNode, opt_where_clause))
        return SUCCESS; //this rule is hit if no where clause was given. So just return in that case

    if (!SQL_ISRULE(parseNode, where_clause))
        {
        BeAssert (false && "Wrong grammar. Expecting where_clause");
        return ERROR;
        }

    unique_ptr<BooleanExp> search_condition = nullptr;
    BentleyStatus stat = parse_search_condition(search_condition, parseNode->getChild(1/*search_condition*/));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<WhereExp>(new WhereExp(move (search_condition)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_search_condition(unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    const auto rule = parseNode->getKnownRuleID();
    switch (rule)
        {
            case OSQLParseNode::search_condition:
            {
            unique_ptr<BooleanExp> op1 = nullptr;
            BentleyStatus stat = parse_search_condition(op1, parseNode->getChild(0/*search_condition*/));
            if (stat != SUCCESS)
                return stat;

            //auto op = parseNode->getChild(1/*SQL_TOKEN_OR*/);
            unique_ptr<BooleanExp> op2 = nullptr;
            stat = parse_search_condition(op2, parseNode->getChild(2/*boolean_tern*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new BinaryBooleanExp(move(op1), BooleanSqlOperator::Or, move(op2)));
            return SUCCESS;
            }
            case OSQLParseNode::boolean_term:
            {
            unique_ptr<BooleanExp> op1 = nullptr;
            BentleyStatus stat = parse_search_condition(op1, parseNode->getChild(0/*search_condition*/));
            if (stat != SUCCESS)
                return stat;

            unique_ptr<BooleanExp> op2 = nullptr;
            stat = parse_search_condition(op2, parseNode->getChild(2/*boolean_tern*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new BinaryBooleanExp(move(op1), BooleanSqlOperator::And, move(op2)));
            return SUCCESS;
            }

            case OSQLParseNode::boolean_factor:
            {
            unique_ptr<BooleanExp> operandValueExp = nullptr;
            BentleyStatus stat = parse_search_condition(operandValueExp, parseNode->getChild(1/*boolean_test*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new BooleanFactorExp(move(operandValueExp), true));
            return SUCCESS;
            }

            case OSQLParseNode::boolean_test:
            {
            unique_ptr<BooleanExp> op1 = nullptr;
            BentleyStatus stat = parse_search_condition(op1, parseNode->getChild(0/*boolean_primary*/));
            if (stat != SUCCESS)
                return stat;

            //auto is = parseNode->getChild(1/*SQL_TOKEN_IS*/);
            bool isNot = false;
            stat = parse_sql_not(isNot, parseNode->getChild(2/*sql_not*/));
            if (stat != SUCCESS)
                return stat;

            unique_ptr<ValueExp> truth_value_expr = nullptr;
            stat = parse_trueth_value(truth_value_expr, parseNode->getChild(3/*truth_value*/));
            if (stat != SUCCESS)
                return stat;

            // X IS [NOT] NULL|TRUE|FALSE|UNKNOWN
            exp = unique_ptr<BooleanExp>(
                new BinaryBooleanExp(
                    move(op1),
                    isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is,
                    move(truth_value_expr)));
            return SUCCESS;
            }

            case OSQLParseNode::boolean_primary:
            {
            BeAssert(parseNode->count() == 3);
            BentleyStatus stat = parse_search_condition(exp, parseNode->getChild(1/*search_condition*/));
            if (stat != SUCCESS)
                return stat;

            exp->SetHasParentheses();
            return SUCCESS;
            }

            case OSQLParseNode::unary_predicate:
                return parse_unary_predicate(exp, parseNode);

            case OSQLParseNode::comparison_predicate:
            {
            if (parseNode->count() == 3 /*row_value_constructor comparison row_value_constructor*/)
                {
                unique_ptr<ValueExp> op1 = nullptr;
                BentleyStatus stat = parse_row_value_constructor(op1, parseNode->getChild(0/*row_value_constructor*/));
                if (stat != SUCCESS)
                    return stat;

                BooleanSqlOperator op = BooleanSqlOperator::And;
                stat = parse_comparison(op, parseNode->getChild(1/*comparison*/));
                if (stat != SUCCESS)
                    return stat;

                unique_ptr<ValueExp> op2 = nullptr;
                stat = parse_row_value_constructor(op2, parseNode->getChild(2/*row_value_constructor*/));
                if (stat != SUCCESS)
                    return stat;

                exp = unique_ptr<BooleanExp>(new BinaryBooleanExp(move(op1), op, move(op2)));
                return SUCCESS;
                }

            break;
            }

            case OSQLParseNode::between_predicate:
            {
            unique_ptr<ValueExp> lhsOperand = nullptr;
            BentleyStatus stat = parse_row_value_constructor(lhsOperand, parseNode->getChild(0/*row_value_constructor*/));
            if (stat != SUCCESS)
                return stat;

            auto between_predicate_part_2 = parseNode->getChild(1/*between_predicate_part_2*/);
            bool isNot = false;
            stat = parse_sql_not(isNot, between_predicate_part_2->getChild(0/*sql_not*/));
            if (stat != SUCCESS)
                return stat;

            const BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotBetween : BooleanSqlOperator::Between;

            unique_ptr<ValueExp> lowerBound = nullptr;
            stat = parse_row_value_constructor(lowerBound, between_predicate_part_2->getChild(2/*row_value_constructor*/));
            if (stat != SUCCESS)
                return stat;

            unique_ptr<ValueExp> upperBound = nullptr;
            stat = parse_row_value_constructor(upperBound, between_predicate_part_2->getChild(4/*row_value_constructor*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new BinaryBooleanExp(move(lhsOperand), op, std::unique_ptr<ValueExp>(new BetweenRangeValueExp(move(lowerBound), move(upperBound)))));
            return SUCCESS;
            }

            case OSQLParseNode::all_or_any_predicate:
            {
            unique_ptr<ValueExp> op1 = nullptr;
            BentleyStatus stat = parse_row_value_constructor(op1, parseNode->getChild(0/*comparison*/));
            if (stat != SUCCESS)
                return stat;

            auto quantified_comparison_predicate_part_2 = parseNode->getChild(1/*quantified_comparison_predicate_part_2*/);

            BooleanSqlOperator comparison = BooleanSqlOperator::EqualTo;
            stat = parse_comparison(comparison, quantified_comparison_predicate_part_2->getChild(0/*sql_not*/));
            if (stat != SUCCESS)
                return stat;

            SqlCompareListType any_all_some = SqlCompareListType::All;
            stat = parse_any_all_some(any_all_some, quantified_comparison_predicate_part_2->getChild(1/*any_all_some*/));
            if (stat != SUCCESS)
                return stat;

            unique_ptr<SubqueryExp> subquery = nullptr;
            stat = parse_subquery(subquery, quantified_comparison_predicate_part_2->getChild(4/*row_value_constructor*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new AllOrAnyExp(move(op1), comparison, any_all_some, move(subquery)));
            return SUCCESS;
            }

            case OSQLParseNode::existence_test:
            {
            unique_ptr<SubqueryExp> subquery = nullptr;
            BentleyStatus stat = parse_subquery(subquery, parseNode->getChild(1/*subquery*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new SubqueryTestExp(SubqueryTestOperator::Exists, move(subquery)));
            return SUCCESS;
            }

            case OSQLParseNode::unique_test:
            {
            unique_ptr<SubqueryExp> subquery = nullptr;
            BentleyStatus stat = parse_subquery(subquery, parseNode->getChild(1/*subquery*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new SubqueryTestExp(SubqueryTestOperator::Unique, move(subquery)));
            return SUCCESS;
            }

            case OSQLParseNode::test_for_null:
            {
            unique_ptr<ValueExp> row_value_constructor = nullptr;
            BentleyStatus stat = parse_row_value_constructor(row_value_constructor, parseNode->getChild(0/*row_value_constructor*/));
            if (stat != SUCCESS)
                return stat;

            auto null_predicate_part_2 = parseNode->getChild(1/*row_value_constructor*/);
            bool isNot = false;
            stat = parse_sql_not(isNot, null_predicate_part_2->getChild(1/*sql_not*/));
            if (stat != SUCCESS)
                return stat;

            unique_ptr<ValueExp> nullExp = nullptr;
            stat = parse_value_exp(nullExp, null_predicate_part_2->getChild(2/*NULL*/));
            if (stat != SUCCESS)
                return stat;

            exp = unique_ptr<BooleanExp>(new BinaryBooleanExp(move(row_value_constructor), isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is,
                                                              move(nullExp)));
            return SUCCESS;
            }

            case OSQLParseNode::in_predicate:
                return parse_in_predicate(exp, parseNode);

            case OSQLParseNode::like_predicate:
                return parse_like_predicate(exp, parseNode);

            case OSQLParseNode::rtreematch_predicate:
                return parse_rtreematch_predicate(exp, parseNode);
        }

    BeAssert(false && "Invalid grammar");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlParser::IsPredicate(OSQLParseNode const* parseNode)
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
BentleyStatus ECSqlParser::parse_in_predicate (unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    if(!SQL_ISRULE (parseNode, in_predicate))
        {
        BeAssert (false && "Wrong grammar. Expecting in_predicate");
        return ERROR;
        }

    const auto firstChildNode = parseNode->getChild (0);
    if (SQL_ISRULE (firstChildNode, in_predicate_part_2))
        {
        BeAssert (false);
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "IN predicate without left-hand side property not supported.");
        return ERROR;
        }

    //first item is row value ctor node
    unique_ptr<ValueExp> lhsExp = nullptr;
    BentleyStatus stat = parse_row_value_constructor(lhsExp, firstChildNode);
    if (stat != SUCCESS)
        return stat;

    auto inOperator = BooleanSqlOperator::In;
    unique_ptr<ComputedExp> rhsExp = nullptr;
    stat = parse_in_predicate_part_2(rhsExp, inOperator, parseNode->getChild(1));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<BooleanExp> (new BinaryBooleanExp (move (lhsExp), inOperator, move(rhsExp)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_in_predicate_part_2 (unique_ptr<ComputedExp>& exp, BooleanSqlOperator& inOperator, OSQLParseNode const* parseNode) const
    {
    //in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value
    if (!SQL_ISRULE (parseNode, in_predicate_part_2))
        {
        BeAssert (false && "Invalid grammar. Expecting in_predicate_part_2");
        return ERROR;
        }

    bool isNot = false;
    BentleyStatus stat = parse_sql_not(isNot, parseNode->getChild(0));
    if (stat != SUCCESS)
        return stat;

    inOperator = isNot ? BooleanSqlOperator::NotIn : BooleanSqlOperator::In;

    //third item is the predicate value (second item is IN token)
    auto in_predicate_valueNode = parseNode->getChild (2);

    OSQLParseNode* in_predicate_valueFirstChildNode = nullptr;
    if (SQL_ISRULE((in_predicate_valueFirstChildNode = in_predicate_valueNode->getChild(0)), subquery))
        {
        unique_ptr<ValueExp> valExp = nullptr;
        stat = parse_value_exp(valExp, in_predicate_valueFirstChildNode);
        exp = move(valExp);
        return stat;
        }

    //if no subquery it must be '(' value_exp_commalist ')'. Safety check is done in parse_value_exp_commalist method
    unique_ptr<ValueExpListExp> valueExpListExp = nullptr;
    stat = parse_value_exp_commalist (valueExpListExp, in_predicate_valueNode->getChild (1));
    exp = move(valueExpListExp);
    return stat;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_like_predicate (unique_ptr<BooleanExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if(!SQL_ISRULE (parseNode, like_predicate))
        {
        BeAssert (false && "Wrong grammar. Expecting like_predicate");
        return ERROR;
        }
    
    //first item is row value ctor node
    const auto firstChildNode = parseNode->getChild (0);
    unique_ptr<ValueExp> lhsExp = nullptr;
    BentleyStatus stat = parse_row_value_constructor(lhsExp, firstChildNode);
    if (stat != SUCCESS)
        return stat;

    unique_ptr<ComputedExp> rhsExp = nullptr;
    BooleanSqlOperator likeOperator = BooleanSqlOperator::Like;
    stat = parse_like_predicate_part_2(rhsExp, likeOperator, parseNode->getChild(1));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<BooleanExp> (new BinaryBooleanExp (move (lhsExp), likeOperator, move(rhsExp)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_like_predicate_part_2 (unique_ptr<ComputedExp>& exp, BooleanSqlOperator& likeOperator, OSQLParseNode const* parseNode) const
    {
    //character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape
    //other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape
    if (!SQL_ISRULE (parseNode, character_like_predicate_part_2) && !SQL_ISRULE (parseNode, other_like_predicate_part_2))
        {
        BeAssert (false && "Invalid grammar. Expecting character_like_predicate_part_2 or other_like_predicate_part_2");
        return ERROR;
        }

    bool isNot = false;
    BentleyStatus stat = parse_sql_not(isNot, parseNode->getChild(0));
    if (stat != SUCCESS)
        return stat;

    likeOperator = isNot ? BooleanSqlOperator::NotLike : BooleanSqlOperator::Like;

    //third item is value_exp_primary or string_value_exp value (second item is LIKE token)
    auto valueExpNode = parseNode->getChild (2);
    unique_ptr<ValueExp> rhsExp = nullptr;
    stat = parse_value_exp(rhsExp, valueExpNode);
    if (stat != SUCCESS)
        return stat;

    //fourth item is escape clause. Escape clause node always exists. If no escape was specified, the node is empty
    unique_ptr<ValueExp> escapeExp = nullptr;
    auto escapeClauseNode = parseNode->getChild (3);
    const size_t escapeChildNodeCount = escapeClauseNode->count ();
    //no nodes means no escape clause
    if (escapeChildNodeCount > 0)
        {
        if (escapeChildNodeCount != 2)
            {
            BeAssert (false && "Invalid grammar. Corrupt opt_escape expression");
            return ERROR;
            }

        //second child node has escape expression (first node is ESCAPE token)
        stat = parse_value_exp (escapeExp, escapeClauseNode->getChild (1));
        if (stat != SUCCESS)
            return stat;
        }

    exp = unique_ptr<ComputedExp> (new LikeRhsValueExp (move (rhsExp), move (escapeExp)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    05/2015
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_rtreematch_predicate(std::unique_ptr<BooleanExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, rtreematch_predicate))
        {
        BeAssert(false && "Wrong grammar. Expecting rtreematch_predicate");
        return ERROR;
        }

    OSQLParseNode const* lhsNode = parseNode->getChild(0);
    unique_ptr<ValueExp> lhsExp = nullptr;
    BentleyStatus stat = parse_row_value_constructor(lhsExp, lhsNode);
    if (stat != SUCCESS)
        return stat;

    //rest of predicate is in match_predicate_part_2 rule node
    OSQLParseNode const* part2Node = parseNode->getChild(1);

    bool isNot = false;
    stat = parse_sql_not(isNot, part2Node->getChild(0));
    if (stat != SUCCESS)
        return stat;

    const BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotMatch : BooleanSqlOperator::Match;

    //second child node is SQL_TOKEN_MATCH, and third therefore the rhs function call
    unique_ptr<ValueExp> rhsExp = nullptr;
    stat = parse_fct_spec(rhsExp, part2Node->getChild(2));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<BooleanExp>(new BinaryBooleanExp(move(lhsExp), op, move(rhsExp)));
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_subquery (unique_ptr<SubqueryExp>& exp, OSQLParseNode const* parseNode) const
    {
    if(!SQL_ISRULE(parseNode, subquery))
        {
        BeAssert (false && "Wrong grammar. Expecting subquery");
        return ERROR;
        }

    unique_ptr<SelectStatementExp> compound_select = nullptr;
    BentleyStatus stat = parse_select_statement(compound_select, parseNode->getChild(1/*query_exp*/));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<SubqueryExp> (new SubqueryExp (std::move(compound_select)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_row_value_constructor(unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    return parse_value_exp(exp, parseNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                04/2015
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_row_value_constructor_commalist(unique_ptr<ValueExpListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, row_value_constructor_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting row_value_constructor_commalist");
        return ERROR;
        }

    auto valueListExp = unique_ptr<ValueExpListExp>(new ValueExpListExp());
    const size_t childCount = parseNode->count();
    for (size_t i = 0; i < childCount; i++)
        {
        unique_ptr<ValueExp> valueExp = nullptr;
        BentleyStatus stat = parse_row_value_constructor(valueExp, parseNode->getChild(i));
        if (stat != SUCCESS)
            return stat;

        valueListExp->AddValueExp(valueExp);
        }

    exp = move(valueListExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_comparison(BooleanSqlOperator& op, OSQLParseNode const* parseNode) const
    {
    op = BooleanSqlOperator::LessThanOrEqualTo;

    if (SQL_ISRULE(parseNode, comparison))
        {
        if (parseNode->count() == 4 /*SQL_TOKEN_IS sql_not SQL_TOKEN_DISTINCT SQL_TOKEN_FROM*/)
            {
            GetIssueReporter().Report(ECDbIssueSeverity::Error,"'IS [NOT] DISTINCT FROM' operator not supported in ECSQL.");
            return ERROR;
            }
        if (parseNode->count() == 2 /*SQL_TOKEN_IS sql_not*/)
            {
            bool isNot = false;
            BentleyStatus stat = parse_sql_not(isNot, parseNode->getChild(1/*sql_not*/));
            if (stat != SUCCESS)
                return stat;

            op = isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is;
            return SUCCESS;
            }
        }

    switch(parseNode->getNodeType())
        {
            case SQL_NODE_LESS: op = BooleanSqlOperator::LessThan; break;
        case SQL_NODE_NOTEQUAL: op = BooleanSqlOperator::NotEqualTo; break;
        case SQL_NODE_EQUAL: op = BooleanSqlOperator::EqualTo; break;
        case SQL_NODE_GREAT: op = BooleanSqlOperator::GreaterThan; break;
        case SQL_NODE_LESSEQ: op = BooleanSqlOperator::LessThanOrEqualTo; break;
        case SQL_NODE_GREATEQ: op = BooleanSqlOperator::GreaterThanOrEqualTo; break;
        default:
            BeAssert(false && "'comparison' rule not handled");
            return ERROR;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
 BentleyStatus ECSqlParser::parse_any_all_some(SqlCompareListType& compareListType, OSQLParseNode const* parseNode) const
    {
    compareListType = SqlCompareListType::All;
    switch(parseNode->getTokenID())
        {
        case SQL_TOKEN_ANY: 
            compareListType = SqlCompareListType::Any; 
            return SUCCESS;
        case SQL_TOKEN_SOME: 
            compareListType = SqlCompareListType::Some;
            return SUCCESS;
        case SQL_TOKEN_ALL: 
            compareListType = SqlCompareListType::All;
            return SUCCESS;
        default:
            BeAssert (false && "Invalid grammar");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_trueth_value(unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    return parse_value_exp (exp, parseNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_sql_not(bool& isNot, OSQLParseNode const* parseNode) const
    {
    isNot = false;

    if (SQL_ISRULE(parseNode, sql_not))
        {
        isNot = false;
        return SUCCESS;
        }

    if (parseNode->getNodeType() == SQL_NODE_KEYWORD)
        {
        isNot = parseNode->getTokenID() == SQL_TOKEN_NOT;
        return SUCCESS;
        }

    BeAssert (false && "Invalid grammar");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_all(bool& isAll, OSQLParseNode const* parseNode) const
    {
    if (parseNode->getNodeType() == SQL_NODE_KEYWORD)
        isAll = parseNode->getTokenID() == SQL_TOKEN_ALL;
    else
        {
        //if ALL wasn't specified, it is a rule node (and no keyword node)
        isAll = false;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_group_by_clause (unique_ptr<GroupByExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, opt_group_by_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting opt_group_by_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS; //User never provided a GROUP BY clause 
    
    unique_ptr<ValueExpListExp> listExp = nullptr;
    BentleyStatus stat = parse_value_exp_commalist(listExp, parseNode->getChild(2));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<GroupByExp>(new GroupByExp(move(listExp)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_having_clause (unique_ptr<HavingExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, opt_having_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting opt_having_clause");
        return ERROR;
        }

    if (parseNode->count () == 0) 
        return SUCCESS; //User never provided a HAVING clause 

    unique_ptr<BooleanExp> searchConditionExp = nullptr;
    BentleyStatus stat = parse_search_condition(searchConditionExp, parseNode->getChild(1));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<HavingExp>(new HavingExp(move(searchConditionExp)));
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
 BentleyStatus ECSqlParser::parse_opt_asc_desc(OrderBySpecExp::SortDirection& sortDirection, OSQLParseNode const* parseNode) const
    {
    sortDirection = OrderBySpecExp::SortDirection::NotSpecified;
    if (SQL_ISRULE(parseNode, opt_asc_desc))
        return SUCCESS; //not specified

    switch(parseNode->getTokenID())
        {
        case SQL_TOKEN_ASC:
            sortDirection = OrderBySpecExp::SortDirection::Ascending;
            return SUCCESS;
        case SQL_TOKEN_DESC:
            sortDirection = OrderBySpecExp::SortDirection::Descending;
            return SUCCESS;
        default:
            BeAssert(false);
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_order_by_clause (unique_ptr<OrderByExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, opt_order_by_clause))        
        { 
        BeAssert (false && "Invalid grammar. Expecting opt_order_by_clause");
        return ERROR;
        }

    if (parseNode->count () == 0) 
        return SUCCESS; //User never provided a ORDER BY clause 

    std::vector<unique_ptr<OrderBySpecExp>> orderBySpecs;
    auto ordering_spec_commalist = parseNode->getChild (2 /*ordering_spec_commalist*/);
    for( size_t nPos = 0; nPos < ordering_spec_commalist->count(); nPos++)
        {
        auto ordering_spec = ordering_spec_commalist->getChild(nPos);
        auto row_value_constructor_elem = ordering_spec->getChild (0/*row_value_constructor_elem*/);
        std::unique_ptr<ComputedExp> sortValue = nullptr;
        BentleyStatus stat;
        if (IsPredicate(row_value_constructor_elem))
            {
            unique_ptr<BooleanExp> predExp = nullptr;
            stat = parse_search_condition(predExp, row_value_constructor_elem);
            sortValue = move(predExp);
            }
        else
            {
            unique_ptr<ValueExp> valueExp = nullptr;
            stat = parse_row_value_constructor(valueExp, row_value_constructor_elem);
            sortValue = move(valueExp);
            }
        
        if (stat != SUCCESS)
            return stat;

        OrderBySpecExp::SortDirection sortDirection = OrderBySpecExp::SortDirection::NotSpecified;
        stat = parse_opt_asc_desc(sortDirection, ordering_spec->getChild(1/*opt_asc_desc*/));
        if (stat != SUCCESS)
            return stat;

        orderBySpecs.push_back(unique_ptr<OrderBySpecExp>(new OrderBySpecExp(sortValue, sortDirection)));
        }
    
    exp = unique_ptr<OrderByExp>(new OrderByExp(orderBySpecs));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  07/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_limit_offset_clause(unique_ptr<LimitOffsetExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    //If no limit clause was specified in the ECSQL, the parse node has the rule opt_limit_offset_clause, i.e. it is empty.
    //In this case no further processing needed.
    if (SQL_ISRULE(parseNode, opt_limit_offset_clause))
        return SUCCESS;

    //If a limit clause was specified in the ECSQL, the parse node has the rule limit_offset_clause
    if (!SQL_ISRULE(parseNode, limit_offset_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting limit_offset_clause");
        return ERROR;
        }

    unique_ptr<ValueExp> limitExpr = nullptr;
    BentleyStatus stat = parse_value_exp(limitExpr, parseNode->getChild(1));
    if (stat != SUCCESS)
        return stat;

    auto offsetNode = parseNode->getChild(2);
    if (offsetNode == nullptr)
        {
        BeAssert(false && "Invalid grammar. Offset parse node is never expected to be null in limit_offset_clause");
        return ERROR;
        }

    if (offsetNode->count() == 0)
        {
        exp = unique_ptr<LimitOffsetExp>(new LimitOffsetExp(move(limitExpr)));
        return SUCCESS;
        }

    unique_ptr<ValueExp> offsetExpr = nullptr;
    stat = parse_value_exp(offsetExpr, offsetNode->getChild(1));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<LimitOffsetExp>(new LimitOffsetExp(move(limitExpr), move(offsetExpr)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_literal(Utf8StringR literalVal, ECSqlTypeInfo& dataType, connectivity::OSQLParseNode const& parseNode) const
    {
    //constant value
    literalVal = nullptr;
    switch (parseNode.getNodeType())
        {
            case SQL_NODE_INTNUM:
                literalVal.assign(parseNode.getTokenValue());
                dataType = ECSqlTypeInfo(PRIMITIVETYPE_Long);
                break;
            case SQL_NODE_APPROXNUM:
                literalVal.assign(parseNode.getTokenValue());
                dataType = ECSqlTypeInfo(PRIMITIVETYPE_Double);
                break;
            case SQL_NODE_STRING:
                literalVal.assign(parseNode.getTokenValue());
                dataType = ECSqlTypeInfo(PRIMITIVETYPE_String);
                break;
            case SQL_NODE_KEYWORD:
            {
            if (parseNode.getTokenID() == SQL_TOKEN_NULL)
                {
                literalVal.assign("NULL");
                dataType = ECSqlTypeInfo(ECSqlTypeInfo::Kind::Null);
                }
            else if (parseNode.getTokenID() == SQL_TOKEN_TRUE)
                {
                literalVal.assign("TRUE");
                dataType = ECSqlTypeInfo(PRIMITIVETYPE_Boolean);
                }
            else if (parseNode.getTokenID() == SQL_TOKEN_FALSE)
                {
                literalVal.assign("FALSE");
                dataType = ECSqlTypeInfo(PRIMITIVETYPE_Boolean);
                }
            break;
            }
            default:
                BeAssert(false && "Node type not handled.");
                return ERROR;
        };

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_opt_all_distinct (SqlSetQuantifier& setQuantifier, OSQLParseNode const* parseNode) const
    {
    if (SQL_ISTOKEN(parseNode, ALL)) 
        setQuantifier = SqlSetQuantifier::All;
    else if (SQL_ISTOKEN(parseNode, DISTINCT)) 
        setQuantifier = SqlSetQuantifier::Distinct;
    else
        setQuantifier = SqlSetQuantifier::NotSpecified;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_compound_select_op(SelectStatementExp::Operator& op, connectivity::OSQLParseNode const* parseNode) const
    {
    if (SQL_ISTOKEN(parseNode, UNION))
        op = SelectStatementExp::Operator::Union;
    else if (SQL_ISTOKEN(parseNode, INTERSECT))
        op = SelectStatementExp::Operator::Intersect;
    else if (SQL_ISTOKEN(parseNode, EXCEPT))
        op = SelectStatementExp::Operator::Except;
    else
        op = SelectStatementExp::Operator::None;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    04/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_unary_predicate(unique_ptr<BooleanExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, unary_predicate) || parseNode->count() != 1)
        {
        BeAssert(false && "Invalid grammar. Expecting unary_predicate");
        return ERROR;
        }

    unique_ptr<ValueExp> valueExp = nullptr;
    BentleyStatus stat = parse_value_exp(valueExp, parseNode->getChild(0));
    if (stat != SUCCESS)
        return stat;

    exp = unique_ptr<BooleanExp>(new UnaryPredicateExp(move(valueExp)));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_select_statement (std::unique_ptr<SelectStatementExp>& exp, connectivity::OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE (parseNode, select_statement))
        {
        BeAssert(false && "Invalid grammar. Expecting select_statement with four child nodes");
        return ERROR;
        }

    if (parseNode->count () == 1)
        {
        unique_ptr<SingleSelectStatementExp> single_select = nullptr;
        if (SUCCESS != parse_single_select_statement(single_select, parseNode->getChild(0)))
            return ERROR;

        exp = std::unique_ptr<SelectStatementExp> (new SelectStatementExp (std::move (single_select)));
        return SUCCESS;
        }
    else if (parseNode->count () == 4)
        {
        unique_ptr<SingleSelectStatementExp> single_select = nullptr;
        if (SUCCESS != parse_single_select_statement (single_select, parseNode->getChild (0)))
            return ERROR;

        if (!single_select->IsCoreSelect())
            {
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "SELECT statement in UNION must not contain ORDER BY or LIMIT clause: %s", single_select->ToECSql().c_str());
            return ERROR;
            }

        SelectStatementExp::Operator op = SelectStatementExp::Operator::None;
        if (SUCCESS != parse_compound_select_op(op, parseNode->getChild(1)))
            return ERROR;

        bool isAll = false;
        if (SUCCESS !=parse_all(isAll, parseNode->getChild(2)))
            return ERROR;

        unique_ptr<SelectStatementExp> compound_select = nullptr;
        if (SUCCESS != parse_select_statement(compound_select, parseNode->getChild(3)))
            return ERROR;

        exp = std::unique_ptr<SelectStatementExp> (new SelectStatementExp (std::move (single_select), op, isAll, std::move (compound_select)));
        return SUCCESS;
        }

    BeAssert (false && "Invalid grammar. Expecting select_statement with four child nodes or exactly one child");
    return ERROR;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::parse_value_exp(unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    BeAssert(parseNode != nullptr);
    if (parseNode->isRule())
        {
        switch (parseNode->getKnownRuleID())
            {
                case OSQLParseNode::cast_spec:
                    return parse_cast_spec(valueExp, parseNode);
                case OSQLParseNode::column_ref:
                {
                unique_ptr<PropertyNameExp> propNameExp = nullptr;
                BentleyStatus stat = parse_column_ref(propNameExp, parseNode);
                valueExp = move(propNameExp);
                return stat;
                }
                case OSQLParseNode::num_value_exp:
                    return parse_num_value_exp(valueExp, parseNode);
                case OSQLParseNode::concatenation:
                    return parse_concatenation(valueExp, parseNode);
                case OSQLParseNode::datetime_value_exp:
                    return parse_datetime_value_exp(valueExp, parseNode);
                case OSQLParseNode::ecclassid_fct_spec:
                    return parse_ecclassid_fct_spec(valueExp, parseNode);
                case OSQLParseNode::factor:
                    return parse_factor(valueExp, parseNode);
                case OSQLParseNode::fold:
                    return parse_fold(valueExp, parseNode);
                case OSQLParseNode::general_set_fct:
                    return parse_general_set_fct(valueExp, parseNode);
                case OSQLParseNode::fct_spec:
                    return parse_fct_spec(valueExp, parseNode);
                case OSQLParseNode::term:
                    return parse_term(valueExp, parseNode);
                case OSQLParseNode::parameter:
                    return parse_parameter(valueExp, parseNode);
                case OSQLParseNode::subquery:
                {
                unique_ptr<SubqueryExp> subQueryExp = nullptr;
                //Must return just one column in select list
                //We can tell that until we resolve all the columns
                BentleyStatus stat = parse_subquery(subQueryExp, parseNode);
                if (SUCCESS != stat)
                    return stat;

                valueExp = unique_ptr<SubqueryValueExp>(new SubqueryValueExp(move(subQueryExp)));
                return SUCCESS;
                }
                case OSQLParseNode::value_exp_primary:
                    return parse_value_exp_primary(valueExp, parseNode);

                default:
                    BeAssert(false && "Grammar rule not handled.");
                    return ERROR;

            };
        }

    //constant value
    Utf8String value;
    ECSqlTypeInfo dataType;
    if (SUCCESS != parse_literal(value, dataType, *parseNode))
        return ERROR;

    return LiteralValueExp::Create(valueExp, *m_context, value.c_str(), dataType);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_value_exp_commalist (unique_ptr<ValueExpListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, value_exp_commalist))
        {
        BeAssert (false && "Invalid grammar. Expecting value_exp_commalist");
        return ERROR;
        }

    auto valueListExp = unique_ptr<ValueExpListExp> (new ValueExpListExp ());
    const size_t childCount = parseNode->count ();
    for (size_t i = 0; i < childCount; i++)
        {
        unique_ptr<ValueExp> valueExp = nullptr;
        BentleyStatus stat = parse_value_exp(valueExp, parseNode->getChild(i));
        if (SUCCESS != stat)
            return stat;

        valueListExp->AddValueExp (valueExp);
        }

    exp = move(valueListExp);
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::parse_values_or_query_spec(unique_ptr<ValueExpListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, values_or_query_spec))
        {
        BeAssert (false && "Invalid grammar. Expecting values_or_query_spec");
        return ERROR;
        }

    //1st: VALUES, 2nd:(, 3rd: row_value_constructor_commalist, 4th:)
    BeAssert (parseNode->count () == 4);
    OSQLParseNode const* listNode = parseNode->getChild(2);
    return parse_row_value_constructor_commalist(exp, listNode);
    }

//-----------------------------------------------------------------------------------------
// ECSqlParseContext
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParseContext::FinalizeParsing(Exp& rootExp)
    {
    if (SUCCESS != rootExp.FinalizeParsing(*this))
        return ERROR;

    for (ParameterExp* parameterExp : m_parameterExpList)
        {
        if (!parameterExp->TryDetermineParameterExpType(*this, *parameterExp))
            parameterExp->SetDefaultTargetExpInfo();
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlParseContext::PushFinalizeParseArg(void const* const arg)
    {
    m_finalizeParseArgs.push_back(arg);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void const* const ECSqlParseContext::GetFinalizeParseArg() const
    {
    if (m_finalizeParseArgs.empty())
        return nullptr;

    return m_finalizeParseArgs[m_finalizeParseArgs.size() - 1];
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlParseContext::PopFinalizeParseArg()
    {
    m_finalizeParseArgs.pop_back();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParseContext::TryResolveClass(shared_ptr<ClassNameExp::Info>& classNameExpInfo, Utf8StringCR schemaNameOrPrefix, Utf8StringCR className)
    {
    ECClassCP resolvedClass = m_ecdb.Schemas().GetECClass(schemaNameOrPrefix.c_str(), className.c_str(), ResolveSchema::AutoDetect);

    if (resolvedClass == nullptr)
        {
        if (schemaNameOrPrefix.empty())
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECClass '%s' does not exist. Try using fully qualified class name: <schema name>.<class name>.", className.c_str());
        else
            GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECClass '%s.%s' does not exist.", schemaNameOrPrefix.c_str(), className.c_str());

        return ERROR;
        }

    auto key = resolvedClass->GetSchema().GetName() + ":" + resolvedClass->GetName();
    auto search = m_classNameExpInfoList.find(key);
    if (search != m_classNameExpInfoList.end())
        {
        classNameExpInfo = search->second;
        return SUCCESS;
        }

    IClassMap const* map = m_ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*resolvedClass);
    if (map == nullptr)
        return ERROR;

    auto policy = ECDbPolicyManager::GetClassPolicy(*map, IsValidInECSqlPolicyAssertion::Get());
    if (!policy.IsSupported())
        {
        GetIssueReporter().Report(ECDbIssueSeverity::Error, "Invalid ECClass '%s': %s", className.c_str(), policy.GetNotSupportedMessage());
        return ERROR;
        }

    IClassMap const& classMapView = map->GetView(m_classMapViewMode);
    classNameExpInfo = ClassNameExp::Info::Create(classMapView);
    m_classNameExpInfoList[key] = classNameExpInfo;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlParseContext::TrackECSqlParameter(ParameterExp& parameterExp)
    {
    m_parameterExpList.push_back(&parameterExp);

    const bool isNamedParameter = parameterExp.IsNamedParameter();
    Utf8CP paramName = isNamedParameter ? parameterExp.GetParameterName() : nullptr;

    if (isNamedParameter)
        {
        auto it = m_ecsqlParameterNameToIndexMapping.find(paramName);
        if (it != m_ecsqlParameterNameToIndexMapping.end())
            return it->second;
        }

    m_currentECSqlParameterIndex++;
    if (isNamedParameter)
        m_ecsqlParameterNameToIndexMapping[paramName] = m_currentECSqlParameterIndex;

    return m_currentECSqlParameterIndex;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlParseContext::GetSubclasses(ClassListById& classes, ECClassCR ecClass)
    {
    for (auto derivedClass : Schemas().GetDerivedECClasses(const_cast<ECClassR>(ecClass)))
        {
        if (classes.find(derivedClass->GetId()) == classes.end())
            {
            classes[derivedClass->GetId()] = derivedClass;
            GetSubclasses(classes, *derivedClass);
            }
        }
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlParseContext::GetConstraintClasses(ClassListById& classes, ECRelationshipConstraintCR constraintEnd, bool* containAnyClass)
    {
    if (containAnyClass)
        *containAnyClass = false;
    for (auto ecClass : constraintEnd.GetClasses())
        {
        if (containAnyClass && !(*containAnyClass) && ecClass->GetName() == "AnyClass" && ecClass->GetSchema().GetName() == "Bentley_Standard_Classes")
            *containAnyClass = true;

        if (classes.find(ecClass->GetId()) == classes.end())
            {
            classes[ecClass->GetId()] = ecClass;
            if (constraintEnd.GetIsPolymorphic())
                GetSubclasses(classes, *ecClass);
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlParseContext::IsEndClassOfRelationship(ECClassCR searchClass, ECRelationshipEnd searchEnd, ECRelationshipClassCR relationshipClass)
    {
    ECRelationshipConstraintCR constraintEnd =
        (searchEnd == ECRelationshipEnd::ECRelationshipEnd_Source) ? relationshipClass.GetSource() : relationshipClass.GetTarget();

    bmap<ECClassId, ECClassCP> classes;
    bool containAnyClass;
    GetConstraintClasses(classes, constraintEnd, &containAnyClass);
    if (containAnyClass)
        return true;

    return classes.find(searchClass.GetId()) != classes.end();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlParseContext::GenerateAlias()
    {
    Utf8String alias;
    alias.Sprintf("K%d", m_aliasCount++);
    return alias;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
