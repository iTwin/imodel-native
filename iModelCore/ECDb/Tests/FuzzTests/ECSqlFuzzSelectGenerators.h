/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This file is included inside struct ECSqlFuzzGenerator (ECSqlFuzzGenerator.h).
// It contains all SELECT query generators.

    //======================================================================
    // SELECT generators
    //======================================================================

    //! Generate a simple SELECT: properties, FROM, optional WHERE, ORDER BY, LIMIT
    Utf8String GenSelectSimple()
        {
        auto const& ci = RandClassInfo();
        Utf8String sql;
        sql.Sprintf("SELECT %s FROM %s", GenSelectList(ci).c_str(), GenFromClause(ci).c_str());

        if (Coin(60))
            { sql.append(" WHERE "); sql.append(GenExpression(ci)); }
        if (Coin(30) && !ci.allProps.empty())
            {
            sql.append(" ORDER BY "); sql.append(Pick(ci.allProps)); sql.append(Coin() ? " ASC" : " DESC");
            // NULLS FIRST / NULLS LAST
            if (Coin(25))
                sql.append(Coin() ? " NULLS FIRST" : " NULLS LAST");
            }
        if (Coin(20))
            { Utf8String lim; lim.Sprintf(" LIMIT %d", RandInt(1, 100)); sql.append(lim); }
        else if (Coin(10))
            { Utf8String off; off.Sprintf(" OFFSET %d", RandInt(0, 50)); sql.append(off); }
        if (Coin(15))
            { Utf8String off; off.Sprintf(" OFFSET %d", RandInt(0, 50)); sql.append(off); }

        return sql;
        }

    //! Generate a complex SELECT: JOINs, subqueries, CTEs, window functions
    Utf8String GenSelectComplex()
        {
        int feature = RandInt(0, 11);
        switch (feature)
            {
            case 0: return GenSelectWithJoin();
            case 1: return GenSelectWithSubquery();
            case 2: return GenSelectWithCTE();
            case 3: return GenSelectWithWindowFunction();
            case 4: return GenSelectWithCaseWhen();
            case 5: return GenSelectWithCast();
            case 6: return GenSelectCorrelatedSubquery();
            case 7: return GenSelectDerivedTable();
            case 8: return GenSelectMultipleCTE();
            case 9: return GenSelectMultipleJoin();
            case 10: return GenSelectAdvancedWindow();
            default: return GenSelectWithOptions();
            }
        }

    //! SELECT with JOINs
    Utf8String GenSelectWithJoin()
        {
        auto const& ci1 = GetClassInfo("FSub1");
        auto const& ci2 = GetClassInfo("FStandalone");
        static const char* joinTypes[] = {"INNER JOIN", "LEFT JOIN", "RIGHT JOIN", "CROSS JOIN"};
        Utf8String sql;
        sql.Sprintf("SELECT a.%s, b.%s FROM %s a %s %s b",
            Pick(ci1.allProps).c_str(), Pick(ci2.allProps).c_str(),
            QualifiedClass(ci1.name).c_str(),
            joinTypes[RandInt(0, 3)],
            QualifiedClass(ci2.name).c_str());
        if (RandInt(0, 3) != 3) // not for CROSS JOIN
            sql.Sprintf("%s ON a.I = b.Code", sql.c_str());
        return sql;
        }

    //! SELECT with scalar subquery or EXISTS
    Utf8String GenSelectWithSubquery()
        {
        auto const& ci = RandClassInfo();
        auto const& other = RandClassInfo();
        if (Coin())
            {
            Utf8String sql;
            sql.Sprintf("SELECT *, (SELECT COUNT(*) FROM %s) AS cnt FROM %s",
                QualifiedClass(other.name).c_str(), QualifiedClass(ci.name).c_str());
            if (Coin(60))
                { sql.append(" WHERE "); sql.append(GenExpression(ci)); }
            return sql;
            }
        else
            {
            Utf8String sql;
            sql.Sprintf("SELECT * FROM %s WHERE EXISTS (SELECT 1 FROM %s WHERE %s = %s)",
                QualifiedClass(ci.name).c_str(), QualifiedClass(other.name).c_str(),
                ci.intProps.empty() ? "1" : Pick(ci.intProps).c_str(),
                other.intProps.empty() ? "1" : Pick(other.intProps).c_str());
            return sql;
            }
        }

    //! SELECT with Common Table Expression
    Utf8String GenSelectWithCTE()
        {
        auto const& ci = RandClassInfo();
        bool recursive = Coin(30);
        if (recursive)
            {
            return Utf8String("WITH RECURSIVE cte(val) AS (SELECT 1 UNION ALL SELECT val+1 FROM cte WHERE val < 10) SELECT val FROM cte");
            }
        else
            {
            Utf8String sql;
            sql.Sprintf("WITH cte AS (SELECT %s FROM %s) SELECT * FROM cte",
                GenSelectList(ci).c_str(), QualifiedClass(ci.name).c_str());
            if (Coin(40))
                { sql.append(" LIMIT "); sql.append(RandIntLiteral()); }
            return sql;
            }
        }

    //! SELECT with window functions
    Utf8String GenSelectWithWindowFunction()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty() || ci.allProps.size() < 2) return GenSelectSimple();

        static const char* winFuncs[] = {"ROW_NUMBER()", "RANK()", "DENSE_RANK()", "NTILE(4)"};
        Utf8String orderProp = Pick(ci.allProps);
        Utf8String partProp = Pick(ci.allProps);
        Utf8String sql;
        sql.Sprintf("SELECT %s, %s OVER(PARTITION BY %s ORDER BY %s) FROM %s",
            orderProp.c_str(), winFuncs[RandInt(0, 3)], partProp.c_str(), orderProp.c_str(),
            QualifiedClass(ci.name).c_str());
        return sql;
        }

    //! SELECT with CASE/WHEN or IIF
    Utf8String GenSelectWithCaseWhen()
        {
        auto const& ci = RandClassInfo();
        if (Coin())
            {
            if (ci.intProps.empty()) return GenSelectSimple();
            Utf8String prop = Pick(ci.intProps);
            Utf8String sql;
            sql.Sprintf("SELECT CASE WHEN %s > 0 THEN 'positive' WHEN %s = 0 THEN 'zero' ELSE 'negative' END FROM %s",
                prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
            return sql;
            }
        else
            {
            if (ci.boolProps.empty()) return GenSelectSimple();
            Utf8String sql;
            sql.Sprintf("SELECT IIF(%s, 'yes', 'no') FROM %s",
                Pick(ci.boolProps).c_str(), QualifiedClass(ci.name).c_str());
            return sql;
            }
        }

    //! SELECT with CAST
    Utf8String GenSelectWithCast()
        {
        auto const& ci = RandClassInfo();
        static const char* types[] = {"INTEGER", "REAL", "TEXT", "BLOB", "int", "long", "double", "string", "boolean"};
        Utf8String prop = Pick(ci.allProps);
        Utf8String sql;
        sql.Sprintf("SELECT CAST(%s AS %s) FROM %s", prop.c_str(), types[RandInt(0, 8)], QualifiedClass(ci.name).c_str());
        return sql;
        }

    //! SELECT with correlated subquery (references outer table)
    Utf8String GenSelectCorrelatedSubquery()
        {
        auto const& ci = GetClassInfo("FSub1");
        auto const& other = GetClassInfo("FStandalone");
        int style = RandInt(0, 2);
        switch (style)
            {
            case 0:
                {
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s AS outer_t WHERE I > (SELECT COUNT(*) FROM %s WHERE Code = outer_t.I)",
                    QualifiedClass(ci.name).c_str(), QualifiedClass(other.name).c_str());
                return sql;
                }
            case 1:
                {
                Utf8String sql;
                sql.Sprintf("SELECT *, (SELECT MAX(Code) FROM %s WHERE Code < outer_t.I) AS mx FROM %s AS outer_t",
                    QualifiedClass(other.name).c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            default:
                {
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s AS outer_t WHERE EXISTS (SELECT 1 FROM %s WHERE Code = outer_t.I AND Name IS NOT NULL)",
                    QualifiedClass(ci.name).c_str(), QualifiedClass(other.name).c_str());
                return sql;
                }
            }
        }

    //! SELECT from derived table (subquery in FROM)
    Utf8String GenSelectDerivedTable()
        {
        auto const& ci = RandClassInfo();
        Utf8String innerSelect;
        if (!ci.intProps.empty() && !ci.strProps.empty())
            innerSelect.Sprintf("SELECT %s, %s FROM %s", Pick(ci.intProps).c_str(), Pick(ci.strProps).c_str(), QualifiedClass(ci.name).c_str());
        else
            innerSelect.Sprintf("SELECT * FROM %s", QualifiedClass(ci.name).c_str());

        Utf8String sql;
        sql.Sprintf("SELECT * FROM (%s) AS derived_t", innerSelect.c_str());
        if (Coin(40))
            { Utf8String lim; lim.Sprintf(" LIMIT %d", RandInt(1, 50)); sql.append(lim); }
        return sql;
        }

    //! SELECT with multiple CTEs
    Utf8String GenSelectMultipleCTE()
        {
        auto const& ci1 = GetClassInfo("FSub1");
        auto const& ci2 = GetClassInfo("FStandalone");
        int style = RandInt(0, 2);
        switch (style)
            {
            case 0:
                {
                // Two independent CTEs joined
                Utf8String sql;
                sql.Sprintf("WITH cte1 AS (SELECT I, S FROM %s), cte2 AS (SELECT Code, Name FROM %s) "
                    "SELECT cte1.I, cte2.Name FROM cte1, cte2",
                    QualifiedClass(ci1.name).c_str(), QualifiedClass(ci2.name).c_str());
                if (Coin(50))
                    sql.append(" LIMIT 10");
                return sql;
                }
            case 1:
                {
                // CTE referencing another CTE
                Utf8String sql;
                sql.Sprintf("WITH cte1 AS (SELECT I, S FROM %s), cte2 AS (SELECT * FROM cte1 WHERE I > 0) "
                    "SELECT * FROM cte2",
                    QualifiedClass(ci1.name).c_str());
                return sql;
                }
            default:
                {
                // Three CTEs
                Utf8String sql;
                sql.Sprintf("WITH a AS (SELECT I FROM %s), b AS (SELECT Code FROM %s), c AS (SELECT * FROM a UNION ALL SELECT * FROM b) "
                    "SELECT * FROM c",
                    QualifiedClass(ci1.name).c_str(), QualifiedClass(ci2.name).c_str());
                return sql;
                }
            }
        }

    //! SELECT with multiple chained JOINs
    Utf8String GenSelectMultipleJoin()
        {
        static const char* joinTypes[] = {"INNER JOIN", "LEFT JOIN", "RIGHT JOIN"};
        Utf8String sql;
        sql.Sprintf("SELECT a.I, b.Code, c.Sub2Code FROM %s a %s %s b ON a.I = b.Code %s %s c ON a.I = c.I",
            QualifiedClass("FSub1").c_str(),
            joinTypes[RandInt(0, 2)],
            QualifiedClass("FStandalone").c_str(),
            joinTypes[RandInt(0, 2)],
            QualifiedClass("FSub2").c_str());
        if (Coin(40))
            { sql.append(" WHERE "); sql.append("a.I > 0"); }
        return sql;
        }

    //! SELECT with advanced window function (frames, lead/lag)
    Utf8String GenSelectAdvancedWindow()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty()) return GenSelectSimple();
        Utf8String prop = Pick(ci.intProps);
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // Window with frame spec ROWS BETWEEN
                Utf8String sql;
                sql.Sprintf("SELECT %s, SUM(%s) OVER(ORDER BY %s ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING) FROM %s",
                    prop.c_str(), prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // Window with RANGE UNBOUNDED PRECEDING
                Utf8String sql;
                sql.Sprintf("SELECT %s, AVG(%s) OVER(ORDER BY %s RANGE UNBOUNDED PRECEDING) FROM %s",
                    prop.c_str(), prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 2:
                {
                // LAG / LEAD
                static const char* funcs[] = {"LAG", "LEAD"};
                Utf8String sql;
                sql.Sprintf("SELECT %s, %s(%s) OVER(ORDER BY %s), %s(%s, 2) OVER(ORDER BY %s) FROM %s",
                    prop.c_str(), funcs[0], prop.c_str(), prop.c_str(),
                    funcs[1], prop.c_str(), prop.c_str(),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                // FIRST_VALUE / LAST_VALUE
                static const char* funcs[] = {"FIRST_VALUE", "LAST_VALUE"};
                Utf8String sql;
                sql.Sprintf("SELECT %s, %s(%s) OVER(ORDER BY %s ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) FROM %s",
                    prop.c_str(), funcs[RandInt(0, 1)], prop.c_str(), prop.c_str(),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // NTH_VALUE
                Utf8String sql;
                sql.Sprintf("SELECT %s, NTH_VALUE(%s, 2) OVER(ORDER BY %s) FROM %s",
                    prop.c_str(), prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            default:
                {
                // Multiple window functions + PARTITION BY
                Utf8String partProp = Pick(ci.allProps);
                Utf8String sql;
                sql.Sprintf("SELECT %s, ROW_NUMBER() OVER(PARTITION BY %s ORDER BY %s), SUM(%s) OVER(PARTITION BY %s ORDER BY %s ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING) FROM %s",
                    prop.c_str(), partProp.c_str(), prop.c_str(),
                    prop.c_str(), partProp.c_str(), prop.c_str(),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            }
        }

    //! SELECT with ECSQLOPTIONS clause
    Utf8String GenSelectWithOptions()
        {
        auto const& ci = RandClassInfo();
        Utf8String sql;
        sql.Sprintf("SELECT %s FROM %s", GenSelectList(ci).c_str(), QualifiedClass(ci.name).c_str());
        if (Coin(60))
            { sql.append(" WHERE "); sql.append(GenExpression(ci)); }
        // Append ECSQLOPTIONS
        sql.append(" ECSQLOPTIONS NoECClassIdFilter");
        return sql;
        }

    //======================================================================
    // Relationship / struct / navigation property queries
    //======================================================================
    Utf8String GenSelectRelationship()
        {
        int style = RandInt(0, 9);
        switch (style)
            {
            case 0: // direct relationship query
                {
                Utf8String rel = Pick(m_relationships);
                Utf8String sql;
                sql.Sprintf("SELECT ECInstanceId, SourceECInstanceId, TargetECInstanceId FROM %s.%s", s_alias, rel.c_str());
                return sql;
                }
            case 1: // N:N relationship with properties
                return Utf8String("SELECT RefD, RefI, RefS, SourceECInstanceId, TargetECInstanceId FROM fz.FStandaloneRefsFBase");
            case 2: // navigation property access with RelECClassId
                {
                if (Coin())
                    return Utf8String("SELECT Parent.Id, Parent.RelECClassId, Sub1Name FROM fz.FSub1");
                else
                    return Utf8String("SELECT Owner.Id, Owner.RelECClassId, Name FROM fz.FStandalone");
                }
            case 3: // struct property deep access
                {
                auto const& ci = GetClassInfo("FSub1");
                if (!ci.structProps.empty())
                    {
                    Utf8String sql;
                    sql.Sprintf("SELECT %s FROM %s", Pick(ci.structProps).c_str(), QualifiedClass(ci.name).c_str());
                    return sql;
                    }
                return GenSelectSimple();
                }
            case 4: // ECClassId IS filter
                {
                Utf8String cls = Pick(m_concreteClasses);
                Utf8String sql;
                sql.Sprintf("SELECT * FROM fz.FBase WHERE ECClassId IS (ONLY %s.%s)", s_alias, cls.c_str());
                return sql;
                }
            case 5: // ECClassId IS NOT filter
                {
                Utf8String cls = Pick(m_concreteClasses);
                Utf8String sql;
                sql.Sprintf("SELECT * FROM fz.FBase WHERE ECClassId IS NOT (ONLY %s.%s)", s_alias, cls.c_str());
                return sql;
                }
            case 6: // Point property component access (P2D.X, P2D.Y, P3D.Z)
                {
                static const char* pointExprs[] = {
                    "SELECT P2D.X, P2D.Y FROM fz.FSub1",
                    "SELECT P3D.X, P3D.Y, P3D.Z FROM fz.FSub1",
                    "SELECT Pt2.X, Pt2.Y FROM fz.FStandalone",
                    "SELECT Pt3.X, Pt3.Y, Pt3.Z FROM fz.FStandalone",
                    "SELECT * FROM fz.FSub1 WHERE P2D.X > 0",
                    "SELECT * FROM fz.FStandalone WHERE Pt3.Z IS NOT NULL"
                    };
                return Utf8String(pointExprs[RandInt(0, 5)]);
                }
            case 7: // Array property access with indexing
                {
                static const char* arrayExprs[] = {
                    "SELECT IntArray FROM fz.FSub2",
                    "SELECT StrArray FROM fz.FSub2",
                    "SELECT StructArray FROM fz.FSub2",
                    "SELECT NestedStructArray FROM fz.FSub2",
                    "SELECT * FROM fz.FSub2 WHERE IntArray IS NOT NULL",
                    "SELECT BoundedArray FROM fz.FSub2"
                    };
                return Utf8String(arrayExprs[RandInt(0, 5)]);
                }
            case 8: // ECInstanceId direct use
                {
                Utf8String cls = Pick(m_concreteClasses);
                Utf8String sql;
                sql.Sprintf("SELECT ECInstanceId, ECClassId FROM %s", QualifiedClass(cls).c_str());
                if (Coin(50))
                    sql.Sprintf("%s WHERE ECInstanceId > 0", sql.c_str());
                return sql;
                }
            default: // Mixin query
                return Utf8String("SELECT MixinLabel, I, S FROM fz.FMixin");
            }
        }

    //======================================================================
    // Set operations
    //======================================================================
    Utf8String GenSelectSetOperation()
        {
        auto const& ci1 = RandClassInfo();
        auto const& ci2 = RandClassInfo();
        static const char* ops[] = {"UNION", "UNION ALL", "INTERSECT", "EXCEPT"};
        Utf8String sel1, sel2;
        // Use compatible columns (both integer)
        if (!ci1.intProps.empty() && !ci2.intProps.empty())
            {
            sel1.Sprintf("SELECT %s FROM %s", Pick(ci1.intProps).c_str(), QualifiedClass(ci1.name).c_str());
            sel2.Sprintf("SELECT %s FROM %s", Pick(ci2.intProps).c_str(), QualifiedClass(ci2.name).c_str());
            }
        else
            {
            sel1.Sprintf("SELECT COUNT(*) FROM %s", QualifiedClass(ci1.name).c_str());
            sel2.Sprintf("SELECT COUNT(*) FROM %s", QualifiedClass(ci2.name).c_str());
            }
        Utf8String sql;
        sql.Sprintf("%s %s %s", sel1.c_str(), ops[RandInt(0, 3)], sel2.c_str());
        return sql;
        }

    //======================================================================
    // Aggregates with GROUP BY / HAVING
    //======================================================================
    Utf8String GenSelectAggregate()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty() || ci.strProps.empty()) return GenSelectSimple();

        Utf8String groupProp = ci.strProps.empty() ? Pick(ci.allProps) : Pick(ci.strProps);
        Utf8String aggProp = Pick(ci.intProps);
        static const char* aggs[] = {"COUNT", "SUM", "AVG", "MIN", "MAX", "GROUP_CONCAT"};
        Utf8CP agg = aggs[RandInt(0, 5)];

        int style = RandInt(0, 3);
        switch (style)
            {
            case 0:
                {
                // Standard GROUP BY + HAVING + ORDER BY
                Utf8String sql;
                sql.Sprintf("SELECT %s, %s(%s) FROM %s GROUP BY %s",
                    groupProp.c_str(), agg, aggProp.c_str(), QualifiedClass(ci.name).c_str(), groupProp.c_str());
                if (Coin(40))
                    sql.Sprintf("%s HAVING %s(%s) > %d", sql.c_str(), agg, aggProp.c_str(), RandInt(0, 5));
                if (Coin(30))
                    sql.Sprintf("%s ORDER BY %s(%s) DESC", sql.c_str(), agg, aggProp.c_str());
                return sql;
                }
            case 1:
                {
                // HAVING without GROUP BY (implicit scalar grouping)
                Utf8String sql;
                sql.Sprintf("SELECT COUNT(*) FROM %s HAVING COUNT(*) > %d",
                    QualifiedClass(ci.name).c_str(), RandInt(0, 5));
                return sql;
                }
            case 2:
                {
                // GROUP BY with expressions
                Utf8String sql;
                sql.Sprintf("SELECT %s + 1, %s(%s) FROM %s GROUP BY %s + 1 ORDER BY %s(%s) DESC",
                    aggProp.c_str(), agg, aggProp.c_str(),
                    QualifiedClass(ci.name).c_str(),
                    aggProp.c_str(), agg, aggProp.c_str());
                return sql;
                }
            default:
                {
                // FILTER clause on aggregate
                Utf8String sql;
                sql.Sprintf("SELECT %s, COUNT(*) FILTER(WHERE %s > 0), SUM(%s) FILTER(WHERE %s IS NOT NULL) FROM %s GROUP BY %s",
                    groupProp.c_str(), aggProp.c_str(), aggProp.c_str(), aggProp.c_str(),
                    QualifiedClass(ci.name).c_str(), groupProp.c_str());
                return sql;
                }
            }
        }
    //======================================================================
    // NATURAL / FULL OUTER / USING JOINs
    //======================================================================
    Utf8String GenSelectNaturalJoin()
        {
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                return Utf8String("SELECT * FROM fz.FSub1 NATURAL JOIN fz.FStandalone");
            case 1:
                return Utf8String("SELECT * FROM fz.FSub1 NATURAL LEFT JOIN fz.FStandalone");
            case 2:
                return Utf8String("SELECT * FROM fz.FSub1 NATURAL RIGHT JOIN fz.FStandalone");
            case 3:
                return Utf8String("SELECT a.I, b.Code FROM fz.FSub1 a FULL OUTER JOIN fz.FStandalone b ON a.I = b.Code");
            case 4:
                return Utf8String("SELECT a.I, b.Code FROM fz.FSub1 a FULL JOIN fz.FStandalone b ON a.I = b.Code WHERE a.I IS NOT NULL OR b.Code IS NOT NULL");
            case 5:
                {
                // JOIN ... USING (ECSQL relationship navigation)
                Utf8String rel = Pick(m_relationships);
                static const char* dirs[] = {"FORWARD", "BACKWARD"};
                Utf8String sql;
                sql.Sprintf("SELECT * FROM fz.FBase JOIN fz.FStandalone USING %s.%s %s",
                    s_alias, rel.c_str(), dirs[RandInt(0, 1)]);
                return sql;
                }
            case 6:
                {
                // NATURAL JOIN with WHERE
                auto const& ci = GetClassInfo("FSub1");
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s NATURAL JOIN %s WHERE %s",
                    QualifiedClass("FSub1").c_str(), QualifiedClass("FStandalone").c_str(),
                    GenExpression(ci).c_str());
                return sql;
                }
            default:
                {
                // Multiple JOINs mixing types
                return Utf8String("SELECT a.I, b.Code, c.Sub2Code FROM fz.FSub1 a "
                    "INNER JOIN fz.FStandalone b ON a.I = b.Code "
                    "FULL OUTER JOIN fz.FSub2 c ON a.I = c.Sub2Code");
                }
            }
        }

    //======================================================================
    // Bitwise operators
    //======================================================================
    Utf8String GenSelectBitwise()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty()) return GenSelectSimple();
        Utf8String iProp = Pick(ci.intProps);
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s & %d FROM %s", iProp.c_str(), RandInt(0, 255), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s | %d FROM %s", iProp.c_str(), RandInt(0, 255), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 2:
                {
                Utf8String sql;
                sql.Sprintf("SELECT ~%s FROM %s", iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s << %d FROM %s", iProp.c_str(), RandInt(1, 8), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s >> %d FROM %s", iProp.c_str(), RandInt(1, 8), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 5:
                {
                // Mixed bitwise + comparison
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE (%s & %d) = %d",
                    QualifiedClass(ci.name).c_str(), iProp.c_str(), RandInt(1, 255), RandInt(0, 255));
                return sql;
                }
            case 6:
                {
                // Complex bitwise expression
                Utf8String sql;
                sql.Sprintf("SELECT (%s | %d) & (~%s >> 1) FROM %s",
                    iProp.c_str(), RandInt(0, 255), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            default:
                {
                // Bitwise in WHERE with ORDER BY
                Utf8String sql;
                sql.Sprintf("SELECT %s, %s & 0xFF AS masked FROM %s WHERE (%s & %d) > 0 ORDER BY masked",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str(), iProp.c_str(), RandInt(1, 255));
                return sql;
                }
            }
        }

    //======================================================================
    // COLLATE clause
    //======================================================================
    Utf8String GenSelectCollate()
        {
        auto const& ci = RandClassInfo();
        if (ci.strProps.empty()) return GenSelectSimple();
        Utf8String sProp = Pick(ci.strProps);
        static const char* collations[] = {"NOCASE", "RTRIM", "BINARY"};
        Utf8CP col = collations[RandInt(0, 2)];
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s FROM %s ORDER BY %s COLLATE %s",
                    sProp.c_str(), QualifiedClass(ci.name).c_str(), sProp.c_str(), col);
                return sql;
                }
            case 1:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s FROM %s WHERE %s COLLATE %s = 'alpha'",
                    sProp.c_str(), QualifiedClass(ci.name).c_str(), sProp.c_str(), col);
                return sql;
                }
            case 2:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s FROM %s WHERE %s LIKE '%%test%%' COLLATE %s",
                    sProp.c_str(), QualifiedClass(ci.name).c_str(), sProp.c_str(), col);
                return sql;
                }
            case 3:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s FROM %s GROUP BY %s COLLATE %s",
                    sProp.c_str(), QualifiedClass(ci.name).c_str(), sProp.c_str(), col);
                return sql;
                }
            case 4:
                {
                Utf8String sql;
                sql.Sprintf("SELECT DISTINCT %s COLLATE %s FROM %s ORDER BY %s COLLATE %s ASC",
                    sProp.c_str(), col, QualifiedClass(ci.name).c_str(), sProp.c_str(), col);
                return sql;
                }
            default:
                {
                Utf8String sql;
                sql.Sprintf("SELECT %s FROM %s ORDER BY %s COLLATE %s ASC NULLS LAST",
                    sProp.c_str(), QualifiedClass(ci.name).c_str(), sProp.c_str(), col);
                return sql;
                }
            }
        }

    //======================================================================
    // Advanced window frames
    //======================================================================
    Utf8String GenSelectAdvancedWindowFrame()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty() || ci.allProps.size() < 2) return GenSelectSimple();
        Utf8String iProp = Pick(ci.intProps);
        Utf8String partProp = Pick(ci.allProps);
        int style = RandInt(0, 9);
        switch (style)
            {
            case 0:
                {
                // ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW
                Utf8String sql;
                sql.Sprintf("SELECT %s, SUM(%s) OVER(ORDER BY %s ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // ROWS BETWEEN N PRECEDING AND N FOLLOWING
                Utf8String sql;
                sql.Sprintf("SELECT %s, AVG(%s) OVER(ORDER BY %s ROWS BETWEEN %d PRECEDING AND %d FOLLOWING) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), RandInt(1, 5), RandInt(1, 5),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 2:
                {
                // RANGE BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING
                Utf8String sql;
                sql.Sprintf("SELECT %s, COUNT(*) OVER(ORDER BY %s RANGE BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) FROM %s",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                // ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING
                Utf8String sql;
                sql.Sprintf("SELECT %s, MIN(%s) OVER(ORDER BY %s ROWS BETWEEN CURRENT ROW AND UNBOUNDED FOLLOWING) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // EXCLUDE CURRENT ROW
                Utf8String sql;
                sql.Sprintf("SELECT %s, SUM(%s) OVER(ORDER BY %s ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE CURRENT ROW) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 5:
                {
                // EXCLUDE GROUP
                Utf8String sql;
                sql.Sprintf("SELECT %s, SUM(%s) OVER(PARTITION BY %s ORDER BY %s ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE GROUP) FROM %s",
                    iProp.c_str(), iProp.c_str(), partProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 6:
                {
                // EXCLUDE TIES
                Utf8String sql;
                sql.Sprintf("SELECT %s, MAX(%s) OVER(ORDER BY %s ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE TIES) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 7:
                {
                // EXCLUDE NO OTHERS
                Utf8String sql;
                sql.Sprintf("SELECT %s, SUM(%s) OVER(ORDER BY %s RANGE UNBOUNDED PRECEDING EXCLUDE NO OTHERS) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 8:
                {
                // Multiple window functions with different frames
                Utf8String sql;
                sql.Sprintf("SELECT %s, "
                    "SUM(%s) OVER(ORDER BY %s ROWS BETWEEN 2 PRECEDING AND 2 FOLLOWING) AS rolling_sum, "
                    "AVG(%s) OVER(ORDER BY %s ROWS UNBOUNDED PRECEDING) AS running_avg, "
                    "ROW_NUMBER() OVER(ORDER BY %s) AS rn "
                    "FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(),
                    iProp.c_str(), iProp.c_str(), iProp.c_str(),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            default:
                {
                // PARTITION BY + frame + EXCLUDE
                Utf8String sql;
                sql.Sprintf("SELECT %s, FIRST_VALUE(%s) OVER(PARTITION BY %s ORDER BY %s "
                    "ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING EXCLUDE TIES) FROM %s",
                    iProp.c_str(), iProp.c_str(), partProp.c_str(), iProp.c_str(),
                    QualifiedClass(ci.name).c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Advanced CASE expressions
    //======================================================================
    Utf8String GenSelectCaseAdvanced()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty()) return GenSelectSimple();
        Utf8String iProp = Pick(ci.intProps);
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                // Nested CASE
                Utf8String sql;
                sql.Sprintf("SELECT CASE WHEN %s > 0 THEN CASE WHEN %s > 100 THEN 'big' ELSE 'small' END ELSE 'zero' END FROM %s",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // CASE in WHERE
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE CASE WHEN %s > 0 THEN 1 ELSE 0 END = 1",
                    QualifiedClass(ci.name).c_str(), iProp.c_str());
                return sql;
                }
            case 2:
                {
                // CASE in GROUP BY
                Utf8String sql;
                sql.Sprintf("SELECT CASE WHEN %s > 0 THEN 'pos' ELSE 'neg' END AS grp, COUNT(*) FROM %s GROUP BY grp",
                    iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                // CASE in ORDER BY
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s ORDER BY CASE WHEN %s IS NULL THEN 1 ELSE 0 END, %s ASC",
                    QualifiedClass(ci.name).c_str(), iProp.c_str(), iProp.c_str());
                return sql;
                }
            case 4:
                {
                // CASE in aggregate
                Utf8String sql;
                sql.Sprintf("SELECT SUM(CASE WHEN %s > 0 THEN %s ELSE 0 END) FROM %s",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 5:
                {
                // Simple CASE (value-based)
                Utf8String sql;
                sql.Sprintf("SELECT CASE %s WHEN 1 THEN 'one' WHEN 2 THEN 'two' WHEN 3 THEN 'three' ELSE 'other' END FROM %s",
                    iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 6:
                {
                // CASE with type mismatch
                Utf8String sql;
                sql.Sprintf("SELECT CASE WHEN %s > 0 THEN %s WHEN %s = 0 THEN 'zero_str' ELSE NULL END FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            default:
                {
                // CASE in HAVING
                Utf8String sql;
                sql.Sprintf("SELECT %s, COUNT(*) FROM %s GROUP BY %s HAVING SUM(CASE WHEN %s > 0 THEN 1 ELSE 0 END) > 0",
                    iProp.c_str(), QualifiedClass(ci.name).c_str(), iProp.c_str(), iProp.c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Advanced recursive CTEs
    //======================================================================
    Utf8String GenSelectRecursiveCTE()
        {
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // Fibonacci-like
                return Utf8String("WITH RECURSIVE fib(a, b) AS (SELECT 0, 1 UNION ALL SELECT b, a + b FROM fib WHERE a < 100) SELECT a FROM fib");
                }
            case 1:
                {
                // Power of 2
                return Utf8String("WITH RECURSIVE pow2(val) AS (SELECT 1 UNION ALL SELECT val * 2 FROM pow2 WHERE val < 1024) SELECT val FROM pow2");
                }
            case 2:
                {
                // Recursive CTE joined with real table
                Utf8String sql;
                sql.Sprintf("WITH RECURSIVE seq(n) AS (SELECT 1 UNION ALL SELECT n + 1 FROM seq WHERE n < 5) "
                    "SELECT s.n, f.I FROM seq s LEFT JOIN %s f ON s.n = f.I", QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 3:
                {
                // Recursive CTE with aggregation in outer query
                return Utf8String("WITH RECURSIVE cnt(x) AS (SELECT 1 UNION ALL SELECT x + 1 FROM cnt WHERE x < 20) "
                    "SELECT SUM(x), AVG(x), MAX(x), MIN(x) FROM cnt");
                }
            case 4:
                {
                // Multiple CTEs where one is recursive
                Utf8String sql;
                sql.Sprintf("WITH base AS (SELECT I, S FROM %s), "
                    "RECURSIVE seq(n) AS (SELECT 1 UNION ALL SELECT n + 1 FROM seq WHERE n < 10) "
                    "SELECT b.I, s.n FROM base b, seq s WHERE b.I = s.n",
                    QualifiedClass("FSub1").c_str());
                return sql;
                }
            default:
                {
                // Recursive CTE with variable depth
                int depth = RandInt(5, 50);
                Utf8String sql;
                sql.Sprintf("WITH RECURSIVE cte(val, lvl) AS (SELECT 1, 0 UNION ALL SELECT val + 1, lvl + 1 FROM cte WHERE lvl < %d) SELECT val, lvl FROM cte", depth);
                return sql;
                }
            }
        }

    //======================================================================
    // Advanced type predicates (ECClassId IS with type lists)
    //======================================================================
    Utf8String GenSelectTypePredicate()
        {
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                return Utf8String("SELECT * FROM fz.FBase WHERE ECClassId IS (fz.FSub1)");
            case 1:
                return Utf8String("SELECT * FROM fz.FBase WHERE ECClassId IS (fz.FSub1, fz.FSub2)");
            case 2:
                return Utf8String("SELECT * FROM fz.FBase WHERE ECClassId IS NOT (fz.FSub3)");
            case 3:
                return Utf8String("SELECT * FROM fz.FBase WHERE ECClassId IS (ONLY fz.FSub1)");
            case 4:
                return Utf8String("SELECT * FROM fz.FBase WHERE ECClassId IS (fz.FSub1, ONLY fz.FSub2, fz.FSub3)");
            case 5:
                {
                // Type predicate in JOIN
                return Utf8String("SELECT a.I, b.Code FROM fz.FBase a INNER JOIN fz.FStandalone b ON a.I = b.Code WHERE a.ECClassId IS (fz.FSub1, fz.FSub2)");
                }
            case 6:
                {
                // Type predicate with other WHERE conditions
                auto const& ci = GetClassInfo("FBase");
                Utf8String sql;
                sql.Sprintf("SELECT * FROM fz.FBase WHERE ECClassId IS (fz.FSub1) AND %s", GenExpression(ci).c_str());
                return sql;
                }
            default:
                {
                // Type predicate in subquery
                return Utf8String("SELECT * FROM fz.FBase WHERE ECInstanceId IN (SELECT ECInstanceId FROM fz.FBase WHERE ECClassId IS (fz.FSub1))");
                }
            }
        }

    //======================================================================
    // Property path access (deep struct, array subscript, wildcards)
    //======================================================================
    Utf8String GenSelectPropertyPath()
        {
        int style = RandInt(0, 9);
        switch (style)
            {
            case 0:
                return Utf8String("SELECT SimpleStruct.sb, SimpleStruct.si, SimpleStruct.sl, SimpleStruct.sd, SimpleStruct.ss FROM fz.FSub1");
            case 1:
                return Utf8String("SELECT DeepStruct.ds, DeepStruct.nested.ns, DeepStruct.nested.inner.si, DeepStruct.nested.inner.sd FROM fz.FSub1");
            case 2:
                return Utf8String("SELECT Details.sb, Details.si, Details.sd, Details.ss, Details.sdt FROM fz.FStandalone");
            case 3:
                return Utf8String("SELECT * FROM fz.FSub1 WHERE DeepStruct.nested.inner.si > 0");
            case 4:
                return Utf8String("SELECT * FROM fz.FSub1 ORDER BY DeepStruct.nested.inner.sd ASC NULLS LAST");
            case 5:
                return Utf8String("SELECT IntArray, StrArray, DblArray FROM fz.FSub2");
            case 6:
                return Utf8String("SELECT StructArray, NestedStructArray FROM fz.FSub2");
            case 7:
                {
                // Struct property in GROUP BY
                return Utf8String("SELECT SimpleStruct.sb, COUNT(*) FROM fz.FSub1 GROUP BY SimpleStruct.sb");
                }
            case 8:
                {
                // Struct property in HAVING
                return Utf8String("SELECT SimpleStruct.si, COUNT(*) FROM fz.FSub1 GROUP BY SimpleStruct.si HAVING SimpleStruct.si > 0");
                }
            default:
                {
                // Deep struct in CASE
                return Utf8String("SELECT CASE WHEN DeepStruct.nested.inner.si > 0 THEN 'pos' ELSE 'neg' END FROM fz.FSub1");
                }
            }
        }

    //======================================================================
    // Advanced aggregation patterns
    //======================================================================
    Utf8String GenSelectAdvancedAggregate()
        {
        auto const& ci = RandClassInfo();
        if (ci.intProps.empty() || ci.strProps.empty()) return GenSelectSimple();
        Utf8String iProp = Pick(ci.intProps);
        Utf8String sProp = Pick(ci.strProps);
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                // Multiple DISTINCT aggregates
                Utf8String sql;
                sql.Sprintf("SELECT COUNT(DISTINCT %s), COUNT(DISTINCT %s) FROM %s",
                    iProp.c_str(), sProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // GROUP_CONCAT with separator
                Utf8String sql;
                sql.Sprintf("SELECT GROUP_CONCAT(%s, ';') FROM %s GROUP BY %s",
                    sProp.c_str(), QualifiedClass(ci.name).c_str(), iProp.c_str());
                return sql;
                }
            case 2:
                {
                // Aggregate over expression
                Utf8String sql;
                sql.Sprintf("SELECT SUM(%s * 2 + 1), AVG(%s / 10.0) FROM %s",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                // Multiple FILTER clauses
                Utf8String sql;
                sql.Sprintf("SELECT COUNT(*) FILTER(WHERE %s > 0), COUNT(*) FILTER(WHERE %s < 0), SUM(%s) FILTER(WHERE %s IS NOT NULL) FROM %s",
                    iProp.c_str(), iProp.c_str(), iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // Aggregate in HAVING with complex condition
                Utf8String sql;
                sql.Sprintf("SELECT %s, COUNT(*), AVG(%s) FROM %s GROUP BY %s HAVING COUNT(*) > 1 AND AVG(%s) > 0",
                    sProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str(), sProp.c_str(), iProp.c_str());
                return sql;
                }
            case 5:
                {
                // Nested aggregate (aggregate of CASE)
                Utf8String sql;
                sql.Sprintf("SELECT SUM(CASE WHEN %s > 0 THEN 1 ELSE 0 END) AS pos_count, "
                    "SUM(CASE WHEN %s <= 0 THEN 1 ELSE 0 END) AS neg_count FROM %s",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 6:
                {
                // Aggregate with COALESCE
                Utf8String sql;
                sql.Sprintf("SELECT COALESCE(SUM(%s), 0), COALESCE(AVG(%s), 0.0) FROM %s WHERE %s IS NOT NULL",
                    iProp.c_str(), iProp.c_str(), QualifiedClass(ci.name).c_str(), iProp.c_str());
                return sql;
                }
            default:
                {
                // GROUP BY with multiple columns + ORDER BY aggregate
                Utf8String sql;
                sql.Sprintf("SELECT %s, %s, COUNT(*), MIN(%s), MAX(%s) FROM %s GROUP BY %s, %s ORDER BY COUNT(*) DESC, MAX(%s) ASC",
                    sProp.c_str(), iProp.c_str(), iProp.c_str(), iProp.c_str(),
                    QualifiedClass(ci.name).c_str(), sProp.c_str(), iProp.c_str(), iProp.c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Chained set operations
    //======================================================================
    Utf8String GenSelectChainedSetOps()
        {
        auto const& ci1 = RandClassInfo();
        auto const& ci2 = RandClassInfo();
        auto const& ci3 = RandClassInfo();
        static const char* ops[] = {"UNION", "UNION ALL", "INTERSECT", "EXCEPT"};
        int style = RandInt(0, 3);
        switch (style)
            {
            case 0:
                {
                // Triple UNION
                Utf8String sql;
                sql.Sprintf("SELECT I FROM %s %s SELECT I FROM %s %s SELECT I FROM %s",
                    QualifiedClass(ci1.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci2.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci3.name).c_str());
                return sql;
                }
            case 1:
                {
                // Set op with ORDER BY on result
                Utf8String sql;
                sql.Sprintf("SELECT I FROM %s %s SELECT I FROM %s ORDER BY I ASC LIMIT %d",
                    QualifiedClass(ci1.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci2.name).c_str(), RandInt(1, 20));
                return sql;
                }
            case 2:
                {
                // Parenthesized set ops
                Utf8String sql;
                sql.Sprintf("(SELECT I FROM %s %s SELECT I FROM %s) %s SELECT I FROM %s",
                    QualifiedClass(ci1.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci2.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci3.name).c_str());
                return sql;
                }
            default:
                {
                // Set op with COUNT(*)
                Utf8String sql;
                sql.Sprintf("SELECT COUNT(*) FROM %s %s SELECT COUNT(*) FROM %s %s SELECT COUNT(*) FROM %s",
                    QualifiedClass(ci1.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci2.name).c_str(), ops[RandInt(0, 3)],
                    QualifiedClass(ci3.name).c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Advanced PRAGMA variations
    //======================================================================
    Utf8String GenPragmaAdvanced()
        {
        int style = RandInt(0, 9);
        switch (style)
            {
            case 0:
                {
                // PRAGMA with complex inner query
                Utf8String inner = GenSelectComplex();
                Utf8String sql;
                sql.Sprintf("PRAGMA explain_query('%s')", inner.c_str());
                return sql;
                }
            case 1:
                {
                // parse_tree with complex query
                Utf8String inner = GenSelectComplex();
                Utf8String sql;
                sql.Sprintf("PRAGMA parse_tree('%s')", inner.c_str());
                return sql;
                }
            case 2:
                {
                // disqualify_type_index for specific class
                Utf8String sql;
                sql.Sprintf("PRAGMA disqualify_type_index=%s FOR %s",
                    Coin() ? "true" : "false", QualifiedClass(Pick(m_allClasses)).c_str());
                return sql;
                }
            case 3:
                return Utf8String("PRAGMA purge_orphan_relationships");
            case 4:
                {
                // explain_query with JOIN
                Utf8String inner = GenSelectWithJoin();
                Utf8String sql;
                sql.Sprintf("PRAGMA explain_query('%s')", inner.c_str());
                return sql;
                }
            case 5:
                {
                // explain_query with CTE
                Utf8String inner = GenSelectWithCTE();
                Utf8String sql;
                sql.Sprintf("PRAGMA explain_query('%s')", inner.c_str());
                return sql;
                }
            case 6:
                {
                // explain_query with set operation
                Utf8String inner = GenSelectSetOperation();
                Utf8String sql;
                sql.Sprintf("PRAGMA explain_query('%s')", inner.c_str());
                return sql;
                }
            case 7:
                {
                // checksum with different targets
                static const char* targets[] = {"ec_schema", "ec_map", "db_schema", "ec_class", "ec_property"};
                Utf8String sql;
                sql.Sprintf("PRAGMA checksum('%s')", targets[RandInt(0, 4)]);
                return sql;
                }
            case 8:
                return Utf8String("PRAGMA integrity_check");
            default:
                return Utf8String("PRAGMA ecdb_ver");
            }
        }

    //======================================================================
    // Schema import fuzz — generates random ECSchema XML
    //======================================================================
    Utf8String GenSchemaXml()
        {
        int style = RandInt(0, 9);
        Utf8String schemaName;
        schemaName.Sprintf("FuzzSchema_%d_%d", RandInt(1, 9999), RandInt(1, 9999));
        switch (style)
            {
            case 0:
                {
                // Minimal valid schema
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='TestEntity'>"
                    "<ECProperty propertyName='Name' typeName='string'/>"
                    "</ECEntityClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            case 1:
                {
                // Schema with all primitive types
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='AllTypes'>"
                    "<ECProperty propertyName='B' typeName='boolean'/>"
                    "<ECProperty propertyName='I' typeName='int'/>"
                    "<ECProperty propertyName='L' typeName='long'/>"
                    "<ECProperty propertyName='D' typeName='double'/>"
                    "<ECProperty propertyName='S' typeName='string'/>"
                    "<ECProperty propertyName='Bi' typeName='binary'/>"
                    "<ECProperty propertyName='Dt' typeName='dateTime'/>"
                    "<ECProperty propertyName='P2' typeName='point2d'/>"
                    "<ECProperty propertyName='P3' typeName='point3d'/>"
                    "</ECEntityClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            case 2:
                {
                // Schema with struct
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECStructClass typeName='Inner'>"
                    "<ECProperty propertyName='X' typeName='int'/>"
                    "</ECStructClass>"
                    "<ECEntityClass typeName='Outer'>"
                    "<ECStructProperty propertyName='Data' typeName='Inner'/>"
                    "</ECEntityClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            case 3:
                {
                // Schema with array properties
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='WithArrays'>"
                    "<ECArrayProperty propertyName='Ints' typeName='int' minOccurs='0' maxOccurs='unbounded'/>"
                    "<ECArrayProperty propertyName='Strs' typeName='string' minOccurs='0' maxOccurs='unbounded'/>"
                    "</ECEntityClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            case 4:
                {
                // Schema with inheritance
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='Base' modifier='Abstract'>"
                    "<ECProperty propertyName='Val' typeName='int'/>"
                    "</ECEntityClass>"
                    "<ECEntityClass typeName='Derived'>"
                    "<BaseClass>Base</BaseClass>"
                    "<ECProperty propertyName='Extra' typeName='string'/>"
                    "</ECEntityClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            case 5:
                {
                // Empty schema
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "</ECSchema>", schemaName.c_str());
                return xml;
                }
            case 6:
                {
                // Malformed XML
                return Utf8String("<?xml version='1.0'?><ECSchema schemaName='Bad'><not-closed>");
                }
            case 7:
                {
                // Schema with many properties
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='ManyProps'>", schemaName.c_str());
                int nProps = RandInt(10, 50);
                for (int i = 0; i < nProps; i++)
                    {
                    static const char* types[] = {"int", "string", "double", "boolean", "long"};
                    Utf8String prop;
                    prop.Sprintf("<ECProperty propertyName='P%d' typeName='%s'/>", i, types[RandInt(0, 4)]);
                    xml.append(prop);
                    }
                xml.append("</ECEntityClass></ECSchema>");
                return xml;
                }
            case 8:
                {
                // Schema with duplicate property names (invalid)
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='DupProps'>"
                    "<ECProperty propertyName='X' typeName='int'/>"
                    "<ECProperty propertyName='X' typeName='string'/>"
                    "</ECEntityClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            default:
                {
                // Schema with relationship
                Utf8String xml;
                xml.Sprintf("<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='%s' alias='fzz' version='01.00.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
                    "<ECEntityClass typeName='Parent'>"
                    "<ECProperty propertyName='Name' typeName='string'/>"
                    "</ECEntityClass>"
                    "<ECEntityClass typeName='Child'>"
                    "<ECProperty propertyName='Val' typeName='int'/>"
                    "</ECEntityClass>"
                    "<ECRelationshipClass typeName='ParentHasChild' strength='embedding' modifier='None'>"
                    "<Source multiplicity='(0..1)' roleLabel='has' polymorphic='true'>"
                    "<Class class='Parent'/></Source>"
                    "<Target multiplicity='(0..*)' roleLabel='belongs to' polymorphic='true'>"
                    "<Class class='Child'/></Target>"
                    "</ECRelationshipClass></ECSchema>", schemaName.c_str());
                return xml;
                }
            }
        }

    //======================================================================
    // Advanced UPDATE patterns
    //======================================================================
    Utf8String GenUpdateAdvanced()
        {
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // UPDATE with CASE in SET
                auto const& ci = GetClassInfo("FStandalone");
                Utf8String sql;
                sql.Sprintf("UPDATE %s SET Code = CASE WHEN Code > 0 THEN Code + 1 ELSE 0 END WHERE Code IS NOT NULL",
                    QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 1:
                {
                // UPDATE with COALESCE in SET
                Utf8String sql;
                sql.Sprintf("UPDATE %s SET Amount = COALESCE(Amount, 0.0) + 1.0",
                    QualifiedClass("FStandalone").c_str());
                return sql;
                }
            case 2:
                {
                // UPDATE struct property
                return Utf8String("UPDATE fz.FSub1 SET SimpleStruct.si = 42 WHERE SimpleStruct.si IS NOT NULL");
                }
            case 3:
                {
                // UPDATE navigation property
                return Utf8String("UPDATE fz.FStandalone SET Name = 'updated' WHERE Owner.Id IS NOT NULL");
                }
            case 4:
                {
                // UPDATE with complex WHERE using subquery
                Utf8String sql;
                sql.Sprintf("UPDATE %s SET Code = 0 WHERE Code IN (SELECT I FROM %s WHERE I > 5)",
                    QualifiedClass("FStandalone").c_str(), QualifiedClass("FSub1").c_str());
                return sql;
                }
            default:
                {
                // UPDATE with multiple SET + LIMIT (if supported)
                Utf8String sql;
                sql.Sprintf("UPDATE %s SET Name = 'test', Code = Code + 1, Amount = 0.0 WHERE Flag = true",
                    QualifiedClass("FStandalone").c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Advanced DELETE patterns
    //======================================================================
    Utf8String GenDeleteAdvanced()
        {
        int style = RandInt(0, 3);
        switch (style)
            {
            case 0:
                {
                // DELETE with subquery WHERE
                Utf8String sql;
                sql.Sprintf("DELETE FROM %s WHERE Code IN (SELECT I FROM %s)",
                    QualifiedClass("FStandalone").c_str(), QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 1:
                {
                // DELETE with EXISTS
                Utf8String sql;
                sql.Sprintf("DELETE FROM %s WHERE EXISTS (SELECT 1 FROM %s WHERE I = Code)",
                    QualifiedClass("FStandalone").c_str(), QualifiedClass("FSub1").c_str());
                return sql;
                }
            case 2:
                {
                // DELETE with ONLY and complex WHERE
                auto const& ci = GetClassInfo("FBase");
                Utf8String sql;
                sql.Sprintf("DELETE FROM ONLY %s WHERE %s AND %s",
                    QualifiedClass("FBase").c_str(), GenExpression(ci).c_str(), GenExpression(ci).c_str());
                return sql;
                }
            default:
                {
                // DELETE with NOT EXISTS
                Utf8String sql;
                sql.Sprintf("DELETE FROM %s WHERE NOT EXISTS (SELECT 1 FROM %s WHERE I = Code)",
                    QualifiedClass("FStandalone").c_str(), QualifiedClass("FSub1").c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Parameter binding patterns
    //======================================================================
    Utf8String GenSelectWithParameters()
        {
        auto const& ci = RandClassInfo();
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // Positional parameters
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE %s = ? AND %s = ?",
                    QualifiedClass(ci.name).c_str(),
                    ci.intProps.empty() ? "ECInstanceId" : Pick(ci.intProps).c_str(),
                    ci.strProps.empty() ? "ECInstanceId" : Pick(ci.strProps).c_str());
                return sql;
                }
            case 1:
                {
                // Named parameters
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE %s = :val1 AND %s = :val2",
                    QualifiedClass(ci.name).c_str(),
                    ci.intProps.empty() ? "ECInstanceId" : Pick(ci.intProps).c_str(),
                    ci.strProps.empty() ? "ECInstanceId" : Pick(ci.strProps).c_str());
                return sql;
                }
            case 2:
                {
                // Parameter in INSERT
                Utf8String sql;
                sql.Sprintf("INSERT INTO %s(%s) VALUES(?)",
                    QualifiedClass(Pick(m_concreteClasses)).c_str(),
                    ci.allProps.empty() ? "ECInstanceId" : ci.allProps[0].c_str());
                return sql;
                }
            case 3:
                {
                // Parameter in UPDATE SET
                Utf8String sql;
                sql.Sprintf("UPDATE %s SET %s = ? WHERE ECInstanceId = ?",
                    QualifiedClass(Pick(m_concreteClasses)).c_str(),
                    ci.allProps.empty() ? "ECInstanceId" : ci.allProps[0].c_str());
                return sql;
                }
            case 4:
                {
                // Many parameters
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE %s IN (?, ?, ?, ?, ?)",
                    QualifiedClass(ci.name).c_str(),
                    ci.intProps.empty() ? "ECInstanceId" : Pick(ci.intProps).c_str());
                return sql;
                }
            default:
                {
                // Parameter in BETWEEN
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE %s BETWEEN ? AND ?",
                    QualifiedClass(ci.name).c_str(),
                    ci.intProps.empty() ? "ECInstanceId" : Pick(ci.intProps).c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Comment injection patterns
    //======================================================================
    Utf8String GenSelectWithComments()
        {
        auto const& ci = RandClassInfo();
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // Line comment at end
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s -- this is a comment", QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 1:
                {
                // Block comment in middle
                Utf8String sql;
                sql.Sprintf("SELECT /* comment */ * FROM %s", QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 2:
                {
                // Block comment around keyword
                Utf8String sql;
                sql.Sprintf("SELECT * FROM /* ignored */ %s", QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                // Nested block comments
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s /* outer /* inner */ still comment */", QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // Comment only
                return Utf8String("-- just a comment");
                }
            default:
                {
                // Mixed comments
                Utf8String sql;
                sql.Sprintf("/* start */ SELECT -- line\n* FROM %s /* end */", QualifiedClass(ci.name).c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // Literal edge cases
    //======================================================================
    Utf8String GenSelectLiteralEdgeCases()
        {
        auto const& ci = RandClassInfo();
        int style = RandInt(0, 9);
        switch (style)
            {
            case 0:
                {
                // Hex literal
                return Utf8String("SELECT 0x1A2B3C FROM fz.FBase LIMIT 1");
                }
            case 1:
                {
                // Very large number
                return Utf8String("SELECT 9999999999999999999 FROM fz.FBase LIMIT 1");
                }
            case 2:
                {
                // Very small number
                return Utf8String("SELECT -9999999999999999999 FROM fz.FBase LIMIT 1");
                }
            case 3:
                {
                // Float with exponent
                return Utf8String("SELECT 1.23e10, 4.56E-7, -1e100 FROM fz.FBase LIMIT 1");
                }
            case 4:
                {
                // Empty string
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s WHERE %s = ''",
                    QualifiedClass(ci.name).c_str(),
                    ci.strProps.empty() ? "S" : Pick(ci.strProps).c_str());
                return sql;
                }
            case 5:
                {
                // String with escaped quotes
                return Utf8String("SELECT 'it''s a test', 'hello ''world''' FROM fz.FBase LIMIT 1");
                }
            case 6:
                {
                // BLOB literal
                return Utf8String("SELECT X'48454C4C4F' FROM fz.FBase LIMIT 1");
                }
            case 7:
                {
                // Boolean comparisons
                return Utf8String("SELECT * FROM fz.FBase WHERE B = true AND B IS NOT false");
                }
            case 8:
                {
                // NULL comparisons
                return Utf8String("SELECT NULL = NULL, NULL IS NULL, NULL IS NOT NULL, COALESCE(NULL, 1) FROM fz.FBase LIMIT 1");
                }
            default:
                {
                // Date/time literals
                return Utf8String("SELECT * FROM fz.FStandalone WHERE Created > '2020-01-01' AND DateOnly < '2025-12-31'");
                }
            }
        }

    //======================================================================
    // Advanced navigation property patterns
    //======================================================================
    Utf8String GenSelectNavigationAdvanced()
        {
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                return Utf8String("SELECT Owner, Owner.Id, Owner.RelECClassId FROM fz.FStandalone WHERE Owner IS NOT NULL");
            case 1:
                return Utf8String("SELECT Parent, Parent.Id, Parent.RelECClassId FROM fz.FSub1 WHERE Parent IS NOT NULL");
            case 2:
                {
                // Navigation in JOIN
                return Utf8String("SELECT a.Name, b.I FROM fz.FStandalone a INNER JOIN fz.FBase b ON a.Owner.Id = b.ECInstanceId");
                }
            case 3:
                {
                // Navigation in aggregate
                return Utf8String("SELECT Owner.Id, COUNT(*) FROM fz.FStandalone GROUP BY Owner.Id");
                }
            case 4:
                {
                // Navigation in ORDER BY
                return Utf8String("SELECT * FROM fz.FStandalone ORDER BY Owner.Id ASC NULLS LAST");
                }
            case 5:
                {
                // Navigation in CASE
                return Utf8String("SELECT CASE WHEN Owner.Id IS NOT NULL THEN 'owned' ELSE 'orphan' END FROM fz.FStandalone");
                }
            case 6:
                {
                // Multiple navigation accesses
                return Utf8String("SELECT Parent.Id, Parent.RelECClassId FROM fz.FSub1 WHERE Parent.Id IS NOT NULL AND Parent.RelECClassId IS NOT NULL");
                }
            default:
                {
                // Navigation with subquery
                return Utf8String("SELECT * FROM fz.FStandalone WHERE Owner.Id IN (SELECT ECInstanceId FROM fz.FBase WHERE I > 0)");
                }
            }
        }

    //======================================================================
    // VALUES clause patterns
    //======================================================================
    Utf8String GenSelectValues()
        {
        int style = RandInt(0, 3);
        switch (style)
            {
            case 0:
                return Utf8String("VALUES(1), (2), (3)");
            case 1:
                return Utf8String("VALUES(1, 'a'), (2, 'b'), (3, 'c')");
            case 2:
                {
                // VALUES with many rows
                Utf8String sql = "VALUES";
                int rows = RandInt(5, 20);
                for (int i = 0; i < rows; i++)
                    {
                    if (i > 0) sql.append(",");
                    Utf8String row;
                    row.Sprintf("(%s, %s)", RandIntLiteral().c_str(), RandStringLiteral().c_str());
                    sql.append(row);
                    }
                return sql;
                }
            default:
                {
                // VALUES in UNION
                Utf8String sql;
                sql.Sprintf("SELECT I FROM %s UNION VALUES(999), (888)", QualifiedClass("FSub1").c_str());
                return sql;
                }
            }
        }

    //======================================================================
    // ORDER BY advanced patterns
    //======================================================================
    Utf8String GenSelectOrderByAdvanced()
        {
        auto const& ci = RandClassInfo();
        if (ci.allProps.size() < 2) return GenSelectSimple();
        int style = RandInt(0, 5);
        switch (style)
            {
            case 0:
                {
                // Multiple ORDER BY with mixed directions + NULLS
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s ORDER BY %s ASC NULLS FIRST, %s DESC NULLS LAST",
                    QualifiedClass(ci.name).c_str(), Pick(ci.allProps).c_str(), Pick(ci.allProps).c_str());
                return sql;
                }
            case 1:
                {
                // ORDER BY with expression
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s ORDER BY %s + 1, LENGTH(%s) DESC",
                    QualifiedClass(ci.name).c_str(),
                    ci.intProps.empty() ? "ECInstanceId" : Pick(ci.intProps).c_str(),
                    ci.strProps.empty() ? "''" : Pick(ci.strProps).c_str());
                return sql;
                }
            case 2:
                {
                // ORDER BY with CASE
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s ORDER BY CASE WHEN %s IS NULL THEN 1 ELSE 0 END, %s ASC",
                    QualifiedClass(ci.name).c_str(), Pick(ci.allProps).c_str(), Pick(ci.allProps).c_str());
                return sql;
                }
            case 3:
                {
                // ORDER BY ordinal
                Utf8String sql;
                sql.Sprintf("SELECT %s, %s FROM %s ORDER BY 1 ASC, 2 DESC",
                    Pick(ci.allProps).c_str(), Pick(ci.allProps).c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // ORDER BY with COLLATE
                if (!ci.strProps.empty())
                    {
                    Utf8String sql;
                    sql.Sprintf("SELECT * FROM %s ORDER BY %s COLLATE NOCASE ASC, %s COLLATE BINARY DESC",
                        QualifiedClass(ci.name).c_str(), Pick(ci.strProps).c_str(), Pick(ci.strProps).c_str());
                    return sql;
                    }
                return GenSelectSimple();
                }
            default:
                {
                // ORDER BY with LIMIT + OFFSET
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s ORDER BY %s ASC LIMIT %d OFFSET %d",
                    QualifiedClass(ci.name).c_str(), Pick(ci.allProps).c_str(), RandInt(1, 50), RandInt(0, 20));
                return sql;
                }
            }
        }
    //======================================================================
    // SQL/ECSQL function-heavy queries
    //======================================================================
    Utf8String GenSelectFunctions()
        {
        auto const& ci = RandClassInfo();
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                // DateTime functions
                static const char* dtFuncs[] = {"date", "datetime", "time", "julianday"};
                Utf8String sql;
                sql.Sprintf("SELECT %s('now'), %s('2020-01-01') FROM %s",
                    dtFuncs[RandInt(0, 3)], dtFuncs[RandInt(0, 3)], QualifiedClass(ci.name).c_str());
                if (Coin(30))
                    sql.Sprintf("%s LIMIT 1", sql.c_str());
                return sql;
                }
            case 1:
                {
                // strftime
                static const char* fmts[] = {"'%Y'", "'%m'", "'%d'", "'%H:%M'", "'%Y-%m-%d'", "'%s'"};
                Utf8String sql;
                sql.Sprintf("SELECT strftime(%s, 'now') FROM %s LIMIT 1",
                    fmts[RandInt(0, 5)], QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 2:
                {
                // JSON functions
                static const char* jsonFuncs[] = {
                    "SELECT json_array(1, 2, 3)",
                    "SELECT json_object('key', 'value')",
                    "SELECT json_valid('{}')",
                    "SELECT json_type('null')",
                    "SELECT json_quote('hello')",
                    "SELECT json_extract('{\"a\":1}', '$.a')"
                    };
                Utf8String base = jsonFuncs[RandInt(0, 5)];
                Utf8String sql;
                sql.Sprintf("%s FROM %s LIMIT 1", base.c_str(), QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 3:
                {
                // String functions: substr, replace, instr
                if (!ci.strProps.empty())
                    {
                    Utf8String prop = Pick(ci.strProps);
                    int sub = RandInt(0, 2);
                    Utf8String sql;
                    if (sub == 0)
                        sql.Sprintf("SELECT substr(%s, 1, 3) FROM %s", prop.c_str(), QualifiedClass(ci.name).c_str());
                    else if (sub == 1)
                        sql.Sprintf("SELECT replace(%s, 'a', 'b') FROM %s", prop.c_str(), QualifiedClass(ci.name).c_str());
                    else
                        sql.Sprintf("SELECT instr(%s, 'test') FROM %s", prop.c_str(), QualifiedClass(ci.name).c_str());
                    return sql;
                    }
                return GenSelectSimple();
                }
            case 4:
                {
                // Math functions: abs, round, random, max/min scalar
                if (!ci.dblProps.empty())
                    {
                    Utf8String prop = Pick(ci.dblProps);
                    Utf8String sql;
                    sql.Sprintf("SELECT abs(%s), round(%s, 2), random() FROM %s",
                        prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
                    return sql;
                    }
                return Utf8String("SELECT abs(-42), round(3.14159, 2), random()");
                }
            case 5:
                {
                // COALESCE / NULLIF / IIF / TYPEOF combined
                if (ci.allProps.size() >= 2)
                    {
                    Utf8String p1 = Pick(ci.allProps);
                    Utf8String p2 = Pick(ci.allProps);
                    Utf8String sql;
                    sql.Sprintf("SELECT COALESCE(%s, %s), NULLIF(%s, %s), TYPEOF(%s), IIF(%s IS NULL, 'null', 'not_null') FROM %s",
                        p1.c_str(), RandLiteral().c_str(),
                        p1.c_str(), p2.c_str(),
                        p1.c_str(), p2.c_str(),
                        QualifiedClass(ci.name).c_str());
                    return sql;
                    }
                return GenSelectSimple();
                }
            case 6:
                {
                // hex, quote, zeroblob, unicode
                if (!ci.strProps.empty())
                    {
                    Utf8String prop = Pick(ci.strProps);
                    Utf8String sql;
                    sql.Sprintf("SELECT hex(%s), quote(%s), unicode(%s) FROM %s",
                        prop.c_str(), prop.c_str(), prop.c_str(), QualifiedClass(ci.name).c_str());
                    return sql;
                    }
                return Utf8String("SELECT hex('hello'), quote(42), zeroblob(10)");
                }
            default:
                {
                // printf / format
                if (!ci.intProps.empty())
                    {
                    Utf8String sql;
                    sql.Sprintf("SELECT printf('%%d', %s) FROM %s",
                        Pick(ci.intProps).c_str(), QualifiedClass(ci.name).c_str());
                    return sql;
                    }
                return GenSelectSimple();
                }
            }
        }

    //======================================================================
    // ECSQL-specific features (polymorphism, system props, options)
    //======================================================================
    Utf8String GenSelectECSqlSpecific()
        {
        int style = RandInt(0, 7);
        switch (style)
            {
            case 0:
                {
                // Polymorphic query on abstract base
                Utf8String sql;
                sql.Sprintf("SELECT ECInstanceId, ECClassId, S FROM fz.FBase");
                if (Coin(50))
                    {
                    Utf8String cls = Pick(m_concreteClasses);
                    sql.Sprintf("%s WHERE ECClassId IS (%s fz.%s)", sql.c_str(), Coin() ? "ONLY" : "", cls.c_str());
                    }
                return sql;
                }
            case 1:
                {
                // ECClassId IS NOT (exclusion filter)
                Utf8String cls = Pick(m_concreteClasses);
                Utf8String sql;
                sql.Sprintf("SELECT * FROM fz.FBase WHERE ECClassId IS NOT (ONLY fz.%s)", cls.c_str());
                return sql;
                }
            case 2:
                {
                // Point component access
                static const char* exprs[] = {
                    "SELECT P2D.X, P2D.Y FROM fz.FSub1 WHERE P2D.X IS NOT NULL",
                    "SELECT P3D.X, P3D.Y, P3D.Z FROM fz.FSub1",
                    "SELECT Pt2.X, Pt2.Y, Pt3.X, Pt3.Y, Pt3.Z FROM fz.FStandalone",
                    "SELECT * FROM fz.FSub1 WHERE P2D.X > 0 AND P2D.Y < 100"
                    };
                return Utf8String(exprs[RandInt(0, 3)]);
                }
            case 3:
                {
                // ECSQLOPTIONS on different statement types
                auto const& ci = RandClassInfo();
                Utf8String sql;
                sql.Sprintf("SELECT * FROM %s ECSQLOPTIONS NoECClassIdFilter", QualifiedClass(ci.name).c_str());
                return sql;
                }
            case 4:
                {
                // Navigation property deep access
                static const char* exprs[] = {
                    "SELECT Parent, Parent.Id, Parent.RelECClassId FROM fz.FSub1",
                    "SELECT Owner, Owner.Id, Owner.RelECClassId FROM fz.FStandalone",
                    "SELECT * FROM fz.FSub1 WHERE Parent.Id IS NOT NULL",
                    "SELECT * FROM fz.FStandalone WHERE Owner IS NOT NULL"
                    };
                return Utf8String(exprs[RandInt(0, 3)]);
                }
            case 5:
                {
                // Mixin queries
                return Utf8String("SELECT ECInstanceId, MixinLabel FROM fz.FMixin");
                }
            case 6:
                {
                // Array property queries
                static const char* exprs[] = {
                    "SELECT IntArray, StrArray, DblArray FROM fz.FSub2",
                    "SELECT StructArray, NestedStructArray FROM fz.FSub2",
                    "SELECT BoundedArray FROM fz.FSub2 WHERE BoundedArray IS NOT NULL",
                    "SELECT * FROM fz.FSub2 WHERE IntArray IS NOT NULL"
                    };
                return Utf8String(exprs[RandInt(0, 3)]);
                }
            default:
                {
                // Struct deep access
                static const char* exprs[] = {
                    "SELECT SimpleStruct, SimpleStruct.sb, SimpleStruct.si, SimpleStruct.ss FROM fz.FSub1",
                    "SELECT DeepStruct.ds, DeepStruct.nested.ns, DeepStruct.nested.inner.si FROM fz.FSub1",
                    "SELECT Details, Details.sb, Details.si, Details.sd FROM fz.FStandalone",
                    "SELECT * FROM fz.FSub1 WHERE SimpleStruct.si > 0"
                    };
                return Utf8String(exprs[RandInt(0, 3)]);
                }
            }
        }
