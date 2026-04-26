/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included inside struct ECSqlFuzzGenerator (ECSqlFuzzGenerator.h).
// It contains the mutation engine, boundary input generator, and deep nesting generator.

    //======================================================================
    // Mutation engine — apply targeted corruptions to valid ECSQL
    //======================================================================

    //! Apply a random mutation to the given ECSQL string
    Utf8String Mutate(Utf8StringCR ecsql)
        {
        if (ecsql.empty()) return ecsql;

        int mutation = RandInt(0, 13);
        Utf8String result(ecsql);

        switch (mutation)
            {
            case 0: return MutateTruncate(result);
            case 1: return MutateTokenDrop(result);
            case 2: return MutateTokenSwap(result);
            case 3: return MutateTokenDuplicate(result);
            case 4: return MutateIdentifierCorrupt(result);
            case 5: return MutateKeywordReplace(result);
            case 6: return MutateStringInject(result);
            case 7: return MutateDeepNesting(result);
            case 8: return MutateNullByte(result);
            case 9: return MutateUnicode(result);
            case 10: return MutateVeryLong(result);
            case 11: return MutateRepeatedClause(result);
            case 12: return MutateTypeConfusion(result);
            default: return MutateTruncate(result);
            }
        }

    //! Generate known boundary/edge-case inputs
    Utf8String GenBoundaryInput()
        {
        switch (RandInt(0, 24))
            {
            case 0: return "";
            case 1: return " ";
            case 2: return "   \t\n  ";
            case 3: return "SELECT";
            case 4: return "FROM";
            case 5: return "SELECT FROM";
            case 6: return "SELECT * FROM";
            case 7: return "SELECT *";
            case 8: return ";;;";
            case 9: return "SELECT 1, 2, 3";
            case 10:
                {
                // very long identifier chain
                Utf8String s = "SELECT ";
                for (int i = 0; i < 200; i++) s.append("a.");
                s.append("b FROM fz.FBase");
                return s;
                }
            case 11:
                {
                // deeply nested parens
                Utf8String s = "SELECT ";
                for (int i = 0; i < 50; i++) s.append("(");
                s.append("1");
                for (int i = 0; i < 50; i++) s.append(")");
                s.append(" FROM fz.FBase");
                return s;
                }
            case 12: return "INSERT INTO";
            case 13: return "UPDATE SET";
            case 14: return "DELETE FROM WHERE";
            case 15: return "WITH RECURSIVE";
            case 16: return "SELECT DISTINCT";
            case 17: return "SELECT * FROM fz.FBase WHERE";
            case 18: return "SELECT * FROM fz.FBase ORDER BY";
            case 19: return "SELECT * FROM fz.FBase GROUP BY";
            case 20: return "SELECT * FROM fz.FBase LIMIT";
            case 21: return "PRAGMA";
            case 22: return "SELECT CAST(NULL AS INTEGER) FROM fz.FBase";
            case 23: return "SELECT * FROM fz.FBase WHERE ECClassId IS ()";
            default: return "SELECT * FROM fz.FBase WHERE 1=1 AND 2=2 OR 3<>4";
            }
        }

    //! Generate deeply nested expression with varied patterns
    Utf8String GenDeepNesting()
        {
        int style = RandInt(0, 4);
        int depth = RandInt(10, 100);
        switch (style)
            {
            case 0:
                {
                // Deeply nested arithmetic
                Utf8String sql = "SELECT ";
                for (int i = 0; i < depth; i++) sql.append("(");
                sql.append("1");
                for (int i = 0; i < depth; i++) sql.append("+1)");
                sql.append(" FROM fz.FBase");
                return sql;
                }
            case 1:
                {
                // Deeply nested AND/OR conditions
                Utf8String sql = "SELECT * FROM fz.FBase WHERE ";
                for (int i = 0; i < depth; i++) sql.append("(");
                sql.append("1=1");
                for (int i = 0; i < depth; i++) { sql.append(i % 2 ? " AND 1=1)" : " OR 1=1)"); }
                return sql;
                }
            case 2:
                {
                // Nested NOT NOT NOT...
                Utf8String sql = "SELECT * FROM fz.FBase WHERE ";
                int notDepth = RandInt(5, 30);
                for (int i = 0; i < notDepth; i++) sql.append("NOT ");
                sql.append("(I > 0)");
                return sql;
                }
            case 3:
                {
                // Nested COALESCE
                Utf8String sql = "SELECT ";
                Utf8String inner = "NULL";
                int coalesceDepth = RandInt(5, 20);
                for (int i = 0; i < coalesceDepth; i++)
                    {
                    Utf8String wrap;
                    wrap.Sprintf("COALESCE(%s, NULL)", inner.c_str());
                    inner = wrap;
                    }
                sql.append(inner);
                sql.append(" FROM fz.FBase");
                return sql;
                }
            default:
                {
                // Nested subqueries (SELECT (SELECT (SELECT ...)))
                int subDepth = RandInt(3, 10);
                Utf8String inner = "1";
                for (int i = 0; i < subDepth; i++)
                    {
                    Utf8String wrap;
                    wrap.Sprintf("(SELECT %s)", inner.c_str());
                    inner = wrap;
                    }
                Utf8String sql;
                sql.Sprintf("SELECT %s FROM fz.FBase", inner.c_str());
                return sql;
                }
            }
        }

