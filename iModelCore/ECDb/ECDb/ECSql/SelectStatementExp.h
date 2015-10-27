/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SelectStatementExp.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
struct DerivedPropertyExp : Exp
    {
DEFINE_EXPR_TYPE(DerivedProperty) 

private:
    Utf8String m_columnAlias;
    Utf8String m_nestedAlias;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override
        {
        Utf8String str("DerivedProperty [Column alias: ");
        str.append(m_columnAlias).append("]");
        return str;
        }

public:
    DerivedPropertyExp (std::unique_ptr<ValueExp> valueExp, Utf8CP columnAlias);

    ValueExp const* GetExpression () const { return GetChild<ValueExp> (0);}
    Utf8String GetName () const;
    Utf8StringCR GetColumnAlias () const;
    Utf8StringCR GetNestedAlias () const { return m_nestedAlias; }
    void SetNestedAlias (Utf8CP nestedAlias) { m_nestedAlias = nestedAlias; }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct FromExp : Exp
    {
DEFINE_EXPR_TYPE(FromClause) 
private:
    void FindRangeClassRefs(RangeClassRefList&, ClassRefExp const&) const;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "FromClause"; }

public:
    FromExp () : Exp () {}

    BentleyStatus TryAddClassRef(ECSqlParseContext&, std::unique_ptr<ClassRefExp>);

    std::unique_ptr<RangeClassRefList> FindRangeClassRefExpressions () const;

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                       05/2013
    //For subquery it return "" if subquery has no alias
    //+---------------+---------------+---------------+---------------+---------------+------
    RangeClassRefExp const* FindRangeClassRefById (Utf8StringCR id) const;
    void FindRangeClassRefs(RangeClassRefList&) const;
    };

struct ValueExpListExp;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct GroupByExp : Exp
    {
    DEFINE_EXPR_TYPE(GroupBy) 
private:
    size_t m_groupingValueListExpIndex;

    virtual Exp::FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "GroupBy"; }
    
public:
    explicit GroupByExp(std::unique_ptr<ValueExpListExp>);
    ~GroupByExp() {}

    ValueExpListExp const* GetGroupingValueListExp() const;
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct HavingExp : Exp
    {
    DEFINE_EXPR_TYPE(Having) 
private:
    size_t m_searchConditionExpIndex;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "Having"; }

public:
    explicit HavingExp(std::unique_ptr<BooleanExp> searchConditionExp);
    ~HavingExp() {}

    BooleanExp const* GetSearchConditionExp() const;
    };

//=======================================================================================
//! @bsiclass                                              Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct LimitOffsetExp : Exp
    {
DEFINE_EXPR_TYPE(LimitOffset) 
private:
    size_t m_limitExpIndex;
    int m_offsetExpIndex;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "LimitOffset"; }

    virtual Exp::FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;
    static bool IsValidChildExp(ValueExp const&);
public:
    explicit LimitOffsetExp (std::unique_ptr<ValueExp> limit, std::unique_ptr<ValueExp> offset = nullptr);
    ~LimitOffsetExp () {}

    ValueExp const* GetLimitExp () const;
    bool HasOffset () const;
    ValueExp const* GetOffsetExp () const;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      08/2013
