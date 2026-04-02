/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSqlBisonParser.h"
#include "ECSqlRDParser.h"
#include "../IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! Dispatches ECSQL parsing to either the Bison or recursive-descent parser based on
//! the ECSqlConfig::GetUseBisonParser() setting on the ECDb instance.
//! When the Bison parser is active and fails to parse an expression, it automatically
//! falls back to the recursive-descent parser to maintain full coverage while the
//! Bison grammar is still being developed.
//======================================================================================
inline std::unique_ptr<Exp> ParseECSql(ECDbCR ecdb, Utf8CP ecsql, IssueDataSource const& issues)
    {
    if (ecdb.GetECSqlConfig().GetUseBisonParser())
        {
        std::unique_ptr<Exp> result;
        {
        // Suppress issues from the Bison attempt so that a fallback to the RD parser
        // does not leave spurious error messages when Bison fails on unsupported syntax.
        IssueDataSource::FilterScope ignoreScope(issues, [](auto...) { return IssueDataSource::FilterAction::Ignore; });
        ECSqlBisonParser bisonParser;
        result = bisonParser.Parse(ecdb, ecsql, issues);
        }
        if (result != nullptr)
            return result;
        // Bison parser failed; fall back to the recursive-descent parser
        }
    // Use the recursive-descent parser (either as primary when Bison is disabled, or as fallback)
    ECSqlRDParser rdParser;
    return rdParser.Parse(ecdb, ecsql, issues);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
