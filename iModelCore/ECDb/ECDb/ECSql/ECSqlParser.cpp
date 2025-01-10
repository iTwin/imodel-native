/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "Parser/SqlNode.h"
#include "Parser/SqlParse.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace connectivity;
USING_NAMESPACE_BENTLEY_EC

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+-----g----------+---------------+--------
Utf8String ECSqlParseContext::ClassViewPrepareStack::GetStackAsString() const {
    Utf8String stack;
    for (auto i = 0; i < m_ctx.m_viewPrepareStack.size(); ++i) {
        if (i > 0) {
            stack.append(" -> ");
        }
        stack.append(m_ctx.m_viewPrepareStack[i]->GetFullName());
    }
    return stack;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlParseContext::ClassViewPrepareStack::ClassViewPrepareStack(ECSqlParseContext& ctx, ECN::ECClassCR viewClass) : m_ctx(ctx) {
    m_ctx.m_viewPrepareStack.push_back(&viewClass);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlParseContext::ClassViewPrepareStack::IsOnStack(ECN::ECClassCR viewClass) const {
    for (auto i = 0; i< m_ctx.m_viewPrepareStack.size() - 1; ++i) {
        if (m_ctx.m_viewPrepareStack[i] == &viewClass)
            return true;
    }
    return false;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlParseContext::ClassViewPrepareStack::~ClassViewPrepareStack() { BeAssert(!m_ctx.m_viewPrepareStack.empty()); m_ctx.m_viewPrepareStack.pop_back(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<Exp> ECSqlParser::Parse(ECDbCR ecdb, Utf8CP ecsql, IssueDataSource const& issues, const ECSqlParser* parentParser) const {
    if (parentParser == this) {
        BeAssert(false && "parent parser cannot be same as current");
        return nullptr;
    }
    ScopedContext scopedContext(*this, ecdb , issues, parentParser);
    //Parse statement
    Utf8String error;
    OSQLParser parser;
    auto parseTree = parser.parseTree(error, ecsql);

    if (parseTree == nullptr || !error.empty()) {
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0471, "Failed to parse ECSQL '%s': %s", ecsql, error.c_str());
        return nullptr;
    }

    if (!parseTree->isRule()) {
        BeAssert(false && "ECSQL grammar has changed, but parser wasn't adopted.");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0472, "ECSQL grammar has changed, but parser wasn't adopted.");
        return nullptr;
    }

    std::unique_ptr<Exp> exp = nullptr;
    switch (parseTree->getKnownRuleID()) {
        case OSQLParseNode::pragma: {
            std::unique_ptr<PragmaStatementExp> pragmaExp = nullptr;
            if (SUCCESS != ParsePragmaStatement(pragmaExp, *parseTree))
                return nullptr;

            exp = std::move(pragmaExp);
            break;
        }
        case OSQLParseNode::insert_statement: {
            std::unique_ptr<InsertStatementExp> insertExp = nullptr;
            if (SUCCESS != ParseInsertStatement(insertExp, *parseTree))
                return nullptr;

            exp = std::move(insertExp);
            break;
        }
        case OSQLParseNode::update_statement_searched: {
            std::unique_ptr<UpdateStatementExp> updateExp = nullptr;
            if (SUCCESS != ParseUpdateStatementSearched(updateExp, *parseTree))
                return nullptr;

            exp = std::move(updateExp);
            break;
        }
        case OSQLParseNode::delete_statement_searched: {
            std::unique_ptr<DeleteStatementExp> deleteExp = nullptr;
            if (SUCCESS != ParseDeleteStatementSearched(deleteExp, *parseTree))
                return nullptr;

            exp = std::move(deleteExp);
            break;
        }
        case OSQLParseNode::select_statement: {
            std::unique_ptr<SelectStatementExp> selectExp = nullptr;
            if (SUCCESS != ParseSelectStatement(selectExp, *parseTree))
                return nullptr;

            exp = std::move(selectExp);
            break;
        }
        case OSQLParseNode::cte: {
            std::unique_ptr<CommonTableExp> cteExp;
            if (SUCCESS != ParseCTE(cteExp, parseTree))
                return nullptr;

            exp = std::move(cteExp);
            break;
        }
        case OSQLParseNode::manipulative_statement:
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0473, "Manipulative statements are not supported.");
            return nullptr;

        default:
            BeAssert(false && "Not a valid statement");
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0474, "Not a valid statement");
            return nullptr;
        };


    if (exp == nullptr) {
        BeAssert(false);
        return nullptr;
    }

    //resolve types and references now that first pass parsing is done and all nodes are available
    if (SUCCESS != m_context->FinalizeParsing(*exp))
        return nullptr;

    return exp;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseCTEBlock(std::unique_ptr<CommonTableBlockExp>& exp, OSQLParseNode const* parseNode, bool const& isRecursive) const {
    BeAssert(parseNode != nullptr);
    if (!SQL_ISRULE(parseNode, cte_table_name))
        return ERROR;

    if(parseNode->count() == 8)
    {
        auto blockName = parseNode->getChild(0)->getTokenValue();
        auto pColumnList = parseNode->getChild(2);
        auto pSelectStmt = parseNode->getChild(6);

        std::unique_ptr<SelectStatementExp> selectStmt;
        if (SUCCESS != ParseSelectStatement(selectStmt, *pSelectStmt))
            return ERROR;
        // Grab column names in block definition
        std::vector<Utf8String> columns;
        for (size_t i = 0; i < pColumnList->count(); ++i) {
            columns.push_back(pColumnList->getChild(i)->getTokenValue());
        }

        if(columns.size() == 0)
            return ERROR;
        /* Defered test
        if (selectStmt->GetSelection()->GetChildrenCount() != columns.size()) {
            error
        }
        */
        exp = std::make_unique<CommonTableBlockExp>(blockName.c_str(), columns, std::move(selectStmt));
        return SUCCESS;
    }
    if(parseNode->count() == 5)
    {
        if(isRecursive)
            return ERROR;
        auto blockName = parseNode->getChild(0)->getTokenValue();
        auto pSelectStmt = parseNode->getChild(3);

        std::unique_ptr<SelectStatementExp> selectStmt;
        if (SUCCESS != ParseSelectStatement(selectStmt, *pSelectStmt))
            return ERROR;
        
        exp = std::make_unique<CommonTableBlockExp>(blockName.c_str(), std::move(selectStmt));
        return SUCCESS;
    }
    return ERROR;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseCTE(std::unique_ptr<CommonTableExp>& exp, OSQLParseNode const* parseNode) const {
    BeAssert(parseNode != nullptr);
    if (!SQL_ISRULE(parseNode, cte))
        return ERROR;

    auto recursive = parseNode->getChild(1)->getTokenID() == SQL_TOKEN_RECURSIVE;
    auto pCteBlockList = parseNode->getChild(2);
    auto pSelectStmt = parseNode->getChild(3);

    std::vector<std::unique_ptr<CommonTableBlockExp>> blockList;
    for (size_t i=0; i< pCteBlockList->count(); ++i) {
        std::unique_ptr<CommonTableBlockExp> cteBlock;
        if (SUCCESS != ParseCTEBlock(cteBlock, pCteBlockList->getChild(i), recursive))
            return ERROR;

        blockList.push_back(std::move(cteBlock));
    }

    if (blockList.empty()) {
        BeAssert(false && "expecting one or more blocks");
        return ERROR;
    }

    std::unique_ptr<SelectStatementExp>  selectStmt;
    if (SUCCESS != ParseSelectStatement(selectStmt, *pSelectStmt))
        return ERROR;

    exp = std::make_unique<CommonTableExp>(std::move(selectStmt), std::move(blockList), recursive);
    return SUCCESS;
}
//****************** Parsing SELECT statement ***********************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseSingleSelectStatement(std::unique_ptr<SingleSelectStatementExp>& exp, OSQLParseNode const* parseNode) const
    {
    BeAssert(parseNode != nullptr);
    if (SQL_ISRULE(parseNode, values_or_query_spec))
        {
        //values_or_query_spec
        std::vector<std::unique_ptr<ValueExp>> valueExpList;
        if (SUCCESS != ParseValuesOrQuerySpec(valueExpList, *parseNode))
            return ERROR;

        exp = std::make_unique<SingleSelectStatementExp>(valueExpList);
        return SUCCESS;
        }

    SqlSetQuantifier opt_all_distinct;
    if (SUCCESS != ParseAllOrDistinctToken(opt_all_distinct, parseNode->getChild(1)))
        return ERROR;

    std::unique_ptr<SelectClauseExp> selectClauseExp = nullptr;
    if (SUCCESS != ParseSelection(selectClauseExp, parseNode->getChild(2)))
        return ERROR;

    OSQLParseNode const* tableExpNode = parseNode->getChild(3);
    if (tableExpNode == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    if (tableExpNode->count() == 0) {
        exp = std::make_unique<SingleSelectStatementExp>(opt_all_distinct,std::move(selectClauseExp), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        return SUCCESS;
    }

    BeAssert(tableExpNode->count() == 8);
    std::unique_ptr<FromExp> fromExp = nullptr;
    if (SUCCESS != ParseFromClause(fromExp, tableExpNode->getChild(0)))
        return ERROR;

    std::unique_ptr<WhereExp> whereExp = nullptr;
    if (SUCCESS != ParseWhereClause(whereExp, tableExpNode->getChild(1)))
        return ERROR;

    std::unique_ptr<GroupByExp> groupByExp = nullptr;
    if (SUCCESS != ParseGroupByClause(groupByExp, tableExpNode->getChild(2)))
        return ERROR;

    std::unique_ptr<HavingExp> havingExp = nullptr;
    if (SUCCESS != ParseHavingClause(havingExp, tableExpNode->getChild(3)))
        return ERROR;

    std::unique_ptr<WindowFunctionClauseExp> windowFunctionClauseExp = nullptr;
    if (SUCCESS != ParseWindowClause(windowFunctionClauseExp, tableExpNode->getChild(4)))
        return ERROR;

    std::unique_ptr<OrderByExp> orderByExp = nullptr;
    if (SUCCESS != ParseOrderByClause(orderByExp, tableExpNode->getChild(5)))
        return ERROR;

    std::unique_ptr<LimitOffsetExp> limitOffsetExp = nullptr;
    if (SUCCESS != ParseLimitOffsetClause(limitOffsetExp, tableExpNode->getChild(6)))
        return ERROR;

    std::unique_ptr<OptionsExp> optionsExp = nullptr;
    if (SUCCESS != ParseOptECSqlOptionsClause(optionsExp, tableExpNode->getChild(7)))
        return ERROR;

    if (selectClauseExp == nullptr || fromExp == nullptr)
        {
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0475, "ECSQL without select clause or from clause is invalid.");
        return ERROR;
        }

    exp = std::make_unique<SingleSelectStatementExp>(opt_all_distinct,std::move(selectClauseExp), std::move(fromExp),
                                                     std::move(whereExp), std::move(orderByExp), std::move(windowFunctionClauseExp), std::move(groupByExp), std::move(havingExp),
                                                     std::move(limitOffsetExp), std::move(optionsExp));

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseSelection(std::unique_ptr<SelectClauseExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (SQL_ISRULE(parseNode, selection))
        {
        OSQLParseNode const* n = parseNode->getChild(0);
        if (Exp::IsAsteriskToken(n->getTokenValue()))
            {
            PropertyPath pp;
            pp.Push(Exp::ASTERISK_TOKEN);
            exp = std::make_unique<SelectClauseExp>();
            exp->AddProperty(std::make_unique<DerivedPropertyExp>(std::make_unique<PropertyNameExp>(std::move(pp)), nullptr));
            return SUCCESS;
            }
        }

    if (!SQL_ISRULE(parseNode, scalar_exp_commalist))
        {
        BeAssert(false && "Wrong grammar");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0476, "Wrong grammar");
        return ERROR;
        }

    std::unique_ptr<SelectClauseExp> selectClauseExp = std::make_unique<SelectClauseExp>();

    for (size_t n = 0; n < parseNode->count(); n++)
        {
        std::unique_ptr<DerivedPropertyExp> derivedPropExp = nullptr;
        BentleyStatus stat = ParseDerivedColumn(derivedPropExp, parseNode->getChild(n));
        if (SUCCESS != stat)
            return stat;

        if (derivedPropExp != nullptr)
            selectClauseExp->AddProperty(std::move(derivedPropExp));
        }

    exp = std::move(selectClauseExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseDerivedColumn(std::unique_ptr<DerivedPropertyExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, derived_column))
        {
        BeAssert(false && "Wrong grammar");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0476, "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* first = parseNode->getChild(0);
    OSQLParseNode const* opt_as_clause = parseNode->getChild(1);

    std::unique_ptr<ValueExp> valExp = nullptr;
    BentleyStatus stat = ParseValueExp(valExp, first);
    if (stat != SUCCESS)
        return stat;

    Utf8String columnAlias;
    if (opt_as_clause->count() > 0)
        columnAlias = opt_as_clause->getChild(1)->getTokenValue();
    else
        columnAlias = opt_as_clause->getTokenValue();

    exp = std::make_unique<DerivedPropertyExp>(std::move(valExp), columnAlias.c_str());
    return SUCCESS;
    }
//****************** Parsing PRAGMA statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParsePragmaStatement(std::unique_ptr<PragmaStatementExp>& pragmaExp, OSQLParseNode const& parseNode) const {
    using ParseNode = OSQLParseNode;
    using ParseFunc = std::function<BentleyStatus(ParseNode const*)>;
    pragmaExp = nullptr;

    if (false == SQL_ISRULE(&parseNode, pragma)) {
        return ERROR;
    }

    Utf8String outPragmaName;
    std::vector<Utf8String> outPathTokens;
    PragmaVal outPragmaVal;
    bool outReadValue = true;
    ParseFunc parse_pragma_path = [&](ParseNode const* parseNode) {
        if (!SQL_ISRULE(parseNode, pragma_path)) {
            return ERROR;
        }
        if (parseNode->count() == 0) {
            return ERROR;
        }
        for(int i=0; i< parseNode->count(); ++i) {
            outPathTokens.push_back(parseNode->getChild(i)->getTokenValue());
        }
        return SUCCESS;
    };
    ParseFunc parse_opt_pragma_value = [&](ParseNode const* parseNode) {
        if (parseNode->getNodeType() == SQL_NODE_INTNUM) {
            outPragmaVal = PragmaVal((int64_t)std::stoll(parseNode->getTokenValue()));
        } else if (parseNode->getNodeType() == SQL_NODE_APPROXNUM) {
            outPragmaVal = PragmaVal(std::stod(parseNode->getTokenValue()));
        } else if (parseNode->getNodeType() == SQL_NODE_STRING) {
            outPragmaVal = PragmaVal(parseNode->getTokenValue(), false);
        } else if (parseNode->getNodeType() == SQL_NODE_NAME) {
            outPragmaVal = PragmaVal(parseNode->getTokenValue(), true);
        } else if (parseNode->getNodeType() == SQL_NODE_KEYWORD && parseNode->getTokenID() == SQL_TOKEN_TRUE) {
            outPragmaVal = PragmaVal(true);
        } else if (parseNode->getNodeType() == SQL_NODE_KEYWORD && parseNode->getTokenID()== SQL_TOKEN_FALSE) {
            outPragmaVal = PragmaVal(false);
        } else if (parseNode->getNodeType() == SQL_NODE_KEYWORD && parseNode->getTokenID()== SQL_TOKEN_NULL) {
            outPragmaVal = PragmaVal::Null();
        } else {
            BeAssert(false && "unhandled token id");
            Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECSQL, 
                ECDbIssueId::ECDb_0478,
                "Unsupported PRAGMA value Token Id %" PRIu32 ".",
                parseNode->getTokenID()
            );
            return ERROR;
        }
        return SUCCESS;
    };
    ParseFunc parse_opt_pragma_func = [&](ParseNode const* parseNode) {
       if (!SQL_ISRULE(parseNode, opt_pragma_func)) {
            return SUCCESS;
        }
        return parse_opt_pragma_value(parseNode->getChild(0));
    };
    ParseFunc parse_opt_pragma_set_val = [&](ParseNode const* parseNode) {
        if (!SQL_ISRULE(parseNode, opt_pragma_set_val)) {
            return SUCCESS;
        }
        outReadValue = false;
        return parse_opt_pragma_value(parseNode->getChild(0));
    };
    ParseFunc parse_opt_pragma_set = [&](ParseNode const* parseNode) {
        if (SQL_ISRULE(parseNode, opt_pragma_set)) {
            return SUCCESS;
        }
        if (SQL_ISRULE(parseNode, opt_pragma_set_val)) {
            return parse_opt_pragma_set_val(parseNode);
        }
        if (SQL_ISRULE(parseNode, opt_pragma_func)) {
            return parse_opt_pragma_func(parseNode);
        }
        BeAssert(false && "Unhandled case");
        return ERROR;
    };
    ParseFunc parse_opt_pragma_for = [&](ParseNode const* parseNode) {
        if (!SQL_ISRULE(parseNode, opt_pragma_for)) {
            return ERROR;
        }
        if (parseNode->count() == 0) {
            return SUCCESS;
        }
        return parse_pragma_path(parseNode->getChild(0 /* pragma_path */));
    };
    auto parse_pragma = [&](ParseNode const* parseNode) {
        outPragmaName = parseNode->getChild(1 /* SQL_TOKEN_NAME */)->getTokenValue();
        if (SUCCESS != parse_opt_pragma_set(parseNode->getChild(2 /* opt_pragma_set */))) {
            return ERROR;
        }
        if (SUCCESS != parse_opt_pragma_for(parseNode->getChild(3 /* opt_pragma_for */))) {
            return ERROR;
        }
        return SUCCESS;
    };
    if (SUCCESS != parse_pragma(&parseNode)) {
        return ERROR;
    }

    std::unique_ptr<OptionsExp> options;
    if (SUCCESS != ParseOptECSqlOptionsClause(options, parseNode.getChild(4 /* opt_ecsql_options */))) {
        return ERROR;
    }
    pragmaExp = std::make_unique<PragmaStatementExp>(outPragmaName, outPragmaVal, outReadValue, outPathTokens, std::move(options));
    return SUCCESS;
    }

//****************** Parsing INSERT statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseInsertStatement(std::unique_ptr<InsertStatementExp>& insertExp, OSQLParseNode const& parseNode) const
    {
    insertExp = nullptr;
    //insert does not support polymorphic classes. Passing false therefore.
    std::unique_ptr<ClassNameExp> classNameExp = nullptr;
    BentleyStatus stat = ParseTableNode(classNameExp, parseNode.count() == 6 ? *parseNode.getChild(3) : *parseNode.getChild(2), ECSqlType::Insert, PolymorphicInfo::Only());
    if (SUCCESS != stat)
        return stat;

    std::unique_ptr<PropertyNameListExp> insertPropertyNameListExp = nullptr;
    stat = ParseOptColumnRefCommalist(insertPropertyNameListExp, parseNode.count() == 6 ? parseNode.getChild(4) : parseNode.getChild(3));
    if (SUCCESS != stat)
        return stat;

    OSQLParseNode const* valuesOrQuerySpecNode = parseNode.count() == 6 ? parseNode.getChild(5) : parseNode.getChild(4);
    if (valuesOrQuerySpecNode == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    std::vector<std::unique_ptr<ValueExp>> valueExpList;
    stat = ParseValuesOrQuerySpec(valueExpList, *valuesOrQuerySpecNode);
    if (SUCCESS != stat)
        return stat;

    insertExp = std::make_unique<InsertStatementExp>(classNameExp, insertPropertyNameListExp, valueExpList);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseALLorONLY(PolymorphicInfo& constraint, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, OSQLParseNode::Rule::opt_only_all))
        return ERROR;

    if (parseNode->count() == 0)
        {
        constraint = PolymorphicInfo::NotSpecified();
        return SUCCESS;
        }

    const auto constraintNode = parseNode->getChild(0);

    auto type = PolymorphicInfo::Type::All;
    if (!PolymorphicInfo::TryParseToken(type , constraintNode->getTokenValue()))
        return ERROR;

    constraint = PolymorphicInfo(type, false);
    return SUCCESS;
    }

//****************** Parsing UPDATE statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseUpdateStatementSearched(std::unique_ptr<UpdateStatementExp>& exp, OSQLParseNode const& parseNode) const
    {
    exp = nullptr;
    //rule: update_statement_searched: SQL_TOKEN_UPDATE opt_only_all table_node SQL_TOKEN_SET assignment_commalist opt_where_clause opt_ecsqloptions_clause
    PolymorphicInfo constraint;
    if(SUCCESS != ECSqlParser::ParseALLorONLY(constraint, parseNode.getChild(1)))
        return ERROR;

    std::unique_ptr<ClassNameExp> classNameExp = nullptr;
    BentleyStatus stat = ParseTableNode(classNameExp, *parseNode.getChild(2), ECSqlType::Update, constraint);
    if (SUCCESS != stat)
        return stat;

    std::unique_ptr<AssignmentListExp> assignmentListExp = nullptr;
    stat = ParseAssignmentCommalist(assignmentListExp, parseNode.getChild(4));
    if (SUCCESS != stat)
        return stat;

    std::unique_ptr<WhereExp> opt_where_clause = nullptr;
    stat = ParseWhereClause(opt_where_clause, parseNode.getChild(5));
    if (SUCCESS != stat)
        return stat;

    std::unique_ptr<OptionsExp> opt_options_clause = nullptr;
    stat = ParseOptECSqlOptionsClause(opt_options_clause, parseNode.getChild(6));
    if (SUCCESS != stat)
        return stat;

    exp = std::make_unique<UpdateStatementExp>(std::move(classNameExp), std::move(assignmentListExp), std::move(opt_where_clause), std::move(opt_options_clause));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseAssignmentCommalist(std::unique_ptr<AssignmentListExp>& exp, OSQLParseNode const* parseNode) const
    {
    std::unique_ptr<AssignmentListExp> listExp = std::make_unique<AssignmentListExp>();
    const size_t assignmentCount = parseNode->count();
    for (size_t i = 0; i < assignmentCount; i++)
        {
        OSQLParseNode const* assignmentNode = parseNode->getChild(i);
        BeAssert(SQL_ISRULE(assignmentNode, assignment) && assignmentNode->count() == 3 && "Wrong ECSQL grammar. Expected rule assignment.");

        std::unique_ptr<PropertyNameExp> lhsExp = nullptr;
        BentleyStatus stat = ParseColumnRefAsPropertyNameExp(lhsExp, assignmentNode->getChild(0));
        if (SUCCESS != stat)
            return stat;

        std::unique_ptr<ValueExp> rhsExp = nullptr;
        stat = ParseValueExp(rhsExp, assignmentNode->getChild(2));
        if (SUCCESS != stat)
            return stat;

        listExp->AddAssignmentExp(std::make_unique<AssignmentExp>(std::move(lhsExp), std::move(rhsExp)));
        }

    exp = std::move(listExp);
    return SUCCESS;
    }

//****************** Parsing DELETE statement ***********************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseDeleteStatementSearched(std::unique_ptr<DeleteStatementExp>& exp, OSQLParseNode const& parseNode) const
    {
    //rule: delete_statement_searched: SQL_TOKEN_DELETE SQL_TOKEN_FROM opt_only_all table_node opt_where_clause opt_ecsqloptions_clause
    PolymorphicInfo constraint;
    if(SUCCESS != ECSqlParser::ParseALLorONLY(constraint, parseNode.getChild(2)))
        return ERROR;
    
    std::unique_ptr<ClassNameExp> classNameExp = nullptr;
    BentleyStatus stat = ParseTableNode(classNameExp, *parseNode.getChild(3), ECSqlType::Delete, constraint);
    if (SUCCESS != stat)
        return stat;

    std::unique_ptr<WhereExp> opt_where_clause = nullptr;
    stat = ParseWhereClause(opt_where_clause, parseNode.getChild(4));
    if (SUCCESS != stat)
        return stat;

    std::unique_ptr<OptionsExp> opt_options_clause = nullptr;
    stat = ParseOptECSqlOptionsClause(opt_options_clause, parseNode.getChild(5));
    if (SUCCESS != stat)
        return stat;

    exp = std::make_unique<DeleteStatementExp>(std::move(classNameExp), std::move(opt_where_clause), std::move(opt_options_clause));
    return SUCCESS;
    }

//****************** Parsing common expression ***********************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseColumn(std::unique_ptr<PropertyNameExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (parseNode->getNodeType() != SQL_NODE_NAME)
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    BeAssert(!parseNode->getTokenValue().empty());
    PropertyPath propPath;
    propPath.Push(parseNode->getTokenValue());
    exp = std::make_unique<PropertyNameExp>(propPath);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseOptColumnRefCommalist(std::unique_ptr<PropertyNameListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_column_ref_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_column_ref_commalist");
        return ERROR;
        }

    const size_t childCount = parseNode->count();
    if (childCount == 0)
        return SUCCESS; //User never provided a insert column name list clause (no error)

    BeAssert(childCount == 3);
    //first and third nodes are ( and ). Second node is the the list node
    return ParseColumnRefCommalist(exp, parseNode->getChild(1));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseOptECSqlOptionsClause(std::unique_ptr<OptionsExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;
    if (SQL_ISRULE(parseNode, opt_ecsqloptions_clause))
        return SUCCESS; //this rule is hit if no options clause was given. So just return in that case

    if (!SQL_ISRULE(parseNode, ecsqloptions_clause))
        {
        BeAssert(false && "Wrong grammar. Expecting ecsqloptions_clause");
        return ERROR;
        }

    //first node is ECSQLOPTIONS keyword, second node is option list
    OSQLParseNode const* optionListNode = parseNode->getChild(1);

    const size_t childCount = optionListNode->count();
    if (childCount == 0)
        return SUCCESS; //User never provided options

    std::unique_ptr<OptionsExp> optionsExp = std::make_unique<OptionsExp>();
    for (size_t i = 0; i < childCount; i++)
        {
        std::unique_ptr<OptionExp> optionExp = nullptr;
        BentleyStatus stat = ParseECSqlOption(optionExp, optionListNode->getChild(i));
        if (SUCCESS != stat)
            return stat;

        if (SUCCESS != optionsExp->AddOptionExp(std::move(optionExp), Issues()))
            return ERROR;
        }

    exp = std::move(optionsExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseColumnRefCommalist(std::unique_ptr<PropertyNameListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, column_ref_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting column_ref_commalist");
        return ERROR;
        }

    const size_t columnCount = parseNode->count();
    std::unique_ptr<PropertyNameListExp> listExp = std::make_unique<PropertyNameListExp>();
    for (size_t i = 0; i < columnCount; i++)
        {
        std::unique_ptr<PropertyNameExp> propertyNameExp = nullptr;
        BentleyStatus stat = ParseColumnRefAsPropertyNameExp(propertyNameExp, parseNode->getChild(i));
        if (SUCCESS != stat)
            return stat;

        listExp->AddPropertyNameExp(propertyNameExp);
        }

    exp = std::move(listExp);
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseExpressionPath(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode, bool forceIntoPropertyNameExp) const
    {
    if (!SQL_ISRULE(parseNode, property_path))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    PropertyPath propertyPath;
    bool propertyPathContainsArrayIndex = false;
    for (size_t i = 0; i < parseNode->count(); i++)
        {
        OSQLParseNode const* property_path_entry = parseNode->getChild(i);
        OSQLParseNode const* first = property_path_entry->getFirst();
        Utf8StringCR tokenValue = first->getTokenValue();
        if (first->getNodeType() == SQL_NODE_NAME)
            {
            OSQLParseNode const* opt_column_array_idx = property_path_entry->getChild(1);
            int arrayIndex = -1;
            if (opt_column_array_idx->getFirst() != nullptr)
                {
                arrayIndex = atoi(opt_column_array_idx->getFirst()->getTokenValue().c_str());
                propertyPathContainsArrayIndex = true;
                }

            propertyPath.Push(tokenValue, arrayIndex);
            }
        else if (first->getNodeType() == SQL_NODE_PUNCTUATION)
            {
            propertyPath.Push(tokenValue);
            }
        else
            {
            BeAssert(false && "Wrong grammar");
            return ERROR;
            }
        }

    // if this could be an enumeration exp, try to parse it. If not found, interpret as property name exp
    if (!forceIntoPropertyNameExp && propertyPath.Size() == 3 && !propertyPathContainsArrayIndex)
        {
        ECEnumerationCP ecEnum = m_context->GetECDb().Schemas().GetEnumeration(propertyPath[0].GetName(), propertyPath[1].GetName(), SchemaLookupMode::AutoDetect);
        if (ecEnum != nullptr)
            {
            ECEnumeratorCP enumerator = ecEnum->FindEnumeratorByName(propertyPath[2].GetName().c_str());
            if (enumerator != nullptr)
                {
                exp = std::make_unique<EnumValueExp>(*enumerator, propertyPath);
                return SUCCESS;
                }
            }
        }

    exp = std::make_unique<PropertyNameExp>(std::move(propertyPath));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseColumnRefAsPropertyNameExp(std::unique_ptr<PropertyNameExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, column_ref))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    std::unique_ptr<ValueExp> valueExp;
    if (SUCCESS != ParseExpressionPath(valueExp, parseNode->getFirst(), true))
        return ERROR;

    exp = std::unique_ptr<PropertyNameExp>((PropertyNameExp*) valueExp.release());
    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseColumnRef(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode, bool forceIntoPropertyNameExp) const {
    if (!SQL_ISRULE(parseNode, column_ref)) {
        BeAssert(false && "Wrong grammar");
        return ERROR;
    }

    std::unique_ptr<ValueExp> lhsExp;
    const auto rc =  ParseExpressionPath(lhsExp, parseNode->getFirst(), forceIntoPropertyNameExp);
    if (rc != SUCCESS) {
        return rc;
    }

    const auto opt_extract_value = parseNode->getLast();
    if (!SQL_ISRULE(opt_extract_value, opt_extract_value)) {
        BeAssert(false && "Wrong grammar");
        return ERROR;
    }

    const auto isExtractProp  = (opt_extract_value->count() != 0);
    const auto isOptionalProp = (opt_extract_value->count() == 2 && opt_extract_value->getLast()->count() == 1);
    if (isExtractProp) {
        if (lhsExp->GetType() != Exp::Type::PropertyName) {
            Issues().Report(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECSQL,
                ECDbIssueId::ECDb_0609,
                "Invalid grammar. instance property exp must be follow syntax '[<alias>.]$ -> <access-string>'");
            return ERROR;
        }
    }

    if (lhsExp->GetType() == Exp::Type::PropertyName) {
        auto lhsPropExp = lhsExp->GetAsCP<PropertyNameExp>();
        if (InstanceValueExp::IsInstancePath(lhsPropExp->GetResolvedPropertyPath())) {
            if (!InstanceValueExp::IsValidSourcePath(lhsPropExp->GetResolvedPropertyPath())) {
                Issues().Report(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0610,
                    "Invalid grammar. Instance exp must be follow syntax '[<alias>.]$'");
                return ERROR;
            }
            if (!isExtractProp) {
                exp = std::make_unique<ExtractInstanceValueExp>(lhsPropExp->GetResolvedPropertyPath());
                return SUCCESS;
            } else {
                std::unique_ptr<ValueExp> rhsExp;
                const auto rc =  ParseExpressionPath(rhsExp, opt_extract_value->getFirst(), false);
                if (rc != SUCCESS) {
                    return rc;
                }
                if (rhsExp->GetType() != Exp::Type::PropertyName) {
                    Issues().Report(
                        IssueSeverity::Error,
                        IssueCategory::BusinessProperties,
                        IssueType::ECSQL,
                        ECDbIssueId::ECDb_0611,
                        "Invalid grammar. instance property exp must be follow syntax '[<alias>.]$ -> <access-string>'");
                    return ERROR;
                }

                auto rhsPropExp = rhsExp->GetAsCP<PropertyNameExp>();
                exp = std::make_unique<ExtractPropertyValueExp>(lhsPropExp->GetResolvedPropertyPath(), rhsPropExp->GetResolvedPropertyPath(), isOptionalProp);
                return SUCCESS;
            }
        } else {
            if (lhsPropExp->GetResolvedPropertyPath().Last().GetName().EndsWith("?")) {
                Issues().Report(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECSQL,
                    ECDbIssueId::ECDb_0612,
                    "Invalid grammar. Property access string cannot end with '?'.");
                return ERROR;
            }
        }
    }
    exp = std::move(lhsExp);
    return SUCCESS;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseParameter(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, parameter))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    Utf8StringCR paramTokenValue = parseNode->getChild(0)->getTokenValue();

    Utf8CP paramName = nullptr;
    if (paramTokenValue.Equals(":"))
        {
        OSQLParseNode const* param_nameNode = parseNode->getChild(1);
        paramName = param_nameNode->getTokenValue().c_str();
        }
    else
        {
        if (!paramTokenValue.Equals("?"))
            {
            BeAssert(paramTokenValue.Equals("?") && "Invalid grammar. Only : or ? allowed as parameter tokens");
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0481, "Invalid grammar. Only : or ? allowed as parameter tokens");
            return ERROR;
            }
        }

    exp = std::make_unique<ParameterExp>(paramName);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTerm(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, term) && parseNode->count() == 3)
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* operand_left = parseNode->getChild(0);
    OSQLParseNode const* opNode = parseNode->getChild(1);
    OSQLParseNode const* operand_right = parseNode->getChild(2);

    std::unique_ptr<ValueExp> operand_left_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_left_expr, operand_left))
        return ERROR;

    std::unique_ptr<ValueExp> operand_right_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_right_expr, operand_right))
        return ERROR;

    Utf8StringCR opStr = opNode->getTokenValue();
    BinarySqlOperator op;
    if (opStr.Equals("*"))
        op = BinarySqlOperator::Multiply;
    else if (opStr.Equals("/"))
        op = BinarySqlOperator::Divide;
    else if (opStr.Equals("%"))
        op = BinarySqlOperator::Modulo;
    else
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    exp = std::make_unique<BinaryValueExp>(std::move(operand_left_expr), op, std::move(operand_right_expr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseCastSpec(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, cast_spec) && parseNode->count() == 3)
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* castOperandNode = parseNode->getChild(2);
    OSQLParseNode const* castTargetNode = parseNode->getChild(4);

    std::unique_ptr<ValueExp> castOperandExp = nullptr;
    if (SUCCESS != ParseValueExp(castOperandExp, castOperandNode))
        return ERROR;

    const bool isArrayTargetType = SQL_ISRULE(castTargetNode, cast_target_array);
    OSQLParseNode const* scalarTargetNode = nullptr;

    if (isArrayTargetType)
        {
        scalarTargetNode = castTargetNode->getChild(0);
        BeAssert(castTargetNode->getChild(1)->getNodeType() == SQL_NODE_ARRAY_INDEX);
        if (!castTargetNode->getChild(1)->getTokenValue().empty())
            {
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0482,
                "Invalid syntax for CAST target array type. Array type must be specified with empty square brackets.");
            return ERROR;
            }
        }
    else
        scalarTargetNode = castTargetNode;

    BeAssert(SQL_ISRULE(scalarTargetNode, cast_target_scalar));

    const bool isPrimitiveTargetType = scalarTargetNode->count() == 1;
    if (isPrimitiveTargetType)
        {
        OSQLParseNode const* primTypeNode = scalarTargetNode->getChild(0);
        Utf8CP castTargetTypeName = nullptr;
        if (SQL_NODE_KEYWORD == primTypeNode->getNodeType())
            {
            castTargetTypeName = SqlDataTypeKeywordToString(primTypeNode->getTokenID());
            if (Utf8String::IsNullOrEmpty(castTargetTypeName))
                return ERROR;
            }
        else
            castTargetTypeName = primTypeNode->getTokenValue().c_str();

        exp = std::make_unique<CastExp>(std::move(castOperandExp), castTargetTypeName, isArrayTargetType);
        return SUCCESS;
        }

    //struct or enum type
    if (scalarTargetNode->count() != 3)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8StringCR targetSchemaName = scalarTargetNode->getChild(0)->getTokenValue();
    Utf8StringCR targetTypeName = scalarTargetNode->getChild(2)->getTokenValue();
    exp = std::make_unique<CastExp>(std::move(castOperandExp), targetSchemaName, targetTypeName, isArrayTargetType);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseFctSpec(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, fct_spec))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* functionNameNode = parseNode->getChild(0);
    auto getFunctionNameFromTokenID = [&](uint32_t tokenID){
        switch (tokenID)
            {
            case SQL_TOKEN_RTRIM:
                return "RTRIM";
            default:
                return "";
            }
    };
    Utf8StringCR functionName = functionNameNode->getTokenValue().empty() ? getFunctionNameFromTokenID(functionNameNode->getTokenID()) : functionNameNode->getTokenValue();
    if (functionName.empty())
        {
        const uint32_t tokenId = functionNameNode->getTokenID();
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0483, "Function with token ID %" PRIu32 " not yet supported.", tokenId);
        return ERROR;
        }

    const size_t childCount = parseNode->count();
    if (childCount == 5)
        return ParseSetFct(exp, *parseNode, functionName, false);

    std::unique_ptr<FunctionCallExp> functionCallExp = std::make_unique<FunctionCallExp>(functionName);
    //parse function args. (if child parse node count is < 4, function doesn't have args)
    if (childCount == 4)
        {
        OSQLParseNode const* argumentsNode = parseNode->getChild(2);
        if (SQL_ISRULE(argumentsNode, function_args_commalist))
            {
            for (size_t i = 0; i < argumentsNode->count(); i++)
                {
                if (SUCCESS != ParseAndAddFunctionArg(*functionCallExp, argumentsNode->getChild(i)))
                    return ERROR;
                }
            }
        else
            {
            if (SUCCESS != ParseAndAddFunctionArg(*functionCallExp, argumentsNode))
                return ERROR;
            }
        }

    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseSetFct(std::unique_ptr<ValueExp>& exp, OSQLParseNode const& parseNode, Utf8StringCR functionName, bool isStandardSetFunction) const
    {
    SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified;
    if (SUCCESS != ParseAllOrDistinctToken(setQuantifier, parseNode.getChild(2)))
        return ERROR;

    std::unique_ptr<FunctionCallExp> functionCallExp = std::make_unique<FunctionCallExp>(functionName, setQuantifier, isStandardSetFunction);

    if (functionName.EqualsIAscii("count") && Exp::IsAsteriskToken(parseNode.getChild(2)->getTokenValue()))
        {
        std::unique_ptr<ValueExp> argExp = nullptr;
        if (SUCCESS != LiteralValueExp::Create(argExp, *m_context, Exp::ASTERISK_TOKEN, ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies)))
            return ERROR;

        functionCallExp->AddArgument(std::move(argExp));
        }
    else
        {
        if (SUCCESS != ParseAndAddFunctionArg(*functionCallExp, parseNode.getChild(3/*function_arg*/)))
            return ERROR;
        }

    if (functionName.EqualsIAscii("group_concat"))
        {
        if (parseNode.getChild(4/*opt_function_arg*/)->count() != 0)
            {
            if (SUCCESS != ParseAndAddFunctionArg(*functionCallExp, parseNode.getChild(4/*opt_function_arg*/)->getChild(1/*function_arg*/)))
                return ERROR;
            }
        }

    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseAndAddFunctionArg(FunctionCallExp& functionCallExp, OSQLParseNode const* argNode) const
    {
    std::unique_ptr<ValueExp> argument_expr = nullptr;
    BentleyStatus stat = ParseFunctionArg(argument_expr, *argNode);
    if (SUCCESS != stat)
        return stat;

    functionCallExp.AddArgument(std::move(argument_expr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseAggregateFct(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, aggregate_fct) &&
        (parseNode->count() == 4 || parseNode->count() == 5))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* functionNameNode = parseNode->getChild(0);
    if (!functionNameNode->getTokenValue().empty())
        {
        BeAssert(false && "aggregate_fct expects no function name to be set");
        return ERROR;
        }

    Utf8CP functionName = nullptr;
    switch (functionNameNode->getTokenID())
        {
            case SQL_TOKEN_ANY:
                functionName = "ANY";
                break;
            case SQL_TOKEN_AVG:
                functionName = "AVG";
                break;
            case SQL_TOKEN_COUNT:
                functionName = "COUNT";
                break;
            case SQL_TOKEN_EVERY:
                functionName = "EVERY";
                break;
            case SQL_TOKEN_MAX:
                functionName = "MAX";
                break;
            case SQL_TOKEN_MIN:
                functionName = "MIN";
                break;
            case SQL_TOKEN_SOME:
                functionName = "SOME";
                break;
            case SQL_TOKEN_SUM:
                functionName = "SUM";
                break;
            case SQL_TOKEN_GROUP_CONCAT:
                functionName = "GROUP_CONCAT";
                break;
            case SQL_TOKEN_TOTAL:
                functionName = "TOTAL";
                break;
            default:
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0484, "Unsupported standard set function with token ID %" PRIu32, functionNameNode->getTokenID());
            return ERROR;
            }
        }

    return ParseSetFct(exp, *parseNode, Utf8String(functionName), true);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseECSqlOption(std::unique_ptr<OptionExp>& exp, OSQLParseNode const* parseNode) const {
    const size_t childNodeCount = parseNode->count();
    BeAssert(childNodeCount == 1 || childNodeCount == 3);

    Utf8CP optionName = parseNode->getChild(0)->getTokenValue().c_str();
    Utf8String optionValue;
    ECSqlTypeInfo dataType;
    if (childNodeCount == 3) {
        OSQLParseNode const* valNode = parseNode->getChild(2);
        BeAssert(valNode != nullptr);
        if (valNode->getNodeType() == SQL_NODE_NAME) {
            optionValue = valNode->getTokenValue();
            dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        } else {
            if (SUCCESS != ParseLiteral(optionValue, dataType, *valNode)) {
                return ERROR;
            }
        }
    }
    exp = std::make_unique<OptionExp>(optionName, optionValue.c_str(), dataType);
    return SUCCESS;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseValueExpPrimary(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, value_exp_primary) && parseNode->count() == 3)
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    //This rule is expected to always have parentheses
    BeAssert(parseNode->getChild(0)->getTokenValue() == "(" &&
             parseNode->getChild(2)->getTokenValue() == ")");

    if (SUCCESS != ParseValueExp(exp, parseNode->getChild(1)))
        return ERROR;

    if (parseNode->getChild(0)->getTokenValue().Equals("("))
        exp->SetHasParentheses();

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTermAddSub(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, term_add_sub))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* operand_left = parseNode->getChild(0);
    OSQLParseNode const* opNode = parseNode->getChild(1);
    OSQLParseNode const* operand_right = parseNode->getChild(2);

    std::unique_ptr<ValueExp> operand_left_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_left_expr, operand_left))
        return ERROR;

    std::unique_ptr<ValueExp> operand_right_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_right_expr, operand_right))
        return ERROR;

    BinarySqlOperator op;
    if (opNode->getTokenValue() == "+")
        op = BinarySqlOperator::Plus;
    else if (opNode->getTokenValue() == "-")
        op = BinarySqlOperator::Minus;
    else
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    exp = std::make_unique<BinaryValueExp>(std::move(operand_left_expr), op, std::move(operand_right_expr));
    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseNumValueExp(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, num_value_exp))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* operand_left = parseNode->getChild(0);
    OSQLParseNode const* opNode = parseNode->getChild(1);
    OSQLParseNode const* operand_right = parseNode->getChild(2);

    std::unique_ptr<ValueExp> operand_left_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_left_expr, operand_left))
        return ERROR;

    std::unique_ptr<ValueExp> operand_right_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_right_expr, operand_right))
        return ERROR;

    BinarySqlOperator op;
    if (opNode->getTokenValue() == "<<")
        op = BinarySqlOperator::ShiftLeft;
    else if (opNode->getTokenValue() == ">>")
        op = BinarySqlOperator::ShiftRight;
    else if (opNode->getTokenValue() == "|")
        op = BinarySqlOperator::BitwiseOr;
    else if (opNode->getTokenValue() == "&")
        op = BinarySqlOperator::BitwiseAnd;
    else
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    exp = std::make_unique<BinaryValueExp>(std::move(operand_left_expr), op, std::move(operand_right_expr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseFactor(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, factor))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* opNode = parseNode->getChild(0);
    OSQLParseNode const* operandNode = parseNode->getChild(1);

    std::unique_ptr<ValueExp> operand_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_expr, operandNode))
        return ERROR;

    Utf8StringCR opStr = opNode->getTokenValue();
    UnaryValueExp::Operator op;
    if (opStr.Equals("+"))
        op = UnaryValueExp::Operator::Plus;
    else if (opStr.Equals("-"))
        op = UnaryValueExp::Operator::Minus;
    else if (opStr.Equals("~"))
        op = UnaryValueExp::Operator::BitwiseNot;
    else
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    exp = std::make_unique<UnaryValueExp>(operand_expr, op);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseConcatenation(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, concatenation))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* operand_left = parseNode->getChild(0);
    OSQLParseNode const* operand_right = parseNode->getChild(2);

    std::unique_ptr<ValueExp> operand_left_expr = nullptr;
    if (SUCCESS != ParseValueExp(operand_left_expr, operand_left))
        return ERROR;

    std::unique_ptr<ValueExp> operand_right_expr = nullptr;
    if(SUCCESS != ParseValueExp(operand_right_expr, operand_right))
        return ERROR;

    exp = std::make_unique<BinaryValueExp>(std::move(operand_left_expr), BinarySqlOperator::Concat, std::move(operand_right_expr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseDatetimeValueExp(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, datetime_value_exp))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* termNode = parseNode->getByRule(OSQLParseNode::datetime_term);
    if (termNode == nullptr)
        return ERROR;

    OSQLParseNode const* factorNode = termNode->getByRule(OSQLParseNode::datetime_factor);
    if (factorNode == nullptr)
        return ERROR;

    OSQLParseNode const* primaryNode = factorNode->getByRule(OSQLParseNode::datetime_primary);
    if (primaryNode == nullptr)
        return ERROR;

    OSQLParseNode const* valueFctNode = primaryNode->getByRule(OSQLParseNode::datetime_value_fct);
    if (valueFctNode == nullptr)
        return ERROR;

    return ParseDatetimeValueFct(exp, *valueFctNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseDatetimeValueFct(std::unique_ptr<ValueExp>& exp, OSQLParseNode const& parseNode) const
    {
    const size_t parseNodeChildCount = parseNode.count();
    if (!SQL_ISRULE(&parseNode, datetime_value_fct) && (parseNodeChildCount == 1 || parseNodeChildCount == 2))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    OSQLParseNode const* columnTypeNode = parseNode.getChild(0);
    if (parseNodeChildCount == 1) //Keyword
        {
        Utf8CP fctName = nullptr;
        if (columnTypeNode->getTokenID() == SQL_TOKEN_CURRENT_DATE)
            fctName = FunctionCallExp::CURRENT_DATE();
        else if (columnTypeNode->getTokenID() == SQL_TOKEN_CURRENT_TIMESTAMP)
            fctName = FunctionCallExp::CURRENT_TIMESTAMP();
        else if (columnTypeNode->getTokenID() == SQL_TOKEN_CURRENT_TIME)
            fctName = FunctionCallExp::CURRENT_TIME();
        else
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0485, "Unrecognized keyword '%s'.", parseNode.getTokenValue().c_str());
            return ERROR;
            }

        exp = std::make_unique<FunctionCallExp>(fctName, SqlSetQuantifier::NotSpecified, false, true);
        return SUCCESS;
        }

    Utf8CP unparsedValue = parseNode.getChild(1)->getTokenValue().c_str();
    if (columnTypeNode->getTokenID() == SQL_TOKEN_DATE || columnTypeNode->getTokenID() == SQL_TOKEN_TIMESTAMP || columnTypeNode->getTokenID() == SQL_TOKEN_TIME)
        return LiteralValueExp::Create(exp, *m_context, unparsedValue, ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_DateTime));

    exp = nullptr;
    BeAssert(false && "Wrong grammar");
    return ERROR;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseFromClause(std::unique_ptr<FromExp>& exp, OSQLParseNode const* parseNode) const
    {
    std::unique_ptr<FromExp> from_clause = std::make_unique<FromExp>();
    OSQLParseNode* table_ref_commalist = parseNode->getChild(1);
    for (size_t n = 0; n < table_ref_commalist->count(); n++)
        {
        std::unique_ptr<ClassRefExp> classRefExp = nullptr;
        BentleyStatus stat = ParseTableRef(classRefExp, table_ref_commalist->getChild(n), ECSqlType::Select);
        if (stat != SUCCESS)
            return stat;

        stat = from_clause->TryAddClassRef(*m_context, std::move(classRefExp));
        if (stat != SUCCESS)
            return stat;
        }

    exp = std::move(from_clause);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParsePolymorphicConstraint(PolymorphicInfo& constraint, OSQLParseNode const* parseNode)
    {
    if (!SQL_ISRULE(parseNode, OSQLParseNode::Rule::opt_only))
        return ERROR;

    if (parseNode->count() == 0)
        {
        constraint = PolymorphicInfo::NotSpecified();
        return SUCCESS;
        }

    const auto disqualifyNode = parseNode->getChild(0);
    if (!SQL_ISRULE(disqualifyNode, OSQLParseNode::Rule::opt_disqualify_polymorphic_constraint))
        return ERROR;

    const auto constraintNode = parseNode->getChild(1);
    bool disqualify = false;
    if (disqualifyNode->count() == 1)
        disqualify = true;

    auto type = PolymorphicInfo::Type::All;
    if (!PolymorphicInfo::TryParseToken(type , constraintNode->getTokenValue()))
        return ERROR;

    constraint = PolymorphicInfo(type, disqualify);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTableRef(std::unique_ptr<ClassRefExp>& exp, OSQLParseNode const* parseNode, ECSqlType ecsqlType) const
    {
    if (SQL_ISRULE(parseNode, OSQLParseNode::Rule::qualified_join) || SQL_ISRULE(parseNode, OSQLParseNode::Rule::cross_union) || SQL_ISRULE(parseNode, OSQLParseNode::Rule::ecrelationship_join))
        {
        std::unique_ptr<JoinExp> joinExp = nullptr;
        if (SUCCESS != ParseJoinedTable(joinExp, parseNode))
            return ERROR;

        exp = std::move(joinExp);
        return SUCCESS;
        }

    if (!SQL_ISRULE(parseNode, table_ref))
        {
        BeAssert(false && "Wrong grammar. Expecting table_name.");
        return ERROR;
        }

    OSQLParseNode const* opt_only = parseNode->getChild(0);
    OSQLParseNode const* secondNode = parseNode->getChild(1);
    PolymorphicInfo polymorphicConst;
    if (SUCCESS != ECSqlParser::ParsePolymorphicConstraint(polymorphicConst, opt_only))
        return ERROR;

    if (SQL_ISRULE(secondNode, OSQLParseNode::Rule::subquery))
        {
        std::unique_ptr<SubqueryExp> subqueryExp = nullptr;
        if (SUCCESS != ParseSubquery(subqueryExp, secondNode))
            return ERROR;

        OSQLParseNode const* range_variable = parseNode->getChild(2/*range_variable*/);
        Utf8CP alias = nullptr;
        if (range_variable->count() > 0)
            alias = range_variable->getChild(1/*SQL_TOKEN_NAME*/)->getTokenValue().c_str();

        exp = std::make_unique<SubqueryRefExp>(std::move(subqueryExp), alias, polymorphicConst);
        return SUCCESS;
        }

    if (SQL_ISRULE(secondNode, OSQLParseNode::Rule::opt_disqualify_primary_join)) {
        const bool disqualifyPrimaryJoin = (secondNode->count() == 1);
        OSQLParseNode const* thirdNode = parseNode->getChild(2);
        std::unique_ptr<RangeClassRefExp> rangeClassRef;
        if (SQL_ISRULE(thirdNode, OSQLParseNode::Rule::table_node_with_opt_member_func_call)) {
            if ( ecsqlType==ECSqlType::Select && polymorphicConst.IsPolymorphic()) {
                std::unique_ptr<CommonTableBlockNameExp> cteBlockNameExp;
                std::unique_ptr<TableValuedFunctionExp> tableValueFunc;
                if (SUCCESS == ParseCommonTableBlockName(cteBlockNameExp, *thirdNode)) {
                    rangeClassRef = std::move(cteBlockNameExp);
                }
                if (SUCCESS == ParseTableValuedFunction(tableValueFunc, *thirdNode)) {
                    rangeClassRef = std::move(tableValueFunc);
                }
            }
            if ( rangeClassRef == nullptr) {
                std::unique_ptr<ClassNameExp> classNameExp = nullptr;
                if (SUCCESS != ParseTableNodeWithOptMemberCall(classNameExp, *thirdNode, ecsqlType, polymorphicConst, disqualifyPrimaryJoin))
                    return ERROR;

                // check and parse view query
                auto classCP = m_context->GetECDb().Schemas().GetClass(classNameExp->GetSchemaName(), classNameExp->GetClassName(), SchemaLookupMode::AutoDetect, classNameExp->GetTableSpace().c_str());
                if (classCP != nullptr && ClassViews::IsViewClass(*classCP)) {
                    Utf8String ecsql;
                    if (!ClassViews::TryGetQuery(ecsql,*classCP)) {
                        m_context->Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECSQL, ECDbIssueId::ECDb_0710, "Invalid View Class '%s'. There is not query supplied for view.", classCP->GetFullName());
                        return ERROR;
                    }

                    if (polymorphicConst.IsOnly()) {
                        m_context->Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECSQL, ECDbIssueId::ECDb_0711, "ONLY keyword is not supported for view classes (ViewClass: %s).", classCP->GetFullName());
                        return ERROR;
                    }

                    ECSqlParseContext::ClassViewPrepareStack viewStack(*m_context, *classCP);
                    ECSqlParser parser;
                    if (viewStack.IsOnStack(*classCP)) {
                        m_context->Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECSQL, ECDbIssueId::ECDb_0712, "Invalid View Class '%s'. View query references itself recusively (%s).", classCP->GetFullName(), viewStack.GetStackAsString().c_str());
                        return ERROR;
                    }

                    auto parseTree = parser.Parse(m_context->GetECDb(), ecsql.c_str(), m_context->Issues(), this);
                    if (parseTree == nullptr) {
                        m_context->Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECSQL, ECDbIssueId::ECDb_0713, "Invalid View Class '%s'. View ECSQL failed to parse.", classCP->GetFullName());
                        return ERROR;
                    }

                    if (parseTree->GetType() != Exp::Type::Select) {
                        m_context->Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECSQL, ECDbIssueId::ECDb_0714, "Invalid View Class '%s'. View ECSQL is not a SELECT statement.", classCP->GetFullName());
                        return ERROR;
                    }

                    std::unique_ptr<SelectStatementExp> selectExp;
                    selectExp.reset(static_cast<SelectStatementExp*>(parseTree.get()));
                    parseTree.release();
                    rangeClassRef = std::make_unique<SubqueryRefExp>(
                        std::make_unique<SubqueryExp>(
                            std::move(selectExp)),
                            classNameExp->GetAlias().empty()? classNameExp->GetClassName().c_str() : classNameExp->GetAlias().c_str(),
                            polymorphicConst,
                            std::move(classNameExp));
                } else {
                    rangeClassRef = std::move(classNameExp);
                }
            }
            OSQLParseNode const* table_primary_as_range_column = parseNode->getChild(3);
            if (table_primary_as_range_column->count() > 0)
                {
                OSQLParseNode* table_alias = table_primary_as_range_column->getChild(1);
                OSQLParseNode* opt_column_commalist = table_primary_as_range_column->getChild(2);
                if (opt_column_commalist->count() > 0)
                    {
                    BeAssert(false && "Range column not supported");
                    return ERROR;
                    }

                if (!table_alias->getTokenValue().empty())
                    rangeClassRef->SetAlias(table_alias->getTokenValue());
                }
            exp = std::move(rangeClassRef);
            return SUCCESS;
            }
    }

    BeAssert(false && "Case not supported");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseJoinedTable(std::unique_ptr<JoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    switch (parseNode->getKnownRuleID())
        {
            case OSQLParseNode::qualified_join:
                return ParseQualifiedJoin(exp, parseNode);

            case OSQLParseNode::cross_union:
            {
            std::unique_ptr<CrossJoinExp> joinExp = nullptr;
            if (SUCCESS != ParseCrossUnion(joinExp, parseNode))
                return ERROR;

            exp = std::move(joinExp);
            return SUCCESS;
            }
            case OSQLParseNode::ecrelationship_join:
            {
            std::unique_ptr<UsingRelationshipJoinExp> joinExp = nullptr;
            if (SUCCESS != ParseECRelationshipJoin(joinExp, parseNode))
                return ERROR;

            exp = std::move(joinExp);
            return SUCCESS;
            }

            default:
                BeAssert(false && "Wrong grammar");
                return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseECRelationshipJoin(std::unique_ptr<UsingRelationshipJoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, ecrelationship_join))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    BeAssert(parseNode->count() == 7);
    std::unique_ptr<ClassRefExp> from_table_ref = nullptr;
    if (SUCCESS != ParseTableRef(from_table_ref, parseNode->getChild(0/*table_ref*/), ECSqlType::Select))
        return ERROR;

    ECSqlJoinType join_type = ECSqlJoinType::InnerJoin;
    if (SUCCESS != ParseJoinType(join_type, parseNode->getChild(1/*join_type*/)))
        return ERROR;

    std::unique_ptr<ClassRefExp> to_table_ref = nullptr;
    if (SUCCESS != ParseTableRef(to_table_ref, parseNode->getChild(3/*table_ref*/), ECSqlType::Select))
        return ERROR;

    //TODO: need to decide whether we support ONLY in USING clause.
    std::unique_ptr<ClassRefExp> table_node_ref = nullptr;
    if (SUCCESS != ParseTableNodeRef(table_node_ref, *parseNode->getChild(5/*table_node_ref*/), ECSqlType::Select))
        return ERROR;

    OSQLParseNode const* op_relationship_direction = parseNode->getChild(6/*op_relationship_direction*/);

    if (!(join_type == ECSqlJoinType::InnerJoin || join_type == ECSqlJoinType::None))
        {
        BeAssert(false && "Supported join type is INNER JOIN");
        return ERROR;
        }

    JoinDirection direction = JoinDirection::Implied;
    if (op_relationship_direction->getTokenID() == SQL_TOKEN_FORWARD)
        direction = JoinDirection::Forward;
    else if (op_relationship_direction->getTokenID() == SQL_TOKEN_BACKWARD)
        direction = JoinDirection::Backward;

    exp = std::make_unique<UsingRelationshipJoinExp>(std::move(from_table_ref), std::move(to_table_ref), std::move(table_node_ref), direction);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseQualifiedJoin(std::unique_ptr<JoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, qualified_join))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    std::unique_ptr<ClassRefExp> from_table_ref = nullptr;
    if (SUCCESS != ParseTableRef(from_table_ref, parseNode->getChild(0/*table_ref*/), ECSqlType::Select))
        return ERROR;

    ECSqlJoinType joinType = ECSqlJoinType::InnerJoin;
    auto key = parseNode->getChild(1);
    if (key->getTokenID() == SQL_TOKEN_NATURAL)
        {
        if (SUCCESS != ParseJoinType(joinType, parseNode->getChild(2/*join_type*/)))
            return ERROR;

        std::unique_ptr<ClassRefExp> to_table_ref = nullptr;
        if (SUCCESS != ParseTableRef(to_table_ref, parseNode->getChild(4/*table_ref*/), ECSqlType::Select))
            return ERROR;

        exp = std::make_unique<NaturalJoinExp>(std::move(from_table_ref), std::move(to_table_ref), joinType);
        return SUCCESS;
        }

    if (SUCCESS != ParseJoinType(joinType, parseNode->getChild(1/*join_type*/)))
        return ERROR;

    std::unique_ptr<ClassRefExp> to_table_ref = nullptr;
    if (SUCCESS != ParseTableRef(to_table_ref, parseNode->getChild(3/*table_ref*/), ECSqlType::Select))
        return ERROR;

    std::unique_ptr<JoinSpecExp> join_spec = nullptr;
    if (SUCCESS != ParseJoinSpec(join_spec, parseNode->getChild(4/*join_spec*/)))
        return ERROR;

    exp = std::make_unique<QualifiedJoinExp>(std::move(from_table_ref), std::move(to_table_ref), joinType, std::move(join_spec));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseCrossUnion(std::unique_ptr<CrossJoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, cross_union))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    std::unique_ptr<ClassRefExp> from_table_ref = nullptr;
    if (SUCCESS != ParseTableRef(from_table_ref, parseNode->getChild(0/*table_ref*/), ECSqlType::Select))
        return ERROR;

    std::unique_ptr<ClassRefExp> to_table_ref = nullptr;
    if (SUCCESS != ParseTableRef(to_table_ref, parseNode->getChild(3/*table_ref*/), ECSqlType::Select))
        return ERROR;

    exp = std::make_unique<CrossJoinExp>(std::move(from_table_ref), std::move(to_table_ref));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseJoinType(ECSqlJoinType& joinType, OSQLParseNode const* parseNode) const
    {
    joinType = ECSqlJoinType::None;

    if (SQL_ISRULE(parseNode, join_type))
        {
        if (parseNode->count() == 0) //default value
            {
            joinType = ECSqlJoinType::InnerJoin;
            return SUCCESS;
            }

        OSQLParseNode const* first = parseNode->getChild(0);
        if (first->getTokenID() == SQL_TOKEN_INNER)
            {
            joinType = ECSqlJoinType::InnerJoin;
            return SUCCESS;
            }

        if (SQL_ISRULE(first, outer_join_type))
            return ParseOuterJoinType(joinType, first);
        }

    if (SQL_ISRULE(parseNode, outer_join_type))
        return ParseOuterJoinType(joinType, parseNode);

    // outer_join_type SQL_TOKEN_OUTER
    if (parseNode->count() == 2) {
        if (SQL_ISRULE(parseNode->getChild(0), outer_join_type))
            return ParseOuterJoinType(joinType, parseNode->getChild(0));
    }
    BeAssert(false && "Invalid grammar. Expected JoinType");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseOuterJoinType(ECSqlJoinType& joinType, OSQLParseNode const* parseNode) const
    {
    joinType = ECSqlJoinType::None;

    if (!SQL_ISRULE(parseNode, outer_join_type))
        {
        BeAssert(false && "Invalid grammar. Expected OuterJoinType");
        return ERROR;
        }

    OSQLParseNode const* n = parseNode->getChild(0);
    switch (n->getTokenID())
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseJoinSpec(std::unique_ptr<JoinSpecExp>& exp, OSQLParseNode const* parseNode) const
    {
    switch (parseNode->getKnownRuleID())
        {
            case OSQLParseNode::join_condition:
            {
            std::unique_ptr<JoinConditionExp> joinCondExp = nullptr;
            if (SUCCESS != ParseJoinCondition(joinCondExp, parseNode))
                return ERROR;

            exp = std::move(joinCondExp);
            return SUCCESS;
            }
            case OSQLParseNode::named_columns_join:
            {
            std::unique_ptr<NamedPropertiesJoinExp> namedPropJoinExp = nullptr;
            if (SUCCESS != ParseNamedColumnsJoin(namedPropJoinExp, parseNode))
                return ERROR;

            exp = std::move(namedPropJoinExp);
            return SUCCESS;
            }

            default:
                BeAssert(false && "Wrong grammar");
                return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseJoinCondition(std::unique_ptr<JoinConditionExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, join_condition))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    std::unique_ptr<BooleanExp> search_condition = nullptr;
    BentleyStatus stat = ParseSearchCondition(search_condition, parseNode->getChild(1/*search_condition*/));
    if (SUCCESS != stat)
        return stat;

    exp = std::make_unique<JoinConditionExp>(std::move(search_condition));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseNamedColumnsJoin(std::unique_ptr<NamedPropertiesJoinExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, named_columns_join))
        {
        BeAssert(false && "Wrong grammar");
        return ERROR;
        }

    exp = std::make_unique<NamedPropertiesJoinExp>();
    OSQLParseNode const* column_commalist = parseNode->getChild(2/*column_commalist*/);
    for (size_t i = 0; i < column_commalist->count(); i++)
        exp->Append(column_commalist->getChild(i)->getTokenValue());

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTableNode(std::unique_ptr<ClassNameExp>& exp, OSQLParseNode const& tableNode, ECSqlType ecsqlType, PolymorphicInfo polymorphic) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(&tableNode, table_node))
        {
        BeAssert(false && "Wrong grammar. Expecting table_node");
        return ERROR;
        }

    BeAssert(tableNode.count() == 1);
    OSQLParseNode const& nameNode = *tableNode.getChild(0);
    Utf8CP tableSpaceName = nullptr;

    OSQLParseNode const* qualifiedClassNameNode = nullptr;
    if (SQL_ISRULE(&nameNode, OSQLParseNode::tablespace_qualified_class_name))
        {
        tableSpaceName = nameNode.getChild(0)->getTokenValue().c_str();
        qualifiedClassNameNode = nameNode.getChild(2);
        }
    else if (SQL_ISRULE(&nameNode, OSQLParseNode::qualified_class_name))
        qualifiedClassNameNode = &nameNode;
    else
        {
        BeAssert(false && "Wrong grammar. First child of table_node must either be qualified_class_name or tablespace_qualified_class_name");
        return ERROR;
        }

    BeAssert(qualifiedClassNameNode->count() == 3);

    Utf8StringCP schemaName = &qualifiedClassNameNode->getChild(0)->getTokenValue();
    OSQLParseNode const& classNameNode = *qualifiedClassNameNode->getChild(2);
    BeAssert(classNameNode.count() == 1);
    Utf8StringCP className = &classNameNode.getChild(0)->getTokenValue();
    BeAssert(className != nullptr && !className->empty() && schemaName != nullptr && !schemaName->empty());

    std::shared_ptr<ClassNameExp::Info> classNameExpInfo = nullptr;
    if (SUCCESS != m_context->TryResolveClass(classNameExpInfo, tableSpaceName, *schemaName, *className, ecsqlType, polymorphic.IsPolymorphic(), tableNode))
        return ERROR;

    exp = std::make_unique<ClassNameExp>(*className, *schemaName, tableSpaceName, classNameExpInfo, polymorphic, nullptr);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseCommonTableBlockName(std::unique_ptr<CommonTableBlockNameExp>& exp, OSQLParseNode const& tableNode) const {
    if (!SQL_ISRULE(&tableNode, OSQLParseNode::Rule::table_node_with_opt_member_func_call)) {
        BeAssert(false && "Wrong grammar. Expecting table_node_with_opt_member_func_call");
        return ERROR;
    }

    OSQLParseNode const* pathNode = tableNode.getFirst();
    if (!SQL_ISRULE(pathNode, OSQLParseNode::Rule::table_node_path)) {
        BeAssert(false && "Wrong grammar. Expecting table_node_path");
        return ERROR;
    }

    const size_t pathLength = pathNode->count();
    if (pathLength  != 1)
        return ERROR;

    if (0 != pathNode->getFirst()->getChild(1)->count()) {
        // Table value function
        return ERROR;
    }
    exp = std::make_unique<CommonTableBlockNameExp>(pathNode->getFirst()->getFirst()->getTokenValue().c_str());
    return SUCCESS;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTableValuedFunction(std::unique_ptr<TableValuedFunctionExp>& exp, OSQLParseNode const& tableNode) const {
    if (!SQL_ISRULE(&tableNode, OSQLParseNode::Rule::table_node_with_opt_member_func_call)) {
        BeAssert(false && "Wrong grammar. Expecting table_node_with_opt_member_func_call");
        return ERROR;
    }

    OSQLParseNode const* pathNode = tableNode.getFirst();
    if (!SQL_ISRULE(pathNode, OSQLParseNode::Rule::table_node_path)) {
        BeAssert(false && "Wrong grammar. Expecting table_node_path");
        return ERROR;
    }

    const size_t pathLength = pathNode->count();
    if (pathLength  != 2) {
        return ERROR;
    }

    auto schemaNode = pathNode->getChild(0);
    auto functionNode = pathNode->getChild(1);

    auto schemaName = schemaNode->getFirst()->getTokenValue();
    if (!schemaNode->getChild(1)->isLeaf()) {
        return ERROR;
    }
    if (functionNode->getChild(1)->isLeaf()) {
        return ERROR;
    }

    std::unique_ptr<MemberFunctionCallExp> memberFuncCall;
    if (functionNode != nullptr)
        {
        if (SUCCESS != ParseMemberFunctionCall(memberFuncCall, *functionNode, true))
            return ERROR;
        }

    exp = std::make_unique<TableValuedFunctionExp>(schemaName.c_str(), std::move(memberFuncCall), PolymorphicInfo::NotSpecified());
    return SUCCESS;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTableNodeRef(std::unique_ptr<ClassRefExp>& exp, OSQLParseNode const& tableNode, ECSqlType ecsqlType) const
    {
    if (!SQL_ISRULE(&tableNode, OSQLParseNode::Rule::table_node_ref))
        {
        BeAssert(false && "Wrong grammar. Expecting table_node_ref");
        return ERROR;
        }

    std::unique_ptr<ClassNameExp> table_node = nullptr;
    if (SUCCESS != ParseTableNodeWithOptMemberCall(table_node, *tableNode.getChild(0/*table_node_with_opt_member_call*/), ecsqlType, PolymorphicInfo::NotSpecified(), false))
        return ERROR;

    if (tableNode.count() == 1)
        {
        exp = std::move(table_node);
        return SUCCESS;
        }

    std::unique_ptr<RangeClassRefExp> rangeClassRef = std::move(table_node);
    OSQLParseNode const* table_primary_as_range_column = tableNode.getChild(1/*table_primary_as_range_column*/);
    if (table_primary_as_range_column->count() > 0)
        {
        OSQLParseNode* table_alias = table_primary_as_range_column->getChild(1);
        OSQLParseNode* opt_column_commalist = table_primary_as_range_column->getChild(2);
        if (opt_column_commalist->count() > 0)
            {
            BeAssert(false && "Range column not supported");
            return ERROR;
            }

        if (!table_alias->getTokenValue().empty())
            rangeClassRef->SetAlias(table_alias->getTokenValue());
        }

    exp = std::move(rangeClassRef);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTableNodeWithOptMemberCall(std::unique_ptr<ClassNameExp>& exp, OSQLParseNode const& tableNode, ECSqlType ecsqlType, PolymorphicInfo polymorphic, bool disqualifyPrimaryJoin) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(&tableNode, OSQLParseNode::Rule::table_node_with_opt_member_func_call))
        {
        BeAssert(false && "Wrong grammar. Expecting table_node_with_opt_member_func_call");
        return ERROR;
        }

    OSQLParseNode const* pathNode = tableNode.getFirst();
    if (!SQL_ISRULE(pathNode, OSQLParseNode::Rule::table_node_path))
        {
        BeAssert(false && "Wrong grammar. Expecting table_node_path");
        return ERROR;
        }

    const size_t pathLength = pathNode->count();
    if (pathLength < 2 || pathLength > 4)
        {
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0486,
            "Invalid ECSQL class expression: Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]");
        return ERROR;
        }

    OSQLParseNode const* memberFunctionNode = nullptr;
    std::vector<Utf8StringCP> entryNames;
    entryNames.reserve(pathLength);

    for (size_t i = 0; i < pathLength; i++)
        {
        OSQLParseNode const* entryNode = pathNode->getChild(i);
        if (!SQL_ISRULE(entryNode, OSQLParseNode::Rule::table_node_path_entry))
            {
            BeAssert(false && "Wrong grammar. Expecting table_node_path_entry");
            return ERROR;
            }

        OSQLParseNode const* optMemberFunctionNode = entryNode->getChild(1);
        if (!optMemberFunctionNode->isLeaf())
            {
            if (i == pathLength - 1)
                {
                memberFunctionNode = entryNode;
                }
            else
                {
                Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0487,
                    "Invalid ECSQL class expression. Class member function calls must appear after the class. Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]");
                return ERROR;
                }
            }

        entryNames.push_back(&entryNode->getFirst()->getTokenValue());
        }
    BeAssert(entryNames.size() == pathLength);
    BeAssert(pathLength >= 2 && "already checked for above");
    const size_t classNameNodeIx = memberFunctionNode != nullptr ? pathLength - 2 : pathLength - 1;
    BeAssert(pathNode->getChild(classNameNodeIx) != nullptr);
    const int schemaNameNodeIx = (int) classNameNodeIx - 1;
    const int tableSpaceNodeIx = schemaNameNodeIx - 1;
    if (schemaNameNodeIx < 0)
        {
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0488,
            "Invalid ECSQL class expression. Cannot specify a class name without schema name or alias. Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]");
        return ERROR;
        }
    BeAssert(tableSpaceNodeIx <= 0);

    Utf8StringCP className = entryNames[classNameNodeIx];
    Utf8StringCP schemaName = entryNames[schemaNameNodeIx];
    Utf8CP tableSpaceName = tableSpaceNodeIx == 0 ? entryNames[tableSpaceNodeIx]->c_str() : nullptr;

    std::shared_ptr<ClassNameExp::Info> classNameExpInfo = nullptr;
    if (SUCCESS != m_context->TryResolveClass(classNameExpInfo, tableSpaceName, *schemaName, *className, ecsqlType, polymorphic.IsPolymorphic(), tableNode))
        return ERROR;

    std::unique_ptr<MemberFunctionCallExp> memberFuncCall;
    if (memberFunctionNode != nullptr)
        {
        if (SUCCESS != ParseMemberFunctionCall(memberFuncCall, *memberFunctionNode, false))
            return ERROR;
        }

    exp = std::make_unique<ClassNameExp>(*className, *schemaName, tableSpaceName, classNameExpInfo, polymorphic, std::move(memberFuncCall), disqualifyPrimaryJoin);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+-----------------+---------------+------
