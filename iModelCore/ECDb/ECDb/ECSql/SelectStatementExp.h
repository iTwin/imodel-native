/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassRefExp.h"
#include "JoinExp.h"
#include "ListExp.h"
#include "OptionsExp.h"
#include "PropertyNameExp.h"
#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
enum class NonJoinQueryOperator
    {
    Union,
    Intersect,
    Except
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
enum class SqlCompareListType
    {
    All,
    Any,
    Some
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
enum class SubqueryTestOperator
    {
    Unique,
    Exists
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct DerivedPropertyExp final : Exp
    {
    private:
        mutable Utf8String m_columnAlias;
        Utf8String m_nestedAlias;
        mutable Utf8String m_cteColumnName;
        mutable Utf8String m_subQueryAlias;
        std::vector<Utf8String> m_resultSetColumns;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { Utf8String str("DerivedProperty [Column alias: "); str.append(m_columnAlias).append("]"); return str; }
        
    public:
        DerivedPropertyExp(std::unique_ptr<ValueExp> valueExp, Utf8CP columnAlias);

        ValueExp const* GetExpression() const { return GetChild<ValueExp>(0); }
        Utf8String GetName() const;
        
        Utf8StringCR GetColumnAlias() const;
        Utf8StringCR GetCteColumnName() const { return m_cteColumnName; }
        Utf8StringCR GetNestedAlias() const { return m_cteColumnName.empty() ? m_nestedAlias : m_cteColumnName; }
        Utf8String GetAliasRecursively() const;
        void SetNestedAlias(Utf8CP nestedAlias) { m_nestedAlias = nestedAlias; }
        void SetCteColumnName(Utf8CP name) const { m_cteColumnName = name ;}
        std::vector<Utf8String>& SqlResultSetR() { return m_resultSetColumns; }
        std::vector<Utf8String> const& SqlResultSet() { return m_resultSetColumns; }
        bool HasAlias() const { return !m_columnAlias.empty(); }
        void OverrideAlias(Utf8CP str) const {
            BeAssert(m_columnAlias.empty());
            if (m_columnAlias.empty())
                m_columnAlias = str;
        }
        bool IsComputed() const;
        bool OriginateInASubQuery() const { return nullptr != this->FindParent(Exp::Type::Subquery); }
        bool IsWildCard() const;
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct FromExp final : Exp
    {
    private:
        void FindRangeClassRefs(std::vector<RangeClassInfo>&, ClassRefExp const&,RangeClassInfo::Scope) const;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "FromClause"; }

    public:
        FromExp() : Exp(Type::FromClause) {}

        BentleyStatus TryAddClassRef(ECSqlParseContext&, std::unique_ptr<ClassRefExp>);

        std::vector<RangeClassInfo> FindRangeClassRefExpressions() const;

        //-----------------------------------------------------------------------------------------
        // @bsimethod
        //For subquery it return "" if subquery has no alias
        //+---------------+---------------+---------------+---------------+---------------+------
        void FindRangeClassRefs(std::vector<RangeClassInfo>&, RangeClassInfo::Scope scope = RangeClassInfo::Scope::Local) const;
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct GroupByExp final : Exp
    {
    private:
        size_t m_groupingValueListExpIndex;

        Exp::FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext& ctx) const override { ctx.AppendToECSql("GROUP BY ").AppendToECSql(*GetGroupingValueListExp()); }
        Utf8String _ToString() const override { return "GroupBy"; }

    public:
        explicit GroupByExp(std::unique_ptr<ValueExpListExp> groupingValueListExp) : Exp(Type::GroupBy) { m_groupingValueListExpIndex = AddChild(std::move(groupingValueListExp)); }
        ~GroupByExp() {}

        ValueExpListExp const* GetGroupingValueListExp() const { return GetChild<ValueExpListExp>(m_groupingValueListExpIndex); }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct HavingExp final : Exp
    {
    private:
        size_t m_searchConditionExpIndex;

        void _ToECSql(ECSqlRenderContext& ctx) const override { ctx.AppendToECSql("HAVING ").AppendToECSql(*GetSearchConditionExp()); }
        Utf8String _ToString() const override { return "Having"; }

    public:
        explicit HavingExp(std::unique_ptr<BooleanExp> searchConditionExp) : Exp(Type::Having) { m_searchConditionExpIndex = AddChild(std::move(searchConditionExp)); }
        ~HavingExp() {}

        BooleanExp const* GetSearchConditionExp() const { return GetChild<BooleanExp>(m_searchConditionExpIndex); }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct LimitOffsetExp final : Exp
    {
    private:
        size_t m_limitExpIndex;
        int m_offsetExpIndex = UNSET_CHILDINDEX;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "LimitOffset"; }

        Exp::FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        static bool IsValidChildExp(ValueExp const&);
    public:
        explicit LimitOffsetExp(std::unique_ptr<ValueExp> limit, std::unique_ptr<ValueExp> offset = nullptr);
        ~LimitOffsetExp() {}

        ValueExp const* GetLimitExp() const { return GetChild<ValueExp>(m_limitExpIndex); }
        bool HasOffset() const { return m_offsetExpIndex >= 0; }
        ValueExp const* GetOffsetExp() const;
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct OrderBySpecExp final : Exp
    {
    public:
        enum class SortDirection
            {
            Ascending,
            Descending,
            NotSpecified
            };
    private:
        SortDirection m_direction;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        OrderBySpecExp(std::unique_ptr<ComputedExp>& expr, SortDirection direction) : Exp(Type::OrderBySpec), m_direction(direction) { AddChild(std::move(expr)); }
        ComputedExp const* GetSortExpression() const { return GetChild<ComputedExp>(0); }
        SortDirection GetSortDirection() const { return m_direction; }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SingleSelectStatementExp;
struct OrderByExp final : Exp
    {
    private:
        std::vector<SingleSelectStatementExp const*> m_unionClauses;

        void _ToECSql(ECSqlRenderContext& ctx) const override;
        Utf8String _ToString() const override { return "OrderBy"; }
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        ComputedExp const* FindIncompatibleOrderBySpecExpForUnion() const;

    public:
        explicit OrderByExp(std::vector<std::unique_ptr<OrderBySpecExp>>& specs) : Exp(Type::OrderBy)
            {
            for (auto& spec : specs)
                AddChild(move(spec));
            }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SelectClauseExp final : Exp
    {
    private:
        BentleyStatus ReplaceAsteriskExpression(ECSqlParseContext const&, DerivedPropertyExp const& asteriskExp, std::vector<RangeClassInfo> const&);
        BentleyStatus ReplaceAsteriskExpressions(ECSqlParseContext const&, std::vector<RangeClassInfo> const&);

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "Selection (aka SelectClause)"; }

    public:
        SelectClauseExp() : Exp(Type::Selection) {}

        void AddProperty(std::unique_ptr<DerivedPropertyExp> propertyExp) { AddChild(std::move(propertyExp)); }
    };

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct QueryExp : Exp
    {
    protected:
        explicit QueryExp(Type type) : Exp(type) {}

        virtual SelectClauseExp const* _GetSelection() const = 0;
        virtual PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const& propertyPath, const PropertyMatchOptions& options) const = 0;
    public:
        virtual ~QueryExp() {}

        PropertyMatchResult FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const { return _FindProperty(ctx, propertyPath, options); }
        SelectClauseExp const* GetSelection() const { return _GetSelection(); }
    };


//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SingleSelectStatementExp final : QueryExp
    {
    private:
        SqlSetQuantifier m_selectionType = SqlSetQuantifier::NotSpecified;
        size_t m_selectClauseIndex;
        int m_fromClauseIndex = UNSET_CHILDINDEX;
        int m_whereClauseIndex = UNSET_CHILDINDEX;
        int m_orderByClauseIndex = UNSET_CHILDINDEX;
        int m_groupByClauseIndex = UNSET_CHILDINDEX;
        int m_havingClauseIndex = UNSET_CHILDINDEX;
        int m_limitOffsetClauseIndex = UNSET_CHILDINDEX;
        int m_optionsClauseIndex = UNSET_CHILDINDEX;
        std::vector<RangeClassInfo> m_rangeClassRefExpCache;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    protected:
        PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override;
        SelectClauseExp const* _GetSelection() const override { return GetChild<SelectClauseExp>(m_selectClauseIndex); }

    public:
        SingleSelectStatementExp(SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp>, std::unique_ptr<FromExp>, std::unique_ptr<WhereExp>,
                                 std::unique_ptr<OrderByExp>, std::unique_ptr<GroupByExp>, std::unique_ptr<HavingExp>, std::unique_ptr<LimitOffsetExp> limitOffsetExp, std::unique_ptr<OptionsExp>);

        explicit SingleSelectStatementExp(std::vector<std::unique_ptr<ValueExp>>&);
        bool IsRowConstructor() const { return m_fromClauseIndex == UNSET_CHILDINDEX;}

        SqlSetQuantifier GetSelectionType() const { return m_selectionType; }
        FromExp const* GetFrom() const 
            { 
            if (m_fromClauseIndex < 0)
                return nullptr;

            return GetChild<FromExp>((size_t) m_fromClauseIndex); 
            }

        WhereExp const* GetWhere() const
            {
            if (m_whereClauseIndex < 0)
                return nullptr;

            return GetChild<WhereExp>((size_t) m_whereClauseIndex);
            }

        OrderByExp const* GetOrderBy() const
            {
            if (m_orderByClauseIndex < 0)
                return nullptr;

            return GetChild<OrderByExp>((size_t) m_orderByClauseIndex);
            }

        GroupByExp const* GetGroupBy() const
            {
            if (m_groupByClauseIndex < 0)
                return nullptr;

            return GetChild<GroupByExp>((size_t) m_groupByClauseIndex);
            }

        HavingExp const* GetHaving() const
            {
            if (m_havingClauseIndex < 0)
                return nullptr;

            return GetChild<HavingExp>((size_t) m_havingClauseIndex);
            }

        LimitOffsetExp const* GetLimitOffset() const
            {
            if (m_limitOffsetClauseIndex < 0)
                return nullptr;

            return GetChild<LimitOffsetExp>((size_t) m_limitOffsetClauseIndex);
            }

        OptionsExp const* GetOptions() const
            {
            if (m_optionsClauseIndex < 0)
                return nullptr;

            return GetChild<OptionsExp>((size_t) m_optionsClauseIndex);
            }
        

        bool IsCoreSelect() const { return m_limitOffsetClauseIndex == UNSET_CHILDINDEX && m_optionsClauseIndex == UNSET_CHILDINDEX; }
    };

//********* QueryExp subclasses ***************************
//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SelectStatementExp;
struct SubqueryExp final : QueryExp
    {
    private:       
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "Subquery"; }

    protected:
        PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override;
        SelectClauseExp const* _GetSelection() const override;
    public:
        explicit SubqueryExp(std::unique_ptr<SelectStatementExp>);
        SelectStatementExp const* GetQuery() const;
    };


//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SelectStatementExp final : QueryExp
    {
        enum class CompoundOperator
        {
        None, //TopLevel/First
        Union,
        Intersect,
        Except
        };

    private:
        size_t m_firstSingleSelectStatementExpIndex;
        int m_rhsSelectStatementExpIndex;
        CompoundOperator m_operator;
        bool m_isAll;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "SelectStatementExp"; }
        PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override{ return GetFirstStatement().FindProperty(ctx, propertyPath, options); }
        SelectClauseExp const* _GetSelection() const override { return GetFirstStatement().GetSelection(); }

        static Utf8CP OperatorToString(CompoundOperator);
        virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext& parseContext, FinalizeParseMode parseMode) override;
    public:
        explicit SelectStatementExp(std::unique_ptr<SingleSelectStatementExp>);
        SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs, CompoundOperator, bool isAll, std::unique_ptr<SelectStatementExp> rhs);
        virtual ~SelectStatementExp() {}

        bool IsCompound() const { return m_operator != CompoundOperator::None; }
        SingleSelectStatementExp const& GetFirstStatement() const { return *GetChild<SingleSelectStatementExp>(m_firstSingleSelectStatementExpIndex); }
        SelectStatementExp const* GetRhsStatement() const;
        bool IsAll()const { return m_isAll; }
        CompoundOperator GetOperator() const { return m_operator; }
        std::vector<SingleSelectStatementExp const*> GetFlatListOfStatements() const {
            std::vector<SingleSelectStatementExp const *> list;
            auto cur = this;
            do {
                list.push_back(&cur->GetFirstStatement());
                cur = cur->GetRhsStatement();
            } while(cur != nullptr);
            return list;
        }
        
    };


//********* SubqueryRefExp ***************************

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SubqueryRefExp final : RangeClassRefExp
    {
    friend struct ECSqlParser;
    private:
        Utf8StringCR _GetId() const override { return GetAlias(); }
	    PropertyMatchResult _FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const override {
            PropertyMatchOptions overrideOptions = options;
            overrideOptions.SetAlias(GetAlias().c_str());
            return GetSubquery()->GetQuery()->FindProperty(ctx, propertyPath, overrideOptions);
            }
        void _ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        SubqueryRefExp(std::unique_ptr<SubqueryExp>, Utf8CP alias, PolymorphicInfo polymorphic);
        SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp>(0); }
    };

