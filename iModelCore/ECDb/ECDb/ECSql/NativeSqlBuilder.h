/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "../ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlParameter;

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct NativeSqlBuilder final
    {
    public:
        typedef std::vector<NativeSqlBuilder> List;
        typedef std::vector<List> ListOfLists;

    private:
        Utf8String m_nativeSql;
        std::vector<Utf8String> m_stack;

    public:
        NativeSqlBuilder() {}
        explicit NativeSqlBuilder(Utf8CP initialStr) : m_nativeSql(initialStr) {}
        explicit NativeSqlBuilder(Utf8StringCR initialStr) : m_nativeSql(initialStr) {}
        ~NativeSqlBuilder() {}
        NativeSqlBuilder(NativeSqlBuilder const& rhs) : m_nativeSql(rhs.m_nativeSql) {}
        NativeSqlBuilder& operator= (NativeSqlBuilder const& rhs);
        NativeSqlBuilder(NativeSqlBuilder&& rhs) : m_nativeSql(std::move(rhs.m_nativeSql)) {}
        NativeSqlBuilder& operator= (NativeSqlBuilder&& rhs);

        NativeSqlBuilder& Append(NativeSqlBuilder const& builder) { return Append(builder.GetSql()); }
        NativeSqlBuilder& Append(Utf8StringCR str) { m_nativeSql.append(str); return *this; }
        NativeSqlBuilder& Append(Utf8CP str) { m_nativeSql.append(str); return *this; }

        void Push();
        Utf8String Pop();

        //!@param[in] separator The separator used to separate the items in @p builderList. If nullptr is passed,
        //!                     the items will be separated by comma.
        NativeSqlBuilder& Append(List const& builderList, Utf8CP separator = nullptr);
        NativeSqlBuilder& AppendFormatted(Utf8CP format, ...);
        NativeSqlBuilder& AppendFullyQualified(Utf8CP qualifier, Utf8CP identifier);
        NativeSqlBuilder& AppendFullyQualified(Utf8StringCR qualifier, Utf8StringCR identifier);
        NativeSqlBuilder& AppendEscaped(Utf8StringCR identifier) { return Append("[").Append(identifier).Append("]"); }
        NativeSqlBuilder& AppendEscaped(Utf8CP identifier) { return Append("[").Append(identifier).Append("]"); }
        NativeSqlBuilder& AppendQuoted(Utf8CP stringLiteral) { return Append("'").Append(stringLiteral).Append("'"); }
        NativeSqlBuilder& Append(ECN::ECClassId id);
        NativeSqlBuilder& AppendSpace() { return Append(" "); }
        NativeSqlBuilder& AppendComma() { return Append(","); }
        NativeSqlBuilder& AppendDot() { return Append("."); }
        NativeSqlBuilder& AppendParenLeft() { return Append("("); }
        NativeSqlBuilder& AppendParenRight() { return Append(")"); }
        NativeSqlBuilder& AppendIf(bool condition, Utf8CP stringLiteral) { if (condition) Append(stringLiteral); return *this; }
        NativeSqlBuilder& AppendIIf(bool condition, Utf8CP trueStr, Utf8CP falseStr) { return condition ? Append(trueStr) : Append(falseStr); }

        void Clear() { m_nativeSql.clear(); m_stack.clear(); }
        bool IsEmpty() const { return m_nativeSql.empty(); }
        Utf8StringCR GetSql() const { return m_nativeSql; }

        static List FlattenJaggedList(ListOfLists const& listOfLists, std::vector<size_t> const& indexSkipList);
    };


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SqlUpdateBuilder final
    {
private:
    Utf8String m_table;
    std::vector<std::pair<Utf8String, ECN::ECValue>> m_updateExpressions;
    std::vector<std::pair<Utf8String, ECN::ECValue>> m_whereExpressions;

    BentleyStatus Bind(BeSQLite::Statement&, int paramIndex, Utf8CP columnName, ECN::ECValueCR) const;

public:
    explicit SqlUpdateBuilder(Utf8CP table) : m_table(table) {}

    bool IsValid() const { return !m_table.empty() && !m_updateExpressions.empty(); }
    void AddSetExp(Utf8CP columnName, Utf8CP value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddSetExp(Utf8CP columnName, double value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddSetExp(Utf8CP columnName, bool value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddSetExp(Utf8CP columnName, uint32_t value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue((int32_t) value))); }
    void AddSetExp(Utf8CP columnName, uint64_t value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddSetExp(Utf8CP columnName, int32_t value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddSetExp(Utf8CP columnName, int64_t value) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddSetToNull(Utf8CP columnName) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue())); }
    void AddWhereExp(Utf8CP columnName, uint64_t value) { m_whereExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddWhereExp(Utf8CP columnName, int64_t value) { m_whereExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }
    void AddWhereExp(Utf8CP columnName, int32_t value) { m_whereExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }

    BentleyStatus ExecuteSql(ECDb const&) const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