private:
    //----------------------------------------------------------------------
    // Individual mutation implementations
    //----------------------------------------------------------------------

    // Split into whitespace-delimited tokens
    static std::vector<Utf8String> Tokenize(Utf8StringCR s)
        {
        std::vector<Utf8String> tokens;
        std::istringstream iss(s.c_str());
        std::string tok;
        while (iss >> tok)
            tokens.push_back(Utf8String(tok.c_str()));
        return tokens;
        }

    static Utf8String JoinTokens(std::vector<Utf8String> const& tokens)
        {
        Utf8String s;
        for (size_t i = 0; i < tokens.size(); i++)
            {
            if (i > 0) s.append(" ");
            s.append(tokens[i]);
            }
        return s;
        }

    Utf8String MutateTruncate(Utf8StringCR s)
        {
        if (s.length() < 2) return s;
        size_t pos = (size_t)RandInt(1, (int)s.length() - 1);
        return Utf8String(s.c_str(), pos);
        }

    Utf8String MutateTokenDrop(Utf8StringCR s)
        {
        auto tokens = Tokenize(s);
        if (tokens.size() < 2) return s;
        size_t idx = (size_t)RandInt(0, (int)tokens.size() - 1);
        tokens.erase(tokens.begin() + (ptrdiff_t)idx);
        return JoinTokens(tokens);
        }

    Utf8String MutateTokenSwap(Utf8StringCR s)
        {
        auto tokens = Tokenize(s);
        if (tokens.size() < 3) return s;
        size_t idx = (size_t)RandInt(0, (int)tokens.size() - 2);
        std::swap(tokens[idx], tokens[idx + 1]);
        return JoinTokens(tokens);
        }

    Utf8String MutateTokenDuplicate(Utf8StringCR s)
        {
        auto tokens = Tokenize(s);
        if (tokens.empty()) return s;
        size_t idx = (size_t)RandInt(0, (int)tokens.size() - 1);
        tokens.insert(tokens.begin() + (ptrdiff_t)idx, tokens[idx]);
        return JoinTokens(tokens);
        }

    Utf8String MutateIdentifierCorrupt(Utf8StringCR s)
        {
        // Replace a schema-qualified name with a corrupted version
        Utf8String result(s);
        auto pos = result.find("fz.");
        if (pos != Utf8String::npos && pos + 3 < result.length())
            {
            // Corrupt the character after "fz."
            size_t charPos = pos + 3;
            if (charPos < result.length())
                result[charPos] = (char)('a' + RandInt(0, 25));
            }
        return result;
        }

    Utf8String MutateKeywordReplace(Utf8StringCR s)
        {
        static const char* keywords[] = {"SELECT","FROM","WHERE","AND","OR","INSERT","UPDATE","DELETE","SET","INTO","VALUES","ORDER","BY","GROUP","HAVING","LIMIT","JOIN","ON","AS","UNION","EXCEPT","INTERSECT","CASE","WHEN","THEN","ELSE","END","CAST","LIKE","BETWEEN","IN","IS","NOT","NULL","EXISTS","ONLY","PRAGMA","WITH","RECURSIVE","OVER","PARTITION"};
        auto tokens = Tokenize(s);
        if (tokens.empty()) return s;
        size_t idx = (size_t)RandInt(0, (int)tokens.size() - 1);
        tokens[idx] = Utf8String(keywords[RandInt(0, 40)]);
        return JoinTokens(tokens);
        }

    Utf8String MutateStringInject(Utf8StringCR s)
        {
        static const char* injections[] = {"'; DROP TABLE", "/**/", "--", "' OR '1'='1", "\"", "\\", "' UNION SELECT 1--"};
        Utf8String result(s);
        size_t pos = (size_t)RandInt(0, (int)result.length());
        result.insert(pos, injections[RandInt(0, 6)]);
        return result;
        }

    Utf8String MutateDeepNesting(Utf8StringCR s)
        {
        int depth = RandInt(20, 80);
        Utf8String prefix, suffix;
        for (int i = 0; i < depth; i++) { prefix.append("("); suffix.append(")"); }
        // Wrap the whole thing in parens — usually invalid but tests parser depth
        Utf8String result;
        result.Sprintf("SELECT %s%s%s", prefix.c_str(), s.c_str(), suffix.c_str());
        return result;
        }

    Utf8String MutateNullByte(Utf8StringCR s)
        {
        if (s.empty()) return s;
        Utf8String result(s);
        size_t pos = (size_t)RandInt(0, (int)result.length() - 1);
        result[pos] = '\0';
        return Utf8String(result.c_str()); // truncates at null
        }

    Utf8String MutateUnicode(Utf8StringCR s)
        {
        if (s.empty()) return s;
        Utf8String result(s);
        size_t pos = (size_t)RandInt(0, (int)result.length());
        // Insert a multi-byte UTF-8 sequence (é = 0xC3 0xA9)
        result.insert(pos, "\xC3\xA9");
        return result;
        }

    Utf8String MutateVeryLong(Utf8StringCR s)
        {
        // Create a very long identifier
        int len = RandInt(1000, 10000);
        Utf8String longId;
        for (int i = 0; i < len; i++) longId.append("x");
        Utf8String result;
        result.Sprintf("SELECT %s FROM fz.FBase", longId.c_str());
        return result;
        }

    Utf8String MutateRepeatedClause(Utf8StringCR s)
        {
        static const char* clauses[] = {" WHERE 1=1", " ORDER BY 1", " GROUP BY 1", " LIMIT 1", " HAVING 1=1"};
        Utf8String result(s);
        int repeats = RandInt(2, 5);
        Utf8CP clause = clauses[RandInt(0, 4)];
        for (int i = 0; i < repeats; i++)
            result.append(clause);
        return result;
        }

    Utf8String MutateTypeConfusion(Utf8StringCR s)
        {
        // Replace a numeric literal with a string or vice versa
        auto tokens = Tokenize(s);
        for (auto& tok : tokens)
            {
            // Check if token looks numeric
            if (!tok.empty() && (tok[0] == '-' || (tok[0] >= '0' && tok[0] <= '9')))
                {
                tok = Utf8String("'not_a_number'");
                break;
                }
            }
        return JoinTokens(tokens);
        }
