/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlRDParser.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
USING_NAMESPACE_BENTLEY_EC

// Error reporting macro – wraps message with "Failed to parse ECSQL '<sql>': <detail>"
// to match the old Bison parser's error format. Silenced during speculative parsing.
#define ECSQLERR(fmt, ...) do { \
    if (!m_suppressErrors) \
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, \
            ECDbIssueId::ECDb_0471, "Failed to parse ECSQL '%s': " fmt, m_ecsql ? m_ecsql : "", ##__VA_ARGS__); \
} while(0)

//=============================================================================
// Token stream helpers
//=============================================================================

ECSqlToken ECSqlRDParser::Advance()
    {
    ECSqlToken old = m_current;
    m_current = m_lexer->Next();
    return old;
    }

bool ECSqlRDParser::Expect(ECSqlTokenType t)
    {
    if (!At(t))
        {
        ECSQLERR("Expected token %d, got '%s'", static_cast<int>(t), m_current.GetText().c_str());
        return false;
        }
    Advance();
    return true;
    }

bool ECSqlRDParser::TryConsume(ECSqlTokenType t)
    {
    if (!At(t))
        return false;
    Advance();
    return true;
    }

Utf8String ECSqlRDParser::TokenTextUpper() const
    {
    Utf8String s = m_current.GetText();
    s.ToUpper();
    return s;
    }

bool ECSqlRDParser::IsJoinKeyword() const
    {
    return At(ECSqlTokenType::KW_JOIN)
        || At(ECSqlTokenType::KW_INNER)
        || At(ECSqlTokenType::KW_LEFT)
        || At(ECSqlTokenType::KW_RIGHT)
        || At(ECSqlTokenType::KW_FULL)
        || At(ECSqlTokenType::KW_CROSS)
        || At(ECSqlTokenType::KW_NATURAL);
    }

//=============================================================================
// Parse() – top-level entry point
//=============================================================================

std::unique_ptr<Exp> ECSqlRDParser::Parse(ECDbCR ecdb, Utf8CP ecsql, IssueDataSource const& issues, ECSqlRDParser const* parentParser)
    {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return nullptr;

    m_ecsql = ecsql;
    ScopedContext ctx(*this, ecdb, issues, parentParser);
    ECSqlLexer lexer(ecsql);
    m_lexer  = &lexer;
    m_current = lexer.Next();  // prime the lookahead

    std::unique_ptr<Exp> exp;

    if (At(ECSqlTokenType::KW_WITH))
        {
        std::unique_ptr<CommonTableExp> cteExp;
        if (SUCCESS != ParseCTE(cteExp))
            { m_ecsql = nullptr; return nullptr; }
        exp = std::move(cteExp);
        }
    else if (At(ECSqlTokenType::KW_SELECT) || At(ECSqlTokenType::KW_VALUES))
        {
        std::unique_ptr<SelectStatementExp> selExp;
        if (SUCCESS != ParseSelectStatement(selExp))
            { m_ecsql = nullptr; return nullptr; }
        exp = std::move(selExp);
        }
    else if (At(ECSqlTokenType::KW_INSERT))
        {
        std::unique_ptr<InsertStatementExp> ins;
        if (SUCCESS != ParseInsertStatement(ins))
            { m_ecsql = nullptr; return nullptr; }
        exp = std::move(ins);
        }
    else if (At(ECSqlTokenType::KW_UPDATE))
        {
        std::unique_ptr<UpdateStatementExp> upd;
        if (SUCCESS != ParseUpdateStatementSearched(upd))
            { m_ecsql = nullptr; return nullptr; }
        exp = std::move(upd);
        }
    else if (At(ECSqlTokenType::KW_DELETE))
        {
        std::unique_ptr<DeleteStatementExp> del;
        if (SUCCESS != ParseDeleteStatementSearched(del))
            { m_ecsql = nullptr; return nullptr; }
        exp = std::move(del);
        }
    else if (At(ECSqlTokenType::KW_PRAGMA))
        {
        std::unique_ptr<PragmaStatementExp> pragma;
        if (SUCCESS != ParsePragmaStatement(pragma))
            { m_ecsql = nullptr; return nullptr; }
        exp = std::move(pragma);
        }
    else
        {
        // Empty input (comment-only) or truly unrecognised token.
        // Use "syntax error" to match the old Bison parser's generic error.
        if (AtEnd())
            ECSQLERR("syntax error");
        else
            ECSQLERR("Unexpected token '%s' at start of ECSQL statement", m_current.GetText().c_str());
        m_ecsql = nullptr;
        return nullptr;
        }

    TryConsume(ECSqlTokenType::Semicolon);  // optional trailing semicolon

    // Ensure all input was consumed
    if (!AtEnd())
        {
        ECSQLERR("Unexpected token '%s' after end of ECSQL statement", m_current.GetText().c_str());
        m_ecsql = nullptr;
        return nullptr;
        }

    m_lexer = nullptr;
    m_ecsql = nullptr;
    if (exp == nullptr)
        return nullptr;

    if (SUCCESS != m_context->FinalizeParsing(*exp))
        return nullptr;

    return exp;
    }

//=============================================================================
// INSERT statement
// Grammar: INSERT [INTO] schema.ClassName [(col1, col2, ...)] VALUES (v1, v2, ...)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseInsertStatement(std::unique_ptr<InsertStatementExp>& insertExp)
    {
    insertExp = nullptr;
    Advance();                            // consume INSERT
    if (!Expect(ECSqlTokenType::KW_INTO))  // INTO is required
        return ERROR;

    // Optional ONLY polymorphic constraint (INSERT INTO ONLY ts.A ...)
    // NOTE: ALL is NOT valid for INSERT (only for SELECT/UPDATE/DELETE)
    PolymorphicInfo constraint = PolymorphicInfo::Only();  // INSERT defaults to ONLY
    if (At(ECSqlTokenType::KW_ALL))
        {
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL,
            ECDbIssueId::ECDb_0551,
            "ALL is not supported with INSERT statement.");
        return ERROR;
        }
    else if (TryConsume(ECSqlTokenType::KW_ONLY))
        constraint = PolymorphicInfo(PolymorphicInfo::Type::Only, false);

    std::unique_ptr<ClassNameExp> classNameExp;
    if (SUCCESS != ParseTableNode(classNameExp, ECSqlType::Insert, constraint))
        return ERROR;

    std::unique_ptr<PropertyNameListExp> insertPropertyNameListExp;
    if (SUCCESS != ParseOptColumnRefCommalist(insertPropertyNameListExp))
        return ERROR;

    std::vector<std::unique_ptr<ValueExp>> valueExpList;
    if (SUCCESS != ParseValuesOrQuerySpec(valueExpList))
        return ERROR;

    insertExp = std::make_unique<InsertStatementExp>(classNameExp, insertPropertyNameListExp, valueExpList);
    return SUCCESS;
    }

//=============================================================================
// UPDATE statement
// Grammar: UPDATE [ALL|ONLY] schema.ClassName SET assignments [WHERE cond] [ECSQLOPTIONS ...]
//=============================================================================

BentleyStatus ECSqlRDParser::ParseUpdateStatementSearched(std::unique_ptr<UpdateStatementExp>& exp)
    {
    exp = nullptr;
    Advance();  // consume UPDATE

    // Optional ALL | ONLY polymorphic constraint
    PolymorphicInfo constraint = PolymorphicInfo::NotSpecified();
    if (TryConsume(ECSqlTokenType::KW_ALL))
        constraint = PolymorphicInfo(PolymorphicInfo::Type::All, false);
    else if (TryConsume(ECSqlTokenType::KW_ONLY))
        constraint = PolymorphicInfo(PolymorphicInfo::Type::Only, false);

    std::unique_ptr<ClassNameExp> classNameExp;
    if (SUCCESS != ParseTableNode(classNameExp, ECSqlType::Update, constraint))
        return ERROR;

    if (!Expect(ECSqlTokenType::KW_SET))
        return ERROR;

    std::unique_ptr<AssignmentListExp> assignmentListExp;
    if (SUCCESS != ParseAssignmentCommalist(assignmentListExp))
        return ERROR;

    std::unique_ptr<WhereExp> whereExp;
    if (SUCCESS != ParseWhereClause(whereExp))
        return ERROR;

    std::unique_ptr<OptionsExp> optionsExp;
    if (SUCCESS != ParseOptECSqlOptionsClause(optionsExp))
        return ERROR;

    exp = std::make_unique<UpdateStatementExp>(std::move(classNameExp), std::move(assignmentListExp),
                                               std::move(whereExp), std::move(optionsExp));
    return SUCCESS;
    }

//=============================================================================
// DELETE statement
// Grammar: DELETE FROM [ALL|ONLY] schema.ClassName [WHERE cond] [ECSQLOPTIONS ...]
//=============================================================================

BentleyStatus ECSqlRDParser::ParseDeleteStatementSearched(std::unique_ptr<DeleteStatementExp>& exp)
    {
    exp = nullptr;
    Advance();  // consume DELETE

    if (!Expect(ECSqlTokenType::KW_FROM))
        return ERROR;

    // Optional ALL | ONLY polymorphic constraint
    PolymorphicInfo constraint = PolymorphicInfo::NotSpecified();
    if (TryConsume(ECSqlTokenType::KW_ALL))
        constraint = PolymorphicInfo(PolymorphicInfo::Type::All, false);
    else if (TryConsume(ECSqlTokenType::KW_ONLY))
        constraint = PolymorphicInfo(PolymorphicInfo::Type::Only, false);

    std::unique_ptr<ClassNameExp> classNameExp;
    if (SUCCESS != ParseTableNode(classNameExp, ECSqlType::Delete, constraint))
        return ERROR;

    std::unique_ptr<WhereExp> whereExp;
    if (SUCCESS != ParseWhereClause(whereExp))
        return ERROR;

    std::unique_ptr<OptionsExp> optionsExp;
    if (SUCCESS != ParseOptECSqlOptionsClause(optionsExp))
        return ERROR;

    exp = std::make_unique<DeleteStatementExp>(std::move(classNameExp), std::move(whereExp),
                                               std::move(optionsExp));
    return SUCCESS;
    }

//=============================================================================
// PRAGMA statement
// Grammar: PRAGMA name [= value | (value)] [FOR path] [ECSQLOPTIONS ...]
//=============================================================================

BentleyStatus ECSqlRDParser::ParsePragmaStatement(std::unique_ptr<PragmaStatementExp>& pragmaExp)
    {
    pragmaExp = nullptr;
    Advance();  // consume PRAGMA

    if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
        {
        ECSQLERR("Expected pragma name after PRAGMA keyword");
        return ERROR;
        }

    Utf8String pragmaName = TokenText();
    Advance();

    PragmaVal pragmaVal;
    bool isReadValue = true;

    // Helper lambda to parse a single pragma value from the current token.
    auto parsePragmaValue = [&]() -> BentleyStatus
        {
        if (At(ECSqlTokenType::KW_TRUE))
            {
            pragmaVal = PragmaVal(true);
            Advance();
            return SUCCESS;
            }
        if (At(ECSqlTokenType::KW_FALSE))
            {
            pragmaVal = PragmaVal(false);
            Advance();
            return SUCCESS;
            }
        if (At(ECSqlTokenType::KW_NULL))
            {
            pragmaVal = PragmaVal::Null();
            Advance();
            return SUCCESS;
            }
        if (At(ECSqlTokenType::IntNum))
            {
            pragmaVal = PragmaVal(static_cast<int64_t>(std::stoll(TokenText())));
            Advance();
            return SUCCESS;
            }
        if (At(ECSqlTokenType::ApproxNum))
            {
            pragmaVal = PragmaVal(std::stod(TokenText()));
            Advance();
            return SUCCESS;
            }
        if (At(ECSqlTokenType::String))
            {
            Utf8String s = TokenText();
            // strip surrounding single quotes: 'value' -> value
            if (s.size() >= 2 && (unsigned char)s.front() == 39)
                s = s.substr(1, s.size() - 2);
            pragmaVal = PragmaVal(s.c_str(), false);
            Advance();
            return SUCCESS;
            }
        if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
            {
            pragmaVal = PragmaVal(TokenText().c_str(), true);
            Advance();
            return SUCCESS;
            }
        ECSQLERR("Expected pragma value");
        return ERROR;
        };

    if (TryConsume(ECSqlTokenType::Equal))
        {
        isReadValue = false;
        if (SUCCESS != parsePragmaValue())
            return ERROR;
        }
    else if (At(ECSqlTokenType::LParen))
        {
        Advance();  // consume (
        isReadValue = false;
        if (SUCCESS != parsePragmaValue())
            return ERROR;
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        }

    // Optional FOR <path>
    std::vector<Utf8String> forPath;
    if (At(ECSqlTokenType::KW_FOR))
        {
        Advance();  // consume FOR
        while (At(ECSqlTokenType::Name) || m_current.IsKeyword())
            {
            forPath.push_back(TokenText());
            Advance();
            if (!TryConsume(ECSqlTokenType::Dot))
                break;
            }
        }

    std::unique_ptr<OptionsExp> options;
    if (SUCCESS != ParseOptECSqlOptionsClause(options))
        return ERROR;

    pragmaExp = std::make_unique<PragmaStatementExp>(pragmaName, pragmaVal, isReadValue,
                                                     forPath, std::move(options));
    return SUCCESS;
    }

//=============================================================================
// CTE (Common Table Expression)
// Grammar: WITH [RECURSIVE] name [(cols)] AS (body) [, ...] select_stmt
//=============================================================================

BentleyStatus ECSqlRDParser::ParseCTE(std::unique_ptr<CommonTableExp>& exp)
    {
    exp = nullptr;
    Advance();  // consume WITH
    bool recursive = TryConsume(ECSqlTokenType::KW_RECURSIVE);

    std::vector<std::unique_ptr<CommonTableBlockExp>> blockList;
    do
        {
        std::unique_ptr<CommonTableBlockExp> block;
        if (SUCCESS != ParseCTEBlock(block, recursive))
            return ERROR;
        blockList.push_back(std::move(block));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    if (blockList.empty())
        {
        ECSQLERR("CTE must have at least one block");
        return ERROR;
        }

    if (!At(ECSqlTokenType::KW_SELECT) && !At(ECSqlTokenType::KW_VALUES))
        {
        ECSQLERR("CTE must be followed by a SELECT statement");
        return ERROR;
        }

    std::unique_ptr<SelectStatementExp> selectStmt;
    if (SUCCESS != ParseSelectStatement(selectStmt))
        return ERROR;

    exp = std::make_unique<CommonTableExp>(std::move(selectStmt), std::move(blockList), recursive);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseCTEBlock(std::unique_ptr<CommonTableBlockExp>& exp, bool isRecursive)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
        {
        ECSQLERR("Expected CTE block name");
        return ERROR;
        }

    Utf8String blockName = TokenText();
    Advance();

    // Optional column list: (col1, col2, ...)
    std::vector<Utf8String> columns;
    if (TryConsume(ECSqlTokenType::LParen))
        {
        do
            {
            if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
                {
                ECSQLERR("Expected column name in CTE column list");
                return ERROR;
                }
            columns.push_back(TokenText());
            Advance();
            }
        while (TryConsume(ECSqlTokenType::Comma));

        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        }

    if (!Expect(ECSqlTokenType::KW_AS))
        return ERROR;
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    if (!At(ECSqlTokenType::KW_SELECT) && !At(ECSqlTokenType::KW_VALUES))
        {
        ECSQLERR("Expected SELECT or VALUES in CTE body");
        return ERROR;
        }

    std::unique_ptr<SelectStatementExp> selectStmt;
    if (SUCCESS != ParseSelectStatement(selectStmt))
        return ERROR;

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    if (!columns.empty())
        exp = std::make_unique<CommonTableBlockExp>(blockName.c_str(), columns, std::move(selectStmt));
    else
        {
        if (isRecursive)
            {
            ECSQLERR("Recursive CTE must have a column list");
            return ERROR;
            }
        exp = std::make_unique<CommonTableBlockExp>(blockName.c_str(), std::move(selectStmt));
        }

    return SUCCESS;
    }

//=============================================================================
// SELECT statement (handles UNION / INTERSECT / EXCEPT chains and VALUES)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseSelectStatement(std::unique_ptr<SelectStatementExp>& exp)
    {
    exp = nullptr;

    // VALUES clause: VALUES (r1), (r2) [UNION [ALL] ...]
    if (At(ECSqlTokenType::KW_VALUES))
        {
        Advance(); // consume VALUES

        // Collect all comma-separated rows
        std::vector<std::vector<std::unique_ptr<ValueExp>>> rows;
        do
            {
            if (!Expect(ECSqlTokenType::LParen))
                return ERROR;
            std::vector<std::unique_ptr<ValueExp>> row;
            if (SUCCESS != ParseRowValueConstructorCommalist(row))
                return ERROR;
            if (!Expect(ECSqlTokenType::RParen))
                return ERROR;
            rows.push_back(std::move(row));
            }
        while (TryConsume(ECSqlTokenType::Comma));

        // Check for compound operator after all comma rows (VALUES (r) UNION [ALL] ...)
        SelectStatementExp::CompoundOperator op = SelectStatementExp::CompoundOperator::None;
        if (At(ECSqlTokenType::KW_UNION))
            op = SelectStatementExp::CompoundOperator::Union;
        else if (At(ECSqlTokenType::KW_INTERSECT))
            op = SelectStatementExp::CompoundOperator::Intersect;
        else if (At(ECSqlTokenType::KW_EXCEPT))
            op = SelectStatementExp::CompoundOperator::Except;

        // Build the compound chain right-to-left
        // For N rows + optional compound rhs: rows[0] UNION ALL (rows[1] UNION ALL (... UNION [isAll] rhs))
        std::unique_ptr<SelectStatementExp> chain;
        if (op != SelectStatementExp::CompoundOperator::None)
            {
            Advance(); // consume UNION | INTERSECT | EXCEPT
            bool isAll = TryConsume(ECSqlTokenType::KW_ALL);
            std::unique_ptr<SelectStatementExp> rhs;
            if (SUCCESS != ParseSelectStatement(rhs))
                return ERROR;
            // Connect the last row to the compound rhs
            chain = std::make_unique<SelectStatementExp>(
                std::make_unique<SingleSelectStatementExp>(rows.back()),
                op, isAll, std::move(rhs));
            }
        else
            {
            chain = std::make_unique<SelectStatementExp>(
                std::make_unique<SingleSelectStatementExp>(rows.back()));
            }

        // Connect remaining rows as UNION ALL (right-to-left)
        for (int i = (int)rows.size() - 2; i >= 0; --i)
            {
            chain = std::make_unique<SelectStatementExp>(
                std::make_unique<SingleSelectStatementExp>(rows[i]),
                SelectStatementExp::CompoundOperator::Union, /*isAll=*/true,
                std::move(chain));
            }

        exp = std::move(chain);
        return SUCCESS;
        }

    std::unique_ptr<SingleSelectStatementExp> single;
    if (SUCCESS != ParseSingleSelectStatement(single))
        return ERROR;

    // Check for compound operator
    SelectStatementExp::CompoundOperator op = SelectStatementExp::CompoundOperator::None;
    if (At(ECSqlTokenType::KW_UNION))
        op = SelectStatementExp::CompoundOperator::Union;
    else if (At(ECSqlTokenType::KW_INTERSECT))
        op = SelectStatementExp::CompoundOperator::Intersect;
    else if (At(ECSqlTokenType::KW_EXCEPT))
        op = SelectStatementExp::CompoundOperator::Except;

    if (op == SelectStatementExp::CompoundOperator::None)
        {
        exp = std::make_unique<SelectStatementExp>(std::move(single));
        return SUCCESS;
        }

    // The left-hand SELECT must not have ORDER BY or LIMIT
    if (!single->IsCoreSelect())
        {
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL,
            ECDbIssueId::ECDb_0492,
            "SELECT statement in UNION/INTERSECT/EXCEPT must not contain ORDER BY or LIMIT clause: %s",
            single->ToECSql().c_str());
        return ERROR;
        }

    Advance();  // consume UNION | INTERSECT | EXCEPT
    bool isAll = TryConsume(ECSqlTokenType::KW_ALL);

    std::unique_ptr<SelectStatementExp> rhs;
    if (SUCCESS != ParseSelectStatement(rhs))
        return ERROR;

    exp = std::make_unique<SelectStatementExp>(std::move(single), op, isAll, std::move(rhs));
    return SUCCESS;
    }

