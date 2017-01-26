/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SelectStatementExp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "JoinExp.h"
#include "OptionsExp.h"
#include "PropertyNameExp.h"
#include "WhereExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
enum class SubqueryTestOperator
    {
    Unique,
    Exists
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
enum class NonJoinQueryOperator
    {
    Union,
    Intersect,
    Except
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct DerivedPropertyExp final : Exp
    {
    private:
        Utf8String m_columnAlias;
        Utf8String m_nestedAlias;

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override
            {
            Utf8String str("DerivedProperty [Column alias: ");
            str.append(m_columnAlias).append("]");
            return str;
            }

    public:
        DerivedPropertyExp(std::unique_ptr<ValueExp> valueExp, Utf8CP columnAlias);

        ValueExp const* GetExpression() const { return GetChild<ValueExp>(0); }
        Utf8String GetName() const;
        Utf8StringCR GetColumnAlias() const;
        Utf8StringCR GetNestedAlias() const { return m_nestedAlias; }
        void SetNestedAlias(Utf8CP nestedAlias) { m_nestedAlias = nestedAlias; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct FromExp final : Exp
    {
    private:
        void FindRangeClassRefs(RangeClasssInfo::List&, ClassRefExp const&,RangeClasssInfo::Scope) const;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "FromClause"; }

    public:
        FromExp() : Exp(Type::FromClause) {}

        BentleyStatus TryAddClassRef(ECSqlParseContext&, std::unique_ptr<ClassRefExp>);

        RangeClasssInfo::List FindRangeClassRefExpressions() const;

        //-----------------------------------------------------------------------------------------
        // @bsimethod                                    Affan.Khan                       05/2013
        //For subquery it return "" if subquery has no alias
        //+---------------+---------------+---------------+---------------+---------------+------
        void FindRangeClassRefs(RangeClasssInfo::List&, RangeClasssInfo::Scope scope = RangeClasssInfo::Scope::Local) const;
    };

struct ValueExpListExp;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct GroupByExp final : Exp
    {
    private:
        size_t m_groupingValueListExpIndex;

        Exp::FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "GroupBy"; }

    public:
        explicit GroupByExp(std::unique_ptr<ValueExpListExp>);
        ~GroupByExp() {}

        ValueExpListExp const* GetGroupingValueListExp() const;
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct HavingExp final : Exp
    {
    private:
        size_t m_searchConditionExpIndex;

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "Having"; }

    public:
        explicit HavingExp(std::unique_ptr<BooleanExp> searchConditionExp);
        ~HavingExp() {}

        BooleanExp const* GetSearchConditionExp() const;
    };

//=======================================================================================
//! @bsiclass                                              Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct LimitOffsetExp final : Exp
    {
    private:
        size_t m_limitExpIndex;
        int m_offsetExpIndex;

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "LimitOffset"; }

        Exp::FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
        static bool IsValidChildExp(ValueExp const&);
    public:
        explicit LimitOffsetExp(std::unique_ptr<ValueExp> limit, std::unique_ptr<ValueExp> offset = nullptr);
        ~LimitOffsetExp() {}

        ValueExp const* GetLimitExp() const;
        bool HasOffset() const;
        ValueExp const* GetOffsetExp() const;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      08/2013
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
        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override;

        void AppendSortDirection(Utf8String& str, bool addLeadingBlank) const;
    public:
        OrderBySpecExp(std::unique_ptr<ComputedExp>& expr, SortDirection direction);
        ComputedExp const* GetSortExpression() const { return GetChild<ComputedExp>(0); }
        SortDirection GetSortDirection() const { return m_direction; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct OrderByExp final : Exp
    {
    private:
        Utf8String _ToECSql() const override
            {
            Utf8String ecsql = "ORDER BY ";

            bool isFirstItem = true;
            for (auto childExp : GetChildren())
                {
                if (!isFirstItem)
                    ecsql.append(", ");

                ecsql.append(childExp->ToECSql());
                isFirstItem = false;
                }

            return ecsql;
            }
        Utf8String _ToString() const override { return "OrderBy"; }

    public:
        explicit OrderByExp(std::vector<std::unique_ptr<OrderBySpecExp>>& specs)
            : Exp(Type::OrderBy)
            {
            for (auto& spec : specs)
                AddChild(move(spec));
            }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SelectClauseExp final : Exp
    {
    private:
        BentleyStatus ReplaceAsteriskExpression(DerivedPropertyExp const& asteriskExp, RangeClasssInfo::List const&);
        BentleyStatus ReplaceAsteriskExpressions(RangeClasssInfo::List const&);

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "Selection (aka SelectClause)"; }

    public:
        SelectClauseExp() : Exp(Type::Selection) {}

        void AddProperty(std::unique_ptr<DerivedPropertyExp> propertyExp)
            {
            AddChild(std::move(propertyExp));
            }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct QueryExp : Exp
    {
    protected:
        explicit QueryExp(Type type) : Exp(type) {}

        virtual DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const = 0;
        virtual SelectClauseExp const* _GetSelection() const = 0;

    public:
        virtual ~QueryExp() {}

        DerivedPropertyExp const* FindProperty(Utf8CP propertyName) const { return _FindProperty(propertyName); }
        SelectClauseExp const* GetSelection() const { return _GetSelection(); }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SingleSelectStatementExp final : QueryExp
    {
    private:
        SqlSetQuantifier m_selectionType;
        size_t m_fromClauseIndex;
        size_t m_selectClauseIndex;
        int m_whereClauseIndex;
        int m_orderByClauseIndex;
        int m_groupByClauseIndex;
        int m_havingClauseIndex;
        int m_limitOffsetClauseIndex;
        int m_optionsClauseIndex;

        RangeClasssInfo::List m_finalizeParsingArgCache;

        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override;

    protected:
        DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const override;
        SelectClauseExp const* _GetSelection() const override { return GetChild<SelectClauseExp>(m_selectClauseIndex); }

    public:
        SingleSelectStatementExp(SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp>, std::unique_ptr<FromExp>, std::unique_ptr<WhereExp>,
                                 std::unique_ptr<OrderByExp>, std::unique_ptr<GroupByExp>, std::unique_ptr<HavingExp>, std::unique_ptr<LimitOffsetExp> limitOffsetExp, std::unique_ptr<OptionsExp>);

        FromExp const* GetFrom() const { return GetChild<FromExp>(m_fromClauseIndex); }

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
        
        bool IsCoreSelect() const { return GetLimitOffset() == nullptr && GetOrderBy() == nullptr; }
        SqlSetQuantifier GetSelectionType() const { return m_selectionType; }
    };

//********* QueryExp subclasses ***************************
//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SelectStatementExp;
struct SubqueryExp final : QueryExp
    {
    private:
        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "Subquery"; }

    protected:
        DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const override;
        SelectClauseExp const* _GetSelection() const override;
    public:
        explicit SubqueryExp(std::unique_ptr<SelectStatementExp>);
        SelectStatementExp const* GetQuery() const { return GetChild<SelectStatementExp>(0); }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
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

        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override { return "SelectStatementExp"; }
        DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const override { return GetFirstStatement().FindProperty(propertyName); }
        SelectClauseExp const* _GetSelection() const override { return GetFirstStatement().GetSelection(); }

        static Utf8CP OperatorToString(CompoundOperator);

    public:
        explicit SelectStatementExp(std::unique_ptr<SingleSelectStatementExp>);
        SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs, CompoundOperator, bool isAll, std::unique_ptr<SelectStatementExp> rhs);
        virtual ~SelectStatementExp() {}

        bool IsCompound() const { return m_operator != CompoundOperator::None; }
        SingleSelectStatementExp const& GetFirstStatement() const { return *GetChild<SingleSelectStatementExp>(m_firstSingleSelectStatementExpIndex); }
        SelectStatementExp const* GetRhsStatement() const;
        bool IsAll()const { return m_isAll; }
        CompoundOperator GetOperator() const { return m_operator; }
    };


//********* SubqueryRefExp ***************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryRefExp final : RangeClassRefExp
    {
    friend struct ECSqlParser;
    private:
        Utf8StringCR _GetId() const override { return GetAlias(); }
        bool _ContainProperty(Utf8CP propertyName) const override { return GetSubquery()->GetQuery()->FindProperty(propertyName) != nullptr; }
        BentleyStatus _CreatePropertyNameExpList(std::function<void(std::unique_ptr<PropertyNameExp>&)> addDelegate) const override;
        Utf8String _ToECSql() const override;
        Utf8String _ToString() const override;

    public:
        SubqueryRefExp(std::unique_ptr<SubqueryExp>, Utf8StringCR alias, bool isPolymorphic);
        SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp>(0); }
    };

//******************* Select related boolean exp *******************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct AllOrAnyExp final : BooleanExp
    {
    private:
        SqlCompareListType m_type;
        BooleanSqlOperator m_operator;
        size_t m_operandExpIndex;
        size_t m_subqueryExpIndex;

        void _DoToECSql(Utf8StringR ecsql) const override;
        Utf8String _ToString() const override;

    public:
        AllOrAnyExp(std::unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery);

        ValueExp const* GetOperand() const { return GetChild<ValueExp>(m_operandExpIndex); }
        SqlCompareListType GetCompareType() const { return m_type; }
        SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp>(m_subqueryExpIndex); }
        BooleanSqlOperator GetOperator() const { return m_operator; }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryTestExp final : BooleanExp
    {
    private:
        SubqueryTestOperator m_op;

        void _DoToECSql(Utf8StringR ecsql) const override;
        Utf8String _ToString() const override;

    public:
        SubqueryTestExp(SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery);

        QueryExp const* GetQuery() const { return GetChild<QueryExp>(0); }
        SubqueryTestOperator  GetOperator() const { return m_op; }
    };

//******************* Select related value exp *******************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryValueExp final : ValueExp
    {
    private:
        FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
        void _DoToECSql(Utf8StringR ecsql) const override { ecsql.append(GetQuery()->ToECSql()); }
        Utf8String _ToString() const override { return "SubqueryValue"; }

    public:
        explicit SubqueryValueExp(std::unique_ptr<SubqueryExp>);

        SubqueryExp const* GetQuery() const { return GetChild<SubqueryExp>(0); }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

