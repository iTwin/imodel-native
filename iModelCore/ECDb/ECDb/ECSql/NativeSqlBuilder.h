/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NativeSqlBuilder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "../ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlParameter;

//=======================================================================================
// @bsiclass                                             Krischan.Eberle    08/2013
//+===============+===============+===============+===============+===============+======
struct NativeSqlBuilder
    {
    public:
        struct ECSqlParameterIndex
            {
        private:
            int m_index;
            int m_componentIndex;

        public:
            ECSqlParameterIndex(int index, int componentIndex) : m_index(index), m_componentIndex(componentIndex) {}
            int GetIndex() const { return m_index; }
            int GetComponentIndex() const { return m_componentIndex; }
            };

        typedef std::vector<NativeSqlBuilder> List;
        typedef std::vector<List> ListOfLists;

    private:
        Utf8String m_nativeSql;
        std::vector<ECSqlParameterIndex> m_parameterIndexMappings;
        std::vector<Utf8String> m_stack;

    public:
        NativeSqlBuilder() {}
        explicit NativeSqlBuilder(Utf8CP initialStr) : m_nativeSql(initialStr) {}
        ~NativeSqlBuilder() {}
        NativeSqlBuilder(NativeSqlBuilder const& rhs) : m_nativeSql(rhs.m_nativeSql), m_parameterIndexMappings(rhs.m_parameterIndexMappings) {}
        NativeSqlBuilder& operator= (NativeSqlBuilder const& rhs);
        NativeSqlBuilder(NativeSqlBuilder&& rhs) : m_nativeSql(std::move(rhs.m_nativeSql)), m_parameterIndexMappings(std::move(rhs.m_parameterIndexMappings)) {}
        NativeSqlBuilder& operator= (NativeSqlBuilder&& rhs);

        NativeSqlBuilder& CopyParameters(NativeSqlBuilder const& sqlBuilder);

        NativeSqlBuilder& Append(NativeSqlBuilder const& builder);
        NativeSqlBuilder& Append(Utf8CP str) { m_nativeSql.append(str); return *this; }

        void Push(bool clear = true);
        Utf8String Pop();

        //!@param[in] separator The separator used to separate the items in @p builderList. If nullptr is passed,
        //!                     the items will be separated by comma.
        NativeSqlBuilder& Append(List const& builderList, Utf8CP separator = nullptr);

        NativeSqlBuilder& Append(List const& lhsBuilderList, Utf8CP operatorStr, List const& rhsBuilderList, Utf8CP separator = nullptr);
        NativeSqlBuilder& AppendFormatted(Utf8CP format, ...);
        NativeSqlBuilder& Append(Utf8CP classIdentifier, Utf8CP identifier);
        NativeSqlBuilder& Append(BinarySqlOperator);
        NativeSqlBuilder& Append(BooleanSqlOperator);
        NativeSqlBuilder& Append(SqlSetQuantifier);
        NativeSqlBuilder& Append(UnarySqlOperator);
        //!@param[in] ecsqlParameterName Parameter name of nullptr if parameter is unnamed
        NativeSqlBuilder& AppendParameter(Utf8CP ecsqlParameterName, int ecsqlParameterIndex, int ecsqlParameterComponentIndex);
        //!@param[in] ecsqlParameterName Parameter name of nullptr if parameter is unnamed
        NativeSqlBuilder& AppendParameter(Utf8CP ecsqlParameterName, int ecsqlParameterComponentIndex);

        NativeSqlBuilder& AppendEscaped(Utf8CP identifier) { return Append("[").Append(identifier).Append("]"); }

        NativeSqlBuilder& Append(Utf8CP identifier, bool escape) { return escape ? AppendEscaped(identifier) : Append(identifier); };
        NativeSqlBuilder& AppendQuoted(Utf8CP stringLiteral) { return Append("'").Append(stringLiteral).Append("'"); }

        NativeSqlBuilder& AppendSpace() { return Append(" "); }
        NativeSqlBuilder& AppendComma() { return Append(","); }
        NativeSqlBuilder& AppendDot() { return Append("."); }
        NativeSqlBuilder& AppendParenLeft() { return Append("("); }
        NativeSqlBuilder& AppendParenRight() { return Append(")"); }
        NativeSqlBuilder& AppendLine(Utf8CP str) { return Append(str).AppendEol(); }
        NativeSqlBuilder& AppendEol() { return Append("\n"); }
        NativeSqlBuilder& AppendIf(bool condition, Utf8CP stringLiteral) { if (condition) Append(stringLiteral); return *this; }
        NativeSqlBuilder& AppendIIf(bool condition, Utf8CP trueStr, Utf8CP falseStr) { return condition ? Append(trueStr) : Append(falseStr); }

        void Reset();
        bool IsEmpty() const { return m_nativeSql.empty(); }
        Utf8CP ToString() const { return m_nativeSql.c_str(); }

        std::vector<ECSqlParameterIndex> const& GetParameterIndexMappings() const { return m_parameterIndexMappings; }

        static List FlattenJaggedList(ListOfLists const& listOfLists, std::vector<size_t> const& indexSkipList);
    };


//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SqlUpdateBuilder
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
    void AddSetExp(Utf8CP columnName) { m_updateExpressions.push_back(std::make_pair(columnName, ECN::ECValue())); }
    void AddWhereExp(Utf8CP columnName, int64_t value) { m_whereExpressions.push_back(std::make_pair(columnName, ECN::ECValue(value))); }

    BentleyStatus ExecuteSql(ECDb const&) const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
