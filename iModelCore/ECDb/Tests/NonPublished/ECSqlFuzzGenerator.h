/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"
#include <random>
#include <sstream>
#include <algorithm>

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Structure-aware ECSQL generator for fuzz testing.
// Uses knowledge of the FuzzSchema to produce syntactically plausible ECSQL that reaches
// deep code paths in the parser and preparer (schema resolution, property mapping, etc.).
// Also includes a mutation engine for error-handling path coverage.
// @bsiclass
//=======================================================================================
struct ECSqlFuzzGenerator
{
private:
    std::mt19937 m_rng;

    int RandInt(int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(m_rng); }
    bool Coin(int pctTrue = 50) { return RandInt(1, 100) <= pctTrue; }

    template<typename T>
    T const& Pick(std::vector<T> const& v) { return v[(size_t)RandInt(0, (int)v.size() - 1)]; }

    static constexpr Utf8CP s_alias = "fz";

    // --- Schema metadata: class names ---
    std::vector<Utf8String> m_concreteClasses {"FSub1", "FSub2", "FSub3", "FStandalone"};
    std::vector<Utf8String> m_allClasses      {"FBase", "FSub1", "FSub2", "FSub3", "FStandalone", "FMixin"};
    std::vector<Utf8String> m_relationships   {"FBaseOwnsStandalone", "FStandaloneRefsFBase", "FBaseHasSub1", "FSub1HasSub2"};

    // Properties per class (subset of what's available, enough for generation)
    struct ClassInfo
        {
        Utf8String name;
        std::vector<Utf8String> intProps;
        std::vector<Utf8String> strProps;
        std::vector<Utf8String> dblProps;
        std::vector<Utf8String> boolProps;
        std::vector<Utf8String> allProps;   // union of all scalar props
        std::vector<Utf8String> structProps; // struct dot-path candidates
        std::vector<Utf8String> arrayProps;
        std::vector<Utf8String> navProps;
        };

    std::vector<ClassInfo> m_classInfos;

    void InitClassInfos()
        {
        // FBase (abstract — queried polymorphically)
        m_classInfos.push_back({"FBase",
            {"I", "L"}, {"S"}, {"D"}, {"B"},
            {"B","I","L","D","S","Bi","Dt","DtUtc","P2D","P3D","Color"},
            {}, {}, {}});

        // FSub1
        m_classInfos.push_back({"FSub1",
            {"I", "L"}, {"S", "Sub1Name"}, {"D"}, {"B"},
            {"B","I","L","D","S","Sub1Name","Dt","DtUtc","P2D","P3D","Color"},
            {"SimpleStruct.sb", "SimpleStruct.si", "SimpleStruct.ss", "DeepStruct.ds", "DeepStruct.nested.ns", "DeepStruct.nested.inner.si"},
            {},
            {"Parent"}});

        // FSub2
        m_classInfos.push_back({"FSub2",
            {"I", "L", "Sub2Code"}, {"S", "MixinLabel"}, {"D"}, {"B"},
            {"B","I","L","D","S","Sub2Code","MixinLabel","Dt","DtUtc","P2D","P3D","Color"},
            {},
            {"IntArray", "StrArray", "DblArray", "BoundedArray", "StructArray", "NestedStructArray"},
            {}});

        // FSub3
        m_classInfos.push_back({"FSub3",
            {"I", "L"}, {"S", "Sub1Name", "Sub3Tag"}, {"D", "Sub3Val"}, {"B"},
            {"B","I","L","D","S","Sub1Name","Sub3Tag","Sub3Val","Dt","DtUtc","P2D","P3D","Color"},
            {"SimpleStruct.sb", "SimpleStruct.si", "SimpleStruct.ss"},
            {},
            {"Parent"}});

        // FStandalone
        m_classInfos.push_back({"FStandalone",
            {"Code"}, {"Name"}, {"Amount"}, {"Flag"},
            {"Name","Code","Amount","Flag","Created","DateOnly","Pt2","Pt3","Color"},
            {"Details.sb", "Details.si", "Details.ss", "Details.sd"},
            {"Tags"},
            {"Owner"}});
        }

    ClassInfo const& GetClassInfo(Utf8StringCR name)
        {
        for (auto const& ci : m_classInfos)
            if (ci.name.Equals(name))
                return ci;
        return m_classInfos[0]; // fallback
        }