BentleyStatus ECSqlParser::ParseMemberFunctionCall(std::unique_ptr<MemberFunctionCallExp>& memberFunCallExp, OSQLParseNode const& parseNode, bool tableValuedFunc) const
    {
    if (!SQL_ISRULE(&parseNode, OSQLParseNode::table_node_path_entry))
        {
        BeAssert(false && "Wrong grammar. Expecting table_node_path_entry");
        return ERROR;
        }

    BeAssert(parseNode.count() == 2);
    OSQLParseNode const* argsNode = parseNode.getChild(1);
    BeAssert(argsNode != nullptr);
    if (argsNode->isLeaf())
        {
        BeAssert(false && "ParseNode passed to ParseMemberFunctionCall is expected to have a non-empty second child node");
        return ERROR;
        }

    BeAssert(argsNode->count() == 3);

    Utf8StringCR functionName = parseNode.getChild(0)->getTokenValue();
    memberFunCallExp = std::make_unique<MemberFunctionCallExp>(functionName, tableValuedFunc);
    OSQLParseNode const* argListNode = argsNode->getChild(1);
    BeAssert(SQL_ISRULE(argListNode, OSQLParseNode::Rule::function_args_commalist));
    for (size_t i = 0; i < argListNode->count(); i++)
        {
        std::unique_ptr<ValueExp> argument_expr = nullptr;
        if (SUCCESS != ParseFunctionArg(argument_expr, *argListNode->getChild(i)))
            return ERROR;

        Utf8String err;
        if (memberFunCallExp->AddArgument(std::move(argument_expr), err) != SUCCESS)
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0489, err.c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWhereClause(std::unique_ptr<WhereExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (SQL_ISRULE(parseNode, opt_where_clause))
        return SUCCESS; //this rule is hit if no where clause was given. So just return in that case

    if (!SQL_ISRULE(parseNode, where_clause))
        {
        BeAssert(false && "Wrong grammar. Expecting where_clause");
        return ERROR;
        }

    std::unique_ptr<BooleanExp> search_condition = nullptr;
    if (SUCCESS != ParseSearchCondition(search_condition, parseNode->getChild(1/*search_condition*/)))
        return ERROR;

    exp = std::make_unique<WhereExp>(std::move(search_condition));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseSearchCondition(std::unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    const OSQLParseNode::Rule rule = parseNode->getKnownRuleID();
    switch (rule)
        {
        case OSQLParseNode::search_condition:
            {
            std::unique_ptr<BooleanExp> op1 = nullptr;
            if (SUCCESS != ParseSearchCondition(op1, parseNode->getChild(0/*search_condition*/)))
                return ERROR;

            //auto op = parseNode->getChild(1/*SQL_TOKEN_OR*/);
            std::unique_ptr<BooleanExp> op2 = nullptr;
            if (SUCCESS != ParseSearchCondition(op2, parseNode->getChild(2/*boolean_tern*/)))
                return ERROR;

            exp = std::make_unique<BinaryBooleanExp>(std::move(op1), BooleanSqlOperator::Or, std::move(op2));
            return SUCCESS;
            }
        case OSQLParseNode::boolean_term:
            {
            std::unique_ptr<BooleanExp> op1 = nullptr;
            if (SUCCESS != ParseSearchCondition(op1, parseNode->getChild(0/*search_condition*/)))
                return ERROR;

            std::unique_ptr<BooleanExp> op2 = nullptr;
            if (SUCCESS != ParseSearchCondition(op2, parseNode->getChild(2/*boolean_tern*/)))
                return ERROR;

            exp = std::make_unique<BinaryBooleanExp>(std::move(op1), BooleanSqlOperator::And, std::move(op2));
            return SUCCESS;
            }
        case OSQLParseNode::boolean_factor:
            {
            std::unique_ptr<BooleanExp> operandValueExp = nullptr;
            if (SUCCESS != ParseSearchCondition(operandValueExp, parseNode->getChild(1/*boolean_test*/)))
                return ERROR;

            exp = std::make_unique<BooleanFactorExp>(std::move(operandValueExp), true);
            return SUCCESS;
            }
        case OSQLParseNode::boolean_test:
            {
            std::unique_ptr<BooleanExp> op1 = nullptr;
            if (SUCCESS != ParseSearchCondition(op1, parseNode->getChild(0/*boolean_primary*/)))
                return ERROR;

            bool isNot = false;
            if (SUCCESS != ParseNotToken(isNot, parseNode->getChild(2/*sql_not*/)))
                return ERROR;

            std::unique_ptr<ValueExp> truthValueExp = nullptr;
            if (SUCCESS != ParseTruthValue(truthValueExp, parseNode->getChild(3/*truth_value*/)))
                return ERROR;

            // X IS [NOT] NULL|TRUE|FALSE|UNKNOWN
            exp = std::make_unique<BinaryBooleanExp>(std::move(op1), isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is, std::move(truthValueExp));
            return SUCCESS;
            }
        case OSQLParseNode::boolean_primary:
            {
            BeAssert(parseNode->count() == 3);
            if (SUCCESS != ParseSearchCondition(exp, parseNode->getChild(1/*search_condition*/)))
                return ERROR;

            exp->SetHasParentheses();
            return SUCCESS;
            }
        case OSQLParseNode::comparison_predicate:
            {
            if (parseNode->count() == 3 /*row_value_constructor comparison row_value_constructor*/)
                {
                std::unique_ptr<ValueExp> op1 = nullptr;
                if (SUCCESS != ParseRowValueConstructor(op1, parseNode->getChild(0/*row_value_constructor*/)))
                    return ERROR;

                BooleanSqlOperator op = BooleanSqlOperator::And;
                if (SUCCESS != ParseComparison(op, parseNode->getChild(1/*comparison*/)))
                    return ERROR;

                std::unique_ptr<ValueExp> op2 = nullptr;
                if (SUCCESS != ParseRowValueConstructor(op2, parseNode->getChild(2/*row_value_constructor*/)))
                    return ERROR;

                exp = std::make_unique<BinaryBooleanExp>(std::move(op1), op, std::move(op2));
                return SUCCESS;
                }

            break;
            }
        case OSQLParseNode::between_predicate:
            {
            std::unique_ptr<ValueExp> lhsOperand = nullptr;
            if (SUCCESS != ParseRowValueConstructor(lhsOperand, parseNode->getChild(0/*row_value_constructor*/)))
                return ERROR;

            OSQLParseNode const* between_predicate_part_2 = parseNode->getChild(1/*between_predicate_part_2*/);
            bool isNot = false;
            if (SUCCESS != ParseNotToken(isNot, between_predicate_part_2->getChild(0/*sql_not*/)))
                return ERROR;

            const BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotBetween : BooleanSqlOperator::Between;

            std::unique_ptr<ValueExp> lowerBound = nullptr;
            if (SUCCESS != ParseRowValueConstructor(lowerBound, between_predicate_part_2->getChild(2/*row_value_constructor*/)))
                return ERROR;

            std::unique_ptr<ValueExp> upperBound = nullptr;
            if (SUCCESS != ParseRowValueConstructor(upperBound, between_predicate_part_2->getChild(4/*row_value_constructor*/)))
                return ERROR;

            exp = std::make_unique<BinaryBooleanExp>(std::move(lhsOperand), op, std::make_unique<BetweenRangeValueExp>(std::move(lowerBound), std::move(upperBound)));
            return SUCCESS;
            }
        case OSQLParseNode::all_or_any_predicate:
            {
            std::unique_ptr<ValueExp> op1 = nullptr;
            if (SUCCESS != ParseRowValueConstructor(op1, parseNode->getChild(0/*comparison*/)))
                return ERROR;

            OSQLParseNode const* quantified_comparison_predicate_part_2 = parseNode->getChild(1/*quantified_comparison_predicate_part_2*/);

            BooleanSqlOperator comparison;
            if (SUCCESS != ParseComparison(comparison, quantified_comparison_predicate_part_2->getChild(0/*comparison*/)))
                return ERROR;

            SqlCompareListType any_all_some;
            if (SUCCESS != ParseAnyOrAllOrSomeToken(any_all_some, quantified_comparison_predicate_part_2->getChild(1/*any_all_some*/)))
                return ERROR;

            std::unique_ptr<SubqueryExp> subquery = nullptr;
            if (SUCCESS != ParseSubquery(subquery, quantified_comparison_predicate_part_2->getChild(2/*subquery*/)))
                return ERROR;

            exp = std::make_unique<AllOrAnyExp>(std::move(op1), comparison, any_all_some, std::move(subquery));
            return SUCCESS;
            }
        case OSQLParseNode::existence_test:
            {
            std::unique_ptr<SubqueryExp> subquery = nullptr;
            if (SUCCESS != ParseSubquery(subquery, parseNode->getChild(1/*subquery*/)))
                return ERROR;

            exp = std::make_unique<SubqueryTestExp>(SubqueryTestOperator::Exists, std::move(subquery));
            return SUCCESS;
            }
        case OSQLParseNode::unique_test:
            {
            std::unique_ptr<SubqueryExp> subquery = nullptr;
            if (SUCCESS != ParseSubquery(subquery, parseNode->getChild(1/*subquery*/)))
                return ERROR;

            exp = std::make_unique<SubqueryTestExp>(SubqueryTestOperator::Unique, std::move(subquery));
            return SUCCESS;
            }

        case OSQLParseNode::test_for_null:
            {
            std::unique_ptr<ValueExp> row_value_constructor = nullptr;
            if (SUCCESS != ParseRowValueConstructor(row_value_constructor, parseNode->getChild(0/*row_value_constructor*/)))
                return ERROR;

            OSQLParseNode const* null_predicate_part_2 = parseNode->getChild(1/*row_value_constructor*/);
            bool isNot = false;
            if(SUCCESS != ParseNotToken(isNot, null_predicate_part_2->getChild(1/*sql_not*/)))
                return ERROR;

            std::unique_ptr<ValueExp> nullExp = nullptr;
            if (SUCCESS != ParseValueExp(nullExp, null_predicate_part_2->getChild(2/*NULL*/)))
                return ERROR;

            exp = std::make_unique<BinaryBooleanExp>(std::move(row_value_constructor), isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is,
                                                                    std::move(nullExp));
            return SUCCESS;
            }
        case OSQLParseNode::in_predicate:
            return ParseInPredicate(exp, parseNode);

        case OSQLParseNode::like_predicate:
            return ParseLikePredicate(exp, parseNode);

        case OSQLParseNode::rtreematch_predicate:
            return ParseRTreeMatchPredicate(exp, parseNode);
        }

    std::unique_ptr<ValueExp> valueExp = nullptr;
    if (SUCCESS != ParseValueExp(valueExp, parseNode))
        return ERROR;

    exp = std::make_unique<UnaryPredicateExp>(std::move(valueExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool ECSqlParser::IsPredicate(OSQLParseNode const& parseNode)
    {
    const OSQLParseNode::Rule rule = parseNode.getKnownRuleID();
    switch (rule)
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseInPredicate(std::unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, in_predicate))
        {
        BeAssert(false && "Wrong grammar. Expecting in_predicate");
        return ERROR;
        }

    OSQLParseNode const* firstChildNode = parseNode->getChild(0);
    if (SQL_ISRULE(firstChildNode, in_predicate_part_2))
        {
        BeAssert(false);
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0490, "IN predicate without left-hand side property not supported.");
        return ERROR;
        }

    //first item is row value ctor node
    std::unique_ptr<ValueExp> lhsExp = nullptr;
    if (SUCCESS != ParseRowValueConstructor(lhsExp, firstChildNode))
        return ERROR;

    BooleanSqlOperator inOperator = BooleanSqlOperator::In;
    std::unique_ptr<ComputedExp> rhsExp = nullptr;
    if (SUCCESS != ParseInPredicatePart2(rhsExp, inOperator, parseNode->getChild(1)))
        return ERROR;

    exp = std::make_unique<BinaryBooleanExp>(std::move(lhsExp), inOperator, std::move(rhsExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseInPredicatePart2(std::unique_ptr<ComputedExp>& exp, BooleanSqlOperator& inOperator, OSQLParseNode const* parseNode) const
    {
    //in_predicate_part_2: sql_not SQL_TOKEN_IN in_predicate_value
    if (!SQL_ISRULE(parseNode, in_predicate_part_2))
        {
        BeAssert(false && "Invalid grammar. Expecting in_predicate_part_2");
        return ERROR;
        }

    bool isNot = false;
    if (SUCCESS != ParseNotToken(isNot, parseNode->getChild(0)))
        return ERROR;

    inOperator = isNot ? BooleanSqlOperator::NotIn : BooleanSqlOperator::In;

    //third item is the predicate value (second item is IN token)
    OSQLParseNode const* in_predicate_valueNode = parseNode->getChild(2);

    OSQLParseNode const* in_predicate_valueFirstChildNode = nullptr;
    if (SQL_ISRULE((in_predicate_valueFirstChildNode = in_predicate_valueNode->getChild(0)), subquery))
        {
        std::unique_ptr<ValueExp> valExp = nullptr;
        if (SUCCESS != ParseValueExp(valExp, in_predicate_valueFirstChildNode))
            return ERROR;

        exp = std::move(valExp);
        return SUCCESS;
        }

    //if no subquery it must be '(' value_exp_commalist ')'. Safety check is done in parse_value_exp_commalist method
    std::unique_ptr<ValueExpListExp> valueExpListExp = nullptr;
    if (SUCCESS != ParseValueExpCommalist(valueExpListExp, in_predicate_valueNode->getChild(1)))
        return ERROR;

    exp = std::move(valueExpListExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseLikePredicate(std::unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, like_predicate))
        {
        BeAssert(false && "Wrong grammar. Expecting like_predicate");
        return ERROR;
        }

    //first item is row value ctor node
    OSQLParseNode const* firstChildNode = parseNode->getChild(0);
    std::unique_ptr<ValueExp> lhsExp = nullptr;
    if (SUCCESS != ParseRowValueConstructor(lhsExp, firstChildNode))
        return ERROR;

    std::unique_ptr<ComputedExp> rhsExp = nullptr;
    BooleanSqlOperator likeOperator = BooleanSqlOperator::Like;
    if (SUCCESS != ParseLikePredicatePart2(rhsExp, likeOperator, parseNode->getChild(1)))
        return ERROR;

    exp = std::make_unique<BinaryBooleanExp>(std::move(lhsExp), likeOperator, std::move(rhsExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseLikePredicatePart2(std::unique_ptr<ComputedExp>& exp, BooleanSqlOperator& likeOperator, OSQLParseNode const* parseNode) const
    {
    //character_like_predicate_part_2: sql_not SQL_TOKEN_LIKE string_value_exp opt_escape
    //other_like_predicate_part_2: sql_not SQL_TOKEN_LIKE value_exp_primary opt_escape
    if (!SQL_ISRULE(parseNode, character_like_predicate_part_2) && !SQL_ISRULE(parseNode, other_like_predicate_part_2))
        {
        BeAssert(false && "Invalid grammar. Expecting character_like_predicate_part_2 or other_like_predicate_part_2");
        return ERROR;
        }

    bool isNot = false;
    BentleyStatus stat = ParseNotToken(isNot, parseNode->getChild(0));
    if (stat != SUCCESS)
        return stat;

    likeOperator = isNot ? BooleanSqlOperator::NotLike : BooleanSqlOperator::Like;

    //third item is value_exp_primary or string_value_exp value (second item is LIKE token)
    OSQLParseNode const* valueExpNode = parseNode->getChild(2);
    std::unique_ptr<ValueExp> rhsExp = nullptr;
    stat = ParseValueExp(rhsExp, valueExpNode);
    if (stat != SUCCESS)
        return stat;

    //fourth item is escape clause. Escape clause node always exists. If no escape was specified, the node is empty
    std::unique_ptr<ValueExp> escapeExp = nullptr;
    OSQLParseNode const* escapeClauseNode = parseNode->getChild(3);
    const size_t escapeChildNodeCount = escapeClauseNode->count();
    //no nodes means no escape clause
    if (escapeChildNodeCount > 0)
        {
        if (escapeChildNodeCount != 2)
            {
            BeAssert(false && "Invalid grammar. Corrupt opt_escape expression");
            return ERROR;
            }

        //second child node has escape expression (first node is ESCAPE token)
        stat = ParseValueExp(escapeExp, escapeClauseNode->getChild(1));
        if (stat != SUCCESS)
            return stat;
        }

    exp = std::make_unique<LikeRhsValueExp>(std::move(rhsExp), std::move(escapeExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseRTreeMatchPredicate(std::unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, rtreematch_predicate))
        {
        BeAssert(false && "Wrong grammar. Expecting rtreematch_predicate");
        return ERROR;
        }

    OSQLParseNode const* lhsNode = parseNode->getChild(0);
    std::unique_ptr<ValueExp> lhsExp = nullptr;
    BentleyStatus stat = ParseRowValueConstructor(lhsExp, lhsNode);
    if (stat != SUCCESS)
        return stat;

    //rest of predicate is in match_predicate_part_2 rule node
    OSQLParseNode const* part2Node = parseNode->getChild(1);

    bool isNot = false;
    stat = ParseNotToken(isNot, part2Node->getChild(0));
    if (stat != SUCCESS)
        return stat;

    const BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotMatch : BooleanSqlOperator::Match;

    //second child node is SQL_TOKEN_MATCH, and third therefore the rhs function call
    std::unique_ptr<ValueExp> rhsExp = nullptr;
    stat = ParseFctSpec(rhsExp, part2Node->getChild(2));
    if (stat != SUCCESS)
        return stat;

    exp = std::make_unique<BinaryBooleanExp>(std::move(lhsExp), op, std::move(rhsExp));
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseSubquery(std::unique_ptr<SubqueryExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, subquery))
        {
        BeAssert(false && "Wrong grammar. Expecting subquery");
        return ERROR;
        }

    OSQLParseNode const* queryExpNode = parseNode->getChild(1/*query_exp*/);
    if (SQL_ISRULE(queryExpNode, select_statement))
        {
        //select_statement
        std::unique_ptr<SelectStatementExp> compound_select = nullptr;
        if (SUCCESS != ParseSelectStatement(compound_select, *queryExpNode))
            return ERROR;

        exp = std::make_unique<SubqueryExp>(std::move(compound_select));
        }
    else if(SQL_ISRULE(queryExpNode, cte))
        {
        //cte
        std::unique_ptr<CommonTableExp> cte = nullptr;
        if (SUCCESS != ParseCTE(cte, queryExpNode))
            return ERROR;

        exp = std::make_unique<SubqueryExp>(std::move(cte));
        }

    OSQLParseNode const* valuesCommalistNode = parseNode->getChild(2/*values_commalist*/);
    if (SQL_ISRULE(valuesCommalistNode, values_commalist))
        {
        //values_commalist
        std::unique_ptr<SelectStatementExp> compound_select = nullptr;
        if (SUCCESS != ParseValuesCommalist(compound_select, *valuesCommalistNode))
            return ERROR;

        exp = std::make_unique<SubqueryExp>(std::move(compound_select));
        }
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseValuesCommalist(std::unique_ptr<SelectStatementExp>& exp, OSQLParseNode const& parseNode) const
    {
    if (!SQL_ISRULE(&parseNode, values_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting values_commalist");
        return ERROR;
        }

    // 1st:(, 2nd: row_value_constructor_commalist, 3rd:)
    BeAssert(parseNode.count() % 3 == 0);

    std::unique_ptr<SelectStatementExp> prevSelectExp = nullptr;
    // traverse in reverse order so the converted native SQL is in the same order as input
    for (int i = (int)parseNode.count() - 1; i >= 0; i = i - 3)
        {
        OSQLParseNode const* listNode = parseNode.getChild(i - 1);
        if (listNode == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        if (!SQL_ISRULE(listNode, row_value_constructor_commalist))
            {
            BeAssert(false && "Invalid grammar. Expecting row_value_constructor_commalist");
            return ERROR;
            }

        std::vector<std::unique_ptr<ValueExp>> valueExpList;
        if (SUCCESS != ParseRowValueConstructorCommalist(valueExpList, *listNode))
            return ERROR;

        std::unique_ptr<SingleSelectStatementExp> singleSelect = std::make_unique<SingleSelectStatementExp>(valueExpList);
        if (prevSelectExp == nullptr)
            prevSelectExp = std::make_unique<SelectStatementExp>(std::move(singleSelect));
        else
            {
            std::unique_ptr<SelectStatementExp> selectExp = nullptr;
            selectExp = std::make_unique<SelectStatementExp>(std::move(singleSelect), SelectStatementExp::CompoundOperator::Union, true /*isAll*/, std::move(prevSelectExp));
            prevSelectExp = std::move(selectExp);
            }
        }

    exp = std::move(prevSelectExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseRowValueConstructorCommalist(std::vector<std::unique_ptr<ValueExp>>& valueExpList, OSQLParseNode const& parseNode) const
    {
    if (!SQL_ISRULE(&parseNode, row_value_constructor_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting row_value_constructor_commalist");
        return ERROR;
        }

    const size_t childCount = parseNode.count();
    for (size_t i = 0; i < childCount; i++)
        {
        OSQLParseNode const* childNode = parseNode.getChild(i);
        if (childNode == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }

        std::unique_ptr<ValueExp> valueExp = nullptr;
        if (SUCCESS != ParseRowValueConstructor(valueExp, childNode))
            return ERROR;

        valueExpList.push_back(std::move(valueExp));
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseComparison(BooleanSqlOperator& op, OSQLParseNode const* parseNode) const
    {
    op = BooleanSqlOperator::LessThanOrEqualTo;

    if (SQL_ISRULE(parseNode, comparison))
        {
        if (parseNode->count() == 4 /*SQL_TOKEN_IS sql_not SQL_TOKEN_DISTINCT SQL_TOKEN_FROM*/)
            {
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0491, "'IS [NOT] DISTINCT FROM' operator not supported in ECSQL.");
            return ERROR;
            }
        if (parseNode->count() == 2 /*SQL_TOKEN_IS sql_not*/)
            {
            bool isNot = false;
            BentleyStatus stat = ParseNotToken(isNot, parseNode->getChild(1/*sql_not*/));
            if (stat != SUCCESS)
                return stat;

            op = isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is;
            return SUCCESS;
            }
        }

    switch (parseNode->getNodeType())
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseAnyOrAllOrSomeToken(SqlCompareListType& compareListType, OSQLParseNode const* parseNode) const
    {
    compareListType = SqlCompareListType::All;
    switch (parseNode->getTokenID())
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
                BeAssert(false && "Invalid grammar");
                return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseNotToken(bool& isNot, OSQLParseNode const* parseNode) const
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

    BeAssert(false && "Invalid grammar");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseAllToken(bool& isAll, OSQLParseNode const* parseNode) const
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseGroupByClause(std::unique_ptr<GroupByExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, opt_group_by_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_group_by_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS; //User never provided a GROUP BY clause

    std::unique_ptr<ValueExpListExp> listExp = nullptr;
    if (SUCCESS != ParseValueExpCommalist(listExp, parseNode->getChild(2)))
        return ERROR;

    exp = std::make_unique<GroupByExp>(std::move(listExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseHavingClause(std::unique_ptr<HavingExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, opt_having_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_having_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS; //User never provided a HAVING clause

    std::unique_ptr<BooleanExp> searchConditionExp = nullptr;
    BentleyStatus stat = ParseSearchCondition(searchConditionExp, parseNode->getChild(1));
    if (stat != SUCCESS)
        return stat;

    exp = std::make_unique<HavingExp>(std::move(searchConditionExp));
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseAscOrDescToken(OrderBySpecExp::SortDirection& sortDirection, OSQLParseNode const* parseNode) const
    {
    sortDirection = OrderBySpecExp::SortDirection::NotSpecified;
    if (SQL_ISRULE(parseNode, opt_asc_desc))
        return SUCCESS; //not specified

    switch (parseNode->getTokenID())
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseOrderByClause(std::unique_ptr<OrderByExp>& exp, OSQLParseNode const* parseNode) const
    {
    exp = nullptr;

    if (!SQL_ISRULE(parseNode, opt_order_by_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_order_by_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS; //User never provided a ORDER BY clause

    std::vector<std::unique_ptr<OrderBySpecExp>> orderBySpecs;
    OSQLParseNode const* ordering_spec_commalist = parseNode->getChild(2 /*ordering_spec_commalist*/);
    for (size_t nPos = 0; nPos < ordering_spec_commalist->count(); nPos++)
        {
        OSQLParseNode const* ordering_spec = ordering_spec_commalist->getChild(nPos);
        OSQLParseNode const* row_value_constructor_elem = ordering_spec->getChild(0/*row_value_constructor_elem*/);
        std::unique_ptr<ComputedExp> sortValue = nullptr;
        if (IsPredicate(*row_value_constructor_elem))
            {
            std::unique_ptr<BooleanExp> predExp = nullptr;
            if (SUCCESS != ParseSearchCondition(predExp, row_value_constructor_elem))
                return ERROR;

            sortValue = std::move(predExp);
            }
        else
            {
            std::unique_ptr<ValueExp> valueExp = nullptr;
            if (SUCCESS != ParseRowValueConstructor(valueExp, row_value_constructor_elem))
                return ERROR;

            sortValue = std::move(valueExp);
            }

        OrderBySpecExp::SortDirection sortDirection = OrderBySpecExp::SortDirection::NotSpecified;
        if (SUCCESS != ParseAscOrDescToken(sortDirection, ordering_spec->getChild(1/*opt_asc_desc*/)))
            return ERROR;

        auto nulls_order = ordering_spec->getChild(2 /*null order*/);
        auto nullsOrder = OrderBySpecExp::NullsOrder::NotSpecified;
        if ( nulls_order->count() == 2) {
            auto firstOrLast = nulls_order->getChild(1 /*FIRST OR LAST*/)->getTokenID();
            if (firstOrLast == SQL_TOKEN_FIRST) {
                nullsOrder = OrderBySpecExp::NullsOrder::First;
            } else {
                nullsOrder = OrderBySpecExp::NullsOrder::Last;
            }
        }
        orderBySpecs.push_back(std::make_unique<OrderBySpecExp>(sortValue, sortDirection, nullsOrder));
        }

    exp = std::make_unique<OrderByExp>(orderBySpecs);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseLimitOffsetClause(std::unique_ptr<LimitOffsetExp>& exp, OSQLParseNode const* parseNode) const
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

    std::unique_ptr<ValueExp> limitExpr = nullptr;
    if (SUCCESS != ParseValueExp(limitExpr, parseNode->getChild(1)))
        return ERROR;

    OSQLParseNode const* offsetNode = parseNode->getChild(2);
    if (offsetNode == nullptr)
        {
        BeAssert(false && "Invalid grammar. Offset parse node is never expected to be null in limit_offset_clause");
        return ERROR;
        }

    if (offsetNode->count() == 0)
        {
        exp = std::make_unique<LimitOffsetExp>(std::move(limitExpr));
        return SUCCESS;
        }

    std::unique_ptr<ValueExp> offsetExpr = nullptr;
    if (SUCCESS != ParseValueExp(offsetExpr, offsetNode->getChild(1)))
        return ERROR;

    exp = std::make_unique<LimitOffsetExp>(std::move(limitExpr), std::move(offsetExpr));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseLiteral(Utf8StringR literalVal, ECSqlTypeInfo& dataType, connectivity::OSQLParseNode const& parseNode) const
    {
    //constant value
    literalVal = nullptr;
    switch (parseNode.getNodeType())
        {
            case SQL_NODE_INTNUM:
                literalVal.assign(parseNode.getTokenValue());
                dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
                break;
            case SQL_NODE_APPROXNUM:
                literalVal.assign(parseNode.getTokenValue());
                dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double);
                break;
            case SQL_NODE_STRING:
                literalVal.assign(parseNode.getTokenValue());
                dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
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
                dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
                }
            else if (parseNode.getTokenID() == SQL_TOKEN_FALSE)
                {
                literalVal.assign("FALSE");
                dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseAllOrDistinctToken(SqlSetQuantifier& setQuantifier, OSQLParseNode const* parseNode) const
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseSelectCompoundOperator(SelectStatementExp::CompoundOperator& op, OSQLParseNode const* parseNode) const
    {
    if (SQL_ISTOKEN(parseNode, UNION))
        op = SelectStatementExp::CompoundOperator::Union;
    else if (SQL_ISTOKEN(parseNode, INTERSECT))
        op = SelectStatementExp::CompoundOperator::Intersect;
    else if (SQL_ISTOKEN(parseNode, EXCEPT))
        op = SelectStatementExp::CompoundOperator::Except;
    else
        op = SelectStatementExp::CompoundOperator::None;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseUnaryPredicate(std::unique_ptr<BooleanExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, unary_predicate) || parseNode->count() != 1)
        {
        BeAssert(false && "Invalid grammar. Expecting unary_predicate");
        return ERROR;
        }

    std::unique_ptr<ValueExp> valueExp = nullptr;
    if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(0)))
        return ERROR;

    exp = std::make_unique<UnaryPredicateExp>(std::move(valueExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseSelectStatement(std::unique_ptr<SelectStatementExp>& exp, OSQLParseNode const& parseNode) const
    {
    if (!SQL_ISRULE(&parseNode, select_statement))
        {
        BeAssert(false && "Invalid grammar. Expecting select_statement with four child nodes");
        return ERROR;
        }

    if (parseNode.count() == 1)
        {
        std::unique_ptr<SingleSelectStatementExp> single_select = nullptr;
        if (SUCCESS != ParseSingleSelectStatement(single_select, parseNode.getChild(0)))
            return ERROR;

        exp = std::make_unique<SelectStatementExp>(std::move(single_select));
        return SUCCESS;
        }

    if (parseNode.count() == 4)
        {
        std::unique_ptr<SingleSelectStatementExp> single_select = nullptr;
        if (SUCCESS != ParseSingleSelectStatement(single_select, parseNode.getChild(0)))
            return ERROR;

        if (!single_select->IsCoreSelect())
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0492,
                "SELECT statement in UNION must not contain ORDER BY or LIMIT clause: %s", single_select->ToECSql().c_str());
            return ERROR;
            }

        SelectStatementExp::CompoundOperator op = SelectStatementExp::CompoundOperator::None;
        if (SUCCESS != ParseSelectCompoundOperator(op, parseNode.getChild(1)))
            return ERROR;

        bool isAll = false;
        if (SUCCESS != ParseAllToken(isAll, parseNode.getChild(2)))
            return ERROR;

        std::unique_ptr<SelectStatementExp> compound_select = nullptr;
        if (SUCCESS != ParseSelectStatement(compound_select, *parseNode.getChild(3)))
            return ERROR;

        exp = std::make_unique<SelectStatementExp>(std::move(single_select), op, isAll, std::move(compound_select));
        return SUCCESS;
        }

    BeAssert(false && "Invalid grammar. Expecting select_statement with four child nodes or exactly one child");
    return ERROR;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseCaseExp(std::unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, searched_case))
        {
        BeAssert(false && "Invalid grammar. Expecting searched_case");
        return ERROR;
        }

    const auto searched_when_clause_list = parseNode->getChild(1);
    const auto else_clause = parseNode->getChild(2);
    const size_t childCount = searched_when_clause_list->count();
    std::vector<std::unique_ptr<SearchedWhenClauseExp>> whenList;
    for (size_t i = 0; i < childCount; i++)
        {
        std::unique_ptr<BooleanExp> whenExp = nullptr;
        std::unique_ptr<ValueExp> thenExp = nullptr;
        const auto when = searched_when_clause_list->getChild(i)->getChild(1);
        const auto then = searched_when_clause_list->getChild(i)->getChild(3);

        if (SUCCESS != ParseSearchCondition(whenExp, when))
            return ERROR;

        if (SUCCESS != ParseValueExp(thenExp, then))
            return ERROR;

        whenList.push_back(std::make_unique<SearchedWhenClauseExp>(whenExp, thenExp));
        }

    std::unique_ptr<ValueExp> elseExp = nullptr;
    if (else_clause->count() > 0)
        {
        if (SUCCESS != ParseValueExp(elseExp, else_clause->getChild(1)))
            return ERROR;
        }

    valueExp = std::make_unique<SearchCaseValueExp>(whenList, elseExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseIIFExp(std::unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, iif_spec))
        {
        BeAssert(false && "Invalid grammar. Expecting iif_spec");
        return ERROR;
        }

    const auto cond = parseNode->getChild(1);
    const auto trueVal = parseNode->getChild(2);
    const auto falseVal = parseNode->getChild(3);

    std::unique_ptr<BooleanExp> whenExp = nullptr;
    if (SUCCESS != ParseSearchCondition(whenExp, cond))
        return ERROR;

    std::unique_ptr<ValueExp> thenExp = nullptr;
    if (SUCCESS != ParseValueExp(thenExp, trueVal))
        return ERROR;

    std::unique_ptr<ValueExp> elseExp = nullptr;
    if (SUCCESS != ParseValueExp(elseExp, falseVal))
        return ERROR;

    valueExp = std::make_unique<IIFExp>(whenExp, thenExp, elseExp);
    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseTypePredicate(std::unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, type_predicate))
        {
        BeAssert(false && "Invalid grammar. Expecting type_predicate");
        return ERROR;
        }

    const auto type_list = parseNode->getChild(1);
    const auto count = type_list->count();
    std::vector<std::unique_ptr<ClassNameExp>> typeList;
    for (int i = 0; i < count; i++)
        {
        const auto type_list_item = type_list->getChild(i);
        const auto opt_only = type_list_item->getChild(0);
        PolymorphicInfo polyConstraint;
        if (SUCCESS != ParsePolymorphicConstraint(polyConstraint, opt_only))
            return ERROR;

        const auto table_node = type_list_item->getChild(1);
        std::unique_ptr<ClassNameExp> classNameExp = nullptr;
        BentleyStatus stat = ParseTableNode(classNameExp, *table_node, ECSqlType::Select, polyConstraint);
        if (SUCCESS != stat)
            return stat;

        typeList.push_back(std::move(classNameExp));
        }

    valueExp = std::make_unique<TypeListExp>(typeList);
    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseValueExp(std::unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    BeAssert(parseNode != nullptr);
    if (parseNode->isRule())
        {
        switch (parseNode->getKnownRuleID())
            {
                case OSQLParseNode::cast_spec:
                    return ParseCastSpec(valueExp, parseNode);
                case OSQLParseNode::column_ref:
                    return ParseColumnRef(valueExp, parseNode, false);
                case OSQLParseNode::num_value_exp:
                    return ParseNumValueExp(valueExp, parseNode);
                case OSQLParseNode::term_add_sub:
                    return ParseTermAddSub(valueExp, parseNode);
                case OSQLParseNode::concatenation:
                    return ParseConcatenation(valueExp, parseNode);
                case OSQLParseNode::datetime_value_exp:
                    return ParseDatetimeValueExp(valueExp, parseNode);
                case OSQLParseNode::factor:
                    return ParseFactor(valueExp, parseNode);
                case OSQLParseNode::aggregate_fct:
                    return ParseAggregateFct(valueExp, parseNode);
                case OSQLParseNode::fct_spec:
                    return ParseFctSpec(valueExp, parseNode);
                case OSQLParseNode::property_path:
                    return ParseExpressionPath(valueExp, parseNode);
                case OSQLParseNode::term:
                    return ParseTerm(valueExp, parseNode);
                case OSQLParseNode::parameter:
                    return ParseParameter(valueExp, parseNode);
                case OSQLParseNode::searched_case:
                    return ParseCaseExp(valueExp, parseNode);
                case OSQLParseNode::iif_spec:
                    return ParseIIFExp(valueExp, parseNode);
                case OSQLParseNode::type_predicate:
                    return ParseTypePredicate(valueExp, parseNode);
                case OSQLParseNode::subquery:
                {
                std::unique_ptr<SubqueryExp> subQueryExp = nullptr;
                //Must return just one column in select list
                //We can tell that until we resolve all the columns
                BentleyStatus stat = ParseSubquery(subQueryExp, parseNode);
                if (SUCCESS != stat)
                    return stat;

                valueExp = std::make_unique<SubqueryValueExp>(std::move(subQueryExp));
                return SUCCESS;
                }
                case OSQLParseNode::value_exp_primary:
                    return ParseValueExpPrimary(valueExp, parseNode);
                case OSQLParseNode::window_function:
                    return ParseWindowFunction(valueExp, parseNode);
                case OSQLParseNode::value_creation_fct:
                    return ParseValueCreationFuncExp(valueExp, parseNode);
                default:
                    Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0493,
                        "ECSQL Parse error: Unsupported value_exp type: %d", (int) parseNode->getKnownRuleID());
                    return ERROR;

            };
        }

    //constant value
    Utf8String value;
    ECSqlTypeInfo dataType;
    if (SUCCESS != ParseLiteral(value, dataType, *parseNode))
        return ERROR;

    return LiteralValueExp::Create(valueExp, *m_context, value.c_str(), dataType);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseValueExpCommalist(std::unique_ptr<ValueExpListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, value_exp_commalist))
        {
        BeAssert(false && "Invalid grammar. Expecting value_exp_commalist");
        return ERROR;
        }

    std::unique_ptr<ValueExpListExp> valueListExp = std::make_unique<ValueExpListExp>();
    const size_t childCount = parseNode->count();
    for (size_t i = 0; i < childCount; i++)
        {
        std::unique_ptr<ValueExp> valueExp = nullptr;
        if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(i)))
            return ERROR;

        valueListExp->AddValueExp(valueExp);
        }

    exp = std::move(valueListExp);
    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseValuesOrQuerySpec(std::vector<std::unique_ptr<ValueExp>>& valueExpList, OSQLParseNode const& parseNode) const
    {
    if (!SQL_ISRULE(&parseNode, values_or_query_spec))
        {
        BeAssert(false && "Invalid grammar. Expecting values_or_query_spec");
        return ERROR;
        }

    //1st: VALUES, 2nd:(, 3rd: row_value_constructor_commalist, 4th:)
    BeAssert(parseNode.count() == 4);
    OSQLParseNode const* listNode = parseNode.getChild(2);
    if (listNode == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    return ParseRowValueConstructorCommalist(valueExpList, *listNode);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowClause(std::unique_ptr<WindowFunctionClauseExp>& windowFunctionClauseExp, OSQLParseNode const *parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_window_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_window_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS;

    std::unique_ptr<WindowDefinitionListExp> windowDefinitionListExp = nullptr;
    if (SUCCESS != ParseWindowDefinitionListExp(windowDefinitionListExp, parseNode->getChild(1)))
        return ERROR;

    windowFunctionClauseExp = std::make_unique<WindowFunctionClauseExp>(std::move(windowDefinitionListExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowDefinitionListExp(std::unique_ptr<WindowDefinitionListExp>& windowDefinitionListExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_definition_list))
        {
        BeAssert(false && "Invalid grammar. Expecting window_definition_list");
        return ERROR;
        }

    std::vector<std::unique_ptr<WindowDefinitionExp>> windowDefinitionList;
    for (size_t nPos = 0; nPos < parseNode->count(); nPos++)
        {
        std::unique_ptr<WindowDefinitionExp> windowDefinitionExp = nullptr;
        if (SUCCESS != ParseWindowDefinitionExp(windowDefinitionExp, parseNode->getChild(nPos)))
            return ERROR;

        windowDefinitionList.push_back(std::move(windowDefinitionExp));
        }

    windowDefinitionListExp = std::make_unique<WindowDefinitionListExp>(windowDefinitionList);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowDefinitionExp(std::unique_ptr<WindowDefinitionExp>& windowDefinitionExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_definition))
        {
        BeAssert(false && "Invalid grammar. Expecting window_definition");
        return ERROR;
        }

    std::unique_ptr<WindowSpecification> windowSpecification = nullptr;
    if (SUCCESS != ParseWindowSpecification(windowSpecification, parseNode->getChild(2)))
        return ERROR;

    windowDefinitionExp = std::make_unique<WindowDefinitionExp>(parseNode->getChild(0)->getTokenValue(), std::move(windowSpecification));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFunction(std::unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_function))
        {
        BeAssert(false && "Invalid grammar. Expecting window_function");
        return ERROR;
        }

    std::unique_ptr<ValueExp> functionCallExp = nullptr;

    if (SUCCESS != ParseWindowFunctionType(functionCallExp, parseNode->getChild(0)))
        return ERROR;

    std::unique_ptr<FilterClauseExp> filterClauseExp = nullptr;
    if (SUCCESS != ParseFilterClause(filterClauseExp, parseNode->getChild(1)))
        return ERROR;

    if (SQL_ISRULE(parseNode->getChild(3), window_specification))
        {
        std::unique_ptr<WindowSpecification> windowSpecificationExp = nullptr;
        if (SUCCESS != ParseWindowSpecification(windowSpecificationExp, parseNode->getChild(3)))
            return ERROR;

        valueExp = std::make_unique<WindowFunctionExp>(std::move(functionCallExp), std::move(filterClauseExp), std::move(windowSpecificationExp));
        return SUCCESS;
        }
    else if (parseNode->getChild(3)->getNodeType() == SQLNodeType::SQL_NODE_NAME)
        {
        Utf8CP windowName = parseNode->getChild(3)->getTokenValue().c_str();
        valueExp = std::make_unique<WindowFunctionExp>(std::move(functionCallExp), std::move(filterClauseExp), windowName);
        return SUCCESS;
        }

    BeAssert(false && "Invalid grammar. Expecting window_name");
    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFunctionType(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    switch (parseNode->getKnownRuleID())
        {
        case OSQLParseNode::window_function_type:
            return ParseArgumentlessWindowFunction(exp, parseNode);
        case OSQLParseNode::aggregate_fct:
            return ParseAggregateFct(exp, parseNode);
        case OSQLParseNode::ntile_function:
            return ParseNtileFunction(exp, parseNode);
        case OSQLParseNode::lead_or_lag_function:
            return ParseLeadOrLagFunction(exp, parseNode);
        case OSQLParseNode::first_or_last_value_function:
            return ParseFirstOrLastValueFunction(exp, parseNode);
        case OSQLParseNode::nth_value_function:
            return ParseNthValueFunction(exp, parseNode);
        default:
            BeAssert(false && "Unsupported window function type");
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0668, "Unsupported window function type");
            return ERROR;
        }

    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseArgumentlessWindowFunction(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_function_type))
        {
        BeAssert(false && "Invalid grammar. Expecting window_function_type");
        return ERROR;
        }

    switch (parseNode->getChild(0)->getTokenID())
        {
        case SQL_TOKEN_ROW_NUMBER:
            {
            exp = std::make_unique<FunctionCallExp>("ROW_NUMBER");
            return SUCCESS;
            }
        case SQL_TOKEN_RANK:
            {
            exp = std::make_unique<FunctionCallExp>("RANK");
            return SUCCESS;
            }
        case SQL_TOKEN_DENSE_RANK:
            {
            exp = std::make_unique<FunctionCallExp>("DENSE_RANK");
            return SUCCESS;
            }
        case SQL_TOKEN_PERCENT_RANK:
            {
            exp = std::make_unique<FunctionCallExp>("PERCENT_RANK");
            return SUCCESS;
            }
        case SQL_TOKEN_CUME_DIST:
            {
            exp = std::make_unique<FunctionCallExp>("CUME_DIST");
            return SUCCESS;
            }
        default:
            {
            BeAssert(false && "Unsupported window function");
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0669, "Unsupported window function type");
            return ERROR;
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseNtileFunction(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, ntile_function))
        {
        BeAssert(false && "Invalid grammar. Expecting ntile_function");
        return ERROR;
        }

    std::unique_ptr<FunctionCallExp> functionCallExp = std::make_unique<FunctionCallExp>("ntile");
    std::unique_ptr<ValueExp> firstArgument = nullptr;
    if (SUCCESS != ParseValueExp(firstArgument, parseNode->getChild(2)))
        return ERROR;

    functionCallExp->AddArgument(std::move(firstArgument));
    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseLeadOrLagFunction(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, lead_or_lag_function))
        {
        BeAssert(false && "Invalid grammar. Expecting lead_or_lag_function");
        return ERROR;
        }

    std::unique_ptr<FunctionCallExp> functionCallExp = nullptr;
    sal_uInt32 tokenId = parseNode->getChild(0)->getTokenID();
    if (tokenId == SQL_TOKEN_LEAD)
        functionCallExp = std::make_unique<FunctionCallExp>("LEAD");
    else if (tokenId == SQL_TOKEN_LAG)
        functionCallExp = std::make_unique<FunctionCallExp>("LAG");
    else
        {
        BeAssert(false && "Unsupported lead_or_lag_function type");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0670, "Unsupported lead_or_lag_function type");
        return ERROR;
        }
    
    std::unique_ptr<ValueExp> firstArgument = nullptr;
    if (SUCCESS != ParseValueExp(firstArgument, parseNode->getChild(2)))
        return ERROR;

    functionCallExp->AddArgument(std::move(firstArgument));

    if (SUCCESS != ParseOptLeadOrLagFunctionArguments(functionCallExp, parseNode->getChild(3)))
        return ERROR;

    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseOptLeadOrLagFunctionArguments(std::unique_ptr<FunctionCallExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_lead_or_lag_function))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_lead_or_lag_function");
        return ERROR;
        }
    
    if (parseNode->count() == 0)
        return SUCCESS;
    
    std::unique_ptr<ValueExp> secondArgument = nullptr;
    if (SUCCESS != ParseValueExp(secondArgument, parseNode->getChild(1)))
        return ERROR;
    
    exp->AddArgument(std::move(secondArgument));

    if (parseNode->count() == 2)
        return SUCCESS;

    std::unique_ptr<ValueExp> thirdArgument = nullptr;
    if (SUCCESS != ParseValueExp(thirdArgument, parseNode->getChild(3)))
        return ERROR;
    
    exp->AddArgument(std::move(thirdArgument));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseFirstOrLastValueFunction(std::unique_ptr<ValueExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, first_or_last_value_function))
        {
        BeAssert(false && "Invalid grammar. Expecting first_or_last_value_function");
        return ERROR;
        }
  
    std::unique_ptr<FunctionCallExp> functionCallExp = nullptr;    
    sal_uInt32 tokenId = parseNode->getChild(0)->getTokenID();
    if (tokenId == SQL_TOKEN_FIRST_VALUE)
        functionCallExp = std::make_unique<FunctionCallExp>("FIRST_VALUE");
    else if (tokenId == SQL_TOKEN_LAST_VALUE)
        functionCallExp = std::make_unique<FunctionCallExp>("LAST_VALUE");
    else
        {
        BeAssert(false && "Unsupported first_or_last_value_function");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0671, "Unsupported first_or_last_value_function");
        return ERROR;
        }
    
    std::unique_ptr<ValueExp> firstArgument = nullptr;
    if (SUCCESS != ParseValueExp(firstArgument, parseNode->getChild(2)))
        return ERROR;

    functionCallExp->AddArgument(std::move(firstArgument));
    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseNthValueFunction(std::unique_ptr<ValueExp>&exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, nth_value_function))
        {
        BeAssert(false && "Invalid grammar. Expecting nth_value_function");
        return ERROR;
        }

    std::unique_ptr<FunctionCallExp> functionCallExp = std::make_unique<FunctionCallExp>("NTH_VALUE");
    std::unique_ptr<ValueExp> firstArgument = nullptr;
    std::unique_ptr<ValueExp> secondArgument = nullptr;
    if (SUCCESS != ParseValueExp(firstArgument, parseNode->getChild(2)))
        return ERROR;
    
    if (SUCCESS != ParseValueExp(secondArgument, parseNode->getChild(4)))
        return ERROR;

    functionCallExp->AddArgument(std::move(firstArgument));
    functionCallExp->AddArgument(std::move(secondArgument));

    exp = std::move(functionCallExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowSpecification(std::unique_ptr<WindowSpecification>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_specification))
        {
        BeAssert(false && "Invalid grammar. Expecting window_specification");
        return ERROR;
        }

    std::unique_ptr<WindowPartitionColumnReferenceListExp> windowPartitionExp = nullptr;
    if (SUCCESS != ParseWindowPartitionClause(windowPartitionExp, parseNode->getChild(2)))
        return ERROR;
    
    std::unique_ptr<OrderByExp> orderByExp = nullptr;
    if (SUCCESS != ParseOrderByClause(orderByExp, parseNode->getChild(3)))
        return ERROR;

    std::unique_ptr<WindowFrameClauseExp> windowFrameClauseExp = nullptr;
    if (SUCCESS != ParseWindowFrameClause(windowFrameClauseExp, parseNode->getChild(4)))
        return ERROR;

    exp = std::make_unique<WindowSpecification>(parseNode->getChild(1)->getTokenValue(), std::move(windowPartitionExp), std::move(orderByExp), std::move(windowFrameClauseExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowPartitionClause(std::unique_ptr<WindowPartitionColumnReferenceListExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_window_partition_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting window_partition_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS;

    const OSQLParseNode* window_partition_column_reference_list = parseNode->getChild(2);
    std::vector<std::unique_ptr<WindowPartitionColumnReferenceExp>> windowPartitionColumnRefs;
    for (size_t nPos = 0; nPos < window_partition_column_reference_list->count(); nPos++)
        {
        std::unique_ptr<WindowPartitionColumnReferenceExp> windowPartitionColumnReferenceExp = nullptr;
        const OSQLParseNode* windowPartitionColumnRef = window_partition_column_reference_list->getChild(nPos);
        ParseWindowPartitionColumnRef(windowPartitionColumnReferenceExp, windowPartitionColumnRef);
        windowPartitionColumnRefs.push_back(std::move(windowPartitionColumnReferenceExp));
        }

    exp = std::make_unique<WindowPartitionColumnReferenceListExp>(windowPartitionColumnRefs);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowPartitionColumnRef(std::unique_ptr<WindowPartitionColumnReferenceExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_partition_column_reference))
        {
        BeAssert(false && "Invalid grammar. Expecting window_partition_column_reference");
        return ERROR;
        }
    
    std::unique_ptr<ValueExp> columnRefExp = nullptr;
    if (SUCCESS != ParseColumnRef(columnRefExp, parseNode->getChild(0), false))
        return ERROR;
    
    WindowPartitionColumnReferenceExp::CollateClauseFunction collateClauseFunction = WindowPartitionColumnReferenceExp::CollateClauseFunction::NotSpecified;
    if (SUCCESS != ParseCollateClause(collateClauseFunction, parseNode->getChild(1)))
            return ERROR;
    
    exp = std::make_unique<WindowPartitionColumnReferenceExp>(std::move(columnRefExp), collateClauseFunction);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseCollateClause(WindowPartitionColumnReferenceExp::CollateClauseFunction& collateClauseFunction, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_collate_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_collate_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS;

    switch (parseNode->getChild(1)->getTokenID())
        {
        case SQL_TOKEN_BINARY:
            collateClauseFunction = WindowPartitionColumnReferenceExp::CollateClauseFunction::Binary;
            return SUCCESS;
        case SQL_TOKEN_NOCASE:
            collateClauseFunction = WindowPartitionColumnReferenceExp::CollateClauseFunction::NoCase;
            return SUCCESS;
        case SQL_TOKEN_RTRIM:
            collateClauseFunction = WindowPartitionColumnReferenceExp::CollateClauseFunction::Rtrim;
            return SUCCESS;
        default:
            BeAssert(false && "Unsupported collate function");
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0672, "Unsupported collate function");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseFilterClause(std::unique_ptr<FilterClauseExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_filter_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_filter_clause");
        return ERROR;
        }
    
    if (parseNode->count() == 0)
        return SUCCESS;

    std::unique_ptr<WhereExp> whereExp = nullptr;
    
    if (SUCCESS != ParseWhereClause(whereExp, parseNode->getChild(2)))
        return ERROR;
    
    exp = std::make_unique<FilterClauseExp>(std::move(whereExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFrameClause(std::unique_ptr<WindowFrameClauseExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_window_frame_clause))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_window_frame_clause");
        return ERROR;
        }

    if (parseNode->count() == 0)
        return SUCCESS;

    WindowFrameClauseExp::WindowFrameUnit windowFrameUnit;
    if (SUCCESS != ParseWindowFrameUnit(windowFrameUnit, parseNode->getChild(0)))
        return ERROR;

    WindowFrameClauseExp::WindowFrameExclusionType windowFrameExclusionType = WindowFrameClauseExp::WindowFrameExclusionType::NotSpecified;
    if (SUCCESS != ParseWindowFrameExclusion(windowFrameExclusionType, parseNode->getChild(2)))
        return ERROR;
    
    if (SQL_ISRULE(parseNode->getChild(1), window_frame_start))
        {
        std::unique_ptr<WindowFrameStartExp> windowFrameStartExp = nullptr;
        if (SUCCESS != ParseWindowFrameStart(windowFrameStartExp, parseNode->getChild(1)))
            return ERROR;
        
        exp = std::make_unique<WindowFrameClauseExp>(windowFrameUnit, windowFrameExclusionType, std::move(windowFrameStartExp));
        return SUCCESS;
        }
    else if (SQL_ISRULE(parseNode->getChild(1), window_frame_between))
        {
        std::unique_ptr<WindowFrameBetweenExp> windowFrameBetweenExp = nullptr;
        if (SUCCESS != ParseWindowFrameBetween(windowFrameBetweenExp, parseNode->getChild(1)))
            return ERROR;
        
        exp = std::make_unique<WindowFrameClauseExp>(windowFrameUnit, windowFrameExclusionType, std::move(windowFrameBetweenExp));
        return SUCCESS;
        }
    else
        {
        BeAssert(false && "Unsupported collate window frame clause rule");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0673, "Unsupported collate window frame clause rule");
        return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFrameUnit(WindowFrameClauseExp::WindowFrameUnit& windowFrameUnit, OSQLParseNode const* parseNode) const
    {
    switch (parseNode->getTokenID())
        {
        case SQL_TOKEN_ROWS:
            windowFrameUnit = WindowFrameClauseExp::WindowFrameUnit::Rows;
            return SUCCESS;
        case SQL_TOKEN_RANGE:
            windowFrameUnit = WindowFrameClauseExp::WindowFrameUnit::Range;
            return SUCCESS;
        case SQL_TOKEN_GROUPS:
            windowFrameUnit = WindowFrameClauseExp::WindowFrameUnit::Groups;
            return SUCCESS;
        default:
            BeAssert(false && "Unsupported window frame unit");
            Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0674, "Unsupported window frame unit");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFrameStart(std::unique_ptr<WindowFrameStartExp>& exp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_frame_start))
        {
        BeAssert(false && "Invalid grammar. Expecting window_frame_start");
        return ERROR;
        }

    sal_uInt32 firstTokenId = parseNode->getChild(0)->getTokenID();
    sal_uInt32 secondTokenId = parseNode->getChild(1)->getTokenID();

    if (firstTokenId == SQL_TOKEN_UNBOUNDED && secondTokenId == SQL_TOKEN_PRECEDING)
        {
        exp = std::make_unique<WindowFrameStartExp>(WindowFrameStartExp::WindowFrameStartType::UnboundedPreceding);
        return SUCCESS;
        }
    else if (firstTokenId == SQL_TOKEN_CURRENT && secondTokenId == SQL_TOKEN_ROW)
        {
        exp = std::make_unique<WindowFrameStartExp>(WindowFrameStartExp::WindowFrameStartType::CurrentRow);
        return SUCCESS;
        }
    else if (secondTokenId == SQL_TOKEN_PRECEDING)
        {
        std::unique_ptr<ValueExp> valueExp = nullptr;
        if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(0)))
            return ERROR;
        
        exp = std::make_unique<WindowFrameStartExp>(WindowFrameStartExp::WindowFrameStartType::ValuePreceding, std::move(valueExp));
        return SUCCESS;
        }
    else
        {
        BeAssert(false && "Unsupported window frame start rule");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0675, "Unsupported window frame start rule");
        return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFrameBetween(std::unique_ptr<WindowFrameBetweenExp>& windowFrameBetweenExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, window_frame_between))
        {
        BeAssert(false && "Invalid grammar. Expecting window_frame_between");
        return ERROR;
        }
    
    std::unique_ptr<FirstWindowFrameBoundExp> firstWindowFrameBoundExp = nullptr;
    if (SUCCESS != ParseFirstWindowFrameBound(firstWindowFrameBoundExp, parseNode->getChild(1)))
        return ERROR;

    std::unique_ptr<SecondWindowFrameBoundExp> secondWindowFrameBoundExp = nullptr;
    if (SUCCESS != ParseSecondWindowFrameBound(secondWindowFrameBoundExp, parseNode->getChild(3)))
        return ERROR;

    windowFrameBetweenExp = std::make_unique<WindowFrameBetweenExp>(std::move(firstWindowFrameBoundExp), std::move(secondWindowFrameBoundExp));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseFirstWindowFrameBound(std::unique_ptr<FirstWindowFrameBoundExp>& firstWindowFrameBoundExp, OSQLParseNode const* parseNode) const
    {
    switch (parseNode->getKnownRuleID())
        {
        case OSQLParseNode::window_frame_bound_1:
            {
            if (parseNode->getChild(0)->getTokenID() == SQL_TOKEN_UNBOUNDED && parseNode->getChild(1)->getTokenID() == SQL_TOKEN_PRECEDING)
                {
                firstWindowFrameBoundExp = std::make_unique<FirstWindowFrameBoundExp>(FirstWindowFrameBoundExp::WindowFrameBoundType::UnboundedPreceding);
                return SUCCESS;
                }
            else
                {
                BeAssert(false && "Unsupported grammar for window_frame_bound_1.");
                return ERROR;
                }
            }
        case OSQLParseNode::window_frame_preceding:
            {
            std::unique_ptr<ValueExp> valueExp = nullptr;
            if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(0)))
                return ERROR;

            firstWindowFrameBoundExp = std::make_unique<FirstWindowFrameBoundExp>(std::move(valueExp), FirstWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding);
            return SUCCESS;
            }
        case OSQLParseNode::window_frame_following:
            {
            std::unique_ptr<ValueExp> valueExp = nullptr;
            if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(0)))
                return ERROR;
            
            firstWindowFrameBoundExp = std::make_unique<FirstWindowFrameBoundExp>(std::move(valueExp), FirstWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing);
            return SUCCESS;
            }
        case OSQLParseNode::window_frame_bound:
            {
            if (parseNode->getChild(0)->getTokenID() == SQL_TOKEN_CURRENT && parseNode->getChild(1)->getTokenID() == SQL_TOKEN_ROW)
                {
                firstWindowFrameBoundExp = std::make_unique<FirstWindowFrameBoundExp>(FirstWindowFrameBoundExp::WindowFrameBoundType::CurrentRow);
                return SUCCESS;
                }
            else
                {
                BeAssert(false && "Unsupported grammar for window_frame_bound.");
                return ERROR;
                }
            }
        default:
            BeAssert(false && "Unsupported rule for window_frame_bound_1.");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseSecondWindowFrameBound(std::unique_ptr<SecondWindowFrameBoundExp>& secondWindowFrameBoundExp, OSQLParseNode const* parseNode) const
    {
    switch (parseNode->getKnownRuleID())
        {
        case OSQLParseNode::window_frame_bound_2:
            {
            if (parseNode->getChild(0)->getTokenID() == SQL_TOKEN_UNBOUNDED && parseNode->getChild(1)->getTokenID() == SQL_TOKEN_FOLLOWING)
                {
                secondWindowFrameBoundExp = std::make_unique<SecondWindowFrameBoundExp>(SecondWindowFrameBoundExp::WindowFrameBoundType::UnboundedFollowing);
                return SUCCESS;
                }
            else
                {
                BeAssert(false && "Unsupported grammar for window_frame_bound_2.");
                return ERROR;
                }
            }
        case OSQLParseNode::window_frame_preceding:
            {
            std::unique_ptr<ValueExp> valueExp = nullptr;
            if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(0)))
                return ERROR;

            secondWindowFrameBoundExp = std::make_unique<SecondWindowFrameBoundExp>(std::move(valueExp), SecondWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding);
            return SUCCESS;
            }
        case OSQLParseNode::window_frame_following:
            {
            std::unique_ptr<ValueExp> valueExp = nullptr;
            if (SUCCESS != ParseValueExp(valueExp, parseNode->getChild(0)))
                return ERROR;
            
            secondWindowFrameBoundExp = std::make_unique<SecondWindowFrameBoundExp>(std::move(valueExp), SecondWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing);
            return SUCCESS;
            }
        case OSQLParseNode::window_frame_bound:
            {
            if (parseNode->getChild(0)->getTokenID() == SQL_TOKEN_CURRENT && parseNode->getChild(1)->getTokenID() == SQL_TOKEN_ROW)
                {
                secondWindowFrameBoundExp = std::make_unique<SecondWindowFrameBoundExp>(SecondWindowFrameBoundExp::WindowFrameBoundType::CurrentRow);
                return SUCCESS;
                }
            else
                {
                BeAssert(false && "Unsupported grammar for window_frame_bound.");
                return ERROR;
                }
            }
        default:
            BeAssert(false && "Unsupported rule for window_frame_bound_2.");
            return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParser::ParseWindowFrameExclusion(WindowFrameClauseExp::WindowFrameExclusionType& windowFrameExclusion, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, opt_window_frame_exclusion))
        {
        BeAssert(false && "Invalid grammar. Expecting opt_window_frame_exclusion");
        return ERROR;
        }
    
    if (parseNode->count() == 0)
        return SUCCESS;

    sal_uInt32 firstTokenId = parseNode->getChild(0)->getTokenID();
    sal_uInt32 secondTokenId = parseNode->getChild(1)->getTokenID();

    if (firstTokenId != SQL_TOKEN_EXCLUDE)
        {
        BeAssert(false && "Expecting window frame exclusion to start with EXCLUDE");
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0676, "Expecting window frame exclusion to start with EXCLUDE");
        return ERROR;  
        }

    if (parseNode->count() == 2)
        {
        if (secondTokenId == SQL_TOKEN_GROUP)
            {
            windowFrameExclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeGroup;
            return SUCCESS;
            }
        else if (secondTokenId == SQL_TOKEN_TIES)
            {
            windowFrameExclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeTies;
            return SUCCESS;
            }
        else
            {
            BeAssert(false && "Unsupported grammar in window frame exclusion type");
            return ERROR; 
            }
        }
    else if (parseNode->count() == 3)
        {
        sal_uInt32 thirdTokenId = parseNode->getChild(2)->getTokenID();
        if (secondTokenId == SQL_TOKEN_CURRENT && thirdTokenId == SQL_TOKEN_ROW)
            {
            windowFrameExclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeCurrentRow;
            return SUCCESS;
            }
        else if (secondTokenId == SQL_TOKEN_NO && thirdTokenId == SQL_TOKEN_OTHERS)
            {
            windowFrameExclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeNoOthers;
            return SUCCESS;
            }
        else
            {
            BeAssert(false && "Unsupported gramma in window frame exclusion type");
            return ERROR; 
            }
        }
    else
        {
        BeAssert(false && "Incorrect number of child nodes in window frame exclusion type");
        return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseValueCreationFuncExp(std::unique_ptr<ValueExp>& valueExp, OSQLParseNode const* parseNode) const
    {
    if (!SQL_ISRULE(parseNode, value_creation_fct))
        {
        BeAssert(false && "Invalid grammar. Expecting value_creation_fct");
        return ERROR;
        }

    switch (parseNode->getChild(0)->getTokenID())
        {
        case SQL_TOKEN_NAVIGATION_VALUE:
            {
            std::unique_ptr<NavValueCreationFuncExp> navValueCreationFuncExp = nullptr;
            if (SUCCESS != ParseNavValueCreationFuncExp(navValueCreationFuncExp, parseNode))
                return ERROR;
            valueExp = std::move(navValueCreationFuncExp);
            return SUCCESS;
            }
        default:
            {
            BeAssert(false && "Unhandled tokenID in ValueCreationFuncExp");
            return ERROR;
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParser::ParseNavValueCreationFuncExp(std::unique_ptr<NavValueCreationFuncExp>& valueCreationFuncExp, OSQLParseNode const* parseNode) const
    {
    std::unique_ptr<DerivedPropertyExp> derivedPropertyExp = nullptr;
    if (SUCCESS != ParseDerivedColumn(derivedPropertyExp, parseNode->getChild(2)))
        return ERROR;

    if (derivedPropertyExp->GetExpression()->GetType() != Exp::Type::PropertyName) // NAVIGATION_VALUE function should accept only PropertyName
        return ERROR;

    if (derivedPropertyExp->GetExpression()->GetAs<PropertyNameExp>().GetResolvedPropertyPath().Size() != 3)
        return ERROR;

    PropertyPath propPath = derivedPropertyExp->GetExpression()->GetAs<PropertyNameExp>().GetResolvedPropertyPath();

    ClassMap const* classMap = m_context->GetECDb().Schemas().GetDispatcher().GetClassMap(propPath[0].GetName(), propPath[1].GetName(), SchemaLookupMode::AutoDetect, nullptr);

    if (classMap == nullptr)
        return ERROR;

    propPath.SetClassMap(*classMap);
    std::shared_ptr<ClassNameExp::Info> classNameExpInfo = nullptr;
    if (SUCCESS != m_context->TryResolveClass(classNameExpInfo, nullptr, propPath[0].GetName(), propPath[1].GetName(), ECSqlType::Select, false, *parseNode))
        return ERROR;

    std::unique_ptr<ClassNameExp> classNameExp =  std::make_unique<ClassNameExp>(propPath[1].GetName().c_str(), propPath[0].GetName().c_str(), nullptr, classNameExpInfo);

    if (classMap->GetPropertyMaps().Find(propPath[2].GetName().c_str()) == nullptr)
        return ERROR;

    std::unique_ptr<PropertyNameExp> propNameExp = std::make_unique<PropertyNameExp>(
        *m_context,
        propPath[2].GetName(),
        *classNameExp.get(),
        *classMap
    );

    propNameExp->SetTypeInfo(ECSqlTypeInfo(ECSqlTypeInfo::Kind::Navigation));
    Utf8CP alias = parseNode->getParent()->getChild(1)->getTokenValue().empty() ? nullptr : parseNode->getParent()->getChild(1)->getTokenValue().c_str();
    derivedPropertyExp = std::make_unique<DerivedPropertyExp>(std::move(propNameExp), alias);

    std::unique_ptr<ValueExp> idArgExp = nullptr;
    if (SUCCESS != ParseFunctionArg(idArgExp, *parseNode->getChild(4)))
        return ERROR;

    std::unique_ptr<ValueExp> relECClassIdArgExp = nullptr;
    if (parseNode->getChild(5)->count() != 0 && SUCCESS != ParseFunctionArg(relECClassIdArgExp, *parseNode->getChild(5)->getChild(1)))
        return ERROR;

    valueCreationFuncExp = std::make_unique<NavValueCreationFuncExp>(std::move(derivedPropertyExp), std::move(idArgExp), std::move(relECClassIdArgExp), std::move(classNameExp));
    valueCreationFuncExp->SetTypeInfo(ECSqlTypeInfo(*classMap->GetPropertyMaps().Find(propPath[2].GetName().c_str())));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
Utf8CP ECSqlParser::SqlDataTypeKeywordToString(sal_uInt32 keywordId)
    {
    switch (keywordId)
        {
            case SQL_TOKEN_BINARY:
                return "BINARY";
            case SQL_TOKEN_BLOB:
                return "BLOB";
            case SQL_TOKEN_BOOLEAN:
                return "BOOLEAN";
            case SQL_TOKEN_DATE:
                return "DATE";
            case SQL_TOKEN_DOUBLE:
                return "DOUBLE";
            case SQL_TOKEN_FLOAT:
                return "FLOAT";
            case SQL_TOKEN_INTEGER:
                return "INTEGER";
            case SQL_TOKEN_INT:
                return "INT";
            case SQL_TOKEN_INT64:
                return "INT64";
            case SQL_TOKEN_LONG:
                return "LONG";
            case SQL_TOKEN_REAL:
                return "REAL";
            case SQL_TOKEN_STRING:
                return "STRING";
            case SQL_TOKEN_TIME:
                return "TIME";
            case SQL_TOKEN_TIMESTAMP:
                return "TIMESTAMP";
            case SQL_TOKEN_VARCHAR:
                return "VARCHAR";
            default:
                BeAssert(false && "TokenId unhandled by ECSqlParser::SqlKeywordToString");
                return "";
        }
    }

//-----------------------------------------------------------------------------------------
// ECSqlParseContext
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus ECSqlParseContext::FinalizeParsing(Exp& rootExp)
    {
    if (SUCCESS != rootExp.FinalizeParsing(*this))
        return ERROR;

    if (GetDeferFinalize()) {
        SetDeferFinalize(false);
        if (SUCCESS != rootExp.FinalizeParsing(*this))
            return ERROR;
    }

    for (ParameterExp* parameterExp : m_parameterExpList)
        {
        if (!parameterExp->TryDetermineParameterExpType(*this, *parameterExp))
            parameterExp->SetDefaultTargetExpInfo();
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlParseContext::PushArg(std::unique_ptr<ParseArg> arg) { m_finalizeParseArgs.push_back(std::move(arg));  }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlParseContext::ParseArg const* ECSqlParseContext::CurrentArg() const
    {
    if (m_finalizeParseArgs.empty())
        return nullptr;

    return m_finalizeParseArgs[m_finalizeParseArgs.size() - 1].get();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlParseContext::PopArg() { BeAssert(!m_finalizeParseArgs.empty()); m_finalizeParseArgs.pop_back(); }

//Helper for TryResolveClass to check if te current node is a table node inside a type predicate
//If the class is a custom attribute class,we allow it to be used despite what the policy says
//Example syntax in ECSQL: "IS (mySchema.MyCustomAttribute)"
bool IsSpecialTypePredicateCase(ClassMap const& classMap, OSQLParseNode const& node)
    {
    if (classMap.GetMapStrategy().GetStrategy() != MapStrategy::NotMapped || !classMap.GetClass().IsCustomAttributeClass())
        return false;

    if (!SQL_ISRULE(&node, table_node))
        return false; //node itself must be a table_node

    auto parent = node.getParent();
    if (parent == nullptr)
        return false; //parent node is a helper node (type list item)

    auto parentParent = parent->getParent();
    if (parentParent == nullptr)
        return false; // parent's parent node is a helper node (type list)

    auto parentParentParent = parentParent->getParent();
    if (parentParentParent == nullptr || !SQL_ISRULE(parentParentParent, type_predicate))
        return false; //parent's parent's parent node must be type_predicate

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParseContext::TryResolveClass(std::shared_ptr<ClassNameExp::Info>& classNameExpInfo, Utf8CP tableSpaceName, Utf8StringCR schemaNameOrAlias,
    Utf8StringCR className, ECSqlType ecsqlType, bool isPolymorphicExp, OSQLParseNode const& node)
    {
    BeAssert(!schemaNameOrAlias.empty());
    ClassMap const* classMap = m_ecdb.Schemas().GetDispatcher().GetClassMap(schemaNameOrAlias, className, SchemaLookupMode::AutoDetect, tableSpaceName);
    if (classMap == nullptr)
        {
        if (Utf8String::IsNullOrEmpty(tableSpaceName))
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0494,
                "ECClass '%s.%s' does not exist or could not be loaded.", schemaNameOrAlias.c_str(), className.c_str());
        else
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0495,
                "ECClass '%s.%s.%s' does not exist or could not be loaded.", tableSpaceName, schemaNameOrAlias.c_str(), className.c_str());

        return ERROR;
        }

    Utf8String fullyQualifiedClassName;
    //it is fine if tableSpaceName is nullptr (which means any table space) as the given class will always be found in the same table space even
    //if a class with the same name exists in more than one table space
    fullyQualifiedClassName.Sprintf("%s.%s", tableSpaceName, classMap->GetClass().GetFullName());
    auto it = m_classNameExpInfoList.find(fullyQualifiedClassName);
    if (it != m_classNameExpInfoList.end())
        {
        classNameExpInfo = it->second;
        return SUCCESS;
        }

    Policy policy = PolicyManager::GetPolicy(ClassIsValidInECSqlPolicyAssertion(*classMap, ecsqlType, isPolymorphicExp));
    if (!policy.IsSupported())
        {
        //despite the policy we allow using a custom attribute class if it is used inside a type predicate
        if (IsSpecialTypePredicateCase(*classMap, node))
            {
            classNameExpInfo = ClassNameExp::Info::Create(*classMap);
            return SUCCESS;
            }

        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0496, "Invalid ECClass in ECSQL: %s", policy.GetNotSupportedMessage().c_str());
        return ERROR;
        }

    classNameExpInfo = ClassNameExp::Info::Create(*classMap);
    m_classNameExpInfoList[fullyQualifiedClassName] = classNameExpInfo;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlParseContext::TrackECSqlParameter(ParameterExp& parameterExp)
    {
    m_parameterExpList.push_back(&parameterExp);

    const bool isNamedParameter = parameterExp.IsNamedParameter();

    if (isNamedParameter)
        {
        auto it = m_ecsqlParameterNameToIndexMapping.find(parameterExp.GetParameterName().c_str());
        if (it != m_ecsqlParameterNameToIndexMapping.end())
            return it->second;
        }

    m_currentECSqlParameterIndex++;
    if (isNamedParameter)
        m_ecsqlParameterNameToIndexMapping[parameterExp.GetParameterName().c_str()] = m_currentECSqlParameterIndex;

    return m_currentECSqlParameterIndex;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParseContext::GetSubclasses(ClassListById& classes, ECClassCR ecClass)
    {
    ECDerivedClassesList const* subclasses = Schemas().GetDerivedClassesInternal(const_cast<ECClassR>(ecClass));
    if (subclasses == nullptr)
        return ERROR;

    for (ECClassCP derivedClass : *subclasses)
        {
        if (classes.find(derivedClass->GetId()) == classes.end())
            {
            classes[derivedClass->GetId()] = derivedClass;
            if (SUCCESS != GetSubclasses(classes, *derivedClass))
                return ERROR;
            }
        }

    return SUCCESS;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlParseContext::GetConstraintClasses(ClassListById& classes, ECRelationshipConstraintCR constraintEnd)
    {
    for (ECClassCP ecClass : constraintEnd.GetConstraintClasses())
        {
        if (classes.find(ecClass->GetId()) == classes.end())
            {
            classes[ecClass->GetId()] = ecClass;
            if (constraintEnd.GetIsPolymorphic())
                {
                if (SUCCESS != GetSubclasses(classes, *ecClass))
                    return ERROR;
                }
            }
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlParseContext::GenerateAlias()
    {
    Utf8String alias;
    alias.Sprintf("K%d", m_aliasCount++);
    return alias;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
