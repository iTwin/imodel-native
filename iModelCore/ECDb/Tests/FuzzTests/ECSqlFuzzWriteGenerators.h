/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included inside struct ECSqlFuzzGenerator (ECSqlFuzzGenerator.h).
// It contains INSERT, UPDATE, DELETE, PRAGMA, and schema import generators.

    //======================================================================
    // INSERT — single row, multi-row, subquery values
    //======================================================================
    Utf8String GenInsert()
        {
        auto const& ci = Pick(std::vector<Utf8String>{"FSub1", "FSub2", "FStandalone"});
        auto const& info = GetClassInfo(ci);

        int style = RandInt(0, 2);
        switch (style)
            {
            case 0:
                {
                // Standard single-row INSERT
                int n = RandInt(1, std::min(4, (int)info.allProps.size()));
                Utf8String cols, vals;
                for (int i = 0; i < n; i++)
                    {
                    if (i > 0) { cols.append(","); vals.append(","); }
                    cols.append(info.allProps[(size_t)i]);
                    vals.append(RandLiteral());
                    }
                Utf8String sql;
                sql.Sprintf("INSERT INTO %s(%s) VALUES(%s)", QualifiedClass(ci).c_str(), cols.c_str(), vals.c_str());
                return sql;
                }
            case 1:
                {
                // INSERT with subquery value
                auto const& other = GetClassInfo("FStandalone");
                if (!info.intProps.empty() && !other.intProps.empty())
                    {
                    Utf8String sql;
                    sql.Sprintf("INSERT INTO %s(%s) VALUES((SELECT MAX(%s) FROM %s))",
                        QualifiedClass(ci).c_str(), info.intProps[0].c_str(),
                        other.intProps[0].c_str(), QualifiedClass(other.name).c_str());
                    return sql;
                    }
                return GenInsert(); // retry as standard
                }
            default:
                {
                // INSERT with multiple value rows
                int n = RandInt(1, std::min(3, (int)info.allProps.size()));
                Utf8String cols;
                for (int i = 0; i < n; i++)
                    {
                    if (i > 0) cols.append(",");
                    cols.append(info.allProps[(size_t)i]);
                    }
                Utf8String sql;
                sql.Sprintf("INSERT INTO %s(%s) VALUES", QualifiedClass(ci).c_str(), cols.c_str());
                int rows = RandInt(2, 4);
                for (int r = 0; r < rows; r++)
                    {
                    if (r > 0) sql.append(",");
                    sql.append("(");
                    for (int i = 0; i < n; i++)
                        {
                        if (i > 0) sql.append(",");
                        sql.append(RandLiteral());
                        }
                    sql.append(")");
                    }
                return sql;
                }
            }
        }

    //======================================================================
    // UPDATE — with subquery in SET, ECSQLOPTIONS
    //======================================================================
    Utf8String GenUpdate()
        {
        auto const& ci = Pick(m_concreteClasses);
        auto const& info = GetClassInfo(ci);
        if (info.allProps.empty()) return GenInsert();

        int style = RandInt(0, 2);
        Utf8String setClauses;
        if (style == 1 && !info.intProps.empty())
            {
            // SET with subquery value
            auto const& other = GetClassInfo("FStandalone");
            if (!other.intProps.empty())
                setClauses.Sprintf("%s=(SELECT MIN(%s) FROM %s)",
                    info.intProps[0].c_str(), other.intProps[0].c_str(), QualifiedClass(other.name).c_str());
            else
                setClauses.Sprintf("%s=%s", info.allProps[0].c_str(), RandLiteral().c_str());
            }
        else
            {
            int n = RandInt(1, std::min(3, (int)info.allProps.size()));
            for (int i = 0; i < n; i++)
                {
                if (i > 0) setClauses.append(", ");
                Utf8String clause;
                clause.Sprintf("%s=%s", info.allProps[(size_t)i].c_str(), RandLiteral().c_str());
                setClauses.append(clause);
                }
            }

        Utf8String sql;
        if (Coin(25))
            sql.Sprintf("UPDATE ONLY %s SET %s", QualifiedClass(ci).c_str(), setClauses.c_str());
        else
            sql.Sprintf("UPDATE %s SET %s", QualifiedClass(ci).c_str(), setClauses.c_str());
        if (Coin(60))
            { sql.append(" WHERE "); sql.append(GenExpression(info)); }
        if (Coin(15))
            sql.append(" ECSQLOPTIONS NoECClassIdFilter");
        return sql;
        }

    //======================================================================
    // DELETE
    //======================================================================
    Utf8String GenDelete()
        {
        auto const& ci = Pick(m_concreteClasses);
        auto const& info = GetClassInfo(ci);
        Utf8String sql;
        if (Coin(25))
            sql.Sprintf("DELETE FROM ONLY %s", QualifiedClass(ci).c_str());
        else
            sql.Sprintf("DELETE FROM %s", QualifiedClass(ci).c_str());
        if (Coin(70))
            { sql.append(" WHERE "); sql.append(GenExpression(info)); }
        return sql;
        }

    //======================================================================
    // PRAGMA — extended coverage
    //======================================================================
    Utf8String GenPragma()
        {
        switch (RandInt(0, 11))
            {
            case 0: return "PRAGMA ecdb_ver";
            case 1: return "PRAGMA ecsql_ver";
            case 2:
                {
                Utf8String inner = GenSelectSimple();
                Utf8String sql;
                sql.Sprintf("PRAGMA explain_query('%s')", inner.c_str());
                return sql;
                }
            case 3: return "PRAGMA disqualify_type_index=true";
            case 4: return "PRAGMA disqualify_type_index=false";
            case 5:
                {
                Utf8String inner = GenSelectSimple();
                Utf8String sql;
                sql.Sprintf("PRAGMA parse_tree('%s')", inner.c_str());
                return sql;
                }
            case 6: return "PRAGMA integrity_check";
            case 7:
                {
                static const char* checksumTargets[] = {"ec_schema", "ec_map", "db_schema"};
                Utf8String sql;
                sql.Sprintf("PRAGMA checksum('%s')", checksumTargets[RandInt(0, 2)]);
                return sql;
                }
            case 8: return "PRAGMA experimental_features_enabled=true";
            case 9: return "PRAGMA experimental_features_enabled=false";
            case 10: return "PRAGMA db_list";
            default: return "PRAGMA ecdb_ver";
            }
        }

    //======================================================================
    // Alias-heavy queries
    //======================================================================
    Utf8String GenSelectWithAliases()
        {
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                // Column aliases with ORDER BY referencing them
                auto const& ci = RandClassInfo();
                if (ci.intProps.empty() || ci.strProps.empty()) return GenSelectSimple();
                Utf8String sql;
                sql.Sprintf("SELECT %s AS col_a, %s AS col_b FROM %s ORDER BY col_a DESC",
                    Pick(ci.intProps).c_str(), Pick(ci.strProps).c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // Table alias used throughout (WHERE, ORDER BY, GROUP BY)
                auto const& ci = RandClassInfo();
                if (ci.intProps.empty()) return GenSelectSimple();
                Utf8String prop = Pick(ci.intProps);
                Utf8String sql;
                sql.Sprintf("SELECT t.%s, t.ECInstanceId FROM %s AS t WHERE t.%s > 0 ORDER BY t.%s",
                    prop.c_str(), QualifiedClass(ci.name).c_str(), prop.c_str(), prop.c_str());
                return sql;
                }
            case 2:
                {
                // Self-join with different aliases
                Utf8String sql;
                sql.Sprintf("SELECT a.ECInstanceId, b.ECInstanceId, a.I, b.I FROM %s a, %s b WHERE a.I <> b.I",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FSub1").c_str());
                if (Coin(40))
                    sql.append(" LIMIT 20");
                return sql;
                }
            case 3:
                {
                // Multiple column aliases with GROUP BY
                auto const& ci = RandClassInfo();
                if (ci.intProps.empty() || ci.strProps.empty()) return GenSelectSimple();
                Utf8String sql;
                sql.Sprintf("SELECT %s AS grp, COUNT(*) AS cnt, SUM(%s) AS total FROM %s GROUP BY grp HAVING cnt > 0 ORDER BY total DESC",
                    Pick(ci.strProps).c_str(), Pick(ci.intProps).c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // Subquery with alias used in outer query
                Utf8String sql;
                sql.Sprintf("SELECT sub.val, sub.cnt FROM (SELECT %s AS val, COUNT(*) AS cnt FROM %s GROUP BY %s) AS sub WHERE sub.cnt > 0",
                    "I", QualifiedClass("FSub1").c_str(), "I");
                return sql;
                }
            case 5:
                {
                // Multiple table aliases in JOIN with qualified property access
                Utf8String sql;
                sql.Sprintf("SELECT a.I AS a_int, a.S AS a_str, b.Code AS b_code, b.Name AS b_name "
                    "FROM %s AS a INNER JOIN %s AS b ON a.I = b.Code WHERE a.I > 0 AND b.Name IS NOT NULL",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 6:
                {
                // CTE with alias used in join
                Utf8String sql;
                sql.Sprintf("WITH nums AS (SELECT I AS val FROM %s WHERE I > 0) "
                    "SELECT f.Name, n.val FROM %s AS f INNER JOIN nums AS n ON f.Code = n.val",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            default:
                {
                // Column alias in expression
                auto const& ci = RandClassInfo();
                if (ci.intProps.empty()) return GenSelectSimple();
                Utf8String prop = Pick(ci.intProps);
                Utf8String sql;
                sql.Sprintf("SELECT %s + 1 AS computed, %s * 2 AS doubled FROM %s ORDER BY computed",
                    prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Bracket-quoted identifiers
    //======================================================================
    Utf8String GenSelectBracketQuoted()
        {
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // Bracket-quoted schema and class
                Utf8String cls = Pick(m_concreteClasses);
                Utf8String sql;
                sql.Sprintf("SELECT ECInstanceId, ECClassId FROM [FuzzSchema].[%s]", cls.c_str());
                return sql;
                }
            case 1:
                {
                // Bracket-quoted properties
                auto const& ci = RandClassInfo();
                if (ci.allProps.empty()) return GenSelectSimple();
                Utf8String sql;
                sql.Sprintf("SELECT [%s], [ECInstanceId] FROM %s", Pick(ci.allProps).c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 2:
                {
                // Bracket-quoted alias
                Utf8String sql;
                sql.Sprintf("SELECT [t].[I], [t].[S] FROM %s AS [t] WHERE [t].[I] > 0", QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 3:
                {
                // Mixed bracket and non-bracket
                Utf8String cls = Pick(m_concreteClasses);
                Utf8String sql;
                sql.Sprintf("SELECT [ECInstanceId], S FROM [fz].[%s] WHERE [I] IS NOT NULL", cls.c_str());
                return sql;
                }
            case 4:
                {
                // Bracket-quoted in JOIN
                Utf8String sql;
                sql.Sprintf("SELECT [a].[I], [b].[Code] FROM [fz].[FSub1] AS [a] INNER JOIN [fz].[FStandalone] AS [b] ON [a].[I] = [b].[Code]");
                return sql;
                }
            default:
                {
                // Bracket-quoted relationship
                Utf8String rel = Pick(m_relationships);
                Utf8String sql;
                sql.Sprintf("SELECT [SourceECInstanceId], [TargetECInstanceId] FROM [fz].[%s]", rel.c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Subquery in various positions
    //======================================================================
    Utf8String GenSelectSubqueryPositions()
        {
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                // Subquery in SELECT list (scalar subquery)
                auto const& ci = RandClassInfo();
                auto const& other = RandClassInfo();
                Utf8String sql;
                sql.Sprintf("SELECT ECInstanceId, (SELECT COUNT(*) FROM %s) AS cnt, (SELECT MAX(%s) FROM %s) AS mx FROM %s",
                    QualifiedClass(other.name).c_str(),
                    other.intProps.empty() ? "ECInstanceId" : Pick(other.intProps).c_str(),
                    QualifiedClass(other.name).c_str(),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // Subquery in FROM (derived table) with column aliases
                Utf8String sql;
                sql.Sprintf("SELECT dt.x, dt.y FROM (SELECT I AS x, S AS y FROM %s) AS dt WHERE dt.x > 0",
                    QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 2:
                {
                // Correlated subquery in WHERE
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s AS outer_t WHERE outer_t.I > "
                    "(SELECT AVG(Code) FROM %s WHERE Code < outer_t.I)",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 3:
                {
                // Subquery in HAVING
                Utf8String sql;
                sql.Sprintf("SELECT S, COUNT(*) AS cnt FROM %s GROUP BY S HAVING COUNT(*) > (SELECT 1)",
                    QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 4:
                {
                // Nested subqueries (3 levels)
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE I IN "
                    "(SELECT Code FROM %s WHERE Code IN (SELECT I FROM %s WHERE I > 0))",
                    QualifiedClass("FSub1").c_str(),
                    QualifiedClass("FStandalone").c_str(),
                    QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 5:
                {
                // Subquery in FROM joined with real table
                Utf8String sql;
                sql.Sprintf("SELECT t.ECInstanceId, sub.max_val FROM %s t "
                    "INNER JOIN (SELECT MAX(Code) AS max_val FROM %s) AS sub ON t.I <= sub.max_val",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 6:
                {
                // EXISTS with correlated subquery referencing outer alias
                Utf8String sql;
                sql.Sprintf("SELECT a.ECInstanceId, a.S FROM %s AS a "
                    "WHERE EXISTS (SELECT 1 FROM %s AS b WHERE b.Code = a.I) AND a.I IS NOT NULL",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            default:
                {
                // NOT EXISTS
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s AS a "
                    "WHERE NOT EXISTS (SELECT 1 FROM %s AS b WHERE b.Code = a.I)",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // INSERT...SELECT and advanced write patterns
    //======================================================================
    Utf8String GenInsertSelect()
        {
        int style = RandInt(0, 3);
        switch (style)
            {
            case 0:
                {
                // Basic INSERT...SELECT
                Utf8String sql;
                sql.Sprintf("INSERT INTO %s(I, S) SELECT Code, Name FROM %s WHERE Code IS NOT NULL",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 1:
                {
                // INSERT...SELECT with expression
                Utf8String sql;
                sql.Sprintf("INSERT INTO %s(I, S) SELECT Code + 1000, COALESCE(Name, 'default') FROM %s",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 2:
                {
                // INSERT...SELECT from CTE
                Utf8String sql;
                sql.Sprintf("WITH src AS (SELECT Code AS val, Name AS label FROM %s WHERE Code > 0) "
                    "INSERT INTO %s(I, S) SELECT val, label FROM src",
                    QualifiedClass("FStandalone").c_str(), QualifiedClass("FSub1").c_str());
                return sql;
                }
            default:
                {
                // INSERT...SELECT with aggregate
                Utf8String sql;
                sql.Sprintf("INSERT INTO %s(Code) SELECT COUNT(*) FROM %s",
                    QualifiedClass("FStandalone").c_str(), QualifiedClass("FSub1").c_str());
                return sql;
                }
            }
        }
