/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlBisonParser.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

#include "ECSql.tab.hpp"    // bison-generated: ECSqlYYSTYPE, yyparse(), token constants

// Flex-generated scanner interface (reentrant)
typedef void* yyscan_t;
extern int  yylex_init(yyscan_t* scanner);
extern int  yylex_destroy(yyscan_t scanner);
extern void yy_scan_string(const char* str, yyscan_t scanner);

// Bison-generated parser entry point (also declared in ECSql.tab.hpp)
extern int yyparse(BisonParseResult* result, ECSqlParseContext* ctx, yyscan_t scanner);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<Exp> ECSqlBisonParser::Parse(ECDbCR ecdb, Utf8CP ecsql, IssueDataSource const& issues,
                                              ECSqlBisonParser const* parentParser)
    {
    if (Utf8String::IsNullOrEmpty(ecsql))
        return nullptr;

    // Set up parse context (reuse parent's if this is a nested parse for view expansion)
    if (parentParser != nullptr)
        m_context = parentParser->m_context;
    else
        m_context = std::make_shared<ECSqlParseContext>(ecdb, issues);

    // Initialize the Flex reentrant scanner
    yyscan_t scanner;
    if (yylex_init(&scanner) != 0)
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL,
            ECDbIssueId::ECDb_0475, "Failed to initialize ECSQL lexer");
        m_context = nullptr;
        return nullptr;
        }

    // Feed the ECSQL string to the scanner
    yy_scan_string(ecsql, scanner);

    // Run the bison parser
    BisonParseResult result;
    int parseStatus = yyparse(&result, m_context.get(), scanner);

    // Tear down the scanner
    yylex_destroy(scanner);

    if (parseStatus != 0 || result.exp == nullptr)
        {
        m_context = nullptr;
        return nullptr;
        }

    // Run the finalization pass (class resolution, parameter indexing, etc.)
    if (SUCCESS != m_context->FinalizeParsing(*result.exp))
        {
        m_context = nullptr;
        return nullptr;
        }

    m_context = nullptr;
    return std::move(result.exp);
    }