//=============================================================================
// Single SELECT statement
// Grammar: SELECT [ALL|DISTINCT] selection [FROM ...] [WHERE ...] [GROUP BY ...]
//          [HAVING ...] [WINDOW ...] [ORDER BY ...] [LIMIT ...] [ECSQLOPTIONS ...]
//=============================================================================

BentleyStatus ECSqlRDParser::ParseSingleSelectStatement(std::unique_ptr<SingleSelectStatementExp>& exp)
    {
    exp = nullptr;
    if (!Expect(ECSqlTokenType::KW_SELECT))
        return ERROR;

    // ALL or DISTINCT quantifier
    SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified;
    if (TryConsume(ECSqlTokenType::KW_ALL))
        setQuantifier = SqlSetQuantifier::All;
    else if (TryConsume(ECSqlTokenType::KW_DISTINCT))
        setQuantifier = SqlSetQuantifier::Distinct;

    std::unique_ptr<SelectClauseExp> selectClause;
    if (SUCCESS != ParseSelection(selectClause))
        return ERROR;

    std::unique_ptr<FromExp>                      fromExp;
    std::unique_ptr<WhereExp>                     whereExp;
    std::unique_ptr<GroupByExp>                   groupByExp;
    std::unique_ptr<HavingExp>                    havingExp;
    std::unique_ptr<WindowFunctionClauseExp>      windowExp;
    std::unique_ptr<OrderByExp>                   orderByExp;
    std::unique_ptr<LimitOffsetExp>               limitExp;
    std::unique_ptr<OptionsExp>                   optionsExp;

    if (At(ECSqlTokenType::KW_FROM))
        {
        if (SUCCESS != ParseFromClause(fromExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_WHERE))
        {
        if (SUCCESS != ParseWhereClause(whereExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_GROUP))
        {
        if (SUCCESS != ParseGroupByClause(groupByExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_HAVING))
        {
        if (SUCCESS != ParseHavingClause(havingExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_WINDOW))
        {
        if (SUCCESS != ParseWindowClause(windowExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_ORDER))
        {
        if (SUCCESS != ParseOrderByClause(orderByExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_LIMIT))
        {
        if (SUCCESS != ParseLimitOffsetClause(limitExp))
            return ERROR;
        }

    if (At(ECSqlTokenType::KW_ECSQLOPTIONS))
        {
        if (SUCCESS != ParseOptECSqlOptionsClause(optionsExp))
            return ERROR;
        }

    exp = std::make_unique<SingleSelectStatementExp>(setQuantifier, std::move(selectClause),
        std::move(fromExp), std::move(whereExp), std::move(orderByExp), std::move(windowExp),
        std::move(groupByExp), std::move(havingExp), std::move(limitExp), std::move(optionsExp));
    return SUCCESS;
    }

//=============================================================================
// SELECT clause (selection list)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseSelection(std::unique_ptr<SelectClauseExp>& exp)
    {
    exp = std::make_unique<SelectClauseExp>();

    // SELECT *  or  SELECT *, col1, col2, ...
    if (At(ECSqlTokenType::Star))
        {
        PropertyPath pp;
        pp.Push(Exp::ASTERISK_TOKEN);
        exp->AddProperty(std::make_unique<DerivedPropertyExp>(
            std::make_unique<PropertyNameExp>(pp), nullptr));
        Advance();
        // If there are more columns after the *, continue parsing them
        if (!TryConsume(ECSqlTokenType::Comma))
            return SUCCESS;
        // Fall through to the loop below to parse remaining columns
        }

    // SELECT col1 [AS alias], col2 ...  (also handles * appearing mid-list)
    do
        {
        // Wildcard (*) can appear at any position in the select list
        if (At(ECSqlTokenType::Star))
            {
            PropertyPath pp;
            pp.Push(Exp::ASTERISK_TOKEN);
            exp->AddProperty(std::make_unique<DerivedPropertyExp>(
                std::make_unique<PropertyNameExp>(pp), nullptr));
            Advance();
            continue;
            }
        std::unique_ptr<DerivedPropertyExp> col;
        if (SUCCESS != ParseDerivedColumn(col))
            return ERROR;
        if (col)
            exp->AddProperty(std::move(col));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseDerivedColumn(std::unique_ptr<DerivedPropertyExp>& exp)
    {
    exp = nullptr;

    std::unique_ptr<ValueExp> valExp;
    if (SUCCESS != ParseValueExp(valExp))
        return ERROR;

    // Optional alias: AS name  -or-  implicit alias (bare Name token)
    Utf8String alias;
    if (TryConsume(ECSqlTokenType::KW_AS))
        {
        if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
            {
            ECSQLERR("Expected column alias after AS");
            return ERROR;
            }
        alias = TokenText();
        Advance();
        }
    else if (At(ECSqlTokenType::Name))
        {
        // Implicit alias without AS – only when it is clearly an identifier
        alias = TokenText();
        Advance();
        }

    exp = std::make_unique<DerivedPropertyExp>(std::move(valExp),
        alias.empty() ? nullptr : alias.c_str());
    return SUCCESS;
    }

//=============================================================================
// WHERE clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseWhereClause(std::unique_ptr<WhereExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_WHERE))
        return SUCCESS;  // optional

    Advance();  // consume WHERE

    std::unique_ptr<BooleanExp> cond;
    if (SUCCESS != ParseSearchCondition(cond))
        return ERROR;

    exp = std::make_unique<WhereExp>(std::move(cond));
    return SUCCESS;
    }

//=============================================================================
// GROUP BY clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseGroupByClause(std::unique_ptr<GroupByExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_GROUP))
        return SUCCESS;  // optional

    Advance();  // consume GROUP
    if (!Expect(ECSqlTokenType::KW_BY))
        return ERROR;

    auto listExp = std::make_unique<ValueExpListExp>();
    do
        {
        std::unique_ptr<ValueExp> val;
        if (SUCCESS != ParseValueExp(val))
            return ERROR;
        listExp->AddValueExp(val);
        }
    while (TryConsume(ECSqlTokenType::Comma));

    exp = std::make_unique<GroupByExp>(std::move(listExp));
    return SUCCESS;
    }

//=============================================================================
// HAVING clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseHavingClause(std::unique_ptr<HavingExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_HAVING))
        return SUCCESS;  // optional

    Advance();  // consume HAVING

    std::unique_ptr<BooleanExp> cond;
    if (SUCCESS != ParseSearchCondition(cond))
        return ERROR;

    exp = std::make_unique<HavingExp>(std::move(cond));
    return SUCCESS;
    }

//=============================================================================
// ORDER BY clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseOrderByClause(std::unique_ptr<OrderByExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_ORDER))
        return SUCCESS;  // optional

    Advance();  // consume ORDER
    if (!Expect(ECSqlTokenType::KW_BY))
        return ERROR;

    std::vector<std::unique_ptr<OrderBySpecExp>> specs;
    do
        {
        std::unique_ptr<ValueExp> valExp;
        if (SUCCESS != ParseValueExp(valExp))
            return ERROR;

        // Allow comparison operators in ORDER BY (e.g. ORDER BY I < 123)
        std::unique_ptr<ComputedExp> sortExp;
        if (IsComparisonOp())
            {
            BooleanSqlOperator op = ParseComparisonOp();
            Advance(); // consume the comparison operator token
            std::unique_ptr<ValueExp> rhs;
            if (SUCCESS != ParseValueExp(rhs))
                return ERROR;
            sortExp = std::make_unique<BinaryBooleanExp>(std::move(valExp), op, std::move(rhs));
            }
        else
            {
            sortExp = std::move(valExp);
            }

        OrderBySpecExp::SortDirection dir = OrderBySpecExp::SortDirection::NotSpecified;
        if (TryConsume(ECSqlTokenType::KW_ASC))
            dir = OrderBySpecExp::SortDirection::Ascending;
        else if (TryConsume(ECSqlTokenType::KW_DESC))
            dir = OrderBySpecExp::SortDirection::Descending;

        OrderBySpecExp::NullsOrder nullsOrder = OrderBySpecExp::NullsOrder::NotSpecified;
        if (At(ECSqlTokenType::KW_NULLS))
            {
            Advance();  // consume NULLS
            if (TryConsume(ECSqlTokenType::KW_FIRST))
                nullsOrder = OrderBySpecExp::NullsOrder::First;
            else if (TryConsume(ECSqlTokenType::KW_LAST))
                nullsOrder = OrderBySpecExp::NullsOrder::Last;
            else
                {
                ECSQLERR("Expected FIRST or LAST after NULLS");
                return ERROR;
                }
            }

        specs.push_back(std::make_unique<OrderBySpecExp>(sortExp, dir, nullsOrder));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    exp = std::make_unique<OrderByExp>(specs);
    return SUCCESS;
    }

//=============================================================================
// LIMIT / OFFSET clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseLimitOffsetClause(std::unique_ptr<LimitOffsetExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_LIMIT))
        return SUCCESS;  // optional

    Advance();  // consume LIMIT

    std::unique_ptr<ValueExp> limitVal;
    if (SUCCESS != ParseValueExp(limitVal))
        return ERROR;

    if (At(ECSqlTokenType::KW_OFFSET))
        {
        Advance();  // consume OFFSET
        std::unique_ptr<ValueExp> offsetVal;
        if (SUCCESS != ParseValueExp(offsetVal))
            return ERROR;
        exp = std::make_unique<LimitOffsetExp>(std::move(limitVal), std::move(offsetVal));
        }
    else
        {
        exp = std::make_unique<LimitOffsetExp>(std::move(limitVal));
        }
    return SUCCESS;
    }

//=============================================================================
// ECSQLOPTIONS clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseOptECSqlOptionsClause(std::unique_ptr<OptionsExp>& exp)
    {
    exp = nullptr;
    // Both "ECSQLOPTIONS" and "OPTIONS" are lexed as KW_ECSQLOPTIONS
    if (!At(ECSqlTokenType::KW_ECSQLOPTIONS))
        return SUCCESS;  // optional

    Advance();  // consume ECSQLOPTIONS / OPTIONS

    auto optionsExp = std::make_unique<OptionsExp>();
    do
        {
        if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
            {
            ECSQLERR("Expected option name");
            return ERROR;
            }
        Utf8String optionName = TokenText();
        Advance();

        Utf8String    optionValue;
        ECSqlTypeInfo dataType;

        if (TryConsume(ECSqlTokenType::Equal))
            {
            // value is either a bare identifier / keyword or a typed literal
            if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
                {
                optionValue = TokenText();
                dataType    = ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_String);
                Advance();
                }
            else
                {
                if (SUCCESS != ParseLiteral(optionValue, dataType))
                    return ERROR;
                }
            }

        auto optionExp = std::make_unique<OptionExp>(optionName.c_str(), optionValue.c_str(), dataType);
        if (SUCCESS != optionsExp->AddOptionExp(std::move(optionExp), Issues()))
            return ERROR;

        // Consume optional comma separator between options
        TryConsume(ECSqlTokenType::Comma);
        }
    // Continue only for plain Name tokens; stop at SQL keywords (UNION, ORDER, LIMIT, etc.)
    while (At(ECSqlTokenType::Name));

    exp = std::move(optionsExp);
    return SUCCESS;
    }

//=============================================================================
// WINDOW clause (named window definitions)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseWindowClause(std::unique_ptr<WindowFunctionClauseExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_WINDOW))
        return SUCCESS;  // optional

    Advance();  // consume WINDOW

    std::vector<std::unique_ptr<WindowDefinitionExp>> defs;
    do
        {
        if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
            {
            ECSQLERR("Expected window name in WINDOW clause");
            return ERROR;
            }
        Utf8String windowName = TokenText();
        Advance();

        if (!Expect(ECSqlTokenType::KW_AS))
            return ERROR;
        if (!Expect(ECSqlTokenType::LParen))
            return ERROR;

        std::unique_ptr<WindowSpecification> spec;
        if (SUCCESS != ParseWindowSpecification(spec))
            return ERROR;

        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;

        defs.push_back(std::make_unique<WindowDefinitionExp>(windowName, std::move(spec)));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    auto defList = std::make_unique<WindowDefinitionListExp>(defs);
    exp = std::make_unique<WindowFunctionClauseExp>(std::move(defList));
    return SUCCESS;
    }

//=============================================================================
// VALUES helpers
//=============================================================================

// Parse: VALUES (v1, v2, ...) – single-row form used in INSERT.
BentleyStatus ECSqlRDParser::ParseValuesOrQuerySpec(std::vector<std::unique_ptr<ValueExp>>& valueExpList)
    {
    if (!At(ECSqlTokenType::KW_VALUES))
        {
        ECSQLERR("Expected VALUES keyword");
        return ERROR;
        }
    Advance();  // consume VALUES

    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;
    if (SUCCESS != ParseRowValueConstructorCommalist(valueExpList))
        return ERROR;
    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseRowValueConstructorCommalist(std::vector<std::unique_ptr<ValueExp>>& valueExpList)
    {
    do
        {
        std::unique_ptr<ValueExp> val;
        if (SUCCESS != ParseValueExp(val))
            return ERROR;
        valueExpList.push_back(std::move(val));
        }
    while (TryConsume(ECSqlTokenType::Comma));
    return SUCCESS;
    }

// Parse: VALUES (row1) [, (row2) ...] – builds a UNION ALL chain of single-row selects.
BentleyStatus ECSqlRDParser::ParseValuesCommalist(std::unique_ptr<SelectStatementExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_VALUES))
        {
        ECSQLERR("Expected VALUES");
        return ERROR;
        }
    Advance();  // consume VALUES

    std::vector<std::vector<std::unique_ptr<ValueExp>>> rows;
    do
        {
        if (!Expect(ECSqlTokenType::LParen))
            return ERROR;
        std::vector<std::unique_ptr<ValueExp>> row;
        if (SUCCESS != ParseRowValueConstructorCommalist(row))
            return ERROR;
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        rows.push_back(std::move(row));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    // Build UNION ALL chain in reverse so the output order matches input order:
    //   r1 UNION ALL (r2 UNION ALL r3)
    std::unique_ptr<SelectStatementExp> chain;
    for (int i = static_cast<int>(rows.size()) - 1; i >= 0; --i)
        {
        auto single = std::make_unique<SingleSelectStatementExp>(rows[i]);
        if (!chain)
            chain = std::make_unique<SelectStatementExp>(std::move(single));
        else
            chain = std::make_unique<SelectStatementExp>(std::move(single),
                SelectStatementExp::CompoundOperator::Union, /*isAll=*/true, std::move(chain));
        }

    exp = std::move(chain);
    return SUCCESS;
    }

//=============================================================================
// Polymorphic constraint (full form with optional disqualify prefix)
// Used in SELECT FROM table references.
// Grammar: [+ | !] (ALL | ONLY)
//=============================================================================

BentleyStatus ECSqlRDParser::ParsePolymorphicConstraint(PolymorphicInfo& constraint)
    {
    constraint = PolymorphicInfo::NotSpecified();

    // Optional disqualify marker: '+' (Plus token)
    bool disqualify = false;
    if (At(ECSqlTokenType::Plus))
        {
        disqualify = true;
        Advance();
        }

    if (At(ECSqlTokenType::KW_ALL))
        {
        constraint = PolymorphicInfo(PolymorphicInfo::Type::All, disqualify);
        Advance();
        }
    else if (At(ECSqlTokenType::KW_ONLY))
        {
        constraint = PolymorphicInfo(PolymorphicInfo::Type::Only, disqualify);
        Advance();
        }
    else
        {
        if (disqualify)
            {
            ECSQLERR("Expected ALL or ONLY after '+' disqualify marker");
            return ERROR;
            }
        // NotSpecified – leave constraint as-is
        }

    return SUCCESS;
    }

//=============================================================================
// ParseTableNode – resolve a class name reference [tableSpace.]schema.ClassName
//=============================================================================

BentleyStatus ECSqlRDParser::ParseTableNode(std::unique_ptr<ClassNameExp>& exp, ECSqlType ecsqlType,
                                             PolymorphicInfo polymorphic, bool isInsideTypePredicate)
    {
    exp = nullptr;

    if (!At(ECSqlTokenType::Name))
        {
        ECSQLERR("Expected class name");
        return ERROR;
        }

    Utf8String part1 = TokenText();
    Advance();

    Utf8String part2;
    if (At(ECSqlTokenType::NamedParam))
        {
        // schema:ClassName — colon was consumed by lexer as part of the named param token
        part2 = TokenText();
        Advance();
        }
    else
        {
        if (!Expect(ECSqlTokenType::Dot))
            return ERROR;

        if (!At(ECSqlTokenType::Name))
            {
            ECSQLERR("Expected class name component after '.'");
            return ERROR;
            }
        part2 = TokenText();
        Advance();
        }

    Utf8String tableSpaceName, schemaName, className;
    if (At(ECSqlTokenType::Dot))
        {
        // Three-part name: tableSpace.schema.ClassName
        Advance();  // consume second '.'
        if (!At(ECSqlTokenType::Name))
            {
            ECSQLERR("Expected class name component after second '.'");
            return ERROR;
            }
        Utf8String part3 = TokenText();
        Advance();
        tableSpaceName = part1;
        schemaName     = part2;
        className      = part3;
        }
    else
        {
        // Two-part name: schema.ClassName
        schemaName = part1;
        className  = part2;
        }

    std::shared_ptr<ClassNameExp::Info> info;
    if (SUCCESS != m_context->TryResolveClass(info,
                       tableSpaceName.empty() ? nullptr : tableSpaceName.c_str(),
                       schemaName, className, ecsqlType,
                       polymorphic.IsPolymorphic(), isInsideTypePredicate))
        return ERROR;

    exp = std::make_unique<ClassNameExp>(className, schemaName,
              tableSpaceName.empty() ? nullptr : tableSpaceName.c_str(),
              info, polymorphic);
    return SUCCESS;
    }

//=============================================================================
// ParseOptColumnRefCommalist – optional parenthesised column-reference list
// Used in: INSERT ... (col1, col2, ...)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseOptColumnRefCommalist(std::unique_ptr<PropertyNameListExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::LParen))
        return SUCCESS;  // optional

    return ParseColumnRefCommalist(exp);
    }

BentleyStatus ECSqlRDParser::ParseColumnRefCommalist(std::unique_ptr<PropertyNameListExp>& exp)
    {
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    auto listExp = std::make_unique<PropertyNameListExp>();
    do
        {
        std::unique_ptr<PropertyNameExp> propExp;
        if (SUCCESS != ParseColumnRefAsPropertyNameExp(propExp))
            return ERROR;
        listExp->AddPropertyNameExp(propExp);
        }
    while (TryConsume(ECSqlTokenType::Comma));

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::move(listExp);
    return SUCCESS;
    }

//=============================================================================
// ParseAssignmentCommalist – SET col1 = val1, col2 = val2, ...
//=============================================================================

BentleyStatus ECSqlRDParser::ParseAssignmentCommalist(std::unique_ptr<AssignmentListExp>& exp)
    {
    auto listExp = std::make_unique<AssignmentListExp>();
    do
        {
        std::unique_ptr<PropertyNameExp> lhs;
        if (SUCCESS != ParseColumnRefAsPropertyNameExp(lhs))
            return ERROR;

        if (!Expect(ECSqlTokenType::Equal))
            return ERROR;

        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExp(rhs))
            return ERROR;

        listExp->AddAssignmentExp(
            std::make_unique<AssignmentExp>(std::move(lhs), std::move(rhs)));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    exp = std::move(listExp);
    return SUCCESS;
    }

//=============================================================================
// ParseTableNodeWithOptMemberCall – [tablespace.]schema.class[.memberFunc(...)]
//=============================================================================

BentleyStatus ECSqlRDParser::ParseTableNodeWithOptMemberCall(std::unique_ptr<ClassNameExp>& exp,
    ECSqlType ecsqlType, PolymorphicInfo polymorphic, bool disqualifyPrimaryJoin, bool isInsideTypePredicate)
    {
    exp = nullptr;

    // Collect dot-separated path entries: each is an identifier optionally followed by (args)
    struct PathEntry { Utf8String name; bool hasFuncArgs = false; std::vector<std::unique_ptr<ValueExp>> args; };
    std::vector<PathEntry> path;

    auto readEntry = [&](bool allowFuncArgs) -> bool
        {
        if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
            return false;
        PathEntry entry;
        entry.name = TokenText();
        Advance();
        if (allowFuncArgs && At(ECSqlTokenType::LParen))
            {
            // member-function call
            entry.hasFuncArgs = true;
            Advance(); // consume '('
            if (!At(ECSqlTokenType::RParen))
                {
                do
                    {
                    std::unique_ptr<ValueExp> argExp;
                    if (SUCCESS != ParseValueExp(argExp))
                        return false;
                    entry.args.push_back(std::move(argExp));
                    }
                while (TryConsume(ECSqlTokenType::Comma));
                }
            if (!Expect(ECSqlTokenType::RParen))
                return false;
            }
        path.push_back(std::move(entry));
        return true;
        };

    if (!readEntry(false))
        {
        ECSQLERR("Expected class name");
        return ERROR;
        }

    // Handle schema:ClassName — colon was consumed by lexer as part of a named param token
    if (At(ECSqlTokenType::NamedParam))
        {
        PathEntry namedEntry;
        namedEntry.name = TokenText();
        Advance();
        path.push_back(std::move(namedEntry));
        }

    while (At(ECSqlTokenType::Dot))
        {
        // peek ahead: consume dot then read next entry (last entry may have func args)
        Advance(); // consume '.'
        bool isLastAllowed = true; // last entry may be a member function
        if (!readEntry(isLastAllowed))
            {
            ECSQLERR("Expected name after '.'");
            return ERROR;
            }
        }

    // path must have 2-4 entries
    const size_t pathLen = path.size();
    if (pathLen < 2 || pathLen > 4)
        {
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0486,
            "Invalid ECSQL class expression: Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]");
        return ERROR;
        }

    // The last entry may be a member function call; if so the class is the penultimate entry
    bool hasFunc = path.back().hasFuncArgs;
    size_t classIdx  = hasFunc ? pathLen - 2 : pathLen - 1;
    int    schemaIdx = (int)classIdx - 1;
    int    tsIdx     = schemaIdx - 1;

    if (schemaIdx < 0)
        {
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0488,
            "Invalid ECSQL class expression. Cannot specify a class name without schema name or alias.");
        return ERROR;
        }

    Utf8StringCR className  = path[classIdx].name;
    Utf8StringCR schemaName = path[schemaIdx].name;
    Utf8CP tableSpaceName   = (tsIdx >= 0) ? path[tsIdx].name.c_str() : nullptr;

    std::shared_ptr<ClassNameExp::Info> info;
    if (SUCCESS != m_context->TryResolveClass(info, tableSpaceName, schemaName, className,
                       ecsqlType, polymorphic.IsPolymorphic(), isInsideTypePredicate))
        return ERROR;

    std::unique_ptr<MemberFunctionCallExp> memberFuncCall;
    if (hasFunc)
        {
        PathEntry& fe = path.back();
        memberFuncCall = std::make_unique<MemberFunctionCallExp>(fe.name, /*tableValuedFunc=*/false);
        for (auto& argPtr : fe.args)
            {
            Utf8String emptyAlias;
            memberFuncCall->AddArgument(std::move(argPtr), emptyAlias);
            }
        }

    exp = std::make_unique<ClassNameExp>(className, schemaName, tableSpaceName, info, polymorphic,
                                         std::move(memberFuncCall), disqualifyPrimaryJoin);
    return SUCCESS;
    }