//******************* Select related boolean exp *******************************

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct AllOrAnyExp final : BooleanExp
    {
    private:
        SqlCompareListType m_type;
        BooleanSqlOperator m_operator;
        size_t m_operandExpIndex;
        size_t m_subqueryExpIndex;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        AllOrAnyExp(std::unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery);

        ValueExp const* GetOperand() const { return GetChild<ValueExp>(m_operandExpIndex); }
        SqlCompareListType GetCompareType() const { return m_type; }
        SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp>(m_subqueryExpIndex); }
        BooleanSqlOperator GetOperator() const { return m_operator; }
    };


//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SubqueryTestExp final : BooleanExp
    {
    private:
        SubqueryTestOperator m_op;

        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override;

    public:
        SubqueryTestExp(SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery);

        SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp>(0); }
        SubqueryTestOperator  GetOperator() const { return m_op; }
    };

//******************* Select related value exp *******************************

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct SubqueryValueExp final : ValueExp
    {
    private:
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _ToECSql(ECSqlRenderContext&) const override;
        Utf8String _ToString() const override { return "SubqueryValue"; }

    public:
        explicit SubqueryValueExp(std::unique_ptr<SubqueryExp>);

        SubqueryExp const* GetQuery() const { return GetChild<SubqueryExp>(0); }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