//+===============+===============+===============+===============+===============+======
struct OrderBySpecExp : Exp
    {
DEFINE_EXPR_TYPE(OrderBySpec) 

public:
    enum class SortDirection
        {
        Ascending,
        Descending,
        NotSpecified
        };
private:
    SortDirection m_direction;

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override;

    void AppendSortDirection (Utf8String& str, bool addLeadingBlank) const;
public:
    OrderBySpecExp(std::unique_ptr<ComputedExp>& expr, SortDirection direction);
    ComputedExp const* GetSortExpression() const { return GetChild<ComputedExp>(0);}
    SortDirection GetSortDirection () const { return m_direction;} 
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct OrderByExp : Exp
    {
DEFINE_EXPR_TYPE(OrderBy) 

private:
    virtual Utf8String _ToECSql() const override
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
    virtual Utf8String _ToString() const override { return "OrderBy"; }

public:
    explicit OrderByExp(std::vector<std::unique_ptr<OrderBySpecExp>>& specs)
        {
        for(auto& spec : specs)
            AddChild(move(spec));
        }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SelectClauseExp : Exp
    {
DEFINE_EXPR_TYPE(Selection) 
private:
    BentleyStatus ReplaceAsteriskExpression (DerivedPropertyExp const& asteriskExp, RangeClassRefList const&);
    BentleyStatus ReplaceAsteriskExpressions (RangeClassRefList const&);

    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "Selection (aka SelectClause)"; }

public:
    void AddProperty(std::unique_ptr<DerivedPropertyExp> propertyExp)
        {
        AddChild (std::move(propertyExp));
        }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct QueryExp : Exp
    {    
DEFINE_EXPR_TYPE(Query) 
protected:
    QueryExp () : Exp () {}

    virtual DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const = 0;
    virtual SelectClauseExp const* _GetSelection() const =0;

public:
    virtual ~QueryExp() {}

    DerivedPropertyExp const* FindProperty(Utf8CP propertyName) const { return _FindProperty(propertyName); }
    virtual SelectClauseExp const* GetSelection() const { return _GetSelection(); }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SingleSelectStatementExp : QueryExp
    {
    DEFINE_EXPR_TYPE(SingleSelect)
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

        std::unique_ptr<RangeClassRefList> m_finalizeParsingArgCache;

        virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;

        virtual Utf8String _ToECSql() const override;
        virtual Utf8String _ToString() const override;

    protected:
        virtual DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const override;
        virtual SelectClauseExp const* _GetSelection() const override { return GetChild<SelectClauseExp>(m_selectClauseIndex); }

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
struct SubqueryExp : QueryExp
    {
    DEFINE_EXPR_TYPE(Subquery)
private:
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString() const override { return "Subquery"; }

protected:
    virtual DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const override;
    virtual SelectClauseExp const* _GetSelection () const override;
public:
    explicit SubqueryExp (std::unique_ptr<SelectStatementExp> selectExp);
    SelectStatementExp const* GetQuery () const { return GetChild<SelectStatementExp> (0); }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SelectStatementExp : QueryExp
    {
    DEFINE_EXPR_TYPE(Select)
    enum class Operator
        {
        None, //TopLevel/First
        Union,
        Intersect,
        Except
        };

    private:
        Operator m_operator;
        bool m_isAll;

        virtual Utf8String _ToECSql() const override;
        virtual Utf8String _ToString() const override;
        virtual DerivedPropertyExp const* _FindProperty(Utf8CP propertyName) const override;
        virtual SelectClauseExp const* _GetSelection() const override;

    public:
        SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs);
        SelectStatementExp(std::unique_ptr<SingleSelectStatementExp> lhs, Operator op, bool isAll, std::unique_ptr<SelectStatementExp> rhs);
        virtual ~SelectStatementExp(){}
        SingleSelectStatementExp const& GetCurrent() const;
        SelectStatementExp const* GetNext() const;
        SelectStatementExp const* GetPrevious() const;
        SelectStatementExp const& GetLast() const;
        SelectStatementExp const& GetFirst() const;
        bool IsAll()const;
        Operator GetOP() const;
        bool IsCompound() const;
        static Utf8CP OPToString(Operator op);
    };


//********* SubqueryRefExp ***************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryRefExp : RangeClassRefExp
    {
    friend struct ECSqlParser;
    DEFINE_EXPR_TYPE(SubqueryRef) 
private:
    virtual Utf8StringCR _GetId() const override { return GetAlias(); }
    virtual bool _ContainProperty(Utf8CP propertyName) const override { return GetSubquery ()->GetQuery()->FindProperty(propertyName) != nullptr; }
    virtual BentleyStatus _CreatePropertyNameExpList (std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const override;
    virtual Utf8String _ToECSql() const override;
    virtual Utf8String _ToString () const override;

public:
    SubqueryRefExp(std::unique_ptr<SubqueryExp>, Utf8StringCR alias, bool isPolymorphic);
    SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp> (0);}
    };

//******************* Select related boolean exp *******************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct AllOrAnyExp : BooleanExp
    {
DEFINE_EXPR_TYPE(AllOrAny)
private:
    SqlCompareListType m_type;
    BooleanSqlOperator m_operator;
    size_t m_operandExpIndex;
    size_t m_subqueryExpIndex;

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

public:
    AllOrAnyExp(std::unique_ptr<ValueExp> operand, BooleanSqlOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery);

    ValueExp const* GetOperand() const { return GetChild<ValueExp> (m_operandExpIndex);}
    SqlCompareListType GetCompareType() const { return m_type;}
    SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp> (m_subqueryExpIndex);}
    BooleanSqlOperator GetOperator() const { return m_operator;}
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryTestExp : BooleanExp
    {
DEFINE_EXPR_TYPE(SubqueryTest) 

private:
    SubqueryTestOperator m_op;

    virtual void _DoToECSql(Utf8StringR ecsql) const override;
    virtual Utf8String _ToString() const override;

public:
    SubqueryTestExp(SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery);

    QueryExp const* GetQuery() const { return GetChild<QueryExp> (0);}
    SubqueryTestOperator  GetOperator() const { return m_op;}
    };

//******************* Select related value exp *******************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(SubqueryValue)

private:
    virtual FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode) override;
    virtual void _DoToECSql(Utf8StringR ecsql) const override { ecsql.append(GetQuery()->ToECSql()); }
    virtual Utf8String _ToString() const override { return "SubqueryValue"; }

public:
    explicit SubqueryValueExp (std::unique_ptr<SubqueryExp> subquery);

    SubqueryExp const* GetQuery() const {return GetChild<SubqueryExp> (0);}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