    ClassInfo const& RandClassInfo() { return Pick(m_classInfos); }

    //----------------------------------------------------------------------
    // Value generators
    //----------------------------------------------------------------------
    Utf8String RandIntLiteral()    { Utf8String s; s.Sprintf("%d", RandInt(-999999, 999999)); return s; }
    Utf8String RandDoubleLiteral() { Utf8String s; s.Sprintf("%f", RandInt(-9999, 9999) * 0.01); return s; }
    Utf8String RandStringLiteral()
        {
        static const char* words[] = {"alpha","beta","gamma","delta","epsilon","zeta","eta","theta",""," ","test","foo","bar"};
        Utf8String s; s.Sprintf("'%s'", words[RandInt(0, 12)]);
        return s;
        }
    Utf8String RandBoolLiteral() { return Coin() ? "true" : "false"; }
    Utf8String RandNullLiteral() { return "NULL"; }

    Utf8String RandLiteral()
        {
        switch (RandInt(0, 4))
            {
            case 0: return RandIntLiteral();
            case 1: return RandDoubleLiteral();
            case 2: return RandStringLiteral();
            case 3: return RandBoolLiteral();
            default: return RandNullLiteral();
            }
        }

    Utf8String QualifiedClass(Utf8StringCR cls) { Utf8String s; s.Sprintf("%s.%s", s_alias, cls.c_str()); return s; }

