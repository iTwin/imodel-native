/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SelectStatementExp.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "PropertyNameExp.h"
#include "JoinExp.h"
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
    virtual Utf8String _ToString () const override
        {
        Utf8String str ("DerivedProperty [Column alias: ");
        str.append (m_columnAlias).append ("]");
        return str;
        }

private:
    Utf8String m_columnAlias;
public:
    DerivedPropertyExp (std::unique_ptr<ValueExp> valueExp, Utf8CP columnAlias);

    ValueExp const* GetExpression () const { return GetChild<ValueExp> (0);}
    Utf8String GetName () const;
    Utf8StringCR GetColumnAlias () const { return m_columnAlias; }

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct FromExp : Exp
    {
DEFINE_EXPR_TYPE(FromClause) 
private:
    void FindRangeClassRefs(RangeClassRefList& classRefs, ClassRefExp const* classRef) const;

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override
        {
        return "FromClause";
        }

public:
    FromExp ()
        : Exp ()
        {}

    ECSqlStatus TryAddClassRef(ECSqlParseContext&ctx, ClassRefExp* classRef);

    std::unique_ptr<RangeClassRefList> FindRangeClassRefExpressions () const;

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                    Affan.Khan                       05/2013
    //For subquery it return "" if subquery has no alias
    //+---------------+---------------+---------------+---------------+---------------+------
    RangeClassRefExp const* FindRangeClassRefById (Utf8StringCR id) const;
    void FindRangeClassRefs(RangeClassRefList& classRefs) const;

    virtual Utf8String ToECSql() const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct GroupByExp : Exp
    {
    DEFINE_EXPR_TYPE(GroupBy) 
    private:
        virtual Utf8String _ToString () const override
            {
            return "GroupBy";
            }
    public:
        _NOT_IMP_EXPR_TOECSQL_
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct HavingExp : Exp
    {
    DEFINE_EXPR_TYPE(Having) 
    private:
        virtual Utf8String _ToString () const override
            {
            return "Having";
            }

    public:
        _NOT_IMP_EXPR_TOECSQL_
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

    virtual Utf8String _ToString () const override { return "LimitOffset"; }

    virtual Exp::FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    static bool IsValidChildExp (ValueExp const& exp);
public:
    explicit LimitOffsetExp (std::unique_ptr<ValueExp> limit, std::unique_ptr<ValueExp> offset = nullptr);
    ~LimitOffsetExp () {}

    ValueExp const* GetLimitExp () const;
    bool HasOffset () const;
    ValueExp const* GetOffsetExp () const;

    virtual Utf8String ToECSql () const override;
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

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;
    virtual Utf8String _ToString () const override;

    void AppendSortDirection (Utf8String& str, bool addLeadingBlank) const;
public:
    OrderBySpecExp(std::unique_ptr<ComputedExp>& expr, SortDirection direction);
    ComputedExp const* GetSortExpression() const { return GetChild<ComputedExp>(0);}
    SortDirection GetSortDirection () const { return m_direction;} 
    virtual Utf8String ToECSql () const override;
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct OrderByExp : Exp
    {
DEFINE_EXPR_TYPE(OrderBy) 

private:
    virtual Utf8String _ToString () const override
        {
        return "OrderBy";
        }

public:
    OrderByExp(std::vector<std::unique_ptr<OrderBySpecExp>>& specs)
        {
        for(auto& spec : specs)
            AddChild(move(spec));
        }

    virtual Utf8String ToECSql () const override
        {
        Utf8String ecsql = "ORDER BY ";

        bool isFirstItem = true;
        for(auto childExp : GetChildren ())
            {
            if (!isFirstItem)
                ecsql.append (", ");

            ecsql.append(childExp->ToECSql ());
            isFirstItem = false;
            }

        return ecsql;
        }
    };

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SelectClauseExp : Exp
    {
DEFINE_EXPR_TYPE(Selection) 
private:
    ECSqlStatus ReplaceAsteriskExpression (ECSqlParseContext& ctx, DerivedPropertyExp const& asteriskExp, RangeClassRefList const& rangeClassRefs);
    ECSqlStatus ReplaceAsteriskExpressions (ECSqlParseContext& ctx, RangeClassRefList const& rangeClassRefs);

    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override
        {
        return "Selection (aka SelectClause)";
        }

public:
    void AddProperty(std::unique_ptr<DerivedPropertyExp> propertyExp)
        {
        AddChild (std::move(propertyExp));
        }

    virtual Utf8String ToECSql() const override
        {
        Utf8String tmp;
        bool isFirstItem = true;
        for (auto childExp : GetChildren ())
            {
            if (!isFirstItem)
                tmp.append (", ");

            tmp.append (childExp->ToECSql ());
            isFirstItem = false;
            }

        return tmp;
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

    virtual DerivedPropertyExp const* _FindProperty(Utf8StringCR propertyName) const=0;
    virtual SelectClauseExp const* _GetSelection() const =0;

public:
    virtual ~QueryExp() {}

    DerivedPropertyExp const* FindProperty(Utf8StringCR propertyName) const
        {
        return _FindProperty (propertyName);
        }
    virtual SelectClauseExp const* GetSelection() const
        {
        return _GetSelection();
        }
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct SelectStatementExp : QueryExp
    {
DEFINE_EXPR_TYPE(Select) 
private:
    SqlSetQuantifier m_selectionType;
    size_t m_fromClauseIndex;
    size_t m_selectClauseIndex;
    int m_whereClauseIndex;
    int m_orderByClauseIndex;
    int m_groupByClauseIndex;
    int m_havingClauseIndex;
    int m_limitOffsetClauseIndex;

    std::unique_ptr<RangeClassRefList> m_finalizeParsingArgCache;
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override;

protected:
    virtual DerivedPropertyExp const* _FindProperty(Utf8StringCR propertyName) const override;
    virtual SelectClauseExp const* _GetSelection() const override
        {
        return GetChild<SelectClauseExp> (m_selectClauseIndex);
        }

public :
    SelectStatementExp(SqlSetQuantifier selectionType, std::unique_ptr<SelectClauseExp> selection, std::unique_ptr<FromExp> from, std::unique_ptr<WhereExp> where, 
        std::unique_ptr<OrderByExp> orderby, std::unique_ptr<GroupByExp> groupby, std::unique_ptr<HavingExp> having, std::unique_ptr<LimitOffsetExp> limitOffsetExp);

    FromExp const* GetFrom()       const { return GetChild<FromExp> (m_fromClauseIndex);}
    WhereExp const* GetOptWhere()   const 
        { 
        if (m_whereClauseIndex < 0)
            return nullptr;

        return GetChild<WhereExp> (static_cast<size_t> (m_whereClauseIndex));
        }

    OrderByExp const* GetOptOrderBy() const 
        { 
        if (m_orderByClauseIndex < 0)
            return nullptr;

        return GetChild<OrderByExp> (static_cast<size_t> (m_orderByClauseIndex));
        }

    GroupByExp const* GetOptGroupBy() const 
        { 
        if (m_groupByClauseIndex < 0)
            return nullptr;

        return GetChild<GroupByExp> (static_cast<size_t> (m_groupByClauseIndex));
        }

    HavingExp const* GetOptHaving() const 
        { 
        if (m_havingClauseIndex < 0)
            return nullptr;

        return GetChild<HavingExp> (static_cast<size_t> (m_havingClauseIndex));
        }

    LimitOffsetExp const* GetLimitOffset () const
        {
        if (m_limitOffsetClauseIndex < 0)
            return nullptr;

        return GetChild<LimitOffsetExp> (static_cast<size_t> (m_limitOffsetClauseIndex));
        }

    SqlSetQuantifier GetSelectionType() const {return m_selectionType;}

    virtual Utf8String ToECSql() const override;
    };

//********* QueryExp subclasses ***************************
//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryExp : QueryExp
    {
    DEFINE_EXPR_TYPE(Subquery)
private:
    virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

    virtual Utf8String _ToString () const override
        {
        return "Subquery";
        }

protected:
    virtual DerivedPropertyExp const* _FindProperty(Utf8StringCR propertyName) const override
        {
        return GetQuery ()->FindProperty(propertyName);
        }
    virtual SelectClauseExp const* _GetSelection() const override
        {
        return GetQuery ()->GetSelection();
        }
public:
    SubqueryExp () : QueryExp () {}
    explicit SubqueryExp(QueryExp* subquery)
        : QueryExp ()
        {
        AddChild (std::unique_ptr<Exp> (subquery));
        }

    QueryExp const* GetQuery () const { return GetChild<QueryExp> (0);}
    virtual Utf8String ToECSql() const override;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct NonJoinQueryExp: QueryExp
    {    
    DEFINE_EXPR_TYPE(NonJoinQuery) 
private:
    NonJoinQueryOperator m_op;
    bool m_all;

    static Utf8CP OperatorToString (NonJoinQueryOperator op)
        {
        switch(op)
            {
            case NonJoinQueryOperator::Union:
                return "UNION";
            case NonJoinQueryOperator::Intersect:
                return "INTERSECT";
            case NonJoinQueryOperator::Except:
                return "EXCEPT";

            }
        BeAssert(false && "case not handled");
        return nullptr;
        }

    virtual Utf8String _ToString () const override
        {
        Utf8String str ("NonJoinQuery [Operator: ");
        str.append (OperatorToString (m_op)).append ("]");
        return str;
        }

protected:
    virtual DerivedPropertyExp const* _FindProperty(Utf8StringCR propertyName) const override
        {
        return GetLeftQuery ()->FindProperty(propertyName);
        }
    virtual SelectClauseExp const* _GetSelection() const override
        {
        return GetRightQuery ()->GetSelection();
        }

public:
    NonJoinQueryExp(QueryExp* leftQuery, NonJoinQueryOperator op, bool all, QueryExp* rightQuery)
        : QueryExp (), m_op(op), m_all(all)
        {
        AddChild (std::unique_ptr<Exp> (leftQuery));
        AddChild (std::unique_ptr<Exp> (rightQuery));
        }

    QueryExp const* GetLeftQuery() const { return GetChild<QueryExp> (0);}
    QueryExp const* GetRightQuery() const { return GetChild<QueryExp> (1);};
    NonJoinQueryOperator GetOperator() const { return m_op;}
    bool IsAll() const { return m_all;}

    virtual Utf8String ToECSql() const override;
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
    virtual Utf8StringCR _GetId() const override
        {
        return GetAlias();
        }
    virtual bool _ContainProperty(Utf8StringCR propertyName) const override
        {
        return GetSubquery ()->GetQuery()->FindProperty(propertyName) != nullptr;
        }

    virtual ECSqlStatus _CreatePropertyNameExpList (ECSqlParseContext& ctx, std::function<void (std::unique_ptr<PropertyNameExp>&)> addDelegate) const override;

    virtual Utf8String _ToString () const override
        {
        Utf8String str ("SubqueryRef [");
        str.append (GetId ().c_str ()).append ("]");
        return str;
        }

public:
    SubqueryRefExp(SubqueryExp* subquery, Utf8StringCR alias, bool isPolymorphic)
        : RangeClassRefExp (isPolymorphic)
        {
        SetAlias(alias);

        AddChild (std::unique_ptr<Exp> (subquery));
        }

    SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp> (0);}
    //virtual Utf8StringCR GetPublicId() const;
    virtual Utf8String ToECSql() const override
        {
        return  (!IsPolymorphic() ? "ONLY " : "") + GetSubquery ()->ToString() + (GetAlias().empty() ? "" : " AS " + GetAlias());
        }
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
    SqlBooleanOperator m_operator;
    size_t m_operandExpIndex;
    size_t m_subqueryExpIndex;

    virtual Utf8String _ToString () const override;

public:
    AllOrAnyExp(std::unique_ptr<ValueExp> operand, SqlBooleanOperator op, SqlCompareListType type, std::unique_ptr<SubqueryExp> subquery);

    ValueExp const* GetOperand() const { return GetChild<ValueExp> (m_operandExpIndex);}
    SqlCompareListType GetCompareType() const { return m_type;}
    SubqueryExp const* GetSubquery() const { return GetChild<SubqueryExp> (m_subqueryExpIndex);}
    SqlBooleanOperator GetOperator() const { return m_operator;}

    virtual Utf8String ToECSql() const override;
    };


//=======================================================================================
//! @bsiclass                                                Affan.Khan      04/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryTestExp : BooleanExp
    {
DEFINE_EXPR_TYPE(SubqueryTest) 

private:
    SubqueryTestOperator m_op;

    virtual Utf8String _ToString () const override;

public:
    SubqueryTestExp(SubqueryTestOperator op, std::unique_ptr<SubqueryExp> subquery);

    QueryExp const* GetQuery() const { return GetChild<QueryExp> (0);}
    SubqueryTestOperator  GetOperator() const { return m_op;}

    virtual Utf8String ToECSql() const override;
    };

//******************* Select related value exp *******************************

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
struct SubqueryValueExp : ValueExp
    {
    DEFINE_EXPR_TYPE(SubqueryValue)

    private:
        virtual FinalizeParseStatus _FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode) override;

        virtual Utf8String _ToString () const override
            {
            return "SubqueryValue";
            }

    public:
        explicit SubqueryValueExp (std::unique_ptr<SubqueryExp> subquery);

        SubqueryExp const* GetQuery() const {return GetChild<SubqueryExp> (0);}

        virtual Utf8String ToECSql() const override  
            {
            return GetQuery ()->ToECSql();  
            }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

