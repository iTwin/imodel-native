/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlParser.h"
#include "Exp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! BisonParseResult — transport struct between yyparse() and the wrapper.
//! The bison grammar stores the root AST node here via %parse-param.
//======================================================================================
struct BisonParseResult
    {
    std::unique_ptr<Exp> exp;       //!< The root AST node (nullptr on error)
    Utf8String errorMessage;        //!< Error message from yyerror (if any)
    };

//======================================================================================
//! ECSqlBisonParser — drop-in replacement for ECSqlRDParser using a Bison/Flex
//! generated parser.  Same Parse() signature as ECSqlRDParser.
//!
//! Usage:
//!   ECSqlBisonParser parser;
//!   auto exp = parser.Parse(ecdb, ecsql, issues);
//!   if (!exp) { /* parse error */ }
//======================================================================================
struct ECSqlBisonParser final
    {
private:
    std::shared_ptr<ECSqlParseContext> m_context;

public:
    ECSqlBisonParser() {}
    ~ECSqlBisonParser() {}

    //! Parse an ECSQL string and return the root AST node, or nullptr on error.
    //! @param ecdb         The ECDb instance (for schema resolution during finalization)
    //! @param ecsql        Null-terminated ECSQL string
    //! @param issues       Issue/error reporting sink
    //! @param parentParser Optional parent parser for nested parsing (view expansion)
    std::unique_ptr<Exp> Parse(ECDbCR ecdb, Utf8CP ecsql, IssueDataSource const& issues,
                               ECSqlBisonParser const* parentParser = nullptr);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
