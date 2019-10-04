/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "QueryExecutor.h"
#include "QueryContracts.h"
#include <cctype>

// *** NEEDS WORK: clang for android complains that GetECClassClause and ContractHasNonAggregateFields are unused
// ***              and that ConstraintSupportsClass "is not needed and will not be emitted". I don't understand
// ***              any of that. GetECClassClause, for example, appears be used. For now, I will disable these clang warnings.
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundQueryECValue::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    if (m_value.IsNull())
        {
        stmt.BindNull((int)index);
        return ECSqlStatus::Success;
        }

    switch (m_value.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean: return stmt.BindBoolean((int)index, m_value.GetBoolean());
        case PRIMITIVETYPE_DateTime: return stmt.BindDateTime((int)index, m_value.GetDateTime());
        case PRIMITIVETYPE_Double: return stmt.BindDouble((int)index, m_value.GetDouble());
        case PRIMITIVETYPE_Integer: return stmt.BindInt((int)index, m_value.GetInteger());
        case PRIMITIVETYPE_Long: return stmt.BindInt64((int)index, m_value.GetLong());
        case PRIMITIVETYPE_String: return stmt.BindText((int)index, m_value.GetUtf8CP(), IECSqlBinder::MakeCopy::No);
        case PRIMITIVETYPE_Point2d: return stmt.BindPoint2d((int)index, m_value.GetPoint2d());
        case PRIMITIVETYPE_Point3d: return stmt.BindPoint3d((int)index, m_value.GetPoint3d());
        }

    BeAssert(false);
    return ECSqlStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AreEqual(BoundQueryECValue const* lhs, BoundQueryECValue const* rhs)
    {
    if (lhs == rhs)
        return true;

    if (nullptr == lhs || nullptr == rhs)
        return false;

    return lhs->GetValue() == lhs->GetValue();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundQueryECValue::_Equals(BoundQueryValue const& other) const
    {
    BoundQueryECValue const* otherECValue = dynamic_cast<BoundQueryECValue const*>(&other);
    return (nullptr != otherECValue) ? AreEqual(this, otherECValue) : false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundQueryId::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    return stmt.BindId((int)index, m_id);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundQueryId::_Equals(BoundQueryValue const& other) const
    {
    BoundQueryId const* otherId = dynamic_cast<BoundQueryId const*>(&other);
    if (nullptr == otherId)
        return false;

    return m_id == otherId->m_id;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundQueryIdSet::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    return stmt.BindVirtualSet((int)index, m_set);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundQueryIdSet::_Equals(BoundQueryValue const& other) const
    {
    BoundQueryIdSet const* otherVirtualSet = dynamic_cast<BoundQueryIdSet const*>(&other);
    if (nullptr == otherVirtualSet)
        return false;

    return m_set == otherVirtualSet->m_set;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct NoIdsHandler : FilteredIdsHandler
{
protected:
    Utf8String _GetWhereClause(Utf8CP idSelector, size_t) const override {return "FALSE";}
    void _Accept(BeInt64Id id) override {BeAssert(false);}
    BoundQueryValuesList _GetBoundValues() override {return BoundQueryValuesList();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct VirtualSetIdsHandler : FilteredIdsHandler
{
private:
    IdSet<BeInt64Id> m_ids;
protected:
    Utf8String _GetWhereClause(Utf8CP idSelector, size_t) const override
        {
        return Utf8String("InVirtualSet(?, ").append(idSelector).append(")");
        }
    void _Accept(BeInt64Id id) override {m_ids.insert(id);}
    BoundQueryValuesList _GetBoundValues() override {return {new BoundQueryIdSet(std::move(m_ids))};}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct BoundIdsHandler : FilteredIdsHandler
{
private:
    BoundQueryValuesList m_values;
protected:
    Utf8String _GetWhereClause(Utf8CP idSelector, size_t idsCount) const override
        {
        if (0 == idsCount)
            return "FALSE";
        Utf8String idsArg(idsCount * 2 - 1, '?');
        for (size_t i = 1; i < idsArg.size(); i += 2)
            idsArg[i] = ',';
        return Utf8PrintfString("%s IN (%s)", idSelector, idsArg.c_str());
        }
    void _Accept(BeInt64Id id) override {m_values.push_back(new BoundQueryId(id));}
    BoundQueryValuesList _GetBoundValues() override {return std::move(m_values);}
public:
    ~BoundIdsHandler()
        {
        for (BoundQueryValue const* v : m_values)
            delete v;
        }
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
FilteredIdsHandler* FilteredIdsHandler::Create(size_t inputSize)
    {
    // choosing when it's worth using virtual set vs bound IDs depends on 2 factors:
    // - rows count in the table we're selecting from (more rows make virtual set more expensive)
    // - bound IDs count (more IDs make binding them individually more expensive)
    // assuming that selecting from large tables is a common case and large number of 
    // bounds IDs - not, we chose the boundary pretty high.
    static const size_t BOUNDARY_VirtualSet = 100;

    if (0 == inputSize)
        return new NoIdsHandler();
    if (inputSize > BOUNDARY_VirtualSet)
        return new VirtualSetIdsHandler();
    return new BoundIdsHandler();
    }


enum class IsFunctionTestStage
    {
    Name,
    Space,
    OpenArgs,
    Args,
    CloseArgs,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryHelpers::IsFunction(Utf8StringCR clause)
    {
    IsFunctionTestStage stage = IsFunctionTestStage::Name;
    for (size_t i = 0; i < clause.size(); ++i)
        {
        switch (stage)
            {
            case IsFunctionTestStage::Name:
                {
                if ('0' <= clause[i] && clause[i] <= '9'
                    || 'A' <= clause[i] && clause[i] <= 'Z'
                    || 'a' <= clause[i] && clause[i] <= 'z'
                    || '_' == clause[i])
                    {
                    continue;
                    }
                if (' ' == clause[i])
                    {
                    stage = IsFunctionTestStage::Space;
                    break;
                    }
                if ('(' == clause[i])
                    {
                    stage = IsFunctionTestStage::OpenArgs;
                    break;
                    }
                return false;
                }
            case IsFunctionTestStage::Space:
                {
                if (' ' == clause[i])
                    continue;
                if ('(' == clause[i])
                    {
                    stage = IsFunctionTestStage::OpenArgs;
                    break;
                    }
                return false;
                }
            }
        if (IsFunctionTestStage::OpenArgs == stage)
            break;
        }

    if (IsFunctionTestStage::OpenArgs != stage || ')' != clause[clause.size() - 1])
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsNumeric(Utf8StringCR clause)
    {
    for (Utf8Char c : clause)
        {
        if ((c < '0' || c > '9') && c != '.')
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryHelpers::IsLiteral(Utf8StringCR clause)
    {
    BeAssert(!clause.empty());
    return clause.EqualsI("null")
        || clause.EqualsI("false") || clause.EqualsI("true")
        || IsNumeric(clause)
        || clause.StartsWith("'") && clause.EndsWith("'");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryHelpers::IsWrapped(Utf8StringCR clause)
    {
    BeAssert(!clause.empty());
    return clause.StartsWith("[") && clause.EndsWith("]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryHelpers::Wrap(Utf8StringCR str)
    {
    if (str.empty() || IsFunction(str) || IsLiteral(str) || IsWrapped(str))
        return str;

    return Utf8String("[").append(str).append("]");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PresentationQueryBase::BindValues(ECSqlStatement& stmt) const
    {
    BentleyStatus result = SUCCESS;
    bvector<BoundQueryValue const*> boundValues = GetBoundValues();
    for (size_t i = 0; i < boundValues.size(); ++i)
        {
        BoundQueryValue const* value = boundValues[i];
        ECSqlStatus status = value->Bind(stmt, (uint32_t)(i + 1));
        if (!status.IsSuccess())
            {
            BeAssert(false);
            result = ERROR;
            }
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetECClassClause(ECClassCR ecClass)
    {
    Utf8String fullClassName("[");
    ECSchemaCR schema = ecClass.GetSchema();
    Utf8StringCR schemaAlias = schema.GetAlias();
    if (schemaAlias.empty())
        fullClassName.append(schema.GetName ());
    else
        fullClassName.append(schemaAlias);

    fullClassName.append("].[").append(ecClass.GetName().c_str()).append("]");
    return fullClassName;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::CopyBindings(ComplexPresentationQuery<TBase> const& other)
    {
    if (nullptr != other.m_limit)
        m_limit = new BoundQueryECValue(*other.m_limit);
    if (nullptr != other.m_offset)
        m_offset = new BoundQueryECValue(*other.m_offset);
    for (BoundQueryValue const* whereClauseBinding : other.m_whereClauseBindings)
        m_whereClauseBindings.push_back(whereClauseBinding->Clone());
    for (BoundQueryValue const* havingClauseBinding : other.m_havingClauseBindings)
        m_havingClauseBindings.push_back(havingClauseBinding->Clone());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
typename TBase::Contract const* ComplexPresentationQuery<TBase>::_GetContract(uint64_t contractId) const
    {
    if (m_selectContract.IsValid() && (0 == contractId || m_selectContract->GetId() == contractId))
        return m_selectContract.get();

    if (m_nestedQuery.IsValid())
        return m_nestedQuery->GetContract(contractId);

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
typename TBase::Contract const* ComplexPresentationQuery<TBase>::_GetGroupingContract() const
    {
    if (m_groupingContract.IsValid())
        return m_groupingContract.get();

    if (m_nestedQuery.IsValid())
        return m_nestedQuery->GetGroupingContract();

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ContractHasNonAggregateFields(PresentationQueryContractCR contract)
    {
    bvector<PresentationQueryContractFieldCPtr> fields = contract.GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (!field->IsAggregateField())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bool ComplexPresentationQuery<TBase>::HasClause(PresentationQueryClauses clauses) const
    {
    if (CLAUSE_Select == (CLAUSE_Select & clauses))
        return m_isSelectAll || m_selectContract.IsValid();
        
    if (CLAUSE_From == (CLAUSE_From & clauses))
        return m_nestedQuery.IsValid() || !m_from.empty();
        
    if (CLAUSE_Where == (CLAUSE_Where & clauses))
        return !m_whereClause.empty();

    if (CLAUSE_JoinUsing == (CLAUSE_JoinUsing & clauses))
        return !m_joins.empty();
        
    if (CLAUSE_OrderBy == (CLAUSE_OrderBy & clauses))
        return !m_orderByClause.empty();

    if (CLAUSE_Limit == (CLAUSE_Limit & clauses))
        return nullptr != m_limit;
        
    if (CLAUSE_GroupBy == (CLAUSE_GroupBy & clauses))
        return m_groupingContract.IsValid() && ContractHasNonAggregateFields(*m_groupingContract);
    
    if (CLAUSE_Having == (CLAUSE_Having & clauses))
        return !m_havingClause.empty();

    BeAssert(false);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::AppendToSelectClause(Utf8StringR output, Utf8CP clause)
    {
    if (!output.empty())
        output.append(", ").append(clause);
    else
        output = clause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::Select(Utf8StringR output, Utf8CP clause, bool append)
    {
    if (append)
        AppendToSelectClause(output, clause);
    else
        output = clause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::SelectField(Utf8StringR output, Utf8StringCR clause, Utf8CP alias, bool append)
    {
    Utf8String clauseAndAlias = clause;
    if (!Utf8String::IsNullOrEmpty(alias))
        clauseAndAlias.append(" AS ").append(QueryHelpers::Wrap(alias));
    Select(output, clauseAndAlias.c_str(), append);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::SelectProperty(Utf8StringR output, ECPropertyCR selectProperty, Utf8CP alias, bool append)
    {
    Utf8String clause;
    clause.append(QueryHelpers::Wrap(selectProperty.GetClass().GetName().c_str()))
        .append(".")
        .append(QueryHelpers::Wrap(selectProperty.GetName().c_str()));
    SelectField(output, clause, alias, append);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::SelectString(Utf8StringR output, Utf8CP selectString, Utf8CP alias, bool append)
    {
    Utf8String stringSelectClause;
    stringSelectClause.append("'").append(selectString).append("'");
    if (!Utf8String::IsNullOrEmpty(alias))
        stringSelectClause.append(" AS ").append(QueryHelpers::Wrap(alias));
    Select(output, stringSelectClause.c_str(), append);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::SelectAll()
    {
    TBase::InvalidateQueryString();
    m_isSelectAll = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::SelectContract(Contract const& contract, Utf8CP prefix)
    {
    TBase::InvalidateQueryString();
    m_selectContract = &contract;
    m_selectPrefix = prefix;
    TBase::GetResultParametersR().OnContractSelected(contract);
    return *this;
    }

enum QueryPosition
    {
    None  = 0,
    Inner = 1 << 0,
    Outer = 1 << 1,
    };

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String ComplexPresentationQuery<TBase>::CreateSelectClause() const
    {
    int position = QueryPosition::None;
    if (nullptr == TBase::GetGroupingContract())
        position |= QueryPosition::Inner;
    if (TBase::IsOuterQuery())
        position |= QueryPosition::Outer;

    bool hasNestedInnerFields = false;
    Contract const* contract = m_selectContract.get();
    Contract const* nestedContract = nullptr;
    if (m_nestedQuery.IsValid())
        {
        nestedContract = m_nestedQuery->GetContract();
        if (nullptr != nestedContract)
            {
            if (m_isSelectAll)
                contract = nestedContract;

            hasNestedInnerFields = nestedContract->HasInnerFields();
            }
        }

    bvector<Utf8String> selectClauseFields;

    if (m_isSelectAll && !hasNestedInnerFields)
        {
        selectClauseFields.push_back("*");
        }
    else
        {
        bool selectWithClauses = (contract != nestedContract);
        bvector<PresentationQueryContractFieldCPtr> selectFields = contract->GetFields();
        for (PresentationQueryContractFieldCPtr const& field : selectFields)
            {
            if (field->IsAggregateField())
                continue;

            if (FieldVisibility::Inner == field->GetVisibility() && 0 == (QueryPosition::Inner & position))
                continue;

            if (FieldVisibility::Outer == field->GetVisibility() && 0 == (QueryPosition::Outer & position))
                continue;

            Utf8String wrappedName = QueryHelpers::Wrap(field->GetName());
            Utf8String selectClauseField;
            if (m_nestedQuery.IsNull() || nullptr != m_nestedQuery->AsStringQuery() || selectWithClauses || FieldVisibility::Outer == field->GetVisibility())
                {
                bool skipNestedClauses = m_nestedQuery.IsValid() && (nullptr == m_nestedQuery->AsStringQuery());
                SelectField(selectClauseField, field->GetSelectClause(m_selectPrefix.c_str(), skipNestedClauses), wrappedName.c_str(), true);
                }
            else
                {
                SelectField(selectClauseField, wrappedName, nullptr, true);
                }
            selectClauseFields.push_back(selectClauseField);
            }
        }
    
    Contract const* groupingContract = TBase::GetGroupingContract();
    if (nullptr != groupingContract)
        {
        bvector<PresentationQueryContractFieldCPtr> groupingFields = groupingContract->GetFields();
        for (PresentationQueryContractFieldCPtr const& field : groupingFields)
            {
            if (!field->IsAggregateField())
                continue;
            
            Utf8String selectClauseField;
            if (m_groupingContract.get() == groupingContract)
                SelectField(selectClauseField, field->GetSelectClause(m_selectPrefix.c_str(), m_nestedQuery.IsValid()), field->GetName(), true);
            else
                SelectField(selectClauseField, QueryHelpers::Wrap(field->GetName()), nullptr, true);
            selectClauseFields.push_back(selectClauseField);
            }
        }

    Utf8String selectClause = BeStringUtilities::Join(selectClauseFields, ", ");
    return selectClause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::From(ECClassCR fromClass, bool polymorphic, Utf8CP alias, bool append)
    {
    TBase::InvalidateQueryString();
    if (!append)
        m_from.clear();
    m_from.push_back(FromClause(fromClass, alias, polymorphic));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::From(TBase& nestedQuery, Utf8CP alias)
    {
    TBase::InvalidateQueryString();
    nestedQuery.SetIsOuterQuery(false);

    m_nestedQuery = &nestedQuery;
    m_nestedQueryAlias = alias;

    TBase::GetResultParametersR().MergeWith(m_nestedQuery->GetResultParameters());
    nestedQuery.GetResultParametersR() = ResultParameters();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::Where(Utf8CP whereClause, BoundQueryValuesListCR bindings, bool append)
    {
    TBase::InvalidateQueryString();
    if (!m_whereClause.empty() && append)
        {
        m_whereClauseBindings.insert(m_whereClauseBindings.end(), bindings.begin(), bindings.end());
        m_whereClause.append(" AND (").append(whereClause).append(")");
        }
    else
        {
        for (BoundQueryValue const* value : m_whereClauseBindings)
            DELETE_AND_CLEAR(value);
        m_whereClauseBindings = bindings;
        m_whereClause = Utf8String("(").append(whereClause).append(")");
        }
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::Join(RelatedClass const& relatedClass, bool append)
    {
    TBase::InvalidateQueryString();
    RelatedClassPath path;
    path.push_back(relatedClass);
    return Join(path, relatedClass.IsOuterJoin(), append);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::Join(RelatedClassPath const& path, bool isOuter, bool append)
    {
    TBase::InvalidateQueryString();
    if (!append)
        m_joins.clear();

    bvector<JoinUsingClause> join;
    for (RelatedClass const& relatedClass : path)
        {
        join.push_back(JoinUsingClause(*relatedClass.GetTargetClass(), *relatedClass.GetRelationship(), relatedClass.IsForwardRelationship(),
            isOuter, relatedClass.IsPolymorphic(), relatedClass.GetTargetClassAlias(), relatedClass.GetRelationshipAlias()));
        }
    m_joins.push_back(join);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::OrderBy(Utf8CP orderByClause)
    {
    TBase::InvalidateQueryString();
    m_orderByClause = orderByClause; 
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::GroupByContract(Contract const& contract)
    {
    TBase::InvalidateQueryString();
    m_groupingContract = &contract;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String ComplexPresentationQuery<TBase>::CreateGroupByClause() const
    {
    if (m_groupingContract.IsNull())
        return "";

    Utf8String groupByClause;
    bvector<PresentationQueryContractFieldCPtr> fields = m_groupingContract->GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (field->IsAggregateField() || FieldVisibility::Both != field->GetVisibility())
            continue;

        if (!groupByClause.empty())
            groupByClause.append(", ");
        groupByClause.append(QueryHelpers::Wrap(field->GetGroupingClause()));
        }
    return groupByClause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::Having(Utf8CP havingClause, BoundQueryValuesListCR bindings) 
    {
    TBase::InvalidateQueryString();
    for (BoundQueryValue const* value : m_havingClauseBindings)
        DELETE_AND_CLEAR(value);
    m_havingClauseBindings = bindings;
    m_havingClause = havingClause; 
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ComplexPresentationQuery<TBase>& ComplexPresentationQuery<TBase>::Limit(uint64_t limit, uint64_t offset)
    {
    TBase::InvalidateQueryString();
    DELETE_AND_CLEAR(m_limit);
    m_limit = new BoundQueryECValue(ECValue(limit));
    DELETE_AND_CLEAR(m_offset);
    m_offset = new BoundQueryECValue(ECValue(offset));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static void DetermineFromClass(ECClassCP& ecClass, Utf8StringR name, bvector<typename T::FromClause> const& fromClauses, typename T::JoinUsingClause const& joinClause)
    {
    ECRelationshipConstraintCR constraint = joinClause.m_isForward ? joinClause.m_using->GetSource() : joinClause.m_using->GetTarget();
    ECRelationshipConstraintCR oppositeConstraint = joinClause.m_isForward ? joinClause.m_using->GetTarget() : joinClause.m_using->GetSource();
    for (typename T::FromClause const& fromClause : fromClauses)
        {
        if (constraint.SupportsClass(*fromClause.m_class))
            {
            ecClass = fromClause.m_class;
            BeAssert(oppositeConstraint.SupportsClass(*joinClause.m_join));
            }

        if (nullptr != ecClass)
            {
            name = fromClause.m_alias.empty() ? ecClass->GetName() : fromClause.m_alias;
            break;
            }
        }

    if (nullptr == ecClass)
        {
        BeAssert(false && "Trying to JOIN on a relationship whose neither target nor source exists in the FROM clause");
        return;
        }
    }

#ifndef NDEBUG
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ConstraintSupportsClass(ECRelationshipConstraintCR constraint, ECClassCR ecClass)
    {
    ECRelationshipConstraintClassList const& classes = constraint.GetConstraintClasses();
    for (ECClassCP constraintClass : classes)
        {
        if (constraintClass->GetName().EqualsI("AnyClass"))
            return true;
        
        if (ECClass::ClassesAreEqualByName(constraintClass, &ecClass))
            return true;

        if (constraint.GetIsPolymorphic() && (ecClass.Is(constraintClass) || constraintClass->Is(&ecClass)))
            return true;
        }
    return false;
    }
#endif

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static void DetermineJoinClauses(Utf8StringR thisClause, Utf8StringR nextClause, ECClassCR ecClass, bool wasPreviousJoinForward, typename T::JoinUsingClause const& joinClause)
    {
    ECRelationshipConstraintCP constraint;
    Utf8CP thisClauseECInstanceIdPropertyName, 
           thisClauseECClassIdPropertyName, 
           nextClauseECInstanceIdPropertyName, 
           nextClauseECClassIdPropertyName,
           joinECInstanceIdPropertyName,
           joinECClassIdPropertyName;
    if (joinClause.m_isForward)
        {
        constraint = &joinClause.m_using->GetSource();
        thisClauseECInstanceIdPropertyName = "SourceECInstanceId";
        thisClauseECClassIdPropertyName = "SourceECClassId";
        nextClauseECClassIdPropertyName = "TargetECClassId";
        nextClauseECInstanceIdPropertyName = "TargetECInstanceId";
        }
    else
        {
        constraint = &joinClause.m_using->GetTarget();
        thisClauseECInstanceIdPropertyName = "TargetECInstanceId";
        thisClauseECClassIdPropertyName = "TargetECClassId";
        nextClauseECInstanceIdPropertyName = "SourceECInstanceId";
        nextClauseECClassIdPropertyName = "SourceECClassId";
        }

    if (ecClass.IsRelationshipClass())
        {
#if !defined(NDEBUG) && WIP_TEMPORARILLY_DISABLED
        bool previousRelationshipBinds = false;
        ECRelationshipClassCP relationship = ecClass.GetRelationshipClassCP();
        ECRelationshipConstraintCR relationshipConstraint = wasPreviousJoinForward ? relationship->GetTarget() : relationship->GetSource();
        ECConstraintClassesList relationshipConstraintClasses = relationshipConstraint.GetClasses();
        for (ECClassCP relationshipConstraintClass : relationshipConstraintClasses)
            {
            if (ConstraintSupportsClass(*constraint, *relationshipConstraintClass))
                {
                previousRelationshipBinds = true;
                break;
                }
            }
        BeAssert(previousRelationshipBinds);
#endif

        if (wasPreviousJoinForward)
            {
            joinECInstanceIdPropertyName = "TargetECInstanceId";
            joinECClassIdPropertyName = "TargetECClassId";
            }
        else
            {
            joinECInstanceIdPropertyName = "SourceECInstanceId";
            joinECClassIdPropertyName = "SourceECClassId";
            }
        }
    else
        {
#ifndef NDEBUG
        BeAssert(ConstraintSupportsClass(*constraint, ecClass));
#endif
        joinECInstanceIdPropertyName = "ECInstanceId";
        joinECClassIdPropertyName = "ECClassId";
        }

    static Utf8CP pattern = " ON [%%s].[%s] = [%%s].[%s] AND [%%s].[%s] = [%%s].[%s]";
    thisClause = Utf8PrintfString(pattern, joinECInstanceIdPropertyName, thisClauseECInstanceIdPropertyName, joinECClassIdPropertyName, thisClauseECClassIdPropertyName);
    nextClause = Utf8PrintfString(pattern, "ECInstanceId", nextClauseECInstanceIdPropertyName, "ECClassId", nextClauseECClassIdPropertyName);

    BeAssert(!thisClause .empty() && !nextClause.empty());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static NavigationECPropertyCP GetNavigationProperty(ECClassCR prev, ECRelationshipClassCR usingRel, bool isForward, ECClassCR join)
    {
    ECClassCR src = isForward ? prev : join;
    ECClassCR tgt = isForward ? join : prev;

    ECPropertyIterable srcIterable = src.GetProperties(true);
    for (ECPropertyCP prop : srcIterable)
        {
        if (prop->GetIsNavigation()
            && prop->GetAsNavigationProperty()->GetRelationshipClass() == &usingRel
            && ECRelatedInstanceDirection::Forward == prop->GetAsNavigationProperty()->GetDirection())
            {
            return prop->GetAsNavigationProperty();
            }
        }
    
    ECPropertyIterable tgtIterable = tgt.GetProperties(true);
    for (ECPropertyCP prop : tgtIterable)
        {
        if (prop->GetIsNavigation()
            && prop->GetAsNavigationProperty()->GetRelationshipClass() == &usingRel
            && ECRelatedInstanceDirection::Backward == prop->GetAsNavigationProperty()->GetDirection())
            {
            return prop->GetAsNavigationProperty();
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetNavigationPropertyName(NavigationECPropertyCR navigationProperty)
    {
    return QueryHelpers::Wrap(navigationProperty.GetName()).append(".[Id]");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetOppositeNavigationPropertyName(NavigationECPropertyCR navigationProperty, ECClassCR previouslyJoinedClass, bool wasPreviousJoinForward)
    {
    if (previouslyJoinedClass.IsEntityClass())
        return "[ECInstanceId]";

    if (previouslyJoinedClass.IsRelationshipClass())
        {
        if (navigationProperty.GetRelationshipClass()->GetSource().SupportsClass(previouslyJoinedClass)
            || navigationProperty.GetRelationshipClass()->GetTarget().SupportsClass(previouslyJoinedClass))
            {
            return "[ECInstanceId]";
            }
        return wasPreviousJoinForward ? "[TargetECInstanceId]" : "[SourceECInstanceId]";
        }

    BeAssert(false);
    return "[ECInstanceId]";
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis            10/2016
//=======================================================================================
template<typename TBase>
struct JoinsComparer
    {
    bool operator()(typename ComplexPresentationQuery<TBase>::JoinUsingClause const& lhs, typename ComplexPresentationQuery<TBase>::JoinUsingClause const& rhs) const
        {
        if (lhs.m_join < rhs.m_join)
            return true;
        if (lhs.m_join > rhs.m_join)
            return false;
        if (lhs.m_using < rhs.m_using)
            return true;
        if (lhs.m_using > rhs.m_using)
            return false;
        int joinCmp = lhs.m_joinAlias.CompareTo(rhs.m_joinAlias);
        if (joinCmp < 0)
            return true;
        if (joinCmp > 0)
            return false;
        int usingCmp = lhs.m_usingAlias.CompareTo(rhs.m_usingAlias);
        if (usingCmp < 0)
            return true;
        if (usingCmp > 0)
            return false;
        if (lhs.m_isForward != rhs.m_isForward)
            return !lhs.m_isForward;
        if (lhs.m_isPolymorphic != rhs.m_isPolymorphic)
            return !lhs.m_isPolymorphic;
        return false;
        }
    };

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis            10/2016
//=======================================================================================
struct JoinInfo
    {
    ECClassCP m_previousClass;
    Utf8String m_previousClassName;
    bool m_wasPreviousJoinForward;
    JoinInfo() : m_previousClass(nullptr), m_wasPreviousJoinForward(false) {}
    JoinInfo(ECClassCR previous, Utf8String previousName, bool wasForward)
        : m_previousClass(&previous), m_previousClassName(previousName), m_wasPreviousJoinForward(wasForward)
        {}
    };

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String ComplexPresentationQuery<TBase>::CreateJoinClause() const
    {
    if (m_from.empty())
        {
        BeAssert(false && "Join is only valid when used with 'FROM [ECClass]'");
        return "";
        }

    bmap<JoinUsingClause, JoinInfo, JoinsComparer<TBase>> usedJoins;
    Utf8String joinClause;
    for (auto joinGroupIter = m_joins.begin(); joinGroupIter != m_joins.end(); ++joinGroupIter)
        {
        bvector<JoinUsingClause> const& joins = *joinGroupIter;

        ECClassCP previousClass = nullptr;
        Utf8String previousClassName = "";
        bool wasPreviousJoinForward = false;
        bool first = true;
        for (auto iter = joins.begin(); iter != joins.end(); iter++)
            {
            JoinUsingClause const& join = *iter;
            if (!join.IsValid())
                continue;

            if (nullptr == previousClass)
                {
                DetermineFromClass<ThisType>(previousClass, previousClassName, m_from, join);
                if (nullptr == previousClass)
                    continue;

                BeAssert(previousClass->IsEntityClass());
                }

            auto usedJoinIter = usedJoins.find(join);
            if (usedJoins.end() != usedJoinIter)
                {
                previousClass = usedJoinIter->second.m_previousClass;
                previousClassName = usedJoinIter->second.m_previousClassName;
                wasPreviousJoinForward = usedJoinIter->second.m_wasPreviousJoinForward;
                continue;
                }
                        
            joinClause.append(join.m_isOuterJoin ? " LEFT JOIN " : " INNER JOIN ");

            if (!join.m_isPolymorphic)
                joinClause.append("ONLY ");

            NavigationECPropertyCP navigationProperty = GetNavigationProperty(*previousClass, *join.m_using, join.m_isForward, *join.m_join);
            if (nullptr != navigationProperty)
                {
                BeAssert(!join.m_joinAlias.empty());
                bool isForward = (join.m_isForward == (ECRelatedInstanceDirection::Forward == navigationProperty->GetDirection()));
                joinClause.append(GetECClassClause(*join.m_join));
                joinClause.append(" ").append(QueryHelpers::Wrap(join.m_joinAlias));
                joinClause.append(" ON ").append(QueryHelpers::Wrap(join.m_joinAlias)).append(".");
                joinClause.append(isForward ? GetOppositeNavigationPropertyName(*navigationProperty, *previousClass, wasPreviousJoinForward) : GetNavigationPropertyName(*navigationProperty));
                joinClause.append(" = ").append(QueryHelpers::Wrap(previousClassName)).append(".");
                joinClause.append(isForward ? GetNavigationPropertyName(*navigationProperty) : GetOppositeNavigationPropertyName(*navigationProperty, *previousClass, wasPreviousJoinForward));

                previousClass = join.m_join;
                previousClassName = join.m_joinAlias;
                }
            else
                {
                // determine the join clause based on relationship direction
                Utf8String previousClassJoinClause;
                Utf8String joinedClassJoinClause;
                DetermineJoinClauses<ThisType>(previousClassJoinClause, joinedClassJoinClause, *previousClass, wasPreviousJoinForward, join);

                // append the join on relationship clause
                BeAssert(!join.m_usingAlias.empty());
                joinClause.append(GetECClassClause(*join.m_using));
                joinClause.append(" ").append(QueryHelpers::Wrap(join.m_usingAlias));
                joinClause.append(Utf8PrintfString(previousClassJoinClause.c_str(), previousClassName.c_str(), join.m_usingAlias.c_str(), previousClassName.c_str(), join.m_usingAlias.c_str()));

                // only join the other end of the relationship if this is the last join clause in the group
                if (&join == &joins[joins.size() - 1])
                    {
                    Utf8String joinedClassName = join.m_join->GetName();
                    joinClause.append(join.m_isOuterJoin ? " LEFT JOIN " : " INNER JOIN ").append(GetECClassClause(*join.m_join));
                    if (!join.m_joinAlias.empty())
                        {
                        joinClause.append(" ").append(QueryHelpers::Wrap(join.m_joinAlias));
                        joinedClassName = join.m_joinAlias;
                        }
                    joinClause.append(Utf8PrintfString(joinedClassJoinClause.c_str(), joinedClassName.c_str(), join.m_usingAlias.c_str(), joinedClassName.c_str(), join.m_usingAlias.c_str()));
                    }

                previousClass = join.m_using;
                previousClassName = join.m_usingAlias;
                }
            
            wasPreviousJoinForward = join.m_isForward;
            usedJoins[join] = JoinInfo(*previousClass, previousClassName, wasPreviousJoinForward);
            first = false;
            }
        }

    return joinClause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String ComplexPresentationQuery<TBase>::GetClause(PresentationQueryClauses clause) const
    {
    if (CLAUSE_Select == (CLAUSE_Select & clause))
        return CreateSelectClause();
        
    if (CLAUSE_From == (CLAUSE_From & clause))
        {
        if (m_nestedQuery.IsValid())
            {
            Utf8String clause;
            clause.append("(").append(m_nestedQuery->ToString()).append(")");
            if (!m_nestedQueryAlias.empty())
                clause.append(" ").append(QueryHelpers::Wrap(m_nestedQueryAlias));
            return clause;
            }
        
        Utf8String fromClause;
        bool first = true;
        for (FromClause const& from : m_from)
            {
            if (!first)
                fromClause.append(", ");
            if (!from.m_isPolymorphic)
                fromClause.append("ONLY ");
            fromClause.append(GetECClassClause(*from.m_class).c_str());
            if (!from.m_alias.empty())
                fromClause.append(" ").append(QueryHelpers::Wrap(from.m_alias));
            first = false;
            }
        return fromClause;
        }
        
    if (CLAUSE_Where == (CLAUSE_Where & clause))
        return m_whereClause;

    if (CLAUSE_JoinUsing == (CLAUSE_JoinUsing & clause))
        return CreateJoinClause();
        
    if (CLAUSE_OrderBy == (CLAUSE_OrderBy & clause))
        return m_orderByClause;

    if (CLAUSE_Limit == (CLAUSE_Limit & clause))
        {
        if (nullptr == m_limit)
            return "";
        Utf8String limitClause("?");
        if (nullptr != m_offset)
            limitClause.append(" OFFSET ?");
        return limitClause;
        }
        
    if (CLAUSE_GroupBy == (CLAUSE_GroupBy & clause))
        return CreateGroupByClause();
    
    if (CLAUSE_Having == (CLAUSE_Having & clause))
        return m_havingClause;

    BeAssert(false);
    return "";
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String ComplexPresentationQuery<TBase>::_ToString() const
    {
    Utf8String query;
    query.reserve(1024);

    // SELECT
    if (HasClause(CLAUSE_Select))
        query.append("SELECT ").append(GetClause(CLAUSE_Select));

    // FROM
    if (HasClause(CLAUSE_From))
        query.append(" FROM ").append(GetClause(CLAUSE_From));
        
    // JOIN
    if (HasClause(CLAUSE_JoinUsing))
        query.append(GetClause(CLAUSE_JoinUsing));

    // WHERE
    if (HasClause(CLAUSE_Where))
        query.append(" WHERE ").append(GetClause(CLAUSE_Where));
    
    // GROUP BY
    if (HasClause(CLAUSE_GroupBy))
        query.append(" GROUP BY ").append(GetClause(CLAUSE_GroupBy));
    
    // HAVING
    if (HasClause(CLAUSE_Having))
        query.append(" HAVING ").append(GetClause(CLAUSE_Having));
    
    // ORDER BY
    if (HasClause(CLAUSE_OrderBy))
        query.append(" ORDER BY ").append(GetClause(CLAUSE_OrderBy));

    // LIMIT
    if (HasClause(CLAUSE_Limit))
        query.append(" LIMIT ").append(GetClause(CLAUSE_Limit));

    return query;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bool ComplexPresentationQuery<TBase>::_IsEqual(TBase const& otherBase) const
    {
    if (!TBase::_IsEqual(otherBase))
        return false;

    if (nullptr == otherBase.AsComplexQuery())
        return false;

    ComplexPresentationQuery const& other = *otherBase.AsComplexQuery();

    // SELECT
    if (!CreateSelectClause().Equals(other.CreateSelectClause()))
        return false;
    
    // FROM
    if (m_nestedQuery.IsValid())
        {
        if (other.m_nestedQuery.IsNull() || !m_nestedQuery->IsEqual(*other.m_nestedQuery))
            return false;

        if (!m_nestedQueryAlias.Equals(other.m_nestedQueryAlias))
            return false;
        }
    else
        {
        if (m_from.size() != other.m_from.size())
            return false;

        for (FromClause const& clause : m_from)
            {
            bool found = false;
            for (FromClause const& otherFromClause : other.m_from)
                {
                if (0 == strcmp(clause.m_class->GetFullName(), otherFromClause.m_class->GetFullName()) && clause.m_alias.Equals(otherFromClause.m_alias) && clause.m_isPolymorphic == otherFromClause.m_isPolymorphic)
                    {
                    found = true;
                    break;
                    }
                }
            if (!found)
                return false;
            }
        }
        
    // JOIN
    if (m_joins.size() != other.m_joins.size())
        return false;

    for (bvector<JoinUsingClause> const& joinGroup : m_joins)
        {
        bool foundGroup = false;
        for (bvector<JoinUsingClause> const& otherJoinGroup : other.m_joins)
            {
            if (joinGroup.size() != otherJoinGroup.size())
                continue;

            bool foundAllGroupClauses = true;
            for (JoinUsingClause const& clause : joinGroup)
                {
                bool foundClause = false;
                for (JoinUsingClause const& otherClause : otherJoinGroup)
                    {
                    if (0 == strcmp(clause.m_join->GetFullName(), otherClause.m_join->GetFullName()) && clause.m_joinAlias.Equals(otherClause.m_joinAlias)
                        && 0 == strcmp(clause.m_using->GetFullName(), otherClause.m_using->GetFullName()) && clause.m_usingAlias.Equals(otherClause.m_usingAlias)
                        && clause.m_isForward == otherClause.m_isForward && clause.m_isPolymorphic == otherClause.m_isPolymorphic && clause.m_isOuterJoin == otherClause.m_isOuterJoin)
                        {
                        foundClause = true;
                        break;
                        }
                    }
                if (!foundClause)
                    {
                    foundAllGroupClauses = false;
                    break;
                    }
                }

            if (foundAllGroupClauses)
                {
                foundGroup = true;
                break;
                }
            }
        if (!foundGroup)
            return false;
        }

    // WHERE
    if (!m_whereClause.Equals(other.m_whereClause))
        return false;
    if (m_whereClauseBindings.size() != other.m_whereClauseBindings.size())
        return false;
    for (BoundQueryValue const* value : m_whereClauseBindings)
        {
        bool found = false;
        for (BoundQueryValue const* otherValue : other.m_whereClauseBindings)
            {
            if (value->Equals(*otherValue))
                {
                found = true;
                break;
                }
            }
        if (!found)
            return false;
        }
               
    // LIMIT
    if (!AreEqual(m_limit, other.m_limit))
        return false;
    if (!AreEqual(m_offset, other.m_offset))
        return false;

    // GROUP BY
    if (!CreateGroupByClause().Equals(other.CreateGroupByClause()))
        return false;
    
    // HAVING
    if (!m_havingClause.Equals(other.m_havingClause))
        return false;

    // ORDER BY
    if (!m_orderByClause.Equals(other.m_orderByClause))
        return false;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bvector<Utf8CP> ComplexPresentationQuery<TBase>::_GetSelectAliases(int flags) const
    {
    bvector<Utf8CP> list;

    if (0 != (IQueryInfoProvider::SELECTION_SOURCE_From & flags))
        {
        for (FromClause const& from : m_from)
            {
            if (!from.m_alias.empty())
                list.push_back(from.m_alias.c_str());
            }
        }

    if (0 != (IQueryInfoProvider::SELECTION_SOURCE_Join & flags))
        {
        for (bvector<JoinUsingClause> const& joinPath : m_joins)
            {
            for (JoinUsingClause const& join : joinPath)
                {
                if (!join.m_joinAlias.empty())
                    list.push_back(join.m_joinAlias.c_str());
                }
            }
        }

    return list;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
BoundQueryValuesList ComplexPresentationQuery<TBase>::_GetBoundValues() const
    {
    BoundQueryValuesList values;
    if (m_nestedQuery.IsValid())
        {
        BoundQueryValuesList nestedValues = m_nestedQuery->GetBoundValues();
        values.insert(values.end(), nestedValues.begin(), nestedValues.end());
        }
    values.insert(values.end(), m_whereClauseBindings.begin(), m_whereClauseBindings.end());
    values.insert(values.end(), m_havingClauseBindings.begin(), m_havingClauseBindings.end());
    if (nullptr != m_limit)
        values.push_back(m_limit);
    if (nullptr != m_offset)
        values.push_back(m_offset);
    return values;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ComplexPresentationQuery<TBase>::SetBoundValues(BoundQueryValuesListCR bindings)
    {
    if (m_whereClauseBindings.size() != bindings.size())
        {
        BeAssert(false);
        return;
        }

    for (BoundQueryValue const* value : m_whereClauseBindings)
        DELETE_AND_CLEAR(value);
    m_whereClauseBindings = bindings;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bool UnionPresentationQuery<TBase>::_IsEqual(TBase const& otherBase) const
    {
    if (!TBase::_IsEqual(otherBase))
        return false;

    UnionPresentationQuery const* other = otherBase.AsUnionQuery();
    if (nullptr == other)
        return false;

    return m_orderByClause.Equals(other->m_orderByClause) && AreEqual(m_limit, other->m_limit) && AreEqual(m_offset, other->m_offset)
        && (m_first->IsEqual(*other->m_first) && m_second->IsEqual(*other->m_second)
        || m_first->IsEqual(*other->m_second) && m_second->IsEqual(*other->m_first));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String UnionPresentationQuery<TBase>::_ToString() const
    {
    Utf8String clause;
    if (!m_orderByClause.empty())
        {
        // note: wrapping queries in a subquery is required until the TFS#291221 is fixed
        clause.append("SELECT * FROM (");
        }

    clause.append(m_first->ToString());
    clause.append(" UNION ALL ");
    clause.append(m_second->ToString());
    
    if (!m_orderByClause.empty())
        clause.append(") ORDER BY ").append(m_orderByClause);

    if (nullptr != m_limit)
        {
        clause.append(" LIMIT ?");
        if (nullptr != m_offset)
            clause.append(" OFFSET ?");
        }

    return clause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void UnionPresentationQuery<TBase>::Init()
    {
    TBase::GetResultParametersR().MergeWith(m_first->GetResultParameters());
    TBase::GetResultParametersR().MergeWith(m_second->GetResultParameters());

    m_first->GetResultParametersR() = ResultParameters();
    m_second->GetResultParametersR() = ResultParameters();

    if (nullptr != m_first->AsComplexQuery())
        {
        m_orderByClause = m_first->AsComplexQuery()->GetClause(CLAUSE_OrderBy);
        m_first->AsComplexQuery()->OrderBy("");
        }
    if (nullptr != m_second->AsComplexQuery())
        {
        m_orderByClause = m_second->AsComplexQuery()->GetClause(CLAUSE_OrderBy);
        m_second->AsComplexQuery()->OrderBy("");
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
UnionPresentationQuery<TBase>& UnionPresentationQuery<TBase>::OrderBy(Utf8CP orderByClause)
    {
    TBase::InvalidateQueryString();
    m_orderByClause = orderByClause;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
UnionPresentationQuery<TBase>& UnionPresentationQuery<TBase>::Limit(uint64_t limit, uint64_t offset)
    {
    TBase::InvalidateQueryString();
    DELETE_AND_CLEAR(m_limit);
    m_limit = new BoundQueryECValue(ECValue(limit));
    DELETE_AND_CLEAR(m_offset);
    m_offset = new BoundQueryECValue(ECValue(offset));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
typename TBase::Contract const* UnionPresentationQuery<TBase>::_GetContract(uint64_t contractId) const
    {
    Contract const* contract = m_first->GetContract(contractId);
    if (nullptr != contract)
        return contract;

    contract = m_second->GetContract(contractId);
    if (nullptr != contract)
        return contract;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
typename TBase::Contract const* UnionPresentationQuery<TBase>::_GetGroupingContract() const
    {
    Contract const* contract = m_first->GetGroupingContract();
    if (nullptr != contract)
        return contract;

    contract = m_second->GetGroupingContract();
    if (nullptr != contract)
        return contract;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void UnionPresentationQuery<TBase>::_OnIsOuterQueryValueChanged()
    {
    m_first->SetIsOuterQuery(TBase::IsOuterQuery());
    m_second->SetIsOuterQuery(TBase::IsOuterQuery());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bvector<Utf8CP> UnionPresentationQuery<TBase>::_GetSelectAliases(int flags) const
    {
    bvector<Utf8CP> first = m_first->GetSelectAliases(flags);
    bvector<Utf8CP> second = m_second->GetSelectAliases(flags);
    bvector<Utf8CP> list;
    list.insert(list.end(), first.begin(), first.end());
    list.insert(list.end(), second.begin(), second.end());
    return list;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
BoundQueryValuesList UnionPresentationQuery<TBase>::_GetBoundValues() const
    {
    BoundQueryValuesList lhsValues = m_first->GetBoundValues();
    BoundQueryValuesList rhsValues = m_second->GetBoundValues();
    BoundQueryValuesList values;
    values.reserve(lhsValues.size() + rhsValues.size() + 2);
    values.insert(values.end(), lhsValues.begin(), lhsValues.end());
    values.insert(values.end(), rhsValues.begin(), rhsValues.end());
    if (nullptr != m_limit)
        values.push_back(m_limit);
    if (nullptr != m_offset)
        values.push_back(m_offset);
    return values;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bool ExceptPresentationQuery<TBase>::_IsEqual(TBase const& otherBase) const
    {
    if (!TBase::_IsEqual(otherBase))
        return false;

    ExceptPresentationQuery const* other = otherBase.AsExceptQuery();
    if (nullptr == other)
        return false;

    return m_orderByClause.Equals(other->m_orderByClause) && AreEqual(m_limit, other->m_limit) && AreEqual(m_offset, other->m_offset)
        && m_base->IsEqual(*other->m_base) && m_except->IsEqual(*other->m_except);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String ExceptPresentationQuery<TBase>::_ToString() const
    {
    Utf8PrintfString clause("%s EXCEPT %s", m_base->ToString().c_str(), m_except->ToString().c_str());
    
    if (!m_orderByClause.empty())
        {
        // note: wrapping queries in a subquery is required until the TFS#291221 is fixed
        clause = Utf8PrintfString("SELECT * FROM (%s EXCEPT %s) this ORDER BY %s",
            m_base->ToString().c_str(), m_except->ToString().c_str(), m_orderByClause.c_str());
        }

    if (nullptr != m_limit)
        {
        clause.append(" LIMIT ?");
        if (nullptr != m_offset)
            clause.append(" OFFSET ?");
        }

    return clause;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ExceptPresentationQuery<TBase>::Init()
    {
    TBase::GetResultParametersR().MergeWith(m_base->GetResultParameters());
    
    m_base->GetResultParametersR() = ResultParameters();
    m_except->GetResultParametersR() = ResultParameters();

    if (nullptr != m_base->AsComplexQuery())
        {
        m_orderByClause = m_base->AsComplexQuery()->GetClause(CLAUSE_OrderBy);
        m_base->AsComplexQuery()->OrderBy("");
        }
    if (nullptr != m_except->AsComplexQuery())
        {
        m_orderByClause = m_except->AsComplexQuery()->GetClause(CLAUSE_OrderBy);
        m_except->AsComplexQuery()->OrderBy("");
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ExceptPresentationQuery<TBase>& ExceptPresentationQuery<TBase>::OrderBy(Utf8CP orderByClause)
    {
    TBase::InvalidateQueryString();
    m_orderByClause = orderByClause;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
ExceptPresentationQuery<TBase>& ExceptPresentationQuery<TBase>::Limit(uint64_t limit, uint64_t offset)
    {
    TBase::InvalidateQueryString();
    DELETE_AND_CLEAR(m_limit);
    m_limit = new BoundQueryECValue(ECValue(limit));
    DELETE_AND_CLEAR(m_offset);
    m_offset = new BoundQueryECValue(ECValue(offset));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
typename TBase::Contract const* ExceptPresentationQuery<TBase>::_GetContract(uint64_t contractId) const
    {
    Contract const* contract = m_base->GetContract(contractId);
    if (nullptr != contract)
        return contract;

    contract = m_except->GetContract(contractId);
    if (nullptr != contract)
        return contract;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
typename TBase::Contract const* ExceptPresentationQuery<TBase>::_GetGroupingContract() const
    {
    Contract const* contract = m_base->GetGroupingContract();
    if (nullptr != contract)
        return contract;

    contract = m_except->GetGroupingContract();
    if (nullptr != contract)
        return contract;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void ExceptPresentationQuery<TBase>::_OnIsOuterQueryValueChanged()
    {
    m_base->SetIsOuterQuery(TBase::IsOuterQuery());
    m_except->SetIsOuterQuery(TBase::IsOuterQuery());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bvector<Utf8CP> ExceptPresentationQuery<TBase>::_GetSelectAliases(int flags) const
    {
    bvector<Utf8CP> first = m_base->GetSelectAliases(flags);
    bvector<Utf8CP> second = m_except->GetSelectAliases(flags);
    bvector<Utf8CP> list;
    list.insert(list.end(), first.begin(), first.end());
    list.insert(list.end(), second.begin(), second.end());
    return list;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
BoundQueryValuesList ExceptPresentationQuery<TBase>::_GetBoundValues() const
    {
    BoundQueryValuesList lhsValues = m_base->GetBoundValues();
    BoundQueryValuesList rhsValues = m_except->GetBoundValues();
    BoundQueryValuesList values;
    values.reserve(lhsValues.size() + rhsValues.size() + 2);
    values.insert(values.end(), lhsValues.begin(), lhsValues.end());
    values.insert(values.end(), rhsValues.begin(), rhsValues.end());
    if (nullptr != m_limit)
        values.push_back(m_limit);
    if (nullptr != m_offset)
        values.push_back(m_offset);
    return values;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
bool StringPresentationQuery<TBase>::_IsEqual(TBase const& otherBase) const
    {
    if (!TBase::_IsEqual(otherBase))
        return false;

    StringPresentationQuery const* other = otherBase.AsStringQuery();
    if (nullptr == other)
        return false;

    return m_query.Equals(other->m_query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
Utf8String StringPresentationQuery<TBase>::_ToString() const {return m_query;}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TBase>
void StringPresentationQuery<TBase>::CopyBindings(BoundQueryValuesListCR bindings)
    {
    for (BoundQueryValue const* binding : bindings)
        m_bindings.push_back(binding->Clone());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryResultParameters::MergeWith(NavigationQueryResultParameters const& other)
    {
    if (other.GetResultType() != NavigationQueryResultType::Invalid)
        {
        BeAssert(GetResultType() == NavigationQueryResultType::Invalid || GetResultType() == other.GetResultType());
        SetResultType(other.GetResultType());
        }

    GetNavNodeExtendedDataR().MergeWith(other.GetNavNodeExtendedData());

    if (other.HasInstanceGroups())
        SetHasInstanceGroups(true);

    BeAssert(nullptr == m_specification || nullptr == other.m_specification
        || m_specification == other.m_specification);
    if (nullptr == m_specification)
        m_specification = other.m_specification;

    for (ECClassId relationshipId : other.m_matchingRelationshipIds)
        m_matchingRelationshipIds.insert(relationshipId);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavigationQueryResultParameters::operator==(NavigationQueryResultParameters const& other) const
    {
    return m_resultType == other.m_resultType
        && m_hasInstanceGroups == other.m_hasInstanceGroups
        && m_navNodeExtendedData == other.m_navNodeExtendedData
        && m_specification == other.m_specification
        && m_matchingRelationshipIds == other.m_matchingRelationshipIds;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryExtendedData::AddRangesData(ECPropertyCR prop, PropertyGroupCR spec)
    {
    if (spec.GetRanges().empty())
        return;

    if (!prop.GetIsPrimitive())
        {
        BeAssert(false && "Expecting only primitive properties");
        return;
        }
    
    rapidjson::Value ranges(rapidjson::kArrayType);
    for (PropertyRangeGroupSpecificationCP rangeSpec : spec.GetRanges())
        {
        rapidjson::Value range(rapidjson::kObjectType);
        PrimitiveType type = prop.GetAsPrimitiveProperty()->GetType();
        range.AddMember("from", ValueHelpers::GetJsonFromString(type, rangeSpec->GetFromValue(), &GetAllocator()), GetAllocator());
        range.AddMember("to", ValueHelpers::GetJsonFromString(type, rangeSpec->GetToValue(), &GetAllocator()), GetAllocator());
        if (!rangeSpec->GetImageId().empty())
            range.AddMember("imageId", rapidjson::StringRef(rangeSpec->GetImageId().c_str()), GetAllocator());
        if (!rangeSpec->GetLabel().empty())
            range.AddMember("label", rapidjson::StringRef(rangeSpec->GetLabel().c_str()), GetAllocator());
        ranges.PushBack(range, GetAllocator());
        }
    AddMember(NAVIGATIONQUERY_EXTENDEDDATA_Ranges, std::move(ranges));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int NavigationQueryExtendedData::GetRangeIndex(DbValue const& dbValue) const
    {
    if (!HasRangesData())
        {
        BeAssert(false);
        return -1;
        }

    RapidJsonValueCR ranges = GetJson()[NAVIGATIONQUERY_EXTENDEDDATA_Ranges];
    for (rapidjson::SizeType i = 0; i < ranges.Size(); i++)
        {
        if (ranges[i].HasMember("from") && ranges[i].HasMember("to"))
            {
            RapidJsonValueCR from = ranges[i]["from"];
            RapidJsonValueCR to = ranges[i]["to"];
            BeAssert(from.GetType() == to.GetType());
            switch (from.GetType())
                {
                case rapidjson::kFalseType:
                case rapidjson::kTrueType:
                    {
                    bool value = (dbValue.GetValueInt() != 0);
                    if (from.GetBool() <= value && value <= to.GetBool())
                        return i;
                    break;
                    }
                case rapidjson::kNumberType:
                    {
                    BeAssert(from.IsUint64() || from.IsInt64() || from.IsDouble());
                    if (from.IsUint64() && from.GetUint64() <= (uint64_t)dbValue.GetValueInt64() && (uint64_t)dbValue.GetValueInt64() <= to.GetUint64())
                        return i;
                    if (from.IsInt64() && from.GetInt64() <= dbValue.GetValueInt64() && dbValue.GetValueInt64() <= to.GetInt64())
                        return i;
                    if (from.IsDouble() && from.GetDouble() <= dbValue.GetValueDouble() && dbValue.GetValueDouble() <= to.GetDouble())
                        return i;
                    break;
                    }
                case rapidjson::kStringType:
                    {
                    Utf8CP value = dbValue.GetValueText();
                    if (strcmp(from.GetString(), value) <= 0 && strcmp(value, to.GetString()) <= 0)
                        return i;
                    break;
                    }
                default:
                    BeAssert(false && "Unhandled value type");
                }

            }
        }
    return -1;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavigationQueryExtendedData::GetRangeLabel(int rangeIndex) const
    {
    RapidJsonValueCR ranges = GetJson()[NAVIGATIONQUERY_EXTENDEDDATA_Ranges];
    if (!ranges.IsArray() || rangeIndex < 0 || rangeIndex >= (int)ranges.Size())
        {
        BeAssert(false);
        return "";
        }

    RapidJsonValueCR range = ranges[rangeIndex];
    if (range.HasMember("label"))
        return range["label"].GetString();

    return ValueHelpers::GetJsonAsString(range["from"])
        .append(" - ").append(ValueHelpers::GetJsonAsString(range["to"]));
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP NavigationQueryExtendedData::GetRangeImageId(int rangeIndex) const
    {
    RapidJsonValueCR ranges = GetJson()[NAVIGATIONQUERY_EXTENDEDDATA_Ranges];
    if (!ranges.IsArray() || rangeIndex < 0 || rangeIndex >= (int)ranges.Size())
        {
        BeAssert(false);
        return "";
        }

    RapidJsonValueCR range = ranges[rangeIndex];
    return range.HasMember("imageId") ? range["imageId"].GetString() : "";
    }

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct RapidJsonValueComparer
{
    bool operator() (rapidjson::Value const* left, rapidjson::Value const* right) const
        {
        switch (left->GetType())
            {
            case rapidjson::kNumberType:
                {
                BeAssert(left->IsInt() || left->IsInt64() || left->IsDouble());
                if (left->IsInt())
                    return left->GetInt() < right->GetInt();
                if (left->IsInt64())
                    return left->GetInt64() < right->GetInt64();
                if (left->IsDouble())
                    return (fabs(left->GetDouble() - right->GetDouble()) > 0.0000001 && (left->GetDouble() - right->GetDouble()) < 0);
                }
            case rapidjson::kObjectType:
                {
                rapidjson::StringBuffer firstBuffer;
                rapidjson::StringBuffer secondBuffer;

                rapidjson::Writer<rapidjson::StringBuffer> firstWriter(firstBuffer);
                rapidjson::Writer<rapidjson::StringBuffer> secondWriter(secondBuffer);

                left->Accept(firstWriter);
                right->Accept(secondWriter);

                return 0 > strcmp(firstBuffer.GetString(), secondBuffer.GetString());
                }
            }
        BeAssert(false);
        return false;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct RapidJsonValueSet : BeSQLite::VirtualSet
{
private:
    PrimitiveType m_type;
    rapidjson::Document m_jsonValues;
    bset<rapidjson::Value*, RapidJsonValueComparer> m_keys;
public:
    RapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type) : m_type(type)
        {
        m_jsonValues.CopyFrom(values, m_jsonValues.GetAllocator());
        for (rapidjson::SizeType i = 0; i < m_jsonValues.Size(); i++)
            m_keys.insert(&m_jsonValues[i]);
        }
    RapidJsonValueSet(RapidJsonValueSet const& other) 
        : RapidJsonValueSet(other.m_jsonValues, other.m_type)
        {}
    bool Equals(RapidJsonValueSet const& otherSet) const
        {
        return m_jsonValues == otherSet.m_jsonValues;
        }
    bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const override
        {
        BeAssert(1 == nVals);
        rapidjson::Document jsonValue;

        switch (m_type)
            {
            case PRIMITIVETYPE_Double:
            case PRIMITIVETYPE_DateTime:
                jsonValue.SetDouble(vals[0].GetValueDouble());               
                break;
            case PRIMITIVETYPE_Integer:
                jsonValue.SetInt(vals[0].GetValueInt());
                break;
            case PRIMITIVETYPE_Long:
                jsonValue.SetInt64(vals[0].GetValueInt64());
                break;
            case PRIMITIVETYPE_Point2d:
            case PRIMITIVETYPE_Point3d:
                jsonValue.Parse(vals[0].GetValueText());
                break;
            default:
                BeAssert(false);
                return false;
            }

        return (m_keys.end() != m_keys.find(&jsonValue));
        }
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BoundRapidJsonValueSet::BoundRapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type)
    {
    m_set = new RapidJsonValueSet(values, type);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BoundRapidJsonValueSet::BoundRapidJsonValueSet(BoundRapidJsonValueSet const& other)
    {
    RapidJsonValueSet const* otherVirtualSet = static_cast<RapidJsonValueSet const*>(other.m_set);
    m_set = new RapidJsonValueSet(*otherVirtualSet);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BoundRapidJsonValueSet::~BoundRapidJsonValueSet()
    {
    DELETE_AND_CLEAR(m_set);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatus BoundRapidJsonValueSet::_Bind(ECSqlStatement& stmt, uint32_t index) const
    {
    if (nullptr == m_set)
        {
        BeAssert(false);
        return ECSqlStatus::Error;
        }
    return stmt.BindVirtualSet((int)index, *m_set);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Saulius.Skliutas               06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool BoundRapidJsonValueSet::_Equals(BoundQueryValue const& other) const
    {
    BoundRapidJsonValueSet const* otherVirtualSet = dynamic_cast<BoundRapidJsonValueSet const*>(&other);
    if (nullptr == otherVirtualSet)
        return false;

    RapidJsonValueSet const* firstSet = static_cast<RapidJsonValueSet const*>(m_set);
    RapidJsonValueSet const* secondSet = static_cast<RapidJsonValueSet const*>(otherVirtualSet->m_set);
    return firstSet->Equals(*secondSet);
    }

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
    INSTANTIATE_QUERY_SUBCLASS(NavigationQuery,,)
    INSTANTIATE_QUERY_SUBCLASS(ContentQuery,,)
    INSTANTIATE_QUERY_SUBCLASS(GenericQuery,,)
END_BENTLEY_ECPRESENTATION_NAMESPACE
