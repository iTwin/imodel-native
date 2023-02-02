/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "../../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryClauseAndBindings
{
private:
    Utf8String m_clause;
    BoundQueryValuesList m_bindings;
public:
    QueryClauseAndBindings() {}
    QueryClauseAndBindings(Utf8String clause, BoundQueryValuesList bindings = BoundQueryValuesList())
        : m_clause(clause), m_bindings(bindings)
        {}
    QueryClauseAndBindings(Utf8CP clause, BoundQueryValuesList bindings = BoundQueryValuesList())
        : m_clause(clause), m_bindings(bindings)
        {}
    bool operator==(QueryClauseAndBindings const& other) const
        {
        return m_clause.Equals(other.m_clause)
            && m_bindings == other.m_bindings;
        }
    bool operator!=(QueryClauseAndBindings const& other) const {return !operator==(other);}
    void Reset()
        {
        m_bindings.clear();
        m_clause.clear();
        }
    void Append(QueryClauseAndBindings const& rhs, Utf8StringCR separator)
        {
        if (!m_clause.empty())
            m_clause.append(separator);
        m_clause.append(rhs.GetClause());
        ContainerHelpers::Push(m_bindings, rhs.GetBindings());
        }
    void Append(Utf8String clause, Utf8StringCR separator, BoundQueryValuesList bindings = BoundQueryValuesList())
        {
        if (!m_clause.empty())
            m_clause.append(separator);
        m_clause.append(clause);
        ContainerHelpers::MovePush(m_bindings, bindings);
        }
    void Bind(ECSqlStatement& stmt, uint32_t* bindingIndex = nullptr)
        {
        uint32_t defaultIndex = 1;
        uint32_t& index = bindingIndex ? *bindingIndex : defaultIndex;
        for (auto const& binding : m_bindings)
            binding->Bind(stmt, index++);
        }
    Utf8StringR GetClause() {return m_clause;}
    Utf8StringCR GetClause() const {return m_clause;}
    BoundQueryValuesList const& GetBindings() const {return m_bindings;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FilteredValuesHandler
    {
    virtual ~FilteredValuesHandler() {}
    virtual void _Accept(BeInt64Id) = 0;
    virtual void _Accept(ECValue) = 0;
    virtual BoundQueryValuesList _GetBoundValues() = 0;
    virtual Utf8String _GetWhereClause(Utf8CP valueSelector, bool) const = 0;
    ECPRESENTATION_EXPORT static std::unique_ptr<FilteredValuesHandler> Create(size_t, bool onlyIds);
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ValuesFilteringHelper
{
private:
    std::unique_ptr<FilteredValuesHandler> m_handler;
private:
    template<typename TIterable> void Accept(TIterable const& iterable)
        {
        for (auto const& el : iterable)
            m_handler->_Accept(el);
        }
public:
    ValuesFilteringHelper(bvector<ECValue> const& ecValues)
        : m_handler(FilteredValuesHandler::Create(ecValues.size(), false))
        {
        Accept(ecValues);
        }
    template<typename TIterable> ValuesFilteringHelper(TIterable const& iterable)
        : m_handler(FilteredValuesHandler::Create(iterable.size(), true))
        {
        Accept(iterable);
        }
    Utf8String CreateWhereClause(Utf8CP valueSelector, bool inverse = false) const {return m_handler->_GetWhereClause(valueSelector, inverse);}
    BoundQueryValuesList CreateBoundValues() {return m_handler->_GetBoundValues();}
    QueryClauseAndBindings Create(Utf8CP valueSelector, bool inverse = false) {return QueryClauseAndBindings(CreateWhereClause(valueSelector, inverse), CreateBoundValues());}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryHelpers
    {
    static bool IsFunction(Utf8StringCR clause);
    static bool IsLiteral(Utf8StringCR clause);
    static bool IsWrapped(Utf8StringCR clause);
    static Utf8String Wrap(Utf8StringCR clause);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IQueryInfoProvider
{
    enum SelectionSourceFlags
        {
        SELECTION_SOURCE_From = 1 << 0,
        SELECTION_SOURCE_Join = 1 << 1,
        SELECTION_SOURCE_All  = (SELECTION_SOURCE_From | SELECTION_SOURCE_Join)
        };

protected:
    virtual ~IQueryInfoProvider() {}
    virtual bvector<Utf8CP> _GetSelectAliases(int flags) const = 0;
    virtual bvector<Utf8CP> _GetRelationshipAliases(int flags) const {return bvector<Utf8CP>();};

public:
    bvector<Utf8CP> GetSelectAliases(int flags = SELECTION_SOURCE_All) const {return _GetSelectAliases(flags);}
    bvector<Utf8CP> GetRelationshipAliases(int flags = SELECTION_SOURCE_All) const {return _GetRelationshipAliases(flags);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