    //----------------------------------------------------------------------
    // Expression generator with depth control
    //----------------------------------------------------------------------
    Utf8String GenExpression(ClassInfo const& ci, int depth = 0)
        {
        if (depth > 3 || ci.allProps.empty())
            return RandLiteral();

        int choice = RandInt(0, 21);
        Utf8String prop = Pick(ci.allProps);

        switch (choice)
            {
            case 0: // comparison
            case 1:
                {
                static const char* ops[] = {"=", "<>", "<", ">", "<=", ">="};
                Utf8String s; s.Sprintf("%s %s %s", prop.c_str(), ops[RandInt(0, 5)], RandLiteral().c_str());
                return s;
                }
            case 2: // AND
                {
                Utf8String s; s.Sprintf("(%s AND %s)", GenExpression(ci, depth + 1).c_str(), GenExpression(ci, depth + 1).c_str());
                return s;
                }
            case 3: // OR
                {
                Utf8String s; s.Sprintf("(%s OR %s)", GenExpression(ci, depth + 1).c_str(), GenExpression(ci, depth + 1).c_str());
                return s;
                }
            case 4: // NOT
                {
                Utf8String s; s.Sprintf("NOT (%s)", GenExpression(ci, depth + 1).c_str());
                return s;
                }
            case 5: // IS NULL / IS NOT NULL
                {
                Utf8String s; s.Sprintf("%s IS %sNULL", prop.c_str(), Coin() ? "NOT " : "");
                return s;
                }
            case 6: // BETWEEN with varied types
                {
                if (Coin(70))
                    {
                    Utf8String s; s.Sprintf("%s BETWEEN %s AND %s", prop.c_str(), RandIntLiteral().c_str(), RandIntLiteral().c_str());
                    return s;
                    }
                else if (!ci.strProps.empty())
                    {
                    Utf8String s; s.Sprintf("%s BETWEEN 'a' AND 'z'", Pick(ci.strProps).c_str());
                    return s;
                    }
                Utf8String s; s.Sprintf("%s BETWEEN %s AND %s", prop.c_str(), RandIntLiteral().c_str(), RandIntLiteral().c_str());
                return s;
                }
            case 7: // LIKE with optional ESCAPE
                {
                if (!ci.strProps.empty())
                    {
                    Utf8String sProp = Pick(ci.strProps);
                    if (Coin(30))
                        {
                        Utf8String s; s.Sprintf("%s LIKE '%%test%%' ESCAPE '\\'", sProp.c_str());
                        return s;
                        }
                    Utf8String s; s.Sprintf("%s LIKE '%%test%%'", sProp.c_str());
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 8: // IN list
                {
                Utf8String s; s.Sprintf("%s IN (%s, %s, %s)", prop.c_str(), RandLiteral().c_str(), RandLiteral().c_str(), RandLiteral().c_str());
                return s;
                }
            case 9: // IN subquery
                {
                auto const& other = RandClassInfo();
                if (!other.intProps.empty())
                    {
                    Utf8String s; s.Sprintf("%s IN (SELECT %s FROM %s)", prop.c_str(), Pick(other.intProps).c_str(), QualifiedClass(other.name).c_str());
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 10: // EXISTS
                {
                auto const& other = RandClassInfo();
                Utf8String s; s.Sprintf("EXISTS (SELECT 1 FROM %s)", QualifiedClass(other.name).c_str());
                return s;
                }
            case 11: // struct property access
                {
                if (!ci.structProps.empty())
                    {
                    Utf8String s; s.Sprintf("%s IS NOT NULL", Pick(ci.structProps).c_str());
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 12: // parameter
                return Utf8String("? = ") + RandLiteral();
            case 13: // arithmetic with varied operators
                {
                static const char* arithOps[] = {"+", "-", "*", "/", "%"};
                Utf8String s; s.Sprintf("(%s %s %s)", prop.c_str(), arithOps[RandInt(0, 4)], RandIntLiteral().c_str());
                return s;
                }
            case 14: // string concatenation ||
                {
                if (!ci.strProps.empty())
                    {
                    Utf8String s; s.Sprintf("(%s || '_' || %s)", Pick(ci.strProps).c_str(), RandStringLiteral().c_str());
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 15: // COALESCE
                {
                Utf8String p2 = ci.allProps.size() > 1 ? Pick(ci.allProps) : RandLiteral();
                Utf8String s; s.Sprintf("COALESCE(%s, %s, %s)", prop.c_str(), p2.c_str(), RandLiteral().c_str());
                return s;
                }
            case 16: // NULLIF
                {
                Utf8String s; s.Sprintf("NULLIF(%s, %s)", prop.c_str(), RandLiteral().c_str());
                return s;
                }
            case 17: // TYPEOF
                {
                Utf8String s; s.Sprintf("TYPEOF(%s) = 'integer'", prop.c_str());
                return s;
                }
            case 18: // ALL/ANY quantified subquery comparison
                {
                auto const& other = RandClassInfo();
                if (!other.intProps.empty() && !ci.intProps.empty())
                    {
                    static const char* quantifiers[] = {"ALL", "ANY", "SOME"};
                    static const char* cmpOps[] = {"=", "<>", "<", ">", "<=", ">="};
                    Utf8String s; s.Sprintf("%s %s %s(SELECT %s FROM %s)",
                        Pick(ci.intProps).c_str(), cmpOps[RandInt(0, 5)],
                        quantifiers[RandInt(0, 2)],
                        Pick(other.intProps).c_str(), QualifiedClass(other.name).c_str());
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 19: // math functions: abs, round, random
                {
                static const char* mathFuncs[] = {"abs", "round"};
                if (!ci.dblProps.empty())
                    {
                    Utf8String s; s.Sprintf("%s(%s) > 0", mathFuncs[RandInt(0, 1)], Pick(ci.dblProps).c_str());
                    return s;
                    }
                return Utf8String("random() > 0");
                }
            case 20: // CAST in expression
                {
                static const char* types[] = {"INTEGER", "REAL", "TEXT"};
                Utf8String s; s.Sprintf("CAST(%s AS %s) IS NOT NULL", prop.c_str(), types[RandInt(0, 2)]);
                return s;
                }
            default: // simple comparison fallback
                {
                Utf8String s; s.Sprintf("%s = %s", prop.c_str(), RandLiteral().c_str());
                return s;
                }
            }
        }

    //----------------------------------------------------------------------
    // Property list for SELECT
    //----------------------------------------------------------------------
    Utf8String GenSelectList(ClassInfo const& ci)
        {
        int style = RandInt(0, 10);
        switch (style)
            {
            case 0: return "DISTINCT *";
            case 1: return "COUNT(*)";
            case 2:
                {
                // 1-5 random properties, some with DISTINCT
                int n = RandInt(1, std::min(5, (int)ci.allProps.size()));
                Utf8String s;
                if (Coin(25)) s.append("DISTINCT ");
                for (int i = 0; i < n; i++)
                    {
                    if (i > 0) s.append(", ");
                    s.append(Pick(ci.allProps));
                    }
                return s;
                }
            case 3:
                {
                // struct property access
                if (!ci.structProps.empty())
                    return Pick(ci.structProps);
                return "*";
                }
            case 4:
                {
                // aggregate with GROUP_CONCAT
                if (!ci.intProps.empty())
                    {
                    static const char* aggs[] = {"SUM", "AVG", "MIN", "MAX", "COUNT", "GROUP_CONCAT"};
                    Utf8CP agg = aggs[RandInt(0, 5)];
                    Utf8String s; s.Sprintf("%s(%s)", agg, Pick(ci.intProps).c_str());
                    return s;
                    }
                return "COUNT(*)";
                }
            case 5:
                {
                // string / math functions
                if (!ci.strProps.empty())
                    {
                    static const char* funcs[] = {"UPPER", "LOWER", "LENGTH", "TRIM", "LTRIM", "RTRIM", "HEX", "QUOTE", "TYPEOF"};
                    Utf8String s; s.Sprintf("%s(%s)", funcs[RandInt(0, 8)], Pick(ci.strProps).c_str());
                    return s;
                    }
                return "*";
                }
            case 6:
                {
                // COALESCE / NULLIF / IIF
                if (ci.allProps.size() >= 2)
                    {
                    if (Coin())
                        {
                        Utf8String s; s.Sprintf("COALESCE(%s, %s, %s)",
                            Pick(ci.allProps).c_str(), Pick(ci.allProps).c_str(), RandLiteral().c_str());
                        return s;
                        }
                    Utf8String s; s.Sprintf("NULLIF(%s, %s)", Pick(ci.allProps).c_str(), RandLiteral().c_str());
                    return s;
                    }
                return "*";
                }
            case 7:
                {
                // DateTime functions
                Utf8String s;
                static const char* dtFuncs[] = {"julianday", "date", "datetime", "time"};
                s.Sprintf("%s('now')", dtFuncs[RandInt(0, 3)]);
                return s;
                }
            case 8:
                {
                // CAST expressions
                if (!ci.allProps.empty())
                    {
                    static const char* types[] = {"INTEGER", "REAL", "TEXT", "BLOB"};
                    Utf8String s; s.Sprintf("CAST(%s AS %s)", Pick(ci.allProps).c_str(), types[RandInt(0, 3)]);
                    return s;
                    }
                return "*";
                }
            case 9:
                {
                // Math functions
                if (!ci.dblProps.empty())
                    {
                    static const char* mathFuncs[] = {"abs", "round"};
                    Utf8String s; s.Sprintf("%s(%s)", mathFuncs[RandInt(0, 1)], Pick(ci.dblProps).c_str());
                    return s;
                    }
                return Utf8String("random()");
                }
            default:
                {
                // String concatenation
                if (ci.strProps.size() >= 2)
                    {
                    Utf8String s; s.Sprintf("%s || ' ' || %s", Pick(ci.strProps).c_str(), Pick(ci.strProps).c_str());
                    return s;
                    }
                return "*";
                }
            }
        }

    //----------------------------------------------------------------------
    // FROM clause
    //----------------------------------------------------------------------
    Utf8String GenFromClause(ClassInfo const& ci, Utf8CP alias = nullptr)
        {
        Utf8String from = QualifiedClass(ci.name);
        if (Coin(30))
            from = Utf8String("ONLY ") + from;
        if (alias)
            {
            from.append(" ");
            from.append(alias);
            }
        else if (Coin(40))
            from.append(" t");
        return from;
        }

public:
    explicit ECSqlFuzzGenerator(uint32_t seed) : m_rng(seed)
        {
        InitClassInfos();
        }

    uint32_t GetSeed() const { return 0; /* seed is set at construction */ }

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

    //======================================================================
    // Random statement of any type
    //======================================================================
    Utf8String GenAnyStatement()
        {
        switch (RandInt(0, 17))
            {
            case 0: case 1: return GenSelectSimple();
            case 2: case 3: return GenSelectComplex();
            case 4: return GenSelectRelationship();
            case 5: return GenSelectSetOperation();
            case 6: return GenSelectAggregate();
            case 7: return GenInsert();
            case 8: return GenUpdate();
            case 9: return GenDelete();
            case 10: return GenSelectFunctions();
            case 11: return GenSelectECSqlSpecific();
            case 12: return GenPragma();
            case 13: return GenSelectWithAliases();
            case 14: return GenSelectBracketQuoted();
            case 15: return GenSelectSubqueryPositions();
            case 16: return GenInsertSelect();
            default: return GenSelectSimple();
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
};

END_ECDBUNITTESTS_NAMESPACE
