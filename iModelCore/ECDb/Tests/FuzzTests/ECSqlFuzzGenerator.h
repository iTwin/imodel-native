/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../NonPublished/ECDbPublishedTests.h"
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

        int choice = RandInt(0, 25);
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
            case 21: // Bitwise AND
                {
                if (!ci.intProps.empty())
                    {
                    Utf8String s; s.Sprintf("(%s & %d) > 0", Pick(ci.intProps).c_str(), RandInt(1, 255));
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 22: // Bitwise OR
                {
                if (!ci.intProps.empty())
                    {
                    Utf8String s; s.Sprintf("(%s | %d)", Pick(ci.intProps).c_str(), RandInt(0, 255));
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 23: // Bitwise shift
                {
                if (!ci.intProps.empty())
                    {
                    Utf8String s; s.Sprintf("(%s %s %d)", Pick(ci.intProps).c_str(), Coin() ? "<<" : ">>", RandInt(1, 4));
                    return s;
                    }
                return GenExpression(ci, depth + 1);
                }
            case 24: // COLLATE comparison
                {
                if (!ci.strProps.empty())
                    {
                    static const char* cols[] = {"NOCASE", "RTRIM", "BINARY"};
                    Utf8String s; s.Sprintf("%s COLLATE %s = %s", Pick(ci.strProps).c_str(), cols[RandInt(0, 2)], RandStringLiteral().c_str());
                    return s;
                    }
                return GenExpression(ci, depth + 1);
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

    // --- Generator method groups (included to keep the file manageable) ---
#include "ECSqlFuzzSelectGenerators.h"
#include "ECSqlFuzzWriteGenerators.h"

    //======================================================================
    // Random statement of any type (expanded)
    //======================================================================
    Utf8String GenAnyStatement()
        {
        switch (RandInt(0, 29))
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
            case 17: return GenSelectNaturalJoin();
            case 18: return GenSelectBitwise();
            case 19: return GenSelectCollate();
            case 20: return GenSelectAdvancedWindowFrame();
            case 21: return GenSelectCaseAdvanced();
            case 22: return GenSelectRecursiveCTE();
            case 23: return GenSelectTypePredicate();
            case 24: return GenSelectPropertyPath();
            case 25: return GenSelectAdvancedAggregate();
            case 26: return GenSelectChainedSetOps();
            case 27: return GenSelectWithParameters();
            case 28: return GenSelectLiteralEdgeCases();
            default: return GenSelectSimple();
            }
        }

#include "ECSqlFuzzMutationEngine.h"
};

END_ECDBUNITTESTS_NAMESPACE