//=============================================================================
// ParseTableValuedFunction – schemaName.funcName(args) or funcName(args)
// Precondition: caller has detected that this is a TVF (identifier followed by '(')
//=============================================================================

BentleyStatus ECSqlRDParser::ParseTableValuedFunction(std::unique_ptr<TableValuedFunctionExp>& exp,
    Utf8StringCR schemaName, Utf8StringCR functionName)
    {
    // Consume '('
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    auto funcCall = std::make_unique<MemberFunctionCallExp>(functionName, /*tableValuedFunc=*/true);
    if (!At(ECSqlTokenType::RParen))
        {
        do
            {
            std::unique_ptr<ValueExp> argExp;
            if (SUCCESS != ParseValueExp(argExp))
                return ERROR;
            Utf8String err;
            if (SUCCESS != funcCall->AddArgument(std::move(argExp), err))
                {
                ECSQLERR("%s", err.c_str());
                return ERROR;
                }
            }
        while (TryConsume(ECSqlTokenType::Comma));
        }

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::make_unique<TableValuedFunctionExp>(schemaName.c_str(), std::move(funcCall), PolymorphicInfo::NotSpecified());
    return SUCCESS;
    }

//=============================================================================
// ParseTableRef – single table reference in FROM clause
//=============================================================================

BentleyStatus ECSqlRDParser::ParseTableRef(std::unique_ptr<ClassRefExp>& exp, ECSqlType ecsqlType)
    {
    // 1. Check for JOIN keywords (qualified join, cross join, etc.) — handled by caller
    //    This method handles a single non-joined table reference.

    // 2. Polymorphic constraint (opt_only): [+](ALL|ONLY)
    PolymorphicInfo polymorphic;
    if (SUCCESS != ParsePolymorphicConstraint(polymorphic))
        return ERROR;

    // 3. Disqualify primary join: '!'
    bool disqualifyPrimaryJoin = false;
    if (At(ECSqlTokenType::NotEqual))
        {
        // '!' alone is stored as the first char of '!=' — but the lexer returns != as NotEqual
        // The grammar uses '!' standalone to mean disqualifyPrimaryJoin.
        // The lexer in ECSqlLexer.cpp emits '!=' as NotEqual (2 chars) if followed by '=',
        // but should emit separate tokens for lone '!'. Let's handle both.
        // For now, skip this edge case — disqualify via '!' is very rare.
        }

    // 4. Subquery: '(' SELECT ...
    if (At(ECSqlTokenType::LParen))
        {
        std::unique_ptr<SubqueryExp> subqExp;
        if (SUCCESS != ParseSubquery(subqExp))
            return ERROR;

        Utf8String alias;
        if (TryConsume(ECSqlTokenType::KW_AS))
            {
            if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
                {
                ECSQLERR("Expected alias name after AS");
                return ERROR;
                }
            alias = TokenText();
            Advance();
            }
        else if (At(ECSqlTokenType::Name))
            {
            alias = TokenText();
            Advance();
            }

        exp = std::make_unique<SubqueryRefExp>(std::move(subqExp), alias.empty() ? nullptr : alias.c_str(), polymorphic);
        return SUCCESS;
        }

    // 5. Table name — could be CTE block name, TVF, or class name
    if (!At(ECSqlTokenType::Name))
        {
        ECSQLERR("Expected table name");
        return ERROR;
        }

    Utf8String firstName = TokenText();
    Advance();

    // 5a. Single identifier followed by '(' → TVF (no schema prefix)
    if (At(ECSqlTokenType::LParen) && ecsqlType == ECSqlType::Select && polymorphic.IsPolymorphic())
        {
        std::unique_ptr<TableValuedFunctionExp> tvfExp;
        if (SUCCESS != ParseTableValuedFunction(tvfExp, Utf8String(""), firstName))
            return ERROR;
        // Read optional alias
        Utf8String alias;
        if (TryConsume(ECSqlTokenType::KW_AS))
            { alias = TokenText(); if (At(ECSqlTokenType::Name) || m_current.IsKeyword()) Advance(); }
        else if (At(ECSqlTokenType::Name))
            { alias = TokenText(); Advance(); }
        if (!alias.empty()) tvfExp->SetAlias(alias);
        exp = std::move(tvfExp);
        return SUCCESS;
        }

    // 5b. No dot follows → single-identifier → could be CTE block name
    if (!At(ECSqlTokenType::Dot) && !At(ECSqlTokenType::NamedParam))
        {
        if (ecsqlType == ECSqlType::Select && polymorphic.IsPolymorphic())
            {
            // Check if this matches a known CTE block in scope
            // (ECSqlParseContext tracks active CTEs; use CommonTableBlockNameExp)
            auto cteBlockExp = std::make_unique<CommonTableBlockNameExp>(firstName.c_str());
            // Read optional alias
            Utf8String alias;
            if (TryConsume(ECSqlTokenType::KW_AS))
                { if (At(ECSqlTokenType::Name) || m_current.IsKeyword()) { alias = TokenText(); Advance(); } }
            else if (At(ECSqlTokenType::Name))
                { alias = TokenText(); Advance(); }
            if (!alias.empty()) cteBlockExp->SetAlias(alias);
            exp = std::move(cteBlockExp);
            return SUCCESS;
            }
        ECSQLERR("Expected '.' after schema name '%s'", firstName.c_str());
        return ERROR;
        }

    // 5c. Get secondName either from dot.Name or NamedParam (schema:ClassName)
    Utf8String secondName;
    if (At(ECSqlTokenType::NamedParam))
        {
        // schema:ClassName — colon was consumed by lexer as part of named param token
        secondName = TokenText();
        Advance();
        }
    else
        {
        Advance(); // consume '.'

        if (!At(ECSqlTokenType::Name))
            {
            ECSQLERR("Expected name after '.'");
            return ERROR;
            }
        secondName = TokenText();
        Advance();
        }

    // 5d. schema.funcName( → TVF with schema prefix
    if (At(ECSqlTokenType::LParen) && ecsqlType == ECSqlType::Select && polymorphic.IsPolymorphic())
        {
        std::unique_ptr<TableValuedFunctionExp> tvfExp;
        if (SUCCESS != ParseTableValuedFunction(tvfExp, firstName, secondName))
            return ERROR;
        Utf8String alias;
        if (TryConsume(ECSqlTokenType::KW_AS))
            { if (At(ECSqlTokenType::Name) || m_current.IsKeyword()) { alias = TokenText(); Advance(); } }
        else if (At(ECSqlTokenType::Name))
            { alias = TokenText(); Advance(); }
        if (!alias.empty()) tvfExp->SetAlias(alias);
        exp = std::move(tvfExp);
        return SUCCESS;
        }

    // Now we have firstName.secondName, maybe more
    // Collect remaining parts
    Utf8String tableSpace, schemaName, className;
    bool hasMoreDots = At(ECSqlTokenType::Dot);
    if (hasMoreDots)
        {
        Advance(); // consume '.'
        if (!At(ECSqlTokenType::Name))
            {
            ECSQLERR("Expected name after '.'");
            return ERROR;
            }
        Utf8String thirdName = TokenText();
        Advance();

        // Check for fourth part (member function)
        bool hasFourthDot = At(ECSqlTokenType::Dot);
        if (hasFourthDot)
            {
            Advance();
            if (!At(ECSqlTokenType::Name))
                {
                ECSQLERR("Expected name after '.'");
                return ERROR;
                }
            Utf8String fourthName = TokenText();
            Advance();
            // firstName.secondName.thirdName.fourthName(args)
            tableSpace = firstName; schemaName = secondName; className = thirdName;
            // fourthName is a member function if followed by '('
            if (At(ECSqlTokenType::LParen))
                {
                // member function call on class
                auto info2 = std::shared_ptr<ClassNameExp::Info>();
                if (SUCCESS != m_context->TryResolveClass(info2, tableSpace.c_str(), schemaName, className,
                                    ecsqlType, polymorphic.IsPolymorphic(), false))
                    return ERROR;
                auto funcCall = std::make_unique<MemberFunctionCallExp>(fourthName, false);
                Advance(); // consume '('
                if (!At(ECSqlTokenType::RParen))
                    {
                    do {
                        std::unique_ptr<ValueExp> a; if (SUCCESS != ParseValueExp(a)) return ERROR;
                        Utf8String emptyAlias1;
                        funcCall->AddArgument(std::move(a), emptyAlias1);
                    } while (TryConsume(ECSqlTokenType::Comma));
                    }
                if (!Expect(ECSqlTokenType::RParen)) return ERROR;
                auto classExp = std::make_unique<ClassNameExp>(className, schemaName, tableSpace.c_str(), info2, polymorphic, std::move(funcCall), disqualifyPrimaryJoin);
                Utf8String alias;
                if (TryConsume(ECSqlTokenType::KW_AS)) { if (At(ECSqlTokenType::Name) || m_current.IsKeyword()) { alias = TokenText(); Advance(); } }
                else if (At(ECSqlTokenType::Name)) { alias = TokenText(); Advance(); }
                if (!alias.empty()) classExp->SetAlias(alias);
                exp = std::move(classExp);
                return SUCCESS;
                }
            else
                {
                // fourthName is actually something else — treat as extra
                ECSQLERR("Unexpected fourth name component in class ref");
                return ERROR;
                }
            }

        // Three-part: firstName.secondName.thirdName
        // Could be tableSpace.schema.class OR schema.class.memberFunc
        if (At(ECSqlTokenType::LParen))
            {
            // schema.class.memberFunc(args) — secondName=schema, thirdName=class, wait no:
            // firstName.secondName.thirdName where thirdName is func → firstName=schema, secondName=class, thirdName=func
            schemaName = firstName; className = secondName;
            auto info2 = std::shared_ptr<ClassNameExp::Info>();
            if (SUCCESS != m_context->TryResolveClass(info2, nullptr, schemaName, className,
                                ecsqlType, polymorphic.IsPolymorphic(), false))
                return ERROR;
            auto funcCall = std::make_unique<MemberFunctionCallExp>(thirdName, false);
            Advance(); // consume '('
            if (!At(ECSqlTokenType::RParen))
                {
                do {
                    std::unique_ptr<ValueExp> a; if (SUCCESS != ParseValueExp(a)) return ERROR;
                    Utf8String emptyAlias2;
                    funcCall->AddArgument(std::move(a), emptyAlias2);
                } while (TryConsume(ECSqlTokenType::Comma));
                }
            if (!Expect(ECSqlTokenType::RParen)) return ERROR;
            auto classExp = std::make_unique<ClassNameExp>(className, schemaName, nullptr, info2, polymorphic, std::move(funcCall), disqualifyPrimaryJoin);
            Utf8String alias;
            if (TryConsume(ECSqlTokenType::KW_AS)) { if (At(ECSqlTokenType::Name) || m_current.IsKeyword()) { alias = TokenText(); Advance(); } }
            else if (At(ECSqlTokenType::Name)) { alias = TokenText(); Advance(); }
            if (!alias.empty()) classExp->SetAlias(alias);
            exp = std::move(classExp);
            return SUCCESS;
            }

        // tableSpace.schema.class
        tableSpace = firstName; schemaName = secondName; className = thirdName;
        }
    else
        {
        // Two-part: schema.class
        schemaName = firstName; className = secondName;
        }

    // Now resolve and optionally expand view class
    std::shared_ptr<ClassNameExp::Info> info;
    if (SUCCESS != m_context->TryResolveClass(info, tableSpace.empty() ? nullptr : tableSpace.c_str(),
                       schemaName, className, ecsqlType, polymorphic.IsPolymorphic(), false))
        return ERROR;

    auto classNameExp = std::make_unique<ClassNameExp>(className, schemaName,
                            tableSpace.empty() ? nullptr : tableSpace.c_str(),
                            info, polymorphic, nullptr, disqualifyPrimaryJoin);

    // View class expansion (SELECT only)
    if (ecsqlType == ECSqlType::Select)
        {
        auto classCP = m_context->GetECDb().Schemas().GetClass(classNameExp->GetSchemaName(),
                            classNameExp->GetClassName(), SchemaLookupMode::AutoDetect,
                            classNameExp->GetTableSpace().c_str());
        if (classCP != nullptr && ClassViews::IsViewClass(*classCP))
            {
            Utf8String viewQuery;
            if (!ClassViews::TryGetQuery(viewQuery, *classCP))
                {
                m_context->Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                    IssueType::ECSQL, ECDbIssueId::ECDb_0710,
                    "Invalid View Class '%s'. There is no query supplied for view.", classCP->GetFullName());
                return ERROR;
                }
            if (polymorphic.IsOnly())
                {
                m_context->Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                    IssueType::ECSQL, ECDbIssueId::ECDb_0711,
                    "ONLY keyword is not supported for view classes (ViewClass: %s).", classCP->GetFullName());
                return ERROR;
                }
            ECSqlParseContext::ClassViewPrepareStack viewStack(*m_context, *classCP);
            if (viewStack.IsOnStack(*classCP))
                {
                m_context->Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                    IssueType::ECSQL, ECDbIssueId::ECDb_0712,
                    "Invalid View Class '%s'. View query references itself recursively (%s).",
                    classCP->GetFullName(), viewStack.GetStackAsString().c_str());
                return ERROR;
                }
            ECSqlRDParser viewParser;
            auto parseTree = viewParser.Parse(m_context->GetECDb(), viewQuery.c_str(), m_context->Issues(), this);
            if (parseTree == nullptr)
                {
                m_context->Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                    IssueType::ECSQL, ECDbIssueId::ECDb_0713,
                    "Invalid View Class '%s'. View ECSQL failed to parse.", classCP->GetFullName());
                return ERROR;
                }
            Utf8CP viewAlias = classNameExp->GetAlias().empty()
                             ? classNameExp->GetClassName().c_str()
                             : classNameExp->GetAlias().c_str();
            if (parseTree->GetType() == Exp::Type::Select)
                {
                std::unique_ptr<SelectStatementExp> selExp(static_cast<SelectStatementExp*>(parseTree.release()));
                exp = std::make_unique<SubqueryRefExp>(
                    std::make_unique<SubqueryExp>(std::move(selExp)), viewAlias, polymorphic, std::move(classNameExp));
                }
            else if (parseTree->GetType() == Exp::Type::CommonTable)
                {
                std::unique_ptr<CommonTableExp> cteExp(static_cast<CommonTableExp*>(parseTree.release()));
                exp = std::make_unique<SubqueryRefExp>(
                    std::make_unique<SubqueryExp>(std::move(cteExp)), viewAlias, polymorphic, std::move(classNameExp));
                }
            else
                {
                m_context->Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties,
                    IssueType::ECSQL, ECDbIssueId::ECDb_0714,
                    "Invalid View Class '%s'. View ECSQL is neither a SELECT statement nor a CTE.",
                    classCP->GetFullName());
                return ERROR;
                }
            // Read optional alias after the view expansion
            Utf8String alias2;
            if (TryConsume(ECSqlTokenType::KW_AS)) { if (At(ECSqlTokenType::Name) || m_current.IsKeyword()) { alias2 = TokenText(); Advance(); } }
            else if (At(ECSqlTokenType::Name)) { alias2 = TokenText(); Advance(); }
            if (!alias2.empty() && exp)
                {
                if (auto* rangeExp = dynamic_cast<RangeClassRefExp*>(exp.get()))
                    rangeExp->SetAlias(alias2);
                }
            return SUCCESS;
            }
        }

    // Read optional alias
    Utf8String alias;
    if (TryConsume(ECSqlTokenType::KW_AS))
        {
        if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
            {
            alias = TokenText();
            Advance();
            }
        }
    else if (At(ECSqlTokenType::Name))
        {
        alias = TokenText();
        Advance();
        }
    if (!alias.empty())
        classNameExp->SetAlias(alias);
    exp = std::move(classNameExp);
    return SUCCESS;
    }

//=============================================================================
// ParseFromClause – FROM table_ref, ...
//=============================================================================

BentleyStatus ECSqlRDParser::ParseFromClause(std::unique_ptr<FromExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::KW_FROM));
    Advance(); // consume FROM

    auto fromExp = std::make_unique<FromExp>();
    do
        {
        std::unique_ptr<ClassRefExp> tableRef;
        if (SUCCESS != ParseTableRefAndOptJoins(tableRef))
            return ERROR;
        if (SUCCESS != fromExp->TryAddClassRef(*m_context, std::move(tableRef)))
            return ERROR;
        }
    while (TryConsume(ECSqlTokenType::Comma));

    exp = std::move(fromExp);
    return SUCCESS;
    }

//=============================================================================
// ParseJoinType – INNER | LEFT [OUTER] | RIGHT [OUTER] | FULL [OUTER]
//=============================================================================


// Internal helper: parse one table ref and any following JOINs
BentleyStatus ECSqlRDParser::ParseTableRefAndOptJoins(std::unique_ptr<ClassRefExp>& exp)
    {
    std::unique_ptr<ClassRefExp> lhs;
    if (SUCCESS != ParseTableRef(lhs, ECSqlType::Select))
        return ERROR;

    while (IsJoinKeyword())
        {
        std::unique_ptr<JoinExp> joinExp;
        if (SUCCESS != ParseJoinedTable(joinExp, std::move(lhs), ECSqlType::Select))
            return ERROR;
        lhs = std::move(joinExp);
        }

    exp = std::move(lhs);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseJoinType(ECSqlJoinType& joinType)
    {
    joinType = ECSqlJoinType::InnerJoin;
    if (TryConsume(ECSqlTokenType::KW_INNER))
        { joinType = ECSqlJoinType::InnerJoin; return SUCCESS; }
    if (TryConsume(ECSqlTokenType::KW_LEFT))
        { if (TryConsume(ECSqlTokenType::KW_OUTER)) {} joinType = ECSqlJoinType::LeftOuterJoin; return SUCCESS; }
    if (TryConsume(ECSqlTokenType::KW_RIGHT))
        { if (TryConsume(ECSqlTokenType::KW_OUTER)) {} joinType = ECSqlJoinType::RightOuterJoin; return SUCCESS; }
    if (TryConsume(ECSqlTokenType::KW_FULL))
        { if (TryConsume(ECSqlTokenType::KW_OUTER)) {} joinType = ECSqlJoinType::FullOuterJoin; return SUCCESS; }
    return SUCCESS; // default: inner
    }

//=============================================================================
// ParseJoinSpec – ON condition | USING (col, ...)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseJoinSpec(std::unique_ptr<JoinSpecExp>& exp)
    {
    if (At(ECSqlTokenType::KW_ON))
        {
        Advance(); // consume ON
        std::unique_ptr<BooleanExp> cond;
        if (SUCCESS != ParseSearchCondition(cond))
            return ERROR;
        exp = std::make_unique<JoinConditionExp>(std::move(cond));
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_USING))
        {
        Advance(); // consume USING
        if (!Expect(ECSqlTokenType::LParen))
            return ERROR;
        auto namedJoin = std::make_unique<NamedPropertiesJoinExp>();
        do
            {
            if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
                {
                ECSQLERR("Expected column name in USING clause");
                return ERROR;
                }
            namedJoin->Append(TokenText());
            Advance();
            }
        while (TryConsume(ECSqlTokenType::Comma));
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        exp = std::move(namedJoin);
        return SUCCESS;
        }
    ECSQLERR("Expected ON or USING in join specification");
    return ERROR;
    }

//=============================================================================
// ParseSubquery – ( select_stmt | cte | values )
//=============================================================================

BentleyStatus ECSqlRDParser::ParseSubquery(std::unique_ptr<SubqueryExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::LParen));
    Advance(); // consume '('

    if (At(ECSqlTokenType::KW_WITH))
        {
        std::unique_ptr<CommonTableExp> cteExp;
        if (SUCCESS != ParseCTE(cteExp))
            return ERROR;
        exp = std::make_unique<SubqueryExp>(std::move(cteExp));
        }
    else if (At(ECSqlTokenType::KW_SELECT))
        {
        std::unique_ptr<SelectStatementExp> selExp;
        if (SUCCESS != ParseSelectStatement(selExp))
            return ERROR;
        exp = std::make_unique<SubqueryExp>(std::move(selExp));
        }
    else if (At(ECSqlTokenType::KW_VALUES))
        {
        std::unique_ptr<SelectStatementExp> valExp;
        if (SUCCESS != ParseSelectStatement(valExp))
            return ERROR;
        exp = std::make_unique<SubqueryExp>(std::move(valExp));
        }
    else
        {
        ECSQLERR("Expected SELECT, WITH, or VALUES inside subquery");
        return ERROR;
        }

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;
    return SUCCESS;
    }

//=============================================================================
// Value expression parsers
//=============================================================================

// Top-level dispatcher: handles special forms that can start a value expression
BentleyStatus ECSqlRDParser::ParseValueExp(std::unique_ptr<ValueExp>& exp)
    {
    return ParseValueExpOr(exp);
    }

// |  (bitwise OR) — lowest precedence value binary op
BentleyStatus ECSqlRDParser::ParseValueExpOr(std::unique_ptr<ValueExp>& exp)
    {
    std::unique_ptr<ValueExp> lhs;
    if (SUCCESS != ParseValueExpAnd(lhs))
        return ERROR;
    while (At(ECSqlTokenType::BitwiseOr))
        {
        Advance();
        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExpAnd(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryValueExp>(std::move(lhs), BinarySqlOperator::BitwiseOr, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

// &  (bitwise AND)
BentleyStatus ECSqlRDParser::ParseValueExpAnd(std::unique_ptr<ValueExp>& exp)
    {
    std::unique_ptr<ValueExp> lhs;
    if (SUCCESS != ParseValueExpBitOr(lhs))
        return ERROR;
    while (At(ECSqlTokenType::BitwiseAnd))
        {
        Advance();
        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExpBitOr(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryValueExp>(std::move(lhs), BinarySqlOperator::BitwiseAnd, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

// << >>  (shifts)
BentleyStatus ECSqlRDParser::ParseValueExpBitOr(std::unique_ptr<ValueExp>& exp)
    {
    std::unique_ptr<ValueExp> lhs;
    if (SUCCESS != ParseValueExpBitAnd(lhs))
        return ERROR;
    while (At(ECSqlTokenType::ShiftLeft) || At(ECSqlTokenType::ShiftRight))
        {
        BinarySqlOperator op = At(ECSqlTokenType::ShiftLeft) ? BinarySqlOperator::ShiftLeft : BinarySqlOperator::ShiftRight;
        Advance();
        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExpBitAnd(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryValueExp>(std::move(lhs), op, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

// + -  (additive)
BentleyStatus ECSqlRDParser::ParseValueExpBitAnd(std::unique_ptr<ValueExp>& exp)
    {
    std::unique_ptr<ValueExp> lhs;
    if (SUCCESS != ParseValueExpShift(lhs))
        return ERROR;
    while (At(ECSqlTokenType::Plus) || At(ECSqlTokenType::Minus))
        {
        BinarySqlOperator op = At(ECSqlTokenType::Plus) ? BinarySqlOperator::Plus : BinarySqlOperator::Minus;
        Advance();
        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExpShift(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryValueExp>(std::move(lhs), op, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

// * / %  (multiplicative)
BentleyStatus ECSqlRDParser::ParseValueExpShift(std::unique_ptr<ValueExp>& exp)
    {
    std::unique_ptr<ValueExp> lhs;
    if (SUCCESS != ParseValueExpAddSub(lhs))
        return ERROR;
    while (At(ECSqlTokenType::Star) || At(ECSqlTokenType::Slash) || At(ECSqlTokenType::Percent))
        {
        BinarySqlOperator op;
        if (At(ECSqlTokenType::Star))       op = BinarySqlOperator::Multiply;
        else if (At(ECSqlTokenType::Slash)) op = BinarySqlOperator::Divide;
        else                                op = BinarySqlOperator::Modulo;
        Advance();
        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExpAddSub(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryValueExp>(std::move(lhs), op, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

// ||  (concatenation)
BentleyStatus ECSqlRDParser::ParseValueExpAddSub(std::unique_ptr<ValueExp>& exp)
    {
    std::unique_ptr<ValueExp> lhs;
    if (SUCCESS != ParseValueExpMulDiv(lhs))
        return ERROR;
    while (At(ECSqlTokenType::Concat))
        {
        Advance();
        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExpMulDiv(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryValueExp>(std::move(lhs), BinarySqlOperator::Concat, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

// delegate (no additional op at this level)
BentleyStatus ECSqlRDParser::ParseValueExpMulDiv(std::unique_ptr<ValueExp>& exp)
    {
    return ParseValueExpConcat(exp);
    }

// delegate (no additional op at this level)
BentleyStatus ECSqlRDParser::ParseValueExpConcat(std::unique_ptr<ValueExp>& exp)
    {
    return ParseValueExpUnary(exp);
    }

// unary: - + ~
BentleyStatus ECSqlRDParser::ParseValueExpUnary(std::unique_ptr<ValueExp>& exp)
    {
    if (At(ECSqlTokenType::Minus) || At(ECSqlTokenType::Plus) || At(ECSqlTokenType::BitwiseNot))
        {
        UnaryValueExp::Operator op;
        if      (At(ECSqlTokenType::Minus))      op = UnaryValueExp::Operator::Minus;
        else if (At(ECSqlTokenType::Plus))        op = UnaryValueExp::Operator::Plus;
        else                                      op = UnaryValueExp::Operator::BitwiseNot;
        Advance();
        std::unique_ptr<ValueExp> operand;
        if (SUCCESS != ParseValueExpPrimary(operand))
            return ERROR;
        exp = std::make_unique<UnaryValueExp>(operand, op);
        return SUCCESS;
        }
    return ParseValueExpPrimary(exp);
    }

//=============================================================================
// ParseValueExpPrimary – atoms: literals, params, function calls, column refs,
//                        subqueries, CAST, CASE, IIF, type_predicate
//=============================================================================

BentleyStatus ECSqlRDParser::ParseValueExpPrimary(std::unique_ptr<ValueExp>& exp)
    {
    // NULL / TRUE / FALSE
    if (At(ECSqlTokenType::KW_NULL) || At(ECSqlTokenType::KW_TRUE) || At(ECSqlTokenType::KW_FALSE))
        {
        Utf8String lit;
        ECSqlTypeInfo dtype;
        if (SUCCESS != ParseLiteral(lit, dtype))
            return ERROR;
        return LiteralValueExp::Create(exp, *m_context, lit.c_str(), dtype);
        }

    // Integer literal
    if (At(ECSqlTokenType::IntNum))
        {
        Utf8String val = TokenText();
        Advance();
        ECSqlTypeInfo dtype = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        return LiteralValueExp::Create(exp, *m_context, val.c_str(), dtype);
        }

    // Float literal
    if (At(ECSqlTokenType::ApproxNum))
        {
        Utf8String val = TokenText();
        Advance();
        ECSqlTypeInfo dtype = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double);
        return LiteralValueExp::Create(exp, *m_context, val.c_str(), dtype);
        }

    // String literal
    if (At(ECSqlTokenType::String))
        {
        Utf8String val = StripStringQuotes(TokenText());
        Advance();
        ECSqlTypeInfo dtype = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        return LiteralValueExp::Create(exp, *m_context, val.c_str(), dtype);
        }

    // Positional parameter: ?
    if (At(ECSqlTokenType::Parameter))
        {
        Advance();
        exp = std::make_unique<ParameterExp>(nullptr);
        return SUCCESS;
        }

    // Named parameter: :name
    if (At(ECSqlTokenType::NamedParam))
        {
        Utf8String pname = TokenText();
        Advance();
        exp = std::make_unique<ParameterExp>(pname.c_str());
        return SUCCESS;
        }

    // $ (instance access)
    if (At(ECSqlTokenType::Dollar))
        {
        return ParseColumnRef(exp, false);
        }

    // Parenthesised expression or subquery
    if (At(ECSqlTokenType::LParen))
        {
        // Peek: if SELECT/WITH/VALUES → subquery
        ECSqlToken next = m_lexer->Peek1();
        if (next.type == ECSqlTokenType::KW_SELECT
            || next.type == ECSqlTokenType::KW_WITH
            || next.type == ECSqlTokenType::KW_VALUES)
            {
            std::unique_ptr<SubqueryExp> subExp;
            if (SUCCESS != ParseSubquery(subExp))
                return ERROR;
            exp = std::make_unique<SubqueryValueExp>(std::move(subExp));
            return SUCCESS;
            }
        Advance(); // consume '('
        std::unique_ptr<ValueExp> inner;
        if (SUCCESS != ParseValueExp(inner))
            return ERROR;
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        inner->SetHasParentheses();
        exp = std::move(inner);
        return SUCCESS;
        }

    // CAST
    if (At(ECSqlTokenType::KW_CAST))
        return ParseCastSpec(exp);

    // CASE
    if (At(ECSqlTokenType::KW_CASE))
        return ParseCaseExp(exp);

    // IIF
    if (At(ECSqlTokenType::KW_IIF))
        return ParseIIFExp(exp);

    // NAVIGATION_VALUE
    if (At(ECSqlTokenType::KW_NAVIGATION_VALUE))
        return ParseValueCreationFuncExp(exp);

    // EXISTS is only valid in boolean context (WHERE/HAVING/ON), not as a value expression.
    if (At(ECSqlTokenType::KW_EXISTS))
        {
        ECSQLERR("EXISTS must be used in a boolean context (WHERE/HAVING/ON)");
        return ERROR;
        }

    // CURRENT_DATE / CURRENT_TIME / CURRENT_TIMESTAMP
    if (At(ECSqlTokenType::KW_CURRENT_DATE))
        {
        Advance();
        exp = std::make_unique<FunctionCallExp>(FunctionCallExp::CURRENT_DATE(), SqlSetQuantifier::NotSpecified, false, true);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_CURRENT_TIME))
        {
        Advance();
        exp = std::make_unique<FunctionCallExp>(FunctionCallExp::CURRENT_TIME(), SqlSetQuantifier::NotSpecified, false, true);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_CURRENT_TIMESTAMP))
        {
        Advance();
        exp = std::make_unique<FunctionCallExp>(FunctionCallExp::CURRENT_TIMESTAMP(), SqlSetQuantifier::NotSpecified, false, true);
        return SUCCESS;
        }

    // DATE 'literal' / TIME 'literal' / TIMESTAMP 'literal'
    if (At(ECSqlTokenType::KW_DATE) || At(ECSqlTokenType::KW_TIME) || At(ECSqlTokenType::KW_TIMESTAMP))
        {
        Advance(); // consume type keyword
        if (!At(ECSqlTokenType::String))
            {
            ECSQLERR("Expected date/time string literal");
            return ERROR;
            }
        Utf8String val = StripStringQuotes(TokenText());
        Advance();
        return LiteralValueExp::Create(exp, *m_context, val.c_str(), ECSqlTypeInfo::CreatePrimitive(ECN::PRIMITIVETYPE_DateTime));
        }

    // RTRIM special keyword-function
    if (At(ECSqlTokenType::KW_RTRIM))
        {
        Utf8String fname = "RTRIM";
        Advance(); // consume RTRIM
        return ParseFctSpecByName(exp, fname);
        }

    // Identifier (Name or keyword used as identifier) — could be function call or column ref
    if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
        {
        // Save position to back-track if needed
        return ParseColumnRef(exp, false);
        }

    // Star (used inside COUNT(*) — should be handled by ParseSetFct, not here)
    if (At(ECSqlTokenType::Star))
        {
        Utf8String val = Exp::ASTERISK_TOKEN;
        Advance();
        ECSqlTypeInfo dtype(ECSqlTypeInfo::Kind::Varies);
        return LiteralValueExp::Create(exp, *m_context, val.c_str(), dtype);
        }

    ECSQLERR("Unexpected token '%s' in value expression", m_current.GetText().c_str());
    return ERROR;
    }

//=============================================================================
// ParseColumnRef – column reference, property path, or function call
//=============================================================================



// Helper: parse a property path (a.b.c[0].d) into a PropertyPath object
BentleyStatus ECSqlRDParser::ParsePropertyPathInline(PropertyPath& path)
    {
    if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
        {
        ECSQLERR("Expected property path");
        return ERROR;
        }
    do
        {
        Utf8String name = TokenText();
        Advance();
        if (At(ECSqlTokenType::LBracket))
            {
            Advance();
            if (!At(ECSqlTokenType::IntNum))
                {
                ECSQLERR("Expected array index after '['");
                return ERROR;
                }
            int arrayIdx = std::stoi(TokenText().c_str());
            Advance();
            if (!Expect(ECSqlTokenType::RBracket))
                return ERROR;
            path.Push(name, arrayIdx);
            }
        else
            path.Push(name);
        }
    while (At(ECSqlTokenType::Dot) && (Advance(), true));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseColumnRef(std::unique_ptr<ValueExp>& exp, bool forcePropertyNameExp)
    {
    // Could be: name | name.name | name.name.name | $ | alias.$  | alias.$ -> path
    //         | funcName(args) | schema.funcName(args)

    // Handle $ (instance access starting with $)
    if (At(ECSqlTokenType::Dollar))
        {
        PropertyPath pp;
        pp.Push("$");
        Advance(); // consume $
        if (At(ECSqlTokenType::Arrow)) // ->
            {
            Advance();
            bool isOptional = false;
            if (At(ECSqlTokenType::Question)) { isOptional = true; Advance(); }
            PropertyPath rhsPath;
            if (SUCCESS != ParsePropertyPathInline(rhsPath))
                return ERROR;
            exp = std::make_unique<ExtractPropertyValueExp>(pp, rhsPath, isOptional);
            }
        else
            exp = std::make_unique<ExtractInstanceValueExp>(pp);
        return SUCCESS;
        }

    // Read first name — also remember if it was a keyword token
    if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
        {
        ECSQLERR("Expected identifier in column reference");
        return ERROR;
        }
    bool firstNameIsKeyword = (m_current.type != ECSqlTokenType::Name);
    Utf8String firstName = TokenText();
    Advance();

    // Bare reserved keywords are not valid as unescaped identifiers.
    // They are only allowed here as function names (must be followed by '(').
    if (firstNameIsKeyword && !At(ECSqlTokenType::LParen))
        {
        ECSQLERR("Reserved keyword '%s' cannot be used as an unescaped identifier; use [%s]",
                 firstName.c_str(), firstName.c_str());
        return ERROR;
        }

    // Check for function call: name(args)
    if (!forcePropertyNameExp && At(ECSqlTokenType::LParen))
        {
        // Uppercase known SQL functions (aggregate, window) for canonical column labels.
        // Check both keyword-flag AND known function names to ensure uppercase.
        // User-defined functions that aren't keywords keep original case.
        Utf8String funcName = firstName;
        Utf8String upperName = firstName;
        upperName.ToUpper();
        bool isKnownSqlFunction = firstNameIsKeyword || IsAggregateFunction(upperName) || IsWindowFunctionName(upperName);
        if (isKnownSqlFunction)
            funcName = upperName;
        // ntile: old parser hardcodes lowercase "ntile" for column label compatibility
        if (funcName.EqualsIAscii("NTILE"))
            funcName = "ntile";
        if (IsAggregateFunction(funcName) || IsAggregateFunction(upperName))
            {
            // Parse aggregate first, then check for OVER/FILTER (aggregate used as window function)
            std::unique_ptr<ValueExp> aggExp;
            if (SUCCESS != ParseAggregateFct(aggExp, funcName.empty() ? upperName : funcName))
                return ERROR;
            if (At(ECSqlTokenType::KW_OVER) || At(ECSqlTokenType::KW_FILTER))
                return ParseWindowFunction(exp, std::move(aggExp));
            exp = std::move(aggExp);
            return SUCCESS;
            }
        if (IsWindowFunctionName(funcName) || IsWindowFunctionName(upperName))
            {
            Utf8StringCR canonName = isKnownSqlFunction ? funcName : firstName;
            // Parse as function call, then check for OVER
            std::unique_ptr<ValueExp> funcExp;
            if (SUCCESS != ParseFctSpecByName(funcExp, canonName))
                return ERROR;
            if (At(ECSqlTokenType::KW_OVER) || At(ECSqlTokenType::KW_FILTER))
                return ParseWindowFunction(exp, std::move(funcExp));
            exp = std::move(funcExp);
            return SUCCESS;
            }
        std::unique_ptr<ValueExp> funcExp;
        if (SUCCESS != ParseFctSpecByName(funcExp, funcName))
            return ERROR;
        // Check for window function OVER clause
        if (At(ECSqlTokenType::KW_OVER) || At(ECSqlTokenType::KW_FILTER))
            return ParseWindowFunction(exp, std::move(funcExp));
        exp = std::move(funcExp);
        return SUCCESS;
        }

    // Check for dot-separated path or schema.function call
    if (At(ECSqlTokenType::Dot))
        {
        Advance(); // consume '.'
        if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword() && !At(ECSqlTokenType::Star))
            {
            ECSQLERR("Expected name after '.'");
            return ERROR;
            }
        Utf8String secondName = At(ECSqlTokenType::Star) ? Utf8String("*") : TokenText();
        if (!At(ECSqlTokenType::Star)) Advance();
        else Advance();

        // schema.funcName(args)?
        if (!forcePropertyNameExp && At(ECSqlTokenType::LParen))
            {
            // Two-part function call: schema.funcName(...)
            // Actually ECSQL doesn't support schema-qualified user functions directly here
            // but we still need to parse it
            std::unique_ptr<ValueExp> funcExp;
            Utf8String qualName = firstName + "." + secondName;
            if (SUCCESS != ParseFctSpecByName(funcExp, qualName))
                return ERROR;
            if (At(ECSqlTokenType::KW_OVER) || At(ECSqlTokenType::KW_FILTER))
                return ParseWindowFunction(exp, std::move(funcExp));
            exp = std::move(funcExp);
            return SUCCESS;
            }

        // alias.$ -> path or alias.$
        if (secondName.Equals("$") || (secondName.size() == 1 && secondName[0] == '$'))
            {
            // Instance access via alias
            PropertyPath srcPath;
            srcPath.Push(firstName);
            srcPath.Push("$");
            if (At(ECSqlTokenType::Arrow))
                {
                Advance();
                bool isOptional = false;
                if (At(ECSqlTokenType::Question)) { isOptional = true; Advance(); }
                PropertyPath rhsPath;
                if (SUCCESS != ParsePropertyPathInline(rhsPath))
                    return ERROR;
                exp = std::make_unique<ExtractPropertyValueExp>(srcPath, rhsPath, isOptional);
                }
            else
                exp = std::make_unique<ExtractInstanceValueExp>(srcPath);
            return SUCCESS;
            }

        // Build property path
        PropertyPath propPath;
        propPath.Push(firstName);

        // Handle array index on secondName
        {
        Utf8String name2 = secondName;
        int arrayIdx = -1;
        // Check for [idx]
        if (At(ECSqlTokenType::LBracket))
            {
            Advance();
            if (!At(ECSqlTokenType::IntNum))
                {
                ECSQLERR("Expected integer array index");
                return ERROR;
                }
            arrayIdx = std::stoi(TokenText());
            Advance();
            if (!Expect(ECSqlTokenType::RBracket))
                return ERROR;
            }
        propPath.Push(name2, arrayIdx);
        }

        // Continue reading more path segments
        while (At(ECSqlTokenType::Dot))
            {
            Advance();
            if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
                {
                ECSQLERR("Expected name after '.'");
                return ERROR;
                }
            Utf8String nextName = TokenText();
            Advance();
            int arrayIdx = -1;
            if (At(ECSqlTokenType::LBracket))
                {
                Advance();
                if (!At(ECSqlTokenType::IntNum)) { ECSQLERR("Expected integer array index"); return ERROR; }
                arrayIdx = std::stoi(TokenText());
                Advance();
                if (!Expect(ECSqlTokenType::RBracket)) return ERROR;
                }
            propPath.Push(nextName, arrayIdx);
            }

        // Enumeration check: 3-part path → try schema.enum.enumerator
        if (!forcePropertyNameExp && propPath.Size() == 3)
            {
            ECEnumerationCP ecEnum = m_context->GetECDb().Schemas().GetEnumeration(
                propPath[0].GetName(), propPath[1].GetName(), SchemaLookupMode::AutoDetect);
            if (ecEnum != nullptr)
                {
                ECEnumeratorCP enumerator = ecEnum->FindEnumeratorByName(propPath[2].GetName().c_str());
                if (enumerator != nullptr)
                    {
                    exp = std::make_unique<EnumValueExp>(*enumerator, propPath);
                    return SUCCESS;
                    }
                }
            }

        // Check -> extraction
        if (At(ECSqlTokenType::Arrow))
            {
            Advance();
            bool isOptional = false;
            if (At(ECSqlTokenType::Question)) { isOptional = true; Advance(); }
            PropertyPath rhsPath;
            if (SUCCESS != ParsePropertyPathInline(rhsPath))
                return ERROR;
            if (!InstanceValueExp::IsInstancePath(propPath))
                {
                Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0609,
                    "Invalid grammar. instance property exp must follow syntax '[<alias>.]$ -> <access-string>'");
                return ERROR;
                }
            exp = std::make_unique<ExtractPropertyValueExp>(propPath, rhsPath, isOptional);
            return SUCCESS;
            }

        exp = std::make_unique<PropertyNameExp>(std::move(propPath));
        return SUCCESS;
        }

    // Single identifier with optional array index
    PropertyPath propPath;
    int arrayIdx = -1;
    if (At(ECSqlTokenType::LBracket))
        {
        Advance();
        if (!At(ECSqlTokenType::IntNum)) { ECSQLERR("Expected array index"); return ERROR; }
        arrayIdx = std::stoi(TokenText());
        Advance();
        if (!Expect(ECSqlTokenType::RBracket)) return ERROR;
        }
    propPath.Push(firstName, arrayIdx);
    exp = std::make_unique<PropertyNameExp>(std::move(propPath));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseExpressionPath(std::unique_ptr<ValueExp>& exp, bool forceIntoPropertyNameExp)
    {
    return ParseColumnRef(exp, forceIntoPropertyNameExp);
    }

BentleyStatus ECSqlRDParser::ParseColumnRefAsPropertyNameExp(std::unique_ptr<PropertyNameExp>& exp)
    {
    std::unique_ptr<ValueExp> val;
    if (SUCCESS != ParseColumnRef(val, true))
        return ERROR;
    BeAssert(val && val->GetType() == Exp::Type::PropertyName);
    exp = std::unique_ptr<PropertyNameExp>(static_cast<PropertyNameExp*>(val.release()));
    return SUCCESS;
    }

//=============================================================================
// ParseLiteral – extract literal value and type from current token
//=============================================================================

BentleyStatus ECSqlRDParser::ParseLiteral(Utf8StringR literalVal, ECSqlTypeInfo& dataType)
    {
    if (At(ECSqlTokenType::IntNum))
        {
        literalVal = TokenText();
        dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Long);
        Advance();
        return SUCCESS;
        }
    if (At(ECSqlTokenType::ApproxNum))
        {
        literalVal = TokenText();
        dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double);
        Advance();
        return SUCCESS;
        }
    if (At(ECSqlTokenType::String))
        {
        literalVal = StripStringQuotes(TokenText());
        dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_String);
        Advance();
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_NULL))
        {
        literalVal = "NULL";
        dataType = ECSqlTypeInfo(ECSqlTypeInfo::Kind::Null);
        Advance();
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_TRUE))
        {
        literalVal = "TRUE";
        dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        Advance();
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_FALSE))
        {
        literalVal = "FALSE";
        dataType = ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Boolean);
        Advance();
        return SUCCESS;
        }
    ECSQLERR("Expected literal value, got '%s'", m_current.GetText().c_str());
    return ERROR;
    }

//=============================================================================
// ParseParameter – ? or :name
//=============================================================================

BentleyStatus ECSqlRDParser::ParseParameter(std::unique_ptr<ValueExp>& exp)
    {
    if (At(ECSqlTokenType::Parameter))
        {
        Advance();
        exp = std::make_unique<ParameterExp>(nullptr);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::NamedParam))
        {
        Utf8String name = TokenText();
        Advance();
        exp = std::make_unique<ParameterExp>(name.c_str());
        return SUCCESS;
        }
    ECSQLERR("Expected parameter (? or :name)");
    return ERROR;
    }

//=============================================================================
// Function call parsers
//=============================================================================

// Parse a function call given its name (already consumed or passed in).
// Parses '(' [args] ')'
BentleyStatus ECSqlRDParser::ParseFctSpecByName(std::unique_ptr<ValueExp>& exp, Utf8StringCR functionName)
    {
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    auto funcExp = std::make_unique<FunctionCallExp>(functionName);

    if (!At(ECSqlTokenType::RParen))
        {
        do
            {
            std::unique_ptr<ValueExp> arg;
            if (SUCCESS != ParseValueExp(arg))
                return ERROR;
            funcExp->AddArgument(std::move(arg));
            }
        while (TryConsume(ECSqlTokenType::Comma));
        }

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::move(funcExp);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseFctSpec(std::unique_ptr<ValueExp>& exp)
    {
    // The current token is a function name.
    // Uppercase only keyword-recognized function names (SQL standard / ECSQL builtins)
    // so they produce canonical column labels (e.g. COUNT(*), SUM([x])).
    // User-defined functions (Name token) keep their original case.
    Utf8String funcName = TokenText();
    if (m_current.type != ECSqlTokenType::Name)
        funcName.ToUpper();
    Advance();
    if (IsAggregateFunction(funcName))
        return ParseAggregateFct(exp, funcName);
    return ParseFctSpecByName(exp, funcName);
    }



BentleyStatus ECSqlRDParser::ParseAggregateFct(std::unique_ptr<ValueExp>& exp, Utf8StringCR functionName)
    {
    return ParseSetFct(exp, functionName, true);
    }

BentleyStatus ECSqlRDParser::ParseSetFct(std::unique_ptr<ValueExp>& exp, Utf8StringCR functionName, bool isStandardSetFunction)
    {
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    SqlSetQuantifier setQuantifier = SqlSetQuantifier::NotSpecified;
    if (TryConsume(ECSqlTokenType::KW_ALL))
        setQuantifier = SqlSetQuantifier::All;
    else if (TryConsume(ECSqlTokenType::KW_DISTINCT))
        setQuantifier = SqlSetQuantifier::Distinct;

    auto funcCallExp = std::make_unique<FunctionCallExp>(functionName, setQuantifier, isStandardSetFunction);

    // MAX()/MIN() with 0 args is a syntax error (matching old Bison parser behavior)
    bool isMaxOrMin = functionName.EqualsIAscii("MAX") || functionName.EqualsIAscii("MIN");
    if (isMaxOrMin && At(ECSqlTokenType::RParen))
        {
        ECSQLERR("syntax error");
        return ERROR;
        }

    // COUNT(*) special case
    if (functionName.EqualsIAscii("count") && At(ECSqlTokenType::Star))
        {
        std::unique_ptr<ValueExp> starExp;
        if (SUCCESS != LiteralValueExp::Create(starExp, *m_context, Exp::ASTERISK_TOKEN, ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies)))
            return ERROR;
        funcCallExp->AddArgument(std::move(starExp));
        Advance(); // consume *
        }
    else
        {
        std::unique_ptr<ValueExp> arg;
        if (SUCCESS != ParseValueExp(arg))
            return ERROR;
        funcCallExp->AddArgument(std::move(arg));
        }

    // GROUP_CONCAT optional second argument
    if (functionName.EqualsIAscii("group_concat") && TryConsume(ECSqlTokenType::Comma))
        {
        std::unique_ptr<ValueExp> sepArg;
        if (SUCCESS != ParseValueExp(sepArg))
            return ERROR;
        funcCallExp->AddArgument(std::move(sepArg));
        }

    // MAX/MIN with multiple arguments: emit helpful "Use GREATEST/LEAST" error to
    // match the old Bison parser's diagnostic instead of a cryptic "Expected ')'"
    if (isMaxOrMin && At(ECSqlTokenType::Comma))
        {
        if (functionName.EqualsIAscii("MAX"))
            ECSQLERR("Use GREATEST(arg0, arg1 [, ...]) instead of MAX(arg0, arg1 [, ...])");
        else
            ECSQLERR("Use LEAST(arg0, arg1 [, ...]) instead of MIN(arg0, arg1 [, ...])");
        return ERROR;
        }

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::move(funcCallExp);
    return SUCCESS;
    }

//=============================================================================
// ParseCastSpec – CAST ( expr AS type )
//=============================================================================

BentleyStatus ECSqlRDParser::ParseCastSpec(std::unique_ptr<ValueExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::KW_CAST));
    Advance(); // consume CAST
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    std::unique_ptr<ValueExp> operand;
    if (SUCCESS != ParseValueExp(operand))
        return ERROR;

    if (!Expect(ECSqlTokenType::KW_AS))
        return ERROR;

    // Parse type name: can be a keyword type or schema.typename
    // Also may be followed by [] for array types
    Utf8String typeName;
    bool isArray = false;

    // Check for known type keywords
    auto parseTypeKeyword = [&]() -> Utf8CP {
        switch (m_current.type) {
            case ECSqlTokenType::KW_BINARY:    return "BINARY";
            case ECSqlTokenType::KW_BLOB:      return "BLOB";
            case ECSqlTokenType::KW_BOOLEAN:   return "BOOLEAN";
            case ECSqlTokenType::KW_DATE:      return "DATE";
            case ECSqlTokenType::KW_DOUBLE:    return "DOUBLE";
            case ECSqlTokenType::KW_FLOAT:     return "FLOAT";
            case ECSqlTokenType::KW_INT:       return "INT";
            case ECSqlTokenType::KW_INT64:     return "INT64";
            case ECSqlTokenType::KW_LONG:      return "LONG";
            case ECSqlTokenType::KW_REAL:      return "REAL";
            case ECSqlTokenType::KW_STRING_KW: return "STRING";
            case ECSqlTokenType::KW_TIME:      return "TIME";
            case ECSqlTokenType::KW_TIMESTAMP: return "TIMESTAMP";
            case ECSqlTokenType::KW_VARCHAR:   return "VARCHAR";
            default: return nullptr;
        }
    };

    Utf8CP kwType = parseTypeKeyword();
    if (kwType != nullptr)
        {
        typeName = kwType;
        Advance();
        // Check for [] array marker
        if (At(ECSqlTokenType::LBracket))
            {
            Advance();
            if (!Expect(ECSqlTokenType::RBracket))
                return ERROR;
            isArray = true;
            }
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        exp = std::make_unique<CastExp>(std::move(operand), typeName.c_str(), isArray);
        return SUCCESS;
        }

    // Struct/enum type: schema.TypeName
    if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
        {
        Utf8String schema = TokenText();
        Advance();
        if (At(ECSqlTokenType::Dot))
            {
            Advance();
            if (!At(ECSqlTokenType::Name) && !m_current.IsKeyword())
                {
                ECSQLERR("Expected type name after '.' in CAST");
                return ERROR;
                }
            Utf8String tname = TokenText();
            Advance();
            // Check for [] array marker
            if (At(ECSqlTokenType::LBracket))
                {
                Advance();
                if (!Expect(ECSqlTokenType::RBracket))
                    return ERROR;
                isArray = true;
                }
            if (!Expect(ECSqlTokenType::RParen))
                return ERROR;
            exp = std::make_unique<CastExp>(std::move(operand), schema, tname, isArray);
            return SUCCESS;
            }
        // Just a plain name
        if (At(ECSqlTokenType::LBracket))
            {
            Advance();
            if (!Expect(ECSqlTokenType::RBracket))
                return ERROR;
            isArray = true;
            }
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        exp = std::make_unique<CastExp>(std::move(operand), schema.c_str(), isArray);
        return SUCCESS;
        }

    ECSQLERR("Expected type name in CAST expression");
    return ERROR;
    }

//=============================================================================
// ParseCaseExp – CASE WHEN cond THEN val ... [ELSE val] END
//=============================================================================

BentleyStatus ECSqlRDParser::ParseCaseExp(std::unique_ptr<ValueExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::KW_CASE));
    Advance(); // consume CASE

    std::vector<std::unique_ptr<SearchedWhenClauseExp>> whenList;
    while (At(ECSqlTokenType::KW_WHEN))
        {
        Advance(); // consume WHEN
        std::unique_ptr<BooleanExp> cond;
        if (SUCCESS != ParseSearchCondition(cond))
            return ERROR;
        if (!Expect(ECSqlTokenType::KW_THEN))
            return ERROR;
        std::unique_ptr<ValueExp> thenVal;
        if (SUCCESS != ParseValueExp(thenVal))
            return ERROR;
        whenList.push_back(std::make_unique<SearchedWhenClauseExp>(cond, thenVal));
        }

    std::unique_ptr<ValueExp> elseVal;
    if (At(ECSqlTokenType::KW_ELSE))
        {
        Advance();
        if (SUCCESS != ParseValueExp(elseVal))
            return ERROR;
        }

    if (!Expect(ECSqlTokenType::KW_END))
        return ERROR;

    exp = std::make_unique<SearchCaseValueExp>(whenList, elseVal);
    return SUCCESS;
    }

//=============================================================================
// ParseIIFExp – IIF ( condition, trueVal, falseVal )
//=============================================================================

BentleyStatus ECSqlRDParser::ParseIIFExp(std::unique_ptr<ValueExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::KW_IIF));
    Advance(); // consume IIF
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    std::unique_ptr<BooleanExp> cond;
    if (SUCCESS != ParseSearchCondition(cond))
        return ERROR;
    if (!Expect(ECSqlTokenType::Comma))
        return ERROR;
    std::unique_ptr<ValueExp> thenVal;
    if (SUCCESS != ParseValueExp(thenVal))
        return ERROR;
    if (!Expect(ECSqlTokenType::Comma))
        return ERROR;
    std::unique_ptr<ValueExp> elseVal;
    if (SUCCESS != ParseValueExp(elseVal))
        return ERROR;
    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::make_unique<IIFExp>(cond, thenVal, elseVal);
    return SUCCESS;
    }

//=============================================================================
// ParseTypePredicate – IS ( [opt_only] class, ... )
//=============================================================================

BentleyStatus ECSqlRDParser::ParseTypePredicate(std::unique_ptr<ValueExp>& exp)
    {
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    std::vector<std::unique_ptr<ClassNameExp>> typeList;
    do
        {
        PolymorphicInfo poly;
        if (SUCCESS != ParsePolymorphicConstraint(poly))
            return ERROR;
        std::unique_ptr<ClassNameExp> classExp;
        if (SUCCESS != ParseTableNode(classExp, ECSqlType::Select, poly, /*isInsideTypePredicate=*/true))
            return ERROR;
        typeList.push_back(std::move(classExp));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::make_unique<TypeListExp>(typeList);
    return SUCCESS;
    }

//=============================================================================
// ParseValueExpCommalist – value_exp, value_exp, ...
//=============================================================================

BentleyStatus ECSqlRDParser::ParseValueExpCommalist(std::unique_ptr<ValueExpListExp>& exp)
    {
    auto listExp = std::make_unique<ValueExpListExp>();
    do
        {
        std::unique_ptr<ValueExp> val;
        if (SUCCESS != ParseValueExp(val))
            return ERROR;
        listExp->AddValueExp(val);
        }
    while (TryConsume(ECSqlTokenType::Comma));
    exp = std::move(listExp);
    return SUCCESS;
    }

//=============================================================================
// Search condition parsers (boolean expressions)
//=============================================================================

BentleyStatus ECSqlRDParser::ParseSearchCondition(std::unique_ptr<BooleanExp>& exp)
    {
    return ParseSearchConditionOr(exp);
    }

BentleyStatus ECSqlRDParser::ParseSearchConditionOr(std::unique_ptr<BooleanExp>& exp)
    {
    std::unique_ptr<BooleanExp> lhs;
    if (SUCCESS != ParseSearchConditionAnd(lhs))
        return ERROR;
    while (At(ECSqlTokenType::KW_OR))
        {
        Advance();
        std::unique_ptr<BooleanExp> rhs;
        if (SUCCESS != ParseSearchConditionAnd(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryBooleanExp>(std::move(lhs), BooleanSqlOperator::Or, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseSearchConditionAnd(std::unique_ptr<BooleanExp>& exp)
    {
    std::unique_ptr<BooleanExp> lhs;
    if (SUCCESS != ParseSearchConditionNot(lhs))
        return ERROR;
    while (At(ECSqlTokenType::KW_AND))
        {
        Advance();
        std::unique_ptr<BooleanExp> rhs;
        if (SUCCESS != ParseSearchConditionNot(rhs))
            return ERROR;
        lhs = std::make_unique<BinaryBooleanExp>(std::move(lhs), BooleanSqlOperator::And, std::move(rhs));
        }
    exp = std::move(lhs);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseSearchConditionNot(std::unique_ptr<BooleanExp>& exp)
    {
    if (At(ECSqlTokenType::KW_NOT))
        {
        Advance();
        std::unique_ptr<BooleanExp> inner;
        if (SUCCESS != ParsePredicate(inner))
            return ERROR;
        exp = std::make_unique<BooleanFactorExp>(std::move(inner), true);
        return SUCCESS;
        }
    return ParsePredicate(exp);
    }

BentleyStatus ECSqlRDParser::ParsePredicate(std::unique_ptr<BooleanExp>& exp)
    {
    // Parenthesised expression — could be (value_expr) in comparison or (boolean_expr) predicate.
    // Strategy:
    //   1. Optimistically try parsing as a value expression (handles (I&1)=1, (4|1)&1=1, etc.)
    //   2. If value-expression path succeeds, continue with comparison handling.
    //   3. If it fails (e.g. content is a boolean like "L < 3.14 AND I > 3"), restore and
    //      fall back to the boolean (search_condition) path.
    if (At(ECSqlTokenType::LParen))
        {
        ECSqlToken next = m_lexer->Peek1();
        if (next.type != ECSqlTokenType::KW_SELECT && next.type != ECSqlTokenType::KW_WITH)
            {
            // Save parser+lexer state before trying value path
            ECSqlLexer::Snapshot lexSnap = m_lexer->SavePos();
            ECSqlToken savedTok = m_current;

            // Speculatively try value-expression path (handles value operators & comparisons).
            // Suppress errors during this attempt — if it fails, we'll use the boolean path.
            m_suppressErrors = true;
            std::unique_ptr<ValueExp> lhsVal;
            bool valuePathOk = (SUCCESS == ParseValueExp(lhsVal));
            m_suppressErrors = false;
            if (valuePathOk)
                {
                // Value path succeeded: handle comparison, IS, BETWEEN, LIKE, IN, etc.
                // Fall through to the standard predicate completion logic below.
                if (At(ECSqlTokenType::KW_IS))
                    {
                    Advance();
                    bool isNot = TryConsume(ECSqlTokenType::KW_NOT);
                    if (At(ECSqlTokenType::KW_NULL) || At(ECSqlTokenType::KW_TRUE)
                        || At(ECSqlTokenType::KW_FALSE) || At(ECSqlTokenType::KW_UNKNOWN))
                        {
                        Utf8String lit; ECSqlTypeInfo dtype;
                        if (SUCCESS != ParseLiteral(lit, dtype)) return ERROR;
                        std::unique_ptr<ValueExp> rhsVal;
                        if (SUCCESS != LiteralValueExp::Create(rhsVal, *m_context, lit.c_str(), dtype)) return ERROR;
                        exp = std::make_unique<BinaryBooleanExp>(std::move(lhsVal),
                            isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is, std::move(rhsVal));
                        return SUCCESS;
                        }
                    if (At(ECSqlTokenType::LParen))
                        {
                        std::unique_ptr<ValueExp> typeListExp;
                        if (SUCCESS != ParseTypePredicate(typeListExp)) return ERROR;
                        BooleanSqlOperator op = isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is;
                        exp = std::make_unique<BinaryBooleanExp>(std::move(lhsVal), op, std::move(typeListExp));
                        return SUCCESS;
                        }
                    ECSQLERR("Expected NULL, TRUE, FALSE, UNKNOWN, or type list after IS");
                    return ERROR;
                    }

                bool isNot2 = false;
                if (At(ECSqlTokenType::KW_NOT) || At(ECSqlTokenType::KW_BETWEEN)
                    || At(ECSqlTokenType::KW_LIKE) || At(ECSqlTokenType::KW_IN)
                    || At(ECSqlTokenType::KW_MATCH))
                    {
                    isNot2 = TryConsume(ECSqlTokenType::KW_NOT);
                    if (At(ECSqlTokenType::KW_BETWEEN))
                        return ParseBetweenPredicate(exp, std::move(lhsVal), isNot2);
                    if (At(ECSqlTokenType::KW_LIKE))
                        return ParseLikePredicate(exp, std::move(lhsVal), isNot2);
                    if (At(ECSqlTokenType::KW_IN))
                        return ParseInPredicate(exp, std::move(lhsVal), isNot2);
                    if (At(ECSqlTokenType::KW_MATCH))
                        return ParseMatchPredicate(exp, std::move(lhsVal), isNot2);
                    ECSQLERR("Expected BETWEEN, LIKE, IN, or MATCH after NOT");
                    return ERROR;
                    }

                if (IsComparisonOp())
                    {
                    BooleanSqlOperator op = ParseComparisonOp();
                    Advance();
                    if (At(ECSqlTokenType::KW_ALL) || At(ECSqlTokenType::KW_ANY) || At(ECSqlTokenType::KW_SOME))
                        {
                        SqlCompareListType quantifier = At(ECSqlTokenType::KW_ALL) ? SqlCompareListType::All
                                                      : (At(ECSqlTokenType::KW_ANY) ? SqlCompareListType::Any
                                                                                    : SqlCompareListType::Some);
                        Advance();
                        std::unique_ptr<SubqueryExp> subExp;
                        if (SUCCESS != ParseSubquery(subExp)) return ERROR;
                        exp = std::make_unique<AllOrAnyExp>(std::move(lhsVal), op, quantifier, std::move(subExp));
                        return SUCCESS;
                        }
                    std::unique_ptr<ValueExp> rhs;
                    if (SUCCESS != ParseValueExp(rhs)) return ERROR;
                    exp = std::make_unique<BinaryBooleanExp>(std::move(lhsVal), op, std::move(rhs));
                    return SUCCESS;
                    }

                // Bare value used as boolean predicate
                exp = std::make_unique<UnaryPredicateExp>(std::move(lhsVal));
                return SUCCESS;
                }

            // Value path failed (content is a boolean expression like "L < 3.14 AND I > 3").
            // Restore state and parse as (search_condition).
            m_lexer->RestorePos(lexSnap);
            m_current = savedTok;
            Advance(); // consume '('
            std::unique_ptr<BooleanExp> inner;
            if (SUCCESS != ParseSearchCondition(inner))
                return ERROR;
            if (!Expect(ECSqlTokenType::RParen))
                return ERROR;
            inner->SetHasParentheses();
            exp = std::move(inner);
            return SUCCESS;
            }
        }

    // EXISTS ( subquery )
    if (At(ECSqlTokenType::KW_EXISTS))
        {
        Advance();
        std::unique_ptr<SubqueryExp> subExp;
        if (SUCCESS != ParseSubquery(subExp))
            return ERROR;
        exp = std::make_unique<SubqueryTestExp>(SubqueryTestOperator::Exists, std::move(subExp));
        return SUCCESS;
        }

    // UNIQUE ( subquery )
    if (At(ECSqlTokenType::KW_UNIQUE))
        {
        Advance();
        std::unique_ptr<SubqueryExp> subExp;
        if (SUCCESS != ParseSubquery(subExp))
            return ERROR;
        exp = std::make_unique<SubqueryTestExp>(SubqueryTestOperator::Unique, std::move(subExp));
        return SUCCESS;
        }

    // General: parse a value expression and check what follows
    std::unique_ptr<ValueExp> lhsVal;
    if (SUCCESS != ParseValueExp(lhsVal))
        return ERROR;

    // IS [NOT] (NULL | TRUE | FALSE | UNKNOWN)
    if (At(ECSqlTokenType::KW_IS))
        {
        Advance(); // consume IS
        bool isNot = TryConsume(ECSqlTokenType::KW_NOT);
        if (At(ECSqlTokenType::KW_NULL) || At(ECSqlTokenType::KW_TRUE)
            || At(ECSqlTokenType::KW_FALSE) || At(ECSqlTokenType::KW_UNKNOWN))
            {
            Utf8String lit;
            ECSqlTypeInfo dtype;
            if (SUCCESS != ParseLiteral(lit, dtype))
                return ERROR;
            std::unique_ptr<ValueExp> rhsVal;
            if (SUCCESS != LiteralValueExp::Create(rhsVal, *m_context, lit.c_str(), dtype))
                return ERROR;
            exp = std::make_unique<BinaryBooleanExp>(std::move(lhsVal),
                isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is, std::move(rhsVal));
            return SUCCESS;
            }
        // IS [NOT] ( type_list ) — type predicate
        if (At(ECSqlTokenType::LParen))
            {
            // This is a type_predicate: expr IS [NOT] (schema.Class, ...)
            std::unique_ptr<ValueExp> typeListExp;
            if (SUCCESS != ParseTypePredicate(typeListExp))
                return ERROR;
            BooleanSqlOperator op = isNot ? BooleanSqlOperator::IsNot : BooleanSqlOperator::Is;
            exp = std::make_unique<BinaryBooleanExp>(std::move(lhsVal), op, std::move(typeListExp));
            return SUCCESS;
            }
        ECSQLERR("Expected NULL, TRUE, FALSE, UNKNOWN, or type list after IS");
        return ERROR;
        }

    // [NOT] BETWEEN lhs AND rhs
    if (At(ECSqlTokenType::KW_NOT) || At(ECSqlTokenType::KW_BETWEEN)
        || At(ECSqlTokenType::KW_LIKE) || At(ECSqlTokenType::KW_IN)
        || At(ECSqlTokenType::KW_MATCH))
        {
        bool isNot = TryConsume(ECSqlTokenType::KW_NOT);
        if (At(ECSqlTokenType::KW_BETWEEN))
            return ParseBetweenPredicate(exp, std::move(lhsVal), isNot);
        if (At(ECSqlTokenType::KW_LIKE))
            return ParseLikePredicate(exp, std::move(lhsVal), isNot);
        if (At(ECSqlTokenType::KW_IN))
            return ParseInPredicate(exp, std::move(lhsVal), isNot);
        if (At(ECSqlTokenType::KW_MATCH))
            return ParseMatchPredicate(exp, std::move(lhsVal), isNot);
        ECSQLERR("Expected BETWEEN, LIKE, IN, or MATCH after NOT");
        return ERROR;
        }

    // Comparison operator: = < > <= >= <> !=
    if (IsComparisonOp())
        {
        BooleanSqlOperator op = ParseComparisonOp();
        Advance(); // consume the comparison operator token

        // ALL / ANY / SOME quantifier check
        if (At(ECSqlTokenType::KW_ALL) || At(ECSqlTokenType::KW_ANY) || At(ECSqlTokenType::KW_SOME))
            {
            SqlCompareListType quantifier = SqlCompareListType::All;
            if (At(ECSqlTokenType::KW_ALL))      quantifier = SqlCompareListType::All;
            else if (At(ECSqlTokenType::KW_ANY)) quantifier = SqlCompareListType::Any;
            else                                  quantifier = SqlCompareListType::Some;
            Advance();
            std::unique_ptr<SubqueryExp> subExp;
            if (SUCCESS != ParseSubquery(subExp))
                return ERROR;
            exp = std::make_unique<AllOrAnyExp>(std::move(lhsVal), op, quantifier, std::move(subExp));
            return SUCCESS;
            }

        std::unique_ptr<ValueExp> rhs;
        if (SUCCESS != ParseValueExp(rhs))
            return ERROR;

        // IS [NOT] TRUE/FALSE/UNKNOWN check (boolean test)
        // (already handled above)

        exp = std::make_unique<BinaryBooleanExp>(std::move(lhsVal), op, std::move(rhs));
        return SUCCESS;
        }

    // Bare value expression used as boolean predicate (e.g., a column in WHERE clause)
    exp = std::make_unique<UnaryPredicateExp>(std::move(lhsVal));
    return SUCCESS;
    }

bool ECSqlRDParser::IsComparisonOp() const
    {
    return At(ECSqlTokenType::Equal)
        || At(ECSqlTokenType::Less)
        || At(ECSqlTokenType::Great)
        || At(ECSqlTokenType::LessEq)
        || At(ECSqlTokenType::GreatEq)
        || At(ECSqlTokenType::NotEqual);
    }

BooleanSqlOperator ECSqlRDParser::ParseComparisonOp()
    {
    switch (m_current.type)
        {
        case ECSqlTokenType::Equal:    return BooleanSqlOperator::EqualTo;
        case ECSqlTokenType::Less:     return BooleanSqlOperator::LessThan;
        case ECSqlTokenType::Great:    return BooleanSqlOperator::GreaterThan;
        case ECSqlTokenType::LessEq:   return BooleanSqlOperator::LessThanOrEqualTo;
        case ECSqlTokenType::GreatEq:  return BooleanSqlOperator::GreaterThanOrEqualTo;
        case ECSqlTokenType::NotEqual: return BooleanSqlOperator::NotEqualTo;
        default:                       return BooleanSqlOperator::And; // should not reach
        }
    }

BentleyStatus ECSqlRDParser::ParseInPredicate(std::unique_ptr<BooleanExp>& exp, std::unique_ptr<ValueExp> lhs, bool isNot)
    {
    BeAssert(At(ECSqlTokenType::KW_IN));
    Advance(); // consume IN
    BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotIn : BooleanSqlOperator::In;

    if (!At(ECSqlTokenType::LParen))
        {
        ECSQLERR("Expected '(' after IN");
        return ERROR;
        }

    // Peek: subquery or value list?
    ECSqlToken next = m_lexer->Peek1();
    if (next.type == ECSqlTokenType::KW_SELECT || next.type == ECSqlTokenType::KW_WITH
        || next.type == ECSqlTokenType::KW_VALUES)
        {
        // ParseSubquery itself consumes the surrounding '(' ... ')'
        std::unique_ptr<SubqueryExp> subExp;
        if (SUCCESS != ParseSubquery(subExp))
            return ERROR;
        auto subValExp = std::make_unique<SubqueryValueExp>(std::move(subExp));
        exp = std::make_unique<BinaryBooleanExp>(std::move(lhs), op, std::move(subValExp));
        return SUCCESS;
        }

    // Value list: ( val, val, ... )
    Advance(); // consume '('
    std::unique_ptr<ValueExpListExp> listExp;
    if (SUCCESS != ParseValueExpCommalist(listExp))
        return ERROR;
    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    exp = std::make_unique<BinaryBooleanExp>(std::move(lhs), op, std::move(listExp));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseLikePredicate(std::unique_ptr<BooleanExp>& exp, std::unique_ptr<ValueExp> lhs, bool isNot)
    {
    BeAssert(At(ECSqlTokenType::KW_LIKE));
    Advance(); // consume LIKE
    BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotLike : BooleanSqlOperator::Like;

    std::unique_ptr<ValueExp> pattern;
    if (SUCCESS != ParseValueExp(pattern))
        return ERROR;

    std::unique_ptr<ValueExp> escapeExp;
    if (At(ECSqlTokenType::KW_ESCAPE))
        {
        Advance();
        // ESCAPE must be a single-character string literal
        if (!At(ECSqlTokenType::String))
            {
            ECSQLERR("ESCAPE clause requires a string literal");
            return ERROR;
            }
        if (SUCCESS != ParseValueExp(escapeExp))
            return ERROR;
        }

    auto rhsExp = std::make_unique<LikeRhsValueExp>(std::move(pattern), std::move(escapeExp));
    exp = std::make_unique<BinaryBooleanExp>(std::move(lhs), op, std::move(rhsExp));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseBetweenPredicate(std::unique_ptr<BooleanExp>& exp, std::unique_ptr<ValueExp> lhs, bool isNot)
    {
    BeAssert(At(ECSqlTokenType::KW_BETWEEN));
    Advance(); // consume BETWEEN
    BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotBetween : BooleanSqlOperator::Between;

    std::unique_ptr<ValueExp> lower;
    if (SUCCESS != ParseValueExp(lower))
        return ERROR;
    if (!Expect(ECSqlTokenType::KW_AND))
        return ERROR;
    std::unique_ptr<ValueExp> upper;
    if (SUCCESS != ParseValueExp(upper))
        return ERROR;

    auto rangeExp = std::make_unique<BetweenRangeValueExp>(std::move(lower), std::move(upper));
    exp = std::make_unique<BinaryBooleanExp>(std::move(lhs), op, std::move(rangeExp));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseNullPredicate(std::unique_ptr<BooleanExp>& exp, std::unique_ptr<ValueExp> lhs)
    {
    // IS [NOT] NULL — already handled in ParsePredicate
    BeAssert(false && "ParseNullPredicate should not be called directly");
    return ERROR;
    }

BentleyStatus ECSqlRDParser::ParseMatchPredicate(std::unique_ptr<BooleanExp>& exp, std::unique_ptr<ValueExp> lhs, bool isNot)
    {
    BeAssert(At(ECSqlTokenType::KW_MATCH));
    Advance(); // consume MATCH

    std::unique_ptr<ValueExp> rhs;
    if (SUCCESS != ParseFctSpec(rhs))
        return ERROR;

    BooleanSqlOperator op = isNot ? BooleanSqlOperator::NotMatch : BooleanSqlOperator::Match;
    exp = std::make_unique<BinaryBooleanExp>(std::move(lhs), op, std::move(rhs));
    return SUCCESS;
    }

//=============================================================================
// Window function parsers
//=============================================================================

BentleyStatus ECSqlRDParser::ParseWindowFunction(std::unique_ptr<ValueExp>& exp, std::unique_ptr<ValueExp> functionCallExp)
    {
    // Parse optional FILTER (WHERE ...)
    std::unique_ptr<FilterClauseExp> filterExp;
    if (SUCCESS != ParseFilterClause(filterExp))
        return ERROR;

    if (!Expect(ECSqlTokenType::KW_OVER))
        return ERROR;

    // OVER window_name | OVER ( window_spec )
    if (At(ECSqlTokenType::LParen))
        {
        Advance(); // consume '('
        std::unique_ptr<WindowSpecification> spec;
        if (SUCCESS != ParseWindowSpecification(spec))
            return ERROR;
        if (!Expect(ECSqlTokenType::RParen))
            return ERROR;
        exp = std::make_unique<WindowFunctionExp>(std::move(functionCallExp), std::move(filterExp), std::move(spec));
        }
    else if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
        {
        Utf8String winName = TokenText();
        Advance();
        exp = std::make_unique<WindowFunctionExp>(std::move(functionCallExp), std::move(filterExp), winName.c_str());
        }
    else
        {
        ECSQLERR("Expected window name or '(' after OVER");
        return ERROR;
        }

    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseWindowSpecification(std::unique_ptr<WindowSpecification>& exp)
    {
    // window_spec: [existing_window_name] [PARTITION BY ...] [ORDER BY ...] [frame_clause]
    // Note: closing ')' consumed by caller

    Utf8String existingWindowName;
    if (At(ECSqlTokenType::Name) && !At(ECSqlTokenType::KW_PARTITION)
        && !At(ECSqlTokenType::KW_ORDER) && !At(ECSqlTokenType::KW_ROWS)
        && !At(ECSqlTokenType::KW_RANGE) && !At(ECSqlTokenType::KW_GROUPS))
        {
        // If the NEXT token is a clause-starter (PARTITION/ORDER/frame-unit) or ')' (end of spec),
        // the current Name is the existing window reference.
        // e.g.  OVER(win ROWS BETWEEN ...) — 'win' followed by ROWS → it's the window name.
        ECSqlToken next = m_lexer->Peek1();
        if (next.type == ECSqlTokenType::KW_PARTITION || next.type == ECSqlTokenType::KW_ORDER
            || next.type == ECSqlTokenType::KW_ROWS || next.type == ECSqlTokenType::KW_RANGE
            || next.type == ECSqlTokenType::KW_GROUPS || next.type == ECSqlTokenType::RParen)
            {
            existingWindowName = TokenText();
            Advance();
            }
        }

    std::unique_ptr<WindowPartitionColumnReferenceListExp> partitionExp;
    if (SUCCESS != ParseWindowPartitionClause(partitionExp))
        return ERROR;

    std::unique_ptr<OrderByExp> orderByExp;
    if (At(ECSqlTokenType::KW_ORDER))
        {
        if (SUCCESS != ParseOrderByClause(orderByExp))
            return ERROR;
        }

    std::unique_ptr<WindowFrameClauseExp> frameExp;
    if (SUCCESS != ParseWindowFrameClause(frameExp))
        return ERROR;

    exp = std::make_unique<WindowSpecification>(existingWindowName, std::move(partitionExp), std::move(orderByExp), std::move(frameExp));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseWindowPartitionClause(std::unique_ptr<WindowPartitionColumnReferenceListExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_PARTITION))
        return SUCCESS;
    Advance(); // consume PARTITION
    if (!Expect(ECSqlTokenType::KW_BY))
        return ERROR;

    std::vector<std::unique_ptr<WindowPartitionColumnReferenceExp>> refs;
    do
        {
        std::unique_ptr<ValueExp> colRef;
        if (SUCCESS != ParseColumnRef(colRef, false))
            return ERROR;

        // Optional COLLATE clause
        WindowPartitionColumnReferenceExp::CollateClauseFunction collate = WindowPartitionColumnReferenceExp::CollateClauseFunction::NotSpecified;
        if (At(ECSqlTokenType::KW_COLLATE))
            {
            Advance();
            if (At(ECSqlTokenType::KW_BINARY))        { collate = WindowPartitionColumnReferenceExp::CollateClauseFunction::Binary; Advance(); }
            else if (At(ECSqlTokenType::KW_NOCASE))   { collate = WindowPartitionColumnReferenceExp::CollateClauseFunction::NoCase; Advance(); }
            else if (At(ECSqlTokenType::KW_RTRIM))    { collate = WindowPartitionColumnReferenceExp::CollateClauseFunction::Rtrim; Advance(); }
            else { ECSQLERR("Unsupported collate function"); return ERROR; }
            }

        refs.push_back(std::make_unique<WindowPartitionColumnReferenceExp>(std::move(colRef), collate));
        }
    while (TryConsume(ECSqlTokenType::Comma));

    exp = std::make_unique<WindowPartitionColumnReferenceListExp>(refs);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseWindowFrameClause(std::unique_ptr<WindowFrameClauseExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_ROWS) && !At(ECSqlTokenType::KW_RANGE) && !At(ECSqlTokenType::KW_GROUPS))
        return SUCCESS;

    WindowFrameClauseExp::WindowFrameUnit unit;
    if (At(ECSqlTokenType::KW_ROWS))        { unit = WindowFrameClauseExp::WindowFrameUnit::Rows;   Advance(); }
    else if (At(ECSqlTokenType::KW_RANGE))  { unit = WindowFrameClauseExp::WindowFrameUnit::Range;  Advance(); }
    else                                     { unit = WindowFrameClauseExp::WindowFrameUnit::Groups; Advance(); }

    // BETWEEN ... AND ... | frame_start
    WindowFrameClauseExp::WindowFrameExclusionType exclusion = WindowFrameClauseExp::WindowFrameExclusionType::NotSpecified;

    if (At(ECSqlTokenType::KW_BETWEEN))
        {
        std::unique_ptr<WindowFrameBetweenExp> betweenExp;
        if (SUCCESS != ParseWindowFrameBetween(betweenExp))
            return ERROR;
        // Check for EXCLUDE clause
        if (At(ECSqlTokenType::KW_EXCLUDE))
            {
            Advance();
            if (At(ECSqlTokenType::KW_GROUP))        { exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeGroup; Advance(); }
            else if (At(ECSqlTokenType::KW_TIES))    { exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeTies; Advance(); }
            else if (At(ECSqlTokenType::KW_CURRENT))
                {
                Advance();
                if (!Expect(ECSqlTokenType::KW_ROW)) return ERROR;
                exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeCurrentRow;
                }
            else if (At(ECSqlTokenType::KW_NO))
                {
                Advance();
                if (!Expect(ECSqlTokenType::KW_OTHERS)) return ERROR;
                exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeNoOthers;
                }
            }
        exp = std::make_unique<WindowFrameClauseExp>(unit, exclusion, std::move(betweenExp));
        return SUCCESS;
        }

    // frame_start
    std::unique_ptr<WindowFrameStartExp> startExp;
    if (SUCCESS != ParseWindowFrameStart(startExp))
        return ERROR;

    // Check for EXCLUDE
    if (At(ECSqlTokenType::KW_EXCLUDE))
        {
        Advance();
        if (At(ECSqlTokenType::KW_GROUP))        { exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeGroup; Advance(); }
        else if (At(ECSqlTokenType::KW_TIES))    { exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeTies; Advance(); }
        else if (At(ECSqlTokenType::KW_CURRENT)) { Advance(); if (!Expect(ECSqlTokenType::KW_ROW)) return ERROR; exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeCurrentRow; }
        else if (At(ECSqlTokenType::KW_NO))      { Advance(); if (!Expect(ECSqlTokenType::KW_OTHERS)) return ERROR; exclusion = WindowFrameClauseExp::WindowFrameExclusionType::ExcludeNoOthers; }
        }
    exp = std::make_unique<WindowFrameClauseExp>(unit, exclusion, std::move(startExp));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseWindowFrameStart(std::unique_ptr<WindowFrameStartExp>& exp)
    {
    if (At(ECSqlTokenType::KW_UNBOUNDED))
        {
        Advance();
        if (!Expect(ECSqlTokenType::KW_PRECEDING))
            return ERROR;
        exp = std::make_unique<WindowFrameStartExp>(WindowFrameStartExp::WindowFrameStartType::UnboundedPreceding);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_CURRENT))
        {
        Advance();
        if (!Expect(ECSqlTokenType::KW_ROW))
            return ERROR;
        exp = std::make_unique<WindowFrameStartExp>(WindowFrameStartExp::WindowFrameStartType::CurrentRow);
        return SUCCESS;
        }
    // value PRECEDING
    std::unique_ptr<ValueExp> val;
    if (SUCCESS != ParseValueExp(val))
        return ERROR;
    if (!Expect(ECSqlTokenType::KW_PRECEDING))
        return ERROR;
    exp = std::make_unique<WindowFrameStartExp>(WindowFrameStartExp::WindowFrameStartType::ValuePreceding, std::move(val));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseWindowFrameBetween(std::unique_ptr<WindowFrameBetweenExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::KW_BETWEEN));
    Advance(); // consume BETWEEN

    std::unique_ptr<FirstWindowFrameBoundExp> first;
    if (SUCCESS != ParseFirstWindowFrameBound(first))
        return ERROR;
    if (!Expect(ECSqlTokenType::KW_AND))
        return ERROR;
    std::unique_ptr<SecondWindowFrameBoundExp> second;
    if (SUCCESS != ParseSecondWindowFrameBound(second))
        return ERROR;

    exp = std::make_unique<WindowFrameBetweenExp>(std::move(first), std::move(second));
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseFirstWindowFrameBound(std::unique_ptr<FirstWindowFrameBoundExp>& exp)
    {
    if (At(ECSqlTokenType::KW_UNBOUNDED))
        {
        Advance();
        if (!Expect(ECSqlTokenType::KW_PRECEDING))
            return ERROR;
        exp = std::make_unique<FirstWindowFrameBoundExp>(FirstWindowFrameBoundExp::WindowFrameBoundType::UnboundedPreceding);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_CURRENT))
        {
        Advance();
        if (!Expect(ECSqlTokenType::KW_ROW))
            return ERROR;
        exp = std::make_unique<FirstWindowFrameBoundExp>(FirstWindowFrameBoundExp::WindowFrameBoundType::CurrentRow);
        return SUCCESS;
        }

    std::unique_ptr<ValueExp> val;
    if (SUCCESS != ParseValueExp(val))
        return ERROR;

    if (At(ECSqlTokenType::KW_PRECEDING))
        {
        Advance();
        exp = std::make_unique<FirstWindowFrameBoundExp>(std::move(val), FirstWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_FOLLOWING))
        {
        Advance();
        exp = std::make_unique<FirstWindowFrameBoundExp>(std::move(val), FirstWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing);
        return SUCCESS;
        }
    ECSQLERR("Expected PRECEDING or FOLLOWING");
    return ERROR;
    }

BentleyStatus ECSqlRDParser::ParseSecondWindowFrameBound(std::unique_ptr<SecondWindowFrameBoundExp>& exp)
    {
    if (At(ECSqlTokenType::KW_UNBOUNDED))
        {
        Advance();
        if (!Expect(ECSqlTokenType::KW_FOLLOWING))
            return ERROR;
        exp = std::make_unique<SecondWindowFrameBoundExp>(SecondWindowFrameBoundExp::WindowFrameBoundType::UnboundedFollowing);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_CURRENT))
        {
        Advance();
        if (!Expect(ECSqlTokenType::KW_ROW))
            return ERROR;
        exp = std::make_unique<SecondWindowFrameBoundExp>(SecondWindowFrameBoundExp::WindowFrameBoundType::CurrentRow);
        return SUCCESS;
        }

    std::unique_ptr<ValueExp> val;
    if (SUCCESS != ParseValueExp(val))
        return ERROR;

    if (At(ECSqlTokenType::KW_PRECEDING))
        {
        Advance();
        exp = std::make_unique<SecondWindowFrameBoundExp>(std::move(val), SecondWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding);
        return SUCCESS;
        }
    if (At(ECSqlTokenType::KW_FOLLOWING))
        {
        Advance();
        exp = std::make_unique<SecondWindowFrameBoundExp>(std::move(val), SecondWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing);
        return SUCCESS;
        }
    ECSQLERR("Expected PRECEDING or FOLLOWING");
    return ERROR;
    }

BentleyStatus ECSqlRDParser::ParseFilterClause(std::unique_ptr<FilterClauseExp>& exp)
    {
    exp = nullptr;
    if (!At(ECSqlTokenType::KW_FILTER))
        return SUCCESS;
    Advance(); // consume FILTER
    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;
    std::unique_ptr<WhereExp> whereExp;
    if (SUCCESS != ParseWhereClause(whereExp))
        return ERROR;
    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;
    exp = std::make_unique<FilterClauseExp>(std::move(whereExp));
    return SUCCESS;
    }

//=============================================================================
// NAVIGATION_VALUE function parser
//=============================================================================

BentleyStatus ECSqlRDParser::ParseValueCreationFuncExp(std::unique_ptr<ValueExp>& exp)
    {
    BeAssert(At(ECSqlTokenType::KW_NAVIGATION_VALUE));
    Advance(); // consume NAVIGATION_VALUE
    std::unique_ptr<NavValueCreationFuncExp> navExp;
    if (SUCCESS != ParseNavValueCreationFuncExp(navExp))
        return ERROR;
    exp = std::move(navExp);
    return SUCCESS;
    }

BentleyStatus ECSqlRDParser::ParseNavValueCreationFuncExp(std::unique_ptr<NavValueCreationFuncExp>& exp)
    {
    // Grammar: NAVIGATION_VALUE ( schema.ClassName alias?, derivedPropExp, idArg [, relECClassIdArg] )
    // But actually: NAVIGATION_VALUE ( schema.Class.propertyPath AS alias, idExpr [, relClassIdExpr] )
    // Looking at ECSqlParser.cpp ParseNavValueCreationFuncExp:
    //   parseNode->getChild(2) = derived_column (schema.class.prop [AS alias])
    //   parseNode->getChild(4) = idArg
    //   parseNode->getChild(5) = optional relECClassIdArg (count != 0)
    // The NAVIGATION_VALUE token was already consumed above.

    if (!Expect(ECSqlTokenType::LParen))
        return ERROR;

    // Parse the inner derived column: schema.Class.propName [AS alias]
    std::unique_ptr<DerivedPropertyExp> derivedProp;
    if (SUCCESS != ParseDerivedColumn(derivedProp))
        return ERROR;

    if (!Expect(ECSqlTokenType::Comma))
        return ERROR;

    // The derived property should be schema.Class.propName
    if (!derivedProp || derivedProp->GetExpression()->GetType() != Exp::Type::PropertyName)
        {
        ECSQLERR("NAVIGATION_VALUE: first argument must be a 3-part property path");
        return ERROR;
        }

    PropertyPath const& propPath = derivedProp->GetExpression()->GetAs<PropertyNameExp>().GetResolvedPropertyPath();
    if (propPath.Size() != 3)
        {
        ECSQLERR("NAVIGATION_VALUE: property path must have exactly 3 components (schema.class.property)");
        return ERROR;
        }

    ClassMap const* classMap = m_context->GetECDb().Schemas().GetDispatcher().GetClassMap(
        propPath[0].GetName(), propPath[1].GetName(), SchemaLookupMode::AutoDetect, nullptr);
    if (classMap == nullptr)
        {
        ECSQLERR("NAVIGATION_VALUE: class '%s.%s' not found", propPath[0].GetName().c_str(), propPath[1].GetName().c_str());
        return ERROR;
        }

    PropertyPath mutablePath = propPath;
    mutablePath.SetClassMap(*classMap);

    std::shared_ptr<ClassNameExp::Info> classInfo;
    if (SUCCESS != m_context->TryResolveClass(classInfo, nullptr, propPath[0].GetName(), propPath[1].GetName(), ECSqlType::Select, false, false))
        return ERROR;

    auto classNameExp = std::make_unique<ClassNameExp>(propPath[1].GetName().c_str(), propPath[0].GetName().c_str(), nullptr, classInfo);

    if (classMap->GetPropertyMaps().Find(propPath[2].GetName().c_str()) == nullptr)
        {
        ECSQLERR("NAVIGATION_VALUE: property '%s' not found in class", propPath[2].GetName().c_str());
        return ERROR;
        }

    auto propNameExp = std::make_unique<PropertyNameExp>(
        *m_context,
        propPath[2].GetName(),
        *classNameExp.get(),
        *classMap
    );
    propNameExp->SetTypeInfo(ECSqlTypeInfo(ECSqlTypeInfo::Kind::Navigation));

    // Use alias from derivedProp (if any)
    Utf8CP alias = derivedProp->GetColumnAlias().empty() ? nullptr : derivedProp->GetColumnAlias().c_str();
    auto innerDerived = std::make_unique<DerivedPropertyExp>(std::move(propNameExp), alias);

    // idArg
    std::unique_ptr<ValueExp> idArg;
    if (SUCCESS != ParseValueExp(idArg))
        return ERROR;

    // Optional relECClassIdArg
    std::unique_ptr<ValueExp> relClassIdArg;
    if (TryConsume(ECSqlTokenType::Comma))
        {
        if (SUCCESS != ParseValueExp(relClassIdArg))
            return ERROR;
        }

    if (!Expect(ECSqlTokenType::RParen))
        return ERROR;

    auto navFuncExp = std::make_unique<NavValueCreationFuncExp>(std::move(innerDerived), std::move(idArg), std::move(relClassIdArg), std::move(classNameExp));
    navFuncExp->SetTypeInfo(ECSqlTypeInfo(*classMap->GetPropertyMaps().Find(propPath[2].GetName().c_str())));
    exp = std::move(navFuncExp);
    return SUCCESS;
    }

//=============================================================================
// Helper: IsAggregateFunction
//=============================================================================

bool ECSqlRDParser::IsAggregateFunction(Utf8StringCR name) const
    {
    return name.EqualsIAscii("COUNT")
        || name.EqualsIAscii("SUM")
        || name.EqualsIAscii("AVG")
        || name.EqualsIAscii("MIN")
        || name.EqualsIAscii("MAX")
        || name.EqualsIAscii("GROUP_CONCAT")
        || name.EqualsIAscii("TOTAL")
        || name.EqualsIAscii("ANY")
        || name.EqualsIAscii("EVERY")
        || name.EqualsIAscii("SOME");
    }

bool ECSqlRDParser::IsWindowFunctionName(Utf8StringCR name) const
    {
    return name.EqualsIAscii("ROW_NUMBER")
        || name.EqualsIAscii("RANK")
        || name.EqualsIAscii("DENSE_RANK")
        || name.EqualsIAscii("PERCENT_RANK")
        || name.EqualsIAscii("CUME_DIST")
        || name.EqualsIAscii("NTILE")
        || name.EqualsIAscii("LEAD")
        || name.EqualsIAscii("LAG")
        || name.EqualsIAscii("FIRST_VALUE")
        || name.EqualsIAscii("LAST_VALUE")
        || name.EqualsIAscii("NTH_VALUE");
    }

//=============================================================================
// ParseJoinedTable – builds a JoinExp given the already-parsed LHS table ref
//=============================================================================

BentleyStatus ECSqlRDParser::ParseJoinedTable(std::unique_ptr<JoinExp>& exp,
    std::unique_ptr<ClassRefExp> lhs, ECSqlType ecsqlType)
    {
    // This is called when a JOIN keyword follows a table ref.
    // It handles one join step; caller may call again for chaining.
    bool isNatural = false;
    bool isCross = false;
    ECSqlJoinType joinType = ECSqlJoinType::InnerJoin;

    if (TryConsume(ECSqlTokenType::KW_NATURAL))
        isNatural = true;

    if (At(ECSqlTokenType::KW_CROSS))
        {
        Advance();
        isCross = true;
        if (!Expect(ECSqlTokenType::KW_JOIN))
            return ERROR;
        }
    else if (At(ECSqlTokenType::KW_JOIN))
        {
        Advance();
        joinType = ECSqlJoinType::InnerJoin;
        }
    else
        {
        if (SUCCESS != ParseJoinType(joinType))
            return ERROR;
        if (At(ECSqlTokenType::KW_OUTER))
            Advance(); // optional OUTER
        if (!Expect(ECSqlTokenType::KW_JOIN))
            return ERROR;
        }

    std::unique_ptr<ClassRefExp> rhs;
    if (SUCCESS != ParseTableRef(rhs, ecsqlType))
        return ERROR;

    if (isCross)
        {
        exp = std::make_unique<CrossJoinExp>(std::move(lhs), std::move(rhs));
        return SUCCESS;
        }
    if (isNatural)
        {
        exp = std::make_unique<NaturalJoinExp>(std::move(lhs), std::move(rhs), joinType);
        return SUCCESS;
        }

    // ECRelationship join check: USING RELATIONSHIP
    if (At(ECSqlTokenType::KW_USING))
        {
        ECSqlToken next = m_lexer->Peek1();
        if (next.type != ECSqlTokenType::LParen)
            {
            Advance(); // consume USING
            if (At(ECSqlTokenType::Name) && m_current.TextEqualsI("RELATIONSHIP"))
                Advance();
            std::unique_ptr<ClassRefExp> relClassRef;
            if (SUCCESS != ParseTableRef(relClassRef, ecsqlType))
                return ERROR;
            JoinDirection dir = JoinDirection::Implied;
            if (At(ECSqlTokenType::KW_FORWARD))  { dir = JoinDirection::Forward;  Advance(); }
            else if (At(ECSqlTokenType::KW_BACKWARD)) { dir = JoinDirection::Backward; Advance(); }
            exp = std::make_unique<UsingRelationshipJoinExp>(std::move(lhs), std::move(rhs), std::move(relClassRef), dir);
            return SUCCESS;
            }
        }

    std::unique_ptr<JoinSpecExp> joinSpec;
    if (SUCCESS != ParseJoinSpec(joinSpec))
        return ERROR;

    exp = std::make_unique<QualifiedJoinExp>(std::move(lhs), std::move(rhs), joinType, std::move(joinSpec));
    return SUCCESS;
    }



//=============================================================================
// ParsePragmaValue / ParseALLorONLY – additional helpers
//=============================================================================

BentleyStatus ECSqlRDParser::ParsePragmaValue(PragmaVal& val)
    {
    if (At(ECSqlTokenType::KW_TRUE))    { val = PragmaVal(true);                                          Advance(); return SUCCESS; }
    if (At(ECSqlTokenType::KW_FALSE))   { val = PragmaVal(false);                                         Advance(); return SUCCESS; }
    if (At(ECSqlTokenType::KW_NULL))    { val = PragmaVal::Null();                                        Advance(); return SUCCESS; }
    if (At(ECSqlTokenType::IntNum))     { val = PragmaVal((int64_t)std::stoll(TokenText().c_str()));       Advance(); return SUCCESS; }
    if (At(ECSqlTokenType::ApproxNum))  { val = PragmaVal(std::stod(TokenText().c_str()));                 Advance(); return SUCCESS; }
    if (At(ECSqlTokenType::String))
        {
        Utf8String s = TokenText();
        // strip enclosing quotes 'value' → value
        if ((unsigned char)s.front() == 39)  // 39 = ASCII single-quote
            s = s.substr(1, s.size() - 2);
        val = PragmaVal(s, false);
        Advance(); return SUCCESS;
        }
    if (At(ECSqlTokenType::Name) || m_current.IsKeyword())
        { val = PragmaVal(TokenText(), true); Advance(); return SUCCESS; }
    ECSQLERR("Expected pragma value");
    return ERROR;
    }

BentleyStatus ECSqlRDParser::ParseALLorONLY(PolymorphicInfo& constraint)
    {
    constraint = PolymorphicInfo::NotSpecified();
    if (At(ECSqlTokenType::KW_ALL))
        {
        constraint = PolymorphicInfo(PolymorphicInfo::Type::All, false);
        Advance();
        }
    else if (At(ECSqlTokenType::KW_ONLY))
        {
        constraint = PolymorphicInfo(PolymorphicInfo::Type::Only, false);
        Advance();
        }
    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
